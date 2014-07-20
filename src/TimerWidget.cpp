/*
 * TimerWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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
#include <QFileDialog>
#include <QDir>
#include <QUrl>
#include <iostream>
#include "brewtarget.h"

TimerWidget::TimerWidget(QWidget* parent)
   : QWidget(parent),
     hours(0),
     minutes(0),
     seconds(0),
     start(true),
     timer(new QTimer(this)),
     flashTimer(new QTimer(this)),
     paletteOld(),
     paletteNew(),
#if !defined(NO_PHONON)
     mediaObject(new Phonon::MediaObject(this)),
     audioOutput(new Phonon::AudioOutput(Phonon::MusicCategory, this)),
#endif
     oldColors(true)
{
   setupUi(this);

   timer->setInterval(1000); // One second between timeouts.
   flashTimer->setInterval(500);

   // PlaceholderText only exists in Qt 4.7 or greater.
   //lineEdit->setPlaceholderText( tr("HH:MM:SS") );

#if !defined(NO_PHONON)

   mediaObject->setTransitionTime(0);
   mediaObject->setPrefinishMark(10); // 10 ms.
   Phonon::createPath(mediaObject, audioOutput);

   // The following signal is emitted when we are almost at the end of the sound.
   // The slot re-queues the same song, so we get a loop. 
   // This needs to be inside the ifdef -- connecting the mediaObject outside
   // causes a compile error
   connect( mediaObject, SIGNAL(prefinishMarkReached(qint32)), this, SLOT(doReplay(qint32)) );
   
#endif

   paletteOld = lcdNumber->palette();
   paletteNew = QPalette(paletteOld);
   // Swap colors.
   paletteNew.setColor(QPalette::Active, QPalette::WindowText, paletteOld.color(QPalette::Active, QPalette::Window));
   paletteNew.setColor(QPalette::Active, QPalette::Window, paletteOld.color(QPalette::Active, QPalette::WindowText));

   connect( timer, SIGNAL(timeout()), this, SLOT(subtractOneSecond()) );
   connect( flashTimer, SIGNAL(timeout()), this, SLOT(flash()) );
   connect( this, SIGNAL(timerDone()), this, SLOT(endTimer()) );
   connect( pushButton_set, SIGNAL(clicked()), this, SLOT(setTimer()) );
   connect( pushButton_startStop, SIGNAL(clicked()), this, SLOT(startStop()) );
   connect( pushButton_sound, SIGNAL(clicked()), this, SLOT(getSound()) );

   showChanges();
}

TimerWidget::~TimerWidget()
{
}

void TimerWidget::doReplay(qint32 /*msecToEnd*/)
{
   #if !defined(NO_PHONON)
     mediaObject->enqueue( mediaObject->currentSource() );
   #endif
}

void TimerWidget::getSound()
{
   QDir soundsDir = QString("%1sounds/").arg(Brewtarget::getDataDir());

   #if !defined(NO_PHONON)
   QString soundFile = QFileDialog::getOpenFileName( qobject_cast<QWidget*>(this), tr("Open Sound"), soundsDir.exists() ? soundsDir.canonicalPath() : "", tr("Audio Files (*.wav *.ogg *.mp3 *.aiff)") );
    if (! soundFile.isNull()) {
      mediaObject->clearQueue();
      mediaObject->setCurrentSource(QUrl::fromLocalFile(soundFile));
      pushButton_sound->setCheckable(true); // indicate a sound is loaded
      pushButton_sound->setChecked(true);
    }
   #endif
}

QString TimerWidget::getTimerValue()
{
   return QString("%1:%2:%3").arg(hours,2,10,QChar('0')).arg(minutes,2,10,QChar('0')).arg(seconds,2,10,QChar('0'));
}

void TimerWidget::flash()
{
   oldColors = ! oldColors;

   if( oldColors )
      lcdNumber->setPalette(paletteOld);
   else
      lcdNumber->setPalette(paletteNew);

   // Update doesn't repaint when the window is out of focus...
   //lcdNumber->update();
   lcdNumber->repaint();
}

void TimerWidget::setTimer()
{
   #if !defined(NO_PHONON)
    mediaObject->stop();
   #endif
   stopFlashing();

   setTimer(lineEdit->text());
   emit timerSet(getTimerValue());
}

void TimerWidget::stopFlashing()
{
   flashTimer->stop();
   lcdNumber->setPalette(paletteOld);
   lcdNumber->update();
}

void TimerWidget::endTimer()
{
   timer->stop();
   flashTimer->start();

   #if !defined(NO_PHONON)
    mediaObject->play();
   #endif

   //pushButton_startStop->setText("Start");
   //start = true;
}

void TimerWidget::setTimer(QString text)
{
   QStringList strList = text.split(":", QString::SkipEmptyParts);
   bool conversionOk = true;

   if( strList.size() == 1 )
   {
      seconds = strList[0].toUInt(&conversionOk);
      if( ! conversionOk )
         seconds = 0;

      hours = 0;
      minutes = 0;
   }
   else if( strList.size() == 2 )
   {
      minutes = strList[0].toUInt(&conversionOk);
      if( ! conversionOk )
         minutes = 0;
      seconds = strList[1].toUInt(&conversionOk);
      if( ! conversionOk )
         seconds = 0;

      hours = 0;
   }
   else if( strList.size() == 3 )
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
      pushButton_startStop->setText(tr("Stop"));
      start = false;
   }
   else
   {
      timer->stop();
#if !defined(NO_PHONON)
      mediaObject->stop();
#endif
      stopFlashing();
      pushButton_startStop->setText(tr("Start"));
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
