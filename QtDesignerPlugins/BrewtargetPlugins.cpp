#include "BrewtargetPlugins.h"
#include "BrewDayScrollWidgetPlugin.h"
#include "RecipeExtrasWidgetPlugin.h"

BrewtargetPlugins::BrewtargetPlugins(QObject* parent) : QObject(parent)
{
   plugins.append(new BrewDayScrollWidgetPlugin(this));
   plugins.append(new RecipeExtrasWidgetPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> BrewtargetPlugins::customWidgets() const
{
   return plugins;
}

Q_EXPORT_PLUGIN2( brewtargetplugins, BrewtargetPlugins )

