/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/NamedMashEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Daniel Moreno <danielm5@users.noreply.github.com>
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
#include "editors/NamedMashEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_NamedMashEditor.cpp"
#endif


NamedMashEditor::NamedMashEditor(QWidget* parent, MashStepEditor* editor, bool singleMashEditor) :
   QDialog{parent},
   m_mashObs{nullptr} {
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
   this->m_mashListModel = new MashListModel(mashComboBox);
   this->mashComboBox->setModel(this->m_mashListModel);

   //! Create the table model (and may St. Stevens take pity)
   this->m_mashStepTableModel = new MashStepTableModel(mashStepTableWidget);
   this->mashStepTableWidget->setItemDelegate(new MashStepItemDelegate(this->mashStepTableWidget, *this->m_mashStepTableModel));
   this->mashStepTableWidget->setModel(this->m_mashStepTableModel);

   //! Preserve the step editor
   this->m_mashStepEditor = editor;

   //! And do some fun stuff with the equipment
   this->m_equipListModel = new EquipmentListModel(equipmentComboBox);
   this->equipmentComboBox->setModel(m_equipListModel);

   SMART_FIELD_INIT(NamedMashEditor, label_name      , lineEdit_name      , Mash, PropertyNames::NamedEntity::name             );
   SMART_FIELD_INIT(NamedMashEditor, label_grainTemp , lineEdit_grainTemp , Mash, PropertyNames::Mash::grainTemp_c          , 1);
   SMART_FIELD_INIT(NamedMashEditor, label_spargeTemp, lineEdit_spargeTemp, Mash, PropertyNames::Mash::spargeTemp_c         , 1);
   SMART_FIELD_INIT(NamedMashEditor, label_spargePh  , lineEdit_spargePh  , Mash, PropertyNames::Mash::ph                   , 0);
   SMART_FIELD_INIT(NamedMashEditor, label_tunTemp   , lineEdit_tunTemp   , Mash, PropertyNames::Mash::tunTemp_c            , 1);
   SMART_FIELD_INIT(NamedMashEditor, label_tunMass   , lineEdit_tunMass   , Mash, PropertyNames::Mash::mashTunWeight_kg            );
   SMART_FIELD_INIT(NamedMashEditor, label_tunSpHeat , lineEdit_tunSpHeat , Mash, PropertyNames::Mash::mashTunSpecificHeat_calGC, 1);

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

   auto mash {m_mashListModel->at(mashComboBox->currentIndex())};
   if (mash) {
      this->setMash(ObjectStoreWrapper::getSharedFromRaw(mash));
   }
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
   if (!this->m_mashObs) {
      return;
   }

   qDebug() << Q_FUNC_INFO << "Saving mash (#" << this->m_mashObs->key() << ")";

   // using toCanonical aon the spargePh is something of a cheat, but the btLineEdit
   // class will do the right thing. That is how a plan comes together.

   this->m_mashObs->setEquipAdjust          (true); // BeerXML won't like me, but it's just stupid not to adjust for the equipment when you're able.
   this->m_mashObs->setName                 (lineEdit_name      ->text());
   this->m_mashObs->setGrainTemp_c          (lineEdit_grainTemp ->getNonOptCanonicalQty());
   this->m_mashObs->setSpargeTemp_c         (lineEdit_spargeTemp->getNonOptCanonicalQty());
   this->m_mashObs->setPh                   (lineEdit_spargePh  ->getNonOptCanonicalQty());
   this->m_mashObs->setTunTemp_c            (lineEdit_tunTemp   ->getNonOptCanonicalQty());
   this->m_mashObs->setTunWeight_kg         (lineEdit_tunMass   ->getNonOptCanonicalQty());
   this->m_mashObs->setMashTunSpecificHeat_calGC(lineEdit_tunSpHeat ->getNonOptCanonicalQty());

   this->m_mashObs->setNotes(textEdit_notes->toPlainText());
   return;
}

void NamedMashEditor::setMash(std::shared_ptr<Mash> mash) {
   if (this->m_mashObs) {
      disconnect(this->m_mashObs.get(), 0, this, 0 );
   }

   this->m_mashObs = mash;
   this->m_mashStepTableModel->setMash(this->m_mashObs);

   if (this->m_mashObs) {
      connect(this->m_mashObs.get(), &NamedEntity::changed, this, &NamedMashEditor::changed );
      showChanges();
   }
}

