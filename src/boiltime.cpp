#include "boiltime.h"
/*!
 * \class BoilTime
 * \author Aidan Roberts
 *
 * \brief Simple class to track remaining boil time
 */
BoilTime::BoilTime(QObject* parent): QObject(parent), time(0), started(false)
{ 
}

BoilTime::BoilTime(QObject* parent, bool start, int boilTime): QObject(parent), time(boilTime)
{
    started = start;
}

void BoilTime::setBoilStarted(bool start)
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
    time = time - 1;
    emit BoilTimeChanged();
}
