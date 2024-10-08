#
# meson.build is part of Brewtarget, and is copyright the following authors 2022-2024:
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
#

#-----------------------------------------------------------------------------------------------------------------------
#
# NB: This is now the primary way of building and packaging the software.  You can also still CMake to compile the
#     product and install it locally, but we no longer support using CMake to do packaging.
#
# STEP 1: Ensure Python is installed:
# -----------------------------------
# On Ubuntu and other Debian-based versions of Linux:
#    sudo apt install python3
#
# On Windows, in the 32-bit MSYS2 (https://www.msys2.org/) environment:
#    pacman -S --needed mingw-w64-i686-python
#    pacman -S --needed mingw-w64-i686-python-pip
# On Windows, in the 64-bit MSYS2 environment you would do the following HOWEVER NB WE HAVE NOT GOT PACKAGING WORKING
# FOR 64-BIT BUILDS YET SO THIS IS NOT SUPPORTED AND MAY REQUIRE CHANGES TO THE bt SCRIPT:
#    pacman -S --needed mingw-w64-x86_64-python
#    pacman -S --needed mingw-w64-x86_64-python-pip
#
# On a Mac with homebrew (https://brew.sh/) installed
#    brew install python@3.11
#
#
# STEP 2 (WINDOWS ONLY): Extra set-up
# -----------------------------------
# On Windows, there are a couple of extra things we need to do before running the bt script:
#
#  - For historical reasons, Linux and other platforms need to run both Python v2 (still used by some bits of
#    system) and Python v3 (eg that you installed yourself) so there are usually two corresponding Python
#    executables, python2 and python3.  On Windows there is only whatever Python you installed and it's called
#    python.exe.  To keep the shebang in the bt script working, we just make a softlink to python called python3.
#
#  - Getting Unicode input/output to work is fun.  We should already have a Unicode locale, but it seems we also
#    need to set PYTHONIOENCODING (see https://docs.python.org/3/using/cmdline.html#envvar-PYTHONIOENCODING, even
#    though it seems to imply you don't need to set it on recent versions of Python).
#
#  - The version of Pip we install above does not put it in the "right" place.  Specifically it will not be in the
#    PATH when we run bt.  The following seems to be the least hacky way around this:
#       curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
#       python get-pip.py
#       python -m pip install -U --force-reinstall pip
#    See https://stackoverflow.com/questions/48087004/installing-pip-on-msys for more discussion on this.
#
# TLDR: Here's what you need to run in the MSYS2 Mintty terminal:
#    if [[ ! -f $(dirname $(which python))/python3 ]]; then ln -s $(which python) $(dirname $(which python))/python3; fi
#    export PYTHONIOENCODING=utf8
#    curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
#    python get-pip.py
#    python -m pip install -U --force-reinstall pip
#
#
# STEP 3: Automatically install other dependencies and set up the Meson build:
# ----------------------------------------------------------------------------
# Then everything else can be installed by running:
#    ./bt setup all
#
# This will also set up the Meson build.  Amongst other things, this creates the 'mbuild' directory tree for the Meson
# build (so no clashes with a CMake build in the 'build' directory tree).
#
# Alternatively, if you decided to install all the dependencies manually, or if you need to reset the build directory
# after it got in a bad state, you can run:
#    ./bt setup
#
#
# STEP 4: Compile, test, install:
# -------------------------------
# Everything else is done from the 'mbuild' directory, so start with:
#    cd mbuild
#
# To compile:
#    meson compile
# Alternatively, to get more detailed output:
#    meson compile --verbose
#
# To run unit tests:
#    meson test
#
# Then to install:
#    meson install
# Or, on Linux, you can do:
#    sudo meson install
# which avoids the pop-up window
#
# Note that, on Linux, using `sudo meson install` sometimes creates problems with file permissions - eg
# mbuild/meson-logs/install-log.txt ends up being writable only by root and that results in an error when you run
# `bt package`, but this should be fixed in Meson 1.1.0.
#
# To build source packages (in the meson-dist subdirectory of the build directory):
#    meson dist
# This will build a .tar.xz file and create a corresponding .tar.xz.sha256sum file for it.
#
# STEP 5 (OPTIONAL): Build code documentation
# -------------------------------------------
# To build Doxygen-generated HTML code documentation (from the comments in the source files) to the mbuild/doc
# directory, use:
#    meson compile doc
# Then open the file mbuild/doc/html/index.html in a web browser.
#
# NOTE that as, at 2023-08-05, there are quite a lot of things that Doxygen is unable to parse in the code, which means
#      the generated documentation, whilst extensive, is missing quite a few classes (even though they have Doxygen
#      comments).  It would be good to fix this, but I fear it is not trivial... :-/
#
# STEP 6 (OPTIONAL): Build distributable packages
# -----------------------------------------------
# To build binary packages:
#    ../bt package             ⭐⭐⭐ This is the bit that is not yet working on all platforms ⭐⭐⭐
#
#
# Finally, note that if you want to add new build targets or change the names of existing targets, you have to run the
# following command from the same directory as this file:
#   meson --reconfigure mbuild
# See https://stackoverflow.com/questions/63329923/how-to-make-meson-scan-for-new-target for more on this.
# Alternatively, you can run 'bt setup' again.
#
#-----------------------------------------------------------------------------------------------------------------------


#=======================================================================================================================
#================================================== Project Settings ===================================================
#=======================================================================================================================
#
# We'll get an error if 'project' isn't the first call in this file
#
# Note that we need:
#   - Meson 0.56.0 or newer to use Meson's 'substring' call.
#   - Meson 0.59.0 or newer to use qt.compile_resources, qt.compile_ui and qt.compile_moc
#   - Meson 0.60.0 or newer to use + to append items to lists (aka 'list.<plus>' feature -- at least that's what the
#     warning message says if you've specified a lower minimum version of Meson)
# We would/will need:
#   - Meson 0.62.0 or newer for dep 'dl' custom lookup, but current version of Meson on Ubuntu 22.04 LTS is only 0.61.2
#
# NB: Per https://mesonbuild.com/Reference-manual_functions.html#project the default_options settings "are only used
#     when running Meson for the first time"!  So if you change any of the default_options settings you *MUST* delete
#     the entire build directory and run
#        meson setup <build dir name>
#     again to recreate the build directory and all its contained config.  Eg, if you are in the mbuild directory, you
#     need to run:
#        cd ..
#        rm -r mbuild
#        meson setup mbuild
#        cd mbuild
#        meson compile
#     Otherwise, as explained at
#     https://mesonbuild.com/FAQ.html#why-are-changes-to-default-project-options-ignored, your changes WILL HAVE NO
#     EFFECT.  TLDR this is because you can change all the settings via the command line and it would be hard to keep
#     track of where a setting had last been modified, so the default_options are only ever read once.
#        See also https://github.com/mesonbuild/meson/issues/2193 for further discussion about this.
#
# Default options (mostly ultimately controlling compiler settings):
#
#   - cpp_std         We need C++20 for std::map::contains(), C++17 or later for nested namespaces and structured
#                     bindings, and C++11 or later for lambdas.
#
#   - warning_level   3 is the highest level ie warns about the most things.  For gcc it translates to
#                        -Wall -Winvalid-pch -Wextra -Wpedantic
#                     (Prior to Meson 1.0.0, it also included -Wnon-virtual-dtor, but this was removed because "GCC devs
#                     think it's a bad idea to force projects to use it [and] it belongs in -Weffc++ rather than being
#                     lumped in with -Wall.)
#
#   - prefer_static   We want to try static linking before shared linking because it makes packaging a lot easier on
#                     Windows and Mac.   NB: This requires meson 0.63.0 or newer.  Current version of meson in Ubuntu
#                     22.04 repositories is 0.61.2.  For the moment, we do static setting on a library-by-library basis
#                     (by setting 'static : true' on all the dependency() calls.
#
#   - buildtype   For the moment at least, I'm not making a distinction between debug and release builds.  Unless we
#                 find some compelling performance etc reason to do otherwise, my instinct is to have as much diagnostic
#                 information in the build in "release" as we would in "development/debug", on the grounds that it can
#                 only help if an end user hits a core-dumping bug.
#                    Meson encourages you to use either the buildtype option or the debug and optimization options
#                 rather than setting compiler options directly.  However, this does not give us as much control as we
#                 would like over compiler flags.  Eg switching 'debug' to 'true' turns on the '-g' flag (equivalent to
#                 '-g2') on GCC, but there isn't a way via the meson  options to set '-g3' for GCC.  So, we set
#                 'buildtype=plain' and manage compiler flags directly.
#
#
project('brewtarget', 'cpp',
        version: '4.0.7',
        license: 'GPL-3.0-or-later',
        meson_version: '>=0.60.0',
        default_options : ['cpp_std=c++20',
                           'warning_level=3',
#                           'prefer_static=true',  See comment above for why this is commented-out for now
                           'buildtype=plain'])

