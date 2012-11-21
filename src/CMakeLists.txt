# The current directory in the original source.
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
# The current directory in the output build directory.
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# Put all the required Qt dirs in the includes.
INCLUDE(${QT_USE_FILE})
INCLUDE(${PHONON_INCLUDES})

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
    ${SRCDIR}/Algorithms.cpp
    ${SRCDIR}/BeerXMLElement.cpp
    ${SRCDIR}/BeerXMLSortProxyModel.cpp
    ${SRCDIR}/BrewDayWidget.cpp
    ${SRCDIR}/BrewDayScrollWidget.cpp
    ${SRCDIR}/brewnote.cpp
    ${SRCDIR}/BrewNoteWidget.cpp
    ${SRCDIR}/BrewTargetTreeItem.cpp
    ${SRCDIR}/BrewTargetTreeModel.cpp
    ${SRCDIR}/BrewTargetTreeView.cpp
    ${SRCDIR}/BtTreeFilterProxyModel.cpp
    ${SRCDIR}/BeerColorWidget.cpp
    ${SRCDIR}/BtLabel.cpp
    ${SRCDIR}/brewtarget.cpp
    ${SRCDIR}/BtSplashScreen.cpp
    #${SRCDIR}/BtSqlQuery.cpp
    ${SRCDIR}/CelsiusTempUnitSystem.cpp
    ${SRCDIR}/ColorMethods.cpp
    ${SRCDIR}/ConverterTool.cpp
    ${SRCDIR}/CustomComboBox.cpp
    ${SRCDIR}/database.cpp
    ${SRCDIR}/equipment.cpp
    ${SRCDIR}/EquipmentButton.cpp
    ${SRCDIR}/EquipmentListModel.cpp
    ${SRCDIR}/EquipmentEditor.cpp
    ${SRCDIR}/FahrenheitTempUnitSystem.cpp
    ${SRCDIR}/fermentable.cpp
    ${SRCDIR}/FermentableEditor.cpp
    ${SRCDIR}/FermentableDialog.cpp
    ${SRCDIR}/FermentableSortFilterProxyModel.cpp
    ${SRCDIR}/FermentableTableModel.cpp
    ${SRCDIR}/main.cpp
    ${SRCDIR}/HeatCalculations.cpp
    ${SRCDIR}/hop.cpp
    ${SRCDIR}/HopDialog.cpp
    ${SRCDIR}/HopEditor.cpp
    ${SRCDIR}/HopSortFilterProxyModel.cpp
    ${SRCDIR}/HopTableModel.cpp
    ${SRCDIR}/HtmlViewer.cpp
    ${SRCDIR}/instruction.cpp
    ${SRCDIR}/IbuMethods.cpp
    ${SRCDIR}/ImperialVolumeUnitSystem.cpp
    ${SRCDIR}/InstructionWidget.cpp
    ${SRCDIR}/MainWindow.cpp
    ${SRCDIR}/MaltinessWidget.cpp
    ${SRCDIR}/mash.cpp
    ${SRCDIR}/MashComboBox.cpp
    ${SRCDIR}/MashEditor.cpp
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
    ${SRCDIR}/OgAdjuster.cpp
    ${SRCDIR}/OptionDialog.cpp
    ${SRCDIR}/PreInstruction.cpp
    ${SRCDIR}/PrimingDialog.cpp
    ${SRCDIR}/QueuedMethod.cpp
    ${SRCDIR}/recipe.cpp
    ${SRCDIR}/RecipeFormatter.cpp
    ${SRCDIR}/RecipeListModel.cpp
    ${SRCDIR}/RefractoDialog.cpp
    ${SRCDIR}/SetterCommand.cpp
    ${SRCDIR}/SetterCommandStack.cpp
    ${SRCDIR}/ScaleRecipeTool.cpp
    ${SRCDIR}/SIVolumeUnitSystem.cpp
    ${SRCDIR}/SIWeightUnitSystem.cpp
    ${SRCDIR}/style.cpp
    ${SRCDIR}/StyleButton.cpp
    ${SRCDIR}/StyleListModel.cpp
    ${SRCDIR}/StyleEditor.cpp
    ${SRCDIR}/TimerListDialog.cpp
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
    ${SRCDIR}/RecipeExtrasDialog.cpp
    ${SRCDIR}/MashDesigner.cpp
    ${SRCDIR}/BtDigitWidget.cpp
    ${SRCDIR}/PitchDialog.cpp
    ${SRCDIR}/RecipeExtrasWidget.cpp
)

