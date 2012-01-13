/*
 * TimerWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _TIMERWIDGET_H
#define   _TIMERWIDGET_H

class TimerWidget;

#include "ui_timerWidget.h"
#include <QWidget>
#include <QTimer>
#include <QString>
#include <QPalette>

#if !defined(NO_PHONON)

 #include <mediaobject.h>
 #include <audiooutput.h>

#endif

/*!
 * \class TimerWidget
 * \author Philip G. Lee
 *
 * Countdown timer that plays sounds and flashes when done.
 */
class TimerWidget : public QWidget, public Ui::timerWidget
{
   Q_OBJECT
public:
   TimerWidget(QWidget* parent=0);
   ~TimerWidget();

   //! \returns text version of the timer display.
   QString getTimerValue();

public slots:
   void setTimer(QString text);
   void setTimer();
   void startStop();
   void subtractOneSecond();
   void endTimer();
   void showChanges();
   void flash();
   void getSound();

private slots:
   /*!
    * Puts another copy of the file on queue to be played.
    */
   void doReplay(qint32 msecToEnd);

signals:
   void timerDone();
   void timerSet(QString text);

private:
   void subtractOneMinute();
   void stopFlashing();

   unsigned int hours;
   unsigned int minutes;
   unsigned int seconds;
   bool start;
   QTimer* timer;
   QTimer* flashTimer;
   QPalette paletteOld, paletteNew;
   #if !defined(NO_PHONON)

    Phonon::MediaObject *mediaObject;
    Phonon::AudioOutput *audioOutput;

   #endif
   bool oldColors;
};

#endif   /* _TIMERWIDGET_H */

