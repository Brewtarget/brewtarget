/*
 * EquipmentTableSchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
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

#ifndef _EQUIPTABLESCHEMA_H
#define _EQUIPTABLESCHEMA_H

// Column names
static const QString kcolEquipBoilSize("boil_size");
static const QString kcolEquipBatchSize("batch_size");
static const QString kcolEquipTunVolume("tun_volume");
static const QString kcolEquipTunWeight("tun_weight");
static const QString kcolEquipTunSpecHeat("tun_specific_heat");
static const QString kcolEquipTopUpWater("top_up_water");
static const QString kcolEquipTrubChillLoss("trub_chiller_loss");
static const QString kcolEquipEvapRate("evap_rate");
static const QString kcolEquipBoilTime("boil_time");
static const QString kcolEquipCalcBoilVol("calc_boil_volume");
static const QString kcolEquipLauterSpace("lauter_deadspace");
static const QString kcolEquipTopUpKettle("top_up_kettle");
static const QString kcolEquipHopUtil("hop_utilization");
static const QString kcolEquipRealEvapRate("real_evap_rate");
static const QString kcolEquipBoilingPoint("boiling_point");
static const QString kcolEquipAbsorption("absorption");

// Prop names
// These are defined in the main constants file
// const QString kpropName("name");
// const QString kpropNotes("notes");
static const QString kpropTunVolume("tunVolume_l");
static const QString kpropTopUpWater("topUpWater_l");
static const QString kpropTrubChillLoss("trubChillerLoss_l");
static const QString kpropEvapRate("evapRate_pctHr");
static const QString kpropRealEvapRate("evapRate_lHr");
static const QString kpropCalcBoilVol("calcBoilVolume");
static const QString kpropLauterSpace("lauterDeadspace_l");
static const QString kpropTopUpKettle("topUpKettle_l");
static const QString kpropHopUtil("hopUtilization_pct");
static const QString kpropAbsorption("grainAbsorption_LKg");
static const QString kpropBoilingPoint("boilingPoint_c");

// XML prop names
// const QString kXmlNameProp("NAME");
// const QString kXmlNotesProp("NOTES");
static const QString kxmlPropTunVolume("TUN_VOLUME");
static const QString kxmlPropTopUpWater("TOP_UP_WATER");
static const QString kxmlPropTrubChillLoss("TRUB_CHILLER_LOSS");
static const QString kxmlPropEvapRate("EVAP_RATE");
static const QString kxmlPropRealEvapRate("REAL_EVAP_RATE");
static const QString kxmlPropCalcBoilVol("CALC_BOIL_VOLUME");
static const QString kxmlPropLauterSpace("LAUTER_DEADSPACE");
static const QString kxmlPropTopUpKettle("TOP_UP_KETTLE");
static const QString kxmlPropHopUtil("HOP_UTILIZATION");
static const QString kxmlPropGrainAbsorption("ABSORPTION");
static const QString kxmlPropBoilingPoint("BOILING_POINT");

#endif // define _EQUIPTABLESCHEMA_H
