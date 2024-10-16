#╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
# CMakeLists.txt is part of Brewtarget, and is Copyright the following authors 2009-2024
#  - Chris Pavetto <chrispavetto@gmail.com>
#  - Dan Cavanagh <dan@dancavanagh.com>
#  - Daniel Moreno <danielm5@users.noreply.github.com>
#  - Daniel Pettersson <pettson81@gmail.com>
#  - Kregg Kemper <gigatropolis@yahoo.com>
#  - Matt Young <mfsy@yahoo.com>
#  - Maxime Lavigne (malavv) <duguigne@gmail.com>
#  - Mik Firestone <mikfire@gmail.com>
#  - Philip Greggory Lee <rocketman768@gmail.com>
#  - Robby Workman <rworkman@slackware.com>
#  - Théophane Martin <theophane.m@gmail.com>
#
# Brewtarget is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Brewtarget is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌

# ⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐
# NB: Meson and the `bt` build tool Python script are now the primary way of building and packaging the software.  You
#     can also still CMake to compile the product and install it locally, but we no longer support using CMake to do
#     packaging.  (Over time the intention is to remove packaging-specific code from this script, not least as it does
#     not work properly on Mac and Windows.)
# ⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐⭐

#
# Creates a Makefile in the build directory, from where you can do builds and installs.
#
# NOTE: cmake . -DCMAKE_INSTALL_PREFIX=/tmp/blah && make DESTDIR=/foo
# will install to /foo/tmp/blah.
#
# See also - src/CMakeLists.txt
#
# Standard make targets:
#   * make                  -  Regenerate the makefile if necessary and compile the source code.  (On Linux, also
#                              converts the markdown list of changes (CHANGES.markdown) to Debian package format
#                              changelog.)
#
#                              NB: Other make targets will NOT regenerate the makefile from this CMakeLists.txt file.
#                              That means that, if you make a change that affects "make package" or "make clean" etc,
#                              you must run "make" before you run "make package" or "make clean" etc, otherwise your
#                              change will not be picked up.  (If necessary, you can interrupt the build that "make"
#                              kicks off, as, by that point, the makefile will be updated.)
#
#   * make clean            -  Delete compiled objects so next build starts from scratch
#   * sudo make install     -  Install locally
#   * make test             -  Run unit tests via CTest
#
# Custom make targets:
#   * make source_doc       -  Makes Doxygen HTML documentation of the source in doc/html
#   * make install-data
#   * make install-runtime
#
# CMake options
#   * CMAKE_INSTALL_PREFIX  - /usr/local by default. Set this to /usr on Debian-based systems like Ubuntu.
#   * DO_RELEASE_BUILD      - OFF by default. If ON, will do a release build. Otherwise, debug build.
#   * NO_MESSING_WITH_FLAGS - OFF by default. ON means do not add any build flags whatsoever. May override other options.
# NOTE: You need to run CMake to change the options (you can't change them just by running make, not even by running
# make clean.  Eg, in the build directory, run the following to switch to debug builds:
#     cmake -DDO_RELEASE_BUILD=OFF ..
#

# Uncomment the next "set" line for slightly verbose build output
# Alternatively, for very verbose output, invoke make as follows:
#
#    make VERBOSE=1
#
# On Windows, you need:
#
#    cmake --build . --verbose
#
#set(CMAKE_VERBOSE_MAKEFILE ON)

#=======================================================================================================================
#================================================= CMake Configuration =================================================
#=======================================================================================================================
cmake_minimum_required(VERSION 3.16)
cmake_policy(VERSION 3.16)
# Ensure we are using modern behaviour of the project command (introduced in CMake 3.0) which enables setting version
# numbers via it (rather than manually setting individual major/minor/patch vars).
cmake_policy(SET CMP0048 NEW)

# Compatibility settings used with modern CMAKE (>= 3.29) used to pull BoostConfig.cmake from upstream
# -> FindBoost cmake module was removed in recent versions.
# See https://cmake.org/cmake/help/latest/module/FindBoost.html
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_GREATER_EQUAL 3.29)
cmake_policy(SET CMP0167 NEW)
endif()

#=======================================================================================================================
#================================================ Other preliminaries =================================================
#=======================================================================================================================
# Set the target binary architecture for targets on macOS
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
   # Per https://cmake.org/cmake/help/latest/variable/CMAKE_OSX_ARCHITECTURES.html, "the value of this variable should
   # be set prior to the first project() or enable_language() command invocation because it may influence configuration
   # of the toolchain and flags".
   set(CMAKE_OSX_ARCHITECTURES x86_64, arm64) # Build both x86_64 and arm64 for Apple silicon.
   #set(CMAKE_OSX_ARCHITECTURES i386 x86_64) # Build intel binary.
   #set(CMAKE_OSX_ARCHITECTURES ppc i386 ppc64 x86_64) # Build universal binary.
endif()

#=======================================================================================================================
#============================================== Project name and Version ===============================================
#=======================================================================================================================
# It's simplest to keep the project name all lower-case as it means we can use a lot more of the default settings for
# Linux packaging (where directory names etc are expected to be all lower-case).
project(brewtarget VERSION 4.0.8 LANGUAGES CXX)
message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION}")
message(STATUS "PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}")
# Sometimes we do need the capitalised version of the project name
set(capitalisedProjectName Brewtarget)
# We use this in the program, to tell users where to get support.
set(CONFIG_GITHUB_URL "https://github.com/${capitalisedProjectName}/${projectName}")
set(CONFIG_WEBSITE_URL "https://www.brewtarget.beer/")
set(CONFIG_ORGANIZATION_DOMAIN "brewken.com")

#=======================================================================================================================
#======================================================= Options =======================================================
#=======================================================================================================================
option(DO_RELEASE_BUILD "If on, will do a release build. Otherwise, debug build." OFF)
option(NO_MESSING_WITH_FLAGS "On means do not add any build flags whatsoever. May override other options." OFF)

#=======================================================================================================================
#===================================================== Directories =====================================================
#=======================================================================================================================

#================================================== Source directories =================================================
#
# ${repoDir}                   = the directory containing this (CMakeLists.txt) file
# ${repoDir}/src               = C++ source code
# ${repoDir}/ui                = QML UI layout files
# ${repoDir}/data              = Binary files, including sounds and default database
# ${repoDir}/translations      = Translated texts
# ${repoDir}/mac               = Mac-specific files (desktop icon)
# ${repoDir}/win               = Windows-specific files (desktop icon)
# ${repoDir}/packaging         = Packaging-related config
#
set(repoDir "${CMAKE_CURRENT_SOURCE_DIR}")

# Location of custom CMake modules. (At the moment, there is only one, which is used for Windows packaging
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

