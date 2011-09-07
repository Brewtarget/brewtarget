/*
 * EquipmentEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QInputDialog>
#include <string>
#include <iostream>
#include <QIcon>
#include <QMessageBox>

#include "equipment.h"
#include "EquipmentEditor.h"
#include "EquipmentComboBox.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"
#include "HeatCalculations.h"

EquipmentEditor::EquipmentEditor(QWidget* parent)
        : QDialog(parent)
{
   setupUi(this);

   setWindowIcon(QIcon(SMALLKETTLE));

   equipmentComboBox->startObservingDB();
   obsEquip = 0;

   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newEquipment() ) );
   connect( pushButton_cancel, SIGNAL( clicked() ), this, SLOT( clearAndClose() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT( removeEquipment() ) );
   connect( pushButton_absorption, SIGNAL( clicked() ), this, SLOT( resetAbsorption() ) );
   connect( equipmentComboBox, SIGNAL(currentIndexChanged ( const QString& )), this, SLOT( equipmentSelected(const QString&) ) );

   // Connect all the edit boxen
   connect(lineEdit_name,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_boilSize,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_batchSize,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_tunVolume,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_tunWeight,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_tunSpecificHeat,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_boilTime,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_evaporationRate,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_topUpKettle,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_topUpWater,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_hopUtilization,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_trubChillerLoss,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_lauterDeadspace,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_grainAbsorption,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_grainAbsorption,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_boilingPoint,SIGNAL(editingFinished()),this,SLOT(updateRecord()));

   // The checkbox is the odd thing out
   connect(checkBox_calcBoilVolume, SIGNAL(stateChanged(int)), this, SLOT(updateCheckboxRecord(int)));
}

void EquipmentEditor::setEquipment( Equipment* e )
{
   if( e && e != obsEquip )
   {
      obsEquip = e;
      setObserved(obsEquip);
      showChanges();
   }
}

void EquipmentEditor::removeEquipment()
{
   if( obsEquip )
      Database::getDatabase()->removeEquipment(obsEquip);

   obsEquip = 0;
   setObserved(obsEquip);

   equipmentComboBox->setIndexByEquipmentName("");
   showChanges();
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
   lineEdit_hopUtilization->setText(QString(""));

   lineEdit_trubChillerLoss->setText(QString(""));
   lineEdit_lauterDeadspace->setText(QString(""));

   textEdit_notes->setText("");

   lineEdit_grainAbsorption->setText("");
}

void EquipmentEditor::equipmentSelected( const QString& /*text*/ )
{
   setEquipment( equipmentComboBox->getSelected() );
}

void EquipmentEditor::save()
{
   if( obsEquip == 0 )
   {
      setVisible(false);
      return;
   }

   obsEquip->disableNotification();

   obsEquip->setName( lineEdit_name->text() );
   obsEquip->setBoilSize_l( Brewtarget::volQStringToSI(lineEdit_boilSize->text()) );
   
   obsEquip->setCalcBoilVolume( (checkBox_calcBoilVolume->checkState() == Qt::Checked)? true : false );
   obsEquip->setBatchSize_l( Brewtarget::volQStringToSI(lineEdit_batchSize->text()) );

   obsEquip->setTunVolume_l( Brewtarget::volQStringToSI(lineEdit_tunVolume->text()) );
   obsEquip->setTunWeight_kg( Brewtarget::weightQStringToSI(lineEdit_tunWeight->text()) );
   obsEquip->setTunSpecificHeat_calGC( lineEdit_tunSpecificHeat->text().toDouble() );

   obsEquip->setBoilTime_min( Brewtarget::timeQStringToSI(lineEdit_boilTime->text()) );
   obsEquip->setEvapRate_lHr( Brewtarget::volQStringToSI(lineEdit_evaporationRate->text()) );
   obsEquip->setTopUpKettle_l( Brewtarget::volQStringToSI(lineEdit_topUpKettle->text()) );
   obsEquip->setTopUpWater_l( Brewtarget::volQStringToSI(lineEdit_topUpWater->text()) );
   obsEquip->setHopUtilization_pct( lineEdit_hopUtilization->text().toDouble() );

   obsEquip->setTrubChillerLoss_l( Brewtarget::volQStringToSI(lineEdit_trubChillerLoss->text()) );
   obsEquip->setLauterDeadspace_l( Brewtarget::volQStringToSI(lineEdit_lauterDeadspace->text()) );

   obsEquip->setNotes(textEdit_notes->toPlainText());
   obsEquip->setGrainAbsorption_LKg( lineEdit_grainAbsorption->text().toDouble() );
   obsEquip->setBoilingPoint_c( Brewtarget::tempQStringToSI(lineEdit_boilingPoint->text()));

   obsEquip->reenableNotification();
   obsEquip->forceNotify();

   Database::getDatabase()->resortEquipments(); // If the name changed, need to resort.

   // Do some checks...
   if( obsEquip->getTunVolume_l() <= 0.001 )
      QMessageBox::warning(this, tr("Tun Volume Warning"), tr("The tun volume you entered is 0. This may cause problems."));
   if( obsEquip->getBatchSize_l() <= 0.001 )
      QMessageBox::warning(this, tr("Batch Size Warning"), tr("The batch size you entered is 0. This may cause problems."));

   setVisible(false);
   return;
}

void EquipmentEditor::newEquipment()
{
   QString name = QInputDialog::getText(this, tr("Equipment name"),
                                          tr("Equipment name:"));
   if( name.isEmpty() )
      return;

   Equipment *e = new Equipment();
   e->setName( name );

   Database::getDatabase()->addEquipment(e);

   setEquipment(e);
}

