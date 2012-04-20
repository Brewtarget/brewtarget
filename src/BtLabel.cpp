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
//   QSettings settings("brewtarget");

   cachedMenu = 0;
   whatAmI = lType;
   btParent = parent;

   connect(this,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(popContextMenu(const QPoint&)));

   if ( (property("editField")).isValid() )
   {
      qDebug() << "hmmm";
   }
//   if ( settings.contains(propertyName) )
//      selected = LabelType.valueToKey(settings.value(propertyName).toInt());

}

void BtLabel::popContextMenu(const QPoint& point)
{
   QObject* calledBy = sender();
   QWidget* widgie;
   QAction *invoked;

   if ( calledBy == 0 )
   {
      qDebug() << "No caller";
      return;
   }

   widgie = qobject_cast<QWidget*>(calledBy);
   if ( widgie == 0 )
   {
      qDebug() << "No widgie";
      return;
   }

   propertyName = property("editField").toString();
   qDebug() << "initializing" << propertyName;

   //! If this is the first time we are called, we need to build the menu. 
   if ( cachedMenu == 0 )
   {
      if ( (whatAmI == VOLUME) || (whatAmI == MASS) )
         cachedMenu = setupMassVolumeMenu();
      else if ( whatAmI == GRAVITY )
         cachedMenu = setupGravityMenu();
      else
         return;
   }
   invoked = cachedMenu->exec(widgie->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   QSettings settings("brewtarget");
   settings.setValue(propertyName, invoked->data());
  
   emit labelChanged(propertyName);

}

QMenu* BtLabel::setupGravityMenu()
{
   QMenu* menu = new QMenu(btParent);

   QAction* action = new QAction(menu);
   action->setText(tr("Plato"));
   action->setData(SI);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("Specific Gravity"));
   action->setData(USCustomary);
   menu->addAction(action);

   return menu;
}

QMenu* BtLabel::setupMassVolumeMenu()
{
   QMenu* menu = new QMenu(btParent);

   QAction* action = new QAction(menu);
   action->setText(tr("SI"));
   action->setData(SI);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("US Customary"));
   action->setData(USCustomary);
   menu->addAction(action);

   action = new QAction(menu);
   action->setText(tr("British Imperial"));
   action->setData(Imperial);
   menu->addAction(action);

   return menu;
}

void BtLabel::setSI(){ return; }
void BtLabel::setUsTraditional(){ return; }
void BtLabel::setBritishImperial(){ return; }
void BtLabel::setPlato(){ return; }
void BtLabel::setSg(){ return; }

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
