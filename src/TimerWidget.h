/*
 * TimerWidget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Eric Tamme <etamme@gmail.com>
 * - Julein <j2bweb@gmail.com>
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

#ifndef _TIMERWIDGET_H
#define _TIMERWIDGET_H

#include <QWidget>
#include <QString>
#include <QPalette>
#ifndef NO_QTMULTIMEDIA
#include <QMediaPlayer>
#include <QMediaPlaylist>
#endif
#include <QPushButton>
#include <QLineEdit>
#include <QLCDNumber>
#include <QEvent>
#include "ui_timerWidget.h"
#include "boiltime.h"

/*!
 * \class TimerWidget
 * \author Aidan Roberts
 *
 * \brief Multiple timers, generated from recipe
 */

class TimerWidget : public QWidget, public Ui::timerWidget
{
   Q_OBJECT

public:
   TimerWidget(QWidget* parent=0);
   ~TimerWidget();
   void setBoil(BoilTime* bt);

public slots:
   void flash();
   void getSound();

signals:
   void timerDone();

protected:

   virtual void changeEvent(QEvent* event)
   {
      if(event->type() == QEvent::LanguageChange)
         retranslateUi();
      QWidget::changeEvent(event);
   }

private slots:
   void on_setAlarmSoundButton_clicked();
   void on_setTimeBox_valueChanged(int arg1);

private:
   BoilTime* boilTime;
   void setTimer();
   void endTimer();
   QString getTimerValue();
   void stopFlashing();
   void retranslateUi();
   void updateTime();
   QString timeToString(int t);
   void decrementTime();

   unsigned int time;
   QPalette paletteOld, paletteNew;
#ifndef NO_QTMULTIMEDIA
   QMediaPlayer* mediaPlayer;
   QMediaPlaylist* playlist;
#endif
   bool oldColors;
};

#endif   /* _TIMERWIDGET_H */

