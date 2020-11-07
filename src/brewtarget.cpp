/*
 * brewtarget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
 * - Ted Wright <unsure>
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
#include "unit.h"
#include "USWeightUnitSystem.h"
#include "USVolumeUnitSystem.h"
#include "FahrenheitTempUnitSystem.h"
#include "TimeUnitSystem.h"
#include "SIWeightUnitSystem.h"
#include "SIVolumeUnitSystem.h"
#include "CelsiusTempUnitSystem.h"
#include "ImperialVolumeUnitSystem.h"

#include "EbcColorUnitSystem.h"
#include "SrmColorUnitSystem.h"
#include "SgDensityUnitSystem.h"
#include "PlatoDensityUnitSystem.h"
#include "DiastaticPowerUnitSystem.h"

#include "BtSplashScreen.h"
#include "MainWindow.h"
#include "mash.h"
#include "instruction.h"
#include "water.h"
#include "salt.h"

// Needed for kill(2)
#if defined(Q_OS_UNIX)
#include <sys/types.h>
#include <signal.h>
#endif

// These HAVE to be in the same order as they are listed in
// Brewtarget::DBTable
QStringList Brewtarget::dbTableToName  = QStringList() <<
   QString("none") <<  // need to handle the NOTABLE index
   ktableSettings <<
   ktableEquipment <<
   ktableFermentable <<
   ktableHop <<
   ktableMisc <<
   ktableStyle <<
   ktableYeast <<
   ktableWater <<
   ktableMash <<
   ktableMashStep <<
   ktableRecipe <<
   ktableBrewnote <<
   ktableInstruction <<
   ktableSalt <<
// Now for BT internal tables
   ktableBtEquipment <<
   ktableBtFermentable <<
   ktableBtHop <<
   ktableBtMisc <<
   ktableBtStyle <<
   ktableBtYeast <<
   ktableBtWater <<
// Now the in_recipe tables
   ktableFermInRec <<
   ktableHopInRec <<
   ktableMiscInRec <<
   ktableWaterInRec <<
   ktableYeastInRec <<
   ktableInsInRec <<
   ktableSaltInRec <<
// child tables next
   ktableEquipChildren <<
   ktableFermChildren <<
   ktableHopChildren <<
   ktableMiscChildren <<
   ktableRecChildren <<
   ktableStyleChildren <<
   ktableWaterChildren <<
   ktableYeastChildren <<
// inventory tables last
   ktableFermInventory <<
   ktableHopInventory <<
   ktableMiscInventory <<
   ktableYeastInventory;

MainWindow* Brewtarget::_mainWindow = nullptr;
QDomDocument* Brewtarget::optionsDoc;
QTranslator* Brewtarget::defaultTrans = new QTranslator();
QTranslator* Brewtarget::btTrans = new QTranslator();
bool Brewtarget::userDatabaseDidNotExist = false;
bool Brewtarget::_isInteractive = true;
QDateTime Brewtarget::lastDbMergeRequest = QDateTime::fromString("1986-02-24T06:00:00", Qt::ISODate);

QString Brewtarget::currentLanguage = "en";
QDir Brewtarget::userDataDir = QString();
Brewtarget::DBTypes Brewtarget::_dbType = Brewtarget::NODB;

bool Brewtarget::checkVersion = true;
Log Brewtarget::log(true);

iUnitSystem Brewtarget::weightUnitSystem = SI;
iUnitSystem Brewtarget::volumeUnitSystem = SI;

TempScale Brewtarget::tempScale = Celsius;
Unit::unitDisplay Brewtarget::dateFormat = Unit::displaySI;

Brewtarget::ColorType Brewtarget::colorFormula = Brewtarget::MOREY;
Brewtarget::IbuType Brewtarget::ibuFormula = Brewtarget::TINSETH;
Brewtarget::ColorUnitType Brewtarget::colorUnit = Brewtarget::SRM;
Brewtarget::DensityUnitType Brewtarget::densityUnit = Brewtarget::SG;
Brewtarget::DiastaticPowerUnitType Brewtarget::diastaticPowerUnit = Brewtarget::LINTNER;

QHash<int, UnitSystem*> Brewtarget::thingToUnitSystem;


bool Brewtarget::createDir(QDir dir, QString errText)
{
  if( ! dir.mkpath(dir.absolutePath()) )
  {
    // Write a message to the log, the usablity check below will alert the user
    QString errText(QObject::tr("Error attempting to create directory \"%1\""));
    logE(errText.arg(dir.path()));
  }

  // It's possible that the path exists, but is useless to us
  if( ! dir.exists() || ! dir.isReadable() )
  {
    QString errTitle(QObject::tr("Directory Problem"));

    if( errText == nullptr)
      errText = QString(QObject::tr("\"%1\" cannot be read."));

    logW(errText.arg(dir.path()));

    if (Brewtarget::isInteractive()) {
       QMessageBox::information(
          nullptr,
          errTitle,
          errText.arg(dir.path())
       );
    }
    return false;
  }

  return true;
}

bool Brewtarget::ensureDirectoriesExist()
{
  // A missing dataDir is a serious issue, without it we're missing the default DB, sound files & translations.
  // An attempt could be made to created it, like the other config directories, but an empty data dir is just as bad as a missing one.
  // Because of that, we'll display a little more dire warning, and not try to create it.
  QDir dataDir = getUserDataDir();
  bool dataDirSuccess = true;

  if (! dataDir.exists())
  {
    dataDirSuccess = false;
    QString errMsg = QString(QObject::tr("Data directory \"%1\" is missing.  Some features will be unavaliable.")).arg(dataDir.path());
    logE(errMsg);

    if (Brewtarget::isInteractive()) {
       QMessageBox::critical(
          nullptr,
          QObject::tr("Directory Problem"),
          errMsg
       );
    }
  }


  return
    dataDirSuccess &&
    createDir(getConfigDir()) &&
    createDir(getDocDir()) &&
    createDir(getUserDataDir());
}

void Brewtarget::checkForNewVersion(MainWindow* mw)
{

   // Don't do anything if the checkVersion flag was set false
   if ( checkVersion == false )
      return;

   QNetworkAccessManager manager;
   QUrl url("http://brewtarget.sourceforge.net/version");
   QNetworkReply* reply = manager.get( QNetworkRequest(url) );
   QObject::connect( reply, &QNetworkReply::finished, mw, &MainWindow::finishCheckingVersion );
}

bool Brewtarget::copyDataFiles(const QDir newPath)
{
   QString dbFileName = "database.sqlite";
   return QFile::copy(getUserDataDir().filePath(dbFileName), newPath.filePath(dbFileName));
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
   if( qApp == nullptr )
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
   QDir translations = QDir (getDataDir().canonicalPath() + "/translations_qm");

   if( btTrans->load( filename, translations.canonicalPath() ) )
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

Unit::unitDisplay Brewtarget::getColorUnit()
{
   if ( colorUnit == Brewtarget::SRM )
      return Unit::displaySrm;

   return Unit::displayEbc;
}

Unit::unitDisplay Brewtarget::getDiastaticPowerUnit()
{
   if ( diastaticPowerUnit == Brewtarget::LINTNER )
      return Unit::displayLintner;

   return Unit::displayWK;
}

Unit::unitDisplay Brewtarget::getDateFormat()
{
   return dateFormat;
}

Unit::unitDisplay Brewtarget::getDensityUnit()
{
   if ( densityUnit == Brewtarget::SG )
      return Unit::displaySg;

   return Unit::displayPlato;
}

TempScale Brewtarget::getTemperatureScale()
{
   return tempScale;
}

QDir Brewtarget::getDataDir()
{
   QString dir = qApp->applicationDirPath();
#if defined(Q_OS_LINUX) // Linux OS.

   dir = QString(CONFIGDATADIR);

#elif defined(Q_OS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/";

#elif defined(Q_OS_WIN) // Windows OS.

   dir += "/../data/";

#else
# error "Unsupported OS"
#endif

   if( ! dir.endsWith('/') )
      dir += "/";

   return dir;
}

QDir Brewtarget::getDocDir()
{
   QString dir = qApp->applicationDirPath();
#if defined(Q_OS_LINUX) // Linux OS.

   dir = QString(CONFIGDOCDIR);

#elif defined(Q_OS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/en.lproj/";

#elif defined(Q_OS_WIN) // Windows OS.

   dir += "/../doc/";

#else
# error "Unsupported OS"
#endif

   if( ! dir.endsWith('/') )
      dir += "/";

   return dir;
}

const QDir Brewtarget::getConfigDir()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) // Linux OS or Mac OS.
   QDir dir;
   QFileInfo fileInfo;

   // First, try XDG_CONFIG_HOME.
   // If that variable doesn't exist, create ~/.config
   char* xdg_config_home = getenv("XDG_CONFIG_HOME");

   if (xdg_config_home) {
     logI(QString("XDG_CONFIG_HOME directory is %1").arg(xdg_config_home));
     dir.setPath(QString(xdg_config_home).append("/brewtarget"));
   }
   else {
     // If XDG_CONFIG_HOME doesn't exist, config goes in ~/.config/brewtarget
     logI(QString("XDG_CONFIG_HOME not set.  HOME directory is %1").arg(QDir::homePath()));
     QString dirPath = QDir::homePath().append("/.config/brewtarget");
     dir = QDir(dirPath);
   }

   return dir.absolutePath() + "/";

#elif defined(Q_OS_WIN) // Windows OS.

   QDir dir;
   // This is the bin/ directory.
   dir = QDir(QCoreApplication::applicationDirPath());
   dir.cdUp();
   // Now we should be in the base directory (i.e. Brewtarget-2.0.0/)

   dir.cd("data");
   return dir.absolutePath() + "/";

#else
# error "Unsupported OS"
#endif

}

QDir Brewtarget::getUserDataDir()
{
   return userDataDir;
}

QDir Brewtarget::getDefaultUserDataDir()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) // Linux OS or Mac OS.#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) // Linux OS or Mac OS.
   return getConfigDir();
#elif defined(Q_OS_WIN) // Windows OS.
   // On Windows the Programs directory is normally not writable so we need to get the appData path from the environment instead.
   userDataDir.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
   logD(QString("userDataDir=%1").arg(userDataDir.path()));
   if (!userDataDir.exists()) {
      logD(QString("User data dir \"%1\" does not exist, trying to create").arg(userDataDir.path()));
      createDir(userDataDir);
      logD("UserDatadit Created");
   }
   return userDataDir;
#else
# error "Unsupported OS"
#endif
}

bool Brewtarget::initialize(const QString &userDirectory)
{
   // Need these for changed(QMetaProperty,QVariant) to be emitted across threads.
   qRegisterMetaType<QMetaProperty>();
   qRegisterMetaType<Equipment*>();
   qRegisterMetaType<Mash*>();
   qRegisterMetaType<Style*>();
   qRegisterMetaType<Salt*>();
   qRegisterMetaType<Brewtarget::DBTable>();
   qRegisterMetaType< QList<BrewNote*> >();
   qRegisterMetaType< QList<Hop*> >();
   qRegisterMetaType< QList<Instruction*> >();
   qRegisterMetaType< QList<Fermentable*> >();
   qRegisterMetaType< QList<Misc*> >();
   qRegisterMetaType< QList<Yeast*> >();
   qRegisterMetaType< QList<Water*> >();
   qRegisterMetaType< QList<Salt*> >();


   // Use overwride if present.
   if (!userDirectory.isEmpty() && QDir(userDirectory).exists()) {
      userDataDir.setPath(QDir(userDirectory).canonicalPath());
   }
   // Use directory from app settings.
   else if (hasOption("user_data_dir") && QDir(option("user_data_dir","").toString()).exists()) {
      userDataDir.setPath( QDir(option("user_data_dir","").toString()).canonicalPath());

   }
   // Guess where to put it.
   else {
      logW(QString("User data directory not specified or doesn't exist - using default."));
      userDataDir = getDefaultUserDataDir();
   }

   // If the old options file exists, convert it. Otherwise, just get the
   // system options. I *think* this will work. The installer copies the old
   // one into the new place on Windows.
   if ( option("hadOldConfig", false).toBool() )
      convertPersistentOptions();

   readSystemOptions();
   loadMap();
   log.changeDirectory();

   // Make sure all the necessary directories and files we need exist before starting.
   ensureDirectoriesExist();

   // If the directory doesn't exist, canonicalPath() will return an empty
   // string. By waiting until after we know the directory is created, we
   // make sure it isn't empty
   if ( ! hasOption("user_data_dir") ) {
      setOption("user_data_dir", userDataDir.canonicalPath());
   }

   loadTranslations(); // Do internationalization.

#if defined(Q_OS_MAC)
   qt_set_sequence_auto_mnemonic(true); // turns on Mac Keyboard shortcuts
#endif

   // Check if the database was successfully loaded before
   // loading the main window.
   logD("Loading Database...");
   if (Database::instance().loadSuccessful())
   {
      if ( ! QSettings().contains("converted") )
         Database::instance().convertFromXml();

      return true;
   }
   else
      return false;
}

Brewtarget::DBTypes Brewtarget::dbType()
{
   if ( _dbType == Brewtarget::NODB )
      _dbType = static_cast<Brewtarget::DBTypes>(option("dbType", Brewtarget::SQLITE).toInt());
   return _dbType;
}

QString Brewtarget::dbTrue(Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes whichDb = type;
   QString retval;

   if ( whichDb == Brewtarget::NODB )
      whichDb = dbType();

   switch( whichDb ) {
      case SQLITE:
         retval = "1";
         break;
      case PGSQL:
         retval = "true";
         break;
      default:
         retval = "whiskeytangofoxtrot";
   }
   return retval;
}

QString Brewtarget::dbFalse(Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes whichDb = type;
   QString retval;

   if ( whichDb == Brewtarget::NODB )
      whichDb = dbType();

   switch( whichDb ) {
      case SQLITE:
         retval = "0";
         break;
      case PGSQL:
         retval = "false";
         break;
      default:
         retval = "notwhiskeytangofoxtrot";
   }
   return retval;
}

QString Brewtarget::dbBoolean(bool flag, Brewtarget::DBTypes type)
{
   Brewtarget::DBTypes whichDb = type;
   QString retval;

   if ( whichDb == Brewtarget::NODB )
      whichDb = dbType();

   switch( whichDb ) {
      case SQLITE:
         retval = flag ? QString("1") : QString("0");
         break;
      case PGSQL:
         retval = flag ? QString("true") : QString("false");
         break;
      default:
         retval = "notwhiskeytangofoxtrot";
   }
   return retval;
}

void Brewtarget::cleanup()
{
   log.info("Brewtarget is cleaning up.");
   // Should I do qApp->removeTranslator() first?
   delete defaultTrans;
   delete btTrans;
   delete _mainWindow;

   Database::dropInstance();

}

bool Brewtarget::isInteractive()
{
   return _isInteractive;
}

void Brewtarget::setInteractive(bool val)
{
   _isInteractive = val;
}

int Brewtarget::run(const QString &userDirectory)
{
   int ret = 0;

   BtSplashScreen splashScreen;
   splashScreen.show();
   qApp->processEvents();
   if( !initialize(userDirectory) )
   {
      cleanup();
      return 1;
   }
   log.info(QString("Starting Brewtarget v%1 on %2.").arg(VERSIONSTRING).arg(QSysInfo::prettyProductName()));
   _mainWindow = new MainWindow();
   _mainWindow->setVisible(true);
   splashScreen.finish(_mainWindow);
   QObject::connect( &log, &Log::wroteEntry, _mainWindow, &MainWindow::updateStatus );

   checkForNewVersion(_mainWindow);
   do {
      ret = qApp->exec();
   } while (ret == 1000);

   cleanup();

   return ret;
}

// Read the old options.xml file one more time, then move it out of the way.
void Brewtarget::convertPersistentOptions()
{
   QDir cfgDir = QDir(getConfigDir());
   QFile xmlFile(getConfigDir().filePath("options.xml"));
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
      logW(QString("Could not open %1 for reading.").arg(xmlFile.fileName()));
      // Try changing the permissions
      return;
   }

   if( ! optionsDoc->setContent(&xmlFile, false, &err, &line, &col) )
      logW(QString("Bad document formatting in %1 %2:%3").arg(xmlFile.fileName()).arg(line).arg(col));

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

   //=======================Weight=====================
   text = getOptionValue(*optionsDoc, "weight_unit_system", &hasOption);
   if( hasOption )
   {
      if( text == "Imperial" )
      {
         weightUnitSystem = Imperial;
         thingToUnitSystem.insert(Unit::Mass,UnitSystems::usWeightUnitSystem());
      }
      else if (text == "USCustomary")
      {
         weightUnitSystem = USCustomary;
         thingToUnitSystem.insert(Unit::Mass,UnitSystems::usWeightUnitSystem());
      }
      else
      {
         weightUnitSystem = SI;
         thingToUnitSystem.insert(Unit::Mass,UnitSystems::siWeightUnitSystem());
      }
   }

   //===========================Volume=======================
   text = getOptionValue(*optionsDoc, "volume_unit_system", &hasOption);
   if( hasOption )
   {
      if( text == "Imperial" )
      {
         volumeUnitSystem = Imperial;
         thingToUnitSystem.insert(Unit::Volume,UnitSystems::imperialVolumeUnitSystem());
      }
      else if (text == "USCustomary")
      {
         volumeUnitSystem = USCustomary;
         thingToUnitSystem.insert(Unit::Volume,UnitSystems::usVolumeUnitSystem());
      }
      else
      {
         volumeUnitSystem = SI;
         thingToUnitSystem.insert(Unit::Volume,UnitSystems::siVolumeUnitSystem());
      }
   }

   //=======================Temp======================
   text = getOptionValue(*optionsDoc, "temperature_scale", &hasOption);
   if( hasOption )
   {
      if( text == "Fahrenheit" )
      {
         tempScale = Fahrenheit;
         thingToUnitSystem.insert(Unit::Temp,UnitSystems::fahrenheitTempUnitSystem());
      }
      else
      {
         tempScale = Celsius;
         thingToUnitSystem.insert(Unit::Temp,UnitSystems::celsiusTempUnitSystem());
      }
   }

   //======================Time======================
   // Set the one and only time system.
   thingToUnitSystem.insert(Unit::Time,UnitSystems::timeUnitSystem());

   //===================IBU===================
   text = getOptionValue(*optionsDoc, "ibu_formula", &hasOption);
   if( hasOption )
   {
      if( text == "tinseth" )
         ibuFormula = TINSETH;
      else if( text == "rager" )
         ibuFormula = RAGER;
      else if( text == "noonan")
         ibuFormula = NOONAN;
      else
      {
         Brewtarget::logE(QString("Bad ibu_formula type: %1").arg(text));
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
         Brewtarget::logE(QString("Bad color_formula type: %1").arg(text));
      }
   }

   //========================Density==================
   text = getOptionValue(*optionsDoc, "use_plato", &hasOption);
   if( hasOption )
   {
      if( text == "true" )
      {
         densityUnit = PLATO;
         thingToUnitSystem.insert(Unit::Density,UnitSystems::platoDensityUnitSystem());
      }
      else if( text == "false" )
      {
         densityUnit = SG;
         thingToUnitSystem.insert(Unit::Density,UnitSystems::sgDensityUnitSystem());
      }
      else
      {
         logW(QString("Bad use_plato type: %1").arg(text));
      }
   }

   //=======================Color unit===================
   text = getOptionValue(*optionsDoc, "color_unit", &hasOption);
   if( hasOption )
   {
      if( text == "srm" )
      {
         colorUnit = SRM;
         thingToUnitSystem.insert(Unit::Color,UnitSystems::srmColorUnitSystem());
      }
      else if( text == "ebc" )
      {
         colorUnit = EBC;
         thingToUnitSystem.insert(Unit::Color,UnitSystems::ebcColorUnitSystem());
      }
      else
         logW(QString("Bad color_unit type: %1").arg(text));
   }

   //=======================Diastatic power unit===================
   text = getOptionValue(*optionsDoc, "diastatic_power_unit", &hasOption);
   if( hasOption )
   {
      if( text == "Lintner" )
      {
         diastaticPowerUnit = LINTNER;
         thingToUnitSystem.insert(Unit::DiastaticPower,UnitSystems::lintnerDiastaticPowerUnitSystem());
      }
      else if( text == "WK" )
      {
         diastaticPowerUnit = WK;
         thingToUnitSystem.insert(Unit::DiastaticPower,UnitSystems::wkDiastaticPowerUnitSystem());
      }
      else
      {
         logW(QString("Bad diastatic_power_unit type: %1").arg(text));
      }
   }

   delete optionsDoc;
   optionsDoc = nullptr;
   xmlFile.close();

   // Don't do this on Windows. We have extra work to do and creating the
   // obsolete directory mess it all up. Not sure why that test is still in here
#ifndef Q_OS_WIN
   // This shouldn't really happen, but lets be sure
   if( !cfgDir.exists("obsolete") )
      cfgDir.mkdir("obsolete");

   // copy the old file into obsolete and delete it
   cfgDir.cd("obsolete");
   if( xmlFile.copy(cfgDir.filePath("options.xml")) )
      xmlFile.remove();

#endif
   // And remove the flag
   QSettings().remove("hadOldConfig");
}

QString Brewtarget::getOptionValue(const QDomDocument& optionsDoc, const QString& option, bool* hasOption)
{
   QDomNode node, child;
   QDomText textNode;
   QDomNodeList list;

   list = optionsDoc.elementsByTagName(option);
   if(list.length() <= 0) {
      logW(QString("Could not find the <%1> tag in the option file.").arg(option));
      if( hasOption != nullptr )
         *hasOption = false;
      return "";
   }
   else {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();

      if( hasOption != nullptr )
         *hasOption = true;

      return textNode.nodeValue();
   }
}

void Brewtarget::updateConfig()
{
   int cVersion = option("config_version", QVariant(0)).toInt();
   while ( cVersion < CONFIG_VERSION ) {
      switch ( ++cVersion ) {
         case 1:
            // Update the dbtype, because I had to increase the NODB value from -1 to 0
            int newType = static_cast<Brewtarget::DBTypes>(option("dbType",Brewtarget::NODB).toInt() + 1);
            // Write that back to the config file
            setOption("dbType", static_cast<int>(newType));
            // and make sure we don't do it again.
            setOption("config_version", QVariant(cVersion));
            break;
      }
   }
}

void Brewtarget::readSystemOptions()
{
   QString text;

   // update the config file before we do anything
   updateConfig();

   //================Version Checking========================
   checkVersion = option("check_version", QVariant(false)).toBool();

   //=====================Last DB Merge Request======================
   if( hasOption("last_db_merge_req"))
      lastDbMergeRequest = QDateTime::fromString(option("last_db_merge_req","").toString(), Qt::ISODate);

   //=====================Language====================
   if( hasOption("language") )
      setLanguage(option("language","").toString());

   //=======================Weight=====================
   text = option("weight_unit_system", "SI").toString();
   if( text == "Imperial" )
   {
      weightUnitSystem = Imperial;
      thingToUnitSystem.insert(Unit::Mass,UnitSystems::usWeightUnitSystem());
   }
   else if (text == "USCustomary")
   {
      weightUnitSystem = USCustomary;
      thingToUnitSystem.insert(Unit::Mass,UnitSystems::usWeightUnitSystem());
   }
   else
   {
      weightUnitSystem = SI;
      thingToUnitSystem.insert(Unit::Mass,UnitSystems::siWeightUnitSystem());
   }

   //===========================Volume=======================
   text = option("volume_unit_system", "SI").toString();
   if( text == "Imperial" )
   {
      volumeUnitSystem = Imperial;
      thingToUnitSystem.insert(Unit::Volume,UnitSystems::imperialVolumeUnitSystem());
   }
   else if (text == "USCustomary")
   {
      volumeUnitSystem = USCustomary;
      thingToUnitSystem.insert(Unit::Volume,UnitSystems::usVolumeUnitSystem());
   }
   else
   {
      volumeUnitSystem = SI;
      thingToUnitSystem.insert(Unit::Volume,UnitSystems::siVolumeUnitSystem());
   }

   //=======================Temp======================
   text = option("temperature_scale", "SI").toString();
   if( text == "Fahrenheit" )
   {
      tempScale = Fahrenheit;
      thingToUnitSystem.insert(Unit::Temp,UnitSystems::fahrenheitTempUnitSystem());
   }
   else
   {
      tempScale = Celsius;
      thingToUnitSystem.insert(Unit::Temp,UnitSystems::celsiusTempUnitSystem());
   }

   //======================Time======================
   // Set the one and only time system.
   thingToUnitSystem.insert(Unit::Time,UnitSystems::timeUnitSystem());

   //===================IBU===================
   text = option("ibu_formula", "tinseth").toString();
   if( text == "tinseth" )
      ibuFormula = TINSETH;
   else if( text == "rager" )
      ibuFormula = RAGER;
   else if( text == "noonan" )
       ibuFormula = NOONAN;
   else
   {
      Brewtarget::logE(QString("Bad ibu_formula type: %1").arg(text));
   }

   //========================Color Formula======================
   text = option("color_formula", "morey").toString();
   if( text == "morey" )
      colorFormula = MOREY;
   else if( text == "daniel" )
      colorFormula = DANIEL;
   else if( text == "mosher" )
      colorFormula = MOSHER;
   else
   {
      Brewtarget::logE(QString("Bad color_formula type: %1").arg(text));
   }

   //========================Density==================

   if ( option("use_plato", false).toBool() )
   {
      densityUnit = PLATO;
      thingToUnitSystem.insert(Unit::Density,UnitSystems::platoDensityUnitSystem());
   }
   else
   {
      densityUnit = SG;
      thingToUnitSystem.insert(Unit::Density,UnitSystems::sgDensityUnitSystem());
   }

   //=======================Color unit===================
   text = option("color_unit", "srm").toString();
   if( text == "srm" )
   {
      colorUnit = SRM;
      thingToUnitSystem.insert(Unit::Color,UnitSystems::srmColorUnitSystem());
   }
   else if( text == "ebc" )
   {
      colorUnit = EBC;
      thingToUnitSystem.insert(Unit::Color,UnitSystems::ebcColorUnitSystem());
   }
   else
      logW(QString("Bad color_unit type: %1").arg(text));

   //=======================Diastatic power unit===================
   text = option("diastatic_power_unit", "Lintner").toString();
   if( text == "Lintner" )
   {
      diastaticPowerUnit = LINTNER;
      thingToUnitSystem.insert(Unit::DiastaticPower,UnitSystems::lintnerDiastaticPowerUnitSystem());
   }
   else if( text == "WK" )
   {
      diastaticPowerUnit = WK;
      thingToUnitSystem.insert(Unit::DiastaticPower,UnitSystems::wkDiastaticPowerUnitSystem());
   }
   else
   {
      logW(QString("Bad diastatic_power_unit type: %1").arg(text));
   }

   //=======================Date format===================
   dateFormat = static_cast<Unit::unitDisplay>(option("date_format",Unit::displaySI).toInt());

   //=======================Database type ================
   _dbType = static_cast<Brewtarget::DBTypes>(option("dbType",Brewtarget::SQLITE).toInt());

   //======================Logging options =======================
   log.LoggingEnabled = option("LoggingEnabled", false).toBool();
   log.LoggingLevel = log.getLogTypeFromString(QString(option("LoggingLevel", "INFO").toString()));
   log.LogFilePath = QDir(option("LogFilePath", getUserDataDir().canonicalPath()).toString());
   log.LoggingUseConfigDir = option("LoggingUseConfigDir", true).toBool();
   if( log.LoggingUseConfigDir )
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
      log.LogFilePath = getUserDataDir().canonicalPath();
#else
      log.LogFilePath.setPath(getUserDataDir().canonicalPath());
#endif
}

void Brewtarget::saveSystemOptions()
{
   QString text;

   setOption("check_version", checkVersion);
   setOption("last_db_merge_req", lastDbMergeRequest.toString(Qt::ISODate));
   setOption("language", getCurrentLanguage());
   //setOption("user_data_dir", userDataDir);
   setOption("weight_unit_system", thingToUnitSystem.value(Unit::Mass)->unitType());
   setOption("volume_unit_system",thingToUnitSystem.value(Unit::Volume)->unitType());
   setOption("temperature_scale", thingToUnitSystem.value(Unit::Temp)->unitType());
   setOption("use_plato", densityUnit == PLATO);
   setOption("date_format", dateFormat);

   switch(ibuFormula)
   {
      case TINSETH:
         setOption("ibu_formula", "tinseth");
         break;
      case RAGER:
         setOption("ibu_formula", "rager");
         break;
      case NOONAN:
         setOption("ibu_formula", "noonan");
         break;
   }

   switch(colorFormula)
   {
      case MOREY:
         setOption("color_formula", "morey");
         break;
      case DANIEL:
         setOption("color_formula", "daniel");
         break;
      case MOSHER:
         setOption("color_formula", "mosher");
         break;
   }

   switch(colorUnit)
   {
      case SRM:
         setOption("color_unit", "srm");
         break;
      case EBC:
         setOption("color_unit", "ebc");
         break;
   }

   switch(diastaticPowerUnit)
   {
      case LINTNER:
         setOption("diastatic_power_unit", "Lintner");
         break;
      case WK:
         setOption("diastatic_power_unit", "WK");
         break;
   }

   setOption("LoggingEnabled", log.LoggingEnabled);
   setOption("LoggingLevel", log.getOptionStringFromLogType(log.LoggingLevel));
   setOption("LogFilePath", log.LogFilePath.absolutePath());
   setOption("LoggingUseConfigDir", log.LoggingUseConfigDir);
}

// the defaults come from readSystemOptions. This just fleshes out the hash
// for later use.
void Brewtarget::loadMap()
{
   // ==== mass ====
   thingToUnitSystem.insert(Unit::Mass | Unit::displaySI, UnitSystems::siWeightUnitSystem() );
   thingToUnitSystem.insert(Unit::Mass | Unit::displayUS, UnitSystems::usWeightUnitSystem() );
   thingToUnitSystem.insert(Unit::Mass | Unit::displayImp,UnitSystems::usWeightUnitSystem() );

   // ==== volume ====
   thingToUnitSystem.insert(Unit::Volume | Unit::displaySI, UnitSystems::siVolumeUnitSystem() );
   thingToUnitSystem.insert(Unit::Volume | Unit::displayUS, UnitSystems::usVolumeUnitSystem() );
   thingToUnitSystem.insert(Unit::Volume | Unit::displayImp,UnitSystems::imperialVolumeUnitSystem() );

   // ==== time is empty ==== (this zen moment was free)

   // ==== temp ====
   thingToUnitSystem.insert(Unit::Temp | Unit::displaySI,UnitSystems::celsiusTempUnitSystem() );
   thingToUnitSystem.insert(Unit::Temp | Unit::displayUS,UnitSystems::fahrenheitTempUnitSystem() );

   // ==== color ====
   thingToUnitSystem.insert(Unit::Color | Unit::displaySrm,UnitSystems::srmColorUnitSystem() );
   thingToUnitSystem.insert(Unit::Color | Unit::displayEbc,UnitSystems::ebcColorUnitSystem() );

   // ==== density ====
   thingToUnitSystem.insert(Unit::Density | Unit::displaySg,   UnitSystems::sgDensityUnitSystem() );
   thingToUnitSystem.insert(Unit::Density | Unit::displayPlato,UnitSystems::platoDensityUnitSystem() );

   // ==== diastatic power ====
   thingToUnitSystem.insert(Unit::DiastaticPower | Unit::displayLintner,UnitSystems::lintnerDiastaticPowerUnitSystem() );
   thingToUnitSystem.insert(Unit::DiastaticPower | Unit::displayWK,UnitSystems::wkDiastaticPowerUnitSystem() );
}

void Brewtarget::logE( QString message )
{
   if ( log.LoggingEnabled && log.LoggingLevel <= Log::LogType_ERROR)
      log.error(message);
}

void Brewtarget::logW( QString message )
{
   if ( log.LoggingEnabled && log.LoggingLevel <= Log::LogType_WARNING)
      log.warn(message);
}

void Brewtarget::logI( QString message )
{
   if ( log.LoggingEnabled && log.LoggingLevel <= Log::LogType_INFO)
      log.info(message);
}

void Brewtarget::logD( QString message )
{
   if ( log.LoggingEnabled && log.LoggingLevel <= Log::LogType_DEBUG)
      log.debug(message);
}

/* Qt5 changed how QString::toDouble() works in that it will always convert
   in the C locale. We are instructed to use QLocale::toDouble instead, except
   that will never fall back to the C locale. This doesn't really work for us,
   so I am writing a convenience function that emulates the old behavior.
*/
double Brewtarget::toDouble(QString text, bool* ok)
{
   double ret = 0.0;
   bool success = false;
   QLocale sysDefault = QLocale();

   ret = sysDefault.toDouble(text,&success);

   // If we failed, try C conversion
   if ( ! success )
      ret = text.toDouble(&success);

   // If we were asked to return the success, return it here.
   if ( ok != nullptr )
      *ok = success;

   // Whatever we got, we return it
   return ret;
}

