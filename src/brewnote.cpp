/*
 * brewnote.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#include <QDomNode>
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QDateTime>
#include <algorithm>
#include <QRegExp>
#include <QDebug>
#include <QLocale>
#include "brewnote.h"
#include "brewtarget.h"
#include "Algorithms.h"
#include "mashstep.h"
#include "recipe.h"
#include "equipment.h"
#include "mash.h"
#include "yeast.h"

const QString kBrewDate("brewDate");
const QString kAttenuation("attenuation");
const QString kFermentDate("fermentDate");
const QString kNotes("notes");
const QString kSpecificGravity("sg");
const QString kVolumeIntoBoil("volume_into_bk");
const QString kOriginalGravity("og");
const QString kVolumeIntoFermenter("volume_into_fermenter");
const QString kFinalGravity("fg");
const QString kProjectedPoints("projected_points");
const QString kProjectedFermentationPoints("projected_ferm_points");
const QString kABV("abv");
const QString kEfficiencyIntoBoil("eff_into_bk");
const QString kBrewhouseEfficiency("brewhouse_eff");
const QString kStrikeTemp("strike_temp");
const QString kMashFinalTemp("mash_final_temp");
const QString kPostBoilVolume("post_boil_volume");
const QString kPitchTemp("pitch_temp");
const QString kFinalVolume("final_volume");
const QString kProjectedBoilGravity("projected_boil_grav");
const QString kProjectedVolumeIntoBoil("projected_vol_into_bk");
const QString kProjectedStrikeTemp("projected_strike_temp");
const QString kProjectedMashFinishTemp("projected_mash_fin_temp");
const QString kProjectedOG("projected_og");
const QString kProjectedVolumeIntoFermenter("projected_vol_into_ferm");
const QString kProjectedFG("projected_fg");
const QString kProjectedEfficiency("projected_eff");
const QString kProjectedABV("projected_abv");
const QString kProjectedAttenuation("projected_atten");
const QString kBoilOff("boil_off");
const QString kSugarKg("sugar_kg");
const QString kSugarKg_IgnoreEff("sugar_kg_ignoreEfficiency");


/************** Props **************/
const QString kBrewDateProp("brewDate");
const QString kFermentDateProp("fermentDate");
const QString kNotesProp("notes");
const QString kSpecificGravityProp("sg");
const QString kOriginalGravityProp("og");
const QString kFinalGravityProp("fg");
const QString kABVProp("abv");
const QString kAttenuationProp("attenuation");
const QString kEfficiencyIntoBoilProp("effIntoBK_pct");
const QString kStrikeTempProp("strikeTemp_c");
const QString kMashFinalTempProp("mashFinTemp_c");
const QString kPitchTempProp("pitchTemp_c");
const QString kProjectedStrikeTempProp("projStrikeTemp_c");
const QString kProjectedMashFinishTempProp("projMashFinTemp_c");
const QString kProjectedAttenuationProp("projAtten");
const QString kProjectedABVProp("projABV_pct");
const QString kProjectedEfficiencyProp("projEff_pct");
const QString kProjectedFGProp("projFg");
const QString kProjectedOGProp("projOg");
const QString kProjectedPointsProp("projPoints");
const QString kProjectedFermentationPointsProp("projFermPoints");
const QString kBrewhouseEfficiencyProp("brewhouseEff_pct");
const QString kProjectedBoilGravityProp("projBoilGrav");
const QString kVolumeIntoBoilProp("volumeIntoBK_l");
const QString kVolumeIntoFermenterProp("volumeIntoFerm_l");
const QString kPostBoilVolumeProp("postBoilVolume_l");
const QString kFinalVolumeProp("finalVolume_l");
const QString kProjectedVolumeIntoBoilProp("projVolIntoBK_l");
const QString kProjectedVolumeIntoFermenterProp("projVolIntoFerm_l");
const QString kBoilOffProp("boilOff_l");

