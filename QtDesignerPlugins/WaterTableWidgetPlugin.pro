TEMPLATE = lib
TARGET   = watertablewidgetplugin
CONFIG   += designer plugin

HEADERS = ../WaterTableWidget.h \
          WaterTableWidgetPlugin.h \
          ../WaterTableModel.h \
          ../water.h \
          ../stringparsing.h \
          ../xmlnode.h \
          ../xml.h \
          ../observable.h
SOURCES = ../WaterTableWidget.cpp \
          WaterTableWidgetPlugin.cpp \
          ../WaterTableModel.cpp \
          ../water.cpp \
          ../stringparsing.cpp \
          ../xmlnode.cpp \
          ../xml.cpp \
          ../observable.cpp \
          ../observer.cpp

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

