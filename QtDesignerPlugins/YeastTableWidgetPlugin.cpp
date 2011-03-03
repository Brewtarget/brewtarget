/*
 * YeastTableWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "YeastTableWidgetPlugin.h"
#include "YeastTableWidget.h"

YeastTableWidgetPlugin::YeastTableWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void YeastTableWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool YeastTableWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* YeastTableWidgetPlugin::createWidget(QWidget *parent)
{
   return new YeastTableWidget(parent);
}

QString YeastTableWidgetPlugin::name() const
{
   return "YeastTableWidget";
}

QString YeastTableWidgetPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon YeastTableWidgetPlugin::icon() const
{
   return QIcon();
}

QString YeastTableWidgetPlugin::toolTip() const
{
   return "Table for YEAST ingredients.";
}

QString YeastTableWidgetPlugin::whatsThis() const
{
   return "Table for YEAST ingredients.";
}

bool YeastTableWidgetPlugin::isContainer() const
{
   return false;
}

QString YeastTableWidgetPlugin::domXml() const
{
   return "<widget class=\"YeastTableWidget\" name=\"yeastTableWidget\">\n"
          "</widget>\n";
}

QString YeastTableWidgetPlugin::includeFile() const
{
   return "YeastTableWidget.h";
}
