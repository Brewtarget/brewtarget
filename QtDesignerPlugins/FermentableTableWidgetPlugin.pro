TEMPLATE = lib
TARGET   = fermentabletablewidgetplugin
CONFIG   += designer plugin

HEADERS = ../FermentableTableWidget.h \
          FermentableTableWidgetPlugin.h \
          ../FermentableTableModel.h \
          ../fermentable.h \
          ../stringparsing.h \
          ../xmlnode.h \
          ../xml.h \
          ../observable.h
SOURCES = ../FermentableTableWidget.cpp \
          FermentableTableWidgetPlugin.cpp \
          ../FermentableTableModel.cpp \
          ../fermentable.cpp \
          ../stringparsing.cpp \
          ../xmlnode.cpp \
          ../xml.cpp \
          ../observable.cpp \
          ../observer.cpp

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

