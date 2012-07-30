PROJECT(brewtarget)
cmake_minimum_required(VERSION 2.8)

# Creates a Makefile.
# NOTE: cmake . -DCMAKE_INSTALL_PREFIX=/tmp/blah && make DESTDIR=/foo
# will install to /foo/tmp/blah.

# Where the binary goes.
SET(brewtarget_EXECUTABLE "brewtarget")

#=================================Version======================================
SET( brewtarget_VERSION_MAJOR 2 )
SET( brewtarget_VERSION_MINOR 0 )
SET( brewtarget_VERSION_PATCH 0 )
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
IF( CMAKE_COMPILER_IS_GNUCXX )
   SET( CMAKE_CXX_FLAGS_RELEASE "-Wall -ansi -pedantic -Wno-long-long -O2" )
   SET( CMAKE_CXX_FLAGS_DEBUG "-Wall -g3" )
ENDIF()

# Speed up compilation if using gcc.
IF( UNIX AND NOT APPLE )
   SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe" )
   SET( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pipe" )
ENDIF()

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
   SET( CMAKE_OSX_ARCHITECTURES i386 x86_64 ) # Build intel binary. 
   #SET( CMAKE_OSX_ARCHITECTURES ppc i386 ppc64 x86_64 ) # Build universal binary.
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

# Custom cmake modules.
SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules" )

#==============================OS-specific Commands============================
IF( UNIX )
   SET( RM_COMMAND "rm" )
   SET( RM_ARGS "-f" )
ELSEIF( WIN32 )
   SET( RM_COMMAND "del" )
   SET( RM_ARGS "" )
ENDIF()

# ======================Find Qt4 (>= 4.6.0) for this project===================
SET( EXT_LIBS "" )

IF( ${BUILD_DESIGNER_PLUGINS} )
   FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtNetwork QtSql QtSvg QtWebKit QtXml QtDesigner REQUIRED)
ELSE()
   FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtNetwork QtSql QtSvg QtWebkit QtXml REQUIRED)
ENDIF()

MESSAGE( STATUS "CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}" )

# Some distros seem to treat Qt and Phonon as separate, so try to find Phonon.
IF( NOT ${NO_PHONON} )
   FIND_PACKAGE( Phonon REQUIRED )
ENDIF()

# Some extra files for the "make clean" target.
SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
             ".*~$" # Kate backup files.
             "CMakeLists.txt.user" # From QtCreator I think.
             "CPackConfig.cmake"
             "CPackSourceConfig.cmake"
            )

#=========================Configure brewtarget.qrc.in==========================

SET( brewtarget_QRC "${CMAKE_CURRENT_BINARY_DIR}/brewtarget.qrc" )
CONFIGURE_FILE( brewtarget.qrc.in ${brewtarget_QRC} )

#============================Doxygen Custom Target=============================
FIND_PROGRAM( DOXYGEN_CMD doxygen )
IF( DOXYGEN_CMD )
  ADD_CUSTOM_TARGET( source_doc
                     COMMAND ${DOXYGEN_CMD} Doxyfile
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc"
                   )
ENDIF()

#=============================Database Custom Target===========================
FIND_PROGRAM( SQLITE3_CMD sqlite3 )
IF( NOT SQLITE3_CMD )
   MESSAGE( FATAL_ERROR "Did not find sqlite3 to make the database." )
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
  SET( CMAKE_INSTALL_PREFIX "/" )
  SET( BT_BUNDLE "brewtarget.app" )
  SET( BT_BUNDLE_DIR "/Applications/${BT_BUNDLE}" )
  SET( DATAPATH "${BT_BUNDLE_DIR}/Contents/Resources" )
  SET( TARGETPATH "${BT_BUNDLE_DIR}/Contents/MacOS" )
  SET( DOCPATH  "${BT_BUNDLE_DIR}/Contents/Resources/en.lproj" )
ENDIF()

#=========================Configure brewtarget.desktop=========================

# If it begins with a '/'
IF( IS_ABSOLUTE ${DATAROOTDIR} )
   SET( brewtarget_ICONS_LOCATION "${DATAROOTDIR}/icons/${CMAKE_PROJECT_NAME}" )
