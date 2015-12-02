#include "boiltime.h"
/*!
 * \class BoilTime
 * \author Aidan Roberts
 *
 * \brief Simple class to track remaining boil time
 */
BoilTime::BoilTime(QObject* parent): QObject(parent), time(0), started(false)
{ 
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(decrementTime()));
}

BoilTime::BoilTime(QObject* parent, bool start, int boilTime): QObject(parent), time(boilTime)
{
    started = start;
}

void BoilTime::setBoilTime(int boilTime)
{
    time = boilTime;
}

int BoilTime::getTime()
{
    return time;
}

bool BoilTime::isStarted()
{
    return started;
}

void BoilTime::decrementTime()
{
    if (time == 0)
        emit timesUp();
    else {
    time = time - 1;
    emit BoilTimeChanged();
    }
}

void BoilTime::startTimer()
{
    timer->start();
    started = true;
}

void BoilTime::stopTimer()
{
    timer->stop();
    started = false;
}
