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
import re
import shlex
import shutil
import stat
import subprocess
import sys

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
#  - Snap
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
      sys.exit(1)

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
   #    ├── AppRun   <── Shell script that acts as the entry point for the AppImage  ❇
   #    ├── .DirIcon <── PNG icon in one of the standard image sizes, eg, 128x128 or 256x256 pixels  ❇
   #    ├── [projectName].png <── Not explicitly required, but typically .DirIcon is a SYMLINK to this  ❇
   #    ├── [projectName].svg <── Required but allowed to be SYMLINK to, eg,
   #    │                         usr/share/icons/hicolor/scalable/apps/[projectName].svg  ❇
   #    ├── [reverseDomain].[projectName].desktop = SYMLINK to file in usr/share/applications  ❇
   #    └── usr
   #        ├── bin
   #        │   └── [projectName] ✅   <── the executable
   #        ├── lib   <── dir_appDir_lib
   #        │   └── [All the libraries needed by the application] ❇
   #        ├── plugins   <── dir_appDir_plugins
   #        │   └── [All the plugins needed by the application] ❇
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
   # Make the [projectName].svg symlink
   dir_icons = dir_appDir.joinpath('usr').joinpath('share').joinpath('icons')
   dir_svgIcon = dir_icons.joinpath('hicolor').joinpath('scalable').joinpath('apps')
   os.symlink(dir_svgIcon.relative_to(dir_appDir).joinpath(projectName + '.svg'),
              dir_appDir.joinpath(projectName + '.svg'))

   # Generate .png icon file from the .svg one (via its symlink that we just created above)
   btExecute.abortOnRunFail(
      subprocess.run(
         [
            'rsvg-convert', '-w', '256', '-h', '256',
            '-o', dir_appDir.joinpath(projectName + '.png').as_posix(),
            dir_appDir.joinpath(projectName + '.svg').as_posix()
         ],
         capture_output=False
      )
   )
   # Now make the .DirIcon symlink to the PNG file we just generated
   os.symlink(dir_appDir.joinpath(projectName + '.png'),
              dir_appDir.joinpath('.DirIcon'))

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
      #
      # Because (see below) we set the RUNPATH ELF attribute in the executable, you might think we don't need to set
      # LD_LIBRARY_PATH.  However, we need to ensure that shared libraries look for their own dependencies inside the
      # AppImage first.  Per the discussion athttps://github.com/AppImage/AppImageKit/issues/126, we also allow anything
      # in APPIMAGE_LIBRARY_PATH to take precedence.  (I'm not sure how widely used this is, but it does no harm to
      # support it.)
      #
      appRunFile.write('#!/bin/bash\n')
      appRunFile.write('export LD_LIBRARY_PATH="${APPIMAGE_LIBRARY_PATH}:${APPDIR}/usr/lib:${LD_LIBRARY_PATH}"\n')
      appRunFile.write('exec $APPDIR/usr/bin/' + projectName + '\n')
   # ...and make it executable.  (Note that it "must be executable for all users (owner, group and others)", otherwise
   # the AppImage tool will complain.
   stat_appRun = os.stat(file_appRun)
   os.chmod(file_appRun, stat_appRun.st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

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
   # enables the generation of so-called “new tags”.  Once enabled, requests to populate RPATH will in fact populate
   # RUNPATH.)
   #
   # You can see values (if specified) for RPATH and RUNPATH for an existing binary by running `readelf -d [binary]` or
   # `patchelf --print-rpath [binary]`.
   #
   # Inside an RPATH/RUNPATH, "$ORIGIN" means the directory containing the executable (or shared library if that's what
   # you're building).
   #
   dir_bin = dir_appDir.joinpath('usr').joinpath('bin')
   executableFullPath = dir_bin.joinpath(projectName).as_posix()
   btExecute.abortOnRunFail(
      subprocess.run(
         #
         # Note that, unlike running from the command line, we don't need to add extra quotes here to prevent $ORIGIN
         # being interpreted as a shell variable.
         #
         ["patchelf", "--add-rpath", "$ORIGIN/../lib", executableFullPath]
      )
   )

   #
   # Now we need to copy the shared libraries on which we depend.  There are various ways to obtain the list of these
   # dependencies.  Using ldd is simplest, because it handles recursion.
   #
   # We start here by gathering the direct dependencies of the executable.  When we deal with plug-ins, we'll then add
   # their dependencies.
   #
   lddOutputExecutable = btExecute.abortOnRunFail(
      subprocess.run(
         ['ldd', executableFullPath],
         capture_output=True
      )
   ).stdout.decode('UTF-8')
   btLogger.log.debug('Output of `ldd ' + executableFullPath + '`: ' + lddOutputExecutable)

   allLddOutputs = [lddOutputExecutable]

   #
   # There are certain libraries we depend on that we do _not_ want to include, because the user's local system will
   # have a more appropriate version (and shipping the version from the build machine will likely cause breakage).
   #
   librariesNotToInclude = [
      'libc.so.6',      # Core glibc
      'libm.so.6',      # Core glibc
      'libresolv.so.2', # Core glibc
      'libgcc_s.so.1',  # gcc runtime, tightly coupled to the system
   ]

   #
   # As with shared libraries (see below), we need to include Qt plug-ins we depend on.  We do this before the shared
   # libraries because we'll also need to bundle the shared libraries that the plug-ins themselves depend on.
   #
   # First, we ask Qt (via qmake) where its plug-ins live.  NB we need the Qt6 ones.  (Build machine may have both Qt5
   # and Qt6 installed, so we have to be explicit.)
   #
   qtPlugInsDirRaw = btExecute.abortOnRunFail(
      subprocess.run(
         ['qmake6', '-query', 'QT_INSTALL_PLUGINS'],
         capture_output=True
      )
   ).stdout.decode('UTF-8').rstrip()
   btLogger.log.debug('Output of `qmake6 -query QT_INSTALL_PLUGINS`: ' + qtPlugInsDirRaw)
   qtPlugInsDir = pathlib.Path(qtPlugInsDirRaw)

   #
   # Plugins (aka plug-ins) are just shared libraries where all the loading and function calling is handled dynamically
   # at runtime.
   #
   # With a regular shared library, the linker records all the dependencies at compile-time.  Then, the dynamic linker
   # (ld.so) loads the library automatically when the program runs, and our code can call functions in it directly by
   # name.
   #
   # With a plugin, there is no build-time linking.  Typically, the application doesn't even know which specific plugins
   # exist until it looks in a directory.  A plugin library is loaded explicitly at runtime using dlopen() (eg via Qt's
   # QPluginLoader) and there is an API for finding symbols etc.
   #
   # All this means that it's a bit harder to determine at build time which Qt plug-ins are used by an executable.  At
   # runtime, you can run strace on the executable to get a list of plug-ins being loaded, eg:
   #
   #    strace -e trace=openat ./our_binary 2>&1 | grep -i plugin
   #
   # As of Feb 2026, on Qt 6.4.2, we have the following 68 Qt plug-ins in 20 directories (courtesy of running
   # `tree /usr/lib/x86_64-linux-gnu/qt6/plugins` on Ubuntu 24.04), of which those marked ☀️ are the ones we think we
   # need to ship:
   #    plugins
   #    ├── designer
   #    │   └── libqquickwidget.so
   #    ├── egldeviceintegrations
   #    │   ├── libqeglfs-emu-integration.so
   #    │   ├── libqeglfs-kms-egldevice-integration.so
   #    │   ├── libqeglfs-kms-integration.so
   #    │   └── libqeglfs-x11-integration.so
   #    ├── generic
   #    │   ├── libqevdevkeyboardplugin.so
   #    │   ├── libqevdevmouseplugin.so
   #    │   ├── libqevdevtabletplugin.so
   #    │   ├── libqevdevtouchplugin.so
   #    │   ├── libqlibinputplugin.so
   #    │   ├── libqtslibplugin.so
   #    │   └── libqtuiotouchplugin.so
   #    ├── iconengines
   #    │   └── libqsvgicon.so  ☀️
   #    ├── imageformats
   #    │   ├── libqgif.so    ☀️
   #    │   ├── libqicns.so   ☀️
   #    │   ├── libqico.so    ☀️
   #    │   ├── libqjpeg.so   ☀️
   #    │   ├── libqmng.so    ☀️
   #    │   ├── libqsvg.so    ☀️
   #    │   ├── libqtga.so    ☀️
   #    │   ├── libqtiff.so   ☀️
   #    │   ├── libqwbmp.so   ☀️
   #    │   └── libqwebp.so   ☀️
   #    ├── multimedia
   #    │   ├── libffmpegmediaplugin.so
   #    │   └── libgstreamermediaplugin.so
   #    ├── networkinformation
   #    │   ├── libqglib.so
   #    │   └── libqnetworkmanager.so
   #    ├── platforminputcontexts
   #    │   ├── libcomposeplatforminputcontextplugin.so     ☀️
   #    │   ├── libfcitx5platforminputcontextplugin.so      ☀️
   #    │   ├── libfcitxplatforminputcontextplugin-qt6.so   ☀️
   #    │   └── libibusplatforminputcontextplugin.so        ☀️
   #    ├── platforms
   #    │   ├── libqeglfs.so            ☀️
   #    │   ├── libqlinuxfb.so          ☀️
   #    │   ├── libqminimalegl.so       ☀️
   #    │   ├── libqminimal.so          ☀️
   #    │   ├── libqoffscreen.so        ☀️
   #    │   ├── libqvkkhrdisplay.so     ☀️
   #    │   ├── libqvnc.so              ☀️
   #    │   ├── libqwayland-egl.so      ☀️
   #    │   ├── libqwayland-generic.so  ☀️
   #    │   └── libqxcb.so              ☀️
   #    ├── platformthemes
   #    │   └── libqgtk3.so   ☀️
   #    ├── printsupport
   #    │   └── libcupsprintersupport.so   ☀️
   #    ├── PyQt6
   #    │   └── libpyqt6qmlplugin.so
   #    ├── sqldrivers
   #    │   ├── libqsqlite.so    ☀️
   #    │   └── libqsqlpsql.so   ☀️
   #    ├── tls
   #    │   ├── libqcertonlybackend.so
   #    │   └── libqopensslbackend.so
   #    ├── wayland-decoration-client
   #    │   └── libbradient.so
   #    ├── wayland-graphics-integration-client
   #    │   ├── libdmabuf-server.so
   #    │   ├── libdrm-egl-server.so
   #    │   ├── libqt-plugin-wayland-egl.so
   #    │   ├── libshm-emulation-server.so
   #    │   └── libvulkan-server.so
   #    ├── wayland-graphics-integration-server
   #    │   ├── libqt-wayland-compositor-dmabuf-server-buffer.so
   #    │   ├── libqt-wayland-compositor-drm-egl-server-buffer.so
   #    │   ├── libqt-wayland-compositor-linux-dmabuf-unstable-v1.so
   #    │   ├── libqt-wayland-compositor-shm-emulation-server.so
   #    │   ├── libqt-wayland-compositor-vulkan-server.so
   #    │   ├── libqt-wayland-compositor-wayland-egl.so
   #    │   └── libqt-wayland-compositor-wayland-eglstream-controller.so
   #    ├── wayland-shell-integration
   #    │   ├── libfullscreen-shell-v1.so
   #    │   ├── libivi-shell.so
   #    │   ├── libqt-shell.so
   #    │   ├── libwl-shell-plugin.so
   #    │   └── libxdg-shell.so
   #    └── xcbglintegrations
   #        ├── libqxcb-egl-integration.so
   #        └── libqxcb-glx-integration.so
   #
   #    20 directories, 68 files
   #
   # TLDR, I think we just need to grab some of the plugin directories
   #
   qtPlugInDirsToCopy = [
      'iconengines',
      'imageformats',
      'platforminputcontexts',
      'platforms',
      'platformthemes',
      'printsupport',
      'sqldrivers'
   ]
   dir_appDir_plugins = dir_appDir.joinpath('usr').joinpath('plugins')
   os.makedirs(dir_appDir_plugins)
   for plugInSubDir in qtPlugInDirsToCopy:
      copyFrom = qtPlugInsDir.joinpath(plugInSubDir)
      copyTo = dir_appDir_plugins.joinpath(plugInSubDir)
      btLogger.log.debug('Copying ' + copyFrom.as_posix() + ' to ' + copyTo.as_posix())
      shutil.copytree(copyFrom, copyTo)
      #
      # Now we need to get the dependencies for each plug-in in the directory we just copied
      #
      plugins = os.listdir(copyFrom)
      for plugin in plugins:
         pluginFullPath = copyFrom.joinpath(plugin).as_posix()
         lddOutputForPlugIn = btExecute.abortOnRunFail(
            subprocess.run(
               ['ldd', pluginFullPath],
               capture_output=True
            )
         ).stdout.decode('UTF-8')
         btLogger.log.debug('Initial output of `ldd ' + pluginFullPath + '`: ' + lddOutputForPlugIn)

         allLddOutputs.append(lddOutputForPlugIn)

   #
   # We need Qt to know where to look for the plug-ins when the AppImage is run.  The cleanest way to do this is via a
   # qt.conf file in the same directory as the executable (see https://doc.qt.io/qt-6/qt-conf.html).
   #
   file_qtConf = dir_bin.joinpath('qt.conf')
   with open(file_qtConf, 'w') as qtConfFile:
      #
      # The qt.conf file is an INI text file
      #
      # All paths are relative to the Prefix. On Windows and X11, the Prefix is relative to the directory containing the
      # application executable (QCoreApplication::applicationDirPath()). On macOS, the Prefix is relative to the
      # Contents in the application bundle.
      #
      qtConfFile.write('[Paths]\n')
      qtConfFile.write('Prefix = ..\n')
      qtConfFile.write('Plugins = plugins\n')  # Not strictly necessary as this is the default value

   #
   # Now we are ready to parse the ldd output and bundle all the dependencies in it.
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
   lddOutput = '\n'.join(allLddOutputs)
   for lddOutputLine in lddOutput.splitlines():
      fields = lddOutputLine.strip().split()
      if len(fields) == 4:
         libName = fields[0]
         if libName in librariesNotToInclude:
            btLogger.log.debug('Not bundling ' + libName)
         else:
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
                  ["patchelf", "--add-rpath", "$ORIGIN/../lib", newLibPath.as_posix()]
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
      sys.exit(1)

   btLogger.log.info('XDG_DATA_HOME: ' + os.environ.get('XDG_DATA_HOME', ''))

   exe_flatpak = shutil.which('flatpak')
   if exe_flatpak is None or exe_flatpak == '':
      btLogger.log.info('Installing flatpak')
      btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'update']))
      btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'flatpak']))
      btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'flatpak-builder']))
      btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'appstream']))
      btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', 'appstream-compose']))
      exe_flatpak = shutil.which('flatpak')

   if exe_flatpak is None or exe_flatpak == '':
      btLogger.log.error('Cannot find flatpak.  PATH=' + os.environ['PATH'])
      sys.exit(1)

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
   # It is also useful to understand that Flatpaks run, and are built, inside a "sandboxed" environment (distinct from
   # but akin to chroot, Docker, LXD).
   #

   btLogger.log.info('Add Flathub Repo')
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
   # There isn't a "Qt" runtime, but the KDE one gives us what we need (because KDE is based on Qt).
   #
   # NOTE that, for KDE SDKs and Runtimes, there isn't a version marked "latest", so you have to explicitly specify the
   # one you want.  (If you don't specify which version you want, you'll get an interactive prompt listing all available
   # versions and inviting ou to select one.)
   #
   # So, the first thing we do is, via flatpak, ask for all the runtimes (of which there are over 2000), and then filter
   # and sort to find the latest.
   #
   btLogger.log.info('Getting list of runtimes')
   runtimesList = btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--user', 'remote-ls', 'flathub', '--runtime', '--columns=ref'],
         capture_output=True
      )
   ).stdout.decode('UTF-8')

   versions = []
   for line in runtimesList.splitlines():
      # Match lines like: runtime/org.kde.Sdk/x86_64/6.8
      match = re.search(r'^runtime/org\.kde\.Sdk/[^/]+/(\d+\.\d+)$', line.strip())
      if match:
         versions.append(match.group(1))

   if not versions:
      btLogger.log.critical('No org.kde.Sdk versions found on Flathub')
      sys.exit(1)

   # Sort by version number (not lexicographically)
   latest = sorted(versions, key=lambda v: tuple(int(x) for x in v.split('.')))[-1]
   btLogger.log.info('Latest available KDE SDK version: ' + latest)

   #runtimeVersion = '6.10'  # As of 2025-11-29, this is the "current" version
   runtimeVersion = latest
   #
   # TODO: For the moment, the runtime version is hardcoded in the manifest (see runtime-version in
   #       packaging/linux/flatpackManifest.in.yml), but we could fix that...
   #

   # TBD: in theory we don't need to install the platform -- it's needed to run the code but not to build it
   btLogger.log.info('Installing Flatpak Platform')
   btExecute.abortOnRunFail(
      subprocess.run(['flatpak', '--user', 'install', '--assumeyes', 'org.kde.Platform//' + runtimeVersion])
   )
   btLogger.log.info('Installing Flatpak SDK')
   btExecute.abortOnRunFail(
      subprocess.run(['flatpak', '--user', 'install', '--assumeyes', 'org.kde.Sdk//' + runtimeVersion])
   )

   #
   # Although we could install flatpak builder using `sudo apt install flatpak-builder` (above where we install
   # flatpak itself), this does not install Flatpak Linter (flatpak-builder-lint).  So we also we install flatpak
   # builder via flatpak, as that does give us the linter.  Note, however, that we need to have installed the Platform and Sdk first.
   #
   # Note that Flatpak Linter (flatpak-builder-lint) is included in Flatpak Builder
   #
   btLogger.log.info('Installing Flatpak Builder from Flatpak (to get Linter)')
   btExecute.abortOnRunFail(
#      subprocess.run(['flatpak', '--user', 'install', 'flathub', '--assumeyes', 'org.flatpak.Builder'])
      subprocess.run(['flatpak', '--user', 'install', '--assumeyes', 'org.flatpak.Builder'])
   )

   builderInfo = btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--user', 'info', 'org.flatpak.Builder'],
         capture_output=True
      )
   ).stdout.decode('UTF-8')
   btLogger.log.info('Flatpak Builder Info:\n' + builderInfo)

   installedFlatpakRuntimes = btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--user', 'list', '--runtime', '--columns=ref'],
         capture_output=True
      )
   ).stdout.decode('UTF-8')
   btLogger.log.info('Installed Flatpak Runtimes:\n' + installedFlatpakRuntimes)

   installedFlatpakApps = btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--user', 'list', '--app', '--columns=ref'],
         capture_output=True
      )
   ).stdout.decode('UTF-8')
   btLogger.log.info('Installed Flatpak Apps:\n' + installedFlatpakApps)

   #
   # Since we have to rebuild everything, we need the source code.  We want this in a subdirectory of the one holding
   # the manifest, because flatpak-builder is going to copy it, and we don't want to be trying to copy a directory tree
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
      shutil.copytree(
         btFileSystem.dir_base.joinpath(directoryToCopy),
         dir_copyOfSource.joinpath(directoryToCopy),
         # This is to skip copying the Blaze build directory
         ignore = lambda dir, names: ['build'] if pathlib.Path(dir).name == 'blaze' and 'build' in names else []
      )
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
          '--user',
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
   # `flatkpak build` commands, so we could run those commands individually.  However, this would be going against the
   # grain of how flatpaks are supposed to be submitted to flathub, etc.  So we should bite the bullet and get the
   # manifest working.  Nonetheless, it helps to understand what flatpak-builder is doing under the hood.  Specifically,
   # the manifest file defines how flatpak-builder should:
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
         #
         # See https://docs.flatpak.org/en/latest/flatpak-builder-command-reference.html for flatpak-builder command
         # reference.
         #
         ['flatpak-builder',
                    '--user',
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
   # Inside the build tree, the `files` directory contains what is seen as `/app` inside the flatpak's sandbox
   # environment.
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
#
# NOTE: If you have a VPN client installed, eg Mullvad VPN, there is a known conflict which you need to work around.
# LXD’s FAQ says that if starting containers suddenly fails with a cgroup-related error message (Failed to mount
# "/sys/fs/cgroup"), this might be due to running a VPN client on the host. The problem is that the VPN client mounts
# the net_cls cgroup1 over cgroup2 (which LXD uses).  If you are hitting problems, and running `mount | grep net_cls`
# gives output `/sys/fs/cgroup/net_cls … cgroup … net_cls`, then this is likely the cause.  You can run the following
# commands to move Mullvad’s net_cls mount out of /sys/fs/cgroup:
#
#    $ mount | grep net_cls
#    net_cls on /sys/fs/cgroup/net_cls type cgroup (rw,relatime,net_cls)
#
#    $ sudo systemctl stop mullvad-daemon
#    $ sudo umount /sys/fs/cgroup/net_cls
#
#    $ sudo mkdir -p /etc/systemd/system/mullvad-daemon.service.d
#    $ sudo tee /etc/systemd/system/mullvad-daemon.service.d/net-cls.conf <<EOF
#    [Service]
#    Environment="TALPID_NET_CLS_MOUNT_DIR=/var/lib/mullvad/net_cls/"
#    ExecStartPre=/usr/bin/mkdir -p /var/lib/mullvad/net_cls
#    ExecStartPre=/bin/sh -c 'mountpoint -q /var/lib/mullvad/net_cls || mount -t cgroup -o net_cls net_cls /var/lib/mullvad/net_cls'
#    EOF
#
#    $ sudo systemctl daemon-reload
#    $ sudo systemctl start mullvad-daemon
#    $ mount | grep net_cls
#    net_cls on /var/lib/mullvad/net_cls type cgroup (rw,relatime,net_cls)
#
#-----------------------------------------------------------------------------------------------------------------------
def doSnap():
   sysName = platform.system()
   if sysName != 'Linux':
      btLogger.log.critical('Snap creation not supported on: ' + sysName)
      sys.exit(1)

   #
   # NB: The grp module is only available on Unix-like systems, so we can't import it at the top of this file (which is
   #     imported by buildTool.py on all platforms).
   #
   import grp

   #
   # Read in the variables exported from the Meson build
   #
   btUtils.readBuildConfigFile()
   projectName = btUtils.buildConfig["CONFIG_APPLICATION_NAME_LC"]
   versionString = btUtils.buildConfig["CONFIG_VERSION_STRING"]
   appDirName = projectName + '-' + versionString

   #
   # We use the snapcraft.yaml that Meson generates (via configure_file()) from packaging/linux/snapcraft.in.yaml.  But
   # Meson only regenerates it when it reconfigures -- eg when you next build in mbuild.  So, if you've just edited the
   # template, it's easy to end up running a snap build with the previous version.  Check for this before we do
   # anything else, as a snap build takes a while to fail.
   #
   file_snapProjectTemplate  = btFileSystem.dir_base.joinpath('packaging').joinpath('linux').joinpath('snapcraft.in.yaml')
   file_snapProjectGenerated = btFileSystem.dir_build.joinpath('snapcraft.yaml')
   if not file_snapProjectGenerated.is_file():
      btLogger.log.critical(
         'Cannot find ' + file_snapProjectGenerated.as_posix() + '.  Run `meson setup --reconfigure ' +
         btFileSystem.dir_build.as_posix() + '` (or a build in that directory) and try again.'
      )
      sys.exit(1)
   if file_snapProjectGenerated.stat().st_mtime < file_snapProjectTemplate.stat().st_mtime:
      btLogger.log.critical(
         file_snapProjectGenerated.as_posix() + ' is older than ' + file_snapProjectTemplate.as_posix() +
         '.  Run `meson setup --reconfigure ' + btFileSystem.dir_build.as_posix() + '` (or a build in that ' +
         'directory) to regenerate it, then try again.'
      )
      sys.exit(1)

   #
   # NOTE: AFAICT in order to build snaps, you need, amongst other things, to enable them to be installed on your
   # system -- which not everyone necessarily wants.  We therefore only install the snap tools here rather than in
   # btDependencies.installDependencies.  (If any of these are already installed, `snap install` just says so and
   # returns success.)
   #
   btLogger.log.info('Installing Snapd')
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'update']))
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', '-y', 'snapd']))
   btLogger.log.info('Installing Snapcraft')
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'snap', 'install', '--classic', 'snapcraft']))
   btLogger.log.info('Installing LXD')
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'snap', 'install', 'lxd']))

   #
   # The snap build process requires a "build provider".  According to
   # https://documentation.ubuntu.com/snapcraft/8.9.4/how-to/setup/set-up-snapcraft/, this is "to host an isolated build
   # environment, like a sandbox [inside which] software can be built and packaged as snaps without making potentially
   # destructive changes to the host system".
   #
   # For core22 and later bases, the choice is between LXD (the default on Linux) and Multipass.  We use LXD (see
   # https://canonical.com/lxd), Canonical's System Container and Virtual Machine Manager built on top of LXC (Linux
   # Containers).  We used to try Multipass, but, for core22 and later bases, the Multipass VM is hard-coded to 2 CPUs
   # and 2GB RAM, and the SNAPCRAFT_BUILD_ENVIRONMENT_CPU and SNAPCRAFT_BUILD_ENVIRONMENT_MEMORY environment variables
   # are ignored (see https://bugs.launchpad.net/snapcraft/+bug/2008003).  Several of our source files need more than
   # 2GB RAM to compile (eg src/trees/NamedEntityTreeView.cpp peaks at about 2.2GB with GCC 13), so the build would
   # swap itself to a near standstill and appear to hang.  An LXD container, by contrast, shares the host's RAM and
   # CPUs.
   #
   # LXD needs to be initialised once before first use.  `lxd init --auto` gives a sensible default set-up (a storage
   # pool and a bridged network).  But we don't want to run it if LXD is already set up, so we first check whether
   # there are any storage pools.
   #
   btExecute.abortOnRunFail(subprocess.run(['sudo', 'lxd', 'waitready']))
   storageListResult = btExecute.abortOnRunFail(
      subprocess.run(['sudo', 'lxc', 'storage', 'list', '--format', 'csv'], capture_output = True, encoding = 'utf-8')
   )
   if storageListResult.stdout.strip() == '':
      btLogger.log.info('Initialising LXD')
      btExecute.abortOnRunFail(subprocess.run(['sudo', 'lxd', 'init', '--auto']))
   else:
      btLogger.log.debug('LXD already initialised')

   #
   # For snapcraft to be able to use LXD, the current user needs to be in the 'lxd' group (which the installation of
   # LXD will have created).  Adding the current user to a group will not take immediate effect.  Normally you need to
   # log out and log in again.  (In an interactive shell, the `newgrp` command gets around this but that wouldn't help
   # us here: the script would pause, dump the user out into the new shell, with the group membership, and then only
   # resume, without the group membership, when the user exited that shell.)  We get around this by using the `sg`
   # command to invoke snapcraft when the current process doesn't have the group membership.
   #
   currentUser = getpass.getuser()
   lxdGroup = grp.getgrnam('lxd')
   if currentUser not in lxdGroup.gr_mem:
      btLogger.log.info('Adding user ' + currentUser + ' to lxd group')
      btExecute.abortOnRunFail(subprocess.run(['sudo', 'usermod', '--append', '--groups', 'lxd', currentUser]))
   needSg = lxdGroup.gr_gid not in os.getgroups()
   if needSg:
      btLogger.log.info('Current process does not have lxd group membership, so will invoke snapcraft via sg')

   #
   # Docker and LXD do not play nicely together on the same machine: Docker sets the default policy of the iptables
   # FORWARD chain to DROP, which stops LXD containers getting network access (so snapcraft can't download packages
   # inside the build container).  See "Prevent connectivity issues with LXD and Docker" in the LXD documentation for
   # fixes.  We don't try to fix this automatically, but we do warn about it.
   #
   if shutil.which('docker') is not None:
      btLogger.log.warning(
         'Docker is installed.  If the snap build fails because the LXD container has no network access, see ' +
         '"Prevent connectivity issues with LXD and Docker" in the LXD documentation.'
      )

   #
   # Belt-and-braces: per
   # https://documentation.ubuntu.com/snapcraft/stable/how-to/select-a-build-provider/index.html, the command-line
   # flag takes priority, but we also set the environment variable in case anything we run doesn't take the flag.
   #
   os.environ['SNAPCRAFT_BUILD_ENVIRONMENT'] = 'lxd'

   #
   # Helper function to run snapcraft with the right provider and, if necessary, the right group membership.
   #
   def runSnapcraft(step, extraArgs = []):
      snapcraftCommand = ['snapcraft', step, '--use-lxd', *extraArgs]
      if needSg:
         snapcraftCommand = ['sg', 'lxd', '-c', shlex.join(snapcraftCommand)]
      btLogger.log.info('🫰 Running ' + ' '.join(snapcraftCommand) + ' in ' + pathlib.Path.cwd().as_posix())
      btExecute.abortOnRunFail(subprocess.run(snapcraftCommand))
      return

   #
   # Create the relevant top-level directory and ensure it starts out empty
   # (NB: Any missing parent directories will automatically get created by os.makedirs.  In particular,
   # btFileSystem.dir_contAppPkgs and btFileSystem.dir_snap are guaranteed to exist after this.)
   #
   # Snapcraft names its LXD build instances using, amongst other things, the inode number of the project directory.
   # So, if we just deleted the old project directory and created a new one, each run would leave behind an LXD
   # instance (of several GB) from the previous run.  We therefore run `snapcraft clean` in any old project directory
   # (ie one with a snap/snapcraft.yaml file) before deleting it.  This also guarantees we start each build from
   # scratch, without bits hanging around from earlier builds.
   #
   if btFileSystem.dir_snap.is_dir():
      for oldProjectFile in btFileSystem.dir_snap.glob('*/snap/snapcraft.yaml'):
         oldProjectDir = oldProjectFile.parent.parent
         os.chdir(oldProjectDir)
         btLogger.log.info('Cleaning up LXD instance(s) for ' + oldProjectDir.as_posix())
         # It's not a disaster if this fails, so we don't abort on failure
         runResult = subprocess.run(['snapcraft', 'clean', '--use-lxd'] if not needSg else
                                    ['sg', 'lxd', '-c', 'snapcraft clean --use-lxd'])
         if runResult.returncode != 0:
            btLogger.log.warning('Ignoring failure of snapcraft clean in ' + oldProjectDir.as_posix())
      os.chdir(btFileSystem.dir_base)
      btLogger.log.info('Removing existing ' + btFileSystem.dir_snap.as_posix() + ' directory tree')
      shutil.rmtree(btFileSystem.dir_snap)
   dir_snapBuild = btFileSystem.dir_snap.joinpath(appDirName)
   dir_snapBuildSnap = dir_snapBuild.joinpath('snap')
   btLogger.log.info('Creating directory ' + dir_snapBuildSnap.as_posix())
   os.makedirs(dir_snapBuildSnap)
   os.chdir(dir_snapBuild)
   btLogger.log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())

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
   # As with Flatpak, Snap expects us to build everything from source, so we need to copy the source code etc into the
   # directory tree its build provider has access to.  It helps to understand what the Snapcraft tool does when we run
   # it.  Per info at https://documentation.ubuntu.com/snapcraft/8.9.3/reference/processes/snap-build-process/,
   # snapcraft does the following:
   #    - Starting in the current working directory, it looks for snapcraft.yaml or snap/snapcraft.yaml, and then parses
   #      its contents to begin the build.
   #
   #    - It creates (or reuses) an LXD container in which to do the build.  This will be a "minimal Ubuntu install"
   #      determined by the 'base' in snapcraft.yaml.  The project directory (ie dir_snapBuild) is mounted inside the
   #      container as /root/project.
   #
   #    - Required packages and snaps listed in the snapcraft.yaml file (including the SDK snaps that the kde-neon-6
   #      extension adds) are downloaded and installed into the container.
   #
   #    - The "Pull" build step is run (on each part).
   #      Each part has its own directory under /root/parts in the container.  We only have one part of our own, called
   #      main-part.  Because we put 'source: .' in the 'main-part' section of snap/snapcraft.yaml file, the contents of
   #      the project directory end up in /root/parts/main-part/src (aka ${CRAFT_PART_SRC}).
   #
   #    - The "Build" build step is run (on each part).  For main-part, this is the override-build script in
   #      snap/snapcraft.yaml, which builds libbacktrace, Blaze and then our own code, and installs the latter into
   #      /root/parts/main-part/install (aka ${CRAFT_PART_INSTALL}).
   #
   #    - The "Stage" build step is run (on each part)
   #
   #    - The "Prime" build step is run (on each part)
   #
   #    - The "Pack" build step creates the .snap file in the project directory.  This also runs the snapcraft linters.
   #
   # Note that because each build step ensures the prior ones have been run, we only _need_ to call 'snapcraft pack'.
   # However, it is possible to run steps individually, which is a bit slower but helpful for debugging.  What's even
   # more helpful is that you can drop out into a shell in the container by adding '--shell', '--shell-after' or
   # '--debug' to the 'snapcraft' command (see
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
   # NB flags such as '--shell' have to come after the build step -- eg `snapcraft --debug build` is wrong; needs to be
   # `snapcraft build --debug`.  (And remember to add `--use-lxd` and, if you're not yet in the lxd group in your current
   # login session, run it via `sg lxd -c "..."`.)
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
   # Provided the contents of snapcraft.yaml are correct, snapcraft does everything else for us.
   #
   for step in ['pull', 'build', 'stage', 'prime', 'pack']:
      runSnapcraft(step, ['--verbose'])

   #
   # `snapcraft pack` puts the .snap file(s) in the project directory, named [name]_[version]_[arch].snap.  We move them
   # up a level, to sit alongside the project directory (as we do with the Flatpak bundle), and generate checksums.
   #
   snapFiles = list(dir_snapBuild.glob('*.snap'))
   if len(snapFiles) == 0:
      btLogger.log.critical('Could not find .snap file in ' + dir_snapBuild.as_posix())
      sys.exit(1)
   for snapFile in snapFiles:
      shutil.move(snapFile, btFileSystem.dir_snap.joinpath(snapFile.name))
      btLogger.log.info('Snap package ' + btFileSystem.dir_snap.joinpath(snapFile.name).as_posix())
      btUtils.writeSha256sum(btFileSystem.dir_snap, snapFile.name)

   btLogger.log.info(
      'To install locally for testing: sudo snap install --devmode --dangerous ' +
      btFileSystem.dir_snap.as_posix() + '/[snap file]'
   )

   btLogger.log.info(
      'Afterwards, to remove: snap remove --purge ' + projectName
   )

   return