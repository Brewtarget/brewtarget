TEMPLATE = app
LANGUAGE = C++
TARGET   = tabletest

CONFIG += qt warn_on release
SOURCES += FermentableTableWidget.cpp \
           tabletest.cpp \
           FermentableTableModel.cpp \
           fermentable.cpp \
           observable.cpp \
           observer.cpp \
           xmlnode.cpp \
           stringparsing.cpp \
           xml.cpp \
           xmltree.cpp
HEADERS += FermentableTableWidget.h \
           FermentableTableModel.h \
           fermentable.h \
           observable.h \
           xmlnode.h \
           stringparsing.h \
           xml.h \
           xmltree.h
DBFILE   = FermentableTableWidget.db

