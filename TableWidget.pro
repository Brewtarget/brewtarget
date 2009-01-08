TEMPLATE = app
LANGUAGE = C++
TARGET   = tabletest

CONFIG += qt warn_on release
SOURCES += tabletest.cpp \
           FermentableTableWidget.cpp \
           FermentableTableModel.cpp \
           HopTableWidget.cpp \
           HopTableModel.cpp \
           MiscTableWidget.cpp \
           MiscTableModel.cpp \
           fermentable.cpp \
           hop.cpp \
           misc.cpp \
           observable.cpp \
           observer.cpp \
           xmlnode.cpp \
           stringparsing.cpp \
           xml.cpp \
           xmltree.cpp
HEADERS += FermentableTableWidget.h \
           FermentableTableModel.h \
           HopTableWidget.h \
           HopTableModel.h \
           MiscTableWidget.h \
           MiscTableModel.h \
           fermentable.h \
           hop.h \
           misc.h \
           observable.h \
           xmlnode.h \
           stringparsing.h \
           xml.h \
           xmltree.h
DBFILE   = TableWidget.db

