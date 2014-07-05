/*
 * brewtarget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

MainWindow* Brewtarget::_mainWindow = 0;
QDomDocument* Brewtarget::optionsDoc;
QTranslator* Brewtarget::defaultTrans = new QTranslator();
QTranslator* Brewtarget::btTrans = new QTranslator();
QTextStream* Brewtarget::logStream = 0;
QFile* Brewtarget::logFile = 0;
bool Brewtarget::userDatabaseDidNotExist = false;
QFile Brewtarget::pidFile;
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

   QString errTitle(QObject::tr("Directory Problem"));
   QString errText(QObject::tr("\"%1\" cannot be read."));
   
   // Check data dir
   dir.setPath(getDataDir());
   if( ! dir.exists() || ! dir.isReadable() )
   {
      QMessageBox::information(
         0,
         errTitle,
         errText.arg(dir.path())
      );
      return false;
   }

   // Check doc dir
   dir.setPath(getDocDir());
   if( ! dir.exists() || ! dir.isReadable() )
   {
      QMessageBox::information(
         0,
         errTitle,
         errText.arg(dir.path())
      );
      return false;
   }

   // Check config dir
   dir.setPath(getConfigDir(&success));
   if( !success || ! dir.exists() || ! dir.isReadable() )
   {
      QMessageBox::information(
         0,
         errTitle,
         errText.arg(dir.path())
      );
      return false;
   }

   // Check/create user data directory
   dir.setPath(getUserDataDir());
   if( !dir.exists() && !dir.mkpath(".") )
   {
      QMessageBox::information(
         0,
         errTitle,
         errText.arg(dir.path())
      );
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
   QString dbFileName;
   bool success = true;

   // Database files.
   dbFileName = getUserDataDir() + "database.sqlite";
   success &= QFile::copy(dbFileName, newPath + "database.sqlite");

   return success;
}

bool Brewtarget::ensureDataFilesExist()
{
   QString logFileName;
   bool success = true;
   
   logFile = new QFile();

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

   return displayEbc;
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
   QFile::Permissions sevenFiveFive = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                      QFile::ReadGroup |                     QFile::ExeGroup |
                                      QFile::ReadOther |                     QFile::ExeOther;
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
         
         // chmod 755 ~/.config
         QFile::setPermissions( dir.absolutePath() + "/.config", sevenFiveFive );
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
      
      // chmod 755 ~/.config/brewtarget
      QFile::setPermissions( dir.absolutePath() + "/brewtarget", sevenFiveFive );
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
   // This is the bin/ directory.
   dir = QDir(qApp->applicationDirPath());
   dir.cdUp();
   // Now we should be in the base directory (i.e. Brewtarget-2.0.0/)

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

bool Brewtarget::initialize()
{
   // Need these for changed(QMetaProperty,QVariant) to be emitted across threads.
   qRegisterMetaType<QMetaProperty>();
   qRegisterMetaType<Equipment*>();
   qRegisterMetaType<Mash*>();
   qRegisterMetaType<Style*>();
   qRegisterMetaType<Brewtarget::DBTable>();
   qRegisterMetaType< QList<BrewNote*> >();
   qRegisterMetaType< QList<Hop*> >();
   qRegisterMetaType< QList<Instruction*> >();
   qRegisterMetaType< QList<Fermentable*> >();
   qRegisterMetaType< QList<Misc*> >();
   qRegisterMetaType< QList<Yeast*> >();
   qRegisterMetaType< QList<Water*> >();
   
   // In Unix, make sure the user isn't running 2 copies.
#if defined(Q_WS_X11)
   pidFile.setFileName(QString("%1.pid").arg(getUserDataDir()));
   if( pidFile.exists() )
   {
      // Read the pid.
      qint64 pid;
      pidFile.open(QIODevice::ReadOnly);
      {
         QTextStream pidStream(&pidFile);
         pidStream >> pid;
      }
      pidFile.close();
      
      // If the pid is in the proc filesystem, another instance is running.
      // Have to check /proc, because perhaps the last instance crashed without
      // cleaning up after itself.
      QDir procDir(QString("/proc/%1").arg(pid));
      if( procDir.exists() )
      {
         std::cerr << "Brewtarget is already running. PID: " << pid << std::endl;
         return false;
      }
   }
   
   // Open the pidFile, erasing any contents, and write our pid.
   pidFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
   {
      QTextStream pidStream(&pidFile);
      pidStream << QCoreApplication::applicationPid();
   }
   pidFile.close();
#endif
   userDataDir = getConfigDir();
   
   // If the old options file exists, convert it. Otherwise, just get the
   // system options. I *think* this will work. The installer copies the old
   // one into the new place on Windows.
   if ( option("hadOldConfig", false).toBool() )
      convertPersistentOptions(); 

   readSystemOptions();

   // Make sure all the necessary directories and files we need exist before starting.
   bool success;
   success = ensureDirectoriesExist() && ensureDataFilesExist();
   if(!success)
      return false;

   loadTranslations(); // Do internationalization.

#if defined(Q_WS_MAC)
   qt_set_sequence_auto_mnemonic(TRUE); // turns on Mac Keyboard shortcuts
#endif
  
   // Check if the database was successfully loaded before
   // loading the main window.
   if (Database::instance().loadSuccessful())
   {
      if ( ! QSettings().contains("converted") )
         Database::instance().convertFromXml();
      
      return true;
   }
   else
      return false;

}

void Brewtarget::cleanup()
{
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
   delete _mainWindow;
   
   Database::dropInstance();
#if defined(Q_WS_X11)
   pidFile.remove();
#endif

}

int Brewtarget::run()
{
   int ret = 0;
   
   BtSplashScreen splashScreen;
   splashScreen.show();
   qApp->processEvents();
   if( !initialize() )
   {
      cleanup();
      return 1;
   }
   
   _mainWindow = new MainWindow();
   _mainWindow->setVisible(true);
   splashScreen.finish(_mainWindow);

   checkForNewVersion(_mainWindow);
   do {
      ret = qApp->exec();
   } while (ret == 1000);
   
   cleanup();

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
   std::cerr << m.toUtf8().constData() << std::endl;
   // Then display it in the GUI's status bar.
   if( _mainWindow && _mainWindow->statusBar() )
      _mainWindow->statusBar()->showMessage(m, 3000);

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
   if( Algorithms::isNan(amount) || Algorithms::isInf(amount) )
      return "-";
   
   // Special case.
   if( units == 0 )
      return QString("%L1").arg(amount, fieldWidth, format, precision);

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
      ret = QString("%L1 %2").arg(SIAmount, fieldWidth, format, precision).arg(SIUnitName);

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
      return QString("%L1 %2/%3").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
   else
      return QString("%L1").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
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

// Read the old options.xml file one more time, then move it out of the way.
void Brewtarget::convertPersistentOptions()
{
   QDir cfgDir = QDir(getConfigDir());
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

void Brewtarget::readSystemOptions()
{
   QString text;

   //================Version Checking========================
   checkVersion = option("check_version", QVariant(false)).toBool();

   //=====================Last DB Merge Request======================
   if( hasOption("last_db_merge_req"))
      lastDbMergeRequest = QDateTime::fromString(option("last_db_merge_req","").toString(), Qt::ISODate);

   //=====================Language====================
   if( hasOption("language") )
      setLanguage(option("language","").toString());

   //=======================Data Dir===========================
   if( hasOption("user_data_dir") )
      userDataDir = option("user_data_dir","").toString();

   //=======================Weight=====================
   text = option("weight_unit_system", "SI").toString();
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

   //===========================Volume=======================
   text = option("volume_unit_system", "SI").toString();
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

   //=======================Temp======================
   text = option("temperature_scale", "SI").toString();
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

   //======================Time======================
   // Set the one and only time system.
   timeSystem = UnitSystems::timeUnitSystem();

   //===================IBU===================
   text = option("ibu_formula", "tinseth").toString();
   if( text == "tinseth" )
      ibuFormula = TINSETH;
   else if( text == "rager" )
      ibuFormula = RAGER;
   else
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Bad ibu_formula type: %1").arg(text));
   }

   //========================Color======================
   text = option("color_formula", "morey").toString();
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

   //========================Gravity==================
   usePlato = option("use_plato", false).toBool();

   //=======================Color unit===================
   text = option("color_unit", "srm").toString();
   if( text == "srm" )
      colorUnit = SRM;
   else if( text == "ebc" )
      colorUnit = EBC;
   else
      Brewtarget::logW(QString("Bad color_unit type: %1").arg(text));
}

void Brewtarget::saveSystemOptions()
{
   QString text;

   setOption("check_version", checkVersion); 
   setOption("last_db_merge_req", lastDbMergeRequest.toString(Qt::ISODate));
   setOption("language", getCurrentLanguage());
   setOption("user_data_dir", userDataDir);
   setOption("weight_unit_system", weightSystem->unitType());
   setOption("volume_unit_system",volumeSystem->unitType()); 
   setOption("temperature_scale", tempSystem->unitType());
   setOption("use_plato", usePlato);

   switch(ibuFormula)
   {
      case TINSETH:
         setOption("ibu_formula", "tinseth");
         break; 
      case RAGER:
         setOption("ibu_formula", "rager");
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
}

double Brewtarget::weightQStringToSI(QString qstr, unitDisplay dispUnit)
{
   UnitSystem* temp = findMassUnitSystem(dispUnit);
   return temp->qstringToSI(qstr);
}

double Brewtarget::volQStringToSI(QString qstr, unitDisplay dispUnit)
{
   UnitSystem* temp = findVolumeUnitSystem(dispUnit);
   return temp->qstringToSI(qstr);
}

double Brewtarget::tempQStringToSI(QString qstr, unitDisplay dispUnit)
{
   UnitSystem* temp = findTemperatureSystem(dispUnit);
   return temp->qstringToSI(qstr);
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

QString Brewtarget::ibuFormulaName()
{
   switch ( ibuFormula )
   {
      case Brewtarget::TINSETH:
         return "Tinseth";
      case Brewtarget::RAGER:
         return "Rager";
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

QPair<double,double> Brewtarget::displayRange(BeerXMLElement* element, QObject *object, QString attribute, RangeType _type)
{
   QPair<double,double> range;
   QString minName = QString("%1%2").arg(attribute).arg("Min");
   QString maxName = QString("%1%2").arg(attribute).arg("Max");

   if ( _type == GRAVITY )
   {
      range.first  = displayOG(element, object, minName, false).toDouble();
      range.second = displayOG(element, object, maxName, false).toDouble();
   }
   else 
   {
      range.first = displayColor(element, object, "colorMin_srm", false).toDouble();
      range.second = displayColor(element, object, "colorMax_srm", false).toDouble();
   }

   return range;
}

QPair<double,double> Brewtarget::displayRange(QObject *object, QString attribute, double min, double max, RangeType _type)
{
   QPair<double,double> range;
   unitDisplay displayUnit;

   displayUnit = (unitDisplay)option(attribute, noUnit, object, UNIT).toInt();

   if ( _type == GRAVITY )
   {
      range.first = displayOG(min, displayUnit, false).toDouble();
      range.second = displayOG(max, displayUnit, false).toDouble();
   }
   else
   {
      range.first = displayColor(min, displayUnit, false).toDouble();
      range.second = displayColor(max, displayUnit, false).toDouble();
   }

   return range;
}

QString Brewtarget::displayOG( double og, unitDisplay displayUnit, bool showUnits)
{
   QString ret;

   if( Algorithms::isNan(og) || Algorithms::isInf(og) )
      return "-";
   
   // Field settings override defaults
   if ( displayUnit == noUnit ) 
      displayUnit = usePlato ? displayPlato : displaySg;

   QString tmp = "%L1";
   if ( showUnits && displayUnit == displayPlato )
      tmp = "%L1 %2";

   if( displayUnit == displaySg )
      ret = tmp.arg(og, 0, 'f', 3);
   else // Using Plato...
   {
      if( og >= 1.000 ) // Make sure OG is sane.
         ret = tmp.arg(Algorithms::SG_20C20C_toPlato(og), 0, 'f', 1);
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
   else
      return "?";
}

QString Brewtarget::displayFG( double fg, double og, unitDisplay displayUnit, bool showUnits  )
{
   QString ret = "%L1";

   if( Algorithms::isNan(fg) || Algorithms::isInf(fg) ||
       Algorithms::isNan(og) || Algorithms::isInf(og)
   )
      return "-";
   
   if ( displayUnit == noUnit )
      displayUnit = usePlato ? displayPlato : displaySg;

   if ( showUnits && displayUnit == displayPlato ) 
      ret = "%L1 %2";

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
         //plato = Algorithms::ogFgToPlato( og, fg );

         // The following shows ACTUAL Plato
         plato = Algorithms::SG_20C20C_toPlato(fg);
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

   if ( displayUnit == noUnit ) 
      displayUnit = colorUnit == Brewtarget::SRM ? displaySrm : displayEbc;
   if ( displayUnit == displaySrm )
      ret = showUnits ? QString("%L1 %2").arg(srm,0,'f',1).arg(Units::srm->getUnitName()) : QString("%L1").arg(srm,0,'f',1);
   else
   {
      double ebc = Units::ebc->fromSI(srm);
      ret = showUnits ? QString("%L1 %2").arg(ebc,0,'f',1).arg(Units::ebc->getUnitName()) : QString("%L1").arg(ebc,0,'f',1);
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

   return QSettings().contains(name);
}

void Brewtarget::setOption(QString attribute, QVariant value, const QObject* object, iUnitOps ops)
{
   QString name;

   if ( object )
      name = generateName(attribute,object,ops);
   else
      name = attribute;

   QSettings().setValue(name,value);
}

QVariant Brewtarget::option(QString attribute, QVariant default_value, const QObject* object, iUnitOps ops)
{
   QString name;

   if ( object )
      name = generateName(attribute,object,ops);
   else
      name = attribute;

   return QSettings().value(name,default_value);
}

void Brewtarget::removeOption(QString attribute)
{
   if ( hasOption(attribute) )
        QSettings().remove(attribute);
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
   generateAction(menu, tr("EBC"), displayEbc, unit);
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

MainWindow* Brewtarget::mainWindow()
{
   return _mainWindow;
}
