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

#include "MashStepTableWidgetPlugin.h"
#include "../MashStepTableWidget.h"

MashStepTableWidgetPlugin::MashStepTableWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void MashStepTableWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool MashStepTableWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* MashStepTableWidgetPlugin::createWidget(QWidget *parent)
{
   return new MashStepTableWidget(parent);
}

QString MashStepTableWidgetPlugin::name() const
{
   return "MashStepTableWidget";
}

QString MashStepTableWidgetPlugin::group() const
{
   return "BrewTarget Widgets";
}

QIcon MashStepTableWidgetPlugin::icon() const
{
   return QIcon();
}

QString MashStepTableWidgetPlugin::toolTip() const
{
   return "Table for MASH_STEPs.";
}

QString MashStepTableWidgetPlugin::whatsThis() const
{
   return "Table for MASH_STEPs.";
}

bool MashStepTableWidgetPlugin::isContainer() const
{
   return false;
}

QString MashStepTableWidgetPlugin::domXml() const
{
   return "<widget class=\"MashStepTableWidget\" name=\"mashStepTableWidget\">\n"
          "</widget>\n";
}

QString MashStepTableWidgetPlugin::includeFile() const
{
   return "MashStepTableWidget.h";
}

Q_EXPORT_PLUGIN2( mashsteptablewidgetplugin, MashStepTableWidgetPlugin )
