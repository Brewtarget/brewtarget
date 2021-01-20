/*
 * BtSplashScreen.cpp is part of Brewtarget, and is Copyright the following
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

#include <QPixmap>
#include "BtSplashScreen.h"

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
BtSplashScreen::BtSplashScreen(QWidget* parent) :
   QSplashScreen(parent, QPixmap(":/images/brewtarget.svg"))
{
   setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
   showMessage(tr("Loading..."));
}
#else
BtSplashScreen::BtSplashScreen(QScreen* parent) :
   QSplashScreen(parent, QPixmap(":/images/brewtarget.svg"))
{
   setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
   showMessage(tr("Loading..."));
}
#endif

void BtSplashScreen::showMessage(QString const& message)
{
   QSplashScreen::showMessage(message, Qt::AlignLeft, Qt::white);
}
