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
#   * make package          -  Makes .deb, .rpm, NSIS Installer, and .tar.bz2 binary packages.  (TBD What about Mac?)
#
#                              On Linux, after make package, it's one of the following to install, if you want to
#                              install from the package rather than direct from the build tree (with sudo make install).
#                              (This would typically be for installing the package on a different machine then where the
#                              build was done.)
#                                 $ sudo dpkg -i brewtarget*.deb
#                                 $ sudo rpm -i brewtarget*.rpm
#
#                              On Mac and Windows environments, the `package` target will create an installer
#                              that may be executed to finish the installation.
#
#   * make package_source   -  Makes a .tar.bz2 source package.
#
# Custom make targets:
#   * make plugins          -  Builds the Qt Designer plugins
#   * make source_doc       -  Makes Doxygen HTML documentation of the source in doc/html
#   * make install-data
#   * make install-runtime
#   * make package_lint     -  Runs lintian and rpmlint on packages
#
#
# CMake options
#   * CMAKE_INSTALL_PREFIX  - /usr/local by default. Set this to /usr on Debian-based systems like Ubuntu.
#   * DO_RELEASE_BUILD      - OFF by default. If ON, will do a release build. Otherwise, debug build.
#   * NO_MESSING_WITH_FLAGS - OFF by default. ON means do not add any build flags whatsoever. May override other options.
# NOTE: You need to run CMake to change the options (you can't change them just by running make, not even by running
# make clean.  Eg, in the build directory, run the following to switch to debug builds:
#     cmake -DDO_RELEASE_BUILD=OFF ..
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
#================================================ Other preliminaries =================================================
#=======================================================================================================================
# Set the target binary architecture for targets on macOS
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
   # Per https://cmake.org/cmake/help/latest/variable/CMAKE_OSX_ARCHITECTURES.html, "the value of this variable should
   # be set prior to the first project() or enable_language() command invocation because it may influence configuration
   # of the toolchain and flags".
   set(CMAKE_OSX_ARCHITECTURES x86_64) # Build intel 64-bit binary.
   #set(CMAKE_OSX_ARCHITECTURES i386 x86_64) # Build intel binary.
   #set(CMAKE_OSX_ARCHITECTURES ppc i386 ppc64 x86_64) # Build universal binary.
endif()

#=======================================================================================================================
#============================================== Project name and Version ===============================================
#=======================================================================================================================
# It's simplest to keep the project name all lower-case as it means we can use a lot more of the default settings for
# Linux packaging (where directory names etc are expected to be all lower-case)
project(brewtarget VERSION 3.0.8 LANGUAGES CXX)
message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION}")
message(STATUS "PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}")
# Sometimes we do need the capitalised version of the project name
set(capitalisedProjectName Brewtarget)

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

   # For some damn reason, for the NSIS installer,
   # the prefix needs to be empty. Also, seems that the .exe
   # needs to be in bin/. Fucking piece of shit CPack...
   # Can anybody shed some light on this situation?
   #set(CMAKE_INSTALL_PREFIX "")
   set(CPACK_INSTALL_PREFIX "")

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
      # packaging/linux/rpmLintFilters.toml).
      #
      set(CMAKE_CXX_FLAGS_RELEASE "-Wall -ansi -pedantic -Wno-long-long -O2 -z noexecstack")
      #
      # -g3 should give even more debugging information thatn -g (which is equivalent to -g2)
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
# For the moment, max version we can have here is 5.9.5, because that's what Ubuntu 18.04 topped out at
#
set(QT5_MIN_VERSION 5.9.5)

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
# Although it's possible to do individual find_package commands for each bit of Qt we want to use (Qt5Core, Qt5Widgets,
# Qt5Sql, etc), the newer, and more compact, way of doing things (per
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
find_package(Qt5 ${QT5_MIN_VERSION} COMPONENTS ${qtAllComponents} REQUIRED)
# Each package has its own include directories that we need to make sure the compiler knows about
foreach(qtComponent IN LISTS qtAllComponents)
   # Sometimes it's useful that part of a variable name can be specified by expanding another variable!
   include_directories(${Qt5${qtComponent}_INCLUDE_DIRS})
endforeach()

# Qt wants position independent code in certain circumstances - specifically "You must build your code with position
# independent code if Qt was built with -reduce-relocations. Compile your code with -fPIC (and not with -fPIE)."
if(Qt5_POSITION_INDEPENDENT_CODE)
   # This will initialize the POSITION_INDEPENDENT_CODE property on all the targets
   set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# There's apparently a whole bunch of extra work we need to do to use Qt on Windows and Mac
if(WIN32)
   #====================================================================================================================
   #================================================= Windows Qt Stuff =================================================
   #====================================================================================================================
   # .:TBD:.  Not sure whether/why we need these additional Qt components on Windows
   #find_package(Qt5MultimediaWidgets REQUIRED)
   #find_package(Qt5OpenGL REQUIRED)