void EquipmentEditor::clearAndClose()
{
   setVisible(false);
}

void EquipmentEditor::resetAbsorption()
{
   if( obsEquip == 0 )
      return;

   obsEquip->setGrainAbsorption_LKg( HeatCalculations::absorption_LKg );
}

void EquipmentEditor::notify(Observable* /*notifier*/, QVariant info)
{
   showChanges();
}

void EquipmentEditor::showChanges()
{
   Equipment *e = obsEquip;
   if( e == 0 )
   {
      clear();
      return;
   }

   equipmentComboBox->setIndexByEquipmentName(e->getName());

   lineEdit_name->setText(e->getName());
   lineEdit_name->setCursorPosition(0);
   lineEdit_boilSize->setText( Brewtarget::displayAmount(e->getBoilSize_l(), Units::liters) );
   checkBox_calcBoilVolume->setCheckState( (e->getCalcBoilVolume())? Qt::Checked : Qt::Unchecked );
   lineEdit_batchSize->setText( Brewtarget::displayAmount(e->getBatchSize_l(), Units::liters) );

   lineEdit_tunVolume->setText( Brewtarget::displayAmount(e->getTunVolume_l(), Units::liters) );
   lineEdit_tunWeight->setText(Brewtarget::displayAmount(e->getTunWeight_kg(), Units::kilograms));
   lineEdit_tunSpecificHeat->setText(Brewtarget::displayAmount(e->getTunSpecificHeat_calGC(), 0) );

   lineEdit_boilTime->setText(Brewtarget::displayAmount(e->getBoilTime_min(), Units::minutes) );
   lineEdit_evaporationRate->setText(Brewtarget::displayAmount(e->getEvapRate_lHr(), Units::liters) );
   lineEdit_topUpKettle->setText(Brewtarget::displayAmount(e->getTopUpKettle_l(), Units::liters) );
   lineEdit_topUpWater->setText(Brewtarget::displayAmount(e->getTopUpWater_l(), Units::liters) );
   lineEdit_hopUtilization->setText(Brewtarget::displayAmount(e->getHopUtilization_pct(), 0) );

   lineEdit_trubChillerLoss->setText(Brewtarget::displayAmount(e->getTrubChillerLoss_l(), Units::liters) );
   lineEdit_lauterDeadspace->setText(Brewtarget::displayAmount(e->getLauterDeadspace_l(), Units::liters) );

   textEdit_notes->setText(e->getNotes());

   lineEdit_grainAbsorption->setText(QString("%1").arg(e->getGrainAbsorption_LKg(), 0, 'f', 3));
   lineEdit_boilingPoint->setText(Brewtarget::displayAmount(e->getBoilingPoint_c(), Units::celsius));
}

void EquipmentEditor::updateRecord()
{
   QObject *selection = sender();

   if( obsEquip == 0 )
      return;
   
   if ( selection == lineEdit_name )
      obsEquip->setName( lineEdit_name->text() );
   else if ( selection == lineEdit_boilSize )
      obsEquip->setBoilSize_l( Brewtarget::volQStringToSI(lineEdit_boilSize->text()) );
   else if ( selection == lineEdit_batchSize )
      obsEquip->setBatchSize_l( Brewtarget::volQStringToSI(lineEdit_batchSize->text()) );
   else if ( selection == lineEdit_tunVolume )
      obsEquip->setTunVolume_l( Brewtarget::volQStringToSI(lineEdit_tunVolume->text()) );
   else if ( selection == lineEdit_tunWeight )
      obsEquip->setTunWeight_kg( Brewtarget::weightQStringToSI(lineEdit_tunWeight->text()) );
   else if ( selection == lineEdit_tunSpecificHeat )
      obsEquip->setTunSpecificHeat_calGC( lineEdit_tunSpecificHeat->text().toDouble() );
   else if ( selection == lineEdit_boilTime )
      obsEquip->setBoilTime_min( Brewtarget::timeQStringToSI(lineEdit_boilTime->text()) );
   else if ( selection == lineEdit_evaporationRate )
      obsEquip->setEvapRate_lHr( Brewtarget::volQStringToSI(lineEdit_evaporationRate->text()) );
   else if ( selection == lineEdit_topUpKettle )
      obsEquip->setTopUpKettle_l( Brewtarget::volQStringToSI(lineEdit_topUpKettle->text()) );
   else if ( selection == lineEdit_topUpWater )
      obsEquip->setTopUpWater_l( Brewtarget::volQStringToSI(lineEdit_topUpWater->text()) );
   else if ( selection == lineEdit_hopUtilization )
      obsEquip->setHopUtilization_pct( lineEdit_hopUtilization->text().toDouble() );
   else if ( selection == lineEdit_trubChillerLoss )
      obsEquip->setTrubChillerLoss_l( Brewtarget::volQStringToSI(lineEdit_trubChillerLoss->text()) );
   else if ( selection == lineEdit_lauterDeadspace )
      obsEquip->setLauterDeadspace_l( Brewtarget::volQStringToSI(lineEdit_lauterDeadspace->text()) );
   else if ( selection == lineEdit_grainAbsorption )
      obsEquip->setNotes(textEdit_notes->toPlainText());
   else if ( selection == lineEdit_grainAbsorption )
      obsEquip->setGrainAbsorption_LKg( lineEdit_grainAbsorption->text().toDouble() );
   else if ( selection == lineEdit_boilingPoint )
      obsEquip->setBoilingPoint_c( Brewtarget::tempQStringToSI(lineEdit_boilingPoint->text()));
}

void EquipmentEditor::updateCheckboxRecord(int state)
{
   obsEquip->setCalcBoilVolume(state == Qt::Checked);
}
