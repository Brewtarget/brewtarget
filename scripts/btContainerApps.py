#-----------------------------------------------------------------------------------------------------------------------
# scripts/btContainerApps.py is part of Brewtarget, and is copyright the following authors 2025-2026:
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
# Python built-in modules we use
#-----------------------------------------------------------------------------------------------------------------------
import os
import getpass
import pathlib
import platform
import shutil
import stat
import subprocess

#-----------------------------------------------------------------------------------------------------------------------
# Our own modules
#-----------------------------------------------------------------------------------------------------------------------
import btExecute
import btFileSystem
import btLogger
import btUtils

#
# We attempt to build three different types of container app for Linux:
#  - AppImage
#  - Flatpak
#  - Snap     TODO
#
# There is some overlap between what we do for each container app and what we do in ./bt package, but we accept the
# price of some duplication for keeping things separate.
#

#-----------------------------------------------------------------------------------------------------------------------
# ./bt appimage
#
# Builds an AppImage container app package
#-----------------------------------------------------------------------------------------------------------------------
def doAppImage():
   sysName = platform.system()
   if sysName != 'Linux':
      btLogger.log.critical('AppImage creation not supported on: ' + sysName)
      exit(1)

   # Create the relevant top-level directory and ensure it starts out empty
   # (NB: Any missing parent directories will automatically get created by os.makedirs.  In particular,
   # btFileSystem.dir_contAppPkgs is guaranteed to exist after this.)
   if btFileSystem.dir_appImage.is_dir():
      btLogger.log.info('Removing existing ' + btFileSystem.dir_appImage.as_posix() + ' directory tree')
      shutil.rmtree(btFileSystem.dir_appImage)
   btLogger.log.info('Creating directory ' + btFileSystem.dir_appImage.as_posix())
   os.makedirs(btFileSystem.dir_appImage)

   #
   # Read in the variables exported from the main Meson build
   #
   btUtils.readBuildConfigFile()
   projectName = btUtils.buildConfig["CONFIG_APPLICATION_NAME_LC"]
   versionString = btUtils.buildConfig["CONFIG_VERSION_STRING"]
   appDirName = projectName + '-' + versionString
   reverseDomain = btUtils.buildConfig["CONFIG_ORGANIZATION_REVERSE_DOMAIN"]
   appId = reverseDomain + '.' + btUtils.buildConfig["CONFIG_APPLICATION_NAME_UC"]

   #
   # Make a top level "AppDir" directory for everything that we're going to bundle into the AppImage
   #
   dir_appDir = btFileSystem.dir_appImage.joinpath(appDirName)
   btLogger.log.debug('Creating AppImage top-level directory: ' + dir_appDir.as_posix())
   os.makedirs(dir_appDir)

   #
   # As for when we are doing "regular" packaging, we get Meson to put most of the files we need (bar shared libraries)
   # in a tree where we can get them.  NB: We need to be in the mbuild directory to run Meson.
   #
   # NOTE, per comments in meson.build, this also sets our desired RUNPATH attribute in the ELF section of the
   # executable.
   #
   btUtils.findMesonAndGit()
   btLogger.log.info('Running meson install with --destdir option')
   os.chdir(btFileSystem.dir_build)
   # See https://mesonbuild.com/Commands.html#install for the optional parameters to meson install
   btExecute.abortOnRunFail(
      subprocess.run(
         [btUtils.exe_meson, 'install', '--destdir', dir_appDir.as_posix()],
         capture_output=False
      )
   )

   #
   # See comment in doPackage() in scripts/buildTool.py for why we need to move things up a directory in the Meson
   # output (from usr/local/bin to usr/bin).
   #
   btLogger.log.debug('Moving usr/local files to usr inside ' + dir_appDir.as_posix())
   targetDir = dir_appDir.joinpath('usr')
   sourceDir = targetDir.joinpath('local')
   for fileName in os.listdir(sourceDir.as_posix()):
      btLogger.log.debug('Moving ' + sourceDir.joinpath(fileName).as_posix() + ' to ' + targetDir.as_posix())
      shutil.move(sourceDir.joinpath(fileName), targetDir)
   os.rmdir(sourceDir.as_posix())

   # We change into the AppImage directory.  This doesn't affect the caller (of this script) because we're a separate
   # sub-process from the (typically) shell that invoked us and we cannot change the parent process's working
   # directory.
   os.chdir(btFileSystem.dir_appImage)
   btLogger.log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())

   #
   # We need to download the AppImage tool
   #
   # There are ways to do the equivalent of wget inside Python, but it's a bit of a pain in the neck, so we just call
   # out to the shell to do it.
   #
   appImageTool = 'appimagetool-x86_64.AppImage'
   appImageToolUrl = 'https://github.com/AppImage/appimagetool/releases/download/continuous/' + appImageTool
   btLogger.log.info('Downloading AppImage tool from ' + appImageToolUrl)
   btUtils.findWget()
   btExecute.abortOnRunFail(subprocess.run([btUtils.exe_wget, appImageToolUrl], capture_output=False))
   file_appImageTool = pathlib.Path.cwd().joinpath(appImageTool)
   aitStat = os.stat(file_appImageTool)
   os.chmod(file_appImageTool, aitStat.st_mode | stat.S_IEXEC)

   #
   # The structure of what's inside the AppDir is explained at https://docs.appimage.org/reference/appdir.html, but, in
   # summary, we are trying to create is similar in structure to what we do for a Deb or RPM package.  In the tree
   # below, items marked ✅ are copied as is from the tree generated by meson install with --destdir option, and those
   # marked ❇ are ones we need to relocate, generate or modify:
   #
   #    [projectName]-[version]   <── dir_appDir
   #    ├── AppRun ❇   <── Shell script that acts as the entry point for the AppImage
   #    ├── [reverseDomain].[projectName].desktop = SYMLINK to file in usr/share/applications  ❇
   #    └── usr
   #        ├── bin
   #        │   └── [projectName] ✅   <── the executable
   #        ├── lib   <── dir_appDir_lib
   #        │   └── [All the libraries needed by the application] ❇
   #        └── share
   #            ├── applications   <── dir_applications
   #            │   └── [reverseDomain].[projectName].desktop ✅
   #            ├── [projectName]
   #            │   ├── DefaultData.xml           ✅  <──┬── [filesToInstall_data]
   #            │   ├── default_db.sqlite         ✅  <──┘
   #            │   ├── sounds
   #            │   │   └── [All the filesToInstall_sounds .wav files] ✅
   #            │   └── translations_qm
   #            │       └── [All the .qm files generated by qt.compile_translations] ✅
   #            ├── icons   <── dir_icons
   #            │   └── hicolor
   #            │       └── scalable
   #            │           └── apps
   #            │               └── [projectName].svg ✅  <── [filesToInstall_icons]
   #            └── metainfo   <── dir_metainfo
   #                └── [reverseDomain].[projectName].appdata.xml ❇
   #

   # Make a symlink to the .desktop file in the AppImage root folder
   dir_applications = dir_appDir.joinpath('usr').joinpath('share').joinpath('applications')
   os.symlink(dir_applications.relative_to(dir_appDir).joinpath(appId + '.desktop'),
              dir_appDir.joinpath(appId + '.desktop'))
   # Move [projectName].svg to the correct place for AppImage
   dir_icons = dir_appDir.joinpath('usr').joinpath('share').joinpath('icons')
   shutil.move(dir_icons.joinpath('hicolor').joinpath('scalable').joinpath('apps').joinpath(projectName + '.svg'),
               dir_appDir.as_posix())
   shutil.rmtree(dir_icons)

   #
   # One of the warning messages from the AppImage generator is "AppStream upstream metadata is missing" if it can't
   # find usr/share/metainfo/[reverseDomain].appdata.xml.  It points you to more information at
   # https://www.freedesktop.org/software/appstream/docs/chap-Quickstart.html.  The page at that link tells you the
   # metadata file should be called /usr/share/metainfo/%{id}.metainfo.xml (where %{id} follows a reverse-DNS scheme,
   # consisting of {tld}.{vendor}.{product}.
   #
   # TLDR is we need to rename [reverseDomain].metainfo.xml to [reverseDomain].appdata.xml
   #
   dir_metainfo = dir_appDir.joinpath('usr').joinpath('share').joinpath('metainfo')
   shutil.move(dir_metainfo.joinpath(appId + '.metainfo.xml'),
               dir_metainfo.joinpath(appId + '.appdata.xml'))

   # Create the AppRun shell script...
   file_appRun = dir_appDir.joinpath('AppRun')
   with open(file_appRun, 'w') as appRunFile:
      appRunFile.write('#!/bin/bash\n')
      appRunFile.write('exec $APPDIR/usr/bin/' + projectName + '\n')
   # ...and make it executable
   stat_appRun = os.stat(file_appRun)
   os.chmod(file_appRun, stat_appRun.st_mode | stat.S_IEXEC)

   #
   # AppImage requires the application to be relocatable -- ie able to load its shared libraries relative to its main
   # binary, rather than just hardcoded absolute paths (such as /usr/lib and subdirectories thereof).
   #
   # RPATH (aka DT_RPATH) and RUNPATH (aka DT_RUNPATH) are ELF (Executable and Linkable Format) attributes embedded in
   # binaries or shared libraries.  They specify the directory/directories the dynamic linker/loader should search for
   # dependencies (eg shared libraries that need to be loaded).
   #
   # RPATH path is an older setting than RUNPATH, and generally the latter should be used.  (The difference is simply
   # RPATH takes precedence over the LD_LIBRARY_PATH environment variable, which is bad, whereas RUNPATH does not.)  In
   # various places -- eg parameters to patchelf -- you will see `rpath` used in parameter names where it is actually
   # RUNPATH being displayed/modified, but you generally don't have to worry about this.  (Specifically, per
   # https://twdev.blog/2024/08/rpath/, RUNPATH is available if you specify `--enable-new-dtags` when linking.  This
   # enables the generation of so called “new tags”.  Once enabled, requests to populate RPATH will in fact populate
   # RUNPATH.)
   #
   # You can see values (if specified) for RPATH and RUNPATH for an existing binary by running `readelf -d [binary]` or
   # `patchelf --print-rpath [binary]`.
   #
   # Inside an RPATH/RUNPATH, "$ORIGIN" means the directory containing the executable (or shared library if that's what
   # you're building).
   #
   #
   executableFullPath = dir_appDir.joinpath('usr').joinpath('bin').joinpath(projectName).as_posix()
   btExecute.abortOnRunFail(
      subprocess.run(
         #
         # Note that we don't want $ORIGIN interpreted as a shell variable, so extra quotes needed
         #
         ["patchelf", "--add-rpath", "'$ORIGIN/../lib'", executableFullPath]
      )
   )

   #
   # Now we need to copy the shared libraries on which we depend.  There are various ways to obtain the list of these
   # dependencies.  Using ldd is simplest, because it handles recursion.
   #
   lddOutput = btExecute.abortOnRunFail(
      subprocess.run(
         ['ldd', executableFullPath],
         capture_output=True
      )
   ).stdout.decode('UTF-8')
   btLogger.log.debug('Output of `ldd ' + executableFullPath + '`: ' + lddOutput)

   #
   # Most of the output of ldd will be of the form:
   #
   #    libQt6Core.so.6 => /lib/x86_64-linux-gnu/libQt6Core.so.6 (0x000074b991200000)
   #
   # We can ignore lines not of this format, and also assume they will have fewer fields, eg:
   #    linux-vdso.so.1 (0x000074b99346f000)
   #
   dir_appDir_lib = dir_appDir.joinpath('usr').joinpath('lib')
   os.makedirs(dir_appDir_lib)
   for lddOutputLine in lddOutput.splitlines():
      fields = lddOutputLine.strip().split()
      if len(fields) == 4:
         libPath = fields[2]
         newLibPath = dir_appDir_lib.joinpath(os.path.basename(libPath))
         btLogger.log.debug('Copying ' + libPath + ' to ' + newLibPath.as_posix())
         shutil.copy2(libPath, newLibPath)
         #
         # There's one more thing we need to do here.  Although (courtesy of invoking patchelf above) the executable
         # will look in this directory for its shared libraries, the shared libraries themselves will not.  Thus, if a
         # shared library in this directory depends on another shared library in this directory, it won't find it.
         #
         # So, we need to fix RUNPATH on all the shared libraries -- just as we did on the executable (via install_rpath
         # in meson.build).
         #
         btExecute.abortOnRunFail(
            subprocess.run(
               #
               # Note that we don't want $ORIGIN interpreted as a shell variable, so extra quotes needed
               #
               ["patchelf", "--add-rpath", "'$ORIGIN/../lib'", newLibPath.as_posix()]
            )
         )
      else:
         btLogger.log.debug('Skipping "' + lddOutputLine + '"')
   btLogger.log.info('Libraries to ship (in ' + dir_appDir_lib.as_posix() + '):')
   os.listdir(dir_appDir_lib.as_posix())

   #
   # Now we run the AppImage tool to turn the directory tree into an app image.  Note that we first have to set the ARCH
   # environment variable.
   #
   os.environ['ARCH'] = 'x86_64'
   file_appImage = appDirName + '.AppImage'
   btExecute.abortOnRunFail(
      subprocess.run(
         [file_appImageTool, '--verbose', dir_appDir.as_posix(), file_appImage],
         capture_output=False
      )
   )

   btUtils.writeSha256sum(btFileSystem.dir_appImage, file_appImage)

   return

