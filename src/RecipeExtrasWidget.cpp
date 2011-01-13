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

#include "RecipeExtrasWidget.h"
#include "unit.h"
#include "brewtarget.h"
#include <QDate>

RecipeExtrasWidget::RecipeExtrasWidget(QWidget* parent) : QWidget(parent)
{
   setupUi(this);

   recObs = 0;

   /*
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
   */
   //connect( plainTextEdit_notes, SIGNAL(textChanged()), this, SLOT(updateNotes()) );
   //connect( plainTextEdit_tasteNotes, SIGNAL(textChanged()), this, SLOT(updateTasteNotes()) );
   /** The above 2 signal/slot pairs cause infinite recursion and segfault since updating
     the notes calls textChanged() to be called. **/
}

void RecipeExtrasWidget::setRecipe(Recipe* rec)
{
   if( rec && rec != recObs )
   {
      recObs = rec;
      setObserved(recObs);
      showChanges();
   }
}

void RecipeExtrasWidget::updateBrewer()
{
   if( recObs == 0 )
      return;

   recObs->setBrewer(lineEdit_brewer->text());
}

void RecipeExtrasWidget::updateBrewerAsst()
{
   if( recObs == 0 )
      return;

   recObs->setAsstBrewer(lineEdit_asstBrewer->text());
}

void RecipeExtrasWidget::updateTasteRating()
{
   if( recObs == 0 )
      return;

   recObs->setTasteRating( (double)(spinBox_tasteRating->value()) );
}

void RecipeExtrasWidget::updatePrimaryAge()
{
   if( recObs == 0 )
      return;

   recObs->setPrimaryAge_days( lineEdit_primaryAge->text().toDouble() );
}

void RecipeExtrasWidget::updatePrimaryTemp()
{
   if( recObs == 0 )
      return;

   recObs->setPrimaryTemp_c( Brewtarget::tempQStringToSI(lineEdit_primaryTemp->text()) );
}

void RecipeExtrasWidget::updateSecondaryAge()
{
   if( recObs == 0 )
      return;

   recObs->setSecondaryAge_days( lineEdit_secAge->text().toDouble() );
}

void RecipeExtrasWidget::updateSecondaryTemp()
{
   if( recObs == 0 )
      return;

   recObs->setSecondaryTemp_c( Brewtarget::tempQStringToSI(lineEdit_secTemp->text()) );
}

void RecipeExtrasWidget::updateTertiaryAge()
{
   if( recObs == 0 )
      return;

   recObs->setTertiaryAge_days( lineEdit_tertAge->text().toDouble() );
}

void RecipeExtrasWidget::updateTertiaryTemp()
{
   if( recObs == 0 )
      return;

   recObs->setTertiaryTemp_c( Brewtarget::tempQStringToSI( lineEdit_tertTemp->text() ) );
}

void RecipeExtrasWidget::updateAge()
{
   if( recObs == 0 )
      return;

   recObs->setAge_days( lineEdit_tertAge->text().toDouble() );
}

void RecipeExtrasWidget::updateAgeTemp()
{
   if( recObs == 0 )
      return;

   recObs->setAgeTemp_c( Brewtarget::tempQStringToSI( lineEdit_tertTemp->text() ) );
}

void RecipeExtrasWidget::updateDate()
{
   if( recObs == 0 )
      return;

   recObs->setDate( dateEdit_date->date().toString("dd/mm/yyyy") );
}

void RecipeExtrasWidget::updateCarbonation()
{
   if( recObs == 0 )
      return;

   recObs->setCarbonation_vols( lineEdit_carbVols->text().toDouble() );
}

void RecipeExtrasWidget::updateTasteNotes()
{
   if( recObs == 0 )
      return;

   recObs->setTasteNotes( plainTextEdit_tasteNotes->toPlainText() );
}

void RecipeExtrasWidget::updateNotes()
{
   if( recObs == 0 )
      return;

   recObs->setNotes( plainTextEdit_notes->toPlainText() );
}

void RecipeExtrasWidget::notify(Observable* notifier, QVariant /*info*/)
{
   if( notifier != recObs )
      return;

   showChanges();
}

void RecipeExtrasWidget::saveAll()
{
   recObs->disableNotification();

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

   recObs->reenableNotification();
   recObs->forceNotify();

   hide();
}

void RecipeExtrasWidget::showChanges()
{
   lineEdit_age->setText( Brewtarget::displayAmount(recObs->getAge_days()) );
   lineEdit_ageTemp->setText( Brewtarget::displayAmount(recObs->getAgeTemp_c(), Units::celsius) );
   lineEdit_asstBrewer->setText( recObs->getAsstBrewer() );
   lineEdit_brewer->setText( recObs->getBrewer() );
   lineEdit_carbVols->setText( Brewtarget::displayAmount(recObs->getCarbonation_vols()) );
   lineEdit_primaryAge->setText( Brewtarget::displayAmount(recObs->getPrimaryAge_days()) );
   lineEdit_primaryTemp->setText( Brewtarget::displayAmount(recObs->getPrimaryTemp_c(), Units::celsius) );
   lineEdit_secAge->setText( Brewtarget::displayAmount(recObs->getSecondaryAge_days()) );
   lineEdit_secTemp->setText( Brewtarget::displayAmount(recObs->getSecondaryTemp_c(), Units::celsius) );
   lineEdit_tertAge->setText( Brewtarget::displayAmount(recObs->getTertiaryAge_days()) );
   lineEdit_tertTemp->setText( Brewtarget::displayAmount(recObs->getTertiaryTemp_c(), Units::celsius) );
   spinBox_tasteRating->setValue( (int)(recObs->getTasteRating()) );

   dateEdit_date->setDate( QDate::fromString(recObs->getDate(), "dd/MM/yyyy") );

   plainTextEdit_notes->setPlainText( recObs->getNotes() );
   plainTextEdit_tasteNotes->setPlainText( recObs->getTasteNotes() );
}
