/*
 * EquipmentComboBoxPlugin.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "EquipmentComboBoxPlugin.h"
#include "../EquipmentComboBox.h"

EquipmentComboBoxPlugin::EquipmentComboBoxPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void EquipmentComboBoxPlugin::initialize(QDesignerFormEditorInterface* /*core*/)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool EquipmentComboBoxPlugin::isInitialized() const
{
   return initialized;
}

QWidget* EquipmentComboBoxPlugin::createWidget(QWidget *parent)
{
   return new EquipmentComboBox(parent);
}

QString EquipmentComboBoxPlugin::name() const
{
   return "EquipmentComboBox";
}

QString EquipmentComboBoxPlugin::group() const
{
   return "BrewTarget Widgets";
}

QIcon EquipmentComboBoxPlugin::icon() const
{
   return QIcon();
}

QString EquipmentComboBoxPlugin::toolTip() const
{
   return "Combo box that observes equipments";
}

QString EquipmentComboBoxPlugin::whatsThis() const
{
   return "Combo box that observes equipments";
}

bool EquipmentComboBoxPlugin::isContainer() const
{
   return false;
}

QString EquipmentComboBoxPlugin::domXml() const
{
   return "<widget class=\"EquipmentComboBox\" name=\"equipmentComboBox\">\n"
          "</widget>\n";
}

QString EquipmentComboBoxPlugin::includeFile() const
{
   return "EquipmentComboBox.h";
}

Q_EXPORT_PLUGIN2( equipmentcomboboxplugin, EquipmentComboBoxPlugin )
