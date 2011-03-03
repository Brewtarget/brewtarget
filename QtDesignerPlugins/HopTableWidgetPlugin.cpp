/*
 * HopTableWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "HopTableWidgetPlugin.h"
#include "HopTableWidget.h"

HopTableWidgetPlugin::HopTableWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void HopTableWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool HopTableWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* HopTableWidgetPlugin::createWidget(QWidget *parent)
{
   return new HopTableWidget(parent);
}

QString HopTableWidgetPlugin::name() const
{
   return "HopTableWidget";
}

QString HopTableWidgetPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon HopTableWidgetPlugin::icon() const
{
   return QIcon();
}

QString HopTableWidgetPlugin::toolTip() const
{
   return "Table for HOP ingredients.";
}

QString HopTableWidgetPlugin::whatsThis() const
{
   return "Table for HOP ingredients.";
}

bool HopTableWidgetPlugin::isContainer() const
{
   return false;
}

QString HopTableWidgetPlugin::domXml() const
{
   return "<widget class=\"HopTableWidget\" name=\"hopTableWidget\">\n"
          "</widget>\n";
}

QString HopTableWidgetPlugin::includeFile() const
{
   return "HopTableWidget.h";
}
