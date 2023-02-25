/*
 * MiscEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
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
#include "MiscEditor.h"

#include <QtGui>
#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Misc.h"

MiscEditor::MiscEditor(QWidget * parent) :
   QDialog(parent),
   obsMisc(nullptr) {
   setupUi(this);

   tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   connect(pushButton_new, SIGNAL(clicked()), this, SLOT(newMisc()));
   connect(pushButton_save,   &QAbstractButton::clicked, this, &MiscEditor::save);
   connect(pushButton_cancel, &QAbstractButton::clicked, this, &MiscEditor::clearAndClose);

   return;
}

void MiscEditor::setMisc(Misc * m) {
   if (obsMisc) {
      disconnect(obsMisc, nullptr, this, nullptr);
   }

   obsMisc = m;
   if (obsMisc) {
      connect(obsMisc, SIGNAL(changed(QMetaProperty, QVariant)), this, SLOT(changed(QMetaProperty, QVariant)));
      showChanges();
   }
   return;
}

void MiscEditor::save() {
   if (this->obsMisc == nullptr) {
      setVisible(false);
      return;
   }

   qDebug() << Q_FUNC_INFO << comboBox_type->currentIndex();
   qDebug() << Q_FUNC_INFO << comboBox_use->currentIndex();

   this->obsMisc->setName(lineEdit_name->text());
   this->obsMisc->setType(static_cast<Misc::Type>(comboBox_type->currentIndex()));
   this->obsMisc->setUse(static_cast<Misc::Use>(comboBox_use->currentIndex()));
   this->obsMisc->setTime(lineEdit_time->toCanonical().quantity());
   this->obsMisc->setAmountIsWeight((checkBox_isWeight->checkState() == Qt::Checked) ? true : false);
   this->obsMisc->setUseFor(textEdit_useFor->toPlainText());
   this->obsMisc->setNotes(textEdit_notes->toPlainText());

   if (this->obsMisc->key() < 0) {
      qDebug() << Q_FUNC_INFO << "Inserting into database";
      ObjectStoreWrapper::insert(*this->obsMisc);
   }
   // do this late to make sure we've the row in the inventory table
   this->obsMisc->setInventoryAmount(lineEdit_inventory->toCanonical().quantity());
   setVisible(false);
   return;
}

void MiscEditor::clearAndClose() {
   setMisc(nullptr);
   setVisible(false); // Hide the window.
}

void MiscEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == obsMisc) {
      showChanges(&prop);
   }
}

void MiscEditor::showChanges(QMetaProperty * metaProp) {
   if (obsMisc == nullptr) {
      return;
   }

   QString propName;
   QVariant value;
   bool updateAll = false;
   if (metaProp == nullptr) {
      updateAll = true;
   } else {
      propName = metaProp->name();
      value = metaProp->read(obsMisc);
   }

   if (propName == PropertyNames::NamedEntity::name || updateAll) {
      lineEdit_name->setText(obsMisc->name());
      lineEdit_name->setCursorPosition(0);
      tabWidget_editor->setTabText(0, obsMisc->name());
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Misc::type || updateAll) {
      comboBox_type->setCurrentIndex(static_cast<int>(obsMisc->type()));
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Misc::use || updateAll) {
      comboBox_use->setCurrentIndex(static_cast<int>(obsMisc->use()));
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Misc::time || updateAll) {
      lineEdit_time->setText(obsMisc);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Misc::amountIsWeight || updateAll) {
      checkBox_isWeight->setCheckState(obsMisc->amountIsWeight() ? Qt::Checked : Qt::Unchecked);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::NamedEntityWithInventory::inventory || updateAll) {
      lineEdit_inventory->setText(obsMisc);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Misc::useFor || updateAll) {
      textEdit_useFor->setPlainText(obsMisc->useFor());
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Misc::notes || updateAll) {
      textEdit_notes->setPlainText(obsMisc->notes());
      if (!updateAll) {
         return;
      }
   }
}

void MiscEditor::newMisc(QString folder) {
   QString name = QInputDialog::getText(this, tr("Misc name"),
                                        tr("Misc name:"));
   if (name.isEmpty()) {
      return;
   }

   // .:TODO:. This leads to a memory leak in clearAndClose().  Change to shared_ptr
   Misc * m = new Misc(name);

   if (! folder.isEmpty()) {
      m->setFolder(folder);
   }

   setMisc(m);
   show();
   return;
}

void MiscEditor::newMisc() {
   newMisc(QString());
   return;
}
