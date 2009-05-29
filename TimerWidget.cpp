/*
 * TimerWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "TimerWidget.h"
#include <QStringList>
#include <QChar>

TimerWidget::TimerWidget(QWidget* parent) : QWidget(parent)
{
   setupUi(this);
   
   hours = 0;
   minutes = 0;
   seconds = 0;
   start = true;
   timer = new QTimer();
   timer->setInterval(1000); // One second between timeouts.

   connect( timer, SIGNAL(timeout()), this, SLOT(subtractOneSecond()) );
   connect( this, SIGNAL(timerDone()), this, SLOT(endTimer()) );
   connect( pushButton_set, SIGNAL(clicked()), this, SLOT(setTimer()) );
   connect( pushButton_startStop, SIGNAL(clicked()), this, SLOT(startStop()) );

   showChanges();
}

QString TimerWidget::getTimerValue()
{
   return QString("%1:%2:%3").arg(hours,2,10,QChar('0')).arg(minutes,2,10,QChar('0')).arg(seconds,2,10,QChar('0'));
}

void TimerWidget::setTimer()
{
   setTimer(lineEdit->text());
   emit timerSet(getTimerValue());
}

void TimerWidget::endTimer()
{
   timer->stop();

   pushButton_startStop->setText("Start");
   start = true;
}

void TimerWidget::setTimer(QString text)
{
   QStringList strList = text.split(":", QString::SkipEmptyParts);
   bool conversionOk = true;

   if( strList.length() == 1 )
   {
      seconds = strList[0].toUInt(&conversionOk);
      if( ! conversionOk )
         seconds = 0;

      hours = 0;
      minutes = 0;
   }
   else if( strList.length() == 2 )
   {
      minutes = strList[0].toUInt(&conversionOk);
      if( ! conversionOk )
         minutes = 0;
      seconds = strList[1].toUInt(&conversionOk);
      if( ! conversionOk )
         seconds = 0;

      hours = 0;
   }
   else if( strList.length() == 3 )
   {
      hours = strList[0].toUInt(&conversionOk);
      if( ! conversionOk )
         hours = 0;
      minutes = strList[1].toUInt(&conversionOk);
      if( ! conversionOk )
         minutes = 0;
      seconds = strList[2].toUInt(&conversionOk);
      if( ! conversionOk )
         seconds = 0;
   }
   else
   {
      hours = 0; minutes = 0; seconds = 0;
   }

   if( seconds >= 60 )
   {
      minutes += seconds/(unsigned int)60;
      seconds = seconds % 60;
   }
   if( minutes >= 60 )
   {
      hours += minutes/(unsigned int)60;
      minutes = minutes % 60;
   }

   showChanges();
}

void TimerWidget::startStop()
{
   if( start )
   {
      timer->start();
      pushButton_startStop->setText("Stop");
      start = false;
   }
   else
   {
      timer->stop();
      pushButton_startStop->setText("Start");
      start = true;
   }
}

void TimerWidget::subtractOneSecond()
{
   if( seconds == 0 )
   {
      if( minutes == 0 && hours == 0 )
         emit timerDone();
      else
      {
         subtractOneMinute();
         seconds = 59;
      }
   }
   else
      seconds--;

   showChanges();
}

void TimerWidget::subtractOneMinute()
{
   if( minutes == 0 )
   {
      hours--;
      minutes = 59;
   }
   else
      minutes--;
}

void TimerWidget::showChanges()
{
   lcdNumber->display(getTimerValue());
}