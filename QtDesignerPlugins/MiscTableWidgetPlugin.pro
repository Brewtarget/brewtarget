TEMPLATE = lib
TARGET   = misctablewidgetplugin
CONFIG   += designer plugin

HEADERS = ../MiscTableWidget.h \
          MiscTableWidgetPlugin.h \
          ../MiscTableModel.h \
          ../misc.h \
          ../stringparsing.h \
          ../xmlnode.h \
          ../xml.h \
          ../observable.h
SOURCES = ../MiscTableWidget.cpp \
          MiscTableWidgetPlugin.cpp \
          ../MiscTableModel.cpp \
          ../misc.cpp \
          ../stringparsing.cpp \
          ../xmlnode.cpp \
          ../xml.cpp \
          ../observable.cpp \
          ../observer.cpp

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

