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
#include "equipment.h"
#include "EquipmentEditor.h"
#include "EquipmentListModel.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"
#include "HeatCalculations.h"
#include "PhysicalConstants.h"
#include "EquipmentSortFilterProxyModel.h"

EquipmentEditor::EquipmentEditor(QWidget* parent, bool singleEquipEditor) :
   QDialog(parent),
   obsEquip(0)
{
   setupUi(this);
   setLineEditorsProperties();

   updateAddRemoveButtonState(singleEquipEditor);
   configureFilterLineEdit();
   configureGrainAbsorptionLineEdit();

   // Set grain absorption label based on units.
   Unit* weightUnit = 0;
   Unit* volumeUnit = 0;
   Brewtarget::getThicknessUnits( &volumeUnit, &weightUnit );

   // Set up the model and proxyfilter
   equipmentListModel = new EquipmentListModel(equipmentsListView);
   equipmentSortProxyModel = new EquipmentSortFilterProxyModel(equipmentsListView);
   equipmentSortProxyModel->setDynamicSortFilter(true);
   equipmentSortProxyModel->setSourceModel(equipmentListModel);
   equipmentsListView->setModel(equipmentSortProxyModel);

   // Connect listview
   connect( equipmentsListView, SIGNAL(activated( const QModelIndex& )), this, SLOT( equipmentSelected( const QModelIndex&) ) );
   connect( lineEdit_filter, SIGNAL(textChanged(QString)), this, SLOT(filterChanged(QString) ) );

   // Connect all the edit boxen
   connect(lineEdit_boilTime,SIGNAL(textModified()),this,SLOT(updateCheckboxRecord()));
   connect(lineEdit_evaporationRate,SIGNAL(textModified()),this,SLOT(updateCheckboxRecord()));
   connect(lineEdit_topUpWater,SIGNAL(textModified()),this,SLOT(updateCheckboxRecord()));
   connect(lineEdit_trubChillerLoss,SIGNAL(textModified()),this,SLOT(updateCheckboxRecord()));
   connect(lineEdit_batchSize, SIGNAL( editingFinished() ), this,SLOT(updateCheckboxRecord()));
                     
   // Set up the buttons
   connect( buttonBox, SIGNAL( clicked(QAbstractButton*)), this, SLOT(buttonBoxClicked(QAbstractButton*) ) );
   connect( pushButtonAdd, SIGNAL( clicked() ), this, SLOT( newEquipment() ) );
   connect( pushButtonRemove, SIGNAL( clicked() ), this, SLOT( newEquipment() ) );

   // Check boxen
   connect(checkBox_calcBoilVolume, SIGNAL(stateChanged(int)), this, SLOT(updateCheckboxRecord()));
   connect(checkBox_defaultEquipment, SIGNAL(stateChanged(int)), this, SLOT(updateDefaultEquipment(int)));

   // Labels
   connect(label_boilSize, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_boilSize, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_batchSize, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_batchSize, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_evaporationRate, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_evaporationRate, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_topUpWater, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_topUpWater, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_boilingPoint, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_boilingPoint, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_tunVolume, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_tunVolume, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_tunWeight, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_tunWeight, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_lauterDeadspace, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_lauterDeadspace, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_trubChillerLoss, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_trubChillerLoss, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_topUpKettle, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_topUpKettle, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));
   connect(label_boilTime, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_boilTime, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));

   QMetaObject::connectSlotsByName(this);

   // make sure the dialog gets populated the first time it's opened from the menu
   setEquipment(equipmentListModel->at(0));
   // Ensure correct state of Boil Volume edit box.
   updateCheckboxRecord();
}

