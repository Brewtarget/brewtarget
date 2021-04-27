/*
 * PreInstruction.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include "PreInstruction.h"

bool operator<(const PreInstruction& lhs, const PreInstruction& rhs)
{
   return lhs.time < rhs.time;
}

bool operator>(const PreInstruction& lhs, const PreInstruction& rhs)
{
   return lhs.time > rhs.time;
}

PreInstruction::PreInstruction()
{
   text = "";
   time = 0;
}

PreInstruction::PreInstruction(const QString& txt, const QString& ti, double t)
{
   text = QString(txt);
   title = QString(ti);
   time = t;
}

QString PreInstruction::getText()
{
   return text;
}

QString PreInstruction::getTitle()
{
   return title;
}

double PreInstruction::getTime()
{
   return time;
}
