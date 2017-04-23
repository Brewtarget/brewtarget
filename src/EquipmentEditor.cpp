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
#include "BeerXMLSortProxyModel.h"

EquipmentEditor::EquipmentEditor(QWidget* parent, bool singleEquipEditor)
   : QDialog(parent)
{
   doLayout();

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
   connect(lineEdit_boilTime,&BtLineEdit::textModified,this,&EquipmentEditor::updateCheckboxRecord);
   connect(lineEdit_evaporationRate,&BtLineEdit::textModified,this,&EquipmentEditor::updateCheckboxRecord);
   connect(lineEdit_topUpWater,&BtLineEdit::textModified,this,&EquipmentEditor::updateCheckboxRecord);
   connect(lineEdit_trubChillerLoss,&BtLineEdit::textModified,this,&EquipmentEditor::updateCheckboxRecord);
   connect(lineEdit_batchSize, &QLineEdit::editingFinished, this,&EquipmentEditor::updateCheckboxRecord);
                     
   // Set up the buttons
   connect( pushButton_save, &QAbstractButton::clicked, this, &EquipmentEditor::save );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newEquipment() ) );
   connect( pushButton_cancel, &QAbstractButton::clicked, this, &EquipmentEditor::cancel );
   connect( pushButton_remove, &QAbstractButton::clicked, this, &EquipmentEditor::removeEquipment );
   connect( pushButton_absorption, &QAbstractButton::clicked, this, &EquipmentEditor::resetAbsorption );
   connect( equipmentComboBox, SIGNAL(activated(const QString&)), this, SLOT( equipmentSelected() ) );

   // Check boxen
   connect(checkBox_calcBoilVolume, &QCheckBox::stateChanged, this, &EquipmentEditor::updateCheckboxRecord);
   connect(checkBox_defaultEquipment, &QCheckBox::stateChanged, this, &EquipmentEditor::updateDefaultEquipment);

   // Labels
   connect(label_boilSize, &BtLabel::labelChanged, lineEdit_boilSize, &BtLineEdit::lineChanged);
   connect(label_batchSize, &BtLabel::labelChanged, lineEdit_batchSize, &BtLineEdit::lineChanged);
   connect(label_evaporationRate, &BtLabel::labelChanged, lineEdit_evaporationRate, &BtLineEdit::lineChanged);
   connect(label_topUpWater, &BtLabel::labelChanged, lineEdit_topUpWater, &BtLineEdit::lineChanged);
   connect(label_boilingPoint, &BtLabel::labelChanged, lineEdit_boilingPoint, &BtLineEdit::lineChanged);
   connect(label_tunVolume, &BtLabel::labelChanged, lineEdit_tunVolume, &BtLineEdit::lineChanged);
   connect(label_tunWeight, &BtLabel::labelChanged, lineEdit_tunWeight, &BtLineEdit::lineChanged);
   connect(label_lauterDeadspace, &BtLabel::labelChanged, lineEdit_lauterDeadspace, &BtLineEdit::lineChanged);
   connect(label_trubChillerLoss, &BtLabel::labelChanged, lineEdit_trubChillerLoss, &BtLineEdit::lineChanged);
   connect(label_topUpKettle, &BtLabel::labelChanged, lineEdit_topUpKettle, &BtLineEdit::lineChanged);
   connect(label_boilTime, &BtLabel::labelChanged, lineEdit_boilTime, &BtLineEdit::lineChanged);

   QMetaObject::connectSlotsByName(this);

   // make sure the dialog gets populated the first time it's opened from the menu
   equipmentSelected();
   // Ensure correct state of Boil Volume edit box.
   updateCheckboxRecord();
}

