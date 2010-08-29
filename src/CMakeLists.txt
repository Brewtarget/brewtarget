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
# In other words, classes that have Q_OBJECT declarations and stuff.
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
     ../COPYRIGHT
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
     ../images/BrewtargetIcon.png
)

#==================================Qt Junk=====================================

# Create the moc sources from the headers.
QT4_WRAP_CPP(brewtarget_MOC_SRCS ${brewtarget_MOC_HEADERS})

# Create the ui_*.h files from the *.ui files.
QT4_WRAP_UI(brewtarget_UIS_H ${brewtarget_UIS})

# Generate the resource .cpp from the .qrc file.
QT4_ADD_RESOURCES(brewtarget_QRC_CPP "../brewtarget.qrc")

#===============================Packaging======================================

# NOTE: For now, this only calls CPack when we are unix. Need to add support
# for apple bundles and microsoft NSIS installer.
IF(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake" AND UNIX AND NOT APPLE)
  MESSAGE(STATUS "CMakeRoot: ${CMAKE_ROOT}/Modules/CPack.cmake")

  SET(CPACK_SET_DESTDIR "on")
  SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
  SET(CPACK_GENERATOR "DEB;RPM;TBZ2;")
  SET(CPACK_SOURCE_GENERATOR "TBZ2;")
  SET(CPACK_SOURCE_IGNORE_FILES 
      "/.svn/"
      "~$"
      "/CMakeFiles/"
      "/_CPack_Packages/"
      "brewtarget.*deb"
      "brewtarget.*rpm"
      "brewtarget.*tar.*"
      "CPack.*"
      "Makefile"
      "cmake_install.*"
      "\\\\.o"
      "/brewtarget.dir/"
      "moc_.*"
      "qrc_brewtarget.*"
      "ui_.*h"
      "install_manifest.*"
      "config\\\\.h"
  )

  SET(CPACK_PACKAGE_VENDOR  "Philip G. Lee")
  SET(CPACK_PACKAGE_CONTACT "Philip G. Lee <rocketman768@gmail.com>")
#	SET(CPACK_RESOURCE_FILE_LICENSE		${CMAKE_CURRENT_SOURCE_DIR}//COPYING)
  SET(CPACK_PACKAGE_VERSION_MAJOR       ${brewtarget_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR       ${brewtarget_VERSION_MINOR})	# to create a debian confirm file-version-string
  SET(CPACK_PACKAGE_FILE_NAME           "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_${CMAKE_SYSTEM_PROCESSOR}")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME    "${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_source")

  SET( CPACK_PACKAGE_EXECUTABLES "${brewtarget_EXECUTABLE}" "Brewtarget" )
  #SET(CPACK_STRIP_FILES TRUE)
  SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)

  #-----------------DEB----------------
  SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "Philip G. Lee <rocketman768@gmail.com>" )
  SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "GUI beer brewing software" )
  SET( CPACK_PACKAGE_DESCRIPTION "Brewtarget is a calculator for brewing beer. It is a Qt-based program which
       allows you to create recipes from a database of ingredients. It calculates all
       the important parameters, helps you with mash temperatures, and just makes the
       process of recipe formulation much easier." )

  # NOTE: Use the getdependencies script to get the dependencies!
  SET( CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.6-6~), libc6 (>= 2.4), libgcc1 (>= 1:4.1.1), libphonon4 (>= 4:4.2.0), libqt4-dbus (>= 4:4.5.3), libqt4-svg (>= 4:4.5.3), libqt4-webkit (>= 4:4.5.3), libqt4-xml (>= 4:4.5.3), libqtcore4 (>= 4:4.6.1), libqtgui4 (>= 4:4.5.3), libstdc++6 (>= 4.1.1), libx11-6, phonon (>= 4:4.5.2)
" )
  SET( CPACK_DEBIAN_PACKAGE_SECTION "misc" )
  SET( CPACK_DEBIAN_PACKAGE_VERSION "${brewtarget_VERSION_STRING}-1")
  SET( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )

  FIND_PROGRAM(DPKG_CMD dpkg)
  IF(DPKG_CMD)
    EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
      OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET(CPACK_DEBIAN_PACKAGE_NAME ${CMAKE_PROJECT_NAME}_${brewtarget_VERSION_STRING}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
  ENDIF(DPKG_CMD)

  INCLUDE(CPack)
ENDIF()

#===========================Create the binary==================================

# The executable.
IF( APPLE )
   ADD_EXECUTABLE( ${brewtarget_EXECUTABLE} MACOSX_BUNDLE
                   ${brewtarget_SRCS} ${brewtarget_MOC_SRCS} ${brewtarget_QRC_CPP} ${brewtarget_UIS_H}
                   # Do I need to include this since I want them to go in the bundle?
                   ${brewtarget_DOCS} ${brewtarget_DATA}
   )

   # Want the docs in the en.lproj folder.
   SET_SOURCE_FILES_PROPERTIES(
      ${brewtarget_DOCS}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources/en.lproj
   )

   # Want the data in the resources folder.
   SET_SOURCE_FILES_PROPERTIES(
      ${brewtarget_DATA}
      PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources
   )

   SET( MACOSX_BUNDLE_INFO_STRING "Info string" )
   SET( MACOSX_BUNDLE_ICON_FILE "../mac/BrewtargetIcon.icns" )
   SET( MACOSX_BUNDLE_GUI_IDENTIFIER "Bundle gui identifier" )
   SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}.${brewtarget_VERSION_PATCH}" )
   SET( MACOSX_BUNDLE_BUNDLE_NAME "Brewtarget" )
   SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}" )
   SET( MACOSX_BUNDLE_BUNDLE_VERSION "${brewtarget_VERSION_MAJOR}.${brewtarget_VERSION_MINOR}.${brewtarget_VERSION_PATCH}" )
   SET( MACOSX_BUNDLE_COPYRIGHT "Brewtarget is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Brewtarget is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with brewtarget;if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA, or visit
    http://www.gnu.org/copyleft/gpl.html"
   )
