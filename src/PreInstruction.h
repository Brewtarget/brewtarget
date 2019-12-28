/*
 * PreInstruction.h is part of Brewtarget, and is Copyright the following
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

#ifndef _PREINSTRUCTION_H
#define   _PREINSTRUCTION_H

class PreInstruction;

#include <QString>

/*!
 * \class PreInstruction
 * \author Philip G. Lee
 *
 * \brief Simple class to assist the creation of instructions.
 */
class PreInstruction
{
public:
   PreInstruction();
   PreInstruction(const QString& txt, const QString& title, double t);

   friend bool operator<(const PreInstruction& lhs, const PreInstruction& rhs);
   friend bool operator>(const PreInstruction& lhs, const PreInstruction& rhs);

   QString getText();
   QString getTitle();
   double getTime();
private:
   QString text;
   QString title;
   double time;
};

#endif   /* _PREINSTRUCTION_H */

