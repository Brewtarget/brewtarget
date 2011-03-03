/*
 * BtDigitWidgetPlugin.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2010-2011.
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

#include "BtDigitWidgetPlugin.h"
#include "../src/BtDigitWidget.h"

BtDigitWidgetPlugin::BtDigitWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void BtDigitWidgetPlugin::initialize(QDesignerFormEditorInterface* /*core*/)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool BtDigitWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* BtDigitWidgetPlugin::createWidget(QWidget *parent)
{
   BtDigitWidget* ret = new BtDigitWidget(parent);
   ret->setLowLim(0);
   ret->setHighLim(2);
   ret->display(1.5,3);
   return ret;
}

QString BtDigitWidgetPlugin::name() const
{
   return "BtDigitWidget";
}

QString BtDigitWidgetPlugin::group() const
{
   return "BrewTarget Widgets";
}

QIcon BtDigitWidgetPlugin::icon() const
{
   return QIcon();
}

QString BtDigitWidgetPlugin::toolTip() const
{
   return "A replacement for QLDCNumber";
}

QString BtDigitWidgetPlugin::whatsThis() const
{
   return "A replacement for QLDCNumber";
}

bool BtDigitWidgetPlugin::isContainer() const
{
   return false;
}

QString BtDigitWidgetPlugin::domXml() const
{
   return "<widget class=\"BtDigitWidget\" name=\"btDigitWidget\">\n"
          "</widget>\n";
}

QString BtDigitWidgetPlugin::includeFile() const
{
   return "BtDigitWidget.h";
}

Q_EXPORT_PLUGIN2( btdigitwidgetplugin, BtDigitWidgetPlugin )
