# The current directory in the original source.
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
# The current directory in the output build directory.
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# Put all the required Qt dirs in the includes.
INCLUDE(${QT_USE_FILE})

# Variable that contains all the .cpp files in this project.
SET( brewtarget_SRCS
    ${SRCDIR}/Algorithms.cpp
    ${SRCDIR}/database.cpp
    ${SRCDIR}/BeerXMLElement.cpp
    ${SRCDIR}/BrewDayWidget.cpp
    ${SRCDIR}/BrewDayScrollWidget.cpp
    ${SRCDIR}/brewnote.cpp
    ${SRCDIR}/BrewNoteWidget.cpp
    ${SRCDIR}/BrewTargetTreeItem.cpp
    ${SRCDIR}/BrewTargetTreeModel.cpp
    ${SRCDIR}/BrewTargetTreeView.cpp
    ${SRCDIR}/BeerColorWidget.cpp
    ${SRCDIR}/brewtarget.cpp
    ${SRCDIR}/BtSplashScreen.cpp
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
    ${SRCDIR}/UnitSystem.cpp
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
    ${UIDIR}/brewNoteWidget.ui
    ${UIDIR}/btSplashScreen.ui
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
    ${SRCDIR}/BrewNoteWidget.h
    ${SRCDIR}/BrewTargetTreeModel.h
    ${SRCDIR}/BrewTargetTreeView.h
    ${SRCDIR}/BtDigitWidget.h
    ${SRCDIR}/BtSplashScreen.h
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

SET( FILES_TO_TRANSLATE
     ${brewtarget_SRCS}
     ${brewtarget_UIS}
)

#==============================Setup the config.h==============================

# "#define CONFIGDATADIR ${CONFIGDATADIR}$" in config.in will be replaced
# by the below corresponding value in ${CONFIGDATADIR} below when
# CONFIGURE_FILE() is called.
IF( IS_ABSOLUTE ${DATAPATH} )
   SET( CONFIGDATADIR "${DATAPATH}/" )
ELSE()
   SET( CONFIGDATADIR "/${DATAPATH}/" )
ENDIF()

IF( IS_ABSOLUTE ${DOCPATH} )
   SET( CONFIGDOCDIR "${DOCPATH}/" )
ELSE()
   SET( CONFIGDOCDIR "/${DOCPATH}/" )
ENDIF()

# Tell cmake where the configure file is and where
# to put the output. All variables in config.in written as "${VAR}$
# will be replaced by VAR as determined by cmake in config.h.
# Outputs only in the build directory.
CONFIGURE_FILE( config.in config.h )

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
   # Need to set some linker flags that I don't know how to get
   # automatically.
   ############################################################################

   # MinGW-specific flags.
   #    '-Wl,-subsystem,windows' supresses the output command window.
   #    '-Wl,-s' strips the executable of symbols.
   SET_TARGET_PROPERTIES( ${brewtarget_EXECUTABLE} PROPERTIES LINK_FLAGS "-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc -mthreads -Wl,-subsystem,windows")
ENDIF()

# Link brewtarget against Qt libraries defined by QT_LIBRARIES
TARGET_LINK_LIBRARIES( ${brewtarget_EXECUTABLE} ${QT_LIBRARIES} ${EXT_LIBRARIES} )
MESSAGE( STATUS "Libraries: ${QT_LIBRARIES} ${EXT_LIBRARIES}" )

#=================================Installs=====================================

# Install executable.
INSTALL( TARGETS ${brewtarget_EXECUTABLE}
         RUNTIME DESTINATION ${TARGETPATH} )

# Install the translations.
INSTALL(FILES ${QM_FILES}
        DESTINATION "${DATAPATH}/translations_qm")

IF( APPLE )
   # This SHOULD run macdeployqt on the app bundle at install time.
   # Don't know if it works. Could someone try?
   # May have to add a WORKING_DIRECTORY.
   INSTALL( CODE
            "EXECUTE_PROCESS( COMMAND ${QT_BINARY_DIR}/macdeployqt ${BT_BUNDLE_DIR} )" )
ENDIF()
