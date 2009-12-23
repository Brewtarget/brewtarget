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

#include "brewtarget.h"
#include "config.h"
#include "database.h"

QApplication* Brewtarget::app;
MainWindow* Brewtarget::mainWindow;
QDomDocument* Brewtarget::optionsDoc;
UnitSystem Brewtarget::weightUnitSystem = SI;
UnitSystem Brewtarget::volumeUnitSystem = SI;
TempScale Brewtarget::tempScale = Celsius;
Brewtarget::ColorType Brewtarget::colorFormula = Brewtarget::MOREY;
Brewtarget::IbuType Brewtarget::ibuFormula = Brewtarget::TINSETH;

void Brewtarget::setApp(QApplication& a)
{
   app = &a;
   readPersistentOptions();
}

QApplication* Brewtarget::getApp()
{
   return app;
}

UnitSystem Brewtarget::getWeightUnitSystem()
{
   return weightUnitSystem;
}

UnitSystem Brewtarget::getVolumeUnitSystem()
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

   return QString(CONFIGDATADIR);
   
#elif defined(Q_WS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/";
   return dir;

#else // Windows OS.

   dir += "/";
   return dir;

#endif
}

QString Brewtarget::getDocDir()
{
   QString dir = app->applicationDirPath();
#if defined(Q_WS_X11) // Linux OS.

   return QString(CONFIGDOCDIR);

#elif defined(Q_WS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/en.lproj/";
   return dir;

#else // Windows OS.

   dir += "/doc/";
   return dir;

#endif
}

QString Brewtarget::getConfigDir()
{
#if defined(Q_WS_X11) or defined(Q_WS_MAC) // Linux OS or Mac OS.

   QDir dir;
   char* xdg_config_home = getenv("XDG_CONFIG_HOME");
   if (xdg_config_home)
   {
      dir = xdg_config_home;
   }
   else
   {
      dir = QDir::home();
      if (!dir.exists(".config"))
      {
         dir.mkdir(".config");
      }
      dir.cd(".config");
   }
   if (!dir.exists("brewtarget"))
   {
      dir.mkdir("brewtarget");
   }
   dir.cd("brewtarget");

   return dir.absolutePath() + "/";

#else // Windows OS.

   QString dir= app->applicationDirPath();
   dir += "/";
   return dir;

#endif
}

int Brewtarget::run()
{
   int ret;
   Database::initialize();
   
   mainWindow = new MainWindow();

   mainWindow->show();

   ret = app->exec();
   savePersistentOptions();
   return ret;
}

void Brewtarget::log( LogType lt, std::string message )
{
   log( lt, QString(message.c_str()) );
}

void Brewtarget::log(LogType lt, QString message)
{
   QString m;
   
   if( lt == WARNING )
      m = QObject::tr("WARNING: %1").arg(message);
   else if( lt == ERROR )
      m = QObject::tr("ERROR: %1").arg(message);
   else
      m = message;
   
   // First, write out to stderr.
   std::cerr << m.toStdString() << std::endl;
   // Then display it in the GUI's status bar.
   // Hmm... I can't access the mainWindow from outside the
   // GUI event loop?
   //if( mainWindow->statusBar() != 0 )
   //   mainWindow->statusBar()->showMessage(m, 3000);
}