void EquipmentEditor::setLineEditorsProperties()
{
   lineEdit_boilSize->setProperty("editField", QVariant(QStringLiteral("boilSize_l")));
   lineEdit_boilTime->setProperty("editField", QVariant(QStringLiteral("boilTime_min")));
   lineEdit_evaporationRate->setProperty("editField", QVariant(QStringLiteral("evapRate_lHr")));
   lineEdit_topUpKettle->setProperty("editField", QVariant(QStringLiteral("topUpKettle_l")));
   lineEdit_topUpWater->setProperty("editField", QVariant(QStringLiteral("topUpKettle_l")));
   lineEdit_boilingPoint->setProperty("editField", QVariant(QStringLiteral("boilingPoint_c")));
   lineEdit_hopUtilization->setProperty("editField", QVariant(QStringLiteral("hopUtilization_pct")));
   lineEdit_tunVolume->setProperty("editField", QVariant(QStringLiteral("tunVolume_l")));
   lineEdit_tunWeight->setProperty("editField", QVariant(QStringLiteral("tunWeight_kg")));
   lineEdit_tunSpecificHeat->setProperty("editField", QVariant(QStringLiteral("tunSpecificHeat_calGC")));
   lineEdit_trubChillerLoss->setProperty("editField", QVariant(QStringLiteral("trubChillerLoss_l")));
   lineEdit_lauterDeadspace->setProperty("editField", QVariant(QStringLiteral("lauterDeadspace_l")));
   lineEdit_grainAbsorption->setProperty("editField", QVariant(QStringLiteral("grainAbsorption_LKg")));
}

void EquipmentEditor::updateAddRemoveButtonState(bool isSingleSelection)
{
   pushButtonAdd->setDisabled(isSingleSelection);
   pushButtonRemove->setDisabled(isSingleSelection);
   if(isSingleSelection)
   {
      pushButtonAdd->setToolTip("You cannot add an equipment in this edition mode");
      pushButtonRemove->setToolTip("You cannot remove an equipment in this edition mode");
   }
   else
   {
      pushButtonAdd->setToolTip("Add equipment");
      pushButtonRemove->setToolTip("Remove equipment");
   }
}

void EquipmentEditor::setEquipment( Equipment* e )
{
   if( e )
   {
      obsEquip = e;

      QModelIndex modelIndex = equipmentSortProxyModel->index(equipmentListModel->indexOf(obsEquip),0);
      if( modelIndex.isValid() )
         equipmentsListView->setCurrentIndex(modelIndex);

      showChanges();
   }
}

void EquipmentEditor::buttonBoxClicked(QAbstractButton *button)
{
   QDialogButtonBox::StandardButton standardButton = buttonBox->standardButton(button);
   switch(standardButton)
   {
   // Standard buttons:
   case QDialogButtonBox::Ok:
       cancel();
       break;
   case QDialogButtonBox::Cancel:
       cancel();
       break;
   case QDialogButtonBox::Save:
       save();
       break;
   default:
   // shouldn't happen
       break;
   }
}

void EquipmentEditor::equipmentSelected(const QModelIndex &model)
{
   QModelIndex sourceIndex(equipmentSortProxyModel->mapToSource(model));
   setEquipment(equipmentListModel->at(sourceIndex.row()));
}

void EquipmentEditor::filterChanged(QString newFilter)
{
   QRegExp regExp(newFilter,Qt::CaseInsensitive);
   equipmentSortProxyModel->setFilterRegExp(regExp);
}

void EquipmentEditor::removeEquipment()
{
   if( obsEquip )
      Database::instance().remove(obsEquip);

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
   bool ok = false;

   double grainAbs = Brewtarget::toDouble( lineEdit_grainAbsorption->text(), &ok );
   if ( ! ok )
      Brewtarget::logW( QString("EquipmentEditor::save() could not convert %1 to double").arg(lineEdit_grainAbsorption->text()));

   double ga_LKg = grainAbs * volumeUnit->toSI(1.0) * weightUnit->fromSI(1.0);

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
   newEquipment(QString());
}

void EquipmentEditor::newEquipment(QString folder)
{
   QString name = QInputDialog::getText(this, tr("Equipment name"),
                                          tr("Equipment name:"));
   if( name.isEmpty() )
      return;

   Equipment* e = Database::instance().newEquipment();
   e->setName( name );

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

void EquipmentEditor::configureFilterLineEdit()
{
   const QIcon icon = QIcon(QPixmap(":/images/find.png"));
   lineEdit_filter->addAction(icon,QLineEdit::LeadingPosition);
}

void EquipmentEditor::configureGrainAbsorptionLineEdit()
{
   lineEdit_grainAbsorption->addAction(actionRestoreDefaultValue,QLineEdit::TrailingPosition);
   connect(actionRestoreDefaultValue,SIGNAL(triggered()),this,SLOT(resetAbsorption()));
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
   else lineEdit_boilSize->setEnabled(true);
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
