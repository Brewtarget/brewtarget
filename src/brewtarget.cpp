/*
 * brewtarget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <iostream>
#include <QFile>
#include <QIODevice>
#include <QString>
#include <QDomNode>
#include <QDomElement>
#include <QDomText>
#include <QDomNodeList>
#include <QTextStream>
#include <QObject>
#include <QLocale>
#include <QLibraryInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QEventLoop>
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QObject>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSharedPointer>
#include <QtNetwork/QNetworkRequest>
#include <QPixmap>
#include <QSplashScreen>
#include <QSettings>

#include "brewtarget.h"
#include "config.h"
#include "database.h"
#include "Algorithms.h"
#include "fermentable.h"
#include "UnitSystem.h"
#include "UnitSystems.h"
#include "USWeightUnitSystem.h"
#include "USVolumeUnitSystem.h"
#include "FahrenheitTempUnitSystem.h"
#include "TimeUnitSystem.h"
#include "SIWeightUnitSystem.h"
#include "SIVolumeUnitSystem.h"
#include "CelsiusTempUnitSystem.h"
#include "ImperialVolumeUnitSystem.h"
#include "BtSplashScreen.h"
#include "MainWindow.h"

MainWindow* Brewtarget::mainWindow;
QDomDocument* Brewtarget::optionsDoc;
QTranslator* Brewtarget::defaultTrans = new QTranslator();
QTranslator* Brewtarget::btTrans = new QTranslator();
QTextStream* Brewtarget::logStream = 0;
QFile* Brewtarget::logFile = 0;
QSettings Brewtarget::btSettings("brewtarget");

bool Brewtarget::userDatabaseDidNotExist = false;
QDateTime Brewtarget::lastDbMergeRequest = QDateTime::fromString("1986-02-24T06:00:00", Qt::ISODate);

QString Brewtarget::currentLanguage = "en";
QString Brewtarget::userDataDir = getConfigDir();


bool Brewtarget::checkVersion = true;

iUnitSystem Brewtarget::weightUnitSystem = SI;
iUnitSystem Brewtarget::volumeUnitSystem = SI;

UnitSystem* Brewtarget::weightSystem = UnitSystems::usWeightUnitSystem();
UnitSystem* Brewtarget::volumeSystem = UnitSystems::usVolumeUnitSystem();
UnitSystem* Brewtarget::tempSystem = UnitSystems::fahrenheitTempUnitSystem();
UnitSystem* Brewtarget::timeSystem = UnitSystems::timeUnitSystem();

TempScale Brewtarget::tempScale = Celsius;
Brewtarget::ColorType Brewtarget::colorFormula = Brewtarget::MOREY;
Brewtarget::IbuType Brewtarget::ibuFormula = Brewtarget::TINSETH;
Brewtarget::ColorUnitType Brewtarget::colorUnit = Brewtarget::SRM;

bool Brewtarget::usePlato = false;

bool Brewtarget::ensureDirectoriesExist()
{
   bool success;
   QDir dir;

   dir.setPath(getDataDir());
   if( ! dir.exists() || ! dir.isReadable() )
   {
      QMessageBox::information(0,
                               QObject::tr("Directory Problem"),
                               QObject::tr("\"%1\" cannot be read.").arg(dir.path()));
      return false;
   }

   dir.setPath(getDocDir());
   if( ! dir.exists() || ! dir.isReadable() )
   {
      QMessageBox::information(0,
                               QObject::tr("Directory Problem"),
                               QObject::tr("\"%1\" cannot be read.").arg(dir.path()));
      return false;
   }

   dir.setPath(getConfigDir(&success));
   if( !success || ! dir.exists() || ! dir.isReadable() )
   {
      QMessageBox::information(0,
                               QObject::tr("Directory Problem"),
                               QObject::tr("Config directory \"%1\" cannot be read.").arg(dir.path()));
      return false;
   }

   return true;
}

void Brewtarget::checkForNewVersion(MainWindow* mw)
{

   // Don't do anything if the checkVersion flag was set false
   if ( checkVersion == false ) 
      return;

   QNetworkAccessManager manager;
   QUrl url("http://brewtarget.sourceforge.net/version");
   QNetworkReply* reply = manager.get( QNetworkRequest(url) );
   QObject::connect( reply, SIGNAL(finished()), mw, SLOT(finishCheckingVersion()) );
}

bool Brewtarget::copyDataFiles(QString newPath)
{
   QString dbFileName, recipeFileName, mashFileName, optionsFileName;
   bool success = true;

   // Database files.
   dbFileName = getUserDataDir() + "database.xml";
   recipeFileName = getUserDataDir() + "recipes.xml";
   mashFileName = getUserDataDir() + "mashs.xml";
   //optionsFileName = getUserDataDir() + "options.xml";

   success &= QFile::copy(dbFileName, newPath + "database.xml");
   success &= QFile::copy(recipeFileName, newPath + "recipes.xml");
   success &= QFile::copy(mashFileName, newPath + "mashs.xml");
   //success &= QFile::copy(optionsFileName, newPath + "options.xml");

   return success;
}

bool Brewtarget::ensureOptionFileExists()
{
   QString optionsFileName;
   QFile optionsFile;
   bool success = true;

   optionsFileName = getConfigDir() + "options.xml";
   optionsFile.setFileName(optionsFileName);

   if( !optionsFile.exists() )
   {
      success = QFile::copy(Brewtarget::getDataDir() + "options.xml", optionsFileName);
      if( ! success )
      {
         logE(QString("Could not copy \"%1\" to \"%2\"").arg(Brewtarget::getDataDir() + "options.xml").arg(optionsFileName));
         return false;
      }
   }

   return success;
}

bool Brewtarget::ensureDataFilesExist()
{
   QString dbFileName, recipeFileName, mashFileName, optionsFileName, logFileName;
   QFile dbFile, recipeFile, mashFile, optionsFile;
   bool success = true;
   
   logFile = new QFile();

   // Database files.
   dbFileName = getUserDataDir() + "database.xml";
   recipeFileName = getUserDataDir() + "recipes.xml";
   mashFileName = getUserDataDir() + "mashs.xml";
   
   dbFile.setFileName(dbFileName);
   recipeFile.setFileName(recipeFileName);
   mashFile.setFileName(mashFileName);
   
   // Log file
   logFile->setFileName(getUserDataDir() + "brewtarget_log.txt");
   if( logFile->open(QFile::WriteOnly | QFile::Truncate) )
      logStream = new QTextStream(logFile);
   else
   {
      // Put the log in a temporary directory.
      logFile->setFileName(QDir::tempPath() + "/brewtarget_log.txt");
      if( logFile->open(QFile::WriteOnly | QFile::Truncate ) )
      {
         logW(QString("Log is in a temporary directory: %1").arg(logFile->fileName()) );
         logStream = new QTextStream(logFile);
      }
      else
         logW(QString("Could not create a log file."));
   }

   if( !dbFile.exists() )
   {
      userDatabaseDidNotExist = true;
      success = QFile::copy(Brewtarget::getDataDir() + "database.xml", dbFileName);
      if( ! success )
      {
         logE(QString("Could not copy \"%1\" to \"%2\"").arg(Brewtarget::getDataDir() + "database.xml").arg(dbFileName));
         return false;
      }
   }
   if( !recipeFile.exists() )
   {
      userDatabaseDidNotExist = true;
      success = QFile::copy(Brewtarget::getDataDir() + "recipes.xml", recipeFileName);
      if( ! success )
      {
         logE(QString("Could not copy \"%1\" to \"%2\"").arg(Brewtarget::getDataDir() + "recipes.xml").arg(recipeFileName));
         return false;
      }
   }
   if( !mashFile.exists() )
   {
      userDatabaseDidNotExist = true;
      success &= QFile::copy(Brewtarget::getDataDir() + "mashs.xml", mashFileName);
      if( ! success )
      {
         logE(QString("Could not copy \"%1\" to \"%2\"").arg(Brewtarget::getDataDir() + "mashs.xml").arg(mashFileName));
         return false;
      }
   }

   return success;
}

const QString& Brewtarget::getSystemLanguage()
{
   // QLocale::name() is of the form language_country,
   // where 'language' is a lowercase 2-letter ISO 639-1 language code,
   // and 'country' is an uppercase 2-letter ISO 3166 country code.
   return QLocale::system().name().split("_")[0];
}

void Brewtarget::loadTranslations()
{
   if( qApp == 0 )
      return;

   // Load translators.
   defaultTrans->load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
   if( getCurrentLanguage().isEmpty() )
      setLanguage(getSystemLanguage());
   //btTrans->load("bt_" + getSystemLanguage());

   // Install translators.
   qApp->installTranslator(defaultTrans);
   //qApp->installTranslator(btTrans);
}

void Brewtarget::setLanguage(QString twoLetterLanguage)
{
   currentLanguage = twoLetterLanguage;
   qApp->removeTranslator(btTrans);

   QString filename = QString("bt_%1").arg(twoLetterLanguage);
   QString dir = QString("%1translations_qm/").arg(getDataDir());
   if( btTrans->load( filename, dir ) )
      qApp->installTranslator(btTrans);
}

const QString& Brewtarget::getCurrentLanguage()
{
   return currentLanguage;
}

iUnitSystem Brewtarget::getWeightUnitSystem()
{
   return weightUnitSystem;
}

iUnitSystem Brewtarget::getVolumeUnitSystem()
{
   return volumeUnitSystem;
}

unitDisplay Brewtarget::getColorUnit()
{
   if ( colorUnit == Brewtarget::SRM )
      return displaySrm;

   return displayEcb;
}

TempScale Brewtarget::getTemperatureScale()
{
   return tempScale;
}

QString Brewtarget::getDataDir()
{
   QString dir = qApp->applicationDirPath();
#if defined(Q_WS_X11) // Linux OS.

   dir = QString(CONFIGDATADIR);
   
#elif defined(Q_WS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/";

#else // Windows OS.

   dir += "/../data/";

#endif

   if( ! dir.endsWith('/') )
      dir += "/";

   return dir;
}

QString Brewtarget::getDocDir()
{
   QString dir = qApp->applicationDirPath();
#if defined(Q_WS_X11) // Linux OS.

   dir = QString(CONFIGDOCDIR);

#elif defined(Q_WS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/en.lproj/";

#else // Windows OS.

   dir += "/../doc/";

#endif

   if( ! dir.endsWith('/') )
      dir += "/";

   return dir;
}

QString Brewtarget::getConfigDir(bool *success)
{
#if defined(Q_WS_X11) || defined(Q_WS_MAC) // Linux OS or Mac OS.

   QDir dir;
   QFileInfo fileInfo;
   char* xdg_config_home = getenv("XDG_CONFIG_HOME");
   bool tmp;
   
   // First, try XDG_CONFIG_HOME.
   // If that variable doesn't exist, create ~/.config
   if (xdg_config_home)
   {
      dir = xdg_config_home;
   }
   else
   {
      // Creating config directory.
      dir = QDir::home();
      if( !dir.exists(".config") )
      {
         logW( QString("Config dir \"%1\" did not exist...").arg(dir.absolutePath() + "/.config") );
         tmp = dir.mkdir(".config");
         logW( QString( tmp ? "...created it." : "...could not create it.") );
         if( !tmp )
         {
            // Failure.
            if( success != 0 )
               *success = false;
            return "";
         }
      }

      // CD to config directory.
      if( ! dir.cd(".config") )
      {
         logE( QString("Could not CD to \"%1\".").arg(dir.absolutePath() + "/.config") );
         if( success != 0 )
            *success = false;
         return "";
      }
   }

   // See if brewtarget dir exists.
   if( !dir.exists("brewtarget") )
   {
      logW( QString("\"%1\" does not exist...creating.").arg(dir.absolutePath() + "/brewtarget") );

      // Try to make brewtarget dir.
      if( ! dir.mkdir("brewtarget") )
      {
         logE( QString("Could not create \"%1\"").arg(dir.absolutePath() + "/brewtarget") );
         if( success != 0 )
            *success = false;
         return "";
      }
   }

   if( ! dir.cd("brewtarget") )
   {
      logE(QString("Could not CD into \"%1\"").arg(dir.absolutePath() + "/brewtarget"));
      if( success != 0 )
         *success = false;
      return "";
   }

   if( success != 0 )
      *success = true;
   return dir.absolutePath() + "/";

#else // Windows OS.

   QDir dir;
   // Before the app is running, app==0
   if( app != 0 )
   {
      // This is the bin/ directory.
      dir = QDir(qApp->applicationDirPath());
      dir.cdUp();
   }
   else
   {
      // We're either in bin/ or bin/../
      dir = QDir::current(); // Guess it's bin/../.
      if( ! dir.exists("bin") ) // Already in bin/
         dir.cdUp();
   }
   // Now we should be in the base directory (i.e. Brewtarget-1.2.4/)

   dir.cd("data");
   if( success != 0 )
      *success = true;
   return dir.absolutePath() + "/";

#endif
}

QString Brewtarget::getUserDataDir()
{
   if( userDataDir.endsWith('/') || userDataDir.endsWith('\\') )
      return userDataDir;
   else
      return userDataDir + "/";
}

int Brewtarget::run()
{
   int ret;
   bool success;
   
   //QPixmap splashImg(BTICON);
   //QSplashScreen splashScreen(splashImg);
   BtSplashScreen splashScreen;
   splashScreen.show();
   
   qApp->processEvents(); // So we can process mouse clicks on splash window.
   
   success = ensureDirectoriesExist(); // Make sure all the necessary directories are ok.
   ensureOptionFileExists();
   readPersistentOptions(); // Read all the options for bt.

   if( success )
      success = ensureDataFilesExist(); // Make sure all the files we need exist before starting.
   if( ! success )
      return 1;

   loadTranslations(); // Do internationalization.

#if defined(Q_WS_MAC)
	qt_set_sequence_auto_mnemonic(TRUE); // turns on Mac Keyboard shortcuts
#endif
   qApp->processEvents();
   splashScreen.showMessage("Loading...");
   qApp->processEvents();
  
   // Check if the database was successfully loaded before
   // loading the main window.
   if (Database::instance().loadSuccessful())
   {
      mainWindow = new MainWindow();
      mainWindow->setVisible(true);
   
      splashScreen.finish(mainWindow);

      checkForNewVersion(mainWindow);

      ret = qApp->exec();
   
      savePersistentOptions();
   }
   
   // Close log file.
   if( logStream )
   {
      delete logStream;
      logStream = 0;
   }
   if( logFile != 0 && logFile->isOpen() )
   {
      logFile->close();
      delete logFile;
      logFile = 0;
   }
   
   // Should I do qApp->removeTranslator() first?
   delete defaultTrans;
   delete btTrans;
   delete mainWindow;
   
   Database::dropInstance();
   
   return ret;
}

void Brewtarget::log(LogType lt, QString message)
{
   QString m;
   
   if( lt == WARNING )
      m = QString("WARNING: %1").arg(message);
   else if( lt == ERROR )
      m = QString("ERROR: %1").arg(message);
   else
      m = message;
   
   // First, write out to stderr.
   std::cerr << m.toStdString() << std::endl;
   // Then display it in the GUI's status bar.
   // Hmm... I can't access the mainWindow from outside the
   // GUI event loop?
   //if( mainWindow->statusBar() != 0 )
   //   mainWindow->statusBar()->showMessage(m, 3000);

   // Now, write it to the log file if there is one.
   if( logStream != 0 )
      *logStream << m << "\n";
}

void Brewtarget::logE( QString message )
{
   log( ERROR, message );
}

void Brewtarget::logW( QString message )
{
   log( WARNING, message );
}

// Displays "amount" of units "units" in the proper format.
// If "units" is null, just return the amount.
QString Brewtarget::displayAmount( double amount, Unit* units, int precision, unitDisplay displayUnits, unitScale displayScale)
{
   int fieldWidth = 0;
   char format = 'f';
   UnitSystem* temp;

   // Check for insane values.
   if( Algorithms::Instance().isnan(amount) || Algorithms::Instance().isinf(amount) )
      return "?";
   
   // Special case.
   if( units == 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   QString ret;

   // convert to the current unit system (s).

   if(SIUnitName.compare("kg") == 0) // Dealing with mass.
   {
      temp = findMassUnitSystem(displayUnits);
      ret = temp->displayAmount( amount, units, displayScale );
   }
   else if( SIUnitName.compare("L") == 0 ) // Dealing with volume
   {
      temp = findVolumeUnitSystem(displayUnits);
      ret = temp->displayAmount(amount,units,displayScale);
   }
   else if( SIUnitName.compare("C") == 0 ) // Dealing with temperature.
   {
      temp = findTemperatureSystem(displayUnits);
      ret = temp->displayAmount( amount, units );
   }
   else if( SIUnitName.compare("min") == 0 ) // Time
      ret = timeSystem->displayAmount( amount, units );
   else // If we don't deal with it above, just use the SI amount.
      ret = QString("%1 %2").arg(SIAmount, fieldWidth, format, precision).arg(SIUnitName);

   return ret;
}

QString Brewtarget::displayAmount(BeerXMLElement* element, QObject* object, QString attribute, Unit* units, int precision )
{
   double amount = 0.0;
   unitScale dispScale;
   unitDisplay dispUnit;

   if ( element->property(attribute.toLatin1().constData()).canConvert(QVariant::Double) )
   {
      // Get the amount
      amount = element->property(attribute.toLatin1().constData()).toDouble();
      // Get the display units and scale
      dispUnit  = (unitDisplay)option(attribute, noUnit,  object, UNIT).toInt();
      dispScale = (unitScale)option(  attribute, noScale, object, SCALE).toInt();
      
      return displayAmount(amount, units, precision, dispUnit, dispScale);
   }
   else
      return "?";

}

UnitSystem* Brewtarget::findVolumeUnitSystem( unitDisplay system )
{
   if ( system == noUnit ) 
      return volumeSystem;

   if ( system == displayUS )
      return UnitSystems::usVolumeUnitSystem();
   else if ( system == displayImp )
      return UnitSystems::imperialVolumeUnitSystem();
   else 
      return UnitSystems::siVolumeUnitSystem();
}

UnitSystem* Brewtarget::findMassUnitSystem( unitDisplay system )
{
   if ( system == noUnit ) 
      return weightSystem;

   // Both imperial and US are the same. So I cheat.
   if ( system == displaySI )
      return UnitSystems::siWeightUnitSystem();
   else 
      return UnitSystems::usWeightUnitSystem();
}

UnitSystem* Brewtarget::findTemperatureSystem( unitDisplay system )
{
   if ( system == noUnit )
      return tempSystem;

   // Not sure how to handle Kelvin, but it will have to be upstream.
   if ( system == displayUS )
      return UnitSystems::fahrenheitTempUnitSystem();
   else 
      return UnitSystems::celsiusTempUnitSystem();
}

void Brewtarget::getThicknessUnits( Unit** volumeUnit, Unit** weightUnit )
{
   *volumeUnit = volumeSystem->thicknessUnit();
   *weightUnit = weightSystem->thicknessUnit();
}

QString Brewtarget::displayThickness( double thick_lkg, bool showUnits )
{
   int fieldWidth = 0;
   char format = 'f';
   int precision = 2;

   Unit* volUnit = volumeSystem->thicknessUnit();
   Unit* weightUnit = weightSystem->thicknessUnit();

   double num = volUnit->fromSI(thick_lkg);
   double den = weightUnit->fromSI(1.0);

   if( showUnits )
      return QString("%1 %2/%3").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
   else
      return QString("%1").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
}

QString Brewtarget::getOptionValue(const QDomDocument& optionsDoc, const QString& option, bool* hasOption)
{
   QDomNode node, child;
   QDomText textNode;
   QDomNodeList list;

   list = optionsDoc.elementsByTagName(option);
   if(list.length() <= 0)
   {
      Brewtarget::logW(QString("Could not find the <%1> tag in the option file.").arg(option));
      if( hasOption != 0 )
         *hasOption = false;
      return "";
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();

      if( hasOption != 0 )
         *hasOption = true;

      return textNode.nodeValue();
   }
}

void Brewtarget::readPersistentOptions()
{
   QFile xmlFile(getConfigDir() + "options.xml");
   optionsDoc = new QDomDocument();
   QDomElement root;
   QString err;
   QString text;
   int line;
   int col;
   bool hasOption;

   // Try to open xmlFile.
   if( ! xmlFile.open(QIODevice::ReadOnly) )
   {
      // Now we know we can't open it.
      log(WARNING, QString("Could not open %1 for reading.").arg(xmlFile.fileName()));
      // Try changing the permissions
      return;
   }

   if( ! optionsDoc->setContent(&xmlFile, false, &err, &line, &col) )
      log(WARNING, QString("Bad document formatting in %1 %2:%3").arg(xmlFile.fileName()).arg(line).arg(col));

   root = optionsDoc->documentElement();

   //================Version Checking========================
   text = getOptionValue(*optionsDoc, "check_version");
   if( text == "true" )
      checkVersion = true;
   else
      checkVersion = false;

   //=====================Last DB Merge Request======================
   text = getOptionValue(*optionsDoc, "last_db_merge_req", &hasOption);
   if( hasOption )
      lastDbMergeRequest = QDateTime::fromString(text, Qt::ISODate);

   //=====================Language====================
   text = getOptionValue(*optionsDoc, "language", &hasOption);
   if( hasOption )
      setLanguage(text);

   //=======================Data Dir===========================
   text = getOptionValue(*optionsDoc, "user_data_dir", &hasOption);
   if( hasOption )
      userDataDir = text;

   //=======================Weight=====================
   text = getOptionValue(*optionsDoc, "weight_unit_system", &hasOption);
   if( hasOption )
   {
      if( text == "Imperial" )
      {
         weightUnitSystem = Imperial;
         weightSystem = UnitSystems::usWeightUnitSystem();
      }
      else if (text == "USCustomary")
      {
         weightUnitSystem = USCustomary;
         weightSystem = UnitSystems::usWeightUnitSystem();
      }
      else
      {
         weightUnitSystem = SI;
         weightSystem = UnitSystems::siWeightUnitSystem();
      }
   }

   //===========================Volume=======================
   text = getOptionValue(*optionsDoc, "volume_unit_system", &hasOption);
   if( hasOption )
   {
      if( text == "Imperial" )
      {
         volumeUnitSystem = Imperial;
         volumeSystem = UnitSystems::imperialVolumeUnitSystem();
      }
      else if (text == "USCustomary")
      {
         volumeUnitSystem = USCustomary;
         volumeSystem = UnitSystems::usVolumeUnitSystem();
      }
      else
      {
         volumeUnitSystem = SI;
         volumeSystem = UnitSystems::siVolumeUnitSystem();
      }
   }

   //=======================Temp======================
   text = getOptionValue(*optionsDoc, "temperature_scale", &hasOption);
   if( hasOption )
   {
      if( text == "Fahrenheit" )
      {
         tempScale = Fahrenheit;
         tempSystem = UnitSystems::fahrenheitTempUnitSystem();
      }
      else
      {
         tempScale = Celsius;
         tempSystem = UnitSystems::celsiusTempUnitSystem();
      }
   }

   //======================Time======================
   // Set the one and only time system.
   timeSystem = UnitSystems::timeUnitSystem();

   //===================IBU===================
   text = getOptionValue(*optionsDoc, "ibu_formula", &hasOption);
   if( hasOption )
   {
      if( text == "tinseth" )
         ibuFormula = TINSETH;
      else if( text == "rager" )
         ibuFormula = RAGER;
      else
      {
         Brewtarget::log(Brewtarget::ERROR, QString("Bad ibu_formula type: %1").arg(text));
      }
   }

   //========================Color======================
   text = getOptionValue(*optionsDoc, "color_formula", &hasOption);
   if( hasOption )
   {
      if( text == "morey" )
         colorFormula = MOREY;
      else if( text == "daniel" )
         colorFormula = DANIEL;
      else if( text == "mosher" )
         colorFormula = MOSHER;
      else
      {
         Brewtarget::log(Brewtarget::ERROR, QString("Bad color_formula type: %1").arg(text));
      }
   }

   //========================Gravity==================
   text = getOptionValue(*optionsDoc, "use_plato", &hasOption);
   if( hasOption )
   {
      if( text == "true" )
         usePlato = true;
      else if( text == "false" )
         usePlato = false;
      else
      {
         Brewtarget::logW(QString("Bad use_plato type: %1").arg(text));
      }
   }

   //=======================Color unit===================
   text = getOptionValue(*optionsDoc, "color_unit", &hasOption);
   if( hasOption )
   {
      if( text == "srm" )
         colorUnit = SRM;
      else if( text == "ebc" )
         colorUnit = EBC;
      else
         Brewtarget::logW(QString("Bad color_unit type: %1").arg(text));
   }

   delete optionsDoc;
   optionsDoc = 0;
   xmlFile.close();
}

void Brewtarget::savePersistentOptions()
{
   QFile xmlFile(getConfigDir() + "options.xml");
   optionsDoc = new QDomDocument();
   QDomElement root;
   QDomNode node, child;
   QString text;

   if( ! xmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate) )
   {
      log(WARNING, QObject::tr("Could not open %1 for writing").arg(xmlFile.fileName()));
      return;
   }

   root = optionsDoc->createElement("options");

   // Version checking.
   node = optionsDoc->createElement("check_version");
   child = optionsDoc->createTextNode( checkVersion ? "true" : "false" );
   node.appendChild(child);
   root.appendChild(node);

   // Last DB merge request.
   node = optionsDoc->createElement("last_db_merge_req");
   child = optionsDoc->createTextNode( lastDbMergeRequest.toString(Qt::ISODate) );
   node.appendChild(child);
   root.appendChild(node);

   // User data dir.
   node = optionsDoc->createElement("user_data_dir");
   child = optionsDoc->createTextNode( userDataDir );
   node.appendChild(child);
   root.appendChild(node);

   // Unit Systems.
   node = optionsDoc->createElement("weight_unit_system");
   child = optionsDoc->createTextNode( unitSystemToString(weightUnitSystem) );
   node.appendChild(child);
   root.appendChild(node);

   node = optionsDoc->createElement("volume_unit_system");
   child = optionsDoc->createTextNode( unitSystemToString(volumeUnitSystem) );
   node.appendChild(child);
   root.appendChild(node);

   node = optionsDoc->createElement("temperature_scale");
   child = optionsDoc->createTextNode( tempScaleToString(tempScale) );
   node.appendChild(child);
   root.appendChild(node);

   // Language
   node = optionsDoc->createElement("language");
   child = optionsDoc->createTextNode(getCurrentLanguage());
   node.appendChild(child);
   root.appendChild(node);

   // IBU formula.
   node = optionsDoc->createElement("ibu_formula");
   switch( ibuFormula )
   {
      case TINSETH:
         text = "tinseth";
         break;
      case RAGER:
         text = "rager";
         break;
      default:
         text = "";
         break;
   }
   child = optionsDoc->createTextNode(text);
   node.appendChild(child);
   root.appendChild(node);

   // Color formula.
   node = optionsDoc->createElement("color_formula");
   switch( colorFormula )
   {
      case MOREY:
         text = "morey";
         break;
      case DANIEL:
         text = "daniel";
         break;
      case MOSHER:
         text = "mosher";
         break;
      default:
         text = "";
         break;
   }
   child = optionsDoc->createTextNode(text);
   node.appendChild(child);
   root.appendChild(node);

   // Gravity.
   node = optionsDoc->createElement("use_plato");
   if( usePlato )
      text = "true";
   else
      text = "false";
   child = optionsDoc->createTextNode(text);
   node.appendChild(child);
   root.appendChild(node);

   // Color unit.
   node = optionsDoc->createElement("color_unit");
   if( colorUnit == SRM )
      text = "srm";
   else
      text = "ebc";
   child = optionsDoc->createTextNode(text);
   node.appendChild(child);
   root.appendChild(node);

   // Add root to document.
   optionsDoc->appendChild(root);

   // Write file.
   QTextStream out(&xmlFile);
   out << optionsDoc->toString();

   xmlFile.close();
   delete optionsDoc;
   optionsDoc = 0;
}

double Brewtarget::weightQStringToSI(QString qstr)
{
   return weightSystem->qstringToSI(qstr);
}

double Brewtarget::volQStringToSI(QString qstr)
{
   return volumeSystem->qstringToSI(qstr);
}

double Brewtarget::tempQStringToSI(QString qstr)
{
   return tempSystem->qstringToSI(qstr);
}

double Brewtarget::colorQStringToSI(QString qstr)
{
   if ( colorUnit == SRM )
      return qstr.toDouble();
   else
      return qstr.toDouble() * 12.7/25.0;
}

double Brewtarget::timeQStringToSI(QString qstr)
{
   return timeSystem->qstringToSI(qstr);
}

bool Brewtarget::hasUnits(QString qstr)
{
   // accepts X.YZ as well as .YZ followed by some unit string
   QRegExp amtUnit("(\\d+(?:\\.\\d+)?|\\.\\d+)\\s*(\\w+)?");
   amtUnit.indexIn(qstr);

   return amtUnit.cap(2).size() > 0;
}

QString Brewtarget::displayOG( double og, unitDisplay displayUnit, bool showUnits)
{
   QString ret;

   // Field settings override defaults
   if ( displayUnit == noUnit ) 
      displayUnit = usePlato ? displayPlato : displaySg;

   QString tmp = "%1";
   if ( showUnits && displayUnit == displayPlato )
      tmp = "%1 %2";

   if( displayUnit == displaySg )
      ret = tmp.arg(og, 0, 'f', 3);
   else // Using Plato...
   {
      if( og >= 1.000 ) // Make sure OG is sane.
         ret = tmp.arg(Algorithms::Instance().SG_20C20C_toPlato(og), 0, 'f', 1);
      else
         ret = tmp.arg(0);
   }

   if( showUnits && displayUnit == displayPlato )
      ret = ret.arg("P");

   return ret;
}

QString Brewtarget::displayOG( BeerXMLElement* element, QObject* object, QString attribute, bool showUnits)
{
   double og;
   unitDisplay displayUnit;

   if ( element->property(attribute.toLatin1().constData()).canConvert(QVariant::Double) )
   {
      displayUnit = (unitDisplay)option(attribute, noUnit, object, UNIT).toInt();
      og = element->property(attribute.toLatin1().constData()).toDouble();
      return displayOG(og, displayUnit, showUnits);
   }

   return "?";
}

QString Brewtarget::displayFG( double fg, double og, unitDisplay displayUnit, bool showUnits  )
{
   QString ret = "%1";

   if ( displayUnit == noUnit )
      displayUnit = usePlato ? displayPlato : displaySg;

   if ( showUnits && displayUnit == displayPlato ) 
      ret = "%1 %2";

   if( displayUnit == displaySg )
      ret = ret.arg(fg, 0, 'f', 3);
   else
   {
      double plato;
      if( og < fg || og < 1.000 || fg < 0.001 )
         plato = 0; // Strange input, so just say 0.
      else
      {
         // The following shows Plato as it would be on a
         // hydrometer.
         //plato = Algorithms::Instance().ogFgToPlato( og, fg );

         // The following shows ACTUAL Plato
         plato = Algorithms::Instance().SG_20C20C_toPlato(fg);
      }
      ret = ret.arg( plato, 0, 'f', 1 );
   }

   if( showUnits && displayUnit == displayPlato)
      ret = ret.arg("P");

   return ret;
}

// Damn. That is pretty. I actually got the bloody displayFG to look right.
QString Brewtarget::displayFG(QPair<QString, BeerXMLElement*> fg, QPair<QString, BeerXMLElement*> og, QObject* object, bool showUnits)
{
   QString fgAttr = fg.first;
   BeerXMLElement* fgElem = fg.second;

   QString ogAttr = og.first;
   BeerXMLElement* ogElem = og.second;
   unitDisplay displayUnit;

   if ( fgElem->property(fgAttr.toLatin1().constData()).canConvert(QVariant::Double) &&
        ogElem->property(ogAttr.toLatin1().constData()).canConvert(QVariant::Double) )
   {
      // Note: it is the setting for the FG attribute that drives what unit is
      //       displayed
      displayUnit = (unitDisplay)option(fgAttr, noUnit, object, UNIT).toInt();
      return displayFG(fgElem->property(fgAttr.toLatin1().constData()).toDouble(),
                       ogElem->property(ogAttr.toLatin1().constData()).toDouble(),
                       displayUnit,
                       showUnits);
   }

   return "?";
}

QString Brewtarget::displayColor( double srm, unitDisplay displayUnit, bool showUnits )
{
   QString ret;

   if ( displayUnit == noUnit || displayUnit == displaySrm )
      ret = showUnits ? QString("%1 %2").arg(srm,0,'f',1).arg(Units::srm->getUnitName()) : QString("%1").arg(srm,0,'f',1);
   else
   {
      double ebc = Units::ebc->fromSI(srm);
      ret = showUnits ? QString("%1 %2").arg(ebc,0,'f',1).arg(Units::ebc->getUnitName()) : QString("%1").arg(ebc,0,'f',1);
   }

   return ret;
}

QString Brewtarget::displayColor( BeerXMLElement* element, QObject* object, QString attribute, bool showUnits )
{
   double srm;
   unitDisplay displayUnit;

   if ( element->property( attribute.toLatin1().constData()).canConvert(QVariant::Double)) 
   {
      srm = element->property(attribute.toLatin1().constData()).toDouble();
      displayUnit = (unitDisplay)option(attribute, noUnit, object, UNIT).toInt();

      return displayColor(srm, displayUnit, showUnits);
   }

   return "?";
}

QString Brewtarget::displayDate(QDate const& date )
{
   QLocale loc(QLocale::system().name());
   return date.toString(loc.dateFormat(QLocale::ShortFormat));
}

bool Brewtarget::hasOption(QString attribute, const QObject* object, iUnitOps ops)
{
   QString name;

   if ( object )
      name = generateName(attribute,object,ops);
   else
      name = attribute;

   return btSettings.contains(name);
}

void Brewtarget::setOption(QString attribute, QVariant value, const QObject* object, iUnitOps ops)
{
   QString name;

   if ( object )
      name = generateName(attribute,object,ops);
   else
      name = attribute;

   btSettings.setValue(name,value);
}

QVariant Brewtarget::option(QString attribute, QVariant default_value, const QObject* object, iUnitOps ops)
{
   QString name;

   if ( object )
      name = generateName(attribute,object,ops);
   else
      name = attribute;

   return btSettings.value(name,default_value);
}

QString Brewtarget::generateName(QString attribute, const QObject* object, iUnitOps ops)
{
   QString ret = QString("%1/%2").arg(object->objectName()).arg(attribute);

   if ( ops != NOOP )
      ret += ops == UNIT ? "_unit" : "_scale";

   return ret;
}

// These are used in at least two places. I hate cut'n'paste coding so I am
// putting them here.
QMenu* Brewtarget::setupColorMenu(QWidget* parent, unitDisplay unit)
{
   QMenu* menu = new QMenu(parent);

   generateAction(menu, tr("Default"), noUnit, unit);
   generateAction(menu, tr("ECB"), displayEcb, unit);
   generateAction(menu, tr("SRM"), displaySrm, unit);

   return menu;
}

QMenu* Brewtarget::setupGravityMenu(QWidget* parent, unitDisplay unit)
{
   QMenu* menu = new QMenu(parent);

   generateAction(menu, tr("Default"), noUnit, unit);
   generateAction(menu, tr("Plato"), displayPlato, unit);
   generateAction(menu, tr("Specific Gravity"), displaySg, unit);

   return menu;
}

QMenu* Brewtarget::setupMassMenu(QWidget* parent, unitDisplay unit, unitScale scale, bool generateScale)
{
   QMenu* menu = new QMenu(parent);
   QMenu* sMenu;

   generateAction(menu, tr("Default"), noUnit, unit);
   generateAction(menu, tr("SI"), displaySI, unit);
   generateAction(menu, tr("US Customary"), displayUS, unit);

   // Some places can't do scale -- like yeast tables and misc tables because
   // they can be mixed. It doesn't stop the unit selection from working, but
   // the scale menus don't make sense
   if ( generateScale == false )
      return menu;

   if ( unit == noUnit )
   {
      switch(Brewtarget::getWeightUnitSystem())
      {
         case USCustomary:
            unit = displayUS;
            break;
         default:
            unit = displaySI;
      }
   }

   sMenu = new QMenu(menu);
   switch(unit)
   {
      case displaySI:
         generateAction(sMenu, tr("Default"), noScale, scale);
         generateAction(sMenu, tr("Milligrams"), extrasmall, scale);
         generateAction(sMenu, tr("Grams"), small, scale);
         generateAction(sMenu, tr("Kilograms"), medium, scale);
         break;
      default:
         generateAction(sMenu, tr("Default"), noScale, scale);
         generateAction(sMenu, tr("Ounces"), extrasmall, scale);
         generateAction(sMenu, tr("Pounds"), small, scale);
         break;
   }
   sMenu->setTitle("Scale");
   menu->addMenu(sMenu);

   return menu;
}

QMenu* Brewtarget::setupTemperatureMenu(QWidget* parent, unitDisplay unit)
{
   QMenu* menu = new QMenu(parent);

   generateAction(menu, tr("Default"), noUnit, unit);
   generateAction(menu, tr("Celsius"), displaySI, unit);
   generateAction(menu, tr("Fahrenheit"), displayUS, unit);

   return menu;
}

QMenu* Brewtarget::setupVolumeMenu(QWidget* parent, unitDisplay unit, unitScale scale, bool generateScale)
{
   QMenu* menu = new QMenu(parent);
   QMenu* sMenu;

   generateAction(menu, tr("Default"), noUnit, unit);
   generateAction(menu, tr("SI"), displaySI, unit);
   generateAction(menu, tr("US Customary"), displayUS, unit);
   generateAction(menu, tr("British Imperial"), displayImp, unit);

   if ( generateScale == false )
      return menu;

   if ( unit == noUnit )
   {
      switch(Brewtarget::getVolumeUnitSystem())
      {
         case USCustomary:
            unit = displayUS;
            break;
         case Imperial:
            unit = displayImp;
            break;
         default:
            unit = displaySI;
      }
   }


   sMenu = new QMenu(menu);
   switch(unit)
   {
      case displaySI:
         generateAction(sMenu, tr("Default"), noScale, scale);
         generateAction(sMenu, tr("MilliLiters"), extrasmall, scale);
         generateAction(sMenu, tr("Liters"), small, scale);
         break;
        // I can cheat because Imperial and US use the same names
      default:
         generateAction(sMenu, tr("Default"), noScale, scale);
         generateAction(sMenu, tr("Teaspoons"), extrasmall, scale);
         generateAction(sMenu, tr("Tablespoons"), small, scale);
         generateAction(sMenu, tr("Cups"), medium, scale);
         generateAction(sMenu, tr("Quarts"), large, scale);
         generateAction(sMenu, tr("Gallons"), extralarge, scale);
         break;
   }
   sMenu->setTitle("Scale");
   menu->addMenu(sMenu);

   return menu;
}

void Brewtarget::generateAction(QMenu* menu, QString text, QVariant data, QVariant currentVal)
{
   QAction* action = new QAction(menu);

   action->setText(text);
   action->setData(data);
   action->setCheckable(true);
   action->setChecked(currentVal == data);;

  menu->addAction(action);
}

MainWindow* Brewtarget::getMainWindow()
{
   return mainWindow;
}

