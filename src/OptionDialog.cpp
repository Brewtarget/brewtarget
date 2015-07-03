/*
 * OptionDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
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
#include "EbcColorUnitSystem.h"
#include "SrmColorUnitSystem.h"
#include "PlatoDensityUnitSystem.h"
#include "SgDensityUnitSystem.h"
#include "CelsiusTempUnitSystem.h"
#include <QButtonGroup>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include "MainWindow.h"

OptionDialog::OptionDialog(QWidget* parent)
{
   int i;
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
   gravGroup = new QButtonGroup(this);
   colorUnitGroup = new QButtonGroup(this);
   dateFormatGroup = new QButtonGroup(this);

   ndxToLangCode <<
      "ca" <<
      "cs" <<
      "de" <<
      "en" <<
      "el" <<
      "es" <<
      "fr" <<
      "it" <<
      "nl" <<
      "pl" <<
      "pt" <<
      "ru" <<
      "zh";

   // Do this just to have model indices to set icons.
   comboBox_lang->addItems(ndxToLangCode);
   // MUST correspond to ndxToLangCode.
   langIcons <<
      QIcon(":images/flagCatalonia.svg") <<
      QIcon(":images/flagCzech.svg") <<
      QIcon(":images/flagGermany.svg") <<
      QIcon(":images/flagUK.svg") <<
      QIcon(":images/flagGreece.svg") <<
      QIcon(":images/flagSpain.svg") <<
      QIcon(":images/flagFrance.svg") <<
      QIcon(":images/flagItaly.svg") <<
      QIcon(":images/flagNetherlands.svg") <<
      QIcon(":images/flagPoland.svg") <<
      QIcon(":images/flagBrazil.svg") <<
      QIcon(":images/flagRussia.svg") <<
      QIcon(":images/flagChina.svg");
   // Set icons.
   for( i = 0; i < langIcons.size(); ++i )
      comboBox_lang->setItemIcon(i, langIcons[i]);

   // Call this here to set up translatable strings.
   retranslate();

   // Want you to only be able to select exactly one in each group.
   colorGroup->setExclusive(true);
   ibuGroup->setExclusive(true);
   weightGroup->setExclusive(true);
   volumeGroup->setExclusive(true);
   tempGroup->setExclusive(true);
   gravGroup->setExclusive(true);
   colorUnitGroup->setExclusive(true);
   dateFormatGroup->setExclusive(true);

   // Set up the buttons in the colorGroup
   colorGroup->addButton(radioButton_mosher);
   colorGroup->addButton(radioButton_daniel);
   colorGroup->addButton(radioButton_morey);

   // Same for ibuGroup.
   ibuGroup->addButton(radioButton_tinseth);
   ibuGroup->addButton(radioButton_rager);
   ibuGroup->addButton(radioButton_noonan);

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

   // Density
   gravGroup->addButton(radioButton_sg);
   gravGroup->addButton(radioButton_plato);

   // Color Unit
   colorUnitGroup->addButton(radioButton_srm);
   colorUnitGroup->addButton(radioButton_ebc);

   // date formats
   dateFormatGroup->addButton(radioButton_usDate);
   dateFormatGroup->addButton(radioButton_euDate);
   dateFormatGroup->addButton(radioButton_isoDate);

   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( saveAndClose() ) );
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( cancel() ) );
   connect( pushButton_dbDirBrowse, SIGNAL( clicked() ), this, SLOT( setDataDir() ) );
   connect( pushButton_dbDirDefault, SIGNAL( clicked() ), this, SLOT( defaultDataDir() ) );
}

void OptionDialog::retranslate()
{
   // Let the Ui take care of its business
   retranslateUi(this);
   
   // Retranslate the language combobox.
   // NOTE: the indices MUST correspond to ndxToLangCode.
   QStringList langStrings;
   langStrings <<
      tr("Catalan") <<
      tr("Czech") <<
      tr("German") <<
      tr("English") <<
      tr("Greek") <<
      tr("Spanish") <<
      tr("French") <<
      tr("Italian") <<
      tr("Dutch") <<
      tr("Polish") <<
      tr("Portuguese") <<
      tr("Russian") <<
      tr("Chinese");
   int i;
   for( i = 0; i < langStrings.size(); ++i )
      comboBox_lang->setItemText(i, langStrings[i]);
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
   QAbstractButton* button;
   iUnitSystem weightUnitSystem;
   iUnitSystem volumeUnitSystem;
   TempScale temperatureScale;
   Brewtarget::ColorType cformula;
   Brewtarget::IbuType iformula;
   Brewtarget::ColorUnitType colorUnit;
   QString newUserDataDir;

   button = colorGroup->checkedButton();
   if( button == radioButton_mosher )
      cformula = Brewtarget::MOSHER;
   else if( button == radioButton_daniel )
      cformula = Brewtarget::DANIEL;
   else if( button == radioButton_morey )
      cformula = Brewtarget::MOREY;
   else
      cformula = Brewtarget::MOREY; // Should never get here, but you never know.
   
   button = ibuGroup->checkedButton();
   if( button == radioButton_tinseth )
      iformula = Brewtarget::TINSETH;
   else if( button == radioButton_rager )
      iformula = Brewtarget::RAGER;
   else if( button == radioButton_noonan )
      iformula = Brewtarget::NOONAN;
   else
      iformula = Brewtarget::TINSETH; // Should never get here, but you never know.
   
   // Get gravity setting.
   button = gravGroup->checkedButton();
   if( button == radioButton_sg )
   {
      Brewtarget::densityUnit = Brewtarget::SG;
      Brewtarget::thingToUnitSystem.insert(Unit::Density, UnitSystems::sgDensityUnitSystem());
   }
   else
   {
      Brewtarget::densityUnit = Brewtarget::PLATO;
      Brewtarget::thingToUnitSystem.insert(Unit::Density, UnitSystems::platoDensityUnitSystem());
   }

   button = weightGroup->checkedButton();
   if( button == weight_imperial )
   {
      weightUnitSystem = Imperial;
      Brewtarget::thingToUnitSystem.insert(Unit::Mass, UnitSystems::usWeightUnitSystem());
   }
   else if( button == weight_us)
   {
      weightUnitSystem = USCustomary;
      Brewtarget::thingToUnitSystem.insert(Unit::Mass, UnitSystems::usWeightUnitSystem());
   }
   else
   {
      weightUnitSystem = SI;
      Brewtarget::thingToUnitSystem.insert(Unit::Mass, UnitSystems::siWeightUnitSystem());
   }

   button = volumeGroup->checkedButton();
   if( button == volume_imperial )
   {
      volumeUnitSystem = Imperial;
      Brewtarget::thingToUnitSystem.insert(Unit::Volume,UnitSystems::imperialVolumeUnitSystem());
   }
   else if( button == volume_us )
   {
      volumeUnitSystem = USCustomary;
      Brewtarget::thingToUnitSystem.insert(Unit::Volume,UnitSystems::usVolumeUnitSystem());
   }
   else
   {
      volumeUnitSystem = SI;
      Brewtarget::thingToUnitSystem.insert(Unit::Volume,UnitSystems::siVolumeUnitSystem());
   }

   button = tempGroup->checkedButton();
   if( button == fahrenheit )
   {
      temperatureScale = Fahrenheit;
      Brewtarget::thingToUnitSystem.insert(Unit::Temp,UnitSystems::fahrenheitTempUnitSystem());
   }
   else
   {
      temperatureScale = Celsius;
      Brewtarget::thingToUnitSystem.insert(Unit::Temp,UnitSystems::celsiusTempUnitSystem());
   }
   
   button = colorUnitGroup->checkedButton();
   if( button == radioButton_ebc )
   {
      Brewtarget::thingToUnitSystem.insert(Unit::Color,UnitSystems::ebcColorUnitSystem());
      colorUnit = Brewtarget::EBC;
   }
   else
   {
      Brewtarget::thingToUnitSystem.insert(Unit::Color,UnitSystems::srmColorUnitSystem());
      colorUnit = Brewtarget::SRM;
   }

   button = dateFormatGroup->checkedButton();
   if ( button == radioButton_usDate )
      Brewtarget::dateFormat = Unit::displayUS;
   else if ( button == radioButton_euDate )
      Brewtarget::dateFormat = Unit::displayImp;
   else
      Brewtarget::dateFormat = Unit::displaySI;

   Brewtarget::ibuFormula = iformula;
   Brewtarget::colorFormula = cformula;
   Brewtarget::weightUnitSystem = weightUnitSystem;
   Brewtarget::volumeUnitSystem = volumeUnitSystem;
   Brewtarget::tempScale = temperatureScale;
   Brewtarget::colorUnit = colorUnit;

   // Set the right language.
   Brewtarget::setLanguage( ndxToLangCode[ comboBox_lang->currentIndex() ] );

   // Check the new userDataDir.
   newUserDataDir = lineEdit_dbDir->text();

   // Make sure the dir ends with a "/" or "\"
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
   if( !newUserDataDir.endsWith("/") )
      newUserDataDir += "/";
#else
   if( !newUserDataDir.endsWith("\\") && !newUserDataDir.endsWith("/") )
      newUserDataDir += "\\";
#endif

   if( newUserDataDir != Brewtarget::getUserDataDir() )
   {
      // If there are no data files present...
      if( ! QFileInfo(newUserDataDir + "database.sqlite").exists() )
      {
         // ...tell user we will copy old data files to new location.
         QMessageBox::information(this,
                                  tr("Copy Data"),
                                  tr("There do not seem to be any data files in this directory, so we will copy your old data here.")
                                 );
         Brewtarget::copyDataFiles(newUserDataDir);
      }

      Brewtarget::userDataDir = newUserDataDir;
      QMessageBox::information(
         this,
         tr("Restart"),
         tr("Please restart Brewtarget.")
      );
   }

   Brewtarget::setOption("mashHopAdjustment", lineEdit_mashHop->toSI() / 100);
   Brewtarget::setOption("firstWortHopAdjustment", lineEdit_firstWort->toSI() / 100);
   // Make sure the main window updates.
   if( Brewtarget::mainWindow() )
      Brewtarget::mainWindow()->showChanges();

   setVisible(false);
}

void OptionDialog::cancel()
{
   setVisible(false);
}

void OptionDialog::showChanges()
{
   // Set the right language
   int ndx = ndxToLangCode.indexOf( Brewtarget::getCurrentLanguage() );
   if( ndx >= 0 )
      comboBox_lang->setCurrentIndex(ndx);
   
   // Check the right color formula box.
   switch( Brewtarget::colorFormula )
   {
      case Brewtarget::MOREY:
         radioButton_morey->setChecked(true);
         break;
      case Brewtarget::DANIEL:
         radioButton_daniel->setChecked(true);
         break;
      case Brewtarget::MOSHER:
         radioButton_mosher->setChecked(true);
         break;
   }

   // Check the right ibu formula box.
   switch( Brewtarget::ibuFormula )
   {
      case Brewtarget::TINSETH:
         radioButton_tinseth->setChecked(true);
         break;
      case Brewtarget::RAGER:
         radioButton_rager->setChecked(true);
         break;
      case Brewtarget::NOONAN:
         radioButton_noonan->setChecked(true);
         break;
   }

   // Check the right weight unit system box.
   switch( Brewtarget::weightUnitSystem )
   {
      case Imperial:
         weight_imperial->setChecked(true);
         break;
      case USCustomary:
         weight_us->setChecked(true);
         break;
      case SI:
      default:
         weight_si->setChecked(true);
   }

   // Check the right volume unit system box.
   switch( Brewtarget::volumeUnitSystem )
   {
      case Imperial:
         volume_imperial->setChecked(true);
         break;
      case USCustomary:
         volume_us->setChecked(true);
         break;
      case SI:
      default:
         volume_si->setChecked(true);
   }

   // Check gravity.
   if( Brewtarget::densityUnit == Brewtarget::PLATO )
      radioButton_plato->setChecked(true);
   else
      radioButton_sg->setChecked(true);

   // Temp.
   switch( Brewtarget::tempScale )
   {
      case Fahrenheit:
         fahrenheit->setChecked(true);
         break;
      case Celsius:
      default:
         celsius->setChecked(true);
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

   switch(Brewtarget::dateFormat)
   {
      case Unit::displayImp:
         radioButton_euDate->setChecked(true);
         break;
      case Unit::displayUS:
         radioButton_usDate->setChecked(true);
         break;
      case Unit::displaySI:
      default:
         radioButton_isoDate->setChecked(true);
   }

   // Data directory
   lineEdit_dbDir->setText(Brewtarget::getUserDataDir());

   // The IBU modifications. These will all be calculated from a 60 min boil. This is gonna get confusing.
   double amt = Brewtarget::toDouble(Brewtarget::option("mashHopAdjustment",100).toString(), "OptionDialog::showChanges()");
   lineEdit_mashHop->setText(amt*100);

   amt = Brewtarget::toDouble(Brewtarget::option("firstWortHopAdjustment",100).toString(), "OptionDialog::showChanges()");
   lineEdit_firstWort->setText(amt*100);

}

void OptionDialog::changeEvent(QEvent* e)
{
   switch( e->type() )
   {
      case QEvent::LanguageChange:
         retranslate();
         e->accept();
         break;
      default:
         QDialog::changeEvent(e);
         break;
   }
}

