/*
 * HopEditor.cpp is part of Brewtarget, and is Copyright the following
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

#include <QtGui>
#include <QIcon>
#include <QInputDialog>
#include "model/Hop.h"
#include "HopEditor.h"
#include "BtHorizontalTabs.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"

HopEditor::HopEditor( QWidget* parent )
   : QDialog(parent), obsHop(nullptr)
{
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle( new BtHorizontalTabs );

   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newHop() ) );
   connect( pushButton_save,   &QAbstractButton::clicked, this, &HopEditor::save );
   connect( pushButton_cancel, &QAbstractButton::clicked, this, &HopEditor::clearAndClose );
}

void HopEditor::setHop( Hop* h )
{
   if( obsHop )
      disconnect( obsHop, nullptr, this, nullptr );

   obsHop = h;
   if( obsHop )
   {
      connect( obsHop, &NamedEntity::changed, this, &HopEditor::changed );
      showChanges();
   }
}

void HopEditor::save()
{
   Hop* h = obsHop;

   if( h == nullptr )
   {
      setVisible(false);
      return;
   }

   h->setName(lineEdit_name->text(), h->cacheOnly());
   h->setAlpha_pct(lineEdit_alpha->toSI());
   h->setAmount_kg(lineEdit_amount->toSI());
   h->setUse(static_cast<Hop::Use>(comboBox_use->currentIndex()));
   h->setTime_min(lineEdit_time->toSI());
   h->setType(static_cast<Hop::Type>(comboBox_type->currentIndex()));
   h->setForm(static_cast<Hop::Form>(comboBox_form->currentIndex()));
   h->setBeta_pct(lineEdit_beta->toSI());
   h->setHsi_pct(lineEdit_HSI->toSI());
   h->setOrigin(lineEdit_origin->text());
   h->setHumulene_pct(lineEdit_humulene->toSI());
   h->setCaryophyllene_pct(lineEdit_caryophyllene->toSI());
   h->setCohumulone_pct(lineEdit_cohumulone->toSI());
   h->setMyrcene_pct(lineEdit_myrcene->toSI());

   h->setSubstitutes(textEdit_substitutes->toPlainText());
   h->setNotes(textEdit_notes->toPlainText());

   if ( h->cacheOnly() ) {
      h->insertInDatabase();
   }

   // do this late to make sure we've the row in the inventory table
   h->setInventoryAmount(lineEdit_inventory->toSI());
   setVisible(false);
}

void HopEditor::clearAndClose()
{
   setHop(nullptr);
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
   if( obsHop == nullptr )
      return;

   if( prop == nullptr ) {
      updateAll = true;
   }
   else {
      propName = prop->name();
   }

   if( propName == PropertyNames::NamedEntity::name || updateAll )
   {
      lineEdit_name->setText(obsHop->name());
      lineEdit_name->setCursorPosition(0);
      tabWidget_editor->setTabText(0, obsHop->name() );
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Hop::alpha_pct || updateAll ) {
      lineEdit_alpha->setText(obsHop);
      if( ! updateAll )
         return;
   }
   if( propName == "amount_kg" || updateAll ) {
      lineEdit_amount->setText(obsHop);
      if( ! updateAll )
         return;
   }
   if( propName == "inventory" || updateAll ) {
      lineEdit_inventory->setText(obsHop);
      if( ! updateAll )
         return;
   }
   if( propName == "use" || updateAll ) {
      comboBox_use->setCurrentIndex(obsHop->use());
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Hop::time_min || updateAll ) {
      lineEdit_time->setText(obsHop);
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
   if( propName == PropertyNames::Hop::beta_pct || updateAll ) {
      lineEdit_beta->setText(obsHop);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Hop::hsi_pct || updateAll ) {
      lineEdit_HSI->setText(obsHop);
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
   if( propName == PropertyNames::Hop::humulene_pct || updateAll ) {
      lineEdit_humulene->setText(obsHop);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Hop::caryophyllene_pct || updateAll ) {
      lineEdit_caryophyllene->setText(obsHop);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Hop::cohumulone_pct || updateAll ) {
      lineEdit_cohumulone->setText(obsHop);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Hop::myrcene_pct || updateAll ) {
      lineEdit_myrcene->setText(obsHop);
      if( ! updateAll )
         return;
   }
   if( propName == PropertyNames::Hop::substitutes || updateAll ) {
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

void HopEditor::newHop(QString folder)
{
   QString name = QInputDialog::getText(this, tr("Hop name"),
                                          tr("Hop name:"));
   if( name.isEmpty() )
      return;

   Hop* h = new Hop(name,true);

   if ( ! folder.isEmpty() )
      h->setFolder(folder);

   setHop(h);
   show();
}

void HopEditor::newHop()
{
   newHop(QString());
}

