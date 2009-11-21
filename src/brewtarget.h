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
#include "MainWindow.h"
#include "unit.h"

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
   static QString displayAmount( double amount, Unit* units=0 );

   static UnitSystem getWeightUnitSystem();
   static UnitSystem getVolumeUnitSystem();
   static TempScale getTemperatureScale();
   
   static void readPersistentOptions();
   static void savePersistentOptions();

private:
   static QApplication* app;
   static MainWindow* mainWindow;
   static QDomDocument* optionsDoc;

   // These are options that are ONLY to be edited by the OptionDialog.
   static UnitSystem weightUnitSystem;
   static UnitSystem volumeUnitSystem;
   static TempScale tempScale;
   static ColorType colorFormula;
   static IbuType ibuFormula;
};

#endif	/* _BREWTARGET_H */

