/*
 * AlcoholTool.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
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

#include "Algorithms.h"
#include "brewtarget.h"
#include "Unit.h"

AlcoholTool::AlcoholTool(QWidget* parent) : QDialog(parent),
                                            pushButton_convert{new QPushButton  (this)},
                                            label_og          {new QLabel       (this)},
                                            input_og          {new BtDensityEdit(this)},
                                            label_fg          {new QLabel       (this)},
                                            input_fg          {new BtDensityEdit(this)},
                                            label_result      {new QLabel       (this)},
                                            output_result     {new QLabel       (this)},
                                            hLayout           {new QHBoxLayout  (this)},
                                            formLayout        {new QFormLayout  (this)},
                                            vLayout           {new QVBoxLayout  (this)},
                                            verticalSpacer    {new QSpacerItem{20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding}},
                                            verticalSpacer2   {new QSpacerItem{20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding}},
                                            verticalSpacer3   {new QSpacerItem{20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding}} {
   this->doLayout();
   this->output_result->setText("%");
   connect( this->pushButton_convert, SIGNAL(clicked()), this, SLOT(convert()) );
   connect( this->input_og, &BtLineEdit::textModified, this, &AlcoholTool::convert );
   connect( this->input_fg, &BtLineEdit::textModified, this, &AlcoholTool::convert );

   return;
}

AlcoholTool::~AlcoholTool() {
   //
   // Not much for us to do in the destructor.  Per https://doc.qt.io/qt-5/objecttrees.html, "When you create a QObject
   // with another object as parent, it's added to the parent's children() list, and is deleted when the parent is."
   //
   // I think we also do not need to delete QSpacerItem objects after they have been added to a layout.
   //
/*   delete this->verticalSpacer;
   delete this->verticalSpacer2;
   delete this->verticalSpacer3;*/
   return;
}

void AlcoholTool::doLayout() {

   this->input_og->setMinimumSize(QSize(80, 0));
   this->input_og->setProperty("forcedUnit", QVariant(QStringLiteral("displaySG")));

   this->input_fg->setMinimumSize(QSize(80, 0));
   this->input_fg->setProperty("forcedUnit", QVariant(QStringLiteral("displaySG")));

   this->label_result->setObjectName(QStringLiteral("label_results"));
   this->label_result->setContextMenuPolicy(Qt::CustomContextMenu);

   this->output_result->setMinimumSize(QSize(80, 0));
   this->output_result->setObjectName(QStringLiteral("output_result"));

   this->formLayout->setWidget(0, QFormLayout::LabelRole, label_og);
   this->formLayout->setWidget(0, QFormLayout::FieldRole, input_og);
   this->formLayout->setWidget(1, QFormLayout::LabelRole, label_fg);
   this->formLayout->setWidget(1, QFormLayout::FieldRole, input_fg);
   this->formLayout->setWidget(2, QFormLayout::LabelRole, label_result);
   this->formLayout->setWidget(2, QFormLayout::FieldRole, output_result);

   this->formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

   this->pushButton_convert->setAutoDefault(false);
   this->pushButton_convert->setDefault(true);

   this->vLayout->addItem(verticalSpacer);
   this->vLayout->addWidget(pushButton_convert);
   this->vLayout->addItem(verticalSpacer2);
   this->vLayout->addItem(verticalSpacer3);

   this->hLayout->addLayout(formLayout);
   this->hLayout->addLayout(vLayout);

   this->retranslateUi();
   return;
}

void AlcoholTool::retranslateUi() {
   this->setWindowTitle(tr("Alcohol Tool"));
   this->label_og->setText(tr("OG Reading"));
   this->label_result->setText(tr("ABV"));
   this->label_fg->setText(tr("FG Reading"));
   this->pushButton_convert->setText(tr("Calculate"));

#ifndef QT_NO_TOOLTIP
   this->input_og->setToolTip(tr("Initial Reading"));
   this->output_result->setToolTip(tr("Result"));
   this->input_fg->setToolTip(tr("Final Reading"));
#endif // QT_NO_TOOLTIP
   return;
}

void AlcoholTool::convert() {
   double og = this->input_og->toSI();
   double fg = this->input_fg->toSI();
   if (og != 0.0 && fg != 0.0 && og >= fg) {
      double abv = Algorithms::abvFromOgAndFg(og, fg);
      this->output_result->setText(QString::number(abv, 'f', 1).append("%"));
   } else {
      this->output_result->setText("? %");
   }
   return;
}

void AlcoholTool::changeEvent(QEvent* event) {
   if (event->type() == QEvent::LanguageChange) {
      this->retranslateUi();
   }
   QDialog::changeEvent(event);
   return;
}
