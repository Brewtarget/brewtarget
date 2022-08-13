#
# src/CMakeLists.txt is part of Brewtarget, and is Copyright the following
# authors 2009-2022:
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

# We need to look in two places for _our_ headers.  The "regular" source tree ones will be in the src tree
# (${CMAKE_CURRENT_SOURCE_DIR}), but generated ones (eg generated from UI files or from Qt MOC parsing) will be in
# the build/src directory (${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

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
    ${repoDir}/src/BeerColorWidget.cpp
    ${repoDir}/src/boiltime.cpp
    ${repoDir}/src/BrewDayFormatter.cpp
    ${repoDir}/src/BrewDayScrollWidget.cpp
    ${repoDir}/src/BrewDayWidget.cpp
    ${repoDir}/src/BrewNoteWidget.cpp
    ${repoDir}/src/brewtarget.cpp
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
    ${repoDir}/src/xml/XmlRecordCount.cpp
    ${repoDir}/src/xml/XmlRecord.cpp
    ${repoDir}/src/YeastDialog.cpp
    ${repoDir}/src/YeastEditor.cpp
    ${repoDir}/src/YeastSortFilterProxyModel.cpp
)

# List of all the *.ui
# TODO: can I somehow have a separate CMakeLists.txt
# in the ui/ directory instead of here?
#
# You can recreate the body of this list by running the following from the bash prompt in the build directory:
#    find ../ui -name '*.ui' | sort  | sed 's+^../ui+    ${repoDir}/src+'
#
set(filesToCompile_ui
    ${repoDir}/ui/ancestorDialog.ui
    ${repoDir}/ui/brewDayScrollWidget.ui
    ${repoDir}/ui/brewDayWidget.ui
    ${repoDir}/ui/brewNoteWidget.ui
    ${repoDir}/ui/BtPrintAndPreview.ui
    ${repoDir}/ui/equipmentEditor.ui
    ${repoDir}/ui/fermentableEditor.ui
    ${repoDir}/ui/hopEditor.ui
    ${repoDir}/ui/instructionWidget.ui
    ${repoDir}/ui/mainWindow.ui
    ${repoDir}/ui/mashDesigner.ui
    ${repoDir}/ui/mashEditor.ui
    ${repoDir}/ui/mashStepEditor.ui
    ${repoDir}/ui/mashWizard.ui
    ${repoDir}/ui/miscEditor.ui
    ${repoDir}/ui/namedMashEditor.ui
    ${repoDir}/ui/ogAdjuster.ui
    ${repoDir}/ui/optionsDialog.ui
    ${repoDir}/ui/pitchDialog.ui
    ${repoDir}/ui/primingDialog.ui
    ${repoDir}/ui/recipeExtrasWidget.ui
    ${repoDir}/ui/refractoDialog.ui
    ${repoDir}/ui/strikeWaterDialog.ui
    ${repoDir}/ui/styleEditor.ui
    ${repoDir}/ui/timerMainDialog.ui
    ${repoDir}/ui/timerWidget.ui
    ${repoDir}/ui/waterDialog.ui
    ${repoDir}/ui/waterEditor.ui
    ${repoDir}/ui/yeastEditor.ui
)