#
# Although Meson itself is written in Python, Meson build files uses a slihgtly different syntax and have less
# functionality than Python.  See
# https://mesonbuild.com/FAQ.html#why-is-meson-not-just-a-python-module-so-i-could-code-my-build-setup-in-python and
# links therefrom for the rationale for avoiding being a full programming language.
#
# Besides some (sometimes annoying) variations in syntax, this also means that you sometimes have to do things in a
# slightly more cumbersome way than you would in a Python script.  Eg here, in regular Python, we would write:
#    capitalisedProjectName = projectName.capitalize()
# But meson.project_name() returns a Meson String, not a Python String, so there's a bit more work to do to get the same
# result.
#
projectName = meson.project_name().to_lower()
capitalisedProjectName = projectName.substring(0, 1).to_upper() + projectName.substring(1)
#
# Meson writes out a lot of useful info straight away, eg:
#
#    The Meson build system
#    Version: 1.0.1
#    Source dir: /home/runner/work/brewtarget/brewtarget
#    Build dir: /home/runner/work/brewtarget/brewtarget/mbuild
#    Build type: native build
#    Project name: brewtarget
#    Project version: 3.0.7
#    C++ compiler for the host machine: c++ (gcc 9.4.0 "c++ (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0")
#    C++ linker for the host machine: c++ ld.bfd 2.34
#    Host machine cpu family: x86_64
#    Host machine cpu: x86_64
#
# You can get this info inside the script too:
#
#    Version:                           = meson.version()
#    Source dir:                        = meson.project_source_root()
#    Build dir:                         = meson.project_build_root()
#    Build type:                        ≈ meson.can_run_host_binaries()
#    Project name:                      = meson.project_name()
#    Project version:                   = meson.project_version()
#    C++ compiler for the host machine: = compiler.get_id() and related commands
#    C++ linker for the host machine:   = compiler.get_linker_id() and related commands
#    Host machine cpu family:           = build_machine.cpu_family()
#    Host machine cpu:                  = build_machine.cpu()
#
message('⭐ Building', projectName, 'version', meson.project_version(), 'for', host_machine.system(),
        'on', build_machine.system(), '⭐')
compiler = meson.get_compiler('cpp')

#
# We need two versions of the main executable name because they are different on Windows:
#
#    mainExecutableTargetName = "target" name _without_ the '.exe' suffix, which we pass to the executable() command.
#                               On Windows, Meson will always add its own '.exe' suffix and we don't want to end up
#                               with '.exe.exe' as the suffix!
#
#    mainExecutableTargetName = The actual name of the file, including the '.exe' suffix.  We export this as
#                               CONFIG_EXECUTABLE_NAME for anywhere else that needs it (currently only the NSIS
#                               packaging scripts on Windows)
#
# (We could also obtain mainExecutableTargetName via the full_path() method on the return object from the executable()
# command, but we'd have to strip off the path.)
#
# The default (on Windows and Linux) is to use Unix-style executable names, ie all lower case
#
mainExecutableTargetName = projectName
if host_machine.system() == 'darwin'
   # On Mac we don't need a suffix but have always capitalised the executable name because "Don't question the APPLE".
   mainExecutableTargetName = capitalisedProjectName
endif
if host_machine.system() == 'windows'
   mainExecutableFileName = mainExecutableTargetName + '.exe'
else
   mainExecutableFileName = mainExecutableTargetName
endif

testRunnerTargetName = mainExecutableTargetName + '_tests'

#=======================================================================================================================
#==================================================== Meson modules ====================================================
#=======================================================================================================================
# Import the Qt tools.  See https://mesonbuild.com/Qt5-module.html
qt = import('qt5')
# File System module - see https://mesonbuild.com/Fs-module.html
fs = import('fs')

#=======================================================================================================================
#==================================================== Build options ====================================================
#=======================================================================================================================
prefix = get_option('prefix')

#=======================================================================================================================
#============================================== Frameworks and Libraries ===============================================
#=======================================================================================================================
#
# It would be neat within the build system to automate the _installation_ and upgrade of libraries and frameworks on
# which we depend.  However, I have yet to find a universal pain-free solution.
#
# Meson has its own dependency management system, Wrap, but the list of pre-provided projects at
# https://mesonbuild.com/Wrapdb-projects.html does not include Qt, Boost, Xerces, Xalan or Valijson.
#
# You can bridge out to other systems such as Conan or vcpkg, but it's a bit clunky.
#
# So, for now at least, we manage dependency installation in the `bt` Python script.
#
# Although we request static libraries in a lot of places, we don't always get them, so we assume we need to deal with
# shared libraries (aka DLLs on Windows).
#
# Aside from ensuring all dependencies are present on the build machine, we also have to worry about run-time
# dependencies for packaging.  In particular, on Windows and Mac, because there is not built-in default package manager,
# we typically need to include in our package all the non-system shared libraries on which it depends.  For the Qt
# libraries, there are handy tools `windeployqt` and `macdeployqt` that do most of the necessary work.  However, these
# tools do not (reliably) detect other shared libraries on which we depend.  The paths of these shared libraries should
# be knowable during compilation (or more specifically linking).  We want to get the paths during the build so that we
# can export them for use in the packaging step (which is done outside of Meson by the "build tool" bt Python script.
#
# Finding out how to get shared library paths information was, err, fun because it's not very well documented.
# Eventually, I realised that you can look in meson-private/cmake_XercesC/cmake_trace.txt,
# meson-private/cmake_XalanC/cmake_trace.txt and so on to see what information Meson got back from CMake and thus know
# which CMake variables are exposed via the get_variable() call.
#
sharedLibraryPaths = []

#========================================================= Qt ==========================================================
# We need not just the "core" bit of Qt but various "optional" elements.
#
# We try to keep the minimum Qt version we need as low as we can.
#
# Note that if you change the minimum Qt version, you need to make corresponding changes to the .github/workflows/*.yml
# files so that GitHub uses the appropriate version of Qt for the automated builds.
#
# 2024-05-19: Removed support for Ubuntu 20.04 because the latest version of Qt available in its packages is 5.12.8,
#             which is no longer supported (and, amongst other things, has an old version of SQLite that does not
#             support "DROP COLUMN").  Qt 5.15 is the long-term support version of Qt5.
#
# For the moment, max version we can have here is 5.15.3, because that's what Ubuntu 22.04 topped out at.  But note
# that Meson will be happy with newer versions (5.15.13 on Windows & Mac).

# Tell meson which Qt modules we need
qtCommonDependencies = dependency('qt5',
                                  version : '>=5.15.3',
                                  modules : ['Core',
                                             'Gui',          # Needed for QColor on Mac?
                                             'Multimedia',
                                             'Network',
                                             'PrintSupport',
                                             'Sql',
                                             'Svg',          # Needed to make the deploy scripts pick up the svg plugins
                                             'Widgets',
                                             'Xml'],         # TBD: Not sure we need this any more
                                  include_type : 'system',
                                  static : true)
# The Qt Gui module is only needed for the main program.  (We don't want the tests to try to load it or it could barf
# in a GitHub action that does not have a display running.)
qtMainExeDependencies = dependency('qt5', version : '>=5.15.3', modules: ['Gui'])
# The Qt Test module is only needed for the unit tests
qtTestRunnerDependencies = dependency('qt5', version : '>=5.15.3', modules: ['Test'])

#===================================================== Find Boost ======================================================
# Boost is a collection of separate libraries, some, but not all, of which are header-only.  We only specify the Boost
# libraries that we actually use.
#
# On Linux, there are cases where we need a more recent version of a Boost library than is readily-available in system-
# supplied packages.  I haven't found a slick way to solve this in CMake, though https://github.com/Orphis/boost-cmake
# looks promising.  (For header-only Boost libraries, you might think it would be relatively painless to pull them in
# from where they are hosted on GitHub (see https://github.com/boostorg), but this is not the case.  AFAICT you can't
# easily pull a specific release, and just pulling master doesn't guarantee that everything compiles.)  So, anyway, on
# Debian-based distros of Linux, such as Ubuntu, you need to do the following to install Boost 1.79 in place of whatever
# (if anything) is already installed:
#
#    $ sudo apt remove boost-all-dev
#    $ cd ~
#    $ mkdir boost-tmp
#    $ cd boost-tmp
#    $ wget https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/boost_1_79_0.tar.bz2
#    $ tar --bzip2 -xf boost_1_79_0.tar.bz2
#    $ cd boost_1_79_0
#    $ ./bootstrap.sh --prefix=/usr
#    $ sudo ./b2 install
#    $ cd ../..
#    $ sudo rm -rf boost-tmp
#
# (Obviously if you want to make the necessary change to install an even more recent version than Boost 1.79 then that
# should be fine.)
#
# We do the same in .github/workflows/linux-ubuntu.yml to make GitHub automated builds work.
#
# Note that this means we want to _statically_ link Boost rather than force end users to have to do all the palava above
#
#    ************************
#    *** Boost Stacktrace ***
#    ************************
#
#    We use this for diagnostics.  In certain error cases it's very helpful to be able to log the call stack.
#
#    On Windows, using MSYS2, the mingw-w64-boost packages do not include libboost_stacktrace_backtrace, but
#    https://www.boost.org/doc/libs/1_76_0/doc/html/stacktrace/configuration_and_build.html suggests it is not required
#    (because on Windows, if you have libbacktrace installed, you can set BOOST_STACKTRACE_USE_BACKTRACE in header-only
#    mode).
#
#    .:TODO:. Not sure how to get libboost_stacktrace_backtrace installed on Mac.  It doesn't seem to be findable by
#    CMake after installing Boost via Homebrew (https://brew.sh/).  For the moment, skip trying to use
#    libboost_stacktrace_backtrace on Mac
#
#    .:TODO:. So far don't have stacktraces working properly on Windows (everything shows as register_frame_ctor), so
#    that needs some more investigation.  (It could be that it's a bug in Boost, at least according to
#    https://stackoverflow.com/questions/54333608/boost-stacktrace-not-demangling-names-when-cross-compiled)
#
#    ******************
#    *** Boost JSON ***
#    ******************
#
#    Boost JSON is an (optionally) header-only library that was introduced in Boost 1.75 in December 2020.  One of the
#    features we use, JSON pointers (the equivalent of XML's XPaths) was only introduced in Boost 1.79.  As of March
#    2022, Ubunutu 20.04 LTS only has packages for Boost 1.71 from August 2019, hence the need to manually install a
#    newer Boost.
#
#    ******************
#    *** Boost.Core ***
#    ******************
#
#    Boost.Core, part of collection of the Boost C++ Libraries, is a collection of core utilities used by other Boost
#    libraries.  Boost JSON needs a more recent version than 1.71.
#
# For Boost, per https://mesonbuild.com/Dependencies.html#boost, we only need to supply module names for libraries we
# need to link against.  For the header-only Boost libraries, the 'boost' dependency suffices.
boostModules = []
if host_machine.system() == 'linux'
   boostModules += 'stacktrace_backtrace'
   add_global_arguments('-DBOOST_STACKTRACE_LINK', language : 'cpp')
   add_global_arguments('-DBOOST_STACKTRACE_USE_BACKTRACE', language : 'cpp')
