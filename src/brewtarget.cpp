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

#include "brewtarget.h"
#include "config.h"
#include "database.h"

QApplication* Brewtarget::app;
MainWindow* Brewtarget::mainWindow;
QDomDocument* Brewtarget::optionsDoc;
bool Brewtarget::englishUnits = false;
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

bool Brewtarget::useEnglishUnits()
{
   return englishUnits;
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
      m = "WARNING: " + message;
   else if( lt == ERROR )
      m = "ERROR: " + message;
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
   double SIAmount = Unit::convert( amount, units->getUnitName(), SIUnitName );
   QString ret;

   // Check to see if we have to use pesky english units.
   if( englishUnits )
   {
      if(SIUnitName.compare("kg") == 0) // Dealing with mass.
      {
         if( SIAmount < Units::pounds->toSI(1.0) ) // If less than 1 pound, display ounces.
            ret = QString("%1 %2").arg(Units::ounces->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::ounces->getUnitName().c_str());
         else
            ret = QString("%1 %2").arg(Units::pounds->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::pounds->getUnitName().c_str());
      }
      else if( SIUnitName.compare("L") == 0 ) // Dealing with volume
      {
         if( SIAmount < Units::tablespoons->toSI(1.0) ) // If less than 1 tbsp, show tsp
            ret = QString("%1 %2").arg(Units::teaspoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::teaspoons->getUnitName().c_str());
         else if( SIAmount < Units::cups->toSI(0.25) ) // If less than 1/4 cup, show tbsp
            ret = QString("%1 %2").arg(Units::tablespoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::tablespoons->getUnitName().c_str());
         else if( SIAmount < Units::quarts->toSI(1.0) ) // If less than 1 qt, show cups
            ret = QString("%1 %2").arg(Units::cups->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::cups->getUnitName().c_str());
         else if( SIAmount < Units::gallons->toSI(1.0) ) // If less than 1 gallon, show quarts
            ret = QString("%1 %2").arg(Units::quarts->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::quarts->getUnitName().c_str());
         else
            ret = QString("%1 %2").arg(Units::gallons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::gallons->getUnitName().c_str());
      }
      else if( SIUnitName.compare("C") == 0 ) // Dealing with temperature.
      {
         ret = QString("%1 %2").arg(Units::fahrenheit->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::fahrenheit->getUnitName().c_str());
      }
      else if( SIUnitName.compare("min") == 0 ) // Time
      {
         if( SIAmount < Units::minutes->toSI(1.0) )
            ret = QString("%1 %2").arg(Units::seconds->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::seconds->getUnitName().c_str());
         else if( SIAmount < Units::hours->toSI(1.0) )
            ret = QString("%1 %2").arg(Units::minutes->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::minutes->getUnitName().c_str());
         else
            ret = QString("%1 %2").arg(Units::hours->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::hours->getUnitName().c_str());
      }
      else // If we don't deal with it above, just use the SI amount.
         ret = QString("%1 %2").arg(SIAmount, fieldWidth, format, precision).arg(SIUnitName.c_str());

      return ret;
   }

   // Otherwise, we're dealing with SI units.
   if( SIUnitName.compare("kg") == 0 ) // Mass
   {
      if( SIAmount < Units::grams->toSI(1.0) )
         ret = QString("%1 %2").arg(Units::milligrams->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::milligrams->getUnitName().c_str());
      else if( SIAmount < Units::kilograms->toSI(1.0) )
         ret = QString("%1 %2").arg(Units::grams->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::grams->getUnitName().c_str());
      else
         ret = QString("%1 %2").arg(Units::kilograms->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::kilograms->getUnitName().c_str());
   }
   else if( SIUnitName.compare("L") == 0 ) // Volume
   {
      if( SIAmount < Units::liters->toSI(1.0) )
         ret = QString("%1 %2").arg(Units::milliliters->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::milliliters->getUnitName().c_str());
      else
         ret = QString("%1 %2").arg(Units::liters->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::liters->getUnitName().c_str());
   }
   else if( SIUnitName.compare("C") == 0 ) // Dealing with temperature.
   {
      ret = QString("%1 %2").arg(Units::celsius->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::celsius->getUnitName().c_str());
   }
   else if( SIUnitName.compare("min") == 0 ) // Time
   {
      if( SIAmount < Units::minutes->toSI(1.0) )
         ret = QString("%1 %2").arg(Units::seconds->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::seconds->getUnitName().c_str());
      else if( SIAmount < Units::hours->toSI(1.0) )
         ret = QString("%1 %2").arg(Units::minutes->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::minutes->getUnitName().c_str());
      else
         ret = QString("%1 %2").arg(Units::hours->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::hours->getUnitName().c_str());
   }
   else
      ret = QString("%1 %2").arg(SIAmount, fieldWidth, format, precision).arg(SIUnitName.c_str());

   return ret;
}

void Brewtarget::readPersistentOptions()
{
   QFile xmlFile(getDataDir() + "options.xml");
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
      log(WARNING, "Could not open " + xmlFile.fileName().toStdString() + " for reading.");
      return;
   }

   if( ! optionsDoc->setContent(&xmlFile, false, &err, &line, &col) )
   {
      log(WARNING, "Bad document formatting in " +
                   xmlFile.fileName().toStdString() +
                   QString(" %1:%2.").arg(line).arg(col).toStdString());
      log(WARNING, QString("Bad document formatting in %1 %2:%3.").arg(xmlFile.fileName()).arg(line).arg(col).toStdString() );
   }

   root = optionsDoc->documentElement();
   /*
   for( node = root.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() || ! node.hasChildNodes() )
         continue;

      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

      if( node.nodeName() == "english_units" )
      {
         if( text == "true" )
            englishUnits = true;
         else if( text == "false" )
            englishUnits = false;
      }
   }
   */

   // Get english units.
   list = optionsDoc->elementsByTagName(QString("english_units"));
   if( list.length() <= 0 )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not find the english_units tag in the option file."));
      englishUnits = true;
   }
   else
   {
      node = list.at(0);
      child = node.firstChild();
      textNode = child.toText();
      text = textNode.nodeValue();

      if( text == "true" )
         englishUnits = true;
      else
         englishUnits = false;
   }

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

   delete optionsDoc;
   optionsDoc = 0;
   xmlFile.close();
}

void Brewtarget::savePersistentOptions()
{
   QFile xmlFile(getDataDir() + "options.xml");
   optionsDoc = new QDomDocument();
   QDomElement root;
   QDomNode node, child;
   QString text;

   if( ! xmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate) )
   {
      log(WARNING, "Could not open " + xmlFile.fileName().toStdString() + " for writing.");
      return;
   }

   root = optionsDoc->createElement("options");

   // English units.
   node = optionsDoc->createElement("english_units");
   child = optionsDoc->createTextNode( englishUnits ? "true" : "false" );
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
