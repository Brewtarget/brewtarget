/*
 * BNoteSchema.h is part of Brewtarget, and is Copyright the following
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

#include <QString>
// I am putting this here in the vain hopes I do not forget about it. There
// are two mystery columns defined in the DB for this table: predicted_abv and
// projected_fin_temp. I have no idea what they were meant for, but they are
// not exposed anywhere that I can find.
// They should be removed from the db
// Columns for the brewnote table
static const QString kcolBNoteBrewDate("brewdate");
static const QString kcolBNoteAtten("attenuation");
static const QString kcolBNoteFermDate("fermentdate");
static const QString kcolBNoteNotes("notes");
static const QString kcolBNoteSG("sg");
static const QString kcolBNoteVolIntoBoil("volume_into_bk");
static const QString kcolBNoteOG("og");
static const QString kcolBNoteVolIntoFerm("volume_into_fermenter");
static const QString kcolBNoteFG("fg");
static const QString kcolBNoteABV("abv");
static const QString kcolBNoteEffIntoBoil("eff_into_bk");
static const QString kcolBNoteBrewhsEff("brewhouse_eff");
static const QString kcolBNoteStrikeTemp("strike_temp");
static const QString kcolBNoteMashFinTemp("mash_final_temp");
static const QString kcolBNotePostBoilVol("post_boil_volume");
static const QString kcolBNotePitchTemp("pitch_temp");
static const QString kcolBNoteFinVol("final_volume");
static const QString kcolBNoteBoilOff("boil_off");

static const QString kcolBNoteProjBoilGrav("projected_boil_grav");
static const QString kcolBNoteProjVolIntoBoil("projected_vol_into_bk");
static const QString kcolBNoteProjStrikeTemp("projected_strike_temp");
static const QString kcolBNoteProjMashFinTemp("projected_mash_fin_temp");
static const QString kcolBNoteProjFinTemp("projected_fin_temp");
static const QString kcolBNoteProjOG("projected_og");
static const QString kcolBNoteProjVolIntoFerm("projected_vol_into_ferm");
static const QString kcolBNoteProjFG("projected_fg");
static const QString kcolBNoteProjEff("projected_eff");
static const QString kcolBNoteProjABV("projected_abv");
static const QString kcolBNoteProjAtten("projected_atten");
static const QString kcolBNoteProjPnts("projected_points");
static const QString kcolBNoteProjFermPnts("projected_ferm_points");


// Properties
static const QString kpropBrewDate("brewDate");
static const QString kpropFermDate("fermentDate");
static const QString kpropSG("sg");
static const QString kpropABV("abv");
static const QString kpropAtten("attenuation");
static const QString kpropEffIntoBoil("effIntoBK_pct");
static const QString kpropStrikeTemp("strikeTemp_c");
static const QString kpropMashFinTemp("mashFinTemp_c");
static const QString kpropPitchTemp("pitchTemp_c");
static const QString kpropFinVol("finalVolume_l");
static const QString kpropBrewhsEff("brewhouseEff_pct");
static const QString kpropVolIntoBoil("volumeIntoBK_l");
static const QString kpropVolIntoFerm("volumeIntoFerm_l");
static const QString kpropBoilOff("boilOff_l");

static const QString kpropProjStrikeTemp("projStrikeTemp_c");
static const QString kpropProjMashFinTemp("projMashFinTemp_c");
static const QString kpropProjAtten("projAtten");
static const QString kpropProjABV("projABV_pct");
static const QString kpropProjEff("projEff_pct");
static const QString kpropProjFG("projFg");
static const QString kpropProjOG("projOg");
static const QString kpropProjPnts("projPoints");
static const QString kpropProjFermPnts("projFermPoints");
static const QString kpropProjBoilGrav("projBoilGrav");
static const QString kpropPostBoilVol("postBoilVolume_l");
static const QString kpropProjVolIntoBoil("projVolIntoBK_l");
static const QString kpropProjVolIntoFerm("projVolIntoFerm_l");

// finally, XML props
static const QString kxmlPropBrewDate("BREWDATE");
static const QString kxmlPropFermDate("DATE_FERMENTED_OUT");
static const QString kxmlPropSG("SG");
static const QString kxmlPropVolIntoBoil("VOLUME_INTO_BK");
static const QString kxmlPropStrikeTemp("STRIKE_TEMP");
static const QString kxmlPropMashFinTemp("MASH_FINAL_TEMP");
static const QString kxmlPropPostBoilVol("POST_BOIL_VOLUME");
static const QString kxmlPropVolIntoFerm("VOLUME_INTO_FERMENTER");
static const QString kxmlPropPitchTemp("PITCH_TEMP");
static const QString kxmlPropEffIntoBoil("EFF_INTO_BK");
static const QString kxmlPropABV("ACTUAL_ABV");
static const QString kxmlPropBrewhsEff("BREWHOUSE_EFF");

static const QString kxmlPropProjBoilGrav("PROJECTED_BOIL_GRAV");
static const QString kxmlPropProjStrikeTemp("PROJECTED_STRIKE_TEMP");
static const QString kxmlPropProjMashFinTemp("PROJECTED_MASH_FIN_TEMP");
static const QString kxmlPropProjVolIntoBoil("PROJECTED_VOL_INTO_BK");
static const QString kxmlPropProjOG("PROJECTED_OG");
static const QString kxmlPropProjVolIntoFerm("PROJECTED_VOL_INTO_FERM");
static const QString kxmlPropProjFG("PROJECTED_FG");
static const QString kxmlPropProjEff("PROJECTED_EFF");
static const QString kxmlPropProjABV("PROJECTED_ABV");
static const QString kxmlPropProjPnts("PROJECTED_POINTS");
static const QString kxmlPropProjFermPnts("PROJECTED_FERM_POINTS");
static const QString kxmlPropProjAtten("PROJECTED_ATTEN");
static const QString kxmlPropBoilOff("BOIL_OFF");
static const QString kxmlPropFinVol("FINAL_VOLUME");

#endif // _BREWNOTETABLESCHEMA_H
