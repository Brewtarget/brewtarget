/*
 * HydrometerTool.cpp is part of Brewtarget and was written by Ryan Hoobler
 * (rhoob@yahoo.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2016.
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
#include "unit.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpacerItem>
#include <math.h>
#include <brewtarget.h>

HydrometerTool::HydrometerTool(QWidget* parent) : QDialog(parent)
{
   doLayout();
   
   connect( pushButton_convert, &QAbstractButton::clicked, this, &HydrometerTool::convert );
   connect(label_inputTemp, &BtLabel::labelChanged, lineEdit_inputTemp, &BtLineEdit::lineChanged);
   connect(label_inputSg, &BtLabel::labelChanged, lineEdit_inputSg, &BtLineEdit::lineChanged);
   connect(label_outputSg, &BtLabel::labelChanged, lineEdit_outputSg, &BtLineEdit::lineChanged);

   QMetaObject::connectSlotsByName(this);

}

void HydrometerTool::doLayout()
{
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


    label_outputSg = new BtDensityLabel(groupBox_inputSg);
        label_outputSg ->setContextMenuPolicy(Qt::CustomContextMenu);

    lineEdit_outputSg = new BtDensityEdit(groupBox_inputSg);
        lineEdit_outputSg->setMinimumSize(QSize(80, 0));
        lineEdit_outputSg->setMaximumSize(QSize(80, 16777215));
        lineEdit_outputSg->setProperty("forcedUnit",QVariant(QStringLiteral("displaySG")));
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
  formLayout->setWidget(2, QFormLayout::LabelRole, label_outputSg);
  formLayout->setWidget(2, QFormLayout::FieldRole, lineEdit_outputSg);


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
}

void HydrometerTool::retranslateUi()
{
   setWindowTitle(tr("Hydrometer Tool"));
   label_inputSg->setText(tr("SG Reading")); //TODO translation
   label_inputTemp->setText(tr("Temperature"));  //TODO translation
   label_outputSg->setText(tr("Adjust SG"));  //TODO translation

   pushButton_convert->setText(tr("Convert"));
#ifndef QT_NO_TOOLTIP
   lineEdit_inputSg->setToolTip(tr("Measured gravity"));  //TODO translate
   lineEdit_inputTemp->setToolTip(tr("Temperature"));  //TODO translate
   lineEdit_outputSg->setToolTip(tr("Corrected gravity"));  //TODO translate


#endif // QT_NO_TOOLTIP
}

void HydrometerTool::convert()
{
   /*
   cg = corrected gravity
   mg = measured gravity
   tr = temperature at time of reading in Fahrenheit
   tc = calibration temperature of hydrometer in Fahrenheit*/

   double cg;
   double mg;
   double tr;
   double tc = 60;

   tr = lineEdit_inputTemp->toSI();  //does not always return C
   tr = tr * 1.8 + 32;  //formula below uses Fahrenheit
   mg = lineEdit_inputSg->toSI();

   //formula from http://www.straighttothepint.com/hydrometer-temperature-correction/
   cg = mg * ((1.00130346 - 0.000134722124 * tr + 0.00000204052596 * pow(tr,2) - 0.00000000232820948 * pow(tr,3))
         / (1.00130346 - 0.000134722124 * tc + 0.00000204052596 * pow(tc,2) - 0.00000000232820948 * pow(tc,3)));

   lineEdit_outputSg->setText(cg);
}
