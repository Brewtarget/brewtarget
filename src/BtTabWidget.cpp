/*
 * BtTabWdiget.cpp is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2013-2017.
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

#include <QtGui>
#include "BtTabWidget.h"
#include "BtTreeView.h"
#include "BtTreeItem.h"
#include "database.h"

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
void BtTabWidget::dropEvent(QDropEvent *event)
{
   const QMimeData* mData;
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

   mData = event->mimeData();
   QByteArray itemData = mData->data(acceptMime);
   QDataStream dStream(&itemData,QIODevice::ReadOnly);

   while ( ! dStream.atEnd() )
   {
      dStream >> _type >> id >> name;
      switch( _type ) {
         case BtTreeItem::RECIPE:
            event->acceptProposedAction();
            emit setRecipe(Database::instance().recipe(id));
            return;
         case BtTreeItem::EQUIPMENT:
            event->acceptProposedAction();
            emit setEquipment(Database::instance().equipment(id));
            return;
         case BtTreeItem::STYLE:
            event->acceptProposedAction();
            emit setStyle(Database::instance().style(id));
            return;
         case BtTreeItem::FERMENTABLE:
            ferms.append( Database::instance().fermentable(id));
            break;
         case BtTreeItem::HOP:
            hops.append( Database::instance().hop(id));
            break;
         case BtTreeItem::MISC:
            miscs.append( Database::instance().misc(id));
            break;
         case BtTreeItem::YEAST:
            yeasts.append( Database::instance().yeast(id));
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

