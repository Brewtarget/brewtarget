# Put all the required Qt dirs in the includes.
INCLUDE(${QT_USE_FILE})

# Variable that contains all the .cpp files in this project.
SET( brewtarget_SRCS
    ${SRCDIR}/Algorithms.cpp
    ${SRCDIR}/database.cpp
    ${SRCDIR}/BeerXMLElement.cpp
    ${SRCDIR}/BrewDayWidget.cpp
    ${SRCDIR}/BrewDayScrollWidget.cpp
    ${SRCDIR}/BrewTargetTreeItem.cpp
    ${SRCDIR}/BrewTargetTreeModel.cpp
    ${SRCDIR}/BrewTargetTreeView.cpp
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
    ${SRCDIR}/FermentableSortFilterProxyModel.cpp
    ${SRCDIR}/FermentableTableModel.cpp
    ${SRCDIR}/FermentableTableWidget.cpp
    ${SRCDIR}/main.cpp
    ${SRCDIR}/HeatCalculations.cpp
    ${SRCDIR}/hop.cpp
    ${SRCDIR}/HopDialog.cpp
    ${SRCDIR}/HopEditor.cpp
    ${SRCDIR}/HopSortFilterProxyModel.cpp
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
    ${SRCDIR}/MiscSortFilterProxyModel.cpp
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
    ${SRCDIR}/style.cpp
    ${SRCDIR}/StyleComboBox.cpp
    ${SRCDIR}/StyleEditor.cpp
    ${SRCDIR}/TimerListDialog.cpp
    ${SRCDIR}/TimerWidget.cpp
    ${SRCDIR}/TimeUnitSystem.cpp
    ${SRCDIR}/unit.cpp
    ${SRCDIR}/UnitSystems.cpp
    ${SRCDIR}/USVolumeUnitSystem.cpp
    ${SRCDIR}/USWeightUnitSystem.cpp
    ${SRCDIR}/water.cpp
    ${SRCDIR}/WaterTableModel.cpp
    ${SRCDIR}/WaterTableWidget.cpp
    ${SRCDIR}/WaterEditor.cpp
    ${SRCDIR}/yeast.cpp
    ${SRCDIR}/YeastDialog.cpp
    ${SRCDIR}/YeastEditor.cpp
    ${SRCDIR}/YeastTableModel.cpp
    ${SRCDIR}/YeastTableWidget.cpp
    ${SRCDIR}/YeastSortFilterProxyModel.cpp
    ${SRCDIR}/RecipeExtrasDialog.cpp
    ${SRCDIR}/MashDesigner.cpp
    ${SRCDIR}/BtDigitWidget.cpp
    ${SRCDIR}/PitchDialog.cpp
    ${SRCDIR}/RecipeExtrasWidget.cpp
)

# List of all the *.ui.
# TODO: can I somehow have a separate CMakeLists.txt
# in the ui/ directory instead of here?
SET( brewtarget_UIS
    ${UIDIR}/aboutDialog.ui
    ${UIDIR}/brewDayWidget.ui
    ${UIDIR}/brewDayScrollWidget.ui
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
    ${UIDIR}/waterEditor.ui
    ${UIDIR}/yeastDialog.ui
    ${UIDIR}/yeastEditor.ui
    ${UIDIR}/mashDesigner.ui
    ${UIDIR}/pitchDialog.ui
    ${UIDIR}/brewDayScrollWidget.ui
    ${UIDIR}/recipeExtrasWidget.ui
)

