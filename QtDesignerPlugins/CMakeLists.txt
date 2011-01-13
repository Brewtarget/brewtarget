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
   #${SRCDIR}/Algorithms.cpp
   #${SRCDIR}/BeerColorWidget.cpp
   #${SRCDIR}/BeerXMLElement.cpp
   ${SRCDIR}/BrewDayScrollWidget.cpp
   #${SRCDIR}/BrewDayWidget.cpp
   ${SRCDIR}/brewtarget.cpp
   #${SRCDIR}/BtDigitWidget.cpp
   ${SRCDIR}/CelsiusTempUnitSystem.cpp
   #${SRCDIR}/ColorMethods.cpp
   #${SRCDIR}/ConverterTool.cpp
   #${SRCDIR}/database.cpp
   #${SRCDIR}/EquipmentComboBox.cpp
   #${SRCDIR}/equipment.cpp
   #${SRCDIR}/EquipmentEditor.cpp
   ${SRCDIR}/FahrenheitTempUnitSystem.cpp
   #${SRCDIR}/fermentable.cpp
   #${SRCDIR}/FermentableDialog.cpp
   #${SRCDIR}/FermentableEditor.cpp
   #${SRCDIR}/FermentableSortFilterProxyModel.cpp
   #${SRCDIR}/FermentableTableModel.cpp
   #${SRCDIR}/FermentableTableWidget.cpp
   #${SRCDIR}/HeatCalculations.cpp
   #${SRCDIR}/hop.cpp
   #${SRCDIR}/HopDialog.cpp
   #${SRCDIR}/HopEditor.cpp
   #${SRCDIR}/HopSortFilterProxyModel.cpp
   #${SRCDIR}/HopTableModel.cpp
   #${SRCDIR}/HopTableWidget.cpp
   #${SRCDIR}/HtmlViewer.cpp
   #${SRCDIR}/IbuMethods.cpp
   ${SRCDIR}/ImperialVolumeUnitSystem.cpp
   #${SRCDIR}/instruction.cpp
   ${SRCDIR}/InstructionWidget.cpp # BrewDayScrollWidget
   #${SRCDIR}/MainWindow.cpp
   #${SRCDIR}/MaltinessWidget.cpp
   #${SRCDIR}/MashComboBox.cpp
   #${SRCDIR}/mash.cpp
   #${SRCDIR}/MashDesigner.cpp
   #${SRCDIR}/MashEditor.cpp
   #${SRCDIR}/mashstep.cpp
   #${SRCDIR}/MashStepEditor.cpp
   #${SRCDIR}/MashStepTableModel.cpp
   #${SRCDIR}/MashStepTableWidget.cpp
   #${SRCDIR}/MashWizard.cpp
   #${SRCDIR}/matrix.cpp
   #${SRCDIR}/misc.cpp
   #${SRCDIR}/MiscDialog.cpp
   #${SRCDIR}/MiscEditor.cpp
   #${SRCDIR}/MiscTableModel.cpp
   #${SRCDIR}/MiscTableWidget.cpp
   ${SRCDIR}/observable.cpp
   ${SRCDIR}/observer.cpp
   #${SRCDIR}/OgAdjuster.cpp
   #${SRCDIR}/OptionDialog.cpp
   #${SRCDIR}/PitchDialog.cpp
   #${SRCDIR}/PreInstruction.cpp
   #${SRCDIR}/PrimingDialog.cpp
   #${SRCDIR}/RecipeComboBox.cpp
   #${SRCDIR}/recipe.cpp
   #${SRCDIR}/RecipeExtrasDialog.cpp
   ${SRCDIR}/RecipeExtrasWidget.cpp
   #${SRCDIR}/RecipeFormatter.cpp
   #${SRCDIR}/RefractoDialog.cpp
   #${SRCDIR}/ScaleRecipeTool.cpp
   ${SRCDIR}/SIVolumeUnitSystem.cpp
   ${SRCDIR}/SIWeightUnitSystem.cpp
   #${SRCDIR}/StyleComboBox.cpp
   #${SRCDIR}/style.cpp
   #${SRCDIR}/StyleEditor.cpp
   #${SRCDIR}/TimerListDialog.cpp
   ${SRCDIR}/TimerWidget.cpp # BrewDayScrollWidget
   ${SRCDIR}/TimeUnitSystem.cpp
   ${SRCDIR}/unit.cpp
   ${SRCDIR}/UnitSystem.cpp
   ${SRCDIR}/USVolumeUnitSystem.cpp
   ${SRCDIR}/USWeightUnitSystem.cpp
   #${SRCDIR}/water.cpp
   #${SRCDIR}/WaterEditor.cpp
   #${SRCDIR}/WaterTableModel.cpp
   #${SRCDIR}/WaterTableWidget.cpp
   #${SRCDIR}/yeast.cpp
   #${SRCDIR}/YeastDialog.cpp
   #${SRCDIR}/YeastEditor.cpp
   #${SRCDIR}/YeastSortFilterProxyModel.cpp
   #${SRCDIR}/YeastTableModel.cpp
   #${SRCDIR}/YeastTableWidget.cpp
)

