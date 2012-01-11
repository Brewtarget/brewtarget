/*
* RecipeExtrasWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2011.
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

#include <QDate>
#include "RecipeExtrasWidget.h"
#include "unit.h"
#include "brewtarget.h"
#include "recipe.h"

RecipeExtrasWidget::RecipeExtrasWidget(QWidget* parent)
   : QWidget(parent), recipe(0)
{
   setupUi(this);
   connect( lineEdit_age, SIGNAL(editingFinished()), this, SLOT(updateAge()));
   connect( lineEdit_ageTemp, SIGNAL(editingFinished()), this, SLOT(updateAgeTemp()));
   connect( lineEdit_asstBrewer, SIGNAL(editingFinished()), this, SLOT(updateBrewerAsst()) );
   connect( lineEdit_brewer, SIGNAL(editingFinished()), this, SLOT(updateBrewer()) );
   connect( lineEdit_carbVols, SIGNAL(editingFinished()), this, SLOT(updateCarbonation()) );
   connect( lineEdit_primaryAge, SIGNAL(editingFinished()), this, SLOT(updatePrimaryAge()) );
   connect( lineEdit_primaryTemp, SIGNAL(editingFinished()), this, SLOT(updatePrimaryTemp()) );
   connect( lineEdit_secAge, SIGNAL(editingFinished()), this, SLOT(updateSecondaryAge()) );
   connect( lineEdit_secTemp, SIGNAL(editingFinished()), this, SLOT(updateSecondaryTemp()) );
   connect( lineEdit_tertAge, SIGNAL(editingFinished()), this, SLOT(updateTertiaryAge()) );
   connect( lineEdit_tertTemp, SIGNAL(editingFinished()), this, SLOT(updateTertiaryTemp()) );
   connect( spinBox_tasteRating, SIGNAL(editingFinished()), this, SLOT(updateTasteRating()) );
   connect( dateEdit_date, SIGNAL(editingFinished()), this, SLOT(updateDate()) );

   connect( plainTextEdit_notes, SIGNAL(textChanged()), this, SLOT(updateNotes()) );
   connect( plainTextEdit_tasteNotes, SIGNAL(textChanged()), this, SLOT(updateTasteNotes()) );
   /** The above 2 signal/slot pairs used to cause infinite loops and segfault since updating
     the notes caused textChanged() to be emitted. **/
}

void RecipeExtrasWidget::setRecipe(Recipe* rec)
{
   if( recipe )
      disconnect( recipe, 0, this, 0 );
   
   if( rec )
   {
      recipe = rec;
      connect( recipe, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void RecipeExtrasWidget::updateBrewer()
{
   if( recipe == 0 )
      return;

   recipe->setBrewer(lineEdit_brewer->text());
}

void RecipeExtrasWidget::updateBrewerAsst()
{
   if( recipe == 0 )
      return;

   recipe->setAsstBrewer(lineEdit_asstBrewer->text());
}

void RecipeExtrasWidget::updateTasteRating()
{
   if( recipe == 0 )
      return;

   recipe->setTasteRating( (double)(spinBox_tasteRating->value()) );
}

void RecipeExtrasWidget::updatePrimaryAge()
{
   if( recipe == 0 )
      return;

   recipe->setPrimaryAge_days( lineEdit_primaryAge->text().toDouble() );
}

void RecipeExtrasWidget::updatePrimaryTemp()
{
   if( recipe == 0 )
      return;

   recipe->setPrimaryTemp_c( Brewtarget::tempQStringToSI(lineEdit_primaryTemp->text()) );
}

void RecipeExtrasWidget::updateSecondaryAge()
{
   if( recipe == 0 )
      return;

   recipe->setSecondaryAge_days( lineEdit_secAge->text().toDouble() );
}

void RecipeExtrasWidget::updateSecondaryTemp()
{
   if( recipe == 0 )
      return;

   recipe->setSecondaryTemp_c( Brewtarget::tempQStringToSI(lineEdit_secTemp->text()) );
}

void RecipeExtrasWidget::updateTertiaryAge()
{
   if( recipe == 0 )
      return;

   recipe->setTertiaryAge_days( lineEdit_tertAge->text().toDouble() );
}

void RecipeExtrasWidget::updateTertiaryTemp()
{
   if( recipe == 0 )
      return;

   recipe->setTertiaryTemp_c( Brewtarget::tempQStringToSI( lineEdit_tertTemp->text() ) );
}

void RecipeExtrasWidget::updateAge()
{
   if( recipe == 0 )
      return;

   recipe->setAge_days( lineEdit_age->text().toDouble() );
}

void RecipeExtrasWidget::updateAgeTemp()
{
   if( recipe == 0 )
      return;

   recipe->setAgeTemp_c( Brewtarget::tempQStringToSI( lineEdit_ageTemp->text() ) );
}

void RecipeExtrasWidget::updateDate()
{
   if( recipe == 0 )
      return;

   recipe->setDate( dateEdit_date->date() );
}

void RecipeExtrasWidget::updateCarbonation()
{
   if( recipe == 0 )
      return;

   recipe->setCarbonation_vols( lineEdit_carbVols->text().toDouble() );
}

void RecipeExtrasWidget::updateTasteNotes()
{
   if( recipe == 0 )
      return;

   //recObs->disableNotification();
   recipe->setTasteNotes( plainTextEdit_tasteNotes->toPlainText() );
   //recObs->reenableNotification();
}

void RecipeExtrasWidget::updateNotes()
{
   if( recipe == 0 )
      return;

   // Need to disable notification. Otherwise, when recObs->setNotes()
   // is called, recObs calls notify, then showChanges, which causes
   // the notes to be set, emitting textChanged() and sending us into
   // an infinite loop.
   //recObs->disableNotification();
   recipe->setNotes( plainTextEdit_notes->toPlainText() );
   //recObs->reenableNotification();
}

void RecipeExtrasWidget::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() != recipe )
      return;

   showChanges(&prop);
}

void RecipeExtrasWidget::saveAll()
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

void RecipeExtrasWidget::showChanges(QMetaProperty* prop)
{
   bool updateAll = (prop == 0);
   QString propName;
   QVariant val;
   if( prop )
   {
      propName == prop->name();
      val = prop->read(recipe);
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

   // Prevent signals from being sent when we update the notes.
   // This prevents an infinite loop of changing and updating the notes.
   //plainTextEdit_notes->blockSignals(true);
   else if( propName == "notes" || updateAll )
      plainTextEdit_notes->setPlainText( val.toString() );
   //plainTextEdit_notes->blockSignals(false);
   //plainTextEdit_tasteNotes->blockSignals(true);
   else if( propName == "tasteNotes" || updateAll )
      plainTextEdit_tasteNotes->setPlainText( val.toString() );
   //plainTextEdit_tasteNotes->blockSignals(false);
}
