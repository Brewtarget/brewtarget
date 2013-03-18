/*
* RecipeExtrasDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2010-2013.
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

#include "RecipeExtrasDialog.h"
#include <QDate>
#include "unit.h"
#include "brewtarget.h"
#include "recipe.h"

RecipeExtrasDialog::RecipeExtrasDialog(QWidget* parent)
   : QDialog(parent), recObs(0)
{
   setupUi(this);

   connect( pushButton_save, SIGNAL(clicked()), this, SLOT(saveAndQuit()) );
   connect( pushButton_cancel, SIGNAL(clicked()), this, SLOT(hide()) );
}

void RecipeExtrasDialog::setRecipe(Recipe* rec)
{
   if( recObs )
      disconnect( recObs, 0, this, 0 );
   
   recObs = rec;
   if( recObs )
   {
      connect( recObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void RecipeExtrasDialog::updateBrewer()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_brewer->isModified() )
      recObs->setBrewer(lineEdit_brewer->text());
}

void RecipeExtrasDialog::updateBrewerAsst()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_asstBrewer->isModified() )
      recObs->setAsstBrewer(lineEdit_asstBrewer->text());
}

// TODO: Need to fix this so we only change it when required
void RecipeExtrasDialog::updateTasteRating()
{
   if( recObs == 0 )
      return;

   recObs->setTasteRating( (double)(spinBox_tasteRating->value()) );
}

void RecipeExtrasDialog::updatePrimaryAge()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_primaryAge->isModified() )
      recObs->setPrimaryAge_days( lineEdit_primaryAge->text().toDouble() );
}

void RecipeExtrasDialog::updatePrimaryTemp()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_primaryTemp->isModified() )
      recObs->setPrimaryTemp_c( Brewtarget::tempQStringToSI(lineEdit_primaryTemp->text()) );
}

void RecipeExtrasDialog::updateSecondaryAge()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_secAge->isModified() )
      recObs->setSecondaryAge_days( lineEdit_secAge->text().toDouble() );
}

void RecipeExtrasDialog::updateSecondaryTemp()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_secTemp->isModified() )
      recObs->setSecondaryTemp_c( Brewtarget::tempQStringToSI(lineEdit_secTemp->text()) );
}

void RecipeExtrasDialog::updateTertiaryAge()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_tertAge->isModified() )
      recObs->setTertiaryAge_days( lineEdit_tertAge->text().toDouble() );
}

void RecipeExtrasDialog::updateTertiaryTemp()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_tertTemp->isModified() )
      recObs->setTertiaryTemp_c( Brewtarget::tempQStringToSI( lineEdit_tertTemp->text() ) );
}

void RecipeExtrasDialog::updateAge()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_tertAge->isModified() )
      recObs->setAge_days( lineEdit_tertAge->text().toDouble() );
}

void RecipeExtrasDialog::updateAgeTemp()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_tertTemp->isModified() )
      recObs->setAgeTemp_c( Brewtarget::tempQStringToSI( lineEdit_tertTemp->text() ) );
}

// TODO: Need to fix this to update only when a real change happens
void RecipeExtrasDialog::updateDate()
{
   if( recObs == 0 )
      return;

   recObs->setDate( dateEdit_date->date() );
}

void RecipeExtrasDialog::updateCarbonation()
{
   if( recObs == 0 )
      return;

   if ( lineEdit_carbVols->isModified() )
      recObs->setCarbonation_vols( lineEdit_carbVols->text().toDouble() );
}

void RecipeExtrasDialog::updateTasteNotes()
{
   if( recObs == 0 )
      return;

   if ( btTextEdit_tasteNotes->isModified() )
      recObs->setTasteNotes( btTextEdit_tasteNotes->toPlainText() );
}

void RecipeExtrasDialog::updateNotes()
{
   if( recObs == 0 )
      return;

   if ( btTextEdit_notes->isModified() )
      recObs->setNotes( btTextEdit_notes->toPlainText() );
}

void RecipeExtrasDialog::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() != recObs )
      return;

   showChanges(&prop);
}

void RecipeExtrasDialog::saveAndQuit()
{
   //recObs->disableNotification();

   updateBrewer();
   updateBrewerAsst();
   updateTasteRating();
   updatePrimaryAge();
   updatePrimaryTemp();
   updateSecondaryAge();
   updateSecondaryTemp();
   updateTertiaryAge();
   updateTertiaryTemp();
   updateAge();
   updateAgeTemp();
   updateDate();
   updateCarbonation();
   updateTasteNotes();
   updateNotes();

   //recObs->reenableNotification();
   //recObs->forceNotify();

   hide();
}

void RecipeExtrasDialog::showChanges(QMetaProperty* prop)
{
   bool updateAll = (prop == 0);
   QString propName;
   QVariant val;
   if( prop )
   {
      propName = prop->name();
      val = prop->read(recObs);
   }
   
   if( propName == "age_days" || updateAll )
      lineEdit_age->setText( Brewtarget::displayAmount(val.toDouble()) );
   else if( propName == "ageTemp_c" || updateAll )
      lineEdit_ageTemp->setText( Brewtarget::displayAmount(val.toDouble(), Units::celsius) );
   else if( propName == "asstBrewer" || updateAll )
      lineEdit_asstBrewer->setText( val.toString() );
   else if( propName == "brewer" || updateAll )
      lineEdit_brewer->setText( val.toString() );
   else if( propName == "carbonation_vols" || updateAll )
      lineEdit_carbVols->setText( Brewtarget::displayAmount(val.toDouble()) );
   else if( propName == "primaryAge_days" || updateAll )
      lineEdit_primaryAge->setText( Brewtarget::displayAmount(val.toDouble()) );
   else if( propName == "primaryTemp_c" || updateAll )
      lineEdit_primaryTemp->setText( Brewtarget::displayAmount(val.toDouble(), Units::celsius) );
   else if( propName == "secondaryAge_days" || updateAll )
      lineEdit_secAge->setText( Brewtarget::displayAmount(val.toDouble()) );
   else if( propName == "secondaryTemp_c" || updateAll )
      lineEdit_secTemp->setText( Brewtarget::displayAmount(val.toDouble(), Units::celsius) );
   else if( propName == "tertiaryAge_days" || updateAll )
      lineEdit_tertAge->setText( Brewtarget::displayAmount(val.toDouble()) );
   else if( propName == "tertiaryTemp_c" || updateAll )
      lineEdit_tertTemp->setText( Brewtarget::displayAmount(val.toDouble(), Units::celsius) );
   else if( propName == "tasteRating" || updateAll )
      spinBox_tasteRating->setValue( (int)(val.toDouble()) );
   else if( propName == "date" || updateAll )
      dateEdit_date->setDate( val.toDate() );
   else if( propName == "notes" || updateAll )
      btTextEdit_notes->setPlainText( val.toString() );
   else if( propName == "tasteNotes" || updateAll )
      btTextEdit_tasteNotes->setPlainText( val.toString() );
}
