# Put all the required Qt dirs in the includes.
INCLUDE(${QT_USE_FILE})

# Where the binary goes.
SET(brewtarget_EXECUTABLE "../brewtarget")

# Variable that contains all the .cpp files in this project.
SET( brewtarget_SRCS
    ${SRCDIR}/Algorithms.cpp
    ${SRCDIR}/database.cpp
    ${SRCDIR}/BeerXMLElement.cpp
    ${SRCDIR}/BrewDayWidget.cpp
    ${SRCDIR}/BeerColorWidget.cpp
    ${SRCDIR}/brewtarget.cpp
    ${SRCDIR}/CelsiusTempUnitSystem.cpp
    ${SRCDIR}/ColorMethods.cpp
    ${SRCDIR}/ConverterTool.cpp
    ${SRCDIR}/equipment.cpp
    ${SRCDIR}/EquipmentComboBox.cpp
    ${SRCDIR}/EquipmentEditor.cpp
    ${SRCDIR}/FahrenheitTempUnitSystem.cpp
    ${SRCDIR}/fermentable.cpp
    ${SRCDIR}/FermentableEditor.cpp
    ${SRCDIR}/FermentableDialog.cpp
    ${SRCDIR}/FermentableTableModel.cpp
    ${SRCDIR}/FermentableTableWidget.cpp
    ${SRCDIR}/main.cpp
    ${SRCDIR}/HeatCalculations.cpp
    ${SRCDIR}/hop.cpp
    ${SRCDIR}/HopDialog.cpp
    ${SRCDIR}/HopEditor.cpp
    ${SRCDIR}/HopTableModel.cpp
    ${SRCDIR}/HopTableWidget.cpp
    ${SRCDIR}/HtmlViewer.cpp
    ${SRCDIR}/instruction.cpp
    ${SRCDIR}/IbuMethods.cpp
    ${SRCDIR}/ImperialVolumeUnitSystem.cpp
    ${SRCDIR}/InstructionWidget.cpp
    ${SRCDIR}/MainWindow.cpp
    ${SRCDIR}/MaltinessWidget.cpp
    ${SRCDIR}/mash.cpp
    ${SRCDIR}/MashComboBox.cpp
    ${SRCDIR}/MashEditor.cpp
    ${SRCDIR}/mashstep.cpp
    ${SRCDIR}/MashStepEditor.cpp
    ${SRCDIR}/MashStepTableModel.cpp
    ${SRCDIR}/MashStepTableWidget.cpp
    ${SRCDIR}/MashWizard.cpp
    ${SRCDIR}/matrix.cpp
    ${SRCDIR}/misc.cpp
    ${SRCDIR}/MiscEditor.cpp
    ${SRCDIR}/MiscDialog.cpp
    ${SRCDIR}/MiscTableModel.cpp
    ${SRCDIR}/MiscTableWidget.cpp
    ${SRCDIR}/observable.cpp
    ${SRCDIR}/observer.cpp
    ${SRCDIR}/OgAdjuster.cpp
    ${SRCDIR}/OptionDialog.cpp
    ${SRCDIR}/PreInstruction.cpp
    ${SRCDIR}/PrimingDialog.cpp
    ${SRCDIR}/recipe.cpp
    ${SRCDIR}/RecipeComboBox.cpp
    ${SRCDIR}/RecipeFormatter.cpp
    ${SRCDIR}/RefractoDialog.cpp
    ${SRCDIR}/ScaleRecipeTool.cpp
    ${SRCDIR}/SIVolumeUnitSystem.cpp
    ${SRCDIR}/SIWeightUnitSystem.cpp
    ${SRCDIR}/stringparsing.cpp
    ${SRCDIR}/style.cpp
    ${SRCDIR}/StyleComboBox.cpp
    ${SRCDIR}/StyleEditor.cpp
    ${SRCDIR}/TimerListDialog.cpp
    ${SRCDIR}/TimerWidget.cpp
    ${SRCDIR}/TimeUnitSystem.cpp
    ${SRCDIR}/unit.cpp
    ${SRCDIR}/UnitSystem.cpp
    ${SRCDIR}/USVolumeUnitSystem.cpp
    ${SRCDIR}/USWeightUnitSystem.cpp
    ${SRCDIR}/water.cpp
    ${SRCDIR}/WaterTableModel.cpp
    ${SRCDIR}/WaterTableWidget.cpp
    ${SRCDIR}/xml.cpp
    ${SRCDIR}/xmlnode.cpp
    ${SRCDIR}/xmltree.cpp
    ${SRCDIR}/yeast.cpp
    ${SRCDIR}/YeastDialog.cpp
    ${SRCDIR}/YeastEditor.cpp
    ${SRCDIR}/YeastTableModel.cpp
    ${SRCDIR}/YeastTableWidget.cpp
    ${SRCDIR}/RecipeExtrasDialog.cpp
    ${SRCDIR}/MashDesigner.cpp
    ${SRCDIR}/BtDigitWidget.cpp
    ${SRCDIR}/PitchDialog.cpp
)

