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
#include <QIcon>
#include <QMessageBox>

#include "equipment.h"
#include "EquipmentEditor.h"
#include "EquipmentListModel.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"
#include "HeatCalculations.h"

EquipmentEditor::EquipmentEditor(QWidget* parent)
   : QDialog(parent)
{
   setupUi(this);

   // Set grain absorption label based on units.
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   label_absorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->getUnitName()).arg(weightUnit->getUnitName()));
   
   equipmentListModel = new EquipmentListModel(equipmentComboBox);
   equipmentComboBox->setModel(equipmentListModel);
   
   obsEquip = 0;
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
   connect(textEdit_notes, SIGNAL(textChanged()), this, SLOT(changedText()));
   textEdit_notes->installEventFilter(this);

   // The checkbox is the odd thing out
   connect(checkBox_calcBoilVolume, SIGNAL(stateChanged(int)), this, SLOT(updateCheckboxRecord(int)));
}

void EquipmentEditor::setEquipment( Equipment* e )
{
   if( obsEquip )
      disconnect( obsEquip, 0, this, 0 );
   
   if( e )
   {
      obsEquip = e;
      connect( obsEquip, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void EquipmentEditor::removeEquipment()
{
   if( obsEquip )
      Database::instance().removeEquipment(obsEquip);

   equipmentComboBox->setCurrentIndex(-1);
   setEquipment(0);
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
}

void EquipmentEditor::equipmentSelected( const QString& /*text*/ )
{
   setEquipment( equipmentListModel->at(equipmentComboBox->currentIndex()) );
}

void EquipmentEditor::save()
{
   // NOTE: this isn't actually doing any saving...that's already been done.
   if( obsEquip == 0 )
   {
      setVisible(false);
      return;
   }

   // Do some checks...
   if( obsEquip->tunVolume_l() <= 0.001 )
      QMessageBox::warning(this, tr("Tun Volume Warning"), tr("The tun volume you entered is 0. This may cause problems."));
   if( obsEquip->batchSize_l() <= 0.001 )
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

   Equipment* e = Database::instance().newEquipment();
   e->setName( name );

   setEquipment(e);
}

void EquipmentEditor::clearAndClose()
{
   clear();
   setVisible(false);
}  

void EquipmentEditor::resetAbsorption()
{
   if( obsEquip == 0 )
      return;
   
   // Get weight and volume units for grain absorption.
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   double gaCustomUnits = HeatCalculations::absorption_LKg * volumeUnit->fromSI(1.0) * weightUnit->toSI(1.0);
   
   obsEquip->setGrainAbsorption_LKg( HeatCalculations::absorption_LKg );
   lineEdit_grainAbsorption->setText(QString("%1").arg(gaCustomUnits, 0, 'f', 3));
}

void EquipmentEditor::changed(QMetaProperty /*prop*/, QVariant /*val*/)
{
   if( sender() == obsEquip )
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

   // Get weight and volume units for grain absorption.
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   label_absorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->getUnitName()).arg(weightUnit->getUnitName()));

   //equipmentComboBox->setIndexByEquipment(e);

   lineEdit_name->setText(e->name());
   lineEdit_name->setCursorPosition(0);
   lineEdit_boilSize->setText( Brewtarget::displayAmount(e->boilSize_l(), Units::liters) );
   checkBox_calcBoilVolume->setCheckState( (e->calcBoilVolume())? Qt::Checked : Qt::Unchecked );
   lineEdit_batchSize->setText( Brewtarget::displayAmount(e->batchSize_l(), Units::liters) );

   lineEdit_tunVolume->setText( Brewtarget::displayAmount(e->tunVolume_l(), Units::liters) );
   lineEdit_tunWeight->setText( Brewtarget::displayAmount(e->tunWeight_kg(), Units::kilograms) );
   lineEdit_tunSpecificHeat->setText( Brewtarget::displayAmount(e->tunSpecificHeat_calGC(), 0) );

   lineEdit_boilTime->setText( Brewtarget::displayAmount(e->boilTime_min(), Units::minutes) );
   lineEdit_evaporationRate->setText( Brewtarget::displayAmount(e->evapRate_lHr(), Units::liters) );
   lineEdit_topUpKettle->setText( Brewtarget::displayAmount(e->topUpKettle_l(), Units::liters) );
   lineEdit_topUpWater->setText( Brewtarget::displayAmount(e->topUpWater_l(), Units::liters) );

   lineEdit_trubChillerLoss->setText( Brewtarget::displayAmount(e->trubChillerLoss_l(), Units::liters) );
   lineEdit_lauterDeadspace->setText( Brewtarget::displayAmount(e->lauterDeadspace_l(), Units::liters) );

   textEdit_notes->setText( e->notes() );

   double gaCustomUnits = e->grainAbsorption_LKg() * volumeUnit->fromSI(1.0) * weightUnit->toSI(1.0);
   lineEdit_grainAbsorption->setText( QString("%1").arg( gaCustomUnits, 0, 'f', 3) );
   
   lineEdit_boilingPoint->setText( Brewtarget::displayAmount(e->boilingPoint_c(), Units::celsius) );
}

void EquipmentEditor::updateRecord()
{
   QObject* selection = sender();

   if( obsEquip == 0 )
      return;
 
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );

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
   else if ( selection == lineEdit_trubChillerLoss )
      obsEquip->setTrubChillerLoss_l( Brewtarget::volQStringToSI(lineEdit_trubChillerLoss->text()) );
   else if ( selection == lineEdit_lauterDeadspace )
      obsEquip->setLauterDeadspace_l( Brewtarget::volQStringToSI(lineEdit_lauterDeadspace->text()) );
   else if ( selection == lineEdit_grainAbsorption )
   {
      double ga_LKg = lineEdit_grainAbsorption->text().toDouble() * volumeUnit->toSI(1.0) * weightUnit->fromSI(1.0);
      obsEquip->setGrainAbsorption_LKg( ga_LKg );
   }
   else if ( selection == lineEdit_boilingPoint )
      obsEquip->setBoilingPoint_c( Brewtarget::tempQStringToSI(lineEdit_boilingPoint->text()));

   showChanges();
}

void EquipmentEditor::updateCheckboxRecord(int state)
{
   obsEquip->setCalcBoilVolume(state == Qt::Checked);
}

void EquipmentEditor::changedText()
{
   Equipment* e = obsEquip;
   changeText = (e->notes() != textEdit_notes->toPlainText());
}

bool EquipmentEditor::eventFilter(QObject *object, QEvent* event)
{
   QTextEdit *textptr;
   if ( event->type() == QEvent::FocusOut )
   {
      textptr = qobject_cast<QTextEdit*>(object);
      if( textptr )
      {
         obsEquip->setNotes(textptr->toPlainText());
         changeText = false;
      }
   }
   return false;
}

