/*
 * MashComboBox.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Julein <j2bweb@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "MashComboBox.h"

#include <QList>

#include "database/ObjectStoreWrapper.h"
#include "model/Mash.h"

MashComboBox::MashComboBox(QWidget * parent) :
   QComboBox(parent) {
   this->setCurrentIndex(-1);

   connect(&ObjectStoreTyped<Mash>::getInstance(), &ObjectStoreTyped<Mash>::signalObjectInserted, this, &MashComboBox::addMash);
   connect(&ObjectStoreTyped<Mash>::getInstance(), &ObjectStoreTyped<Mash>::signalObjectDeleted,  this, &MashComboBox::removeMash);
   this->repopulateList();
   return;
}


void MashComboBox::addMash(int mashId) {
   this->add(ObjectStoreWrapper::getByIdRaw<Mash>(mashId));
   return;
}

void MashComboBox::add(Mash * m) {
   if (m && !this->mashObs.contains(m) && m->display() && !m->deleted()) {
      this->mashObs.append(m);
      connect(m, SIGNAL(changed(QMetaProperty, QVariant)), this, SLOT(changed(QMetaProperty, QVariant)));
   }

   this->addItem(m->name());
   return;
}

void MashComboBox::removeMash([[maybe_unused]] int mashId,
                              std::shared_ptr<QObject> object) {
   this->remove(std::static_pointer_cast<Mash>(object).get());
   return;
}

void MashComboBox::remove(Mash * m) {
   if (m) {
      disconnect(m, 0, this, 0);
   }
   int ndx = this->mashObs.indexOf(m);
   if (ndx >= 0) {
      this->mashObs.removeAt(ndx);
      this->removeItem(ndx);
   }
   return;
}

void MashComboBox::removeAllMashs() {
   QList<Mash *> tmpMashs(mashObs);

   for (int i = 0; i < tmpMashs.size(); ++i) {
      this->remove(tmpMashs[i]);
   }
}

void MashComboBox::changed([[maybe_unused]] QMetaProperty prop,
                           [[maybe_unused]] QVariant      val) {
   int i = mashObs.indexOf(qobject_cast<Mash *>(sender()));
   if (i >= 0) {
      // Notice we assume 'i' is an index into both 'mashObs' and also
      // to the text list in this combo box...
      this->setItemText(i, mashObs[i]->name());
   }
   return;
}

void MashComboBox::setIndexByMash(Mash * mash) {
   int ndx = mashObs.indexOf(mash);
   this->setCurrentIndex(ndx);
   return;
}

void MashComboBox::setIndex(int ndx) {
   this->setCurrentIndex(ndx);
}

void MashComboBox::repopulateList() {
   unsigned int i, size;
   this->clear();

   QList<Mash *> tmpMashs(mashObs);
   size = tmpMashs.size();
   for (i = 0; i < size; ++i) {
      this->remove(tmpMashs[i]);
   }

   tmpMashs.clear();
   tmpMashs = ObjectStoreWrapper::getAllRaw<Mash>();

   size = tmpMashs.size();
   for (i = 0; i < size; ++i) {
      this->add(tmpMashs[i]);
   }

   this->setCurrentIndex(-1);
   return;
}

Mash * MashComboBox::getSelectedMash() {
   if (this->currentIndex() >= 0) {
      return this->mashObs[this->currentIndex()];
   }
   return nullptr;
}