# The list of files that need to be moc'd.
# In other words, classes that have Q_OBJECT declarations and stuff.
SET( brewtarget_MOC_HEADERS
    ${SRCDIR}/BeerColorWidget.h
    ${SRCDIR}/BrewDayWidget.h
    ${SRCDIR}/BrewDayScrollWidget.h
    ${SRCDIR}/BrewTargetTreeModel.h
    ${SRCDIR}/BrewTargetTreeView.h
    ${SRCDIR}/BtDigitWidget.h
    ${SRCDIR}/ConverterTool.h
    ${SRCDIR}/EquipmentComboBox.h
    ${SRCDIR}/EquipmentEditor.h
    ${SRCDIR}/FermentableEditor.h
    ${SRCDIR}/FermentableDialog.h
    ${SRCDIR}/FermentableSortFilterProxyModel.h
    ${SRCDIR}/FermentableTableModel.h
    ${SRCDIR}/FermentableTableWidget.h
    ${SRCDIR}/HopDialog.h
    ${SRCDIR}/HopEditor.h
    ${SRCDIR}/HopSortFilterProxyModel.h
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
    ${SRCDIR}/MiscSortFilterProxyModel.h
    ${SRCDIR}/MiscTableModel.h
    ${SRCDIR}/MiscTableWidget.h
    ${SRCDIR}/OgAdjuster.h
    ${SRCDIR}/OptionDialog.h
    ${SRCDIR}/PitchDialog.h
    ${SRCDIR}/PrimingDialog.h
    ${SRCDIR}/RecipeComboBox.h
    ${SRCDIR}/RecipeExtrasDialog.h
    ${SRCDIR}/RecipeExtrasWidget.h
    ${SRCDIR}/RecipeFormatter.h
    ${SRCDIR}/RefractoDialog.h
    ${SRCDIR}/ScaleRecipeTool.h
    ${SRCDIR}/StyleComboBox.h
    ${SRCDIR}/StyleEditor.h
    ${SRCDIR}/TimerListDialog.h
    ${SRCDIR}/TimerWidget.h
    ${SRCDIR}/WaterTableModel.h
    ${SRCDIR}/WaterTableWidget.h
    ${SRCDIR}/WaterEditor.h
    ${SRCDIR}/YeastDialog.h
    ${SRCDIR}/YeastEditor.h
    ${SRCDIR}/YeastSortFilterProxyModel.h
    ${SRCDIR}/YeastTableModel.h
    ${SRCDIR}/YeastTableWidget.h
)