// And a few convenience methods, just for that sweet, sweet syntatic sugar
double Brewtarget::toDouble(const Ingredient* element, QString attribute, QString caller)
{
   double amount = 0.0;
   QString value;
   bool ok = false;

   if ( element->property(attribute.toLatin1().constData()).canConvert(QVariant::String) )
   {
      // Get the amount
      value = element->property(attribute.toLatin1().constData()).toString();
      amount = toDouble( value, &ok );
      if ( ! ok )
         logW( QString("%1 could not convert %2 to double").arg(caller).arg(value));
      // Get the display units and scale
   }
   return amount;
}

double Brewtarget::toDouble(QString text, QString caller)
{
   double ret = 0.0;
   bool success = false;

   ret = toDouble(text,&success);

   if ( ! success )
      logW( QString("%1 could not convert %2 to double").arg(caller).arg(text));

   return ret;
}


// Displays "amount" of units "units" in the proper format.
// If "units" is null, just return the amount.
QString Brewtarget::displayAmount( double amount, Unit* units, int precision, Unit::unitDisplay displayUnits, Unit::unitScale displayScale)
{
   int fieldWidth = 0;
   char format = 'f';
   UnitSystem* temp;

   // Check for insane values.
   if( Algorithms::isNan(amount) || Algorithms::isInf(amount) )
      return "-";

   // Special case.
   if( units == nullptr )
      return QString("%L1").arg(amount, fieldWidth, format, precision);

   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   QString ret;

   // convert to the current unit system (s).
   temp = findUnitSystem(units, displayUnits);
   // If we cannot find a unit system
   if ( temp == nullptr )
      ret = QString("%L1 %2").arg(SIAmount, fieldWidth, format, precision).arg(SIUnitName);
   else
      ret = temp->displayAmount( amount, units, precision, displayScale );

   return ret;
}

