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

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BtTabWidget.cpp"
#endif


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

/**
 * \brief Emits the appropriate signal if the item being dragged and dropped is a match
 */
template<class NE, void (BtTabWidget::*signalSender)(NE *)>
bool BtTabWidget::acceptEventIfMatch(QString const & itemClassName, int const id, QDropEvent & event) {
   // This is the recommended way to compare QString and char const *, per https://wiki.qt.io/Using_QString_Effectively
   if (itemClassName == QLatin1String{NE::staticMetaObject.className()}) {
      event.acceptProposedAction();
      emit (this->*signalSender)(ObjectStoreWrapper::getById<NE>(id).get());
      return true;
   }
   return false;
}

/**
 * \brief Appends to a list if the item being dragged and dropped is a match
 *
 *        It would be neat to pass \c droppedItems as a template parameter (since it's known at compile time) but this
 *        is not possible with a QList because it has non-public data members, so we would get an error that it is "not
 *        a valid type for a template non-type parameter because it is not structural".
 */
template<class NE>
bool BtTabWidget::appendItemIfMatch(QString const & itemClassName, int const id, QList<NE *> & droppedItems) {
   if (itemClassName == QLatin1String{NE::staticMetaObject.className()}) {
      droppedItems.append(ObjectStoreWrapper::getById<NE>(id).get());
      return true;
   }
   return false;
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

   QList<Fermentable *> fermentables;
   QList<Hop         *> hops        ;
   QList<Misc        *> miscs       ;
   QList<Yeast       *> yeasts      ;

   //
   // Drag-and-drop is potentially a very broad operation, because you can drag something from one program to another.
   // The MIME data gives us information about what the user is trying to drag and drop, so we can work out whether to
   // accept or reject the drop.
   //
   // TBD: In future, it would be kind of neat to allow drag-and-drop of BeerXML and BeerJSON files (possibly in both
   // directions).
   //
   //
   //
   QMimeData const * mData = event->mimeData();
   QByteArray itemData = mData->data(this->acceptMime);

   for (QDataStream dStream(&itemData, QIODevice::ReadOnly); !dStream.atEnd(); ) {
      QString itemClassName;
      int id;
      QString name;
      dStream >> itemClassName >> id >> name;
      qDebug() << Q_FUNC_INFO << "Item class name" << itemClassName;

      // For some things we only allow one to be dragged and dropped.  Eg, if the user drags multiple recipes, then
      // we'll set the first one and ignore the rest.
      if (this->acceptEventIfMatch<Recipe   , &BtTabWidget::setRecipe   >(itemClassName, id, *event)) { return; }
      if (this->acceptEventIfMatch<Equipment, &BtTabWidget::setEquipment>(itemClassName, id, *event)) { return; }
      if (this->acceptEventIfMatch<Style    , &BtTabWidget::setStyle    >(itemClassName, id, *event)) { return; }

      // For other things we do allow multiple items to be dragged and dropped
      if (this->appendItemIfMatch<Fermentable>(itemClassName, id, fermentables)) { continue; }
      if (this->appendItemIfMatch<Hop        >(itemClassName, id, hops        )) { continue; }
      if (this->appendItemIfMatch<Misc       >(itemClassName, id, miscs       )) { continue; }
      if (this->appendItemIfMatch<Yeast      >(itemClassName, id, yeasts      )) { continue; }

      qWarning() << Q_FUNC_INFO << "Unexpected item type" << itemClassName << "/" << id << "/" << name;
   }

   bool acceptedDrop = false;
   if (fermentables.size() > 0) { emit setFermentables(fermentables); acceptedDrop = true; }
   if (hops        .size() > 0) { emit setHops        (hops        ); acceptedDrop = true; }
   if (miscs       .size() > 0) { emit setMiscs       (miscs       ); acceptedDrop = true; }
   if (yeasts      .size() > 0) { emit setYeasts      (yeasts      ); acceptedDrop = true; }

   if (acceptedDrop) {
      event->acceptProposedAction();
   }
   return;
}
