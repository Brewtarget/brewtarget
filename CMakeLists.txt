#
# CMakeLists.txt is part of Brewtarget, and is Copyright the following
# authors 2009-2022
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
#

#
# Creates a Makefile in the build directory, from where you can do builds and installs.
#
# NOTE: cmake . -DCMAKE_INSTALL_PREFIX=/tmp/blah && make DESTDIR=/foo
# will install to /foo/tmp/blah.
#
# See also - src/CMakeLists.txt
#          - QtDesignerPlugins/CMakeLists.txt
#
# Standard make targets:
#   * make                  -  Compile the source code
#   * make clean            -  Delete compiled objects so next build starts from scratch
#   * sudo make install     -  Install locally
#   * make package          -  Makes .deb, .rpm, NSIS Installer, and .tar.bz2 binary packages.
#   * make package_source   -  Makes a .tar.bz2 source package.
#
# Custom make targets:
#   * make source_doc       -  Makes html documentation of the source in doc/html.
#   * make changelog
#   * make install-data
#   * make install-runtime
#   * make package_lint     -  Runs lintian and rpmlint on packages
#

#
# Uncomment the next line for slightly verbose build output
# Alternatively, for very verbose output, invoke make as follows:
#    $ make VERBOSE=1
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

#=======================================================================================================================
#============================================== Project name and Version ===============================================
#=======================================================================================================================
project(brewtarget VERSION 2.4.0 LANGUAGES CXX)
message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION}")
message(STATUS "PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}")

#=======================================================================================================================
#======================================================= Options =======================================================
#=======================================================================================================================
OPTION(BUILD_DESIGNER_PLUGINS
        "If on, you will only build and install the designer plugins."
        OFF)
OPTION(DO_RELEASE_BUILD
        "If on, will do a release build. Otherwise, debug build."
        OFF)
OPTION(NO_MESSING_WITH_FLAGS
        "On means do not add any build flags whatsoever. May override other options."
        OFF)

#=======================================================================================================================
#===================================================== Directories =====================================================
#=======================================================================================================================

#================================================== Source directories =================================================
#
# ${repoDir}                   = the directory containing this (CMakeLists.txt) file
# ${repoDir}/src               = C++ source code (with the exception of Qt Designer Plugins)
# ${repoDir}/QtDesignerPlugins = Qt Designer Plugins
# ${repoDir}/ui                = QML UI layout files
# ${repoDir}/data              = Binary files, including sounds and default database
# ${repoDir}/translations      = Translated texts
# ${repoDir}/mac               = Mac-specific files (desktop icon)
# ${repoDir}/win               = Windows-specific files (desktop icon)
# ${repoDir}/packaging         = Packaging-related config
#
set(repoDir "${CMAKE_CURRENT_SOURCE_DIR}")

# AFAICT this is used only in cmake/modules/NSIS.template.in
set(WINDIR "${repoDir}/win")

# Location of custom CMake modules. (At the moment, there is only one, which is used for Windows packaging
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

#=================================================== Build directory ===================================================
set(buildDir "${CMAKE_CURRENT_BINARY_DIR}")

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
   # On Debian and derived systems (such as Ubuntu), it's more normal for apps to go in /usr/bin than /usr/local/bin, so
   # CMAKE_INSTALL_PREFIX can be overridden on the command line via "--prefix usr"
   #
   # .:TBD: We also allow -DEXEC_PREFIX=/usr (used in .github/workflows/linux-ubuntu.yml) but I'm not sure if this is
   #        needed or could be replaced by "--prefix usr"
   #
   # Debian standard directories.
   if(NOT EXEC_PREFIX)
      set(EXEC_PREFIX ${CMAKE_INSTALL_PREFIX})
   endif()

   set(installSubDir_data "share/${CMAKE_PROJECT_NAME}")
   set(installSubDir_doc  "share/doc/${CMAKE_PROJECT_NAME}")
   set(installSubDir_bin  "bin")

elseif(WIN32)
   #============================================ Windows Install Directories ===========================================

   # For some damn reason, for the NSIS installer,
   # the prefix needs to be empty. Also, seems that the .exe
   # needs to be in bin/. Fucking piece of shit CPack...
   # Can anybody shed some light on this situation?
   #SET(CMAKE_INSTALL_PREFIX "")
   SET(CPACK_INSTALL_PREFIX "")

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
   set(fileName_executable "Brewtarget")
else()
   set(fileName_executable "brewtarget")
endif()

#=======================================================================================================================
#=================================================== General Settings ==================================================
#=======================================================================================================================
# This is needed to enable the add_test() command (used in src/CMakeLists.txt)
ENABLE_TESTING()

