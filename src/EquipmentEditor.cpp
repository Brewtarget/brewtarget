/*
 * EquipmentEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - David Grundberg <individ@acc.umu.se>
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

#include <QInputDialog>
#include <QIcon>
#include <QMessageBox>
#include <QDebug>
#include <QCloseEvent>

#include "BtLineEdit.h"
#include "BtLabel.h"

#include "database.h"
#include "model/Equipment.h"
#include "EquipmentEditor.h"
#include "EquipmentListModel.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"
#include "HeatCalculations.h"
#include "PhysicalConstants.h"
#include "NamedEntitySortProxyModel.h"
#include "BtHorizontalTabs.h"

EquipmentEditor::EquipmentEditor(QWidget* parent, bool singleEquipEditor)
   : QDialog(parent)
{
   setupUi(this);

   if( singleEquipEditor )
   {
      //horizontalLayout_equipments->setVisible(false);
      for(int i = 0; i < horizontalLayout_equipments->count(); ++i)
      {
         QWidget* w = horizontalLayout_equipments->itemAt(i)->widget();
         if(w)
            w->setVisible(false);
      }
   }

   this->tabWidget_editor->tabBar()->setStyle( new BtHorizontalTabs );
   // Set grain absorption label based on units.
   Unit* weightUnit = nullptr;
   Unit* volumeUnit = nullptr;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   label_absorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->getUnitName()).arg(weightUnit->getUnitName()));

   equipmentListModel = new EquipmentListModel(equipmentComboBox);
   equipmentSortProxyModel = new NamedEntitySortProxyModel(equipmentListModel);
   equipmentComboBox->setModel(equipmentSortProxyModel);

   obsEquip = nullptr;

   // Connect all the edit boxen
   connect(lineEdit_boilTime,       &BtLineEdit::textModified, this,&EquipmentEditor::updateCheckboxRecord);
   connect(lineEdit_evaporationRate,&BtLineEdit::textModified, this,&EquipmentEditor::updateCheckboxRecord);
   connect(lineEdit_topUpWater,     &BtLineEdit::textModified, this,&EquipmentEditor::updateCheckboxRecord);
   connect(lineEdit_trubChillerLoss,&BtLineEdit::textModified, this,&EquipmentEditor::updateCheckboxRecord);
   connect(lineEdit_batchSize,      &BtLineEdit::textModified, this,&EquipmentEditor::updateCheckboxRecord);

   // Set up the buttons
   connect( pushButton_save, &QAbstractButton::clicked, this, &EquipmentEditor::save );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newEquipment() ) );
   connect( pushButton_cancel, &QAbstractButton::clicked, this, &EquipmentEditor::cancel );
   connect( pushButton_remove, &QAbstractButton::clicked, this, &EquipmentEditor::removeEquipment );
   connect( pushButton_absorption, &QAbstractButton::clicked, this, &EquipmentEditor::resetAbsorption );

   connect( equipmentComboBox, SIGNAL(activated(const QString&)), this, SLOT( equipmentSelected() ) );

   // Check boxen
   connect(checkBox_calcBoilVolume,   &QCheckBox::stateChanged, this, &EquipmentEditor::updateCheckboxRecord);
   connect(checkBox_defaultEquipment, &QCheckBox::stateChanged, this, &EquipmentEditor::updateDefaultEquipment);

   // QMetaObject::connectSlotsByName(this);

   // make sure the dialog gets populated the first time it's opened from the menu
   equipmentSelected();
   // Ensure correct state of Boil Volume edit box.
   updateCheckboxRecord();
}

void EquipmentEditor::setEquipment( Equipment* e )
{
   if( e )
   {
      obsEquip = e;

      // Make sure the combo box gets set to the right place.
      QModelIndex modelIndex(equipmentListModel->find(e));
      QModelIndex viewIndex(equipmentSortProxyModel->mapFromSource(modelIndex));
      if( viewIndex.isValid() )
         equipmentComboBox->setCurrentIndex(viewIndex.row());

      showChanges();
   }
}

void EquipmentEditor::removeEquipment()
{
   if( obsEquip )
      Database::instance().remove(obsEquip);

   equipmentComboBox->setCurrentIndex(-1);
   setEquipment(nullptr);
}

void EquipmentEditor::clear()
{
   lineEdit_name->setText(QString(""));
   lineEdit_name->setCursorPosition(0);
   lineEdit_boilSize->setText(QString(""));
   checkBox_calcBoilVolume->setCheckState( Qt::Unchecked );
   lineEdit_batchSize->setText(QString(""));

   lineEdit_tunVolume->setText(QString(""));
   lineEdit_tunWeight->setText(QString(""));
   lineEdit_tunSpecificHeat->setText(QString(""));

   lineEdit_boilTime->setText(QString(""));
   lineEdit_evaporationRate->setText(QString(""));
   lineEdit_topUpKettle->setText(QString(""));
   lineEdit_topUpWater->setText(QString(""));

   lineEdit_trubChillerLoss->setText(QString(""));
   lineEdit_lauterDeadspace->setText(QString(""));

   lineEdit_hopUtilization->setText(QString(""));
   textEdit_notes->setText("");

   lineEdit_grainAbsorption->setText(QString(""));
}

void EquipmentEditor::equipmentSelected()
{
   QModelIndex modelIndex;
   QModelIndex viewIndex(
      equipmentComboBox->model()->index(equipmentComboBox->currentIndex(),0)
   );

   modelIndex = equipmentSortProxyModel->mapToSource(viewIndex);

   setEquipment( equipmentListModel->at(modelIndex.row()) );
}

void EquipmentEditor::save()
{
   if( obsEquip == nullptr )
   {
      setVisible(false);
      return;
   }

   Unit* weightUnit = nullptr;
   Unit* volumeUnit = nullptr;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   bool ok = false;

   double grainAbs = Brewtarget::toDouble( lineEdit_grainAbsorption->text(), &ok );
   if ( ! ok )
      qWarning() << QString("EquipmentEditor::save() could not convert %1 to double").arg(lineEdit_grainAbsorption->text());

   double ga_LKg = grainAbs * volumeUnit->toSI(1.0) * weightUnit->fromSI(1.0);

   QString message,inform,describe;
   bool problems=false;

   // Do some prewarning things. I would prefer to do this only on change, but
   // we need to be worried about new equipment too.
   message = tr("This equipment profile may break brewtarget's maths");
   inform = QString("%1%2")
            .arg(tr("The following values are not set:"))
            .arg(QString("<ul>"));
   if ( qFuzzyCompare(lineEdit_tunVolume->toSI(),0.0) ) {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("mash tun volume (all-grain and BIAB only)"));
   }

   if ( qFuzzyCompare(lineEdit_batchSize->toSI(), 0.0) ) {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("batch size"));
   }

   if ( qFuzzyCompare(lineEdit_hopUtilization->toSI(), 0.0) ) {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("hop utilization"));
   }
   inform = inform + QString("</ul");

   if ( problems ) {
      QMessageBox theQuestion;
      int retcon;

      theQuestion.setWindowTitle( tr("Calculation Warnings") );
      theQuestion.setText( message );
      theQuestion.setInformativeText( inform );
      theQuestion.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
      theQuestion.setDefaultButton(QMessageBox::Save);
      theQuestion.setIcon(QMessageBox::Warning);

      retcon = theQuestion.exec();
      if ( retcon == QMessageBox::Cancel )
         return;
   }

   obsEquip->setName( lineEdit_name->text(), obsEquip->cacheOnly() );

   obsEquip->setBoilSize_l( lineEdit_boilSize->toSI() );
   obsEquip->setBatchSize_l( lineEdit_batchSize->toSI() );
   obsEquip->setTunVolume_l( lineEdit_tunVolume->toSI() );

   obsEquip->setTunWeight_kg( lineEdit_tunWeight->toSI() );

   obsEquip->setTunSpecificHeat_calGC( lineEdit_tunSpecificHeat->toSI() );
   obsEquip->setBoilTime_min( lineEdit_boilTime->toSI());
   obsEquip->setEvapRate_lHr(  lineEdit_evaporationRate->toSI() );
   obsEquip->setTopUpKettle_l( lineEdit_topUpKettle->toSI() );
   obsEquip->setTopUpWater_l(  lineEdit_topUpWater->toSI() );
   obsEquip->setTrubChillerLoss_l( lineEdit_trubChillerLoss->toSI() );
   obsEquip->setLauterDeadspace_l( lineEdit_lauterDeadspace->toSI() );
   obsEquip->setGrainAbsorption_LKg( ga_LKg );
   obsEquip->setBoilingPoint_c( lineEdit_boilingPoint->toSI() );
   obsEquip->setHopUtilization_pct( lineEdit_hopUtilization->toSI() );

   obsEquip->setNotes(textEdit_notes->toPlainText());
   obsEquip->setCalcBoilVolume(checkBox_calcBoilVolume->checkState() == Qt::Checked);

   if ( obsEquip->cacheOnly() ) {
      obsEquip->insertInDatabase();
   }
   setVisible(false);
   return;
}

void EquipmentEditor::newEquipment()
{
   newEquipment(QString());
}

void EquipmentEditor::newEquipment(QString folder)
{
   QString name = QInputDialog::getText(this, tr("Equipment name"),
                                          tr("Equipment name:"));
   if( name.isEmpty() )
      return;

   Equipment* e = new Equipment(name);

   if ( ! folder.isEmpty() )
      e->setFolder(folder);

   setEquipment(e);
   show();
}

void EquipmentEditor::cancel()
{
   setEquipment(obsEquip);

   setVisible(false);
}

void EquipmentEditor::resetAbsorption()
{
   if( obsEquip == nullptr )
      return;

   // Get weight and volume units for grain absorption.
   Unit* weightUnit = nullptr;
   Unit* volumeUnit = nullptr;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   double gaCustomUnits = PhysicalConstants::grainAbsorption_Lkg * volumeUnit->fromSI(1.0) * weightUnit->toSI(1.0);

   lineEdit_grainAbsorption->setText(gaCustomUnits);
}

void EquipmentEditor::changed(QMetaProperty /*prop*/, QVariant /*val*/)
{
   if( sender() == obsEquip )
      showChanges();
}

