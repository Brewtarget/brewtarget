/*
* CustomComboBoxPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "CustomComboBoxPlugin.h"
#include "CustomComboBox.h"

CustomComboBoxPlugin::CustomComboBoxPlugin(QObject* parent)
: QObject(parent)
{
   initialized = false;
}

void CustomComboBoxPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool CustomComboBoxPlugin::isInitialized() const
{
   return initialized;
}

QWidget* CustomComboBoxPlugin::createWidget(QWidget *parent)
{
   return new CustomComboBox(parent);
}

QString CustomComboBoxPlugin::name() const
{
   return "CustomComboBox";
}

QString CustomComboBoxPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon CustomComboBoxPlugin::icon() const
{
   return QIcon();
}

QString CustomComboBoxPlugin::toolTip() const
{
   return "";
}

QString CustomComboBoxPlugin::whatsThis() const
{
   return "";
}

bool CustomComboBoxPlugin::isContainer() const
{
   return false;
}

QString CustomComboBoxPlugin::domXml() const
{
   return "<widget class=\"CustomComboBox\" name=\"customComboBox\">\n"
   "</widget>\n";
}

QString CustomComboBoxPlugin::includeFile() const
{
   return "CustomComboBox.h";
}
