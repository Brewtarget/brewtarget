/*
 * BtTabWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "BtTabWidget.h"

#include <QtGui>

#include "BtTreeItem.h"
#include "BtTreeView.h"
//#include "database/Database.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Yeast.h"


//! \brief set up the popup window.
BtTabWidget::BtTabWidget(QWidget* parent) : QTabWidget(parent)
{
   setAcceptDrops(true);
   acceptMime = "";
}

void BtTabWidget::dragEnterEvent(QDragEnterEvent *event)
{
   if ( acceptMime.size() == 0 )
      acceptMime = property("mimeAccepted").toString();

   if (event->mimeData()->hasFormat(acceptMime) )
      event->acceptProposedAction();
}

/*
 * This is shaping up quite nicely. I just need to figure out how to handle
 * the remaining drops adn this should pretty much work as envisioned when I
 * started.
 */
void BtTabWidget::dropEvent(QDropEvent *event) {
   int _type;
   QString name;
   int id;
   QList<Fermentable*>ferms;
   QList<Hop*>hops;
   QList<Misc*>miscs;
   QList<Yeast*>yeasts;

   if ( acceptMime.size() == 0 )
      acceptMime = property("mimeAccepted").toString();

   if (! event->mimeData()->hasFormat(acceptMime) )
      return;

   const QMimeData* mData = event->mimeData();
   QByteArray itemData = mData->data(acceptMime);
   QDataStream dStream(&itemData,QIODevice::ReadOnly);

   while ( ! dStream.atEnd() )
   {
      dStream >> _type >> id >> name;
      switch( _type ) {
         case BtTreeItem::RECIPE:
            event->acceptProposedAction();
            emit setRecipe(ObjectStoreWrapper::getById<Recipe>(id).get());
            return;
         case BtTreeItem::EQUIPMENT:
            event->acceptProposedAction();
            emit setEquipment(ObjectStoreWrapper::getById<Equipment>(id).get());
            return;
         case BtTreeItem::STYLE:
            event->acceptProposedAction();
            emit setStyle(ObjectStoreWrapper::getById<Style>(id).get());
            return;
         case BtTreeItem::FERMENTABLE:
            ferms.append(ObjectStoreWrapper::getById<Fermentable>(id).get());
            break;
         case BtTreeItem::HOP:
            hops.append(ObjectStoreWrapper::getById<Hop>(id).get());
            break;
         case BtTreeItem::MISC:
            miscs.append(ObjectStoreWrapper::getById<Misc>(id).get());
            break;
         case BtTreeItem::YEAST:
            yeasts.append(ObjectStoreWrapper::getById<Yeast>(id).get());
            break;
      }
   }
   if ( ferms.size() > 0 )
      emit setFermentables(ferms);
   if ( hops.size() > 0 )
      emit setHops(hops);
   if ( miscs.size() > 0 )
      emit setMiscs(miscs);
   if ( yeasts.size() > 0 )
      emit setYeasts(yeasts);

   event->acceptProposedAction();
}