QString Brewtarget::displayAmount(Ingredient* element, QObject* object, QString attribute, Unit* units, int precision )
{
   double amount = 0.0;
   QString value;
   bool ok = false;
   Unit::unitScale dispScale;
   Unit::unitDisplay dispUnit;

   if ( element->property(attribute.toLatin1().constData()).canConvert(QVariant::Double) )
   {
      // Get the amount
      value = element->property(attribute.toLatin1().constData()).toString();
      amount = toDouble( value, &ok );
      if ( ! ok )
         logW( QString("%1 could not convert %2 to double")
               .arg(Q_FUNC_INFO)
               .arg(value));
      // Get the display units and scale
      dispUnit  = static_cast<Unit::unitDisplay>(option(attribute, Unit::noUnit,  object->objectName(), UNIT).toInt());
      dispScale = static_cast<Unit::unitScale>(option(  attribute, Unit::noScale, object->objectName(), SCALE).toInt());

      return displayAmount(amount, units, precision, dispUnit, dispScale);
   }
   else
      return "?";

}

QString Brewtarget::displayAmount(double amt, QString section, QString attribute, Unit* units, int precision )
{
   Unit::unitScale dispScale;
   Unit::unitDisplay dispUnit;

   // Get the display units and scale
   dispUnit  = static_cast<Unit::unitDisplay>(Brewtarget::option(attribute, Unit::noUnit,  section, UNIT).toInt());
   dispScale = static_cast<Unit::unitScale>(Brewtarget::option(  attribute, Unit::noScale, section, SCALE).toInt());

   return displayAmount(amt, units, precision, dispUnit, dispScale);

}

