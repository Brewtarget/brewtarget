/*
 * OptionDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include "OptionDialog.h"
#include "brewtarget.h"

#include <QButtonGroup>

OptionDialog::OptionDialog(QWidget* parent)
{
   setupUi(this);

   if( parent != 0 )
   {
      setWindowIcon(parent->windowIcon());
   }

   QButtonGroup *colorGroup, *ibuGroup;
   colorGroup = new QButtonGroup(this);
   ibuGroup = new QButtonGroup(this);

   // Want you to only be able to select exactly one in each group.
   colorGroup->setExclusive(true);
   ibuGroup->setExclusive(true);

   // Set up the buttons in the colorGroup
   colorGroup->addButton(checkBox_mosher);
   colorGroup->addButton(checkBox_daniel);
   colorGroup->addButton(checkBox_morey);

   // Same for ibuGroup.
   ibuGroup->addButton(checkBox_tinseth);
   ibuGroup->addButton(checkBox_rager);

   connect( colorGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeColorFormula(QAbstractButton*) ) );
   connect( ibuGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeIbuFormula(QAbstractButton*) ) );
   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( saveAndClose() ) );
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( cancel() ) );
}

void OptionDialog::changeColorFormula(QAbstractButton* button)
{
   Brewtarget::ColorType formula;
   if( button == checkBox_mosher )
      formula = Brewtarget::MOSHER;
   else if( button == checkBox_daniel )
      formula = Brewtarget::DANIEL;
   else if( button == checkBox_morey )
      formula = Brewtarget::MOREY;
   else
      formula = Brewtarget::MOREY; // Should never get here, but you never know.

   Brewtarget::colorFormula = formula;
   Brewtarget::mainWindow->forceRecipeUpdate(); // Tell the recipe to update so we can see the changes.
}

void OptionDialog::changeIbuFormula(QAbstractButton* button)
{
   Brewtarget::IbuType formula;
   if( button == checkBox_tinseth )
      formula = Brewtarget::TINSETH;
   else if( button == checkBox_rager )
      formula = Brewtarget::RAGER;
   else
      formula = Brewtarget::TINSETH; // Should never get here, but you never know.

   Brewtarget::ibuFormula = formula;
   Brewtarget::mainWindow->forceRecipeUpdate(); // Tell the recipe to update so we can see the changes.
}

void OptionDialog::show()
{
   showChanges();
   setVisible(true);
}

void OptionDialog::saveAndClose()
{
   Brewtarget::englishUnits = (checkBox_USUnits->checkState() == Qt::Checked);

   if( Brewtarget::mainWindow != 0 )
      Brewtarget::mainWindow->showChanges(); // Make sure the main window updates.

   setVisible(false);
}

void OptionDialog::cancel()
{
   setVisible(false);
}

void OptionDialog::showChanges()
{
   checkBox_USUnits->setCheckState( Brewtarget::englishUnits ? Qt::Checked : Qt::Unchecked );

   // Check the right color formula box.
   switch( Brewtarget::colorFormula )
   {
      case Brewtarget::MOREY:
         checkBox_morey->setCheckState(Qt::Checked);
         break;
      case Brewtarget::DANIEL:
         checkBox_daniel->setCheckState(Qt::Checked);
         break;
      case Brewtarget::MOSHER:
         checkBox_mosher->setCheckState(Qt::Checked);
         break;
   }

   // Check the right ibu formula box.
   switch( Brewtarget::ibuFormula )
   {
      case Brewtarget::TINSETH:
         checkBox_tinseth->setCheckState(Qt::Checked);
         break;
      case Brewtarget::RAGER:
         checkBox_rager->setCheckState(Qt::Checked);
         break;
   }
}
