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

#
# NB: This file is included from ../CMakeLists.txt
#
# Here, we just list all the .cpp files needed to compile our Qt Designer plugins
#
set(filesToCompile_pluginsCpp
   ${repoDir}/QtDesignerPlugins/BrewtargetPlugins.cpp
   ${repoDir}/QtDesignerPlugins/IbuGuSliderPlugin.cpp
   ${repoDir}/QtDesignerPlugins/StyleRangeWidgetPlugin.cpp
   ${repoDir}/src/IbuGuSlider.cpp
   ${repoDir}/src/RangedSlider.cpp
   ${repoDir}/src/StyleRangeWidget.cpp
)
