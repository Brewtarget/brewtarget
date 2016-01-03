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
#include <QMessageBox>
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

   // Populate combo boxes on the "Units" tab
   weightComboBox->addItem(tr("SI units"), QVariant(SI));
   weightComboBox->addItem(tr("US traditional units"), QVariant(USCustomary));
   weightComboBox->addItem(tr("British imperial units"), QVariant(Imperial));

   temperatureComboBox->addItem(tr("Celsius"), QVariant(Celsius));
   temperatureComboBox->addItem(tr("Fahrenheit"), QVariant(Fahrenheit));

   volumeComboBox->addItem(tr("SI units"), QVariant(SI));
   volumeComboBox->addItem(tr("US traditional units"), QVariant(USCustomary));
   volumeComboBox->addItem(tr("British imperial units"), QVariant(Imperial));

   gravityComboBox->addItem(tr("20C/20C Specific Gravity"), QVariant(Brewtarget::SG));
   gravityComboBox->addItem(tr("Plato/Brix/Balling"), QVariant(Brewtarget::PLATO));

   dateComboBox->addItem(tr("mm-dd-YYYY"), QVariant(Unit::displayUS));
   dateComboBox->addItem(tr("dd-mm-YYYY"), QVariant(Unit::displayImp));
   dateComboBox->addItem(tr("YYYY-mm-dd"), QVariant(Unit::displaySI));

   colorComboBox->addItem(tr("SRM"), QVariant(Brewtarget::SRM));
   colorComboBox->addItem(tr("EBC"), QVariant(Brewtarget::EBC));

   // Populate combo boxes on the "Formulas" tab
   ibuFormulaComboBox->addItem(tr("Tinseth's approximation"), QVariant(Brewtarget::TINSETH));
   ibuFormulaComboBox->addItem(tr("Rager's approximation"), QVariant(Brewtarget::RAGER));
   ibuFormulaComboBox->addItem(tr("Noonan's approximation"), QVariant(Brewtarget::NOONAN));

   colorFormulaComboBox->addItem(tr("Mosher's approximation"), QVariant(Brewtarget::MOSHER));
   colorFormulaComboBox->addItem(tr("Daniel's approximation"), QVariant(Brewtarget::DANIEL));
   colorFormulaComboBox->addItem(tr("Morey's approximation"), QVariant(Brewtarget::MOREY));

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
   QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), Brewtarget::getUserDataDir().canonicalPath(), QFileDialog::ShowDirsOnly);
   if( ! dir.isEmpty() )
      lineEdit_dbDir->setText( dir );
}

void OptionDialog::defaultDataDir()
{
   lineEdit_dbDir->setText( Brewtarget::getConfigDir().canonicalPath() );
}

