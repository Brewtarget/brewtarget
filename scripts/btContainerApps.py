#-----------------------------------------------------------------------------------------------------------------------
# scripts/btContainerApps.py is part of Brewtarget, and is copyright the following authors 2025:
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
   # As for when we are doing "regular" packaging, we get Meson to put most of the files we need (bar shared libaries)
   # in a tree where we can get them.  NB: We need to be in the mbuild directory to run Meson.
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
   #    ├── [projectName].desktop     ❇
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

   # Move [projectName].desktop to the correct place for AppImage
   dir_applications = dir_appDir.joinpath('usr').joinpath('share').joinpath('applications')
   shutil.move(dir_applications.joinpath(appId + '.desktop'),
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
   btLogger.log.debug('Output of `ldd ' + executablePath + '`: ' + lddOutput)

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
         btLogger.log.debug('Copying ' + libPath + ' to ' + dir_appDir_lib.as_posix())
         shutil.copy2(libPath, dir_appDir_lib)
      else:
         btLogger.log.debug('Skipping "' + lddOutputLine + '"')

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
      subprocess.run(
         ['flatpak', '--verbose', 'remote-add', '--if-not-exists', 'flathub', 'https://flathub.org/repo/flathub.flatpakrepo'],
         capture_output=False
      )
   )
   # TBD: in theory we don't need to install the platform -- it's needed to run the code but not to build it
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--verbose', 'install', '--assumeyes', 'org.kde.Platform//' + runtimeVersion],
         capture_output=False
      )
   )
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--verbose', 'install', '--assumeyes', 'org.kde.Sdk//' + runtimeVersion],
         capture_output=False
      )
   )
   # I'm not totally sure that we need Java, but the Xerces build complains if it's not present
   btExecute.abortOnRunFail(
      subprocess.run(
         ['flatpak', '--verbose', 'install', '--assumeyes', 'org.freedesktop.Sdk.Extension.openjdk11//' + openJdkVersion],
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
         ['flatpak', '--verbose', 'install', '--assumeyes', 'org.flatpak.Builder'],
         capture_output=False
      )
   )

   #
   # Since we have to rebuild everything, we need the source code.  We want this in a subdirectory of the one holding
   # the manifest, because flatpak-builder is going to copy it and we don't want to be trying to copy a directory tree
   # inside itself.
   #
   btLogger.log.info('Copy source tree')
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
   # Manifest files can be in either JSON or YAML format.  We use the latter as it's slightly more concise and shares
   # similar "meaningful indentation" rules to Python.
   #
   with open(file_manifest, 'w') as manifestFile:
      manifestFile.write('\n'.join((
         'id: ' + appId,
         'runtime: org.kde.Platform',   # Using KDE here gives us Qt libraries
         'runtime-version: "' + runtimeVersion + '"',
         'sdk: org.kde.Sdk',            # SDK needs to correspond with runtime
         'sdk-extensions:',
         '  - org.freedesktop.Sdk.Extension.openjdk11',
         'command: ' + projectName,     # This is just the executable called to run the application -- in our case the
                                        # application itself
         'desktop-file-name-suffix: " (' + versionString + ')"',  # Optional, but useful, to show version number
         'modules:',
         '  - name: openjdk',
         '    buildsystem: simple',
         '    build-commands:',
         '      - /usr/lib/sdk/openjdk11/install.sh',
         '',
         '  - name: XercesC',
         '    sources:',
         '      - type: archive',
         # There are various locations to download Xerces-C, but this is the official Apache mirror
         '        url: https://dlcdn.apache.org//xerces/c/3/sources/xerces-c-3.3.0.tar.gz',
         '        sha256: 9555f1d06f82987fbb4658862705515740414fd34b4db6ad2ed76a2dc08d3bde',
         '        x-checker-data:',
         '          type: anitya',
         # Project ID comes from https://release-monitoring.org/project/5182/
         '          project-id: 5182',
         '          url-template: https://dlcdn.apache.org/xerces/c/3/sources/xerces-c-$version.tar.gz',
         # Per https://bugs.gentoo.org/931105, this patch is required to build XercesC
         '      - type: patch',
         '        path: xerces-c-3.2.5-cxx17.patch',
         '    buildsystem: cmake-ninja',
         '    builddir: true',
         # See https://xerces.apache.org/xerces-c/build-3.html for Xerces-C build options
         '    config-opts:',
         '      - -DCMAKE_BUILD_TYPE=Release',
         '      - -DBUILD_SHARED_LIBS=ON',
         '      - -DCMAKE_POSITION_INDEPENDENT_CODE=ON',
         '',
         '  - name: XalanC',
         '    sources:',
         '      - type: archive',
         # There are various locations to download Xerces-C, but this is the official Apache mirror
         '        url: https://dlcdn.apache.org/xalan/xalan-c/sources/xalan_c-1.12.tar.gz',
         '        sha512: a9f72f0e8e199ee2cfb4c19ecf390d5007f597aad96a53f55bc475805190302c7e0d800d776b7fb20fe8e2dddb6391e70aa3a8861a2303370135e8b0a5fd15fc',
         '        x-checker-data:',
         '          type: anitya',
         # Project ID comes from https://release-monitoring.org/project/5153/
         '          project-id: 5153',
         '          url-template: https://dlcdn.apache.org/xalan/xalan-c/sources/xalan_c-$version.tar.gz',
         # Per https://bugs.gentoo.org/955386, this patch is required to build XalanC with recent versions of CMake
         '      - type: patch',
         '        path: xalan-c-1.12-cmake-4.patch',
         # Per https://bugs.gentoo.org/936501, this patch is required to build XalanC with recent versions of GCC
         '      - type: patch',
         '        path: xalan-c-1.12-gcc-15.patch',
         # These patches are also listed in Gentoo XalanC build (See
         # https://gitweb.gentoo.org/repo/gentoo.git/tree/dev-libs/xalan-c/files and
         # https://gitweb.gentoo.org/repo/gentoo.git/tree/dev-libs/xalan-c/xalan-c-1.12-r4.ebuild.)
         '      - type: patch',
         '        path: xalan-c-1.12-fix-lto.patch',
         '      - type: patch',
         '        path: xalan-c-1.12-fix-threads.patch',
         '      - type: patch',
         '        path: xalan-c-1.12-icu-75.patch',
         '    buildsystem: cmake-ninja',
         '    builddir: true',
         # See https://xerces.apache.org/xerces-c/build-3.html for Xerces-C build options
         '    config-opts:',
         '      - -DCMAKE_BUILD_TYPE=Release',
         '      - -DBUILD_SHARED_LIBS=ON',
         '      - -DCMAKE_POSITION_INDEPENDENT_CODE=ON',
         '      - -DCMAKE_POLICY_VERSION_MINIMUM=3.5',
         '',
         '  - name: LibBacktrace',
         '    sources:',
         '      - type: git',
         '        url: https://github.com/ianlancetaylor/libbacktrace/',
         '        commit: b9e40069c0b47a722286b94eb5231f7f05c08713', # 2025-11-06
         '',
         '  - name: Boost',
         '    buildsystem: simple',
         # You can build Boost with CMake, but, for the moment at least, they still recommend using the B2 build system
         # (see https://www.boost.org/doc/user-guide/getting-started.html#from-source).  Note that we minimise build time
         # by only specifying the actual Boost libraries we want to build and install.
         '    build-commands:',
         '      - ./bootstrap.sh --prefix=${FLATPAK_DEST} --with-python=python3 --with-libraries=container,json,stacktrace',
         # Note that we need to specify library-path so that libboost_stacktrace_backtrace can link against the
         # LibBacktrace mentioned above.
         '      - ./b2 --with-container --with-json --with-stacktrace boost.stacktrace.backtrace=on -j$FLATPAK_BUILDER_N_JOBS library-path=/app/lib install',
         '    sources:',
         '      - type: archive',
         '        url: https://archives.boost.io/release/1.89.0/source/boost_1_89_0.tar.gz',
         '        sha256: 9de758db755e8330a01d995b0a24d09798048400ac25c03fc5ea9be364b13c93',
         '        x-checker-data:',
         '          type: anitya',
         # Project ID comes from https://release-monitoring.org/project/6845/
         '          project-id: 6845',
         '          stable-only: true',
         '          url-template: https://archives.boost.io/release/$version/source/boost_${major}_${minor}_${patch}.tar.gz',
         '    cleanup:',
         '      - /lib/cmake',
         '',
         '  - name: Valijson',
         '    sources:',
         '      - type: git',
         '        url: https://github.com/tristanpenman/valijson',
         '        commit: 4edda758546436462da479bb8c8514f8a95c35ad', # 2025-05-07 = Valijson v1.0.6
         '    buildsystem: simple',
         '    build-commands:',
         '      - cp -pr include/valijson /app/include/.',
         ''
         '  - name: pandoc',
         '    buildsystem: simple',
         '    build-commands:',
         '      - cp -R . /app/',
         '    sources:',
         '      - type: archive',
         '        url: https://github.com/jgm/pandoc/releases/download/3.8.3/pandoc-3.8.3-linux-amd64.tar.gz',
         '        sha256: c224fab89f827d3623380ecb7c1078c163c769c849a14ac27e8d3bfbb914c9b4',
         '        only-arches:',
         '          - x86_64',
         '        x-checker-data:',
         '          type: json',
         '          url: https://api.github.com/repos/jgm/pandoc/releases/latest',
         '          version-query: .tag_name',
         '          url-query: .assets[] | select(.name=="pandoc-" + $version + "-linux-amd64.tar.gz") | .browser_download_url',
         # TODO: Need to do similar elsewhere to support building on ARM -- eg for Raspberry Pi
         '      - type: archive',
         '        url: https://github.com/jgm/pandoc/releases/download/3.8.3/pandoc-3.8.3-linux-arm64.tar.gz',
         '        sha256: 166a5a37387eb10bd4c4f242a8109beef755ac1e8d4eb039c6b5ebd1d918d8d7',
         '        only-arches:',
         '          - aarch64',
         '        x-checker-data:',
         '          type: json',
         '          url: https://api.github.com/repos/jgm/pandoc/releases/latest',
         '          version-query: .tag_name',
         '          url-query: .assets[] | select(.name=="pandoc-" + $version + "-linux-arm64.tar.gz") | .browser_download_url',
         '',
         #
         # To get the packaging working, we want to be able to tweak things, including build-related stuff such as
         # CMakeLists.txt and meson.build.  Using the dir option (see
         # https://docs.flatpak.org/en/latest/module-sources.html#directory-source) allows us to build from local
         # sources.  However, note that "when submitting an application to software stores like Flathub, dir should be
         # avoided altogether".
         #
         '  - name: ' + projectName,
         '    sources:',
         '      - type: dir',
         '        path: copyOfSource',
         # Although we're using meson, we can't put 'buildsystem: meson' here, as we need to do a couple of extra steps
         '    buildsystem: simple',
         '    build-commands:',
         '      - mkdir mbuild',
         '      - meson setup --includedir /app/include --libdir /app/lib:/app/lib64 mbuild .',
         '      - meson compile -C mbuild --verbose',
         '      - meson install -C mbuild --destdir /app',
         # Telling Meson to install to /app gives us a tree of files under /app/usr/local that we now need to  relocate
         # (and in some cases rename) to the places flatpak expects them to be.  See
         # https://docs.flatpak.org/en/latest/conventions.html for details of what's expected.
         '      - mv /app/usr/local/share/applications /app/share/.',
         '      - mv /app/usr/local/share/icons /app/share/.',
         '      - mv /app/usr/local/bin/* /app/bin/.',
         '    builddir: true',
         '    build-options:',
         '      env:',
         '        CMAKE_INCLUDE_PATH: /app/include',
         '        BOOST_INCLUDEDIR: /app/include',
         '        BOOST_LIBRARYDIR: /app/lib',
         '        BOOST_ROOT: /app',
         '        LIBRARY_PATH: /app/lib:/app/lib64',
         '        LD_LIBRARY_PATH: /app/lib:/app/lib64',
         '        PKG_CONFIG_PATH: /app/lib/pkgconfig:/app/lib64/pkgconfig:/app/share/pkgconfig:/usr/lib/pkgconfig:/usr/share/pkgconfig',
         '        XercesC_ROOT:    /app',
         '        XalanC_ROOT:    /app',
         '',
         #
         # This section defines what permissions the containerised environment for our app will have.
         #
         # TBD: We start by copying some example ones, but these could doubtless be fine-tuned at some point.
         #
         'finish-args:',
         # X11 + XShm access
         '  - --share=ipc',
         '  - --socket=fallback-x11',
         # Wayland access
         '  - --socket=wayland',
         # GPU acceleration if needed
         '  - --device=dri',
         # Needs to talk to the network:
         '  - --share=network',
         # Needs to save files locally
         '  - --filesystem=xdg-documents',
         ''
      )))

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
   # TODO - Start this!

   return
