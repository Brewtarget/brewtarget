/*
 * MiscTableWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "MiscTableWidgetPlugin.h"
#include "MiscTableWidget.h"

MiscTableWidgetPlugin::MiscTableWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void MiscTableWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool MiscTableWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* MiscTableWidgetPlugin::createWidget(QWidget *parent)
{
   return new MiscTableWidget(parent);
}

QString MiscTableWidgetPlugin::name() const
{
   return "MiscTableWidget";
}

QString MiscTableWidgetPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon MiscTableWidgetPlugin::icon() const
{
   return QIcon();
}

QString MiscTableWidgetPlugin::toolTip() const
{
   return "Table for MISC ingredients.";
}

QString MiscTableWidgetPlugin::whatsThis() const
{
   return "Table for MISC ingredients.";
}

bool MiscTableWidgetPlugin::isContainer() const
{
   return false;
}

QString MiscTableWidgetPlugin::domXml() const
{
   return "<widget class=\"MiscTableWidget\" name=\"miscTableWidget\">\n"
          "</widget>\n";
}

QString MiscTableWidgetPlugin::includeFile() const
{
   return "MiscTableWidget.h";
}
