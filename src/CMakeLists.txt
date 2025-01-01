#----------------------------------------------------------------------------------------------------------------------
# src/CMakeLists.txt is part of Brewtarget, and is copyright the following authors 2009-2024:
#   • Aidan Roberts <aidanr67@gmail.com>
#   • Daniel Moreno <danielm5@users.noreply.github.com>
#   • Mark de Wever <koraq@xs4all.nl>
#   • Mattias Måhl <mattias@kejsarsten.com>
#   • Matt Young<mfsy@yahoo.com>
#   • Maxime Lavigne <duguigne@gmail.com>
#   • Mik Firestone <mikfire@gmail.com>
#   • Philip Greggory Lee <rocketman768@gmail.com>
#   • Ryan Hoobler <rhoob@yahoo.com>
#   • Théophane Martin <theophane.m@gmail.com>
#
# Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#----------------------------------------------------------------------------------------------------------------------
#
# See also ../CMakeLists.txt
#

#
# Variable that contains all the .cpp files in this project.
#
# You can recreate the body of this list by running the following from the bash prompt in the build directory:
#    find ../src -name '*.cpp' | LC_ALL=C sort | sed 's+^../src+    ${repoDir}/src+' | grep -v Testing.cpp | grep -v main.cpp
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
    ${repoDir}/src/BrewDayFormatter.cpp
    ${repoDir}/src/BrewDayScrollWidget.cpp
    ${repoDir}/src/BrewNoteWidget.cpp
    ${repoDir}/src/BtColor.cpp
    ${repoDir}/src/BtDatePopup.cpp
    ${repoDir}/src/BtFieldType.cpp
    ${repoDir}/src/BtHorizontalTabs.cpp
    ${repoDir}/src/BtSplashScreen.cpp
    ${repoDir}/src/BtTabWidget.cpp
    ${repoDir}/src/BtTextEdit.cpp
    ${repoDir}/src/ConverterTool.cpp
    ${repoDir}/src/HeatCalculations.cpp
    ${repoDir}/src/HelpDialog.cpp
    ${repoDir}/src/Html.cpp
    ${repoDir}/src/HydrometerTool.cpp
    ${repoDir}/src/IbuGuSlider.cpp
    ${repoDir}/src/InventoryFormatter.cpp
    ${repoDir}/src/LatestReleaseFinder.cpp
    ${repoDir}/src/Localization.cpp
    ${repoDir}/src/Logging.cpp
    ${repoDir}/src/MainWindow.cpp
    ${repoDir}/src/MashDesigner.cpp
    ${repoDir}/src/MashWizard.cpp
    ${repoDir}/src/NamedEntitySortProxyModel.cpp
    ${repoDir}/src/OgAdjuster.cpp
    ${repoDir}/src/OptionDialog.cpp
    ${repoDir}/src/PersistentSettings.cpp
    ${repoDir}/src/PitchDialog.cpp
    ${repoDir}/src/PrimingDialog.cpp
    ${repoDir}/src/PrintAndPreviewDialog.cpp
    ${repoDir}/src/RadarChart.cpp
    ${repoDir}/src/RangedSlider.cpp
    ${repoDir}/src/RecipeExtrasWidget.cpp
    ${repoDir}/src/RecipeFormatter.cpp
    ${repoDir}/src/RefractoDialog.cpp
    ${repoDir}/src/ScaleRecipeTool.cpp
    ${repoDir}/src/StrikeWaterDialog.cpp
    ${repoDir}/src/StyleRangeWidget.cpp
    ${repoDir}/src/TimerListDialog.cpp
    ${repoDir}/src/TimerMainDialog.cpp
    ${repoDir}/src/TimerWidget.cpp
    ${repoDir}/src/WaterDialog.cpp
    ${repoDir}/src/boiltime.cpp
    ${repoDir}/src/buttons/BoilButton.cpp
    ${repoDir}/src/buttons/EquipmentButton.cpp
    ${repoDir}/src/buttons/FermentationButton.cpp
    ${repoDir}/src/buttons/MashButton.cpp
    ${repoDir}/src/buttons/RecipeAttributeButton.cpp
    ${repoDir}/src/buttons/StyleButton.cpp
    ${repoDir}/src/buttons/WaterButton.cpp
    ${repoDir}/src/catalogs/EquipmentCatalog.cpp
    ${repoDir}/src/catalogs/FermentableCatalog.cpp
    ${repoDir}/src/catalogs/HopCatalog.cpp
    ${repoDir}/src/catalogs/MiscCatalog.cpp
    ${repoDir}/src/catalogs/StyleCatalog.cpp
    ${repoDir}/src/catalogs/YeastCatalog.cpp
    ${repoDir}/src/database/BtSqlQuery.cpp
    ${repoDir}/src/database/Database.cpp
    ${repoDir}/src/database/DatabaseSchemaHelper.cpp
    ${repoDir}/src/database/DbTransaction.cpp
    ${repoDir}/src/database/DefaultContentLoader.cpp
    ${repoDir}/src/database/ObjectStore.cpp
    ${repoDir}/src/database/ObjectStoreTyped.cpp
    ${repoDir}/src/editors/BoilEditor.cpp
    ${repoDir}/src/editors/BoilStepEditor.cpp
    ${repoDir}/src/editors/EquipmentEditor.cpp
    ${repoDir}/src/editors/FermentableEditor.cpp
    ${repoDir}/src/editors/FermentationEditor.cpp
    ${repoDir}/src/editors/FermentationStepEditor.cpp
    ${repoDir}/src/editors/HopEditor.cpp
    ${repoDir}/src/editors/MashEditor.cpp
    ${repoDir}/src/editors/MashStepEditor.cpp
    ${repoDir}/src/editors/MiscEditor.cpp
    ${repoDir}/src/editors/NamedMashEditor.cpp
    ${repoDir}/src/editors/StyleEditor.cpp
    ${repoDir}/src/editors/WaterEditor.cpp
    ${repoDir}/src/editors/YeastEditor.cpp
    ${repoDir}/src/measurement/Amount.cpp
    ${repoDir}/src/measurement/ColorMethods.cpp
    ${repoDir}/src/measurement/IbuMethods.cpp
    ${repoDir}/src/measurement/Measurement.cpp
    ${repoDir}/src/measurement/PhysicalQuantity.cpp
    ${repoDir}/src/measurement/SucroseConversion.cpp
    ${repoDir}/src/measurement/SystemOfMeasurement.cpp
    ${repoDir}/src/measurement/Unit.cpp
    ${repoDir}/src/measurement/UnitSystem.cpp
    ${repoDir}/src/model/Boil.cpp
    ${repoDir}/src/model/BoilStep.cpp
    ${repoDir}/src/model/BrewNote.cpp
    ${repoDir}/src/model/Equipment.cpp
    ${repoDir}/src/model/Fermentable.cpp
    ${repoDir}/src/model/Fermentation.cpp
    ${repoDir}/src/model/FermentationStep.cpp
    ${repoDir}/src/model/Folder.cpp
    ${repoDir}/src/model/Hop.cpp
    ${repoDir}/src/model/Ingredient.cpp
    ${repoDir}/src/model/IngredientInRecipe.cpp
    ${repoDir}/src/model/Instruction.cpp
    ${repoDir}/src/model/Inventory.cpp
    ${repoDir}/src/model/Mash.cpp
    ${repoDir}/src/model/MashStep.cpp
    ${repoDir}/src/model/Misc.cpp
    ${repoDir}/src/model/NamedEntity.cpp
    ${repoDir}/src/model/NamedParameterBundle.cpp
    ${repoDir}/src/model/OutlineableNamedEntity.cpp
    ${repoDir}/src/model/OwnedByRecipe.cpp
    ${repoDir}/src/model/Recipe.cpp
    ${repoDir}/src/model/RecipeAddition.cpp
    ${repoDir}/src/model/RecipeAdditionFermentable.cpp
    ${repoDir}/src/model/RecipeAdditionHop.cpp
    ${repoDir}/src/model/RecipeAdditionMisc.cpp
    ${repoDir}/src/model/RecipeAdditionYeast.cpp
    ${repoDir}/src/model/RecipeAdjustmentSalt.cpp
    ${repoDir}/src/model/RecipeUseOfWater.cpp
    ${repoDir}/src/model/Salt.cpp
    ${repoDir}/src/model/Step.cpp
    ${repoDir}/src/model/StepExtended.cpp
    ${repoDir}/src/model/Style.cpp
    ${repoDir}/src/model/Water.cpp
    ${repoDir}/src/model/Yeast.cpp
    ${repoDir}/src/qtModels/listModels/BoilListModel.cpp
    ${repoDir}/src/qtModels/listModels/EquipmentListModel.cpp
    ${repoDir}/src/qtModels/listModels/FermentableListModel.cpp
    ${repoDir}/src/qtModels/listModels/FermentationListModel.cpp
    ${repoDir}/src/qtModels/listModels/HopListModel.cpp
    ${repoDir}/src/qtModels/listModels/MashListModel.cpp
    ${repoDir}/src/qtModels/listModels/MashStepListModel.cpp
    ${repoDir}/src/qtModels/listModels/MiscListModel.cpp
    ${repoDir}/src/qtModels/listModels/RecipeAdditionFermentableListModel.cpp
    ${repoDir}/src/qtModels/listModels/RecipeAdditionHopListModel.cpp
    ${repoDir}/src/qtModels/listModels/RecipeAdditionMiscListModel.cpp
    ${repoDir}/src/qtModels/listModels/RecipeAdditionYeastListModel.cpp
    ${repoDir}/src/qtModels/listModels/RecipeAdjustmentSaltListModel.cpp
    ${repoDir}/src/qtModels/listModels/SaltListModel.cpp
    ${repoDir}/src/qtModels/listModels/StyleListModel.cpp
    ${repoDir}/src/qtModels/listModels/WaterListModel.cpp
    ${repoDir}/src/qtModels/listModels/YeastListModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/BoilSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/EquipmentSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/FermentableSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/FermentationSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/HopSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/MashSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/MiscSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/RecipeAdditionFermentableSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/RecipeAdditionHopSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/RecipeAdditionMiscSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/RecipeAdditionYeastSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/StyleSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/WaterSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/sortFilterProxyModels/YeastSortFilterProxyModel.cpp
    ${repoDir}/src/qtModels/tableModels/BoilStepTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/BoilTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/BtTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/EquipmentTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/FermentableTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/FermentationStepTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/FermentationTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/HopTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/MashStepTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/MashTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/MiscTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/RecipeAdditionFermentableTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/RecipeAdditionHopTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/RecipeAdditionMiscTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/RecipeAdditionYeastTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/RecipeAdjustmentSaltTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/SaltTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/StyleTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/WaterTableModel.cpp
    ${repoDir}/src/qtModels/tableModels/YeastTableModel.cpp
    ${repoDir}/src/serialization/ImportExport.cpp
    ${repoDir}/src/serialization/SerializationRecord.cpp
    ${repoDir}/src/serialization/json/BeerJson.cpp
    ${repoDir}/src/serialization/json/JsonCoding.cpp
    ${repoDir}/src/serialization/json/JsonMeasureableUnitsMapping.cpp
    ${repoDir}/src/serialization/json/JsonRecord.cpp
    ${repoDir}/src/serialization/json/JsonRecordDefinition.cpp
    ${repoDir}/src/serialization/json/JsonSchema.cpp
    ${repoDir}/src/serialization/json/JsonUtils.cpp
    ${repoDir}/src/serialization/json/JsonXPath.cpp
    ${repoDir}/src/serialization/xml/BeerXml.cpp
    ${repoDir}/src/serialization/xml/BtDomErrorHandler.cpp
    ${repoDir}/src/serialization/xml/XercesHelpers.cpp
    ${repoDir}/src/serialization/xml/XmlCoding.cpp
    ${repoDir}/src/serialization/xml/XmlMashRecord.cpp
    ${repoDir}/src/serialization/xml/XmlRecipeRecord.cpp
    ${repoDir}/src/serialization/xml/XmlRecord.cpp
    ${repoDir}/src/serialization/xml/XmlRecordDefinition.cpp
    ${repoDir}/src/trees/TreeFilterProxyModel.cpp
    ${repoDir}/src/trees/TreeModel.cpp
    ${repoDir}/src/trees/TreeNode.cpp
    ${repoDir}/src/trees/TreeView.cpp
    ${repoDir}/src/undoRedo/SimpleUndoableUpdate.cpp
    ${repoDir}/src/undoRedo/Undoable.cpp
    ${repoDir}/src/utils/BtException.cpp
    ${repoDir}/src/utils/BtStringConst.cpp
    ${repoDir}/src/utils/BtStringStream.cpp
    ${repoDir}/src/utils/EnumStringMapping.cpp
    ${repoDir}/src/utils/FileSystemHelpers.cpp
    ${repoDir}/src/utils/Fonts.cpp
    ${repoDir}/src/utils/FuzzyCompare.cpp
    ${repoDir}/src/utils/ImportRecordCount.cpp
    ${repoDir}/src/utils/MetaTypes.cpp
    ${repoDir}/src/utils/OStreamWriterForQFile.cpp
    ${repoDir}/src/utils/OptionalHelpers.cpp
    ${repoDir}/src/utils/PropertyPath.cpp
    ${repoDir}/src/utils/TimerUtils.cpp
    ${repoDir}/src/utils/TypeLookup.cpp
    ${repoDir}/src/widgets/Animator.cpp
    ${repoDir}/src/widgets/BtComboBoxBool.cpp
    ${repoDir}/src/widgets/BtComboBoxEnum.cpp
    ${repoDir}/src/widgets/BtComboBoxNamedEntity.cpp
    ${repoDir}/src/widgets/BtOptionalDateEdit.cpp
    ${repoDir}/src/widgets/InfoButton.cpp
    ${repoDir}/src/widgets/InfoText.cpp
    ${repoDir}/src/widgets/SelectionControl.cpp
    ${repoDir}/src/widgets/SmartAmountSettings.cpp
    ${repoDir}/src/widgets/SmartAmounts.cpp
    ${repoDir}/src/widgets/SmartDigitWidget.cpp
    ${repoDir}/src/widgets/SmartField.cpp
    ${repoDir}/src/widgets/SmartLabel.cpp
    ${repoDir}/src/widgets/SmartLineEdit.cpp
    ${repoDir}/src/widgets/ToggleSwitch.cpp
    ${repoDir}/src/widgets/UnitAndScalePopUpMenu.cpp
)
