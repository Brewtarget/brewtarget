/*
 * EquipmentEditor.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - David Grundberg <individ@acc.umu.se>
 * - Jeff Bailey <skydvr38@verizon.net>
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

#ifndef _EQUIPMENTEDITOR_H
#define _EQUIPMENTEDITOR_H

#include <QDialog>
#include <QMetaProperty>
#include <QVariant>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QCheckBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QEvent>

#include "BtLabel.h"
// Forward declarations
class BtGenericEdit;
class BtMassEdit;
class BtMassLabel;
class BtTemperatureEdit;
class BtTimeLabel;
class BtTimeEdit;
class BtVolumeLabel;
class BtVolumeEdit;
class Equipment;
class EquipmentListModel;
class IngredientSortProxyModel;

/*!
 * \class EquipmentEditor
 * \author Philip G. Lee
 *
 * \brief This is a dialog that edits an equipment record.
 */
class EquipmentEditor : public QDialog
{
   Q_OBJECT

public:
   //! \param singleEquipEditor true if you do not want the necessary elements for viewing all the database elements.
   EquipmentEditor( QWidget *parent=nullptr, bool singleEquipEditor=false );
   virtual ~EquipmentEditor() {}

   //! \name Public UI Variables
   //! @{
   QVBoxLayout *verticalLayout_6;
   QVBoxLayout *topVLayout;
   QHBoxLayout *horizontalLayout_equipments;
   QLabel *label;
   QComboBox *equipmentComboBox;
   QPushButton *pushButton_remove;
   QSpacerItem *horizontalSpacer;
   QCheckBox *checkBox_defaultEquipment;
   QHBoxLayout *horizontalLayout;
   QVBoxLayout *vLayout_left;
   QGroupBox *groupBox_required;
   QVBoxLayout *verticalLayout;
   QFormLayout *formLayout;
   QLabel *label_name;
   QLineEdit *lineEdit_name;
   BtVolumeLabel *label_boilSize;
   BtVolumeEdit *lineEdit_boilSize;
   QLabel *label_calcBoilVolume;
   QCheckBox *checkBox_calcBoilVolume;
   BtVolumeLabel *label_batchSize;
   BtVolumeEdit *lineEdit_batchSize;
   QGroupBox *groupBox_water;
   QVBoxLayout *verticalLayout_3;
   QFormLayout *formLayout_water;
   BtTimeLabel *label_boilTime;
   BtTimeEdit *lineEdit_boilTime;
   BtVolumeLabel *label_evaporationRate;
   BtVolumeEdit *lineEdit_evaporationRate;
   BtVolumeLabel *label_topUpKettle;
   BtVolumeEdit *lineEdit_topUpKettle;
   BtVolumeLabel *label_topUpWater;
   BtVolumeEdit *lineEdit_topUpWater;
   QLabel *label_absorption;
   BtGenericEdit *lineEdit_grainAbsorption;
   QPushButton *pushButton_absorption;
   BtTemperatureEdit *lineEdit_boilingPoint;
   QLabel *label_hopUtilization;
   BtGenericEdit *lineEdit_hopUtilization;
   BtTemperatureLabel *label_boilingPoint;
   QSpacerItem *verticalSpacer_2;
   QVBoxLayout *vLayout_right;
   QGroupBox *groupBox_mashTun;
   QFormLayout *formLayout_mashTun;
   BtVolumeLabel *label_tunVolume;
   BtVolumeEdit *lineEdit_tunVolume;
   BtMassLabel *label_tunWeight;
   BtMassEdit *lineEdit_tunWeight;
   QLabel *label_tunSpecificHeat;
   BtGenericEdit *lineEdit_tunSpecificHeat;
   QGroupBox *groupBox_losses;
   QVBoxLayout *verticalLayout_4;
   QFormLayout *formLayout_losses;
   BtVolumeLabel *label_trubChillerLoss;
   BtVolumeEdit *lineEdit_trubChillerLoss;
   BtVolumeLabel *label_lauterDeadspace;
   BtVolumeEdit *lineEdit_lauterDeadspace;
   QGroupBox *groupBox_notes;
   QVBoxLayout *verticalLayout_notes;
   QTextEdit *textEdit_notes;
   QSpacerItem *verticalSpacer;
   QHBoxLayout *hLayout_buttons;
   QSpacerItem *horizontalSpacer_2;
   QPushButton *pushButton_new;
   QPushButton *pushButton_save;
   QPushButton *pushButton_cancel;
   //! @}

   //! Edit the given equipment.
   void setEquipment( Equipment* e );

   void newEquipment(QString folder);

public slots:
   //! Save the changes to the equipment.
   void save();
   //! Create a new equipment record.
   void newEquipment();
   //! Delete the equipment from the database.
   void removeEquipment();
   //! Set the equipment to default values.
   void clear();
   //! Close the dialog, throwing away changes.
   void cancel();
   //! Set absorption back to default.
   void resetAbsorption();

   //! Edit the equipment currently selected in our combobox.
   void equipmentSelected();
   //! If state==Qt::Checked, set the "calculate boil volume" checkbox. Otherwise, unset.
   void updateCheckboxRecord();
   //! \brief set the default equipment, or unset the current equipment as the default
   void updateDefaultEquipment(int state);

   void changed(QMetaProperty,QVariant);

   double calcBatchSize();

protected:
   //! User closed the dialog
   void closeEvent(QCloseEvent *event);

   virtual void changeEvent(QEvent* event)
   {
      if(event->type() == QEvent::LanguageChange)
         retranslateUi();
      QDialog::changeEvent(event);
   }

private:
   Equipment* obsEquip;
   EquipmentListModel* equipmentListModel;
   IngredientSortProxyModel* equipmentSortProxyModel;

   void showChanges();

   void doLayout();
   void retranslateUi();
};

#endif   /* _EQUIPMENTEDITOR_H */