QHash<QString,QString> BrewNote::tagToProp = BrewNote::tagToPropHash();

QString BrewNote::classNameStr()
{
   static const QString name("BrewNote");
   return name;
}

QHash<QString,QString> BrewNote::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["BREWDATE"] = kBrewDateProp;
   propHash["DATE_FERMENTED_OUT"] = kFermentDateProp;
   propHash["SG"] = kSpecificGravityProp;
   propHash["VOLUME_INTO_BK"] = kVolumeIntoBoilProp;
   propHash["STRIKE_TEMP"] = kStrikeTempProp;
   propHash["MASH_FINAL_TEMP"] = kMashFinalTempProp;
   propHash["OG"] = kOriginalGravityProp;
   propHash["POST_BOIL_VOLUME"] = kPostBoilVolumeProp;
   propHash["VOLUME_INTO_FERMENTER"] = kVolumeIntoFermenterProp;
   propHash["PITCH_TEMP"] = kPitchTempProp;
   propHash["FG"] = kFinalGravityProp;
   propHash["EFF_INTO_BK"] = kEfficiencyIntoBoilProp;
   propHash["PREDICTED_OG"] = kProjectedOGProp;
   propHash["BREWHOUSE_EFF"] = kBrewhouseEfficiencyProp;
   propHash["ACTUAL_ABV"] = kABVProp;
   propHash["ATTENUATION"] = kAttenuationProp;
   propHash["PROJECTED_BOIL_GRAV"] = kProjectedBoilGravityProp;
   propHash["PROJECTED_STRIKE_TEMP"] = kProjectedStrikeTempProp;
   propHash["PROJECTED_MASH_FIN_TEMP"] = kProjectedMashFinishTempProp;
   propHash["PROJECTED_VOL_INTO_BK"] = kProjectedVolumeIntoBoilProp;
   propHash["PROJECTED_OG"] = kProjectedOGProp;
   propHash["PROJECTED_VOL_INTO_FERM"] = kProjectedVolumeIntoFermenterProp;
   propHash["PROJECTED_FG"] = kProjectedFGProp;
   propHash["PROJECTED_EFF"] = kProjectedEfficiencyProp;
   propHash["PROJECTED_ABV"] = kProjectedABVProp;
   propHash["PROJECTED_POINTS"] = kProjectedPointsProp;
   propHash["PROJECTED_FERM_POINTS"] = kProjectedFermentationPointsProp;
   propHash["PROJECTED_ATTEN"] = kProjectedAttenuationProp;
   propHash["BOIL_OFF"] = kBoilOffProp;
   propHash["FINAL_VOLUME"] = kFinalVolumeProp;
   propHash["NOTES"] = kNotesProp;
   return propHash;
}

// operators for sorts and things
bool operator<(BrewNote const& lhs, BrewNote const& rhs)
{
   return lhs.brewDate() < rhs.brewDate();
}

bool operator==(BrewNote const& lhs, BrewNote const& rhs)
{
   return lhs.brewDate() == rhs.brewDate();
}

// Initializers
BrewNote::BrewNote(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key)
{
   loading = false;
}

