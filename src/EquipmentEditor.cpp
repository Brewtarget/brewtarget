/*
 * EquipmentEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include "database.h"
#include "equipment.h"
#include "EquipmentEditor.h"
#include "EquipmentListModel.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"
#include "HeatCalculations.h"
#include "PhysicalConstants.h"
#include "BeerXMLSortProxyModel.h"

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
      
      pushButton_new->setVisible(false);
   }
   
   // Set grain absorption label based on units.
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   label_absorption->setText(tr("Grain absorption (%1/%2)").arg(volumeUnit->getUnitName()).arg(weightUnit->getUnitName()));
   
   equipmentListModel = new EquipmentListModel(equipmentComboBox);
   equipmentSortProxyModel = new BeerXMLSortProxyModel(equipmentListModel);
   equipmentComboBox->setModel(equipmentSortProxyModel);
   
   obsEquip = 0;

   // Connect all the edit boxen
   connect(lineEdit_boilSize,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_batchSize,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_tunVolume,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_tunWeight,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_tunSpecificHeat,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_boilTime,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_evaporationRate,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_topUpKettle,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_topUpWater,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_trubChillerLoss,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_lauterDeadspace,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_grainAbsorption,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_boilingPoint,SIGNAL(editingFinished()),this,SLOT(updateField()));
   connect(lineEdit_hopUtilization,SIGNAL(editingFinished()),this,SLOT(updateField()));

   // Set up the buttons
   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newEquipment() ) );
   connect( pushButton_cancel, SIGNAL( clicked() ), this, SLOT( cancel() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT( removeEquipment() ) );
   connect( pushButton_absorption, SIGNAL( clicked() ), this, SLOT( resetAbsorption() ) );
   connect( equipmentComboBox, SIGNAL(activated(const QString&)), this, SLOT( equipmentSelected() ) );

   // Check boxen
   connect(checkBox_calcBoilVolume, SIGNAL(stateChanged(int)), this, SLOT(updateCheckboxRecord(int)));
   connect(checkBox_defaultEquipment, SIGNAL(stateChanged(int)), this, SLOT(updateDefaultEquipment(int)));
   
   // make sure the dialog gets populated the first time it's opened from the menu
   equipmentSelected();
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

   lineEdit_hopUtilization->setText(QString(""));
   textEdit_notes->setText("");

   lineEdit_grainAbsorption->setText("");
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
   if( obsEquip == 0 )
   {
      setVisible(false);
      return;
   }

   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );
   double ga_LKg = lineEdit_grainAbsorption->text().toDouble() * volumeUnit->toSI(1.0) * weightUnit->fromSI(1.0);

   // Do some prewarning things. I would prefer to do this only on change, but
   // we need to be worried about new equipment too.
   if ( Brewtarget::volQStringToSI(lineEdit_tunVolume->text()) <= 0.001 )
      QMessageBox::warning(this, tr("Tun Volume Warning"), tr("The tun volume you entered is 0. This may cause problems"));

   if ( Brewtarget::volQStringToSI(lineEdit_batchSize->text()) <= 0.001 )
      QMessageBox::warning(this, tr("Batch Size Warning"), tr("The batch size you entered is 0. This may cause problems"));

   if ( lineEdit_hopUtilization->text().toDouble() < 0.001 )
      QMessageBox::warning(this, tr("Hop Utilization Warning"), tr("The hop utilization percentage you entered is 0. This may cause problems"));


   obsEquip->setName( lineEdit_name->text() );
   obsEquip->setBoilSize_l( Brewtarget::volQStringToSI(lineEdit_boilSize->text()) );
   obsEquip->setBatchSize_l( Brewtarget::volQStringToSI(lineEdit_batchSize->text()) );
   obsEquip->setTunVolume_l( Brewtarget::volQStringToSI(lineEdit_tunVolume->text()) );
   obsEquip->setTunWeight_kg( Brewtarget::weightQStringToSI(lineEdit_tunWeight->text()) );
   obsEquip->setTunSpecificHeat_calGC( lineEdit_tunSpecificHeat->text().toDouble() );
   obsEquip->setBoilTime_min( Brewtarget::timeQStringToSI(lineEdit_boilTime->text()) );
   obsEquip->setEvapRate_lHr( Brewtarget::volQStringToSI(lineEdit_evaporationRate->text()) );
   obsEquip->setTopUpKettle_l( Brewtarget::volQStringToSI(lineEdit_topUpKettle->text()) );
   obsEquip->setTopUpWater_l( Brewtarget::volQStringToSI(lineEdit_topUpWater->text()) );
   obsEquip->setTrubChillerLoss_l( Brewtarget::volQStringToSI(lineEdit_trubChillerLoss->text()) );
   obsEquip->setLauterDeadspace_l( Brewtarget::volQStringToSI(lineEdit_lauterDeadspace->text()) );
   obsEquip->setGrainAbsorption_LKg( ga_LKg );
   obsEquip->setBoilingPoint_c( Brewtarget::tempQStringToSI(lineEdit_boilingPoint->text()));
   obsEquip->setHopUtilization_pct( lineEdit_hopUtilization->text().toDouble());

   obsEquip->setNotes(textEdit_notes->toPlainText());
   obsEquip->setCalcBoilVolume(checkBox_calcBoilVolume->checkState() == Qt::Checked);

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
   show();
}

void EquipmentEditor::cancel()
{
   setEquipment(obsEquip);

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
   double gaCustomUnits = PhysicalConstants::grainAbsorption_Lkg * volumeUnit->fromSI(1.0) * weightUnit->toSI(1.0);
   
   lineEdit_grainAbsorption->setText(Brewtarget::displayAmount(gaCustomUnits));
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
   checkBox_calcBoilVolume->blockSignals(true); // Keep next line from emitting a signal and changing e.
   checkBox_calcBoilVolume->setCheckState( (e->calcBoilVolume())? Qt::Checked : Qt::Unchecked );
   checkBox_calcBoilVolume->blockSignals(false);
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
   lineEdit_grainAbsorption->setText( Brewtarget::displayAmount(gaCustomUnits,0,3) );
   
   lineEdit_boilingPoint->setText( Brewtarget::displayAmount(e->boilingPoint_c(), Units::celsius) );

   lineEdit_hopUtilization->setText(Brewtarget::displayAmount(e->hopUtilization_pct(),0,1));
   checkBox_defaultEquipment->blockSignals(true);
   if ( Brewtarget::option("defaultEquipmentKey",-1) == e->key() ) 
      checkBox_defaultEquipment->setCheckState(Qt::Checked);
   else
      checkBox_defaultEquipment->setCheckState(Qt::Unchecked);
   checkBox_defaultEquipment->blockSignals(false);
}

void EquipmentEditor::updateCheckboxRecord(int state)
{
    if ( state == Qt::Checked )
    {
       double bar = calcBatchSize();
       lineEdit_boilSize->setText( Brewtarget::displayAmount(bar, Units::liters));
    }
}

double EquipmentEditor::calcBatchSize()
{
   double size, topUp, trubLoss, time, evapRate;
   size = Brewtarget::volQStringToSI(lineEdit_batchSize->text());
   topUp = Brewtarget::volQStringToSI(lineEdit_topUpWater->text());
   trubLoss = Brewtarget::volQStringToSI(lineEdit_trubChillerLoss->text());
   time = Brewtarget::timeQStringToSI(lineEdit_boilTime->text());
   evapRate = Brewtarget::volQStringToSI(lineEdit_evaporationRate->text());

   return size - topUp + trubLoss + (time/(double)60)*evapRate;
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

void EquipmentEditor::updateField()
{

   QObject* selection = sender();
   QLineEdit* field = qobject_cast<QLineEdit*>(selection);
   QString current;
   double foo;

   if( obsEquip == 0 )
      return;
 
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );

   if ( field == lineEdit_boilTime )
   {
     foo = Brewtarget::timeQStringToSI(field->text());
     field->setText(Brewtarget::displayAmount(foo, Units::minutes) );
   }
   else if ( field == lineEdit_tunWeight )
   {
     foo = Brewtarget::weightQStringToSI(field->text());
     field->setText( Brewtarget::displayAmount(foo, Units::kilograms) );
   }
   else if ( field == lineEdit_boilingPoint )
   {
      foo = Brewtarget::tempQStringToSI(field->text());
      field->setText( Brewtarget::displayAmount(foo, Units::celsius));
   }
   else if ( field == lineEdit_grainAbsorption )
   {
      // Just reformat in a pleasing fashion
      foo = lineEdit_grainAbsorption->text().toDouble(); 
      lineEdit_grainAbsorption->setText( Brewtarget::displayAmount(foo) );
   }
   else if ( field == lineEdit_hopUtilization || field == lineEdit_tunSpecificHeat )
   {
      foo = field->text().toDouble();
      field->setText( Brewtarget::displayAmount(foo));
   }
   else
   {
     foo = Brewtarget::volQStringToSI(field->text());
     field->setText( Brewtarget::displayAmount(foo, Units::liters) );
   }

   // We also need to recalculate the preboil size if a few fields change
   if ( selection == lineEdit_boilTime || selection == lineEdit_evaporationRate || selection == lineEdit_trubChillerLoss || selection == lineEdit_topUpWater )
       updateCheckboxRecord( checkBox_calcBoilVolume->checkState());
}

void EquipmentEditor::closeEvent(QCloseEvent *event)
{
   cancel();
   event->accept();
}
