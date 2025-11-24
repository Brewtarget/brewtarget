#!/usr/bin/env python3
#-----------------------------------------------------------------------------------------------------------------------
# scripts/buildTool.py is part of Brewtarget, and is copyright the following authors 2022-2025:
#   • Matt Young <mfsy@yahoo.com>
#
# Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# This Python script is intended to be invoked by the `bt` bash script in the parent directory.  See comments in that
# script for why.
#
# .:TODO:. We should probably also break this file up into several smaller ones!
#
# Note that Python allows both single and double quotes for delimiting strings.  In Meson, we need single quotes, in
# C++, we need double quotes.  We mostly try to use single quotes below for consistency with Meson, except where using
# double quotes avoids having to escape a single quote.
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# This needs to be imported first to do some more bootstrapping
#-----------------------------------------------------------------------------------------------------------------------
import btInitVenv

#-----------------------------------------------------------------------------------------------------------------------
# Python built-in modules we use
#-----------------------------------------------------------------------------------------------------------------------
import argparse
import datetime
import glob
import logging
import os
import pathlib
import platform
import re
import shutil
import stat
import subprocess
import sys
import packaging.version
import tomlkit

#-----------------------------------------------------------------------------------------------------------------------
# Our own modules
#-----------------------------------------------------------------------------------------------------------------------
import btUtils
import btDependencies
import btLogger
import btExecute
import btFileSystem
import btAppImage

#-----------------------------------------------------------------------------------------------------------------------
# Global constants
#-----------------------------------------------------------------------------------------------------------------------
# There is some inevitable duplication with constants in meson.build, but we try to keep it to a minimum
projectName = 'brewtarget'
capitalisedProjectName = projectName.capitalize()
projectUrl = 'https://github.com/' + capitalisedProjectName + '/' + projectName + '/'
exe_python = shutil.which('python3')

# By default we'll log at logging.INFO, but this can be overridden via the -v and -q command line options -- see below
log = btLogger.getLogger()

#-----------------------------------------------------------------------------------------------------------------------
# Welcome banner and environment info
#-----------------------------------------------------------------------------------------------------------------------
# The '%c' argument to strftime means "Locale’s appropriate date and time representation"
log.info(
   '⭐ ' + capitalisedProjectName + ' Build Tool (bt), invoked as "' + ' '.join(sys.argv) + '" starting run on ' +
   platform.system() + ' (' + platform.release() + '), using Python ' + platform.python_version() + ' from ' +
   exe_python + ', with command line arguments, at ' + datetime.datetime.now().strftime('%c') + ' ⭐'
)

#-----------------------------------------------------------------------------------------------------------------------
# Parse command line arguments
#-----------------------------------------------------------------------------------------------------------------------
# We do this (nearly) first as we want the program to exit straight away if incorrect arguments are specified
# Choosing which action to call is done a the end of the script, after all functions are defined
#
# Using Python argparse saves us writing a lot of boilerplate, although the help text it generates on the command line
# is perhaps a bit more than we want (eg having to separate 'bt --help' and 'bt setup --help' is overkill for us).
# There are ways around this -- eg see
# https://stackoverflow.com/questions/20094215/argparse-subparser-monolithic-help-output -- but they are probably more
# complexity than is merited here.
#
parser = argparse.ArgumentParser(
   prog = 'bt',
   description = capitalisedProjectName + ' build tool.  A utility to help with installing dependencies, Git ' +
                 'setup, Meson build configuration and packaging.',
   epilog = 'See ' + projectUrl + ' for info and latest releases'
)

# Log level
group = parser.add_mutually_exclusive_group()
group.add_argument('-v', '--verbose', action = 'store_true', help = 'Enable debug logging of this script')
group.add_argument('-q', '--quiet',   action = 'store_true', help = 'Suppress info logging of this script')

# Per https://docs.python.org/3/library/argparse.html#sub-commands, you use sub-parsers for sub-commands.
subparsers = parser.add_subparsers(
   dest = 'subCommand',
   required = True,
   title = 'action',
   description = "Exactly one of the following actions must be specified.  (For actions marked ✴, specify -h or "
                 "--help AFTER the action for info about options -- eg '%(prog)s setup --help'.)"
)

# Parser for 'setup'
parser_setup = subparsers.add_parser('setup', help = '✴ Set up meson build directory (mbuild) and git options')
subparsers_setup = parser_setup.add_subparsers(dest = 'setupOption', required = False)
parser_setup_all = subparsers_setup.add_parser(
   'all',
   help = 'Specifying this will also automatically install libraries and frameworks we depend on'
)

# Parser for 'package'
parser_package = subparsers.add_parser('package', help='Build a distributable installer')

# Parser for 'appimage'
parser_appimage = subparsers.add_parser('appimage', help='Build an appimage distributable for Linux')

#
# Process the arguments for use below
#
# This try/expect ensures that help is printed if the script is invoked without arguments.  It's not perfect as you get
# the usage line twice (because parser.parse_args() outputs it to stderr before throwing SystemExit) but it's good
# enough for now at least.
#
try:
   args = parser.parse_args()
except SystemExit as se:
   if (se.code != None and se.code != 0):
      parser.print_help()
   sys.exit(0)

#
# The one thing we do set straight away is log level
# Possible levels are 'CRITICAL', 'ERROR', 'WARNING', 'INFO', 'DEBUG', 'NOTSET'.  We choose 'INFO' for default, 'DEBUG'
# for verbose and 'WARNING' for quiet.  You wouldn't want to suppress warnings, would you? :-)
#
if (args.verbose):
   log.setLevel(logging.DEBUG)
elif (args.quiet):
   log.setLevel(logging.WARNING)

log.debug('Parsed command line arguments as ' + str(args))

#-----------------------------------------------------------------------------------------------------------------------
# Note the working directory from which we were invoked -- though it shouldn't matter as we try to be independent of
# this
#-----------------------------------------------------------------------------------------------------------------------
log.debug('Working directory when invoked: ' + pathlib.Path.cwd().as_posix())

#-----------------------------------------------------------------------------------------------------------------------
# Standard Directories
#-----------------------------------------------------------------------------------------------------------------------
btFileSystem.setGlobalDirVars()

#-----------------------------------------------------------------------------------------------------------------------
# Copy a file, removing comments and folded lines
#
# Have had various problems with comments in debian package control file, even though they are theoretically allowed, so
# we strip them out here, hence slightly more involved code than just
#    shutil.copy2(btFileSystem.dir_build.joinpath('control'), dir_packages_deb_control)
#
# Similarly, some of the fields in the debian control file that we want to split across multiple lines are not actually
# allowed to be so "folded" by the Debian package generator.  So, we do our own folding here.  (At the same time, we
# remove extra spaces that make sense on the unfolded line but not once everything is joined onto single line.)
#-----------------------------------------------------------------------------------------------------------------------
def copyWithoutCommentsOrFolds(inputPath, outputPath):
   with open(inputPath, 'r') as inputFile, open(outputPath, 'w') as outputFile:
      for line in inputFile:
         if (not line.startswith('#')):
            if (not line.endswith('\\\n')):
               outputFile.write(line)
            else:
               foldedLine = ""
               while (line.endswith('\\\n')):
                  foldedLine += line.removesuffix('\\\n')
                  line = next(inputFile)
               foldedLine += line
               # The split and join here is a handy trick for removing repeated spaces from the line without
               # fumbling around with regular expressions.  Note that this takes the newline off the end, hence
               # why we have to add it back manually.
               outputFile.write(' '.join(foldedLine.split()))
               outputFile.write('\n')
   return

#-----------------------------------------------------------------------------------------------------------------------
# Create fileToDistribute.sha256sum for a given fileToDistribute in a given directory
#-----------------------------------------------------------------------------------------------------------------------
def writeSha256sum(directory, fileToDistribute):
   #
   # In Python 3.11 we could use the file_digest() function from the hashlib module to do this.  But it's rather
   # more work to do in Python 3.10, so we just use the `sha256sum` command instead.
   #
   # Note however, that `sha256sum` includes the supplied directory path of a file in its output.  We want just the
   # filename, not its full or partial path on the build machine.  So we change into the directory of the file before
   # running the `sha256sum` command.
   #
   previousWorkingDirectory = pathlib.Path.cwd().as_posix()
   os.chdir(directory)
   with open(directory.joinpath(fileToDistribute + '.sha256sum').as_posix(),'w') as sha256File:
      btExecute.abortOnRunFail(
         subprocess.run(['sha256sum', fileToDistribute],
                        capture_output=False,
                        stdout=sha256File)
      )
   os.chdir(previousWorkingDirectory)
   return

#-----------------------------------------------------------------------------------------------------------------------
# ./bt setup
#-----------------------------------------------------------------------------------------------------------------------
def doSetup(setupOption):
   if (setupOption == 'all'):
      btDependencies.installDependencies()

   btUtils.findMesonAndGit()

   # If this is a git checkout then let's set up git with the project standards
   if (btFileSystem.dir_gitInfo.is_dir()):
      log.info('Setting up ' + capitalisedProjectName + ' git preferences')
      # Enforce indentation with spaces, not tabs.
      btExecute.abortOnRunFail(
         subprocess.run(
            [btUtils.exe_git,
               "config",
               "--file", btFileSystem.dir_gitInfo.joinpath('config').as_posix(),
               "core.whitespace",
               "tabwidth=3,tab-in-indent"],
            capture_output=False
         )
      )

      # Enable the standard pre-commit hook that warns you about whitespace errors
      shutil.copy2(btFileSystem.dir_gitInfo.joinpath('hooks/pre-commit.sample'),
                   btFileSystem.dir_gitInfo.joinpath('hooks/pre-commit'))

      btUtils.ensureSubmodulesPresent()

   # Check whether Meson build directory is already set up.  (Although nothing bad happens, if you run setup twice,
   # it complains and tells you to run configure.)
   # Best clue that set-up has been run (rather than, say, user just created empty mbuild directory by hand) is the
   # presence of meson-info/meson-info.json (which is created by setup for IDE integration -- see
   # https://mesonbuild.com/IDE-integration.html#ide-integration)
   runMesonSetup = True
   warnAboutCurrentDirectory = False
   if (btFileSystem.dir_build.joinpath('meson-info/meson-info.json').is_file()):
      log.info('Meson build directory ' + btFileSystem.dir_build.as_posix() + ' appears to be already set up')
      #
      # You usually only need to reset things after you've done certain edits to defaults etc in meson.build.  There
      # are a whole bunch of things you can control with the 'meson configure' command, but it's simplest in some ways
      # just to reset the build directory and rely on meson setup picking up defaults from meson.build.
      #
      # Note that we don't have to worry about this prompt appearing in a GitHub action, because we are always creating
      # the mbuild directory for the first time when this script is run in such actions -- ie we should never reach this
      # part of the code.
      #
      response = ""
      while (response != 'y' and response != 'n'):
         response = input('Do you want to completely reset the build directory? [y or n] ').lower()
      if (response == 'n'):
         runMesonSetup = False
      else:
         # It's very possible that the user's current working directory is mbuild.  If so, we need to warn them and move
         # up a directory (as 'meson setup' gets upset if current working directory does not exist).
         log.info('Removing existing Meson build directory ' + btFileSystem.dir_build.as_posix())
         if (pathlib.Path.cwd().as_posix() == btFileSystem.dir_build.as_posix()):
            # We write a warning out here for completeness, but we really need to show it further down as it will have
            # scrolled off the top of the terminal with all the output from 'meson setup'
            log.warning('You are currently in the directory we are about to delete.  ' +
                        'You will need to change directory!')
            warnAboutCurrentDirectory = True
            os.chdir(btFileSystem.dir_base)
         shutil.rmtree(btFileSystem.dir_build)

   if (runMesonSetup):
      log.info('Setting up ' + btFileSystem.dir_build.as_posix() + ' meson build directory')
      # See https://mesonbuild.com/Commands.html#setup for all the optional parameters to meson setup
      # Note that meson setup will create the build directory (along with various subdirectories)
      btExecute.abortOnRunFail(subprocess.run([btUtils.exe_meson, "setup", btFileSystem.dir_build.as_posix(), btFileSystem.dir_base.as_posix()],
                                            capture_output=False))

      log.info('Finished setting up Meson build.  Note that the warnings above about path separator and optimization ' +
               'level are expected!')

   if (warnAboutCurrentDirectory):
      print("❗❗❗ Your current directory has been deleted!  You need to run 'cd ../mbuild' ❗❗❗")
   log.debug('Setup done')
   log.debug('PATH=' + os.environ["PATH"])
   print()
   print('You can now build, test, install and run ' + capitalisedProjectName + ' with the following commands:')
   print('   cd ' + os.path.relpath(btFileSystem.dir_build))
   print('   meson compile')
   print('   meson test')
   if (platform.system() == 'Linux'):
      print('   sudo meson install')
   else:
      print('   meson install')
   print('   ' + projectName)


   return

