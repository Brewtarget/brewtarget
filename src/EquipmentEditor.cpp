/*
 * EquipmentEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - David Grundberg <individ@acc.umu.se>
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
#include "EquipmentEditor.h"

#include <QCloseEvent>
#include <QDebug>
#include <QIcon>
#include <QInputDialog>
#include <QMessageBox>

#include "BtHorizontalTabs.h"
#include "database/ObjectStoreWrapper.h"
#include "EquipmentListModel.h"
#include "HeatCalculations.h"
#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Equipment.h"
#include "NamedEntitySortProxyModel.h"
#include "PersistentSettings.h"
#include "PhysicalConstants.h"

EquipmentEditor::EquipmentEditor(QWidget* parent, bool singleEquipEditor) :
   QDialog(parent) {
   setupUi(this);

   if (singleEquipEditor) {
      //horizontalLayout_equipments->setVisible(false);
      for (int i = 0; i < horizontalLayout_equipments->count(); ++i) {
         QWidget* w = horizontalLayout_equipments->itemAt(i)->widget();
         if (w) {
            w->setVisible(false);
         }
      }

   }

   this->tabWidget_editor->tabBar()->setStyle( new BtHorizontalTabs );
   // Set grain absorption label based on units.
   Measurement::Unit const * weightUnit = nullptr;
   Measurement::Unit const * volumeUnit = nullptr;
   Measurement::getThicknessUnits(&volumeUnit, &weightUnit);
   label_grainAbsorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->name).arg(weightUnit->name));

   equipmentListModel = new EquipmentListModel(equipmentComboBox);
   equipmentSortProxyModel = new NamedEntitySortProxyModel(equipmentListModel);
   equipmentComboBox->setModel(equipmentSortProxyModel);

   obsEquip = nullptr;

   SMART_FIELD_INIT(EquipmentEditor, label_tunSpecificHeat, lineEdit_tunSpecificHeat, Equipment, PropertyNames::Equipment::tunSpecificHeat_calGC   );
   SMART_FIELD_INIT(EquipmentEditor, label_grainAbsorption, lineEdit_grainAbsorption, Equipment, PropertyNames::Equipment::grainAbsorption_LKg     );
   SMART_FIELD_INIT(EquipmentEditor, label_hopUtilization , lineEdit_hopUtilization , Equipment, PropertyNames::Equipment::hopUtilization_pct   , 0);
   SMART_FIELD_INIT(EquipmentEditor, label_tunWeight      , lineEdit_tunWeight      , Equipment, PropertyNames::Equipment::tunWeight_kg            );
   SMART_FIELD_INIT(EquipmentEditor, label_name           , lineEdit_name           , Equipment, PropertyNames::NamedEntity::name                  );
   SMART_FIELD_INIT(EquipmentEditor, label_boilingPoint   , lineEdit_boilingPoint   , Equipment, PropertyNames::Equipment::boilingPoint_c       , 1);
   SMART_FIELD_INIT(EquipmentEditor, label_boilTime       , lineEdit_boilTime       , Equipment, PropertyNames::Equipment::boilTime_min            );
   SMART_FIELD_INIT(EquipmentEditor, label_batchSize      , lineEdit_batchSize      , Equipment, PropertyNames::Equipment::batchSize_l             );
   SMART_FIELD_INIT(EquipmentEditor, label_boilSize       , lineEdit_boilSize       , Equipment, PropertyNames::Equipment::boilSize_l              );
   SMART_FIELD_INIT(EquipmentEditor, label_evaporationRate, lineEdit_evaporationRate, Equipment, PropertyNames::Equipment::evapRate_lHr            );
   SMART_FIELD_INIT(EquipmentEditor, label_lauterDeadspace, lineEdit_lauterDeadspace, Equipment, PropertyNames::Equipment::lauterDeadspace_l       );
   SMART_FIELD_INIT(EquipmentEditor, label_topUpKettle    , lineEdit_topUpKettle    , Equipment, PropertyNames::Equipment::topUpKettle_l           );
   SMART_FIELD_INIT(EquipmentEditor, label_topUpWater     , lineEdit_topUpWater     , Equipment, PropertyNames::Equipment::topUpWater_l            );
   SMART_FIELD_INIT(EquipmentEditor, label_trubChillerLoss, lineEdit_trubChillerLoss, Equipment, PropertyNames::Equipment::trubChillerLoss_l       );
   SMART_FIELD_INIT(EquipmentEditor, label_tunVolume      , lineEdit_tunVolume      , Equipment, PropertyNames::Equipment::tunVolume_l             );

   // Connect all the edit boxen
   connect(lineEdit_boilTime,         &SmartLineEdit::textModified,  this, &EquipmentEditor::updateCheckboxRecord     );
   connect(lineEdit_evaporationRate,  &SmartLineEdit::textModified,  this, &EquipmentEditor::updateCheckboxRecord     );
   connect(lineEdit_topUpWater,       &SmartLineEdit::textModified,  this, &EquipmentEditor::updateCheckboxRecord     );
   connect(lineEdit_trubChillerLoss,  &SmartLineEdit::textModified,  this, &EquipmentEditor::updateCheckboxRecord     );
   connect(lineEdit_batchSize,        &SmartLineEdit::textModified,  this, &EquipmentEditor::updateCheckboxRecord     );
   // Set up the buttons
   // Note, per https://wiki.qt.io/New_Signal_Slot_Syntax#Default_arguments_in_slot, the use of a trivial lambda
   // function to allow use of default argument on newEquipment() slot
   connect(pushButton_save,           &QAbstractButton::clicked,      this, &EquipmentEditor::save                     );
   connect(pushButton_new,            &QAbstractButton::clicked,      this, [this]() { this->newEquipment(); return; } );
   connect(pushButton_cancel,         &QAbstractButton::clicked,      this, &EquipmentEditor::cancel                   );
   connect(pushButton_remove,         &QAbstractButton::clicked,      this, &EquipmentEditor::removeEquipment          );
   connect(pushButton_absorption,     &QAbstractButton::clicked,      this, &EquipmentEditor::resetAbsorption          );
   connect(equipmentComboBox,         &QComboBox::currentTextChanged, this, &EquipmentEditor::equipmentSelected        );
   // Check boxen
   connect(checkBox_calcBoilVolume,   &QCheckBox::stateChanged,       this, &EquipmentEditor::updateCheckboxRecord     );
   connect(checkBox_defaultEquipment, &QCheckBox::stateChanged,       this, &EquipmentEditor::updateDefaultEquipment   );

   // make sure the dialog gets populated the first time it's opened from the menu
   equipmentSelected();
   // Ensure correct state of Boil Volume edit box.
   updateCheckboxRecord();
   return;
}

EquipmentEditor::~EquipmentEditor() = default;

void EquipmentEditor::setEquipment(Equipment* e) {
   if (e) {
      obsEquip = e;

      // Make sure the combo box gets set to the right place.
      QModelIndex modelIndex(equipmentListModel->find(e));
      QModelIndex viewIndex(equipmentSortProxyModel->mapFromSource(modelIndex));
      if (viewIndex.isValid()) {
         equipmentComboBox->setCurrentIndex(viewIndex.row());
      }

      showChanges();
   }
   return;
}

void EquipmentEditor::removeEquipment() {
   if( this->obsEquip ) {
      ObjectStoreWrapper::softDelete(*this->obsEquip);
   }

   equipmentComboBox->setCurrentIndex(-1);
   setEquipment(nullptr);
   return;
}

void EquipmentEditor::clear() {
   lineEdit_name           ->setText(QString(""));
   lineEdit_name           ->setCursorPosition(0);
   lineEdit_boilSize       ->setText(QString(""));
   checkBox_calcBoilVolume ->setCheckState(Qt::Unchecked);
   lineEdit_batchSize      ->setText(QString(""));
   lineEdit_tunVolume      ->setText(QString(""));
   lineEdit_tunWeight      ->setText(QString(""));
   lineEdit_tunSpecificHeat->setText(QString(""));
   lineEdit_boilTime       ->setText(QString(""));
   lineEdit_evaporationRate->setText(QString(""));
   lineEdit_topUpKettle    ->setText(QString(""));
   lineEdit_topUpWater     ->setText(QString(""));
   lineEdit_trubChillerLoss->setText(QString(""));
   lineEdit_lauterDeadspace->setText(QString(""));
   lineEdit_hopUtilization ->setText(QString(""));
   textEdit_notes          ->setText(QString(""));
   lineEdit_grainAbsorption->setText(QString(""));
   return;
}

void EquipmentEditor::equipmentSelected() {
   QModelIndex viewIndex(
      equipmentComboBox->model()->index(equipmentComboBox->currentIndex(),0)
   );

   QModelIndex modelIndex = equipmentSortProxyModel->mapToSource(viewIndex);

   setEquipment( equipmentListModel->at(modelIndex.row()) );
   return;
}

void EquipmentEditor::save() {
   if (obsEquip == nullptr) {
      setVisible(false);
      return;
   }

   Measurement::Unit const * weightUnit = nullptr;
   Measurement::Unit const * volumeUnit = nullptr;
   Measurement::getThicknessUnits(&volumeUnit, &weightUnit);

   double grainAbs = Localization::toDouble(lineEdit_grainAbsorption->text(), Q_FUNC_INFO);

   double ga_LKg = grainAbs * volumeUnit->toCanonical(1.0).quantity() * weightUnit->fromCanonical(1.0);

//   QString describe;
   bool problems=false;

   // Do some prewarning things. I would prefer to do this only on change, but
   // we need to be worried about new equipment too.
   QString message = tr("This equipment profile may break Brewtarget's maths");
   QString inform = QString("%1%2").arg(tr("The following values are not set:")).arg(QString("<ul>"));
   if ( qFuzzyCompare(lineEdit_tunVolume->toCanonical().quantity(), 0.0) ) {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("mash tun volume (all-grain and BIAB only)"));
   }

   if ( qFuzzyCompare(lineEdit_batchSize->toCanonical().quantity(), 0.0) ) {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("batch size"));
   }

   if ( qFuzzyCompare(lineEdit_hopUtilization->getNonOptValueAs<double>(), 0.0) ) {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("hop utilization"));
   }
   inform = inform + QString("</ul>");

   if (problems) {
      QMessageBox theQuestion;
      theQuestion.setWindowTitle( tr("Calculation Warnings") );
      theQuestion.setText( message );
      theQuestion.setInformativeText( inform );
      theQuestion.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
      theQuestion.setDefaultButton(QMessageBox::Save);
      theQuestion.setIcon(QMessageBox::Warning);
      if (theQuestion.exec() == QMessageBox::Cancel) {
         return;
      }
   }

   this->obsEquip->setName                 (lineEdit_name           ->text() );
   this->obsEquip->setBoilSize_l           (lineEdit_boilSize       ->toCanonical().quantity() );
   this->obsEquip->setBatchSize_l          (lineEdit_batchSize      ->toCanonical().quantity() );
   this->obsEquip->setTunVolume_l          (lineEdit_tunVolume      ->toCanonical().quantity() );
   this->obsEquip->setTunWeight_kg         (lineEdit_tunWeight      ->toCanonical().quantity() );
   this->obsEquip->setTunSpecificHeat_calGC(lineEdit_tunSpecificHeat->getNonOptValueAs<double>() );  // TODO Convert this to real units!
   this->obsEquip->setBoilTime_min         (lineEdit_boilTime       ->toCanonical().quantity());
   this->obsEquip->setEvapRate_lHr         (lineEdit_evaporationRate->toCanonical().quantity() );
   this->obsEquip->setTopUpKettle_l        (lineEdit_topUpKettle    ->toCanonical().quantity() );
   this->obsEquip->setTopUpWater_l         (lineEdit_topUpWater     ->toCanonical().quantity() );
   this->obsEquip->setTrubChillerLoss_l    (lineEdit_trubChillerLoss->toCanonical().quantity() );
   this->obsEquip->setLauterDeadspace_l    (lineEdit_lauterDeadspace->toCanonical().quantity() );
   this->obsEquip->setGrainAbsorption_LKg  (ga_LKg );
   this->obsEquip->setBoilingPoint_c       (lineEdit_boilingPoint   ->toCanonical().quantity() );
   this->obsEquip->setHopUtilization_pct   (lineEdit_hopUtilization ->getNonOptValueAs<double>() );
   this->obsEquip->setNotes                (textEdit_notes          ->toPlainText());
   this->obsEquip->setCalcBoilVolume       (checkBox_calcBoilVolume ->checkState() == Qt::Checked);

   if (this->obsEquip->key() < 0) {
      ObjectStoreWrapper::insert(*obsEquip);
   }
   setVisible(false);
   return;
}

//void EquipmentEditor::newEquipment() {
//   newEquipment(QString());
//   return;
//}

void EquipmentEditor::newEquipment(QString folder) {
   QString name = QInputDialog::getText(this, tr("Equipment name"),
                                          tr("Equipment name:"));
   if( name.isEmpty() )
      return;

   // .:TODO:. Change to shared_ptr as currently leads to memory leak
   Equipment* e = new Equipment(name);

   if ( ! folder.isEmpty() )
      e->setFolder(folder);

   setEquipment(e);
   show();
}

void EquipmentEditor::cancel() {
   setEquipment(obsEquip);

   setVisible(false);
   return;
}

void EquipmentEditor::resetAbsorption() {
   if( obsEquip == nullptr )
      return;

   // Get weight and volume units for grain absorption.
   Measurement::Unit const * weightUnit = nullptr;
   Measurement::Unit const * volumeUnit = nullptr;
   Measurement::getThicknessUnits(&volumeUnit, &weightUnit);
   double gaCustomUnits = PhysicalConstants::grainAbsorption_Lkg * volumeUnit->fromCanonical(1.0) * weightUnit->toCanonical(1.0).quantity();

   lineEdit_grainAbsorption->setAmount(gaCustomUnits);
   return;
}

void EquipmentEditor::changed(QMetaProperty /*prop*/, QVariant /*val*/) {
   if( sender() == obsEquip ) {
      showChanges();
   }
   return;
}