# List of all the *.ui.
# TODO: can I somehow have a separate CMakeLists.txt
# in the ui/ directory instead of here?
SET( brewtarget_UIS
    ${UIDIR}/aboutDialog.ui
    ${UIDIR}/brewDayWidget.ui
    ${UIDIR}/converterTool.ui
    ${UIDIR}/mainWindow.ui
    ${UIDIR}/mashStepEditor.ui
    ${UIDIR}/miscEditor.ui
    ${UIDIR}/fermentableEditor.ui
    ${UIDIR}/fermentableDialog.ui
    ${UIDIR}/equipmentEditor.ui
    ${UIDIR}/hopDialog.ui
    ${UIDIR}/hopEditor.ui
    ${UIDIR}/htmlViewer.ui
    ${UIDIR}/instructionWidget.ui
    ${UIDIR}/mashEditor.ui
    ${UIDIR}/mashWizard.ui
    ${UIDIR}/miscDialog.ui
    ${UIDIR}/ogAdjuster.ui
    ${UIDIR}/optionsDialog.ui
    ${UIDIR}/primingDialog.ui
    ${UIDIR}/recipeExtrasDialog.ui
    ${UIDIR}/refractoDialog.ui
    ${UIDIR}/scaleRecipeTool.ui
    ${UIDIR}/styleEditor.ui
    ${UIDIR}/timerListDialog.ui
    ${UIDIR}/timerWidget.ui
    ${UIDIR}/yeastDialog.ui
    ${UIDIR}/yeastEditor.ui
    ${UIDIR}/mashDesigner.ui
    ${UIDIR}/pitchDialog.ui
)

# The list of files that need to be moc'd.
# In other words, classes that have Q_OBJECT declarations and stuff.
SET( brewtarget_MOC_HEADERS
    ${SRCDIR}/BeerColorWidget.h
    ${SRCDIR}/BrewDayWidget.h
    ${SRCDIR}/BtDigitWidget.h
    ${SRCDIR}/ConverterTool.h
    ${SRCDIR}/EquipmentComboBox.h
    ${SRCDIR}/EquipmentEditor.h
    ${SRCDIR}/FermentableEditor.h
    ${SRCDIR}/FermentableDialog.h
    ${SRCDIR}/FermentableTableModel.h
    ${SRCDIR}/FermentableTableWidget.h
    ${SRCDIR}/HopDialog.h
    ${SRCDIR}/HopEditor.h
    ${SRCDIR}/HopTableModel.h
    ${SRCDIR}/HopTableWidget.h
    ${SRCDIR}/InstructionWidget.h
    ${SRCDIR}/MainWindow.h
    ${SRCDIR}/MaltinessWidget.h
    ${SRCDIR}/MashComboBox.h
    ${SRCDIR}/MashDesigner.h
    ${SRCDIR}/MashEditor.h
    ${SRCDIR}/MashStepEditor.h
    ${SRCDIR}/MashStepTableModel.h
    ${SRCDIR}/MashStepTableWidget.h
    ${SRCDIR}/MashWizard.h
    ${SRCDIR}/MiscDialog.h
    ${SRCDIR}/MiscEditor.h
    ${SRCDIR}/MiscTableModel.h
    ${SRCDIR}/MiscTableWidget.h
    ${SRCDIR}/OgAdjuster.h
    ${SRCDIR}/OptionDialog.h
    ${SRCDIR}/PitchDialog.h
    ${SRCDIR}/PrimingDialog.h
    ${SRCDIR}/RecipeComboBox.h
    ${SRCDIR}/RecipeExtrasDialog.h
    ${SRCDIR}/RecipeFormatter.h
    ${SRCDIR}/RefractoDialog.h
    ${SRCDIR}/ScaleRecipeTool.h
    ${SRCDIR}/StyleComboBox.h
    ${SRCDIR}/StyleEditor.h
    ${SRCDIR}/TimerListDialog.h
    ${SRCDIR}/TimerWidget.h
    ${SRCDIR}/WaterTableModel.h
    ${SRCDIR}/WaterTableWidget.h
    ${SRCDIR}/YeastDialog.h
    ${SRCDIR}/YeastEditor.h
    ${SRCDIR}/YeastTableModel.h
    ${SRCDIR}/YeastTableWidget.h
)

