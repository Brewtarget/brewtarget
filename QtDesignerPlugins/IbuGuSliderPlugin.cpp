/*
 * IbuGuSliderPlugin.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "IbuGuSliderPlugin.h"
#include "../src/IbuGuSlider.h"

IbuGuSliderPlugin::IbuGuSliderPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void IbuGuSliderPlugin::initialize(QDesignerFormEditorInterface* /*core*/)
{
   if(initialized)
      return;

   initialized = true;
   return;
}

bool IbuGuSliderPlugin::isInitialized() const
{
   return initialized;
}

QWidget* IbuGuSliderPlugin::createWidget(QWidget *parent)
{
   IbuGuSlider* ret = new IbuGuSlider(parent);
   return ret;
}

QString IbuGuSliderPlugin::name() const
{
   return "IbuGuSlider";
}

QString IbuGuSliderPlugin::group() const
{
   return "BrewTarget Widgets";
}

QIcon IbuGuSliderPlugin::icon() const
{
   return QIcon();
}

QString IbuGuSliderPlugin::toolTip() const
{
   return "Slider for IBU/GU ratio.";
}

QString IbuGuSliderPlugin::whatsThis() const
{
   return "Slider for IBU/GU ratio.";
}

bool IbuGuSliderPlugin::isContainer() const
{
   return false;
}

QString IbuGuSliderPlugin::domXml() const
{
   return "<widget class=\"IbuGuSlider\" name=\"ibuGuSlider\">\n"
          "</widget>\n";
}

QString IbuGuSliderPlugin::includeFile() const
{
   return "IbuGuSlider.h";
}
