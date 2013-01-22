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
    ${SRCDIR}/StyleRangeWidget.cpp
)

SET( MOC_FILES
    ${SRCDIR}/StyleRangeWidget.h
)

set( UI_FILES
)

SET( PLUGIN_MOCS
   StyleRangeWidgetPlugin.h
)

SET( PLUGIN_SRCS
   StyleRangeWidgetPlugin.cpp
)

QT4_WRAP_CPP( GEN_MOC_FILES ${MOC_FILES} ${PLUGIN_MOCS} )
QT4_WRAP_UI( GEN_UI_FILES ${UI_FILES})
#QT4_ADD_RESOURCES( GEN_RCC_FILES "${CMAKE_CURRENT_BINARY_DIR}/../brewtarget.qrc")

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