endif
boostDependency = dependency('boost',
                             version : '>=1.79.0',
                             modules : boostModules,
                             static : true)
message('Boost:', boostDependency.name(), 'found =', boostDependency.found(), 'version =', boostDependency.version())

#
# Extra requirements for Boost Stacktrace
#
# Per https://www.boost.org/doc/libs/1_76_0/doc/html/stacktrace/configuration_and_build.html, by default
# Boost.Stacktrace is a header-only library.  However, you get better results by linking (either statically or
# dynamically) with a helper library.  Of the various options, it seems like boost_stacktrace_backtrace gives the most
# functionality over the most platforms.  This has dependencies on:
#   - libdl on POSIX platforms -- but see note below
#   - libbacktrace
# The latter is a widely-used cross-platform open source library available at
# https://github.com/ianlancetaylor/libbacktrace.  On some POSIX plaforms it's already either installed on the system
# (eg see https://man7.org/linux/man-pages/man3/backtrace.3.html) or available as an optional component of the GCC
# compiler.  However, it seems this is something that can change over time.  It's a small and stable library, so we
# just build it from sources -- which is done in the `bt` script when you run `bt setup all`.
#
# Just to make things extra fun, in 2021, the GNU compilers did away with libdl and incorporated its functionality into
# libc, per section 2.3 of release info at https://sourceware.org/glibc/wiki/Release/2.34.  This means, if we're using
# the GNU tool chain and libc is v2.34 or newer, then we should NOT look for libdl, as we won't find it!  To find the
# version of libc, you execute 'ldd --version', which gives multi-line output, of which the first line will be something
# such as:
#    ldd (Ubuntu GLIBC 2.35-0ubuntu3.1) 2.35
# In this case, that means libc is version 2.35
#
# If we _don't_ need libdl, we just create an "unrequired" dependency.  This saves having to repeat a bunch of logic
# later on when we get to the main build.
#
# On newer versions of Meson (>= 0.62.0), the dependency() command has special-case code for dealing with libdl being
# built into the compiler.  This means if we write:
#
#    dlDependency = dependency('dl', required : needLibdl, static : true)
#
# then, on Mac for instance, dlDependency.found() can be true without there being any cmake variable such as
# PACKAGE_LIBRARIES (because the library was found by Meson special case code, not by cmake).
#
# The good news is that, on Mac and Linux, there's a more elegant way of handling the dl dependency, which relies on the
# fact that opening a shared library is a POSIX operation with calls declared in a POSIX-defined header (see eg
# https://man7.org/linux/man-pages/man0/dlfcn.h.0p.html).  We just ask the compiler whether it has dlopen() built-in,
# and, if not, please can it tell us where to find it!
#
# (On Windows, we don't need the dl dependency because Windows doesn't have the POSIX interface for shared library
# opening, and Boost Stacktrace doesn't seem to need anything in its place.)
#
dlDependency = declare_dependency()
if host_machine.system() != 'windows'
   if not compiler.has_function('dlopen', prefix: '#include <dlfcn.h>')
      dlDependency = compiler.find_library('dl', has_headers: 'dlfcn.h')
   endif
endif

# Note that, unlike, say, the parameters to include_directories(), the dirs argument to find_library() must be absolute
# paths
libbacktraceDir = join_paths(meson.project_source_root(), 'third-party/libbacktrace/.libs')
backtraceDependency = compiler.find_library('backtrace',
                                            required : true,
                                            static : true,
                                            dirs : [libbacktraceDir])

#======================================== Find the other libraries we depend on ========================================
#
# See https://mesonbuild.com/Reference-manual_returned_dep.html for what info we can pull from a dependency object
#
# For BeerXML processing we need Xerces-C++ and Xalan-C++.  Meson can find both of these automatically in a couple of
# different ways.  The official way is to use `dependency('xerces-c')` and `dependency('xalan-c')`.  However, I don't
# think this gives us access to the paths of the libraries (which we want to export for the packaging scripts -- see
# various comments about shared libraries in the `bt` build tool script).  Instead, we can have Meson use CMake's
# find_package(), as long as (a) CMake is installed(!) and (b) we provide the right library names ('XercesC' per
# https://cmake.org/cmake/help/latest/module/FindXercesC.html and 'XalanC' per
# https://cmake.org/cmake/help/latest/module/FindXalanC.html)
#
xercesDependency = dependency('XercesC',
                              version : '>=3.2.2',
                              required : true,
                              static : true)
xercesLibPaths = xercesDependency.get_variable(cmake : 'PACKAGE_LIBRARIES')
message('Xerces Library:', xercesDependency.name(), 'found =', xercesDependency.found(),
        'version =', xercesDependency.version(), 'path(s)=', xercesLibPaths)
sharedLibraryPaths += xercesLibPaths
xalanDependency = dependency('XalanC',
                             version : '>=1.11.0',
                             required : true,
                             static : true)
xalanLibPaths = xalanDependency.get_variable(cmake : 'PACKAGE_LIBRARIES')
message('Xalan Library:', xalanDependency.name(), 'found =', xalanDependency.found(),
        'version =', xalanDependency.version(), 'path(s)=', xalanLibPaths)
sharedLibraryPaths += xalanLibPaths

#====================================================== Valijson =======================================================
# Don't need to do anything special, other than set include directories below, as it's header-only and we pull it in as
# a Git submodule.

#==================================================== Other headers ====================================================
# Other directories to search in for headers.  Meson will barf an error if any of these directories does not exist.
includeDirs = include_directories('src', 'third-party/valijson/include', 'third-party/libbacktrace')


#=======================================================================================================================
#============================================= Extra Windows dependencies ==============================================
#=======================================================================================================================
# .:TBD:. Don't think we need this bit any more
if host_machine.system() == 'windows'
   #
   # We can't assume that the person running the code will have MSYS2/MinGW installed, so we need to include the DLLs
   # that ship with them and get pulled in by the packaging process.  There is a bit of trial-and-error in compiling
   # this list, but, mostly, if you miss a needed DLL from the package, Windows will give you an error message at
   # start-up telling you which DLL(s) it needed but could not find.
   #
   foreach extraLib : ['gcc',
                       'winpthread',
                       'stdc++',
                       'xalanMsg']
      extraLibDependency = compiler.find_library(extraLib, required : true)
      if extraLibDependency.found()
#         extraLibPath = extraLibDependency.get_variable(cmake : 'LIB_ARCH_LIST')
#         message(extraLib, ' found at', extraLibPath)
#         sharedLibraryPaths += extraLibPath
      else
#         message(extraLib, ' not found')
      endif
   endforeach
endif

#=======================================================================================================================
#============================================== Extra Apple dependencies ===============================================
#=======================================================================================================================
if host_machine.system() == 'darwin'
   # Statically linking Xalan, Xerces etc requires CFStringLowercase, CFStringUppercase, etc on Mac
   corefoundationDependency = dependency(
      'appleframeworks',
      modules: ['CoreFoundation'],
      required: false,
   )
endif

#=======================================================================================================================
#===================================================== Input Files =====================================================
#=======================================================================================================================
# Sub-directories of the one containing this (meson.build) file are
#    src          = C++ source code
#    ui           = QML UI layout files
#    data         = Binary files, including sounds and default database
#    translations = Translated texts
#    mac          = Mac-specific files (desktop icon)
#    win          = Windows-specific files (desktop icon)
#    packaging    = Packaging-related config
#=======================================================================================================================

