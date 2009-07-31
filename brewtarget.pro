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
TARGET = brewtarget
CONFIG += qt release warn_on
QT += xml webkit
DEPENDPATH += .
INCLUDEPATH += .
RESOURCES = brewtarget.qrc

unix:!macx {
   target.path = /usr/bin
   data.path = /usr/share/brewtarget/
   doc.path = /usr/share/doc/brewtarget
   
   data.files = *.xml
   doc.files = README COPYING doc/manual/*
   INSTALLS += target data doc
}

macx {
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
HEADERS += AboutDialog.h \
           BeerColorWidget.h \
           BeerXMLElement.h \
           BrewDayWidget.h \
           brewtarget.h \
           config.h \
           database.h \
           equipment.h \
           EquipmentComboBox.h \
           EquipmentEditor.h \
           fermentable.h \
           FermentableEditor.h \
           FermentableDialog.h \
           FermentableTableModel.h \
           FermentableTableWidget.h \
           HeatCalculations.h \
           hop.h \
           hoputilization.h \
           HopDialog.h \
           HopEditor.h \
           HopTableModel.h \
           HopTableWidget.h \
           HtmlViewer.h \
           instruction.h \
           InstructionWidget.h \
           MainWindow.h \
           MaltinessWidget.h \
           mash.h \
           MashEditor.h \
           mashstep.h \
           MashStepEditor.h \
           MashStepTableModel.h \
           MashStepTableWidget.h \
           MashWizard.h \
           matrix.h \
           misc.h \
           MiscEditor.h \
           MiscDialog.h \
           MiscTableModel.h \
           MiscTableWidget.h \
           observable.h \
           OptionDialog.h \
           PreInstruction.h \
           recipe.h \
           RecipeComboBox.h \
           stringparsing.h \
           style.h \
           StyleComboBox.h \
           StyleEditor.h \
           TimerWidget.h \
           unit.h \
           water.h \
           WaterTableModel.h \
           WaterTableWidget.h \
           xml.h \
           xmlnode.h \
           xmltree.h \
           yeast.h \
           YeastDialog.h \
           YeastEditor.h \
           YeastTableModel.h \
           YeastTableWidget.h
FORMS +=   aboutDialog.ui \
           brewDayWidget.ui \
           mainWindow.ui \
           mashStepEditor.ui \
           miscEditor.ui \
           fermentableEditor.ui \
           fermentableDialog.ui \
           equipmentEditor.ui \
           hopDialog.ui \
           hopEditor.ui \
           htmlViewer.ui \
           instructionWidget.ui \
           mashEditor.ui \
           mashWizard.ui \
           miscDialog.ui \
           optionsDialog.ui \
           styleEditor.ui \
           timerWidget.ui \
           yeastDialog.ui \
           yeastEditor.ui
SOURCES += database.cpp \
           BeerXMLElement.cpp \
           BrewDayWidget.cpp \
           BeerColorWidget.cpp \
           brewtarget.cpp \
           equipment.cpp \
           EquipmentComboBox.cpp \
           EquipmentEditor.cpp \
           fermentable.cpp \
           FermentableEditor.cpp \
           FermentableDialog.cpp \
           FermentableTableModel.cpp \
           FermentableTableWidget.cpp \
           main.cpp \
           HeatCalculations.cpp \
           hop.cpp \
           hoputilization.cpp \
           HopDialog.cpp \
           HopEditor.cpp \
           HopTableModel.cpp \
           HopTableWidget.cpp \
           HtmlViewer.cpp \
           instruction.cpp \
           InstructionWidget.cpp \
           MainWindow.cpp \
           MaltinessWidget.cpp \
           mash.cpp \
           MashEditor.cpp \
           mashstep.cpp \
           MashStepEditor.cpp \
           MashStepTableModel.cpp \
           MashStepTableWidget.cpp \
           MashWizard.cpp \
           matrix.cpp \
           misc.cpp \
           MiscEditor.cpp \
           MiscDialog.cpp \
           MiscTableModel.cpp \
           MiscTableWidget.cpp \
           observable.cpp \
           observer.cpp \
           OptionDialog.cpp \
           PreInstruction.cpp \
           recipe.cpp \
           RecipeComboBox.cpp \
           stringparsing.cpp \
           style.cpp \
           StyleComboBox.cpp \
           StyleEditor.cpp \
           TimerWidget.cpp \
           unit.cpp \
           water.cpp \
           WaterTableModel.cpp \
           WaterTableWidget.cpp \
           xml.cpp \
           xmlnode.cpp \
           xmltree.cpp \
           yeast.cpp \
           YeastDialog.cpp \
           YeastEditor.cpp \
           YeastTableModel.cpp \
           YeastTableWidget.cpp