void EquipmentEditor::showChanges() {
   if( this->obsEquip == nullptr ) {
      clear();
      return;
   }

   // Get weight and volume units for grain absorption.
   Measurement::Unit const * weightUnit = nullptr;
   Measurement::Unit const * volumeUnit = nullptr;
   Measurement::getThicknessUnits( &volumeUnit, &weightUnit );
   this->label_grainAbsorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->name).arg(weightUnit->name));

   //equipmentComboBox->setIndexByEquipment(this->obsEquip);

   this->lineEdit_name            ->setText(this->obsEquip->name());
   this->lineEdit_name            ->setCursorPosition(0);
   this->tabWidget_editor         ->setTabText(0, this->obsEquip->name());
   this->lineEdit_boilSize        ->setAmount(this->obsEquip->boilSize_l());

   this->checkBox_calcBoilVolume  ->blockSignals(true); // Keep next line from emitting a signal and changing this->obsEquip.
   this->checkBox_calcBoilVolume  ->setCheckState( (this->obsEquip->calcBoilVolume())? Qt::Checked : Qt::Unchecked );
   this->checkBox_calcBoilVolume  ->blockSignals(false);

   this->lineEdit_batchSize       ->setAmount(this->obsEquip->batchSize_l          ());
   this->lineEdit_tunVolume       ->setAmount(this->obsEquip->tunVolume_l          ());
   this->lineEdit_tunWeight       ->setAmount(this->obsEquip->tunWeight_kg         ());
   this->lineEdit_tunSpecificHeat ->setAmount(this->obsEquip->tunSpecificHeat_calGC());
   this->lineEdit_boilTime        ->setAmount(this->obsEquip->boilTime_min         ());
   this->lineEdit_evaporationRate ->setAmount(this->obsEquip->evapRate_lHr         ());
   this->lineEdit_topUpKettle     ->setAmount(this->obsEquip->topUpKettle_l        ());
   this->lineEdit_topUpWater      ->setAmount(this->obsEquip->topUpWater_l         ());
   this->lineEdit_trubChillerLoss ->setAmount(this->obsEquip->trubChillerLoss_l    ());
   this->lineEdit_lauterDeadspace ->setAmount(this->obsEquip->lauterDeadspace_l    ());
   this->textEdit_notes           ->setText  (this->obsEquip->notes                ());

   double gaCustomUnits = this->obsEquip->grainAbsorption_LKg() * volumeUnit->fromCanonical(1.0) * weightUnit->toCanonical(1.0).quantity();
   this->lineEdit_grainAbsorption ->setAmount(gaCustomUnits);

   this->lineEdit_boilingPoint    ->setAmount(this->obsEquip->boilingPoint_c    ());
   this->lineEdit_hopUtilization  ->setAmount(this->obsEquip->hopUtilization_pct());
   this->checkBox_defaultEquipment->blockSignals(true);
   if (PersistentSettings::value(PersistentSettings::Names::defaultEquipmentKey, -1) == this->obsEquip->key()) {
      checkBox_defaultEquipment->setCheckState(Qt::Checked);
   } else {
      checkBox_defaultEquipment->setCheckState(Qt::Unchecked);
   }
   checkBox_defaultEquipment->blockSignals(false);

   return;
}

