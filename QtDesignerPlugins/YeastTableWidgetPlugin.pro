TEMPLATE = lib
TARGET   = yeasttablewidgetplugin
CONFIG   += designer plugin

HEADERS = ../YeastTableWidget.h \
          YeastTableWidgetPlugin.h \
          ../YeastTableModel.h \
          ../yeast.h \
          ../stringparsing.h \
          ../xmlnode.h \
          ../xml.h \
          ../observable.h
SOURCES = ../YeastTableWidget.cpp \
          YeastTableWidgetPlugin.cpp \
          ../YeastTableModel.cpp \
          ../yeast.cpp \
          ../stringparsing.cpp \
          ../xmlnode.cpp \
          ../xml.cpp \
          ../observable.cpp \
          ../observer.cpp

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

