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

   // Set grain absorption label based on units.
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   label_absorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->getUnitName()).arg(weightUnit->getUnitName()));
   
   equipmentComboBox->startObservingDB();
   obsEquip = 0;
   copyEquip = 0;
   changeText = false;

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
   connect(lineEdit_trubChillerLoss,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_lauterDeadspace,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_grainAbsorption,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_grainAbsorption,SIGNAL(editingFinished()),this,SLOT(updateRecord()));
   connect(lineEdit_boilingPoint,SIGNAL(editingFinished()),this,SLOT(updateRecord()));

   // Doing the text properly is actually a two step thing. The first is to
   // connect to the textChanged() signal, but only to set a "has changed" flag. The second
   // is to use an event filter to actually handle the updates.
   connect(textEdit_notes, SIGNAL(textChanged()),this,SLOT(changedText()));
   textEdit_notes->installEventFilter(this);

   // The checkbox is the odd thing out
   connect(checkBox_calcBoilVolume, SIGNAL(stateChanged(int)), this, SLOT(updateCheckboxRecord(int)));
}

void EquipmentEditor::setEquipment( Equipment* e )
{
   if( e && e != obsEquip )
   {
      copyEquip = 0;
      obsEquip = e;
      setObserved(obsEquip);
      showChanges();
   }
}

void EquipmentEditor::resetEquipment()
{
   if( obsEquip )
   {
      copyEquip = 0;
      setObserved(obsEquip);
      showChanges();
   }
}

void EquipmentEditor::removeEquipment()
{
   if( obsEquip )
      Database::getDatabase()->removeEquipment(obsEquip);

   obsEquip = 0;
   copyEquip = 0;

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

   lineEdit_trubChillerLoss->setText(QString(""));
   lineEdit_lauterDeadspace->setText(QString(""));

   textEdit_notes->setText("");

   lineEdit_grainAbsorption->setText("");

   copyEquip = 0;
}

void EquipmentEditor::equipmentSelected( const QString& /*text*/ )
{
   setEquipment( equipmentComboBox->getSelected() );
   copyEquip = 0;
}

/* This works because we make a deep copy on the first write. So all of the
 * values are copied from the orginal to copyEquip. When we set them all back,
 * any unchanged value is still there.
*/
void EquipmentEditor::save()
{
   // Not sure about returning if copyEquip isn't set. But that should imply
   // no changes were made, so there is nothing to save.
   if( obsEquip == 0 || copyEquip == 0 )
   {
      setVisible(false);
      return;
   }

   obsEquip->disableNotification();

   obsEquip->setName(copyEquip->getName());
   obsEquip->setBoilSize_l(copyEquip->getBoilSize_l());
   
   obsEquip->setCalcBoilVolume(copyEquip->getCalcBoilVolume());
   obsEquip->setBatchSize_l(copyEquip->getBatchSize_l());

   obsEquip->setTunVolume_l(copyEquip->getTunVolume_l());
   obsEquip->setTunWeight_kg(copyEquip->getTunWeight_kg());
   obsEquip->setTunSpecificHeat_calGC(copyEquip->getTunSpecificHeat_calGC());

   obsEquip->setBoilTime_min(copyEquip->getBoilTime_min());
   obsEquip->setEvapRate_lHr(copyEquip->getEvapRate_lHr());
   obsEquip->setTopUpKettle_l( copyEquip->getTopUpKettle_l());
   obsEquip->setTopUpWater_l( copyEquip->getTopUpWater_l());

   obsEquip->setTrubChillerLoss_l( copyEquip->getTrubChillerLoss_l());
   obsEquip->setLauterDeadspace_l( copyEquip->getLauterDeadspace_l());

   obsEquip->setNotes(copyEquip->getNotes());
   
   
   obsEquip->setGrainAbsorption_LKg( copyEquip->getGrainAbsorption_LKg());
   obsEquip->setBoilingPoint_c( copyEquip->getBoilingPoint_c());

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
   if ( copyEquip ) 
      resetEquipment();
   setVisible(false);
}

void EquipmentEditor::resetAbsorption()
{
   
   if( copyEquip == 0 )
   {
      if ( obsEquip == 0 )
         return;

      copyEquip = new Equipment(obsEquip);
   }

   // Get weight and volume units for grain absorption.
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   double gaCustomUnits = HeatCalculations::absorption_LKg * volumeUnit->fromSI(1.0) * weightUnit->toSI(1.0);
   
   copyEquip->setGrainAbsorption_LKg( HeatCalculations::absorption_LKg );
   lineEdit_grainAbsorption->setText(QString("%1").arg(gaCustomUnits, 0, 'f', 3));
}

void EquipmentEditor::notify(Observable* /*notifier*/, QVariant info)
{
   showChanges();
}

