#-----------------------------------------------------------------------------------------------------------------------
# scripts/btDependencies.py is part of Brewken, and is copyright the following authors 2022-2025:
#   • Matt Young <mfsy@yahoo.com>
#
# Brewken is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# Brewken is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# This file is included from buildTool.py
#-----------------------------------------------------------------------------------------------------------------------

#-----------------------------------------------------------------------------------------------------------------------
# Python built-in modules we use
#-----------------------------------------------------------------------------------------------------------------------
import platform

#-----------------------------------------------------------------------------------------------------------------------
# Our own modules
#-----------------------------------------------------------------------------------------------------------------------
import btUtils

log = btUtils.getLogger()

#-----------------------------------------------------------------------------------------------------------------------
# Function to install dependencies -- called if the user runs 'bt setup all'
#-----------------------------------------------------------------------------------------------------------------------
def installDependencies():
   log.info('Checking which dependencies need to be installed')
   #
   # I looked at using ConanCenter (https://conan.io/center/) as a source of libraries, so that we could automate
   # installing dependencies, but it does not have all the ones we need.  Eg it has Boost, Qt, Xerces-C and Valijson,
   # but not Xalan-C.  (Someone else has already requested Xalan-C, see
   # https://github.com/conan-io/conan-center-index/issues/5546, but that request has been open a long time, so its
   # fulfilment doesn't seem imminent.)  It also doesn't yet integrate quite as well with meson as we might like (eg
   # as at 2023-01-15, https://docs.conan.io/en/latest/reference/conanfile/tools/meson.html is listed as "experimental
   # and subject to breaking changes".
   #
   # Another option is vcpkg (https://vcpkg.io/en/index.html), which does have both Xerces-C and Xalan-C, along with
   # Boost, Qt and Valijson.  There is an example here https://github.com/Neumann-A/meson-vcpkg of how to use vcpkg from
   # Meson.  However, it's pretty slow to get started with because it builds from source everything it installs
   # (including tools it depends on such as CMake) -- even if they are already installed on your system from another
   # source.  This is laudably correct but I'm too impatient to do things that way.
   #
   # Will probably take another look at Conan in future, subject to working out how to have it use already-installed
   # versions of libraries/frameworks if they are present.  The recommended way to install Conan is via a Python
   # package, which makes that part easy.  However, there is a fair bit of other ramp-up to do, and some breaking
   # changes between "current" Conan 1.X and "soon-to-be-released" Conan 2.0.  So, will leave it for now and stick
   # mostly to native installs for each of the 3 platforms (Linux, Windows, Mac).
   #
   # Other notes:
   #    - GNU coreutils (https://www.gnu.org/software/coreutils/manual/coreutils.html) is probably already installed on
   #      most Linux distros, but not necessarily on Mac and Windows.  It gives us sha256sum.
   #
   match platform.system():

      #-----------------------------------------------------------------------------------------------------------------
      #---------------------------------------------- Linux Dependencies -----------------------------------------------
      #-----------------------------------------------------------------------------------------------------------------
      case 'Linux':
         #
         # NOTE: For the moment at least, we are assuming you are on Ubuntu or another Debian-based Linux.  For other
         # flavours of the OS you need to install libraries and frameworks manually.
         #
         distroInfo = getLinuxDistroInfo()
         distroName = distroInfo["name"]
         distroRelease = distroInfo["release"]
         log.debug('Linux distro: ' + distroName + ', release: ' + distroRelease + ' (' + str(distroInfo["major"]) +
                   '.' + str(distroInfo["minor"]) + ')')

         #
         # For almost everything apart form Boost (see below) we can rely on the distro packages.  A few notes:
         #  - We need CMake even for the Meson build because meson uses CMake as one of its library-finding tools
         #  - The pandoc package helps us create man pages from markdown input
         #  - The build-essential and debhelper packages are for creating Debian packages
         #  - The rpm and rpmlint packages are for creating RPM packages
         #  - We need python-dev to build parts of Boost -- though it may be we could do without this as we only use a
         #    few parts of Boost and most Boost libraries are header-only, so do not require compilation.
         #  - To keep us on our toes, some of the package name formats change between Qt5 and Qt6.  Eg qtmultimedia5-dev
         #    becomes qt6-multimedia-dev, qtbase5-dev becomes qt6-base-dev.  Also libqt5multimedia5-plugins has no
         #    direct successor in Qt6.
         #  - The package called 'libqt6svg6-dev' in Ubuntu 22.04, is renamed to 'qt6-svg-dev' from Ubuntu 24.04.
         #
         # I have struggled to find how to install a Qt6 version of lupdate.  Compilation on Ubuntu 24.04 seems to work
         # fine with the 5.15.13 version of lupdate, so we'll make sure that's installed.  Various other comments below
         # about lupdate are (so far unsuccessful) attempts to get a Qt6 version of lupdate installed.
         #
         log.info('Ensuring libraries and frameworks are installed')
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt-get', 'update']))

         qt6svgDevPackage = 'qt6-svg-dev'
         if ('Ubuntu' == distroName and Decimal(distroRelease) < Decimal('24.04')):
            qt6svgDevPackage = 'libqt6svg6-dev'

         btUtils.abortOnRunFail(
            subprocess.run(
               ['sudo', 'apt', 'install', '-y',

                'build-essential',
                'cmake',
                'coreutils',
                'git',
                #
                # On Ubuntu 22.04, installing the packages for the Qt GUI module, does not automatically install all its
                # dependencies.  At compile-time we get an error "Qt6Gui could not be found because dependency
                # WrapOpenGL could not be found".  Various different posts suggest what packages are needed to satisfy
                # this dependency.  With a bit of trial-and-error, we have the following.
                #
                'libgl1',
                'libglx-dev',
                'libgl1-mesa-dev',
                #
                'libqt6gui6', # Qt GUI module -- needed for QColor (per https://doc.qt.io/qt-6.2/qtgui-module.html)
                'libqt6sql6-psql',
                'libqt6sql6-sqlite',
                'libqt6svg6',
                'libqt6svgwidgets6',
                'libssl-dev', # For OpenSSL headers
                'libxalan-c-dev',
                'libxerces-c-dev',
                'meson',
                'ninja-build',
                'pandoc',
                'python3',
                'python3-dev',
                'qmake6', # Possibly needed for Qt6 lupdate
                'qt6-base-dev',
                'qt6-l10n-tools', # Needed for Qt6 lupdate?
                'qt6-multimedia-dev',
                'qt6-tools-dev',
                'qt6-translations-l10n', # Puts all the *.qm files in /usr/share/qt6/translations
                qt6svgDevPackage,
                'qttools5-dev-tools', # For Qt5 version of lupdate, per comment above
                'qt6-tools-dev-tools',
                #
                # The following are needed to build the install packages (rather than just install locally)
                #
                'debhelper', # Needed to build .deb packages for Debian/Ubuntu
                'lintian'  , # Needed to validate .deb packages
                'rpm'      , # Needed to build RPMs
                'rpmlint'    # Needed to validate RPMs
               ]
            )
         )

         #
         # Thanks to the build-essential package (installed if necessary above), we know there is now _some_ version of
         # g++ on the system.  We actually need g++ 10 or newer because g++ 9 does not includes the <concepts> header.
         # So, on older releases (eg Ubuntu 20.04), we need to install a newer g++ (which will sit alongside the system
         # default one).
         #
         minGppVersion = packaging.version.parse('10.1.0')
         gppVersionOutput = btUtils.abortOnRunFail(
            subprocess.run(['g++', '--version'], encoding = "utf-8", capture_output = True)
         )
         # We only want the first line of the output from `g++ --version`.  The rest is just the copyright notice.
         gppVersionLine = str(gppVersionOutput.stdout).split('\n', 1)[0]
         log.debug('"g++ --version" returned ' + str(gppVersionOutput.stdout))
         # The version line will be something along the lines of "g++ (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0", so we
         # split on spaces and take the last field.
         gppVersionRaw = gppVersionLine.split(' ')[-1]
         gppVersionFound = packaging.version.parse(gppVersionRaw)
         log.debug('Parsed as ' + str(gppVersionFound) + '.')
         if (gppVersionFound < minGppVersion):
            log.info('Installing gcc/g++ 10 as current version is ' + str(gppVersionFound))
            btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt', 'install', '-y', 'gcc-10', 'g++-10']))
            #
            # Now we have to tell the system to use the version 10 compiler by default.  This is a little bit high-
            # handed, but we need a way for the automated "old Linux" build to work and I can't find another way to make
            # both Meson and CMake use the version 10 compiler rather than the system default one.
            #
            # This is relatively easily reversible for anyone setting up a local build.
            #
            log.info('Running "update-alternatives" command to set gcc/g++ 10 as default compiler')
            btUtils.abortOnRunFail(subprocess.run([
               'sudo', 'update-alternatives', '--install', '/usr/bin/gcc', 'gcc', '/usr/bin/gcc-10', '60', '--slave', '/usr/bin/g++', 'g++', '/usr/bin/g++-10'
            ]))

         #
         # We need a recent version of Boost, ie Boost 1.79 or newer, to use Boost.JSON.  For Windows and Mac this is
         # fine if you are installing from MSYS2 (https://packages.msys2.org/package/mingw-w64-x86_64-boost) or
         # Homebrew (https://formulae.brew.sh/formula/boost) respectively.  Unfortunately, as of late-2022, many
         # Linux distros provide only older versions of Boost.  (Eg, on Ubuntu, you can see this by running
         # 'apt-cache policy libboost-dev'.)
         #
         # First, check whether Boost is installed and if so, what version
         #
         # We'll look in the following places:
         #    /usr/include/boost/version.hpp        <-- Distro-installed Boost
         #    /usr/local/include/boost/version.hpp  <-- Manually-installed Boost
         #    ${BOOST_ROOT}/boost/version.hpp       <-- If the BOOST_ROOT environment variable is set it gives an
         #                                              alternative place to look
         #
         # Although things should compile with 1.79.0, if we're going to all the bother of installing Boost manually,
         # we'll install a more recent one.
         #
         minBoostVersion = packaging.version.parse('1.79.0')
         boostVersionToInstall = packaging.version.parse('1.87.0') # NB: This _must_ have the patch version
         maxBoostVersionFound = packaging.version.parse('0')
         possibleBoostVersionHeaders = [pathlib.Path('/usr/include/boost/version.hpp'),
                                        pathlib.Path('/usr/local/include/boost/version.hpp')]
         if ('BOOST_ROOT' in os.environ):
            possibleBoostVersionHeaders.append(pathlib.Path(os.environ['BOOST_ROOT']).joinpath('boost/version.hpp'))
         for boostVersionHeader in possibleBoostVersionHeaders:
            if (boostVersionHeader.is_file()):
               runResult = btUtils.abortOnRunFail(
                  subprocess.run(
                     ['grep', '#define BOOST_LIB_VERSION ', boostVersionHeader.as_posix()],
                     encoding = "utf-8",
                     capture_output = True
                  )
               )
               log.debug('In ' + boostVersionHeader.as_posix() + ' found ' + str(runResult.stdout))
               versionFoundRaw = re.sub(
                  r'^.*BOOST_LIB_VERSION "([0-9_]*)".*$', r'\1', str(runResult.stdout).rstrip()
               ).replace('_', '.')
               versionFound = packaging.version.parse(versionFoundRaw)
               if (versionFound > maxBoostVersionFound):
                  maxBoostVersionFound = versionFound
               log.debug('Parsed as ' + str(versionFound) + '.  (Highest found = ' + str(maxBoostVersionFound) + ')')

         #
         # The Boost version.hpp configuration header file gives two constants for defining the version of Boost
         # installed:
         #
         # BOOST_VERSION is a pure numeric value:
         #    BOOST_VERSION % 100 is the patch level
         #    BOOST_VERSION / 100 % 1000 is the minor version
         #    BOOST_VERSION / 100000 is the major version
         # So, eg, for Boost 1.79.0 (= 1.079.00), BOOST_VERSION = 107900
         #
         # BOOST_LIB_VERSION is a string value with underscores instead of dots (and without the patch level if that's
         # 0).  So, eg for Boost 1.79.0, BOOST_LIB_VERSION = "1_79" (and for 1.23.45 it would be "1_23_45")
         #
         # We use BOOST_LIB_VERSION as it's easy to convert it to a version number that Python can understand
         #
         log.debug(
            'Max version of Boost found: ' + str(maxBoostVersionFound) + '.  Need >= ' + str(minBoostVersion) +
            ', otherwise will try to install ' + str(boostVersionToInstall)
         )
         if (maxBoostVersionFound < minBoostVersion):
            log.info(
               'Installing Boost ' + str(boostVersionToInstall) + ' as newest version found was ' +
               str(maxBoostVersionFound)
            )
            #
            # To manually install the latest version of Boost from source, first we uninstall any old version
            # installed via the distro (eg, on Ubuntu, this means 'sudo apt remove libboost-all-dev'), then we follow
            # the instructions at https://www.boost.org/more/getting_started/index.html.
            #
            # It's best to leave the default install location: headers in the 'boost' subdirectory of
            # /usr/local/include and libraries in /usr/local/lib.
            #
            # (It might initially _seem_ a good idea to put things in the same place as the distro packages, ie
            # running './bootstrap.sh --prefix=/usr' to put headers in /usr/include and libraries in /usr/lib.
            # However, this will mean that Meson cannot find the manually-installed Boost, even though it can find
            # distro-installed Boost in this location.)  So, eg, for Boost 1.80 on Linux, this means the following
            # in the shell:
            #
            #    cd ~
            #    mkdir ~/boost-tmp
            #    cd ~/boost-tmp
            #    wget https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.bz2
            #    tar --bzip2 -xf boost_1_80_0.tar.bz2
            #    cd boost_1_80_0
            #    ./bootstrap.sh
            #    sudo ./b2 install
            #    cd ~
            #    sudo rm -rf ~/boost-tmp
            #
            # EXCEPT that, from time to time, the jfrog link stops working.  (AIUI, JFrog provides hosting to Boost at
            # no cost, but sometimes usage limit is exceeded on their account.)
            #
            # Instead, you can download Boost more reliably from GitHub:
            #
            #    cd ~
            #    mkdir ~/boost-tmp
            #    cd ~/boost-tmp
            #    wget https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-b2-nodocs.tar.xz
            #    tar -xf boost-1.87.0-b2-nodocs.tar.xz
            #    cd boost-1.87.0
            #    ./bootstrap.sh
            #    sudo ./b2 install
            #    cd ~
            #    sudo rm -rf ~/boost-tmp
            #
            # We can handle the temporary directory stuff more elegantly (ie RAII style) in Python however
            #
            # NOTE: On older versions of Linux, there are problems building some of the Boost libraries that I haven't
            #       got to the bottom of.  Since, for now, we only use the following Boost libraries, we use additional
            #       options on the b2 command to limit what it builds:
            #          algorithm
            #          json
            #          stacktrace
            #
            #       The list above can be recreated by running the following in the mbuild directory:
            #          grep -r '#include <boost' ../src | grep -i boost | sed 's+^.*#include <boost/++; s+/.*$++; s+.hpp.*$++' | sort -u
            #
            #       We then need to intersect this list with the output of `./b2 --show-libraries` to see which of the
            #       libraries we use require building.  On Boost 1.84, the output is (with the ones we use marked *):
            #
            #          The following libraries require building:
            #              - atomic
            #              - chrono
            #              - cobalt
            #              - container
            #              - context
            #              - contract
            #              - coroutine
            #              - date_time
            #              - exception
            #              - fiber
            #              - filesystem
            #              - graph
            #              - graph_parallel
            #              - headers
            #              - iostreams
            #              - json                *
            #              - locale
            #              - log
            #              - math
            #              - mpi
            #              - nowide
            #              - program_options
            #              - python
            #              - random
            #              - regex
            #              - serialization
            #              - stacktrace          *
            #              - system
            #              - test
            #              - thread
            #              - timer
            #              - type_erasure
            #              - url
            #              - wave
            #
            #       If we wanted to, we could do all the above programatically here.  But, for now, I think it's not
            #       worth the effort, because it's relatively infrequently that we need to change the hard-coded
            #       parameters below.
            #
            with tempfile.TemporaryDirectory(ignore_cleanup_errors = True) as tmpDirName:
               previousWorkingDirectory = pathlib.Path.cwd().as_posix()
               os.chdir(tmpDirName)
               log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())
               boostBaseName = 'boost-' + str(boostVersionToInstall)
               boostUnderscoreName = 'boost_' + str(boostVersionToInstall).replace('.', '_')