#-----------------------------------------------------------------------------------------------------------------------
# ./bt flatpak
#
# Builds a Flatpak container app package
#-----------------------------------------------------------------------------------------------------------------------
def doFlatpak():
   sysName = platform.system()
   if sysName != 'Linux':
      btLogger.log.critical('Flatpak creation not supported on: ' + sysName)
      exit(1)

   btLogger.log.info('Installing flatpak')
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'update']))
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'flatpak']))
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'flatpak-builder']))

   #
   # Read in the variables exported from the Meson build
   #
   btUtils.readBuildConfigFile()
   projectName = btUtils.buildConfig["CONFIG_APPLICATION_NAME_LC"]
   versionString = btUtils.buildConfig["CONFIG_VERSION_STRING"]
   appDirName = projectName + '-' + versionString

   # Create the relevant top-level directory and ensure it starts out empty
   # (NB: Any missing parent directories will automatically get created by os.makedirs.  In particular,
   # btFileSystem.dir_contAppPkgs and btFileSystem.dir_flatpak are guaranteed to exist after this.)
   if btFileSystem.dir_flatpak.is_dir():
      btLogger.log.info('Removing existing ' + btFileSystem.dir_flatpak.as_posix() + ' directory tree')
      shutil.rmtree(btFileSystem.dir_flatpak)
   dir_flatpakBuild = btFileSystem.dir_flatpak.joinpath(appDirName)
   btLogger.log.info('Creating directory ' + dir_flatpakBuild.as_posix())
   os.makedirs(dir_flatpakBuild)
   os.chdir(btFileSystem.dir_flatpak)
   btLogger.log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())

   #
   # Getting your head around how Flatpak works is a bit of a steep learning curve.  Some of the online documentation is
   # out-of-date or inconsistent, and a lot of the rest of it assumes either that you are doing something trivial or
   # that you already know everything else about Flatpak.
   #
   # Official instructions for generating Flatpaks are at https://docs.flatpak.org/en/latest/first-build.html, but there
   # are some helpful elaborations at https://develop.kde.org/docs/packaging/flatpak/packaging/.  At the start, you read
   # that there are just three steps:
   #    (1) Install the necessary Flatpak SDK and Runtime for your application
   #    (2) Create a Flatpak manifest (in JSON or YAML)
   #    (3) Build the binary
   # But this hides a huge amount of detail required for step 2.  (Note that (1) and (3) can be combined, by telling the
   # build to automatically install dependencies, but this requires doing the build as root, so we don't.)
   #
   # The first thing to understand is that, although we have already built the application, we can't just bundle it up
   # with a few libraries into a Flatpak (eg as we would if we were making a .deb or .rpm package).  There is, for
   # example, no guarantee that the dependencies etc from the build machine would work inside the Flatpak container.
   # Instead, the idea is that you are supposed to build everything from scratch inside a Flatpak container -- except
   # for the bits where you can rely on other Flatpak modules.   See
   # https://docs.flatpak.org/en/latest/under-the-hood.html and https://docs.flathub.org/docs/for-app-authors/submission
   # for more on this.
   #

   #
   # (1) Install the necessary Flatpak SDK and Runtime for your application
   #     We obtain these from the common flatpak repository, Flathub
   #
   # Flatpak allows architectures and versions to be specified using an object’s identifier triple. This takes the form
   # of name/architecture/branch, such as com.company.App/i386/stable. (Branch is the term used to refer to versions of
   # the same object.) The first part of the triple is the ID, the second part is the architecture, and the third part
   # is the branch.
   #
   # Identifier triples can also be used to specify just the architecture or the branch, by leaving part of the triple
   # blank. For example, com.company.App//stable would just specify the branch, and com.company.App/i386// just
   # specifies the architecture.
   #
   # NOTE that, for KDE SDKs and Runtimes, there isn't a version marked "latest", so you have to explicitly specify the
   # one you want.  (If you don't specify which version you want, you'll get an interactive prompt listing all available
   # versions and inviting ou to select one.)
   #
   runtimeVersion = '6.10'  # As of 2025-11-29, this is the "current" version
   openJdkVersion = '25.08' # Ditto!
   btLogger.log.info('Installing Flatpak SDK and Runtime')
   btExecute.abortOnRunFail(
      #
      # Here and elsewhere, using the '--user' option restricts the installs to the current user (rather than
      # system-wide), which is sufficient for our needs for the packaging process.  Moreover, this allows us to run
      # inside a GitHub Action (where attempting the default system-wide install of a flatpak gives an error).
      #
      # NOTE that, if needed, we can add the '--verbose' flag here, and to other invocations of the `flatpak` command.
      # However, we usually don't because we end up generating too much output, and these "standard installs" are not
      # typically where we hit problems.
      #
      subprocess.run(
         ['flatpak', '--user', 'remote-add', '--if-not-exists', 'flathub', 'https://flathub.org/repo/flathub.flatpakrepo'],
         capture_output=False
      )
   )
   # TBD: in theory we don't need to install the platform -- it's needed to run the code but not to build it
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--user', 'install', '--assumeyes', 'org.kde.Platform//' + runtimeVersion],
         capture_output=False
      )
   )
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--user', 'install', '--assumeyes', 'org.kde.Sdk//' + runtimeVersion],
         capture_output=False
      )
   )
   # I'm not totally sure that we need Java, but the Xerces build complains if it's not present
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--user', 'install', '--assumeyes', 'org.freedesktop.Sdk.Extension.openjdk11//' + openJdkVersion],
         capture_output=False
      )
   )
   # We do, however, definitely need this patch for XercesC
   btLogger.log.info('Downloading Xerces patch')
   btExecute.abortOnRunFail(
      subprocess.run(
         ['wget',
          '--output-document=' + btFileSystem.dir_flatpak.joinpath('xerces-c-3.2.5-cxx17.patch').as_posix(),
          'https://gitweb.gentoo.org/repo/gentoo.git/plain/dev-libs/xerces-c/files/xerces-c-3.2.5-cxx17.patch'],
         capture_output=False
      )
   )
   # Various patches needed to build XalanC
   btLogger.log.info('Downloading Xalan patches')
   for xalanPatch in ['xalan-c-1.12-cmake-4.patch',
                      'xalan-c-1.12-gcc-15.patch',
                      'xalan-c-1.12-fix-lto.patch',
                      'xalan-c-1.12-fix-threads.patch',
                      'xalan-c-1.12-icu-75.patch']:
      btExecute.abortOnRunFail(
         subprocess.run(
            ['wget',
             '--output-document=' + btFileSystem.dir_flatpak.joinpath(xalanPatch).as_posix(),
             'https://gitweb.gentoo.org/repo/gentoo.git/plain/dev-libs/xalan-c/files/' + xalanPatch],
            capture_output=False
         )
      )

   btLogger.log.info('Installing Flatpak Linter')
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--user', 'install', '--assumeyes', 'org.flatpak.Builder'],
         capture_output=False
      )
   )

   #
   # Since we have to rebuild everything, we need the source code.  We want this in a subdirectory of the one holding
   # the manifest, because flatpak-builder is going to copy it and we don't want to be trying to copy a directory tree
   # inside itself.
   #
   btLogger.log.info('Copy source tree etc')
   dir_copyOfSource = btFileSystem.dir_flatpak.joinpath('copyOfSource')
   os.makedirs(dir_copyOfSource)
   for directoryToCopy in ['css',
                           'data',
                           'doc',
                           'fonts',
                           'images',
                           'linux',  # For desktop.in
                           'packaging',
                           'third-party',
                           'schemas',
                           'src',
                           'translations',
                           'ui']:
      shutil.copytree(btFileSystem.dir_base.joinpath(directoryToCopy), dir_copyOfSource.joinpath(directoryToCopy))
   for fileToCopy in ['CHANGES.markdown',
                      'CMakeLists.txt',
                      'configure',
                      'COPYRIGHT',
                      'LICENSE',
                      'meson.build',
                      'README.md',
                      'resources.qrc']:
      shutil.copy(btFileSystem.dir_base.joinpath(fileToCopy), dir_copyOfSource)

   #
   # (2) Create a Flatpak manifest
   #
   # Flatpak manifest files are named in reverse-domain name notation
   # (https://en.wikipedia.org/wiki/Reverse_domain_name_notation) -- eg www.company.com -> com.company.application.
   # We don't really have a company or organisation name, so we'll go with:
   #    www.brewtarget.beer -> beer.brewtarget.Brewtarget
   #    www.brewken.com     -> com.brewken.Brewken
   #
   reverseDomain = btUtils.buildConfig["CONFIG_ORGANIZATION_REVERSE_DOMAIN"]
   appId = reverseDomain + '.' + btUtils.buildConfig["CONFIG_APPLICATION_NAME_UC"]
   file_manifest = btFileSystem.dir_flatpak.joinpath(appId + '.yml')
   if file_manifest.is_file():
      btLogger.log.info('Removing existing manifest file ' + file_manifest.as_posix())
      os.remove(file_manifest)

   #
   # Meson generates our flatpak manifest (from a template in the packaging/linux directory), so we just need to copy
   # and rename it here.
   #
   file_manifest_ori = btFileSystem.dir_build.joinpath('flatpackManifest.yml')
   btLogger.log.debug('Copying ' + file_manifest_ori.as_posix() + ' to ' + file_manifest.as_posix())
   shutil.copy2(file_manifest_ori, file_manifest)

   #
   # Before we try to use the manifest, we can run it through the linter to check for errors
   #
   btLogger.log.info('Running Flatpak Linter')
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak',
               '--verbose',
               'run',
               '--command=flatpak-builder-lint',
               'org.flatpak.Builder',
               'manifest',
               file_manifest.as_posix()],
         capture_output=False
      )
   )

   #
   # (3) Build the binary
   #
   # In theory, we have a choice here.  Using `flatpak-builder` is a manifest-driven wrapper around various
   # `flatkpak build` commands, so we could run those commands directly.  However, this would be going against the grain
   # of how flatpaks are supposed to be submitted to flathub, etc.  So we should bite the bullet and get the manifest
   # working.  Nonetheless, it helps to understand what flatpak-builder is doing under the hood.  Specifically, the
   # manifest file defines how flatpak-builder should:
   #
   #     Download all sources
   #
   #     Initialize the application directory with flatpak build-init
   #
   #     Build and install each module with flatpak build
   #
   #     Clean up the final build tree by removing unwanted files and e.g. stripping binaries
   #
   #     Finish the application directory with flatpak build-finish
   #
   # After this you will end up with a build of the application in DIRECTORY , which you can export to a repository with
   # the flatpak build-export command. If you use the --repo option, flatpak-builder will do the export for you at the
   # end of the build process. When flatpak-builder does the export, it also stores the manifest that was used for the
   # build in /app/manifest.json. The manifest is 'resolved', i.e. git branch names are replaced by the actual commit
   # IDs that were used in the build.
   #
   btLogger.log.info('Running Flatpak Builder')
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak-builder',
          '--verbose',
          dir_flatpakBuild.as_posix(),
          file_manifest.as_posix()],
         capture_output=False
      )
   )

   btLogger.log.info('Flatpak built into ' + dir_flatpakBuild.as_posix())

   #
   # (4) Turn what was built into a single-file bundle
   #
   # See https://unix.stackexchange.com/questions/695934/how-do-i-build-a-flatpak-package-file-from-a-flatpak-manifest
   #
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak',
          'build-export',
          'export',
          dir_flatpakBuild.as_posix()],
         capture_output=False
      )
   )

   file_singleFileBundle = appDirName + '.flatpak'
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak',
          'build-bundle',
          'export',
          file_singleFileBundle,
          appId],
         capture_output=False
      )
   )

   btLogger.log.info('Flatpak single-file bundle ' + file_singleFileBundle)

   btUtils.writeSha256sum(btFileSystem.dir_flatpak, file_singleFileBundle)
   return