void NamedMashEditor::changed(QMetaProperty prop, QVariant /*val*/) {
   if (sender() == this->m_mashObs.get()) {
      showChanges(&prop);
   }
   return;
}

void NamedMashEditor::showChanges(QMetaProperty * prop) {
   if (!this->m_mashObs) {
      this->clear();
      return;
   }

   bool updateAll = false;
   QString propName;
   if (!prop) {
      updateAll = true;
   } else {
      propName = prop->name();
   }
   qDebug() << Q_FUNC_INFO << "Updating" << (updateAll ? "all properties" : "property") << propName;

   if (updateAll || propName == PropertyNames::NamedEntity::name          ) {lineEdit_name      ->setText       (m_mashObs->name                 ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::grainTemp_c          ) {lineEdit_grainTemp ->setQuantity   (m_mashObs->grainTemp_c          ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::spargeTemp_c         ) {lineEdit_spargeTemp->setQuantity   (m_mashObs->spargeTemp_c         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::ph                   ) {lineEdit_spargePh  ->setQuantity   (m_mashObs->ph                   ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::tunTemp_c            ) {lineEdit_tunTemp   ->setQuantity   (m_mashObs->tunTemp_c            ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::mashTunWeight_kg     ) {lineEdit_tunMass   ->setQuantity   (m_mashObs->mashTunWeight_kg         ()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::mashTunSpecificHeat_calGC) {lineEdit_tunSpHeat->setQuantity(m_mashObs->mashTunSpecificHeat_calGC()); if (!updateAll) { return; } }
   if (updateAll || propName == PropertyNames::Mash::notes                ) {textEdit_notes     ->setPlainText  (m_mashObs->notes                ()); if (!updateAll) { return; } }
   return;
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
   if (!this->m_mashObs) {
      return;
   }

   // The call to Mash::addMashStep() will also store the MashStep in the ObjectStore / DB
   auto step = std::make_shared<MashStep>();
   this->m_mashObs->add(step);
   m_mashStepEditor->setEditItem(step);
   m_mashStepEditor->setVisible(true);
   return;
}

bool NamedMashEditor::justOne(QModelIndexList selected) {
   int size = selected.size();
   if (!size) {
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
   if (!this->m_mashObs) {
      return;
   }

   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
   if ( !justOne(selected) ) {
      return;
   }

   auto step = m_mashStepTableModel->getRow(selected[0].row());
   this->m_mashObs->remove(step);
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

   m_mashStepTableModel->moveStepUp(row);
   return;
}

void NamedMashEditor::moveMashStepDown() {
   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
   if (selected.isEmpty()) {
      //nothing selected
      return;
   }

   int row = selected[0].row();
   if ( ! justOne(selected) || row >= m_mashStepTableModel->rowCount()-1 ) {
      return;
   }

   m_mashStepTableModel->moveStepDown(row);
   return;
}

void NamedMashEditor::mashSelected([[maybe_unused]] QString const & name) {
   auto selected = ObjectStoreWrapper::getSharedFromRaw(m_mashListModel->at(mashComboBox->currentIndex()));
   if (selected && selected != m_mashObs) {
      setMash(selected);
   }
   return;
}

void NamedMashEditor::fromEquipment([[maybe_unused]] QString const & name) {
   if (!this->m_mashObs) {
      return;
   }
   Equipment * selected = m_equipListModel->at(equipmentComboBox->currentIndex());
   if (selected) {
      lineEdit_tunMass  ->setQuantity(selected->mashTunWeight_kg         ());
      lineEdit_tunSpHeat->setQuantity(selected->mashTunSpecificHeat_calGC());
   }
   return;
}

void NamedMashEditor::removeMash() {
   if (!this->m_mashObs) {
      return;
   }


   // I *think* we want to disconnect the mash first?
   disconnect(this->m_mashObs.get(), 0, this, 0);
   // Delete the mashsteps
   // .:TBD:. Mash should be responsible for deleting its steps.  This is already correctly handled for hard delete, but
   // not for soft delete.
   for (auto step : this->m_mashObs->mashSteps()) {
      ObjectStoreWrapper::softDelete(*step);
   }
   // Delete the mash itself
   ObjectStoreWrapper::softDelete(*this->m_mashObs);

   auto newMash = ObjectStoreWrapper::getSharedFromRaw(
      this->m_mashListModel->at(this->mashComboBox->currentIndex() - 1)
   );
   this->setMash(newMash);
   return;
}
