/*
 * RecipeExtrasWidgetPlugin.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "RecipeExtrasWidgetPlugin.h"
#include "../src/RecipeExtrasWidget.h"

RecipeExtrasWidgetPlugin::RecipeExtrasWidgetPlugin(QObject* parent)
        : QObject(parent)
{
   initialized = false;
}

void RecipeExtrasWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
   if(initialized)
      return;
   
   initialized = true;
   return;
}

bool RecipeExtrasWidgetPlugin::isInitialized() const
{
   return initialized;
}

QWidget* RecipeExtrasWidgetPlugin::createWidget(QWidget *parent)
{
   return new RecipeExtrasWidget(parent);
}

QString RecipeExtrasWidgetPlugin::name() const
{
   return "RecipeExtrasWidget";
}

QString RecipeExtrasWidgetPlugin::group() const
{
   return "Brewtarget Widgets";
}

QIcon RecipeExtrasWidgetPlugin::icon() const
{
   return QIcon();
}

QString RecipeExtrasWidgetPlugin::toolTip() const
{
   return "";
}

QString RecipeExtrasWidgetPlugin::whatsThis() const
{
   return "";
}

bool RecipeExtrasWidgetPlugin::isContainer() const
{
   return false;
}

QString RecipeExtrasWidgetPlugin::domXml() const
{
   return "<widget class=\"RecipeExtrasWidget\" name=\"recipeExtrasWidget\">\n"
          "</widget>\n";
}

QString RecipeExtrasWidgetPlugin::includeFile() const
{
   return "RecipeExtrasWidget.h";
}

Q_EXPORT_PLUGIN2( recipeextraswidgetplugin, RecipeExtrasWidgetPlugin )
