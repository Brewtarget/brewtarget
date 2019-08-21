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
// const QString kcolName("name");
// const QString kcolNotes("notes");
static const QString kcolBoilSize("boil_size");
static const QString kcolBatchSize("batch_size");
static const QString kcolTunVolume("tun_volume");
static const QString kcolTunWeight("tun_weight");
static const QString kcolTunSpecificHeat("tun_specific_heat");
static const QString kcolTopUpWater("top_up_water");
static const QString kcolTrubChillerLoss("trub_chiller_loss");
static const QString kcolEvapRate("evap_rate");
static const QString kcolBoilTime("boil_time");
static const QString kcolCalcBoilVolume("calc_boil_volume");
static const QString kcolLauterDeadspace("lauter_deadspace");
static const QString kcolTopUpKettle("top_up_kettle");
static const QString kcolHopUtilization("hop_utilization");
static const QString kcolRealEvapRate("real_evap_rate");
static const QString kcolBoilingPoint("boiling_point");
static const QString kcolAbsorption("absorption");

// Prop names
// These are defined in the main constants file
// const QString kpropName("name");
// const QString kpropNotes("notes");
static const QString kpropTunVolume("tunVolume_l");
static const QString kpropTopUpWater("topUpWater_l");
static const QString kpropTrubChillerLoss("trubChillerLoss_l");
static const QString kpropEvaporationRate("evapRate_pctHr");
static const QString kpropRealEvaporationRate("evapRate_lHr");
static const QString kpropCalcBoilVolume("calcBoilVolume");
static const QString kpropLauterDeadspace("lauterDeadspace_l");
static const QString kpropTopUpKettle("topUpKettle_l");
static const QString kpropHopUtilization("hopUtilization_pct");
static const QString kpropGrainAbsorption("grainAbsorption_LKg");
static const QString kpropAbsorption("absorption_LKg");
static const QString kpropBoilingPoint("boilingPoint_c");

// XML prop names
// const QString kXmlNameProp("NAME");
// const QString kXmlNotesProp("NOTES");
static const QString kxmlPropTunVolume("TUN_VOLUME");
static const QString kxmlPropTopUpWater("TOP_UP_WATER");
static const QString kxmlPropTrubChillerLoss("TRUB_CHILLER_LOSS");
static const QString kxmlPropEvaporationRate("EVAP_RATE");
static const QString kxmlPropRealEvaporationRate("REAL_EVAP_RATE");
static const QString kxmlPropCalcBoilVolume("CALC_BOIL_VOLUME");
static const QString kxmlPropLauterDeadspace("LAUTER_DEADSPACE");
static const QString kxmlPropTopUpKettle("TOP_UP_KETTLE");
static const QString kxmlPropHopUtilization("HOP_UTILIZATION");
static const QString kxmlPropGrainAbsorption("ABSORPTION");
static const QString kxmlPropBoilingPoint("BOILING_POINT");

#endif // define _EQUIPTABLESCHEMA_H
