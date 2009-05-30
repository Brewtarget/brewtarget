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
public:
   Brewtarget();

   enum LogType {WARNING, ERROR};

   static QApplication* getApp();
   static void setApp(QApplication& a); // This method should be called before any of the others.
   static QString getDataDir();
   static int run();
   static void log( LogType lt, std::string message );
   static QString displayAmount( double amount, Unit* units=0 );

   static void readPersistentOptions();
   static void savePersistentOptions();

private:
   static QApplication* app;
   static MainWindow* mainWindow;
   static QDomDocument* optionsDoc;

   // These are options that are ONLY to be edited by the OptionDialog.
   static bool englishUnits;
};

#endif	/* _BREWTARGET_H */

