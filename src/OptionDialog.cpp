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

   colorGroup = new QButtonGroup(this);
   ibuGroup = new QButtonGroup(this);
   weightGroup = new QButtonGroup(this);
   volumeGroup = new QButtonGroup(this);
   tempGroup = new QButtonGroup(this);

   // Want you to only be able to select exactly one in each group.
   colorGroup->setExclusive(true);
   ibuGroup->setExclusive(true);
   weightGroup->setExclusive(true);
   volumeGroup->setExclusive(true);
   tempGroup->setExclusive(true);

   // Set up the buttons in the colorGroup
   colorGroup->addButton(checkBox_mosher);
   colorGroup->addButton(checkBox_daniel);
   colorGroup->addButton(checkBox_morey);

   // Same for ibuGroup.
   ibuGroup->addButton(checkBox_tinseth);
   ibuGroup->addButton(checkBox_rager);

   // Weight
   weightGroup->addButton(weight_si);
   weightGroup->addButton(weight_us);
   weightGroup->addButton(weight_imperial);

   // Volume
   volumeGroup->addButton(volume_si);
   volumeGroup->addButton(volume_us);
   volumeGroup->addButton(volume_imperial);

   // Temperature

   tempGroup->addButton(celsius);
   tempGroup->addButton(fahrenheit);

   //connect( colorGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeColorFormula(QAbstractButton*) ) );
   //connect( ibuGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeIbuFormula(QAbstractButton*) ) );
   //connect( weightGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeWeightUnitSystem(QAbstractButton*) ) );
   //connect( volumeGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeVolumeUnitSystem(QAbstractButton*) ) );
   //connect( tempGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeTemperatureScale(QAbstractButton*) ) );

   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( saveAndClose() ) );
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( cancel() ) );
}

void OptionDialog::show()
{
   showChanges();
   setVisible(true);
}

void OptionDialog::saveAndClose()
{
   /*
   Brewtarget::weightUnitSystem = weightUnitSystem;
   Brewtarget::volumeUnitSystem = volumeUnitSystem;
   Brewtarget::tempScale = temperatureScale;
   */
   QAbstractButton* button;
   UnitSystem weightUnitSystem;
   UnitSystem volumeUnitSystem;
   TempScale temperatureScale;
   Brewtarget::ColorType cformula;
   Brewtarget::IbuType iformula;
   
   button = colorGroup->checkedButton();
   if( button == checkBox_mosher )
      cformula = Brewtarget::MOSHER;
   else if( button == checkBox_daniel )
      cformula = Brewtarget::DANIEL;
   else if( button == checkBox_morey )
      cformula = Brewtarget::MOREY;
   else
      cformula = Brewtarget::MOREY; // Should never get here, but you never know.
   
   button = ibuGroup->checkedButton();
   if( button == checkBox_tinseth )
      iformula = Brewtarget::TINSETH;
   else if( button == checkBox_rager )
      iformula = Brewtarget::RAGER;
   else
      iformula = Brewtarget::TINSETH; // Should never get here, but you never know.
   
   button = weightGroup->checkedButton();
   if( button == weight_imperial )
      weightUnitSystem = Imperial;
   else if( button == weight_us)
      weightUnitSystem = USCustomary;
   else
      weightUnitSystem = SI;
   
   button = volumeGroup->checkedButton();
   if( button == volume_imperial )
      volumeUnitSystem = Imperial;
   else if( button == volume_us )
      volumeUnitSystem = USCustomary;
   else
      volumeUnitSystem = SI;
   
   button = tempGroup->checkedButton();
   if( button == fahrenheit )
      temperatureScale = Fahrenheit;
   else
      temperatureScale = Celsius;
   
   Brewtarget::ibuFormula = iformula;
   Brewtarget::colorFormula = cformula;
   Brewtarget::weightUnitSystem = weightUnitSystem;
   Brewtarget::volumeUnitSystem = volumeUnitSystem;
   Brewtarget::tempScale = temperatureScale;
   
   if( Brewtarget::mainWindow != 0 ) {
      Brewtarget::mainWindow->showChanges(); // Make sure the main window updates.
   }

   setVisible(false);
}

void OptionDialog::cancel()
{
   setVisible(false);
}

void OptionDialog::showChanges()
{
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

   // Check the right weight unit system box.
   switch( Brewtarget::weightUnitSystem )
   {
      case Imperial:
         weight_imperial->setChecked(TRUE);
         break;
      case USCustomary:
         weight_us->setChecked(TRUE);
         break;
      case SI:
      default:
         weight_si->setChecked(TRUE);
   }

   // Check the right volume unit system box.
   switch( Brewtarget::volumeUnitSystem )
   {
      case Imperial:
         volume_imperial->setChecked(TRUE);
         break;
      case USCustomary:
         volume_us->setChecked(TRUE);
         break;
      case SI:
      default:
         volume_si->setChecked(TRUE);
   }

   switch( Brewtarget::tempScale )
   {
      case Fahrenheit:
         fahrenheit->setChecked(TRUE);
         break;
      case Celsius:
      default:
         celsius->setChecked(TRUE);
         break;
  } 
}
