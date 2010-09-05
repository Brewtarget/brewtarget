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

# Directories.
SET(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}")
SET(SRCDIR "${ROOTDIR}/src")
SET(UIDIR "${ROOTDIR}/ui")
SET(DATADIR "${ROOTDIR}/data")
SET(TRANSLATIONSDIR "${ROOTDIR}/translations")

INCLUDE_DIRECTORIES(${SRCDIR})
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/src") # In case of out-of-source build.

# Find Qt4 (>= 4.6.0) for this project.
FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtSvg QtWebkit QtXml Phonon REQUIRED)

# Some extra files for the "make clean" target.
SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
             ".*~$" # Kate backup files.
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

# Tell cmake to process other CMakeList.txt's
ADD_SUBDIRECTORY(${SRCDIR})