#=======================================================================================================================
#=============================================== Installation Components ===============================================
#=======================================================================================================================
# For architecture-independent data
set(DATA_INSTALL_COMPONENT "Data")
# For architecture-dependent binaries
set(RUNTIME_INSTALL_COMPONENT "Runtime")

#=======================================================================================================================
#================================================= Mac Bundle Settings =================================================
#=======================================================================================================================
SET(MACOSX_BUNDLE_BUNDLE_NAME "Brewtarget")
#SET(MACOSX_BUNDLE_GUI_IDENTIFIER)
#SET(MACOSX_BUNDLE_INFO_STRING)
#SET(MACOSX_BUNDLE_BUNDLE_VERSION)
#SET(MACOSX_BUNDLE_SHORT_VERSION_STRING)
SET(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
SET(MACOSX_BUNDLE_ICON_FILE "BrewtargetIcon.icns")
SET(MACOSX_BUNDLE_COPYRIGHT "GPLv3")

#=======================================================================================================================
#============================================== Compiler settings & flags ==============================================
#=======================================================================================================================
# We use different compilers on different platforms.  Where possible, we want to let CMake handle the actual compiler
# settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# We need C++17 or later for nested namespaces (and C++11 or later for lambdas)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${repoDir}/src)
include_directories("${CMAKE_BINARY_DIR}/src") # In case of out-of-source build.
include_directories("${CMAKE_BINARY_DIR}/QtDesignerPlugins")

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
      # packaging/rpmLintFilters.toml).
      #
      set(CMAKE_CXX_FLAGS_RELEASE "-Wall -ansi -pedantic -Wno-long-long -O2 -z noexecstack")
      set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g3")
   endif()

   # Speed up compilation if using gcc.
   if(UNIX AND NOT APPLE)
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe")
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pipe")
   endif()
endif()

# Windows-specific library linking
if(WIN32 AND MINGW)
   ############################################################################
   # Need to set some linker flags that I don't know how to get
   # automatically.
   ############################################################################

   # MinGW-specific flags.
   #    '-Wl,-subsystem,windows' supresses the output command window.
   #    '-Wl,-s' strips the executable of symbols.
   SET_TARGET_PROPERTIES( ${fileName_executable} PROPERTIES LINK_FLAGS "-Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc -mthreads -Wl,-subsystem,windows")

   if(TARGET Qt5::windeployqt)
      # execute windeployqt in a tmp directory after build
      add_custom_command(TARGET ${fileName_executable}
         POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
         COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" --no-translations --compiler-runtime "$<TARGET_FILE_DIR:${fileName_executable}>/$<TARGET_FILE_NAME:${fileName_executable}>"
   )

   # copy deployment directory during installation
   install(
      DIRECTORY
      "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
      DESTINATION bin
   )
   endif()
endif()

# Mac-specific compilation settings
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
   set(CMAKE_OSX_ARCHITECTURES x86_64) # Build intel 64-bit binary.
   #set(CMAKE_OSX_ARCHITECTURES i386 x86_64) # Build intel binary.
   #set(CMAKE_OSX_ARCHITECTURES ppc i386 ppc64 x86_64) # Build universal binary.
endif()

if(APPLE)
   # As explained at https://stackoverflow.com/questions/5582211/what-does-define-gnu-source-imply, defining _GNU_SOURCE
   # gives access to various non-standard GNU/Linux extension functions and changes the behaviour of some POSIX
   # functions.
   #
   # .:TBD:. Not sure exactly why we need this set on Mac builds
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
   SET(_gch_filename "${CMAKE_CURRENT_BINARY_DIR}/${_header_filename}.gch")
   SET(_header "${CMAKE_CURRENT_SOURCE_DIR}/${_header_filename}")

   LIST(APPEND ${_src_list} ${_gch_filename})

   SET (_args ${CMAKE_CXX_FLAGS})
   LIST(APPEND _args -c ${_header} -o ${_gch_filename})

   GET_DIRECTORY_PROPERTY(DIRINC INCLUDE_DIRECTORIES)
   FOREACH(_inc ${DIRINC})
      LIST(APPEND _args "-I" ${_inc})
   ENDFOREACH(_inc ${DIRINC})

   SEPARATE_ARGUMENTS(_args)

   ADD_CUSTOM_COMMAND(OUTPUT ${_gch_filename}
                      COMMAND rm -f ${_gch_filename}
                      COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_args}
                      DEPENDS ${_header})
ENDMACRO(ADD_PCH_RULE _src_list _header_filename)

