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
#include <QIcon>
#include "YeastEditor.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"

YeastEditor::YeastEditor( QWidget* /*parent*/ )
{
   setupUi(this);

   setWindowIcon(QIcon(SMALLYEAST));
   
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
   {
      setVisible(false);
      return;
   }

   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   y->disableNotification();

   y->setName(lineEdit_name->text().toStdString());
   y->setType(static_cast<Yeast::Type>(comboBox_type->currentIndex()));
   y->setForm(static_cast<Yeast::Form>(comboBox_form->currentIndex()));
   y->setAmountIsWeight( (checkBox_amountIsWeight->checkState() == Qt::Checked)? true : false );
   y->setAmount( y->getAmountIsWeight() ? Brewtarget::weightQStringToSI(lineEdit_amount->text()) : Brewtarget::volQStringToSI(lineEdit_amount->text()) );

   y->setLaboratory( lineEdit_laboratory->text().toStdString() );
   y->setProductID( lineEdit_productID->text().toStdString() );
   y->setMinTemperature_c( Brewtarget::tempQStringToSI(lineEdit_minTemperature->text()) );
   y->setMaxTemperature_c( Brewtarget::tempQStringToSI(lineEdit_maxTemperature->text()) );
   y->setFlocculation( static_cast<Yeast::Flocculation>(comboBox_flocculation->currentIndex()) );
   y->setAttenuation_pct(lineEdit_attenuation->text().toDouble());
   y->setTimesCultured(lineEdit_timesCultured->text().toInt());
   y->setMaxReuse(lineEdit_maxReuse->text().toInt());
   y->setAddToSecondary( (checkBox_addToSecondary->checkState() == Qt::Checked)? true : false );
   y->setNotes(textEdit_notes->toPlainText().toStdString());

   y->reenableNotification();
   y->forceNotify();

   Database::getDatabase()->resortYeasts(); // If the name changed, need to resort.

   setVisible(false);
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

void YeastEditor::notify(Observable* notifier, QVariant info)
{
   if( notifier == obsYeast )
      showChanges();
}

void YeastEditor::showChanges()
{
   Yeast* y = obsYeast;
   if( y == 0 )
      return;

   lineEdit_name->setText(y->getName().c_str());
   lineEdit_name->setCursorPosition(0);
   comboBox_type->setCurrentIndex(y->getType());
   comboBox_form->setCurrentIndex(y->getForm());
   lineEdit_amount->setText( Brewtarget::displayAmount(y->getAmount(), (y->getAmountIsWeight()) ? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
   checkBox_amountIsWeight->setCheckState( (y->getAmountIsWeight())? Qt::Checked : Qt::Unchecked );
   lineEdit_laboratory->setText(y->getLaboratory().c_str());
   lineEdit_laboratory->setCursorPosition(0);
   lineEdit_productID->setText(y->getProductID().c_str());
   lineEdit_productID->setCursorPosition(0);
   lineEdit_minTemperature->setText(Brewtarget::displayAmount(y->getMinTemperature_c(), Units::celsius));
   lineEdit_maxTemperature->setText(Brewtarget::displayAmount(y->getMaxTemperature_c(), Units::celsius));
   comboBox_flocculation->setCurrentIndex( y->getFlocculation() );
   lineEdit_attenuation->setText( Brewtarget::displayAmount(y->getAttenuation_pct(), 0));
   lineEdit_timesCultured->setText(QString::number(y->getTimesCultured()));
   lineEdit_maxReuse->setText(QString::number(y->getMaxReuse()));
   checkBox_addToSecondary->setCheckState( (y->getAddToSecondary())? Qt::Checked : Qt::Unchecked );

   textEdit_bestFor->setPlainText(y->getBestFor().c_str());
   textEdit_notes->setPlainText(y->getNotes().c_str());
}