#================================================= Install directories =================================================
# Note that WIN32 is true "when the target system is Windows, including Win64" (see
# https://cmake.org/cmake/help/latest/variable/WIN32.html), so this is ALL versions of Windows, not just 32-bit.
# UNIX is true when the "the target system is UNIX or UNIX-like", so it is set when we're building for Mac and for
# Linux.  There is a separate flag (APPLE) when we're building for Mac, but, AFAICT, no specific flag for Linux.
#
if(UNIX AND NOT APPLE)
   #============================================= Linux Install Directories ============================================
   # By default, CMAKE_INSTALL_PREFIX is:
   #   - /usr/local                           on Linux (& Mac)
   #   - c:/Program Files/${PROJECT_NAME}     on Windows
   #
   # On a lot of Linux distros, including Debian and derived systems (such as Ubuntu), it's more normal for pretty much
   # all user-installed apps to go in /usr/bin rather than /usr/local/bin, so CMAKE_INSTALL_PREFIX can be overridden on
   # the command line via "--prefix usr"
   #
   # (See http://lists.busybox.net/pipermail/busybox/2010-December/074114.html for a great, albeit slightly depressing,
   # explanation of why there are so many places for binaries to live on Unix/Linux systems.  FWIW, the current
   # "standards" for Linux are at https://refspecs.linuxfoundation.org/fhs.shtml but these are open to interpretation.)
   #
   # .:TBD: We also allow -DEXEC_PREFIX=/usr (used in .github/workflows/linux-ubuntu.yml) but I'm not sure if this is
   #        needed or could be replaced by "--prefix usr"
   #
   # Debian standard directories.
   if(NOT EXEC_PREFIX)
      set(EXEC_PREFIX ${CMAKE_INSTALL_PREFIX})
   endif()

   set(installSubDir_data         "share/${CMAKE_PROJECT_NAME}")
   set(installSubDir_doc          "share/doc/${CMAKE_PROJECT_NAME}")
   set(installSubDir_bin          "bin")
   # According to https://specifications.freedesktop.org/menu-spec/menu-spec-1.0.html#paths, .desktop files need to live
   # in one of the $XDG_DATA_DIRS/applications/.  (Note that $XDG_DATA_DIRS is a colon-separated list of directories, typically
   # defaulting to /usr/local/share/:/usr/share/. but on another system it might be
   # /usr/share/plasma:/usr/local/share:/usr/share:/var/lib/snapd/desktop:/var/lib/snapd/desktop).  When combined with
   # CMAKE_INSTALL_PREFIX, "share/applications" should end up being one of these.
   set(installSubDir_applications "share/applications")
   # It's a similar but slightly more complicated story for where to put icons.  (See
   # https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html#directory_layout for all the
   # details.)
   set(installSubDir_icons        "share/icons")

elseif(WIN32)
   #============================================ Windows Install Directories ===========================================
   set(installSubDir_data "data")
   set(installSubDir_doc  "doc")
   set(installSubDir_bin  "bin")
elseif(APPLE)
   #============================================== Mac Install Directories =============================================
   set(installSubDir_data "Contents/Resources")
   set(installSubDir_doc  "Contents/Resources/en.lproj")
   set(installSubDir_bin  "Contents/MacOS")
endif()

#=======================================================================================================================
#====================================================== File Names =====================================================
#=======================================================================================================================
if(APPLE)
   # Use capital letters. Don't question the APPLE.
   set(fileName_executable "${capitalisedProjectName}")
else()
   set(fileName_executable "${PROJECT_NAME}")
endif()
set(fileName_unitTestRunner "${PROJECT_NAME}_tests")

#=======================================================================================================================
#=================================================== General Settings ==================================================
#=======================================================================================================================
# This is needed to enable the add_test() command
enable_testing()
if (APPLE)
   # On Mac we ask CMake to try to find static libraries when available -- because it's so painful shipping dynamic
   # libraries in a Bundle.
   set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

#=======================================================================================================================
#=============================================== Installation Components ===============================================
#=======================================================================================================================
# For architecture-independent data
set(DATA_INSTALL_COMPONENT "Data")
# For architecture-dependent binaries
set(RUNTIME_INSTALL_COMPONENT "Runtime")

#=======================================================================================================================
#============================================== Compiler settings & flags ==============================================
#=======================================================================================================================
# We use different compilers on different platforms.  Where possible, we want to let CMake handle the actual compiler
# settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# We need C++20 for std::map::contains() and concepts, C++17 or later for nested namespaces and structured bindings, and
# C++11 or later for lambdas.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${repoDir}/third-party/valijson/include/)

include_directories(${repoDir}/src)
include_directories("${CMAKE_BINARY_DIR}/src") # In case of out-of-source build.

# GCC-specific flags
if(NOT ${NO_MESSING_WITH_FLAGS})
   if(CMAKE_COMPILER_IS_GNUCXX)
      #
      # We would like to avoid having an executable stack, partly as a good thing in itself, and partly because, by
      # default, rpmlint with throw a missing-PT_GNU_STACK-section error if we don't.
      #
      # In theory, the compiler should work out automatically whether we need an executable stack, decide the answer is
      # "No" and pass all the right options to the linker.   In practice, it seems this doesn't happen for reasons I
      # have, as yet, to discover.
      #
      # We attempt here to to assert manually that the stack should not be executable.  The "-z noexecstack" should
      # get passed through by gcc the linker (see https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html#Link-Options) and
      # the GNU linker (https://sourceware.org/binutils/docs/ld/Options.html) should recognise "-z noexecstack" as
      # "Marks the object as not requiring executable stack".
      #
      # However, this is not sufficient.  So, for the moment, we suppress the rpmlint error (see
      # packaging/linux/rpmLintFilters.toml).
      #
      # Additionally, NOTE that "-z options are just not supported for Windows versions of ld" (as mentioned at
      # https://stackoverflow.com/questions/55418931/ld-exe-unrecognized-option-z).  So we have to exclude that option
      # on Windows.
      #
      if(NOT WIN32)
         set(CMAKE_CXX_FLAGS_RELEASE "-Wall -ansi -pedantic -Wno-long-long -O2 -z noexecstack")
      else()
         set(CMAKE_CXX_FLAGS_RELEASE "-Wall -ansi -pedantic -Wno-long-long -O2")
      endif()
      #
      # -g3 should give even more debugging information than -g (which is equivalent to -g2)
      # -no-pie -fno-pie -rdynamic are needed for Boost stacktrace to work properly - at least according to comments
      # at https://stackoverflow.com/questions/52583544/boost-stack-trace-not-showing-function-names-and-line-numbers
      #
      # But, for some reason, gcc on Windows does not accept -rdynamic
      #
      if(NOT WIN32)
         set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g3 -no-pie -fno-pie -rdynamic")
      else()
         set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g3 -no-pie -fno-pie")
      endif()
   endif()

   # Speed up compilation if using gcc.
   if(UNIX AND NOT APPLE)
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe")
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pipe")
   endif()
endif()
if(CMAKE_COMPILER_IS_GNUCXX)
   #
   # On older versions of GCC, it is not sufficient to specify C++20 to enable concepts, you also have to set a
   # special compiler flag.
   #
   add_compile_options(-fconcepts)
endif()

#
# On Ubuntu 22.04, the packages for Qt6 differ from those for Qt5 in that they require you to build with the PIC option,
# otherwise we'll get the error: "You must build your code with position independent code if Qt was built with
# -reduce-relocations. " "Compile your code with -fPIC (and not with -fPIE)."
#
# On certain instances of Windows, we'll get "relocation truncated to fit" linker errors if we don't build with position
# independent code (see
# https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean for more
# explanation).
#
if((UNIX AND NOT APPLE) OR WIN32)
   set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Windows-specific compilation settings
