PROJECT(brewtarget)
cmake_minimum_required(VERSION 2.8)

# Creates a Makefile.
# NOTE: cmake . -DCMAKE_INSTALL_PREFIX=/tmp/blah && make DESTDIR=/foo
# will install to /foo/tmp/blah.

# Where the binary goes.
SET(brewtarget_EXECUTABLE "brewtarget")

#=================================Version======================================
SET( brewtarget_VERSION_MAJOR 1 )
SET( brewtarget_VERSION_MINOR 2 )
SET( brewtarget_VERSION_PATCH 3 )
SET( brewtarget_VERSION_STRING "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}.${brewtarget_VERSION_PATCH}" )

#===================================Options====================================
OPTION( BUILD_DESIGNER_PLUGINS
        "If on, you will only build and install the designer plugins."
        OFF )
OPTION( DO_RELEASE_BUILD
        "If on, will do a release build. Otherwise, debug build."
        OFF )
OPTION( ENABLE_PROFILING
        "If on, builds with necessary profiling options."
        OFF )
OPTION( NO_PHONON
        "If on, does not build any Phonon code."
        OFF )
OPTION(UPDATE_TRANSLATIONS
       "Update source translation *.ts files (WARNING: 'make clean' will delete the source .ts files! Danger!)"
       OFF)
#==============================Compile flags===================================
SET( CMAKE_CXX_FLAGS_RELEASE "-Wall -ansi -pedantic -O2" )
SET( CMAKE_CXX_FLAGS_DEBUG "-Wall -gstabs+" )

IF( ${ENABLE_PROFILING} )
   SET( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg" )
ENDIF()

IF( ${NO_PHONON} )
   ADD_DEFINITIONS( -DNO_PHONON )
ENDIF()

IF( ${DO_RELEASE_BUILD} )
   SET( CMAKE_BUILD_TYPE "Release" )
ELSE()
   SET( CMAKE_BUILD_TYPE "Debug" )
ENDIF()

IF( APPLE )
   SET( CMAKE_OSX_ARCHITECTURES ppc i386 ) # Build universal binary
   #SET( CMAKE_OSX_ARCHITECTURES ppc i386 ppc64 x86_64 ) # Build universal binary for 32 and 64 bit.
ENDIF()

#============================Directories=======================================
SET(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}")
SET(SRCDIR "${ROOTDIR}/src")
SET(UIDIR "${ROOTDIR}/ui")
SET(DATADIR "${ROOTDIR}/data")
SET(TRANSLATIONSDIR "${ROOTDIR}/translations")
SET(WINDIR "${ROOTDIR}/win")

INCLUDE_DIRECTORIES(${SRCDIR})
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/src") # In case of out-of-source build.
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/QtDesignerPlugins")

# ======================Find Qt4 (>= 4.6.0) for this project===================
IF( ${BUILD_DESIGNER_PLUGINS} )
   FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtSvg QtWebKit QtXml Phonon QtDesigner REQUIRED)
ELSE()
   FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtSvg QtWebkit QtXml Phonon REQUIRED)
ENDIF()

# Some extra files for the "make clean" target.
SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
             ".*~$" # Kate backup files.
             "CMakeLists.txt.user" # From QtCreator I think.
             "CPackConfig.cmake"
             "CPackSourceConfig.cmake"
            )

#============================Doxygen Custom Target=============================
FIND_PROGRAM( DOXYGEN_CMD doxygen )
IF( DOXYGEN_CMD )
  ADD_CUSTOM_TARGET( source_doc
                     COMMAND ${DOXYGEN_CMD} Doxyfile
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
                   )
ENDIF()

#=========================Detect Debian======================================== 
IF( EXISTS "/etc/debian_version" )
  MESSAGE( STATUS "Debian detected..." )

  IF( NOT ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr" )
    MESSAGE( WARNING "Debian system, but CMAKE_INSTALL_PREFIX != /usr" )
    MESSAGE( WARNING "CMAKE_INSTALL_PREFIX = \"${CMAKE_INSTALL_PREFIX}\"" )
  ENDIF()
ENDIF()

#========================Construct the directories=============================

# Debian standard directories.
SET( EXEC_PREFIX ${CMAKE_INSTALL_PREFIX} )
SET( DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" )
#SET( DATADIR ${DATAROOTDIR} )
SET( BINDIR "${EXEC_PREFIX}/bin" )
SET( DOCDIR "${DATAROOTDIR}/doc/${CMAKE_PROJECT_NAME}" )

# Actual install directories.
IF( UNIX AND NOT APPLE )
  SET( DATAPATH "${DATAROOTDIR}/${CMAKE_PROJECT_NAME}" )
  SET( TARGETPATH ${BINDIR} )
  SET( DOCPATH ${DOCDIR} )
ELSEIF( WIN32 )
  SET( DATAPATH "${CMAKE_INSTALL_PREFIX}/release" )
  SET( TARGETPATH "${CMAKE_INSTALL_PREFIX}/release" )
  SET( DOCPATH "${CMAKE_INSTALL_PREFIX}/release/doc" )
ELSEIF( APPLE )
  SET( DATAPATH "Resources" )
  SET( DOCPATH  "Resources/en.lproj" )
ENDIF()

# DATAPATH=/usr/share/brewtarget
#MESSAGE( STATUS "DATAPATH=${DATAPATH}" )

#==============================Setup the config.h==============================

# "#define CONFIGDATADIR ${CONFIGDATADIR}$" in config.in will be replaced
# by the below corresponding value in ${CONFIGDATADIR} below when
# CONFIGURE_FILE() is called.
SET( CONFIGDATADIR "${DATAPATH}/" )
SET( CONFIGDOCDIR "${DOCPATH}/" )

# Tell cmake where the configure file is and where
# to put the output. All variables in config.in written as "${VAR}$
# will be replaced by VAR as determined by cmake in config.h.
CONFIGURE_FILE( "${SRCDIR}/config.in" "${SRCDIR}/config.h" )


# =====================Process other CMakeList.txt's===========================
IF( ${BUILD_DESIGNER_PLUGINS} )
   MESSAGE( STATUS "Building Qt Designer plugins." )
   ADD_SUBDIRECTORY(QtDesignerPlugins)
ELSE()
   MESSAGE( STATUS "Building Brewtarget." )
   ADD_SUBDIRECTORY(${SRCDIR})
ENDIF()