# List of all the *.ui.
# TODO: can I somehow have a separate CMakeLists.txt
# in the ui/ directory instead of here?
SET( brewtarget_UIS
    ${UIDIR}/aboutDialog.ui
    ${UIDIR}/brewDayWidget.ui
    ${UIDIR}/brewDayScrollWidget.ui
    ${UIDIR}/brewNoteWidget.ui
    ${UIDIR}/btSplashScreen.ui
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

# The list of files that need to be moc'd.
# In other words, classes that have Q_OBJECT declarations and stuff.
SET( brewtarget_MOC_HEADERS
    ${SRCDIR}/BeerColorWidget.h
    ${SRCDIR}/BeerXMLElement.h
    ${SRCDIR}/BeerXMLSortProxyModel.h
    ${SRCDIR}/BrewDayWidget.h
    ${SRCDIR}/BrewDayScrollWidget.h
    ${SRCDIR}/brewnote.h
    ${SRCDIR}/BrewNoteWidget.h
    ${SRCDIR}/brewtarget.h
    ${SRCDIR}/BrewTargetTreeModel.h
    ${SRCDIR}/BrewTargetTreeView.h
    ${SRCDIR}/BtTreeFilterProxyModel.h
    ${SRCDIR}/BtDigitWidget.h
    ${SRCDIR}/BtLabel.h
    ${SRCDIR}/BtSplashScreen.h
    #${SRCDIR}/BtSqlQuery.h
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
    ${SRCDIR}/InstructionWidget.h
    ${SRCDIR}/MainWindow.h
    ${SRCDIR}/MaltinessWidget.h
    ${SRCDIR}/MashComboBox.h
    ${SRCDIR}/MashDesigner.h
    ${SRCDIR}/MashEditor.h
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
    ${SRCDIR}/RecipeExtrasDialog.h
    ${SRCDIR}/RecipeExtrasWidget.h
    ${SRCDIR}/RecipeFormatter.h
    ${SRCDIR}/RecipeListModel.h
    ${SRCDIR}/RefractoDialog.h
    ${SRCDIR}/ScaleRecipeTool.h
    ${SRCDIR}/SetterCommandStack.h
    ${SRCDIR}/StyleButton.h
    ${SRCDIR}/StyleListModel.h
    ${SRCDIR}/StyleEditor.h
    ${SRCDIR}/TimerListDialog.h
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

#======================Some Custom Targets/Commands============================

# Creates a target for ${CMAKE_CURRENT_BINARY_DIR}/blankdb.sqlite needed by
# the brewtarget.qrc resource file.
ADD_CUSTOM_COMMAND( OUTPUT blankdb.sqlite
                    COMMAND ${RM_COMMAND} ARGS ${RM_ARGS} "blankdb.sqlite" # Need to remove it if it exists first.
                    COMMAND ${SQLITE3_CMD} ARGS "blankdb.sqlite" "<" "${ROOTDIR}/create_db.sql"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    MAIN_DEPENDENCY "${ROOTDIR}/create_db.sql"
)

#==================================Qt Junk=====================================

# Create the moc sources from the headers.
QT4_WRAP_CPP(brewtarget_MOC_SRCS ${brewtarget_MOC_HEADERS})

# Create the ui_*.h files from the *.ui files.
QT4_WRAP_UI(brewtarget_UIS_H ${brewtarget_UIS})

# Generate the resource .cpp from the .qrc file.
QT4_ADD_RESOURCES(brewtarget_QRC_CPP ${brewtarget_QRC})

# Translations
IF(UPDATE_TRANSLATIONS)
  # Updates the .ts files with translatable strings, AND creates the binary .qm files
  QT4_CREATE_TRANSLATION(QM_FILES
                         ${FILES_TO_TRANSLATE}
                         ${TRANSLATIONS_FILES}
  )
ELSE()
  # Creates the .qm files without modifying the .ts files.
  QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS_FILES})
ENDIF() 

# Add a target for the QM_FILES so that we can add
# the translations as a dependency for the executable later.
ADD_CUSTOM_TARGET(translations DEPENDS ${QM_FILES})

#============================Icon for Windows==================================

IF( WIN32 AND MINGW )
  ADD_CUSTOM_COMMAND(OUTPUT ${WINDIR}/icon.o
                     COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                     -i${RCFILE}
                     -o${WINDIR}/icon.o
                     DEPENDS ${RCFILE}
  )
  SET(brewtarget_SRCS ${brewtarget_SRCS} ${WINDIR}/icon.o)
ELSEIF(WIN32)
  SET(brewtarget_SRCS ${brewtarget_SRCS} ${RCFILE})
ENDIF()

#===========================Create the binary==================================

ADD_EXECUTABLE( ${brewtarget_EXECUTABLE} ${brewtarget_SRCS} ${brewtarget_MOC_SRCS} ${brewtarget_QRC_CPP} ${brewtarget_UIS_H} )
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
   SET_TARGET_PROPERTIES( ${brewtarget_EXECUTABLE} PROPERTIES LINK_FLAGS "-enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc -mthreads -Wl,-subsystem,windows")
ENDIF()

# Link brewtarget against Qt libraries defined by QT_LIBRARIES
TARGET_LINK_LIBRARIES( ${brewtarget_EXECUTABLE} ${QT_LIBRARIES} ${PHONON_LIBS} ${EXT_LIBRARIES} )
MESSAGE( STATUS "Libraries: ${QT_LIBRARIES} ${PHONON_LIBS} ${EXT_LIBRARIES}" )

#=================================Installs=====================================

# Install executable.
INSTALL( TARGETS ${brewtarget_EXECUTABLE}
         RUNTIME DESTINATION ${TARGETPATH} )

# Install the translations.
INSTALL(FILES ${QM_FILES}
        DESTINATION "${DATAPATH}/translations_qm")

IF( APPLE )
   # This SHOULD run macdeployqt on the app bundle at install time.
   # Don't know if it works. Could someone try?
   # May have to add a WORKING_DIRECTORY.
   INSTALL( CODE
            "EXECUTE_PROCESS( COMMAND ${QT_BINARY_DIR}/macdeployqt ${BT_BUNDLE_DIR} )" )
ENDIF()
