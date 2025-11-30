#-----------------------------------------------------------------------------------------------------------------------
# scripts/btAppImage.py is part of Brewtarget, and is copyright the following authors 2025:
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

log = btLogger.getLogger()

#-----------------------------------------------------------------------------------------------------------------------
# ./bt appimage
#
# There is some overlap with what we do in ./bt package, but we accept the price of some duplication for keeping
# "regular" packaging separate from AppImage etc construction.
#-----------------------------------------------------------------------------------------------------------------------
def doAppImage():
   sysName = platform.system()
   if sysName != 'Linux':
      log.critical('AppImage creation not supported on: ' + sysName)
      exit(1)

   # Create the relevant top-level directory and ensure it starts out empty
   # (NB: Any missing parent directories will automatically get created by os.makedirs.  In particular,
   # btFileSystem.dir_containerized is guaranteed to exist after this.)
   if (btFileSystem.dir_containerized.is_dir()):
      log.info('Removing existing ' + btFileSystem.dir_containerized.as_posix() + ' directory tree')
      shutil.rmtree(btFileSystem.dir_containerized)
   log.info('Creating directory ' + btFileSystem.dir_appImage.as_posix())
   os.makedirs(btFileSystem.dir_appImage)

   #
   # As for when we are doing "regular" packaging, we get Meson to put most of the files we need (bar shared libaries)
   # in a tree where we can get them.  NB: We need to be in the mbuild directory to run Meson.
   #
   btUtils.findMesonAndGit()
   log.info('Running meson install with --destdir option')
   os.chdir(btFileSystem.dir_build)
   # See https://mesonbuild.com/Commands.html#install for the optional parameters to meson install
   btExecute.abortOnRunFail(
      subprocess.run(
         [btUtils.exe_meson, 'install', '--destdir', btFileSystem.dir_containerized.as_posix()],
         capture_output=False
      )
   )

   #
   # See comment in doPackage() in scripts/buildTool.py for why we need to move things up a directory in the Meson
   # output (from usr/local/bin to usr/bin).
   #
   log.debug('Moving usr/local files to usr inside ' + btFileSystem.dir_containerized.as_posix())
   targetDir = btFileSystem.dir_containerized.joinpath('usr')
   sourceDir = targetDir.joinpath('local')
   for fileName in os.listdir(sourceDir.as_posix()):
      log.debug('Moving ' + sourceDir.joinpath(fileName).as_posix() + ' to ' + targetDir.as_posix())
      shutil.move(sourceDir.joinpath(fileName), targetDir)
   os.rmdir(sourceDir.as_posix())

   # We change into the AppImage directory.  This doesn't affect the caller (of this script) because we're a separate
   # sub-process from the (typically) shell that invoked us and we cannot change the parent process's working
   # directory.
   os.chdir(btFileSystem.dir_appImage)
   log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())

   #
   # We need to download the AppImage tool
   #
   # There are ways to do the equivalent of wget inside Python, but it's a bit of a pain in the neck, so we just call
   # out to the shell to do it.
   #
   appImageTool = 'appimagetool-x86_64.AppImage'
   appImageToolUrl = 'https://github.com/AppImage/appimagetool/releases/download/continuous/' + appImageTool
   log.info('Downloading AppImage tool from ' + appImageToolUrl)
   btUtils.findWget()
   btExecute.abortOnRunFail(subprocess.run([btUtils.exe_wget, appImageToolUrl], capture_output=False))
   file_appImageTool = pathlib.Path.cwd().joinpath(appImageTool)
   aitStat = os.stat(file_appImageTool)
   os.chmod(file_appImageTool, aitStat.st_mode | stat.S_IEXEC)

   #
   # Read in the variables exported from the Meson build
   #
   btUtils.readBuildConfigFile()
   projectName = btUtils.buildConfig["CONFIG_APPLICATION_NAME_LC"]
   versionString = btUtils.buildConfig["CONFIG_VERSION_STRING"]
   appDirName = projectName + '-' + versionString

   #
   # Make a top level "AppDir" directory for everything that we're going to bundle into the AppImage
   #
   dir_appDir = btFileSystem.dir_appImage.joinpath(appDirName)
   log.debug('Creating AppImage top-level directory: ' + dir_appDir.as_posix())
   os.makedirs(dir_appDir)

   #
   # The structure of what's inside the AppDir is explained at https://docs.appimage.org/reference/appdir.html, but, in
   # summary, we are trying to create is similar in structure to what we do for a Deb or RPM package.  In the tree
   # below, items marked ✅ are copied as is from the tree generated by meson install with --destdir option, and those
   # marked ❇ are ones we need to relocate, generate or modify:
   #
   #    [projectName]-[version]   <── dir_appDir
   #    ├── AppRun ❇   <── Shell script that acts as the entry point for the AppImage
   #    ├── [projectName].desktop     ❇  <── [filesToInstall_desktop]
   #    └── usr
   #        ├── bin
   #        │   └── [projectName] ✅   <── the executable
   #        ├── lib
   #        │   └── [All the libraries needed by the application]
   #        └── share
   #            ├── [projectName]
   #            │   ├── DefaultData.xml           ✅  <──┬── [filesToInstall_data]
   #            │   ├── default_db.sqlite         ✅  <──┘
   #            │   ├── sounds
   #            │   │   └── [All the filesToInstall_sounds .wav files] ✅
   #            │   └── translations_qm
   #            │       └── [All the .qm files generated by qt.compile_translations] ✅
   #            └── icons
   #                └── hicolor
   #                    └── scalable
   #                        └── apps
   #                            └── [projectName].svg ✅  <── [filesToInstall_icons]
   #

   # Copy the linux/usr tree inside the top-level directory for the app image
   log.debug('Copying package contents')
   shutil.copytree(btFileSystem.dir_containerized.joinpath('usr'), dir_appDir.joinpath('usr'))
   # Move [projectName].desktop to the correct place for AppImage
   dir_applications = dir_appDir.joinpath('usr').joinpath('share').joinpath('applications')
   shutil.move(dir_applications.joinpath(projectName + '.desktop'),
               dir_appDir.as_posix())
   os.rmdir(dir_applications)
   # Move [projectName].svg to the correct place for AppImage
   dir_icons = dir_appDir.joinpath('usr').joinpath('share').joinpath('icons')
   shutil.move(dir_icons.joinpath('hicolor').joinpath('scalable').joinpath('apps').joinpath(projectName + '.svg'),
               dir_appDir.as_posix())
   shutil.rmtree(dir_icons)

   # Create the AppRun shell script...
   file_appRun = dir_appDir.joinpath('AppRun')
   with open(file_appRun, 'w') as appRunFile:
      appRunFile.write('#!/bin/bash')
      appRunFile.write('exec $APPDIR/usr/bin/' + projectName)
   # ...and make it executable
   stat_appRun = os.stat(file_appRun)
   os.chmod(file_appRun, stat_appRun.st_mode | stat.S_IEXEC)

   #
   # Now we need to copy the shared libraries on which we depend.  There are various ways to obtain the list of these
   # dependencies.  Using ldd is simplest, because it handles recursion.
   #
   executablePath = dir_appDir.joinpath('usr').joinpath('bin').joinpath(projectName).as_posix()
   lddOutput = btExecute.abortOnRunFail(
      subprocess.run(
         ['ldd', executablePath],
         capture_output=True
      )
   ).stdout.decode('UTF-8')
   log.debug('Output of `ldd ' + executablePath + '`: ' + lddOutput)

   #
   # Most of the output of ldd will be of the form:
   #
   #    libQt6Core.so.6 => /lib/x86_64-linux-gnu/libQt6Core.so.6 (0x000074b991200000)
   #
   # We can ignore lines not of this format, and also assume they will have fewer fields, eg:
   #    linux-vdso.so.1 (0x000074b99346f000)
   #
   #
   dir_appDir_lib = dir_appDir.joinpath('usr').joinpath('lib')
   for lddOutputLine in lddOutput.splitlines():
      fields = lddOutputLine.strip().split()
      if len(fields) == 4:
         libPath = fields[2]
         log.debug('Copying ' + libPath + ' to ' + dir_appDir_lib.as_posix())
         shutil.copy2(libPath, dir_appDir_lib)
      else:
         log.debug('Skipping "' + lddOutputLine + '"')

   #
   # Now we run the AppImage tool to turn the directory tree into an app image.  Note that we first have to set the ARCH
   # environment variable.
   #
   os.environ['ARCH'] = 'x86_64'
   btExecute.abortOnRunFail(
      subprocess.run(
         [file_appImageTool, '--verbose', dir_appDir.as_posix(), appDirName + '.AppImage'],
         capture_output=False
      )
   )

   return