BrewNote::BrewNote(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key)
{
   setBrewDate( QDateTime::fromString(rec.value(kBrewDate).toString(),Qt::ISODate), true );
   setFermentDate( QDateTime::fromString(rec.value(kFermentDate).toString(),Qt::ISODate), true );
   setNotes( rec.value(kNotes).toString(), true );
   setSg(rec.value(kSpecificGravity).toDouble(), true);
   setABV( rec.value(kABV).toDouble(), true);
   setEffIntoBK_pct( rec.value(kEfficiencyIntoBoil).toDouble(), true );
   setBrewhouseEff_pct(rec.value(kBrewhouseEfficiency).toDouble(), true );
   setVolumeIntoBK_l(rec.value(kVolumeIntoBoil).toDouble(), true );
   setStrikeTemp_c(rec.value(kStrikeTemp).toDouble(), true );
   setMashFinTemp_c(rec.value(kMashFinalTemp).toDouble(), true );
   setOg(rec.value(kOriginalGravity).toDouble(), true );
   setPostBoilVolume_l(rec.value(kPostBoilVolume).toDouble(), true );
   setVolumeIntoFerm_l(rec.value(kVolumeIntoFermenter).toDouble(), true );
   setPitchTemp_c(rec.value(kPitchTemp).toDouble(), true );
   setFg(rec.value(kFinalGravity).toDouble(), true );
   setAttenuation(rec.value(kAttenuation).toDouble(), true );
   setFinalVolume_l(rec.value(kFinalVolume).toDouble(), true );
   setBoilOff_l(rec.value(kBoilOff).toDouble(), true );
   setProjBoilGrav(rec.value(kProjectedBoilGravity).toDouble(), true );
   setProjVolIntoBK_l(rec.value(kProjectedVolumeIntoBoil).toDouble(), true );
   setProjStrikeTemp_c(rec.value(kProjectedStrikeTemp).toDouble(), true );
   setProjMashFinTemp_c(rec.value(kProjectedMashFinishTemp).toDouble(), true );
   setProjOg(rec.value(kProjectedOG).toDouble(), true );
   setProjVolIntoFerm_l(rec.value(kProjectedVolumeIntoFermenter).toDouble(), true );
   setProjFg(rec.value(kProjectedFG).toDouble(), true );
   setProjEff_pct(rec.value(kProjectedEfficiency).toDouble(), true );
   setProjABV_pct(rec.value(kProjectedABV).toDouble(), true );
   setProjPoints(rec.value(kProjectedPoints).toDouble(), true );
   setProjFermPoints(rec.value(kProjectedFermentationPoints).toDouble(), true );
   setProjAtten(rec.value(kProjectedAttenuation).toDouble(), true );
}

void BrewNote::populateNote(Recipe* parent)
{

   Equipment* equip = parent->equipment();
   Mash* mash = parent->mash();
   QList<MashStep*> steps;
   MashStep* mStep;
   QList<Yeast*> yeasts = parent->yeasts();
   Yeast* yeast;
   QHash<QString,double> sugars;
   double atten_pct = -1.0;

   // Since we have the recipe, lets set some defaults The order in which
   // these are done is very specific. Please do not modify them without some
   // serious testing.

   // Everything needs volumes of one type or another. But the individual
   // volumes are fairly independent of anything. Do them all first.
   setProjVolIntoBK_l( parent->boilSize_l() );
   setVolumeIntoBK_l( parent->boilSize_l() );
   setPostBoilVolume_l(parent->postBoilVolume_l());
   setProjVolIntoFerm_l(parent->finalVolume_l());
   setVolumeIntoFerm_l(parent->finalVolume_l());
   setFinalVolume_l(parent->finalVolume_l());

   if ( equip )
      setBoilOff_l( equip->evapRate_lHr() * ( parent->boilTime_min()/60));

   sugars = parent->calcTotalPoints();
   setProjPoints(sugars.value(kSugarKg) + sugars.value(kSugarKg_IgnoreEff));

   sugars = parent->calcTotalPoints();
   setProjFermPoints(sugars.value(kSugarKg) + sugars.value(kSugarKg_IgnoreEff));

   // Out of the gate, we expect projected to be the measured.
   setSg( parent->boilGrav() );
   setProjBoilGrav(parent->boilGrav() );

   if ( mash )
   {
      steps = mash->mashSteps();
      if ( ! steps.isEmpty() )
      {
         mStep = steps.at(0);
         if ( mStep )
         {
            double endTemp = mStep->endTemp_c() > 0.0 ? mStep->endTemp_c() : mStep->stepTemp_c();
            setProjStrikeTemp_c(mStep->infuseTemp_c());

            setMashFinTemp_c(endTemp);
            setProjMashFinTemp_c(endTemp);
         }

         if ( steps.size() > 2 )
         {
            // NOTE: Qt will complain that steps.size()-2 is always positive,
            // and therefore the internal assert that the index is positive is
            // bunk. This is OK, as we just checked that we will not underflow.
            mStep = steps.at( steps.size() - 2 );
            setMashFinTemp_c( mStep->endTemp_c());
            setProjMashFinTemp_c( mStep->endTemp_c());
         }
      }
   }

   setOg( parent->og());
   setProjOg(parent->og());

   setPitchTemp_c(parent->primaryTemp_c());

   setFg( parent->fg());
   setProjFg( parent->fg() );

   setProjEff_pct(parent->efficiency_pct());
   setProjABV_pct( parent->ABV_pct());

   for (int i = 0; i < yeasts.size(); ++i)
   {
      yeast = yeasts.at(i);
      if ( yeast->attenuation_pct() > atten_pct )
         atten_pct = yeast->attenuation_pct();
   }

   if ( yeasts.size() == 0 || atten_pct < 0.0 )
      atten_pct = 75;
   setProjAtten(atten_pct);

}