# List of documentation files to be installed.
SET( brewtarget_DOCS
     ${ROOTDIR}/README
     ${ROOTDIR}/COPYING.GPLv3
     ${ROOTDIR}/COPYING.WTFPL
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
SET( brewtarget_DESKTOP
     ${ROOTDIR}/brewtarget.desktop
)

# Icon files to install.
SET( brewtarget_ICONS
     ${ROOTDIR}/images/BrewtargetIcon.png
)

SET( TRANSLATIONS_FILES
     ${TRANSLATIONSDIR}/bt_de.ts # German
     ${TRANSLATIONSDIR}/bt_en.ts # English
     ${TRANSLATIONSDIR}/bt_es.ts # Spanish
     ${TRANSLATIONSDIR}/bt_fr.ts # French
     ${TRANSLATIONSDIR}/bt_pl.ts # Polish
     ${TRANSLATIONSDIR}/bt_pt.ts # Portuguese
)

SET( FILES_TO_TRANSLATE
     ${brewtarget_SRCS}
     ${brewtarget_UIS}
)

#==================================Qt Junk=====================================

# Create the moc sources from the headers.
QT4_WRAP_CPP(brewtarget_MOC_SRCS ${brewtarget_MOC_HEADERS})

# Create the ui_*.h files from the *.ui files.
QT4_WRAP_UI(brewtarget_UIS_H ${brewtarget_UIS})

# Generate the resource .cpp from the .qrc file.
QT4_ADD_RESOURCES(brewtarget_QRC_CPP "../brewtarget.qrc")

# Translations
OPTION(UPDATE_TRANSLATIONS
       "Update source translation *.ts files (WARNING: make clean will delete the source .ts files! Danger!)"
       OFF)
IF(UPDATE_TRANSLATIONS)
  # Updates the .ts files with translatable strings, AND creates the binary .qm files
  QT4_CREATE_TRANSLATION(QM_FILES
                         ${FILES_TO_TRANSLATE}
                         ${TRANSLATIONS_FILES}
  )
ELSE()
  # Creates the .qm files without modifying the .ts files.
  QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS_FILES})
ENDIF() 

ADD_CUSTOM_TARGET(translations DEPENDS ${QM_FILES})

#INSTALL(FILES ${QM_FILES}
#        DESTINATION ${somewhere}/translations)

#===============================Packaging======================================