#if( UNIX AND NOT APPLE )
#   SET( precompiled_h "equipment.h" "fermentable.h" "hop.h" "instruction.h" "mash.h" "mashstep.h" "misc.h" "recipe.h" "style.h" "unit.h" "water.h" "yeast.h" "database.h" "brewtarget.h" )
#   FOREACH( header ${precompiled_h} )
#      ADD_PCH_RULE( ${header} brewtarget_SRCS )
#   ENDFOREACH()
#endif()


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
endif()

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
# For the moment, max version we can have here is 5.9.5, because that's what Ubuntu 18.04 topped out at
#
SET(QT5_MIN_VERSION 5.9.5)

# Automatically run moc on source files when necessary
SET(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

FIND_PACKAGE(Qt5Core ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Widgets ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Network ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Network_INCLUDE_DIRS})

FIND_PACKAGE(Qt5PrintSupport ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5PrintSupport_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Sql ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Sql_INCLUDE_DIRS})

# This is required to make the deploy scripts pick up the svg plugins
FIND_PACKAGE(Qt5Svg ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Svg_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Xml ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Xml_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Multimedia ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Multimedia_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Test ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Test_INCLUDE_DIRS})

FIND_PACKAGE(Qt5LinguistTools ${QT5_MIN_VERSION} REQUIRED)
INCLUDE_DIRECTORIES(${Qt5LinguistTools_INCLUDE_DIRS})

# Fuckin Qt5 requires -fPIC if Qt5 itself was built with -fPIC
if(Qt5_POSITION_INDEPENDENT_CODE)
   SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# There's apparently a whole bunch of extra work we need to do to use Qt on Windows
