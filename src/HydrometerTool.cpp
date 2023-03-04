/*
 * HydrometerTool.cpp is part of Brewtarget and is copyright the following authors
 * 2016-2023
 *   - Brian Rower <brian.rower@gmail.com>
 *   - Jamie Daws <jdelectronics1@gmail.com>
 *   - Matt Young <mfsy@yahoo.com>
 *   - Ryan Hoobler <rhoob@yahoo.com>
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
#include "HydrometerTool.h"

#include <cmath>

#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

#include "Algorithms.h"
#include "measurement/Unit.h"

HydrometerTool::HydrometerTool(QWidget* parent) : QDialog(parent) {
   this->doLayout();

   connect(pushButton_convert, &QAbstractButton::clicked, this, &HydrometerTool::convert );
   connect(label_inputTemp,      &BtLabel::changedSystemOfMeasurementOrScale, lineEdit_inputTemp,      &BtLineEdit::lineChanged);
   connect(label_inputSg,        &BtLabel::changedSystemOfMeasurementOrScale, lineEdit_inputSg,        &BtLineEdit::lineChanged);
   connect(label_outputSg,       &BtLabel::changedSystemOfMeasurementOrScale, lineEdit_outputSg,       &BtLineEdit::lineChanged);
   connect(label_calibratedTemp, &BtLabel::changedSystemOfMeasurementOrScale, lineEdit_calibratedTemp, &BtLineEdit::lineChanged);

   QMetaObject::connectSlotsByName(this);
   return;
}

void HydrometerTool::doLayout() {
   resize(279, 96);
   QHBoxLayout* hLayout = new QHBoxLayout(this);
     QFormLayout* formLayout = new QFormLayout();
     groupBox_inputSg = new QGroupBox(this);
     groupBox_inputSg->setProperty("configSection", QVariant(QStringLiteral("hydrometerTool")));

     label_inputSg = new BtDensityLabel(groupBox_inputSg);
        label_inputSg ->setContextMenuPolicy(Qt::CustomContextMenu);

     lineEdit_inputSg = new BtDensityEdit(groupBox_inputSg);
        lineEdit_inputSg->setMinimumSize(QSize(80, 0));
        lineEdit_inputSg->setMaximumSize(QSize(80, 16777215));

     label_inputTemp = new BtTemperatureLabel(groupBox_inputSg);
        label_inputTemp ->setObjectName(QStringLiteral("label_inputTemp"));
        label_inputTemp ->setContextMenuPolicy(Qt::CustomContextMenu);

     lineEdit_inputTemp = new BtTemperatureEdit(groupBox_inputSg);
        lineEdit_inputTemp->setMinimumSize(QSize(80, 0));
        lineEdit_inputTemp->setMaximumSize(QSize(80, 16777215));
        lineEdit_inputTemp->setObjectName(QStringLiteral("lineEdit_inputTemp"));

     label_calibratedTemp = new BtTemperatureLabel(groupBox_inputSg);
        label_calibratedTemp ->setObjectName(QStringLiteral("label_calibratedTemp"));
        label_calibratedTemp ->setContextMenuPolicy(Qt::CustomContextMenu);

     lineEdit_calibratedTemp = new BtTemperatureEdit(groupBox_inputSg);
        lineEdit_calibratedTemp->setMinimumSize(QSize(80, 0));
        lineEdit_calibratedTemp->setMaximumSize(QSize(80, 16777215));
        lineEdit_calibratedTemp->setObjectName(QStringLiteral("lineEdit_calibratedTemp"));
        lineEdit_calibratedTemp->setText(15.55555556,1);

    label_outputSg = new BtDensityLabel(groupBox_inputSg);
        label_outputSg ->setContextMenuPolicy(Qt::CustomContextMenu);

    lineEdit_outputSg = new BtDensityEdit(groupBox_inputSg);
        lineEdit_outputSg->setMinimumSize(QSize(80, 0));
        lineEdit_outputSg->setMaximumSize(QSize(80, 16777215));
      lineEdit_outputSg->setForcedSystemOfMeasurement(Measurement::SystemOfMeasurement::SpecificGravity);

        lineEdit_outputSg->setReadOnly(true);

#ifndef QT_NO_SHORTCUT
  label_inputSg->setBuddy(lineEdit_inputSg);
  label_inputTemp->setBuddy(lineEdit_inputTemp);
  label_outputSg->setBuddy(lineEdit_outputSg);
#endif  //QT_NO_SHORTCUT

  formLayout->setWidget(0, QFormLayout::LabelRole, label_inputSg);
  formLayout->setWidget(0, QFormLayout::FieldRole, lineEdit_inputSg);
  formLayout->setWidget(1, QFormLayout::LabelRole, label_inputTemp);
  formLayout->setWidget(1, QFormLayout::FieldRole, lineEdit_inputTemp);
  formLayout->setWidget(2, QFormLayout::LabelRole, label_calibratedTemp);
  formLayout->setWidget(2, QFormLayout::FieldRole, lineEdit_calibratedTemp);
  formLayout->setWidget(3, QFormLayout::LabelRole, label_outputSg);
  formLayout->setWidget(3, QFormLayout::FieldRole, lineEdit_outputSg);

  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  QVBoxLayout* vLayout = new QVBoxLayout();
     QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        pushButton_convert = new QPushButton(this);
           pushButton_convert->setAutoDefault(false);
           pushButton_convert->setDefault(true);
        QSpacerItem* verticalSpacer2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
           vLayout->addItem(verticalSpacer);
           vLayout->addWidget(pushButton_convert);
           vLayout->addWidget(groupBox_inputSg);
           vLayout->addItem(verticalSpacer2);

       QSpacerItem* verticalSpacer3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
           vLayout->addItem(verticalSpacer3);

  hLayout->addLayout(formLayout);
  hLayout->addLayout(vLayout);

  retranslateUi();
  return;
}

void HydrometerTool::retranslateUi() {
   setWindowTitle(tr("Hydrometer Tool"));
   label_inputSg->setText(tr("SG Reading"));
   label_inputTemp->setText(tr("Temperature"));
   label_calibratedTemp->setText(tr("Hydrometer Calibration"));
   label_outputSg->setText(tr("Adjust SG"));

   pushButton_convert->setText(tr("Convert"));
#ifndef QT_NO_TOOLTIP
   lineEdit_inputSg->setToolTip(tr("Measured gravity"));
   lineEdit_inputTemp->setToolTip(tr("Temperature"));
   lineEdit_outputSg->setToolTip(tr("Corrected gravity"));
#endif
   return;
}

void HydrometerTool::convert() {
   double correctedGravity = Algorithms::correctSgForTemperature(
      lineEdit_inputSg->toCanonical().quantity(),       // measured gravity
      lineEdit_inputTemp->toCanonical().quantity(),     // temperature at time of reading in Celsius
      lineEdit_calibratedTemp->toCanonical().quantity() // calibration temperature of hydrometer in Celsius
   );

   lineEdit_outputSg->setText(correctedGravity);
   return;
}

void HydrometerTool::changeEvent(QEvent * event) {
   if (event->type() == QEvent::LanguageChange) {
      this->retranslateUi();
   }
   this->QDialog::changeEvent(event);
   return;
}
