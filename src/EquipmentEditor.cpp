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


#include "EquipmentEditor.h"

#include "config.h"
#include "unit.h"
#include "brewtarget.h"


EquipmentEditor::EquipmentEditor(QWidget* parent, bool singleEquipEditor)
   : QDialog(parent)
{
   doLayout();



   // Connect all the edit boxen
   connect(label_boilingPoint, SIGNAL(labelChanged(Unit::unitDisplay,Unit::unitScale)), lineEdit_boilingPoint, SLOT(lineChanged(Unit::unitDisplay,Unit::unitScale)));

   QMetaObject::connectSlotsByName(this);

}

void EquipmentEditor::doLayout()
{
   resize(0,0);
   topVLayout = new QVBoxLayout(this);
      horizontalLayout = new QHBoxLayout();
         vLayout_left = new QVBoxLayout();
            groupBox_water = new QGroupBox(this);
               groupBox_water->setProperty("configSection", QVariant(QStringLiteral("equipmentEditor")));
               formLayout_water = new QFormLayout(groupBox_water);
                  label_boilingPoint = new BtTemperatureLabel(groupBox_water);
                     label_boilingPoint->setContextMenuPolicy(Qt::CustomContextMenu);
                  lineEdit_boilingPoint = new BtTemperatureEdit(groupBox_water);
                     lineEdit_boilingPoint->setMaximumSize(QSize(100, 16777215));
                  formLayout_water->setWidget(6, QFormLayout::LabelRole, label_boilingPoint);
                  formLayout_water->setWidget(6, QFormLayout::FieldRole, lineEdit_boilingPoint);
            vLayout_left->addWidget(groupBox_water);
         horizontalLayout->addLayout(vLayout_left);

   topVLayout->addLayout(horizontalLayout);


#ifndef QT_NO_SHORTCUT

   label_boilingPoint->setBuddy(lineEdit_boilingPoint);

#endif // QT_NO_SHORTCUT


   retranslateUi();
}

void EquipmentEditor::retranslateUi()
{

   label_boilingPoint->setText(tr("Boiling Point of Water"));


}

void EquipmentEditor::setEquipment( Equipment* e )
{

}

void EquipmentEditor::removeEquipment()
{

}

void EquipmentEditor::clear()
{

}

void EquipmentEditor::equipmentSelected()
{

}

void EquipmentEditor::save()
{

}

void EquipmentEditor::newEquipment()
{

}

void EquipmentEditor::newEquipment(QString folder)
{

}

void EquipmentEditor::cancel()
{

}

void EquipmentEditor::resetAbsorption()
{

}

void EquipmentEditor::changed(QMetaProperty /*prop*/, QVariant /*val*/)
{

}

void EquipmentEditor::showChanges()
{

}

void EquipmentEditor::updateCheckboxRecord()
{

}

double EquipmentEditor::calcBatchSize()
{

}

void EquipmentEditor::updateDefaultEquipment(int state)
{

}

void EquipmentEditor::closeEvent(QCloseEvent *event)
{

}
