/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtTabWidget.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "BtTabWidget.h"

#include <QDebug>
#include <QtGui>

#include "trees/TreeNode.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Equipment.h"
#include "model/Recipe.h"
#include "model/Style.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_BtTabWidget.cpp"


//! \brief set up the popup window.
BtTabWidget::BtTabWidget(QWidget* parent) :
   QTabWidget{parent},
   acceptMime{""} {
   this->setAcceptDrops(true);
   return;
}

void BtTabWidget::dragEnterEvent(QDragEnterEvent *event) {
   if (this->acceptMime.size() == 0) {
      this->acceptMime = property("mimeAccepted").toString();
      qDebug() << Q_FUNC_INFO << "this->acceptMime:" << this->acceptMime;
   }

   if (event->mimeData()->hasFormat(this->acceptMime) ) {
      event->acceptProposedAction();
   }
   return;
}

/*
 * This is shaping up quite nicely. I just need to figure out how to handle
 * the remaining drops adn this should pretty much work as envisioned when I
 * started.
 */
void BtTabWidget::dropEvent(QDropEvent *event) {
   qDebug() << Q_FUNC_INFO;

   if (this->acceptMime.size() == 0) {
      this->acceptMime = property("mimeAccepted").toString();
      qDebug() << Q_FUNC_INFO << "this->acceptMime:" << this->acceptMime;
   }

   if (!event->mimeData()->hasFormat(this->acceptMime)) {
      return;
   }

   QList<Fermentable*>ferms;
   QList<Hop*>hops;
   QList<Misc*>miscs;
   QList<Yeast*>yeasts;

   QMimeData const * mData = event->mimeData();
   QByteArray itemData = mData->data(this->acceptMime);

   for (QDataStream dStream(&itemData, QIODevice::ReadOnly); !dStream.atEnd(); ) {
      int itemTypeRaw;
      int id;
      QString name;
      dStream >> itemTypeRaw >> id >> name;
      qDebug() << Q_FUNC_INFO << "Item type #" << itemTypeRaw;
      TreeNode::Type nodeType{itemTypeRaw};
      switch (nodeType) {
         case TreeNode::Type::Recipe:
            event->acceptProposedAction();
            emit setRecipe(ObjectStoreWrapper::getById<Recipe>(id).get());
            return;
         case TreeNode::Type::Equipment:
            event->acceptProposedAction();
            emit setEquipment(ObjectStoreWrapper::getById<Equipment>(id).get());
            return;
         case TreeNode::Type::Style:
            event->acceptProposedAction();
            emit setStyle(ObjectStoreWrapper::getById<Style>(id).get());
            return;
         case TreeNode::Type::Fermentable:
            ferms.append(ObjectStoreWrapper::getById<Fermentable>(id).get());
            break;
         case TreeNode::Type::Hop:
            hops.append(ObjectStoreWrapper::getById<Hop>(id).get());
            break;
         case TreeNode::Type::Misc:
            miscs.append(ObjectStoreWrapper::getById<Misc>(id).get());
            break;
         case TreeNode::Type::Yeast:
            yeasts.append(ObjectStoreWrapper::getById<Yeast>(id).get());
            break;
         case TreeNode::Type::BrewNote:
         case TreeNode::Type::Folder:
         case TreeNode::Type::Water:
            // These cases shouldn't arise (I think!) but the compiler will emit a warning if we don't explicitly have
            // code to handle them (which is good!).
            qWarning() << Q_FUNC_INFO << "Unexpected item type" << itemTypeRaw;
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
   return;
}
