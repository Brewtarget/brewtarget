/*
 * HopEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QtGui>
#include <iostream>
#include <string>
#include "HopEditor.h"
#include "stringparsing.h"
#include "database.h"

HopEditor::HopEditor( QWidget* parent )
{
   setupUi(this);

   setWindowIcon(parent->windowIcon());
   
   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( save() ));
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( clearAndClose() ));

   obsHop = 0;
}

void HopEditor::setHop( Hop* h )
{
   if( h && h != obsHop )
   {
      obsHop = h;
      setObserved(h);
      showChanges();
   }
}

void HopEditor::save()
{
   Hop *h = obsHop;

   if( h == 0 )
      return;

   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   h->disableNotification();

   h->setName(lineEdit_name->text().toStdString());
   h->setAlpha_pct(parseDouble(lineEdit_alpha->text().toStdString()));
   h->setAmount_kg(parseDouble(lineEdit_amount->text().toStdString()));
   h->setUse(comboBox_use->currentText().toStdString());
   h->setTime_min(parseDouble(lineEdit_time->text().toStdString()));
   h->setType(comboBox_type->currentText().toStdString());
   h->setForm(comboBox_form->currentText().toStdString());
   h->setBeta_pct(parseDouble(lineEdit_beta->text().toStdString()));
   h->setHsi_pct(parseDouble(lineEdit_HSI->text().toStdString()));
   h->setOrigin(lineEdit_origin->text().toStdString());
   h->setHumulene_pct(parseDouble(lineEdit_humulene->text().toStdString()));
   h->setCaryophyllene_pct(parseDouble(lineEdit_caryophyllene->text().toStdString()));
   h->setCohumulone_pct(parseDouble(lineEdit_cohumulone->text().toStdString()));
   h->setMyrcene_pct(parseDouble(lineEdit_myrcene->text().toStdString()));

   h->setSubstitutes(textEdit_substitutes->toPlainText().toStdString());
   h->setNotes(textEdit_notes->toPlainText().toStdString());

   h->reenableNotification();
   h->forceNotify();

   Database::getDatabase()->resortAll(); // If the name changed, need to resort.
}

void HopEditor::clearAndClose()
{
   if( obsHop )
   {
      obsHop->removeObserver(this);
      obsHop = 0;
   }
   setVisible(false); // Hide the window.
}

void HopEditor::notify(Observable* notifier)
{
   if( notifier == obsHop )
      showChanges();
}

void HopEditor::showChanges()
{
   Hop* h = obsHop;
   if( h == 0 )
      return;

   int tmp;

   lineEdit_name->setText(h->getName().c_str());
   lineEdit_name->setCursorPosition(0);
   lineEdit_alpha->setText(QString::number(h->getAlpha_pct()));
   lineEdit_amount->setText(QString::number(h->getAmount_kg()));
   tmp = comboBox_use->findText(h->getUse().c_str());
   comboBox_use->setCurrentIndex(tmp);
   lineEdit_time->setText(QString::number(h->getTime_min()));
   tmp = comboBox_type->findText(h->getType().c_str());
   comboBox_type->setCurrentIndex(tmp);
   tmp = comboBox_form->findText(h->getForm().c_str());
   comboBox_form->setCurrentIndex(tmp);
   lineEdit_beta->setText(QString::number(h->getBeta_pct()));
   lineEdit_HSI->setText(QString::number(h->getHsi_pct()));
   lineEdit_origin->setText(h->getOrigin().c_str());
   lineEdit_origin->setCursorPosition(0);
   lineEdit_humulene->setText(QString::number(h->getHumulene_pct()));
   lineEdit_caryophyllene->setText(QString::number(h->getCaryophyllene_pct()));
   lineEdit_cohumulone->setText(QString::number(h->getCohumulone_pct()));
   lineEdit_myrcene->setText(QString::number(h->getMyrcene_pct()));
   textEdit_substitutes->setPlainText(h->getSubstitutes().c_str());
   textEdit_notes->setPlainText(h->getNotes().c_str());
}
