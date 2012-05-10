/*
 * BtLabel.cpp is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
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

#include "BtLabel.h"
#include <QSettings>
#include <QDebug>

/*! \brief Initialize the BtLabel with the parent and do some things with the type
 * \param parent - QWidget* to the parent object
 * \param lType - the type of label: none, gravity, mass or volume
 * \return the initialized widget
 * \todo Not sure if I can get the name of the widget being created.
 *       Not sure how to signal the parent to redisplay
 */
 
BtLabel::BtLabel(QWidget *parent, LabelType lType)
{
   cachedMenu = 0;
   whatAmI = lType;
   btParent = parent;

   connect(this,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(popContextMenu(const QPoint&)));

}

void BtLabel::popContextMenu(const QPoint& point)
{
   QObject* calledBy = sender();
   QSettings settings("brewtarget");
   QWidget* widgie;
   QAction *invoked;
   QVariant unit;

   if ( calledBy == 0 )
      return;

   widgie = qobject_cast<QWidget*>(calledBy);
   if ( widgie == 0 )
      return;

   propertyName = property("editField").toString();
   unit = settings.value(propertyName);


   //! If this is the first time we are called, we need to build the menu. 
   switch( whatAmI )
   {
      case VOLUME:
      case MASS:
         cachedMenu = setupMassVolumeMenu(unit);
         break;
      case GRAVITY:
         cachedMenu = setupGravityMenu(unit);
         break;
      case TEMPERATURE:
         cachedMenu = setupTemperatureMenu(unit);
         break;
      case COLOR:
         cachedMenu = setupColorMenu(unit);
         break;
      default:
         return;
   }

   invoked = cachedMenu->exec(widgie->mapToGlobal(point));
   if ( invoked == 0 )
      return;


   settings.setValue(propertyName, invoked->data());
  
   emit labelChanged(propertyName);

}

QMenu* BtLabel::setupColorMenu(QVariant unit)
{
   QMenu* menu = new QMenu(btParent);
   QAction* action = new QAction(menu);
   int tUnit;

   if ( unit.isValid() )
      tUnit = unit.toInt();
   else
      tUnit = -1;

   action->setText(tr("Default"));
   action->setData(-1);
   action->setCheckable(true);
   action->setChecked(tUnit == -1);
   menu->addAction(action);

   action->setText(tr("ECB"));
   action->setData(1);
   action->setCheckable(true);
   action->setChecked(tUnit == 1);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("SRM"));
   action->setData(0);
   action->setCheckable(true);
   action->setChecked(tUnit == 0);

   menu->addAction(action);

   return menu;
}

QMenu* BtLabel::setupGravityMenu(QVariant unit)
{
   QMenu* menu = new QMenu(btParent);
   QAction* action = new QAction(menu);
   int tUnit;

   if ( unit.isValid() )
      tUnit = unit.toInt();
   else
      tUnit = -1;

   action->setText(tr("Default"));
   action->setData(-1);
   action->setCheckable(true);
   action->setChecked(tUnit == -1);
   menu->addAction(action);

   action->setText(tr("Plato"));
   action->setData(1);
   action->setCheckable(true);
   action->setChecked(tUnit == 1);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("Specific Gravity"));
   action->setData(0);
   action->setCheckable(true);
   action->setChecked(tUnit == 0);

   menu->addAction(action);

   return menu;
}

QMenu* BtLabel::setupMassVolumeMenu(QVariant unit)
{
   QMenu* menu = new QMenu(btParent);
   QAction* action = new QAction(menu);
   int tUnit;

   if ( unit.isValid() )
      tUnit = unit.toInt();
   else
      tUnit = -1;

   action->setText(tr("Default"));
   action->setData(-1);
   action->setCheckable(true);
   action->setChecked(tUnit == -1);
   menu->addAction(action);

   action->setText(tr("SI"));
   action->setData(SI);
   action->setCheckable(true);
   action->setChecked(tUnit == SI);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("US Customary"));
   action->setData(USCustomary);
   action->setCheckable(true);
   action->setChecked(tUnit == USCustomary);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("British Imperial"));
   action->setData(Imperial);
   action->setCheckable(true);
   action->setChecked(tUnit == Imperial);
   menu->addAction(action);

   return menu;
}

QMenu* BtLabel::setupTemperatureMenu(QVariant unit)
{
   QMenu* menu = new QMenu(btParent);
   QAction* action = new QAction(menu);
   int tUnit;

   if ( unit.isValid() )
      tUnit = unit.toInt();
   else
      tUnit = -1;

   action->setText(tr("Default"));
   action->setData(-1);
   action->setCheckable(true);
   action->setChecked(tUnit == -1);
   menu->addAction(action);

   action->setText(tr("Celsius"));
   action->setData(Celsius);
   action->setCheckable(true);
   action->setChecked(tUnit == Celsius);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("Fahrenheit"));
   action->setData(Fahrenheit);
   action->setCheckable(true);
   action->setChecked(tUnit == Fahrenheit);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("Kelvin"));
   action->setData(Kelvin);
   action->setCheckable(true);
   action->setChecked(tUnit == Kelvin);
   menu->addAction(action);

   return menu;
}

BtColorLabel::BtColorLabel(QWidget *parent)
   : BtLabel(parent,COLOR)
{
}

BtVolumeLabel::BtVolumeLabel(QWidget *parent)
   : BtLabel(parent,VOLUME)
{
}

BtMassLabel::BtMassLabel(QWidget *parent)
   : BtLabel(parent,MASS)
{
}

BtGravityLabel::BtGravityLabel(QWidget *parent)
   : BtLabel(parent,GRAVITY)
{
}

BtTemperatureLabel::BtTemperatureLabel(QWidget *parent)
   : BtLabel(parent,TEMPERATURE)
{
}
