/*
 * SaltTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the yeaste that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SALTSCHEMA_H
#define _SALTSCHEMA_H

#include <QString>
// Columns for the yeast table
// What isn't here (like name) is defined in TableSchemaConstants
static const QString kcolSaltType("stype");
static const QString kcolSaltAddTo("addTo");
static const QString kcolSaltAmtIsWgt("amount_is_weight");
static const QString kcolSaltPctAcid("percent_acid");
static const QString kcolSaltIsAcid("is_acid");

// properties for objects
static const QString kpropAddTo("addTo");
static const QString kpropPctAcid("percentAcid");
static const QString kpropIsAcid("isAcid");


// XML properties

#endif // _SALTSCHEMA_H