#
# List of the C++ source files that are common to the app and the unit tests - ie all .cpp files _except_ main.cpp and
# test.cpp
#
# See https://mesonbuild.com/FAQ.html#why-cant-i-specify-target-files-with-a-wildcard for why it is strongly recommended
# not to use wildcard specification.  (This is common to many build systems.)
#
# You can recreate the body of this list by running the following from the bash prompt in the mbuild directory:
#    find ../src -name '*.cpp' | grep -v 'src/unitTests/' | grep -v '/main.cpp$' | sed "s+^../+   \'+; s+$+\',+" | LC_ALL=C sort
#
# (See https://stackoverflow.com/questions/68319427/bash-sort-command-do-not-treat-dots for why LC_ALL=C is needed!)
#
# The files() wrapper around the array ensures that all the files exist and means you don't have to worry as much about
# subdirectories as you might otherwise -- see https://mesonbuild.com/Reference-manual_functions.html#files
#
commonSourceFiles = files([
   'src/AboutDialog.cpp',
   'src/AlcoholTool.cpp',
   'src/Algorithms.cpp',
   'src/AncestorDialog.cpp',
   'src/Application.cpp',
   'src/BeerColorWidget.cpp',
   'src/BrewDayFormatter.cpp',
   'src/BrewDayScrollWidget.cpp',
   'src/BrewNoteWidget.cpp',
   'src/BtColor.cpp',
   'src/BtDatePopup.cpp',
   'src/BtFieldType.cpp',
   'src/BtHorizontalTabs.cpp',
   'src/BtSplashScreen.cpp',
   'src/BtTabWidget.cpp',
   'src/BtTextEdit.cpp',
   'src/ConverterTool.cpp',
   'src/HeatCalculations.cpp',
   'src/HelpDialog.cpp',
   'src/Html.cpp',
   'src/HydrometerTool.cpp',
   'src/IbuGuSlider.cpp',
   'src/InstructionWidget.cpp',
   'src/InventoryFormatter.cpp',
   'src/Localization.cpp',
   'src/Logging.cpp',
   'src/MainWindow.cpp',
   'src/MashDesigner.cpp',
   'src/MashWizard.cpp',
   'src/NamedEntitySortProxyModel.cpp',
   'src/OgAdjuster.cpp',
   'src/OptionDialog.cpp',
   'src/PersistentSettings.cpp',
   'src/PitchDialog.cpp',
   'src/PrimingDialog.cpp',
   'src/PrintAndPreviewDialog.cpp',
   'src/RadarChart.cpp',
   'src/RangedSlider.cpp',
   'src/RecipeExtrasWidget.cpp',
   'src/RecipeFormatter.cpp',
   'src/RefractoDialog.cpp',
   'src/ScaleRecipeTool.cpp',
   'src/StrikeWaterDialog.cpp',
   'src/StyleRangeWidget.cpp',
   'src/TimerListDialog.cpp',
   'src/TimerMainDialog.cpp',
   'src/TimerWidget.cpp',
   'src/WaterDialog.cpp',
   'src/boiltime.cpp',
   'src/buttons/BoilButton.cpp',
   'src/buttons/EquipmentButton.cpp',
   'src/buttons/FermentationButton.cpp',
   'src/buttons/MashButton.cpp',
   'src/buttons/RecipeAttributeButton.cpp',
   'src/buttons/StyleButton.cpp',
   'src/buttons/WaterButton.cpp',
   'src/catalogs/EquipmentCatalog.cpp',
   'src/catalogs/FermentableCatalog.cpp',
   'src/catalogs/HopCatalog.cpp',
   'src/catalogs/MiscCatalog.cpp',
   'src/catalogs/StyleCatalog.cpp',
   'src/catalogs/YeastCatalog.cpp',
   'src/database/BtSqlQuery.cpp',
   'src/database/Database.cpp',
   'src/database/DatabaseSchemaHelper.cpp',
   'src/database/DbTransaction.cpp',
   'src/database/DefaultContentLoader.cpp',
   'src/database/ObjectStore.cpp',
   'src/database/ObjectStoreTyped.cpp',
   'src/editors/BoilEditor.cpp',
   'src/editors/BoilStepEditor.cpp',
   'src/editors/EquipmentEditor.cpp',
   'src/editors/FermentableEditor.cpp',
   'src/editors/FermentationEditor.cpp',
   'src/editors/FermentationStepEditor.cpp',
   'src/editors/HopEditor.cpp',
   'src/editors/MashEditor.cpp',
   'src/editors/MashStepEditor.cpp',
   'src/editors/MiscEditor.cpp',
   'src/editors/NamedMashEditor.cpp',
   'src/editors/StyleEditor.cpp',
   'src/editors/WaterEditor.cpp',
   'src/editors/YeastEditor.cpp',
   'src/listModels/EquipmentListModel.cpp',
   'src/listModels/FermentableListModel.cpp',
   'src/listModels/HopListModel.cpp',
   'src/listModels/MashListModel.cpp',
   'src/listModels/MashStepListModel.cpp',
   'src/listModels/MiscListModel.cpp',
   'src/listModels/RecipeAdditionFermentableListModel.cpp',
   'src/listModels/RecipeAdditionHopListModel.cpp',
   'src/listModels/RecipeAdditionMiscListModel.cpp',
   'src/listModels/RecipeAdditionYeastListModel.cpp',
   'src/listModels/RecipeAdjustmentSaltListModel.cpp',
   'src/listModels/SaltListModel.cpp',
   'src/listModels/StyleListModel.cpp',
   'src/listModels/WaterListModel.cpp',
   'src/listModels/YeastListModel.cpp',
   'src/measurement/Amount.cpp',
   'src/measurement/ColorMethods.cpp',
   'src/measurement/IbuMethods.cpp',
   'src/measurement/Measurement.cpp',
   'src/measurement/PhysicalQuantity.cpp',
   'src/measurement/SucroseConversion.cpp',
   'src/measurement/SystemOfMeasurement.cpp',
   'src/measurement/Unit.cpp',
   'src/measurement/UnitSystem.cpp',
   'src/model/Boil.cpp',
   'src/model/BoilStep.cpp',
   'src/model/BrewNote.cpp',
   'src/model/Equipment.cpp',
   'src/model/Fermentable.cpp',
   'src/model/Fermentation.cpp',
   'src/model/FermentationStep.cpp',
   'src/model/Folder.cpp',
   'src/model/Hop.cpp',
   'src/model/Ingredient.cpp',
   'src/model/IngredientInRecipe.cpp',
   'src/model/Instruction.cpp',
   'src/model/Inventory.cpp',               # NB: No model/InventoryFermentable.cpp, model/InventoryHop.cpp
   'src/model/Mash.cpp',
   'src/model/MashStep.cpp',
   'src/model/Misc.cpp',
   'src/model/NamedEntity.cpp',
   'src/model/NamedParameterBundle.cpp',
   'src/model/OutlineableNamedEntity.cpp',
   'src/model/OwnedByRecipe.cpp',
   'src/model/Recipe.cpp',
   'src/model/RecipeAddition.cpp',
   'src/model/RecipeAdditionFermentable.cpp',
   'src/model/RecipeAdditionHop.cpp',
   'src/model/RecipeAdditionMisc.cpp',
   'src/model/RecipeAdditionYeast.cpp',
   'src/model/RecipeAdjustmentSalt.cpp',
   'src/model/RecipeUseOfWater.cpp',
   'src/model/Salt.cpp',
   'src/model/Step.cpp',
   'src/model/StepExtended.cpp',
   'src/model/Style.cpp',
   'src/model/Water.cpp',
   'src/model/Yeast.cpp',
   'src/serialization/ImportExport.cpp',
   'src/serialization/SerializationRecord.cpp',
   'src/serialization/json/BeerJson.cpp',
   'src/serialization/json/JsonCoding.cpp',
   'src/serialization/json/JsonMeasureableUnitsMapping.cpp',
   'src/serialization/json/JsonRecord.cpp',
   'src/serialization/json/JsonRecordDefinition.cpp',
   'src/serialization/json/JsonSchema.cpp',
   'src/serialization/json/JsonUtils.cpp',
   'src/serialization/json/JsonXPath.cpp',
   'src/serialization/xml/BeerXml.cpp',
   'src/serialization/xml/BtDomErrorHandler.cpp',
   'src/serialization/xml/XercesHelpers.cpp',
   'src/serialization/xml/XmlCoding.cpp',
   'src/serialization/xml/XmlMashRecord.cpp',
   'src/serialization/xml/XmlMashStepRecord.cpp',
   'src/serialization/xml/XmlRecipeRecord.cpp',
   'src/serialization/xml/XmlRecord.cpp',
   'src/serialization/xml/XmlRecordDefinition.cpp',
   'src/sortFilterProxyModels/EquipmentSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/FermentableSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/HopSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/MiscSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/RecipeAdditionFermentableSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/RecipeAdditionHopSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/RecipeAdditionMiscSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/RecipeAdditionYeastSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/StyleSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/WaterSortFilterProxyModel.cpp',
   'src/sortFilterProxyModels/YeastSortFilterProxyModel.cpp',
   'src/tableModels/BoilStepTableModel.cpp',
   'src/tableModels/BtTableModel.cpp',
   'src/tableModels/EquipmentTableModel.cpp',
   'src/tableModels/FermentableTableModel.cpp',
   'src/tableModels/FermentationStepTableModel.cpp',
   'src/tableModels/HopTableModel.cpp',
   'src/tableModels/MashStepTableModel.cpp',
   'src/tableModels/MiscTableModel.cpp',
   'src/tableModels/RecipeAdditionFermentableTableModel.cpp',
   'src/tableModels/RecipeAdditionHopTableModel.cpp',
   'src/tableModels/RecipeAdditionMiscTableModel.cpp',
   'src/tableModels/RecipeAdditionYeastTableModel.cpp',
   'src/tableModels/RecipeAdjustmentSaltTableModel.cpp',
   'src/tableModels/SaltTableModel.cpp',
   'src/tableModels/StyleTableModel.cpp',
   'src/tableModels/WaterTableModel.cpp',
   'src/tableModels/YeastTableModel.cpp',
   'src/trees/TreeFilterProxyModel.cpp',
   'src/trees/TreeModel.cpp',
   'src/trees/TreeNode.cpp',
   'src/trees/TreeView.cpp',
   'src/undoRedo/SimpleUndoableUpdate.cpp',
   'src/utils/BtException.cpp',
   'src/utils/BtStringConst.cpp',
   'src/utils/BtStringStream.cpp',
   'src/utils/EnumStringMapping.cpp',
   'src/utils/FileSystemHelpers.cpp',
   'src/utils/Fonts.cpp',
   'src/utils/FuzzyCompare.cpp',
   'src/utils/ImportRecordCount.cpp',
   'src/utils/MetaTypes.cpp',
   'src/utils/OStreamWriterForQFile.cpp',
   'src/utils/OptionalHelpers.cpp',
   'src/utils/PropertyPath.cpp',
   'src/utils/TimerUtils.cpp',
   'src/utils/TypeLookup.cpp',
   'src/widgets/Animator.cpp',
   'src/widgets/BtBoolComboBox.cpp',
   'src/widgets/BtComboBox.cpp',
   'src/widgets/BtOptionalDateEdit.cpp',
   'src/widgets/CustomComboBox.cpp',
   'src/widgets/InfoButton.cpp',
   'src/widgets/InfoText.cpp',
   'src/widgets/MashComboBox.cpp',
   'src/widgets/SelectionControl.cpp',
   'src/widgets/SmartAmountSettings.cpp',
   'src/widgets/SmartAmounts.cpp',
   'src/widgets/SmartDigitWidget.cpp',
   'src/widgets/SmartField.cpp',
   'src/widgets/SmartLabel.cpp',
   'src/widgets/SmartLineEdit.cpp',
   'src/widgets/ToggleSwitch.cpp',
   'src/widgets/UnitAndScalePopUpMenu.cpp',
])

