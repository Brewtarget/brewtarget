/*
 * BeerColorWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "BeerColorWidgetPlugin.h"
#include "BeerColorWidget.h"

BeerColorWidgetPlugin::BeerColorWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void BeerColorWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool BeerColorWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* BeerColorWidgetPlugin::createWidget(QWidget *parent)
{
   return new BeerColorWidget(parent);
}

QString BeerColorWidgetPlugin::name() const
{
   return "BeerColorWidget";
}

QString BeerColorWidgetPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon BeerColorWidgetPlugin::icon() const
{
   return QIcon();
}

QString BeerColorWidgetPlugin::toolTip() const
{
   return "";
}

QString BeerColorWidgetPlugin::whatsThis() const
{
   return "";
}

bool BeerColorWidgetPlugin::isContainer() const
{
   return false;
}

QString BeerColorWidgetPlugin::domXml() const
{
   return "<widget class=\"BeerColorWidget\" name=\"beerColorWidget\">\n"
          "</widget>\n";
}

QString BeerColorWidgetPlugin::includeFile() const
{
   return "BeerColorWidget.h";
}