#   get_target_property(QtMultimediaWidgets_location Qt5::MultimediaWidgets         LOCATION_${CMAKE_BUILD_TYPE})
#   get_target_property(QtOpenGL_location            Qt5::OpenGL                    LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtCore_location              Qt5::Core                      LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtGui_location               Qt5::Gui                       LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtMultimedia_location        Qt5::Multimedia                LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtNetwork_location           Qt5::Network                   LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtPrintSupport_location      Qt5::PrintSupport              LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQgif_location              Qt5::QGifPlugin                LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQico_location              Qt5::QICOPlugin                LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQjpeg_location             Qt5::QJpegPlugin               LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQsvgIcon_location          Qt5::QSvgIconPlugin            LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQsvg_location              Qt5::QSvgPlugin                LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQtiff_location             Qt5::QTiffPlugin               LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtQWindows_location          Qt5::QWindowsIntegrationPlugin LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtSqliteDriver_location      Qt5::QSQLiteDriverPlugin       LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtSql_location               Qt5::Sql                       LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtSvg_location               Qt5::Svg                       LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtWidgets_location           Qt5::Widgets                   LOCATION_${CMAKE_BUILD_TYPE})
   get_target_property(QtXml_location               Qt5::Xml                       LOCATION_${CMAKE_BUILD_TYPE})

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

   get_target_property(_qmake_executable Qt5::qmake IMPORTED_LOCATION)
   get_filename_component(QT5_BIN_DIR "${_qmake_executable}" DIRECTORY)
   message("QT5_BIN_DIR = ${QT5_BIN_DIR}")

   #
   # Per https://doc.qt.io/qt-6/windows-deployment.html, the windeployqt executable creates all the necessary folder
   # tree "containing the Qt-related dependencies (libraries, QML imports, plugins, and translations) required to run
   # the application from that folder".
   #
   find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT5_BIN_DIR}")
   if(EXISTS ${WINDEPLOYQT_EXECUTABLE})
      # Per https://cmake.org/cmake/help/latest/command/add_executable.html, "IMPORTED executables are useful for
      # convenient reference from commands like add_custom_command()".
      add_executable(Qt5::windeployqt IMPORTED)
      set_target_properties(Qt5::windeployqt PROPERTIES IMPORTED_LOCATION ${WINDEPLOYQT_EXECUTABLE})
   endif()

   # International Components for Unicode
   file(GLOB IcuDlls "${QT5_BIN_DIR}/libicu*.dll")
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
   find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT5_BIN_DIR}")
   if(EXISTS ${MACDEPLOYQT_EXECUTABLE})
      # Per https://cmake.org/cmake/help/latest/command/add_executable.html, "IMPORTED executables are useful for
      # convenient reference from commands like add_custom_command()".
      add_executable(Qt5::macdeployqt IMPORTED)
      set_target_properties(Qt5::macdeployqt PROPERTIES IMPORTED_LOCATION ${MACDEPLOYQT_EXECUTABLE})
   endif()

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
set(filesToCompile_qrc "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.qrc")

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
# We try to restrict QtDesignerPlugins/CMakeLists.txt and src/CMakeLists.txt to just holding lists of source files,
# otherwise the dependencies and interactions between those files and this one get a bit hard to follow.
include(QtDesignerPlugins/CMakeLists.txt)
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
# Note that qt5_add_translation() _only_ does the latter and it is only qt5_create_translation() which does both.
# HOWEVER, there is a longstanding bug in qt5_create_translation() that means it adds all the .ts files to the list of
# files that get deleted when you invoke "make clean".  (See https://bugreports.qt.io/browse/QTBUG-31860,
# https://bugreports.qt.io/browse/QTBUG-41736, https://bugreports.qt.io/browse/QTBUG-76410,
# https://bugreports.qt.io/browse/QTBUG-96549.)
#
# There are various workarounds proposed on the internet -- eg setting the CLEAN_NO_CUSTOM property on the
# ${repoDir}/translations directory (something which itself requires jumping through a few hoops) -- but I have not had
# success with any of them.  Instead, taking inspiration from
# https://codereview.qt-project.org/c/qt/qttools/+/261912/1/src/linguist/Qt5LinguistToolsMacros.cmake, we run lupdate
# manually and then let qt5_add_translation invoke lrelease.
#
# Of course we have to do declare everything backwards for the dependencies:
#   - The executable will depend on translationsTarget
#   - translationsTarget depends on the binary .qm files, as generated by qt5_add_translation
#   - But before translationsTarget is built, we always run lupdate
#
qt5_add_translation(QM_FILES ${translationSourceFiles})

# Add a target for the QM_FILES so that we can add the translations as a dependency for the executable later.
add_custom_target(translationsTarget DEPENDS ${QM_FILES})