double Brewtarget::amountDisplay( double amount, Unit* units, int precision, Unit::unitDisplay displayUnits, Unit::unitScale displayScale)
{
   UnitSystem* temp;

   // Check for insane values.
   if( Algorithms::isNan(amount) || Algorithms::isInf(amount) )
      return -1.0;

   // Special case.
   if( units == nullptr )
      return amount;

   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   double ret;

   // convert to the current unit system (s).
   temp = findUnitSystem(units, displayUnits);
   // If we cannot find a unit system
   if ( temp == nullptr )
      ret = SIAmount;
   else
      ret = temp->amountDisplay( amount, units, displayScale );

   return ret;
}

double Brewtarget::amountDisplay(Ingredient* element, QObject* object, QString attribute, Unit* units, int precision )
{
   double amount = 0.0;
   QString value;
   bool ok = false;
   Unit::unitScale dispScale;
   Unit::unitDisplay dispUnit;

   if ( element->property(attribute.toLatin1().constData()).canConvert(QVariant::Double) )
   {
      // Get the amount
      value = element->property(attribute.toLatin1().constData()).toString();
      amount = toDouble( value, &ok );
      if ( ! ok )
         logW( QString("Brewtarget::amountDisplay(Ingredient*,QObject*,QString,Unit*,int) could not convert %1 to double").arg(value));
      // Get the display units and scale
      dispUnit  = static_cast<Unit::unitDisplay>(option(attribute, Unit::noUnit,  object->objectName(), UNIT).toInt());
      dispScale = static_cast<Unit::unitScale>(option(  attribute, Unit::noScale, object->objectName(), SCALE).toInt());

      return amountDisplay(amount, units, precision, dispUnit, dispScale);
   }
   else
      return -1.0;
}