applicationMainSourceFile = files([
   'src/main.cpp'
])

unitTestMainSourceFile = files([
   'src/unitTests/Testing.cpp'
])

#
# These are the headers that need to be processed by the Qt Meta Object Compiler (MOC).  Note that this is _not_ all the
# headers in the project.  Also, note that there is a separate (trivial) list of MOC headers for the unit test runner.
#
# You can recreate the body of this list by running the following from the bash prompt in the mbuild directory:
#    grep -rl '^ *Q_OBJECT' ../src | grep -v Testing.h | LC_ALL=C sort | sed "s+^../src/+   \'src/+; s/$/\',/"
#
# (See https://stackoverflow.com/questions/68319427/bash-sort-command-do-not-treat-dots for why LC_ALL=C is needed!)
#
mocHeaders = files([
   'src/AboutDialog.h',
   'src/AlcoholTool.h',
   'src/AncestorDialog.h',
   'src/BeerColorWidget.h',
   'src/BrewDayFormatter.h',
   'src/BrewDayScrollWidget.h',
   'src/BrewNoteWidget.h',
   'src/BtDatePopup.h',
   'src/BtSplashScreen.h',
   'src/BtTabWidget.h',
   'src/BtTextEdit.h',
   'src/ConverterTool.h',
   'src/HelpDialog.h',
   'src/HydrometerTool.h',
   'src/IbuGuSlider.h',
   'src/InstructionWidget.h',
   'src/MainWindow.h',
   'src/MashDesigner.h',
   'src/MashWizard.h',
   'src/NamedEntitySortProxyModel.h',
   'src/OgAdjuster.h',
   'src/OptionDialog.h',
   'src/PitchDialog.h',
   'src/PrimingDialog.h',
   'src/PrintAndPreviewDialog.h',
   'src/RangedSlider.h',
   'src/RecipeExtrasWidget.h',
   'src/RecipeFormatter.h',
   'src/RefractoDialog.h',
   'src/ScaleRecipeTool.h',
   'src/StrikeWaterDialog.h',
   'src/StyleRangeWidget.h',
   'src/TimerListDialog.h',
   'src/TimerMainDialog.h',
   'src/TimerWidget.h',
   'src/WaterDialog.h',
   'src/boiltime.h',
   'src/buttons/BoilButton.h',
   'src/buttons/EquipmentButton.h',
   'src/buttons/FermentationButton.h',
   'src/buttons/MashButton.h',
   'src/buttons/RecipeAttributeButton.h',
   'src/buttons/StyleButton.h',
   'src/buttons/WaterButton.h',
   'src/catalogs/EquipmentCatalog.h',
   'src/catalogs/FermentableCatalog.h',
   'src/catalogs/HopCatalog.h',
   'src/catalogs/MiscCatalog.h',
   'src/catalogs/StyleCatalog.h',
   'src/catalogs/YeastCatalog.h',
   'src/database/ObjectStore.h',
   'src/editors/BoilEditor.h',
   'src/editors/BoilStepEditor.h',
   'src/editors/EquipmentEditor.h',
   'src/editors/FermentableEditor.h',
   'src/editors/FermentationEditor.h',
   'src/editors/FermentationStepEditor.h',
   'src/editors/HopEditor.h',
   'src/editors/MashEditor.h',
   'src/editors/MashStepEditor.h',
   'src/editors/MiscEditor.h',
   'src/editors/NamedMashEditor.h',
   'src/editors/StyleEditor.h',
   'src/editors/WaterEditor.h',
   'src/editors/YeastEditor.h',
   'src/listModels/EquipmentListModel.h',
   'src/listModels/FermentableListModel.h',
   'src/listModels/HopListModel.h',
   'src/listModels/MashListModel.h',
   'src/listModels/MashStepListModel.h',
   'src/listModels/MiscListModel.h',
   'src/listModels/RecipeAdditionFermentableListModel.h',
   'src/listModels/RecipeAdditionHopListModel.h',
   'src/listModels/RecipeAdditionMiscListModel.h',
   'src/listModels/RecipeAdditionYeastListModel.h',
   'src/listModels/RecipeAdjustmentSaltListModel.h',
   'src/listModels/SaltListModel.h',
   'src/listModels/StyleListModel.h',
   'src/listModels/WaterListModel.h',
   'src/listModels/YeastListModel.h',
   'src/model/Boil.h',
   'src/model/BoilStep.h',
   'src/model/BrewNote.h',
   'src/model/Equipment.h',
   'src/model/Fermentable.h',
   'src/model/Fermentation.h',
   'src/model/FermentationStep.h',
   'src/model/Folder.h',
   'src/model/Hop.h',
   'src/model/Ingredient.h',
   'src/model/IngredientInRecipe.h',
   'src/model/Instruction.h',
   'src/model/Inventory.h',
   'src/model/InventoryFermentable.h',  # NB: Function definitions in model/Inventory.cpp
   'src/model/InventoryHop.h',          # NB: Function definitions in model/Inventory.cpp
   'src/model/InventoryMisc.h',         # NB: Function definitions in model/Inventory.cpp
   'src/model/InventorySalt.h',         # NB: Function definitions in model/Inventory.cpp
   'src/model/InventoryYeast.h',        # NB: Function definitions in model/Inventory.cpp
   'src/model/Mash.h',
   'src/model/MashStep.h',
   'src/model/Misc.h',
   'src/model/NamedEntity.h',
   'src/model/OutlineableNamedEntity.h',
   'src/model/OwnedByRecipe.h',
   'src/model/Recipe.h',
   'src/model/RecipeAddition.h',
   'src/model/RecipeAdditionFermentable.h',
   'src/model/RecipeAdditionHop.h',
   'src/model/RecipeAdditionMisc.h',
   'src/model/RecipeAdditionYeast.h',
   'src/model/RecipeAdjustmentSalt.h',
   'src/model/RecipeUseOfWater.h',
   'src/model/Salt.h',
   'src/model/Step.h',
   'src/model/StepExtended.h',
   'src/model/Style.h',
   'src/model/Water.h',
   'src/model/Yeast.h',
   'src/sortFilterProxyModels/EquipmentSortFilterProxyModel.h',
   'src/sortFilterProxyModels/FermentableSortFilterProxyModel.h',
   'src/sortFilterProxyModels/HopSortFilterProxyModel.h',
   'src/sortFilterProxyModels/MiscSortFilterProxyModel.h',
   'src/sortFilterProxyModels/RecipeAdditionFermentableSortFilterProxyModel.h',
   'src/sortFilterProxyModels/RecipeAdditionHopSortFilterProxyModel.h',
   'src/sortFilterProxyModels/RecipeAdditionMiscSortFilterProxyModel.h',
   'src/sortFilterProxyModels/RecipeAdditionYeastSortFilterProxyModel.h',
   'src/sortFilterProxyModels/StyleSortFilterProxyModel.h',
   'src/sortFilterProxyModels/WaterSortFilterProxyModel.h',
   'src/sortFilterProxyModels/YeastSortFilterProxyModel.h',
   'src/tableModels/BoilStepTableModel.h',
   'src/tableModels/BtTableModel.h',
   'src/tableModels/EquipmentTableModel.h',
   'src/tableModels/FermentableTableModel.h',
   'src/tableModels/FermentationStepTableModel.h',
   'src/tableModels/HopTableModel.h',
   'src/tableModels/MashStepTableModel.h',
   'src/tableModels/MiscTableModel.h',
   'src/tableModels/RecipeAdditionFermentableTableModel.h',
   'src/tableModels/RecipeAdditionHopTableModel.h',
   'src/tableModels/RecipeAdditionMiscTableModel.h',
   'src/tableModels/RecipeAdditionYeastTableModel.h',
   'src/tableModels/RecipeAdjustmentSaltTableModel.h',
   'src/tableModels/SaltTableModel.h',
   'src/tableModels/StyleTableModel.h',
   'src/tableModels/WaterTableModel.h',
   'src/tableModels/YeastTableModel.h',
   'src/trees/TreeFilterProxyModel.h',
   'src/trees/TreeModel.h',
   'src/trees/TreeView.h',
   'src/widgets/Animator.h',
   'src/widgets/BtBoolComboBox.h',
   'src/widgets/BtComboBox.h',
   'src/widgets/BtOptionalDateEdit.h',
   'src/widgets/CustomComboBox.h',
   'src/widgets/InfoButton.h',
   'src/widgets/InfoText.h',
   'src/widgets/SelectionControl.h',
   'src/widgets/SmartDigitWidget.h',
   'src/widgets/SmartLabel.h',
   'src/widgets/SmartLineEdit.h',
   'src/widgets/ToggleSwitch.h',
])

unitTestMocHeaders = files([
   'src/unitTests/Testing.h'
])

#
# List of UI files
#
# You can recreate the body of this list by running the following from the bash prompt in the mbuild directory:
#    find ../ui -name '*.ui' | sort | sed "s+^../ui/+   \'ui/+; s/$/\',/"
#
uiFiles = files([
   'ui/ancestorDialog.ui',
   'ui/boilEditor.ui',
   'ui/boilStepEditor.ui',
   'ui/brewDayScrollWidget.ui',
   'ui/brewNoteWidget.ui',
   'ui/BtPrintAndPreview.ui',
   'ui/equipmentEditor.ui',
   'ui/fermentableEditor.ui',
   'ui/fermentationEditor.ui',
   'ui/fermentationStepEditor.ui',
   'ui/hopEditor.ui',
   'ui/instructionWidget.ui',
   'ui/mainWindow.ui',
   'ui/mashDesigner.ui',
   'ui/mashEditor.ui',
   'ui/mashStepEditor.ui',
   'ui/mashWizard.ui',
   'ui/miscEditor.ui',
   'ui/namedMashEditor.ui',
   'ui/ogAdjuster.ui',
   'ui/optionsDialog.ui',
   'ui/pitchDialog.ui',
   'ui/primingDialog.ui',
   'ui/recipeExtrasWidget.ui',
   'ui/refractoDialog.ui',
   'ui/strikeWaterDialog.ui',
   'ui/styleEditor.ui',
   'ui/timerDialog.ui',
   'ui/timerListDialog.ui',
   'ui/timerMainDialog.ui',
   'ui/timerWidget.ui',
   'ui/waterDialog.ui',
   'ui/waterEditor.ui',
   'ui/yeastEditor.ui',
])

