/*
 * OptionDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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
#include "UnitSystem.h"
#include "UnitSystems.h"
#include "USWeightUnitSystem.h"
#include "SIWeightUnitSystem.h"
#include "ImperialVolumeUnitSystem.h"
#include "USVolumeUnitSystem.h"
#include "SIVolumeUnitSystem.h"
#include "FahrenheitTempUnitSystem.h"
#include "CelsiusTempUnitSystem.h"
#include <QButtonGroup>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>

OptionDialog::OptionDialog(QWidget* parent)
{
   setupUi(this);

   if( parent != 0 )
   {
      setWindowIcon(parent->windowIcon());
   }

   languageGroup = new QButtonGroup(this);
   colorGroup = new QButtonGroup(this);
   ibuGroup = new QButtonGroup(this);
   weightGroup = new QButtonGroup(this);
   volumeGroup = new QButtonGroup(this);
   tempGroup = new QButtonGroup(this);
   gravGroup = new QButtonGroup(this);
   colorUnitGroup = new QButtonGroup(this);

   // Set up language map.
   languageToButtonMap["cs"] = pushButton_cs;
   languageToButtonMap["de"] = pushButton_de;
   languageToButtonMap["en"] = pushButton_en;
   languageToButtonMap["es"] = pushButton_es;
   languageToButtonMap["fr"] = pushButton_fr;
   languageToButtonMap["nl"] = pushButton_nl;
   languageToButtonMap["pl"] = pushButton_pl;
   languageToButtonMap["pt"] = pushButton_pt;

   // Want you to only be able to select exactly one in each group.
   languageGroup->setExclusive(true);
   colorGroup->setExclusive(true);
   ibuGroup->setExclusive(true);
   weightGroup->setExclusive(true);
   volumeGroup->setExclusive(true);
   tempGroup->setExclusive(true);
   gravGroup->setExclusive(true);
   colorUnitGroup->setExclusive(true);

   // Disable certain buttons due to lack of actual language support.
   pushButton_de->setEnabled(false);

   // Set up the buttons in languageGroup;
   languageGroup->addButton(pushButton_cs);
   languageGroup->addButton(pushButton_de);
   languageGroup->addButton(pushButton_en);
   languageGroup->addButton(pushButton_es);
   languageGroup->addButton(pushButton_fr);
   languageGroup->addButton(pushButton_nl);
   languageGroup->addButton(pushButton_pl);
   languageGroup->addButton(pushButton_pt);

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

   // Gravity
   gravGroup->addButton(radioButton_sg);
   gravGroup->addButton(radioButton_plato);

   // Color Unit
   colorUnitGroup->addButton(radioButton_srm);
   colorUnitGroup->addButton(radioButton_ebc);

   //connect( colorGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeColorFormula(QAbstractButton*) ) );
   //connect( ibuGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeIbuFormula(QAbstractButton*) ) );
   //connect( weightGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeWeightUnitSystem(QAbstractButton*) ) );
   //connect( volumeGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeVolumeUnitSystem(QAbstractButton*) ) );
   //connect( tempGroup, SIGNAL( buttonClicked(QAbstractButton*) ), this, SLOT( changeTemperatureScale(QAbstractButton*) ) );

   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( saveAndClose() ) );
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( cancel() ) );
   connect( pushButton_dbDirBrowse, SIGNAL( clicked() ), this, SLOT( setDataDir() ) );
   connect( pushButton_dbDirDefault, SIGNAL( clicked() ), this, SLOT( defaultDataDir() ) );
}

void OptionDialog::show()
{
   showChanges();
   setVisible(true);
}

void OptionDialog::setDataDir()
{
   QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), Brewtarget::getUserDataDir(), QFileDialog::ShowDirsOnly);
   if( ! dir.isEmpty() )
      lineEdit_dbDir->setText( dir );
}

void OptionDialog::defaultDataDir()
{
   lineEdit_dbDir->setText( Brewtarget::getConfigDir() );
}

void OptionDialog::saveAndClose()
{
   /*
   Brewtarget::weightUnitSystem = weightUnitSystem;
   Brewtarget::volumeUnitSystem = volumeUnitSystem;
   Brewtarget::tempScale = temperatureScale;
   */
   QAbstractButton* button;
   iUnitSystem weightUnitSystem;
   iUnitSystem volumeUnitSystem;
   TempScale temperatureScale;
   Brewtarget::ColorType cformula;
   Brewtarget::IbuType iformula;
   Brewtarget::ColorUnitType colorUnit;
   QString newUserDataDir;

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
   
   // Get gravity setting.
   button = gravGroup->checkedButton();
   if( button == radioButton_sg )
      Brewtarget::usePlato = false;
   else
      Brewtarget::usePlato = true;

   button = weightGroup->checkedButton();
   if( button == weight_imperial )
   {
      weightUnitSystem = Imperial;
      Brewtarget::weightSystem = UnitSystems::usWeightUnitSystem();
   }
   else if( button == weight_us)
   {
      weightUnitSystem = USCustomary;
      Brewtarget::weightSystem = UnitSystems::usWeightUnitSystem();
   }
   else
   {
      weightUnitSystem = SI;
      Brewtarget::weightSystem = UnitSystems::siWeightUnitSystem();
   }
   
   button = volumeGroup->checkedButton();
   if( button == volume_imperial )
   {
      volumeUnitSystem = Imperial;
      Brewtarget::volumeSystem = UnitSystems::imperialVolumeUnitSystem();
   }
   else if( button == volume_us )
   {
      volumeUnitSystem = USCustomary;
      Brewtarget::volumeSystem = UnitSystems::usVolumeUnitSystem();
   }
   else
   {
      volumeUnitSystem = SI;
      Brewtarget::volumeSystem = UnitSystems::siVolumeUnitSystem();
   }
   
   button = tempGroup->checkedButton();
   if( button == fahrenheit )
   {
      temperatureScale = Fahrenheit;
      Brewtarget::tempSystem = UnitSystems::fahrenheitTempUnitSystem();
   }
   else
   {
      temperatureScale = Celsius;
      Brewtarget::tempSystem = UnitSystems::celsiusTempUnitSystem();
   }
   
   button = colorUnitGroup->checkedButton();
   if( button == radioButton_ebc )
   {
      colorUnit = Brewtarget::EBC;
   }
   else
      colorUnit = Brewtarget::SRM;

   Brewtarget::ibuFormula = iformula;
   Brewtarget::colorFormula = cformula;
   Brewtarget::weightUnitSystem = weightUnitSystem;
   Brewtarget::volumeUnitSystem = volumeUnitSystem;
   Brewtarget::tempScale = temperatureScale;
   Brewtarget::colorUnit = colorUnit;

   // Set the right language.
   Brewtarget::setLanguage( languageToButtonMap.key(reinterpret_cast<QPushButton*>(languageGroup->checkedButton())) );
   
   // Check the new userDataDir.
   newUserDataDir = lineEdit_dbDir->text();
   if( newUserDataDir != Brewtarget::getUserDataDir() )
   {
      // If there are no data files present...
      if( ! QFileInfo(newUserDataDir + "database.xml").exists() )
      {
         // ...tell user we will copy old data files to new location.
         QMessageBox::information(this,
                                  tr("Copy Data"),
                                  tr("There does not seem to be any data files in this directory, so we will copy your old data here.")
                                 );
         Brewtarget::copyDataFiles(newUserDataDir);
      }

      Brewtarget::userDataDir = newUserDataDir;
   }

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
   // Check the right language button
   QPushButton* lButton = languageToButtonMap[Brewtarget::getCurrentLanguage()];
   if( lButton != 0 )
      lButton->setChecked(true);

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

   // Check gravity.
   if( Brewtarget::usePlato )
      radioButton_plato->setChecked(TRUE);
   else
      radioButton_sg->setChecked(TRUE);

   // Temp.
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

   // Color Formula
   switch( Brewtarget::colorUnit )
   {
   case Brewtarget::EBC:
      radioButton_ebc->setChecked(true);
      break;
   case Brewtarget::SRM:
   default:
      radioButton_srm->setChecked(true);
   }

   // Data directory
   lineEdit_dbDir->setText(Brewtarget::getUserDataDir());
}
