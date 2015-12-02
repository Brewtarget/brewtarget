#include "TimerDialog.h"
#include <QChar>
#include <QFileDialog>
#include <QDir>
#include <QUrl>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "brewtarget.h"

TimerDialog::TimerDialog(QWidget *parent, BoilTime* bt) :
    QDialog(parent),
    ui(new Ui::TimerDialog),
    boilTime(bt),
#ifndef NO_QTMULTIMEDIA
         mediaPlayer(new QMediaPlayer(this)),
         playlist(new QMediaPlaylist(mediaPlayer))
#endif
{
    setupUi(this);
    time = setTimeBox->value();
    connect(boilTime, SIGNAL(BoilTimeChanged()), this, SLOT(decrementTime()));
    stopped = false;
    updateTime();
}

TimerDialog::~TimerDialog()
{
    delete ui;
}

void TimerDialog::setTime(int t)
{
    time = t;
}

void TimerDialog::setNote(QString n)
{
    if (noteEdit->text() == "")
        noteEdit->setText(n);
    else
        noteEdit->setText(noteEdit->text() + " and " + n);
}

void TimerDialog::setBoil(BoilTime *bt)
{
    boilTime = bt;
}

void TimerDialog::updateTime()
{
    timeLCD->display(timeToString(time));
}

QString TimerDialog::timeToString(int t)
{
    unsigned int seconds = t;
    unsigned int minutes = 0;
    unsigned int hours = 0;
   if (t == 0)
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
       secStr = "0" + QString::number(seconds);
   else
       secStr = QString::number(seconds);
   if (minutes <10)
       minStr = "0" + QString::number(minutes);
   else
       minStr = QString::number(minutes);
   if (hours < 10)
       hourStr = "0" + QString::number(hours);
   else
       hourStr = QString::number(hours);
   return hourStr + ":" + minStr + ":" + secStr;
}

void TimerDialog::decrementTime()
{
    if (stopped)
        flash();
    if (time == 0)
        timeOut();
    else {
        time = time - 1;
        updateTime();
    }
}

void TimerDialog::on_setSoundButton_clicked()
{
    //Taken directly form old timers
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
    setSoundButton->setCheckable(true);
    setSoundButton->setChecked(true);
}

void TimerDialog::on_setTimeBox_valueChanged(int t)
{
    time = boilTime->getTime() - (t * 60);
    updateTime();
}

void TimerDialog::timeOut()
{
    // Do something cool
    stopped = true;
}

void TimerDialog::flash()
{
    //flash LCD
}

void TimerDialog::reset()
{
    time = boilTime->getTime() - (setTimeBox->value() * 60);
    updateTime();
}