if(WIN32)
   find_package(Qt5MultimediaWidgets REQUIRED)
   find_package(Qt5OpenGL REQUIRED)

   get_target_property(QtCore_location Qt5::Core LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtGui_location Qt5::Gui LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtMultimedia_location Qt5::Multimedia LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtMultimediaWidgets_location Qt5::MultimediaWidgets LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtNetwork_location Qt5::Network LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtOpenGL_location Qt5::OpenGL LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtPrintSupport_location Qt5::PrintSupport LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtSql_location Qt5::Sql LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtSvg_location Qt5::Svg LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtWidgets_location Qt5::Widgets LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtXml_location Qt5::Xml LOCATION_${CMAKE_BUILD_TYPE})

   get_target_property(QtSqliteDriver_location Qt5::QSQLiteDriverPlugin LOCATION_${CMAKE_BUILD_TYPE})

   get_target_property(QtQgif_location Qt5::QGifPlugin LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQico_location Qt5::QICOPlugin LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQjpeg_location Qt5::QJpegPlugin LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQsvg_location Qt5::QSvgPlugin LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQtiff_location Qt5::QTiffPlugin LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQWindows_location Qt5::QWindowsIntegrationPlugin LOCATION_${CMAKE_BUILD_TYPE})

   get_target_property(QtQsvgIcon_location Qt5::QSvgIconPlugin LOCATION_${CMAKE_BUILD_TYPE})

   set(Qt_DLLs            ${QtCore_location}
                          ${QtGui_location}
                          ${QtMultimedia_location}
                          ${QtMultimediaWidgets_location}
                          ${QtNetwork_location}
                          ${QtOpenGL_location}
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

   get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
   get_filename_component(QT5_BIN_DIR "${_qmake_executable}" DIRECTORY)
   FIND_PROGRAM(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT5_BIN_DIR}")
   if(EXISTS ${WINDEPLOYQT_EXECUTABLE})
      add_executable(Qt5::windeployqt IMPORTED)
      set_target_properties(Qt5::windeployqt PROPERTIES IMPORTED_LOCATION ${WINDEPLOYQT_EXECUTABLE})
   endif()

   file(GLOB IcuDlls "${QT5_BIN_DIR}/ic*.dll")

endif()

# Uncomment the following line to show what version of Qt is actually being used
message(STATUS "Using Qt version " ${Qt5Core_VERSION})

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
set(Boost_USE_STATIC_LIBS ON)
if(WIN32)
FIND_PACKAGE(Boost REQUIRED)
ELSEif(APPLE)
FIND_PACKAGE(Boost REQUIRED)
ELSE()
# TBD Some users report problems getting CMake to find libboost_stacktrace_backtrace on Ubuntu and Gentoo, so disable it
# for now and fallback to the header-only version
#FIND_PACKAGE(Boost REQUIRED COMPONENTS stacktrace_backtrace)
FIND_PACKAGE(Boost REQUIRED)
endif()
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
# Uncomment the next two lines if you want to find where Boost headers and DLLs are on your system
#message("Boost include directories: " ${Boost_INCLUDE_DIRS})
#message("Boost libraries: " ${Boost_LIBRARIES})

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
message("Backtrace libs " ${DL_LIBRARY} " | " ${Backtrace_LIBRARIES} " | " ${Boost_LIBRARIES})

#=================================================== Find Xerces-C++ ===================================================
# CMake already knows how to find and configure Xerces-C++, see
# https://cmake.org/cmake/help/latest/module/FindXercesC.html
FIND_PACKAGE(XercesC REQUIRED)
INCLUDE_DIRECTORIES(${XercesC_INCLUDE_DIRS})
# Uncomment the next two lines if you want to find where Xerces headers and DLLs are on your system
#message("Xerces-C++ include directories: " ${XercesC_INCLUDE_DIRS})
#message("Xerces-C++ libraries: " ${XercesC_LIBRARIES})

#==================================================== Find Xalan-C++ ===================================================
# Same comments apply here as for Xerces above
FIND_PACKAGE(XalanC REQUIRED)
INCLUDE_DIRECTORIES(${XalanC_INCLUDE_DIRS})
# Uncomment the next two lines if you want to find where Xalan headers and DLLs are on your system
#message("Xalan-C++ include directories: " ${XalanC_INCLUDE_DIRS})
#message("Xalan-C++ libraries: " ${XalanC_LIBRARIES})

# Shows all the places we are looking for headers
message(STATUS "CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}")
message(STATUS "CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}")

#========================================= Find MinGW (only needed on Windows) =========================================
if(WIN32)

   # Find extra MinGW-specific dlls.
   if(MINGW)
      if(NOT MINGW_BIN_DIR)
         FIND_PATH(MINGW_BIN_DIR "mingw32-make.exe")
      endif()
      if(NOT EXISTS ${MINGW_BIN_DIR})
         MESSAGE(FATAL_ERROR "MinGW bin dir not found. Run cmake again with the option -DMINGW_BIN_DIR=c:/path/to/mingw/bin")
      ELSE()
         get_filename_component(Mingw_Path ${CMAKE_CXX_COMPILER} PATH)
      endif()
   endif()
endif()

include(InstallRequiredSystemLibraries)

#=======================================================================================================================
#=========================================== Generate config.h from config.in ==========================================
#=======================================================================================================================
# Taking src/config.in as input, we generate (in the build subdirectory only) config.h.  This is a way to inject CMake
# variables into the code.
#
# All variables written as "${VAR}" in config.in  will be replaced by the value of VAR in config.h.
# Eg "#define CONFIGDATADIR ${CONFIGDATADIR}" in config.in will be replaced by the below corresponding value in
# ${CONFIGDATADIR} below when configure_file() is called.
#
set(CONFIGDATADIR "${CMAKE_INSTALL_PREFIX}/${installSubDir_data}/")
set(CONFIGDOCDIR "${CMAKE_INSTALL_PREFIX}/${installSubDir_doc}/")
configure_file(src/config.in src/config.h)

#=======================================================================================================================
#=============================================== Embedded Resource Files ===============================================
#=======================================================================================================================
# We don't need to list the embedded resource files themselves here, just the "Resource Collection File" that lists what
# they are.
set(resourceCollectionFile "${CMAKE_CURRENT_SOURCE_DIR}/brewtarget.qrc")

#=======================================================================================================================
#=========================================== Files included with the program ===========================================
#=======================================================================================================================
# These are files that actually ship/install as real files, rather than Qt resources
#
# List of documentation files to be installed.  Note that ${repoDir}/COPYRIGHT is NOT included here as it needs special
# case handling below.
set(filesToInstall_docs ${repoDir}/README.markdown)

# List of data files to be installed.
set(filesToInstall_data ${repoDir}/data/default_db.sqlite
                        ${repoDir}/data/DefaultData.xml
                        # Yes, I know this is 'documentation', but Debian policy suggests it should be
                        # with the data (see section 12.3 of the policy manual).
                        ${repoDir}/doc/manual-en.pdf)

# Desktop files to install.
set(filesToInstall_desktop ${repoDir}/brewtarget.desktop)

# Icon files to install.
set(filesToInstall_icons ${repoDir}/images/brewtarget.svg)

set(filesToInstall_translations ${repoDir}/translations/bt_ca.ts # Catalan
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

set(filesToInstall_macPropertyList "${repoDir}/mac/Info.plist")

set(filesToInstall_macIcons "${repoDir}/mac/BrewtargetIcon.icns")

set(filesToInstall_changeLogUncompressed "${repoDir}/CHANGES.markdown")

# This one gets created by make changeLog
set(filesToInstall_changeLogCompressed "${buildDir}/changelog.gz")

#=======================================================================================================================
# EXPERIMENTAL
#=======================================================================================================================
#=======================================================================================================================
#=========================================== Process other CMakeList.txt files =========================================
#=======================================================================================================================
if(${BUILD_DESIGNER_PLUGINS})
   message(STATUS "Building Qt Designer plugins.")
#   ADD_SUBDIRECTORY(QtDesignerPlugins)
   include(QtDesignerPlugins/CMakeLists.txt)
ELSE()
   message(STATUS "Building Main Application.")
#   ADD_SUBDIRECTORY(src)
   include(src/CMakeLists.txt)
endif()

#=======================================================================================================================
#======================================= Install (both locally and for packages) =======================================
#=======================================================================================================================
# This is for "make install" (or "sudo make install") AND for "make package" and "make package_source".
#
# Note that, per https://cmake.org/cmake/help/latest/module/CPack.html, DESTINATION option of the install() command must
# be a relative path; otherwise installed files are ignored by CPack.
#
# When a relative path is given in the DESTINATION option of the install() command, it is interpreted relative to the
# value of the CMAKE_INSTALL_PREFIX variable.
#
if(NOT ${BUILD_DESIGNER_PLUGINS})

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
      install(FILES ${filesToInstall_icons}
              DESTINATION "${installSubDir_data}/icons/hicolor/scalable/apps/"
              COMPONENT ${DATA_INSTALL_COMPONENT})

      # Install the .desktop file
      install(FILES ${filesToInstall_desktop}
              DESTINATION "${installSubDir_data}/applications"
              COMPONENT ${DATA_INSTALL_COMPONENT})

      # Install friendly-format change log aka release notes
      # Note that lintian does not like having a file called CHANGES.markdown in the doc directory, as it thinks it is a
      # misnamed changelog.Debian.gz (even when changelog.Debian.gz is also present!) so you get a
      # wrong-name-for-upstream-changelog warning.
      # The simplest way round this is to rename CHANGES.markdown to RelaseNotes.markdown
      install(FILES ${filesToInstall_changeLogUncompressed}
              RENAME "RelaseNotes.markdown"
              DESTINATION ${installSubDir_doc}
              COMPONENT ${DATA_INSTALL_COMPONENT})

      # Debian packages need to have the copyright file in a particular place (/usr/share/doc/PACKAGE/copyright)
      # RPM packages don't like having duplicate files in the same directory (eg copyright and COPYRIGHT with same
      # contents).  So the simplest thing is to rename COPYRIGHT to copyright for both.
      install(FILES "${repoDir}/COPYRIGHT"
              RENAME "copyright"
              DESTINATION ${installSubDir_doc}
              COMPONENT ${DATA_INSTALL_COMPONENT})

      # Each Debian package (which provides a /usr/share/doc/pkg directory) must install a Debian changelog file in
      # /usr/share/doc/pkg/changelog.Debian.gz
      install(FILES ${filesToInstall_changeLogCompressed}
              RENAME "changelog.Debian.gz"
              DESTINATION ${installSubDir_doc}
              COMPONENT ${DATA_INSTALL_COMPONENT})
   else()
      #----------- Windows and Mac -----------
      install(FILES "${repoDir}/COPYRIGHT"
              DESTINATION ${installSubDir_doc}
              COMPONENT ${DATA_INSTALL_COMPONENT})
   endif()

   if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
      install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
              DESTINATION bin
              COMPONENT System)
   endif()
endif()


#=======================================================================================================================
#====================================================== Packaging ======================================================
#=======================================================================================================================
# To make the packages, "make package". For source packages, "make package_source"
#
# We use CMake's CPack module (see https://cmake.org/cmake/help/latest/module/CPack.html and
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html) as our preferred way of
# creating install packages.  You have slightly less control compared with doing things more manually,
# but it's a lot less hassle.
#