ELSE()
   SET( brewtarget_ICONS_LOCATION "/${DATAROOTDIR}/icons/${CMAKE_PROJECT_NAME}" )
ENDIF()

CONFIGURE_FILE( brewtarget.desktop.in brewtarget.desktop )

#==============================Setup the config.h==============================

# "#define CONFIGDATADIR ${CONFIGDATADIR}$" in config.in will be replaced
# by the below corresponding value in ${CONFIGDATADIR} below when
# CONFIGURE_FILE() is called.
SET( CONFIGDATADIR "${DATAPATH}/" )
SET( CONFIGDOCDIR "${DOCPATH}/" )

# Tell cmake where the configure file is and where
# to put the output. All variables in config.in written as "${VAR}$
# will be replaced by VAR as determined by cmake in config.h.
# Outputs only in the build directory.
CONFIGURE_FILE( src/config.in src/config.h )

#================================Files=========================================

# List of documentation files to be installed.
SET( brewtarget_DOCS
     ${ROOTDIR}/README
     ${ROOTDIR}/COPYRIGHT
     ${ROOTDIR}/doc/manual/index.html
)

# List of data files to be installed.
SET( brewtarget_DATA
     ${DATADIR}/database.xml
     ${DATADIR}/mashs.xml
     ${DATADIR}/options.xml
     ${DATADIR}/recipes.xml
)

# Desktop files to install.
# This is not prefixed with ${ROOTDIR} b/c it is a configured file that will
# only exist in the build directory.
SET( brewtarget_DESKTOP
     ${CMAKE_CURRENT_BINARY_DIR}/brewtarget.desktop
)

# Icon files to install.
SET( brewtarget_ICONS
     ${ROOTDIR}/images/brewtarget_icon.svg
)

SET( TRANSLATIONS_FILES
     ${TRANSLATIONSDIR}/bt_ca.ts # Catalan
     ${TRANSLATIONSDIR}/bt_cs.ts # Czech
     ${TRANSLATIONSDIR}/bt_de.ts # German
     ${TRANSLATIONSDIR}/bt_en.ts # English
     ${TRANSLATIONSDIR}/bt_es.ts # Spanish
     ${TRANSLATIONSDIR}/bt_fr.ts # French
     ${TRANSLATIONSDIR}/bt_nb.ts # Norwegian Bokmal
     ${TRANSLATIONSDIR}/bt_it.ts # Italian
     ${TRANSLATIONSDIR}/bt_nl.ts # Dutch
     ${TRANSLATIONSDIR}/bt_pl.ts # Polish
     ${TRANSLATIONSDIR}/bt_pt.ts # Portuguese
     ${TRANSLATIONSDIR}/bt_hu.ts # Hungarian
     ${TRANSLATIONSDIR}/bt_sv.ts # Swedish
)

SET( RCFILE ${WINDIR}/icon.rc )

SET( brewtarget_SOUNDS
     ${DATADIR}/sounds/45minLeft.mp3
     ${DATADIR}/sounds/addFuckinHops.mp3
     ${DATADIR}/sounds/aromaHops.mp3
     ${DATADIR}/sounds/bitteringHops.mp3
     ${DATADIR}/sounds/checkBoil.mp3
     ${DATADIR}/sounds/checkFirstRunnings.mp3
     ${DATADIR}/sounds/checkGravity.mp3
     ${DATADIR}/sounds/checkHydrometer.mp3
     ${DATADIR}/sounds/checkMashTemps.mp3
     ${DATADIR}/sounds/checkTemp.mp3
     ${DATADIR}/sounds/clarifyingAgent.mp3
     ${DATADIR}/sounds/cleanup.mp3
     ${DATADIR}/sounds/closeFuckinValves.mp3
     ${DATADIR}/sounds/closeValves.mp3
     ${DATADIR}/sounds/doughIn.mp3
     ${DATADIR}/sounds/drinkAnotherHomebrew.mp3
     ${DATADIR}/sounds/drinkHomebrew.mp3
     ${DATADIR}/sounds/emptyMashTun.mp3
     ${DATADIR}/sounds/extraPropane.mp3
     ${DATADIR}/sounds/flameout.mp3
     ${DATADIR}/sounds/flavorHops.mp3
     ${DATADIR}/sounds/heatWater.mp3
     ${DATADIR}/sounds/mashHops.mp3
     ${DATADIR}/sounds/pitchYeast.mp3
     ${DATADIR}/sounds/sanitize.mp3
     ${DATADIR}/sounds/sparge.mp3
     ${DATADIR}/sounds/startBurner.mp3
     ${DATADIR}/sounds/startChill.mp3
     ${DATADIR}/sounds/stirMash.mp3
)

