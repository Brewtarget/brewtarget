#include "TimerDialog.h"
#include <QChar>
#include <QFileDialog>
#include <QDir>
#include <QUrl>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "brewtarget.h"
#include <QMessageBox>

TimerDialog::TimerDialog(QWidget *parent, BoilTime* bt) :
    QDialog(parent),
    paletteOld(),
    paletteNew(),
    oldColors(true),
    ui(new Ui::TimerDialog),
    boilTime(bt),
#ifndef NO_QTMULTIMEDIA
         mediaPlayer(new QMediaPlayer(this)),
         playlist(new QMediaPlaylist(mediaPlayer))
#endif
{
    setupUi(this);
    //Default all timers to Boil time
    time = boilTime->getTime();
    connect(boilTime, SIGNAL(BoilTimeChanged()), this, SLOT(decrementTime()));
    started = false;
    stopped = false;
    updateTime();
    setDefualtAlarmSound();
    stopButton->setEnabled(false);
    // Taken from old times
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
}

TimerDialog::~TimerDialog()
{
    delete ui;
}

void TimerDialog::setTime(int t)
{
    //Special case if timer auto created
    if (setTimeBox->value() != t/60)
        setTimeBox->setValue(t/60);

    time = boilTime->getTime() - t;
    stopped = false;
    //timer starts automatically when time is set
    started = true;
    updateTime();
}

void TimerDialog::setNote(QString n)
{
    if (noteEdit->text() == "Notes..." || noteEdit->text() == "")
        noteEdit->setText(n);
    else
        noteEdit->setText(noteEdit->text() + " and " + n);
}

void TimerDialog::setBoil(BoilTime *bt)
{
    boilTime = bt;
}

int TimerDialog::getTime()
{
    return boilTime->getTime() - time;
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
    if (started) {
        if (stopped)
            flash();
        else if (time == 0)
            timesUp();
        else {
            time = time - 1;
            updateTime();
        }
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
    setSound(soundFile);
    // Indicate a sound is loaded
    setSoundButton->setCheckable(true);
    setSoundButton->setChecked(true);
}

void TimerDialog::setDefualtAlarmSound()
{
    QString soundFile = QString("%1sounds/").arg(Brewtarget::getDataDir()) + "beep.ogg";
    setSound(soundFile);
}

void TimerDialog::setSound(QString s)
{
#ifndef NO_QTMULTIMEDIA
   if( !playlist->clear() )
      Brewtarget::logW(playlist->errorString());
   if( !playlist->addMedia(QUrl::fromLocalFile(s)) )
      Brewtarget::logW(playlist->errorString());
   playlist->setCurrentIndex(0);
#endif
}

void TimerDialog::on_setTimeBox_valueChanged(int t)
{
    if (t*60 > boilTime->getTime()) {
        QMessageBox::warning(this, "Error", "Addition time cannot be longer than remaining boil time");
        time = 0;
        started = false;
    } else {
        setTime(t*60);
    }
}

void TimerDialog::timesUp()
{
    if (!stopped) {
        if (this->isHidden())
            this->show();
        this->setFocus();
        startAlarm();
        stopped = true;
    }
    else
        flash();
}

void TimerDialog::flash()
{
    oldColors = ! oldColors;

    if( oldColors )
       timeLCD->setPalette(paletteOld);
    else
       timeLCD->setPalette(paletteNew);

    timeLCD->repaint();
}

void TimerDialog::reset()
{
    time = boilTime->getTime() - (setTimeBox->value() * 60);
    if (stopped)
        stopped = false;
    mediaPlayer->stop();
    updateTime();
}

void TimerDialog::startAlarm()
{
#ifndef NO_QTMULTIMEDIA
   mediaPlayer->play();
#endif
   stopButton->setEnabled(true);
}

void TimerDialog::on_stopButton_clicked()
{
    //stop button
    mediaPlayer->stop();
    stopButton->setEnabled(false);
    started = false;
}
