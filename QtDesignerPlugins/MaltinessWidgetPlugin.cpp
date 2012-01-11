/*
* MaltinessWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "MaltinessWidgetPlugin.h"
#include "MaltinessWidget.h"

MaltinessWidgetPlugin::MaltinessWidgetPlugin(QObject* parent)
: QObject(parent)
{
   initialized = false;
}

void MaltinessWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool MaltinessWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* MaltinessWidgetPlugin::createWidget(QWidget *parent)
{
   return new MaltinessWidget(parent);
}

QString MaltinessWidgetPlugin::name() const
{
   return "MaltinessWidget";
}

QString MaltinessWidgetPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon MaltinessWidgetPlugin::icon() const
{
   return QIcon();
}

QString MaltinessWidgetPlugin::toolTip() const
{
   return "";
}

QString MaltinessWidgetPlugin::whatsThis() const
{
   return "";
}

bool MaltinessWidgetPlugin::isContainer() const
{
   return false;
}

QString MaltinessWidgetPlugin::domXml() const
{
   return "<widget class=\"MaltinessWidget\" name=\"maltinessWidget\">\n"
   "</widget>\n";
}

QString MaltinessWidgetPlugin::includeFile() const
{
   return "MaltinessWidget.h";
}