// Displays "amount" of units "units" in the proper format.
// If "units" is null, just return the amount.
QString Brewtarget::displayAmount( double amount, Unit* units )
{
   int fieldWidth = 0;
   char format = 'f';
   int precision = 3;

   // Special case.
   if( units == 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   std::string SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   QString ret;

   // convert to the current unit system (s).

   if(SIUnitName.compare("kg") == 0) // Dealing with mass.
   {
      switch ( weightUnitSystem )
      {
         case USCustomary:
         case Imperial:
         {
            if( SIAmount < Units::pounds->toSI(1.0) ) // If less than 1 pound, display ounces.
               ret = QString("%1 %2").arg(Units::ounces->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::ounces->getUnitName().c_str());
            else
               ret = QString("%1 %2").arg(Units::pounds->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::pounds->getUnitName().c_str());
            
            return ret;
         }

         case SI:
         default:
         {
            if( SIAmount < Units::grams->toSI(1.0) )
               ret = QString("%1 %2").arg(Units::milligrams->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::milligrams->getUnitName().c_str());
            else if( SIAmount < Units::kilograms->toSI(1.0) )
               ret = QString("%1 %2").arg(Units::grams->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::grams->getUnitName().c_str());
            else
               ret = QString("%1 %2").arg(Units::kilograms->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::kilograms->getUnitName().c_str());
            return ret;
         }
      }
   }
   else if( SIUnitName.compare("L") == 0 ) // Dealing with volume
   {
      switch ( volumeUnitSystem )
      {
         case USCustomary:
         {
            if( SIAmount < Units::tablespoons->toSI(1.0) ) // If less than 1 tbsp, show tsp
                     ret = QString("%1 %2").arg(Units::teaspoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::teaspoons->getUnitName().c_str());
            else if( SIAmount < Units::us_cups->toSI(0.25) ) // If less than 1/4 cup, show tbsp
               ret = QString("%1 %2").arg(Units::tablespoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::tablespoons->getUnitName().c_str());
            else if( SIAmount < Units::us_quarts->toSI(1.0) ) // If less than 1 qt, show us_cups
               ret = QString("%1 %2").arg(Units::us_cups->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_cups->getUnitName().c_str());
            else if( SIAmount < Units::us_gallons->toSI(1.0) ) // If less than 1 gallon, show us_quarts
               ret = QString("%1 %2").arg(Units::us_quarts->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_quarts->getUnitName().c_str());
            else
               ret = QString("%1 %2").arg(Units::us_gallons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_gallons->getUnitName().c_str());

            return ret;
         }

         case Imperial:
         {
            if( SIAmount < Units::tablespoons->toSI(1.0) ) // If less than 1 tbsp, show tsp
                     ret = QString("%1 %2").arg(Units::teaspoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::teaspoons->getUnitName().c_str());
            else if( SIAmount < Units::imperial_cups->toSI(0.25) ) // If less than 1/4 cup, show tbsp
               ret = QString("%1 %2").arg(Units::tablespoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::tablespoons->getUnitName().c_str());
            else if( SIAmount < Units::imperial_quarts->toSI(1.0) ) // If less than 1 qt, show imperial_cups
               ret = QString("%1 %2").arg(Units::imperial_cups->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_cups->getUnitName().c_str());
            else if( SIAmount < Units::imperial_gallons->toSI(1.0) ) // If less than 1 gallon, show imperial_quarts
               ret = QString("%1 %2").arg(Units::imperial_quarts->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_quarts->getUnitName().c_str());
            else
               ret = QString("%1 %2").arg(Units::imperial_gallons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_gallons->getUnitName().c_str());

             return ret;
         }

         case SI:
         default:
         {
            if( SIAmount < Units::liters->toSI(1.0) )
               ret = QString("%1 %2").arg(Units::milliliters->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::milliliters->getUnitName().c_str());
            else
               ret = QString("%1 %2").arg(Units::liters->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::liters->getUnitName().c_str());
       
            return ret;
         }
      }
   }
   else if( SIUnitName.compare("C") == 0 ) // Dealing with temperature.
   {
      switch (tempScale)
      {
         case Fahrenheit:
         {
            ret = QString("%1 %2").arg(Units::fahrenheit->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::fahrenheit->getUnitName().c_str());

            return ret;
         }
         case Celsius:
         default:
         {
            ret = QString("%1 %2").arg(Units::celsius->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::celsius->getUnitName().c_str());
            return ret;
         }
      }
   }
   else if( SIUnitName.compare("min") == 0 ) // Time
   {
      if( SIAmount < Units::minutes->toSI(1.0) )
         ret = QString("%1 %2").arg(Units::seconds->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::seconds->getUnitName().c_str());
      else if( SIAmount < Units::hours->toSI(1.0) )
         ret = QString("%1 %2").arg(Units::minutes->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::minutes->getUnitName().c_str());
      else
         ret = QString("%1 %2").arg(Units::hours->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::hours->getUnitName().c_str());
      return ret;
   }
   else // If we don't deal with it above, just use the SI amount.
   {
      ret = QString("%1 %2").arg(SIAmount, fieldWidth, format, precision).arg(SIUnitName.c_str());

      return ret;
   }
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

   if( ! xmlFile.open(QIODevice::ReadOnly) )
   {
      log(WARNING, QObject::tr("Could not open %1 for reading.").arg(xmlFile.fileName()));
      return;
   }

   if( ! optionsDoc->setContent(&xmlFile, false, &err, &line, &col) )
      log(WARNING, QObject::tr("Bad document formatting in %1 %2:%3").arg(xmlFile.fileName()).arg(line).arg(col));

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
      }
      else
      {
         weightUnitSystem = SI;
         volumeUnitSystem = SI;
         tempScale = Celsius;
      }
 
   }

   list = optionsDoc->elementsByTagName(QString("weight_unit_system"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not find the weight_unit_system tag in the option file."));
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

      if( text == "Imperial" )
         weightUnitSystem = Imperial;
      else if (text == "USCustomary")
         weightUnitSystem = USCustomary;
      else
         weightUnitSystem = SI;
   }

   list = optionsDoc->elementsByTagName(QString("volume_unit_system"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not find the volume_unit_system tag in the option file."));
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

      if( text == "Imperial" )
         volumeUnitSystem = Imperial;
      else if (text == "USCustomary")
         volumeUnitSystem = USCustomary;
      else
         volumeUnitSystem = SI;
   }

   list = optionsDoc->elementsByTagName(QString("temperature_scale"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not find the temperature_scale tag in the option file."));
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

      if( text == "Fahrenheit" )
         tempScale = Fahrenheit;
      else 
         tempScale = Celsius;
   }


   // Get IBU formula.
   list = optionsDoc->elementsByTagName(QString("ibu_formula"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not find the ibu_formula tag in the option file."));
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
         Brewtarget::log(Brewtarget::ERROR, QObject::tr("Bad ibu_formula type: %1").arg(text));
      }
   }

   // Get color formula.
   list = optionsDoc->elementsByTagName(QString("color_formula"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not find the color_formula tag in the option file."));
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
         Brewtarget::log(Brewtarget::ERROR, QObject::tr("Bad color_formula type: %1").arg(text));
      }
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
   switch( ibuFormula )
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

   // Add root to document.
   optionsDoc->appendChild(root);

   // Write file.
   QTextStream out(&xmlFile);
   out << optionsDoc->toString();

   xmlFile.close();
   delete optionsDoc;
   optionsDoc = 0;
}
