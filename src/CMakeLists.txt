# Put all the required Qt dirs in the includes.
INCLUDE(${QT_USE_FILE})

# Where the binary goes.
SET(brewtarget_EXECUTABLE "../brewtarget")

# Variable that contains all the .cpp files in this project.
SET( brewtarget_SRCS
    Algorithms.cpp
    database.cpp
    BeerXMLElement.cpp
    BrewDayWidget.cpp
    BeerColorWidget.cpp
    brewtarget.cpp
    CelsiusTempUnitSystem.cpp
    ColorMethods.cpp
    ConverterTool.cpp
    equipment.cpp
    EquipmentComboBox.cpp
    EquipmentEditor.cpp
    FahrenheitTempUnitSystem.cpp
    fermentable.cpp
    FermentableEditor.cpp
    FermentableDialog.cpp
    FermentableTableModel.cpp
    FermentableTableWidget.cpp
    main.cpp
    HeatCalculations.cpp
    hop.cpp
    HopDialog.cpp
    HopEditor.cpp
    HopTableModel.cpp
    HopTableWidget.cpp
    HtmlViewer.cpp
    instruction.cpp
    IbuMethods.cpp
    ImperialVolumeUnitSystem.cpp
    InstructionWidget.cpp
    MainWindow.cpp
    MaltinessWidget.cpp
    mash.cpp
    MashComboBox.cpp
    MashEditor.cpp
    mashstep.cpp
    MashStepEditor.cpp
    MashStepTableModel.cpp
    MashStepTableWidget.cpp
    MashWizard.cpp
    matrix.cpp
    misc.cpp
    MiscEditor.cpp
    MiscDialog.cpp
    MiscTableModel.cpp
    MiscTableWidget.cpp
    observable.cpp
    observer.cpp
    OgAdjuster.cpp
    OptionDialog.cpp
    PreInstruction.cpp
    PrimingDialog.cpp
    recipe.cpp
    RecipeComboBox.cpp
    RecipeFormatter.cpp
    RefractoDialog.cpp
    ScaleRecipeTool.cpp
    SIVolumeUnitSystem.cpp
    SIWeightUnitSystem.cpp
    stringparsing.cpp
    style.cpp
    StyleComboBox.cpp
    StyleEditor.cpp
    TimerListDialog.cpp
    TimerWidget.cpp
    TimeUnitSystem.cpp
    unit.cpp
    UnitSystem.cpp
    USVolumeUnitSystem.cpp
    USWeightUnitSystem.cpp
    water.cpp
    WaterTableModel.cpp
    WaterTableWidget.cpp
    xml.cpp
    xmlnode.cpp
    xmltree.cpp
    yeast.cpp
    YeastDialog.cpp
    YeastEditor.cpp
    YeastTableModel.cpp
    YeastTableWidget.cpp
    RecipeExtrasDialog.cpp
    MashDesigner.cpp
    BtDigitWidget.cpp
    PitchDialog.cpp
)

# List of all the *.ui.
# TODO: can I somehow have a separate CMakeLists.txt
# in the ui/ directory instead of here?
SET( brewtarget_UIS
    ../ui/aboutDialog.ui
    ../ui/brewDayWidget.ui
    ../ui/converterTool.ui
    ../ui/mainWindow.ui
    ../ui/mashStepEditor.ui
    ../ui/miscEditor.ui
    ../ui/fermentableEditor.ui
    ../ui/fermentableDialog.ui
    ../ui/equipmentEditor.ui
    ../ui/hopDialog.ui
    ../ui/hopEditor.ui
    ../ui/htmlViewer.ui
    ../ui/instructionWidget.ui
    ../ui/mashEditor.ui
    ../ui/mashWizard.ui
    ../ui/miscDialog.ui
    ../ui/ogAdjuster.ui
    ../ui/optionsDialog.ui
    ../ui/primingDialog.ui
    ../ui/recipeExtrasDialog.ui
    ../ui/refractoDialog.ui
    ../ui/scaleRecipeTool.ui
    ../ui/styleEditor.ui
    ../ui/timerListDialog.ui
    ../ui/timerWidget.ui
    ../ui/yeastDialog.ui
    ../ui/yeastEditor.ui
    ../ui/mashDesigner.ui
    ../ui/pitchDialog.ui
)

