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
#include <QTextStream>

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
   enum ColorUnitType {SRM, EBC};
   enum IbuType {TINSETH, RAGER};

   static QApplication* getApp();
   static void setApp(QApplication& a); // This method should be called before any of the others.
   static QString getDataDir();
   static QString getDocDir();
   static QString getConfigDir(bool* success = 0);
   static int run();
   static void log( LogType lt, QString message );
   static void logE( QString message ); // Error message.
   static void logW( QString message ); // Warning message.

   /*!
    * Produces the appropriate string for 'amount' which has units 'units'.
    * Variable 'precision' controls how many decimal places.
    */
   static QString displayAmount( double amount, Unit* units=0, int precision=3 );
   //! Displays thickness in appropriate units from standard thickness in L/kg.
   static QString displayThickness( double thick_lkg );
   //! Display gravity appropriately.
   static QString displayOG( double og, bool showUnits=false );
   static QString displayFG( double fg, double og, bool showUnits=false ); // Need OG if we're using plato.
   static QString displayColor( double srm, bool showUnits );
   static double weightQStringToSI( QString qstr );
   static double volQStringToSI( QString qstr );
   static double tempQStringToSI( QString qstr );
   static double timeQStringToSI( QString qstr );

   static iUnitSystem getWeightUnitSystem();
   static iUnitSystem getVolumeUnitSystem();
   static TempScale getTemperatureScale();
   
   static void readPersistentOptions();
   static void savePersistentOptions();

   /*!
    * Loads the brewtarget translator with two letter ISO 639-1 code
    * 'twoLetterLanguage'. For example, for spanish, it would
    * be 'es'.
    * Currently, this does NO checking to make sure the locale
    * code is acceptable.
    */
   static void setLanguage(QString twoLetterLanguage);
   /*!
    * Gets the 2-letter ISO 639-1 language code we are currently using.
    */
   static const QString& getCurrentLanguage();
   /*!
    * Gets the ISO 639-1 language code for the system.
    */
   static const QString& getSystemLanguage();

private:
   static QApplication* app;
   static MainWindow* mainWindow;
   static QDomDocument* optionsDoc;
   static QTranslator* defaultTrans;
   static QTranslator* btTrans;
   static QFile* logFile;
   static QTextStream* logStream;
   static QString currentLanguage;

   //! Ensure the db and option files exist.
   static bool ensureFilesExist();
   //! Load translation files.
   static void loadTranslations();
   
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
   static ColorUnitType colorUnit;
   static IbuType ibuFormula;
};

#endif	/* _BREWTARGET_H */

