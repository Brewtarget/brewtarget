PROJECT(brewtarget)
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.8 )
CMAKE_POLICY( VERSION 2.8.11 )

# Creates a Makefile.
# NOTE: cmake . -DCMAKE_INSTALL_PREFIX=/tmp/blah && make DESTDIR=/foo
# will install to /foo/tmp/blah.

# Where the binary goes.
IF( APPLE )
   # Use capital letters. Don't question the APPLE.
   SET(brewtarget_EXECUTABLE "Brewtarget")
ELSE()
   SET(brewtarget_EXECUTABLE "brewtarget")
ENDIF()

#=================================Version======================================
SET( brewtarget_VERSION_MAJOR 2 )
SET( brewtarget_VERSION_MINOR 3 )
SET( brewtarget_VERSION_PATCH 0 )
SET( brewtarget_VERSION_STRING "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}.${brewtarget_VERSION_PATCH}" )

#===================================Options====================================
OPTION( BUILD_DESIGNER_PLUGINS
        "If on, you will only build and install the designer plugins."
        OFF )
OPTION( DO_RELEASE_BUILD
        "If on, will do a release build. Otherwise, debug build."
        OFF )
OPTION( NO_MESSING_WITH_FLAGS
        "On means do not add any build flags whatsoever. May override other options."
        OFF )
OPTION( NO_QTMULTIMEDIA
        "On means QtMultimedia won't be linked to the final binary and related functionalities will be disabled."
        OFF )

# Do this right off the bat
ENABLE_TESTING()

#==============================Bundle Settings================================
SET( MACOSX_BUNDLE_BUNDLE_NAME "Brewtarget" )
#SET( MACOSX_BUNDLE_GUI_IDENTIFIER )
#SET( MACOSX_BUNDLE_INFO_STRING )
#SET( MACOSX_BUNDLE_BUNDLE_VERSION )
#SET( MACOSX_BUNDLE_SHORT_VERSION_STRING )
SET( MACOSX_BUNDLE_LONG_VERSION_STRING ${brewtarget_VERSION_STRING} )
SET( MACOSX_BUNDLE_ICON_FILE "BrewtargetIcon.icns" )
SET( MACOSX_BUNDLE_COPYRIGHT "GPLv3" )