if(WIN32)
   # See https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Link-Options.html#Link-Options for more on GCC linker options
   # In theory, we could specify "-static-libgcc -static-libstdc++ -static" to statically link all the GCC and MinGW
   # libraries, so we don't have to ship DLLs for them.  In practice, this (a) does not prevent us needing a fair few
   # other DLLs and (b) per https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Link-Options.html#Link-Options it's advised
   # not to because:
   #    "There are several situations in which an application should use the shared libgcc instead of the static
   #    version. The most common of these is when the application wishes to throw and catch exceptions across different
   #    shared libraries. In that case, each of the libraries as well as the application itself should use the shared
   #    libgcc.
   #       "Therefore, the G++ driver automatically adds -shared-libgcc whenever you build a shared library or a main
   #    executable, because C++ programs typically use exceptions, so this is the right thing to do."
   #
#   set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

if(APPLE)
   # As explained at https://stackoverflow.com/questions/5582211/what-does-define-gnu-source-imply, defining _GNU_SOURCE
   # gives access to various non-standard GNU/Linux extension functions and changes the behaviour of some POSIX
   # functions.
   #
   # This is needed for Boost stacktrace on Mac
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE")
endif()

#======Speed up compilation by using precompiled headers (PCH) for development======

# (ADD_PCH_RULE  _header_filename _src_list)
# Version 7/26/2010
#
# use this macro before "add_executable"
#
# _header_filename
#  header to make a .gch
#
# _src_list
#   the variable name (do not use ${..}) which contains a
#     a list of sources (a.cpp b.cpp c.cpp ...)
#  This macro will append a header file to it, then this src_list can be used in
#  "add_executable..."
#
#
# Now a .gch file should be generated and gcc should use it.
#        (add -Winvalid-pch to the cpp flags to verify)
#
# make clean should delete the pch file
#
# example : ADD_PCH_RULE(headers.h myprog_SRCS)
MACRO (ADD_PCH_RULE  _header_filename _src_list)
   set(_gch_filename "${CMAKE_CURRENT_BINARY_DIR}/${_header_filename}.gch")
   set(_header "${CMAKE_CURRENT_SOURCE_DIR}/${_header_filename}")

   LIST(APPEND ${_src_list} ${_gch_filename})

   SET (_args ${CMAKE_CXX_FLAGS})
   LIST(APPEND _args -c ${_header} -o ${_gch_filename})

   GET_DIRECTORY_PROPERTY(DIRINC include_directories)
   FOREACH(_inc ${DIRINC})
      LIST(APPEND _args "-I" ${_inc})
   ENDFOREACH(_inc ${DIRINC})

   SEPARATE_ARGUMENTS(_args)

   ADD_CUSTOM_COMMAND(OUTPUT ${_gch_filename}
                      COMMAND rm -f ${_gch_filename}
                      COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_args}
                      DEPENDS ${_header})
ENDMACRO(ADD_PCH_RULE _src_list _header_filename)

#=======================================================================================================================
#=================================================== Set build type ====================================================
#=======================================================================================================================
# We might always to tell the compiler to include debugging information (eg via the -g option on gcc).  It makes the
# binaries slightly bigger on Linux, but helps greatly in analysing core dumps etc.  (In closed-source projects people
# sometimes turn it off for release builds to make it harder to reverse engineer the software, but obviously that's not
# an issue for us.)
#
# However, setting CMAKE_BUILD_TYPE to "Debug", also changes other things, such as the default location for config
# files, which we don't want on a release build, so we would probably need to set compiler flags directly.
#
# .:TBD:. Investigate whether setting CMAKE_BUILD_TYPE to "RelWithDebInfo" does what we want.
#
if(${DO_RELEASE_BUILD})
   set(CMAKE_BUILD_TYPE "Release")
else()
   set(CMAKE_BUILD_TYPE "Debug")
   set(CMAKE_ENABLE_EXPORTS true)
endif()
message(STATUS "Doing ${CMAKE_BUILD_TYPE} build (DO_RELEASE_BUILD = ${DO_RELEASE_BUILD})")

#=======================================================================================================================
#========================================= Find various libraries we depend on =========================================
#=======================================================================================================================

#======================================================= Find Qt =======================================================
# We need not just the "core" bit of Qt but various "optional" elements.
#
# We try to keep the minimum Qt version we need as low as we can.
#
# Note that if you change the minimum Qt version, you need to make corresponding changes to the .github/workflows/*.yml
# files so that GitHub uses the appropriate version of Qt for the automated builds.
#
if(UNIX AND NOT APPLE)
   execute_process(COMMAND lsb_release -rs OUTPUT_VARIABLE RELEASE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
   # As of 2024-09-30:
   #   - Qt 6.2.4 is the maximum available in Ubuntu 22.04 (Jammy).
   #   - Qt 6.4.2 is the maximum available in Ubuntu 24.04 (Noble).
   set(QT_MIN_VERSION 6.2.4)
else()
   # Windows and Mac may have newer versions, but we keep everything the same for now
   set(QT_MIN_VERSION 6.2.4)
endif()

if(APPLE)
   #
   # The Qt6 documentation to using CMake at https://doc.qt.io/qt-5/cmake-get-started.html says we need to set
   # CMAKE_PREFIX_PATH environment variable to the Qt 5 installation prefix.  In theory, setting the corresponding CMake
   # variable should do the same job (per https://cmake.org/cmake/help/latest/variable/CMAKE_PREFIX_PATH.html).
   #
   # It seems this is only needed on MacOS.  According to ./third-party/valijson/README.md we are not the only ones to
   # experience this!
   #
   execute_process(COMMAND brew --prefix qt6 OUTPUT_VARIABLE CMAKE_PREFIX_PATH)
   message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
endif()

# Set the AUTOMOC property on all targets.  This tells CMake to automatically handle the Qt Meta-Object Compiler (moc)
# preprocessor (ie the thing that handles Qt's C++ extensions), without having to use commands such as QT4_WRAP_CPP(),
# QT5_WRAP_CPP(), etc.  In particular, it also means we no longer have to manually identify which headers have Q_OBJECT
# declarations etc.
set(CMAKE_AUTOMOC ON)

# Set the AUTOUIC property on all targets.  This tells CMake to automatically handle the Qt uic code generator, without
# having to use commands such as QT4_WRAP_UI(), QT5_WRAP_UI(), etc.
set(CMAKE_AUTOUIC ON)

# This tells CMake where to look for .ui files when AUTOUIC is on
set(CMAKE_AUTOUIC_SEARCH_PATHS ${repoDir}/ui)

# Set the AUTORCC property on all targets.  This tells CMake to automatically handle the Qt Resource Compiler (rcc),
# without having to use commands such as QT4_ADD_RESOURCES(), QT5_ADD_RESOURCES(), etc.
# Note that you need to add your .qrc file(s) as sources to the target you are building
set(CMAKE_AUTORCC ON)

# Name of FOLDER for *_autogen targets that are added automatically by CMake for targets for which AUTOMOC is enabled.
# Note that this replaces AUTOMOC_TARGETS_FOLDER.
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/autogen)

