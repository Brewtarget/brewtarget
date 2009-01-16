TEMPLATE = lib
TARGET   = recipecomboboxplugin
CONFIG   += designer plugin

HEADERS = ../RecipeComboBox.h \
          RecipeComboBoxPlugin.h \
          ../recipe.h \
          ../stringparsing.h \
          ../xmlnode.h \
          ../xml.h \
          ../observable.h \
          ../fermentable.h \
          ../yeast.h \
          ../style.h \
          ../hop.h \
          ../misc.h \
          ../water.h \
          ../mash.h \
          ../equipment.h \
          ../mashstep.h
SOURCES = ../RecipeComboBox.cpp \
          RecipeComboBoxPlugin.cpp \
          ../recipe.cpp \
          ../stringparsing.cpp \
          ../xmlnode.cpp \
          ../xml.cpp \
          ../observable.cpp \
          ../observer.cpp \
          ../fermentable.cpp \
          ../yeast.cpp \
          ../style.cpp \
          ../hop.cpp \
          ../misc.cpp \
          ../water.cpp \
          ../mash.cpp \
          ../equipment.cpp \
          ../mashstep.cpp

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