ELSE()
   ADD_EXECUTABLE( ${brewtarget_EXECUTABLE} ${brewtarget_SRCS} ${brewtarget_MOC_SRCS} ${brewtarget_QRC_CPP} ${brewtarget_UIS_H} )
ENDIF()

# Link brewtarget against Qt libraries defined by QT_USE_FILE.
TARGET_LINK_LIBRARIES(${brewtarget_EXECUTABLE} ${QT_LIBRARIES})

#========================Construct the directories=============================

# Debian standard directories.
SET( EXEC_PREFIX ${CMAKE_INSTALL_PREFIX} )
SET( DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share" )
SET( DATADIR ${DATAROOTDIR} )
SET( BINDIR "${EXEC_PREFIX}/bin" )
SET( DOCDIR "${DATAROOTDIR}/doc/${CMAKE_PROJECT_NAME}" )

# Actual install directories.
IF( UNIX AND NOT APPLE )
  SET( DATAPATH "${DATADIR}/${CMAKE_PROJECT_NAME}" )
  SET( TARGETPATH ${BINDIR} )
  SET( DOCPATH ${DOCDIR} )
ELSEIF( WIN32 )
  SET( DATAPATH "${CMAKE_INSTALL_PREFIX}/release" )
  SET( TARGETPATH "${CMAKE_INSTALL_PREFIX}/release" )
  SET( DOCPATH "${CMAKE_INSTALL_PREFIX}/release/doc" )
ELSEIF( APPLE )
  # No directories to configure here I think.
ENDIF()

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

IF(APPLE)

   # I don't think we need to do any install targets here.

ELSE()

   # Install the executable.
   INSTALL( TARGETS ${brewtarget_EXECUTABLE}
            RUNTIME DESTINATION ${TARGETPATH} )

   # Install the data.
   INSTALL( FILES ${brewtarget_DATA}
            DESTINATION ${DATAPATH} )

   # Install the documentation.
   INSTALL( FILES ${brewtarget_DOCS}
            DESTINATION ${DOCPATH} )

ENDIF()

# Some extra targets for unix.
IF( UNIX AND NOT APPLE )

   # Install the icons.
   INSTALL( FILES ${brewtarget_ICONS}
            DESTINATION "${DATAROOTDIR}/icons/${CMAKE_PROJECT_NAME}" )

   # Install the .desktop file.
   INSTALL( FILES ${brewtarget_DESKTOP}
            DESTINATION "${DATAROOTDIR}/applications" )

ENDIF()