# Directory where AUTOMOC, AUTOUIC and AUTORCC generate files for the target.
set_property(GLOBAL PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/autogen)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
# Although it's possible to do individual find_package commands for each bit of Qt we want to use (Qt6Core, Qt6Widgets,
# Qt6Sql, etc), the newer, and more compact, way of doing things (per
# https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html and https://doc.qt.io/qt-5/cmake-get-started.html) is to do
# one find for Qt as a whole and to list the components that we need.  Depending on what versions of CMake and Qt you
# have installed, the way to find out what components exist varies a bit, but there is a relatively recent list at
# https://stackoverflow.com/questions/62676472/how-to-list-all-cmake-components-of-qt5
#
set(qtCommonComponents
   Core
   Gui
   Multimedia
   Network
   PrintSupport
   Sql
   Svg          # Required to make the deploy scripts pick up the svg plugins
   Widgets
   Xml          # TBD: Not sure we need this any more
)
set(qtTestComponents
   Test
)
set(qtToolsComponents
   LinguistTools
)
list(APPEND qtAllComponents ${qtCommonComponents} ${qtToolsComponents} ${qtTestComponents})
find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS ${qtAllComponents} REQUIRED)
# Each package has its own include directories that we need to make sure the compiler knows about
foreach(qtComponent IN LISTS qtAllComponents)
   # Sometimes it's useful that part of a variable name can be specified by expanding another variable!
   include_directories(${Qt6${qtComponent}_INCLUDE_DIRS})
endforeach()

# Qt wants position independent code in certain circumstances - specifically "You must build your code with position
# independent code if Qt was built with -reduce-relocations. Compile your code with -fPIC (and not with -fPIE)."
if(Qt6_POSITION_INDEPENDENT_CODE)
   # This will initialize the POSITION_INDEPENDENT_CODE property on all the targets
   set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# There's apparently a whole bunch of extra work we need to do to use Qt on Windows and Mac
if(WIN32)
   #====================================================================================================================
   #================================================= Windows Qt Stuff =================================================
   #====================================================================================================================
   # .:TBD:.  Not sure whether/why we need these additional Qt components on Windows
   #find_package(Qt6MultimediaWidgets REQUIRED)
   #find_package(Qt6OpenGL REQUIRED)

#   get_target_property(QtMultimediaWidgets_location Qt6::MultimediaWidgets         LOCATION_${CMAKE_BUILD_TYPE})
#   get_target_property(QtOpenGL_location            Qt6::OpenGL                    LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtCore_location              Qt6::Core                      LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtGui_location               Qt6::Gui                       LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtMultimedia_location        Qt6::Multimedia                LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtNetwork_location           Qt6::Network                   LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtPrintSupport_location      Qt6::PrintSupport              LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQgif_location              Qt6::QGifPlugin                LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQico_location              Qt6::QICOPlugin                LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQjpeg_location             Qt6::QJpegPlugin               LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQsvgIcon_location          Qt6::QSvgIconPlugin            LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQsvg_location              Qt6::QSvgPlugin                LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQtiff_location             Qt6::QTiffPlugin               LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQWindows_location          Qt6::QWindowsIntegrationPlugin LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtSqliteDriver_location      Qt6::QSQLiteDriverPlugin       LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtSql_location               Qt6::Sql                       LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtSvg_location               Qt6::Svg                       LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtWidgets_location           Qt6::Widgets                   LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtXml_location               Qt6::Xml                       LOCATION_${CMAKE_BUILD_TYPE})

   # .:TBD:. Not clear whether/where these xxx_DLLs variables get used
   set(Qt_DLLs            ${QtCore_location}
                          ${QtGui_location}
                          ${QtMultimedia_location}
#                          ${QtMultimediaWidgets_location}
                          ${QtNetwork_location}
#                          ${QtOpenGL_location}
                          ${QtPrintSupport_location}
                          ${QtSql_location}
                          ${QtSvg_location}
                          ${QtWebKit_location}
                          ${QtWebKitWidgets_location}
                          ${QtWidgets_location}
                          ${QtXml_location})

   set(SQL_Drivers_DLLs   ${QtSqliteDriver_location})

   set(Image_Formats_DLLs ${QtQgif_location}
                          ${QtQico_location}
                          ${QtQjpeg_location}
                          ${QtQmng_location}
                          ${QtQsvg_location}
                          ${QtQtiff_location})

   set(Icon_Engines_DLLs ${QtQsvgIcon_location})

   set(Platform_DLLs     ${QtQWindows_location})

   get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
   get_filename_component(QT_BIN_DIR "${_qmake_executable}" DIRECTORY)
   message("QT_BIN_DIR = ${QT_BIN_DIR}")

   #
   # Per https://doc.qt.io/qt-6/windows-deployment.html, the windeployqt executable creates all the necessary folder
   # tree "containing the Qt-related dependencies (libraries, QML imports, plugins, and translations) required to run
   # the application from that folder".
   #
   # On some systems at least, looks like Qt6::windeployqt is already available in CMake (when Qt5::windeployqt) was
   # not.  If it is, then we can't try to set it up again as we'll get an error ("add_executable cannot create imported
   # target "Qt6::windeployqt" because another target with the same name already exists").
   #
   if (NOT TARGET Qt6::windeployqt)
      find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
      if(EXISTS ${WINDEPLOYQT_EXECUTABLE})
         # Per https://cmake.org/cmake/help/latest/command/add_executable.html, "IMPORTED executables are useful for
         # convenient reference from commands like add_custom_command()".
         add_executable(Qt6::windeployqt IMPORTED)
         set_target_properties(Qt6::windeployqt PROPERTIES IMPORTED_LOCATION ${WINDEPLOYQT_EXECUTABLE})
      endif()
   endif()

   # International Components for Unicode
   file(GLOB IcuDlls "${QT_BIN_DIR}/libicu*.dll")
elseif(APPLE)
   #====================================================================================================================
   #=================================================== Mac Qt Stuff ===================================================
   #====================================================================================================================

   # The macdeployqt executable shipped with Qt does for Mac what windeployqt does for Windows -- see
   # https://doc.qt.io/qt-6/macos-deployment.html#the-mac-deployment-tool
   #
   # At first glance, you might thanks that, with a few name changes, we might share all the CMake code for macdeployqt
   # and windeployqt.  However, as you will see below, the two programs share _only_ a top-level goal ("automate the
   # process of creating a deployable [folder / applicaiton bundle] that contains [the necessary Qt dependencies]" - ie
   # so that the end user does not have to install Qt to run our software).  They have completely different
   # implementations and command line options, so it would be unhelpful to try to treat them identically.
   find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
   if(EXISTS ${MACDEPLOYQT_EXECUTABLE})
      # Per https://cmake.org/cmake/help/latest/command/add_executable.html, "IMPORTED executables are useful for
      # convenient reference from commands like add_custom_command()".
      add_executable(Qt6::macdeployqt IMPORTED)
      set_target_properties(Qt6::macdeployqt PROPERTIES IMPORTED_LOCATION ${MACDEPLOYQT_EXECUTABLE})
   endif()

endif()

# Uncomment the following line to show what version of Qt is actually being used
message(STATUS "Using Qt version " ${Qt6Core_VERSION})

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
set(Boost_USE_STATIC_LIBS ON)
if(WIN32)
   find_package(Boost 1.79.0 REQUIRED)
elseif(APPLE)
   find_package(Boost 1.79.0 REQUIRED)
else()
   # Note that header-only libraries don't have a component
   find_package(Boost 1.79.0 REQUIRED COMPONENTS stacktrace_backtrace)
endif()
include_directories(${Boost_INCLUDE_DIRS})
# Uncomment the next two lines if you want to find where Boost headers and DLLs are on your system
message("Boost include directories: ${Boost_INCLUDE_DIRS}")
message("Boost libraries: ${Boost_LIBRARIES}")

