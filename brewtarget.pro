# brewtarget.pro is part of Brewtarget, and is Copyright Philip G. Lee
# (rocketman768@gmail.com), 2009.
#
# Brewtarget is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Brewtarget is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TEMPLATE = app
CONFIG += qt release warn_on
QT += xml webkit
DEPENDPATH += .
INCLUDEPATH += .
RESOURCES = brewtarget.qrc

# Where binary goes.
DESTDIR =

# Where objects go.
OBJECTS_DIR =

# Where uic output goes.
UI_DIR = src

# Where moc output goes.
MOC_DIR = src

unix : !macx {
        QT += phonon
        TARGET = brewtarget
        target.path = /usr/bin
        data.path = /usr/share/brewtarget
        doc.path = /usr/share/doc/brewtarget

        data.files = *.xml
        doc.files = README COPYING doc/manual/*
        INSTALLS += target data doc
    }

macx {
    QT += phonon
    TARGET = Brewtarget
    CONFIG += x86
    CONFIG += ppc
    #QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
    #QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk

    #DEFINES += HAVE_ROUND
    #LIBS += -dead-strip
    #QMAKE_POST_LINK=strip brewtarget.app/Contents/MacOS/brewtarget

    # Install everything into an app bundle.
    data.path = brewtarget.app/Contents/Resources
    data.files = *.xml mac/BrewtargetIcon.icns

    doc.path = brewtarget.app/Contents/Resources/en.lproj
    doc.files = README COPYING doc/manual/*

    misc.path = brewtarget.app/Contents
    misc.files = mac/Info.plist mac/PkgInfo

    INSTALLS += data doc misc
}

win32 {
    TARGET = brewtarget
    RC_FILE = win\icon.rc
    CONFIG -= console

    target.path = release

    data.path = release
    #data.files = *.xml win/brewtarget.ico README COPYING
    data.files = *.xml README COPYING

    doc.path = release/doc
    doc.files = README COPYING doc/manual/*

    INSTALLS += target data doc 
}

# Input
TRANSLATIONS += bt_en.ts \
                bt_fr.ts \
                bt_es.ts

HEADERS += src/AboutDialog.h \
           src/Algorithms.h \
           src/BeerColorWidget.h \
           src/BeerXMLElement.h \
           src/BrewDayWidget.h \
           src/brewtarget.h \
           src/config.h \
           src/ConverterTool.h \
           src/ColorMethods.h \
           src/database.h \
           src/equipment.h \
           src/EquipmentComboBox.h \
           src/EquipmentEditor.h \
           src/fermentable.h \
           src/FermentableEditor.h \
           src/FermentableDialog.h \
           src/FermentableTableModel.h \
           src/FermentableTableWidget.h \
           src/HeatCalculations.h \
           src/hop.h \
           src/HopDialog.h \
           src/HopEditor.h \
           src/HopTableModel.h \
           src/HopTableWidget.h \
           src/HtmlViewer.h \
           src/instruction.h \
           src/IbuMethods.h \
           src/InstructionWidget.h \
           src/MainWindow.h \
           src/MaltinessWidget.h \
           src/mash.h \
           src/MashComboBox.h \
           src/MashEditor.h \
           src/mashstep.h \
           src/MashStepEditor.h \
           src/MashStepTableModel.h \
           src/MashStepTableWidget.h \
           src/MashWizard.h \
           src/matrix.h \
           src/misc.h \
           src/MiscEditor.h \
           src/MiscDialog.h \
           src/MiscTableModel.h \
           src/MiscTableWidget.h \
           src/observable.h \
           src/OgAdjuster.h \
           src/OptionDialog.h \
           src/PreInstruction.h \
           src/PrimingDialog.h \
           src/recipe.h \
           src/RecipeComboBox.h \
	        src/RecipeFormatter.h \
	        src/ScaleRecipeTool.h \
           src/stringparsing.h \
           src/style.h \
           src/StyleComboBox.h \
           src/StyleEditor.h \
           src/TimerListDialog.h \
           src/TimerWidget.h \
           src/unit.h \
           src/water.h \
           src/WaterTableModel.h \
           src/WaterTableWidget.h \
           src/xml.h \
           src/xmlnode.h \
           src/xmltree.h \
           src/yeast.h \
           src/YeastDialog.h \
           src/YeastEditor.h \
           src/YeastTableModel.h \
           src/YeastTableWidget.h
FORMS +=   ui/aboutDialog.ui \
           ui/brewDayWidget.ui \
           ui/converterTool.ui \
           ui/mainWindow.ui \
           ui/mashStepEditor.ui \
           ui/miscEditor.ui \
           ui/fermentableEditor.ui \
           ui/fermentableDialog.ui \
           ui/equipmentEditor.ui \
           ui/hopDialog.ui \
           ui/hopEditor.ui \
           ui/htmlViewer.ui \
           ui/instructionWidget.ui \
           ui/mashEditor.ui \
           ui/mashWizard.ui \
           ui/miscDialog.ui \
           ui/ogAdjuster.ui \
           ui/optionsDialog.ui \
           ui/primingDialog.ui \
	   ui/scaleRecipeTool.ui \
           ui/styleEditor.ui \
           ui/timerListDialog.ui \
           ui/timerWidget.ui \
           ui/yeastDialog.ui \
           ui/yeastEditor.ui
SOURCES += src/Algorithms.cpp \
           src/database.cpp \
           src/BeerXMLElement.cpp \
           src/BrewDayWidget.cpp \
           src/BeerColorWidget.cpp \
           src/brewtarget.cpp \
           src/ColorMethods.cpp \
           src/ConverterTool.cpp \
           src/equipment.cpp \
           src/EquipmentComboBox.cpp \
           src/EquipmentEditor.cpp \
           src/fermentable.cpp \
           src/FermentableEditor.cpp \
           src/FermentableDialog.cpp \
           src/FermentableTableModel.cpp \
           src/FermentableTableWidget.cpp \
           src/main.cpp \
           src/HeatCalculations.cpp \
           src/hop.cpp \
           src/HopDialog.cpp \
           src/HopEditor.cpp \
           src/HopTableModel.cpp \
           src/HopTableWidget.cpp \
           src/HtmlViewer.cpp \
           src/instruction.cpp \
           src/IbuMethods.cpp \
           src/InstructionWidget.cpp \
           src/MainWindow.cpp \
           src/MaltinessWidget.cpp \
           src/mash.cpp \
           src/MashComboBox.cpp \
           src/MashEditor.cpp \
           src/mashstep.cpp \
           src/MashStepEditor.cpp \
           src/MashStepTableModel.cpp \
           src/MashStepTableWidget.cpp \
           src/MashWizard.cpp \
           src/matrix.cpp \
           src/misc.cpp \
           src/MiscEditor.cpp \
           src/MiscDialog.cpp \
           src/MiscTableModel.cpp \
           src/MiscTableWidget.cpp \
           src/observable.cpp \
           src/observer.cpp \
           src/OgAdjuster.cpp \
           src/OptionDialog.cpp \
           src/PreInstruction.cpp \
           src/PrimingDialog.cpp \
           src/recipe.cpp \
           src/RecipeComboBox.cpp \
	        src/RecipeFormatter.cpp \
	        src/ScaleRecipeTool.cpp \
           src/stringparsing.cpp \
           src/style.cpp \
           src/StyleComboBox.cpp \
           src/StyleEditor.cpp \
           src/TimerListDialog.cpp \
           src/TimerWidget.cpp \
           src/unit.cpp \
           src/water.cpp \
           src/WaterTableModel.cpp \
           src/WaterTableWidget.cpp \
           src/xml.cpp \
           src/xmlnode.cpp \
           src/xmltree.cpp \
           src/yeast.cpp \
           src/YeastDialog.cpp \
           src/YeastEditor.cpp \
           src/YeastTableModel.cpp \
           src/YeastTableWidget.cpp
