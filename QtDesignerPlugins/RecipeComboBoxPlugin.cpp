/*
 * RecipeComboBoxPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QObject>
#include <QDesignerCustomWidgetInterface>
#include <QString>
#include <QWidget>
#include <QIcon>
#include <QtPlugin>

#include "RecipeComboBoxPlugin.h"
#include "../RecipeComboBox.h"

RecipeComboBoxPlugin::RecipeComboBoxPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void RecipeComboBoxPlugin::initialize(QDesignerFormEditorInterface* /*core*/)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool RecipeComboBoxPlugin::isInitialized() const
{
   return initialized;
}

QWidget* RecipeComboBoxPlugin::createWidget(QWidget *parent)
{
   return new RecipeComboBox(parent);
}

QString RecipeComboBoxPlugin::name() const
{
   return "RecipeComboBox";
}

QString RecipeComboBoxPlugin::group() const
{
   return "BrewTarget Widgets";
}

QIcon RecipeComboBoxPlugin::icon() const
{
   return QIcon();
}

QString RecipeComboBoxPlugin::toolTip() const
{
   return "Combo box that observes recipes";
}

QString RecipeComboBoxPlugin::whatsThis() const
{
   return "Combo box that observes recipes";
}

bool RecipeComboBoxPlugin::isContainer() const
{
   return false;
}

QString RecipeComboBoxPlugin::domXml() const
{
   return "<widget class=\"RecipeComboBox\" name=\"recipeComboBox\">\n"
          "</widget>\n";
}

QString RecipeComboBoxPlugin::includeFile() const
{
   return "RecipeComboBox.h";
}

Q_EXPORT_PLUGIN2( recipecomboboxplugin, RecipeComboBoxPlugin )
