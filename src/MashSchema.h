/*
 * MashTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the mashe that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MASHTABLESCHEMA_H
#define _MASHTABLESCHEMA_H

// Columns for the mash table
static const QString kcolMashGrainTemp("grain_temp");
static const QString kcolMashTunTemp("tun_temp");
static const QString kcolMashSpargeTemp("sparge_temp");
static const QString kcolMashTunWeight("tun_weight");
static const QString kcolMashTunSpecHeat("tun_specific_heat");
static const QString kcolMashEquipAdjust("equip_adjust");

static const QString kpropGrainTemp("grainTemp_c");
static const QString kpropTunTemp("tunTemp_c");
static const QString kpropSpargeTemp("spargeTemp_c");
static const QString kpropEquipAdjust("equipAdjust");

static const QString kxmlPropGrainTemp("GRAIN_TEMP");
static const QString kxmlPropTunTemp("TUN_TEMP");
static const QString kxmlPropSpargeTemp("SPARGE_TEMP");
static const QString kxmlPropEquipAdjust("EQUIP_ADJUST");
#endif // _MASHTABLESCHEMA_H
