# The current directory in the original source.
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
# The current directory in the output build directory.
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# Put all the required Qt dirs in the includes. 
#   That isn't quite what this include does. QT_USE_FILE references
#   a cmake macro include, which is good but not required.
#INCLUDE(${QT_USE_FILE})

# (ADD_PCH_RULE  _header_filename _src_list)
# Version 7/26/2010
#
# use this macro before "add_executable"
#
# _header_filename
#  header to make a .gch
#
# _src_list 
#   the variable name (do not use ${..}) which contains a
#     a list of sources (a.cpp b.cpp c.cpp ...)
#  This macro will append a header file to it, then this src_list can be used in
#  "add_executable..."
#
#
# Now a .gch file should be generated and gcc should use it. 
#        (add -Winvalid-pch to the cpp flags to verify)
#
# make clean should delete the pch file
#
# example : ADD_PCH_RULE(headers.h myprog_SRCS)
MACRO (ADD_PCH_RULE  _header_filename _src_list)
   SET(_gch_filename "${CMAKE_CURRENT_BINARY_DIR}/${_header_filename}.gch")
   SET(_header "${CMAKE_CURRENT_SOURCE_DIR}/${_header_filename}")

   LIST(APPEND ${_src_list} ${_gch_filename})

   SET (_args ${CMAKE_CXX_FLAGS})
   LIST(APPEND _args -c ${_header} -o ${_gch_filename})

   GET_DIRECTORY_PROPERTY(DIRINC INCLUDE_DIRECTORIES)
   FOREACH(_inc ${DIRINC})
      LIST(APPEND _args "-I" ${_inc})
   ENDFOREACH(_inc ${DIRINC})

   SEPARATE_ARGUMENTS(_args)

   ADD_CUSTOM_COMMAND(OUTPUT ${_gch_filename}
                      COMMAND rm -f ${_gch_filename}
                      COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_args}
                      DEPENDS ${_header})
ENDMACRO(ADD_PCH_RULE _src_list _header_filename)