#
# The list of files that need to be run through Qt's Meta-Object Compiler (moc) which handles Qt's C++ extensions.
# This is ONLY headers of classes that that have Q_OBJECT declarations and stuff; it is NOT a list of ALL header files!
#
# You can recreate the body of this list by running the following from the bash prompt in the build directory:
#    grep -rl Q_OBJECT ../src | sort | sed 's+^../src+    ${repoDir}/src+' | grep -v Testing.h
#
set(filesToCompile_qtMocHeaders
    ${repoDir}/src/AboutDialog.h
    ${repoDir}/src/AlcoholTool.h
    ${repoDir}/src/AncestorDialog.h
    ${repoDir}/src/BeerColorWidget.h
    ${repoDir}/src/boiltime.h
    ${repoDir}/src/BrewDayFormatter.h
    ${repoDir}/src/BrewDayScrollWidget.h
    ${repoDir}/src/BrewDayWidget.h
    ${repoDir}/src/brewtarget.h
    ${repoDir}/src/BrewNoteWidget.h
    ${repoDir}/src/BtDatePopup.h
    ${repoDir}/src/BtDigitWidget.h
    ${repoDir}/src/BtFolder.h
    ${repoDir}/src/BtLabel.h
    ${repoDir}/src/BtLineEdit.h
    ${repoDir}/src/BtSplashScreen.h
    ${repoDir}/src/BtTabWidget.h
    ${repoDir}/src/BtTextEdit.h
    ${repoDir}/src/BtTreeFilterProxyModel.h
    ${repoDir}/src/BtTreeModel.h
    ${repoDir}/src/BtTreeView.h
    ${repoDir}/src/CMakeLists.txt
    ${repoDir}/src/ConverterTool.h
    ${repoDir}/src/CustomComboBox.h
    ${repoDir}/src/database/ObjectStore.h
    ${repoDir}/src/EquipmentButton.h
    ${repoDir}/src/EquipmentEditor.h
    ${repoDir}/src/EquipmentListModel.h
    ${repoDir}/src/FermentableDialog.h
    ${repoDir}/src/FermentableEditor.h
    ${repoDir}/src/FermentableSortFilterProxyModel.h
    ${repoDir}/src/HopDialog.h
    ${repoDir}/src/HopEditor.h
    ${repoDir}/src/HopSortFilterProxyModel.h
    ${repoDir}/src/HydrometerTool.h
    ${repoDir}/src/IbuGuSlider.h
    ${repoDir}/src/InstructionWidget.h
    ${repoDir}/src/MainWindow.h
    ${repoDir}/src/MashButton.h
    ${repoDir}/src/MashComboBox.h
    ${repoDir}/src/MashDesigner.h
    ${repoDir}/src/MashEditor.h
    ${repoDir}/src/MashListModel.h
    ${repoDir}/src/MashStepEditor.h
    ${repoDir}/src/MashStepTableWidget.h
    ${repoDir}/src/MashWizard.h
    ${repoDir}/src/measurement/Unit.h
    ${repoDir}/src/MiscDialog.h
    ${repoDir}/src/MiscEditor.h
    ${repoDir}/src/MiscSortFilterProxyModel.h
    ${repoDir}/src/model/BrewNote.h
    ${repoDir}/src/model/Equipment.h
    ${repoDir}/src/model/Fermentable.h
    ${repoDir}/src/model/Hop.h
    ${repoDir}/src/model/Instruction.h
    ${repoDir}/src/model/Inventory.h
    ${repoDir}/src/model/Mash.h
    ${repoDir}/src/model/MashStep.h
    ${repoDir}/src/model/Misc.h
    ${repoDir}/src/model/NamedEntity.h
    ${repoDir}/src/model/NamedEntityWithInventory.h
    ${repoDir}/src/model/Recipe.h
    ${repoDir}/src/model/Salt.h
    ${repoDir}/src/model/Style.h
    ${repoDir}/src/model/Water.h
    ${repoDir}/src/model/Yeast.h
    ${repoDir}/src/NamedEntitySortProxyModel.h
    ${repoDir}/src/NamedMashEditor.h
    ${repoDir}/src/OgAdjuster.h
    ${repoDir}/src/OptionDialog.h
    ${repoDir}/src/PitchDialog.h
    ${repoDir}/src/PrimingDialog.h
    ${repoDir}/src/PrintAndPreviewDialog.h
    ${repoDir}/src/RangedSlider.h
    ${repoDir}/src/RecipeExtrasWidget.h
    ${repoDir}/src/RecipeFormatter.h
    ${repoDir}/src/RefractoDialog.h
    ${repoDir}/src/ScaleRecipeTool.h
    ${repoDir}/src/SimpleUndoableUpdate.h
    ${repoDir}/src/StrikeWaterDialog.h
    ${repoDir}/src/StyleButton.h
    ${repoDir}/src/StyleEditor.h
    ${repoDir}/src/StyleListModel.h
    ${repoDir}/src/StyleRangeWidget.h
    ${repoDir}/src/StyleSortFilterProxyModel.h
    ${repoDir}/src/tableModels/BtTableModel.h
    ${repoDir}/src/tableModels/FermentableTableModel.h
    ${repoDir}/src/tableModels/HopTableModel.h
    ${repoDir}/src/tableModels/MashStepTableModel.h
    ${repoDir}/src/tableModels/MiscTableModel.h
    ${repoDir}/src/tableModels/SaltTableModel.h
    ${repoDir}/src/tableModels/WaterTableModel.h
    ${repoDir}/src/tableModels/YeastTableModel.h
    ${repoDir}/src/TimerListDialog.h
    ${repoDir}/src/TimerMainDialog.h
    ${repoDir}/src/TimerWidget.h
    ${repoDir}/src/WaterButton.h
    ${repoDir}/src/WaterDialog.h
    ${repoDir}/src/WaterEditor.h
    ${repoDir}/src/WaterListModel.h
    ${repoDir}/src/WaterSortFilterProxyModel.h
    ${repoDir}/src/WaterTableWidget.h
    ${repoDir}/src/widgets/Animator.h
    ${repoDir}/src/widgets/SelectionControl.h
    ${repoDir}/src/widgets/ToggleSwitch.h
    ${repoDir}/src/widgets/UnitAndScalePopUpMenu.h
    ${repoDir}/src/YeastDialog.h
    ${repoDir}/src/YeastEditor.h
    ${repoDir}/src/YeastSortFilterProxyModel.h
)