UnitSystem* Brewtarget::findUnitSystem(Unit* unit, Unit::unitDisplay display)
{
   if ( ! unit )
      return nullptr;

   int key = unit->getUnitType();

   // noUnit means get the default UnitSystem. Through little planning on my
   // part, it happens that is equivalent to just the unitType
   if ( display != Unit::noUnit )
      key |= display;

   if ( thingToUnitSystem.contains( key ) )
      return thingToUnitSystem.value(key);

   return nullptr;
}

void Brewtarget::getThicknessUnits( Unit** volumeUnit, Unit** weightUnit )
{
   *volumeUnit = thingToUnitSystem.value(Unit::Volume | Unit::displayDef)->thicknessUnit();
   *weightUnit = thingToUnitSystem.value(Unit::Mass   | Unit::displayDef)->thicknessUnit();
}

QString Brewtarget::displayThickness( double thick_lkg, bool showUnits )
{
   int fieldWidth = 0;
   char format = 'f';
   int precision = 2;

   Unit* volUnit    = thingToUnitSystem.value(Unit::Volume | Unit::displayDef)->thicknessUnit();
   Unit* weightUnit = thingToUnitSystem.value(Unit::Mass   | Unit::displayDef)->thicknessUnit();

   double num = volUnit->fromSI(thick_lkg);
   double den = weightUnit->fromSI(1.0);

   if( showUnits )
      return QString("%L1 %2/%3").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
   else
      return QString("%L1").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
}

