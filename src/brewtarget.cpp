/*
 * brewtarget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "brewtarget.h"
#include "config.h"
#include "database.h"
#include "UnitSystem.h"
#include "Algorithms.h"
#include "fermentable.h"

QApplication* Brewtarget::app;
MainWindow* Brewtarget::mainWindow;
QDomDocument* Brewtarget::optionsDoc;
QTranslator* Brewtarget::defaultTrans = new QTranslator();
QTranslator* Brewtarget::btTrans = new QTranslator();
QTextStream* Brewtarget::logStream = 0;
QFile* Brewtarget::logFile = 0;

QString Brewtarget::currentLanguage = "en";

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

void Brewtarget::setApp(QApplication& a)
{
   app = &a;
   ensureFilesExist(); // Make sure all the files we need exist before starting.
   readPersistentOptions(); // Read all the options for bt.
   loadTranslations(); // Do internationalization.
}

bool Brewtarget::ensureFilesExist()
{
   QString dbFileName, recipeFileName, mashFileName, optionsFileName, logFileName;
   QFile dbFile, recipeFile, mashFile, optionsFile;
   bool success = true;
   
   logFile = new QFile();

   // Database files.
   dbFileName = getConfigDir() + "database.xml";
   recipeFileName = getConfigDir() + "recipes.xml";
   mashFileName = getConfigDir() + "mashs.xml";
   optionsFileName = getConfigDir() + "options.xml";
   
   dbFile.setFileName(dbFileName);
   recipeFile.setFileName(recipeFileName);
   mashFile.setFileName(mashFileName);
   optionsFile.setFileName(optionsFileName);
   
   if( !dbFile.exists() )
      success &= QFile::copy(Brewtarget::getDataDir() + "database.xml", dbFileName);
   if( !recipeFile.exists() )
      success &= QFile::copy(Brewtarget::getDataDir() + "recipes.xml", recipeFileName);
   if( !mashFile.exists() )
      success &= QFile::copy(Brewtarget::getDataDir() + "mashs.xml", mashFileName);
   if( !optionsFile.exists() )
      success &= QFile::copy(Brewtarget::getDataDir() + "options.xml", optionsFileName);
   
   // Log file
   logFile->setFileName(getConfigDir() + "brewtarget_log.txt");
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
   if( app == 0 )
      return;

   // Load translators.
   defaultTrans->load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
   if( getCurrentLanguage().isEmpty() )
      setLanguage(getSystemLanguage());
   //btTrans->load("bt_" + getSystemLanguage());

   // Install translators.
   app->installTranslator(defaultTrans);
   //app->installTranslator(btTrans);
}

void Brewtarget::setLanguage(QString twoLetterLanguage)
{
   currentLanguage = twoLetterLanguage;
   app->removeTranslator(btTrans);

   QString filename = QString("bt_%1").arg(twoLetterLanguage);
   QString dir = QString("%1translations_qm/").arg(getDataDir());
   if( btTrans->load( filename, dir ) )
      app->installTranslator(btTrans);
}

const QString& Brewtarget::getCurrentLanguage()
{
   return currentLanguage;
}

QApplication* Brewtarget::getApp()
{
   return app;
}

iUnitSystem Brewtarget::getWeightUnitSystem()
{
   return weightUnitSystem;
}

iUnitSystem Brewtarget::getVolumeUnitSystem()
{
   return volumeUnitSystem;
}

TempScale Brewtarget::getTemperatureScale()
{
   return tempScale;
}

QString Brewtarget::getDataDir()
{
   QString dir = app->applicationDirPath();
#if defined(Q_WS_X11) // Linux OS.

   dir = QString(CONFIGDATADIR);
   
#elif defined(Q_WS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/";

#else // Windows OS.

   dir += "/";

#endif

   if( ! dir.endsWith('/') )
      dir += "/";

   return dir;
}

QString Brewtarget::getDocDir()
{
   QString dir = app->applicationDirPath();
#if defined(Q_WS_X11) // Linux OS.

   dir = QString(CONFIGDOCDIR);

#elif defined(Q_WS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/en.lproj/";

#else // Windows OS.

   dir += "/doc/";

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

   QString dir= app->applicationDirPath();
   dir += "/";
   if( success != 0 )
      *success = true;
   return dir;

#endif
}

int Brewtarget::run()
{
   int ret;
   Database::initialize();
   
   mainWindow = new MainWindow();

   mainWindow->setVisible(true);

   ret = app->exec();
   savePersistentOptions();
   // Close log file.
   if( logFile != 0 && logFile->isOpen() )
      logFile->close();
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
QString Brewtarget::displayAmount( double amount, Unit* units, int precision )
{
   int fieldWidth = 0;
   char format = 'f';

   // Special case.
   if( units == 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   QString ret;

   // convert to the current unit system (s).

   if(SIUnitName.compare("kg") == 0) // Dealing with mass.
      ret = weightSystem->displayAmount( amount, units );
   else if( SIUnitName.compare("L") == 0 ) // Dealing with volume
      ret = volumeSystem->displayAmount( amount, units );
   else if( SIUnitName.compare("C") == 0 ) // Dealing with temperature.
      ret = tempSystem->displayAmount( amount, units );
   else if( SIUnitName.compare("min") == 0 ) // Time
      ret = timeSystem->displayAmount( amount, units );
   else // If we don't deal with it above, just use the SI amount.
   {
      ret = QString("%1 %2").arg(SIAmount, fieldWidth, format, precision).arg(SIUnitName);
   }

   return ret;
}

QString Brewtarget::displayThickness(double thick_lkg)
{
   int fieldWidth = 0;
   char format = 'f';
   int precision = 2;

   Unit* volUnit = volumeSystem->thicknessUnit();
   Unit* weightUnit = weightSystem->thicknessUnit();

   double num = volUnit->fromSI(thick_lkg);
   double den = weightUnit->fromSI(1.0);

   return QString("%1 %2/%3").arg(num/den, fieldWidth, format, precision).arg(volUnit->getUnitName()).arg(weightUnit->getUnitName());
}

void Brewtarget::readPersistentOptions()
{
   QFile xmlFile(getConfigDir() + "options.xml");
   optionsDoc = new QDomDocument();
   QDomElement root;
   QDomNode node, child;
   QDomText textNode;
   QDomNodeList list;
   QString err;
   QString text;
   int line;
   int col;

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

   // backwards compat for old 'english units' - these were US Customary
   // for weight and volume and fahrenheit for temp
   list = optionsDoc->elementsByTagName(QString("english_units"));
   if( list.length() <= 0 )
   {
      //could be pre 'english_units', in which case default to US Customary
      weightUnitSystem = USCustomary;
      volumeUnitSystem = USCustomary;
      tempScale = Fahrenheit;

      weightSystem = UnitSystems::usWeightUnitSystem();
      volumeSystem = UnitSystems::usVolumeUnitSystem();
      tempSystem = UnitSystems::fahrenheitTempUnitSystem();
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

      if( text == "true" )
      {
         weightUnitSystem = USCustomary;
         volumeUnitSystem = USCustomary;
         tempScale = Fahrenheit;

         weightSystem = UnitSystems::usWeightUnitSystem();
         volumeSystem = UnitSystems::usVolumeUnitSystem();
         tempSystem = UnitSystems::fahrenheitTempUnitSystem();
      }
      else
      {
         weightUnitSystem = SI;
         volumeUnitSystem = SI;
         tempScale = Celsius;

         weightSystem = UnitSystems::siWeightUnitSystem();
         volumeSystem = UnitSystems::siVolumeUnitSystem();
         tempSystem = UnitSystems::celsiusTempUnitSystem();
      }
 
   }

   //=====================Language====================
   list = optionsDoc->elementsByTagName(QString("language"));
   if(list.length() <= 0)
      Brewtarget::logW(QString("Could not find the language tag in the option file."));
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

      setLanguage(text);
   }

   //=======================Weight=====================
   list = optionsDoc->elementsByTagName(QString("weight_unit_system"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not find the weight_unit_system tag in the option file."));
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

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
   list = optionsDoc->elementsByTagName(QString("volume_unit_system"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not find the volume_unit_system tag in the option file."));
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

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
   list = optionsDoc->elementsByTagName(QString("temperature_scale"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not find the temperature_scale tag in the option file."));
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

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

   // Set the one and only time system.
   timeSystem = UnitSystems::timeUnitSystem();

   //===================IBU===================
   // Get IBU formula.
   list = optionsDoc->elementsByTagName(QString("ibu_formula"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not find the ibu_formula tag in the option file."));
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

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
   // Get color formula.
   list = optionsDoc->elementsByTagName(QString("color_formula"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not find the color_formula tag in the option file."));
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

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
   // Get plato usage.
   list = optionsDoc->elementsByTagName(QString("use_plato"));
   if( list.length() <= 0 )
      Brewtarget::logW( "Could not find the use_plato tag in the option file." );
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

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
   list = optionsDoc->elementsByTagName("color_unit");
   if( list.length() <= 0 )
      Brewtarget::logW( "Could not find the color_unit tag in the option file." );
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

      if( text == "srm" )
         colorUnit = SRM;
      else if( text == "ebc" )
         colorUnit = EBC;
      else
         Brewtarget::logW(QString("Bad color_unit type: %1").arg(text));;
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

double Brewtarget::timeQStringToSI(QString qstr)
{
   return timeSystem->qstringToSI(qstr);
}

QString Brewtarget::displayOG( double og, bool showUnits )
{
   QString tmp = (showUnits & usePlato) ? "%1 %2" : "%1";
   QString ret;

   if( usePlato == false )
      ret = tmp.arg(og, 0, 'f', 3);
   else // Using Plato...
   {
      if( og >= 1.000 ) // Make sure OG is sane.
         ret = tmp.arg(Algorithms::Instance().SG_20C20C_toPlato(og), 0, 'f', 1);
      else
         ret = tmp.arg(0);
   }

   if( showUnits )
      ret = usePlato? ret.arg("P") : ret;

   return ret;
}

QString Brewtarget::displayFG( double fg, double og, bool showUnits )
{
   QString ret = (showUnits & usePlato) ? "%1 %2" : "%1";
   if( usePlato == false )
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

   if( showUnits )
      ret = usePlato ? ret.arg("P") : ret;

   return ret;
}

QString Brewtarget::displayColor( double srm, bool showUnits )
{
   QString ret;

   if( colorUnit == SRM )
      ret = showUnits ? QString("%1 %2").arg(srm,0,'f',1).arg(Units::srm->getUnitName()) : QString("%1").arg(srm,0,'f',1);
   else
   {
      double ebc = Units::ebc->fromSI(srm);
      ret = showUnits ? QString("%1 %2").arg(ebc,0,'f',1).arg(Units::ebc->getUnitName()) : QString("%1").arg(ebc,0,'f',1);
   }

   return ret;
}
