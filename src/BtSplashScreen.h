/*
 * BtSplashScreen.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BTSPLASHSCREEN_H
#define _BTSPLASHSCREEN_H

#include <QWidget>
#include <QSplashScreen>
#include <QString>
#include <QLabel>

/*!
 * \class BtSplashScreen
 * \author Philip G. Lee
 *
 * \brief A class for showing the brewtarget splash screen on startup.
 */
class BtSplashScreen : public QSplashScreen
{
   Q_OBJECT

public:
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   BtSplashScreen(QWidget* parent=nullptr);
#else
   BtSplashScreen(QScreen* parent=nullptr);
#endif

   void showMessage(QString const& message);
};

#endif /*_BTSPLASHSCREEN_H*/
