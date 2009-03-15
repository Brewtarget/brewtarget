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
DEPENDPATH += .
INCLUDEPATH += .
RESOURCES = brewtarget.qrc

unix:!macx {
   target.path = /usr/local/bin
   data.path = /usr/local/share/brewtarget/
   doc.path = /usr/local/share/doc/brewtarget
   
   data.files = *.xml
   doc.files = README COPYING
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
    data.files = *.xml

    doc.path = brewtarget.app/Contents/Resources/en.lproj
    doc.files = README COPYING

    misc.path = brewtarget.app/Contents
    misc.files = Info.plist PkgInfo

    INSTALLS += data doc misc
}

win32 {
    #RC_FILE = win\icon.rc
    CONFIG -= console

    target.path = release

    data.path = release
    #data.files = *.xml win/brewtarget.ico README COPYING
    data.files = *.xml README COPYING

    doc.path = release/doc
    doc.files = README COPYING

    INSTALLS += target data doc 
}

# Input
HEADERS += AboutDialog.h \
           BeerColorWidget.h \
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
           hop.h \
           hoputilization.h \
           HopDialog.h \
           HopEditor.h \
           HopTableModel.h \
           HopTableWidget.h \
           MainWindow.h \
           mash.h \
           mashstep.h \
           MashStepTableModel.h \
           MashStepTableWidget.h \
           matrix.h \
           misc.h \
           MiscEditor.h \
           MiscDialog.h \
           MiscTableModel.h \
           MiscTableWidget.h \
           observable.h \
           recipe.h \
           RecipeComboBox.h \
           stringparsing.h \
           style.h \
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
           mainWindow.ui \
           miscEditor.ui \
           fermentableEditor.ui \
           fermentableDialog.ui \
           equipmentEditor.ui \
           hopDialog.ui \
           hopEditor.ui \
           miscDialog.ui \
           yeastDialog.ui \
           yeastEditor.ui
SOURCES += database.cpp \
           brewtarget.cpp \
           BeerColorWidget.cpp \
           equipment.cpp \
           EquipmentComboBox.cpp \
           EquipmentEditor.cpp \
           fermentable.cpp \
           FermentableEditor.cpp \
           FermentableDialog.cpp \
           FermentableTableModel.cpp \
           FermentableTableWidget.cpp \
           main.cpp \
           hop.cpp \
           hoputilization.cpp \
           HopDialog.cpp \
           HopEditor.cpp \
           HopTableModel.cpp \
           HopTableWidget.cpp \
           MainWindow.cpp \
           mash.cpp \
           mashstep.cpp \
           MashStepTableModel.cpp \
           MashStepTableWidget.cpp \
           matrix.cpp \
           misc.cpp \
           MiscEditor.cpp \
           MiscDialog.cpp \
           MiscTableModel.cpp \
           MiscTableWidget.cpp \
           observable.cpp \
           observer.cpp \
           recipe.cpp \
           RecipeComboBox.cpp \
           stringparsing.cpp \
           style.cpp \
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