# NOTE: For now, this only calls CPack when we are unix. Need to add support
# for apple bundles and microsoft NSIS installer.
#
# To make the packages, "make package". For source packages, "make package_source"
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake" AND UNIX AND NOT APPLE)
  MESSAGE(STATUS "CMakeRoot: ${CMAKE_ROOT}/Modules/CPack.cmake")

  SET(CPACK_SET_DESTDIR "on")
  SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
  SET(CPACK_GENERATOR "DEB;RPM;TBZ2;")
  SET(CPACK_SOURCE_GENERATOR "TBZ2;")
  SET(CPACK_SOURCE_IGNORE_FILES 
      "/.svn/"
      "~$"
      "/CMakeFiles/"
      "/_CPack_Packages/"
      "brewtarget.*deb"
      "brewtarget.*rpm"
      "brewtarget.*tar.*"
      "CPack.*"
      "Makefile"
      "cmake_install.*"
      "\\\\.o"
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
  SET( CPACK_PACKAGE_DESCRIPTION "Brewtarget is a calculator for brewing beer. It is a Qt-based program which
       allows you to create recipes from a database of ingredients. It calculates all
       the important parameters, helps you with mash temperatures, and just makes the
       process of recipe formulation much easier." )

#	SET(CPACK_RESOURCE_FILE_LICENSE		${CMAKE_CURRENT_SOURCE_DIR}//COPYING)
  SET(CPACK_PACKAGE_VERSION_MAJOR       ${brewtarget_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR       ${brewtarget_VERSION_MINOR})	# to create a debian confirm file-version-string
  SET(CPACK_PACKAGE_FILE_NAME           "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_${CMAKE_SYSTEM_PROCESSOR}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME    "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_source")
  SET( CPACK_PACKAGE_EXECUTABLES "${brewtarget_EXECUTABLE}" "Brewtarget" )
  #SET(CPACK_STRIP_FILES TRUE)
  SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)

  #-----------------DEB----------------
  SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "Philip G. Lee <rocketman768@gmail.com>" )
  # NOTE: Use the getdependencies script to get the dependencies!
  SET( CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.6-6~), libc6 (>= 2.4), libgcc1 (>= 1:4.1.1), libphonon4 (>= 4:4.2.0), libqt4-dbus (>= 4:4.5.3), libqt4-svg (>= 4:4.5.3), libqt4-webkit (>= 4:4.5.3), libqt4-xml (>= 4:4.5.3), libqtcore4 (>= 4:4.6.1), libqtgui4 (>= 4:4.5.3), libstdc++6 (>= 4.1.1), libx11-6, phonon (>= 4:4.5.2)
" )
  SET( CPACK_DEBIAN_PACKAGE_SECTION "misc" )
  SET( CPACK_DEBIAN_PACKAGE_VERSION "${brewtarget_VERSION_STRING}-1")
  SET( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )

  FIND_PROGRAM(DPKG_CMD dpkg)
  IF(DPKG_CMD)
    EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET(CPACK_DEBIAN_PACKAGE_NAME ${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
  ENDIF(DPKG_CMD)

  #-----------------RPM-----------------
  SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
  SET(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")
  SET(CPACK_RPM_PACKAGE_REQUIRES "qt(>= 4.6.0), phonon(>= 4.2.0)") # NOTE: I'm just guessing here. Somebody needs to check.

  INCLUDE(CPack)
ENDIF()

#===========================Create the binary==================================

# The executable.
IF( APPLE )
   ADD_EXECUTABLE( ${brewtarget_EXECUTABLE} MACOSX_BUNDLE
                   ${brewtarget_SRCS} ${brewtarget_MOC_SRCS} ${brewtarget_QRC_CPP} ${brewtarget_UIS_H}
                   # Do I need to include this since I want them to go in the bundle?
                   ${brewtarget_DOCS} ${brewtarget_DATA}
   )

   # Want the docs in the en.lproj folder.
   SET_SOURCE_FILES_PROPERTIES(
      ${brewtarget_DOCS}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources/en.lproj
   )

   # Want the data in the resources folder.
   SET_SOURCE_FILES_PROPERTIES(
      ${brewtarget_DATA}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
   )

   SET( MACOSX_BUNDLE_INFO_STRING "Info string" )
   SET( MACOSX_BUNDLE_ICON_FILE "../mac/BrewtargetIcon.icns" )
   SET( MACOSX_BUNDLE_GUI_IDENTIFIER "Bundle gui identifier" )
   SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}.${brewtarget_VERSION_PATCH}" )
   SET( MACOSX_BUNDLE_BUNDLE_NAME "Brewtarget" )
   SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}" )
   SET( MACOSX_BUNDLE_BUNDLE_VERSION "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}.${brewtarget_VERSION_PATCH}" )
   SET( MACOSX_BUNDLE_COPYRIGHT "Brewtarget is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Brewtarget is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with brewtarget;if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA, or visit
    http://www.gnu.org/copyleft/gpl.html"
   )
ELSE()
   ADD_EXECUTABLE( ${brewtarget_EXECUTABLE} ${brewtarget_SRCS} ${brewtarget_MOC_SRCS} ${brewtarget_QRC_CPP} ${brewtarget_UIS_H} )
ENDIF()

# Link brewtarget against Qt libraries defined by QT_USE_FILE.
TARGET_LINK_LIBRARIES(${brewtarget_EXECUTABLE} ${QT_LIBRARIES})

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
  # No directories to configure here I think.
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

#==============================Setup install targets===========================

IF(APPLE)

   # I don't think we need to do any install targets here.

ELSE()

   # Install the executable.
   INSTALL( TARGETS ${brewtarget_EXECUTABLE}
            RUNTIME DESTINATION ${TARGETPATH} )

   # Install the data.
   INSTALL( FILES ${brewtarget_DATA}
            DESTINATION ${DATAPATH} )

   # Install the documentation.
   INSTALL( FILES ${brewtarget_DOCS}
            DESTINATION ${DOCPATH} )

ENDIF()

# Some extra targets for unix.
IF( UNIX AND NOT APPLE )

   # Install the icons.
   INSTALL( FILES ${brewtarget_ICONS}
            DESTINATION "${DATAROOTDIR}/icons/${CMAKE_PROJECT_NAME}" )

   # Install the .desktop file.
   INSTALL( FILES ${brewtarget_DESKTOP}
            DESTINATION "${DATAROOTDIR}/applications" )

ENDIF()
