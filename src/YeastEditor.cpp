/*
 * YeastEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2020
 * - Kregg K <gigatropolis@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
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

#include "math.h"
#include <QInputDialog>
#include "YeastEditor.h"
#include "BtHorizontalTabs.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"
#include "model/Yeast.h"

YeastEditor::YeastEditor( QWidget* parent )
   : QDialog(parent), obsYeast(nullptr)
{
   setupUi(this);

   tabWidget_editor->tabBar()->setStyle( new BtHorizontalTabs );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newYeast() ) );
   connect( pushButton_save,   &QAbstractButton::clicked, this, &YeastEditor::save );
   connect( pushButton_cancel, &QAbstractButton::clicked, this, &YeastEditor::clearAndClose );
}

void YeastEditor::setYeast( Yeast* y )
{
   if( obsYeast )
      disconnect( obsYeast, nullptr, this, nullptr );

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

   if( y == nullptr )
   {
      setVisible(false);
      return;
   }

   y->setName(lineEdit_name->text(), y->cacheOnly());
   y->setType(static_cast<Yeast::Type>(comboBox_type->currentIndex()));
   y->setForm(static_cast<Yeast::Form>(comboBox_form->currentIndex()));
   y->setAmountIsWeight( (checkBox_amountIsWeight->checkState() == Qt::Checked)? true : false );
   y->setAmount( lineEdit_amount->toSI());

   y->setLaboratory( lineEdit_laboratory->text() );
   y->setProductID( lineEdit_productID->text() );
   y->setMinTemperature_c( lineEdit_minTemperature->toSI());
   y->setMaxTemperature_c( lineEdit_maxTemperature->toSI());
   y->setFlocculation( static_cast<Yeast::Flocculation>(comboBox_flocculation->currentIndex()) );
   y->setAttenuation_pct(lineEdit_attenuation->toSI());

   y->setTimesCultured(lineEdit_timesCultured->text().toInt());
   y->setMaxReuse(lineEdit_maxReuse->text().toInt());
   y->setAddToSecondary( (checkBox_addToSecondary->checkState() == Qt::Checked)? true : false );
   y->setBestFor(textEdit_bestFor->toPlainText());
   y->setNotes(textEdit_notes->toPlainText());

   if ( y->cacheOnly() ) {
      y->insertInDatabase();
   }
   // do this late to make sure we've the row in the inventory table
   y->setInventoryQuanta( lineEdit_inventory->text().toInt() );
   setVisible(false);
}

void YeastEditor::clearAndClose()
{
   setYeast(nullptr);
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
   if( y == nullptr )
      return;

   QString propName;
   QVariant value;
   bool updateAll = false;
   if( metaProp == nullptr )
      updateAll = true;
   else
   {
      propName = metaProp->name();
      value = metaProp->read(y);
   }

   if( propName == PropertyNames::NamedEntity::name || updateAll )
   {
      lineEdit_name->setText(obsYeast->name());
      lineEdit_name->setCursorPosition(0);

      tabWidget_editor->setTabText(0, obsYeast->name());
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
      lineEdit_amount->setText( obsYeast );
      if( ! updateAll )
         return;
   }
   if( propName == "inventory" || updateAll ) {
      lineEdit_inventory->setText( obsYeast->inventory(),0 );
      if( ! updateAll )
         return;
   }
   if( propName == "amountIsWeight" || updateAll ) {
      checkBox_amountIsWeight->setCheckState( (obsYeast->amountIsWeight())? Qt::Checked : Qt::Unchecked );
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::laboratory || updateAll ) {
      lineEdit_laboratory->setText(obsYeast->laboratory());
      lineEdit_laboratory->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::productID || updateAll ) {
      lineEdit_productID->setText(obsYeast->productID());
      lineEdit_productID->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::minTemperature_c || updateAll ) {
      lineEdit_minTemperature->setText(obsYeast);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::maxTemperature_c || updateAll ) {
      lineEdit_maxTemperature->setText(obsYeast);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::flocculation || updateAll ) {
      comboBox_flocculation->setCurrentIndex( obsYeast->flocculation() );
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::attenuation_pct || updateAll ) {
      lineEdit_attenuation->setText( obsYeast);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::timesCultured || updateAll ) {
      lineEdit_timesCultured->setText(obsYeast->timesCultured(),0);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::maxReuse || updateAll ) {
      lineEdit_maxReuse->setText(obsYeast->maxReuse(),0);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::addToSecondary || updateAll ) {
      checkBox_addToSecondary->setCheckState( (obsYeast->addToSecondary())? Qt::Checked : Qt::Unchecked );
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Yeast::bestFor || updateAll ) {
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

void YeastEditor::newYeast()
{
   newYeast(QString());
}

void YeastEditor::newYeast(QString folder)
{
   QString name = QInputDialog::getText(this, tr("Yeast name"),
                                          tr("Yeast name:"));
   if( name.isEmpty() )
      return;

   Yeast* y = new Yeast(name);

   if ( ! folder.isEmpty() )
      y->setFolder(folder);

   setYeast(y);
   show();
}
