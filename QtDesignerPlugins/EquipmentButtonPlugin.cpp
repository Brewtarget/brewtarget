/*
* EquipmentButtonPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2011.
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

#include "EquipmentButtonPlugin.h"
#include "EquipmentButton.h"

EquipmentButtonPlugin::EquipmentButtonPlugin(QObject* parent)
: QObject(parent)
{
   initialized = false;
}

void EquipmentButtonPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool EquipmentButtonPlugin::isInitialized() const
{
   return initialized;
}

QWidget* EquipmentButtonPlugin::createWidget(QWidget *parent)
{
   return new EquipmentButton(parent);
}

QString EquipmentButtonPlugin::name() const
{
   return "EquipmentButton";
}

QString EquipmentButtonPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon EquipmentButtonPlugin::icon() const
{
   return QIcon();
}

QString EquipmentButtonPlugin::toolTip() const
{
   return "";
}

QString EquipmentButtonPlugin::whatsThis() const
{
   return "";
}

bool EquipmentButtonPlugin::isContainer() const
{
   return false;
}

QString EquipmentButtonPlugin::domXml() const
{
   return "<widget class=\"EquipmentButton\" name=\"equipmentButton\">\n"
   "</widget>\n";
}

QString EquipmentButtonPlugin::includeFile() const
{
   return "EquipmentButton.h";
}
