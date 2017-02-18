/*
 * RecipeExtrasWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Peter Buelow <goballstate@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include <QDate>
#include <QWidget>
#include <QDebug>
#include "RecipeExtrasWidget.h"
#include "unit.h"
#include "brewtarget.h"
#include "recipe.h"
#include "BtLabel.h"

RecipeExtrasWidget::RecipeExtrasWidget(QWidget* parent)
   : QWidget(parent), recipe(0)
{
   setupUi(this);

   ratingChanged = false;

   connect( lineEdit_age,        &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateAge);
   connect( lineEdit_ageTemp,    &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateAgeTemp);
   connect( lineEdit_asstBrewer, &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateBrewerAsst );
   connect( lineEdit_brewer,     &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateBrewer );
   connect( lineEdit_carbVols,   &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateCarbonation );
   connect( lineEdit_primaryAge, &BtLineEdit::textModified, this, &RecipeExtrasWidget::updatePrimaryAge );
   connect( lineEdit_primaryTemp,&BtLineEdit::textModified, this, &RecipeExtrasWidget::updatePrimaryTemp );
   connect( lineEdit_secAge,     &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateSecondaryAge );
   connect( lineEdit_secTemp,    &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateSecondaryTemp );
   connect( lineEdit_tertAge,    &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateTertiaryAge );
   connect( lineEdit_tertTemp,   &BtLineEdit::textModified, this, &RecipeExtrasWidget::updateTertiaryTemp );

   connect( spinBox_tasteRating, SIGNAL(valueChanged(int)), this, SLOT(changeRatings(int)) );
   connect( spinBox_tasteRating, &QAbstractSpinBox::editingFinished, this, &RecipeExtrasWidget::updateTasteRating );

   connect( dateEdit_date, &QDateTimeEdit::dateChanged, this, &RecipeExtrasWidget::updateDate );

   connect(btTextEdit_notes, &BtTextEdit::textModified, this, &RecipeExtrasWidget::updateNotes);
   connect(btTextEdit_tasteNotes, &BtTextEdit::textModified, this, &RecipeExtrasWidget::updateTasteNotes);

}

void RecipeExtrasWidget::setRecipe(Recipe* rec)
{
   if( recipe )
      disconnect( recipe, 0, this, 0 );
   
   if( rec )
   {
      recipe = rec;
      connect( recipe, &BeerXMLElement::changed, this, &RecipeExtrasWidget::changed );
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

   if ( lineEdit_asstBrewer->isModified() )
      recipe->setAsstBrewer(lineEdit_asstBrewer->text());
}

void RecipeExtrasWidget::changeRatings(int rating) { ratingChanged = true; }

void RecipeExtrasWidget::updateTasteRating()
{
   if( recipe == 0 )
      return;

   if ( ratingChanged )
   {
      recipe->setTasteRating( (double)(spinBox_tasteRating->value()) );
      ratingChanged = false;
   }
}

void RecipeExtrasWidget::updatePrimaryAge()
{
   if( recipe == 0 )
      return;

   recipe->setPrimaryAge_days( lineEdit_primaryAge->toSI() );
}

void RecipeExtrasWidget::updatePrimaryTemp()
{
   if( recipe == 0 )
      return;

   recipe->setPrimaryTemp_c( lineEdit_primaryTemp->toSI() );
}

void RecipeExtrasWidget::updateSecondaryAge()
{
   if( recipe == 0 )
      return;

   recipe->setSecondaryAge_days( lineEdit_secAge->toSI() );
}

void RecipeExtrasWidget::updateSecondaryTemp()
{
   if( recipe == 0 )
      return;

   recipe->setSecondaryTemp_c( lineEdit_secTemp->toSI() );
}

void RecipeExtrasWidget::updateTertiaryAge()
{
   if( recipe == 0 )
      return;

   recipe->setTertiaryAge_days( lineEdit_tertAge->toSI() );
}

void RecipeExtrasWidget::updateTertiaryTemp()
{
   if( recipe == 0 )
      return;

   recipe->setTertiaryTemp_c( lineEdit_tertTemp->toSI() );
}

void RecipeExtrasWidget::updateAge()
{
   if( recipe == 0 )
      return;

   recipe->setAge_days( lineEdit_age->toSI() );
}

void RecipeExtrasWidget::updateAgeTemp()
{
   if( recipe == 0 )
      return;

   recipe->setAgeTemp_c( lineEdit_ageTemp->toSI() );
}

void RecipeExtrasWidget::updateDate(const QDate& date)
{
   if( recipe == 0 )
      return;

   if ( date.isNull()  ) 
      recipe->setDate( dateEdit_date->date() );
   else
      recipe->setDate( date );
}

void RecipeExtrasWidget::updateCarbonation()
{
   if( recipe == 0 )
      return;

   recipe->setCarbonation_vols( lineEdit_carbVols->toSI() );
}

void RecipeExtrasWidget::updateTasteNotes()
{
   if( recipe == 0 )
      return;

   recipe->setTasteNotes( btTextEdit_tasteNotes->toPlainText() );
}

void RecipeExtrasWidget::updateNotes()
{
   if( recipe == 0 )
      return;

   recipe->setNotes( btTextEdit_notes->toPlainText() );
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
      propName = prop->name();
      val = prop->read(recipe);
   }
   
   if( ! recipe )
      return;

   // I think we may be going circular here? LineEdit says "change is made",
   // which signals the widget which changes the db, which signals "change is
   // made" which signals the widget, which changes the LineEdit, which says
   // "change is made" ... rinse, lather, repeat
   // Unlike other editors, this one needs to read from recipe when it gets an
   // updateAll
   if ( updateAll )
   {

      lineEdit_age->setText(recipe);
      lineEdit_ageTemp->setText(recipe);
      lineEdit_asstBrewer->setText(recipe);
      lineEdit_brewer->setText(recipe);
      lineEdit_carbVols->setText(recipe);
      lineEdit_primaryAge->setText(recipe);
      lineEdit_primaryTemp->setText(recipe);

      lineEdit_secAge->setText(recipe);
      lineEdit_secTemp->setText(recipe);
      lineEdit_tertAge->setText(recipe);
      lineEdit_tertTemp->setText(recipe);
      spinBox_tasteRating->setValue((int)(recipe->tasteRating()));
      dateEdit_date->setDate(recipe->date());
      btTextEdit_notes->setPlainText(recipe->notes());
      btTextEdit_tasteNotes->setPlainText(recipe->tasteNotes());
   }
   else if( propName == "age_days" )
      lineEdit_age->setText(recipe);
   else if( propName == "ageTemp_c" )
      lineEdit_ageTemp->setText(recipe);
   else if( propName == "asstBrewer" )
      lineEdit_asstBrewer->setText(recipe);
   else if( propName == "brewer" )
      lineEdit_brewer->setText(recipe);
   else if( propName == "carbonation_vols" )
      lineEdit_carbVols->setText(recipe);
   else if( propName == "primaryAge_days" )
      lineEdit_primaryAge->setText(recipe);
   else if( propName == "primaryTemp_c" )
      lineEdit_primaryTemp->setText(recipe);
   else if( propName == "secondaryAge_days" )
      lineEdit_secAge->setText(recipe);
   else if( propName == "secondaryTemp_c" )
      lineEdit_secTemp->setText(recipe);
   else if( propName == "tertiaryAge_days" )
      lineEdit_tertAge->setText(recipe);
   else if( propName == "tertiaryTemp_c" )
      lineEdit_tertTemp->setText(recipe);
   else if( propName == "tasteRating" )
      spinBox_tasteRating->setValue( val.toInt() );
   else if( propName == "date" )
      dateEdit_date->setDate( val.toDate() );
   else if( propName == "notes" )
      btTextEdit_notes->setPlainText( val.toString() );
   else if( propName == "tasteNotes" )
      btTextEdit_tasteNotes->setPlainText( val.toString() );

}