#-----------------------------------------------------------------------------------------------------------------------
# ./bt snap
#
# Builds a Snap container app package
#-----------------------------------------------------------------------------------------------------------------------
def doSnap():
   sysName = platform.system()
   if sysName != 'Linux':
      btLogger.log.critical('Snap creation not supported on: ' + sysName)
      exit(1)

   #
   # Read in the variables exported from the Meson build
   #
   btUtils.readBuildConfigFile()
   projectName = btUtils.buildConfig["CONFIG_APPLICATION_NAME_LC"]
   versionString = btUtils.buildConfig["CONFIG_VERSION_STRING"]
   appDirName = projectName + '-' + versionString

   # Create the relevant top-level directory and ensure it starts out empty
   # (NB: Any missing parent directories will automatically get created by os.makedirs.  In particular,
   # btFileSystem.dir_contAppPkgs and btFileSystem.dir_flatpak are guaranteed to exist after this.)
   if btFileSystem.dir_snap.is_dir():
      btLogger.log.info('Removing existing ' + btFileSystem.dir_snap.as_posix() + ' directory tree')
      shutil.rmtree(btFileSystem.dir_snap)
   dir_snapBuild = btFileSystem.dir_snap.joinpath(appDirName)
   btLogger.log.info('Creating directory ' + dir_snapBuild.as_posix())
   os.makedirs(dir_snapBuild)
   os.chdir(btFileSystem.dir_snap)
   btLogger.log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())

   #--------------------------------------------------------------------------------------------------------------------
   #---------------------------------------------------- Build Snap ----------------------------------------------------
   #--------------------------------------------------------------------------------------------------------------------
   sysName = platform.system()
   if sysName != 'Linux':
      btLogger.log.critical('Snap creation not supported on: ' + sysName)
      exit(1)

   # TODO: Haven't managed to get this working yet.  Meson build keeps hanging.  Next thing might be to try the CMake
   #       build to see if it gives us more/different diagnostics.

   #
   # Read in the variables exported from the Meson build
   #
   btUtils.readBuildConfigFile()
   projectName = btUtils.buildConfig["CONFIG_APPLICATION_NAME_LC"]
   versionString = btUtils.buildConfig["CONFIG_VERSION_STRING"]
   appDirName = projectName + '-' + versionString

   #
   # Create the relevant top-level directory and ensure it starts out empty
   # (NB: Any missing parent directories will automatically get created by os.makedirs.  In particular,
   # btFileSystem.dir_contAppPkgs and btFileSystem.dir_snap are guaranteed to exist after this.)
   if btFileSystem.dir_snap.is_dir():
      btLogger.log.info('Removing existing ' + btFileSystem.dir_snap.as_posix() + ' directory tree')
      shutil.rmtree(btFileSystem.dir_snap)
   dir_snapBuild = btFileSystem.dir_snap.joinpath(appDirName)
   dir_snapBuildSnap = dir_snapBuild.joinpath('snap')
   btLogger.log.info('Creating directory ' + dir_snapBuildSnap.as_posix())
   os.makedirs(dir_snapBuildSnap)
   os.chdir(dir_snapBuild)
   btLogger.log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())

   #
   # NOTE: AFAICT in order to build snaps, you need, amongst other things, to enable them to be installed on your
   # system -- which not everyone necessarily wants.  We therefore only install the snap tools here rather than in
   # btDependencies.installDependencies.
   #
   btLogger.log.info('Installing Snapd')
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'update']))
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'snapd']))
   btLogger.log.info('Installing Snapcraft')
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'snap', 'install', '--classic', 'snapcraft']))
   # See comment below for why we install Multipass
   btLogger.log.info('Installing Multipass')
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'snap', 'install', 'multipass']))

   #
   # Part of the Snap infrastucture which gets installed with snapd is Canonical's LXD (see https://www.lxdubuntu.com/),
   # a System Container and Virtual Machine Manager built on top of LXC (Linux Containers).
   #
   # The snap build process requires a "build provider".  According to
   # https://documentation.ubuntu.com/snapcraft/8.9.4/how-to/setup/set-up-snapcraft/, this is "to host an isolated build
   # environment, like a sandbox [inside which] software can be built and packaged as snaps without making potentially
   # destructive changes to the host system".
   #
   # LXD is the default "build provider" when you run `snapcraft pack`.  For it to work, the current user needs to be
   # added to the 'lxd' group (which the installation of snapd will have created).  This is a bit fiddly, but is
   # achievable.  Adding the current user to a group will not take immediate effect.  Normally you need to log out and
   # log in again.  (In an interactive shell, the `newgrp` command gets around this but that wouldn't help us here: the
   # script would pause, dump the user out into the new shell, with the group membership, and then only resume, without
   # the group membership, when the user exited that shell.)  We can get around this by using the `sg` command to invoke
   # snapcraft.  HOWEVER, when I did all this on my machine I didn't get too far: `snapcraft pack` barfed with an error
   # about "lxc --project snapcraft launch craft-com.ubuntu.cloud-buildd:core24 ..." failing because "Failed to run:
   # /snap/lxd/current/sbin/lxd forkstart snapcraft_base-instance-snapcraft-buildd-base-v71--f02c2da881cfba9d7924 ...
   # exit status 1".  I struggled to get more detailed diagnostics or to find additional useful info on the web.
   #
   # So, I switched to the "build provider", Multipass (see https://canonical.com/multipass), which is a simpler tool
   # for creating VMs on-demand.
   #
   # https://documentation.ubuntu.com/snapcraft/stable/how-to/select-a-build-provider/index.html explains the various
   # ways to set choice of build provider.  (Although the command-line flag would be simpler, there is only '--use-lxd'
   # and no '--use-multipass'!)
   #
   os.environ['SNAPCRAFT_BUILD_ENVIRONMENT'] = 'multipass'

   #
   # By default, the build VM is assigned 2 CPUs and 2 GB RAM.  I wondered if this might be what was causing the
   # compilation to hang, so bumped it a bit here, but it seems to hang at the same place regardless.
   #
   os.environ['SNAPCRAFT_BUILD_ENVIRONMENT_CPU'] = '4'
   os.environ['SNAPCRAFT_BUILD_ENVIRONMENT_MEMORY'] = '16G'

   #
   # The how-to guide tells you to run `snapcraft init` now, but that doesn't so much useful -- just create a 'snap'
   # subdirectory (which we already did) and put in it a default 'snapcraft.yaml' file (which we would need to modify
   # anyway, so we might as well just create it ourselves).
   #
   file_snapProjectFrom = btFileSystem.dir_build.joinpath('snapcraft.yaml')
   file_snapProjectTo   = dir_snapBuildSnap.joinpath('snapcraft.yaml')
   btLogger.log.debug('Copying ' + file_snapProjectFrom.as_posix() + ' to ' + file_snapProjectTo.as_posix())
   shutil.copy2(file_snapProjectFrom, file_snapProjectTo)

   #
   # It would be nice at this point to run `snapcraft lint snap/snapcraft.yaml`.  However, this barfs the following
   # error:
   #    'snapcraft lint' is not supported with Multipass as the build provider
   # So, we skip the lint.
   #

   #
   # Per comments below, the build VM gets reused, so we want to ensure there isn't stuff from previous builds
   # cluttering it up.  (This is more important when we're actively working on the snap build process, but feels like
   # good general practice too.)
   #
   btLogger.log.info('Running snapcraft clean in ' + pathlib.Path.cwd().as_posix())
   btExecute.abortOnRunFail(subprocess.run(['snapcraft', 'clean']))

   #
   # As with Flatpak, Snap expects us to build everything from source, so we need to copy the source code etc into the
   # directory tree its build provider has access to.  It helps to understand what the Snapcraft tool does when we run
   # it.  Per info at https://documentation.ubuntu.com/snapcraft/8.9.3/reference/processes/snap-build-process/,
   # snapcraft does the following:
   #    - Starting in the current working directory, it looks for snapcraft.yaml or snap/snapcraft.yaml, and then parses
   #      its contents to begin the build.
   #
   #    - It creates a build VM (via Multipass or LXD) in which to do the build.  This VM will be a "minimal Ubuntu
   #      install" determined by the 'base' in snapcraft.yaml.  NOTE that the build VM doesn't get recreated from
   #      scratch if you've run a build before, so, during development, it's helpful to run 'snapcraft clean' between
   #      builds, otherwise it can get confusing with bits hanging around from earlier builds.
   #
   #    - Required packages listed in the snapcraft.yaml file are downloaded and installed into the build VM.  Inside
   #      this VM, the /root/project directory will hold a copy of everything in the dir_snapBuild directory (from
   #      outside the VM).
   #
   #    - The "Pull" build step is run (on each part).
   #      Each part has its own directory under /root/parts in the build VM.
   #      We only have one part, called main-part, in our snap/snapcraft.yaml file.  Because we put 'source: ../project'
   #      in the 'main-part' section of snap/snapcraft.yaml file, we'll end up with the
   #      entire contents of the dir_snapBuild directory in /root/parts/main-part/src in the build VM.
   #
   #    - The "Build" build step is run (on each part)
   #      Snapcraft creates /root/parts/main-part/build to do the build, which will be the current working directory
   #      when the "Build" build step is run.  This is different from our usual set-up -- hence why in our
   #      snap/snapcraft.yaml file we specify `meson setup . ../src` rather than `meson setup mbuild .`
   #
   #    - The "Stage" build step is run (on each part)
   #
   #    - The "Prime" build step is run (on each part)
   #
   #    - The "Pack" build step is run (on each part)
   #
   # Note that because each build step ensures the prior ones have been run, we only _need_ to call 'snapcraft pack'.
   # However, it is possible to run steps individually, which is a bit slower (because the VM gets fired up and down
   # more times) but helpful for debugging.  What's even more helpful is that you can drop out into a shell in the VM by
   # adding '--shell', '--shell-after' or '--debug' to the 'snapcraft' command (see
   # https://documentation.ubuntu.com/snapcraft/stable/how-to/debugging/debug-a-snap/#inspect-the-snap-s-contents).
   # Specifically:
   #
   #    'snapcraft foobar --shell'       runs all the build steps _prior_ to foobar, then opens a shell into the
   #                                     environment.
   #    'snapcraft foobar --shell-after' runs all the build steps up to and _including_ foobar, then opens a shell into
   #                                     the environment.
   #    'snapcraft foobar --debug'       runs all the build steps up to and _including_ foobar, then, if there was an
   #                                     error, opens a shell into the environment.
   #
   # NB flags such as '--shell' have to come after the build step -- eg `snacraft --debug build` is wrong; needs to be
   # `snapcraft build --debug`.
   #
   # NOTE that everything in dir_snapBuild EXCEPT the 'snap'
   # subdirectory (ie dir_snapBuildSnap) ends up under /root/parts/main-part/src/project/ (aka
   # ${CRAFT_PART_SRC}/project/) in the virtual build environment (where 'main-part' is the part name defined in
   # snapcraft.yaml).  The 'snap' subdirectory ends up as /root/project/snap/ (aka ${CRAFT_PROJECT_DIR}/snap/).
   #
   # What the build
   #
   btLogger.log.info('Copy source tree etc')
   for directoryToCopy in ['css',
                           'data',
                           'doc',
                           'fonts',
                           'images',
                           'linux',  # For desktop.in
                           'packaging',
                           'third-party',
                           'schemas',
                           'src',
                           'translations',
                           'ui']:
      shutil.copytree(btFileSystem.dir_base.joinpath(directoryToCopy), dir_snapBuild.joinpath(directoryToCopy))
   for fileToCopy in ['CHANGES.markdown',
                      'CMakeLists.txt',
                      'configure',
                      'COPYRIGHT',
                      'LICENSE',
                      'meson.build',
                      'README.md',
                      'resources.qrc']:
      shutil.copy(btFileSystem.dir_base.joinpath(fileToCopy), dir_snapBuild)

   #
   # Provided the contents of snapcraft.yaml are correct, snapcraft supposedly does everything else for us.
   #
   # The "finger snap" emoji is to make these lines stand out in the log file!
   #
   btLogger.log.info('🫰 Running snapcraft pull in ' + pathlib.Path.cwd().as_posix())
   btExecute.abortOnRunFail(subprocess.run(['snapcraft', 'pull', '--verbose']))
   btLogger.log.info('🫰 Running snapcraft build in ' + pathlib.Path.cwd().as_posix())
   btExecute.abortOnRunFail(subprocess.run(['snapcraft', 'build', '--verbose']))
   btLogger.log.info('🫰 Running snapcraft stage in ' + pathlib.Path.cwd().as_posix())
   btExecute.abortOnRunFail(subprocess.run(['snapcraft', 'stage', '--verbose']))
   btLogger.log.info('🫰 Running snapcraft prime in ' + pathlib.Path.cwd().as_posix())
   btExecute.abortOnRunFail(subprocess.run(['snapcraft', 'prime', '--verbose']))
   btLogger.log.info('🫰 Running snapcraft pack in ' + pathlib.Path.cwd().as_posix())
   btExecute.abortOnRunFail(subprocess.run(['snapcraft', 'pack', '--verbose']))

   # TODO - Finish this!

   return
