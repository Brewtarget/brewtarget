/*
 * YeastEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <string>
#include "YeastEditor.h"
#include "stringparsing.h"
#include "database.h"

YeastEditor::YeastEditor( QWidget* parent )
{
   setupUi(this);

   setWindowIcon(parent->windowIcon());
   
   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( save() ));
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( clearAndClose() ));

   obsYeast = 0;
}

void YeastEditor::setYeast( Yeast* y )
{
   if( y && y != obsYeast )
   {
      obsYeast = y;
      setObserved(y);
      showChanges();
   }
}

void YeastEditor::save()
{
   Yeast *y = obsYeast;

   if( y == 0 )
      return;

   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   y->disableNotification();

   y->setName(lineEdit_name->text().toStdString());
   y->setType(comboBox_type->currentText().toStdString());
   y->setForm(comboBox_form->currentText().toStdString());
   y->setAmount(parseDouble(lineEdit_amount->text().toStdString()));

   y->setAmountIsWeight( (checkBox_amountIsWeight->checkState() == Qt::Checked)? true : false );
   y->setLaboratory( lineEdit_laboratory->text().toStdString() );
   y->setProductID( lineEdit_productID->text().toStdString() );
   y->setMinTemperature_c( parseDouble(lineEdit_minTemperature->text().toStdString()) );
   y->setMaxTemperature_c( parseDouble(lineEdit_maxTemperature->text().toStdString()) );
   y->setFlocculation( comboBox_flocculation->currentText().toStdString() );
   y->setAttenuation_pct(parseDouble(lineEdit_attenuation->text().toStdString()));
   y->setTimesCultured(parseInt(lineEdit_timesCultured->text().toStdString()));
   y->setMaxReuse(parseInt(lineEdit_maxReuse->text().toStdString()));
   y->setAddToSecondary( (checkBox_addToSecondary->checkState() == Qt::Checked)? true : false );
   y->setNotes(textEdit_notes->toPlainText().toStdString());

   y->reenableNotification();
   y->forceNotify();

   Database::getDatabase()->resortAll(); // If the name changed, need to resort.
}

void YeastEditor::clearAndClose()
{
   if( obsYeast )
   {
      obsYeast->removeObserver(this);
      obsYeast = 0;
   }
   setVisible(false); // Hide the window.
}

void YeastEditor::notify(Observable* notifier)
{
   if( notifier == obsYeast )
      showChanges();
}

void YeastEditor::showChanges()
{
   Yeast* y = obsYeast;
   if( y == 0 )
      return;

   int tmp;

   lineEdit_name->setText(y->getName().c_str());
   lineEdit_name->setCursorPosition(0);
   tmp = comboBox_type->findText(y->getType().c_str());
   comboBox_type->setCurrentIndex(tmp);
   tmp = comboBox_form->findText(y->getForm().c_str());
   comboBox_form->setCurrentIndex(tmp);
   lineEdit_amount->setText( QString::number(y->getAmount()) );
   checkBox_amountIsWeight->setCheckState( (y->getAmountIsWeight())? Qt::Checked : Qt::Unchecked );
   lineEdit_laboratory->setText(y->getLaboratory().c_str());
   lineEdit_laboratory->setCursorPosition(0);
   lineEdit_productID->setText(y->getProductID().c_str());
   lineEdit_productID->setCursorPosition(0);
   lineEdit_minTemperature->setText(QString::number(y->getMinTemperature_c()));
   lineEdit_maxTemperature->setText(QString::number(y->getMaxTemperature_c()));
   tmp = comboBox_flocculation->findText(y->getFlocculation().c_str());
   comboBox_flocculation->setCurrentIndex(tmp);
   lineEdit_attenuation->setText( QString::number(y->getAttenuation_pct()));
   lineEdit_timesCultured->setText(QString::number(y->getTimesCultured()));
   lineEdit_maxReuse->setText(QString::number(y->getMaxReuse()));
   checkBox_addToSecondary->setCheckState( (y->getAddToSecondary())? Qt::Checked : Qt::Unchecked );

   textEdit_bestFor->setPlainText(y->getBestFor().c_str());
   textEdit_notes->setPlainText(y->getNotes().c_str());
}