set(FILES_TO_TRANSLATE ${filesToCompile_cpp}
                       ${filesToCompile_ui})

#==================================Qt Junk=====================================

# Create the ui_*.h files from the *.ui files.
QT5_WRAP_UI(brewtarget_UIS_H ${filesToCompile_ui})
# Generate the resource .cpp from the .qrc file.
QT5_ADD_RESOURCES(brewtarget_QRC_CPP ${resourceCollectionFile})
QT5_ADD_RESOURCES(brewtarget_cfg_QRC_CPP ${brewtarget_cfg_QRC})

# Translations. CREATE_TRANSLATION will update the .ts files (do not want)
#QT5_CREATE_TRANSLATION(${filesToInstall_translations} ${FILES_TO_TRANSLATE})
# Creates the .qm files from the .ts files
QT5_ADD_TRANSLATION(QM_FILES ${filesToInstall_translations})

# Add a target for the QM_FILES so that we can add
# the translations as a dependency for the executable later.
ADD_CUSTOM_TARGET(translations DEPENDS ${QM_FILES})

#============================Icon for Windows==================================

SET(brewtarget_ICON "")

IF( WIN32 AND MINGW )
  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/src/icon.o
                     COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                     -i${filesToInstall_windowsIcon}
                     -o${CMAKE_BINARY_DIR}/src/icon.o
                     DEPENDS ${filesToInstall_windowsIcon}
  )
  SET(brewtarget_ICON ${CMAKE_BINARY_DIR}/src/icon.o)
ELSEIF(WIN32)
  SET(brewtarget_ICON ${filesToInstall_windowsIcon})
ENDIF()

#===========================Create the binary==================================

# This creates a "library" of object files so that we do not have to recompile
# the source files once per target, but rather, just once EVER.
add_library(
   btobjlib
   OBJECT
   ${filesToCompile_cpp}
   ${brewtarget_MOC_SRCS}
   ${brewtarget_QRC_CPP}
   ${brewtarget_UIS_H}
)

IF( APPLE )
   # It seems I have to set these properties in this file, or things don't
   # get installed
   SET_SOURCE_FILES_PROPERTIES(
      ${filesToInstall_macIcons}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources"
   )
   SET_SOURCE_FILES_PROPERTIES(
      ${filesToInstall_data}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources"
   )
   SET_SOURCE_FILES_PROPERTIES(
      ${filesToInstall_docs}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources/en.lproj"
   )
   SET_SOURCE_FILES_PROPERTIES(
      ${filesToInstall_sounds}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources/sounds"
   )
   SET_SOURCE_FILES_PROPERTIES(
      ${QM_FILES}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources/translations_qm"
   )
   ADD_EXECUTABLE(
      ${fileName_executable}
      MACOSX_BUNDLE
      ${repoDir}/src/main.cpp
      ${filesToInstall_macIcons}
      ${filesToInstall_data}
      ${filesToInstall_docs}
      ${filesToInstall_sounds}
      ${QM_FILES}
      $<TARGET_OBJECTS:btobjlib>
   )
