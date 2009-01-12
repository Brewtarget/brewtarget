TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += equipment.h \
           fermentable.h \
           FermentableTableModel.h \
           FermentableTableWidget.h \
           hop.h \
           HopTableModel.h \
           HopTableWidget.h \
           mash.h \
           mashstep.h \
           MashStepTableModel.h \
           MashStepTableWidget.h \
           matrix.h \
           misc.h \
           miscEditor.h \
           MiscTableModel.h \
           MiscTableWidget.h \
           observable.h \
           recipe.h \
           stringparsing.h \
           style.h \
           unit.h \
           water.h \
           xml.h \
           xmlnode.h \
           xmltree.h \
           yeast.h
FORMS +=   miscEditor.ui
SOURCES += equipment.cpp \
           fermentable.cpp \
           FermentableTableModel.cpp \
           FermentableTableWidget.cpp \
           guitest.cpp \
           hop.cpp \
           HopTableModel.cpp \
           HopTableWidget.cpp \
           mash.cpp \
           mashstep.cpp \
           MashStepTableModel.cpp \
           MashStepTableWidget.cpp \
           matrix.cpp \
           misc.cpp \
           miscEditor.cpp \
           MiscTableModel.cpp \
           MiscTableWidget.cpp \
           observable.cpp \
           observer.cpp \
           recipe.cpp \
           stringparsing.cpp \
           style.cpp \
           unit.cpp \
           water.cpp \
           xml.cpp \
           xmlnode.cpp \
           xmltree.cpp \
           yeast.cpp