double Brewtarget::qStringToSI(QString qstr, Unit* unit, Unit::unitDisplay dispUnit, Unit::unitScale dispScale)
{
   UnitSystem* temp = findUnitSystem(unit, dispUnit);
   return temp->qstringToSI(qstr,temp->unit(),false,dispScale);
}

QString Brewtarget::ibuFormulaName()
{
   switch ( ibuFormula )
   {
      case Brewtarget::TINSETH:
         return "Tinseth";
      case Brewtarget::RAGER:
         return "Rager";
      case Brewtarget::NOONAN:
         return "Noonan";
   }
  return tr("Unknown");
}

QString Brewtarget::colorFormulaName()
{

   switch( Brewtarget::colorFormula )
   {
      case Brewtarget::MOREY:
         return "Morey";
      case Brewtarget::DANIEL:
         return "Daniels";
      case Brewtarget::MOSHER:
         return "Mosher";
   }
   return tr("Unknown");
}

QString Brewtarget::colorUnitName(Unit::unitDisplay display)
{
   if ( display == Unit::noUnit )
      display = getColorUnit();

   if ( display == Unit::displaySrm )
      return QString("SRM");
   else
      return QString("EBC");
}

QString Brewtarget::diastaticPowerUnitName(Unit::unitDisplay display)
{
   if ( display == Unit::noUnit )
      display = getDiastaticPowerUnit();

   if ( display == Unit::displayLintner )
      return QString("Lintner");
   else
      return QString("WK");
}

