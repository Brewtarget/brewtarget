/*======================================================================================================================
 * BoilTimer.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Aidan Roberts <aidanr67@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#include "BoilTimer.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BoilTimer.cpp"
#endif

BoilTimer::BoilTimer(QObject* parent) :
    QObject{parent},
    m_timer{new QTimer(this)},
    m_time(0),
    m_started(false),
    m_completed(false) {
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &BoilTimer::decrementTime);
    return;
}

void BoilTimer::setBoilTime(unsigned int boilTime) {
    m_time = boilTime;
    if (m_completed) {
        m_completed = false;
    }
    return;
}

int BoilTimer::getTime() {
    return m_time;
}

bool BoilTimer::isStarted() {
    return m_started;
}

bool BoilTimer::isCompleted() {
    return m_completed;
}

void BoilTimer::decrementTime() {
    if (m_time == 0) {
        emit timesUp();
        m_completed = true;
    } else {
        --m_time;
        emit BoilTimeChanged();
    }
    return;
}

void BoilTimer::startTimer() {
    m_timer->start();
    m_started = true;
    return;
}

void BoilTimer::stopTimer() {
    m_timer->stop();
    m_started = false;
    return;
}