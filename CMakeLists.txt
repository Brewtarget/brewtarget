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
SET( brewtarget_VERSION_PATCH 4 )
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
OPTION( UPDATE_TRANSLATIONS
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

IF( APPLE AND NOT CMAKE_OSX_ARCHITECTURES )
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
SET( EXT_LIBS "" )

IF( ${BUILD_DESIGNER_PLUGINS} )
   FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtNetwork QtSvg QtWebKit QtXml Phonon QtDesigner REQUIRED)
ELSE()
   FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtNetwork QtSvg QtWebkit QtXml Phonon REQUIRED)
ENDIF()

MESSAGE( STATUS "CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}" )

# Some distros seem to treat Qt and Phonon as separate, so try to find Phonon.
IF( NOT QT_PHONON_FOUND )
   MESSAGE( STATUS "Phonon not found as a part of Qt; trying another approach." )

   FIND_LIBRARY( EXT_PHONON_LIB phonon )
   FIND_PATH( EXT_PHONON_PATH "mediaobject.h" )

   IF( EXISTS ${EXT_PHONON_LIB} AND EXISTS ${EXT_PHONON_PATH} )
      MESSAGE( STATUS "Hooray! Phonon found." )
      MESSAGE( STATUS "Phonon dir: ${EXT_PHONON_PATH}")
      MESSAGE( STATUS "Phonon lib: ${EXT_PHONON_LIB}")

      SET( EXT_LIBS ${EXT_LIBS} ${EXT_PHONON_LIB} )
      INCLUDE_DIRECTORIES( ${EXT_PHONON_PATH} )
   ELSE()
      MESSAGE( FATAL_ERROR "Phonon not found. Run cmake again with the option -DCMAKE_PREFIX_PATH=/path/to/phonon" )
   ENDIF()
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
IF( NOT EXEC_PREFIX )
   SET( EXEC_PREFIX ${CMAKE_INSTALL_PREFIX} )
ENDIF()
   
SET( DATAROOTDIR "${EXEC_PREFIX}/share" )
#SET( DATADIR ${DATAROOTDIR} )
SET( BINDIR "${EXEC_PREFIX}/bin" )
SET( DOCDIR "${DATAROOTDIR}/doc/${CMAKE_PROJECT_NAME}" )

# Actual install directories.
IF( UNIX AND NOT APPLE )
  SET( DATAPATH "${DATAROOTDIR}/${CMAKE_PROJECT_NAME}" )
  SET( TARGETPATH ${BINDIR} )
  SET( DOCPATH ${DOCDIR} )
ELSEIF( WIN32 )
  # Works for regular "make install", but not CPack/NSIS installer.
  #SET( CMAKE_INSTALL_PREFIX "C:/Program Files/brewtarget-${brewtarget_VERSION_STRING}" )
  #SET( DATAPATH "brewtarget-${brewtarget_VERSION_STRING}" )
  #SET( TARGETPATH "brewtarget-${brewtarget_VERSION_STRING}" )
  #SET( DOCPATH "brewtarget-${brewtarget_VERSION_STRING}/doc" )
  
  # For some damn reason, for the NSIS installer,
  # the prefix needs to be empty. Also, seems that the .exe
  # needs to be in bin/. Fucking piece of shit CPack...
  # Can anybody shed some light on this situation?
  SET( CMAKE_INSTALL_PREFIX "" )
  SET( DATAPATH "data" )
  SET( TARGETPATH "bin" )
  SET( DOCPATH "doc" )
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

#==========================Find dlls for Win32=================================

IF( WIN32 )
   SET( Qt_DLLs
        ${QT_BINARY_DIR}/phonon4.dll
        ${QT_BINARY_DIR}/QtCore4.dll
        ${QT_BINARY_DIR}/QtGui4.dll
        ${QT_BINARY_DIR}/QtNetwork4.dll
        ${QT_BINARY_DIR}/QtSvg4.dll
        ${QT_BINARY_DIR}/QtWebKit4.dll
        ${QT_BINARY_DIR}/QtXml4.dll
   )
   SET( Phonon_Backend_DLLs
        ${QT_PLUGINS_DIR}/phonon_backend/phonon_ds94.dll
   )
   SET( Image_Formats_DLLs
        ${QT_PLUGINS_DIR}/imageformats/qgif4.dll
        ${QT_PLUGINS_DIR}/imageformats/qico4.dll
        ${QT_PLUGINS_DIR}/imageformats/qjpeg4.dll
        ${QT_PLUGINS_DIR}/imageformats/qmng4.dll
        ${QT_PLUGINS_DIR}/imageformats/qsvg4.dll
        ${QT_PLUGINS_DIR}/imageformats/qtiff4.dll
   )
   SET( Icon_Engines_DLLs
        ${QT_PLUGINS_DIR}/iconengines/qsvgicon4.dll
   )

   # Find extra MinGW-specific dlls.
   IF( MINGW )
      IF( NOT MINGW_BIN_DIR )
         FIND_PATH(MINGW_BIN_DIR "mingw32-c++.exe")
      ENDIF()
     
     IF( NOT EXISTS ${MINGW_BIN_DIR} )
        MESSAGE( FATAL_ERROR "MinGW bin dir not found. Run cmake again with the option -DMINGW_BIN_DIR=c:/path/to/mingw" )
     ELSE()
        SET( Qt_DLLs
             ${Qt_DLLs}
             ${MINGW_BIN_DIR}/libgcc_s_dw2-1.dll
             ${MINGW_BIN_DIR}/libstdc++-6.dll
             ${MINGW_BIN_DIR}/mingwm10.dll
        )
     ENDIF()
   ENDIF()
   
   INSTALL( FILES ${Qt_DLLs}
            DESTINATION "bin" )
   INSTALL( FILES ${Phonon_Backend_DLLs}
            DESTINATION "bin/phonon_backend" )
   INSTALL( FILES ${Image_Formats_DLLs}
            DESTINATION "bin/imageformats" )
   INSTALL( FILES ${Icon_Engines_DLLs}
            DESTINATION "bin/iconengines" )
ENDIF()

# =====================Process other CMakeList.txt's===========================
IF( ${BUILD_DESIGNER_PLUGINS} )
   MESSAGE( STATUS "Building Qt Designer plugins." )
   ADD_SUBDIRECTORY(QtDesignerPlugins)
ELSE()
   MESSAGE( STATUS "Building Brewtarget." )
   ADD_SUBDIRECTORY(${SRCDIR})
ENDIF()