# Variable that contains all the .cpp files in this project.
SET( brewtarget_SRCS
    ${SRCDIR}/AboutDialog.cpp
    ${SRCDIR}/Algorithms.cpp
    ${SRCDIR}/BeerXMLElement.cpp
    ${SRCDIR}/BeerXMLSortProxyModel.cpp
    ${SRCDIR}/boiltime.cpp
    ${SRCDIR}/BrewDayWidget.cpp
    ${SRCDIR}/BrewDayScrollWidget.cpp
    ${SRCDIR}/brewnote.cpp
    ${SRCDIR}/BrewNoteWidget.cpp
    ${SRCDIR}/BtTabWidget.cpp
    ${SRCDIR}/BtTreeItem.cpp
    ${SRCDIR}/BtTreeModel.cpp
    ${SRCDIR}/BtTreeView.cpp
    ${SRCDIR}/BtTreeFilterProxyModel.cpp
    ${SRCDIR}/BeerColorWidget.cpp
    ${SRCDIR}/BtDatePopup.cpp
    ${SRCDIR}/BtFolder.cpp
    ${SRCDIR}/BtLabel.cpp
    ${SRCDIR}/BtLineEdit.cpp
    ${SRCDIR}/BtTextEdit.cpp
    ${SRCDIR}/brewtarget.cpp
    ${SRCDIR}/BtSplashScreen.cpp
    ${SRCDIR}/CelsiusTempUnitSystem.cpp
    ${SRCDIR}/ColorMethods.cpp
    ${SRCDIR}/ConverterTool.cpp
    ${SRCDIR}/CustomComboBox.cpp
    ${SRCDIR}/database.cpp
    ${SRCDIR}/DatabaseSchemaHelper.cpp
    ${SRCDIR}/DiastaticPowerUnitSystem.cpp
    ${SRCDIR}/equipment.cpp
    ${SRCDIR}/EbcColorUnitSystem.cpp
    ${SRCDIR}/EquipmentButton.cpp
    ${SRCDIR}/EquipmentListModel.cpp
    ${SRCDIR}/EquipmentEditor.cpp
    ${SRCDIR}/FahrenheitTempUnitSystem.cpp
    ${SRCDIR}/fermentable.cpp
    ${SRCDIR}/FermentableEditor.cpp
    ${SRCDIR}/FermentableDialog.cpp
    ${SRCDIR}/FermentableSortFilterProxyModel.cpp
    ${SRCDIR}/FermentableTableModel.cpp
    ${SRCDIR}/HeatCalculations.cpp
    ${SRCDIR}/hop.cpp
    ${SRCDIR}/HopDialog.cpp
    ${SRCDIR}/HopEditor.cpp
    ${SRCDIR}/HopSortFilterProxyModel.cpp
    ${SRCDIR}/HopTableModel.cpp
    ${SRCDIR}/Html.cpp
    ${SRCDIR}/HydrometerTool.cpp
    ${SRCDIR}/instruction.cpp
    ${SRCDIR}/IbuGuSlider.cpp
    ${SRCDIR}/IbuMethods.cpp
    ${SRCDIR}/ImperialVolumeUnitSystem.cpp
    ${SRCDIR}/InventoryFormatter.cpp
    ${SRCDIR}/InstructionWidget.cpp
    ${SRCDIR}/Log.cpp
    ${SRCDIR}/MainWindow.cpp
    ${SRCDIR}/mash.cpp
    ${SRCDIR}/MashButton.cpp
    ${SRCDIR}/MashEditor.cpp
    ${SRCDIR}/MashListModel.cpp
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
    ${SRCDIR}/NamedMashEditor.cpp
    ${SRCDIR}/OgAdjuster.cpp
    ${SRCDIR}/OptionDialog.cpp
    ${SRCDIR}/PlatoDensityUnitSystem.cpp
    ${SRCDIR}/PreInstruction.cpp
    ${SRCDIR}/PrimingDialog.cpp
    ${SRCDIR}/QueuedMethod.cpp
    ${SRCDIR}/RangedSlider.cpp
    ${SRCDIR}/recipe.cpp
    ${SRCDIR}/RecipeFormatter.cpp
    ${SRCDIR}/RefractoDialog.cpp
    ${SRCDIR}/ScaleRecipeTool.cpp
    ${SRCDIR}/SgDensityUnitSystem.cpp
    ${SRCDIR}/SIVolumeUnitSystem.cpp
    ${SRCDIR}/SIWeightUnitSystem.cpp
    ${SRCDIR}/SrmColorUnitSystem.cpp
    ${SRCDIR}/StrikeWaterDialog.cpp
    ${SRCDIR}/style.cpp
    ${SRCDIR}/StyleButton.cpp
    ${SRCDIR}/StyleListModel.cpp
    ${SRCDIR}/StyleEditor.cpp
    ${SRCDIR}/StyleRangeWidget.cpp
    ${SRCDIR}/StyleSortFilterProxyModel.cpp
    ${SRCDIR}/TimerListDialog.cpp
    ${SRCDIR}/TimerMainDialog.cpp
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
    ${SRCDIR}/YeastSortFilterProxyModel.cpp
    ${SRCDIR}/MashDesigner.cpp
    ${SRCDIR}/BtDigitWidget.cpp
    ${SRCDIR}/PitchDialog.cpp
    ${SRCDIR}/RecipeExtrasWidget.cpp
)

