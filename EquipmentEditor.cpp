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

#include "equipment.h"
#include "EquipmentEditor.h"
#include "EquipmentComboBox.h"
#include "stringparsing.h"

EquipmentEditor::EquipmentEditor(QWidget* parent)
        : QDialog(parent)
{
   setupUi(this);

   if( parent )
   {
      setWindowIcon(parent->windowIcon());
   }

   equipmentComboBox->startObservingDB();

   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newEquipment() ) );
   connect( pushButton_cancel, SIGNAL( clicked() ), this, SLOT( clearAndClose() ) );
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

void EquipmentEditor::equipmentSelected( const QString& /*text*/ )
{
   setEquipment( equipmentComboBox->getSelected() );
}

void EquipmentEditor::save()
{
   obsEquip->disableNotification();

   obsEquip->setName( lineEdit_name->text().toStdString() );
   obsEquip->setBoilSize_l( parseDouble(lineEdit_boilSize->text().toStdString()) );
   obsEquip->setCalcBoilVolume( (checkBox_calcBoilVolume->checkState() == Qt::Checked)? true : false );
   obsEquip->setBatchSize_l( parseDouble(lineEdit_batchSize->text().toStdString()) );

   obsEquip->setTunVolume_l( parseDouble(lineEdit_tunVolume->text().toStdString()) );
   obsEquip->setTunWeight_kg( parseDouble(lineEdit_tunWeight->text().toStdString()) );
   obsEquip->setTunSpecificHeat_calGC( parseDouble(lineEdit_tunSpecificHeat->text().toStdString()) );

   obsEquip->setBoilTime_min( parseDouble(lineEdit_boilTime->text().toStdString()) );
   obsEquip->setEvapRate_pctHr( parseDouble(lineEdit_evaporationRate->text().toStdString()) );
   obsEquip->setTopUpKettle_l( parseDouble(lineEdit_topUpKettle->text().toStdString()) );
   obsEquip->setTopUpWater_l( parseDouble(lineEdit_topUpWater->text().toStdString()) );
   obsEquip->setHopUtilization_pct( parseDouble(lineEdit_hopUtilization->text().toStdString()) );

   obsEquip->setTrubChillerLoss_l( parseDouble(lineEdit_trubChillerLoss->text().toStdString()) );
   obsEquip->setLauterDeadspace_l( parseDouble(lineEdit_lauterDeadspace->text().toStdString()) );

   obsEquip->setNotes(textEdit_notes->toPlainText().toStdString());

   obsEquip->reenableNotification();
   obsEquip->forceNotify();

   Database::getDatabase()->resortAll(); // If the name changed, need to resort.
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

void EquipmentEditor::notify(Observable* /*notifier*/)
{
   showChanges();
}

void EquipmentEditor::showChanges()
{
   Equipment *e = obsEquip;

   equipmentComboBox->setIndexByEquipmentName(e->getName());

   lineEdit_name->setText(e->getName().c_str());
   lineEdit_name->setCursorPosition(0);
   lineEdit_boilSize->setText(doubleToString(e->getBoilSize_l()).c_str());
   checkBox_calcBoilVolume->setCheckState( (e->getCalcBoilVolume())? Qt::Checked : Qt::Unchecked );
   lineEdit_batchSize->setText(doubleToString(e->getBatchSize_l()).c_str());

   lineEdit_tunVolume->setText(doubleToString(e->getTunVolume_l()).c_str());
   lineEdit_tunWeight->setText(doubleToString(e->getTunWeight_kg()).c_str());
   lineEdit_tunSpecificHeat->setText(doubleToString(e->getTunSpecificHeat_calGC()).c_str());

   lineEdit_boilTime->setText(doubleToString(e->getBoilTime_min()).c_str());
   lineEdit_evaporationRate->setText(doubleToString(e->getEvapRate_pctHr()).c_str());
   lineEdit_topUpKettle->setText(doubleToString(e->getTopUpKettle_l()).c_str());
   lineEdit_topUpWater->setText(doubleToString(e->getTopUpWater_l()).c_str());
   lineEdit_hopUtilization->setText(doubleToString(e->getHopUtilization_pct()).c_str());

   lineEdit_trubChillerLoss->setText(doubleToString(e->getTrubChillerLoss_l()).c_str());
   lineEdit_lauterDeadspace->setText(doubleToString(e->getLauterDeadspace_l()).c_str());

   textEdit_notes->setText(e->getNotes().c_str());
}
