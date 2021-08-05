/*
 * brewtarget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
 * - Ted Wright <unsure>
 * - Mattias Måhl <mattias@kejsarsten.com>
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
#include "brewtarget.h"

#include <iostream>

#include <QDebug>
#include <QDesktopServices>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QEventLoop>
#include <QFile>
#include <QIODevice>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QObject>
#include <QPixmap>
#include <QSettings>
#include <QSharedPointer>
#include <QSplashScreen>
#include <QString>
#include <QTextStream>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>

#include "Algorithms.h"
#include "BtSplashScreen.h"
#include "config.h"
#include "database/Database.h"
#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"
#include "Unit.h"
#include "UnitSystem.h"

// Needed for kill(2)
#if defined(Q_OS_UNIX)
#include <sys/types.h>
#include <signal.h>
#endif

namespace {
   /**
    * \brief Create a directory if it doesn't exist, popping a error dialog if creation fails
    */
   bool createDir(QDir dir) {
      if( ! dir.mkpath(dir.absolutePath()) ) {
         // Write a message to the log, the usablity check below will alert the user
         QString errText(QObject::tr("Error attempting to create directory \"%1\""));
         qCritical() << errText.arg(dir.path());
      }

      // It's possible that the path exists, but is useless to us
      if( ! dir.exists() || ! dir.isReadable() ) {

         QString errText{QObject::tr("\"%1\" cannot be read.")};
         qWarning() << errText.arg(dir.path());
         if (Brewtarget::isInteractive()) {
            QString errTitle(QObject::tr("Directory Problem"));
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

   /**
    * \brief Ensure our directories exist.
    */
   bool ensureDirectoriesExist() {
      // A missing resource directory is a serious issue, without it we're missing the default DB, sound files &
      // translations.  We could attempt to create it, like the other config/data directories, but an empty resource
      // dir is just as bad as a missing one.  So, instead, we'll display a little more dire warning, and not try to
      // create it.
      QDir resourceDir = Brewtarget::getResourceDir();
      bool resourceDirSuccess = resourceDir.exists();
      if (!resourceDirSuccess) {
         QString errMsg{
            QObject::tr("Resource directory \"%1\" is missing.  Some features will be unavailable.").arg(resourceDir.path())
         };
         qCritical() << Q_FUNC_INFO << errMsg;

         if (Brewtarget::isInteractive()) {
            QMessageBox::critical(
               nullptr,
               QObject::tr("Directory Problem"),
               errMsg
            );
         }
      }

      return resourceDirSuccess &&
             createDir(PersistentSettings::getConfigDir()) &&
             createDir(PersistentSettings::getUserDataDir());
   }

}

MainWindow* Brewtarget::m_mainWindow = nullptr;
QDomDocument* Brewtarget::optionsDoc;
QTranslator* Brewtarget::defaultTrans = new QTranslator();
QTranslator* Brewtarget::btTrans = new QTranslator();
bool Brewtarget::userDatabaseDidNotExist = false;
bool Brewtarget::_isInteractive = true;
QDir Brewtarget::userDataDir = QString();
QString Brewtarget::currentLanguage = "en";

bool Brewtarget::checkVersion = true;

SystemOfMeasurement Brewtarget::weightUnitSystem = SI;
SystemOfMeasurement Brewtarget::volumeUnitSystem = SI;

TempScale Brewtarget::tempScale = Celsius;
Unit::unitDisplay Brewtarget::dateFormat = Unit::displaySI;

Brewtarget::ColorType Brewtarget::colorFormula = Brewtarget::MOREY;
Brewtarget::IbuType Brewtarget::ibuFormula = Brewtarget::TINSETH;
Brewtarget::ColorUnitType Brewtarget::colorUnit = Brewtarget::SRM;
Brewtarget::DensityUnitType Brewtarget::densityUnit = Brewtarget::SG;
Brewtarget::DiastaticPowerUnitType Brewtarget::diastaticPowerUnit = Brewtarget::LINTNER;

QHash<int, UnitSystem const *> Brewtarget::thingToUnitSystem;




// .:TODO:. This needs to be updated to look at github
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
   return QFile::copy(PersistentSettings::getUserDataDir().filePath(dbFileName), newPath.filePath(dbFileName));
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
   QDir translations = QDir (getResourceDir().canonicalPath() + "/translations_qm");

   if( btTrans->load( filename, translations.canonicalPath() ) )
      qApp->installTranslator(btTrans);

}

const QString& Brewtarget::getCurrentLanguage()
{
   return currentLanguage;
}

SystemOfMeasurement Brewtarget::getWeightUnitSystem()
{
   return weightUnitSystem;
}

SystemOfMeasurement Brewtarget::getVolumeUnitSystem()
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
     qInfo() << QString("XDG_CONFIG_HOME directory is %1").arg(xdg_config_home);
     dir.setPath(QString(xdg_config_home).append("/brewtarget"));
   }
   else {
     // If XDG_CONFIG_HOME doesn't exist, config goes in ~/.config/brewtarget
      qInfo() << QString("XDG_CONFIG_HOME not set.  HOME directory is %1").arg(QDir::homePath());
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
   qDebug() << QString("userDataDir=%1").arg(userDataDir.path());
   if (!userDataDir.exists()) {
      qDebug() << QString("User data dir \"%1\" does not exist, trying to create").arg(userDataDir.path());
      createDir(userDataDir);
      qDebug() << "UserDatadit Created";
   }
   return userDataDir;
#else
# error "Unsupported OS"
#endif
}