#               downloadFile(
#                  'https://boostorg.jfrog.io/artifactory/main/release/' + str(boostVersionToInstall) + '/source/' +
#                  boostUnderscoreName + '.tar.bz2'
#               )
               downloadFile(
                  'https://github.com/boostorg/boost/releases/download/' +
                  boostBaseName +  '/' + boostBaseName + '-b2-nodocs.tar.xz'
               )
               log.debug('Boost download completed')
#               shutil.unpack_archive(boostUnderscoreName + '.tar.bz2')
               shutil.unpack_archive(boostBaseName + '-b2-nodocs.tar.xz')
               log.debug('Boost archive extracted')
#               os.chdir(boostUnderscoreName)
               os.chdir(boostBaseName)
               log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())
               btUtils.abortOnRunFail(subprocess.run(['./bootstrap.sh', '--with-python=python3']))
               log.debug('Boost bootstrap finished')
               btUtils.abortOnRunFail(subprocess.run(
                  ['sudo', './b2', '--with-json',
                                   '--with-stacktrace',
                                   'install'])
               )
               log.debug('Boost install finished')
               os.chdir(previousWorkingDirectory)
               log.debug('Working directory now ' + pathlib.Path.cwd().as_posix() + '.  Removing ' + tmpDirName)
               #
               # The only issue with the RAII approach to removing the temporary directory is that some of the files
               # inside it will be owned by root, so there will be a permissions error when Python attempts to delete
               # the directory tree.  Fixing the permissions beforehand is a slightly clunky way around this.
               #
               btUtils.abortOnRunFail(
                  subprocess.run(
                     ['sudo', 'chmod', '--recursive', 'a+rw', tmpDirName]
                  )
               )

         #
         # Although Ubuntu 24.04 gives us Meson 1.3.2, Ubuntu 22.04 packages only have Meson 0.61.2.  We need Meson
         # 0.63.0 or later.  In this case it means we have to install Meson via pip, which is not ideal on Linux.
         #
         # Specifically, as explained at https://mesonbuild.com/Getting-meson.html#installing-meson-with-pip, although
         # using the pip3 install gets a newer version, we have to do the pip install as root (which is normally not
         # recommended).  If we don't do this, then running `meson install` (or even `sudo meson install`) will barf on
         # Linux (because we need to be able to install files into system directories).
         #
         # So, where a sufficiently recent version of Meson is available in the distro packages (eg
         # `sudo apt install meson` on Ubuntu etc) it is much better to install this.   Installing via pip is a last
         # resort.
         #
         # The distro ID we get from 'lsb_release -is' will be 'Ubuntu' for all the variants of Ubuntu (eg including
         # Kubuntu).  Not sure what happens on derivatives such as Linux Mint though.
         #
         # ANOTHER problem on Ubuntu 22.04 is that lupdate doesn't work with Qt6, because it runs qtchooser which does
         # not work with Qt6 on Ubuntu 22.04 because of the following "won't fix"
         # bug: https://bugs.launchpad.net/ubuntu/+source/qtchooser/+bug/1964763.  The workaround suggested at
         # https://askubuntu.com/questions/1460242/ubuntu-22-04-with-qt6-qmake-could-not-find-a-qt-installation-of is
         # to run `sudo qtchooser -install qt6 $(which qmake6)`, so that's what we do here after sorting out the Meson
         # install.
         #
         if ('Ubuntu' == distroName and Decimal(distroRelease) < Decimal('24.04')):
            log.info('Installing newer version of Meson the hard way')
            btUtils.abortOnRunFail(subprocess.run(['sudo', 'apt', 'remove', '-y', 'meson']))
            btUtils.abortOnRunFail(subprocess.run(['sudo', 'pip3', 'install', 'meson']))
            #
            # Now fix lupdate
            #
            fullPath_qmake6 = shutil.which('qmake6')
            btUtils.abortOnRunFail(subprocess.run(['sudo', 'qtchooser', '-install', 'qt6', fullPath_qmake6]))

      #-----------------------------------------------------------------------------------------------------------------
      #--------------------------------------------- Windows Dependencies ----------------------------------------------
      #-----------------------------------------------------------------------------------------------------------------
      case 'Windows':
         log.debug('Windows')
         #
         # First thing is to detect whether we're in the MSYS2 environment, and, if so, whether we're in the right
         # version of it.
         #
         # We take the existence of an executable `uname` in the path as a pretty good indicator that we're in MSYS2
         # or similar environment).  Then the result of running that should tell us if we're in the 32-bit version of
         # MSYS2.  (See comment below on why we don't yet support the 64-bit version, though I'm sure we'll fix this one
         # day.)
         #
         exe_uname = shutil.which('uname')
         if (exe_uname is None or exe_uname == ''):
            log.critical('Cannot find uname.  This script needs to be run under MSYS2 - see https://www.msys2.org/')
            exit(1)
         # We could just run uname without the -a option, but the latter gives some useful diagnostics to log
         unameResult = str(
            btUtils.abortOnRunFail(subprocess.run([exe_uname, '-a'], encoding = "utf-8", capture_output = True)).stdout
         ).rstrip()
         log.debug('Running uname -a gives ' + unameResult)
         # Output from `uname -a` will be of the form
         #    MINGW64_NT-10.0-19044 Matt-Virt-Win 3.4.3.x86_64 2023-01-11 20:20 UTC x86_64 Msys
         # We just need the bit before the first underscore, eg
         #    MINGW64
         terminalVersion = unameResult.split(sep='_', maxsplit=1)[0]

         if (terminalVersion != 'MINGW64'):
            # In the past, we built only 32-bit packages (i686 architecture) on Windows because of problems getting
            # 64-bit versions of NSIS plugins to work.  However, we now invoke NSIS without plugins, so the 64-bit build
            # seems to be working.
            #
            # As of January 2024, some of the 32-bit MSYS2 packages/groups we were previously relying on previously are
            # no longer available.  So now, we only build 64-bit packages (x86_64 architecture) on Windows.
            log.critical('Running in ' + terminalVersion + ' but need to run in MINGW64 (ie 64-bit build environment)')
            exit(1)

         # Ensure pip is up-to-date.  This is what the error message tells you to run if it's not!
         log.info('Ensuring Python pip is up-to-date')
         btUtils.abortOnRunFail(subprocess.run([exe_python, '-m', 'pip', 'install', '--upgrade', 'pip']))

         #
         # When we update packages below, we get "error: failed to commit transaction (conflicting files)" errors for a
         # bunch of Python packaging files unless we force Pacman to overwrite them.  This is somewhat less of a hack
         # than specifying --overwrite globally.
         #
         # We have to do both 32-bit and 64-bit versions of Python here to be certain.
         #
         # Note that the rules for glob expansion are different when invoking a command from Python then when typing it
         # on the command line, hence why the overwrite parameter is '*python*' not '"*python*"'.
         #
         log.info('Ensuring Python packaging is up-to-date')
         btUtils.abortOnRunFail(subprocess.run(['pacman', '-S', '--noconfirm', '--overwrite', '*python*', 'mingw-w64-i686-python-packaging']))
         btUtils.abortOnRunFail(subprocess.run(['pacman', '-S', '--noconfirm', '--overwrite', '*python*', 'mingw-w64-x86_64-python-packaging']))

         #
         # Before we install packages, we want to make sure the MSYS2 installation itself is up-to-date, otherwise you
         # can hit problems
         #
         #   pacman -S -y should download a fresh copy of the master package database
         #   pacman -S -u should upgrades all currently-installed packages that are out-of-date
         #
         log.info('Ensuring required libraries and frameworks are installed')
         btUtils.abortOnRunFail(subprocess.run(['pacman', '-S', '-y', '--noconfirm']))
         btUtils.abortOnRunFail(subprocess.run(['pacman', '-S', '-u', '--noconfirm']))

         #
         # We _could_ just invoke pacman once with the list of everything we want to install.  However, this can make
         # debugging a bit harder when there is a pacman problem, because it doesn't always give the most explanatory
         # error messages.  So we loop round and install one thing at a time.
         #
         # Note that the --disable-download-timeout option on Pacman proved often necessary because of slow mirror
         # sites, so we now specify it routinely.
         #
         # As noted above, we no longer support 32-bit ('i686') builds and now only support 64-bit ('x86_64') ones.
         # NOTE that, as explained at
         # https://forum.qt.io/topic/140029/i-ve-downloaded-the-qt6-version-and-mingw-for-gcc-11-version/7, we will
         # still see mention of "mingw32" in bits of the toolchain on 64-bit builds, but the "32" in the name is there
         # for historical reasons and does not mean it's not a fully 64-bit build!
         #
         # Compiling the list of required packages here involves a bit of trial-and-error.  A good starting point for
         # what we probably need is found by searching for qt6 in the list at https://packages.msys2.org/base.  However,
         # it can still be challenging to work out which package provided the missing binary or library that is
         # preventing your build from working.
         #
         # Eg, when you install mingw-w64-x86_64-qt6-static, you get a message saying mingw-w64-x86_64-clang-libs is an
         # "optional dependency" required for lupdate and qdoc.  Since we need lupdate, we therefore need to install
         # clang-libs, even though our own compilation is done with GCC.  (In fact, per comments in meson.build, lupdate
         # also gets a name change to lupdate-qt6, but we don't have to worry about that here!)
         #
         # So, it may be that the list below is not minimal, but it should be sufficient!
         #
         # 2024-07-29: TBD: Not totally sure we need angleproject.  It wasn't previously a requirement, but, as of
         #                  recently, windeployqt complains if it can't find it.  The alternative would be to pass
         #                  "-no-angle" as a parameter to windeployqt.  However, that option seems to not be present
         #                  in Qt 6 (see https://doc.qt.io/qt-6/windows-deployment.html vs
         #                  https://doc.qt.io/qt-5/windows-deployment.html).
         #
         arch = 'x86_64'
         installList = ['base-devel',
                        'cmake',
                        'coreutils',
                        'doxygen',
                        'gcc',
                        'git',
                        'mingw-w64-' + arch + '-boost',
                        'mingw-w64-' + arch + '-cmake',
                        'mingw-w64-' + arch + '-clang-libs', # Needed for lupdate
                        'mingw-w64-' + arch + '-libbacktrace',
                        'mingw-w64-' + arch + '-meson',
                        'mingw-w64-' + arch + '-nsis',
                        'mingw-w64-' + arch + '-freetype',
                        'mingw-w64-' + arch + '-harfbuzz',
                        'mingw-w64-' + arch + '-librsvg', # Possibly needed to include in packaging for SVG display
                        'mingw-w64-' + arch + '-openssl', # OpenSSL headers and library
                        'mingw-w64-' + arch + '-qt6-base',
                        'mingw-w64-' + arch + '-qt6-declarative', # Also needed for lupdate?
                        'mingw-w64-' + arch + '-qt6-static',
                        'mingw-w64-' + arch + '-qt6-svg',
                        'mingw-w64-' + arch + '-qt6-tools',
                        'mingw-w64-' + arch + '-qt6-translations',
                        'mingw-w64-' + arch + '-qt6',
                        'mingw-w64-' + arch + '-toolchain',
                        'mingw-w64-' + arch + '-xalan-c',
                        'mingw-w64-' + arch + '-xerces-c',
#                        'mingw-w64-' + arch + '-7zip', # To unzip NSIS plugins
                        'mingw-w64-' + arch + '-angleproject', # See comment above
                        'mingw-w64-' + arch + '-ntldd', # Dependency tool useful for running manually -- see below
                        ]
         for packageToInstall in installList:
            log.debug('Installing ' + packageToInstall)
            btUtils.abortOnRunFail(
               subprocess.run(
                  ['pacman', '-S', '--needed', '--noconfirm', '--disable-download-timeout', packageToInstall]
               )
            )

         #
         # Download NSIS plugins
         #
         # In theory we can use RAII here, eg:
         #
         #   with tempfile.TemporaryDirectory(ignore_cleanup_errors = True) as tmpDirName:
         #      previousWorkingDirectory = pathlib.Path.cwd().as_posix()
         #      os.chdir(tmpDirName)
         #      ...
         #      os.chdir(previousWorkingDirectory)
         #
         # However, in practice, this gets messy when there is an error (eg download fails) because Windows doesn't like
         # deleting files or directories that are in use.  So, in the event of the script needing to terminate early,
         # you get loads of errors, up to and including "maximum recursion depth exceeded" which rather mask whatever
         # the original problem was.
         #
         tmpDirName = tempfile.mkdtemp()
         previousWorkingDirectory = pathlib.Path.cwd().as_posix()
         os.chdir(tmpDirName)
         downloadFile('https://nsis.sourceforge.io/mediawiki/images/a/af/Locate.zip')
         shutil.unpack_archive('Locate.zip', 'Locate')
         downloadFile('https://nsis.sourceforge.io/mediawiki/images/7/76/Nsislog.zip')
         shutil.unpack_archive('Nsislog.zip', 'Nsislog')
         copyFilesToDir(['Locate/Include/Locate.nsh'], '/mingw32/share/nsis/Include/')
         copyFilesToDir(['Locate/Plugin/locate.dll',
                         'Nsislog/plugin/nsislog.dll'],'/mingw32/share/nsis/Plugins/ansi/')
         os.chdir(previousWorkingDirectory)
         shutil.rmtree(tmpDirName, ignore_errors=False)

      #-----------------------------------------------------------------------------------------------------------------
      #---------------------------------------------- Mac OS Dependencies ----------------------------------------------
      #-----------------------------------------------------------------------------------------------------------------
      case 'Darwin':
         log.debug('Mac')
         #
         # There are one or two things we can't install automatically because Apple won't let us.  Eg, to install Xcode,
         # you either need to "Open the Mac App Store" or to download from
         # https://developer.apple.com/downloads/index.action, which requires you to have an Apple Developer account,
         # which you can only get by paying Apple $100 per year.
         #
         # Other things should be possible -- eg Homebrew and MacPorts -- but are a bit fiddly.  We're working on those.
         #
         # But most things we attempt to do below.
         #

         #
         # It's useful to know what version of MacOS we're running on.  Getting the version number is straightforward,
         # so we start with that.
         #
         macOsVersionRaw = btUtils.abortOnRunFail(
            subprocess.run(['sw_vers', '-productVersion'], capture_output=True)
         ).stdout.decode('UTF-8').rstrip()
         log.debug('MacOS version: ' + macOsVersionRaw)
         parsedMacOsVersion = packaging.version.parse(macOsVersionRaw)
         log.debug('MacOS version parsed: ' + str(parsedMacOsVersion))
         #
         # Getting the "release name" (aka "friendly name") is a bit more tricky.  See
         # https://apple.stackexchange.com/questions/333452/how-can-i-find-the-friendly-name-of-the-operating-system-from-the-shell-term
         # for various approaches with varying reliability.  However, in reality, it's simpler to hard-code the info in
         # this script by copying it from https://en.wikipedia.org/wiki/MacOS#Timeline_of_releases.  We just have to
         # update the list below whenever a new version of MacOS comes out.
         #
         macOsVersionToReleaseName = {
            '15'    : 'Sequoia'      ,
            '14'    : 'Sonoma'       ,
            # Can't guarantee that other parts of the build/packaging system will work on these older versions, but
            # doesn't hurt to at least be able to look them up.
            '13'    : 'Ventura'      ,
            '12'    : 'Monterey'     ,
            '11'    : 'Big Sur'      ,
            '10.15' : 'Catalina'     ,
            '10.14' : 'Mojave'       ,
            '10.13' : 'High Sierra'  ,
            '10.12' : 'Sierra'       ,
            '10.11' : 'El Capitan'   ,
            '10.10' : 'Yosemite'     ,
            '10.9'  : 'Mavericks'    ,
            '10.8'  : 'Mountain Lion',
            '10.7'  : 'Lion'         ,
            '10.6'  : 'Snow Leopard' ,
            '10.5'  : 'Leopard'      ,
            '10.4'  : 'Tiger'        ,
            '10.3'  : 'Panther'      ,
            '10.2'  : 'Jaguar'       ,
            '10.1'  : 'Puma'         ,
            '10.0'  : 'Cheetah'
         }
         #
         # Version number is major.minor.micro.
         #
         # Prior to MacOS 10, we need the major and minor part of the version number - because 10.15 has a different
         # name than 10.14.  From 11 on, we only need the major part as, eg 14.6 and 14.7 are both "Sonoma".
         #
         macOsVersion = str(parsedMacOsVersion.major)
         if (macOsVersion == '10'):
            macOsVersion += '.' + str(parsedMacOsVersion.minor)
         macOsReleaseName = macOsVersionToReleaseName[macOsVersion]
         log.debug('MacOS ' + macOsVersion + ' release name: ' + macOsReleaseName)

         #
         # The two main "package management" systems for MacOS are Homebrew (https://brew.sh/), which provides the
         # `brew` command, and MacPorts (https://ports.macports.org/), which provides the `port` command.  They work in
         # different ways, and have different philosophies.  Homebrew distributes binaries and MacPorts (mostly) builds
         # everything from source.  MacPorts installs for all users and requires sudo.  Homebrew installs only for the
         # current user and does not require sudo.  This means they install things to different locations:
         #    - Homebrew packages are installed under /usr/local/Cellar/ with symlinks in /usr/local/opt/
         #    - MacPorts packages are installed under /opt/local
         # Note too that package names can vary slightly between HomeBrew and MacPorts.
         #
         # Unfortunately, the different approaches mean there are limits on the extent to which you can mix-and-match
         # between the two systems.
         #
         # In the past, we installed everything via Homebrew as it was very quick and seemed to work, provided we had
         # both directories in the include path when we came to compile (because CMake and Meson can generally take care
         # of finding a library automatically once given its name).
         #
         # However, as at 2023-12-01, Homebrew has stopped supplying a package for Xalan-C.  So, we started installing
         # Xalan and Xerces using MacPorts, whilst still installing everything else via Homebrew.  This seemed to work
         # for a while, but in 2024, after upgrading to Qt6, we started having problems with the Qt `macdeployqt`
         # command (which is used to pull all the necessary Qt libraries into the app bundle we distribute).  AFAICT
         # this is a known issue (https://github.com/orgs/Homebrew/discussions/2823).  So now we are trying installing
         # Qt6 via MacPorts.
         #
         # In the expectation that we might well chop and change between what we install via which package manager, we
         # aim to support both below and to make it relatively easy to change which one is used to install which
         # packages.
         #
         # Both package managers handle dependencies, so we could make our list of what to install very minimal (eg
         # installing Xalan-C will cause Xerces-C to be installed too, as the former depends on the latter).  However, I
         # think it's clearer to explicitly list all the _direct_ dependencies (eg we do make calls directly into
         # Xerces, so we should list it as an explicit dependency).
         #

         #
         # Installing Homebrew is, in theory, somewhat easier and more self-contained than MacPorts as you just run the
         # following:
         #    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
         # In practice, invoking that command from this script is a bit fiddly to get right.  For the moment, we simply
         # assume Homebrew is already installed (because it is on the GitHub actions).
         #

         #
         # .:TBD:. Installing Boost here doesn't seem to give us libboost_stacktrace_backtrace
         #         Also, trying to use the "--cc=clang" option to install boost gives an error ("Error: boost: no bottle
         #         available!")  For the moment, we're just using Boost header files on Mac though, so this should be
         #         OK.
         #
         # We install the tree command here as, although it's not needed to do the build itself, it's useful for
         # diagnosing certain build problems (eg to see what changes certain parts of the build have made to the build
         # directory tree) when the build is running as a GitHub action.
         #
         installListBrew = [
#                            'llvm',
#                            'gcc',
                            'coreutils', # Needed for sha256sum
#                            'cmake',
#                            'ninja',
#                            'meson',
                            'boost',
                            'doxygen',
#                            'git',
#                            'pandoc',
                            'tree',
                            'dylibbundler',
#                            'qt@6',
                            'openssl@3', # OpenSSL headers and library
#                            'xalan-c',
#                            'xerces-c'
                            ]
         for packageToInstall in installListBrew:
            #
            # If we try to install a Homebrew package that is already installed, we'll get a warning.  This isn't
            # horrendous, but it looks a bit bad on the GitHub automated builds (because a lot of things are already
            # installed by the time this script runs).  As explained at
            # https://apple.stackexchange.com/questions/284379/with-homebrew-how-to-check-if-a-software-package-is-installed,
            # the simplest (albeit perhaps not the most elegant) way to check whether a package is already installed is
            # to run `brew list`, throw away the output, and look at the return code, which will be 0 if the package is
            # already installed and 1 if it is not.  In the shell, we can use the magic of short-circuit evaluation
            # (https://en.wikipedia.org/wiki/Short-circuit_evaluation) to, at a small legibility cost, do the whole
            # check-and-install, in a single line.  But in Python, it's easier to do it in two steps.
            #
            log.debug('Checking ' + packageToInstall)
            brewListResult = subprocess.run(['brew', 'list', packageToInstall],
                                            stdout = subprocess.DEVNULL,
                                            stderr = subprocess.DEVNULL,
                                            capture_output = False)
            if (brewListResult.returncode == 0):
               log.debug('Homebrew reports ' + packageToInstall + ' already installed')
            else:
               log.debug('Installing ' + packageToInstall + ' via Homebrew')
               #
               # We specify --formula here because sometimes we want to disambiguate from a "formula" (what Homebrew
               # calls its regular package definitions) and a "cask" (package definitions used in an extension to
               # Homebrew for installing graphical applications).  In cases of ambiguity, Homebrew will always assume
               # formula if neither '--formula' nor '--cask' is specified, but it emits a warning, which we might as
               # well suppress, since we know we always want the formula.
               #
               btUtils.abortOnRunFail(subprocess.run(['brew', 'install', '--formula', packageToInstall]))

         #
         # Having installed things it depends on, we should now be able to install MacPorts -- either from source or
         # precompiled binary.
         #
         # The instructions at https://guide.macports.org/#installing say that we probably don't need to install Xcode
         # as only a few ports need it.  So, for now, we haven't tried to install that.
         #
         # Code to install both from binary and from source is below, as we have hit various problems in the past.
         # *** Obviously only one block needs to be uncommented at a time. ***
         #
         # In both cases, curl options are:
         #    -L = If the server reports that the requested page has moved to a different location (indicated with a
         #         Location: header and a 3XX  response  code),  this option makes curl redo the request on the new
         #         place.
         #    -O = Write output to a local file named like the remote file we get. (Only the file part of the remote
         #         file is used, the path is cut off.)  The file is saved in the current working directory.
         #
         # TBD: For the moment we hard-code the version of MacPorts, but we should probably find it out from GitHub in
         #      a similar way that we check our own latest releases in the C++ code.
         #
         macPortsVersion = '2.11.5'
         macPortsName = 'MacPorts-' + macPortsVersion
         #
         # The instructions for binary install at https://guide.macports.org/#installing.macports.binary require user
         # interaction ("Double-click the downloaded package installer"), but, with a little research, the steps can be
         # scripted.
         #
         log.debug('Installing MacPorts from binary')
         btUtils.abortOnRunFail(subprocess.run(['pwd']))
         btUtils.abortOnRunFail(subprocess.run(['ls', '-l']))
         macPortsPackage = macPortsName + '-' + macOsVersion + '-' + macOsReleaseName + '.pkg'
         downloadUrl = 'https://github.com/macports/macports-base/releases/download/v' + macPortsVersion + '/' + macPortsPackage
         btUtils.abortOnRunFail(subprocess.run(['curl', '-L', '-O', downloadUrl]))
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'installer', '-package', macPortsPackage, '-target', '/']))
         btUtils.abortOnRunFail(subprocess.run(['ls', '-l']))

         #
         # Instructions for source install are at https://guide.macports.org/#installing.macports.source.
         #
