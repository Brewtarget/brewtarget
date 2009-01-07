TEMPLATE = lib
TARGET   = hoptablewidgetplugin
CONFIG   += designer plugin

HEADERS = ../HopTableWidget.h \
          HopTableWidgetPlugin.h \
          ../HopTableModel.h \
          ../hop.h \
          ../stringparsing.h \
          ../xmlnode.h \
          ../xml.h \
          ../observable.h
SOURCES = ../HopTableWidget.cpp \
          HopTableWidgetPlugin.cpp \
          ../HopTableModel.cpp \
          ../hop.cpp \
          ../stringparsing.cpp \
          ../xmlnode.cpp \
          ../xml.cpp \
          ../observable.cpp \
          ../observer.cpp

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

