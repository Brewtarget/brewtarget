/*
 * FermentableEditor.cpp is part of Brewtarget, and is Copyright the following
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
#include "FermentableEditor.h"

#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Fermentable.h"

FermentableEditor::FermentableEditor(QWidget* parent) :
   QDialog(parent), obsFerm(nullptr) {
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   connect( pushButton_new,    SIGNAL( clicked() ),       this, SLOT( newFermentable() ) );
   connect(pushButton_save,   &QAbstractButton::clicked, this, &FermentableEditor::save);
   connect(pushButton_cancel, &QAbstractButton::clicked, this, &FermentableEditor::clearAndClose);
   return;
}

FermentableEditor::~FermentableEditor() = default;

void FermentableEditor::setFermentable(Fermentable * newFerm) {
   if (newFerm) {
      obsFerm = newFerm;
      showChanges();
   }
   return;
}

void FermentableEditor::save() {
   if (!obsFerm) {
      setVisible(false);
      return;
   }

   obsFerm->setName(lineEdit_name->text());

   // NOTE: the following assumes that Fermentable::Type is enumerated in the same
   // order as the combobox.
   obsFerm->setType( static_cast<Fermentable::Type>(comboBox_type->currentIndex()) );

   obsFerm->setYield_pct(lineEdit_yield->toCanonical().quantity());
   obsFerm->setColor_srm(lineEdit_color->toCanonical().quantity());
   obsFerm->setAddAfterBoil((checkBox_addAfterBoil->checkState() == Qt::Checked)? true : false);
   obsFerm->setOrigin(lineEdit_origin->text());
   obsFerm->setSupplier(lineEdit_supplier->text());
   obsFerm->setCoarseFineDiff_pct(lineEdit_coarseFineDiff->toCanonical().quantity());
   obsFerm->setMoisture_pct(lineEdit_moisture->toCanonical().quantity());
   obsFerm->setDiastaticPower_lintner(lineEdit_diastaticPower->toCanonical().quantity());
   obsFerm->setProtein_pct(lineEdit_protein->toCanonical().quantity());
   obsFerm->setMaxInBatch_pct(lineEdit_maxInBatch->toCanonical().quantity());
   obsFerm->setRecommendMash((checkBox_recommendMash->checkState() == Qt::Checked) ? true : false);
   obsFerm->setIsMashed((checkBox_isMashed->checkState() == Qt::Checked) ? true : false);
   obsFerm->setIbuGalPerLb(lineEdit_ibuGalPerLb->toCanonical().quantity());
   obsFerm->setNotes(textEdit_notes->toPlainText());

   if (this->obsFerm->key() < 0) {
      ObjectStoreWrapper::insert(*this->obsFerm);
   }

   // Since inventory amount isn't really an attribute of the Fermentable, it's best to store it after we know the
   // Fermentable has a DB record.
   this->obsFerm->setInventoryAmount(lineEdit_inventory->toCanonical().quantity());

   setVisible(false);
   return;
}

void FermentableEditor::clearAndClose() {
   setFermentable(nullptr);
   setVisible(false); // Hide the window.
}

void FermentableEditor::showChanges(QMetaProperty* metaProp) {
   if (!this->obsFerm) {
      return;
   }

   QString propName;
   bool updateAll = false;
   if (metaProp == nullptr) {
      updateAll = true;
   } else {
      propName = metaProp->name();
   }

   if( propName == PropertyNames::NamedEntity::name || updateAll ) {
      lineEdit_name->setText(obsFerm->name());
      lineEdit_name->setCursorPosition(0);

      tabWidget_editor->setTabText(0, obsFerm->name() );
      if( ! updateAll ) {
         return;
      }
   }
   if (propName == PropertyNames::Fermentable::type || updateAll) {
      // NOTE: assumes the comboBox entries are in same order as Fermentable::Type
      comboBox_type->setCurrentIndex(obsFerm->type());
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::NamedEntityWithInventory::inventory || updateAll) {
      lineEdit_inventory->setText(obsFerm);
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::yield_pct || updateAll) {
      lineEdit_yield->setText(obsFerm);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Fermentable::color_srm || updateAll) {
      lineEdit_color->setText(obsFerm, 0);
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::addAfterBoil || updateAll) {
      checkBox_addAfterBoil->setCheckState( obsFerm->addAfterBoil() ? Qt::Checked : Qt::Unchecked );
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Fermentable::origin || updateAll) {
      lineEdit_origin->setText(obsFerm->origin());
      lineEdit_origin->setCursorPosition(0);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Fermentable::supplier || updateAll) {
      lineEdit_supplier->setText(obsFerm->supplier());
      lineEdit_supplier->setCursorPosition(0);
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::coarseFineDiff_pct || updateAll) {
      lineEdit_coarseFineDiff->setText(obsFerm);
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::moisture_pct || updateAll) {
      lineEdit_moisture->setText(obsFerm);
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::diastaticPower_lintner || updateAll) {
      lineEdit_diastaticPower->setText(obsFerm);
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::protein_pct || updateAll) {
      lineEdit_protein->setText(obsFerm);
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::maxInBatch_pct || updateAll) {
      lineEdit_maxInBatch->setText(obsFerm);
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::recommendMash || updateAll) {
      checkBox_recommendMash->setCheckState( obsFerm->recommendMash() ? Qt::Checked : Qt::Unchecked );
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::isMashed || updateAll) {
      checkBox_isMashed->setCheckState( obsFerm->isMashed() ? Qt::Checked : Qt::Unchecked );
      if (!updateAll) {
         return;
      }
   }
   if( propName == PropertyNames::Fermentable::ibuGalPerLb || updateAll) {
      lineEdit_ibuGalPerLb->setText(obsFerm);
      if (!updateAll) {
         return;
      }
   }
   if (propName == PropertyNames::Fermentable::notes || updateAll) {
      textEdit_notes->setPlainText( obsFerm->notes() );
      if (!updateAll) {
         return;
      }
   }
   return;
}

void FermentableEditor::newFermentable(QString folder)  {
   QString name = QInputDialog::getText(this, tr("Fermentable name"),
                                          tr("Fermentable name:"));
   if (name.isEmpty()) {
      return;
   }

   // .:TODO:. Change to shared_ptr as currently leads to memory leak in clearAndClose()
   Fermentable * f = new Fermentable(name);

   if (! folder.isEmpty()) {
      f->setFolder(folder);
   }

   setFermentable(f);
   show();
   return;
}

void FermentableEditor::newFermentable() {
   newFermentable(QString());
   return;
}