#
# Extra requirements for Boost Stacktrace
#
# Per https://www.boost.org/doc/libs/1_76_0/doc/html/stacktrace/configuration_and_build.html, by default
# Boost.Stacktrace is a header-only library.  However, you get better results by linking (either statically or
# dynamically) with a helper library.  Of the various options, it seems like boost_stacktrace_backtrace gives the most
# functionality over the most platforms.  This has dependencies on:
#   - libdl on POSIX platforms -- but see note below
#   - libbacktrace
# The latter is an external library on Windows.  On POSIX plaforms it's typically already either installed on the system
# (eg see https://man7.org/linux/man-pages/man3/backtrace.3.html) or built in to the compiler.  Fortunately, CMake knows
# how to do the right thing in either case, thanks to https://cmake.org/cmake/help/latest/module/FindBacktrace.html.
#
# Just to make things extra fun, in 2021, the GNU compilers did away with libdl and incorporated its functionality into
# libc, per the announcement of GNU C Library v2.3.4 at
# https://sourceware.org/pipermail/libc-alpha/2021-August/129718.html.  This means, if we're using the GNU tool chain
# and libc is v2.3.4 or newer, then we should NOT look for libdl, as we won't find it!
#
# Fortunately, CMake has a special variable, CMAKE_DL_LIBS, that is, essentially "whatever library you need to link to
# for dlopen and dlclose", so we don't need to worry about libc versions.
#
if(NOT WIN32)
   set(DL_LIBRARY ${CMAKE_DL_LIBS})
endif()
find_package(Backtrace REQUIRED)
# For the moment, leave default settings for Mac as can't work out how to get the backtrace version of stacktrace
# working.  (Should still get stack traces on Mac, but might not get as much info in them as we'd like.)
if(NOT APPLE)
   if(NOT WIN32)
# TBD Some users report problems getting CMake to find libboost_stacktrace_backtrace on Ubuntu and Gentoo, so disable it
# for now and fallback to the header-only version
#      add_compile_definitions(BOOST_STACKTRACE_DYN_LINK)
   endif()
#   add_compile_definitions(BOOST_STACKTRACE_USE_BACKTRACE)
endif()
message("Backtrace libs  ${DL_LIBRARY}  |  ${Backtrace_LIBRARIES}  |  ${Boost_LIBRARIES}")

# Defining BOOST_JSON_STANDALONE tells Boost.JSON to use std::string_view (requires C++17) rather than
# boost::string_view (part of Boost.Utility).  However, as of recent versions of Boost.JSON, this is "deprecated and
# will be removed in a future release of Boost.JSON".
#ADD_COMPILE_DEFINITIONS(BOOST_JSON_STANDALONE)

#=================================================== Find Xerces-C++ ===================================================
# CMake already knows how to find and configure Xerces-C++, see
# https://cmake.org/cmake/help/latest/module/FindXercesC.html
find_package(XercesC REQUIRED)
include_directories(${XercesC_INCLUDE_DIRS})
# Uncomment the next two lines if you want to find where Xerces headers and DLLs are on your system
message("Xerces-C++ include directories: ${XercesC_INCLUDE_DIRS}")
message("Xerces-C++ libraries: ${XercesC_LIBRARIES}")

#==================================================== Find Xalan-C++ ===================================================
# Same comments apply here as for Xerces above
find_package(XalanC REQUIRED)
include_directories(${XalanC_INCLUDE_DIRS})
# Uncomment the next two lines if you want to find where Xalan headers and DLLs are on your system
message("Xalan-C++ include directories: ${XalanC_INCLUDE_DIRS}")
message("Xalan-C++ libraries: ${XalanC_LIBRARIES}")

if(APPLE)
# TBD: Is this also needed when static linking Xerces on MacOS?
find_package(CURL REQUIRED)
endif()

