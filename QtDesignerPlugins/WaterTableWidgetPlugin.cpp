/*
 * WaterTableWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "WaterTableWidgetPlugin.h"
#include "../WaterTableWidget.h"

WaterTableWidgetPlugin::WaterTableWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void WaterTableWidgetPlugin::initialize(QDesignerFormEditorInterface* /*core*/)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool WaterTableWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* WaterTableWidgetPlugin::createWidget(QWidget *parent)
{
   return new WaterTableWidget(parent);
}

QString WaterTableWidgetPlugin::name() const
{
   return "WaterTableWidget";
}

QString WaterTableWidgetPlugin::group() const
{
   return "BrewTarget Widgets";
}

QIcon WaterTableWidgetPlugin::icon() const
{
   return QIcon();
}

QString WaterTableWidgetPlugin::toolTip() const
{
   return "Table for WATERs.";
}

QString WaterTableWidgetPlugin::whatsThis() const
{
   return "Table for WATERs.";
}

bool WaterTableWidgetPlugin::isContainer() const
{
   return false;
}

QString WaterTableWidgetPlugin::domXml() const
{
   return "<widget class=\"WaterTableWidget\" name=\"waterTableWidget\">\n"
          "</widget>\n";
}

QString WaterTableWidgetPlugin::includeFile() const
{
   return "WaterTableWidget.h";
}

Q_EXPORT_PLUGIN2( watertablewidgetplugin, WaterTableWidgetPlugin )