void EquipmentEditor::doLayout()
{
   resize(0,0);
   topVLayout = new QVBoxLayout(this);
      horizontalLayout_equipments = new QHBoxLayout();
         label = new QLabel(this);
         equipmentComboBox = new QComboBox(this);
            equipmentComboBox->setObjectName(QStringLiteral("equipmentComboBox"));
            equipmentComboBox->setMinimumSize(QSize(200, 0));
            equipmentComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
         pushButton_remove = new QPushButton(this);
            pushButton_remove->setObjectName(QStringLiteral("pushButton_remove"));
            QIcon icon;
            icon.addFile(QStringLiteral(":/images/smallMinus.svg"), QSize(), QIcon::Normal, QIcon::Off);
            pushButton_remove->setIcon(icon);
            pushButton_remove->setAutoDefault(false);
         horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
         checkBox_defaultEquipment = new QCheckBox(this);
            checkBox_defaultEquipment->setObjectName(QStringLiteral("checkBox_defaultEquipment"));
         horizontalLayout_equipments->addWidget(label);
         horizontalLayout_equipments->addWidget(equipmentComboBox);
         horizontalLayout_equipments->addWidget(pushButton_remove);
         horizontalLayout_equipments->addItem(horizontalSpacer);
         horizontalLayout_equipments->addWidget(checkBox_defaultEquipment);
      horizontalLayout = new QHBoxLayout();
         vLayout_left = new QVBoxLayout();
            groupBox_required = new QGroupBox(this);
               groupBox_required->setProperty("configSection", QVariant(QStringLiteral("equipmentEditor")));
               formLayout = new QFormLayout(groupBox_required);
                  label_name = new QLabel(groupBox_required);
                     label_name->setObjectName(QStringLiteral("label_name"));
                     QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
                     sizePolicy1.setHorizontalStretch(0);
                     sizePolicy1.setVerticalStretch(0);
                     sizePolicy1.setHeightForWidth(label_name->sizePolicy().hasHeightForWidth());
                     label_name->setSizePolicy(sizePolicy1);
                  lineEdit_name = new QLineEdit(groupBox_required);
                     lineEdit_name->setObjectName(QStringLiteral("lineEdit_name"));
                     QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
                     sizePolicy2.setHorizontalStretch(100);
                     sizePolicy2.setVerticalStretch(0);
                     sizePolicy2.setHeightForWidth(lineEdit_name->sizePolicy().hasHeightForWidth());
                     lineEdit_name->setSizePolicy(sizePolicy2);
                     lineEdit_name->setMinimumSize(QSize(100, 0));
                     lineEdit_name->setMaximumSize(QSize(100, 16777215));
                  label_boilSize = new BtVolumeLabel(groupBox_required);
                     label_boilSize->setObjectName(QStringLiteral("label_boilSize"));
                     sizePolicy1.setHeightForWidth(label_boilSize->sizePolicy().hasHeightForWidth());
                     label_boilSize->setSizePolicy(sizePolicy1);
                     label_boilSize->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_boilSize = new BtVolumeEdit(groupBox_required);
                     lineEdit_boilSize->setObjectName(QStringLiteral("lineEdit_boilSize"));
                     sizePolicy2.setHeightForWidth(lineEdit_boilSize->sizePolicy().hasHeightForWidth());
                     lineEdit_boilSize->setSizePolicy(sizePolicy2);
                     lineEdit_boilSize->setMinimumSize(QSize(100, 0));
                     lineEdit_boilSize->setMaximumSize(QSize(100, 16777215));
                     lineEdit_boilSize->setProperty("editField", QVariant(QStringLiteral("boilSize_l")));
                  label_batchSize = new BtVolumeLabel(groupBox_required);
                     label_batchSize->setObjectName(QStringLiteral("label_batchSize"));
                     sizePolicy1.setHeightForWidth(label_batchSize->sizePolicy().hasHeightForWidth());
                     label_batchSize->setSizePolicy(sizePolicy1);
                     label_batchSize->setContextMenuPolicy(Qt::CustomContextMenu);
                  checkBox_calcBoilVolume = new QCheckBox(groupBox_required);
                     checkBox_calcBoilVolume->setObjectName(QStringLiteral("checkBox_calcBoilVolume"));
                  label_calcBoilVolume = new QLabel(groupBox_required);
                     label_calcBoilVolume->setObjectName(QStringLiteral("label_calcBoilVolume"));
                     sizePolicy1.setHeightForWidth(label_calcBoilVolume->sizePolicy().hasHeightForWidth());
                     label_calcBoilVolume->setSizePolicy(sizePolicy1);


                  lineEdit_batchSize = new BtVolumeEdit(groupBox_required);
                     lineEdit_batchSize->setObjectName(QStringLiteral("lineEdit_batchSize"));
                     sizePolicy2.setHeightForWidth(lineEdit_batchSize->sizePolicy().hasHeightForWidth());
                     lineEdit_batchSize->setSizePolicy(sizePolicy2);
                     lineEdit_batchSize->setMinimumSize(QSize(100, 0));
                     lineEdit_batchSize->setMaximumSize(QSize(100, 16777215));
                     lineEdit_batchSize->setProperty("editField", QVariant(QStringLiteral("batchSize_l")));
                     
                  formLayout->setWidget(0, QFormLayout::LabelRole, label_name);
                  formLayout->setWidget(0, QFormLayout::FieldRole, lineEdit_name);
                  formLayout->setWidget(1, QFormLayout::LabelRole, label_batchSize);
                  formLayout->setWidget(1, QFormLayout::FieldRole, lineEdit_batchSize);
                  formLayout->setWidget(2, QFormLayout::LabelRole, label_boilSize);
                  formLayout->setWidget(2, QFormLayout::FieldRole, lineEdit_boilSize);
                  formLayout->setWidget(3, QFormLayout::LabelRole, label_calcBoilVolume);
                  formLayout->setWidget(3, QFormLayout::FieldRole, checkBox_calcBoilVolume);
                  
            groupBox_water = new QGroupBox(this);
               groupBox_water->setProperty("configSection", QVariant(QStringLiteral("equipmentEditor")));
               formLayout_water = new QFormLayout(groupBox_water);
                  formLayout_water->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
                  label_boilTime = new BtTimeLabel(groupBox_water);
                     label_boilTime->setObjectName(QStringLiteral("label_boilTime"));
                     sizePolicy1.setHeightForWidth(label_boilTime->sizePolicy().hasHeightForWidth());
                     label_boilTime->setSizePolicy(sizePolicy1);
                     label_boilTime->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_boilTime = new BtTimeEdit(groupBox_water);
                     lineEdit_boilTime->setObjectName(QStringLiteral("lineEdit_boilTime"));
                     sizePolicy2.setHeightForWidth(lineEdit_boilTime->sizePolicy().hasHeightForWidth());
                     lineEdit_boilTime->setSizePolicy(sizePolicy2);
                     lineEdit_boilTime->setMinimumSize(QSize(100, 0));
                     lineEdit_boilTime->setMaximumSize(QSize(100, 16777215));
                     lineEdit_boilTime->setProperty("editField", QVariant(QStringLiteral("boilTime_min")));
                  label_evaporationRate = new BtVolumeLabel(groupBox_water);
                     label_evaporationRate->setObjectName(QStringLiteral("label_evaporationRate"));
                     sizePolicy1.setHeightForWidth(label_evaporationRate->sizePolicy().hasHeightForWidth());
                     label_evaporationRate->setSizePolicy(sizePolicy1);
                     label_evaporationRate->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_evaporationRate = new BtVolumeEdit(groupBox_water);
                     lineEdit_evaporationRate->setObjectName(QStringLiteral("lineEdit_evaporationRate"));
                     sizePolicy2.setHeightForWidth(lineEdit_evaporationRate->sizePolicy().hasHeightForWidth());
                     lineEdit_evaporationRate->setSizePolicy(sizePolicy2);
                     lineEdit_evaporationRate->setMinimumSize(QSize(100, 0));
                     lineEdit_evaporationRate->setMaximumSize(QSize(100, 16777215));
                     lineEdit_evaporationRate->setProperty("editField", QVariant(QStringLiteral("evapRate_lHr")));
                  label_topUpKettle = new BtVolumeLabel(groupBox_water);
                     label_topUpKettle->setObjectName(QStringLiteral("label_topUpKettle"));
                     sizePolicy1.setHeightForWidth(label_topUpKettle->sizePolicy().hasHeightForWidth());
                     label_topUpKettle->setSizePolicy(sizePolicy1);
                     label_topUpKettle->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_topUpKettle = new BtVolumeEdit(groupBox_water);
                     lineEdit_topUpKettle->setObjectName(QStringLiteral("lineEdit_topUpKettle"));
                     sizePolicy2.setHeightForWidth(lineEdit_topUpKettle->sizePolicy().hasHeightForWidth());
                     lineEdit_topUpKettle->setSizePolicy(sizePolicy2);
                     lineEdit_topUpKettle->setMinimumSize(QSize(100, 0));
                     lineEdit_topUpKettle->setMaximumSize(QSize(100, 16777215));
                     lineEdit_topUpKettle->setProperty("editField", QVariant(QStringLiteral("topUpKettle_l")));
                  label_topUpWater = new BtVolumeLabel(groupBox_water);
                     label_topUpWater->setObjectName(QStringLiteral("label_topUpWater"));
                     sizePolicy1.setHeightForWidth(label_topUpWater->sizePolicy().hasHeightForWidth());
                     label_topUpWater->setSizePolicy(sizePolicy1);
                     label_topUpWater->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_topUpWater = new BtVolumeEdit(groupBox_water);
                     lineEdit_topUpWater->setObjectName(QStringLiteral("lineEdit_topUpWater"));
                     sizePolicy2.setHeightForWidth(lineEdit_topUpWater->sizePolicy().hasHeightForWidth());
                     lineEdit_topUpWater->setSizePolicy(sizePolicy2);
                     lineEdit_topUpWater->setMinimumSize(QSize(100, 0));
                     lineEdit_topUpWater->setMaximumSize(QSize(100, 16777215));
                     lineEdit_topUpWater->setProperty("editField", QVariant(QStringLiteral("topUpWater_l")));
                  label_absorption = new QLabel(groupBox_water);
                     label_absorption->setObjectName(QStringLiteral("label_absorption"));
                  lineEdit_grainAbsorption = new BtGenericEdit(groupBox_water);
                     lineEdit_grainAbsorption->setObjectName(QStringLiteral("lineEdit_grainAbsorption"));
                     QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
                     sizePolicy3.setHorizontalStretch(0);
                     sizePolicy3.setVerticalStretch(0);
                     sizePolicy3.setHeightForWidth(lineEdit_grainAbsorption->sizePolicy().hasHeightForWidth());
                     lineEdit_grainAbsorption->setSizePolicy(sizePolicy3);
                     lineEdit_grainAbsorption->setMaximumSize(QSize(100, 16777215));
                     lineEdit_grainAbsorption->setProperty("editField", QVariant(QStringLiteral("grainAbsorption_LKg")));
                  pushButton_absorption = new QPushButton(groupBox_water);
                     pushButton_absorption->setObjectName(QStringLiteral("pushButton_absorption"));
                  label_boilingPoint = new BtTemperatureLabel(groupBox_water);
                     label_boilingPoint->setObjectName(QStringLiteral("label_boilingPoint"));
                     label_boilingPoint->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_boilingPoint = new BtTemperatureEdit(groupBox_water);
                     lineEdit_boilingPoint->setObjectName(QStringLiteral("lineEdit_boilingPoint"));
                     sizePolicy3.setHeightForWidth(lineEdit_boilingPoint->sizePolicy().hasHeightForWidth());
                     lineEdit_boilingPoint->setSizePolicy(sizePolicy3);
                     lineEdit_boilingPoint->setMaximumSize(QSize(100, 16777215));
                     lineEdit_boilingPoint->setProperty("editField", QVariant(QStringLiteral("boilingPoint_c")));
                  label_hopUtilization = new QLabel(groupBox_water);
                     label_hopUtilization->setObjectName(QStringLiteral("label_hopUtilization"));
                  lineEdit_hopUtilization = new BtGenericEdit(groupBox_water);
                     lineEdit_hopUtilization->setObjectName(QStringLiteral("lineEdit_hopUtilization"));
                     sizePolicy3.setHeightForWidth(lineEdit_hopUtilization->sizePolicy().hasHeightForWidth());
                     lineEdit_hopUtilization->setSizePolicy(sizePolicy3);
                     lineEdit_hopUtilization->setMaximumSize(QSize(100, 16777215));
                     lineEdit_hopUtilization->setProperty("editField", QVariant(QStringLiteral("hopUtilization_pct")));
                  formLayout_water->setWidget(0, QFormLayout::LabelRole, label_boilTime);
                  formLayout_water->setWidget(0, QFormLayout::FieldRole, lineEdit_boilTime);
                  formLayout_water->setWidget(1, QFormLayout::LabelRole, label_evaporationRate);
                  formLayout_water->setWidget(1, QFormLayout::FieldRole, lineEdit_evaporationRate);
                  formLayout_water->setWidget(2, QFormLayout::LabelRole, label_topUpKettle);
                  formLayout_water->setWidget(2, QFormLayout::FieldRole, lineEdit_topUpKettle);
                  formLayout_water->setWidget(3, QFormLayout::LabelRole, label_topUpWater);
                  formLayout_water->setWidget(3, QFormLayout::FieldRole, lineEdit_topUpWater);
                  formLayout_water->setWidget(4, QFormLayout::LabelRole, label_absorption);
                  formLayout_water->setWidget(4, QFormLayout::FieldRole, lineEdit_grainAbsorption);
                  formLayout_water->setWidget(5, QFormLayout::LabelRole, pushButton_absorption);
                  formLayout_water->setWidget(6, QFormLayout::LabelRole, label_boilingPoint);
                  formLayout_water->setWidget(6, QFormLayout::FieldRole, lineEdit_boilingPoint);
                  formLayout_water->setWidget(7, QFormLayout::LabelRole, label_hopUtilization);
                  formLayout_water->setWidget(7, QFormLayout::FieldRole, lineEdit_hopUtilization);
            verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            vLayout_left->addWidget(groupBox_required);
            vLayout_left->addWidget(groupBox_water);
            vLayout_left->addItem(verticalSpacer_2);
         vLayout_right = new QVBoxLayout();
            groupBox_mashTun = new QGroupBox(this);
               groupBox_mashTun->setObjectName(QStringLiteral("groupBox_mashTun"));
               groupBox_mashTun->setProperty("configSection", QVariant(QStringLiteral("equipmentEditor")));
               formLayout_mashTun = new QFormLayout(groupBox_mashTun);
                  formLayout_mashTun->setObjectName(QStringLiteral("formLayout_mashTun"));
                  label_tunVolume = new BtVolumeLabel(groupBox_mashTun);
                     label_tunVolume->setObjectName(QStringLiteral("label_tunVolume"));
                     sizePolicy1.setHeightForWidth(label_tunVolume->sizePolicy().hasHeightForWidth());
                     label_tunVolume->setSizePolicy(sizePolicy1);
                     label_tunVolume->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_tunVolume = new BtVolumeEdit(groupBox_mashTun);
                     lineEdit_tunVolume->setObjectName(QStringLiteral("lineEdit_tunVolume"));
                     sizePolicy2.setHeightForWidth(lineEdit_tunVolume->sizePolicy().hasHeightForWidth());
                     lineEdit_tunVolume->setSizePolicy(sizePolicy2);
                     lineEdit_tunVolume->setMinimumSize(QSize(100, 0));
                     lineEdit_tunVolume->setMaximumSize(QSize(100, 16777215));
                     lineEdit_tunVolume->setProperty("editField", QVariant(QStringLiteral("tunVolume_l")));
                  label_tunWeight = new BtMassLabel(groupBox_mashTun);
                     label_tunWeight->setObjectName(QStringLiteral("label_tunWeight"));
                     sizePolicy1.setHeightForWidth(label_tunWeight->sizePolicy().hasHeightForWidth());
                     label_tunWeight->setSizePolicy(sizePolicy1);
                     label_tunWeight->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_tunWeight = new BtMassEdit(groupBox_mashTun);
                     lineEdit_tunWeight->setObjectName(QStringLiteral("lineEdit_tunWeight"));
                     sizePolicy2.setHeightForWidth(lineEdit_tunWeight->sizePolicy().hasHeightForWidth());
                     lineEdit_tunWeight->setSizePolicy(sizePolicy2);
                     lineEdit_tunWeight->setMinimumSize(QSize(100, 0));
                     lineEdit_tunWeight->setMaximumSize(QSize(100, 16777215));
                     lineEdit_tunWeight->setProperty("editField", QVariant(QStringLiteral("tunWeight_kg")));
                  label_tunSpecificHeat = new QLabel(groupBox_mashTun);
                     label_tunSpecificHeat->setObjectName(QStringLiteral("label_tunSpecificHeat"));
                     sizePolicy1.setHeightForWidth(label_tunSpecificHeat->sizePolicy().hasHeightForWidth());
                     label_tunSpecificHeat->setSizePolicy(sizePolicy1);
                  lineEdit_tunSpecificHeat = new BtGenericEdit(groupBox_mashTun);
                     lineEdit_tunSpecificHeat->setObjectName(QStringLiteral("lineEdit_tunSpecificHeat"));
                     sizePolicy2.setHeightForWidth(lineEdit_tunSpecificHeat->sizePolicy().hasHeightForWidth());
                     lineEdit_tunSpecificHeat->setSizePolicy(sizePolicy2);
                     lineEdit_tunSpecificHeat->setMinimumSize(QSize(100, 0));
                     lineEdit_tunSpecificHeat->setMaximumSize(QSize(100, 16777215));
                     lineEdit_tunSpecificHeat->setProperty("editField", QVariant(QStringLiteral("tunSpecificHeat_calGC")));
                  formLayout_mashTun->setWidget(0, QFormLayout::LabelRole, label_tunVolume);
                  formLayout_mashTun->setWidget(0, QFormLayout::FieldRole, lineEdit_tunVolume);
                  formLayout_mashTun->setWidget(1, QFormLayout::LabelRole, label_tunWeight);
                  formLayout_mashTun->setWidget(1, QFormLayout::FieldRole, lineEdit_tunWeight);
                  formLayout_mashTun->setWidget(2, QFormLayout::LabelRole, label_tunSpecificHeat);
                  formLayout_mashTun->setWidget(2, QFormLayout::FieldRole, lineEdit_tunSpecificHeat);
            groupBox_losses = new QGroupBox(this);
               groupBox_losses->setProperty("configSection", QVariant(QStringLiteral("equipmentEditor")));
               formLayout_losses = new QFormLayout(groupBox_losses);
                  label_trubChillerLoss = new BtVolumeLabel(groupBox_losses);
                     label_trubChillerLoss->setObjectName(QStringLiteral("label_trubChillerLoss"));
                     sizePolicy1.setHeightForWidth(label_trubChillerLoss->sizePolicy().hasHeightForWidth());
                     label_trubChillerLoss->setSizePolicy(sizePolicy1);
                     label_trubChillerLoss->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_trubChillerLoss = new BtVolumeEdit(groupBox_losses);
                     lineEdit_trubChillerLoss->setObjectName(QStringLiteral("lineEdit_trubChillerLoss"));
                     sizePolicy2.setHeightForWidth(lineEdit_trubChillerLoss->sizePolicy().hasHeightForWidth());
                     lineEdit_trubChillerLoss->setSizePolicy(sizePolicy2);
                     lineEdit_trubChillerLoss->setMinimumSize(QSize(100, 0));
                     lineEdit_trubChillerLoss->setMaximumSize(QSize(100, 16777215));
                     lineEdit_trubChillerLoss->setProperty("editField", QVariant(QStringLiteral("trubChillerLoss_l")));
                  label_lauterDeadspace = new BtVolumeLabel(groupBox_losses);
                     label_lauterDeadspace->setObjectName(QStringLiteral("label_lauterDeadspace"));
                     sizePolicy1.setHeightForWidth(label_lauterDeadspace->sizePolicy().hasHeightForWidth());
                     label_lauterDeadspace->setSizePolicy(sizePolicy1);
                     label_lauterDeadspace->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_lauterDeadspace = new BtVolumeEdit(groupBox_losses);
                     lineEdit_lauterDeadspace->setObjectName(QStringLiteral("lineEdit_lauterDeadspace"));
                     sizePolicy2.setHeightForWidth(lineEdit_lauterDeadspace->sizePolicy().hasHeightForWidth());
                     lineEdit_lauterDeadspace->setSizePolicy(sizePolicy2);
                     lineEdit_lauterDeadspace->setMinimumSize(QSize(100, 0));
                     lineEdit_lauterDeadspace->setMaximumSize(QSize(100, 16777215));
                     lineEdit_lauterDeadspace->setProperty("editField", QVariant(QStringLiteral("lauterDeadspace_l")));
                  formLayout_losses->setWidget(0, QFormLayout::LabelRole, label_trubChillerLoss);
                  formLayout_losses->setWidget(0, QFormLayout::FieldRole, lineEdit_trubChillerLoss);
                  formLayout_losses->setWidget(1, QFormLayout::LabelRole, label_lauterDeadspace);
                  formLayout_losses->setWidget(1, QFormLayout::FieldRole, lineEdit_lauterDeadspace);
            groupBox_notes = new QGroupBox(this);
               verticalLayout_notes = new QVBoxLayout(groupBox_notes);
                  verticalLayout_notes->setObjectName(QStringLiteral("verticalLayout_notes"));
                  textEdit_notes = new QTextEdit(groupBox_notes);
                     textEdit_notes->setObjectName(QStringLiteral("textEdit_notes"));
                  verticalLayout_notes->addWidget(textEdit_notes);
            verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            vLayout_right->addWidget(groupBox_mashTun);
            vLayout_right->addWidget(groupBox_losses);
            vLayout_right->addWidget(groupBox_notes);
            vLayout_right->addItem(verticalSpacer);
         horizontalLayout->addLayout(vLayout_left);
         horizontalLayout->addLayout(vLayout_right);
      hLayout_buttons = new QHBoxLayout();
         horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
         pushButton_new = new QPushButton(this);
            pushButton_new->setObjectName(QStringLiteral("pushButton_new"));
            QIcon icon1;
            icon1.addFile(QStringLiteral(":/images/smallPlus.svg"), QSize(), QIcon::Normal, QIcon::Off);
            pushButton_new->setIcon(icon1);
            pushButton_new->setAutoDefault(false);
         pushButton_save = new QPushButton(this);
            pushButton_save->setObjectName(QStringLiteral("pushButton_save"));
            QIcon icon2;
            icon2.addFile(QStringLiteral(":/images/filesave.svg"), QSize(), QIcon::Normal, QIcon::Off);
            pushButton_save->setIcon(icon2);
            pushButton_save->setAutoDefault(false);
            pushButton_save->setDefault(true);
         pushButton_cancel = new QPushButton(this);
            pushButton_cancel->setObjectName(QStringLiteral("pushButton_cancel"));
            QIcon icon3;
            icon3.addFile(QStringLiteral(":/images/exit.svg"), QSize(), QIcon::Normal, QIcon::Off);
            pushButton_cancel->setIcon(icon3);
            pushButton_cancel->setAutoDefault(false);
         hLayout_buttons->addItem(horizontalSpacer_2);
         hLayout_buttons->addWidget(pushButton_new);
         hLayout_buttons->addWidget(pushButton_save);
         hLayout_buttons->addWidget(pushButton_cancel);
   topVLayout->addLayout(horizontalLayout_equipments);
   topVLayout->addLayout(horizontalLayout);
   topVLayout->addLayout(hLayout_buttons);

#ifndef QT_NO_SHORTCUT
   label_name->setBuddy(lineEdit_name);
   label_boilSize->setBuddy(lineEdit_boilSize);
   label_calcBoilVolume->setBuddy(checkBox_calcBoilVolume);
   label_batchSize->setBuddy(lineEdit_batchSize);
   label_boilTime->setBuddy(lineEdit_boilTime);
   label_evaporationRate->setBuddy(lineEdit_evaporationRate);
   label_topUpKettle->setBuddy(lineEdit_topUpKettle);
   label_topUpWater->setBuddy(lineEdit_topUpWater);
   label_absorption->setBuddy(lineEdit_grainAbsorption);
   label_hopUtilization->setBuddy(lineEdit_hopUtilization);
   label_boilingPoint->setBuddy(lineEdit_boilingPoint);
   label_tunVolume->setBuddy(lineEdit_tunVolume);
   label_tunWeight->setBuddy(lineEdit_tunWeight);
   label_tunSpecificHeat->setBuddy(lineEdit_tunSpecificHeat);
   label_trubChillerLoss->setBuddy(lineEdit_trubChillerLoss);
   label_lauterDeadspace->setBuddy(lineEdit_lauterDeadspace);
#endif // QT_NO_SHORTCUT

   QWidget::setTabOrder(equipmentComboBox, pushButton_remove);
   QWidget::setTabOrder(pushButton_remove, checkBox_defaultEquipment);
   QWidget::setTabOrder(checkBox_defaultEquipment, lineEdit_name);
   QWidget::setTabOrder(checkBox_calcBoilVolume, lineEdit_batchSize);
   QWidget::setTabOrder(lineEdit_boilSize, checkBox_calcBoilVolume);
   QWidget::setTabOrder(lineEdit_name, lineEdit_boilSize);
   QWidget::setTabOrder(lineEdit_batchSize, lineEdit_boilTime);
   QWidget::setTabOrder(lineEdit_boilTime, lineEdit_evaporationRate);
   QWidget::setTabOrder(lineEdit_evaporationRate, lineEdit_topUpKettle);
   QWidget::setTabOrder(lineEdit_topUpKettle, lineEdit_topUpWater);
   QWidget::setTabOrder(lineEdit_topUpWater, lineEdit_grainAbsorption);
   QWidget::setTabOrder(lineEdit_grainAbsorption, pushButton_absorption);
   QWidget::setTabOrder(pushButton_absorption, lineEdit_boilingPoint);
   QWidget::setTabOrder(lineEdit_boilingPoint, lineEdit_hopUtilization);
   QWidget::setTabOrder(lineEdit_hopUtilization, lineEdit_tunVolume);
   QWidget::setTabOrder(lineEdit_tunVolume, lineEdit_tunWeight);
   QWidget::setTabOrder(lineEdit_tunWeight, lineEdit_tunSpecificHeat);
   QWidget::setTabOrder(lineEdit_tunSpecificHeat, lineEdit_trubChillerLoss);
   QWidget::setTabOrder(lineEdit_trubChillerLoss, lineEdit_lauterDeadspace);
   QWidget::setTabOrder(lineEdit_lauterDeadspace, textEdit_notes);
   QWidget::setTabOrder(textEdit_notes, pushButton_new);
   QWidget::setTabOrder(pushButton_new, pushButton_save);
   QWidget::setTabOrder(pushButton_save, pushButton_cancel);

   retranslateUi();
}

