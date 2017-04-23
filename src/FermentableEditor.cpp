/*
 * FermentableEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include <QIcon>
#include "FermentableEditor.h"
#include "fermentable.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"

FermentableEditor::FermentableEditor( QWidget* parent )
        : QDialog(parent), obsFerm(nullptr)
{
   setupUi(this);

   connect( this, &QDialog::accepted, this, &FermentableEditor::save);
   connect( this, &QDialog::rejected, this, &FermentableEditor::clearAndClose);

}

void FermentableEditor::setFermentable( Fermentable* newFerm )
{
   if(newFerm)
   {
      obsFerm = newFerm;
      showChanges();
   }
}

void FermentableEditor::save()
{
   if( !obsFerm )
   {
      setVisible(false);
      return;
   }

   obsFerm->setName(lineEdit_name->text());

   // NOTE: the following assumes that Fermentable::Type is enumerated in the same
   // order as the combobox.
   obsFerm->setType( static_cast<Fermentable::Type>(comboBox_type->currentIndex()) );

   obsFerm->setAmount_kg(lineEdit_amount->toSI());
   obsFerm->setInventoryAmount(lineEdit_inventory->toSI());
   obsFerm->setYield_pct(lineEdit_yield->toSI());
   obsFerm->setColor_srm(lineEdit_color->toSI());
   obsFerm->setAddAfterBoil( (checkBox_addAfterBoil->checkState() == Qt::Checked)? true : false );
   obsFerm->setOrigin( lineEdit_origin->text() );
   obsFerm->setSupplier( lineEdit_supplier->text() );
   obsFerm->setCoarseFineDiff_pct( lineEdit_coarseFineDiff->toSI() );
   obsFerm->setMoisture_pct( lineEdit_moisture->toSI() );
   obsFerm->setDiastaticPower_lintner( lineEdit_diastaticPower->toSI() );
   obsFerm->setProtein_pct( lineEdit_protein->toSI() );
   obsFerm->setMaxInBatch_pct( lineEdit_maxInBatch->toSI() );
   obsFerm->setRecommendMash( (checkBox_recommendMash->checkState() == Qt::Checked) ? true : false );
   obsFerm->setIsMashed( (checkBox_isMashed->checkState() == Qt::Checked) ? true : false );
   obsFerm->setIbuGalPerLb( lineEdit_ibuGalPerLb->toSI() );
   obsFerm->setNotes( textEdit_notes->toPlainText() );
   obsFerm->save();

   setVisible(false);
}

void FermentableEditor::clearAndClose()
{
   setFermentable(0);
   setVisible(false); // Hide the window.
}

void FermentableEditor::showChanges(QMetaProperty* metaProp)
{
   if( !obsFerm )
      return;

   QString propName;
   bool updateAll = false;
   if( metaProp == 0 )
      updateAll = true;
   else
   {
      propName = metaProp->name();
   }

   if( propName == "name" || updateAll )
   {
      lineEdit_name->setText(obsFerm->name());
      lineEdit_name->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == "type" || updateAll) {
      // NOTE: assumes the comboBox entries are in same order as Fermentable::Type
      comboBox_type->setCurrentIndex(obsFerm->type());
       if( ! updateAll )
         return;
   }
   if( propName == "amount_kg" || updateAll) {
      lineEdit_amount->setText(obsFerm);
      if( ! updateAll )
         return;
   }

   if( propName == "inventory" || updateAll) {
      lineEdit_inventory->setText(obsFerm);
      if( ! updateAll )
         return;
   }
   if( propName == "yield_pct" || updateAll) {
      lineEdit_yield->setText(obsFerm);
      if( ! updateAll )
         return;
   }
   if( propName == "color_srm" || updateAll) {
      lineEdit_color->setText(obsFerm, 0);
       if( ! updateAll )
         return;
   }
   if( propName == "addAfterBoil" || updateAll) {
      checkBox_addAfterBoil->setCheckState( obsFerm->addAfterBoil() ? Qt::Checked : Qt::Unchecked );
       if( ! updateAll )
         return;
   }
   if( propName == "origin" || updateAll)
   {
      lineEdit_origin->setText(obsFerm->origin());
      lineEdit_origin->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == "supplier" || updateAll)
   {
      lineEdit_supplier->setText(obsFerm->supplier());
      lineEdit_supplier->setCursorPosition(0);
       if( ! updateAll )
         return;
   }
   if( propName == "coarseFineDiff_pct" || updateAll) {
      lineEdit_coarseFineDiff->setText(obsFerm);
      if( ! updateAll )
         return;
   }
   if( propName == "moisture_pct" || updateAll) {
      lineEdit_moisture->setText(obsFerm);
      if( ! updateAll )
         return;
   }
   if( propName == "diastaticPower_lintner" || updateAll) {
      lineEdit_diastaticPower->setText(obsFerm);
      if( ! updateAll )
         return;
   }
   if( propName == "protein_pct" || updateAll) {
      lineEdit_protein->setText(obsFerm);
      if( ! updateAll )
         return;
   }
   if( propName == "maxInBatch_pct" || updateAll) {
      lineEdit_maxInBatch->setText(obsFerm);
      if( ! updateAll )
         return;
   }
   if( propName == "recommendMash" || updateAll) {
      checkBox_recommendMash->setCheckState( obsFerm->recommendMash() ? Qt::Checked : Qt::Unchecked );
      if( ! updateAll )
         return;
   }
   if( propName == "isMashed" || updateAll) {
      checkBox_isMashed->setCheckState( obsFerm->isMashed() ? Qt::Checked : Qt::Unchecked );
       if( ! updateAll )
         return;
   }
   if( propName == "ibuGalPerLb" || updateAll) {
      lineEdit_ibuGalPerLb->setText(obsFerm);
      if( ! updateAll )
         return;
   }
   if( propName == "notes" || updateAll) {
      textEdit_notes->setPlainText( obsFerm->notes() );
      if( ! updateAll )
         return;
   }
}