# The list of files that need to be moc'd.
SET( brewtarget_MOC_HEADERS
    BeerColorWidget.h
    BrewDayWidget.h
    BtDigitWidget.h
    ConverterTool.h
    EquipmentComboBox.h
    EquipmentEditor.h
    FermentableEditor.h
    FermentableDialog.h
    FermentableTableModel.h
    FermentableTableWidget.h
    HopDialog.h
    HopEditor.h
    HopTableModel.h
    HopTableWidget.h
    InstructionWidget.h
    MainWindow.h
    MaltinessWidget.h
    MashComboBox.h
    MashDesigner.h
    MashEditor.h
    MashStepEditor.h
    MashStepTableModel.h
    MashStepTableWidget.h
    MashWizard.h
    MiscDialog.h
    MiscEditor.h
    MiscTableModel.h
    MiscTableWidget.h
    OgAdjuster.h
    OptionDialog.h
    PitchDialog.h
    PrimingDialog.h
    RecipeComboBox.h
    RecipeExtrasDialog.h
    RecipeFormatter.h
    RefractoDialog.h
    ScaleRecipeTool.h
    StyleComboBox.h
    StyleEditor.h
    TimerListDialog.h
    TimerWidget.h
    WaterTableModel.h
    WaterTableWidget.h
    YeastDialog.h
    YeastEditor.h
    YeastTableModel.h
    YeastTableWidget.h
)

# List of documentation files to be installed.
SET( brewtarget_DOCS
     ../README
     ../COPYING.GPLv3
     ../COPYING.WTFPL
     ../doc/manual/index.html
)

# List of data files to be installed.
SET( brewtarget_DATA
     ../database.xml
     ../mashs.xml
     ../options.xml
     ../recipes.xml
)

# Desktop files to install.
SET( brewtarget_DESKTOP
     ../brewtarget.desktop
)

# Icon files to install.
SET( brewtarget_ICONS
     ../icons/BrewtargetIcon.png
)

#==================================Qt Junk=====================================

# Create the moc sources from the headers.
QT4_WRAP_CPP(brewtarget_MOC_SRCS ${brewtarget_MOC_HEADERS})

# Create the ui_*.h files from the *.ui files.
QT4_WRAP_UI(brewtarget_UIS_H ${brewtarget_UIS})

# Generate the resource .cpp from the .qrc file.
QT4_ADD_RESOURCES(brewtarget_QRC_CPP "../brewtarget.qrc")

#===========================Create the binary==================================

# The executable.
ADD_EXECUTABLE( ${brewtarget_EXECUTABLE} ${brewtarget_SRCS} ${brewtarget_MOC_SRCS} ${brewtarget_QRC_CPP} ${brewtarget_UIS_H} )

# Link brewtarget against Qt libraries defined by QT_USE_FILE.
TARGET_LINK_LIBRARIES(${brewtarget_EXECUTABLE} ${QT_LIBRARIES})

#========================Construct the directories=============================

# Directories...Debian standards.
SET( EXEC_PREFIX ${CMAKE_INSTALL_PREFIX} )
SET( DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" )
SET( DATADIR ${DATAROOTDIR} )
SET( BINDIR "${EXEC_PREFIX}/bin" )
SET( DOCDIR "${DATAROOTDIR}/doc/${CMAKE_PROJECT_NAME}" )

# Actual install directories.
SET( DATAPATH "${DATADIR}/${CMAKE_PROJECT_NAME}" )
SET( TARGETPATH ${BINDIR} )
SET( DOCPATH ${DOCDIR} )

#==============================Setup the config.h==============================

# NOTE: need to replace "#define CONFIGDATADIR blah" etc. in config.in
# with "#define CONFIGDATADIR ${CONFIGDATADIR}" so that they will be replaced
# by the below corresponding values when CONFIGURE_FILE()
# is called.
SET( CONFIGDATADIR "${DATAPATH}/" )
SET( CONFIGDOCDIR "${DOCPATH}/" )

# Tell cmake where the configure file is and where
# to put the output. All variables in config.in written as "${VAR}$
# will be replaced by VAR as determined by cmake in config.h.
CONFIGURE_FILE( "${SRCDIR}/config.in" "${SRCDIR}/config.h" )

#==============================Setup install targets===========================

# Install the executable.
INSTALL( TARGETS ${brewtarget_EXECUTABLE}
         RUNTIME DESTINATION ${TARGETPATH} )

# Install the data.
INSTALL( FILES ${brewtarget_DATA}
         DESTINATION ${DATAPATH} )

# Install the documentation.
INSTALL( FILES ${brewtarget_DOCS}
         DESTINATION ${DOCPATH} )

# Install the icons.
INSTALL( FILES ${brewtarget_ICONS}
         DESTINATION "${DATAROOTDIR}/icons/${CMAKE_PROJECT_NAME}" )

INSTALL( FILES ${brewtarget_DESKTOP}
         DESTINATION "${DATAROOTDIR}/applications" )