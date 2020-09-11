/*
 * MiscTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the misce that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QString>

#ifndef _MISCTABLESCHEMA_H
#define _MISCTABLESCHEMA_H
// Columns for the misc table
// Everything else is globally defined. A little depressing, actually
static const QString kcolMiscType("mtype");
static const QString kcolMiscAmtIsWgt("amount_is_weight");
static const QString kcolMiscUseFor("use_for");

static const QString kpropUseFor("useFor");
static const QString kpropTypeStr("typeString");
static const QString kpropMiscTime("time");

static const QString kxmlPropUseFor("USE_FOR");
#endif // _MISCTABLESCHEMA_H