# List of documentation files to be installed.
SET( brewtarget_DOCS
     ${ROOTDIR}/README
#     ${ROOTDIR}/COPYING.GPLv3
#     ${ROOTDIR}/COPYING.WTFPL
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
     ${TRANSLATIONSDIR}/bt_cs.ts # Czech
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

#==================================Qt Junk=====================================

# Create the moc sources from the headers.
QT4_WRAP_CPP(brewtarget_MOC_SRCS ${brewtarget_MOC_HEADERS})

# Create the ui_*.h files from the *.ui files.
QT4_WRAP_UI(brewtarget_UIS_H ${brewtarget_UIS})

# Generate the resource .cpp from the .qrc file.
QT4_ADD_RESOURCES(brewtarget_QRC_CPP "../brewtarget.qrc")

# Translations
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

# Add a target for the QM_FILES so that we can add
# the translations as a dependency for the executable later.
ADD_CUSTOM_TARGET(translations DEPENDS ${QM_FILES})

#===============================Packaging======================================

# NOTE: For now, this only calls CPack when we are unix. Need to add support
# for apple bundles and microsoft NSIS installer.
#
# To make the packages, "make package". For source packages, "make package_source"
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake" AND UNIX AND NOT APPLE)
  MESSAGE(STATUS "CMakeRoot: ${CMAKE_ROOT}/Modules/CPack.cmake")

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
  SET( CPACK_PACKAGE_DESCRIPTION "Brewtarget is a calculator for brewing beer. It is a Qt-based program which
       allows you to create recipes from a database of ingredients. It calculates all
       the important parameters, helps you with mash temperatures, and just makes the
       process of recipe formulation much easier." )

#	SET(CPACK_RESOURCE_FILE_LICENSE		${CMAKE_CURRENT_SOURCE_DIR}//COPYING)
  SET(CPACK_PACKAGE_VERSION_MAJOR       ${brewtarget_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR       ${brewtarget_VERSION_MINOR})	# to create a debian confirm file-version-string
  SET(CPACK_PACKAGE_FILE_NAME           "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_${CMAKE_SYSTEM_PROCESSOR}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME    "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_source")
  SET(CPACK_PACKAGE_EXECUTABLES "${brewtarget_EXECUTABLE}" "Brewtarget" )
  #SET(CPACK_STRIP_FILES TRUE)
  SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)

 #===========================UNIX======================================
 IF(UNIX)
  SET(CPACK_GENERATOR "DEB;RPM;TBZ2;")

  #-----------------DEB----------------
  SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "Philip G. Lee <rocketman768@gmail.com>" )
  # NOTE: Use the getdependencies script to get the dependencies!
  SET( CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.6-6~), libc6 (>= 2.4), libgcc1 (>= 1:4.1.1), libphonon4 (>= 4:4.2.0), libqt4-dbus (>= 4:4.5.3), libqt4-svg (>= 4:4.5.3), libqt4-webkit (>= 4:4.5.3), libqt4-xml (>= 4:4.5.3), libqtcore4 (>= 4:4.6.1), libqtgui4 (>= 4:4.5.3), libstdc++6 (>= 4.1.1), libx11-6, phonon (>= 4:4.5.2)" )

  SET( CPACK_DEBIAN_PACKAGE_SECTION "misc" )
  SET( CPACK_DEBIAN_PACKAGE_VERSION "${brewtarget_VERSION_STRING}-1")
  SET( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )

  #FIND_PROGRAM(DPKG_CMD dpkg)
  #IF(DPKG_CMD)
  #  EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
  #    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
  #    OUTPUT_STRIP_TRAILING_WHITESPACE)
  #  SET(CPACK_DEBIAN_PACKAGE_NAME ${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
  #ENDIF(DPKG_CMD)

  SET(CPACK_DEBIAN_PACKAGE_NAME ${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING})

  #-----------------RPM-----------------
  SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
  SET(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")
  SET(CPACK_RPM_PACKAGE_REQUIRES "qt >= 4.6.0, phonon >= 4.2.0") # NOTE: I'm just guessing here. Somebody needs to check.

 #============================APPLE=================================
 ELSEIF(APPLE)
  #---------------App Bundle---------------
  SET( CPACK_GENERATOR "Bundle" )

  SET( CPACK_BUNDLE_NAME            "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}" )
  SET( CPACK_BUNDLE_ICON            "../mac/BrewtargetIcon.icns" )
  SET( CPACK_BUNDLE_PLIST           "../mac/Info.plist" )
  SET( CPACK_BUNDLE_STARTUP_COMMAND "${brewtarget_EXECUTABLE}")

 ENDIF()#========================================================================

 INCLUDE(CPack)
ENDIF()

#============================Icon for Windows==================================

IF( WIN32 AND MINGW )
  ADD_CUSTOM_COMMAND(OUTPUT ${WINDIR}/icon.o
                     COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                     -i${RCFILE}
                     -o${WINDIR}/icon.o
                     DEPENDS ${RCFILE}
  )
  SET(brewtarget_SRCS ${brewtarget_SRCS} ${WINDIR}/icon.o)
ELSEIF(WIN32)
  SET(brewtarget_SRCS ${brewtarget_SRCS} ${RCFILE})
ENDIF()

#===========================Create the binary==================================

ADD_EXECUTABLE( ${brewtarget_EXECUTABLE} ${brewtarget_SRCS} ${brewtarget_MOC_SRCS} ${brewtarget_QRC_CPP} ${brewtarget_UIS_H} )
ADD_DEPENDENCIES( ${brewtarget_EXECUTABLE} translations )

# Link brewtarget against appropriate libraries.
IF( WIN32 AND MINGW )
   ############################################################################
   # I don't know why, but QT_LIBRARIES is NOT correct under mingw32.
   # It outputs things like "C:/Qt/4.7.1/lib/QtSvg4.lib".
   #
   # Also, need to set some linker flags that I don't know how to get
   # automatically.
   ############################################################################
   TARGET_LINK_LIBRARIES(${brewtarget_EXECUTABLE} mingw32 QtWebKit4 QtXml4 QtNetwork4 QtGui4 QtCore4 QtSvg4 phonon4)
   SET_TARGET_PROPERTIES( ${brewtarget_EXECUTABLE} PROPERTIES LINK_FLAGS "-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc -Wl,-s -mthreads -Wl,-subsystem,windows -Lc:/Qt/4.7.1/lib" )
ELSE()
   # Link brewtarget against Qt libraries defined by QT_LIBRARIES
   TARGET_LINK_LIBRARIES(${brewtarget_EXECUTABLE} ${QT_LIBRARIES})
ENDIF()

MESSAGE( STATUS "Libraries: ${QT_LIBRARIES}" )

#==============================Setup install targets===========================

# Install the data.
INSTALL( FILES ${brewtarget_DATA}
         DESTINATION "${DATAPATH}" )

# Install the translations
INSTALL(FILES ${QM_FILES}
        DESTINATION "${DATAPATH}/translations_qm")

# Install the documentation.
INSTALL( FILES ${brewtarget_DOCS}
         DESTINATION "${DOCPATH}" )

# Install sounds.
INSTALL( FILES ${brewtarget_SOUNDS}
         DESTINATION "${DATAPATH}/sounds" )

# ---------Unix installs-----------
IF( UNIX AND NOT APPLE )

   INSTALL( TARGETS ${brewtarget_EXECUTABLE}
            RUNTIME DESTINATION "${TARGETPATH}" )

   # Install the icons.
   INSTALL( FILES ${brewtarget_ICONS}
            DESTINATION "${DATAROOTDIR}/icons/${CMAKE_PROJECT_NAME}" )

   # Install the .desktop file.
   INSTALL( FILES ${brewtarget_DESKTOP}
            DESTINATION "${DATAROOTDIR}/applications" )
ENDIF()

#--------Windows installs---------
IF( WIN32 )
   INSTALL( TARGETS ${brewtarget_EXECUTABLE}
            RUNTIME DESTINATION "${TARGETPATH}" )
ENDIF()