add_custom_command(TARGET translationsTarget PRE_BUILD
                   COMMAND ${Qt5_LUPDATE_EXECUTABLE}
                   ARGS  ${filesToCompile_cpp} ${filesToCompile_ui} -I ${repoDir}/src -ts ${translationSourceFiles}
                   VERBATIM)
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
       "Copyright 2009-2022.  Distributed under the terms of the GNU General Public License (version 3).")

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

   # This is from https://stackoverflow.com/questions/41193584/deploy-all-qt-dependencies-when-building
   if(TARGET Qt5::windeployqt)
      # First, execute windeployqt in a temporary directory after the build to create the directory structure and files
      # that Qt needs on Windows...
      add_custom_command(TARGET ${fileName_executable}
         POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
         COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" --no-translations --compiler-runtime "$<TARGET_FILE_DIR:${fileName_executable}>/$<TARGET_FILE_NAME:${fileName_executable}>"
      )

      # ...then we copy that directory tree to (a) the installation directory (when "make install" is run) and (b)
      # the directory where we run the test executable (straight away)
      install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/" DESTINATION bin)
      add_custom_target(
         copy-runtime-files ALL
         COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_BINARY_DIR}/windeployqt/" "${CMAKE_BINARY_DIR}/bin"
         DEPENDS ${fileName_executable}
      )
   endif()
