/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * LatestReleaseFinder.h is part of Brewtarget, and is copyright the following authors 2024:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef LATESTRELEASEFINDER_H
#define LATESTRELEASEFINDER_H
#pragma once

#include <QObject>
#include <QVersionNumber>

/**
 * \class LatestReleaseFinder
 *
 *        This class runs on a background thread, just after application start-up.  On receiving the
 *        \c checkForNewVersion signal, it makes a simple HTTP request to the latest version of the program available at
 *        its main GitHub repository.  If this succeeds, it then sends a \c newVersionFound signal to the main code so
 *        that it can tell the user.
 *
 *        It would be possible to have done this in slightly less code by inheriting from \c QThread and overriding
 *        \c QThread::run (as suggested as an alternate approach at https://doc.qt.io/qt-6/qthread.html#details).
 *        However, that feels like a "wrong" design decision in that it tightly couples "code we want to run on a
 *        thread" with "mechanism for instantiating and running a thread".  So, instead, we keep \c QThread separate,
 *        and use signals and slots to kick off some work and to communicate its results.  It's very slightly more code,
 *        but it feels less clunky.
 *
 *        For the moment at least, this is the only bit of HTTP that the program does, so we haven't made things more
 *        generic -- eg by writing an HttpRequester object or some such.
 */
class LatestReleaseFinder : public QObject {
   Q_OBJECT

public slots:
   void checkMainRespository();

signals:
   void foundLatestRelease(QVersionNumber const latestRelease);
};

#endif