#         log.debug('Installing MacPorts from source')
#         btUtils.abortOnRunFail(subprocess.run(['curl', '-L', '-O', 'https://distfiles.macports.org/MacPorts/' + macPortsName + '.tar.bz2']))
#         btUtils.abortOnRunFail(subprocess.run(['tar', 'xf', macPortsName + '.tar.bz2']))
#         btUtils.abortOnRunFail(subprocess.run(['cd', macPortsName]))
#         btUtils.abortOnRunFail(subprocess.run(['./configure']))
#         btUtils.abortOnRunFail(subprocess.run(['make']))
#         btUtils.abortOnRunFail(subprocess.run(['sudo', 'make', 'install']))
#         btUtils.abortOnRunFail(subprocess.run(['cd', '..']))
#         btUtils.abortOnRunFail(subprocess.run(['pwd']))
#         btUtils.abortOnRunFail(subprocess.run(['ls', '-l']))

         #
         # Neither binary nor source install automatically adds the port command to the path, so we do it here.
         # As below, we want these additional paths to show up permanently.  Using a file in /etc/paths.d/ should work
         # for someone doing this set-up locally...
         #
         macPortsPrefix = '/opt/local'
         log.debug('Before fix-up, PATH=' + os.environ["PATH"])
         os.environ["PATH"] = macPortsPrefix + '/bin' + os.pathsep + macPortsPrefix + '/sbin' + os.pathsep + os.environ["PATH"]
         log.debug('After fix-up, PATH=' + os.environ["PATH"])
         macPortsDirFile = '/etc/paths.d/90-macPortsPaths'
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'touch'              , macPortsDirFile]))
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'chown', 'root:wheel', macPortsDirFile]))
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'chmod', 'a+rw'      , macPortsDirFile]))
         with open(macPortsDirFile, 'a+') as macPortsDirPaths:
            macPortsDirPaths.write(macPortsPrefix + '/bin'  + '\n')
            macPortsDirPaths.write(macPortsPrefix + '/sbin' + '\n')
            macPortsDirPaths.write(macPortsPrefix + '/lib'  + '\n')
         #
         # ...but, for GitHub actions, writing to the file in the GITHUB_PATH environment variable is the supported way
         # to add something to the path for subsequent steps.
         #
         if 'GITHUB_PATH' in os.environ:
            githubPathFile = os.environ['GITHUB_PATH']
            log.debug('GITHUB_PATH=' + githubPathFile)
            if githubPathFile:
               with open(githubPathFile, 'a+') as githubPaths:
                  githubPaths.write(macPortsPrefix + '/bin'  + '\n')
                  githubPaths.write(macPortsPrefix + '/sbin' + '\n')
                  githubPaths.write(macPortsPrefix + '/lib'  + '\n')

         #
         # Just because we have MacPorts installed, doesn't mean its list of software etc will be up-to-date.  So fix
         # that first.
         #
         # If there is an error, MacPorts tells you to run again with the -v option to find out why, so we just run with
         # that from the outset, and live with the fact that it generates a lot of logging.
         #
         log.debug('First run of MacPorts selfupdate')
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'port', '-v', 'selfupdate']))

         #
         # Sometimes you need to run selfupdate twice, because MacPorts itself was too out of date to update the ports
         # tree.  (You'll get an error that "Not all sources could be fully synced using the old version of MacPorts.
         # Please run selfupdate again now that MacPorts base has been updated."
         #
         # Rather than try to detect this, we just always run selfupdate twice.  If the second time is a no-op then no
         # harm is done.
         #
         log.debug('Second run of MacPorts selfupdate')
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'port', 'selfupdate']))

         # Per https://guide.macports.org/#using.port.diagnose this will tell us about "common issues in the user's
         # environment".
         log.debug('Check environment is OK')
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'port', 'diagnose', '--quiet']))

         # Per https://guide.macports.org/#using.port.installed, this tells us what ports are already installed
         log.debug('List ports already installed')
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'port', 'installed']))

         #
         # Now install packages we want from MacPorts
         #
         # Note that it is not sufficient to install 'boost' here because, as at 2024-11-09, this still only gives us
         # Boost 1.76 (from April 2021) and we need at least Boost 1.79.  Installing 'boost181' gives us Boost 1.81
         # (from December 2022) which seems to be the newest version available in MacPorts.
         #
         installListPort = [
                            'llvm-19',
                            'cmake',
                            'ninja',
                            'meson',
#                            'boost181',
#                            'doxygen',
                            'openssl',
#                            'tree',
#                            'dylibbundler',
                            'pandoc',
                            'xercesc3',
                            'xalanc',
                            'qt6',
                            'qt6-qttranslations',
                            'dbus'
                            ]
         for packageToInstall in installListPort:
            log.debug('Installing ' + packageToInstall + ' via MacPorts')
            #
            # Add the '-v' option here for "verbose", which is useful in diagnosing problems with port installs:
            #
            #    btUtils.abortOnRunFail(subprocess.run(['sudo', 'port', '-v', 'install', packageToInstall]))
            #
            # However, it generates a _lot_ of output, so we normally leave it turned off.
            #
            btUtils.abortOnRunFail(subprocess.run(['sudo', 'port', 'install', packageToInstall]))

         #
         # Sometimes MacPorts prompts you to upgrade already installed ports with the `port upgrade outdated` command.
         # I'm not convinced it is always harmless to do this!  Uncomment the following if we decide it's a good idea.
         #