#
# Cross-platform packaging options
#
# Note that we don't bother setting the following
#   - CPACK_PACKAGE_NAME as it defaults to the project name, which is what we want
#   - CPACK_PACKAGE_DIRECTORY as it defaults to the build directory, which is what we want
#   - CPACK_PACKAGE_VERSION_MAJOR as it defaults to the version given in the project command above, which is what we
#                                 want
#   - CPACK_PACKAGE_VERSION_MINOR for the same reason
#   - CPACK_PACKAGE_VERSION_PATCH for the same reason
#   - CPACK_SET_DESTDIR as it is only needed to is to "install software at non-default location"
#   - CPACK_PACKAGING_INSTALL_PREFIX as the defaults from each generator should usually be fine
#   - CPACK_PACKAGE_CONTACT as, AFAICT, it's only used to provide a default value for CPACK_DEBIAN_PACKAGE_MAINTAINER
#
set(CPACK_PACKAGE_VENDOR "The Brewtarget Team")
string(
   CONCAT
   CPACK_PACKAGE_DESCRIPTION
   # Note that the lines here are not supposed to be more than 79 characters long, otherwise rpmlint will spew an error.
   # (If they are more than 80 characters long, then lintian will also complain, though only with a warning.)
   "Brewtarget is a calculator for brewing beer. It is a Qt-based program which\n"
   "allows you to create recipes from a database of ingredients. It calculates\n"
   "all the important parameters, helps you with mash temperatures, and just\n"
   "makes the process of recipe formulation much easier."
)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GUI beer brewing software")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Brewtarget/brewtarget")
#
message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("PROJECT_VERSION = ${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Brewtarget-${PROJECT_VERSION}")
set(CPACK_PACKAGE_ICON "${repoDir}/images/brewtarget.svg")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_RESOURCE_FILE_LICENSE "${repoDir}/COPYING.GPLv3")
set(CPACK_STRIP_FILES true)
set(CPACK_VERBATIM_VARIABLES true)
set(CPACK_PACKAGE_EXECUTABLES "${fileName_executable}" "Brewtarget")

