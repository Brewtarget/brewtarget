/*
 * EquipmentEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

   obsEquip->setName( lineEdit_name->text().toStdString() );
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

   obsEquip->setNotes(textEdit_notes->toPlainText().toStdString());
   obsEquip->setGrainAbsorption_LKg( lineEdit_grainAbsorption->text().toDouble() );

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
   e->setName( name.toStdString() );

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

   lineEdit_name->setText(e->getName().c_str());
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

   textEdit_notes->setText(e->getNotes().c_str());

   lineEdit_grainAbsorption->setText(QString("%1").arg(e->getGrainAbsorption_LKg(), 0, 'f', 3));
}