#-----------------------------------------------------------------------------------------------------------------------
# ./bt package
#-----------------------------------------------------------------------------------------------------------------------
def doPackage():
   #
   # Meson does not offer a huge amount of help on creating installable packages.  It has no equivalent to CMake's CPack
   # and there is generally not a lot of info out there about how to do packaging in Meson.  In fact, it seems unlikely
   # that packaging will ever come within it scope.  (Movement is rather in the other direction - eg there _used_ to be
   # a Meson module for creating RPMs, but it was discontinued per
   # https://mesonbuild.com/Release-notes-for-0-62-0.html#removal-of-the-rpm-module because it was broken and badly
   # designed etc.)
   #
   # At first, this seemed disappointing, but I've rather come around to thinking a different way about it.  Although
   # CPack has lots of features, it is also very painful to use.  Some of the things you can do are undocumented; some
   # of the things you want to be able to do seem nigh on impossible.  So perhaps taking a completely different
   # approach, eg using a scripting language rather than a build tool to do packaging, is ultimately a good thing.
   #
   # I spent some time looking at and trying to use the Qt-Installer-Framework (QtIFW).  Upsides are:
   #   - In principle we could write one set of install config that would then create install packages for Windows, Mac
   #     and Linux.
   #   - It should already know how to package Qt libraries(!)
   #   - It's the same licence as the rest of Qt.
   #   - We could use it in GitHub actions (courtesy of https://github.com/jurplel/install-qt-action).
   #   - It can handle in-place upgrades (including the check for whether an upgraded version is available), per
   #     https://doc.qt.io/qtinstallerframework/ifw-updates.html.
   # Downsides are:
   #   - Outside of packaging Qt itself, I'm not sure that it's hugely widely used.  It can be hard to find "how tos" or
   #     other assistance.
   #   - It's not a great advert for itself -- eg when I installed it locally on Kubuntu by downloading directly from
   #     https://download.qt.io/official_releases/qt-installer-framework/, it didn't put its own tools in the PATH,
   #     so I had to manually add ~/Qt/QtIFW-4.5.0/bin/ to my PATH.
   #   - It usually necessary to link against a static build of Qt, which is a bit of a pain as you have to download the
   #     source files for Qt and compile it locally -- see eg
   #     https://stackoverflow.com/questions/14932315/how-to-compile-qt-5-under-windows-or-linux-32-or-64-bit-static-or-dynamic-on-v
   #     for the whole process.
   #   - It's a change of installation method for people who have previously downloaded deb packages, RPMs, Mac DMG
   #     files, etc.
   #   - It puts things in different places than 'native' installers.  Eg, on Linux, everything gets installed to
   #     subdirectories of the user's home directory rather than the "usual" system directories).  Amongst other things,
   #     this makes it harder for distros etc that want to ship our software as "standard" packages.
   #
   # The alternative approach, which I resisted for a fair while, but have ultimately become persuaded is right, is to
   # do Windows, Mac and Linux packaging separately:
   #   - For Mac, there is some info at https://mesonbuild.com/Creating-OSX-packages.html on creating app bundles
   #   - For Linux, there is some mention in the Meson manual of building deb and rpm packages eg
   #     https://mesonbuild.com/Installing.html#destdir-support, but I think you have to do most of the work yourself.
   #     https://blog.devgenius.io/how-to-build-debian-packages-from-meson-ninja-d1c28b60e709 gives some sketchy
   #     starting info on how to build deb packages.  Maybe we could find the equivalent for creating RPMs.  Also look
   #     at https://openbuildservice.org/.
   #   - For Windows, we use NSIS (Nullsoft Scriptable Install System -- see https://nsis.sourceforge.io/) -- to create
   #     a Windows installer.
   #
   # Although a lot of packaging is platform-specific, the initial set-up is generic.
   #
   #    1. This script (as invoked directly) creates some packaging sub-directories of the build directory and then
   #       invokes Meson
   #    2. Meson installs all the binaries, data files and so on that we need to ship into the packaging directory tree
   #    3. Meson also exports a bunch of build information into a TOML file that we read in.  This saves us duplicating
   #       too many meson.build settings in this file.
   #

   btUtils.findMesonAndGit()

   #
   # The top-level directory structure we create inside the build directory (mbuild) for packaging is:
   #
   #    packages/   Holds the subdirectories below, plus the source tarball and its checksum
   #    │
   #    ├── windows/ For Windows
   #    │
   #    ├── darwin/  For Mac
   #    │
   #    ├── linux/   For Linux
   #    │
   #    └── source/   For source code tarball
   #
   # NB: If we wanted to change this, we would need to make corresponding changes in meson.build
   #

   # Step 1 : Create a top-level package directory structure
   #          We'll make the relevant top-level directory and ensure it starts out empty
   #          (We don't have to make dir_packages as it will automatically get created by os.makedirs when we ask it to
   #          create btFileSystem.dir_packages_platform.)
   if (btFileSystem.dir_packages_platform.is_dir()):
      log.info('Removing existing ' + btFileSystem.dir_packages_platform.as_posix() + ' directory tree')
      shutil.rmtree(btFileSystem.dir_packages_platform)
   log.info('Creating directory ' + btFileSystem.dir_packages_platform.as_posix())
   os.makedirs(btFileSystem.dir_packages_platform)

   # We change into the build directory.  This doesn't affect the caller (of this script) because we're a separate
   # sub-process from the (typically) shell that invoked us and we cannot change the parent process's working
   # directory.
   os.chdir(btFileSystem.dir_build)
   log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())

   #
   # Meson can't do binary packaging, but it can create a source tarball for us via `meson dist`.  We use the following
   # options:
   #    --no-tests  = stops Meson doing a full build and test, on the assumption that we've already done this by the
   #                  time we come to packaging
   #    --allow-dirty  = allow uncommitted changes, which is needed in Meson 0.62 and later to prevent Meson emitting a
   #                     fatal error if there are uncommitted changes on the current git branch.  (In previous versions
   #                     of Meson, this was just a warning.)  NOTE that, even with this option specified, uncommitted
   #                     changes will be ignored (ie excluded from the source tarball).
   #
   # Of course, we could create a compressed tarball directly in this script, but the advantage of having Meson do it is
   # that it will (I believe) include only source & data files actually in the git repository in meson.build, so you
   # won't pick up other things that happen to be hanging around in the source etc directory trees.
   #
   log.info('Creating source tarball')
   if (btUtils.mesonVersion >= packaging.version.parse('0.62.0')):
      btExecute.abortOnRunFail(
         subprocess.run([btUtils.exe_meson, 'dist', '--no-tests', '--allow-dirty'], capture_output=False)
      )
   else:
      btExecute.abortOnRunFail(
         subprocess.run([btUtils.exe_meson, 'dist', '--no-tests'], capture_output=False)
      )

   #
   # The compressed source tarball and its checksum end up in the meson-dist subdirectory of mbuild, so we just move
   # them into the packages/source directory (first making sure the latter exists and is empty!).
   #
   # The filename of the compressed tarball etc generated by Meson are always:
   #    [project_name]-[project_version].tar.xz
   #    [project_name]-[project_version].tar.xz.sha256sum
   #
   # We would prefer the names to be:
   #    [project_name]-[project_version]-Source_Code.tar.xz
   #    [project_name]-[project_version]-Source_Code.tar.xz.sha256sum
   #
   # TODO We should do this renaming and regenerate the sha256sum file (so it contains the new filename)
   #
   # We are only talking about 2 files, so some of this is overkill, but it's easier to be consistent with what we do
   # for the other subdirectories of mbuild/packages
   #
   if (btFileSystem.dir_packages_source.is_dir()):
      log.info('Removing existing ' + btFileSystem.dir_packages_source.as_posix() + ' directory tree')
      shutil.rmtree(btFileSystem.dir_packages_source)
   log.info('Creating directory ' + btFileSystem.dir_packages_source.as_posix())
   os.makedirs(btFileSystem.dir_packages_source)
   meson_dist_dir = btFileSystem.dir_build.joinpath('meson-dist')
   for fileName in os.listdir(meson_dist_dir.as_posix()):
      log.debug('Moving ' + fileName + ' from ' + meson_dist_dir.as_posix() + ' to ' + btFileSystem.dir_packages_source.as_posix())
      # shutil.move will error rather than overwrite an existing file, so we handle that case manually (although in
      # theory it should never arise)
      targetFile = btFileSystem.dir_packages_source.joinpath(fileName)
      if os.path.exists(targetFile):
         log.debug('Removing old ' + targetFile)
         os.remove(targetFile)
      shutil.move(meson_dist_dir.joinpath(fileName), btFileSystem.dir_packages_source)

   #
   # Running 'meson install' with the --destdir option will put all the installable files (program executable,
   # translation files, data files, etc) in subdirectories of the platform-specific packaging directory.  However, it
   # will not bundle up any shared libraries that we need to ship with the application on Windows and Mac.  We handle
   # this in the platform-specific code below.
   #
   log.info('Running meson install with --destdir option')
   # See https://mesonbuild.com/Commands.html#install for the optional parameters to meson install
   btExecute.abortOnRunFail(subprocess.run([btUtils.exe_meson, 'install', '--destdir', btFileSystem.dir_packages_platform.as_posix()],
                                 capture_output=False))

   #
   # At the direction of meson.build, Meson should have generated a config.toml file in the build directory that we can
   # read in to get useful settings exported from the build system.
   #
   btUtils.readBuildConfigFile()
   log.debug('Shared libraries: ' + ', '.join(btUtils.buildConfig["CONFIG_SHARED_LIBRARY_PATHS"]))

   #
   # Note however that there are some things that are (often intentionally) difficult or impossible to import to or
   # export from Meson.  (See
   # https://mesonbuild.com/FAQ.html#why-is-meson-not-just-a-python-module-so-i-could-code-my-build-setup-in-python for
   # why it an explicitly design goal not to have the Meson configuration language be Turing-complete.)
   #
   # We deal with some of these in platform-specific code below
   #

   #
   # If meson install worked, we can now do the actual packaging.
   #
   match platform.system():

      #-----------------------------------------------------------------------------------------------------------------
      #------------------------------------------------ Linux Packaging ------------------------------------------------
      #-----------------------------------------------------------------------------------------------------------------
      case 'Linux':
         #
         # There are, of course, multiple package managers in the Linux world.  We cater for two of the main ones,
         # Debian and RPM.
         #
         # Note, per https://en.wikipedia.org/wiki/X86-64, that x86_64 and amd64 are the same thing; the latter is just
         # a rebranding of the former by AMD.  Debian packages use 'amd64' in the filename, while RPM ones use 'x86_64',
         # but it's the same code being packaged and pretty much the same directory structure being installed into.
         #
         # Some of the processing we need to do is the same for Debian and RPM, so do that first before we copy things
         # into separate trees for actually building the packages
         #
         log.debug('Linux Packaging')

         #
         # First, note that Meson is geared up for building and installing locally.  (It doesn't really know about
         # packaging.)  This means it installs locally to /usr/local/bin, /usr/local/share, etc.  This is "correct" for
         # locally-built software but not for packaged software, which needs to go in /usr/bin, /usr/share, etc.  So,
         # inside the mbuild/packages directory tree, we just need to move everything out of linux/usr/local up one
         # level into linux/usr and then remove the empty linux/usr/local directory
         #
         log.debug('Moving usr/local files to usr inside ' + btFileSystem.dir_packages_platform.as_posix())
         targetDir = btFileSystem.dir_packages_platform.joinpath('usr')
         sourceDir = targetDir.joinpath('local')
         for fileName in os.listdir(sourceDir.as_posix()):
            shutil.move(sourceDir.joinpath(fileName), targetDir)
         os.rmdir(sourceDir.as_posix())

         #
         # Debian and RPM both want the debugging information stripped from the executable.
         #
         # .:TBD:. One day perhaps we could be friendly and still ship the debugging info, just in a separate .dbg
         # file.  The procedure to do this is described in the 'only-keep-debug' section of `man objcopy`.  However, we
         # need to work out where to put the .dbg file so that it remains usable but lintian does not complain about it.
         #
         dir_packages_bin = btFileSystem.dir_packages_platform.joinpath('usr').joinpath('bin')
         log.debug('Stripping debug symbols')
         btExecute.abortOnRunFail(
            subprocess.run(
               ['strip',
                '--strip-unneeded',
                '--remove-section=.comment',
                '--remove-section=.note binaries',
                dir_packages_bin.joinpath(projectName)],
               capture_output=False
            )
         )

         #--------------------------------------------------------------------------------------------------------------
         #-------------------------------------------- Debian .deb Package ---------------------------------------------
         #--------------------------------------------------------------------------------------------------------------
         #
         # There are some relatively helpful notes on building debian packages at:
         #    https://unix.stackexchange.com/questions/30303/how-to-create-a-deb-file-manually
         #    https://www.internalpointers.com/post/build-binary-deb-package-practical-guide
         #
         # We skip a lot of things because we are not trying to ship a Debian source package, just a binary one.
         # (Debian wants source packages to be built with an old-fashioned makefile, which seems a bit too painful to
         # me.  Since there are other very easy routes for people to get the source code, I'm not rushing to jump
         # through a lot of hoops to package it up in a .deb file.)
         #
         # Skipping the source package means we don't (and indeed can't) use all the tools that come with dh-make and it
         # means we need to do a tiny bit more manual work in creating some parts of the install tree.  But, overall,
         # the process itself is simple once you've worked out what you need to do (which was slightly more painful than
         # you might have hoped).
         #
         # To create a deb package, we create the following directory structure, where items marked ✅ are copied as is
         # from the tree generated by meson install with --destdir option, and those marked ❇ are ones we need to
         # relocate, generate or modify.
         #
         # (When working on this bit, use ❌ for things that are generated automatically but not actually needed, and ✴
         # for things we still need to add.  Not currently not aware of any of either.)
         #    debbuild
         #    └── [projectName]-[versionNumber]-1_amd64
         #        ├── DEBIAN
         #        │   └── control          ❇  # Contains info about dependencies, maintainer, etc
         #        │
         #        └── usr
         #            ├── bin
         #            │   └── [projectName] ✅   <── the executable
         #            └── share
         #                ├── applications
         #                │   └── [projectName].desktop     ✅  <── [filesToInstall_desktop]
         #                ├── [projectName]
         #                │   ├── DefaultContent001-DefaultData.xml       ✅  <──┬── [filesToInstall_data]
         #                │   ├── DefaultContent002-BJCP_2021_Styles.json ✅  <──┤
         #                │   ├── DefaultContent003-...                   ✅  <──┤
         #                │   ├── ...etc                                  ✅  <──┤
         #                │   ├── default_db.sqlite                       ✅  <──┘
         #                │   ├── sounds
         #                │   │   └── [All the filesToInstall_sounds .wav files] ✅
         #                │   └── translations_qm
         #                │       └── [All the .qm files generated by qt.compile_translations] ✅
         #                ├── doc
         #                │    └── [projectName]
         #                │        ├── changelog.Debian.gz            ✅
         #                │        ├── copyright                      ✅
         #                │        ├── README.md (or README.markdown) ✅
         #                │        └── RelaseNotes.markdown           ✅
         #                ├── icons
         #                │   └── hicolor
         #                │       └── scalable
         #                │           └── apps
         #                │               └── [projectName].svg ✅  <── [filesToInstall_icons]
         #                └── man
         #                    └── man1
         #                        └── [projectName].1.gz ❇ <── English version of man page (compressed)
         #

         # Make the top-level directory for the deb package and the DEBIAN subdirectory for the package control files
         # etc
         log.debug('Creating debian package top-level directories')
         debPackageDirName = projectName + '-' + btUtils.buildConfig['CONFIG_VERSION_STRING'] + '-1_amd64'
         dir_packages_deb = btFileSystem.dir_packages_platform.joinpath('debbuild').joinpath(debPackageDirName)
         dir_packages_deb_control = dir_packages_deb.joinpath('DEBIAN')
         os.makedirs(dir_packages_deb_control) # This will also automatically create parent directories
         dir_packages_deb_doc = dir_packages_deb.joinpath('usr/share/doc').joinpath(projectName)

         # Copy the linux/usr tree inside the top-level directory for the deb package
         log.debug('Copying deb package contents')
         shutil.copytree(btFileSystem.dir_packages_platform.joinpath('usr'), dir_packages_deb.joinpath('usr'))

         #
         # Copy the Debian Binary package control file to where it belongs
         #
         # The meson build will have generated this file from packaging/linux/control.in
         #
         log.debug('Copying deb package control file')
         copyWithoutCommentsOrFolds(btFileSystem.dir_build.joinpath('control').as_posix(),
                                    dir_packages_deb_control.joinpath('control').as_posix())


         #
         # Generate compressed changelog for Debian package from markdown
         #
         # Each Debian package (which provides a /usr/share/doc/pkg directory) must install a Debian changelog file in
         # /usr/share/doc/pkg/changelog.Debian.gz
         #
         # This is done by a shell script because we already wrote that
         #
         log.debug('Generating compressed changelog')
         os.environ['CONFIG_APPLICATION_NAME_LC'    ] = btUtils.buildConfig['CONFIG_APPLICATION_NAME_LC'    ]
         os.environ['CONFIG_CHANGE_LOG_UNCOMPRESSED'] = btUtils.buildConfig['CONFIG_CHANGE_LOG_UNCOMPRESSED']
         os.environ['CONFIG_CHANGE_LOG_COMPRESSED'  ] = dir_packages_deb_doc.joinpath('changelog.Debian.gz').as_posix()
         os.environ['CONFIG_PACKAGE_MAINTAINER'     ] = btUtils.buildConfig['CONFIG_PACKAGE_MAINTAINER'     ]
         btExecute.abortOnRunFail(
            subprocess.run([btFileSystem.dir_base.joinpath('packaging').joinpath('generateCompressedChangeLog.sh')],
                           capture_output=False)
         )
         # Shell script gives wrong permissions on output (which lintian would complain about), so fix them here (from
         # rw-rw-r-- to rw-r--r--).
         os.chmod(dir_packages_deb_doc.joinpath('changelog.Debian.gz'),
                  stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH)

         #
         # Debian packages want man pages to be compressed with gzip with the highest compression available (-9n).
         #
         # TBD: We'll need to expand this slightly when we support man pages in multiple languages.
         #
         # We _could_ do this all in Python with the gzip module, but it's somewhat less coding just to invoke the gzip
         # program directly
         #
         dir_packages_deb_man = dir_packages_deb.joinpath('usr').joinpath('share').joinpath('man')
         dir_packages_deb_man1 = dir_packages_deb_man.joinpath('man1')
         log.debug('Compressing man page')
         btExecute.abortOnRunFail(
            subprocess.run(['gzip', '-9n', dir_packages_deb_man1.joinpath(projectName + '.1')], capture_output=False)
         )

         #
         # Now we actually generate the package
         #
         # Generates the package with the same name as the package directory plus '.deb' on the end
         log.info('Generating deb package')
         previousWorkingDirectory = pathlib.Path.cwd().as_posix()
         os.chdir(btFileSystem.dir_packages_platform.joinpath('debbuild'))
         btExecute.abortOnRunFail(
            subprocess.run(['dpkg-deb', '--build', '--root-owner-group', debPackageDirName], capture_output=False)
         )

         # The debian package name is (I think) derived from the name of the directory we supplied as build parameter
         debPackageName = debPackageDirName + '.deb'

         # Running lintian does a very strict check on the Debian package.  You can find a list of all the error and
         # warning codes at https://lintian.debian.org/tags.
         #
         # Some of the warnings are things that only matter for packages that actually ship with Debian itself - ie they
         # won't stop the package working but are not strictly within the standards that the Debian project sets for the
         # packages included in the distro.
         #
         # Still, we try to fix as many warnings as possible.  As at 2022-08-11 we currently have one warning that we do
         # not ship a man page.  We should get to this at some point.
         log.info('Running lintian to check the created deb package for errors and warnings')
         btExecute.abortOnRunFail(
            subprocess.run(['lintian', '--no-tag-display-limit', debPackageName], capture_output=False)
         )

         # Move the .deb file to the top-level directory
         shutil.move(debPackageName, btFileSystem.dir_packages_platform)

         #
         # If we are on Ubuntu, we would like to edit the package name to include the Ubuntu major version number.
         # This is because we typically release two versions of the Linux packages -- one built on the last Ubuntu LTS
         # release, and one built on the previous one.
         #
         distroInfo = btUtils.getLinuxDistroInfo()
         if ('Ubuntu' == distroInfo["name"]):
            #
            # We want to rename, eg "brewtarget-4.2.1-1_amd64.deb" to "brewtarget-4.2.1_22-1_amd64.deb" or
            # "brewken-1.0.3-1_amd64.deb" to "brewken-1.0.3_22-1_amd64.deb".  This is relatively easy if we can assume
            # there are always two '-' characters.  (So we never want to rename "brewtarget" to "brew-target"! :o>)
            #
            oldDebPackageName = debPackageName
            nameParts = debPackageName.split('-')
            debPackageName = nameParts[0] + '-' + nameParts[1] + '_' + str(distroInfo["major"]) + '-' + nameParts[2]
            os.chdir(btFileSystem.dir_packages_platform)
            log.info('Renaming ' + oldDebPackageName + ' to ' + debPackageName)
            os.rename(oldDebPackageName, debPackageName)

         # We don't particularly need to change back to the previous working directory, but it's tidy to do so.
         os.chdir(previousWorkingDirectory)

         #
         # Make the checksum file
         #
         log.info('Generating checksum file for ' + debPackageName)
         writeSha256sum(btFileSystem.dir_packages_platform, debPackageName)

         #--------------------------------------------------------------------------------------------------------------
         #---------------------------------------------- RPM .rpm Package ----------------------------------------------
         #--------------------------------------------------------------------------------------------------------------
         # This script is written assuming you are on a Debian-based Linux.
         #
         # In theory we can use `alien` to convert a .deb to a .rpm, but I worry that this would not handle dependencies
         # very well.  So we prefer to build a bit more manually.
         #
         # To create a rpm package, we create the following directory structure, where items marked ✅ are copied as is
         # from the tree generated by meson install with --destdir option, and those marked ❇ are ones we either
         # generate or modify.
         #
         # (When working on this bit, use ❌ for things that are generated automatically but not actually needed, and ✴
         # for things we still need to add.  Not currently not aware of any of either.)
         #    rpmbuild
         #    ├── SPECS
         #    │   └── rpm.spec ❇
         #    └── BUILDROOT
         #        └── usr
         #            ├── bin
         #            │   └── [projectName] ✅   <── the executable
         #            ├── lib
         #            │   └── .build-id
         #            └── share
         #                ├── applications
         #                │   └── [projectName].desktop     ✅  <── [filesToInstall_desktop]
         #                ├── [projectName]
         #                │   ├── DefaultData.xml           ✅  <──┬── [filesToInstall_data]
         #                │   ├── default_db.sqlite         ✅  <──┘
         #                │   ├── sounds
         #                │   │   └── [All the filesToInstall_sounds .wav files] ✅
         #                │   └── translations_qm
         #                │       └── [All the .qm files generated by qt.compile_translations] ✅
         #                ├── doc
         #                │    └── [projectName]
         #                │        ├── copyright                      ✅
         #                │        ├── README.md (or README.markdown) ✅
         #                │        └── RelaseNotes.markdown           ✅
         #                ├── icons
         #                │   └── hicolor
         #                │       └── scalable
         #                │           └── apps
         #                │               └── [projectName].svg ✅  <── [filesToInstall_icons]
         #                └── man
         #                    └── man1
         #                        └── [projectName].1.bz2 ❇ <── English version of man page (compressed)
         #
         #

         # Make the top-level directory for the rpm package and the SPECS subdirectory etc
         log.debug('Creating rpm package top-level directories')
         rpmPackageDirName = 'rpmbuild'
         dir_packages_rpm = btFileSystem.dir_packages_platform.joinpath(rpmPackageDirName)
         dir_packages_rpm_specs = dir_packages_rpm.joinpath('SPECS')
         os.makedirs(dir_packages_rpm_specs) # This will also automatically create dir_packages_rpm
         dir_packages_rpm_buildroot = dir_packages_rpm.joinpath('BUILDROOT')
         os.makedirs(dir_packages_rpm_buildroot)

         # Copy the linux/usr tree inside the top-level directory for the rpm package
         log.debug('Copying rpm package contents')
         shutil.copytree(btFileSystem.dir_packages_platform.joinpath('usr'), dir_packages_rpm_buildroot.joinpath('usr'))

         # Copy the RPM spec file, doing the same unfolding etc as for the Debian control file above
         log.debug('Copying rpm spec file')
         copyWithoutCommentsOrFolds(btFileSystem.dir_build.joinpath('rpm.spec').as_posix(),
                                    dir_packages_rpm_specs.joinpath('rpm.spec').as_posix())

         #
         # In Debian packaging, the change log is a separate file.  However, for RPM packaging, the change log needs to
         # be, included in the spec file.  The simplest way to do that is for us to append it to the file we've just
         # copied.  (NB: This relies on the last line of that file being `%changelog` -- ie the macro that introduces
         # the change log.)
         #
         # Since we store our change log internally in markdown, we also convert it to the RPM format at the same time
         # as appending it.  (This is different from the Debian changelog format, so we can't just reuse what we've done
         # above.)  Per https://docs.fedoraproject.org/en-US/packaging-guidelines/#changelogs, the format we need is:
         #    %changelog
         #    * Wed Jun 14 2003 Joe Packager <joe at gmail.com> - 1.0-2
         #    - Added README file (#42).
         # (Note that we don't have to write '%changelog' as it's already in the spec file.)
         # The format we have is:
         #    ## v3.0.2
         #    Minor bug fixes for the 3.0.1 release (ie bugs in 3.0.1 are fixed in this 3.0.2 release).
         #
         #    ### New Features
         #
         #    * None
         #
         #    ### Bug Fixes
         #    * LGPL-2.1-only and LGPL-3.0-only license text not shipped [#664](https://github.com/Brewtarget/brewtarget/issues/664)
         #    * Release 3.0.1 is uninstallable on Ubuntu 22.04.1 [#665](https://github.com/Brewtarget/brewtarget/issues/665)
         #    * Turkish Language selection in settings not working [#670])https://github.com/Brewtarget/brewtarget/issues/670)
         #
         #    ### Release Timestamp
         #    Wed, 26 Oct 2022 10:10:10 +0100
         #
         #    ## v3.0.1
         #    etc
         #
         with open(os.environ['CONFIG_CHANGE_LOG_UNCOMPRESSED'], 'r') as markdownChangeLog, open(dir_packages_rpm_specs.joinpath('rpm.spec'), 'a') as specFile:
            inIntro = True
            releaseDate = ''
            versionNumber = ''
            changes = []
            for line in markdownChangeLog:
               if (inIntro):
                  # Skip over the introductory headings and paragraphs of CHANGES.markdown until we get to the first
                  # version line, which begins with '## v'.
                  if (not line.startswith('## v')):
                     # Skip straight to processing the next line
                     continue
                  # We've reached the end of the introductory stuff, so the current line is the first one that we
                  # process "as normal" below.
                  inIntro = False
               # If this is a version line, it's the start of a change block (and the end of the previous one if there
               # was one).  Grab the version number (and write out the previous block if there was one).  Note that we
               # have to add the '-1' "package release" number on the end of the version number (but before the
               # newline!), otherwise rpmlint will complain about "incoherent-version-in-changelog".
               if (line.startswith('## v')):
                  nextVersionNumber = line.removeprefix('## v').replace('\n', '-1\n')
                  log.debug('Extracted version "' + nextVersionNumber.rstrip() + '" from ' + line.rstrip())
                  if (len(changes) > 0):
                     specFile.write('* ' + releaseDate + ' ' + btUtils.buildConfig['CONFIG_PACKAGE_MAINTAINER'] + ' - ' +
                                    versionNumber)
                     for change in changes:
                        specFile.write('- ' + change)
                     changes = []
                  versionNumber = nextVersionNumber
                  continue
               # If this is a line starting with '* ' then it's either a new feature or a bug fix.  RPM doesn't
               # distinguish, so we just add it to the list, stripping the '* ' off the front.  EXCEPT, if the line
               # says "* None" it probably means this is a release with no new features -- just bug fixes.  So we don't
               # want to include the "* None" line!
               if (line.startswith('* ')):
                  if (line.rstrip() != '* None'):
                     changes.append(line.removeprefix('* '))
                  continue
               # If this line is '### Release Timestamp' then we want to grab the next line as the release timestamp
               if (line.startswith('### Release Timestamp')):
                  #
                  # We need to:
                  #   - take the comma out after the day of the week
                  #   - change date format from "day month year" to "month day year"
                  #   - strip the time off the end of the line
                  #   - strip the newline off the end of the line
                  # We can do all of it all in one regexp with relatively little pain(!).  Note the use of raw string
                  # notation (r prefix on string literal) to avoid the backslash plague (see
                  # https://docs.python.org/3/howto/regex.html#the-backslash-plague).
                  #
                  line = next(markdownChangeLog)
                  releaseDate = re.compile(r', (\d{1,2}) ([A-Z][a-z][a-z]) (\d\d\d\d).*\n$').sub(r' \2 \1 \3', line)
                  log.debug('Extracted date "' + releaseDate + '" from ' + line.rstrip())
                  continue
            # Once we got to the end of the input, we need to write the last change block
            if (len(changes) > 0):
               specFile.write('* ' + releaseDate + ' ' + btUtils.buildConfig['CONFIG_PACKAGE_MAINTAINER'] + ' - ' +
                              versionNumber)
               for change in changes:
                  specFile.write('- ' + change)

         #
         # RPM packages want man pages to be compressed with bzip2.  Other than that, the same comments above for
         # compressing man pages for deb packages apply here.
         #
         dir_packages_rpm_man = dir_packages_rpm_buildroot.joinpath('usr').joinpath('share').joinpath('man')
         dir_packages_rpm_man1 = dir_packages_rpm_man.joinpath('man1')
         log.debug('Compressing man page')
         btExecute.abortOnRunFail(
            subprocess.run(
               ['bzip2', '--compress', dir_packages_rpm_man1.joinpath(projectName + '.1')],
               capture_output=False
            )
         )

         #
         # Run rpmbuild to build the package
         #
         # Again, as with the .deb packaging, we are just trying to build a binary package and not use all the built-in
         # magical makefiles of the full RPM build system.
         #
         # Note, per comments at
         # https://unix.stackexchange.com/questions/553169/rpmbuild-isnt-using-the-current-working-directory-instead-using-users-home
         # that you have to set the _topdir macro to stop rpmbuild wanting to put all its output under the current
         # user's home directory.  Also, we do not put quotes around this define because the subprocess module will do
         # this already (I think) because it works out there's a space in the string.  (If we do put quotes, we get an
         # error "Macro % has illegal name".)
         #
         log.info('Generating rpm package')
         btExecute.abortOnRunFail(
            subprocess.run(
               ['rpmbuild',
                '--define=_topdir ' + dir_packages_rpm.as_posix(),
                '--noclean', # Do not remove the build tree after the packages are made
                '--buildroot',
                dir_packages_rpm_buildroot.as_posix(),
                '--bb',
                dir_packages_rpm_specs.joinpath('rpm.spec').as_posix()],
               capture_output=False
            )
         )

         # rpmbuild will have put its output in RPMS/x86_64/[projectName]-[versionNumber]-1.x86_64.rpm
         dir_packages_rpm_output = dir_packages_rpm.joinpath('RPMS').joinpath('x86_64')
         rpmPackageName = projectName + '-' + btUtils.buildConfig['CONFIG_VERSION_STRING'] + '-1.x86_64.rpm'

         #
         # Running rpmlint is the lintian equivalent exercise for RPMs.  Many, but by no means all, of the error and
         # warning codes are listed at https://fedoraproject.org/wiki/Common_Rpmlint_issues, though there are some
         # mistakes on that page (eg suggestion for dealing with unstripped-binary-or-object warning is "Make sure
         # binaries are executable"!)
         #
         # See packaging/linux/rpmLintfilters.toml for suppression of various rpmlint warnings (with explanations of
         # why).
         #
         # We don't however run rpmlint on old versions of Ubuntu (ie 20.04 or earlier) because they are still on
         # version 1.X of the tool and there were a lot of big changes in the 2.0 release in May 2021, including in the
         # call syntax -- see https://github.com/rpm-software-management/rpmlint/releases/tag/2.0.0 for details.
         # (Interestingly, as of that 2.0 release, rpmlint is entirely written in Python and can even be installed via
         # `pip install rpmlint` and imported as a Python module -- see https://pypi.org/project/rpmlint/.  We should
         # have a look at this, provided we can use it without messing up anything the user has already installed from
         # distro packages.)
         #
         rawVersion = btExecute.abortOnRunFail(
            subprocess.run(['rpmlint', '--version'], capture_output=True)).stdout.decode('UTF-8'
         ).rstrip()
         log.debug('rpmlint version raw: ' + rawVersion)
         # Older versions of rpmlint output eg "rpmlint version 1.11", whereas newer ones output eg "2.2.0".  With the
         # magic of regular expressions we can fix this.
         trimmedVersion = re.sub(r'^[^0-9]*', '', rawVersion).replace('_', '.')
         log.debug('rpmlint version trimmed: ' + trimmedVersion)
         rpmlintVersion = packaging.version.parse(trimmedVersion)
         log.debug('rpmlint version parsed: ' + str(rpmlintVersion))
         if (rpmlintVersion < packaging.version.parse('2.0.0')):
            log.info('Skipping invocation of rpmlint as installed version (' + str(rpmlintVersion) +
                     ') is too old (< 2.0)')
         else:
            log.info('Running rpmlint (v' + str(rpmlintVersion) +
                     ') to check the created rpm package for errors and warnings')
            btExecute.abortOnRunFail(
               subprocess.run(
                  ['rpmlint',
                   '--config',
                   btFileSystem.dir_base.joinpath('packaging/linux'),
                   dir_packages_rpm_output.joinpath(rpmPackageName).as_posix()],
                  capture_output=False
               )
            )

         # Move the .rpm file to the top-level directory
         shutil.move(dir_packages_rpm_output.joinpath(rpmPackageName), btFileSystem.dir_packages_platform)

         #
         # As with the .deb file above, if we are on Ubuntu, we would like to edit the package name to include the
         # Ubuntu major version number.
         #
         if ('Ubuntu' == distroInfo["name"]):
            #
            # We want to rename, eg "brewtarget-4.2.1-1_x86_64.rpm" to "brewtarget-4.2.1_22-1_x86_64.rpm" or
            # "brewken-1.0.3-1_x86_64.rpm" to "brewken-1.0.3_22-1_x86_64.rpm".  This is the same structure as the .deb
            # files, so it's the same logic here.
            #
            oldRpmPackageName = rpmPackageName
            nameParts = rpmPackageName.split('-')
            rpmPackageName = nameParts[0] + '-' + nameParts[1] + '_' + str(distroInfo["major"]) + '-' + nameParts[2]
            os.chdir(btFileSystem.dir_packages_platform)
            log.info('Renaming ' + oldRpmPackageName + ' to ' + rpmPackageName)
            os.rename(oldRpmPackageName, rpmPackageName)

         # We don't particularly need to change back to the previous working directory, but it's tidy to do so.
         os.chdir(previousWorkingDirectory)

         #
         # Make the checksum file
         #
         log.info('Generating checksum file for ' + rpmPackageName)
         writeSha256sum(btFileSystem.dir_packages_platform, rpmPackageName)

      #-----------------------------------------------------------------------------------------------------------------
      #----------------------------------------------- Windows Packaging -----------------------------------------------
      #-----------------------------------------------------------------------------------------------------------------
      case 'Windows':
         log.debug('Windows Packaging')
         #
         # There are three main open-source packaging tools available for Windows:
         #
         #    - NSIS (Nullsoft Scriptable Install System) -- see https://nsis.sourceforge.io/
         #      This is widely used and reputedly simple to learn.  Actually the documentation, although OK overall, is
         #      not brilliant for beginners.  When you are trying to write your first installer script, you will find a
         #      frustrating number of errors, omissions and broken links in the documentation.  If you give up on this
         #      and take an existing working script as a starting point, the reference documentation to explain each
         #      command is not too bad.  Plus there are lots of useful titbits on Stack Overflow etc.
         #         What's less good is that the scripting language is rather primitive.  Once you start looking at
         #      variable scope and how to pass arguments to functions, you'll have a good feel for what it was like to
         #      write mainframe assembly language in the 1970s.
         #         There is one other advantage that NSIS has over Wix and Inno Setup, specifically that it is available
         #      as an MSYS2 package (mingw-w64-x86_64-nsis for 64-bit and mingw-w64-i686-nsis for 32-bit), whereas the
         #      others are not.  This makes it easier to script installations, including for the automated builds on
         #      GitHub.
         #
         #    - WiX -- see https://wixtoolset.org/ and https://github.com/wixtoolset/
         #      This is apparently used by a lot of Microsoft's own products and is supposedly pretty robust.  Looks
         #      like you configure/script it with XML and PowerShell.  Most discussion of it says you really first need
         #      to have a good understanding of Windows Installer (https://en.wikipedia.org/wiki/Windows_Installer) and
         #      its MSI package format.  There is a 260 page book called "The Definitive Guide to Windows Installer"
         #      which either is or isn't beginner-friendly depending on who you ask but, either way is about 250 pages
         #      more than I want to have to know about Windows package installation.  If we decided we _needed_ to
         #      produce MSI installers though, this would be the only choice.
         #
         #    - Inno Setup -- see https://jrsoftware.org/isinfo.php and https://github.com/jrsoftware/issrc
         #      Has been around for ages, but is less widely used than NSIS.  Basic configuration is supposedly simpler
         #      than NSIS, as it's based on an INI file (https://en.wikipedia.org/wiki/INI_file), but you also, by
         #      default, have a bit less control over how the installer works.  If you do need to script something you
         #      have to do it in Pascal, so a trip back to the 1980s rather than the 1970s.
         #
         # For the moment, we're sticking with NSIS, which is the devil we know, aka what we've historically used.
         #
         # In the past, we built only 32-bit packages (i686 architecture) on Windows because of problems getting 64-bit
         # versions of NSIS plugins to work.  However, we now invoke NSIS without plugins, so the 64-bit build seems to
         # be working.
         #
         # As of January 2024, some of the 32-bit MSYS2 packages/groups we were previously relying on previously are no
         # longer available.  So now, we only build 64-bit packages (x86_64 architecture) on Windows.
         #

         #
         # As mentioned above, not all information about what Meson does is readily exportable.   In particular, I can
         # find no simple way to get the actual directory that a file was installed to.  Eg, on Windows, in an MSYS2
         # environment, the main executable will be in mbuild/packages/windows/msys64/mingw32/bin/ or similar.  The
         # beginning (mbuild/packages/windows) and the end (bin) are parts we specify, but the middle bit
         # (msys64/mingw32) is magicked up by Meson and not explicitly exposed to build script commands.
         #
         # Fortunately, we can just search for a directory called bin inside the platform-specific packaging directory
         # and we'll have the right thing.
         #
         # (An alternative approach would be to invoke meson with the --bindir parameter to manually choose the
         # directory for the executable.)
         #
         packageBinDirList = glob.glob('./**/bin/', root_dir=btFileSystem.dir_packages_platform.as_posix(), recursive=True)
         if (len(packageBinDirList) == 0):
            log.critical(
               'Cannot find bin subdirectory of ' + btFileSystem.dir_packages_platform.as_posix() + ' packaging directory'
            )
            exit(1)
         if (len(packageBinDirList) > 1):
            log.warning(
               'Found more than one bin subdirectory of ' + btFileSystem.dir_packages_platform.as_posix() +
               ' packaging directory: ' + '; '.join(packageBinDirList) + '.  Assuming first is the one we need'
            )

         dir_packages_win_bin = btFileSystem.dir_packages_platform.joinpath(packageBinDirList[0])
         log.debug('Package bin dir: ' + dir_packages_win_bin.as_posix())

         #
         # We could do the same search for data and doc directories, but we happen to know that they should just be
         # sibling directories of the bin directory we just found.
         #
         dir_packages_win_data = dir_packages_win_bin.parent.joinpath('data')
         dir_packages_win_doc  = dir_packages_win_bin.parent.joinpath('doc')

         #
         # Now we have to deal with shared libraries.  Windows does not have a built-in package manager and it's not
         # realistic for us to require end users to install and use one.  So, any shared library that we cannot
         # statically link into the application needs to be included in the installer.  This mainly applies to Qt.
         # (Although you can, in principle, statically link against Qt, it requires downloading the entire Qt source
         # code and doing a custom build.)  Fortunately, Qt provides a handy utility called windeployqt that should do
         # most of the work for us.
         #
         # Per https://doc.qt.io/qt-6/windows-deployment.html, the windeployqt executable creates all the necessary
         # folder tree "containing the Qt-related dependencies (libraries, QML imports, plugins, and translations)
         # required to run the application from that folder".
         #
         # In the MSYS2 packaging of Qt6 at least, per https://packages.msys2.org/packages/mingw-w64-x86_64-qt6-base,
         # windeployqt is renamed to windeployqt6.
         #
         log.debug('Running windeployqt')
         previousWorkingDirectory = pathlib.Path.cwd().as_posix()
         os.chdir(dir_packages_win_bin)
         btExecute.abortOnRunFail(
            subprocess.run(['windeployqt6',
                            '--verbose', '2',        # 2 is the maximum
                            projectName + '.exe'],
                           capture_output=False)
         )
         os.chdir(previousWorkingDirectory)

         #
         # We're not finished with shared libraries.  Although windeployqt is theoretically capable of detecting all the
         # shared libraries we need, including non-Qt ones, it doesn't, in practice, seem to be that good on the non-Qt
         # bit.  And although, somewhere in the heart of the Meson implementation, you would think it would or could
         # know the full paths to the shared libraries on which we depend, this is not AFAICT extractable in the
         # meson.build script.  So, here, we have a list of libraries that we know we depend on and we search for them
         # in the paths listed in the PATH environment variable.  It's a bit less painful than you might think to
         # construct and maintain this list of libraries, because, for the most part, if you miss a needed DLL from the
         # package, Windows will give you an error message at start-up telling you which DLL(s) it needed but could not
         # find.
         #
         # There are also various platform-specific free-standing tools that claim to examine an executable and
         # tell you what shared libraries it depends on.  In particular ntldd
         # (see https://packages.msys2.org/packages/mingw-w64-x86_64-ntldd) seems useful.  Note that you need to run it
         # with the `-R` (recursive) option to catch all the dependencies.  (Unlike with Linux packaging, we can't just
         # specify the top level dependencies and rely on everything else to get pulled in automatically.)  Eg, the
         # following is a useful starting point:
         #
         #    ntldd -R brewtarget.exe | grep -v "not found" | grep -v ext | grep -v WINDOWS | sed -e 's/^[\t ]*//; s/\.dll.*$//' | sort -u
         #
         # We assume that the library 'foo' has a dll called 'libfoo.dll' or 'libfoo-X.dll' or 'libfooX.dll' where X is
         # a (possibly multi-digit) version number present on some, but not all, libraries.  If we find more matches
         # than we were expecting, we log a warning and just include everything we found.  (Sometimes we include the
         # version number in the library name because we really are looking for a specific version or there are always
         # multiple versions)  It's not super pretty, but it should work.
         #
         # Note that there are libraries with names of form 'libfoo-Y-X.dll'.  For the moment, we require the '-Y' part
         # to be included in the list below, rather than adding more logic to deduce it.
         #
         # Just to keep us on our toes, the Python os module has two similarly-named but different things:
         #    - os.pathsep is the separator between paths (usually ';' or ':') eg in the PATH environment variable
         #    - os.sep is the separator between directories (usually '/' or '\\') in a path
         #
         # The comments below about the source of libraries are just FYI.  In almost all cases, we are actually
         # installing these things on the build machine via pacman, so we don't have to go directly to the upstream
         # project.
         #
         pathsToSearch = os.environ['PATH'].split(os.pathsep)
         extraLibs = [
            #
            # Following should have been handled automatically by windeployqt
            #
            #'Qt6Core'        ,
            #'Qt6Gui'         ,
            #'Qt6Multimedia'  ,
            #'Qt6Network'     ,
            #'Qt6PrintSupport',
            #'Qt6Sql'         ,
            #'Qt6Widgets'     ,
            #
            # Following are not handled by windeployqt.  The application will install and run without them, but it just
            # won't show any .svg icons (and won't log any errors about them either).
            # See also https://stackoverflow.com/questions/76047551/icons-shown-in-qt5-not-showing-in-qt6
            #
            'Qt6SvgWidgets', # See https://doc.qt.io/qt-6/qsvgwidget.html
            'Qt6Svg'       , # Needed for Qt6SvgWidgets.dll to display .svg icons
            #
            #
            'libb2'               , # BLAKE hash functions -- https://en.wikipedia.org/wiki/BLAKE_(hash_function)
            'libbrotlicommon'     , # Brotli compression -- see https://en.wikipedia.org/wiki/Brotli
            'libbrotlidec'        , # Brotli compression
            'libbrotlienc'        , # Brotli compression
            'libbz2'              , # BZip2 compression -- see https://en.wikipedia.org/wiki/Bzip2
            'libdouble-conversion', # Binary-decimal & decimal-binary routines for IEEE doubles -- see https://github.com/google/double-conversion
            'libfreetype'         , # Font rendering -- see https://freetype.org/
            #
            # 32-bit and 64-bit MinGW use different exception handling (see
            # https://sourceforge.net/p/mingw-w64/wiki2/Exception%20Handling/) hence the different naming of libgcc in
            # the 32-bit and 64-bit environments.
            #
#            'libgcc_s_dw2' , # 32-bit GCC library
            'libgcc_s_seh' , # 64-bit GCC library
            'libglib-2.0'  ,
            'libgraphite'  ,
            'libharfbuzz'  , # HarfBuzz text shaping engine -- see https://github.com/harfbuzz/harfbuzz
            'libiconv'     , # See https://www.gnu.org/software/libiconv/
            'libicudt'     , # Part of International Components for Unicode
            'libicuin'     , # Part of International Components for Unicode
            'libicuuc'     , # Part of International Components for Unicode
            'libintl'      , # See https://www.gnu.org/software/gettext/
            'libmd4c'      , # Markdown for C -- see https://github.com/mity/md4c
            'libpcre2-8'   , # Perl Compatible Regular Expressions
            'libpcre2-16'  , # Perl Compatible Regular Expressions
            'libpcre2-32'  , # Perl Compatible Regular Expressions
            'libpng16'     , # Official PNG reference library -- see http://www.libpng.org/pub/png/libpng.html
            'libsqlite3'   , # Need this IN ADDITION to bin/sqldrivers/qsqlite.dll, which gets installed by windeployqt
            'libstdc++'    ,
            'librsvg-2'    , # SVG rendering library -- see https://wiki.gnome.org/Projects/LibRsvg
            'libwinpthread',
            'libxalan-c'   ,
            'libxalanMsg'  ,
            'libxerces-c-3',
            'libzstd'      , # ZStandard (aka zstd) = fast lossless compression algorithm
            'zlib'         , # ZLib compression library
         ]
         btUtils.findAndCopyLibs(pathsToSearch, extraLibs, 'dll', '-?[0-9]*.dll', dir_packages_win_bin)

         # Copy the NSIS installer script to where it belongs
         shutil.copy2(btFileSystem.dir_build.joinpath('NsisInstallerScript.nsi'), btFileSystem.dir_packages_platform)

         # We change into the packaging directory and invoke the NSIS Compiler (aka MakeNSIS.exe)
         os.chdir(btFileSystem.dir_packages_platform)
         log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())
         btExecute.abortOnRunFail(
            # FYI, we don't need it here, but if you run makensis from the MSYS2 command line (Mintty), you need double
            # slashes on the options (//V4 instead of /V4 etc).
            subprocess.run(
               [
                  'MakeNSIS.exe', # 'makensis' would also work on MSYS2
                  '/V4',          # Max verbosity/logging
                  # Variables coming from this script are passed in as command-line defines.  Fortunately there aren't
                  # too many of them.
                  '/DBT_PACKAGING_BIN_DIR="'  + dir_packages_win_bin.as_posix()  + '"',
                  '/DBT_PACKAGING_DATA_DIR="' + dir_packages_win_data.as_posix() + '"',
                  '/DBT_PACKAGING_DOC_DIR="'  + dir_packages_win_doc.as_posix()  + '"',
                  'NsisInstallerScript.nsi',
               ],
               capture_output=False
            )
         )

         #
         # Make the checksum file.
         #
         # Note that the name of the installer file is controlled by packaging/windows/NsisInstallerScript.nsi.in, so
         # we have to align here with what that says.
         #
         winInstallerName = capitalisedProjectName + ' ' + btUtils.buildConfig['CONFIG_VERSION_STRING'] + ' Windows Installer.exe'
         log.info('Generating checksum file for ' + winInstallerName)
         writeSha256sum(btFileSystem.dir_packages_platform, winInstallerName)

         #--------------------------------------------------------------------------------------------------------------
         # Signing Windows binaries is a separate step.  For Brewtarget, it is possible, with the help of SignPath, to
         # do via GitHub Actions.  (For Brewken, we do not yet have enough standing/users to qualify for the SignPath
         # Open Source Software sponsorship.)
         #--------------------------------------------------------------------------------------------------------------

      #-----------------------------------------------------------------------------------------------------------------
      #------------------------------------------------- Mac Packaging -------------------------------------------------
      #-----------------------------------------------------------------------------------------------------------------
      case 'Darwin':
         log.debug('Mac Packaging')
         #
         # See https://stackoverflow.com/questions/1596945/building-osx-app-bundle for essential info on building Mac
         # app bundles.  Also https://mesonbuild.com/Creating-OSX-packages.html suggests how to do this with Meson,
         # though it's mostly through having Meson call shell scripts, so I think we're better off sticking to this
         # Python script.
         #
         # https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/BundleTypes/BundleTypes.html
         # is the "official" Apple info about the directory structure.
         #
         # To create a Mac app bundle , we create the following directory structure, where items marked ✅ are copied as
         # is from the tree generated by meson install with --destdir option, those marked 🟢 are (mostly) handled by
         # `macdeployqt`, and those marked ❇ are ones we need to relocate, generate or modify ourselves.  (We have to
         # some additional work to address shortcomings and bugs in macdeployqt.)
         #
         # (When working on this bit, use ❌ for things that are generated automatically but not actually needed, and ✴
         # for things we still need to add.)
         #    [projectName]_[versionNumber]_MacOS.app
         #    └── Contents
         #        ├── Info.plist ❇  <── "Information property list" file = required configuration information (in XML)
         #        │                      This includes things such as Bundle ID.  It is generated by the Meson build
         #        │                      from packaging/darwin/Info.plist.in
         #        ├── Frameworks  <── Contains any private shared libraries and frameworks used by the executable
         #        │   ├── QtCore.framework * NB: Directory and its contents *  🟢
         #        │   ├── [Other Qt .framework directories and their contents] 🟢
         #        │   ├── libfreetype.6.dylib    🟢
         #        │   ├── libglib-2.0.0.dylib    🟢
         #        │   ├── libgthread-2.0.0.dylib 🟢
         #        │   ├── libintl.8.dylib        🟢
         #        │   ├── libjpeg.8.dylib        🟢
         #        │   ├── libpcre2-16.0.dylib    🟢
         #        │   ├── libpcre2-8.0.dylib     🟢
         #        │   ├── libpng16.16.dylib      🟢
         #        │   ├── libsharpyuv.0.dylib    🟢
         #        │   ├── libtiff.5.dylib        🟢
         #        │   ├── libwebp.7.dylib        🟢
         #        │   ├── libwebpdemux.2.dylib   🟢
         #        │   ├── libwebpmux.3.dylib     🟢
         #        │   ├── libxalan-c.112.dylib   🟢
         #        │   ├── libxerces-c-3.2.dylib  🟢
         #        │   ├── libzstd.1.dylib        🟢
         #        │   └── libxalanMsg.112.dylib  ❇ ✴
         #        ├── MacOS
         #        │   └── [capitalisedProjectName] ❇  <── the executable
         #        ├── Plugins  <── Contains loadable bundles that extend the basic features of the application
         #        │   ├── audio
         #        │   │   └── libqtaudio_coreaudio.dylib 🟢
         #        │   ├── bearer
         #        │   │   └── libqgenericbearer.dylib 🟢
         #        │   ├── iconengines
         #        │   │   └── libqsvgicon.dylib 🟢
         #        │   ├── imageformats
         #        │   │   ├── libqgif.dylib     🟢
         #        │   │   ├── libqicns.dylib    🟢
         #        │   │   ├── libqico.dylib     🟢
         #        │   │   ├── libqjpeg.dylib    🟢
         #        │   │   ├── libqmacheif.dylib 🟢
         #        │   │   ├── libqmacjp2.dylib  🟢
         #        │   │   ├── libqsvg.dylib     🟢
         #        │   │   ├── libqtga.dylib     🟢
         #        │   │   ├── libqtiff.dylib    🟢
         #        │   │   ├── libqwbmp.dylib    🟢
         #        │   │   └── libqwebp.dylib    🟢
         #        │   ├── mediaservice
         #        │   │   ├── libqavfcamera.dylib          🟢
         #        │   │   ├── libqavfmediaplayer.dylib     🟢
         #        │   │   └── libqtmedia_audioengine.dylib 🟢
         #        │   ├── platforms
         #        │   │   └── libqcocoa.dylib 🟢
         #        │   ├── printsupport
         #        │   │   └── libcocoaprintersupport.dylib 🟢
         #        │   ├── sqldrivers
         #        │   │   ├── libqsqlite.dylib  🟢
         #        │   │   ├── libqsqlodbc.dylib ✴  Not sure we need this one, but it got shipped with Brewtarget 2.3
         #        │   │   └── libqsqlpsql.dylib ✴
         #        │   ├── styles
         #        │   │  └── libqmacstyle.dylib 🟢
         #        │   └── virtualkeyboard
         #        │       ├── libqtvirtualkeyboard_hangul.dylib  🟢
         #        │       ├── libqtvirtualkeyboard_openwnn.dylib 🟢
         #        │       ├── libqtvirtualkeyboard_pinyin.dylib  🟢
         #        │       ├── libqtvirtualkeyboard_tcime.dylib   🟢
         #        │       └── libqtvirtualkeyboard_thai.dylib    🟢
         #        └── Resources
         #            ├── [capitalisedProjectName]Icon.icns ✅  <── Icon file
         #            ├── DefaultData.xml   ✅
         #            ├── default_db.sqlite ✅
         #            ├── en.lproj        <── Localized resources
         #            │   ├── COPYRIGHT ✅
         #            │   └── README.md ✅
         #            ├── qt.conf ✅
         #            ├── sounds
         #            │   └── [All the filesToInstall_sounds .wav files] ✅
         #            └── translations_qm
         #                └── [All the .qm files generated by qt.compile_translations] ✅
         #
         # This will ultimately get bundled up into a disk image (.dmg) file.
         #

         #
         # Make the top-level directories that we're going to copy files into
         #
         log.debug('Creating Mac app bundle top-level directories')
         macBundleDirName = projectName + '_' + btUtils.buildConfig['CONFIG_VERSION_STRING'] + '_MacOS.app'
         # btFileSystem.dir_packages_platform = mbuild/packages/darwin
         dir_packages_mac = btFileSystem.dir_packages_platform.joinpath(macBundleDirName).joinpath('Contents')
         dir_packages_mac_bin = dir_packages_mac.joinpath('MacOS')
         dir_packages_mac_rsc = dir_packages_mac.joinpath('Resources')
         dir_packages_mac_frm = dir_packages_mac.joinpath('Frameworks')
         dir_packages_mac_plg = dir_packages_mac.joinpath('Plugins')
         os.makedirs(dir_packages_mac_bin) # This will also automatically create parent directories
         os.makedirs(dir_packages_mac_frm)
         os.makedirs(dir_packages_mac_plg)

         #
         # From time to time, things change in the Mac toolchain.  It used to be that Meson would put:
         #
         #    - resources in mbuild/packages/darwin/usr/local/Contents/Resources
         #    - binary    in mbuild/packages/darwin/usr/local/bin
         #
         # Something changed in 2024 so that the locations became:
         #
         #    - resources in mbuild/packages/darwin/opt/homebrew/Contents/Resources
         #    - binary    in mbuild/packages/darwin/opt/homebrew/bin
         #
         # But then, later in the year, it changed back again.
         #
         # It's possible that we are somehow triggering this by other things we do in this script - possibly to do with
         # what we install from Homebrew and what from MacPorts.  Or perhaps things change from version to version of
         # one of the tools or libraries we are using.  For the moment, rather than spend a lot of time trying to get to
         # the bottom of it, we just detect which set of paths has been used.
         #
         # We also have:
         #
         #    - man page in mbuild/packages/darwin/opt/homebrew/share/man/man1/
         #
         # However, we are not currently shipping man page on Mac
         #
         btFileSystem.dir_buildOutputRoot = ''
         possible_buildOutputRoots = ['usr/local', 'opt/homebrew']
         for subDir in possible_buildOutputRoots:
            candidateDir = btFileSystem.dir_packages_platform.joinpath(subDir)
            log.debug('Is ' + candidateDir.as_posix() + ' a directory? ' + str(os.path.isdir(candidateDir)))
            if (os.path.isdir(candidateDir)):
               btFileSystem.dir_buildOutputRoot = candidateDir
               break

         if ('' == btFileSystem.dir_buildOutputRoot):
            log.error('Unable to find build output root!')
         else:
            log.debug('Detected build output root as ' + btFileSystem.dir_buildOutputRoot.as_posix())

         #
         # If we get errors about things not being found, the following can be a helpful diagnostic
         #
         log.debug('Directory tree of ' + btFileSystem.dir_packages_platform.as_posix())
         btExecute.abortOnRunFail(
            subprocess.run(['tree', '-sh', btFileSystem.dir_packages_platform.as_posix()], capture_output=False)
         )

         # Rather than create dir_packages_mac_rsc directly, it's simplest to copy the whole Resources tree from
         # mbuild/mackages/darwin/usr/local/Contents/Resources, as we want everything that's inside it
         log.debug(
            'Copying Resources from ' + btFileSystem.dir_buildOutputRoot.joinpath('Contents/Resources').as_posix() +
            ' to ' + dir_packages_mac_rsc.as_posix()
         )
         shutil.copytree(btFileSystem.dir_buildOutputRoot.joinpath('Contents/Resources'), dir_packages_mac_rsc)

         # Copy the Information Property List file to where it belongs
         log.debug('Copying Information Property List file')
         shutil.copy2(btFileSystem.dir_build.joinpath('Info.plist').as_posix(), dir_packages_mac)

         # Because Meson is geared towards local installs, in the mbuild/mackages/darwin directory, it is going to have
         # placed the executable in the usr/local/bin or opt/homebrew/bin subdirectory.  Copy it to the right place.
         log.debug('Copying executable')
         shutil.copy2(btFileSystem.dir_buildOutputRoot.joinpath('bin').joinpath(capitalisedProjectName).as_posix(),
                      dir_packages_mac_bin)

         #
         # The macdeployqt executable shipped with Qt does for Mac what windeployqt does for Windows -- see
         # https://doc.qt.io/qt-6/macos-deployment.html#the-mac-deployment-tool
         #
         # At first glance, you might thanks that, with a few name changes, we might share all the bt code for
         # macdeployqt and windeployqt.  However, the two programs share _only_ a top-level goal ("automate the process
         # of creating a deployable [folder / application bundle] that contains [the necessary Qt dependencies]" - ie so
         # that the end user does not have to install Qt to run our software).  They have completely different
         # implementations and command line options, so it would be unhelpful to try to treat them identically.
         #
         # With the verbose logging on, you can see that macdeployqt is calling:
         #    - otool (see https://www.unix.com/man-page/osx/1/otool/) to get information about which libraries etc the
         #      executable depends on
         #    - install_name_tool (see https://www.unix.com/man-page/osx/1/install_name_tool/) to change the paths in
         #      which the executable looks for a library
         #    - strip (see https://www.unix.com/man-page/osx/1/strip/) to remove symbols from shared libraries
         #
         # As discussed at https://stackoverflow.com/questions/2809930/macdeployqt-and-third-party-libraries, there are
         # usually cases where you have to do some of the same work by hand because macdeployqt doesn't automatically
         # detect all the dependencies.  One example of this is that, if a shared library depends on another shared
         # library then macdeployqt won't detect it, because it does not recursively run its dependency checking.
         #
         # For us, macdeployqt does seem to cover almost all the shared libraries and frameworks we need, including
         # those that are not part of Qt.  The exceptions are:
         #    - libxalanMsg -- a library that libxalan-c uses (so an indirect rather than direct dependency)
         #    - libqsqlpsql.dylib -- which would be needed for any user that wants to use PostgreSQL instead of SQLite
         #
         # Note per https://www.unix.com/man_page/osx/1/dyld/ that the dynamic link editor (dyld), which is what loads
         # shared libraries etc, recognises the following variables:
         #
         #    @executable_path -- the path to the directory containing the main executable for the process
         #
         #    @loader_path     -- the path to the directory containing the mach-o binary which contains the load command
         #                        using @loader_path.  Thus, in every binary, @loader_path resolves to a different path,
         #                        whereas @executable_path always resolves to the same path
         #
         #    @rpath           -- Dyld maintains a current stack of paths called the run path list.  When @rpath is
         #                        encountered it is substituted with each path in the run path list until a loadable
         #                        dylib if found.
         #
         previousWorkingDirectory = pathlib.Path.cwd().as_posix()
         log.debug('Running otool before macdeployqt')
         os.chdir(dir_packages_mac_bin)
         otoolOutputExe = btExecute.abortOnRunFail(
            subprocess.run(['otool',
                            '-L',
                            capitalisedProjectName],
                           capture_output=True)
         ).stdout.decode('UTF-8')
         log.debug('Output of `otool -L ' + capitalisedProjectName + '`: ' + otoolOutputExe)
         #
         # The output from otool at this stage will be along the following lines:
         #
         #    [capitalisedProjectName]:
         #       /opt/homebrew/opt/qt/lib/QtCore.framework/Versions/A/QtCore (compatibility version 6.0.0, current version 6.7.2)
         #       /opt/homebrew/opt/qt/lib/QtGui.framework/Versions/A/QtGui (compatibility version 6.0.0, current version 6.7.2)
         #       /opt/homebrew/opt/qt/lib/QtMultimedia.framework/Versions/A/QtMultimedia (compatibility version 6.0.0, current version 6.7.2)
         #       /opt/homebrew/opt/qt/lib/QtPrintSupport.framework/Versions/A/QtPrintSupport (compatibility version 6.0.0, current version 6.7.2)
         #       /opt/homebrew/opt/qt/lib/QtSql.framework/Versions/A/QtSql (compatibility version 6.0.0, current version 6.7.2)
         #       /opt/homebrew/opt/qt/lib/QtWidgets.framework/Versions/A/QtWidgets (compatibility version 6.0.0, current version 6.7.2)
         #       /opt/local/lib/libxerces-c-3.2.dylib (compatibility version 0.0.0, current version 0.0.0)
         #       /opt/local/lib/libxalan-c.112.dylib (compatibility version 112.0.0, current version 112.0.0)
         #       /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 1700.255.5)
         #       /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1345.120.2)
         #
         # Similarly, here's an example from when we were using Qt5:
         #
         #    [capitalisedProjectName]:
         #       /usr/local/opt/qt@5/lib/QtCore.framework/Versions/5/QtCore (compatibility version 5.15.0, current version 5.15.8)
         #       /usr/local/opt/qt@5/lib/QtGui.framework/Versions/5/QtGui (compatibility version 5.15.0, current version 5.15.8)
         #       /usr/local/opt/qt@5/lib/QtMultimedia.framework/Versions/5/QtMultimedia (compatibility version 5.15.0, current version 5.15.8)
         #       /usr/local/opt/qt@5/lib/QtNetwork.framework/Versions/5/QtNetwork (compatibility version 5.15.0, current version 5.15.8)
         #       /usr/local/opt/qt@5/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport (compatibility version 5.15.0, current version 5.15.8)
         #       /usr/local/opt/qt@5/lib/QtSql.framework/Versions/5/QtSql (compatibility version 5.15.0, current version 5.15.8)
         #       /usr/local/opt/qt@5/lib/QtWidgets.framework/Versions/5/QtWidgets (compatibility version 5.15.0, current version 5.15.8)
         #       /usr/local/opt/xerces-c/lib/libxerces-c-3.2.dylib (compatibility version 0.0.0, current version 0.0.0)
         #       /usr/local/opt/xalan-c/lib/libxalan-c.112.dylib (compatibility version 112.0.0, current version 112.0.0)
         #       /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 1300.36.0)
         #       /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1319.0.0)
         #
         # After running `macdeployqt`, all the paths for non-system libraries will be changed to ones beginning
         # '@loader_path/../Frameworks/', as will be seen from the subsequent output of running `otool`.
         #
         # We want to grab:
         #   - the directory containing libxalan-c, as that's the same directory in which we should find libxalanMsg
         #   - information that would allow us to find libqsqlpsql.dylib .:TODO:. Still to work out how to do this.  For
         #     now, I think that means users requiring PostgreSQL support on MacOS will need to build the app from
         #     source.
         #
         xalanDir = ''
         xalanLibName = ''
         xalanMatch = re.search(r'^\s*(\S+/)(libxalan-c\S*.dylib)', otoolOutputExe, re.MULTILINE)
         if (xalanMatch):
            # The [1] index gives us the first parenthesized subgroup of the regexp match, which in this case should be
            # the directory path to libxalan-c.xxx.dylib
            xalanDir = xalanMatch[1]
            xalanLibName = xalanMatch[2]
         else:
            log.warning(
               'Could not find libxalan dependency in ' + capitalisedProjectName +
               ' so assuming /usr/local/opt/xalan-c/lib/'
            )
            xalanDir = '/usr/local/opt/xalan-c/lib/'
            xalanLibName = 'libxalan-c.112.dylib'
         log.debug('xalanDir: ' + xalanDir + '; contents:')
         btExecute.abortOnRunFail(subprocess.run(['ls', '-l', xalanDir], capture_output=False))

         #
         # Strictly speaking, we should look at every /usr/local/opt/.../*.dylib dependency of our executable, and run
         # each of those .dylib files through otool to get its dependencies, then repeat until we find no new
         # dependencies.  Then we should ensure each dependency is copied into the app bundle and whatever depends on it
         # knows where to find it etc.  Pretty soon we'd have ended up reimplementing macdeployqt.  Fortunately, in
         # practice, for Xalan, it suffices to grab libxalanMsg and put it in the same directory in the bundle as
         # libxalanc.
         #
         # We use otool to get the right name for libxalanMsg, which is typically listed as a relative path dependency
         # eg '@rpath/libxalanMsg.112.dylib'.
         #
         # Per https://www.mikeash.com/pyblog/friday-qa-2009-11-06-linking-and-install-names.html:
         #
         #    @executable_path - will expand at run time to the absolute path of the app bundle's executable directory,
         #                       ie [projectName]_[versionNumber].app/Contents/MacOS for us
         #
         #    @loader_path     - will expand at run time to the absolute path of whatever is loading the library,
         #                       typically either the executable directory (if it's the executable loading the library
         #                       directly) or, for us, the [projectName]_[versionNumber].app/Contents/Frameworks
         #                       directory if it's another shared library requesting the load
         #
         #    @rpath           - means search a list of locations specified at the point the application was linked (by
         #                       means of the -rpath linker flag), so, eg, including
         #                       '-rpath @executable_path/../Frameworks' at link time means, for us, that
         #                       [projectName]_[versionNumber].app/Contents/Frameworks is one of the places to search
         #                       when @rpath is specified
         #
         log.debug('Running otool -L on ' + xalanDir + xalanLibName)
         otoolOutputXalan = btExecute.abortOnRunFail(
            subprocess.run(['otool',
                            '-L',
                            xalanDir + xalanLibName],
                           capture_output=True)
         ).stdout.decode('UTF-8')
         log.debug('Output of `otool -L ' + xalanDir + xalanLibName + '`: ' + otoolOutputXalan)
         xalanMsgLibName = ''
         xalanMsgMatch =  re.search(r'^\s*(\S+/)(libxalanMsg\S*.dylib)', otoolOutputXalan, re.MULTILINE)
         if (xalanMsgMatch):
            xalanMsgLibName = xalanMsgMatch[2]
         else:
            log.warning(
               'Could not find libxalanMsg dependency in ' + xalanDir + xalanLibName +
               ' so assuming libxalanMsg.112.dylib'
            )
            xalanMsgLibName = 'libxalanMsg.112.dylib'
         log.debug('Copying ' + xalanDir + xalanMsgLibName + ' to ' + dir_packages_mac_frm.as_posix())
         shutil.copy2(xalanDir + xalanMsgLibName, dir_packages_mac_frm)

         #
         # The dylibbundler tool (https://github.com/auriamg/macdylibbundler/) proposes a ready-made solution to make
         # incorporating shared libraries into app bundles simple.  We try it here.
         #
         # The --dest-dir parameter is where we want dylibbundler to put the fixed-up shared libraries.
         # The --install-path parameter is where the app will look for shared libraries, so it's essentially the
         # relative path from the executable to the same directory we specified with --dest-dir.
         #
         log.debug('Running' +
                   ' dylibbundler' +
                   ' --dest-dir ' + dir_packages_mac_frm.as_posix() +
                   ' --bundle-deps' +
                   ' --fix-file ' + dir_packages_mac_bin.joinpath(capitalisedProjectName).as_posix() +
                   ' --install-path ' + '@executable_path/' + os.path.relpath(dir_packages_mac_frm, dir_packages_mac_bin))
         btExecute.abortOnRunFail(
            subprocess.run(
               ['dylibbundler',
                '--dest-dir', dir_packages_mac_frm.as_posix(),
                '--bundle-deps',
                '--fix-file', dir_packages_mac_bin.joinpath(capitalisedProjectName).as_posix(),
                '--install-path', '@executable_path/' + os.path.relpath(dir_packages_mac_frm, dir_packages_mac_bin)],
               capture_output=False
            )
         )

         #
         # Since moving to Qt6, we also have to do some extra things to avoid the following errors:
         #    - Library not loaded: @rpath/QtDBus.framework/Versions/A/QtDBus
         #    - Library not loaded: @rpath/QtNetwork.framework/Versions/A/QtNetwork
         #
         # I _think_ the problem with these is that they are not direct dependencies of our application (eg, as shown
         # below, they do not appear in the output from otool), but rather dependencies of other Qt libraries.  The
         # detailed error messages imply it is QtGui that needs QtDBus and QtMultimedia that needs QtNetwork.  However,
         # running `otool -L` on /opt/homebrew/opt/qt/lib/QtGui.framework/Versions/A/QtGui does not yield any dbus
         # dependency.
         #
         # The first thing is to manually add in any missing frameworks.  Eg, since we know QtMultimedia requires
         # QtNetwork, we look to see if QtMultimedia is one of our dependencies and, if it is, we copy the QtNetwork
         # framework into our package.  (In this example, the QtMultimedia itself will get copied in by macdeployqt.)
         #
         qtFrameworksDir = ''
         extraFrameworkDependencies = {
            "QtMultimedia": ["QtNetwork", ],
            "QtGui"       : ["QtDBus"   , ],
         }
         for framework, dependencies in extraFrameworkDependencies.items():
            #
            # Eg to see if we depend on QtMultimedia, we are looking for something along the following lines in the
            # otool output from earlier:
            #    /opt/homebrew/opt/qt/lib/QtMultimedia.framework/Versions/A/QtMultimedia
            #
            # We want to change QtMultimedia to QtNetwork and then copy the whole of the .framework directory:
            #    /opt/homebrew/opt/qt/lib/QtNetwork.framework
            #
            frameworkMatch = re.search(r'^\s*(/\S+/' + framework + '.framework)', otoolOutputExe, re.MULTILINE)
            if (frameworkMatch):
               frameworkPath = frameworkMatch[1]
               log.debug('Doing extra dependencies for ' + frameworkPath)

               # Capture where the frameworks live, so we can tell macdeployqt below
               if (not qtFrameworksDir):
                  qtFrameworksDir = os.path.dirname(frameworkPath)

               for dependency in dependencies:
                  #
                  # We assume the dependency path takes the same form as the framework that requires it.  Eg
                  # QtMultimedia -> QtNetwork means we transform
                  #    /opt/homebrew/opt/qt/lib/QtMultimedia.framework/Versions/A/QtMultimedia
                  # to:
                  #    /opt/homebrew/opt/qt/lib/QtNetwork.framework/Versions/A/QtNetwork
                  #
                  dependencyPath = frameworkPath.replace(framework, dependency)
                  dependencyTarget = dir_packages_mac_frm.joinpath(dependency + '.framework')
                  #
                  # It seems there are problems when we copy the framework trees.  Users trying to install the app who
                  # run `codesign` get an error "bundle format is ambiguous (could be app or framework)".  We suspect
                  # this may be related to the way we handle symlinks when we copy the tree, so this diagnostic is to
                  # list in detail all the files in the tree before we copy it.
                  #
                  # Looks like symlinks are all relative and point inside the tree we are copying, so it's safe to copy
                  # them _as_ symlinks below.
                  #
                  btExecute.abortOnRunFail(
                     subprocess.run(
                        ['find', dependencyPath, '-exec', 'ls', '-ld', '{}', '+'],
                        capture_output=False
                     )
                  )
                  log.debug('Copying tree ' + dependencyPath + ' to ' + dependencyTarget.as_posix())
                  shutil.copytree(dependencyPath, dependencyTarget.as_posix(), symlinks=True)
                  #
                  # It is not enough to just copy, eg, QtDBus framework into the app bundle.  We need to fix its
                  # dependencies to point inside the bundle.  Eg after we copy
                  # /opt/homebrew/opt/qt/lib/QtDBus.framework/ to
                  # [projectName]_[versionNumber]_MacOS.app/Contents/Frameworks/QtDBus.framework/, the other Qt
                  # dependencies of the library inside that framework directory will still be on the "system" paths (eg
                  # /opt/homebrew/opt/qt/lib/QtCore.framework/ etc).  We need to change them to point inside the app
                  # bundle (eg [projectName]_[versionNumber]_MacOS.app/Contents/Frameworks/QtCore.framework/ etc).
                  #
                  # The variable names risk getting a bit confusing here because we are talking about dependencies of
                  # dependency.  Hence why we start referring to dependency as copiedLibrary.
                  #
                  copiedLibrary = dependencyTarget.joinpath('Versions', 'Current', dependency)
                  log.debug('Fixing absolute dependencies for ' + dependency + ' (at' + copiedLibrary.as_posix() + ')')

                  otoolOutputCopiedLibrary = btExecute.abortOnRunFail(
                     subprocess.run(['otool',
                                    '-L',
                                    copiedLibrary.as_posix()],
                                    capture_output=True)
                  ).stdout.decode('UTF-8')
                  log.debug('Output of `otool -L ' + copiedLibrary.as_posix() + '`: ' + otoolOutputCopiedLibrary)

                  for outputLine in otoolOutputCopiedLibrary.splitlines():
                     #
                     # If we find a dependency of the form:
                     #    /opt/local/libexec/qt6/lib/QtCore.framework/Versions/A/QtCore
                     # we want to change it to the form:
                     #    @executable_path/../Frameworks/QtCore.framework/Versions/A/QtCore
                     #
                     qtAbsoluteDependencyMatch = re.search(r'^\s*(/\S+/qt\S+/lib/)(Qt\S+) ', outputLine, re.MULTILINE)
                     if (qtAbsoluteDependencyMatch):
                        qtDepAbsPrefix = qtAbsoluteDependencyMatch[1]
                        qtDepFramework = qtAbsoluteDependencyMatch[2]
                        qtDepRelPrefix = '@executable_path/../Frameworks/'
                        qtDepAbsPath = '' + qtDepAbsPrefix + qtDepFramework
                        qtDepRelPath = '' + qtDepRelPrefix + qtDepFramework
                        #
                        # Per https://www.unix.com/man_page/osx/1/install_name_tool/, "install_name_tool changes the
                        # dynamic shared library install names and or adds, changes or deletes the rpaths recorded in a
                        # Mach-O binary".
                        #
                        # Specifically:
                        #
                        #    -id name
                        #          Changes the shared library identification name of a dynamic shared library to name
                        #
                        #    -change old new
                        #          Changes the dependent shared library install name old to new in the specified Mach-O
                        #          binary.
                        #
                        # We need the -id option to change the path the shared library thinks it lives in (which is the
                        # first line of output of otool -L).
                        #
                        # We need the -change option to change where the shared library looks for other shared libraries
                        # on which it depends.
                        #
                        # We rely here on the different library names being sufficiently different that none contains
                        # the name of another -- ie there is no library name (eg QtFoo) that is contained in another (eg
                        # QtFooQtBar).
                        #
                        if (qtDepFramework == dependency):
                           log.debug(
                              'Running install_name_tool -id ' + qtDepRelPath + ' ' + copiedLibrary.as_posix()
                           )

                           btExecute.abortOnRunFail(
                              subprocess.run(
                                 ['install_name_tool',
                                 '-id',
                                 qtDepRelPath,
                                 copiedLibrary.as_posix()],
                                 capture_output=False
                              )
                           )
                        else:
                           log.debug(
                              'Running install_name_tool -change ' + qtDepAbsPath + ' ' + qtDepRelPath + ' ' +
                              copiedLibrary.as_posix()
                           )

                           btExecute.abortOnRunFail(
                              subprocess.run(
                                 ['install_name_tool',
                                 '-change',
                                 qtDepAbsPath,
                                 qtDepRelPath,
                                 copiedLibrary.as_posix()],
                                 capture_output=False
                              )
                           )

                  otoolOutputCopiedLibrary = btExecute.abortOnRunFail(
                     subprocess.run(['otool',
                                    '-L',
                                    copiedLibrary],
                                    capture_output=True)
                  ).stdout.decode('UTF-8')
                  log.debug('Output of `otool -L ' + copiedLibrary.as_posix() + '`: ' + otoolOutputCopiedLibrary)

         #
         # From https://doc.qt.io/qt-6/macos-issues.html#d-bus-and-macos, we know we need to ship:
         #
         #    - libdbus-1 library
         #
         # See https://github.com/orgs/Homebrew/discussions/2823 for problems using macdeployqt with homebrew
         # installation of Qt
         #
         # Various links online talk about LD_LIBRARY_PATH and DYLD_LIBRARY_PATH, but neither of these environment
         # variables seems to be set in GitHub MacOS actions.
         #
         log.debug('PATH=' + os.environ['PATH'])

         pathsToSearch = os.environ['PATH'].split(os.pathsep)
         extraLibs = [
            'libdbus'  , # Eg libdbus-1.3.dylib
         ]
         btUtils.findAndCopyLibs(pathsToSearch, extraLibs, 'dylib', '.*.dylib', dir_packages_mac_bin)

         #
         # Before we try to run macdeployqt, we need to make sure its directory is in the PATH.  (Depending on how Qt
         # was installed, this may or may not have happened automatically.)
         #
         exe_macdeployqt = shutil.which('macdeployqt')
         if (exe_macdeployqt is None or exe_macdeployqt == ''):
            log.debug('Before reading /etc/paths.d/01-qtToolPaths, PATH=' + os.environ['PATH'])
            with open('/etc/paths.d/01-qtToolPaths', 'r') as qtToolPaths:
               for line in qtToolPaths:
                  os.environ["PATH"] = os.environ["PATH"] + os.pathsep + line
            log.debug('After reading /etc/paths.d/01-qtToolPaths, PATH=' + os.environ['PATH'])
            exe_macdeployqt = shutil.which('macdeployqt')

         if (exe_macdeployqt is None or exe_macdeployqt == ''):
            log.error('Cannot find macdeployqt.  PATH=' + os.environ['PATH'])

         #
         # Now let macdeployqt do most of the heavy lifting
         #
         # Note that it is best to run macdeployqt in the directory that contains the [projectName]_[versionNumber].app
         # folder (otherwise, eg, the dmg name it creates will be wrong, as explained at
         # https://doc.qt.io/qt-6/macos-deployment.html.
         #
         # In a previous iteration of this script, I skipped the -dmg option and tried to build the disk image with
         # dmgbuild (code at https://github.com/dmgbuild/dmgbuild, docs at
         # https://dmgbuild.readthedocs.io/en/latest/index.html).  The advantages of this would be that it would make it
         # possible to do further fix-up work on the directory tree (if needed) and, potentially, give us more control
         # over the disk image (eg to specify an icon for it).  However, it seemed to be fiddly to get it to work.  It's
         # a lot simpler to let macdeployqt create the disk image, and we currently don't think we need to do further
         # fix-up work after it's run.  A custom icon on the disk image would be nice, but is far from essential.
         #
         # .:TBD:. Ideally we would sign our application here using the `-codesign=<ident>` command line option to
         #         macdeployqt.  For the GitHub builds, we would have to import a code signing certificate using
         #         https://github.com/Apple-Actions/import-codesign-certs.  (Note that we would need to sign both the
         #         app and the disk image.)
         #
         #         However, getting an identity and certificate with which to sign is a bit complicated. For a start,
         #         Apple pretty much require you to sign up to their $99/year developer program.
         #
         #         As explained at https://wiki.lazarus.freepascal.org/Code_Signing_for_macOS, Apple do not want you to
         #         run unsigned MacOS applications, and are making it every harder to do so.  As of 2024, if you try to
         #         launch an unsigned executable on MacOS that you didn't download from an Apple-approved source, you'll
         #         get two layers of errors:
         #            - First you'll be told that the application "is damaged and can’t be opened. You should move it to
         #              the Trash".  You can fix this by running the xattr command (as suggested at
         #              https://discussions.apple.com/thread/253714860)
         #            - If you now try to run the application, you'll get a different error: that the application "quit
         #              unexpectedly".  When you click on "Report...", you'll see, buried in amongst a huge amount of
         #              other information, the message "Exception Type: EXC_BAD_ACCESS (SIGKILL (Code Signature
         #              Invalid))".  This can apparently be fixed by doing an "ad hoc" signing with the codesign command
         #              (as explained at the aforementioned https://wiki.lazarus.freepascal.org/Code_Signing_for_macOS).
         #
         #         ❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄
         #         ❄
         #         ❄ TLDR for Mac users, once you've built or downloaded the app, you still need to do the following
         #         ❄ "Simon says run this app" incantations to get it to work.  (This is because Apple knows best.
         #         ❄ Do not question the Apple.  Do not step outside the reality distortion field.  Do not pass Go.
         #         ❄ Etc.)  Make sure you have Xcode installed from the Mac App Store (see
         #         ❄ https://developer.apple.com/support/xcode/).  Open the console and run the following (with the
         #         ❄ appropriate substitution for <path/to/application.app>):
         #         ❄
         #         ❄    $ xattr -c <path/to/application.app>
         #         ❄    $ codesign --force --deep -s - <path/to/application.app>
         #         ❄
         #         ❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄❄
         #
         log.debug('Running macdeployqt (PATH=' + os.environ['PATH'] + ')')
         log.debug('qtFrameworksDir=' + qtFrameworksDir)
         os.chdir(btFileSystem.dir_packages_platform)
         btExecute.abortOnRunFail(
            #
            # NOTE: For at least some macdeployqt errors, it will not return an error code, but will merely log an ERROR
            #       message (eg "ERROR: Cannot resolve rpath") and carry on.
            #
            #
            # Note that app bundle name has to be the first parameter and options come afterwards.
            # The -executable argument is to automatically alter the search path of the executable for the Qt libraries
            # (ie so the executable knows where to find them inside the bundle)
            #
            subprocess.run(['macdeployqt',
                            macBundleDirName,
                            '-libpath=' + qtFrameworksDir,
                            '-verbose=2',        # 0 = no output, 1 = error/warning (default), 2 = normal, 3 = debug
                            '-executable=' + macBundleDirName + '/Contents/MacOS/' + capitalisedProjectName,
                            '-dmg'],
                           capture_output=False)
         )

         #
         # The result of specifying the `-dmg' flag should be a [projectName]_[versionNumber].dmg file
         #
         log.debug('Directory tree after running macdeployqt')
         btExecute.abortOnRunFail(subprocess.run(['tree', '-sh'], capture_output=False))
         dmgFileName = macBundleDirName.replace('.app', '.dmg')

         #
         # otool -l = Display the load commands
         # otool -L = Display the names and version numbers of the shared libraries that the object file uses
         #
         log.debug('Running otool on ' + capitalisedProjectName + ' executable after macdeployqt')
         os.chdir(dir_packages_mac_bin)
         btExecute.abortOnRunFail(subprocess.run(['otool', '-L', capitalisedProjectName], capture_output=False))
         btExecute.abortOnRunFail(subprocess.run(['otool', '-l', capitalisedProjectName], capture_output=False))
         log.debug('Running otool on ' + xalanDir + xalanLibName + ' library after macdeployqt')
         os.chdir(dir_packages_mac_frm)
         btExecute.abortOnRunFail(subprocess.run(['otool', '-L', xalanDir + xalanLibName], capture_output=False))

         log.info('Created ' + dmgFileName + ' in directory ' + btFileSystem.dir_packages_platform.as_posix())

         #
         # We can now mount the disk image and check its contents.  (I don't think we can modify the contents though.)
         #
         # By default, a disk image called foobar.dmg will get mounted at /Volumes/foobar.
         #
         log.debug('Running hdiutil to mount ' + dmgFileName)
         os.chdir(btFileSystem.dir_packages_platform)
         btExecute.abortOnRunFail(
            subprocess.run(
               ['hdiutil', 'attach', '-verbose', dmgFileName]
            )
         )
         mountPoint = '/Volumes/' + dmgFileName.replace('.dmg', '')
         log.debug('Directory tree of disk image')
         btExecute.abortOnRunFail(
            subprocess.run(['tree', '-sh', mountPoint], capture_output=False)
         )
         log.debug('Running hdiutil to unmount ' + mountPoint)
         os.chdir(btFileSystem.dir_packages_platform)
         btExecute.abortOnRunFail(
            subprocess.run(
               ['hdiutil', 'detach', '-verbose', mountPoint]
            )
         )

         #
         # Make the checksum file
         #
         log.info('Generating checksum file for ' + dmgFileName)
         writeSha256sum(btFileSystem.dir_packages_platform, dmgFileName)

         os.chdir(previousWorkingDirectory)

      case _:
         log.critical('Unrecognised platform: ' + platform.system())
         exit(1)

   # If we got this far, everything must have worked
   print()
   print('⭐ Packaging complete ⭐')
   print('See:')
   print('   ' + btFileSystem.dir_packages_platform.as_posix() + ' for binaries')
   print('   ' + btFileSystem.dir_packages_source.as_posix() + ' for source')
   return

#-----------------------------------------------------------------------------------------------------------------------
# Act on command line arguments
#-----------------------------------------------------------------------------------------------------------------------
# See above for parsing
match args.subCommand:

   case 'setup':
      doSetup(setupOption = args.setupOption)

   case 'package':
      doPackage()

   case 'appimage':
      btAppImage.doAppImage()

   # If we get here, it's a coding error as argparse should have already validated the command line arguments
   case _:
      log.error('Unrecognised command "' + command + '"')
      exit(1)