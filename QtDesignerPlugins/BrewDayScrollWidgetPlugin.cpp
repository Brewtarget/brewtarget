/*
 * BrewDayScrollWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "BrewDayScrollWidgetPlugin.h"
#include "BrewDayScrollWidget.h"

BrewDayScrollWidgetPlugin::BrewDayScrollWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void BrewDayScrollWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool BrewDayScrollWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* BrewDayScrollWidgetPlugin::createWidget(QWidget *parent)
{
   return new BrewDayScrollWidget(parent);
}

QString BrewDayScrollWidgetPlugin::name() const
{
   return "BrewDayScrollWidget";
}

QString BrewDayScrollWidgetPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon BrewDayScrollWidgetPlugin::icon() const
{
   return QIcon();
}

QString BrewDayScrollWidgetPlugin::toolTip() const
{
   return "";
}

QString BrewDayScrollWidgetPlugin::whatsThis() const
{
   return "";
}

bool BrewDayScrollWidgetPlugin::isContainer() const
{
   return false;
}

QString BrewDayScrollWidgetPlugin::domXml() const
{
   return "<widget class=\"BrewDayScrollWidget\" name=\"brewDayScrollWidget\">\n"
          "</widget>\n";
}

QString BrewDayScrollWidgetPlugin::includeFile() const
{
   return "BrewDayScrollWidget.h";
}

