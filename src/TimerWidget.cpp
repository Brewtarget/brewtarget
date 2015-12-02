/*
 * TimerWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Julein <j2bweb@gmail.com>
 * - Maxime Lavigne (malavv) <duguigne@gmail.com>
 * - mik firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - przybysh
 * - Ted Wright
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

#include "TimerWidget.h"
#include <QStringList>
#include <QChar>
#include <QFileDialog>
#include <QDir>
#include <QUrl>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "brewtarget.h"

TimerWidget::TimerWidget(QWidget* parent)
   : QWidget(parent),
     time(0),
     paletteOld(),
     paletteNew(),
#ifndef NO_QTMULTIMEDIA
     mediaPlayer(new QMediaPlayer(this)),
     playlist(new QMediaPlaylist(mediaPlayer)),
#endif
      oldColors(true)
{
#ifndef NO_QTMULTIMEDIA
   playlist->setPlaybackMode(QMediaPlaylist::Loop);
   mediaPlayer->setVolume(100);
   mediaPlayer->setPlaylist(playlist);
#endif
   paletteOld = timeLCD->palette();
   paletteNew = QPalette(paletteOld);
   // Swap colors.
   paletteNew.setColor(QPalette::Active, QPalette::WindowText, paletteOld.color(QPalette::Active, QPalette::Window));
   paletteNew.setColor(QPalette::Active, QPalette::Window, paletteOld.color(QPalette::Active, QPalette::WindowText));
   time = setTimeBox->value();
   connect(boilTime, SIGNAL(BoilTimeChanged()), this, SLOT(decrementTime()));

   updateTime();
}

TimerWidget::~TimerWidget()
{
#ifndef NO_QTMULTIMEDIA
   mediaPlayer->stop();
   playlist->clear();
#endif
}

void TimerWidget::setBoil(BoilTime* bt)
{
    boilTime = bt;
}

void TimerWidget::retranslateUi()
{
/*#ifndef QT_NO_TOOLTIP
   pushButton_set->setToolTip(tr("Set the timer to the specified value"));
   lineEdit->setToolTip(tr("HH:MM:SS"));
   pushButton_startStop->setToolTip(tr("Start/Stop timer"));
   pushButton_sound->setToolTip(tr("Set a sound as the alarm"));
#endif // QT_NO_TOOLTIP

   pushButton_set->setText(tr("Set"));
   pushButton_startStop->setText(tr("Start"));
   pushButton_sound->setText(tr("Sound"));

   lineEdit->setPlaceholderText(tr("HH:MM:SS"));
*/}
void TimerWidget::getSound()
{
   QDir soundsDir = QString("%1sounds/").arg(Brewtarget::getDataDir());
   QString soundFile = QFileDialog::getOpenFileName( qobject_cast<QWidget*>(this), tr("Open Sound"), soundsDir.exists() ? soundsDir.canonicalPath() : "", tr("Audio Files (*.wav *.ogg *.mp3 *.aiff)") );

   if( soundFile.isNull() )
   {
      Brewtarget::logW("Null sound file.");
      return;
   }
#ifndef NO_QTMULTIMEDIA
   if( !playlist->clear() )
      Brewtarget::logW(playlist->errorString());
   if( !playlist->addMedia(QUrl::fromLocalFile(soundFile)) )
      Brewtarget::logW(playlist->errorString());
   playlist->setCurrentIndex(0);
#endif
   // Indicate a sound is loaded
   setAlarmSoundButton->setCheckable(true);
   setAlarmSoundButton->setChecked(true);
}

QString TimerWidget::getTimerValue()
{
    return QString::number(timeLCD->value());
}

void TimerWidget::flash()
{
   oldColors = ! oldColors;

   if( oldColors )
       timeLCD->setPalette(paletteOld);
   else
      timeLCD->setPalette(paletteNew);

   timeLCD->repaint();
}

void TimerWidget::setTimer()
{
#ifndef NO_QTMULTIMEDIA
   mediaPlayer->stop();
#endif
   stopFlashing();
}

void TimerWidget::stopFlashing()
{
   timeLCD->setPalette(paletteOld);
   timeLCD->update();
}

void TimerWidget::endTimer()
{
#ifndef NO_QTMULTIMEDIA
   mediaPlayer->play();
#endif
}

void TimerWidget::on_setAlarmSoundButton_clicked()
{
    getSound();
}

void TimerWidget::on_setTimeBox_valueChanged(int t)
{
    time = t;
}

void TimerWidget::decrementTime()
{
    time = time -1;
    updateTime();
}

void TimerWidget::updateTime()
{
    timeLCD->display(timeToString(time));
}

QString TimerWidget::timeToString(int t)
{
    int seconds = 0;
    int minutes = 0;
    int hours = 0;
   if (t <= 0)
       return "00:00:00";
   if (t > 59){
       seconds = t%60;
       minutes = t/60;
       if (minutes > 59){
          hours = minutes/60;
          minutes = minutes%60;
       }
   }
   QString secStr, minStr, hourStr;
   if (seconds < 10)
       secStr = "0" + seconds;
   else
       secStr = seconds;
   if (minutes <10)
       minStr = "0" + minutes;
   else
       minStr = minutes;
   if (hours < 10)
       hourStr = "0" + hours;
   else
       hourStr = hours;
   return hourStr + ":" + minStr + ":" + secStr;
}

