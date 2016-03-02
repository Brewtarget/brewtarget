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
#include "BtLineEdit.h"
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
#include "database.h"
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
      "da" <<
      "de" <<
      "el" <<
      "en" <<
      "es" <<
      "et" <<
      "eu" <<
      "fr" <<
      "gl" <<
      "hu" <<
      "it" <<
      "lv" <<
      "nb" <<
      "nl" <<
      "pl" <<
      "pt" <<
      "ru" <<
      "sr" <<
      "sv" <<
      "tr" <<
      "zh";

   // Do this just to have model indices to set icons.
   comboBox_lang->addItems(ndxToLangCode);
   // MUST correspond to ndxToLangCode.
   langIcons <<
      /*ca*/ QIcon(":images/flagCatalonia.svg") <<
      /*cs*/ QIcon(":images/flagCzech.svg") <<
      /*da*/ QIcon() <<
      /*de*/ QIcon(":images/flagGermany.svg") <<
      /*el*/ QIcon(":images/flagGreece.svg") <<
      /*en*/ QIcon(":images/flagUK.svg") <<
      /*es*/ QIcon(":images/flagSpain.svg") <<
      /*et*/ QIcon() <<
      /*eu*/ QIcon() <<
      /*fr*/ QIcon(":images/flagFrance.svg") <<
      /*gl*/ QIcon() <<
      /*hu*/ QIcon() <<
      /*it*/ QIcon(":images/flagItaly.svg") <<
      /*lv*/ QIcon() <<
      /*nb*/ QIcon() <<
      /*nl*/ QIcon(":images/flagNetherlands.svg") <<
      /*pl*/ QIcon(":images/flagPoland.svg") <<
      /*pt*/ QIcon(":images/flagBrazil.svg") <<
      /*ru*/ QIcon(":images/flagRussia.svg") <<
      /*sr*/ QIcon() <<
      /*sv*/ QIcon() <<
      /*tr*/ QIcon() <<
      /*zh*/ QIcon(":images/flagChina.svg");
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

   // database panel stuff
   comboBox_engine->addItem( tr("SQLite (default)"), QVariant(Brewtarget::SQLITE));
   comboBox_engine->addItem( tr("PostgreSQL"), QVariant(Brewtarget::PGSQL));
   connect( comboBox_engine, SIGNAL( currentIndexChanged(int) ), this, SLOT( setEngine(int) ) );
   connect( pushButton_testConnection, SIGNAL( clicked() ), this, SLOT(testConnection()));
   connect( checkBox_savePassword, SIGNAL(clicked(bool)), this, SLOT(savePassword(bool)));

   // I hope this works
   connect( btStringEdit_hostname, SIGNAL( textModified() ), this, SLOT(testRequired()));
   connect( btStringEdit_portnum, SIGNAL( textModified() ), this, SLOT(testRequired()));
   connect( btStringEdit_schema, SIGNAL( textModified() ), this, SLOT(testRequired()));
   connect( btStringEdit_dbname, SIGNAL( textModified() ), this, SLOT(testRequired()));
   connect( btStringEdit_username, SIGNAL( textModified() ), this, SLOT(testRequired()));
   connect( btStringEdit_password, SIGNAL( textModified() ), this, SLOT(testRequired()));

   pushButton_testConnection->setEnabled(false);

}

