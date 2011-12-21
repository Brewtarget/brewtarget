add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_SHARED)

INCLUDE( ${QT_USE_FILE} )
INCLUDE_DIRECTORIES(
   ${QT_INCLUDE_DIR}
   ${SRCDIR}
)

LINK_DIRECTORIES(
   ${QT_LIBRARY_DIR}
)

# By default only QtCore and QtGui are enabled
set( QT_USE_QTDESIGNER TRUE )

set( SRC_FILES 
   ${SRCDIR}/Algorithms.cpp
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
    ${SRCDIR}/database.cpp
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
    ${SRCDIR}/OgAdjuster.cpp
    ${SRCDIR}/OptionDialog.cpp
    ${SRCDIR}/PreInstruction.cpp
    ${SRCDIR}/PrimingDialog.cpp
    ${SRCDIR}/recipe.cpp
    ${SRCDIR}/RecipeComboBox.cpp
    ${SRCDIR}/RecipeFormatter.cpp
    ${SRCDIR}/RecipeStyleNameButton.cpp
    ${SRCDIR}/RefractoDialog.cpp
    ${SRCDIR}/SetterCommand.cpp
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

set( MOC_FILES
    ${SRCDIR}/BeerColorWidget.h
    ${SRCDIR}/BeerXMLElement.h
    ${SRCDIR}/BrewDayWidget.h
    ${SRCDIR}/BrewDayScrollWidget.h
    ${SRCDIR}/brewnote.h
    ${SRCDIR}/BrewNoteWidget.h
    ${SRCDIR}/BrewTargetTreeModel.h
    ${SRCDIR}/BrewTargetTreeView.h
    ${SRCDIR}/BtDigitWidget.h
    ${SRCDIR}/BtSplashScreen.h
    ${SRCDIR}/ConverterTool.h
    ${SRCDIR}/database.h
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
    ${SRCDIR}/RecipeStyleNameButton.h
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
    ${SRCDIR}/equipment.h
    ${SRCDIR}/fermentable.h
    ${SRCDIR}/hop.h
    ${SRCDIR}/instruction.h
    ${SRCDIR}/mash.h
    ${SRCDIR}/mashstep.h
    ${SRCDIR}/misc.h
    ${SRCDIR}/recipe.h
    ${SRCDIR}/style.h
    ${SRCDIR}/water.h
    ${SRCDIR}/yeast.h
)

set( UI_FILES
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

SET( PLUGIN_MOCS
   BrewtargetPlugins.h
   BeerColorWidgetPlugin.h
   BrewDayScrollWidgetPlugin.h
   RecipeExtrasWidgetPlugin.h
)

SET( PLUGIN_SRCS
   BrewtargetPlugins.cpp
   BeerColorWidgetPlugin.cpp
   BrewDayScrollWidgetPlugin.cpp
   RecipeExtrasWidgetPlugin.cpp
)

QT4_WRAP_CPP( GEN_MOC_FILES ${MOC_FILES} ${PLUGIN_MOCS} )
QT4_WRAP_UI( GEN_UI_FILES ${UI_FILES})
QT4_ADD_RESOURCES( GEN_RCC_FILES "../brewtarget.qrc")
#QT4_WRAP_CPP( GEN_MOC_UI_FILES ${GEN_UI_FILES})

#========================All Plugins Together==================================

add_library( brewtargetplugins SHARED 
        ${SRC_FILES}
        ${PLUGIN_SRCS}
        ${GEN_RCC_FILES}
        ${GEN_UI_FILES}
        ${GEN_MOC_FILES}
)

TARGET_LINK_LIBRARIES( brewtargetplugins
   ${QT_LIBRARIES}
)

INSTALL( TARGETS brewtargetplugins DESTINATION ${QT_PLUGINS_DIR}/designer )