# List of all the *.ui.
# TODO: can I somehow have a separate CMakeLists.txt
# in the ui/ directory instead of here?
SET( brewtarget_UIS
    ${UIDIR}/brewDayWidget.ui
    ${UIDIR}/brewDayScrollWidget.ui
    ${UIDIR}/brewNoteWidget.ui
    ${UIDIR}/mainWindow.ui
    ${UIDIR}/mashStepEditor.ui
    ${UIDIR}/miscEditor.ui
    ${UIDIR}/fermentableEditor.ui
    ${UIDIR}/hopEditor.ui
    ${UIDIR}/instructionWidget.ui
    ${UIDIR}/mashEditor.ui
    ${UIDIR}/namedMashEditor.ui
    ${UIDIR}/mashWizard.ui
    ${UIDIR}/ogAdjuster.ui
    ${UIDIR}/optionsDialog.ui
    ${UIDIR}/primingDialog.ui
    ${UIDIR}/refractoDialog.ui
    ${UIDIR}/strikeWaterDialog.ui
    ${UIDIR}/styleEditor.ui
    ${UIDIR}/timerMainDialog.ui
    ${UIDIR}/timerWidget.ui
    ${UIDIR}/waterEditor.ui
    ${UIDIR}/yeastEditor.ui
    ${UIDIR}/mashDesigner.ui
    ${UIDIR}/pitchDialog.ui
    ${UIDIR}/brewDayScrollWidget.ui
    ${UIDIR}/recipeExtrasWidget.ui
)

# The list of files that need to be moc'd.
# In other words, classes that have Q_OBJECT declarations and stuff.
SET( brewtarget_MOC_HEADERS
    ${SRCDIR}/AboutDialog.h
    ${SRCDIR}/BeerColorWidget.h
    ${SRCDIR}/BeerXMLElement.h
    ${SRCDIR}/BeerXMLSortProxyModel.h
    ${SRCDIR}/boiltime.h
    ${SRCDIR}/BrewDayWidget.h
    ${SRCDIR}/BrewDayScrollWidget.h
    ${SRCDIR}/brewnote.h
    ${SRCDIR}/BrewNoteWidget.h
    ${SRCDIR}/brewtarget.h
    ${SRCDIR}/BtTabWidget.h
    ${SRCDIR}/BtTreeModel.h
    ${SRCDIR}/BtTreeView.h
    ${SRCDIR}/BtTreeFilterProxyModel.h
    ${SRCDIR}/BtDigitWidget.h
    ${SRCDIR}/BtDatePopup.h
    ${SRCDIR}/BtFolder.h
    ${SRCDIR}/BtLabel.h
    ${SRCDIR}/BtLineEdit.h
    ${SRCDIR}/BtTextEdit.h
    ${SRCDIR}/BtSplashScreen.h
    ${SRCDIR}/ConverterTool.h
    ${SRCDIR}/CustomComboBox.h
    ${SRCDIR}/database.h
    ${SRCDIR}/EquipmentButton.h
    ${SRCDIR}/EquipmentListModel.h
    ${SRCDIR}/EquipmentEditor.h
    ${SRCDIR}/FermentableEditor.h
    ${SRCDIR}/FermentableDialog.h
    ${SRCDIR}/FermentableSortFilterProxyModel.h
    ${SRCDIR}/FermentableTableModel.h
    ${SRCDIR}/HopDialog.h
    ${SRCDIR}/HopEditor.h
    ${SRCDIR}/HopSortFilterProxyModel.h
    ${SRCDIR}/HopTableModel.h
    ${SRCDIR}/HydrometerTool.h
    ${SRCDIR}/IbuGuSlider.h
    ${SRCDIR}/InstructionWidget.h
    ${SRCDIR}/Log.h
    ${SRCDIR}/MainWindow.h
    ${SRCDIR}/MashButton.h
    ${SRCDIR}/MashDesigner.h
    ${SRCDIR}/MashEditor.h
    ${SRCDIR}/NamedMashEditor.h
    ${SRCDIR}/MashListModel.h
    ${SRCDIR}/MashStepEditor.h
    ${SRCDIR}/MashStepTableModel.h
    ${SRCDIR}/MashStepTableWidget.h
    ${SRCDIR}/MashWizard.h
    ${SRCDIR}/MiscDialog.h
    ${SRCDIR}/MiscEditor.h
    ${SRCDIR}/MiscSortFilterProxyModel.h
    ${SRCDIR}/MiscTableModel.h
    ${SRCDIR}/OgAdjuster.h
    ${SRCDIR}/OptionDialog.h
    ${SRCDIR}/PitchDialog.h
    ${SRCDIR}/PrimingDialog.h
    ${SRCDIR}/QueuedMethod.h
    ${SRCDIR}/RangedSlider.h
    ${SRCDIR}/RecipeExtrasWidget.h
    ${SRCDIR}/RecipeFormatter.h
    ${SRCDIR}/RefractoDialog.h
    ${SRCDIR}/ScaleRecipeTool.h
    ${SRCDIR}/StrikeWaterDialog.h
    ${SRCDIR}/StyleButton.h
    ${SRCDIR}/StyleListModel.h
    ${SRCDIR}/StyleEditor.h
    ${SRCDIR}/StyleRangeWidget.h
    ${SRCDIR}/StyleSortFilterProxyModel.h
    ${SRCDIR}/TimerListDialog.h
    ${SRCDIR}/TimerMainDialog.h
    ${SRCDIR}/TimerWidget.h
    ${SRCDIR}/unit.h
    ${SRCDIR}/WaterTableModel.h
    ${SRCDIR}/WaterTableWidget.h
    ${SRCDIR}/WaterEditor.h
    ${SRCDIR}/YeastDialog.h
    ${SRCDIR}/YeastEditor.h
    ${SRCDIR}/YeastSortFilterProxyModel.h
    ${SRCDIR}/YeastTableModel.h
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

SET( FILES_TO_TRANSLATE
     ${brewtarget_SRCS}
     ${brewtarget_UIS}
)

#======Speed up compilation by using precompiled headers for development======
#IF( UNIX AND NOT APPLE )
#   SET( precompiled_h "equipment.h" "fermentable.h" "hop.h" "instruction.h" "mash.h" "mashstep.h" "misc.h" "recipe.h" "style.h" "unit.h" "water.h" "yeast.h" "database.h" "brewtarget.h" )
#   FOREACH( header ${precompiled_h} )
#      ADD_PCH_RULE( ${header} brewtarget_SRCS )
#   ENDFOREACH()
#ENDIF()

#==================================Qt Junk=====================================

# Create the ui_*.h files from the *.ui files.
QT5_WRAP_UI(brewtarget_UIS_H ${brewtarget_UIS})
# Generate the resource .cpp from the .qrc file.
QT5_ADD_RESOURCES(brewtarget_QRC_CPP ${brewtarget_QRC})
QT5_ADD_RESOURCES(brewtarget_cfg_QRC_CPP ${brewtarget_cfg_QRC})

# Translations. CREATE_TRANSLATION will update the .ts files (do not want)
#QT5_CREATE_TRANSLATION(${TRANSLATIONS_FILES} ${FILES_TO_TRANSLATE})
# Creates the .qm files from the .ts files
QT5_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS_FILES})

