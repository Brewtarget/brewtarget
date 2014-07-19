/*
 * YeastEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

   connect( lineEdit_amount, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_minTemperature, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_maxTemperature, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_attenuation, SIGNAL(editingFinished()), this, SLOT(updateField()));
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
   y->setInventoryQuanta( lineEdit_inventory->text().toInt() );


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

void YeastEditor::updateField()
{

   QObject* selection = sender();
   QLineEdit* field = qobject_cast<QLineEdit*>(selection);
   double val;
  
   if ( field == lineEdit_amount )
   {
      // this is a bit harder, since we have to do something different based
      // on teh "Amount is weight" box
      if ( checkBox_amountIsWeight->checkState() == Qt::Checked ) 
      {
         val = Brewtarget::weightQStringToSI(field->text());
         field->setText(Brewtarget::displayAmount( val, Units::kilograms));
      }
      else 
      {
         val = Brewtarget::volQStringToSI(field->text());
         field->setText(Brewtarget::displayAmount( val, Units::liters));
      }
   }
   else if ( field == lineEdit_minTemperature ||  field == lineEdit_maxTemperature )
   {
      val = Brewtarget::tempQStringToSI(field->text());
      field->setText(Brewtarget::displayAmount( val, Units::celsius));
   }
   else 
   {
      //Everything else is a %, so just format nicely
      val = field->text().toDouble();
      field->setText(Brewtarget::displayAmount(val));
   }

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
      lineEdit_name->setText(obsYeast->name());
      lineEdit_name->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == "type" || updateAll ) {
      comboBox_type->setCurrentIndex(obsYeast->type());
      if( ! updateAll )
         return;
   }
   if( propName == "form" || updateAll ) {
      comboBox_form->setCurrentIndex(obsYeast->form());
      if( ! updateAll )
         return;
   }
   if( propName == "amount" || updateAll ) {
      lineEdit_amount->setText( Brewtarget::displayAmount(obsYeast->amount(), (obsYeast->amountIsWeight()) ? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
      if( ! updateAll )
         return;
   }
   if( propName == "inventory" || updateAll ) {
      lineEdit_inventory->setText( QString::number(obsYeast->inventory()) );
      if( ! updateAll )
         return;
   }
   if( propName == "amountIsWeight" || updateAll ) {
      checkBox_amountIsWeight->setCheckState( (obsYeast->amountIsWeight())? Qt::Checked : Qt::Unchecked );
      if( ! updateAll )
         return;
   }
   if( propName == "laboratory" || updateAll ) {
      lineEdit_laboratory->setText(obsYeast->laboratory());
      lineEdit_laboratory->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == "productID" || updateAll ) {
      lineEdit_productID->setText(obsYeast->productID());
      lineEdit_productID->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == "minTemperature_c" || updateAll ) {
      lineEdit_minTemperature->setText(Brewtarget::displayAmount(obsYeast->minTemperature_c(), Units::celsius));
      if( ! updateAll )
         return;
   }
   if( propName == "maxTemperature_c" || updateAll ) {
      lineEdit_maxTemperature->setText(Brewtarget::displayAmount(obsYeast->maxTemperature_c(), Units::celsius));
      if( ! updateAll )
         return;
   }
   if( propName == "flocculation" || updateAll ) {
      comboBox_flocculation->setCurrentIndex( obsYeast->flocculation() );
      if( ! updateAll )
         return;
   }
   if( propName == "attenutation_pc" || updateAll ) {
      lineEdit_attenuation->setText( Brewtarget::displayAmount(obsYeast->attenuation_pct(), 0));
      if( ! updateAll )
         return;
   }
   if( propName == "timesCultured" || updateAll ) {
      lineEdit_timesCultured->setText(QString::number(obsYeast->timesCultured()));
      if( ! updateAll )
         return;
   }
   if( propName == "maxReuse" || updateAll ) {
      lineEdit_maxReuse->setText(QString::number(obsYeast->maxReuse()));
      if( ! updateAll )
         return;
   }
   if( propName == "addToSecondary" || updateAll ) {
      checkBox_addToSecondary->setCheckState( (obsYeast->addToSecondary())? Qt::Checked : Qt::Unchecked );
      if( ! updateAll )
         return;
   }
   if( propName == "bestFor" || updateAll ) {
      textEdit_bestFor->setPlainText(obsYeast->bestFor());
      if( ! updateAll )
         return;
   }
   if( propName == "notes" || updateAll ) {
      textEdit_notes->setPlainText(obsYeast->notes());
      if( ! updateAll )
         return;
   }
}