#
# Variables for Source Package Generators
#
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}_source")
set(CPACK_SOURCE_GENERATOR "TBZ2;")
set(CPACK_SOURCE_IGNORE_FILES "/.svn/"
                              "~$"
                              "/CMakeFiles/"
                              "/_CPack_Packages/"
                              "^brewtarget.*deb$"
                              "^brewtarget.*rpm$"
                              "^brewtarget.*tar.*$"
                              "CPack.*"
                              "Makefile"
                              "cmake_install.*"
                              "\\\\.o$"
                              "/brewtarget.dir/"
                              "moc_.*"
                              "qrc_brewtarget.*"
                              "ui_.*h"
                              "install_manifest.*"
                              "config\\\\.h")

if(UNIX AND NOT APPLE)
   #================================================= Linux Packaging ==================================================
   set(CPACK_GENERATOR "DEB;RPM;TBZ2;")

   #-----------------DEB----------------
   # See https://cmake.org/cmake/help/latest/cpack_gen/deb.html

   # We don't set CPACK_DEBIAN_PACKAGE_NAME as it defaults to CMAKE_PROJECT_NAME, which is what we want
   # (We don't want to put the version in the package name as it gets added to the file name automatically, so you'd
   # end up with it twice.  Plus you'd get a bad-package-name error from lintian.)

   set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

   set(CPACK_DEBIAN_PACKAGE_VERSION "${PROJECT_VERSION}-1")

   # See https://www.debian.org/doc/debian-policy/ch-binary.html#s-maintainer for more on the "maintainer", but
   # essentially it's a required package property that needs to be either one person or a group mailing list.  In the
   # latter case, the individual maintainers need be listed in a separate property called "uploaders", though I can't
   # see a way to set this via CPack.
   #
   # Also note, per https://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-maintainer, that it's simplest
   # to avoid having full stops in the maintainer's name.
   set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Philip Greggory Lee <rocketman768@gmail.com>")

   # This should save us having to set CPACK_DEBIAN_PACKAGE_DEPENDS manually
   set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

   set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
   set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

   # By default, CPack sets directory permissions with group_write enabled (775 in the old money).  Lintian doesn't like
   # this and wants only the owner to have write permission (755 in the old money).  So we have to tell CPack here what
   # permissions to use.
   set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                                   GROUP_READ GROUP_EXECUTE
                                                   WORLD_READ WORLD_EXECUTE)
   #-----------------RPM-----------------
   # See https://cmake.org/cmake/help/latest/cpack_gen/rpm.html

   set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

   # RPM file name is supposed to be <NAME>-<VERSION>-<RELEASE>.<ARCH>.rpm.  However, CPack doesn't, by default, stick
   # RELEASE in there, so we add it to version.  (The reason for having both version and release is that "version is
   # controlled by the original author and release is controlled by whoever constructed the RPM".  So we always set
   # release to 1.)
   set(CPACK_RPM_PACKAGE_VERSION "${PROJECT_VERSION}-1")

   # CPACK_RPM_PACKAGE_DESCRIPTION defaults to CPACK_PACKAGE_DESCRIPTION_FILE rather than CPACK_PACKAGE_DESCRIPTION,
   # so we have to set it explicitly here
   set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION})

   # This has to match one of the ValidLicenses values in packaging/rpmLintFilters.toml.  (See comment in that file for
   # more info.)
   set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")

   set(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")

   # This should (I think) save us having to set CPACK_RPM_PACKAGE_REQUIRES manually
   set(CPACK_RPM_PACKAGE_AUTOREQ yes)

   set(CPACK_RPM_SPEC_MORE_DEFINE "%define ignore \#")
   set(CPACK_RPM_USER_FILELIST "%ignore /usr" "%ignore /usr/local" "%ignore /usr/local/bin" "%ignore /usr/local/share" "%ignore /usr/local/share/applications")
elseif(WIN32)
   #================================================ Windows Packaging =================================================
   # http://www.thegigsite.com/cmake-2.6.0/CMakeCPackOptions.cmake
   set(CPACK_GENERATOR "NSIS")


   set(CPACK_NSIS_PACKAGE_NAME "Brewtarget-${PROJECT_VERSION}")
   set(CPACK_NSIS_DISPLAY__NAME "Brewtarget-${PROJECT_VERSION}")
   set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Brewtarget-${PROJECT_VERSION}")
   set(CPACK_NSIS_MODIFY_PATH ON)

   # Extra start menu items.
   #set(CPACK_NSIS_MENU_LINKS
   #     "bin/${fileName_executable}" "My Brewtarget"
   #  )
elseif(APPLE)
   #================================================== Mac Packaging ===================================================
#  set(CPACK_GENERATOR "Bundle")
#
#  set(CPACK_BUNDLE_NAME            "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}")
#  set(CPACK_BUNDLE_ICON            "${filesToInstall_macIcons}")
#  set(CPACK_BUNDLE_PLIST           "${filesToInstall_macPropertyList}")
#  set(CPACK_BUNDLE_STARTUP_COMMAND "${fileName_executable}")
#
   #set(CPACK_GENERATOR "DragNDrop")
   #set(CPACK_DMG_FORMAT "UDBZ")
endif()

# The documentation implies that all the CPACK_* variables need to be set before we include the CPack module, so that's
# what we do.
INCLUDE(CPack)

#=======================================================================================================================
#================================================= Custom Make Targets =================================================
#=======================================================================================================================
# These go at the end of the file so that they can use any of the variables created above

# `make install-data` or `make install-runtime`
ADD_CUSTOM_TARGET(
   install-data
   COMMAND "${CMAKE_COMMAND}"
           -DCOMPONENT=${DATA_INSTALL_COMPONENT}
           -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
ADD_CUSTOM_TARGET(
   install-runtime
   DEPENDS ${fileName_executable}
   COMMAND "${CMAKE_COMMAND}"
           -DCOMPONENT=${RUNTIME_INSTALL_COMPONENT}
           -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)

# Doxygen Custom Target
FIND_PROGRAM(DOXYGEN_CMD doxygen)
if(DOXYGEN_CMD)
  SET(DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile")
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in" ${DOXYFILE})
  ADD_CUSTOM_TARGET(source_doc
                     COMMAND ${DOXYGEN_CMD} ${DOXYFILE}
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
                  )
endif()

#
# ChangeLog custom target
#
# This is to create a compressed changelog in a Debian-friendly format
#
# Our change log (CHANGES.markdown) uses markdown format, with the following raw structure:
#    ## v1.2.3
#
#    Optional one-line description of the release.
#
#    ### New Features
#
#    * Blah blah blah
#    * etc
#
#    ### Bug Fixes
#
#    * Blah blah blah
#    * etc
#
#    ### Incompatibilities
#
#    None
#
#    ### Release Timestamp
#    Sun, 06 Feb 2022 12:02:58 +0100
#
# However, per https://www.debian.org/doc/debian-policy/ch-source.html#debian-changelog-debian-changelog, Debian
# change logs need to be in the following format:
#    package (version) distribution(s); urgency=urgency
#      [optional blank line(s), stripped]
#      * change details
#      more change details
#      [blank line(s), included in output of dpkg-parsechangelog]
#      * even more change details
#      [optional blank line(s), stripped]
#     -- maintainer name <email address>[two spaces]  date
#
# We are being a bit fast-and-loose in hard-coding the same maintainer name for each release, but I don't thing it's a
# huge issue.
#
# Note that, to keep us on our toes, Debian change log lines are not supposed to be more than 80 characters long.  This
# is non-trivial, but the ghastly bit of awk below gets us most of the way there.
#
if(UNIX AND NOT APPLE)
   # Note that, whilst using VERBATIM below makes things a lot less painful, we still have to double-escape things -
   # once for CMake and once for bash.  (Where something is single-escaped it probably means either that CMake needs the
   # escape and the bash command does not, or vice versa.)  Also, we have more invocations of sed that we strictly need
   # because it makes things slightly easier to follow and debug.
   string(
      CONCAT changeLogProcessCommands
      "cat ${filesToInstall_changeLogUncompressed} | "
      # Skip over the introductory headings and paragraphs of CHANGES.markdown until we get to the first version line
      "sed -n '/^## v/,$p' | "
      # We want to change the release timestamp to maintainer + timestamp, but we don't want to create too long a line
      # before we do the fold command below, so use "÷÷maintainer÷÷" as a placeholder for
      # " -- ${CPACK_DEBIAN_PACKAGE_MAINTAINER}  "
      "sed -z 's/\\n### Release Timestamp\\n\\([^\\n]*\\)\\n/\\n÷÷maintainer÷÷\\1\\n/g' | "
      # Join continued lines in bullet lists
      "sed -z 's/\\n  / /g' | "
      # Change the version to package (version) etc.  Stick a '÷' on the front of the line to protect it from
      # modification below
      "sed 's/^## v\\(.*\\)$/÷${PROJECT_NAME} (\\1-1) unstable\; urgency=low/' | "
      # Change bullets to sub-bullets
      "sed 's/^\\* /    - /' | "
      # Change headings to bullets
      "sed 's/^### /  * /' | "
      # Change any lines that don't start with space OR a ÷ character to be bullets
      "sed 's/^\\([^ ÷]\\)/  * \\1/' | "
      # Split any long lines.  Make the width less than 80 so we've got a margin go insert spaces at the start of
      # bullet continuation lines.
      "fold -s --width=76 | "
      # With a lot of help from awk, reindent the lines that were split off from a long bullet line so that they align
      # with that previous line.
      "awk 'BEGIN { inBullet=0 } "
      "{"
         "if (!inBullet) {"
            "inBullet=match($0, \"^( +)[^ ] \", spaces)\;"
            "print\;"
         "} else {"
            "bulletContinues=match($0, \"^[^ ]\")\;"
            "if (!bulletContinues) {"
               "inBullet=match($0, \"^( +)[^ ] \", spaces)\;"
               "print\;"
            "} else {"
               "print spaces[1] \"  \" $0\;"
            "}"
         "}"
      "}' | "
      # Fix the "÷÷maintainer÷÷" placeholders
      "sed 's/÷÷maintainer÷÷/ -- ${CPACK_DEBIAN_PACKAGE_MAINTAINER}  /' | "
      # Remove the protective '÷' from the start of any other lines
      "sed 's/^÷//' | "
      "gzip --best -n --to-stdout > ${filesToInstall_changeLogCompressed}"
   )
   ADD_CUSTOM_COMMAND(
      OUTPUT ${filesToInstall_changeLogCompressed}
      COMMAND bash -c ${changeLogProcessCommands}
      VERBATIM
      COMMENT "Processing ${filesToInstall_changeLogUncompressed} to ${filesToInstall_changeLogCompressed}"
      MAIN_DEPENDENCY ${filesToInstall_changeLogUncompressed}
   )
   ADD_CUSTOM_TARGET(changelog ALL DEPENDS ${filesToInstall_changeLogCompressed})
endif()

# Some extra files for the "make clean" target.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             APPEND
             PROPERTY ADDITIONAL_CLEAN_FILES ".*~$" # Kate backup files.
                                             "CMakeLists.txt.user" # From QtCreator I think.
                                             "CPackConfig.cmake"
                                             "CPackSourceConfig.cmake")
set_property(DIRECTORY ${buildDir}
             APPEND
             PROPERTY ADDITIONAL_CLEAN_FILES "*.deb"
                                             "*.deb.sha256"
                                             "*.rpm"
                                             "*.rpm.sha256"
                                             "*.tar.bz2"
                                             "*.tar.bz2.sha256")

# make addframeworks should copy the Qt libraries to the app.
if(APPLE)
   set(QT_BINARY_DIR "${_qt5Core_install_prefix}/bin")
   ADD_CUSTOM_TARGET(
      addframeworks ALL
      COMMAND ${QT_BINARY_DIR}/macdeployqt "Brewtarget.app" -dmg
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${fileName_executable}
   )
endif()

if(UNIX AND NOT APPLE)
   ADD_CUSTOM_TARGET(
      package_lint
      # Running lintian does a very strict check on the Debian package.  You can find a list of all the error and
      # warning codes at https://lintian.debian.org/tags.
      #
      # Some of the warnings are things that only matter for packages that actually ship with Debian itself - ie they
      # won't stop the package working but are not strictly within the standards that the Debian project sets for the
      # packages included in the distro.
      #
      # Still, we try to fix as many warnings as possible.  As at 2022-08-11 we currently have one warning that we do
      # not ship a man page.  We should get to this at some point.
      COMMAND lintian --no-tag-display-limit *.deb
      # Running rpmlint is the equivalent exercise for RPMs.  Most common error and warning codes are listed at
      # https://fedoraproject.org/wiki/Common_Rpmlint_issues
      COMMAND rpmlint --config ${repoDir}/packaging *.rpm
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Running lintian and on deb package and rpmlint on rpm package.  Warnings about man pages are expected!"
   )
endif()