// the v2 release had some bugs in the efficiency calcs. They have been fixed.
// This should allow the users to redo those calculations
void BrewNote::recalculateEff(Recipe* parent)
{

   QHash<QString,double> sugars;

   sugars = parent->calcTotalPoints();
   setProjPoints(sugars.value(kSugarKg) + sugars.value(kSugarKg_IgnoreEff));

   sugars = parent->calcTotalPoints();
   setProjFermPoints(sugars.value(kSugarKg) + sugars.value(kSugarKg_IgnoreEff));

   calculateEffIntoBK_pct();
   calculateBrewHouseEff_pct();
}

BrewNote::BrewNote(BrewNote const& other)
   : BeerXMLElement(other)
{
}

// Setters=====================================================================
void BrewNote::setBrewDate(QDateTime const& date, bool cacheOnly)
{
   _brewDate = date;
   if ( ! cacheOnly ) {
      set(kBrewDateProp, kBrewDate, date.toString(Qt::ISODate));
      emit brewDateChanged(date);
   }
}

void BrewNote::setFermentDate(QDateTime const& date, bool cacheOnly)
{
   _fermentDate = date;
   if (! cacheOnly ) {
      set(kFermentDateProp, kFermentDate, date.toString(Qt::ISODate));
   }
}

void BrewNote::setNotes(QString const& var, bool cacheOnly)
{
   _notes = var;
   if ( ! cacheOnly ) {
      set(kNotesProp, kNotes, var, false);
   }
}

void BrewNote::setLoading(bool flag) { loading = flag; }

// These five items cause the calculated fields to change. I should do this
// with signals/slots, likely, but the *only* slot for the signal will be
// the brewnote.
void BrewNote::setSg(double var, bool cacheOnly)
{
   _sg = var;
   if ( ! cacheOnly ) {
      set(kSpecificGravityProp, kSpecificGravity, var);
      calculateEffIntoBK_pct();
      calculateOg();
   }
}

void BrewNote::setVolumeIntoBK_l(double var, bool cacheOnly)
{
   _volumeIntoBK_l = var;
   if ( ! cacheOnly ) {
      set(kVolumeIntoBoilProp, kVolumeIntoBoil, var);

      calculateEffIntoBK_pct();
      calculateOg();
      calculateBrewHouseEff_pct();
   }
}

void BrewNote::setOg(double var, bool cacheOnly)
{
   _og = var;
   if ( ! cacheOnly ) {
      set(kOriginalGravityProp, kOriginalGravity, var);

      calculateBrewHouseEff_pct();
      calculateABV_pct();
      calculateActualABV_pct();
      calculateAttenuation_pct();
   }
}

