#
# src/CMakeLists.txt is part of Brewtarget, and is Copyright the following
# authors 2009-2023:
#  - Aidan Roberts <aidanr67@gmail.com>
#  - Daniel Moreno <danielm5@users.noreply.github.com>
#  - Mark de Wever <koraq@xs4all.nl>
#  - Mattias Måhl <mattias@kejsarsten.com>
#  - Matt Young<mfsy@yahoo.com>
#  - Maxime Lavigne <duguigne@gmail.com>
#  - Mik Firestone <mikfire@gmail.com>
#  - Philip Greggory Lee <rocketman768@gmail.com>
#  - Ryan Hoobler <rhoob@yahoo.com>
#  - Théophane Martin <theophane.m@gmail.com>
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
# See also ../CMakeLists.txt, which is the master CMake file from which this one gets included
#

#
# Variable that contains all the .cpp files in this project.
#
# You can recreate the body of this list by running the following from the bash prompt in the build directory:
#    find ../src -name '*.cpp' | sort  | sed 's+^../src+    ${repoDir}/src+' | grep -v Testing.cpp | grep -v main.cpp
#
# It is tempting to ask ourselves why we don't have CMake figure out this list itself.  Well, according to the
# documentation (https://cmake.org/cmake/help/latest/command/file.html#filesystem) "We do not recommend using GLOB to
# collect a list of source files from your source tree. If no CMakeLists.txt file changes when a source is added or
# removed then the generated build system cannot know when to ask CMake to regenerate."
#
set(filesToCompile_cpp
    ${repoDir}/src/AboutDialog.cpp
    ${repoDir}/src/AlcoholTool.cpp
    ${repoDir}/src/Algorithms.cpp
    ${repoDir}/src/AncestorDialog.cpp
    ${repoDir}/src/Application.cpp
    ${repoDir}/src/BeerColorWidget.cpp
    ${repoDir}/src/boiltime.cpp
    ${repoDir}/src/BrewDayScrollWidget.cpp
    ${repoDir}/src/BrewDayFormatter.cpp
    ${repoDir}/src/BrewNoteWidget.cpp
    ${repoDir}/src/BtColor.cpp
    ${repoDir}/src/BtDatePopup.cpp
    ${repoDir}/src/BtDigitWidget.cpp
    ${repoDir}/src/BtFieldType.cpp
    ${repoDir}/src/BtFolder.cpp
    ${repoDir}/src/BtLabel.cpp
    ${repoDir}/src/BtLineEdit.cpp
    ${repoDir}/src/BtSplashScreen.cpp
    ${repoDir}/src/BtTabWidget.cpp
    ${repoDir}/src/BtTextEdit.cpp
    ${repoDir}/src/BtTreeFilterProxyModel.cpp
    ${repoDir}/src/BtTreeItem.cpp
    ${repoDir}/src/BtTreeModel.cpp
    ${repoDir}/src/BtTreeView.cpp
    ${repoDir}/src/ConverterTool.cpp
    ${repoDir}/src/CustomComboBox.cpp
    ${repoDir}/src/database/BtSqlQuery.cpp
    ${repoDir}/src/database/Database.cpp
    ${repoDir}/src/database/DatabaseSchemaHelper.cpp
    ${repoDir}/src/database/DbTransaction.cpp
    ${repoDir}/src/database/ObjectStore.cpp
    ${repoDir}/src/database/ObjectStoreTyped.cpp
    ${repoDir}/src/EquipmentButton.cpp
    ${repoDir}/src/EquipmentEditor.cpp
    ${repoDir}/src/EquipmentListModel.cpp
    ${repoDir}/src/FermentableDialog.cpp
    ${repoDir}/src/FermentableEditor.cpp
    ${repoDir}/src/FermentableSortFilterProxyModel.cpp
    ${repoDir}/src/HeatCalculations.cpp
    ${repoDir}/src/HopDialog.cpp
    ${repoDir}/src/HopEditor.cpp
    ${repoDir}/src/HopSortFilterProxyModel.cpp
    ${repoDir}/src/Html.cpp
    ${repoDir}/src/HydrometerTool.cpp
    ${repoDir}/src/IbuGuSlider.cpp
    ${repoDir}/src/InstructionWidget.cpp
    ${repoDir}/src/InventoryFormatter.cpp
    ${repoDir}/src/Localization.cpp
    ${repoDir}/src/Logging.cpp
    ${repoDir}/src/MainWindow.cpp
    ${repoDir}/src/MashButton.cpp
    ${repoDir}/src/MashComboBox.cpp
    ${repoDir}/src/MashDesigner.cpp
    ${repoDir}/src/MashEditor.cpp
    ${repoDir}/src/MashListModel.cpp
    ${repoDir}/src/MashStepEditor.cpp
    ${repoDir}/src/MashStepTableWidget.cpp
    ${repoDir}/src/MashWizard.cpp
    ${repoDir}/src/matrix.cpp
    ${repoDir}/src/measurement/Amount.cpp
    ${repoDir}/src/measurement/ColorMethods.cpp
    ${repoDir}/src/measurement/IbuMethods.cpp
    ${repoDir}/src/measurement/Measurement.cpp
    ${repoDir}/src/measurement/PhysicalQuantity.cpp
    ${repoDir}/src/measurement/SucroseConversion.cpp
    ${repoDir}/src/measurement/SystemOfMeasurement.cpp
    ${repoDir}/src/measurement/Unit.cpp
    ${repoDir}/src/measurement/UnitSystem.cpp
    ${repoDir}/src/MiscDialog.cpp
    ${repoDir}/src/MiscEditor.cpp
    ${repoDir}/src/MiscSortFilterProxyModel.cpp
    ${repoDir}/src/model/BrewNote.cpp
    ${repoDir}/src/model/Equipment.cpp
    ${repoDir}/src/model/Fermentable.cpp
    ${repoDir}/src/model/Hop.cpp
    ${repoDir}/src/model/Instruction.cpp
    ${repoDir}/src/model/Inventory.cpp
    ${repoDir}/src/model/Mash.cpp
    ${repoDir}/src/model/MashStep.cpp
    ${repoDir}/src/model/Misc.cpp
    ${repoDir}/src/model/NamedEntity.cpp
    ${repoDir}/src/model/NamedEntityWithInventory.cpp
    ${repoDir}/src/model/NamedParameterBundle.cpp
    ${repoDir}/src/model/Recipe.cpp
    ${repoDir}/src/model/Salt.cpp
    ${repoDir}/src/model/Style.cpp
    ${repoDir}/src/model/Water.cpp
    ${repoDir}/src/model/Yeast.cpp
    ${repoDir}/src/NamedEntitySortProxyModel.cpp
    ${repoDir}/src/NamedMashEditor.cpp
    ${repoDir}/src/OgAdjuster.cpp
    ${repoDir}/src/OptionDialog.cpp
    ${repoDir}/src/PersistentSettings.cpp
    ${repoDir}/src/PitchDialog.cpp
    ${repoDir}/src/PreInstruction.cpp
    ${repoDir}/src/PrimingDialog.cpp
    ${repoDir}/src/PrintAndPreviewDialog.cpp
    ${repoDir}/src/RadarChart.cpp
    ${repoDir}/src/RangedSlider.cpp
    ${repoDir}/src/RecipeExtrasWidget.cpp
    ${repoDir}/src/RecipeFormatter.cpp
    ${repoDir}/src/RefractoDialog.cpp
    ${repoDir}/src/ScaleRecipeTool.cpp
    ${repoDir}/src/SimpleUndoableUpdate.cpp
    ${repoDir}/src/StrikeWaterDialog.cpp
    ${repoDir}/src/StyleButton.cpp
    ${repoDir}/src/StyleEditor.cpp
    ${repoDir}/src/StyleListModel.cpp
    ${repoDir}/src/StyleRangeWidget.cpp
    ${repoDir}/src/StyleSortFilterProxyModel.cpp
    ${repoDir}/src/tableModels/BtTableModel.cpp
    ${repoDir}/src/tableModels/BtTableModelInventory.cpp
    ${repoDir}/src/tableModels/FermentableTableModel.cpp
    ${repoDir}/src/tableModels/HopTableModel.cpp
    ${repoDir}/src/tableModels/MashStepTableModel.cpp
    ${repoDir}/src/tableModels/MiscTableModel.cpp
    ${repoDir}/src/tableModels/SaltTableModel.cpp
    ${repoDir}/src/tableModels/WaterTableModel.cpp
    ${repoDir}/src/tableModels/YeastTableModel.cpp
    ${repoDir}/src/TimerListDialog.cpp
    ${repoDir}/src/TimerMainDialog.cpp
    ${repoDir}/src/TimerWidget.cpp
    ${repoDir}/src/UiAmountWithUnits.cpp
    ${repoDir}/src/utils/BtException.cpp
    ${repoDir}/src/utils/BtStringConst.cpp
    ${repoDir}/src/utils/BtStringStream.cpp
    ${repoDir}/src/utils/EnumStringMapping.cpp
    ${repoDir}/src/utils/ImportRecordCount.cpp
    ${repoDir}/src/utils/TimerUtils.cpp
    ${repoDir}/src/WaterButton.cpp
    ${repoDir}/src/WaterDialog.cpp
    ${repoDir}/src/WaterEditor.cpp
    ${repoDir}/src/WaterListModel.cpp
    ${repoDir}/src/WaterSortFilterProxyModel.cpp
    ${repoDir}/src/WaterTableWidget.cpp
    ${repoDir}/src/widgets/Animator.cpp
    ${repoDir}/src/widgets/SelectionControl.cpp
    ${repoDir}/src/widgets/ToggleSwitch.cpp
    ${repoDir}/src/widgets/UnitAndScalePopUpMenu.cpp
    ${repoDir}/src/xml/BeerXml.cpp
    ${repoDir}/src/xml/BtDomErrorHandler.cpp
    ${repoDir}/src/xml/XercesHelpers.cpp
    ${repoDir}/src/xml/XmlCoding.cpp
    ${repoDir}/src/xml/XmlMashRecord.cpp
    ${repoDir}/src/xml/XmlMashStepRecord.cpp
    ${repoDir}/src/xml/XmlRecipeRecord.cpp
    ${repoDir}/src/xml/XmlRecord.cpp
    ${repoDir}/src/YeastDialog.cpp
    ${repoDir}/src/YeastEditor.cpp
    ${repoDir}/src/YeastSortFilterProxyModel.cpp
)
