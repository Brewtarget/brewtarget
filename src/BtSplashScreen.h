/*
* BtSplashScreen.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2012.
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

#ifndef _BTSPLASHSCREEN_H
#define _BTSPLASHSCREEN_H

class BtSplashScreen;

#include <QWidget>
#include <QSplashScreen>
#include <QString>
#include <QColor>
#include "ui_btSplashScreen.h"

/*!
 * \class BtSplashScreen
 * \author Philip G. Lee
 *
 * \brief A class for showing the brewtarget splash screen on startup.
 */
class BtSplashScreen : public QSplashScreen, public Ui::btSplashScreen
{
   Q_OBJECT

public:
   BtSplashScreen(QWidget* parent=0);
   
   //! Overloaded from \b QSplashScreen
   void showMessage( const QString& message,
                     int alignment = Qt::AlignLeft,
                     const QColor& color = Qt::black );
   //! Overloaded from \b QSplashScreen
   void clearMessage();
};

#endif /*_BTSPLASHSCREEN_H*/