#
# List of translation files to update (from translatable strings in the source code) and from which the binary .qm files
# will be generated and shipped.  Note that src/OptionDialog.cpp controls which languages are shown to the user as
# options for the UI
#
# .:TBD:. At the moment we are hitting a warning message similar to the one described at
# https://github.com/mesonbuild/meson/issues/5019.  I _think_ this is a minor Meson bug, but it might be that I've
# misunderstood how best to reference files in subdirectories.
#
translationSourceFiles = files([
   'translations/bt_ca.ts', # Catalan
   'translations/bt_cs.ts', # Czech
   'translations/bt_de.ts', # German
   'translations/bt_en.ts', # English
   'translations/bt_el.ts', # Greek
   'translations/bt_es.ts', # Spanish
   'translations/bt_et.ts', # Estonian
   'translations/bt_eu.ts', # Basque
   'translations/bt_fr.ts', # French
   'translations/bt_gl.ts', # Galician
   'translations/bt_nb.ts', # Norwegian Bokmal
   'translations/bt_it.ts', # Italian
   'translations/bt_lv.ts', # Latvian
   'translations/bt_nl.ts', # Dutch
   'translations/bt_pl.ts', # Polish
   'translations/bt_pt.ts', # Portuguese
   'translations/bt_hu.ts', # Hungarian
   'translations/bt_ru.ts', # Russian
   'translations/bt_sr.ts', # Serbian
   'translations/bt_sv.ts', # Swedish
   'translations/bt_tr.ts', # Turkish
   'translations/bt_zh.ts', # Chinese
])

# List of documentation files to be installed.  Note that ${repoDir}/COPYRIGHT is NOT included here as it needs special
# case handling below.

filesToInstall_docs = files([
  'README.md'
])

filesToInstall_data = files([
   'data/default_db.sqlite',
   'data/DefaultContent001-OriginalDefaultData.xml',
   'data/DefaultContent002-BJCP_2021_Styles.json',
   'data/DefaultContent003-Ingredients-Hops-Yeasts.json',
   'data/DefaultContent004-MoreYeasts.json'
])

filesToInstall_desktop = files([
   'linux/' + projectName + '.desktop'
])

filesToInstall_icons = files([
   'images/' + projectName + '.svg'
])

filesToInstall_windowsIcon = files([
   'win/icon.rc'
])

filesToInstall_sounds = files([
   'data/sounds/45minLeft.wav',
   'data/sounds/addFuckinHops.wav',
   'data/sounds/aromaHops.wav',
   'data/sounds/beep.wav',
   'data/sounds/bitteringHops.wav',
   'data/sounds/checkBoil.wav',
   'data/sounds/checkFirstRunnings.wav',
   'data/sounds/checkGravity.wav',
   'data/sounds/checkHydrometer.wav',
   'data/sounds/checkMashTemps.wav',
   'data/sounds/checkTemp.wav',
   'data/sounds/clarifyingAgent.wav',
   'data/sounds/cleanup.wav',
   'data/sounds/closeFuckinValves.wav',
   'data/sounds/closeValves.wav',
   'data/sounds/doughIn.wav',
   'data/sounds/drinkAnotherHomebrew.wav',
   'data/sounds/drinkHomebrew.wav',
   'data/sounds/emptyMashTun.wav',
   'data/sounds/extraPropane.wav',
   'data/sounds/flameout.wav',
   'data/sounds/flavorHops.wav',
   'data/sounds/heatWater.wav',
   'data/sounds/mashHops.wav',
   'data/sounds/pitchYeast.wav',
   'data/sounds/sanitize.wav',
   'data/sounds/sparge.wav',
   'data/sounds/startBurner.wav',
   'data/sounds/startChill.wav',
   'data/sounds/stirMash.wav',
])

filesToInstall_macPropertyList = files([
   'mac/Info.plist'
])

filesToInstall_macIcons = files([
   'mac/' + capitalisedProjectName + 'Icon.icns'
])

# This has to be a string because we're going to pass it into a script.
# AFAICT Meson does not provide a way for you to extract, say, full path from a file object
filesToInstall_changeLogUncompressed = 'CHANGES.markdown'

# Summary copyright file, with names of all authors
filesToInstall_copyright = files([
   'COPYRIGHT'
])

#
# GPL v3 Licence
#
# See https://www.gnu.org/licenses/translations.html for why this is only in English
#
# TBD: We have two files "COPYING.GPLv3" and "LICENSE" with identical content.  I wonder if we can do away with one of
# them
# We cannot wrap this in a files() call because we need to be able to pass the name into join_paths below
#
filesToInstall_license = 'LICENSE'

#=======================================================================================================================
#============================================ Installation sub-directories =============================================
#=======================================================================================================================
# .:TBD:. We don't currently use installSubDir_bin, instead letting Meson decide where to put the binary
if host_machine.system() == 'linux'
   #============================================= Linux Install Directories ============================================
   installSubDir_data = 'share/' + projectName
   installSubDir_doc  = 'share/doc/' + projectName
   installSubDir_bin  = 'bin'
   # According to https://specifications.freedesktop.org/menu-spec/menu-spec-1.0.html#paths, .desktop files need to live
   # in one of the $XDG_DATA_DIRS/applications/.  (Note that $XDG_DATA_DIRS is a colon-separated list of directories,
   # typically defaulting to /usr/local/share/:/usr/share/. but on another system it might be
   # /usr/share/plasma:/usr/local/share:/usr/share:/var/lib/snapd/desktop:/var/lib/snapd/desktop).  When combined with
   # CMAKE_INSTALL_PREFIX, "share/applications" should end up being one of these.
   installSubDir_applications = 'share/applications'
   # It's a similar but slightly more complicated story for where to put icons.  (See
   # https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html#directory_layout for all the
   # details.)
   installSubDir_icons        = 'share/icons'
elif host_machine.system() == 'windows'
   #============================================ Windows Install Directories ===========================================
   installSubDir_data = 'data'
   installSubDir_doc  = 'doc'
   installSubDir_bin  = 'bin'
elif host_machine.system() == 'darwin'
   #============================================== Mac Install Directories =============================================
   installSubDir_data = 'Contents/Resources'
   installSubDir_doc  = 'Contents/Resources/en.lproj'
   installSubDir_bin  = 'Contents/MacOS'
else
   error('Unrecognised target OS type:', host_machine.system())
endif

#============================================== Common Install Directories =============================================
installSubDir_translations = installSubDir_data + '/translations_qm'


#=======================================================================================================================
#=========================================== Qt Meta Object Compilation etc ============================================
#=======================================================================================================================

# Compile Qt's resources collection files (.qrc) into C++ files for compilation
generatedFromQrc = qt.compile_resources(sources : 'resources.qrc')

# Compile Qt's ui files (.ui) into header files.
generatedFromUi = qt.compile_ui(sources : uiFiles)

# Compile Qt's moc files (.moc) into header and/or source files
generatedFromMoc = qt.compile_moc(headers : mocHeaders,
                                  dependencies : qtCommonDependencies)
generatedFromMocForUnitTests = qt.compile_moc(headers : unitTestMocHeaders,
                                              dependencies : qtCommonDependencies)

#
# We need to do two processes with Translation Source (.ts) XML files:
#   - Update them from the source code, ie to ensure they have all the tr(), QObject::tr() etc calls from the .cpp files
#     and all the translatable strings from the .ui files -- which can be done manually from the command line with
#     lupdate
#   - Generate the binary .qm files that ship with the application and are used at run time -- which can be done
#     manually from the command line with lrelease
# Calling qt.compile_translations will do only the latter, so we need to do the former directly
#

# Call lupdate to ensure the .ts files are synced with the source code.  We need:
#    lupdate meson.project_source_root()/src meson.project_source_root()/ui -ts [list of .ts files]
# This tells lupdate to recursively scan the src/ and ui/ subdirectories and update the specified ts files
# Fortunately, we can pass a list of File objects as a parameter to run_command and Meson does the right thing
#
# We make a point here of displaying the output of run_command because we want to show message emitted by lupdate about
# what it did.
message('Running lupdate on the following ts files:', run_command('ls', translationSourceFiles, check: true).stdout())
message(
   run_command('lupdate',
               meson.project_source_root() + '/src',
               meson.project_source_root() + '/ui',
               '-ts',
               translationSourceFiles,
               check: true).stdout()
)

# Now we can generate the necessary targets to build translation files with lrelease
# Setting install to true means we want to ship all the .qm files (so users can change language at run time).
translations = qt.compile_translations(ts_files         : translationSourceFiles,
                                       build_by_default : true,
                                       install          : true,
                                       install_dir      : installSubDir_translations)

#=======================================================================================================================
#=============================================== Lists of Dependencies =================================================
#=======================================================================================================================
commonDependencies = [qtCommonDependencies,
                      xercesDependency,
                      xalanDependency,
                      boostDependency,
                      dlDependency,
                      backtraceDependency]
mainExeDependencies = commonDependencies + qtMainExeDependencies
testRunnerDependencies = commonDependencies + qtTestRunnerDependencies

