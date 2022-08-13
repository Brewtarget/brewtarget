#
# QtDesignerPlugins/CMakeLists.txt is part of Brewtarget, and is Copyright the following
# authors 2009-2022
#  - Matt Young <mfsy@yahoo.com>
#  - Philip Greggory Lee <rocketman768@gmail.com>
#
# Brewtarget is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Brewtarget is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# See also ../CMakeLists.txt
#
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

INCLUDE( ${QT_USE_FILE} )
INCLUDE_DIRECTORIES(
   ${QT_INCLUDE_DIR}
   ${repoDir}/src
)

LINK_DIRECTORIES(
   ${QT_LIBRARY_DIR}
)

# By default only QtCore and QtGui are enabled
SET( QT_USE_QTDESIGNER TRUE )

SET( SRC_FILES
   ${repoDir}/src/IbuGuSlider.cpp
   ${repoDir}/src/RangedSlider.cpp
   ${repoDir}/src/StyleRangeWidget.cpp
)

SET( MOC_FILES
   ${repoDir}/src/IbuGuSlider.h
   ${repoDir}/src/RangedSlider.h
   ${repoDir}/src/StyleRangeWidget.h
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
