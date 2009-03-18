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

#include "equipment.h"
#include "EquipmentEditor.h"
#include "EquipmentComboBox.h"
#include "stringparsing.h"
#include "config.h"
#include "unit.h"

EquipmentEditor::EquipmentEditor(QWidget* parent)
        : QDialog(parent)
{
   setupUi(this);

   setWindowIcon(QIcon(SMALLKETTLE));

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
   //obsEquip->setBoilSize_l( Unit::qstringToSI((lineEdit_boilSize->text()) );
   obsEquip->setBoilSize_l( Unit::qstringToSI(lineEdit_boilSize->text()) );
   
   obsEquip->setCalcBoilVolume( (checkBox_calcBoilVolume->checkState() == Qt::Checked)? true : false );
   obsEquip->setBatchSize_l( Unit::qstringToSI(lineEdit_batchSize->text()) );

   obsEquip->setTunVolume_l( Unit::qstringToSI(lineEdit_tunVolume->text()) );
   obsEquip->setTunWeight_kg( Unit::qstringToSI(lineEdit_tunWeight->text()) );
   obsEquip->setTunSpecificHeat_calGC( Unit::qstringToSI(lineEdit_tunSpecificHeat->text()) );

   obsEquip->setBoilTime_min( Unit::qstringToSI(lineEdit_boilTime->text()) );
   obsEquip->setEvapRate_pctHr( Unit::qstringToSI(lineEdit_evaporationRate->text()) );
   obsEquip->setTopUpKettle_l( Unit::qstringToSI(lineEdit_topUpKettle->text()) );
   obsEquip->setTopUpWater_l( Unit::qstringToSI(lineEdit_topUpWater->text()) );
   obsEquip->setHopUtilization_pct( Unit::qstringToSI(lineEdit_hopUtilization->text()) );

   obsEquip->setTrubChillerLoss_l( Unit::qstringToSI(lineEdit_trubChillerLoss->text()) );
   obsEquip->setLauterDeadspace_l( Unit::qstringToSI(lineEdit_lauterDeadspace->text()) );

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