set( MOC_FILES
    #${SRCDIR}/BeerColorWidget.h
    #${SRCDIR}/BrewDayWidget.h
    ${SRCDIR}/BrewDayScrollWidget.h
    #${SRCDIR}/BtDigitWidget.h
    #${SRCDIR}/ConverterTool.h
    #${SRCDIR}/EquipmentComboBox.h
    #${SRCDIR}/EquipmentEditor.h
    #${SRCDIR}/FermentableEditor.h
    #${SRCDIR}/FermentableDialog.h
    #${SRCDIR}/FermentableSortFilterProxyModel.h
    #${SRCDIR}/FermentableTableModel.h
    #${SRCDIR}/FermentableTableWidget.h
    #${SRCDIR}/HopDialog.h
    #${SRCDIR}/HopEditor.h
    #${SRCDIR}/HopSortFilterProxyModel.h
    #${SRCDIR}/HopTableModel.h
    #${SRCDIR}/HopTableWidget.h
    ${SRCDIR}/InstructionWidget.h # BrewDayScrollWidget
    #${SRCDIR}/MainWindow.h
    #${SRCDIR}/MaltinessWidget.h
    #${SRCDIR}/MashComboBox.h
    #${SRCDIR}/MashDesigner.h
    #${SRCDIR}/MashEditor.h
    #${SRCDIR}/MashStepEditor.h
    #${SRCDIR}/MashStepTableModel.h
    #${SRCDIR}/MashStepTableWidget.h
    #${SRCDIR}/MashWizard.h
    #${SRCDIR}/MiscDialog.h
    #${SRCDIR}/MiscEditor.h
    #${SRCDIR}/MiscTableModel.h
    #${SRCDIR}/MiscTableWidget.h
    #${SRCDIR}/OgAdjuster.h
    #${SRCDIR}/OptionDialog.h
    #${SRCDIR}/PitchDialog.h
    #${SRCDIR}/PrimingDialog.h
    #${SRCDIR}/RecipeComboBox.h
    #${SRCDIR}/RecipeExtrasDialog.h
    ${SRCDIR}/RecipeExtrasWidget.h
    #${SRCDIR}/RecipeFormatter.h
    #${SRCDIR}/RefractoDialog.h
    #${SRCDIR}/ScaleRecipeTool.h
    #${SRCDIR}/StyleComboBox.h
    #${SRCDIR}/StyleEditor.h
    #${SRCDIR}/TimerListDialog.h
    ${SRCDIR}/TimerWidget.h # BrewDayScrollWidget
    #${SRCDIR}/WaterTableModel.h
    #${SRCDIR}/WaterTableWidget.h
    #${SRCDIR}/WaterEditor.h
    #${SRCDIR}/YeastDialog.h
    #${SRCDIR}/YeastEditor.h
    #${SRCDIR}/YeastSortFilterProxyModel.h
    #${SRCDIR}/YeastTableModel.h
    #${SRCDIR}/YeastTableWidget.h
)

set( UI_FILES
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

SET( PLUGIN_MOCS
   BrewtargetPlugins.h
   BrewDayScrollWidgetPlugin.h
   RecipeExtrasWidgetPlugin.h
)

SET( PLUGIN_SRCS
   BrewtargetPlugins.cpp
   BrewDayScrollWidgetPlugin.cpp
   RecipeExtrasWidgetPlugin.cpp
)

QT4_WRAP_CPP( GEN_MOC_FILES ${MOC_FILES} ${PLUGIN_MOCS} )
QT4_WRAP_UI( GEN_UI_FILES ${UI_FILES})
#QT4_ADD_RESOURCES( GEN_RCC_FILES "../brewtarget.qrc")
#QT4_WRAP_CPP( GEN_MOC_UI_FILES ${GEN_UI_FILES})

#========================All Plugins Together==================================

add_library( brewtargetplugins SHARED 
        ${SRC_FILES}
        ${PLUGIN_SRCS}
        ${GEN_UI_FILES}
        ${GEN_MOC_FILES}
)

TARGET_LINK_LIBRARIES( brewtargetplugins
   ${QT_LIBRARIES}
)

INSTALL( TARGETS brewtargetplugins DESTINATION ${QT_PLUGINS_DIR}/designer )