bool Brewtarget::hasUnits(QString qstr)
{
   // accepts X,XXX.YZ (or X.XXX,YZ for EU users) as well as .YZ (or ,YZ) followed by
   // some unit string
   QString decimal = QRegExp::escape( QLocale::system().decimalPoint());
   QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

   QRegExp amtUnit("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
   amtUnit.indexIn(qstr);

   return amtUnit.cap(2).size() > 0;
}

QPair<double,double> Brewtarget::displayRange(Ingredient* element, QObject *object, QString attribute, RangeType _type)
{
   QPair<double,double> range;
   QString minName = QString("%1%2").arg(attribute).arg("Min");
   QString maxName = QString("%1%2").arg(attribute).arg("Max");

   if ( ! element ) {
      range.first  = 0.0;
      range.second = 100.0;
   }
   else if ( _type != DENSITY )
   {
      range.first  = amountDisplay(element, object, "colorMin_srm", Units::srm,0);
      range.second = amountDisplay(element, object, "colorMax_srm", Units::srm,0);
   }
   else
   {
      range.first  = amountDisplay(element, object, minName, Units::sp_grav,0);
      range.second = amountDisplay(element, object, maxName, Units::sp_grav,0);
   }

   return range;
}

QPair<double,double> Brewtarget::displayRange(QObject *object, QString attribute, double min, double max, RangeType _type)
{
   QPair<double,double> range;
   Unit::unitDisplay displayUnit;

   displayUnit = static_cast<Unit::unitDisplay>(option(attribute, Unit::noUnit, object->objectName(), UNIT).toInt());

   if ( _type == DENSITY )
   {
      range.first  = amountDisplay(min, Units::sp_grav, 0, displayUnit );
      range.second = amountDisplay(max, Units::sp_grav, 0, displayUnit );
   }
   else
   {
      range.first  = amountDisplay(min, Units::srm, 0, displayUnit );
      range.second = amountDisplay(max, Units::srm, 0, displayUnit );
   }

   return range;
}

QString Brewtarget::displayDate(QDate const& date )
{
   QLocale loc(QLocale::system().name());
   return date.toString(loc.dateFormat(QLocale::ShortFormat));
}

QString Brewtarget::displayDateUserFormated(QDate const &date) {
   QString format;
   switch (Brewtarget::getDateFormat()) {
      case Unit::displayUS:
         format = "MM-dd-yyyy";
         break;
      case Unit::displayImp:
         format = "dd-MM-yyyy";
         break;
      default:
      case Unit::displaySI:
         format = "yyyy-MM-dd";
   }
   return date.toString(format);
}

bool Brewtarget::hasOption(QString attribute, const QString section, iUnitOps ops)
{
   QString name;

   if ( section.isNull() )
      name = attribute;
   else
      name = generateName(attribute,section,ops);

   return QSettings().contains(name);
}

void Brewtarget::setOption(QString attribute, QVariant value, const QString section, iUnitOps ops)
{
   QString name;

   if ( section.isNull() )
      name = attribute;
   else
      name = generateName(attribute,section,ops);

   QSettings().setValue(name,value);
}

QVariant Brewtarget::option(QString attribute, QVariant default_value, QString section, iUnitOps ops)
{
   QString name;

   if ( section.isNull() )
      name = attribute;
   else
      name = generateName(attribute,section,ops);

   return QSettings().value(name,default_value);
}

void Brewtarget::removeOption(QString attribute, QString section)
{
   QString name;

   if ( section.isNull() )
      name = attribute;
   else
      name = generateName(attribute,section,NOOP);

   if ( hasOption(name) )
        QSettings().remove(name);
}

QString Brewtarget::generateName(QString attribute, const QString section, iUnitOps ops)
{
   QString ret = QString("%1/%2").arg(section).arg(attribute);

   if ( ops != NOOP )
      ret += ops == UNIT ? "_unit" : "_scale";

   return ret;
}

// These are used in at least two places. I hate cut'n'paste coding so I am
// putting them here.
// I use a QActionGroup to make sure only one button is ever selected at once.
// It allows me to cache the menus later and speeds the response time up.
QMenu* Brewtarget::setupColorMenu(QWidget* parent, Unit::unitDisplay unit)
{
   QMenu* menu = new QMenu(parent);
   QActionGroup* qgrp = new QActionGroup(parent);

   generateAction(menu, tr("Default"), Unit::noUnit, unit, qgrp);
   generateAction(menu, tr("EBC"), Unit::displayEbc, unit, qgrp);
   generateAction(menu, tr("SRM"), Unit::displaySrm, unit, qgrp);

   return menu;
}

QMenu* Brewtarget::setupDateMenu(QWidget* parent, Unit::unitDisplay unit)
{
   QMenu* menu = new QMenu(parent);
   QActionGroup* qgrp = new QActionGroup(parent);

   generateAction(menu, tr("Default"),    Unit::noUnit,     unit, qgrp);
   generateAction(menu, tr("YYYY-mm-dd"), Unit::displaySI,  unit, qgrp);
   generateAction(menu, tr("dd-mm-YYYY"), Unit::displayImp, unit, qgrp);
   generateAction(menu, tr("mm-dd-YYYY"), Unit::displayUS,  unit, qgrp);

   return menu;
}

QMenu* Brewtarget::setupDensityMenu(QWidget* parent, Unit::unitDisplay unit)
{
   QMenu* menu = new QMenu(parent);
   QActionGroup* qgrp = new QActionGroup(parent);

   generateAction(menu, tr("Default"), Unit::noUnit, unit, qgrp);
   generateAction(menu, tr("Plato"), Unit::displayPlato, unit, qgrp);
   generateAction(menu, tr("Specific Gravity"), Unit::displaySg, unit, qgrp);

   return menu;
}

QMenu* Brewtarget::setupMassMenu(QWidget* parent, Unit::unitDisplay unit, Unit::unitScale scale, bool generateScale)
{
   QMenu* menu = new QMenu(parent);
   QMenu* sMenu;
   QActionGroup* qgrp = new QActionGroup(parent);

   generateAction(menu, tr("Default"), Unit::noUnit, unit, qgrp);
   generateAction(menu, tr("SI"), Unit::displaySI, unit, qgrp);
   generateAction(menu, tr("US Customary"), Unit::displayUS, unit, qgrp);

   // Some places can't do scale -- like yeast tables and misc tables because
   // they can be mixed. It doesn't stop the unit selection from working, but
   // the scale menus don't make sense
   if ( generateScale == false )
      return menu;

   if ( unit == Unit::noUnit )
   {
      if ( thingToUnitSystem.value(Unit::Mass) == UnitSystems::usWeightUnitSystem() )
         unit = Unit::displayUS;
      else
         unit = Unit::displaySI;
   }

   sMenu = new QMenu(menu);
   QActionGroup* qsgrp = new QActionGroup(menu);
   switch(unit)
   {
      case Unit::displaySI:
         generateAction(sMenu, tr("Default"), Unit::noScale, scale,qsgrp);
         generateAction(sMenu, tr("Milligrams"), Unit::scaleExtraSmall, scale,qsgrp);
         generateAction(sMenu, tr("Grams"), Unit::scaleSmall, scale,qsgrp);
         generateAction(sMenu, tr("Kilograms"), Unit::scaleMedium, scale,qsgrp);
         break;
      default:
         generateAction(sMenu, tr("Default"), Unit::noScale, scale,qsgrp);
         generateAction(sMenu, tr("Ounces"), Unit::scaleExtraSmall, scale,qsgrp);
         generateAction(sMenu, tr("Pounds"), Unit::scaleSmall, scale,qsgrp);
         break;
   }
   sMenu->setTitle(tr("Scale"));
   menu->addMenu(sMenu);

   return menu;
}

QMenu* Brewtarget::setupTemperatureMenu(QWidget* parent, Unit::unitDisplay unit)
{
   QMenu* menu = new QMenu(parent);
   QActionGroup* qgrp = new QActionGroup(parent);

   generateAction(menu, tr("Default"), Unit::noUnit, unit, qgrp);
   generateAction(menu, tr("Celsius"), Unit::displaySI, unit, qgrp);
   generateAction(menu, tr("Fahrenheit"), Unit::displayUS, unit, qgrp);

   return menu;
}

// Time menus only have scale
QMenu* Brewtarget::setupTimeMenu(QWidget* parent, Unit::unitScale scale)
{
   QMenu* menu = new QMenu(parent);
   QMenu* sMenu = new QMenu(menu);
   QActionGroup* qgrp = new QActionGroup(parent);

   generateAction(sMenu, tr("Default"), Unit::noScale, scale, qgrp);
   generateAction(sMenu, tr("Seconds"), Unit::scaleExtraSmall, scale, qgrp);
   generateAction(sMenu, tr("Minutes"), Unit::scaleSmall, scale, qgrp);
   generateAction(sMenu, tr("Hours"),   Unit::scaleMedium, scale, qgrp);
   generateAction(sMenu, tr("Days"),    Unit::scaleLarge, scale, qgrp);

   sMenu->setTitle(tr("Scale"));
   menu->addMenu(sMenu);

   return menu;
}

QMenu* Brewtarget::setupVolumeMenu(QWidget* parent, Unit::unitDisplay unit, Unit::unitScale scale, bool generateScale)
{
   QMenu* menu = new QMenu(parent);
   QActionGroup* qgrp = new QActionGroup(parent);
   QMenu* sMenu;

   generateAction(menu, tr("Default"), Unit::noUnit, unit, qgrp);
   generateAction(menu, tr("SI"), Unit::displaySI, unit, qgrp);
   generateAction(menu, tr("US Customary"), Unit::displayUS, unit, qgrp);
   generateAction(menu, tr("British Imperial"), Unit::displayImp, unit, qgrp);

   if ( generateScale == false )
      return menu;

   if ( unit == Unit::noUnit )
   {
      if ( thingToUnitSystem.value(Unit::Volume) == UnitSystems::usVolumeUnitSystem() )
         unit = Unit::displayUS;
      else if ( thingToUnitSystem.value(Unit::Volume) == UnitSystems::imperialVolumeUnitSystem() )
         unit = Unit::displayImp;
      else
         unit = Unit::displaySI;
   }


   sMenu = new QMenu(menu);
   QActionGroup* qsgrp = new QActionGroup(menu);
   switch(unit)
   {
      case Unit::displaySI:
         generateAction(sMenu, tr("Default"), Unit::noScale, scale,qsgrp);
         generateAction(sMenu, tr("MilliLiters"), Unit::scaleExtraSmall, scale,qsgrp);
         generateAction(sMenu, tr("Liters"), Unit::scaleSmall, scale,qsgrp);
         break;
        // I can cheat because Imperial and US use the same names
      default:
         generateAction(sMenu, tr("Default"), Unit::noScale, scale,qsgrp);
         generateAction(sMenu, tr("Teaspoons"), Unit::scaleExtraSmall, scale,qsgrp);
         generateAction(sMenu, tr("Tablespoons"), Unit::scaleSmall, scale,qsgrp);
         generateAction(sMenu, tr("Cups"), Unit::scaleMedium, scale,qsgrp);
         generateAction(sMenu, tr("Quarts"), Unit::scaleLarge, scale,qsgrp);
         generateAction(sMenu, tr("Gallons"), Unit::scaleExtraLarge, scale,qsgrp);
         generateAction(sMenu, tr("Barrels"), Unit::scaleHuge, scale,qsgrp);
         break;
   }
   sMenu->setTitle(tr("Scale"));
   menu->addMenu(sMenu);

   return menu;
}

QMenu* Brewtarget::setupDiastaticPowerMenu(QWidget* parent, Unit::unitDisplay unit)
{
   QMenu* menu = new QMenu(parent);
   QActionGroup* qgrp = new QActionGroup(parent);

   generateAction(menu, tr("Default"), Unit::noUnit, unit, qgrp);
   generateAction(menu, tr("WK"), Unit::displayWK, unit, qgrp);
   generateAction(menu, tr("Lintner"), Unit::displayLintner, unit, qgrp);

   return menu;
}

void Brewtarget::generateAction(QMenu* menu, QString text, QVariant data, QVariant currentVal, QActionGroup* qgrp)
{
   QAction* action = new QAction(menu);

   action->setText(text);
   action->setData(data);
   action->setCheckable(true);
   action->setChecked(currentVal == data);;
   if ( qgrp )
      qgrp->addAction(action);

  menu->addAction(action);
}

MainWindow* Brewtarget::mainWindow()
{
   return _mainWindow;
}