elseif(APPLE)
   #
   # We want to achieve the same goal as with Qt5::windeployqt above, but note that the Qt5::macdeployqt functions
   # rather differently, with different options, so our CMake code here is correspondingly different
   #
   # Note that although macdeployqt can create the disk image file, we do that part with CPack to be consistent with our
   # approach to packaging on other platforms
   #
   # Using the MACOSX_BUNDLE paramater on add_executable() above will have created a directory tree akin to the
   # following:
   #
   #    ${fileName_executable}.app   # Top level folder that the desktop user sees as "the application"
   #    └── Contents
   #        ├── Info.plist
   #        ├── MacOS
   #        │   └── ${fileName_executable}    # The main executable
   #        └──  Resources
   #            ├── en.lproj
   #            ├── sounds
   #            └── translations_qm
   #
   # Running macdeployqt will add "Frameworks" and "Plugins" directory trees inside the "Contents" directory, and
   # populate these with the various Qt shared libraries etc (including database drivers) needed for the code to run.
   # It will also copy some, but not all, of the other shared libraries we need.
   #
   # We still need to:
   #    - Copy across some non-Qt libraries we need (part of Xalan that macdeployqt does not detect)
   #    - Copy sound files into Contents/Resources/sounds and translation files into Contents/Resources/translations_qm
   #    - Copy default DB and default data into Contents/Resources (which is also where the user manual lives for
   #      Brewtarget)
   # The last two should happen courtesy of the set_source_files_properties(... MACOSX_PACKAGE_LOCATION ...) commands
   # above.  (They won't show up in the diagnostic here because that copying happens _after_ building the executable.
   #
   if(TARGET Qt5::macdeployqt)
      # Note that macdeployqt needs any options flags to be _after_ the name of the app.  The only flag we use is:
      #   -verbose=<0-3>   0 = no output, 1 = error/warning (default), 2 = normal, 3 = debug
      add_custom_command(TARGET ${fileName_executable}
         POST_BUILD
         # Uncomment the  echo  and  tree  lines to provide extra diagnostics.  These are useful eg if you don't own a
         # Mac, and are relying on GitHub actions to do Mac builds.  (Just make sure  tree  is installed, usually via
         # brew install tree.
#         COMMAND echo "Contents of ${fileName_executable}.app before running macdeployqt"
#         COMMAND tree -sh ${fileName_executable}.app
         COMMAND Qt5::macdeployqt "$<TARGET_FILE_NAME:${fileName_executable}>.app" -verbose=2
#         COMMAND echo "Contents of ${fileName_executable}.app after running macdeployqt"
#         COMMAND tree -sh ${fileName_executable}.app
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
   endif()
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
   list(APPEND appAndTestCommonLibraries "Qt5::${qtComponent}")
endforeach()
message("appAndTestCommonLibraries: ${appAndTestCommonLibraries}")
target_link_libraries(${fileName_executable} ${appAndTestCommonLibraries})

#=================================Tests========================================
# We build the unit test executable in the bin subdirectory because, on Windows, we're going to copy a lot of other
# files in there (see below).
add_executable(${fileName_unitTestRunner}
               ${repoDir}/src/unitTests/Testing.cpp
               $<TARGET_OBJECTS:btobjlib>)
set_target_properties(${fileName_unitTestRunner} PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)

# Test app needs all the same libraries as the main app, plus Qt5::Test
target_link_libraries(${fileName_unitTestRunner} ${appAndTestCommonLibraries} Qt5::Test)

add_test(NAME pstdintTest                 COMMAND bin/${fileName_unitTestRunner} pstdintTest                )
add_test(NAME recipeCalcTest_allGrain     COMMAND bin/${fileName_unitTestRunner} recipeCalcTest_allGrain    )
add_test(NAME postBoilLossOgTest          COMMAND bin/${fileName_unitTestRunner} postBoilLossOgTest         )
add_test(NAME testUnitConversions         COMMAND bin/${fileName_unitTestRunner} testUnitConversions        )
add_test(NAME testNamedParameterBundle    COMMAND bin/${fileName_unitTestRunner} testNamedParameterBundle   )
add_test(NAME testNumberDisplayAndParsing COMMAND bin/${fileName_unitTestRunner} testNumberDisplayAndParsing)
add_test(NAME testAlgorithms              COMMAND bin/${fileName_unitTestRunner} testAlgorithms             )
add_test(NAME testTypeLookups             COMMAND bin/${fileName_unitTestRunner} testTypeLookups            )
add_test(NAME testLogRotation             COMMAND bin/${fileName_unitTestRunner} testLogRotation            )

#=======================================================================================================================
#============================================== Debian-friendly ChangeLog ==============================================
#=======================================================================================================================
#
# This is to create a compressed changelog in a Debian-friendly format
#
# NB: Even though this is a target you build (with make changelog) before doing make package, this section needs to
#     appear below the stuff for make package so that we can reuse variables such as CPACK_DEBIAN_PACKAGE_MAINTAINER
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
      "fold -s --width=72 | "
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
   add_custom_command(
      TARGET ${fileName_executable} POST_BUILD
      COMMAND bash -c ${changeLogProcessCommands}
      VERBATIM
      COMMENT "Processing ${filesToInstall_changeLogUncompressed} to ${filesToInstall_changeLogCompressed}"
      MAIN_DEPENDENCY ${filesToInstall_changeLogUncompressed}
   )
endif()

#=================================Installs=====================================

# Install executable.
install(TARGETS ${fileName_executable}
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${installSubDir_bin}
        COMPONENT ${RUNTIME_INSTALL_COMPONENT})

#=======================================================================================================================
#==================================== Shipping shared libraries on Windows and Mac =====================================
#=======================================================================================================================
if(WIN32)
   #
   # Shared libraries (DLLs) are a bit complicated on Windows.  Because there is not really a standard package
   # management system, we can't easily just say "we require libraries X, Y and Z".  Instead, we need to ship said
   # libraries in the same directory as our application's executable.
   #
   # (Of course, if you're just running on the same machine where you built the software, you could simply set the PATH
   # variable to include all the directories where the DLLs are.  But, in the general case, we're trying to create a
   # package that does not require people to have pre-installed Qt, Xerces, Xalan, etc.)
   #
   # In theory, CMake can work out what DLLs we need and thereby generate a list of files that we need to ensure are
   # copied as part of the install/packaging.  In practice, everything seems to be a bit more complicated.
   #
   # See https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:TARGET_RUNTIME_DLLS
   # (This replaces use of GET_RUNTIME_DEPENDENCIES which I just could not get to work correctly, despite following all
   # sorts of helpful advice at
   # https://stackoverflow.com/questions/62884439/how-to-use-cmake-file-get-runtime-dependencies-in-an-install-statement
   # I always hit a "Failed to start objdump" error.)
   #
   # Note, however that this does not find all the DLLs we need.  Perhaps this is related to the mention in the CMake
   # doco that "Many Find Modules produce imported targets with the UNKNOWN type and therefore will be ignored."
   #
   add_custom_command(
      TARGET ${fileName_executable} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:brewtarget> $<TARGET_FILE_DIR:brewtarget>
      COMMAND_EXPAND_LISTS
   )

   #
   # On MSYS2/MinGW, static libraries (including .dll.a files) live in /mingw64/lib (usually = C:/msys64/mingw64/lib/)
   # and dynamically linked libraries (aka DLLs) live in /mingw64/bin (usually = C:/msys64/mingw64/bin/).
   #
   # First, we do a bit of a hack to get to the bin directory from the lib one, without hard-coding either (in case
   # your 64-bit instance of MSYS2 is not installed at C:/msys64/).  XercesC_LIBRARIES will be the .dll.a file for
   # Xerces.  Usually only one library for Xerces, but we take first in the list in case.
   #
   list(GET XercesC_LIBRARIES 0 xercesFirstLibrary)
   get_filename_component(staticLibDir ${xercesFirstLibrary} DIRECTORY)
   set(dynamicLibDir ${staticLibDir}/../bin)
   message(STATUS "dynamicLibDir=" ${dynamicLibDir})

   #
   # Now, for each static library (/mingw64/lib/BlahBlah.dll.a) we know about, we get the corresponding dynamic one
   # (/mingw64/bin/BlahBlah.dll) and copy it into the same directory as the .exe file.
   #
   # Just to make our lives fun, sometimes the static and dynamic versions of the library will have slightly different
   # naming conventions.  Eg, for Xalan, we have:
   #    /mingw64/bin/libxalan-c.dll
   #    /mingw64/lib/libxalan-c.dll.a
   # But for Xerces, we have:
   #    /mingw64/lib/libxerces-c.dll.a
   #    /mingw64/bin/libxerces-c-3-2.dll
   # Hence the extra '*' in the glob expression below.
   #
   # Finally, note that, to make debugging more interesting, the message command below outputs during "make" rather
   # than during "make install".
   #
   foreach(staticLib IN LISTS XercesC_LIBRARIES XalanC_LIBRARIES)
      get_filename_component(libBaseName ${staticLib} NAME_WE)
      file(GLOB dynamicallyLinkedLibrary ${dynamicLibDir}/${libBaseName}*.dll)
      message(STATUS "Install will copy DLL ${dynamicallyLinkedLibrary} to ${installSubDir_bin}")
      install(FILES ${dynamicallyLinkedLibrary} DESTINATION ${installSubDir_bin})
   endforeach()

   #
   # Now, because we're also not assuming that the person running the code will have MSYS2/MinGW installed, we need to
   # include the DLLs that ship with them and get pulled in by GCC.  I am hoping to find a less manual way of doing this
   # but have not yet hit paydirt.
   #
   set(msys2Dlls
      libbrotli                ###
      libbrotlicommon        # Brotli compression -- see https://en.wikipedia.org/wiki/Brotli
      libbrotlidec           # Brotli compression
      libbrotlienc           # Brotli compression
      libbz2                 # BZip2 compression -- see https://en.wikipedia.org/wiki/Bzip2
      libdouble-conversion   # See https://github.com/google/double-conversion
      libfreetype            # See https://freetype.org/
      libgcc_s_dw2
      libglib                  ###
      libglib-2.0
      libgraphite
      libharfbuzz            # HarfBuzz text shaping engine -- see https://github.com/harfbuzz/harfbuzz
      libiconv               # See https://www.gnu.org/software/libiconv/
      libicudt               # Part of International Components for Unicode
      libicuin               # Part of International Components for Unicode
      libicuuc               # Part of International Components for Unicode
      libintl                # See https://www.gnu.org/software/gettext/
      libmd4c                # Markdown for C -- see https://github.com/mity/md4c
      libpcre2-16            # Perl Compatible Regular Expressions
      libpcre2-32            # Perl Compatible Regular Expressions
      libpcre2-8             # Perl Compatible Regular Expressions
      libpng16               # Official PNG reference library -- see http://www.libpng.org/pub/png/libpng.html
      libsqlite3             # Need this IN ADDITION to bin/sqldrivers/qsqlite.dll, which gets installed by windeployqt
      libstdc++
      libwinpthread
      libxalan-c
      libxalanMsg
      libxerces-c-3
      libzstd                # ZStandard (aka zstd) = fast lossless compression algorithm
      zlib1                    ###
      zlib                   # ZLib compression library
   )
   foreach(dynamicLib IN LISTS msys2Dlls)
      file(GLOB dynamicallyLinkedLibrary ${dynamicLibDir}/${dynamicLib}*.dll)
      message(STATUS "Install will also copy DLL ${dynamicallyLinkedLibrary} to ${installSubDir_bin}")
      install(FILES ${dynamicallyLinkedLibrary} DESTINATION ${installSubDir_bin})
   endforeach()

   # These are something to do with International Components for Unicode
   message(STATUS "Install will copy ${IcuDlls} to ${installSubDir_bin}")
   install(FILES ${IcuDlls} DESTINATION ${installSubDir_bin})

   add_custom_command(
      TARGET ${fileName_executable} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:brewtarget>
   )
elseif(APPLE)
   #
   # For similar reasons to Windows, the Mac Bundle needs to have shared libraries copied into it.  Mostly this is done
   # automatically with macdeployqt, which picks up Qt shared libraries, and other obvious dependencies such as
   # libxerces-c.dylib and libxalan-c.dylib, and puts them in the Contents/Frameworks folder inside the bundle.
   #
   # However, where a library depends on another library, this is not picked up by macdeployqt.  In particular,
   # libxalan-c.dylib depends on libxalanMsg.dylib.  The latter does not show up in XalanC_LIBRARIES either.  So we
   # have to take a more manual approach.
   #
   # Firstly, as https://stackoverflow.com/questions/63310608/find-library-cannot-find-basic-shared-library-macos-clion
   # explains you must remove the "lib" prefix from the library name before asking CMake to find it.
   #
   # Then, once you have the path to the library, you have to deal with the case that it will probably be a symbolic
   # link, so you need to dereference it rather than let CMake just copy the symlink -- see
   # https://stackoverflow.com/questions/11487638/cmake-fileinstall-files-destination-dir-with-symbolic-links.  (Except
   # you also need to pay attention that the get_filename_component() command is now "superseded by cmake_path()
   # command, except REALPATH now offered by file(REAL_PATH) command and PROGRAM now available in
   # separate_arguments(PROGRAM) command".  And because it's CMake, you have to pay close attention to which parameters
   # are values (eg expanded variables) and which are variable names.)
   #
   # Subsequently, you discover that you need to rename things because:
   #    /usr/local/lib/libxalanMsg.dylib     is a symbolic link to /usr/local/lib/libxalanMsg.112.dylib
   #    /usr/local/lib/libxalanMsg.112.dylib is a symbolic link to /usr/local/lib/libxalanMsg.112.0.dylib
   # but when otool is run by fixup_bundle, it looks for @rpath/libxalanMsg.112.dylib and will not accept any other
   # name for the library.   So, we change ".0.dylib" at the end of the library file name to ".dylib".
   #
   # Then you have to remember, as commented elsewhere, the DESTINATION option of the install() command must be a
   # relative path; otherwise installed files are ignored by CPack - ie won't end up in the DMG file.  (Yes, this does
   # contrast with the requirement of fixup_bundle to have an absolute path.  You have to stay on your toes with CMake.)
   # This is where it gets a bit confusing(!)  We seem to have two copies of the bundle in the build tree:
   #    1: ${fileName_executable}.app
   #    2: _CPack_Packages/Darwin/DragNDrop/${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}_x86_64/ALL_IN_ONE/${fileName_executable}.app
   # They look to be the same, except that installing with an absolute path beginning
   # ${CMAKE_BINARY_DIR}/${fileName_executable}.app/ puts things in the first of these (but not the second), while
   # installing with a relative path puts them in the second (but not the first).
   #
   # AFAICT it's the second bundle that ends up in the DMG file even though it's the first one that we're running
   # fixup_bundle on.  For the moment I'm just going to install libxalanMsg.112.dylib into both, on the grounds that it
   # surely can't hurt.
   #
   # One day, I hope we will find a more elegant and less brittle way to do all this!
   #
   find_library(XalanCMessage_LIBRARY NAMES xalanMsg)
   file(REAL_PATH "${XalanCMessage_LIBRARY}" XalanCMessage_LIBRARY_resolved)
   cmake_path(GET XalanCMessage_LIBRARY_resolved FILENAME XalanCMessage_LIBRARY_withoutPath)
   string(REGEX REPLACE "\.0\.dylib$" ".dylib" XalanCMessage_LIBRARY_tweaked "${XalanCMessage_LIBRARY_withoutPath}")
   message(STATUS "Found libxalanMsg at ${XalanCMessage_LIBRARY} which resolves to ${XalanCMessage_LIBRARY_resolved}")
   message(STATUS "Tweaked libxalanMsg file name from ${XalanCMessage_LIBRARY_withoutPath} to ${XalanCMessage_LIBRARY_tweaked}")
   install(FILES ${XalanCMessage_LIBRARY_resolved}
           DESTINATION ${fileName_executable}.app/Contents/Frameworks/
           RENAME ${XalanCMessage_LIBRARY_tweaked})
   install(FILES ${XalanCMessage_LIBRARY_resolved}
           DESTINATION ${CMAKE_BINARY_DIR}/${fileName_executable}.app/Contents/Frameworks/
           RENAME ${XalanCMessage_LIBRARY_tweaked})

   #
   # On MacOS, if you are shipping dynamic libraries with your application, you can tell the application where to look
   # for those dynamic libraries via "Runpath Search Path" aka "@rpath".  See
   # https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling for more details.
   #
   #
   # Setting CMAKE_BUILD_WITH_INSTALL_RPATH to true means "the software is always built with the install path for the
   # RPATH and does not need to be relinked when installed".  This is recommended at
   # https://discourse.cmake.org/t/osx-frameworks-rpath-and-image-not-found/3094/9
   #
   # Setting CMAKE_INSTALL_RPATH_USE_LINK_PATH to true "will append to the runtime search path (rpath) of installed
   # binaries any directories outside the project that are in the linker search path or contain linked library files".
   #
   # CMAKE_INSTALL_RPATH is "a semicolon-separated list specifying the rpath to use in installed targets".
   #
   set(CMAKE_BUILD_WITH_INSTALL_RPATH    true)
   set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)
   message(STATUS "CMAKE_INSTALL_RPATH = ${CMAKE_INSTALL_RPATH}, End Of Line")

   #
   # Having copied the shared libraries into the bundle, we now need to use CMake's fixup_bundle
   # (see https://cmake.org/cmake/help/latest/module/BundleUtilities.html) to make sure the application inside the
   # bundle looks in the bundle, not the system, for each of these shared libraries.
   #
   # After this, there is a verify_app command which checks that everything "appears valid".
   #
   # Note that the CMake doco says "DO NOT USE THESE FUNCTIONS AT CONFIGURE TIME (from CMakeLists.txt)! Instead, invoke
   # them from an install(CODE) or install(SCRIPT) rule."  Note that this means we have to be careful about which
   # variables are available to us.  A lot of things are simply not set inside install(CODE).  As explained at
   # https://stackoverflow.com/questions/73815300/how-do-i-get-project-root-dir-in-the-cmake-install-code-section, there
   # are various techniques to inject variables into install(CODE).  We use the string(CONFIGURE) approach as it avoids
   # too much escaping.
   #
   # The include(BundleUtilities) line is the bit the CMake documentation doesn't tell you about (or at least, not in as
   # obvious a way as one might like).  Without it you'll get an error about fixup_bundle being an unknown CMake
   # command.
   #
   # Another thing you have to find out the hard way is that the first parameter to fixup_bundle ("<app>") cannot be a
   # relative path.  If you pass in "foo.app", then fixup_bundle will look for "/foo.app".
   #
   # Besides the full path to the bundle to fix, fixup_bundle takes two other parameters --
   # fixup_bundle(<app> <libs> <dirs>).  The <libs> parameter "is a list of libraries that must be fixed up, but that
   # cannot be determined by otool output analysis (i.e. plugins)".  The <dirs> parameter is undocumented (welcome to
   # the joy of CMake) and often appears to be set to empty string in examples on stackoverflow etc.
   #
   # For diagnostics, it's often helpful to add lines such as the following in the code block below:
   #    execute_process(COMMAND echo "Pre fixup_bundle - build tree")
   #    execute_process(COMMAND pwd)
   #    execute_process(COMMAND tree -sh .)
   #
   string(CONFIGURE [[
      include(BundleUtilities)
      fixup_bundle("${CMAKE_BINARY_DIR}/${fileName_executable}.app" "" "${CMAKE_BINARY_DIR}/${fileName_executable}.app/Contents/Frameworks")
      verify_app("${CMAKE_BINARY_DIR}/${fileName_executable}.app")
   ]] installCode)
   install(CODE "${installCode}")