void OptionDialog::saveAndClose()
{
   bool okay = false;

   switch (weightComboBox->itemData(weightComboBox->currentIndex()).toInt(&okay))
   {
      case SI:
      default:
         Brewtarget::weightUnitSystem = SI;
         Brewtarget::thingToUnitSystem.insert(Unit::Mass, UnitSystems::siWeightUnitSystem());
         break;
      case USCustomary:
         Brewtarget::weightUnitSystem  = USCustomary;
         Brewtarget::thingToUnitSystem.insert(Unit::Mass, UnitSystems::usWeightUnitSystem());
         break;
      case Imperial:
         Brewtarget::weightUnitSystem  = Imperial;
         Brewtarget::thingToUnitSystem.insert(Unit::Mass, UnitSystems::usWeightUnitSystem());
         break;
   }

   switch (temperatureComboBox->itemData(temperatureComboBox->currentIndex()).toInt(&okay))
   {
      case Celsius:
      default:
         Brewtarget::tempScale = Celsius;
         Brewtarget::thingToUnitSystem.insert(Unit::Temp,UnitSystems::celsiusTempUnitSystem());
         break;
      case Fahrenheit:
         Brewtarget::tempScale = Fahrenheit;
         Brewtarget::thingToUnitSystem.insert(Unit::Temp,UnitSystems::fahrenheitTempUnitSystem());
         break;
   }


   switch (volumeComboBox->itemData(volumeComboBox->currentIndex()).toInt(&okay))
   {
      case SI:
      default:
         Brewtarget::volumeUnitSystem = SI;
         Brewtarget::thingToUnitSystem.insert(Unit::Volume,UnitSystems::siVolumeUnitSystem());
         break;
      case USCustomary:
         Brewtarget::volumeUnitSystem = USCustomary;
         Brewtarget::thingToUnitSystem.insert(Unit::Volume,UnitSystems::usVolumeUnitSystem());
         break;
      case Imperial:
         Brewtarget::volumeUnitSystem = Imperial;
         Brewtarget::thingToUnitSystem.insert(Unit::Volume,UnitSystems::imperialVolumeUnitSystem());
         break;
   }

   switch (gravityComboBox->itemData(gravityComboBox->currentIndex()).toInt(&okay))
   {
      case Brewtarget::SG:
      default:
         Brewtarget::densityUnit = Brewtarget::SG;
         Brewtarget::thingToUnitSystem.insert(Unit::Density, UnitSystems::sgDensityUnitSystem());
         break;
      case Brewtarget::PLATO:
         Brewtarget::densityUnit = Brewtarget::PLATO;
         Brewtarget::thingToUnitSystem.insert(Unit::Density, UnitSystems::platoDensityUnitSystem());
         break;
   }

   switch (dateComboBox->itemData(dateComboBox->currentIndex()).toInt(&okay))
   {
      case Unit::displayUS:
      default:
         Brewtarget::dateFormat = Unit::displayUS;
         break;
      case Unit::displayImp:
         Brewtarget::dateFormat = Unit::displayImp;
         break;
      case Unit::displaySI:
         Brewtarget::dateFormat = Unit::displaySI;
         break;
   }

   switch (colorComboBox->itemData(colorComboBox->currentIndex()).toInt(&okay))
   {
      case Brewtarget::SRM:
      default:
         Brewtarget::thingToUnitSystem.insert(Unit::Color,UnitSystems::srmColorUnitSystem());
         Brewtarget::colorUnit = Brewtarget::SRM;
         break;
      case Brewtarget::EBC:
         Brewtarget::thingToUnitSystem.insert(Unit::Color,UnitSystems::ebcColorUnitSystem());
         Brewtarget::colorUnit = Brewtarget::EBC;
         break;
   }

   Brewtarget::ibuFormula = Brewtarget::IbuType(
           ibuFormulaComboBox->itemData(ibuFormulaComboBox->currentIndex()).toInt(&okay));

   Brewtarget::colorFormula = Brewtarget::ColorType(
           colorFormulaComboBox->itemData(colorFormulaComboBox->currentIndex()).toInt(&okay));

   // Set the right language.
   Brewtarget::setLanguage( ndxToLangCode[ comboBox_lang->currentIndex() ] );

   // Check the new userDataDir.
   QString newUserDataDir = lineEdit_dbDir->text();

   QDir userDirectory(newUserDataDir);

   if( userDirectory != Brewtarget::getUserDataDir() )
   {
      // If there are no data files present...
      if( ! QFileInfo(userDirectory, "database.sqlite").exists() )
      {
         // ...tell user we will copy old data files to new location.
         QMessageBox::information(this,
                                  tr("Copy Data"),
                                  tr("There do not seem to be any data files in this directory, so we will copy your old data here.")
                                 );
         Brewtarget::copyDataFiles(newUserDataDir);
      }

      Brewtarget::userDataDir = newUserDataDir;
      Brewtarget::setOption("user_data_dir", newUserDataDir);
      QMessageBox::information(
         this,
         tr("Restart"),
         tr("Please restart Brewtarget.")
      );
   }

   Brewtarget::setOption("mashHopAdjustment", ibuAdjustmentMashHopDoubleSpinBox->value() / 100);
   Brewtarget::setOption("firstWortHopAdjustment", ibuAdjustmentFirstWortDoubleSpinBox->value() / 100);

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


   weightComboBox->setCurrentIndex(weightComboBox->findData(Brewtarget::weightUnitSystem));
   temperatureComboBox->setCurrentIndex(temperatureComboBox->findData(Brewtarget::tempScale));
   volumeComboBox->setCurrentIndex(volumeComboBox->findData(Brewtarget::volumeUnitSystem));
   gravityComboBox->setCurrentIndex(gravityComboBox->findData(Brewtarget::densityUnit));
   dateComboBox->setCurrentIndex(dateComboBox->findData(Brewtarget::dateFormat));
   colorComboBox->setCurrentIndex(colorComboBox->findData(Brewtarget::colorUnit));

   colorFormulaComboBox->setCurrentIndex(colorFormulaComboBox->findData(Brewtarget::ibuFormula));
   ibuFormulaComboBox->setCurrentIndex(ibuFormulaComboBox->findData(Brewtarget::colorFormula));

   // Data directory
   lineEdit_dbDir->setText(Brewtarget::getUserDataDir().canonicalPath());

   // The IBU modifications. These will all be calculated from a 60 min boil. This is gonna get confusing.
   double amt = Brewtarget::toDouble(Brewtarget::option("mashHopAdjustment",100).toString(), "OptionDialog::showChanges()");
   ibuAdjustmentMashHopDoubleSpinBox->setValue(amt*100);

   amt = Brewtarget::toDouble(Brewtarget::option("firstWortHopAdjustment",100).toString(), "OptionDialog::showChanges()");
   ibuAdjustmentFirstWortDoubleSpinBox->setValue(amt*100);

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

