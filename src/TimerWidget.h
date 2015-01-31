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
#include <QTimer>
#include <QString>
#include <QPalette>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QPushButton>
#include <QLineEdit>
#include <QLCDNumber>
#include <QEvent>

/*!
 * \class TimerWidget
 * \author Philip G. Lee
 *
 * \brief Countdown timer that plays sounds and flashes when done.
 */
class TimerWidget : public QWidget
{
   Q_OBJECT
public:
   TimerWidget(QWidget* parent=0);
   ~TimerWidget();

   //! \returns text version of the timer display.
   QString getTimerValue();

   //! \name Public UI Variables
   //! @{
   QPushButton* pushButton_set;
   QLineEdit* lineEdit;
   QLCDNumber* lcdNumber;
   QPushButton* pushButton_startStop;
   QPushButton* pushButton_sound;
   //! @}

public slots:
   void setTimer(QString text);
   void setTimer();
   void startStop();
   void subtractOneSecond();
   void endTimer();
   void showChanges();
   void flash();
   void getSound();

signals:
   void timerDone();
   void timerSet(QString text);

protected:

   virtual void changeEvent(QEvent* event)
   {
      if(event->type() == QEvent::LanguageChange)
         retranslateUi();
      QWidget::changeEvent(event);
   }

private:
   void subtractOneMinute();
   void stopFlashing();

   void doLayout();
   void retranslateUi();

   unsigned int hours;
   unsigned int minutes;
   unsigned int seconds;
   bool start;
   QTimer* timer;
   QTimer* flashTimer;
   QPalette paletteOld, paletteNew;
   QMediaPlayer* mediaPlayer;
   QMediaPlaylist* playlist;
   bool oldColors;
};

#endif   /* _TIMERWIDGET_H */