#         log.debug('Ensuring installed ports up-to-date')
#         btUtils.abortOnRunFail(subprocess.run(['sudo', 'port', 'upgrade', 'outdated']))

         #--------------------------------------------------------------------------------------------------------------
         # By default, even once Qt is installed, whether from Homebrew or MacPorts, Meson will not find it.  Apparently
         # this is intentional to allow two versions of Qt to be installed at the same time.  The way to fix things
         # differs between the two package managers.  We include both sets of fix-up code.
         #--------------------------------------------------------------------------------------------------------------
         qtInstalledBy = []
         if ('qt6' in installListPort):
            qtInstalledBy.append('MacPorts')
         if ('qt@6' in installListBrew):
            qtInstalledBy.append('Homebrew')
         log.debug('Qt installed by ' + ', '.join(qtInstalledBy))

         if ([] == qtInstalledBy):
            log.error('Did not understand how Qt was installed!')

         if (len(qtInstalledBy) > 1):
            log.error('Qt installed twice!')

         qtBaseDir = ''
         if ('Homebrew' in qtInstalledBy):
            #
            # ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
            # ┃ ××××××××××××××××××××××××××××××××× Fix-ups for Homebrew-installed Qt ××××××××××××××××××××××××××××××××× ┃
            # ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
            #

            #
            # For a Homebrew install, the suggestion at
            # https://stackoverflow.com/questions/29431882/get-qt5-up-and-running-on-a-new-mac is to run
            # `brew link qt5 --force` to "symlink the various Qt binaries and libraries into your /usr/local/bin and
            # /usr/local/lib directories".
            #
            btUtils.abortOnRunFail(subprocess.run(['brew', 'link', '--force', 'qt6']))

            qtBaseDir = btUtils.abortOnRunFail(
               subprocess.run(['brew', '--prefix', 'qt@6'], capture_output=True)
            ).stdout.decode('UTF-8').rstrip()

            qmakePath = findFirstMatchingFile('qmake', qtBaseDir)
            if ('' == qmakePath):
               log.error('Unable to write to find qmake under ' + qtBaseDir)
            else:
               log.debug('Found qmake at ' + qmakePath)

            qtBinDir = os.path.dirname(qmakePath)

            #
            # Further notes from when we did this for Qt5:
            #    ┌─────────────────────────────────────────────────────────────────────────────────────────────────────┐
            #    │ Additionally, per lengthy discussion at https://github.com/Homebrew/legacy-homebrew/issues/29938,   │
            #    │ it seems we might also need either:                                                                 │
            #    │    ln -s /usr/local/Cellar/qt5/5.15.7/mkspecs /usr/local/mkspecs                                    │
            #    │    ln -s /usr/local/Cellar/qt5/5.15.7/plugins /usr/local/plugins                                    │
            #    │ or:                                                                                                 │
            #    │    export PATH=/usr/local/opt/qt5/bin:$PATH                                                         │
            #    │ The former gives permission errors, so we do the latter in mac.yml (but NB it's only needed for     │
            #    │ CMake).                                                                                             │
            #    └─────────────────────────────────────────────────────────────────────────────────────────────────────┘
            #

         elif ('MacPorts' in qtInstalledBy):
            #
            # ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
            # ┃ ××××××××××××××××××××××××××××××××× Fix-ups for MacPorts-installed Qt ××××××××××××××××××××××××××××××××× ┃
            # ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
            #

            #
            # For a MacPorts install, the suggestion at
            # https://stackoverflow.com/questions/29431882/get-qt5-up-and-running-on-a-new-mac is to search for qmake
            # under the /opt directory and then make a symlink to it in the /opt/local/bin/ directory.  Eg, if qmake
            # were found in /opt/local/libexec/qt5/bin/, then we'd want to run
            # `ln -s /opt/local/libexec/qt5/bin/qmake /opt/local/bin/qmake`.
            #
            qmakePath = findFirstMatchingFile('qmake', '/opt')
            if ('' == qmakePath):
               log.error('Unable to write to find qmake under /opt')
            else:
               log.debug('Found qmake at ' + qmakePath)
               #
               # You might think we could just create the symlink directly in Python, eg by running
               # `pathlib.Path('/opt/local/bin/qmake').symlink_to(qmakePath)`.  However, this will give a "Permission
               # denied" error.  We need to do it as root, via sudo.
               #
               btUtils.abortOnRunFail(subprocess.run(['sudo', 'ln', '-s', qmakePath, '/opt/local/bin/qmake'], capture_output=False))

            qtBinDir = os.path.dirname(qmakePath)
            qtBaseDir = os.path.dirname(qtBinDir)

         #
         # Normally leave the next line commented out as it generates a _lot_ of output.  Can be useful for diagnosing
         # problems with GitHub action builds.
         #
