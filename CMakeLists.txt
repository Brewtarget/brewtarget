PROJECT(brewtarget)
cmake_minimum_required(VERSION 2.8)

# Creates a Makefile.
# NOTE: cmake . -DCMAKE_INSTALL_PREFIX=/tmp/blah && make DESTDIR=/foo
# will install to /foo/tmp/blah.

# Version
SET( brewtarget_VERSION_MAJOR 1 )
SET( brewtarget_VERSION_MINOR 2 )
SET( brewtarget_VERSION_PATCH 3 )
SET( brewtarget_VERSION_STRING "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}.${brewtarget_VERSION_PATCH}" )

OPTION( BUILD_DESIGNER_PLUGINS "If on, you will only build and install the designer plugins."
        OFF )

# Where the binary goes.
SET(brewtarget_EXECUTABLE "brewtarget")

# Compile flags
SET( CMAKE_CXX_FLAGS_RELEASE "-Wall -ansi -pedantic -O2" )
SET( CMAKE_CXX_FLAGS_DEBUG "-Wall -gstabs+" )

SET( CMAKE_BUILD_TYPE "Debug" )

# Directories.
SET(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}")
SET(SRCDIR "${ROOTDIR}/src")
SET(UIDIR "${ROOTDIR}/ui")
SET(DATADIR "${ROOTDIR}/data")
SET(TRANSLATIONSDIR "${ROOTDIR}/translations")
SET(WINDIR "${ROOTDIR}/win")

INCLUDE_DIRECTORIES(${SRCDIR})
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/src") # In case of out-of-source build.
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/QtDesignerPlugins")

# Find Qt4 (>= 4.6.0) for this project.
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

# Add a target for doxygen documentation.
FIND_PROGRAM( DOXYGEN_CMD doxygen )
IF( DOXYGEN_CMD )
  ADD_CUSTOM_TARGET( source_doc
                     COMMAND ${DOXYGEN_CMD} Doxyfile
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
                   )
ENDIF()

# See if this is debian.
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
SET( DATADIR ${DATAROOTDIR} )
SET( BINDIR "${EXEC_PREFIX}/bin" )
SET( DOCDIR "${DATAROOTDIR}/doc/${CMAKE_PROJECT_NAME}" )

# Actual install directories.
IF( UNIX AND NOT APPLE )
  SET( DATAPATH "${DATADIR}/${CMAKE_PROJECT_NAME}" )
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

#==============================Setup the config.h==============================

# NOTE: need to replace "#define CONFIGDATADIR blah" etc. in config.in
# with "#define CONFIGDATADIR ${CONFIGDATADIR}" so that they will be replaced
# by the below corresponding values when CONFIGURE_FILE()
# is called.
SET( CONFIGDATADIR "${DATAPATH}/" )
SET( CONFIGDOCDIR "${DOCPATH}/" )

# Tell cmake where the configure file is and where
# to put the output. All variables in config.in written as "${VAR}$
# will be replaced by VAR as determined by cmake in config.h.
CONFIGURE_FILE( "${SRCDIR}/config.in" "${SRCDIR}/config.h" )


# =====================Process other CMakeList.txt's===========================
IF( ${BUILD_DESIGNER_PLUGINS} )
   MESSAGE( STATUS "Making plugins..." )
   ADD_SUBDIRECTORY(QtDesignerPlugins)
ELSE()
   MESSAGE( STATUS "Building Brewtarget..." )
   ADD_SUBDIRECTORY(${SRCDIR})
ENDIF()
