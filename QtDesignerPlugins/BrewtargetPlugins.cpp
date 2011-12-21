/*
 * BrewtargetPlugins.h is part of Brewtarget, and is Copyright Philip G. Lee
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


#include "BrewtargetPlugins.h"
#include "BeerColorWidgetPlugin.h"
#include "BrewDayScrollWidgetPlugin.h"
#include "RecipeExtrasWidgetPlugin.h"

BrewtargetPlugins::BrewtargetPlugins(QObject* parent) : QObject(parent)
{
   plugins.append(new BeerColorWidgetPlugin(this));
   plugins.append(new BrewDayScrollWidgetPlugin(this));
   plugins.append(new RecipeExtrasWidgetPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> BrewtargetPlugins::customWidgets() const
{
   return plugins;
}

Q_EXPORT_PLUGIN2( brewtargetplugins, BrewtargetPlugins )