#=======================================================================================================================
#================================================= Exported variables ==================================================
#=======================================================================================================================
#
# There are a number of places where we want to "export" variables from this file.  In a couple of instances we are
# generating a file (using configure_file()) and in other places we are running a shell script (with run_command() or
# run_target()).  Although it's not always the exact same set of variables that we need to export, there is, I think,
# enough overlap that it's worth defining all the exports once to avoid repeating ourselves.
#
# The file generation and script execution take different types of object for their "exported variables": a
# Configuration Data (cfg_data) object (see https://mesonbuild.com/Reference-manual_returned_cfg_data.html) in for the
# former and an Environment (env) object (see https://mesonbuild.com/Reference-manual_returned_env.html) for the latter.
# Fortunately however, both types of object can be constructed from a Dictionary (see
# https://mesonbuild.com/Syntax.html#dictionaries), so that is what we define here.
#
# Also, note that the export is done during build, not install, so, eg the value of prefix below will typically _not_ be
# affected by any `--destdir` option passed in to `meson install` because the generation was done previously when
# `meson compile` was called.
#
exportedVariables = {
   'CONFIG_VERSION_STRING'       : meson.project_version(),
   'CONFIG_DATA_DIR'             : prefix + '/' + installSubDir_data, # This is a bit of a hack...
   'CONFIG_APPLICATION_NAME_UC'  : capitalisedProjectName,
   'CONFIG_APPLICATION_NAME_LC'  : projectName,
   'CONFIG_APPLICATION_NAME_AC'  : projectName.to_upper(),
   'CONFIG_EXECUTABLE_NAME'      : mainExecutableFileName,
   # NB CMAKE_HOST_SYSTEM means something different than meson host_machine
   'CONFIG_BUILD_SYSTEM'         : build_machine.system(),
   'CONFIG_RUN_SYSTEM'           : host_machine.system(),
   'CONFIG_CXX_COMPILER_ID'      : compiler.get_id(),
   # Meson doesn't directly give you a way to obtain the current date and time.  But it does allow you turn an external
   # command, so this is one way to get it - relying on the fact that MSYS2 on Windows, Linux and Mac all have date
   # available from the command line.
   'CONFIG_BUILD_TIMESTAMP'      : run_command('date', check: true).stdout().strip(),
   # Similarly, an approximate date (eg February 2023) is generated for use on man pages
   'CONFIG_BUILD_MONTH_AND_YEAR' : run_command('date', '+"%B %Y"', check: true).stdout().strip(),
   #
   # This is exported for generating the compressed changelog for building a Debian package
   #
   'CONFIG_CHANGE_LOG_UNCOMPRESSED' : join_paths(meson.current_source_dir(), filesToInstall_changeLogUncompressed),

   # See https://www.debian.org/doc/debian-policy/ch-binary.html#s-maintainer for more on the "maintainer", but
   # essentially it's a required package property that needs to be either one person or a group mailing list.  In the
   # latter case, the individual maintainers need be listed in a separate property called "uploaders".  Right now, we
   # don't have a mailing list, so this is a moot point.
   #
   # Also note, per https://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-maintainer, that it's simplest
   # to avoid having full stops in the maintainer's name.
   'CONFIG_PACKAGE_MAINTAINER'      : 'Matt Young <mfsy@yahoo.com>',

   # Info for InstallerWindowIcon which is "Filename for a custom window icon in PNG format for the Installer
   # application.
   # Used on Windows and Linux, no functionality on macOS."
   'CONFIG_INSTALLER_WINDOW_ICON'     : capitalisedProjectName + 'Logo.png',
   'CONFIG_INSTALLER_WINDOW_ICON_DIR' : 'images',

   # Full path of file containing full GPL v3 license text
   'CONFIG_LICENSE_TEXT_PATH' : join_paths(meson.current_source_dir(), filesToInstall_license),

   # Some installers/package formats want a one-line description
   'CONFIG_DESCRIPTION_STRING' : 'Open source brewing software',

   # Some installers, eg NSIS on Windows, want a brief copyright string
   'CONFIG_COPYRIGHT_STRING' : 'Copyright 2009-2023.  Distributed under the terms of the GNU General Public License (version 3).',

   # Installers often want the name of the organisation supplying the product, so we need something to put there
   'CONFIG_ORGANIZATION_NAME' : 'The ' + capitalisedProjectName + ' Team',

   # Similarly, installers often want a URL link.  We also use this in the program, to tell users where to get support.
   'CONFIG_GITHUB_URL' : 'https://github.com/' + capitalisedProjectName + '/' + projectName,
   'CONFIG_WEBSITE_URL' : 'https://www.brewtarget.beer/',
   'CONFIG_ORGANIZATION_DOMAIN' : 'brewken.com',

   #
   # On Windows and Mac, the external packaging step (managed by the `bt` build tool script) needs to know about all the
   # non-Qt shared libraries on which we depend.
   #
   # Meson only allows you to directly export strings and ints, not lists.  So, we do a bit of a hack to convert the
   # list to a string that will, in the TOML format file that we care about, be interpreted as a list.  This isn't
   # strictly correct when the list is empty, but we hereby assert that it never will be, so it doesn't matter.
   #
   'CONFIG_SHARED_LIBRARY_PATHS' : '["' + '", "'.join(sharedLibraryPaths) + '"]',

   # This is used by the Doxygen configuration file (see docs/Doxyfile.meson.in)
   'CONFIG_BUILD_DIR' : meson.project_build_root(),

}

# We need to set TargetDir in config.xml for the Qt IFW installer
if host_machine.system() == 'linux'
   exportedVariables += {
      'CONFIG_TARGET_INSTALL_DIR' : '/'
   }
endif

#
# Export the name/location of the desktop icon (which, on Windows, is also the same icon for the installer) for the
# packaging scripts.
#
if host_machine.system() == 'windows'
   exportedVariables += {
      'CONFIG_INSTALLER_APPLICATION_ICON_PATH' : join_paths(meson.current_source_dir(), 'win', capitalisedProjectName + 'Icon_96.ico'),
   }
elif host_machine.system() == 'darwin'
   exportedVariables += {
      'CONFIG_INSTALLER_APPLICATION_ICON'      : capitalisedProjectName + 'Icon.icns',
   }
endif

#=======================================================================================================================
#===================================== Generate config.h etc from config.h.in etc ======================================
#=======================================================================================================================

#
# First we inject build-system variables into the source code.  This saves defining certain things twice - in this file
# and in a C++ source code file.  It also makes it easier for Brewken and Brewtarget to share code, because there are
# fewer places where the application name is hard-coded.
#
# Taking src/config.h.in as input, we generate (in the build subdirectory only) config.h.
#
# All variables written as "@VAR@" in src/config.h.in (input file) will be replaced in config.h (output file) by the
# value of VAR in the configuration_data dictionary we define here.
#
configure_file(input  : 'src/config.h.in',
               output : 'config.h',
               configuration : exportedVariables,
               install : false)

#
# Next we make build-system variables available to the `bt` build helper Python script by injecting them into a TOML
# file.
#
configure_file(input  : 'packaging/config.toml.in',
               output : 'config.toml',
               configuration : exportedVariables,
               install : false)

#
# We also want to inject build-system variables into various platform-specific packaging scripts, control files and so
# on.
#
# .:TODO:.: We ultimately want the generated files inside the 'packaging' subtree of the the build directory.  Currently,
# that gets done by the bt script, but we could use the subdir('packaging') command here to have Meson put the file in
# the right place.  We should do this at some point (and make the corresponding changes to the bt script).
#
if host_machine.system() == 'linux'
   #
   # Linux: Debian Binary package control file
   #
   configure_file(input  : 'packaging/linux/control.in',
                  output : 'control',
                  configuration : exportedVariables,
                  install : false)
   #
   # Linux: RPM binary package spec file
   #
   configure_file(input  : 'packaging/linux/rpm.spec.in',
                  output : 'rpm.spec',
                  configuration : exportedVariables,
                  install : false)
elif host_machine.system() == 'windows'
   #
   # Windows: NSIS installer script
   #
   configure_file(input  : 'packaging/windows/NsisInstallerScript.nsi.in',
                  output : 'NsisInstallerScript.nsi',
                  configuration : exportedVariables,
                  install : false)
elif host_machine.system() == 'darwin'
   #
   # Mac: Information Property List file
   #
   configure_file(input  : 'packaging/darwin/Info.plist.in',
                  output : 'Info.plist',
                  configuration : exportedVariables,
                  install : false)
endif

#=======================================================================================================================
#========================================= Generate manpage for Linux and Mac ==========================================
#=======================================================================================================================
if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
   message('Generating man page')
   #
   # On Linux (and TBD Mac) we inject build system variables into the markdown text that will be used to generate the
   # man page for the application.
   #
   manPageMarkdown = configure_file(input  : 'doc/manpage.1.md.in',
                                    output : 'manpage.1.md',
                                    configuration : exportedVariables,
                                    install : false)

   #
   # Now we take that markdown-with-injected-variables and generate a man page from it using the pandoc utility.
   #
   # We use Meson's configure_file() command again here, but this time to wrap pandoc, which means we can use Meson file
   # objects.  Note that, although configure_file() can take stdout and turn it into the 'output' file, I have yet to
   # find a way to have it pass the contents of the 'input' file to the command as stdin.  However, it can pass the name
   # of the input file via '@INPUT@', and this is all we need in this instance.
   #
   # The pandoc options we use are:
   #    --verbose      = give verbose debugging output (though currently this does not have much effect)
   #    -t man         = generate output in man format
   #    -s             = generate a complete man page (rather than just some text in man format)
   #    -o <filename>  = specifies the output file
   #
   # Note that, although man pages are frequently compressed, the exact compression mechanism is distro-specific, so
   # Meson now considers such compression outside the scope of the build system.  (We therefore do it in the bt build
   # tool script.)
   #
   # TODO: For the moment we are only producing an English man page.  It would not be huge effort to produce them for
   # non-English locales, so we should do that at some point.
   #
   generatedManPage = configure_file(
      input: manPageMarkdown,
      output: projectName + '.1',
      capture : true,
      command: ['pandoc', '@INPUT@', '--verbose', '-t', 'man', '-s'],
   )
   install_man(generatedManPage)