# Add a target for the QM_FILES so that we can add
# the translations as a dependency for the executable later.
ADD_CUSTOM_TARGET(translations DEPENDS ${QM_FILES})

#============================Icon for Windows==================================

SET(brewtarget_ICON "")

IF( WIN32 AND MINGW )
  ADD_CUSTOM_COMMAND(OUTPUT ${WINDIR}/icon.o
                     COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                     -i${RCFILE}
                     -o${WINDIR}/icon.o
                     DEPENDS ${RCFILE}
  )
  SET(brewtarget_ICON ${WINDIR}/icon.o)
ELSEIF(WIN32)
  SET(brewtarget_ICON ${RCFILE})
ENDIF()

#===========================Create the binary==================================

# This creates a "library" of object files so that we do not have to recompile
# the source files once per target, but rather, just once EVER.
ADD_LIBRARY(
   btobjlib
   OBJECT
   ${brewtarget_SRCS}
   ${brewtarget_MOC_SRCS}
   ${brewtarget_QRC_CPP}
   ${brewtarget_UIS_H}
)

IF( APPLE )
   # It seems I have to set these properties in this file, or things don't
   # get installed
   SET_SOURCE_FILES_PROPERTIES(
      ${brewtarget_MAC_ICNS}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources"
   )
   SET_SOURCE_FILES_PROPERTIES(
      ${brewtarget_DATA}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources"
   )
   SET_SOURCE_FILES_PROPERTIES(
      ${brewtarget_DOCS}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources/en.lproj"
   )
   SET_SOURCE_FILES_PROPERTIES(
      ${brewtarget_SOUNDS}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources/sounds"
   )
   SET_SOURCE_FILES_PROPERTIES(
      ${QM_FILES}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION "Resources/translations_qm"
   )
   ADD_EXECUTABLE(
      ${brewtarget_EXECUTABLE}
      MACOSX_BUNDLE
      ${SRCDIR}/main.cpp
      ${brewtarget_MAC_ICNS}
      ${brewtarget_DATA}
      ${brewtarget_DOCS}
      ${brewtarget_SOUNDS}
      ${QM_FILES}
      $<TARGET_OBJECTS:btobjlib>
   )