#=======================================================Valijson========================================================
# Since Valijson is also hosted at github (https://github.com/tristanpenman/valijson) we just add it as a submodule
# (via git clone https://github.com/tristanpenman/valijson third-party/valijson).  Then the following also come from
# https://github.com/tristanpenman/valijson#readme
#
# Valijson is a header-only library, so we don't need to build or link to any libraries etc (unless you want to build
# its test suite - in which case, uncomment the add_subdirectory line and add ValiJSON::valijson to both lists of
# target_link_libraries in src/CMakeLists.txt).
#
# Previously we were using CMake ExternalProject_Add to pull Valijson in, but this is unnecessary (because we don't need
# to build Valijson) and somewhat harder to debug when it goes wrong.  So, instead, per the readme at
# https://github.com/tristanpenman/valijson, we just added ValiJSON as a git submodule from the command line.
#
message(STATUS "Valijson source at  ${CMAKE_CURRENT_SOURCE_DIR}/third-party/valijson")
# Per comment above, leave the next line commented out unless you want to build the ValiJSON test suite
#add_subdirectory(third-party/valijson)
add_compile_definitions(VALIJSON_USE_EXCEPTIONS)
include_directories(${ROOTDIR}/third-party/valijson/include)
execute_process(COMMAND git config --global --add safe.directory third-party/valijson
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
execute_process(COMMAND git submodule update --init --recursive
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

#========================================= Find MinGW (only needed on Windows) =========================================
if(WIN32)
   # This is to detect whether we're using MSYS2 and/or MinGW
   if(WIN32)
      execute_process(COMMAND uname OUTPUT_VARIABLE uname)
      message(STATUS "Uname is " ${uname})
      if(uname MATCHES "^MSYS" OR uname MATCHES "^MINGW")
         message(STATUS "Running on MSYS/MinGW")
         set(MINGW true)
      endif()
   endif()

   # Find extra MinGW-specific dlls.
   if(MINGW)
      if(NOT MINGW_BIN_DIR)
         # Yes, it's mingw32-make.exe even on 64-bit systems
         FIND_PATH(MINGW_BIN_DIR "mingw32-make.exe")
      endif()
      if(NOT EXISTS ${MINGW_BIN_DIR})
         message(FATAL_ERROR "MinGW bin dir not found. Run cmake again with the option -DMINGW_BIN_DIR=c:/path/to/mingw/bin")
      else()
         get_filename_component(Mingw_Path ${CMAKE_CXX_COMPILER} PATH)
      endif()
      message(STATUS "MINGW_BIN_DIR " ${MINGW_BIN_DIR})
   endif()
endif()

# Shows all the places we are looking for headers
message(STATUS "CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}")
message(STATUS "CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}")

include(InstallRequiredSystemLibraries)

#=======================================================================================================================
#=========================================== Generate config.h from config.in ==========================================
#=======================================================================================================================
# Taking src/config.in as input, we generate (in the build subdirectory only) config.h.  This is a way to inject CMake
# variables into the code.
#
# All variables written as "${VAR}" in config.in  will be replaced by the value of VAR in config.h.
# Eg "#define CONFIG_DATA_DIR ${CONFIG_DATA_DIR}" in config.in will be replaced by the below corresponding value in
# ${CONFIG_DATA_DIR} below when configure_file() is called.
#
set(CONFIG_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${installSubDir_data}/")
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S (UTC)" UTC)
configure_file(src/config.in src/config.h)

#=======================================================================================================================
#=============================================== Embedded Resource Files ===============================================
#=======================================================================================================================
# We don't need to list the embedded resource files themselves here, just the "Resource Collection File" that lists what
# they are.
set(filesToCompile_qrc "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")

#=======================================================================================================================
#=========================================== Files included with the program ===========================================
#=======================================================================================================================
# These are files that actually ship/install as real files, rather than Qt resources
#
# List of documentation files to be installed.  Note that ${repoDir}/COPYRIGHT is NOT included here as it needs special
# case handling below.
set(filesToInstall_docs ${repoDir}/README.md)

# List of data files to be installed.
set(filesToInstall_data ${repoDir}/data/default_db.sqlite
                        ${repoDir}/data/DefaultContent001-OriginalDefaultData.xml
                        ${repoDir}/data/DefaultContent002-BJCP_2021_Styles.json
                        ${repoDir}/data/DefaultContent003-Ingredients-Hops-Yeasts.json
                        ${repoDir}/data/DefaultContent004-MoreYeasts.json)

# Desktop files to install.
set(filesToInstall_desktop ${repoDir}/linux/${PROJECT_NAME}.desktop)

# Icon files to install.
set(filesToInstall_icons ${repoDir}/images/${PROJECT_NAME}.svg)

# This is the list of translation files to update (from translatable strings in the source code) and from which the
# binary .qm files will be generated and shipped.  Note that src/OptionDialog.cpp controls which languages are shown to
# the user as options for the UI
set(translationSourceFiles ${repoDir}/translations/bt_ca.ts # Catalan
                           ${repoDir}/translations/bt_cs.ts # Czech
                           ${repoDir}/translations/bt_de.ts # German
                           ${repoDir}/translations/bt_en.ts # English
                           ${repoDir}/translations/bt_el.ts # Greek
                           ${repoDir}/translations/bt_es.ts # Spanish
                           ${repoDir}/translations/bt_et.ts # Estonian
                           ${repoDir}/translations/bt_eu.ts # Basque
                           ${repoDir}/translations/bt_fr.ts # French
                           ${repoDir}/translations/bt_gl.ts # Galician
                           ${repoDir}/translations/bt_nb.ts # Norwegian Bokmal
                           ${repoDir}/translations/bt_it.ts # Italian
                           ${repoDir}/translations/bt_lv.ts # Latvian
                           ${repoDir}/translations/bt_nl.ts # Dutch
                           ${repoDir}/translations/bt_pl.ts # Polish
                           ${repoDir}/translations/bt_pt.ts # Portuguese
                           ${repoDir}/translations/bt_hu.ts # Hungarian
                           ${repoDir}/translations/bt_ru.ts # Russian
                           ${repoDir}/translations/bt_sr.ts # Serbian
                           ${repoDir}/translations/bt_sv.ts # Swedish
                           ${repoDir}/translations/bt_tr.ts # Turkish
                           ${repoDir}/translations/bt_zh.ts) # Chinese

set(filesToInstall_windowsIcon ${repoDir}/win/icon.rc)

set(filesToInstall_sounds ${repoDir}/data/sounds/45minLeft.wav
                          ${repoDir}/data/sounds/addFuckinHops.wav
                          ${repoDir}/data/sounds/aromaHops.wav
                          ${repoDir}/data/sounds/beep.wav
                          ${repoDir}/data/sounds/bitteringHops.wav
                          ${repoDir}/data/sounds/checkBoil.wav
                          ${repoDir}/data/sounds/checkFirstRunnings.wav
                          ${repoDir}/data/sounds/checkGravity.wav
                          ${repoDir}/data/sounds/checkHydrometer.wav
                          ${repoDir}/data/sounds/checkMashTemps.wav
                          ${repoDir}/data/sounds/checkTemp.wav
                          ${repoDir}/data/sounds/clarifyingAgent.wav
                          ${repoDir}/data/sounds/cleanup.wav
                          ${repoDir}/data/sounds/closeFuckinValves.wav
                          ${repoDir}/data/sounds/closeValves.wav
                          ${repoDir}/data/sounds/doughIn.wav
                          ${repoDir}/data/sounds/drinkAnotherHomebrew.wav
                          ${repoDir}/data/sounds/drinkHomebrew.wav
                          ${repoDir}/data/sounds/emptyMashTun.wav
                          ${repoDir}/data/sounds/extraPropane.wav
                          ${repoDir}/data/sounds/flameout.wav
                          ${repoDir}/data/sounds/flavorHops.wav
                          ${repoDir}/data/sounds/heatWater.wav
                          ${repoDir}/data/sounds/mashHops.wav
                          ${repoDir}/data/sounds/pitchYeast.wav
                          ${repoDir}/data/sounds/sanitize.wav
                          ${repoDir}/data/sounds/sparge.wav
                          ${repoDir}/data/sounds/startBurner.wav
                          ${repoDir}/data/sounds/startChill.wav
                          ${repoDir}/data/sounds/stirMash.wav)

# We mostly don't need to explicitly specify the .ui files because AUTOUIC will find them all for us.  However, I
# haven't found a way to connect AUTOUIC with the translation stuff below, so grab a list of all the .ui files here for
# that.
file(GLOB_RECURSE filesToCompile_ui "${repoDir}/ui/*.ui")

set(filesToInstall_macPropertyList "${repoDir}/mac/Info.plist")

set(filesToInstall_macIcons "${repoDir}/mac/BrewtargetIcon.icns")

set(filesToInstall_changeLogUncompressed "${repoDir}/CHANGES.markdown")

# See below for how this one gets created from filesToInstall_changeLogUncompressed
set(filesToInstall_changeLogCompressed "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")

#=======================================================================================================================
#=========================================== Process other CMakeList.txt files =========================================
#=======================================================================================================================
# We try to restrict src/CMakeLists.txt to just holding lists of source files,
# otherwise the dependencies and interactions between those files and this one get a bit hard to follow.
include(src/CMakeLists.txt)

#=======================================================================================================================
#==================================================== Translations =====================================================
#=======================================================================================================================
#
# We need to do two processes with Translation Source (.ts) XML files:
#   - Update them from the source code, ie to ensure they have all the tr(), QObject::tr() etc calls from the .cpp files
#     and all the translatable strings from the .ui files -- which can be done manually from the command line with
#     lupdate
#   - Generate the binary .qm files that ship with the application and are used at run time -- which can be done
#     manually from the command line with lrelease
#
# Getting both these things done in Qt5 was a bit complicated as qt_add_translation() _only_ does the latter.  But,
# with Qt6, we now have qt_add_lupdate that does the former.  NOTE that we will want to tweak the syntax of
# qt_add_lupdate once we are on Qt 6.7 -- per https://doc.qt.io/qt-6/qtlinguist-cmake-qt-add-lupdate.html -- because a
# change is introduced with that release and the syntax used here then becomes deprecated.
#
qt_add_translation(QM_FILES ${translationSourceFiles})

# Add a target for the QM_FILES so that we can add the translations as a dependency for the executable later.
add_custom_target(translationsTarget DEPENDS ${QM_FILES})
qt_add_lupdate(translationsTarget TS_FILES ${repoDir}/src -ts ${translationSourceFiles} SOURCES ${filesToCompile_cpp} ${filesToCompile_ui})

#============================Icon for Windows==================================

set(desktopIcon "")

if(WIN32 AND MINGW)
  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/src/icon.o
                     COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                     -i${filesToInstall_windowsIcon}
                     -o${CMAKE_BINARY_DIR}/src/icon.o
                     DEPENDS ${filesToInstall_windowsIcon}
  )
  set(desktopIcon ${CMAKE_BINARY_DIR}/src/icon.o)
elseif(WIN32)
  set(desktopIcon ${filesToInstall_windowsIcon})
endif()

#===========================File ownership==================================
#
# When you do "make install", the last thing CMake does is generate a file called install_manifest.txt in the build
# output directory containing a list of all the files that were installed.  On Linux, since we need to run make install
# as root (eg via "sudo make install"), this file would get created with root:root ownership.  That's a problem because
# you then when you run "make package" as a non-root user, you get an error "file failed to open for writing (Permission
# denied)".  The workaround is to create the install_manifest.txt file as a normal user when "make" is run, so that
# "sudo make install" is just updating an existing file rather than creating it from scratch.
#
file(TOUCH ${CMAKE_BINARY_DIR}/install_manifest.txt)

#===========================Create the binary==================================

# This intermediate library target simplifies building the main app and the test app from largely the same sources
# Note that using this is why we can't include src/main.cpp in filesToCompile_cpp.
add_library(btobjlib
            OBJECT
            ${filesToCompile_cpp}
            ${filesToCompile_qrc})



if(APPLE)
   #
   # We have to tell CMake what things other than the executable etc to include in the Mac Applicaiton Bundle
   #
   set_source_files_properties(${filesToInstall_macIcons}
                              PROPERTIES
                              MACOSX_PACKAGE_LOCATION "Resources")
   set_source_files_properties(${filesToInstall_data}
                              PROPERTIES
                              MACOSX_PACKAGE_LOCATION "Resources")
   set_source_files_properties(${filesToInstall_docs}
                              PROPERTIES
                              MACOSX_PACKAGE_LOCATION "Resources/en.lproj")
   set_source_files_properties(${filesToInstall_sounds}
                              PROPERTIES
                              MACOSX_PACKAGE_LOCATION "Resources/sounds")
   set_source_files_properties(${QM_FILES}
                              PROPERTIES
                              MACOSX_PACKAGE_LOCATION "Resources/translations_qm")

   # The MACOSX_BUNDLE parameter here sets the MACOSX_BUNDLE property on the created target.  This means the executable
   # is built as an Application Bundle, which makes it a GUI executable that can be launched from the Finder.
   #
   # TBD: When we move to Qt6, look at qt_add_executable
   add_executable(${fileName_executable}
                  MACOSX_BUNDLE
                  ${repoDir}/src/main.cpp
                  ${translationSourceFiles}
                  ${QM_FILES}
                  ${filesToInstall_macIcons}
                  ${filesToInstall_data}
                  ${filesToInstall_docs}
                  ${filesToInstall_sounds}
                  $<TARGET_OBJECTS:btobjlib>)

   #====================================================================================================================
   #================================================ Mac Bundle Settings ===============================================
   #====================================================================================================================
   # See https://cmake.org/cmake/help/latest/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html for the CMake doco and
   # https://developer.apple.com/documentation/bundleresources/information_property_list/bundle_configuration for the
   # Apple documentation
   #

   # Sets the CFBundleName bundle property.  "This name can contain up to 15 characters. The system may display it to
   # users if CFBundleDisplayName isn't set."  (I can't see a way in CMake for us to set CFBundleDisplayName.)
   set(MACOSX_BUNDLE_BUNDLE_NAME "${capitalisedProjectName}")

   # Sets the CFBundleIdentifier bundle property which is a case-insensitive string that "uniquely identifies a single
   # app throughout the system ... Typically ... a reverse-DNS format".  It is used when "applying specified
   # preferences" (whatever that means), to "locate an app capable of opening a particular file", and for validating an
   # app's signature.
   set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.${PROJECT_NAME}.${fileName_executable}")

   # We do not set MACOSX_BUNDLE_INFO_STRING, which sets the CFBundleGetInfoString bundle property, as AFAICT
   # CFBundleGetInfoString is obsolete.

   # Sets the CFBundleVersion, which is "a machine-readable string composed of one to three period-separated integers,
   # such as 10.14.1. The string can only contain numeric characters (0-9) and periods."
   set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

   #
   # This sets the bundle property key CFBundleShortVersionString, which is "a user-visible string for the version of
   # the bundle. The required format is three period-separated integers, such as 10.14.1. The string can only contain
   # numeric characters (0-9) and periods."
   #
   # One might think that CFBundleShortVersionString and CFBundleVersion are so similar as not to merit being separate
   # keys, but it is not for us to question whether the left hand of Apple knows what the right hand is doing.
   #
   # Confusingly there is also a CMake target property called MACOSX_BUNDLE_LONG_VERSION_STRING which claims to set a
   # CFBundleLongVersionString bundle property key.  However, it seems this bundle property key is long obsolete.
   #
   set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})

   # Sets the CFBundleIconFile bundle property key
   set(MACOSX_BUNDLE_ICON_FILE "Brewtarget.icns")

   # Sets the NSHumanReadableCopyright bundle property key, which is "a human-readable copyright notice for the bundle".
   set(MACOSX_BUNDLE_COPYRIGHT
       "Copyright 2009-2024.  Distributed under the terms of the GNU General Public License (version 3).")

