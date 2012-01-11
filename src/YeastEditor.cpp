/*
 * YeastEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#include "YeastEditor.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"
#include "yeast.h"

YeastEditor::YeastEditor( QWidget* parent )
   : QDialog(parent), obsYeast(0)
{
   setupUi(this);
   
   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( save() ));
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( clearAndClose() ));
}

void YeastEditor::setYeast( Yeast* y )
{
   if( obsYeast )
      disconnect( obsYeast, 0, this, 0 );
   
   obsYeast = y;
   if( obsYeast )
   {
      connect( obsYeast, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void YeastEditor::save()
{
   Yeast* y = obsYeast;

   if( y == 0 )
   {
      setVisible(false);
      return;
   }

   // TODO: check this out with 1.2.5
   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   //y->disableNotification();

   y->setName(lineEdit_name->text());
   y->setType(static_cast<Yeast::Type>(comboBox_type->currentIndex()));
   y->setForm(static_cast<Yeast::Form>(comboBox_form->currentIndex()));
   y->setAmountIsWeight( (checkBox_amountIsWeight->checkState() == Qt::Checked)? true : false );
   y->setAmount( y->amountIsWeight() ? Brewtarget::weightQStringToSI(lineEdit_amount->text()) : Brewtarget::volQStringToSI(lineEdit_amount->text()) );

   y->setLaboratory( lineEdit_laboratory->text() );
   y->setProductID( lineEdit_productID->text() );
   y->setMinTemperature_c( Brewtarget::tempQStringToSI(lineEdit_minTemperature->text()) );
   y->setMaxTemperature_c( Brewtarget::tempQStringToSI(lineEdit_maxTemperature->text()) );
   y->setFlocculation( static_cast<Yeast::Flocculation>(comboBox_flocculation->currentIndex()) );
   y->setAttenuation_pct(lineEdit_attenuation->text().toDouble());
   y->setTimesCultured(lineEdit_timesCultured->text().toInt());
   y->setMaxReuse(lineEdit_maxReuse->text().toInt());
   y->setAddToSecondary( (checkBox_addToSecondary->checkState() == Qt::Checked)? true : false );
   y->setBestFor(textEdit_bestFor->toPlainText());
   y->setNotes(textEdit_notes->toPlainText()); 

   //y->reenableNotification();
   //y->forceNotify();

   setVisible(false);
}

void YeastEditor::clearAndClose()
{
   setYeast(0);
   setVisible(false); // Hide the window.
}

void YeastEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == obsYeast )
      showChanges(&prop);
}

void YeastEditor::showChanges(QMetaProperty* metaProp)
{
   Yeast* y = obsYeast;
   if( y == 0 )
      return;

   QString propName;
   QVariant value;
   bool updateAll = false;
   if( metaProp == 0 )
      updateAll = true;
   else
   {
      propName = metaProp->name();
      value = metaProp->read(y);
   }
   
   if( propName == "name" || updateAll )
   {
      lineEdit_name->setText(value.toString());
      lineEdit_name->setCursorPosition(0);
   }
   else if( propName == "type" || updateAll )
      comboBox_type->setCurrentIndex(value.toInt());
   else if( propName == "form" || updateAll )
      comboBox_form->setCurrentIndex(value.toInt());
   else if( propName == "amount" || updateAll )
      lineEdit_amount->setText( Brewtarget::displayAmount(value.toDouble(), (y->amountIsWeight()) ? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
   else if( propName == "amountIsWeight" || updateAll )
      checkBox_amountIsWeight->setCheckState( (value.toBool())? Qt::Checked : Qt::Unchecked );
   else if( propName == "laboratory" || updateAll )
   {
      lineEdit_laboratory->setText(value.toString());
      lineEdit_laboratory->setCursorPosition(0);
   }
   else if( propName == "productID" || updateAll )
   {
      lineEdit_productID->setText(value.toString());
      lineEdit_productID->setCursorPosition(0);
   }
   else if( propName == "minTemperature_c" || updateAll )
      lineEdit_minTemperature->setText(Brewtarget::displayAmount(value.toDouble(), Units::celsius));
   else if( propName == "maxTemperature_c" || updateAll )
      lineEdit_maxTemperature->setText(Brewtarget::displayAmount(value.toDouble(), Units::celsius));
   else if( propName == "flocculation" || updateAll )
      comboBox_flocculation->setCurrentIndex( value.toInt() );
   else if( propName == "attenutation_pc" || updateAll )
      lineEdit_attenuation->setText( Brewtarget::displayAmount(value.toDouble(), 0));
   else if( propName == "timesCultured" || updateAll )
      lineEdit_timesCultured->setText(QString::number(value.toInt()));
   else if( propName == "maxReuse" || updateAll )
      lineEdit_maxReuse->setText(QString::number(value.toInt()));
   else if( propName == "addToSecondary" || updateAll )
      checkBox_addToSecondary->setCheckState( (value.toBool())? Qt::Checked : Qt::Unchecked );
   else if( propName == "bestFor" || updateAll )
      textEdit_bestFor->setPlainText(value.toString());
   else if( propName == "notes" || updateAll )
      textEdit_notes->setPlainText(value.toString());
}