void EquipmentEditor::showChanges()
{
   Equipment *e = obsEquip;
   if( e == nullptr ) {
      clear();
      return;
   }

   // Get weight and volume units for grain absorption.
   Unit* weightUnit = nullptr;
   Unit* volumeUnit = nullptr;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   label_absorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->getUnitName()).arg(weightUnit->getUnitName()));

   //equipmentComboBox->setIndexByEquipment(e);

   lineEdit_name->setText(e->name());
   lineEdit_name->setCursorPosition(0);
   tabWidget_editor->setTabText(0, e->name() );
   lineEdit_boilSize->setText(e->boilSize_l());

   checkBox_calcBoilVolume->blockSignals(true); // Keep next line from emitting a signal and changing e.
   checkBox_calcBoilVolume->setCheckState( (e->calcBoilVolume())? Qt::Checked : Qt::Unchecked );
   checkBox_calcBoilVolume->blockSignals(false);

   lineEdit_batchSize->setText(e);
   lineEdit_tunVolume->setText(e);
   lineEdit_tunWeight->setText(e);
   lineEdit_tunSpecificHeat->setText(e);

   lineEdit_boilTime->setText(e);
   lineEdit_evaporationRate->setText(e);
   lineEdit_topUpKettle->setText(e);
   lineEdit_topUpWater->setText(e);

   lineEdit_trubChillerLoss->setText(e);
   lineEdit_lauterDeadspace->setText(e);

   textEdit_notes->setText( e->notes() );

   double gaCustomUnits = e->grainAbsorption_LKg() * volumeUnit->fromSI(1.0) * weightUnit->toSI(1.0);
   lineEdit_grainAbsorption->setText(gaCustomUnits);

   lineEdit_boilingPoint->setText(e);

   lineEdit_hopUtilization->setText(e);
   checkBox_defaultEquipment->blockSignals(true);
   if ( Brewtarget::option("defaultEquipmentKey",-1) == e->key() )
      checkBox_defaultEquipment->setCheckState(Qt::Checked);
   else
      checkBox_defaultEquipment->setCheckState(Qt::Unchecked);
   checkBox_defaultEquipment->blockSignals(false);
}

