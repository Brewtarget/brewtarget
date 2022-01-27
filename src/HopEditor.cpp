/*
 * HopEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Kregg K <gigatropolis@yahoo.com>
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
#include "HopEditor.h"

#include <QtGui>
#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Hop.h"

HopEditor::HopEditor(QWidget * parent) :
   QDialog(parent),
   obsHop(nullptr) {
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   connect(pushButton_new, SIGNAL(clicked()), this, SLOT(newHop()));
   connect(pushButton_save,   &QAbstractButton::clicked, this, &HopEditor::save);
   connect(pushButton_cancel, &QAbstractButton::clicked, this, &HopEditor::clearAndClose);

   return;
}

void HopEditor::setHop(Hop * h) {
   if (obsHop) {
      disconnect(obsHop, nullptr, this, nullptr);
   }

   obsHop = h;
   if (obsHop) {
      connect(obsHop, &NamedEntity::changed, this, &HopEditor::changed);
      showChanges();
   }
}

void HopEditor::save() {
   if (!this->obsHop) {
      setVisible(false);
      return;
   }

   this->obsHop->setName(lineEdit_name->text());
   this->obsHop->setAlpha_pct(lineEdit_alpha->toSI().quantity);
   this->obsHop->setUse(static_cast<Hop::Use>(comboBox_use->currentIndex()));
   this->obsHop->setTime_min(lineEdit_time->toSI().quantity);
   this->obsHop->setType(static_cast<Hop::Type>(comboBox_type->currentIndex()));
   this->obsHop->setForm(static_cast<Hop::Form>(comboBox_form->currentIndex()));
   this->obsHop->setBeta_pct(lineEdit_beta->toSI().quantity);
   this->obsHop->setHsi_pct(lineEdit_HSI->toSI().quantity);
   this->obsHop->setOrigin(lineEdit_origin->text());
   this->obsHop->setHumulene_pct(lineEdit_humulene->toSI().quantity);
   this->obsHop->setCaryophyllene_pct(lineEdit_caryophyllene->toSI().quantity);
   this->obsHop->setCohumulone_pct(lineEdit_cohumulone->toSI().quantity);
   this->obsHop->setMyrcene_pct(lineEdit_myrcene->toSI().quantity);

   this->obsHop->setSubstitutes(textEdit_substitutes->toPlainText());
   this->obsHop->setNotes(textEdit_notes->toPlainText());

   if (this->obsHop->key() < 0) {
      ObjectStoreWrapper::insert(*this->obsHop);
   }

   // do this late to make sure we've the row in the inventory table
   this->obsHop->setInventoryAmount(lineEdit_inventory->toSI().quantity);
   setVisible(false);
   return;
}

void HopEditor::clearAndClose() {
   setHop(nullptr);
   setVisible(false); // Hide the window.
}

void HopEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == obsHop) {
      showChanges(&prop);
   }
}

void HopEditor::showChanges(QMetaProperty * prop) {
   bool updateAll = false;
   QString propName;
   if (obsHop == nullptr) {
      return;
   }

   if (prop == nullptr) {
      updateAll = true;
   } else {
      propName = prop->name();
   }

   if (propName == PropertyNames::NamedEntity::name || updateAll) {
      lineEdit_name->setText(obsHop->name());
      lineEdit_name->setCursorPosition(0);
      tabWidget_editor->setTabText(0, obsHop->name());
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::alpha_pct || updateAll) {
      lineEdit_alpha->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::NamedEntityWithInventory::inventory || updateAll) {
      lineEdit_inventory->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::use || updateAll) {
      comboBox_use->setCurrentIndex(obsHop->use());
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::time_min || updateAll) {
      lineEdit_time->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::type || updateAll) {
      comboBox_type->setCurrentIndex(obsHop->type());
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::form || updateAll) {
      comboBox_form->setCurrentIndex(obsHop->form());
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::beta_pct || updateAll) {
      lineEdit_beta->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::hsi_pct || updateAll) {
      lineEdit_HSI->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::origin || updateAll) {
      lineEdit_origin->setText(obsHop->origin());
      lineEdit_origin->setCursorPosition(0);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::humulene_pct || updateAll) {
      lineEdit_humulene->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::caryophyllene_pct || updateAll) {
      lineEdit_caryophyllene->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::cohumulone_pct || updateAll) {
      lineEdit_cohumulone->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::myrcene_pct || updateAll) {
      lineEdit_myrcene->setText(obsHop);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::substitutes || updateAll) {
      textEdit_substitutes->setPlainText(obsHop->substitutes());
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Hop::notes || updateAll) {
      textEdit_notes->setPlainText(obsHop->notes());
      if (!updateAll) {
         return;
      }
   }
}


void HopEditor::newHop(QString folder) {
   QString name = QInputDialog::getText(this, tr("Hop name"),
                                        tr("Hop name:"));
   if (name.isEmpty()) {
      return;
   }

   // .:TODO:. Change this to shared_ptr as currently results in memory leak in clearAndClose()
   Hop * h = new Hop(name);

   if (! folder.isEmpty()) {
      h->setFolder(folder);
   }

   setHop(h);
   show();
   return;
}

void HopEditor::newHop() {
   newHop(QString());
   return;
}
