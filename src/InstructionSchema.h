/*
 * InstructionTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the instructione that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INSTRUCTIONTABLESCHEMA_H
#define _INSTRUCTIONTABLESCHEMA_H

#include <QString>
// Columns for the instruction table
static const QString kcolInstructionDirections("directions");
static const QString kcolInstructionHasTimer("hastimer");
static const QString kcolInstructionTimerValue("timervalue");
static const QString kcolInstructionCompleted("completed");
static const QString kcolInstructionInterval("interval");

static const QString kpropDirections("directions");
static const QString kpropHasTimer("hasTimer");
static const QString kpropTimerValue("timerValue");
static const QString kpropCompleted("completed");
static const QString kpropInterval("interval");

static const QString kxmlPropDirections("directions");
static const QString kxmlPropHasTimer("hasTimer");
static const QString kxmlPropTimerValue("timervalue");
static const QString kxmlPropCompleted("completed");
static const QString kxmlPropInterval("interval");


// small cheat here. InstructionInRecipe tables have a spare column. Rather
// than define a unique header file, I am including it here.
static const QString kpropInstructionNumber("instruction_number");
static const QString kcolInstructionNumber("instruction_number");
#endif // _INSTRUCTIONTABLESCHEMA_H