#==============================Compile flags===================================
IF( NOT ${NO_MESSING_WITH_FLAGS} )
   IF( CMAKE_COMPILER_IS_GNUCXX )
      SET( CMAKE_CXX_FLAGS_RELEASE "-Wall -ansi -pedantic -Wno-long-long -O2" )
      SET( CMAKE_CXX_FLAGS_DEBUG "-Wall -g3" )
   ENDIF()

   # Speed up compilation if using gcc.
   IF( UNIX AND NOT APPLE )
      SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe" )
      SET( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pipe" )
   ENDIF()
ENDIF()

IF( ${NO_QTMULTIMEDIA} )
   ADD_DEFINITIONS( -DNO_QTMULTIMEDIA )
ENDIF()

IF( ${DO_RELEASE_BUILD} )
   SET( CMAKE_BUILD_TYPE "Release" )
ELSE()
   SET( CMAKE_BUILD_TYPE "Debug" )
ENDIF()

IF( APPLE AND NOT CMAKE_OSX_ARCHITECTURES )
   SET( CMAKE_OSX_ARCHITECTURES x86_64 ) # Build intel 64-bit binary. 
   #SET( CMAKE_OSX_ARCHITECTURES i386 x86_64 ) # Build intel binary. 
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

#==================================Find Qt5====================================

# Automatically run moc on source files when necessary
SET(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

FIND_PACKAGE(Qt5Core REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Widgets REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Network REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Network_INCLUDE_DIRS})

FIND_PACKAGE(Qt5PrintSupport REQUIRED)
INCLUDE_DIRECTORIES(${Qt5PrintSupport_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Qml REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Qml_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Sql REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Sql_INCLUDE_DIRS})

FIND_PACKAGE(Qt5WebKit REQUIRED)
INCLUDE_DIRECTORIES(${Qt5WebKit_INCLUDE_DIRS})

FIND_PACKAGE(Qt5WebKitWidgets REQUIRED)
INCLUDE_DIRECTORIES(${Qt5WebKitWidgets_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Xml REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Xml_INCLUDE_DIRS})

IF( NOT ${NO_QTMULTIMEDIA} )
FIND_PACKAGE(Qt5Multimedia REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Multimedia_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(Qt5Test REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Test_INCLUDE_DIRS})

FIND_PACKAGE(Qt5LinguistTools REQUIRED)
INCLUDE_DIRECTORIES(${Qt5LinguistTools_INCLUDE_DIRS})

# Need to find SVG component to locate svg dlls later
IF(WIN32)
   FIND_PACKAGE(Qt5Svg REQUIRED)
   INCLUDE_DIRECTORIES(${Qt5Svg_INCLUDE_DIRS})
ENDIF()

# Fuckin Qt5 requires -fPIC if Qt5 itself was built with -fPIC
IF(Qt5_POSITION_INDEPENDENT_CODE)
   SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()

#IF( ${BUILD_DESIGNER_PLUGINS} )
#   FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtNetwork QtSql QtWebKit QtXml QtDesigner REQUIRED)
#ELSE()
#   # This is a major kludge, but linking with QtSvg is the easiest way to get
#   # an app that includes the svg image format plugin that works.
#   IF( APPLE )
#      FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtNetwork QtSql QtWebkit QtXml QtSvg REQUIRED)
#   ELSE()
#      FIND_PACKAGE(Qt4 4.6.0 COMPONENTS QtCore QtGui QtNetwork QtSql QtWebkit QtXml REQUIRED)
#   ENDIF()
#ENDIF()

MESSAGE( STATUS "CMAKE_SYSTEM_INCLUDE_PATH: ${CMAKE_SYSTEM_INCLUDE_PATH}" )

# Some extra files for the "make clean" target.
SET_PROPERTY(
   DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
   ".*~$" # Kate backup files.
   "CMakeLists.txt.user" # From QtCreator I think.
   "CPackConfig.cmake"
   "CPackSourceConfig.cmake"
)

#=========================Configure brewtarget.qrc.in==========================

SET( brewtarget_QRC "${CMAKE_CURRENT_SOURCE_DIR}/brewtarget.qrc" )

#============================Doxygen Custom Target=============================
FIND_PROGRAM( DOXYGEN_CMD doxygen )
IF( DOXYGEN_CMD )
  SET( DOXYFILE "${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile" )
  CONFIGURE_FILE( "${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in" ${DOXYFILE} )
  ADD_CUSTOM_TARGET( source_doc
                     COMMAND ${DOXYGEN_CMD} ${DOXYFILE}
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
SET( BINDIR "${EXEC_PREFIX}/bin" )
IF( NOT DOCDIR )
   SET( DOCDIR "${DATAROOTDIR}/doc/${CMAKE_PROJECT_NAME}" )
ENDIF()

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
  #SET( CMAKE_INSTALL_PREFIX "" )
  SET( CPACK_INSTALL_PREFIX "" )
  SET( DATAPATH "data" )
  SET( TARGETPATH "bin" )
  SET( DOCPATH "doc" )
ELSEIF( APPLE )
  SET( DATAPATH "${BT_BUNDLE_DIR}/Contents/Resources" )
  SET( TARGETPATH "${BT_BUNDLE_DIR}/Contents/MacOS" )
  SET( DOCPATH  "${BT_BUNDLE_DIR}/Contents/Resources/en.lproj" )
ENDIF()

#==============================Setup the config.h==============================

# "#define CONFIGDATADIR ${CONFIGDATADIR}$" in config.in will be replaced
# by the below corresponding value in ${CONFIGDATADIR} below when
# CONFIGURE_FILE() is called.
IF( ${DATAPATH} MATCHES "^/" )
   SET( CONFIGDATADIR "${DATAPATH}/" )
ELSE()
   SET( CONFIGDATADIR "/${DATAPATH}/" )
ENDIF()

IF( ${DOCPATH} MATCHES "^/" )
   SET( CONFIGDOCDIR "${DOCPATH}/" )
ELSE()
   SET( CONFIGDOCDIR "/${DOCPATH}/" )
ENDIF()

# Tell cmake where the configure file is and where
# to put the output. All variables in config.in written as "${VAR}$
# will be replaced by VAR as determined by cmake in config.h.
# Outputs only in the build directory.
CONFIGURE_FILE( src/config.in src/config.h )

#================================Files=========================================

# List of documentation files to be installed.
SET( brewtarget_DOCS
     ${ROOTDIR}/README.markdown
     ${ROOTDIR}/COPYRIGHT
)

# List of data files to be installed.
SET( brewtarget_DATA
     ${DATADIR}/default_db.sqlite
# Yes, I know this is 'documentation', but Debian policy suggests it should be
# with the data (see section 12.3 of the policy manual).
     ${ROOTDIR}/doc/manual-en.pdf
)

# Desktop files to install.
SET( brewtarget_DESKTOP
     ${ROOTDIR}/brewtarget.desktop
)

# Icon files to install.
SET( brewtarget_ICONS
     ${ROOTDIR}/images/brewtarget.svg
)

SET( TRANSLATIONS_FILES
     ${TRANSLATIONSDIR}/bt_ca.ts # Catalan
     ${TRANSLATIONSDIR}/bt_cs.ts # Czech
     ${TRANSLATIONSDIR}/bt_de.ts # German
     ${TRANSLATIONSDIR}/bt_en.ts # English
     ${TRANSLATIONSDIR}/bt_el.ts # Greek
     ${TRANSLATIONSDIR}/bt_es.ts # Spanish
     ${TRANSLATIONSDIR}/bt_et.ts # Estonian
     ${TRANSLATIONSDIR}/bt_eu.ts # Basque
     ${TRANSLATIONSDIR}/bt_fr.ts # French
     ${TRANSLATIONSDIR}/bt_gl.ts # Galician
     ${TRANSLATIONSDIR}/bt_nb.ts # Norwegian Bokmal
     ${TRANSLATIONSDIR}/bt_it.ts # Italian
     ${TRANSLATIONSDIR}/bt_lv.ts # Latvian
     ${TRANSLATIONSDIR}/bt_nl.ts # Dutch
     ${TRANSLATIONSDIR}/bt_pl.ts # Polish
     ${TRANSLATIONSDIR}/bt_pt.ts # Portuguese
     ${TRANSLATIONSDIR}/bt_hu.ts # Hungarian
     ${TRANSLATIONSDIR}/bt_ru.ts # Russian
     ${TRANSLATIONSDIR}/bt_sr.ts # Serbian
     ${TRANSLATIONSDIR}/bt_sv.ts # Swedish
     ${TRANSLATIONSDIR}/bt_tr.ts # Turkish
     ${TRANSLATIONSDIR}/bt_zh.ts # Chinese
)

SET( RCFILE ${WINDIR}/icon.rc )

SET( brewtarget_SOUNDS
     ${DATADIR}/sounds/45minLeft.ogg
     ${DATADIR}/sounds/addFuckinHops.ogg
     ${DATADIR}/sounds/aromaHops.ogg
     ${DATADIR}/sounds/beep.ogg
     ${DATADIR}/sounds/bitteringHops.ogg
     ${DATADIR}/sounds/checkBoil.ogg
     ${DATADIR}/sounds/checkFirstRunnings.ogg
     ${DATADIR}/sounds/checkGravity.ogg
     ${DATADIR}/sounds/checkHydrometer.ogg
     ${DATADIR}/sounds/checkMashTemps.ogg
     ${DATADIR}/sounds/checkTemp.ogg
     ${DATADIR}/sounds/clarifyingAgent.ogg
     ${DATADIR}/sounds/cleanup.ogg
     ${DATADIR}/sounds/closeFuckinValves.ogg
     ${DATADIR}/sounds/closeValves.ogg
     ${DATADIR}/sounds/doughIn.ogg
     ${DATADIR}/sounds/drinkAnotherHomebrew.ogg
     ${DATADIR}/sounds/drinkHomebrew.ogg
     ${DATADIR}/sounds/emptyMashTun.ogg
     ${DATADIR}/sounds/extraPropane.ogg
     ${DATADIR}/sounds/flameout.ogg
     ${DATADIR}/sounds/flavorHops.ogg
     ${DATADIR}/sounds/heatWater.ogg
     ${DATADIR}/sounds/mashHops.ogg
     ${DATADIR}/sounds/pitchYeast.ogg
     ${DATADIR}/sounds/sanitize.ogg
     ${DATADIR}/sounds/sparge.ogg
     ${DATADIR}/sounds/startBurner.ogg
     ${DATADIR}/sounds/startChill.ogg
     ${DATADIR}/sounds/stirMash.ogg
)

SET( brewtarget_INFO_PLIST "${ROOTDIR}/mac/Info.plist" )

SET( brewtarget_MAC_ICNS "${ROOTDIR}/mac/BrewtargetIcon.icns" )

#==========================Find dlls for Win32=================================

IF( WIN32 )
   IF( NOT ${NO_QTMULTIMEDIA} )
   FIND_PACKAGE(Qt5MultimediaWidgets REQUIRED)
   ENDIF()
   FIND_PACKAGE(Qt5OpenGL REQUIRED)
   FIND_PACKAGE(Qt5Sensors REQUIRED)
   FIND_PACKAGE(Qt5Quick REQUIRED)
   FIND_PACKAGE(Qt5Positioning REQUIRED)

   GET_TARGET_PROPERTY(QtCore_location Qt5::Core LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtGui_location Qt5::Gui LOCATION_${CMAKE_BUILD_TYPE})
   IF( NOT ${NO_QTMULTIMEDIA} )
   GET_TARGET_PROPERTY(QtMultimedia_location Qt5::Multimedia LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtMultimediaWidgets_location Qt5::MultimediaWidgets LOCATION_${CMAKE_BUILD_TYPE})
   ENDIF()
   GET_TARGET_PROPERTY(QtNetwork_location Qt5::Network LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtOpenGL_location Qt5::OpenGL LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtPositioning_location Qt5::Positioning LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtPrintSupport_location Qt5::PrintSupport LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtQml_location Qt5::Qml LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtQuick_location Qt5::Quick LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtSensors_location Qt5::Sensors LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtSql_location Qt5::Sql LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtSvg_location Qt5::Svg LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtWebKit_location Qt5::WebKit LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtWebKitWidgets_location Qt5::WebKitWidgets LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtWidgets_location Qt5::Widgets LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtXml_location Qt5::Xml LOCATION_${CMAKE_BUILD_TYPE})

   GET_TARGET_PROPERTY(QtSqliteDriver_location Qt5::QSQLiteDriverPlugin LOCATION_${CMAKE_BUILD_TYPE})

   GET_TARGET_PROPERTY(QtQgif_location Qt5::QGifPlugin LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtQico_location Qt5::QICOPlugin LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtQjpeg_location Qt5::QJpegPlugin LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtQmng_location Qt5::QMngPlugin LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtQsvg_location Qt5::QSvgPlugin LOCATION_${CMAKE_BUILD_TYPE})
   GET_TARGET_PROPERTY(QtQtiff_location Qt5::QTiffPlugin LOCATION_${CMAKE_BUILD_TYPE})

   GET_TARGET_PROPERTY(QtQsvgIcon_location Qt5::QSvgIconPlugin LOCATION_${CMAKE_BUILD_TYPE})

   SET( Qt_DLLs
        ${QtCore_location}
        ${QtWidgets_location}
        ${QtNetwork_location}
        ${QtPrintSupport_location}
        ${QtQml_location}
        ${QtSql_location}
        ${QtSvg_location}
        ${QtXml_location}
        ${QtWebKit_location}
        ${QtGui_location}
        ${QtWebKitWidgets_location}
        ${QtOpenGL_location}
        ${QtSensors_location}
        ${QtQuick_location}
        ${QtPositioning_location}
   )
   IF( NOT ${NO_QTMULTIMEDIA})
      SET( Qt_DLLs
        ${Qt_DLLs}
        ${QtMultimedia_location}
        ${QtMultimediaWidgets_location}
   )
   ENDIF()
   IF (${Qt5Core_VERSION_STRING} VERSION_GREATER "5.3.0")
      FIND_PACKAGE(Qt5WebChannel REQUIRED)
      GET_TARGET_PROPERTY(QtWebChannel_location Qt5::WebChannel LOCATION_${CMAKE_BUILD_TYPE})
      list(APPEND Qt_DLLs ${QtWebChannel_location})
   ENDIF()
   SET( SQL_Drivers_DLLs
        ${QtSqliteDriver_location}
   )
   SET( Image_Formats_DLLs
        ${QtQgif_location}
        ${QtQico_location}
        ${QtQjpeg_location}
        ${QtQmng_location}
        ${QtQsvg_location}
        ${QtQtiff_location}
   )
   SET( Icon_Engines_DLLs
        ${QtQsvgIcon_location}
   )
   FILE( GLOB IcuDlls ${CMAKE_PREFIX_PATH}/bin/ic*.dll )

   # Find extra MinGW-specific dlls.
   IF( MINGW )
      IF( NOT MINGW_BIN_DIR )
         FIND_PATH(MINGW_BIN_DIR "mingw32-make.exe")
      ENDIF()
     IF( NOT EXISTS ${MINGW_BIN_DIR} )
        MESSAGE( FATAL_ERROR "MinGW bin dir not found. Run cmake again with the option -DMINGW_BIN_DIR=c:/path/to/mingw/bin" )
     ELSE()
        SET( Qt_DLLs
             ${Qt_DLLs}
             ${IcuDlls}
             ${MINGW_BIN_DIR}/libgcc_s_dw2-1.dll
             ${MINGW_BIN_DIR}/libstdc++-6.dll
             ${MINGW_BIN_DIR}/libwinpthread-1.dll
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
    SET(CPACK_RPM_SPEC_MORE_DEFINE "%define ignore \#")
    SET(CPACK_RPM_USER_FILELIST "%ignore /usr" "%ignore /usr/local" "%ignore /usr/local/bin" "%ignore /usr/local/share" "%ignore /usr/local/share/applications")
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
    #SET( CPACK_GENERATOR "DragNDrop" )
    #SET( CPACK_DMG_FORMAT "UDBZ" )
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
               DESTINATION "${DATAROOTDIR}/icons/hicolor/scalable/apps/" )

      # Install the .desktop file.
      INSTALL( FILES ${brewtarget_DESKTOP}
               DESTINATION "${DATAROOTDIR}/applications" )
   ENDIF()

   #--------Windows--------
   IF( WIN32 )
      INSTALL( FILES ${Qt_DLLs}
               DESTINATION "bin" )
      INSTALL( FILES ${SQL_Drivers_DLLs}
               DESTINATION "bin/sqldrivers" )
      INSTALL( FILES ${Image_Formats_DLLs}
               DESTINATION "bin/imageformats" )
      INSTALL( FILES ${Icon_Engines_DLLs}
               DESTINATION "bin/iconengines" )
   ENDIF()
ENDIF()

# =====================Process other CMakeList.txt's===========================
IF( ${BUILD_DESIGNER_PLUGINS} )
   MESSAGE( STATUS "Building Qt Designer plugins." )
   ADD_SUBDIRECTORY(QtDesignerPlugins)
ELSE()
   MESSAGE( STATUS "Building Brewtarget." )
   ADD_SUBDIRECTORY(${SRCDIR})
ENDIF()