else()
   add_executable(${fileName_executable}
                  ${repoDir}/src/main.cpp
                  ${translationSourceFiles}
                  ${QM_FILES}
                  ${desktopIcon}
                  $<TARGET_OBJECTS:btobjlib>)
endif()

#=======================================================================================================================
#=======================================================================================================================
# Windows-specific library linking
if(WIN32 AND MINGW)
   ############################################################################
   # Need to set some linker flags that I don't know how to get
   # automatically.
   ############################################################################

   # MinGW-specific flags.
   #    -Wl,-subsystem,windows - suppresses the output command window.
   #    -Wl,-enable-stdcall-fixup - If the link finds a symbol that it cannot resolve, it will attempt to do “fuzzy
   #                                linking” by looking for another defined symbol that differs only in the format of
   #                                the symbol name (cdecl vs stdcall) and will resolve that symbol by linking to the
   #                                match (and also print a warning).
   #    -Wl,-enable-auto-import - Do sophisticated linking of _symbol to __imp__symbol for DATA imports from DLLs, and
   #                              create the necessary thunking symbols when building the import libraries with those
   #                              DATA exports.
   #    -Wl,-enable-runtime-pseudo-reloc - Fixes some of the problems that can occur with -enable-auto-import
   #    -mthreads - specifies that MinGW-specific thread support is to be used
   set_target_properties(
      ${fileName_executable}
      PROPERTIES
      LINK_FLAGS "-Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc -mthreads -Wl,-subsystem,windows"
   )

endif()

add_dependencies(${fileName_executable} translationsTarget)

# All the libraries (except the Qt ones we add immediately below) that are used by both the main app and the unit
# testing app
set(appAndTestCommonLibraries
   ${Backtrace_LIBRARIES}
   ${Boost_LIBRARIES}
   ${DL_LIBRARY}
   ${XalanC_LIBRARIES}
   ${XercesC_LIBRARIES}
)
if(APPLE)
   # Static linking Xerces and Xalan on MacOS means we have to explicitly say what libraries and frameworks they in turn
   # depend on.  It would be neat to find some automated tool that does this for us.
   list(APPEND appAndTestCommonLibraries CURL::libcurl
                                         "-framework CoreFoundation"
                                         "-framework CoreServices"
                                         "-framework Carbon"
                                         "-framework Foundation"
                                         "-framework Cocoa"
                                         "-framework ApplicationServices")
endif()
foreach(qtComponent IN LISTS qtCommonComponents)
   list(APPEND appAndTestCommonLibraries "Qt6::${qtComponent}")
endforeach()
message("appAndTestCommonLibraries: ${appAndTestCommonLibraries}")
target_link_libraries(${fileName_executable} ${appAndTestCommonLibraries})