#         btUtils.abortOnRunFail(subprocess.run(['tree', '-sh', qtBaseDir], capture_output=False))

         #
         # We now fix various environment variables needed for the builds to pick up Qt headers, libraries, etc.
         #
         # When intalling Qt5 via homebrew, the brew command explicitly tells us to do the following.  We do a slightly
         # more generic version to work with any verison of Qt and regardless of whether Homebrew or MacPorts was used
         # to install Qt.
         #    ┌─────────────────────────────────────────────────────────────────────────────────────────────────────┐
         #    │ But the brew command to install Qt also tells us to do the following:                               │
         #    │                                                                                                     │
         #    │    echo 'export PATH="/usr/local/opt/qt@5/bin:$PATH"' >> ~/.bash_profile                            │
         #    │    export LDFLAGS="-L/usr/local/opt/qt@5/lib"                                                       │
         #    │    export CPPFLAGS="-I/usr/local/opt/qt@5/include"                                                  │
         #    │    export PKG_CONFIG_PATH="/usr/local/opt/qt@5/lib/pkgconfig"                                       │
         #    │                                                                                                     │
         #    │ Note however that, in a GitHub runner, the first of these will give "[Errno 13] Permission denied". │
         #    └─────────────────────────────────────────────────────────────────────────────────────────────────────┘
         #
         # We also make sure that the Qt bin directory is in the PATH (otherwise, when we invoke Meson from this script
         # to set up the mbuild directory, it will give an error about not being able to find Qt tools such as
         # `lupdate`).
         #
         log.debug('Qt Base Dir: ' + qtBaseDir + ', Bin Dir: ' + qtBinDir)
         os.environ["PATH"] = qtBinDir + os.pathsep + os.environ["PATH"]
         #
         # Equally, the Qt lib directory is something we need in the path for packaging
         #
         qtLibDir = os.path.normpath(os.path.join(qtBaseDir, "lib"))
         log.debug('Qt Lib Dir: ' + qtLibDir)

         #
         # See
         # https://stackoverflow.com/questions/1466000/difference-between-modes-a-a-w-w-and-r-in-built-in-open-function
         # for a good summary (clearer than the Python official docs) of the mode flag on open.
         #
         # As always, we have to remember to explicitly do things that would be done for us automatically by the
         # shell (eg expansion of '~').
         #
         # Also, although you might think it is a reasonable assumption that ~/.bash_profile is owned by the current
         # user, it turns out this is not always the case.  In 2025 we started seeing the script fail here on the
         # GithHub actions because ~/.bash_profile was owned by root and not writable by the current user ("runner").
         # So we force the ownership back to what it should be before attempting to open the file for writing.
         #
         bashProfilePath = os.path.expanduser('~/.bash_profile')
         log.debug('Adding Qt Bin and Lib Dirs ' + qtBinDir + '; ' + qtLibDir + ' to PATH in ' + bashProfilePath)
         btUtils.abortOnRunFail(subprocess.run(['ls', '-l', bashProfilePath], capture_output=False))
         currentUser = getpass.getuser()
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'chown', currentUser, bashProfilePath], capture_output=False))
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'chmod', 'u+w', bashProfilePath], capture_output=False))
         btUtils.abortOnRunFail(subprocess.run(['ls', '-l', bashProfilePath], capture_output=False))
         with open(bashProfilePath, 'a+') as bashProfile:
            bashProfile.write('export PATH="' + qtBinDir + os.pathsep + qtLibDir + os.pathsep + '$PATH"')
         #
         # Another way to "permanently" add something to PATH on MacOS, is by either appending to the /etc/paths file or
         # creating a file in the /etc/paths.d directory.  We do the latter, as (a) it's best practice and (b) it allows
         # us to explicitly read it in again later (eg on a subsequent invocation of this script to do packaging).
         #
         # The contents of the files in the /etc/paths.d directory get added to PATH by /usr/libexec/path_helper, which
         # gets run from /etc/profile.  We have some belt-and-braces code below in the Mac packaging section to read
         # /etc/paths.d/01-qtToolPaths in ourselves.
         #
         # The slight complication is that you need to be root to create a file in /etc/paths.d/, so we need to go via
         # the shell to run sudo.
         #
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'touch', '/etc/paths.d/01-qtToolPaths']))
         btUtils.abortOnRunFail(subprocess.run(['sudo', 'chmod', 'a+rw', '/etc/paths.d/01-qtToolPaths']))
         with open('/etc/paths.d/01-qtToolPaths', 'a+') as qtToolPaths:
            qtToolPaths.write(qtBinDir + '\n')
            qtToolPaths.write(qtLibDir + '\n')
         #
         # ...but, for GitHub actions, writing to the file in the GITHUB_PATH environment variable is the supported way
         # to add something to the path for subsequent steps.
         #
         if 'GITHUB_PATH' in os.environ:
            githubPathFile = os.environ['GITHUB_PATH']
            log.debug('GITHUB_PATH=' + githubPathFile)
            if githubPathFile:
               with open(githubPathFile, 'a+') as githubPaths:
                  githubPaths.write(qtBinDir + '\n')
                  githubPaths.write(qtLibDir + '\n')

         os.environ['LDFLAGS'] = '-L' + qtBaseDir + '/lib'
         os.environ['CPPFLAGS'] = '-I' + qtBaseDir + '/include'
         os.environ['PKG_CONFIG_PATH'] = qtBaseDir + 'lib/pkgconfig'

         #
         # See comment about CMAKE_PREFIX_PATH in CMakeLists.txt.  I think this is rather too soon to try to do this,
         # but it can't hurt.
         #
         # Typically, this is going to set CMAKE_PREFIX_PATH to /usr/local/opt/qt@6 for a Homebrew Qt install and
         # /opt/local/libexec/qt6 for a MacPorts one.
         #
         os.environ['CMAKE_PREFIX_PATH'] = qtBaseDir;

         #
         # NOTE: This is commented out as, per comments later in this script, we have macdeployqt create the .dmg file.
         #
         # dmgbuild is a Python package that provides a command line tool to create macOS disk images (aka .dmg
         # files) -- see https://dmgbuild.readthedocs.io/en/latest/
         #
         # Note that we install with the [badge_icons] extra so we can use the badge_icon setting (see
         # https://dmgbuild.readthedocs.io/en/latest/settings.html#badge_icon)
         #
