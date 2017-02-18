/*
 * boiltime.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Aidan Roberts <aidanr67@gmail.com>
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

#include "boiltime.h"

BoilTime::BoilTime(QObject* parent): QObject(parent),
    time(0),
    started(false),
    completed(false)
{ 
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, &BoilTime::decrementTime);
}

void BoilTime::setBoilTime(int boilTime)
{
    time = boilTime;
    if (completed)
        completed = false;
}

int BoilTime::getTime()
{
    return time;
}

bool BoilTime::isStarted()
{
    return started;
}

bool BoilTime::isCompleted()
{
    return completed;
}

void BoilTime::decrementTime()
{
    if (time == 0) {
        emit timesUp();
        completed = true;
    }
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