ELSE()
   ADD_EXECUTABLE(
      ${brewtarget_EXECUTABLE}
      ${SRCDIR}/main.cpp
      ${brewtarget_ICON}
      $<TARGET_OBJECTS:btobjlib>
   )
ENDIF()

ADD_DEPENDENCIES( ${brewtarget_EXECUTABLE} translations )

# Link brewtarget against appropriate libraries.
IF( WIN32 AND MINGW )
   ############################################################################
   # Need to set some linker flags that I don't know how to get
   # automatically.
   ############################################################################

   # MinGW-specific flags.
   #    '-Wl,-subsystem,windows' supresses the output command window.
   #    '-Wl,-s' strips the executable of symbols.
   SET_TARGET_PROPERTIES( ${brewtarget_EXECUTABLE} PROPERTIES LINK_FLAGS "-Wl,-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc -mthreads -Wl,-subsystem,windows")
ENDIF()

SET( QT5_USE_MODULES_LIST
   ${brewtarget_EXECUTABLE}
   Widgets
   Network
   PrintSupport
   Sql
   Svg
   Xml
   )

IF( NOT ${NO_QTMULTIMEDIA})
SET( QT5_USE_MODULES_LIST ${QT5_USE_MODULES_LIST} Multimedia)
ENDIF()

QT5_USE_MODULES( ${QT5_USE_MODULES_LIST})

#=================================Tests========================================

#QT4_WRAP_CPP( testing_MOC_SRCS ${SRCDIR}/Testing.h )
ADD_EXECUTABLE(
   brewtarget_tests
   ${SRCDIR}/Testing.cpp
   ${testing_MOC_SRCS}
   $<TARGET_OBJECTS:btobjlib>
)

SET( QT5_USE_MODULES_LIST
   brewtarget_tests
   Widgets
   Network
   PrintSupport
   Sql
   Svg
   Xml
   Test
   )

IF( NOT ${NO_QTMULTIMEDIA})
SET( QT5_USE_MODULES_LIST ${QT5_USE_MODULES_LIST} Multimedia)
ENDIF()

QT5_USE_MODULES(${QT5_USE_MODULES_LIST})

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
#=================================Installs=====================================

# Install executable.
INSTALL( TARGETS ${brewtarget_EXECUTABLE}
         BUNDLE DESTINATION .
         RUNTIME DESTINATION ${TARGETPATH}
         COMPONENT ${RUNTIME_INSTALL_COMPONENT} )

# Install the translations.
INSTALL(FILES ${QM_FILES}
        DESTINATION "${DATAPATH}/translations_qm"
        COMPONENT ${DATA_INSTALL_COMPONENT} )

# http://pmarinc-tidylib.googlecode.com/hg/src/Sigil/CMakeLists.txt?r=8276c61e05bc385d5ffbcc58e1f007f84b0c52df
IF( APPLE )
   # make addframeworks should copy the Qt libraries to the app.
   SET(QT_BINARY_DIR "${_qt5Core_install_prefix}/bin")
   ADD_CUSTOM_TARGET(
      addframeworks ALL
      COMMAND ${QT_BINARY_DIR}/macdeployqt "Brewtarget.app" -dmg
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${brewtarget_EXECUTABLE}
   )
ENDIF()

