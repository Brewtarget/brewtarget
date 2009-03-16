/*
 * StyleComboBoxPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include "StyleComboBoxPlugin.h"
#include "../StyleComboBox.h"

StyleComboBoxPlugin::StyleComboBoxPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void StyleComboBoxPlugin::initialize(QDesignerFormEditorInterface* /*core*/)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool StyleComboBoxPlugin::isInitialized() const
{
   return initialized;
}

QWidget* StyleComboBoxPlugin::createWidget(QWidget *parent)
{
   return new StyleComboBox(parent);
}

QString StyleComboBoxPlugin::name() const
{
   return "StyleComboBox";
}

QString StyleComboBoxPlugin::group() const
{
   return "BrewTarget Widgets";
}

QIcon StyleComboBoxPlugin::icon() const
{
   return QIcon();
}

QString StyleComboBoxPlugin::toolTip() const
{
   return "Combo box that observes styles";
}

QString StyleComboBoxPlugin::whatsThis() const
{
   return "Combo box that observes styles";
}

bool StyleComboBoxPlugin::isContainer() const
{
   return false;
}

QString StyleComboBoxPlugin::domXml() const
{
   return "<widget class=\"StyleComboBox\" name=\"styleComboBox\">\n"
          "</widget>\n";
}

QString StyleComboBoxPlugin::includeFile() const
{
   return "StyleComboBox.h";
}

Q_EXPORT_PLUGIN2( stylecomboboxplugin, StyleComboBoxPlugin )
