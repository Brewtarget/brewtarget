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
   ${SRCDIR}/IbuGuSlider.cpp
   ${SRCDIR}/RangedSlider.cpp
   ${SRCDIR}/StyleRangeWidget.cpp
)

SET( MOC_FILES
   ${SRCDIR}/IbuGuSlider.h
   ${SRCDIR}/RangedSlider.h
   ${SRCDIR}/StyleRangeWidget.h
)

set( UI_FILES
)

SET( PLUGIN_MOCS
   BrewtargetPlugins.h
   IbuGuSliderPlugin.h
   StyleRangeWidgetPlugin.h
)

SET( PLUGIN_SRCS
   BrewtargetPlugins.cpp
   IbuGuSliderPlugin.cpp
   StyleRangeWidgetPlugin.cpp
)

QT4_WRAP_CPP( GEN_MOC_FILES ${MOC_FILES} ${PLUGIN_MOCS} )
QT4_WRAP_UI( GEN_UI_FILES ${UI_FILES})

#========================All Plugins Together==================================

ADD_LIBRARY( brewtargetplugins SHARED
   ${SRC_FILES} 
   ${PLUGIN_SRCS}
   ${GEN_UI_FILES}
   ${GEN_MOC_FILES}
)

TARGET_LINK_LIBRARIES( brewtargetplugins
   ${QT_LIBRARIES}
)

INSTALL( TARGETS brewtargetplugins DESTINATION ${QT_PLUGINS_DIR}/designer )
