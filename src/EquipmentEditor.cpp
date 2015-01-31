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
   connect(lineEdit_boilTime,SIGNAL(textModified()),this,SLOT(updateCheckboxRecord()));
   connect(lineEdit_evaporationRate,SIGNAL(textModified()),this,SLOT(updateCheckboxRecord()));
   connect(lineEdit_topUpWater,SIGNAL(textModified()),this,SLOT(updateCheckboxRecord()));
   connect(lineEdit_trubChillerLoss,SIGNAL(textModified()),this,SLOT(updateCheckboxRecord()));

   // Set up the buttons
   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newEquipment() ) );
   connect( pushButton_cancel, SIGNAL( clicked() ), this, SLOT( cancel() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT( removeEquipment() ) );
   connect( pushButton_absorption, SIGNAL( clicked() ), this, SLOT( resetAbsorption() ) );
   connect( equipmentComboBox, SIGNAL(activated(const QString&)), this, SLOT( equipmentSelected() ) );

   // Check boxen
   connect(checkBox_calcBoilVolume, SIGNAL(stateChanged(int)), this, SLOT(updateCheckboxRecord()));
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
   if ( lineEdit_tunVolume->toSI() <= 0.001 )
      QMessageBox::warning(this, tr("Tun Volume Warning"), tr("The tun volume you entered is 0. This may cause problems"));

   if ( lineEdit_batchSize->toSI() <= 0.001 )
      QMessageBox::warning(this, tr("Batch Size Warning"), tr("The batch size you entered is 0. This may cause problems"));

   if ( lineEdit_hopUtilization->toSI() < 0.001 )
      QMessageBox::warning(this, tr("Hop Utilization Warning"), tr("The hop utilization percentage you entered is 0. This may cause problems"));


   obsEquip->setName( lineEdit_name->text() );
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

   lineEdit_grainAbsorption->displayAmount(gaCustomUnits);
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
   lineEdit_boilSize->setText(e);
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

void EquipmentEditor::closeEvent(QCloseEvent *event)
{
   cancel();
   event->accept();
}
