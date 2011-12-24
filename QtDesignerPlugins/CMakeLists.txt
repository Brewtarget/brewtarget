ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

INCLUDE( ${QT_USE_FILE} )
INCLUDE_DIRECTORIES(
   ${QT_INCLUDE_DIR}
   ${SRCDIR}
)

LINK_DIRECTORIES(
   ${QT_LIBRARY_DIR}
)

# By default only QtCore and QtGui are enabled
SET( QT_USE_QTDESIGNER TRUE )

SET( SRC_FILES
    ${SRCDIR}/BeerColorWidget.cpp
    ${SRCDIR}/BrewDayScrollWidget.cpp
    ${SRCDIR}/CustomComboBox.cpp
    ${SRCDIR}/MaltinessWidget.cpp
    ${SRCDIR}/RecipeExtrasWidget.cpp

    ${SRCDIR}/BeerXMLElement.cpp
    ${SRCDIR}/brewnote.cpp
    ${SRCDIR}/brewtarget.cpp
    ${SRCDIR}/CelsiusTempUnitSystem.cpp
    ${SRCDIR}/database.cpp
    ${SRCDIR}/equipment.cpp
    ${SRCDIR}/FahrenheitTempUnitSystem.cpp
    ${SRCDIR}/fermentable.cpp
    ${SRCDIR}/HeatCalculations.cpp
    ${SRCDIR}/ImperialVolumeUnitSystem.cpp
    ${SRCDIR}/hop.cpp
    ${SRCDIR}/instruction.cpp
    ${SRCDIR}/mash.cpp
    ${SRCDIR}/mashstep.cpp
    ${SRCDIR}/misc.cpp
    ${SRCDIR}/recipe.cpp
    ${SRCDIR}/SIVolumeUnitSystem.cpp
    ${SRCDIR}/SIWeightUnitSystem.cpp
    ${SRCDIR}/style.cpp
    ${SRCDIR}/TimeUnitSystem.cpp
    ${SRCDIR}/unit.cpp
    ${SRCDIR}/UnitSystem.cpp
    ${SRCDIR}/UnitSystems.cpp
    ${SRCDIR}/USVolumeUnitSystem.cpp
    ${SRCDIR}/USWeightUnitSystem.cpp
    ${SRCDIR}/water.cpp
    ${SRCDIR}/yeast.cpp
)

SET( MOC_FILES
    ${SRCDIR}/BeerColorWidget.h
    ${SRCDIR}/BrewDayScrollWidget.h
    ${SRCDIR}/CustomComboBox.h
    ${SRCDIR}/MaltinessWidget.h
    ${SRCDIR}/RecipeExtrasWidget.h

    ${SRCDIR}/BeerXMLElement.h
    ${SRCDIR}/brewnote.h
    ${SRCDIR}/database.h
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
    ${UIDIR}/brewDayScrollWidget.ui
    ${UIDIR}/btSplashScreen.ui
    ${UIDIR}/instructionWidget.ui
    ${UIDIR}/mainWindow.ui
    ${UIDIR}/recipeExtrasWidget.ui
    ${UIDIR}/timerWidget.ui
)

SET( PLUGIN_MOCS
   BrewtargetPlugins.h
   BeerColorWidgetPlugin.h
   BrewDayScrollWidgetPlugin.h
   CustomComboBoxPlugin.h
   MaltinessWidgetPlugin.h
   RecipeExtrasWidgetPlugin.h
)

SET( PLUGIN_SRCS
   BrewtargetPlugins.cpp
   BeerColorWidgetPlugin.cpp
   BrewDayScrollWidgetPlugin.cpp
   CustomComboBoxPlugin.cpp
   MaltinessWidgetPlugin.cpp
   RecipeExtrasWidgetPlugin.cpp
)

QT4_WRAP_CPP( GEN_MOC_FILES ${MOC_FILES} ${PLUGIN_MOCS} )
QT4_WRAP_UI( GEN_UI_FILES ${UI_FILES})
QT4_ADD_RESOURCES( GEN_RCC_FILES "../brewtarget.qrc")
#QT4_WRAP_CPP( GEN_MOC_UI_FILES ${GEN_UI_FILES})

#========================All Plugins Together==================================

ADD_LIBRARY( brewtargetplugins SHARED
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