QDir Brewtarget::getResourceDir() {
   // Unlike some of the other directories, the resource dir needs to be something that can be determined at
   // compile-time
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

   if (!dir.endsWith('/')) {
      dir += "/";
   }

   return dir;
}

bool Brewtarget::initialize()
{
   // Need these for changed(QMetaProperty,QVariant) to be emitted across threads.
   qRegisterMetaType<QMetaProperty>();
   qRegisterMetaType<Equipment*>();
   qRegisterMetaType<Mash*>();
   qRegisterMetaType<Style*>();
   qRegisterMetaType<Salt*>();
   qRegisterMetaType< QList<BrewNote*> >();
   qRegisterMetaType< QList<Hop*> >();
   qRegisterMetaType< QList<Instruction*> >();
   qRegisterMetaType< QList<Fermentable*> >();
   qRegisterMetaType< QList<Misc*> >();
   qRegisterMetaType< QList<Yeast*> >();
   qRegisterMetaType< QList<Water*> >();
   qRegisterMetaType< QList<Salt*> >();

   // Make sure all the necessary directories and files we need exist before starting.
   ensureDirectoriesExist();

   readSystemOptions();

   loadMap();

   loadTranslations(); // Do internationalization.

#if defined(Q_OS_MAC)
   qt_set_sequence_auto_mnemonic(true); // turns on Mac Keyboard shortcuts
#endif

   // Check if the database was successfully loaded before
   // loading the main window.
   qDebug() << "Loading Database...";
   return Database::instance().loadSuccessful();
}

void Brewtarget::cleanup() {
   qDebug() << "Brewtarget is cleaning up.";
   // Should I do qApp->removeTranslator() first?
   delete defaultTrans;
   delete btTrans;
   delete m_mainWindow;

   Database::instance().unload();
   return;
}

bool Brewtarget::isInteractive()
{
   return _isInteractive;
}

void Brewtarget::setInteractive(bool val)
{
   _isInteractive = val;
}

int Brewtarget::run() {
   int ret = 0;

   BtSplashScreen splashScreen;
   splashScreen.show();
   qApp->processEvents();
   if( !initialize() )
   {
      cleanup();
      return 1;
   }
   qInfo() << QString("Starting Brewtarget v%1 on %2.").arg(VERSIONSTRING).arg(QSysInfo::prettyProductName());
   Database::instance().checkForNewDefaultData();
   m_mainWindow = new MainWindow();
   m_mainWindow->init();
   m_mainWindow->setVisible(true);
   splashScreen.finish(m_mainWindow);

   checkForNewVersion(m_mainWindow);
   do {
      ret = qApp->exec();
   } while (ret == 1000);

   cleanup();

   qDebug() << Q_FUNC_INFO << "Cleaned up.  Returning " << ret;

   return ret;
}