SET( brewtarget_INFO_PLIST "${ROOTDIR}/mac/Info.plist" )

SET( brewtarget_MAC_ICNS "${ROOTDIR}/mac/BrewtargetIcon.icns" )

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
ENDIF()

#===============================Packaging======================================
# To make the packages, "make package". For source packages, "make package_source"

IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  SET(CPACK_SET_DESTDIR "on")
  SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
  SET(CPACK_SOURCE_GENERATOR "TBZ2;")
  SET(CPACK_SOURCE_IGNORE_FILES 
      "/.svn/"
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
      "config\\\\.h"
  )

  SET( CPACK_PACKAGE_VERSION ${brewtarget_VERSION_STRING} )
  SET( CPACK_PACKAGE_VENDOR  "Philip G. Lee" )
  SET( CPACK_PACKAGE_CONTACT "Philip G. Lee <rocketman768@gmail.com>" )
  SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "GUI beer brewing software" )
  SET( CPACK_PACKAGE_DESCRIPTION
       "Brewtarget is a calculator for brewing beer. It is a Qt-based program which
       allows you to create recipes from a database of ingredients. It calculates all
       the important parameters, helps you with mash temperatures, and just makes the
       process of recipe formulation much easier." )

  SET(CPACK_PACKAGE_VERSION_MAJOR       ${brewtarget_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR       ${brewtarget_VERSION_MINOR})   # to create a debian confirm file-version-string
  SET(CPACK_PACKAGE_FILE_NAME           "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_${CMAKE_SYSTEM_PROCESSOR}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME    "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_source")
  SET(CPACK_PACKAGE_EXECUTABLES "${brewtarget_EXECUTABLE}" "Brewtarget" )
  IF( ${DO_RELEASE_BUILD} )
    SET(CPACK_STRIP_FILES ${brewtarget_EXECUTABLE})
  ENDIF()

  #===========================UNIX======================================
  IF(UNIX AND NOT APPLE)
    SET(CPACK_GENERATOR "DEB;RPM;TBZ2;")

    #-----------------DEB----------------
    SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "Philip G. Lee <rocketman768@gmail.com>" )
    # NOTE: Use the getdependencies script to get the dependencies!
    SET( CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.6-6~), libc6 (>= 2.1.3), libgcc1 (>= 1:4.1.1), libphonon4 (>= 4:4.2.0), libqt4-dbus (>= 4:4.5.3), libqt4-network (>= 4:4.5.3), libqt4-svg (>= 4:4.5.3), libqt4-webkit (>= 4:4.5.3), libqt4-xml (>= 4:4.5.3), libqtcore4 (>= 4:4.6.1), libqtgui4 (>= 4:4.5.3), libstdc++6 (>= 4.1.1), phonon (>= 4:4.5.2)" )

    SET( CPACK_DEBIAN_PACKAGE_SECTION "misc" )
    SET( CPACK_DEBIAN_PACKAGE_VERSION "${brewtarget_VERSION_STRING}-1")
    SET( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )

    SET(CPACK_DEBIAN_PACKAGE_NAME ${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING})

    #-----------------RPM-----------------
    SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
    SET(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")
    SET(CPACK_RPM_PACKAGE_REQUIRES "qt >= 4.6.0, phonon >= 4.2.0") # NOTE: I'm just guessing here. Somebody needs to check.
  #============================WINDOWS===============================
  ELSEIF(WIN32)
    # http://www.thegigsite.com/cmake-2.6.0/CMakeCPackOptions.cmake
    SET( CPACK_GENERATOR "NSIS" )
   
    SET( CPACK_RESOURCE_FILE_LICENSE "${ROOTDIR}/COPYING.GPLv3" )

    SET( CPACK_NSIS_PACKAGE_NAME "Brewtarget-${brewtarget_VERSION_STRING}" )
    SET( CPACK_NSIS_DISPLAY__NAME "Brewtarget-${brewtarget_VERSION_STRING}" )
    SET( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Brewtarget-${brewtarget_VERSION_STRING}" )
    SET( CPACK_PACKAGE_INSTALL_DIRECTORY "Brewtarget-${brewtarget_VERSION_STRING}" )
    SET( CPACK_NSIS_MODIFY_PATH ON )

    # Extra start menu items.
    #SET( CPACK_NSIS_MENU_LINKS
    #     "bin/${brewtarget_EXECUTABLE}" "My Brewtarget"
    #   )
  #============================APPLE=================================
  #ELSEIF(APPLE)
  #  SET( CPACK_GENERATOR "Bundle" )
  #
  #  SET( CPACK_BUNDLE_NAME            "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}" )
  #  SET( CPACK_BUNDLE_ICON            "../mac/BrewtargetIcon.icns" )
  #  SET( CPACK_BUNDLE_PLIST           "../mac/Info.plist" )
  #  SET( CPACK_BUNDLE_STARTUP_COMMAND "${brewtarget_EXECUTABLE}")
  #
  ELSEIF(APPLE)
    SET( CPACK_GENERATOR "DragNDrop" )
    SET( CPACK_DMG_FORMAT "UDBZ" )
  ENDIF()

 INCLUDE(CPack)
ENDIF()

#================================Installs======================================

IF( NOT ${BUILD_DESIGNER_PLUGINS} )

   # Install the data.
   INSTALL( FILES ${brewtarget_DATA}
            DESTINATION ${DATAPATH} )

   # Install the documentation.
   INSTALL( FILES ${brewtarget_DOCS}
            DESTINATION ${DOCPATH} )

   # Install sounds.
   INSTALL( FILES ${brewtarget_SOUNDS}
            DESTINATION "${DATAPATH}/sounds" )

   #-----------Unix-----------
   IF( UNIX AND NOT APPLE )
      # Install the icons.
      INSTALL( FILES ${brewtarget_ICONS}
               DESTINATION "${DATAROOTDIR}/icons/${CMAKE_PROJECT_NAME}" )

      # Install the .desktop file.
      INSTALL( FILES ${brewtarget_DESKTOP}
               DESTINATION "${DATAROOTDIR}/applications" )
   ENDIF()

   #--------Windows--------
   IF( WIN32 )
      INSTALL( FILES ${Qt_DLLs}
               DESTINATION "bin" )
      INSTALL( FILES ${Phonon_Backend_DLLs}
               DESTINATION "bin/phonon_backend" )
      INSTALL( FILES ${Image_Formats_DLLs}
               DESTINATION "bin/imageformats" )
      INSTALL( FILES ${Icon_Engines_DLLs}
               DESTINATION "bin/iconengines" )
   ENDIF()
ENDIF()

#---------Mac--------
# http://pmarinc-tidylib.googlecode.com/hg/src/Sigil/CMakeLists.txt?r=8276c61e05bc385d5ffbcc58e1f007f84b0c52df
IF( APPLE )
   # Info.plist
   INSTALL( FILES ${brewtarget_INFO_PLIST}
            DESTINATION "${BT_BUNDLE_DIR}/Contents" )
   # Icons
   INSTALL( FILES ${brewtarget_MAC_ICNS}
            DESTINATION ${DATAPATH} )

   # make addframeworks should copy the Qt libraries to the app.
   # May have to add a WORKING_DIRECTORY.
   ADD_CUSTOM_TARGET( addframeworks ALL
                      COMMAND ${QT_BINARY_DIR}/macdeployqt ${BT_BUNDLE_DIR} )
ENDIF()

# =====================Process other CMakeList.txt's===========================
IF( ${BUILD_DESIGNER_PLUGINS} )
   MESSAGE( STATUS "Building Qt Designer plugins." )
   ADD_SUBDIRECTORY(QtDesignerPlugins)
ELSE()
   MESSAGE( STATUS "Building Brewtarget." )
   ADD_SUBDIRECTORY(${SRCDIR})
ENDIF()
