/*
 * BrewnoteSchema.h is part of Brewtarget, and is Copyright the following
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

#ifndef _BREWNOTETABLESCHEMA_H
#define _BREWNOTETABLESCHEMA_H

// I am putting this here in the vain hopes I do not forget about it. There
// are two mystery columns defined in the DB for this table: predicted_abv and
// projected_fin_temp. I have no idea what they were meant for, but they are
// not exposed anywhere that I can find.
// They should be removed from the db
// Columns for the brewnote table
static const QString kcolBrewnoteBrewDate("brewDate");
static const QString kcolBrewnoteAttenuation("attenuation");
static const QString kcolBrewnoteFermentDate("fermentDate");
static const QString kcolBrewnoteNotes("notes");
static const QString kcolBrewnoteSpecificGravity("sg");
static const QString kcolBrewnoteVolumeIntoBoil("volume_into_bk");
static const QString kcolBrewnoteOriginalGravity("og");
static const QString kcolBrewnoteVolumeIntoFermenter("volume_into_fermenter");
static const QString kcolBrewnoteFinalGravity("fg");
static const QString kcolBrewnoteABV("abv");
static const QString kcolBrewnoteEfficiencyIntoBoil("eff_into_bk");
static const QString kcolBrewnoteBrewhouseEfficiency("brewhouse_eff");
static const QString kcolBrewnoteStrikeTemp("strike_temp");
static const QString kcolBrewnoteMashFinalTemp("mash_final_temp");
static const QString kcolBrewnotePostBoilVolume("post_boil_volume");
static const QString kcolBrewnotePitchTemp("pitch_temp");
static const QString kcolBrewnoteFinalVolume("final_volume");
static const QString kcolBrewnoteBoilOff("boil_off");

static const QString kcolBrewnoteProjectedBoilGravity("projected_boil_grav");
static const QString kcolBrewnoteProjectedVolumeIntoBoil("projected_vol_into_bk");
static const QString kcolBrewnoteProjectedStrikeTemp("projected_strike_temp");
static const QString kcolBrewnoteProjectedMashFinishTemp("projected_mash_fin_temp");
static const QString kcolBrewnoteProjectedOG("projected_og");
static const QString kcolBrewnoteProjectedVolumeIntoFermenter("projected_vol_into_ferm");
static const QString kcolBrewnoteProjectedFG("projected_fg");
static const QString kcolBrewnoteProjectedEfficiency("projected_eff");
static const QString kcolBrewnoteProjectedABV("projected_abv");
static const QString kcolBrewnoteProjectedAttenuation("projected_atten");
static const QString kcolBrewnoteProjectedPoints("projected_points");
static const QString kcolBrewnoteProjectedFermentationPoints("projected_ferm_points");

static const QString kcolBrewnoteRecipeId("recipe_id");

// Properties
static const QString kpropBrewDate("brewDate");
static const QString kpropFermentDate("fermentDate");
static const QString kpropSpecificGravity("sg");
static const QString kpropOriginalGravity("og");
static const QString kpropFinalGravity("fg");
static const QString kpropABV("abv");
static const QString kpropAttenuation("attenuation");
static const QString kpropEfficiencyIntoBoil("effIntoBK_pct");
static const QString kpropStrikeTemp("strikeTemp_c");
static const QString kpropMashFinalTemp("mashFinTemp_c");
static const QString kpropPitchTemp("pitchTemp_c");
static const QString kpropFinalVolume("finalVolume_l");
static const QString kpropBrewhouseEfficiency("brewhouseEff_pct");
static const QString kpropVolumeIntoBoil("volumeIntoBK_l");
static const QString kpropVolumeIntoFermenter("volumeIntoFerm_l");
static const QString kpropBoilOff("boilOff_l");

static const QString kpropProjectedStrikeTemp("projStrikeTemp_c");
static const QString kpropProjectedMashFinishTemp("projMashFinTemp_c");
static const QString kpropProjectedAttenuation("projAtten");
static const QString kpropProjectedABV("projABV_pct");
static const QString kpropProjectedEfficiency("projEff_pct");
static const QString kpropProjectedFG("projFg");
static const QString kpropProjectedOG("projOg");
static const QString kpropProjectedPoints("projPoints");
static const QString kpropProjectedFermentationPoints("projFermPoints");
static const QString kpropProjectedBoilGravity("projBoilGrav");
static const QString kpropPostBoilVolume("postBoilVolume_l");
static const QString kpropProjectedVolumeIntoBoil("projVolIntoBK_l");
static const QString kpropProjectedVolumeIntoFermenter("projVolIntoFerm_l");

static const QString kpropRecipeId("recipe_id");

// finally, XML props
static const QString kxmlPropBrewDate("BREWDATE");
static const QString kxmlPropFermentDate("DATE_FERMENTED_OUT");
static const QString kxmlPropSpecificGravity("SG");
static const QString kxmlPropVolumeIntoBoil("VOLUME_INTO_BK");
static const QString kxmlPropStrikeTemp("STRIKE_TEMP");
static const QString kxmlPropMashFinalTemp("MASH_FINAL_TEMP");
static const QString kxmlPropOriginalGravity("OG");
static const QString kxmlPropPostBoilVolume("POST_BOIL_VOLUME");
static const QString kxmlPropVolumeIntoFermenter("VOLUME_INTO_FERMENTER");
static const QString kxmlPropPitchTemp("PITCH_TEMP");
static const QString kxmlPropFinalGravity("FG");
static const QString kxmlPropEfficiencyIntoBoil("EFF_INTO_BK");
static const QString kxmlPropBrewhouseEfficiency("BREWHOUSE_EFF");
static const QString kxmlPropABV("ACTUAL_ABV");
static const QString kxmlPropProjectedBoilGravity("PROJECTED_BOIL_GRAV");
static const QString kxmlPropProjectedStrikeTemp("PROJECTED_STRIKE_TEMP");
static const QString kxmlPropProjectedMashFinishTemp("PROJECTED_MASH_FIN_TEMP");
static const QString kxmlPropProjectedVolumeIntoBoil("PROJECTED_VOL_INTO_BK");
static const QString kxmlPropProjectedOG("PROJECTED_OG");
static const QString kxmlPropProjectedVolumeIntoFermenter("PROJECTED_VOL_INTO_FERM");
static const QString kxmlPropProjectedFG("PROJECTED_FG");
static const QString kxmlPropProjectedEfficiency("PROJECTED_EFF");
static const QString kxmlPropProjectedABV("PROJECTED_ABV");
static const QString kxmlPropProjectedPoints("PROJECTED_POINTS");
static const QString kxmlPropProjectedFermentationPoints("PROJECTED_FERM_POINTS");
static const QString kxmlPropProjectedAttenuation("PROJECTED_ATTEN");
static const QString kxmlPropBoilOff("BOIL_OFF");
static const QString kxmlPropFinalVolume("FINAL_VOLUME");

#endif // _BREWNOTETABLESCHEMA_H