void EquipmentEditor::updateCheckboxRecord()
{
   int state = checkBox_calcBoilVolume->checkState();
   if ( state == Qt::Checked )
   {
      double bar = calcBatchSize();
      lineEdit_boilSize->setText(bar);
      lineEdit_boilSize->setEnabled(false);
   }
   else {
      lineEdit_boilSize->setText(lineEdit_batchSize->toSI());
      lineEdit_boilSize->setEnabled(true);
   }
}

double EquipmentEditor::calcBatchSize()
{
   double size, topUp, trubLoss, time, evapRate;
   size     = lineEdit_batchSize->toSI();
   topUp    = lineEdit_topUpWater->toSI();
   trubLoss = lineEdit_trubChillerLoss->toSI();
   evapRate = lineEdit_evaporationRate->toSI();
   time     = lineEdit_boilTime->toSI();

   return size - topUp + trubLoss + (time/(double)60.0)*evapRate;
}

void EquipmentEditor::updateDefaultEquipment(int state)
{
   QString optionName = "defaultEquipmentKey";

   QVariant currentDefault = Brewtarget::option(optionName, -1);
   if ( state == Qt::Checked )
   {
      Brewtarget::setOption(optionName, obsEquip->key());
   }
   else if ( currentDefault == obsEquip->key() )
   {
      Brewtarget::setOption(optionName,-1);
   }
}

void EquipmentEditor::closeEvent(QCloseEvent *event)
{
   cancel();
   event->accept();
}
