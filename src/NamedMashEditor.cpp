/*
 * NamedMashEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
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
#include "NamedMashEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/Recipe.h"


NamedMashEditor::NamedMashEditor(QWidget* parent, MashStepEditor* editor, bool singleMashEditor) :
   QDialog{parent},
   mashObs{nullptr} {
   setupUi(this);

   if (singleMashEditor) {
      for (int i = 0; i < horizontalLayout_mashs->count(); ++i) {
         QWidget* w = horizontalLayout_mashs->itemAt(i)->widget();
         if (w) {
            w->setVisible(false);
         }
      }
      // pushButton_new->setVisible(false);
   }

   //! Create the list model and assign it to the combo box
   this->mashListModel = new MashListModel(mashComboBox);
   this->mashComboBox->setModel( mashListModel );

   //! Create the table model (and may St. Stevens take pity)
   this->mashStepTableModel = new MashStepTableModel(mashStepTableWidget);
   this->mashStepTableWidget->setItemDelegate(new MashStepItemDelegate());
   this->mashStepTableWidget->setModel(mashStepTableModel);

   //! Preserve the step editor
   this->mashStepEditor = editor;

   //! And do some fun stuff with the equipment
   this->equipListModel = new EquipmentListModel(equipmentComboBox);
   this->equipmentComboBox->setModel(equipListModel);

   SMART_FIELD_INIT(NamedMashEditor, label_name      , lineEdit_name      , Mash, PropertyNames::NamedEntity::name             );
   SMART_FIELD_INIT(NamedMashEditor, label_grainTemp , lineEdit_grainTemp , Mash, PropertyNames::Mash::grainTemp_c          , 1);
   SMART_FIELD_INIT(NamedMashEditor, label_spargeTemp, lineEdit_spargeTemp, Mash, PropertyNames::Mash::spargeTemp_c         , 1);
   SMART_FIELD_INIT(NamedMashEditor, label_spargePh  , lineEdit_spargePh  , Mash, PropertyNames::Mash::ph                   , 0);
   SMART_FIELD_INIT(NamedMashEditor, label_tunTemp   , lineEdit_tunTemp   , Mash, PropertyNames::Mash::tunTemp_c            , 1);
   SMART_FIELD_INIT(NamedMashEditor, label_tunMass   , lineEdit_tunMass   , Mash, PropertyNames::Mash::tunWeight_kg            );
   SMART_FIELD_INIT(NamedMashEditor, label_tunSpHeat , lineEdit_tunSpHeat , Mash, PropertyNames::Mash::tunSpecificHeat_calGC, 1);

   connect(this->equipmentComboBox,         &QComboBox::currentTextChanged, this, &NamedMashEditor::fromEquipment   );
   // ok and cancel buttons
   connect(this->pushButton_save,           &QAbstractButton::clicked,      this, &NamedMashEditor::saveAndClose    );
   connect(this->pushButton_cancel,         &QAbstractButton::clicked,      this, &NamedMashEditor::closeEditor     );
   // new mash step, delete mash step, move mash step up and down
   connect(this->pushButton_addMashStep,    &QAbstractButton::clicked,      this, &NamedMashEditor::addMashStep     );
   connect(this->pushButton_removeMashStep, &QAbstractButton::clicked,      this, &NamedMashEditor::removeMashStep  );
   connect(this->pushButton_mashUp,         &QAbstractButton::clicked,      this, &NamedMashEditor::moveMashStepUp  );
   connect(this->pushButton_mashDown,       &QAbstractButton::clicked,      this, &NamedMashEditor::moveMashStepDown);
   // finally, the combo box and the remove mash button
   connect(this->mashComboBox,              &QComboBox::currentTextChanged, this, &NamedMashEditor::mashSelected    );
   connect(this->pushButton_remove,         &QAbstractButton::clicked,      this, &NamedMashEditor::removeMash      );

   this->setMash(mashListModel->at(mashComboBox->currentIndex()));
   return;
}

void NamedMashEditor::showEditor() {
   this->showChanges();
   this->setVisible(true);
   return;
}

void NamedMashEditor::closeEditor() {
   this->setVisible(false);
   return;
}

void NamedMashEditor::saveAndClose() {
   if (this->mashObs == nullptr) {
      return;
   }

   qDebug() << Q_FUNC_INFO << "Saving mash (#" << this->mashObs->key() << ")";

   // using toCanonical aon the spargePh is something of a cheat, but the btLineEdit
   // class will do the right thing. That is how a plan comes together.

   this->mashObs->setEquipAdjust          (true); // BeerXML won't like me, but it's just stupid not to adjust for the equipment when you're able.
   this->mashObs->setName                 (lineEdit_name      ->text());
   this->mashObs->setGrainTemp_c          (lineEdit_grainTemp ->toCanonical().quantity());
   this->mashObs->setSpargeTemp_c         (lineEdit_spargeTemp->toCanonical().quantity());
   this->mashObs->setPh                   (lineEdit_spargePh  ->toCanonical().quantity());
   this->mashObs->setTunTemp_c            (lineEdit_tunTemp   ->toCanonical().quantity());
   this->mashObs->setTunWeight_kg         (lineEdit_tunMass   ->toCanonical().quantity());
   this->mashObs->setTunSpecificHeat_calGC(lineEdit_tunSpHeat ->toCanonical().quantity());

   this->mashObs->setNotes( textEdit_notes->toPlainText() );
   return;
}

void NamedMashEditor::setMash(Mash* mash) {
   if( mashObs ) {
      disconnect( mashObs, 0, this, 0 );
   }

   mashObs = mash;
   mashStepTableModel->setMash(mashObs);

   if (mashObs) {
      connect( mashObs, &NamedEntity::changed, this, &NamedMashEditor::changed );
      showChanges();
   }
}

void NamedMashEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == mashObs) {
      showChanges(&prop);
   }
   return;
}

void NamedMashEditor::showChanges(QMetaProperty* prop) {
   bool updateAll = false;
   QString propName;

   if (this->mashObs == nullptr) {
      this->clear();
      return;
   }

   if (prop == nullptr) {
      updateAll = true;
   } else {
      propName = prop->name();
   }
   qDebug() << Q_FUNC_INFO << "Updating" << (updateAll ? "all" : "property") << propName;

   if (updateAll || propName == PropertyNames::NamedEntity::name          ) {lineEdit_name      ->setText     (mashObs->name                 ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::grainTemp_c          ) {lineEdit_grainTemp ->setAmount   (mashObs->grainTemp_c          ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::spargeTemp_c         ) {lineEdit_spargeTemp->setAmount   (mashObs->spargeTemp_c         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::ph                   ) {lineEdit_spargePh  ->setAmount   (mashObs->ph                   ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::tunTemp_c            ) {lineEdit_tunTemp   ->setAmount   (mashObs->tunTemp_c            ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::tunWeight_kg         ) {lineEdit_tunMass   ->setAmount   (mashObs->tunWeight_kg         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::tunSpecificHeat_calGC) {lineEdit_tunSpHeat ->setAmount   (mashObs->tunSpecificHeat_calGC()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::notes                ) {textEdit_notes     ->setPlainText(mashObs->notes                ()); if (!updateAll) { return; } }
}

void NamedMashEditor::clear() {
   lineEdit_name      ->setText(QString(""));
   lineEdit_grainTemp ->setText(QString(""));
   lineEdit_spargeTemp->setText(QString(""));
   lineEdit_spargePh  ->setText(QString(""));
   lineEdit_tunTemp   ->setText(QString(""));
   lineEdit_tunMass   ->setText(QString(""));
   lineEdit_tunSpHeat ->setText(QString(""));
   textEdit_notes     ->setPlainText("");
   return;
}

void NamedMashEditor::addMashStep() {
   if (!this->mashObs) {
      return;
   }

   // The call to Mash::addMashStep() will also store the MashStep in the ObjectStore / DB
   auto step = std::make_shared<MashStep>();
   this->mashObs->addMashStep(step);
   mashStepEditor->setMashStep(step);
   mashStepEditor->setVisible(true);
   return;
}

bool NamedMashEditor::justOne(QModelIndexList selected) {
   int size = selected.size();
   if ( ! size ) {
      return false;
   }

   int row = selected[0].row();
   for (int ii = 1; ii < size; ++ii) {
      if (selected[ii].row() != row) {
         return false;
      }
   }
   return true;
}

void NamedMashEditor::removeMashStep() {
   if (!this->mashObs) {
      return;
   }

   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
   if ( !justOne(selected) ) {
      return;
   }

   auto step = mashStepTableModel->getRow(selected[0].row());
   this->mashObs->removeMashStep(step);
   return;
}

void NamedMashEditor::moveMashStepUp() {
   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
   if (selected.isEmpty()) {
      //nothing selected
      return;
   }

   int row = selected[0].row();
   if ( ! justOne(selected) || row < 1) {
      return;
   }

   mashStepTableModel->moveStepUp(row);
   return;
}

void NamedMashEditor::moveMashStepDown() {
   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
   if (selected.isEmpty()) {
      //nothing selected
      return;
   }

   int row = selected[0].row();
   if ( ! justOne(selected) || row >= mashStepTableModel->rowCount()-1 ) {
      return;
   }

   mashStepTableModel->moveStepDown(row);
   return;
}

void NamedMashEditor::mashSelected([[maybe_unused]] QString const & name) {
   Mash* selected = mashListModel->at(mashComboBox->currentIndex());
   if (selected && selected != mashObs) {
      setMash(selected);
   }
   return;
}

void NamedMashEditor::fromEquipment([[maybe_unused]] QString const & name) {
   if (!this->mashObs) {
      return;
   }
   Equipment * selected = equipListModel->at(equipmentComboBox->currentIndex());
   if (selected) {
      lineEdit_tunMass  ->setAmount(selected->tunWeight_kg         ());
      lineEdit_tunSpHeat->setAmount(selected->tunSpecificHeat_calGC());
   }
   return;
}

void NamedMashEditor::removeMash() {
   if (!this->mashObs) {
      return;
   }

   int newMash = this->mashComboBox->currentIndex() - 1;

   // I *think* we want to disconnect the mash first?
   disconnect(this->mashObs, 0, this, 0);
   // Delete the mashsteps
   // .:TBD:. Mash should be responsible for deleting its steps.  This is already correctly handled for hard delete, but
   // not for soft delete.
   for (auto step : this->mashObs->mashSteps()) {
      ObjectStoreWrapper::softDelete(*step);
   }
   // Delete the mash itself
   ObjectStoreWrapper::softDelete(*this->mashObs);

   this->setMash(this->mashListModel->at(newMash));
   return;
}