#=================================Tests========================================
# We used to build the unit test executable in the bin subdirectory because, on Windows, we're going to copy a lot of
# other files in there (see below).  HOWEVER, we've never done that on the Meson build and it's a pain in the neck to
# do things differently on the CMake build: when we're running the unit tests because of the way we look for the data
# directory (see initResourceDir() function in Application.cpp).
add_executable(${fileName_unitTestRunner}
               ${repoDir}/src/unitTests/Testing.cpp
               $<TARGET_OBJECTS:btobjlib>)
#set_target_properties(${fileName_unitTestRunner} PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)

# Test app needs all the same libraries as the main app, plus Qt6::Test
target_link_libraries(${fileName_unitTestRunner} ${appAndTestCommonLibraries} Qt6::Test)

message("Unit Test Runner: ./${fileName_unitTestRunner}")

add_test(NAME pstdintTest                 COMMAND ./${fileName_unitTestRunner} pstdintTest                )
add_test(NAME recipeCalcTest_allGrain     COMMAND ./${fileName_unitTestRunner} recipeCalcTest_allGrain    )
add_test(NAME postBoilLossOgTest          COMMAND ./${fileName_unitTestRunner} postBoilLossOgTest         )
add_test(NAME testUnitConversions         COMMAND ./${fileName_unitTestRunner} testUnitConversions        )
add_test(NAME testNamedParameterBundle    COMMAND ./${fileName_unitTestRunner} testNamedParameterBundle   )
add_test(NAME testNumberDisplayAndParsing COMMAND ./${fileName_unitTestRunner} testNumberDisplayAndParsing)
add_test(NAME testAlgorithms              COMMAND ./${fileName_unitTestRunner} testAlgorithms             )
add_test(NAME testTypeLookups             COMMAND ./${fileName_unitTestRunner} testTypeLookups            )
add_test(NAME testInventory               COMMAND ./${fileName_unitTestRunner} testInventory              )
add_test(NAME testLogRotation             COMMAND ./${fileName_unitTestRunner} testLogRotation            )

#=================================Installs=====================================

# Install executable.
install(TARGETS ${fileName_executable}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${installSubDir_bin}
        COMPONENT ${RUNTIME_INSTALL_COMPONENT})

# Install the translations.
install(FILES ${QM_FILES}
        DESTINATION "${installSubDir_data}/translations_qm"
        COMPONENT ${DATA_INSTALL_COMPONENT} )

#=======================================================================================================================
#================================================== Install (locally) ==================================================
#=======================================================================================================================
# This is for "make install" (or "sudo make install")
#
# When a relative path is given in the DESTINATION option of the install() command, it is interpreted relative to the
# value of the CMAKE_INSTALL_PREFIX variable.
#
# Install the data
install(FILES ${filesToInstall_data}
        DESTINATION ${installSubDir_data}
        COMPONENT ${DATA_INSTALL_COMPONENT})

# Install the documentation
install(FILES ${filesToInstall_docs}
        DESTINATION ${installSubDir_doc}
        COMPONENT ${DATA_INSTALL_COMPONENT})

# Install sounds
install(FILES ${filesToInstall_sounds}
        DESTINATION "${installSubDir_data}/sounds"
        COMPONENT ${DATA_INSTALL_COMPONENT})

if(UNIX AND NOT APPLE)
   #----------- Linux -----------
   # Install the icons
   # Per https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html#install_icons, "installing a
   # svg icon in $prefix/share/icons/hicolor/scalable/apps means most desktops will have one icon that works for all
   # sizes".
   install(FILES ${filesToInstall_icons}
           DESTINATION "${installSubDir_icons}/hicolor/scalable/apps/"
           COMPONENT ${DATA_INSTALL_COMPONENT})

   # Install the .desktop file
   install(FILES ${filesToInstall_desktop}
           DESTINATION "${installSubDir_applications}"
           COMPONENT ${DATA_INSTALL_COMPONENT})
endif()

if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
   install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
           DESTINATION bin
           COMPONENT System)
endif()

#=======================================================================================================================
#=============================================== Post-compilation message ==============================================
#=======================================================================================================================
#
# After running `make`, you usually want to run `make install`.  This is an easy step to miss for someone compiling from
# source for the first time, so it's common practice to give a reminder about it.  This trick of making a special cmake
# target comes from https://stackoverflow.com/questions/25240105/how-to-print-messages-after-make-done-with-cmake.  It
# does mean the message also gets printed during `sudo make install` which is, at best, unnecessary.  But I think, on
# balance, it's better than not having a message.
#
# Putting the message in a different color is usually helpful because it makes it stand out from all the other CMake
# output.  Hard-coding the color here is a small risk in that we don't know how well it will show up on whatever color
# scheme the terminal is using.  OTOH, AIUI the color of CMake's own messages is hard-coded, so, if we use one of the
# same colors, we're not making things any worse.
# (BTW, I cannot find reference to cmake_echo_color in the CMake documentation, but it seems to work!)
#
add_custom_target(
   FinalMessage ALL
   ${CMAKE_COMMAND} -E cmake_echo_color --bold --green "⭐⭐⭐ Finished compiling ${capitalisedProjectName}.  Please run  sudo make install  to install locally. ⭐⭐⭐"
   COMMENT "Final Message"
)
add_dependencies(FinalMessage ${fileName_executable})

#=======================================================================================================================
#================================================= Custom Make Targets =================================================
#=======================================================================================================================
# These go at the end of the file so that they can use any of the variables created above

# `make install-data` or `make install-runtime`
add_custom_target(
   install-data
   COMMAND "${CMAKE_COMMAND}"
           -DCOMPONENT=${DATA_INSTALL_COMPONENT}
           -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(
   install-runtime
   DEPENDS ${fileName_executable}
   COMMAND "${CMAKE_COMMAND}"
           -DCOMPONENT=${RUNTIME_INSTALL_COMPONENT}
           -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)

# Doxygen Custom Target
FIND_PROGRAM(DOXYGEN_CMD doxygen)
if(DOXYGEN_CMD)
  set(DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile")
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.cmake.in" ${DOXYFILE})
  add_custom_target(source_doc
                     COMMAND ${DOXYGEN_CMD} ${DOXYFILE}
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
                  )
endif()

# Some extra files for the "make clean" target
# Note that the ADDITIONAL_CLEAN_FILES property does NOT do any sort of glob pattern matching, so we have to use the
# file command to do that.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             APPEND
             PROPERTY ADDITIONAL_CLEAN_FILES ".*~$" # Kate backup files.
                                             "CMakeLists.txt.user" # From QtCreator I think.
                                             )
file(GLOB packagesDebFiles      "${CMAKE_CURRENT_BINARY_DIR}/*.deb")
file(GLOB packagesRpmFiles      "${CMAKE_CURRENT_BINARY_DIR}/*.rpm")
file(GLOB packagesTarBz2Files   "${CMAKE_CURRENT_BINARY_DIR}/*.tar.bz2")
file(GLOB packagesChecksumFiles "${CMAKE_CURRENT_BINARY_DIR}/*.*.sha256")
set_property(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
             APPEND
             PROPERTY ADDITIONAL_CLEAN_FILES "install_manifest.txt"
                                             ${packagesDebFiles}
                                             ${packagesRpmFiles}
                                             ${packagesTarBz2Files}
                                             ${packagesChecksumFiles})