void BrewNote::setVolumeIntoFerm_l(double var, bool cacheOnly)
{
   _volumeIntoFerm_l = var;
   if ( ! cacheOnly ) {
      set(kVolumeIntoFermenterProp, kVolumeIntoFermenter, var);

      calculateBrewHouseEff_pct();
   }
}

void BrewNote::setFg(double var, bool cacheOnly)
{
   _fg = var;
   if ( ! cacheOnly ) {
      set(kFinalGravityProp, kFinalGravity, var);

      calculateActualABV_pct();
      calculateAttenuation_pct();
   }
}

// This one is a bit of an odd ball. We need to convert to pure glucose points
// before we store it in the database. TODO: Prove this works. I've modified
// the logic to ignore the "loading" flag. I am not sure it will get the right
// thing done.
void BrewNote::setProjPoints(double var, bool cacheOnly)
{
   _projPoints = var;
   if ( ! cacheOnly ) {
      double convertPnts;
      double plato, total_g;

      plato = Algorithms::getPlato(var, projVolIntoBK_l());
      total_g = Algorithms::PlatoToSG_20C20C( plato );
      convertPnts = (total_g - 1.0 ) * 1000;

      set(kProjectedPointsProp, kProjectedPoints, convertPnts);
   }

}

void BrewNote::setProjFermPoints(double var, bool cacheOnly)
{
   _projFermPoints = var;

   if ( ! cacheOnly ) {
      double convertPnts;
      double plato, total_g;

      plato = Algorithms::getPlato(var, projVolIntoFerm_l());
      total_g = Algorithms::PlatoToSG_20C20C( plato );
      convertPnts = (total_g - 1.0 ) * 1000;

      set(kProjectedPointsProp, kProjectedFermentationPoints, convertPnts);
   }
}

void BrewNote::setABV(double var, bool cacheOnly)
{
   _abv = var;
   if ( ! cacheOnly ) {
      set(kABVProp, kABV, var);
   }
}

void BrewNote::setAttenuation(double var, bool cacheOnly)
{
   _attenuation = var;
   if ( ! cacheOnly ) {
      set(kAttenuationProp, kAttenuation, var);
   }
}

void BrewNote::setEffIntoBK_pct(double var, bool cacheOnly)
{
   _effIntoBK_pct = var;
   if ( ! cacheOnly ) {
      set(kEfficiencyIntoBoilProp, kEfficiencyIntoBoil, var);
   }
}

void BrewNote::setBrewhouseEff_pct(double var, bool cacheOnly)
{
   _brewhouseEff_pct = var;
   if ( ! cacheOnly ) {
      set(kBrewhouseEfficiencyProp, kBrewhouseEfficiency, var);
   }
}

void BrewNote::setStrikeTemp_c(double var, bool cacheOnly)
{
   _strikeTemp_c = var;
   if ( ! cacheOnly ) {
      set(kStrikeTempProp, kStrikeTemp, var);
   }
}

void BrewNote::setMashFinTemp_c(double var, bool cacheOnly)
{
   _mashFinTemp_c = var;
   if ( ! cacheOnly ) {
      set(kMashFinalTempProp, kMashFinalTemp, var);
   }
}

void BrewNote::setPostBoilVolume_l(double var, bool cacheOnly)
{
   _postBoilVolume_l = var;
   if ( ! cacheOnly ) {
      set(kPostBoilVolumeProp, kPostBoilVolume, var);
   }
}

void BrewNote::setPitchTemp_c(double var, bool cacheOnly)
{
   _pitchTemp_c = var;
   if ( ! cacheOnly ) {
      set(kPitchTempProp, kPitchTemp, var);
   }
}

void BrewNote::setFinalVolume_l(double var, bool cacheOnly)
{
   _finalVolume_l = var;
   if ( ! cacheOnly ) {
      set(kFinalVolumeProp, kFinalVolume, var);
   }
}

void BrewNote::setProjBoilGrav(double var, bool cacheOnly)
{
   _projBoilGrav = var;
   if ( ! cacheOnly ) {
      set(kProjectedBoilGravityProp, kProjectedBoilGravity, var);
   }
}