void Brewtarget::updateConfig() {
   int cVersion = PersistentSettings::value(PersistentSettings::Names::config_version, QVariant(0)).toInt();
   while ( cVersion < CONFIG_VERSION ) {
      switch ( ++cVersion ) {
         case 1:
            // Update the dbtype, because I had to increase the NODB value from -1 to 0
            int newType = static_cast<Database::DbType>(PersistentSettings::value(PersistentSettings::Names::dbType, Database::NODB).toInt() + 1);
            // Write that back to the config file
            PersistentSettings::insert(PersistentSettings::Names::dbType, static_cast<int>(newType));
            // and make sure we don't do it again.
            PersistentSettings::insert(PersistentSettings::Names::config_version, QVariant(cVersion));
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
   checkVersion = PersistentSettings::value(PersistentSettings::Names::check_version, QVariant(false)).toBool();

   //=====================Last DB Merge Request======================
   if (PersistentSettings::contains(PersistentSettings::Names::last_db_merge_req)) {
      Database::lastDbMergeRequest = QDateTime::fromString(PersistentSettings::value(PersistentSettings::Names::last_db_merge_req,"").toString(), Qt::ISODate);
   }

   //=====================Language====================
   if( PersistentSettings::contains(PersistentSettings::Names::language) ) {
      setLanguage(PersistentSettings::value(PersistentSettings::Names::language,"").toString());
   }

   //=======================Weight=====================
   text = PersistentSettings::value(PersistentSettings::Names::weight_unit_system, "SI").toString();
   if( text == "Imperial" )
   {
      weightUnitSystem = Imperial;
      thingToUnitSystem.insert(Unit::Mass,&UnitSystems::usWeightUnitSystem);
   }
   else if (text == "USCustomary")
   {
      weightUnitSystem = USCustomary;
      thingToUnitSystem.insert(Unit::Mass,&UnitSystems::usWeightUnitSystem);
   }
   else
   {
      weightUnitSystem = SI;
      thingToUnitSystem.insert(Unit::Mass,&UnitSystems::siWeightUnitSystem);
   }

   //===========================Volume=======================
   text = PersistentSettings::value(PersistentSettings::Names::volume_unit_system, "SI").toString();
   if( text == "Imperial" )
   {
      volumeUnitSystem = Imperial;
      thingToUnitSystem.insert(Unit::Volume,&UnitSystems::imperialVolumeUnitSystem);
   }
   else if (text == "USCustomary")
   {
      volumeUnitSystem = USCustomary;
      thingToUnitSystem.insert(Unit::Volume,&UnitSystems::usVolumeUnitSystem);
   }
   else
   {
      volumeUnitSystem = SI;
      thingToUnitSystem.insert(Unit::Volume,&UnitSystems::siVolumeUnitSystem);
   }

   //=======================Temp======================
   text = PersistentSettings::value(PersistentSettings::Names::temperature_scale, "SI").toString();
   if( text == "Fahrenheit" )
   {
      tempScale = Fahrenheit;
      thingToUnitSystem.insert(Unit::Temp,&UnitSystems::fahrenheitTempUnitSystem);
   }
   else
   {
      tempScale = Celsius;
      thingToUnitSystem.insert(Unit::Temp,&UnitSystems::celsiusTempUnitSystem);
   }

   //======================Time======================
   // Set the one and only time system.
   thingToUnitSystem.insert(Unit::Time,&UnitSystems::timeUnitSystem);

   //===================IBU===================
   text = PersistentSettings::value(PersistentSettings::Names::ibu_formula, "tinseth").toString();
   if( text == "tinseth" )
      ibuFormula = TINSETH;
   else if( text == "rager" )
      ibuFormula = RAGER;
   else if( text == "noonan" )
       ibuFormula = NOONAN;
   else
   {
      qCritical() << QString("Bad ibu_formula type: %1").arg(text);
   }

   //========================Color Formula======================
   text = PersistentSettings::value(PersistentSettings::Names::color_formula, "morey").toString();
   if( text == "morey" )
      colorFormula = MOREY;
   else if( text == "daniel" )
      colorFormula = DANIEL;
   else if( text == "mosher" )
      colorFormula = MOSHER;
   else
   {
      qCritical() << QString("Bad color_formula type: %1").arg(text);
   }

   //========================Density==================

   if ( PersistentSettings::value(PersistentSettings::Names::use_plato, false).toBool() )
   {
      densityUnit = PLATO;
      thingToUnitSystem.insert(Unit::Density,&UnitSystems::platoDensityUnitSystem);
   }
   else
   {
      densityUnit = SG;
      thingToUnitSystem.insert(Unit::Density,&UnitSystems::sgDensityUnitSystem);
   }

   //=======================Color unit===================
   text = PersistentSettings::value(PersistentSettings::Names::color_unit, "srm").toString();
   if( text == "srm" )
   {
      colorUnit = SRM;
      thingToUnitSystem.insert(Unit::Color,&UnitSystems::srmColorUnitSystem);
   }
   else if( text == "ebc" )
   {
      colorUnit = EBC;
      thingToUnitSystem.insert(Unit::Color,&UnitSystems::ebcColorUnitSystem);
   }
   else
      qWarning() << QString("Bad color_unit type: %1").arg(text);

   //=======================Diastatic power unit===================
   text = PersistentSettings::value(PersistentSettings::Names::diastatic_power_unit, "Lintner").toString();
   if( text == "Lintner" )
   {
      diastaticPowerUnit = LINTNER;
      thingToUnitSystem.insert(Unit::DiastaticPower,&UnitSystems::lintnerDiastaticPowerUnitSystem);
   }
   else if( text == "WK" )
   {
      diastaticPowerUnit = WK;
      thingToUnitSystem.insert(Unit::DiastaticPower,&UnitSystems::wkDiastaticPowerUnitSystem);
   }
   else
   {
      qWarning() << QString("Bad diastatic_power_unit type: %1").arg(text);
   }

   //=======================Date format===================
   dateFormat = static_cast<Unit::unitDisplay>(PersistentSettings::value(PersistentSettings::Names::date_format,Unit::displaySI).toInt());

   return;

}

void Brewtarget::saveSystemOptions() {
   QString text;

   PersistentSettings::insert(PersistentSettings::Names::check_version, checkVersion);
   PersistentSettings::insert(PersistentSettings::Names::last_db_merge_req, Database::lastDbMergeRequest.toString(Qt::ISODate));
   PersistentSettings::insert(PersistentSettings::Names::language, getCurrentLanguage());
   //setOption("user_data_dir", userDataDir);
   PersistentSettings::insert(PersistentSettings::Names::weight_unit_system, thingToUnitSystem.value(Unit::Mass)->unitType());
   PersistentSettings::insert(PersistentSettings::Names::volume_unit_system, thingToUnitSystem.value(Unit::Volume)->unitType());
   PersistentSettings::insert(PersistentSettings::Names::temperature_scale, thingToUnitSystem.value(Unit::Temp)->unitType());
   PersistentSettings::insert(PersistentSettings::Names::use_plato, densityUnit == PLATO);
   PersistentSettings::insert(PersistentSettings::Names::date_format, dateFormat);

   switch(ibuFormula)
   {
      case TINSETH:
         PersistentSettings::insert(PersistentSettings::Names::ibu_formula, "tinseth");
         break;
      case RAGER:
         PersistentSettings::insert(PersistentSettings::Names::ibu_formula, "rager");
         break;
      case NOONAN:
         PersistentSettings::insert(PersistentSettings::Names::ibu_formula, "noonan");
         break;
   }

   switch(colorFormula)
   {
      case MOREY:
         PersistentSettings::insert(PersistentSettings::Names::color_formula, "morey");
         break;
      case DANIEL:
         PersistentSettings::insert(PersistentSettings::Names::color_formula, "daniel");
         break;
      case MOSHER:
         PersistentSettings::insert(PersistentSettings::Names::color_formula, "mosher");
         break;
   }

   switch(colorUnit)
   {
      case SRM:
         PersistentSettings::insert(PersistentSettings::Names::color_unit, "srm");
         break;
      case EBC:
         PersistentSettings::insert(PersistentSettings::Names::color_unit, "ebc");
         break;
   }

   switch(diastaticPowerUnit)
   {
      case LINTNER:
         PersistentSettings::insert(PersistentSettings::Names::diastatic_power_unit, "Lintner");
         break;
      case WK:
         PersistentSettings::insert(PersistentSettings::Names::diastatic_power_unit, "WK");
         break;
   }

   return;
}

// the defaults come from readSystemOptions. This just fleshes out the hash
// for later use.
void Brewtarget::loadMap()
{
   // ==== mass ====
   thingToUnitSystem.insert(Unit::Mass | Unit::displaySI, &UnitSystems::siWeightUnitSystem );
   thingToUnitSystem.insert(Unit::Mass | Unit::displayUS, &UnitSystems::usWeightUnitSystem );
   thingToUnitSystem.insert(Unit::Mass | Unit::displayImp,&UnitSystems::usWeightUnitSystem );

   // ==== volume ====
   thingToUnitSystem.insert(Unit::Volume | Unit::displaySI, &UnitSystems::siVolumeUnitSystem );
   thingToUnitSystem.insert(Unit::Volume | Unit::displayUS, &UnitSystems::usVolumeUnitSystem );
   thingToUnitSystem.insert(Unit::Volume | Unit::displayImp,&UnitSystems::imperialVolumeUnitSystem );

   // ==== time is empty ==== (this zen moment was free)

   // ==== temp ====
   thingToUnitSystem.insert(Unit::Temp | Unit::displaySI,&UnitSystems::celsiusTempUnitSystem );
   thingToUnitSystem.insert(Unit::Temp | Unit::displayUS,&UnitSystems::fahrenheitTempUnitSystem );

   // ==== color ====
   thingToUnitSystem.insert(Unit::Color | Unit::displaySrm,&UnitSystems::srmColorUnitSystem );
   thingToUnitSystem.insert(Unit::Color | Unit::displayEbc,&UnitSystems::ebcColorUnitSystem );

   // ==== density ====
   thingToUnitSystem.insert(Unit::Density | Unit::displaySg,   &UnitSystems::sgDensityUnitSystem );
   thingToUnitSystem.insert(Unit::Density | Unit::displayPlato,&UnitSystems::platoDensityUnitSystem );

   // ==== diastatic power ====
   thingToUnitSystem.insert(Unit::DiastaticPower | Unit::displayLintner,&UnitSystems::lintnerDiastaticPowerUnitSystem );
   thingToUnitSystem.insert(Unit::DiastaticPower | Unit::displayWK,&UnitSystems::wkDiastaticPowerUnitSystem );
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
   if ( ! success ) {
      ret = text.toDouble(&success);
   }

   // If we were asked to return the success, return it here.
   if ( ok != nullptr ) {
      *ok = success;
   }

   // Whatever we got, we return it
   return ret;
}

// And a few convenience methods, just for that sweet, sweet syntatic sugar
double Brewtarget::toDouble(const NamedEntity* element, BtStringConst const & propertyName, QString caller) {
   double amount = 0.0;
   QString value;
   bool ok = false;

   if ( element->property(*propertyName).canConvert(QVariant::String) ) {
      // Get the amount
      value = element->property(*propertyName).toString();
      amount = toDouble( value, &ok );
      if (!ok) {
         qWarning() << Q_FUNC_INFO << caller << "could not convert" << value << "to double";
      }
      // Get the display units and scale
   }
   return amount;
}

double Brewtarget::toDouble(QString text, QString caller) {
   bool success = false;

   double ret = toDouble(text,&success);

   if ( ! success ) {
      qWarning() << Q_FUNC_INFO << caller << "could not convert" << text << "to double";
   }

   return ret;
}


// Displays "amount" of units "units" in the proper format.
// If "units" is null, just return the amount.
QString Brewtarget::displayAmount( double amount, Unit const * units, int precision, Unit::unitDisplay displayUnits, Unit::unitScale displayScale)
{
   int fieldWidth = 0;
   char format = 'f';

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
   UnitSystem const * temp = findUnitSystem(units, displayUnits);
   // If we cannot find a unit system
   if ( temp == nullptr )
      ret = QString("%L1 %2").arg(SIAmount, fieldWidth, format, precision).arg(SIUnitName);
   else
      ret = temp->displayAmount( amount, units, precision, displayScale );

   return ret;
}

QString Brewtarget::displayAmount(NamedEntity* element, QObject* object, BtStringConst const & propertyName, Unit const * units, int precision )
{
   double amount = 0.0;
   QString value;
   bool ok = false;
   Unit::unitScale dispScale;
   Unit::unitDisplay dispUnit;

   if ( element->property(*propertyName).canConvert(QVariant::Double) )
   {
      // Get the amount
      value = element->property(*propertyName).toString();
      amount = toDouble( value, &ok );
      if ( ! ok )
         qWarning() << QString("%1 could not convert %2 to double")
               .arg(Q_FUNC_INFO)
               .arg(value);
      // Get the display units and scale
      dispUnit  = static_cast<Unit::unitDisplay>(PersistentSettings::value(propertyName, Unit::noUnit,  object->objectName(), PersistentSettings::UNIT).toInt());
      dispScale = static_cast<Unit::unitScale>(PersistentSettings::value(  propertyName, Unit::noScale, object->objectName(), PersistentSettings::SCALE).toInt());

      return displayAmount(amount, units, precision, dispUnit, dispScale);
   }
   else
      return "?";

}

QString Brewtarget::displayAmount(double amt,
                               BtStringConst const & section,
                               BtStringConst const & propertyName,
                               Unit const * units,
                               int precision) {
   Unit::unitScale dispScale;
   Unit::unitDisplay dispUnit;

   // Get the display units and scale
   dispUnit  = static_cast<Unit::unitDisplay>(PersistentSettings::value(propertyName, Unit::noUnit,  section, PersistentSettings::UNIT).toInt());
   dispScale = static_cast<Unit::unitScale>(PersistentSettings::value(  propertyName, Unit::noScale, section, PersistentSettings::SCALE).toInt());

   return displayAmount(amt, units, precision, dispUnit, dispScale);

}

double Brewtarget::amountDisplay( double amount, Unit const * units, int precision, Unit::unitDisplay displayUnits, Unit::unitScale displayScale) {

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
   UnitSystem const * temp = findUnitSystem(units, displayUnits);
   // If we cannot find a unit system
   if ( temp == nullptr )
      ret = SIAmount;
   else
      ret = temp->amountDisplay( amount, units, displayScale );

   return ret;
}

double Brewtarget::amountDisplay(NamedEntity* element, QObject* object, BtStringConst const & propertyName, Unit const * units, int precision ) {
   double amount = 0.0;
   QString value;
   bool ok = false;
   Unit::unitScale dispScale;
   Unit::unitDisplay dispUnit;

   if ( element->property(*propertyName).canConvert(QVariant::Double) ) {
      // Get the amount
      value = element->property(*propertyName).toString();
      amount = toDouble( value, &ok );
      if ( ! ok ) {
         qWarning() << Q_FUNC_INFO << "Could not convert" << value << "to double";
      }
      // Get the display units and scale
      dispUnit  = static_cast<Unit::unitDisplay>(PersistentSettings::value(propertyName, Unit::noUnit,  object->objectName(), PersistentSettings::UNIT).toInt());
      dispScale = static_cast<Unit::unitScale>(PersistentSettings::value(  propertyName, Unit::noScale, object->objectName(), PersistentSettings::SCALE).toInt());

      return amountDisplay(amount, units, precision, dispUnit, dispScale);
   }
   else
      return -1.0;
}

UnitSystem const * Brewtarget::findUnitSystem(Unit const * unit, Unit::unitDisplay display) {
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

void Brewtarget::getThicknessUnits( Unit const ** volumeUnit, Unit const ** weightUnit )
{
   *volumeUnit = thingToUnitSystem.value(Unit::Volume | Unit::displayDef)->thicknessUnit();
   *weightUnit = thingToUnitSystem.value(Unit::Mass   | Unit::displayDef)->thicknessUnit();
}

QString Brewtarget::displayThickness( double thick_lkg, bool showUnits )
{
   int fieldWidth = 0;
   char format = 'f';
   int precision = 2;

   Unit const * volUnit    = thingToUnitSystem.value(Unit::Volume | Unit::displayDef)->thicknessUnit();
   Unit const * weightUnit = thingToUnitSystem.value(Unit::Mass   | Unit::displayDef)->thicknessUnit();

   double num = volUnit->fromSI(thick_lkg);
   double den = weightUnit->fromSI(1.0);

   if( showUnits )
      return QString("%L1 %2/%3").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
   else
      return QString("%L1").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
}

double Brewtarget::qStringToSI(QString qstr, Unit const * unit, Unit::unitDisplay dispUnit, Unit::unitScale dispScale)
{
   UnitSystem const * temp = findUnitSystem(unit, dispUnit);
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

QPair<double,double> Brewtarget::displayRange(NamedEntity* element,
                                           QObject *object,
                                           BtStringConst const & propertyNameMin,
                                           BtStringConst const & propertyNameMax,
                                           RangeType _type) {
   QPair<double,double> range;

   if ( ! element ) {
      range.first  = 0.0;
      range.second = 100.0;
   }
   else if ( _type != DENSITY )
   {
      range.first  = amountDisplay(element, object, PropertyNames::Style::colorMin_srm, &Units::srm,0);
      range.second = amountDisplay(element, object, PropertyNames::Style::colorMax_srm, &Units::srm,0);
   }
   else
   {
      range.first  = amountDisplay(element, object, propertyNameMin, &Units::sp_grav,0);
      range.second = amountDisplay(element, object, propertyNameMax, &Units::sp_grav,0);
   }

   return range;
}

QPair<double,double> Brewtarget::displayRange(QObject *object,
                                           BtStringConst const & propertyName,
                                           double min,
                                           double max,
                                           RangeType _type)
{
   QPair<double,double> range;
   Unit::unitDisplay displayUnit;

   displayUnit = static_cast<Unit::unitDisplay>(PersistentSettings::value(propertyName,
                                                                          Unit::noUnit,
                                                                          object->objectName(),
                                                                          PersistentSettings::UNIT).toInt());

   if ( _type == DENSITY )
   {
      range.first  = amountDisplay(min, &Units::sp_grav, 0, displayUnit );
      range.second = amountDisplay(max, &Units::sp_grav, 0, displayUnit );
   }
   else
   {
      range.first  = amountDisplay(min, &Units::srm, 0, displayUnit );
      range.second = amountDisplay(max, &Units::srm, 0, displayUnit );
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
      if ( thingToUnitSystem.value(Unit::Mass) == &UnitSystems::usWeightUnitSystem )
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
      if ( thingToUnitSystem.value(Unit::Volume) == &UnitSystems::usVolumeUnitSystem )
         unit = Unit::displayUS;
      else if ( thingToUnitSystem.value(Unit::Volume) == &UnitSystems::imperialVolumeUnitSystem )
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
   return m_mainWindow;
}
