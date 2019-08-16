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
static QString kcolBoilSize("boil_size");
static QString kcolBatchSize("batch_size");
static QString kcolTunVolume("tun_volume");
static QString kcolTunWeight("tun_weight");
static QString kcolTunSpecificHeat("tun_specific_heat");
static QString kcolTopUpWater("top_up_water");
static QString kcolTrubChillerLoss("trub_chiller_loss");
static QString kcolEvapRate("evap_rate");
static QString kcolBoilTime("boil_time");
static QString kcolCalcBoilVolume("calc_boil_volume");
static QString kcolLauterDeadspace("lauter_deadspace");
static QString kcolTopUpKettle("top_up_kettle");
static QString kcolHopUtilization("hop_utilization");
static QString kcolRealEvapRate("real_evap_rate");
static QString kcolBoilingPoint("boiling_point");
static QString kcolAbsorption("absorption");

// Prop names
// These are defined in the main constants file
// const QString kpropName("name");
// const QString kpropNotes("notes");
const QString kpropBoilSize("boilSize_l");
const QString kpropBatchSize("batchSize_l");
const QString kpropTunVolume("tunVolume_l");
const QString kpropTopUpWater("topUpWater_l");
const QString kpropTrubChillerLoss("trubChillerLoss_l");
const QString kpropEvaporationRate("evapRate_pctHr");
const QString kpropRealEvaporationRate("evapRate_lHr");
const QString kpropBoilTime("boilTime_min");
const QString kpropCalcBoilVolume("calcBoilVolume");
const QString kpropLauterDeadspace("lauterDeadspace_l");
const QString kpropTopUpKettle("topUpKettle_l");
const QString kpropHopUtilization("hopUtilization_pct");
const QString kpropGrainAbsorption("grainAbsorption_LKg");
const QString kpropAbsorption("absorption_LKg");
const QString kpropBoilingPoint("boilingPoint_c");

// XML prop names
// const QString kXmlNameProp("NAME");
// const QString kXmlNotesProp("NOTES");
const QString kxmlPropBoilSize("BOIL_SIZE");
const QString kxmlPropBatchSize("BATCH_SIZE");
const QString kxmlPropTunVolume("TUN_VOLUME");
const QString kxmlPropTopUpWater("TOP_UP_WATER");
const QString kxmlPropTrubChillerLoss("TRUB_CHILLER_LOSS");
const QString kxmlPropEvaporationRate("EVAP_RATE");
const QString kxmlPropRealEvaporationRate("REAL_EVAP_RATE");
const QString kxmlPropBoilTime("BOIL_TIME");
const QString kxmlPropCalcBoilVolume("CALC_BOIL_VOLUME");
const QString kxmlPropLauterDeadspace("LAUTER_DEADSPACE");
const QString kxmlPropTopUpKettle("TOP_UP_KETTLE");
const QString kxmlPropHopUtilization("HOP_UTILIZATION");
const QString kxmlPropGrainAbsorption("ABSORPTION");
const QString kxmlPropBoilingPoint("BOILING_POINT");

#endif // define _EQUIPTABLESCHEMA_H