endif()

# Install the translations.
install(FILES ${QM_FILES}
        DESTINATION "${installSubDir_data}/translations_qm"
        COMPONENT ${DATA_INSTALL_COMPONENT} )

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
   ${CMAKE_COMMAND} -E cmake_echo_color --bold --green "⭐⭐⭐ Finished compiling ${capitalisedProjectName}.  Please run  sudo make install  to install. ⭐⭐⭐"
   COMMENT "Final Message"
)
add_dependencies(FinalMessage ${fileName_executable})

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
set(CPACK_PACKAGE_VENDOR "The ${capitalisedProjectName} Team")
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
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${capitalisedProjectName}-${PROJECT_VERSION}")
if(NOT WIN32)
   # Setting this on Windows breaks the NSIS packager with some confusing message saying it can't find the file
   set(CPACK_PACKAGE_ICON "${repoDir}/images/${PROJECT_NAME}.svg")
endif()
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_RESOURCE_FILE_LICENSE "${repoDir}/COPYING.GPLv3")
set(CPACK_STRIP_FILES true)
set(CPACK_VERBATIM_VARIABLES true)
set(CPACK_PACKAGE_EXECUTABLES "${fileName_executable}" "${capitalisedProjectName}")

#
# Variables for Source Package Generators
#
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PROJECT_VERSION}_source")
set(CPACK_SOURCE_GENERATOR "TBZ2;")
set(CPACK_SOURCE_IGNORE_FILES "/.svn/"
                              "~$"
                              "/CMakeFiles/"
                              "/_CPack_Packages/"
                              "^${PROJECT_NAME}.*deb$"
                              "^${PROJECT_NAME}.*rpm$"
                              "^${PROJECT_NAME}.*tar.*$"
                              "CPack.*"
                              "Makefile"
                              "cmake_install.*"
                              "\\\\.o$"
                              "/${PROJECT_NAME}.dir/"
                              "moc_.*"
                              "qrc_${PROJECT_NAME}.*"
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
   set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Matt Young <mfsy@yahoo.com>")

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

   # This has to match one of the ValidLicenses values in packaging/linux/rpmLintFilters.toml.  (See comment in that file for
   # more info.)
   set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")

   set(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")

   # This should (I think) save us having to set CPACK_RPM_PACKAGE_REQUIRES manually
   set(CPACK_RPM_PACKAGE_AUTOREQ yes)

   # .:TBD:. Not sure what these are for...
   set(CPACK_RPM_SPEC_MORE_DEFINE "%define ignore \#")
   set(CPACK_RPM_USER_FILELIST "%ignore /usr"
                               "%ignore /usr/local"
                               "%ignore /usr/local/bin"
                               "%ignore /usr/local/share"
                               "%ignore /usr/local/share/applications")
elseif(WIN32)
   #================================================ Windows Packaging =================================================
   # http://www.thegigsite.com/cmake-2.6.0/CMakeCPackOptions.cmake
   set(CPACK_GENERATOR "NSIS")

   # The title displayed at the top of the installer.
   set(CPACK_NSIS_PACKAGE_NAME "${capitalisedProjectName}-${PROJECT_VERSION}")
   # The display name string that appears in the Windows Apps & features in Control Panel
   set(CPACK_NSIS_DISPLAY_NAME "${capitalisedProjectName}-${PROJECT_VERSION}")
   # ?
   set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${capitalisedProjectName}-${PROJECT_VERSION}")
   # URL to a web site providing more information about your application
   set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/Brewtarget/brewtarget")
   # The name of a *.ico file used as the main icon for the generated install program
   set(CPACK_NSIS_MUI_ICON ${repoDir}/win/BrewtargetIcon_96.ico)

   # Lists each of the executables and associated text label to be used to create Start Menu shortcuts.
   # For example, setting this to the list ccmake;CMake would create a shortcut named "CMake" that would execute the
   # installed executable ccmake.
   set(CPACK_PACKAGE_EXECUTABLES "${fileName_executable};${capitalisedProjectName}")

   # List of desktop links to create. Each desktop link requires a corresponding start menu shortcut as created by
   # CPACK_PACKAGE_EXECUTABLES.
   set(CPACK_CREATE_DESKTOP_LINKS ${fileName_executable})

   # In theory CMake should work out for itself which tool to use for dependency resolution.  In practice, on Windows,
   # it seems there are issues.  In an MSYS2/MinGW environment we want to use objdump rather than the Visual Studio
   # dumpbin.
   #
   # .:TBD:. This is not sufficient to get objdump used...
   set(CMAKE_OBJDUMP "objdump")
   set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL objdump)
   set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND /usr/bin/objdump)

   message(STATUS "CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM is " ${CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM})
   message(STATUS "CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL is "     ${CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL})
   message(STATUS "CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND is "  ${CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND})

   set(CPACK_NSIS_MODIFY_PATH ON)

