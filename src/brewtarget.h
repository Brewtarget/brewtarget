/*
 * brewtarget.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _BREWTARGET_H
#define	_BREWTARGET_H

class Brewtarget;

#include <QApplication>
#include <QString>
#include <string>
#include <QDomDocument>
#include <QTranslator>
#include "MainWindow.h"
#include "unit.h"
#include "UnitSystem.h"

class Brewtarget
{
   friend class OptionDialog;
   friend class IbuMethods;
   friend class ColorMethods;
   friend class RecipeFormatter;
   friend class Unit;
public:
   Brewtarget();

   enum LogType {WARNING, ERROR};
   enum ColorType {MOSHER, DANIEL, MOREY};
   enum IbuType {TINSETH, RAGER};

   static QApplication* getApp();
   static void setApp(QApplication& a); // This method should be called before any of the others.
   static QString getDataDir();
   static QString getDocDir();
   static QString getConfigDir();
   static int run();
   static void log( LogType lt, std::string message );
   static void log( LogType lt, QString message );
   static void logE( QString message ); // Error message.
   static void logW( QString message ); // Warning message.

   static QString displayAmount( double amount, Unit* units=0 );
   // Displays thickness in appropriate units from standard thickness in L/kg.
   static QString displayThickness( double thick_lkg );
   // Display gravity appropriately.
   static QString displayOG( double og );
   static QString displayFG( double fg, double og ); // Need OG if we're using plato.
   static double weightQStringToSI( QString qstr );
   static double volQStringToSI( QString qstr );
   static double tempQStringToSI( QString qstr );
   static double timeQStringToSI( QString qstr );

   static iUnitSystem getWeightUnitSystem();
   static iUnitSystem getVolumeUnitSystem();
   static TempScale getTemperatureScale();
   
   static void readPersistentOptions();
   static void savePersistentOptions();

private:
   static QApplication* app;
   static MainWindow* mainWindow;
   static QDomDocument* optionsDoc;
   static QTranslator* defaultTrans;
   static QTranslator* btTrans;

   static bool ensureFilesExist(); // Ensure the db and option files exist.
   static void loadTranslations(); // Load translation files.
   
   // These are options that are ONLY to be edited by the OptionDialog.
   static bool usePlato; // Whether or not to display plato instead of SG.
   //
   static iUnitSystem weightUnitSystem;
   static iUnitSystem volumeUnitSystem;
   //
   static UnitSystem* weightSystem;
   static UnitSystem* volumeSystem;
   static UnitSystem* tempSystem;
   static UnitSystem* timeSystem;
   //
   static TempScale tempScale;
   static ColorType colorFormula;
   static IbuType ibuFormula;
};

#endif	/* _BREWTARGET_H */