void BrewNote::setProjVolIntoBK_l(double var, bool cacheOnly)
{
   _projVolIntoBK_l = var;
   if ( ! cacheOnly ) {
      set(kProjectedVolumeIntoBoilProp, kProjectedVolumeIntoBoil, var);
   }
}

void BrewNote::setProjStrikeTemp_c(double var, bool cacheOnly)
{
   _projStrikeTemp_c = var;
   if ( ! cacheOnly ) {
      set(kProjectedStrikeTempProp, kProjectedStrikeTemp, var);
   }
}

void BrewNote::setProjMashFinTemp_c(double var, bool cacheOnly)
{
   _projMashFinTemp_c = var;
   if ( ! cacheOnly ) {
      set(kProjectedMashFinishTempProp, kProjectedMashFinishTemp, var);
   }
}

void BrewNote::setProjOg(double var, bool cacheOnly)
{
   _projOg = var;
   if ( ! cacheOnly ) {
      set(kProjectedOGProp, kProjectedOG, var);
   }
}

void BrewNote::setProjVolIntoFerm_l(double var, bool cacheOnly)
{
   _projVolIntoFerm_l = var;
   if ( ! cacheOnly ) {
      set(kProjectedVolumeIntoFermenterProp, kProjectedVolumeIntoFermenter, var);
   }
}

void BrewNote::setProjFg(double var, bool cacheOnly)
{
   _projFg = var;
   if ( ! cacheOnly ) {
      set(kProjectedFGProp, kProjectedFG, var);
   }
}

void BrewNote::setProjEff_pct(double var, bool cacheOnly)
{
   _projEff_pct = var;
   if ( ! cacheOnly ) {
      set(kProjectedEfficiencyProp, kProjectedEfficiency, var);
   }
}

void BrewNote::setProjABV_pct(double var, bool cacheOnly)
{
   _projABV_pct = var;
   if ( ! cacheOnly ) {
      set(kProjectedABVProp, kProjectedABV, var);
   }
}

void BrewNote::setProjAtten(double var, bool cacheOnly)
{
   _projAtten = var;
   if ( ! cacheOnly ) {
      set(kProjectedAttenuationProp, kProjectedAttenuation, var);
   }
}

void BrewNote::setBoilOff_l(double var, bool cacheOnly)
{
   _boilOff_l = var;
   if ( ! cacheOnly ) {
      set(kBoilOffProp, kBoilOff, var);
   }
}

// Getters
QDateTime BrewNote::brewDate() const { return _brewDate; }
QString BrewNote::brewDate_str() const { return _brewDate.toString(); }
QString BrewNote::brewDate_short() const { return Brewtarget::displayDateUserFormated(_brewDate.date()); }

QDateTime BrewNote::fermentDate() const { return _fermentDate; }
QString BrewNote::fermentDate_str() const { return _fermentDate.toString(); }
QString BrewNote::fermentDate_short() const { return Brewtarget::displayDateUserFormated(_fermentDate.date()); }

QString BrewNote::notes() const { return _notes; }