void EquipmentEditor::retranslateUi()
{
   setWindowTitle(tr("Equipment Editor"));
   label->setText(tr("Equipment"));
   pushButton_remove->setText(QString());
   checkBox_defaultEquipment->setText(tr("Set as Default"));
   groupBox_required->setTitle(tr("Required Fields"));
   label_name->setText(tr("Name"));
   label_boilSize->setText(tr("Pre-boil volume"));
   label_calcBoilVolume->setText(tr("Calculate pre-boil volume"));
   checkBox_calcBoilVolume->setText(QString());
   label_batchSize->setText(tr("Batch size"));
   groupBox_water->setTitle(tr("Boiling && Water"));
   label_boilTime->setText(tr("Boil time"));
   label_evaporationRate->setText(tr("Evaporation rate (per hr)"));
   label_topUpKettle->setText(tr("Kettle top-up water"));
   label_topUpWater->setText(tr("Final top-up water"));
   label_absorption->setText(tr("Grain Absorption (L/kg)"));
   pushButton_absorption->setText(tr("Default Absorption"));
   label_hopUtilization->setText(tr("Hop Utilization "));
   label_boilingPoint->setText(tr("Boiling Point of Water"));
   groupBox_mashTun->setTitle(tr("Mash Tun"));
   label_tunVolume->setText(tr("Volume"));
   label_tunWeight->setText(tr("Mass"));
   label_tunSpecificHeat->setText(QApplication::translate("equipmentEditor", "Specific heat (cal/(g*K))", 0));
   groupBox_losses->setTitle(QApplication::translate("equipmentEditor", "Losses", 0));
   groupBox_losses->setProperty("configSection", QVariant(QApplication::translate("equipmentEditor", "equipmentEditor", 0)));
   label_trubChillerLoss->setText(QApplication::translate("equipmentEditor", "Kettle to fermenter", 0));
   label_lauterDeadspace->setText(QApplication::translate("equipmentEditor", "Lauter deadspace", 0));
   pushButton_new->setText(QString());
   pushButton_save->setText(QString());
   pushButton_cancel->setText(QString());
#ifndef QT_NO_TOOLTIP
   pushButton_remove->setToolTip(tr("Remove equipment"));
   lineEdit_name->setToolTip(tr("Name"));
   lineEdit_boilSize->setToolTip(tr("Pre-boil volume"));
   label_calcBoilVolume->setToolTip(tr("If checked, we will calculate your pre-boil volume based on your desired batch size, boil time, evaporation rate, losses, etc."));
   checkBox_calcBoilVolume->setToolTip(tr("Automatically fill in pre-boil volume"));
   lineEdit_batchSize->setToolTip(tr("Batch size"));
   lineEdit_boilTime->setToolTip(tr("Boil time"));
   lineEdit_evaporationRate->setToolTip(tr("How much water boils off per hour"));
   lineEdit_topUpKettle->setToolTip(tr("How much water is added to kettle immediately pre-boil"));
   lineEdit_topUpWater->setToolTip(tr("Water added to fermenter"));
   lineEdit_tunVolume->setToolTip(tr("Volume of mash tun"));
   lineEdit_tunWeight->setToolTip(tr("Mass or weight of mash tun"));
   lineEdit_trubChillerLoss->setToolTip(tr("Wort lost between kettle and fermenter"));
   lineEdit_lauterDeadspace->setToolTip(tr("Volume of wort lost to lauter deadspace"));
   pushButton_new->setToolTip(tr("New equipment"));
   pushButton_save->setToolTip(tr("Save"));
   pushButton_cancel->setToolTip(tr("Cancel"));
#endif // QT_NO_TOOLTIP
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
   bool ok = false;

   double grainAbs = Brewtarget::toDouble( lineEdit_grainAbsorption->text(), &ok );
   if ( ! ok )
      Brewtarget::logW( QString("EquipmentEditor::save() could not convert %1 to double").arg(lineEdit_grainAbsorption->text()));

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