void EquipmentEditor::updateCheckboxRecord() {
   if (Qt::Checked == this->checkBox_calcBoilVolume->checkState()) {
      this->lineEdit_boilSize->setAmount(this->calcBatchSize());
      this->lineEdit_boilSize->setEnabled(false);
   } else {
      this->lineEdit_boilSize->setAmount(this->lineEdit_batchSize->toCanonical().quantity());
      this->lineEdit_boilSize->setEnabled(true);
   }
   return;
}

double EquipmentEditor::calcBatchSize() {
   double size     = lineEdit_batchSize      ->toCanonical().quantity();
   double topUp    = lineEdit_topUpWater     ->toCanonical().quantity();
   double trubLoss = lineEdit_trubChillerLoss->toCanonical().quantity();
   double evapRate = lineEdit_evaporationRate->toCanonical().quantity();
   double time     = lineEdit_boilTime       ->toCanonical().quantity();

   return size - topUp + trubLoss + (time/60.0)*evapRate;
}

void EquipmentEditor::updateDefaultEquipment(int state) {
   QVariant currentDefault = PersistentSettings::value(PersistentSettings::Names::defaultEquipmentKey, -1);
   if ( state == Qt::Checked ) {
      PersistentSettings::insert(PersistentSettings::Names::defaultEquipmentKey, obsEquip->key());
   } else if ( currentDefault == obsEquip->key() ) {
      PersistentSettings::insert(PersistentSettings::Names::defaultEquipmentKey, -1);
   }
   return;
}

void EquipmentEditor::closeEvent(QCloseEvent *event) {
   cancel();
   event->accept();
   return;
}