#         btUtils.abortOnRunFail(subprocess.run(['pip3', 'install', 'dmgbuild[badge_icons]']))

         #
         # TBD: If, in future, we have further problems installing Xerces and/or Xalan-C++ from ports, the commented
         #      code here is a first stab at Plan C -- building from source ourselves.
         #
#         xalanCSourceUrl = 'https://github.com/apache/xalan-c/releases/download/Xalan-C_1_12_0/xalan_c-1.12.tar.gz'
#         log.debug('Downloading Xalan-C++ source from ' + xalanCSourceUrl)
#         btUtils.abortOnRunFail(subprocess.run([
#            'wget',
#            xalanCSourceUrl
#         ]))
#         btUtils.abortOnRunFail(subprocess.run(['tar', 'xf', 'xalan_c-1.12.tar.gz']))
#
#         os.chdir('xalan_c-1.12')
#         log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())
#         os.makedirs('build')
#         os.chdir('build')
#         log.debug('Working directory now ' + pathlib.Path.cwd().as_posix())
#         btUtils.abortOnRunFail(subprocess.run([
#            'cmake',
#            '-G',
#            'Ninja',
#            '-DCMAKE_INSTALL_PREFIX=/opt/Xalan-c',
#            '-DCMAKE_BUILD_TYPE=Release',
#            '-Dnetwork-accessor=curl',
#            '..'
#         ]))
#         log.debug('Building Xalan-C++')
#         btUtils.abortOnRunFail(subprocess.run(['ninja']))
#         log.debug('Running Xalan-C++ tests')
#         btUtils.abortOnRunFail(subprocess.run(['ctest', '-V', '-j', '8']))
#         log.debug('Installing Xalan-C++')
#         btUtils.abortOnRunFail(subprocess.run(['sudo', 'ninja', 'install']))

      case _:
         log.critical('Unrecognised platform: ' + platform.system())
         exit(1)

   #--------------------------------------------------------------------------------------------------------------------
   #------------------------------------------- Cross-platform Dependencies --------------------------------------------
   #--------------------------------------------------------------------------------------------------------------------
   #
   # We use libbacktrace from https://github.com/ianlancetaylor/libbacktrace.  It's not available as a Debian package
   # and not any more included with GCC by default.  It's not a large library so, unless and until we start using Conan,
   # the easiest approach seems to be to bring it in as a Git submodule and compile from source.
   #
   ensureSubmodulesPresent()
   log.info('Checking libbacktrace is built')
   previousWorkingDirectory = pathlib.Path.cwd().as_posix()
   backtraceDir = dir_gitSubmodules.joinpath('libbacktrace')
   os.chdir(backtraceDir)
   log.debug('Run configure and make in ' + backtraceDir.as_posix())
   #
   # We only want to configure and compile libbacktrace once, so we do it here rather than in Meson.build
   #
   # Libbacktrace uses autoconf/automake so it's relatively simple to build, but for a couple of gotchas
   #
   # Note that, although on Linux you can just invoke `./configure`, this doesn't work in the MSYS2 environment, so,
   # knowing that 'configure' is a shell script, we invoke it as such.  However, we must be careful to run it with the
   # correct shell, specifically `sh` (aka dash on Linux) rather than `bash`.  Otherwise, the Makefile it generates will
   # not work properly, and we'll end up building a library with missing symbols that gives link errors on our own
   # executables.
   #
   # (I haven't delved deeply into this but, confusingly, if you run `sh ./configure` it puts 'SHELL = /bin/bash' in the
   # Makefile, whereas, if you run `bash ./configure`, it puts the line 'SHELL = /bin/sh' in the Makefile.)
   #
   btUtils.abortOnRunFail(subprocess.run(['sh', './configure']))
   btUtils.abortOnRunFail(subprocess.run(['make']))
   os.chdir(previousWorkingDirectory)

   log.info('*** Finished checking / installing dependencies ***')
   return