double BrewNote::sg() const { return _sg; }
double BrewNote::abv() const { return _abv; }
double BrewNote::attenuation() const { return _attenuation; }
double BrewNote::volumeIntoBK_l() const { return _volumeIntoBK_l; }
double BrewNote::effIntoBK_pct() const { return _effIntoBK_pct; }
double BrewNote::brewhouseEff_pct() const { return _brewhouseEff_pct; }
double BrewNote::strikeTemp_c() const { return _strikeTemp_c; }
double BrewNote::mashFinTemp_c() const { return _mashFinTemp_c; }
double BrewNote::og() const { return _og; }
double BrewNote::volumeIntoFerm_l() const { return _volumeIntoFerm_l; }
double BrewNote::postBoilVolume_l() const { return _postBoilVolume_l; }
double BrewNote::pitchTemp_c() const { return _pitchTemp_c; }
double BrewNote::fg() const { return _fg; }
double BrewNote::finalVolume_l() const { return _finalVolume_l; }
double BrewNote::projBoilGrav() const { return _projBoilGrav; }
double BrewNote::projVolIntoBK_l() const { return _projVolIntoFerm_l; }
double BrewNote::projStrikeTemp_c() const { return _projStrikeTemp_c; }
double BrewNote::projMashFinTemp_c() const { return _projMashFinTemp_c; }
double BrewNote::projOg() const { return _projOg; }
double BrewNote::projVolIntoFerm_l() const { return _projVolIntoFerm_l; }
double BrewNote::projFg() const { return _projFg; }
double BrewNote::projEff_pct() const { return _projEff_pct; }
double BrewNote::projABV_pct() const { return _projABV_pct; }
double BrewNote::projPoints() const { return _projPoints; }
double BrewNote::projFermPoints() const { return _projFermPoints; }
double BrewNote::projAtten() const { return _projAtten; }
double BrewNote::boilOff_l() const { return _boilOff_l; }

int BrewNote::key() const
{
   return _key;
}

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewNote::calculateEffIntoBK_pct()
{
   double effIntoBK;
   double maxPoints, actualPoints;

   // I don't think we need a lot of math here. Points has already been
   // translated from SG into pure glucose points
   maxPoints = _projPoints * _projVolIntoBK_l;

   actualPoints = (_sg - 1) * 1000 * _volumeIntoBK_l;

   // this can happen under normal circumstances (eg, load)
   if (maxPoints <= 0.0)
      return 0.0;

   effIntoBK = actualPoints/maxPoints * 100;
   setEffIntoBK_pct(effIntoBK);

   return effIntoBK;
}

// The idea is that based on the preboil gravity, estimate what the actual OG
// will be.
double BrewNote::calculateOg()
{
   double cOG;
   double points, expectedVol, actualVol;

   points = (sg()-1) * 1000;
   expectedVol = projVolIntoBK_l() - boilOff_l();
   actualVol   = volumeIntoBK_l();

   if ( expectedVol <= 0.0 )
      return 0.0;

   cOG = 1+ ((points * actualVol / expectedVol) / 1000);
   setProjOg(cOG);

   return cOG;
}

double BrewNote::calculateBrewHouseEff_pct()
{
   double expectedPoints, actualPoints;
   double brewhouseEff;

   expectedPoints = projFermPoints() * projVolIntoFerm_l();
   actualPoints = (og()-1.0) * 1000.0 * volumeIntoFerm_l();

   brewhouseEff = actualPoints/expectedPoints * 100.0;
   setBrewhouseEff_pct(brewhouseEff);

   return brewhouseEff;
}

// Need to do some work here to figure out what the expected FG will be based
// on the actual OG, not the calculated.
double BrewNote::calculateABV_pct()
{
   double atten_pct = projAtten();
   double calculatedABV;
   double estFg;

   // This looks weird, but the math works. (Yes, I am showing my work)
   // 1 + [(og-1) * 1000 * (1.0 - %/100)] / 1000  =
   // 1 + [(og - 1) * (1.0 - %/100)]
   estFg = 1 + ((og()-1.0)*(1.0 - atten_pct/100.0));

   calculatedABV = (og()-estFg)*130;
   setProjABV_pct(calculatedABV);

   return calculatedABV;
}

double BrewNote::calculateActualABV_pct()
{
   double abv;

   abv = (og() - fg()) * 130;
   setABV(abv);

   return abv;
}

double BrewNote::calculateAttenuation_pct()
{
    // Calculate measured attenuation based on user-reported values for
    // post-boil OG and post-ferment FG
    double attenuation = ((og() - fg()) / (og() - 1)) * 100;

    setAttenuation(attenuation);

    return attenuation;
}