void EquipmentEditor::showChanges()
{
   Equipment *e;

   if ( copyEquip == 0 )
      e = obsEquip;
   else
      e = copyEquip;

   if( e == 0 )
   {
      clear();
      return;
   }

   // Get weight and volume units for grain absorption.
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   label_absorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->getUnitName()).arg(weightUnit->getUnitName()));

   equipmentComboBox->setIndexByEquipmentName(e->getName());

   lineEdit_name->setText(e->getName());
   lineEdit_name->setCursorPosition(0);
   lineEdit_boilSize->setText( Brewtarget::displayAmount(e->getBoilSize_l(), Units::liters) );
   checkBox_calcBoilVolume->setCheckState( (e->getCalcBoilVolume())? Qt::Checked : Qt::Unchecked );
   lineEdit_batchSize->setText( Brewtarget::displayAmount(e->getBatchSize_l(), Units::liters) );

   lineEdit_tunVolume->setText( Brewtarget::displayAmount(e->getTunVolume_l(), Units::liters) );
   lineEdit_tunWeight->setText( Brewtarget::displayAmount(e->getTunWeight_kg(), Units::kilograms) );
   lineEdit_tunSpecificHeat->setText( Brewtarget::displayAmount(e->getTunSpecificHeat_calGC(), 0) );

   lineEdit_boilTime->setText( Brewtarget::displayAmount(e->getBoilTime_min(), Units::minutes) );
   lineEdit_evaporationRate->setText( Brewtarget::displayAmount(e->getEvapRate_lHr(), Units::liters) );
   lineEdit_topUpKettle->setText( Brewtarget::displayAmount(e->getTopUpKettle_l(), Units::liters) );
   lineEdit_topUpWater->setText( Brewtarget::displayAmount(e->getTopUpWater_l(), Units::liters) );

   lineEdit_trubChillerLoss->setText( Brewtarget::displayAmount(e->getTrubChillerLoss_l(), Units::liters) );
   lineEdit_lauterDeadspace->setText( Brewtarget::displayAmount(e->getLauterDeadspace_l(), Units::liters) );

   textEdit_notes->setText( e->getNotes() );

   double gaCustomUnits = e->getGrainAbsorption_LKg() * volumeUnit->fromSI(1.0) * weightUnit->toSI(1.0);
   lineEdit_grainAbsorption->setText( QString("%1").arg( gaCustomUnits, 0, 'f', 3) );
   
   lineEdit_boilingPoint->setText( Brewtarget::displayAmount(e->getBoilingPoint_c(), Units::celsius) );
}

void EquipmentEditor::updateRecord()
{
   QObject *selection = sender();

   if( obsEquip == 0 )
      return;
 
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   
   // First change, perform the deep copy
   if ( copyEquip == 0 )
      copyEquip = new Equipment( obsEquip );

   if ( selection == lineEdit_name )
      copyEquip->setName( lineEdit_name->text() );
   else if ( selection == lineEdit_boilSize )
      copyEquip->setBoilSize_l( Brewtarget::volQStringToSI(lineEdit_boilSize->text()) );
   else if ( selection == lineEdit_batchSize )
      copyEquip->setBatchSize_l( Brewtarget::volQStringToSI(lineEdit_batchSize->text()) );
   else if ( selection == lineEdit_tunVolume )
      copyEquip->setTunVolume_l( Brewtarget::volQStringToSI(lineEdit_tunVolume->text()) );
   else if ( selection == lineEdit_tunWeight )
      copyEquip->setTunWeight_kg( Brewtarget::weightQStringToSI(lineEdit_tunWeight->text()) );
   else if ( selection == lineEdit_tunSpecificHeat )
      copyEquip->setTunSpecificHeat_calGC( lineEdit_tunSpecificHeat->text().toDouble() );
   else if ( selection == lineEdit_boilTime )
      copyEquip->setBoilTime_min( Brewtarget::timeQStringToSI(lineEdit_boilTime->text()) );
   else if ( selection == lineEdit_evaporationRate )
      copyEquip->setEvapRate_lHr( Brewtarget::volQStringToSI(lineEdit_evaporationRate->text()) );
   else if ( selection == lineEdit_topUpKettle )
      copyEquip->setTopUpKettle_l( Brewtarget::volQStringToSI(lineEdit_topUpKettle->text()) );
   else if ( selection == lineEdit_topUpWater )
      copyEquip->setTopUpWater_l( Brewtarget::volQStringToSI(lineEdit_topUpWater->text()) );
   else if ( selection == lineEdit_trubChillerLoss )
      copyEquip->setTrubChillerLoss_l( Brewtarget::volQStringToSI(lineEdit_trubChillerLoss->text()) );
   else if ( selection == lineEdit_lauterDeadspace )
      copyEquip->setLauterDeadspace_l( Brewtarget::volQStringToSI(lineEdit_lauterDeadspace->text()) );
   else if ( selection == lineEdit_grainAbsorption )
   {
      double ga_LKg = lineEdit_grainAbsorption->text().toDouble() * volumeUnit->toSI(1.0) * weightUnit->fromSI(1.0);
      copyEquip->setGrainAbsorption_LKg( ga_LKg );
   }
   else if ( selection == lineEdit_boilingPoint )
      copyEquip->setBoilingPoint_c( Brewtarget::tempQStringToSI(lineEdit_boilingPoint->text()));

   showChanges();
}

void EquipmentEditor::updateCheckboxRecord(int state)
{
   if ( copyEquip == 0 )
      copyEquip = new Equipment( obsEquip );

   copyEquip->setCalcBoilVolume(state == Qt::Checked);
}

void EquipmentEditor::changedText()
{
   Equipment* e = copyEquip == 0 ? obsEquip : copyEquip;

   changeText = (e->getNotes() != textEdit_notes->toPlainText());
}

bool EquipmentEditor::eventFilter(QObject *object, QEvent* event)
{
   QTextEdit *textptr;
   if ( event->type() == QEvent::FocusOut )
   {
      if ( copyEquip == 0 )
         copyEquip = new Equipment( obsEquip );

      textptr = qobject_cast<QTextEdit*>(object);
      if( textptr )
      {
         copyEquip->setNotes( textptr->toPlainText());
         changeText = false;
      }
   }
   
   return false;
}

