TEMPLATE = lib
TARGET   = mashsteptablewidgetplugin
CONFIG   += designer plugin

HEADERS = ../MashStepTableWidget.h \
          MashStepTableWidgetPlugin.h \
          ../MashStepTableModel.h \
          ../mashstep.h \
          ../stringparsing.h \
          ../xmlnode.h \
          ../xml.h \
          ../observable.h
SOURCES = ../MashStepTableWidget.cpp \
          MashStepTableWidgetPlugin.cpp \
          ../MashStepTableModel.cpp \
          ../mashstep.cpp \
          ../stringparsing.cpp \
          ../xmlnode.cpp \
          ../xml.cpp \
          ../observable.cpp \
          ../observer.cpp

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target