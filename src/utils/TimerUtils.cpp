/*
 * utils/TimerUtils.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Aidan Roberts <aidanr67@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
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
#include "utils/TimerUtils.h"


QString TimerUtils::timeToString(int t) {
   if (t == 0) {
       return "00:00:00";
   }

   unsigned int seconds = t;
   unsigned int minutes = 0;
   unsigned int hours = 0;
   if (t > 59) {
       seconds = t % 60;
       minutes = t / 60;
       if (minutes > 59) {
          hours = minutes / 60;
          minutes = minutes % 60;
       }
   }
   QString secStr, minStr, hourStr;
   if (seconds < 10) {
       secStr = "0" + QString::number(seconds);
   } else {
       secStr = QString::number(seconds);
   }
   if (minutes < 10) {
       minStr = "0" + QString::number(minutes);
   } else {
       minStr = QString::number(minutes);
   }
   if (hours < 10) {
       hourStr = "0" + QString::number(hours);
   } else {
       hourStr = QString::number(hours);
   }
   return hourStr + ":" + minStr + ":" + secStr;
}