void OptionDialog::retranslate()
{
   // Let the Ui take care of its business
   retranslateUi(this);

   // Retranslate the language combobox.
   // NOTE: the indices MUST correspond to ndxToLangCode.
   QStringList langStrings;
   langStrings <<
      /*ca*/ tr("Catalan") <<
      /*cs*/ tr("Czech") <<
      /*da*/ tr("Danish") <<
      /*de*/ tr("German") <<
      /*el*/ tr("Greek") <<
      /*en*/ tr("English") <<
      /*es*/ tr("Spanish") <<
      /*et*/ tr("Estonian") <<
      /*eu*/ tr("Basque") <<
      /*fr*/ tr("French") <<
      /*gl*/ tr("Galician") <<
      /*hu*/ tr("Hungarian") <<
      /*it*/ tr("Italian") <<
      /*lv*/ tr("Latvian") <<
      /*nb*/ tr("Norwegian Bokmål") <<
      /*nl*/ tr("Dutch") <<
      /*pl*/ tr("Polish") <<
      /*pt*/ tr("Portuguese") <<
      /*ru*/ tr("Russian") <<
      /*sr*/ tr("Serbian") <<
      /*sv*/ tr("Swedish") <<
      /*tr*/ tr("Turkish") <<
      /*zh*/ tr("Chinese");
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
   bool saveDbConfig = true;

   // TODO:: FIX THIS UI. I am really not sure what the best approach is here.
   if ( status == OptionDialog::NEEDSTEST || status == OptionDialog::TESTFAILED )
   {
      QMessageBox::critical(0,
            tr("Test connection or cancel"),
            tr("Saving the options without testing the connection can cause brewtarget to not restart. Your changes have been discarded, which is likely really, really crappy UX. Please open a bug explaining exactly how you got to this message.")
            );
      return;
   }

   if ( status == OptionDialog::TESTPASSED ) {
      // This got unpleasant. There are multiple possible transer paths.
      // SQLite->Pgsql, Pgsql->Pgsql and Pgsql->SQLite. This will ensure we
      // preserve the information required.
      try {
         QString theQuestion = tr("Would you like brewtarget to transfer your data to the new database? NOTE: If you've already loaded the data, say No");
         if ( QMessageBox::Yes == QMessageBox::question(this, tr("Transfer database"), theQuestion) )
            Database::instance().convertDatabase(btStringEdit_hostname->text(), btStringEdit_dbname->text(),
                                                 btStringEdit_username->text(), btStringEdit_password->text(),
                                                 btStringEdit_portnum->text().toInt(),
                                                 (Brewtarget::DBTypes)comboBox_engine->currentIndex());
         // Database engine stuff
         Brewtarget::setOption("dbType", comboBox_engine->currentIndex());
         Brewtarget::setOption("dbHostname", btStringEdit_hostname->text());
         Brewtarget::setOption("dbPortnum", btStringEdit_portnum->text());
         Brewtarget::setOption("dbSchema", btStringEdit_schema->text());
         Brewtarget::setOption("dbName", btStringEdit_dbname->text());
         Brewtarget::setOption("dbUsername", btStringEdit_username->text());
         QMessageBox::information(this, tr("Restart"), tr("Please restart brewtarget to connect to the new database"));
      }
      catch (QString e) {
         Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
         saveDbConfig = false;
      }
   }

   if ( saveDbConfig && checkBox_savePassword->checkState() == Qt::Checked ) {
      Brewtarget::setOption("dbPassword", btStringEdit_password->text());
   }
   else {
      Brewtarget::removeOption("dbPassword");
   }
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

   int ndx = ibuFormulaComboBox->itemData(ibuFormulaComboBox->currentIndex()).toInt(&okay);
   Brewtarget::ibuFormula = static_cast<Brewtarget::IbuType>(ndx);
   ndx = colorFormulaComboBox->itemData(colorFormulaComboBox->currentIndex()).toInt(&okay);
   Brewtarget::colorFormula = static_cast<Brewtarget::ColorType>(ndx);

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

   colorFormulaComboBox->setCurrentIndex(colorFormulaComboBox->findData(Brewtarget::colorFormula));
   ibuFormulaComboBox->setCurrentIndex(ibuFormulaComboBox->findData(Brewtarget::ibuFormula));

   // Data directory
   lineEdit_dbDir->setText(Brewtarget::getUserDataDir().canonicalPath());

   // The IBU modifications. These will all be calculated from a 60 min boil. This is gonna get confusing.
   double amt = Brewtarget::toDouble(Brewtarget::option("mashHopAdjustment",0).toString(), "OptionDialog::showChanges()");
   ibuAdjustmentMashHopDoubleSpinBox->setValue(amt*100);

   amt = Brewtarget::toDouble(Brewtarget::option("firstWortHopAdjustment",1.1).toString(), "OptionDialog::showChanges()");
   ibuAdjustmentFirstWortDoubleSpinBox->setValue(amt*100);

   // Database stuff -- this looks weird, but trust me. We want SQLITE to be
   // the default for this field
   int tmp = (Brewtarget::DBTypes)Brewtarget::option("dbType",Brewtarget::SQLITE).toInt();
   comboBox_engine->setCurrentIndex(tmp);
   groupBox_dbConfig->setEnabled(tmp != Brewtarget::SQLITE);

   btStringEdit_hostname->setText(Brewtarget::option("dbHostname","localhost").toString());
   btStringEdit_portnum->setText(Brewtarget::option("dbPort","5432").toString());
   btStringEdit_schema->setText(Brewtarget::option("dbSchema","public").toString());
   btStringEdit_dbname->setText(Brewtarget::option("dbName","brewtarget").toString());
   btStringEdit_username->setText(Brewtarget::option("dbUsername","brewtarget").toString());
   btStringEdit_password->setText(Brewtarget::option("dbPassword","").toString());
   checkBox_savePassword->setChecked( Brewtarget::hasOption("dbPassword") );

   status = OptionDialog::NOCHANGE;
   changeColors();
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

void OptionDialog::setEngine(int selected)
{
   Brewtarget::DBTypes newEngine = (Brewtarget::DBTypes)selected;

   groupBox_dbConfig->setEnabled(newEngine != Brewtarget::SQLITE);
   testRequired();

}

void OptionDialog::testConnection()
{
   bool success;
   QString hostname, schema, database, username, password;
   int port;

   Brewtarget::DBTypes newType = (Brewtarget::DBTypes)comboBox_engine->currentIndex();
   // Do nothing if nothing is required.
   if ( status == OptionDialog::NOCHANGE || status == OptionDialog::TESTPASSED)
   {
      return;
   }

   switch( newType )
   {
      case Brewtarget::PGSQL:
         hostname = btStringEdit_hostname->text();
         schema   = btStringEdit_schema->text();
         database = btStringEdit_dbname->text();
         username = btStringEdit_username->text();
         password = btStringEdit_password->text();
         port     = (btStringEdit_portnum->text()).toInt();

         success = Database::verifyDbConnection(newType,hostname,port,schema,database,username,password);
         break;
      default:
         hostname = QString("%1/%2").arg(lineEdit_dbDir->text()).arg("database.sqlite");
         success = Database::verifyDbConnection(newType,hostname);
   }

   if ( success ) 
   {
      QMessageBox::information(0,
                           QObject::tr("Connection Test"),
                           QString(QObject::tr("Connection to database was successful"))
                           );
      status = OptionDialog::TESTPASSED;
   }
   else 
   {
      // Database::testConnection already popped the dialog
      status = OptionDialog::TESTFAILED;
   }
   changeColors();
}

void OptionDialog::testRequired()
{
   status = OptionDialog::NEEDSTEST;
   changeColors();
}

void OptionDialog::changeColors()
{
   // Yellow when the test is needed
   // Red when the test failed
   // Green when the test passed
   // Black otherwise.

   switch(status)
   {
      case OptionDialog::NEEDSTEST:
         buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
         pushButton_testConnection->setEnabled(true);
         pushButton_testConnection->setStyleSheet("color:rgb(240,225,25)");
         break;
      case OptionDialog::TESTFAILED:
         buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
         pushButton_testConnection->setStyleSheet("color:red");
         break;
      case OptionDialog::TESTPASSED:
         pushButton_testConnection->setStyleSheet("color:green");
         buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
         pushButton_testConnection->setEnabled(false);
         break;
      case OptionDialog::NOCHANGE:
         pushButton_testConnection->setStyleSheet("color:grey");
         buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
         pushButton_testConnection->setEnabled(false);
         break;
   }
}

void OptionDialog::savePassword(bool state)
{
   if ( state ) {
      QMessageBox::warning(0, QObject::tr("Plaintext"), 
                              QObject::tr("Passwords are saved in plaintext. We make no effort to hide, obscure or otherwise protect the password. By enabling this option, you take full responsibility for any potential problems."));
   }
}
