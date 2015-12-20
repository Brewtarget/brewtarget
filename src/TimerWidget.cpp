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
     hours(0),
     minutes(0),
     seconds(0),
     start(true),
     timer(new QTimer(this)),
     flashTimer(new QTimer(this)),
     paletteOld(),
     paletteNew(),
#ifndef NO_QTMULTIMEDIA
     mediaPlayer(new QMediaPlayer(this)),
     playlist(new QMediaPlaylist(mediaPlayer)),
#endif
     oldColors(true)
{
   doLayout();

   // One second between timeouts.
   timer->setInterval(1000);
   flashTimer->setInterval(500);

#ifndef NO_QTMULTIMEDIA
   playlist->setPlaybackMode(QMediaPlaylist::Loop);
   mediaPlayer->setVolume(100);
   mediaPlayer->setPlaylist(playlist);
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
#ifndef NO_QTMULTIMEDIA
   mediaPlayer->stop();
   playlist->clear();
#endif
}

void TimerWidget::doLayout()
{
   QHBoxLayout* hLayout = new QHBoxLayout(this);
      QFrame* frame = new QFrame(this);
         frame->setFrameShape(QFrame::StyledPanel);
         frame->setFrameShadow(QFrame::Raised);
         QVBoxLayout* vLayout = new QVBoxLayout(frame);
            QHBoxLayout* hLayout1 = new QHBoxLayout();
               QSpacerItem* hSpacer1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
               pushButton_set = new QPushButton(frame);
                  pushButton_set->setMinimumSize(QSize(0, 0));
                  pushButton_set->setAutoDefault(true);
                  pushButton_set->setDefault(false);
               lineEdit = new QLineEdit(frame);
               QSpacerItem* hSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
               hLayout1->addItem(hSpacer1);
               hLayout1->addWidget(pushButton_set);
               hLayout1->addWidget(lineEdit);
               hLayout1->addItem(hSpacer2);
            QHBoxLayout* hLayout2 = new QHBoxLayout();
               QSpacerItem* hSpacer3 = new QSpacerItem(17, 20, QSizePolicy::Ignored, QSizePolicy::Minimum);
               lcdNumber = new QLCDNumber(frame);
                  QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                  sizePolicy.setHorizontalStretch(0);
                  sizePolicy.setVerticalStretch(0);
                  sizePolicy.setHeightForWidth(lcdNumber->sizePolicy().hasHeightForWidth());
                  lcdNumber->setSizePolicy(sizePolicy);
                  lcdNumber->setMinimumSize(QSize(170, 40));
                  lcdNumber->setFrameShape(QFrame::WinPanel);
                  lcdNumber->setFrameShadow(QFrame::Raised);
                  lcdNumber->setDigitCount(8);
                  lcdNumber->setSegmentStyle(QLCDNumber::Flat);
               QSpacerItem* hSpacer4 = new QSpacerItem(17, 20, QSizePolicy::Ignored, QSizePolicy::Minimum);
               hLayout2->addItem(hSpacer3);
               hLayout2->addWidget(lcdNumber);
               hLayout2->addItem(hSpacer4);
            QHBoxLayout* hLayout3 = new QHBoxLayout();
               QSpacerItem* hSpacer5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
               pushButton_startStop = new QPushButton(frame);
                  pushButton_startStop->setMinimumSize(QSize(0, 0));
               pushButton_sound = new QPushButton(frame);
                  pushButton_sound->setMinimumSize(QSize(0, 0));
               QSpacerItem* hSpacer6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
               hLayout3->addItem(hSpacer5);
               hLayout3->addWidget(pushButton_startStop);
               hLayout3->addWidget(pushButton_sound);
               hLayout3->addItem(hSpacer6);
         vLayout->addLayout(hLayout1);
         vLayout->addLayout(hLayout2);
         vLayout->addLayout(hLayout3);
      hLayout->addWidget(frame);

   retranslateUi();
}

void TimerWidget::retranslateUi()
{
#ifndef QT_NO_TOOLTIP
   pushButton_set->setToolTip(tr("Set the timer to the specified value"));
   lineEdit->setToolTip(tr("HH:MM:SS"));
   pushButton_startStop->setToolTip(tr("Start/Stop timer"));
   pushButton_sound->setToolTip(tr("Set a sound as the alarm"));
#endif // QT_NO_TOOLTIP

   pushButton_set->setText(tr("Set"));
   pushButton_startStop->setText(tr("Start"));
   pushButton_sound->setText(tr("Sound"));

   lineEdit->setPlaceholderText(tr("HH:MM:SS"));
}

void TimerWidget::getSound()
{
   QDir soundsDir(Brewtarget::getDataDir().canonicalPath() + "/sounds");
   //QDir soundsDir = QString("%1sounds/").arg(Brewtarget::getDataDir());
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
   pushButton_sound->setCheckable(true);
   pushButton_sound->setChecked(true);
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

   lcdNumber->repaint();
}

void TimerWidget::setTimer()
{
#ifndef NO_QTMULTIMEDIA
   mediaPlayer->stop();
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

#ifndef NO_QTMULTIMEDIA
   mediaPlayer->play();
#endif
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
#ifndef NO_QTMULTIMEDIA
      mediaPlayer->stop();
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
