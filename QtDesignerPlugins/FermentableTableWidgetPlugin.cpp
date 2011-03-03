/*
 * FermentableTableWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "FermentableTableWidgetPlugin.h"
#include "FermentableTableWidget.h"

FermentableTableWidgetPlugin::FermentableTableWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void FermentableTableWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool FermentableTableWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* FermentableTableWidgetPlugin::createWidget(QWidget *parent)
{
   return new FermentableTableWidget(parent);
}

QString FermentableTableWidgetPlugin::name() const
{
   return "FermentableTableWidget";
}

QString FermentableTableWidgetPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon FermentableTableWidgetPlugin::icon() const
{
   return QIcon();
}

QString FermentableTableWidgetPlugin::toolTip() const
{
   return "Table for FERMENTABLE ingredients.";
}

QString FermentableTableWidgetPlugin::whatsThis() const
{
   return "Table for FERMENTABLE ingredients.";
}

bool FermentableTableWidgetPlugin::isContainer() const
{
   return false;
}

QString FermentableTableWidgetPlugin::domXml() const
{
   return "<widget class=\"FermentableTableWidget\" name=\"fermentableTableWidget\">\n"
          "</widget>\n";
}

QString FermentableTableWidgetPlugin::includeFile() const
{
   return "FermentableTableWidget.h";
}
