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

#include "brewtarget.h"
#include "config.h"
#include "database.h"

QApplication* Brewtarget::app;
MainWindow* Brewtarget::mainWindow;

void Brewtarget::setApp(QApplication& a)
{
   app = &a;
}

QApplication* Brewtarget::getApp()
{
   return app;
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

int Brewtarget::run()
{
   Database::initialize();
   
   mainWindow = new MainWindow();

   mainWindow->show();

   return app->exec();
}
