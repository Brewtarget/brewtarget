TEMPLATE = lib
TARGET   = btdigitwidgetplugin
CONFIG   += designer plugin

HEADERS = BtDigitWidgetPlugin.h \
          ../src/BtDigitWidget.h
SOURCES = BtDigitWidgetPlugin.cpp \
          ../src/BtDigitWidget.cpp

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

