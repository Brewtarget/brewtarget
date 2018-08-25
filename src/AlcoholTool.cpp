 /*
 * AlcoholTool.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2018
 * - Ryan Hoobler <rhoob@yahoo.com>
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

#include "AlcoholTool.h"
#include "unit.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpacerItem>
#include <math.h>
#include <brewtarget.h>
#include "BtLineEdit.h"

AlcoholTool::AlcoholTool(QWidget* parent) : QDialog(parent)
{
   doLayout();
   connect( pushButton_convert, SIGNAL(clicked()), this, SLOT(convert()) );
}

void AlcoholTool::doLayout()
{
   resize(279, 96);
   QHBoxLayout* hLayout = new QHBoxLayout(this);
      QFormLayout* formLayout = new QFormLayout();
         label_og = new QLabel(this);
         lineEdit_og = new BtDensityEdit(this);
            lineEdit_og->setMinimumSize(QSize(80, 0));
            lineEdit_og->setMaximumSize(QSize(80, 16777215));
            lineEdit_og->setProperty("forcedUnit",QVariant(QStringLiteral("displaySG")));

        label_fg = new QLabel(this);
        lineEdit_fg = new BtDensityEdit(this);
           lineEdit_fg->setMinimumSize(QSize(80, 0));
           lineEdit_fg->setMaximumSize(QSize(80, 16777215));
           lineEdit_fg->setProperty("forcedUnit",QVariant(QStringLiteral("displaySG")));

         label_result = new QLabel(this);
            label_result ->setObjectName(QStringLiteral("label_results"));
            label_result ->setContextMenuPolicy(Qt::CustomContextMenu);

         lineEdit_result = new QLineEdit(this);
            lineEdit_result->setMinimumSize(QSize(80, 0));
            lineEdit_result->setMaximumSize(QSize(80, 16777215));
            lineEdit_result->setReadOnly(true);
            lineEdit_result->setObjectName(QStringLiteral("lineEdit_result"));


         formLayout->setWidget(0, QFormLayout::LabelRole, label_og);
         formLayout->setWidget(0, QFormLayout::FieldRole, lineEdit_og);
         formLayout->setWidget(1, QFormLayout::LabelRole, label_fg);
         formLayout->setWidget(1, QFormLayout::FieldRole, lineEdit_fg);
         formLayout->setWidget(2, QFormLayout::LabelRole, label_result);
         formLayout->setWidget(2, QFormLayout::FieldRole, lineEdit_result);


         formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
         QVBoxLayout* vLayout = new QVBoxLayout();
         QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
         pushButton_convert = new QPushButton(this);
            pushButton_convert->setAutoDefault(false);
            pushButton_convert->setDefault(true);
         QSpacerItem* verticalSpacer2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
         vLayout->addItem(verticalSpacer);
         vLayout->addWidget(pushButton_convert);
         vLayout->addItem(verticalSpacer2);

         QSpacerItem* verticalSpacer3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
         vLayout->addItem(verticalSpacer3);

   hLayout->addLayout(formLayout);
   hLayout->addLayout(vLayout);


   retranslateUi();
}

void AlcoholTool::retranslateUi()
{
   setWindowTitle(tr("Alcohol Tool"));
   label_og->setText(tr("OG Reading")); //TODO translation
   label_result->setText(tr("ABV"));  //TODO translation
   label_fg->setText(tr("FG Reading"));  //TODO translation

   pushButton_convert->setText(tr("Calculate"));
#ifndef QT_NO_TOOLTIP
   lineEdit_og->setToolTip(tr("Initial Reading"));  //TODO translate
   lineEdit_result->setToolTip(tr("Result"));  //TODO translate
   lineEdit_fg->setToolTip(tr("Final Reading"));  //TODO translate


#endif // QT_NO_TOOLTIP
}

void AlcoholTool::convert()
{
    double abv = 0;
    double og = 0;
    double fg = 0;

    og = lineEdit_og->toSI();
    fg = lineEdit_fg->toSI();

    abv =(76.08 * (og-fg) / (1.775-og)) * (fg / 0.794);
   //http://www.brewersfriend.com/2011/06/16/alcohol-by-volume-calculator-updated/

   lineEdit_result->setText(QString::number(abv, 'f', 1).append("%"));
}
