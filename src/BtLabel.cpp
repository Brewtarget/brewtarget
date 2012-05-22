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
#include "brewtarget.h"
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
   whatAmI = lType;
   btParent = parent;

   connect(this,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(popContextMenu(const QPoint&)));

}

void BtLabel::popContextMenu(const QPoint& point)
{
   QObject* calledBy = sender();
   QWidget* widgie;
   QAction *invoked;
   QMenu* menu;
   QVariant unit,scale;

   if ( calledBy == 0 )
      return;

   widgie = qobject_cast<QWidget*>(calledBy);
   if ( widgie == 0 )
      return;

   propertyName = property("editField").toString();
   unit = Brewtarget::option(propertyName, -1, btParent, Brewtarget::UNIT);
   scale = Brewtarget::option(propertyName, -1, btParent, Brewtarget::SCALE);

   switch( whatAmI )
   {
      case COLOR:
         menu = Brewtarget::setupColorMenu(btParent,unit);
         break;
      case GRAVITY:
         menu = Brewtarget::setupGravityMenu(btParent,unit);
         break;
      case MASS:
         menu = Brewtarget::setupMassMenu(btParent,unit,scale);
         break;
      case TEMPERATURE:
         menu = Brewtarget::setupTemperatureMenu(btParent,unit);
         break;
      case VOLUME:
         menu = Brewtarget::setupVolumeMenu(btParent,unit,scale);
         break;
      default:
         return;
   }

   invoked = menu->exec(widgie->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   QWidget* pMenu = invoked->parentWidget();
   if ( pMenu == menu )
   {
      Brewtarget::setOption(propertyName, invoked->data(), btParent, Brewtarget::UNIT);
      if ( Brewtarget::hasOption(propertyName, btParent, Brewtarget::SCALE) )
         Brewtarget::setOption(propertyName, noscale, btParent, Brewtarget::SCALE);
   }
   else
      Brewtarget::setOption(propertyName, invoked->data(), btParent, Brewtarget::SCALE);

   // To make this all work, I need to set ogMin and ogMax when og is set.
   if ( propertyName == "og" )
   {
      Brewtarget::setOption("ogMin", invoked->data(),btParent, Brewtarget::UNIT);
      Brewtarget::setOption("ogMax", invoked->data(),btParent, Brewtarget::UNIT);
   }
   else if ( propertyName == "color_srm" )
   {
      Brewtarget::setOption("colorMin_srm", invoked->data(),btParent, Brewtarget::UNIT);
      Brewtarget::setOption("colorMax_srm", invoked->data(),btParent, Brewtarget::UNIT);
   }
  
   emit labelChanged(propertyName);

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
