/*
 * StyleRangeWidgetPlugin.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#include "StyleRangeWidgetPlugin.h"
#include "../src/StyleRangeWidget.h"

StyleRangeWidgetPlugin::StyleRangeWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void StyleRangeWidgetPlugin::initialize(QDesignerFormEditorInterface* /*core*/)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool StyleRangeWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* StyleRangeWidgetPlugin::createWidget(QWidget *parent)
{
   StyleRangeWidget* ret = new StyleRangeWidget(parent);
   return ret;
}

QString StyleRangeWidgetPlugin::name() const
{
   return "StyleRangeWidget";
}

QString StyleRangeWidgetPlugin::group() const
{
   return "BrewTarget Widgets";
}

QIcon StyleRangeWidgetPlugin::icon() const
{
   return QIcon();
}

QString StyleRangeWidgetPlugin::toolTip() const
{
   return "Shows recipe stats with style context.";
}

QString StyleRangeWidgetPlugin::whatsThis() const
{
   return "Shows recipe stats with style context.";
}

bool StyleRangeWidgetPlugin::isContainer() const
{
   return false;
}

QString StyleRangeWidgetPlugin::domXml() const
{
   return "<widget class=\"StyleRangeWidget\" name=\"styleRangeWidget\">\n"
          "</widget>\n";
}

QString StyleRangeWidgetPlugin::includeFile() const
{
   return "StyleRangeWidget.h";
}

Q_EXPORT_PLUGIN2( stylerangewidgetplugin, StyleRangeWidgetPlugin )