ELSE()
   ADD_EXECUTABLE(
      ${fileName_executable}
      ${repoDir}/src/main.cpp
      ${brewtarget_ICON}
      $<TARGET_OBJECTS:btobjlib>
   )
ENDIF()

ADD_DEPENDENCIES( ${fileName_executable} translations )

SET( QT5_USE_MODULES_LIST
   ${fileName_executable}
   Qt5::Widgets
   Qt5::Network
   Qt5::PrintSupport
   Qt5::Sql
   Qt5::Svg
   Qt5::Xml
   )

SET( QT5_USE_MODULES_LIST ${QT5_USE_MODULES_LIST} Qt5::Multimedia)

#
# All the libraries
#
# ********************************************************************************************************************
# * Note that if you change this you need to make corresponding changes to this to target_link_libraries in the Tests
# * section below.
# *
# * .:TODO:. Work out how to avoid this copy-and-paste duplication
# ********************************************************************************************************************
#
target_link_libraries(
   ${QT5_USE_MODULES_LIST}
   ${XercesC_LIBRARIES}
   ${XalanC_LIBRARIES}
   ${Boost_LIBRARIES}
   ${DL_LIBRARY}
   ${Backtrace_LIBRARIES}
)

#=================================Tests========================================

#QT4_WRAP_CPP( testing_MOC_SRCS ${repoDir}/src/unitTests/Testing.h )
ADD_EXECUTABLE(
   brewtarget_tests
   ${repoDir}/src/unitTests/Testing.cpp
   ${testing_MOC_SRCS}
   $<TARGET_OBJECTS:btobjlib>
)

SET( QT5_USE_MODULES_LIST
   brewtarget_tests
   Qt5::Widgets
   Qt5::Network
   Qt5::PrintSupport
   Qt5::Sql
   Qt5::Svg
   Qt5::Xml
   Qt5::Test
   )

SET( QT5_USE_MODULES_LIST ${QT5_USE_MODULES_LIST} Qt5::Multimedia)

# NB Needs to be same as the one a few lines above. Sigh.
target_link_libraries(
   ${QT5_USE_MODULES_LIST}
   ${XercesC_LIBRARIES}
   ${XalanC_LIBRARIES}
   ${Boost_LIBRARIES}
   ${DL_LIBRARY}
   ${Backtrace_LIBRARIES}
)

ADD_TEST(
   NAME pstdintTest
   COMMAND brewtarget_tests pstdintTest
)
ADD_TEST(
   NAME recipeCalcTest_allGrain
   COMMAND brewtarget_tests recipeCalcTest_allGrain
)
ADD_TEST(
   NAME postBoilLossOgTest
   COMMAND brewtarget_tests postBoilLossOgTest
)
ADD_TEST(
   NAME testUnitConversions
   COMMAND brewtarget_tests testUnitConversions
)
add_test(
   NAME testLogRotation
   COMMAND brewtarget_tests testLogRotation
)
#=================================Installs=====================================

# Install executable.
INSTALL( TARGETS ${fileName_executable}
         BUNDLE DESTINATION .
         RUNTIME DESTINATION ${installSubDir_bin}
         COMPONENT ${RUNTIME_INSTALL_COMPONENT} )

IF( WIN32 )
      install(CODE
      "
         file(GET_RUNTIME_DEPENDENCIES
            RESOLVED_DEPENDENCIES_VAR deps
            UNRESOLVED_DEPENDENCIES_VAR confdeps
            EXECUTABLES src/${fileName_executable}.exe
            PRE_EXCLUDE_REGEXES \"^api-ms-win.*\$|^ext-ms.*\$\"
            POST_EXCLUDE_REGEXES \".*system32.*\"
            DIRECTORIES ${Mingw_Path}
         )
         #message(\"Installing: \${deps}\")
         #message(\"Not found: \${confdeps}\")
         FOREACH (fdep IN LISTS deps)
            message( \"\${fdep}\" )
            file( INSTALL
               DESTINATION bin
               TYPE SHARED_LIBRARY
               FOLLOW_SYMLINK_CHAIN
               FILES \"\${fdep}\")
         ENDFOREACH()
      ")
   ENDIF()

# Install the translations.
INSTALL(FILES ${QM_FILES}
        DESTINATION "${installSubDir_data}/translations_qm"
        COMPONENT ${DATA_INSTALL_COMPONENT} )