endif

#=======================================================================================================================
#======================= Install files that we ship with the software (sounds, default DB, etc) ========================
#=======================================================================================================================
# Note that we'll get a bunch of stuff in the meson logs about 'Failed to guess install tag' but this is entirely
# harmless as we are not using tags.
install_data(filesToInstall_data,   install_dir : installSubDir_data)
install_data(filesToInstall_docs,   install_dir : installSubDir_doc)
install_data(filesToInstall_sounds, install_dir : installSubDir_data + '/sounds')
if host_machine.system() == 'linux'
   # Install the icons
   # Per https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html#install_icons, "installing a
   # svg icon in $prefix/share/icons/hicolor/scalable/apps means most desktops will have one icon that works for all
   # sizes".
   install_data(filesToInstall_icons, install_dir : installSubDir_icons + '/hicolor/scalable/apps/')

   # Install the .desktop file
   install_data(filesToInstall_desktop, install_dir : installSubDir_applications)

   # Install friendly-format change log aka release notes
   # Note that lintian does not like having a file called CHANGES.markdown in the doc directory, as it thinks it is a
   # misnamed changelog.Debian.gz (even when changelog.Debian.gz is also present!) so you get a
   # wrong-name-for-upstream-changelog warning.
   # The simplest way round this is to rename CHANGES.markdown to RelaseNotes.markdown
   install_data(filesToInstall_changeLogUncompressed, rename : 'RelaseNotes.markdown', install_dir : installSubDir_doc)

   # Debian packages need to have the copyright file in a particular place (/usr/share/doc/PACKAGE/copyright)
   # RPM packages don't like having duplicate files in the same directory (eg copyright and COPYRIGHT with same
   # contents).  So the simplest thing is to rename COPYRIGHT to copyright for both.
   install_data(filesToInstall_copyright, rename : 'copyright', install_dir : installSubDir_doc)

else
   #----------- Windows and Mac -----------
   install_data(filesToInstall_copyright, install_dir : installSubDir_doc)
endif

if host_machine.system() == 'darwin'
   # Desktop icon
   install_data(filesToInstall_macIcons, install_dir : installSubDir_data)
endif

#=======================================================================================================================
#========================================= Compiler-specific settings & flags ==========================================
#=======================================================================================================================
if compiler.get_id() == 'gcc'
   #
   # -g3   should give even more debugging information than -g (which is equivalent to -g2)
   #
   # -O2   is hopefully a sensible optimisation level.  It means "GCC performs nearly all supported optimizations that do
   #       not involve a space-speed tradeoff. As compared to -O, this option increases both compilation time and the
   #       performance of the generated code."
   #
   # -z noexecstack  Is, in theory at least, to ensure/assert we do not have an executable stack.  This is partly as a
   #                 good thing in itself, and partly because, by default, rpmlint with throw a
   #                 missing-PT_GNU_STACK-section error if we don't.
   #                    In theory, the compiler should work out automatically whether we need an executable stack,
   #                 decide the answer is "No" and pass all the right options to the linker.   In practice, it seems
   #                 this doesn't happen for reasons I have, as yet, to discover.
   #                    So, we attempt to assert manually that the stack should not be executable.  The "-z noexecstack"
   #                 should get passed through by gcc the linker (see
   #                 https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html#Link-Options) and the GNU linker
   #                 (https://sourceware.org/binutils/docs/ld/Options.html) should recognise "-z noexecstack" as "Marks
   #                 the object as not requiring executable stack".
   #                    However, even this is not sufficient(!).  So, for the moment, we suppress the rpmlint error (see
   #                 packaging/rpmLintFilters.toml).
   #
   # -fconcepts  Is needed on older versions of GCC to enable C++20 concepts (because specifying C++20 was not enough
   #             for some reason).
   #
   # The following are, according to some comments at
   # https://stackoverflow.com/questions/52583544/boost-stack-trace-not-showing-function-names-and-line-numbers, needed
   # for Boost stacktrace to work properly:
   #    -no-pie
   #    -fno-pie
   #    -rdynamic
   #
   # HOWEVER, there are a couple of gotchas:
   #    - For some reason, gcc on Windows does not accept -rdynamic -- so we only set this on Linux
   #    - On Linux, executables in Debian packages are supposed to be compiled as position-independent code, otherwise
   #      we'll get a 'hardening-no-pie' Lintian warning -- so we only set no-pie options on Windows
   #
   add_global_arguments(['-g3',
                         '-O2',
                         '-z', 'noexecstack', # NB Not '-z noexecstack' as otherwise will be passed to gcc in quotes!
                         '-fconcepts'
                         ], language : 'cpp')
   if host_machine.system() == 'windows'
      add_global_arguments     (['-no-pie', '-fno-pie'], language : 'cpp')
      if compiler.get_linker_id() == 'ld.bfd'
         #
         # GNU Linker
         #
         # See https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html for options
         #
         add_global_link_arguments(['-no-pie',
                                    '-fno-pie'], language : 'cpp')
      endif
   else
      add_global_arguments     (['-pie', '-fpie'], language : 'cpp')
      add_global_link_arguments(['-pie', '-fpie'], language : 'cpp')
      add_global_arguments     ('-rdynamic', language : 'cpp')
      add_global_link_arguments('-rdynamic', language : 'cpp')
   endif
endif

if host_machine.system() == 'darwin'
   # As explained at https://stackoverflow.com/questions/5582211/what-does-define-gnu-source-imply, defining _GNU_SOURCE
   # gives access to various non-standard GNU/Linux extension functions and changes the behaviour of some POSIX
   # functions.
   #
   # This is needed for Boost stacktrace on Mac
   add_project_arguments('-D_GNU_SOURCE', language : 'cpp')
endif

#=======================================================================================================================
#========================================== Linker-specific settings & flags ===========================================
#=======================================================================================================================
#
# On MacOS, there are some extra linker flags to tell the OS where to look for shared libraries that we ship in the app
# bundle.  These are passed in via the install_rpath parameter on Meson's `executable` function.  We don't want to pass
# this parameter on Windows or Linux.  (I think it's harmless on Windows but might have some undesirable effect on
# Linux.)  Fortunately we can put additional arguments in a platform-specific dictionary that can be passed to any
# Meson function via the special 'kwargs' argument (provided there is no overlap between arguments specified directly in
# the function call and those included in the dictionary).
#
# See comments in the `bt` build tool script for more on where/how MacOS searches for shared libraries.
#
if host_machine.system() == 'darwin'
   platformSpecificArgs = {'install_rpath': '@executable_path/../Frameworks'}
else
   platformSpecificArgs = {}
endif

#=======================================================================================================================
#===================================================== Main builds =====================================================
#=======================================================================================================================
#
# To keep things simple, we share almost all code between the actual executable and the unit test runner.  However, we
# don't want to compile everything twice.  So, as a trick we compile into a static library everything except the code
# that differs between actual executable and unit test runner, then this library is linked into both programs.
#
# Note however that you cannot put generatedFromQrc in the static_library as it doesn't work there.
#
commonCodeStaticLib = static_library('common_code',
                                     commonSourceFiles, generatedFromUi, generatedFromMoc, translations,
                                     include_directories : includeDirs,
                                     dependencies: commonDependencies,
                                     install : false)

mainExecutable = executable(mainExecutableTargetName,
                            applicationMainSourceFile,
                            generatedFromQrc,
                            include_directories : includeDirs,
                            dependencies : mainExeDependencies,
                            link_with : commonCodeStaticLib,
                            kwargs : platformSpecificArgs,
                            install : true)

testRunner = executable(testRunnerTargetName,
                        unitTestMainSourceFile,
                        generatedFromQrc,
                        generatedFromMocForUnitTests,
                        include_directories : includeDirs,
                        dependencies : testRunnerDependencies,
                        link_with : commonCodeStaticLib,
                        install : false)

#=======================================================================================================================
#===================================================== Unit Tests ======================================================
#=======================================================================================================================
test('Test integer sizes',                   testRunner, args : ['pstdintTest'])
test('Test recipe calculations - all grain', testRunner, args : ['recipeCalcTest_allGrain'])
test('Test post boil loss OG',               testRunner, args : ['postBoilLossOgTest'])
test('Test unit conversions',                testRunner, args : ['testUnitConversions'])
test('Test NamedParameterBundle',            testRunner, args : ['testNamedParameterBundle'])
test('Test number display and parsing',      testRunner, args : ['testNumberDisplayAndParsing'])
test('Test algorithms',                      testRunner, args : ['testAlgorithms'])
test('Test type lookups',                    testRunner, args : ['testTypeLookups'])
test('Test inventory',                       testRunner, args : ['testInventory'])
# Need a bit longer than the default 30 second timeout for the log rotation test on some platforms
test('Test log rotation',                    testRunner, args : ['testLogRotation'], timeout : 60)

#===


doxygen = find_program('doxygen', required : false)
if not doxygen.found()
   warning('Doxygen not found - continuing without Doxygen support')
else
##   src_doxygen = files(
##      # source files
##      join_paths(meson.source_root(), 'src'),
##   )
   doxyfile = configure_file(input : 'doc/Doxyfile.meson.in',
                             output : 'Doxyfile',
                             configuration : exportedVariables,
                             install : false)
   custom_target('doc',
                 command: [doxygen, doxyfile],
                 input: doxyfile,
                 output: 'html',
                 install: false)
endif