elseif(APPLE)
   #================================================== Mac Packaging ===================================================
   # Broadly speaking, what we want for Mac is a a DMG disk image which the end user can mount and from s/he can copy
   # the application bundle to her/his Applications folder.  (On MacOS, a bundle a directory structure that appears as a
   # single entity when viewed in the Finder.)
   #
   # It seems like there are two CPack generators that could potentially be used for this:
   #   - Bundle (https://cmake.org/cmake/help/latest/cpack_gen/bundle.html) which I think is used in conjunction with
   #     the DragNDrop generator
   #   - DragNDrop (https://cmake.org/cmake/help/latest/cpack_gen/dmg.html) which generates a DMG disk image from which
   #     the end user copies the application to her/his Applications folder
   #
   # According to
   # https://stackoverflow.com/questions/46011440/cmake-cpack-preferred-package-generators-for-different-platforms, it's
   # best to "Avoid the Bundle generator, it is older and more limited in what it supports, [compared with] the DMG
   # [aka DragNDrop] generator."
   #
   # There is also the "official" Qt way to do things for Mac at https://doc.qt.io/qt-6/macos-deployment.html.  This
   # does not use CPack but does offer options for doing things more directly in CMake.  (NB: As of January 2021, Qt is
   # migrating from qmake to cmake -- see https://www.qt.io/blog/qt-and-cmake-the-past-the-present-and-the-future.)  We
   # have to be a bit careful about mixing and matching approaches as we don't want to end up eg with a bundle inside a
   # bundle.
   #
   # Another potential option would be the Qt Installation Framework (IFW) -- see
   # https://doc.qt.io/qtinstallerframework/ --  for which there is a CPack generator
   # (https://cmake.org/cmake/help/latest/cpack_gen/ifw.html).  This creates a cross-platform GUI installer using Qt.
   # However, there are downsides to having a custom tool for install and upgrade, and, additionally, there's not a huge
   # amount of info about how to get the CPack IFW generator working on Mac.
   #
   # Since we're using CPack for Windows and Linux, it feels like it makes sense to use it for Mac too, we we go with
   # the DragNDrop generator.  Creation of the bundle is handled prior in CMake, prior to invocation of CPack, by the
   # MACOSX_BUNDLE target property etc (per https://cmake.org/cmake/help/latest/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html).
   #
   # NOTE If you are on Linux and you want to check the contents of a Mac disk image file called, say, foo.dmg, you can
   #      extract it into regular files from the command line with:
   #         7z x foo.dmg
   #
   set(CPACK_GENERATOR "DragNDrop")

   # The volume name of the generated disk image. Defaults to CPACK_PACKAGE_FILE_NAME, which we set above to be
   # something very specific for the benefit of Linux packages.  We choose a simpler/nicer name for the Mac disk image.
   set(CPACK_DMG_VOLUME_NAME "${CMAKE_PROJECT_NAME} ${PROJECT_VERSION}")

   message(STATUS "Building Mac package")
endif()

# The documentation implies that all the CPACK_* variables need to be set before we include the CPack module, so that's
# what we do.
INCLUDE(CPack)

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
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in" ${DOXYFILE})
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
                                             "CPackConfig.cmake"
                                             "CPackSourceConfig.cmake")
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

if(UNIX AND NOT APPLE)
   add_custom_target(
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
      COMMAND rpmlint --config ${repoDir}/packaging/linux *.rpm
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Running lintian and on deb package and rpmlint on rpm package.  Warnings about man pages are expected!"
   )
endif()
