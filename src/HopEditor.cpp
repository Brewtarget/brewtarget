/*
 * HopEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QtGui>
#include <QIcon>
#include "hop.h"
#include "HopEditor.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"

HopEditor::HopEditor( QWidget* parent )
   : QDialog(parent), obsHop(0)
{
   setupUi(this);
   
   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( save() ));
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( clearAndClose() ));
}

void HopEditor::setHop( Hop* h )
{
   if( obsHop )
      disconnect( obsHop, 0, this, 0 );
   
   obsHop = h;
   if( obsHop )
   {
      connect( obsHop, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void HopEditor::save()
{
   Hop* h = obsHop;

   if( h == 0 )
   {
      setVisible(false);
      return;
   }

   // TODO: check this out with 1.2.5.
   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   //h->disableNotification();

   h->setName(lineEdit_name->text());
   h->setAlpha_pct(lineEdit_alpha->text().toDouble());
   h->setAmount_kg(Brewtarget::weightQStringToSI(lineEdit_amount->text()));
   h->setInventoryAmount(Brewtarget::weightQStringToSI(lineEdit_inventory->text()));
   h->setUse(static_cast<Hop::Use>(comboBox_use->currentIndex()));
   h->setTime_min(Brewtarget::timeQStringToSI(lineEdit_time->text()));
   h->setType(static_cast<Hop::Type>(comboBox_type->currentIndex()));
   h->setForm(static_cast<Hop::Form>(comboBox_form->currentIndex()));
   h->setBeta_pct(lineEdit_beta->text().toDouble());
   h->setHsi_pct(lineEdit_HSI->text().toDouble());
   h->setOrigin(lineEdit_origin->text());
   h->setHumulene_pct(lineEdit_humulene->text().toDouble());
   h->setCaryophyllene_pct(lineEdit_caryophyllene->text().toDouble());
   h->setCohumulone_pct(lineEdit_cohumulone->text().toDouble());
   h->setMyrcene_pct(lineEdit_myrcene->text().toDouble());

   h->setSubstitutes(textEdit_substitutes->toPlainText());
   h->setNotes(textEdit_notes->toPlainText());

   //h->reenableNotification();
   //h->forceNotify();

   setVisible(false);
}

void HopEditor::clearAndClose()
{
   setHop(0);
   setVisible(false); // Hide the window.
}

void HopEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == obsHop )
      showChanges(&prop);
}

void HopEditor::showChanges(QMetaProperty* prop)
{
   bool updateAll = false;
   QString propName;
   if( obsHop == 0 )
      return;

   if( prop == 0 )
      updateAll = true;
   else
   {
      propName = prop->name();
   }
   
   if( propName == "name" || updateAll )
   {
      lineEdit_name->setText(obsHop->name());
      lineEdit_name->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == "alpha_pct" || updateAll ) {
      lineEdit_alpha->setText(Brewtarget::displayAmount(obsHop->alpha_pct(), 0));
      if( ! updateAll )
         return;
   }
   if( propName == "amount_kg" || updateAll ) {
      lineEdit_amount->setText(Brewtarget::displayAmount(obsHop->amount_kg(), Units::kilograms));
      if( ! updateAll )
         return;
   }
   if( propName == "inventory" || updateAll ) {
      lineEdit_inventory->setText(Brewtarget::displayAmount(obsHop->inventory(), Units::kilograms));
      if( ! updateAll )
         return;
   }
   if( propName == "use" || updateAll ) {
      comboBox_use->setCurrentIndex(obsHop->use());
      if( ! updateAll )
         return;
   }
   if( propName == "time_min" || updateAll ) {
      lineEdit_time->setText(Brewtarget::displayAmount(obsHop->time_min(), Units::minutes));
      if( ! updateAll )
         return;
   }
   if( propName == "type" || updateAll ) {
      comboBox_type->setCurrentIndex(obsHop->type());
      if( ! updateAll )
         return;
   }
   if( propName == "form" || updateAll ) {
      comboBox_form->setCurrentIndex(obsHop->form());
      if( ! updateAll )
         return;
   }
   if( propName == "beta_pct" || updateAll ) {
      lineEdit_beta->setText(Brewtarget::displayAmount(obsHop->beta_pct(), 0));
      if( ! updateAll )
         return;
   }
   if( propName == "hsi_pct" || updateAll ) {
      lineEdit_HSI->setText(Brewtarget::displayAmount(obsHop->hsi_pct(), 0));
      if( ! updateAll )
         return;
   }
   if( propName == "origin" || updateAll )
   {
      lineEdit_origin->setText(obsHop->origin());
      lineEdit_origin->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == "humulene_pct" || updateAll ) {
      lineEdit_humulene->setText(Brewtarget::displayAmount(obsHop->humulene_pct(), 0));
      if( ! updateAll )
         return;
   }
   if( propName == "caryophyllene_pct" || updateAll ) {
      lineEdit_caryophyllene->setText(Brewtarget::displayAmount(obsHop->caryophyllene_pct(), 0));
      if( ! updateAll )
         return;
   }
   if( propName == "cohumulone_pct" || updateAll ) {
      lineEdit_cohumulone->setText(Brewtarget::displayAmount(obsHop->cohumulone_pct(), 0));
      if( ! updateAll )
         return;
   }
   if( propName == "myrcene_pct" || updateAll ) {
      lineEdit_myrcene->setText(Brewtarget::displayAmount(obsHop->myrcene_pct(), 0));
      if( ! updateAll )
         return;
   }
   if( propName == "substitutes" || updateAll ) {
      textEdit_substitutes->setPlainText(obsHop->substitutes());
      if( ! updateAll )
         return;
   }
   if( propName == "notes" || updateAll ) {
      textEdit_notes->setPlainText(obsHop->notes());
      if( ! updateAll )
         return;
   }
}
