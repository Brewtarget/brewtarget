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

QHash<QString,QString> BrewNote::tagToProp = BrewNote::tagToPropHash();

QHash<QString,QString> BrewNote::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["BREWDATE"] = "brewDate" ;
   propHash["DATE_FERMENTED_OUT"] = "fermentDate" ;
   propHash["SG"] = "sg" ;
   propHash["VOLUME_INTO_BK"] = "volumeIntoBK_l" ;
   propHash["STRIKE_TEMP"] = "strikeTemp_c" ;
   propHash["MASH_FINAL_TEMP"] = "mashFinTemp_c" ;
   propHash["OG"] = "og" ;
   propHash["POST_BOIL_VOLUME"] = "postBoilVolume_l" ;
   propHash["VOLUME_INTO_FERMENTER"] = "volumeIntoFerm_l" ;
   propHash["PITCH_TEMP"] = "pitchTemp_c" ;
   propHash["FG"] = "fg" ;
   propHash["EFF_INTO_BK"] = "effIntoBK_pct" ;
   propHash["PREDICTED_OG"] = "projOg" ;
   propHash["BREWHOUSE_EFF"] = "brewhouseEff_pct" ;
   //propHash["PREDICTED_ABV"] = "projABV_pct" ;
   propHash["ACTUAL_ABV"] = "abv" ;
   propHash["PROJECTED_BOIL_GRAV"] = "projBoilGrav" ;
   propHash["PROJECTED_STRIKE_TEMP"] = "projStrikeTemp_c" ;
   propHash["PROJECTED_MASH_FIN_TEMP"] = "projMashFinTemp_c" ;
   propHash["PROJECTED_VOL_INTO_BK"] = "projVolIntoBK_l" ;
   propHash["PROJECTED_OG"] = "projOg" ;
   propHash["PROJECTED_VOL_INTO_FERM"] = "projVolIntoFerm_l" ;
   propHash["PROJECTED_FG"] = "projFg" ;
   propHash["PROJECTED_EFF"] = "projEff_pct" ;
   propHash["PROJECTED_ABV"] = "projABV_pct" ;
   propHash["PROJECTED_POINTS"] = "projPoints" ;
   propHash["PROJECTED_FERM_POINTS"] = "projFermPoints" ;
   propHash["PROJECTED_ATTEN"] = "projAtten" ;
   propHash["BOIL_OFF"] = "boilOff_l" ;
   propHash["FINAL_VOLUME"] = "finalVolume_l" ;
   propHash["NOTES"] = "notes" ;
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
BrewNote::BrewNote()
   : BeerXMLElement()
{
   loading = false;
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
   setProjPoints(sugars.value("sugar_kg") + sugars.value("sugar_kg_ignoreEfficiency"));

   sugars = parent->calcTotalPoints();
   setProjFermPoints(sugars.value("sugar_kg") + sugars.value("sugar_kg_ignoreEfficiency"));

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
   setProjPoints(sugars.value("sugar_kg") + sugars.value("sugar_kg_ignoreEfficiency"));

   sugars = parent->calcTotalPoints();
   setProjFermPoints(sugars.value("sugar_kg") + sugars.value("sugar_kg_ignoreEfficiency"));

   calculateEffIntoBK_pct();
   calculateBrewHouseEff_pct();
}

BrewNote::BrewNote(BrewNote const& other)
   : BeerXMLElement(other)
{
}

// Setters=====================================================================
void BrewNote::setBrewDate(QDateTime const& date)
{
   set("brewDate", "brewDate", date.toString(Qt::ISODate));
   emit brewDateChanged(date);
}

void BrewNote::setFermentDate(QDateTime const& date)
{
   set("fermentDate", "fermentDate", date.toString(Qt::ISODate));
}

void BrewNote::setNotes(QString const& var, bool notify)
{
   set("notes", "notes", var, notify);
}

void BrewNote::setLoading(bool flag) { loading = flag; }

// These five items cause the calculated fields to change. I should do this
// with signals/slots, likely, but the *only* slot for the signal will be
// the brewnote.
void BrewNote::setSg(double var)
{
   set("sg", "sg", var);

   if ( loading )
      return;

   calculateEffIntoBK_pct();
   calculateOg();
}

void BrewNote::setVolumeIntoBK_l(double var)
{
   set("volumeIntoBK_l", "volume_into_bk", var);

   if ( loading )
      return;

   calculateEffIntoBK_pct();
   calculateOg();
   calculateBrewHouseEff_pct();
}

void BrewNote::setOg(double var)
{
   set("og", "og", var);

   if ( loading )
      return;

   calculateBrewHouseEff_pct();
   calculateABV_pct();
   calculateActualABV_pct();
}

void BrewNote::setVolumeIntoFerm_l(double var)
{
   set("volumeIntoFerm_l", "volume_into_fermenter", var);

   if ( loading )
      return;

   calculateBrewHouseEff_pct();
}

void BrewNote::setFg(double var)
{
   set("fg", "fg", var);

   if ( loading )
      return;

   calculateActualABV_pct();
}

// This one is a bit of an odd ball. We need to convert to pure glucose points
// before we store it in the database.
void BrewNote::setProjPoints(double var)
{
   double convertPnts;
   double plato, total_g;

   if ( loading )
      convertPnts = var;
   else
   {
      plato = Algorithms::getPlato(var, projVolIntoBK_l());
      total_g = Algorithms::PlatoToSG_20C20C( plato );
      convertPnts = (total_g - 1.0 ) * 1000;
   }

   set("projPoints", "projected_points", convertPnts);
}

void BrewNote::setProjFermPoints(double var)
{
   double convertPnts;
   double plato, total_g;

   if ( loading )
      convertPnts = var;
   else
   {
      plato = Algorithms::getPlato(var, projVolIntoFerm_l());
      total_g = Algorithms::PlatoToSG_20C20C( plato );
      convertPnts = (total_g - 1.0 ) * 1000;
   }

   set("projPoints", "projected_ferm_points", convertPnts);
}

void BrewNote::setABV(double var)               { set("abv", "abv", var); }
void BrewNote::setEffIntoBK_pct(double var)     { set("effIntoBK_pct", "eff_into_bk", var); }
void BrewNote::setBrewhouseEff_pct(double var)  { set("brewhouseEff_pct", "brewhouse_eff", var); }
void BrewNote::setStrikeTemp_c(double var)      { set("strikeTemp_c", "strike_temp", var); }
void BrewNote::setMashFinTemp_c(double var)     { set("mashFinTemp_c", "mash_final_temp", var); }
void BrewNote::setPostBoilVolume_l(double var)  { set("postBoilVolume_l", "post_boil_volume", var); }
void BrewNote::setPitchTemp_c(double var)       { set("pitchTemp_c", "pitch_temp", var); }
void BrewNote::setFinalVolume_l(double var)     { set("finalVolume_l", "final_volume", var); }
void BrewNote::setProjBoilGrav(double var)      { set("projBoilGrav", "projected_boil_grav", var); }
void BrewNote::setProjVolIntoBK_l(double var)   { set("projVolIntoBK_l", "projected_vol_into_bk", var); }
void BrewNote::setProjStrikeTemp_c(double var)  { set("projStrikeTemp_c", "projected_strike_temp", var); }
void BrewNote::setProjMashFinTemp_c(double var) { set("projMashFinTemp_c", "projected_mash_fin_temp", var); }
void BrewNote::setProjOg(double var)            { set("projOg", "projected_og", var); }
void BrewNote::setProjVolIntoFerm_l(double var) { set("projVolIntoFerm_l", "projected_vol_into_ferm", var); }
void BrewNote::setProjFg(double var)            { set("projFg", "projected_fg", var); }
void BrewNote::setProjEff_pct(double var)       { set("projEff_pct", "projected_eff", var); }
void BrewNote::setProjABV_pct(double var)       { set("projABV_pct", "projected_abv", var); }
void BrewNote::setProjAtten(double var)         { set("projAtten", "projected_atten", var); }
void BrewNote::setBoilOff_l(double var)         { set("boilOff_l", "boil_off", var); }

// Getters
QDateTime BrewNote::brewDate()      const { return QDateTime::fromString(get("brewDate").toString(),Qt::ISODate); }
QString BrewNote::brewDate_str()    const { return get("brewDate").toString(); }
QDateTime BrewNote::fermentDate()   const { return QDateTime::fromString(get("fermentDate").toString(),Qt::ISODate); }
QString BrewNote::fermentDate_str() const { return get("fermentDate").toString(); }
QString BrewNote::fermentDate_short() const { return fermentDate().toString("yyyy-MM-dd"); }
QString BrewNote::notes()           const { return get("notes").toString(); }
QString BrewNote::brewDate_short()  const { return Brewtarget::displayDateUserFormated(brewDate().date()); }

double BrewNote::sg() const                { return get("sg").toDouble(); }
double BrewNote::abv() const               { return get("abv").toDouble(); }
double BrewNote::volumeIntoBK_l() const    { return get("volume_into_bk").toDouble(); }
double BrewNote::effIntoBK_pct() const     { return get("eff_into_bk").toDouble(); }
double BrewNote::brewhouseEff_pct() const  { return get("brewhouse_eff").toDouble(); }
double BrewNote::strikeTemp_c() const      { return get("strike_temp").toDouble(); }
double BrewNote::mashFinTemp_c() const     { return get("mash_final_temp").toDouble(); }
double BrewNote::og() const                { return get("og").toDouble(); }
double BrewNote::volumeIntoFerm_l() const  { return get("volume_into_fermenter").toDouble(); }
double BrewNote::postBoilVolume_l() const  { return get("post_boil_volume").toDouble(); }
double BrewNote::pitchTemp_c() const       { return get("pitch_temp").toDouble(); }
double BrewNote::fg() const                { return get("fg").toDouble(); }
double BrewNote::finalVolume_l() const     { return get("final_volume").toDouble(); }
double BrewNote::projBoilGrav() const      { return get("projected_boil_grav").toDouble(); }
double BrewNote::projVolIntoBK_l() const   { return get("projected_vol_into_bk").toDouble(); }
double BrewNote::projStrikeTemp_c() const  { return get("projected_strike_temp").toDouble(); }
double BrewNote::projMashFinTemp_c() const { return get("projected_mash_fin_temp").toDouble(); }
double BrewNote::projOg() const            { return get("projected_og").toDouble(); }
double BrewNote::projVolIntoFerm_l() const { return get("projected_vol_into_ferm").toDouble(); }
double BrewNote::projFg() const            { return get("projected_fg").toDouble(); }
double BrewNote::projEff_pct() const       { return get("projected_eff").toDouble(); }
double BrewNote::projABV_pct() const       { return get("projected_abv").toDouble(); }
double BrewNote::projPoints() const        { return get("projected_points").toDouble(); }
double BrewNote::projFermPoints() const    { return get("projected_ferm_points").toDouble(); }
double BrewNote::projAtten() const         { return get("projected_atten").toDouble(); }
double BrewNote::boilOff_l() const         { return get("boil_off").toDouble(); }

int BrewNote::key() const                  { return _key; }

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewNote::calculateEffIntoBK_pct()
{
   double effIntoBK;
   double maxPoints, actualPoints;

   // I don't think we need a lot of math here. Points has already been
   // translated from SG into pure glucose points
   maxPoints = projPoints() * projVolIntoBK_l();

   actualPoints = (sg() - 1) * 1000 * volumeIntoBK_l();

   if (maxPoints <= 0.0)
   {
      Brewtarget::logW(QString("calculateEffIntoBK :: Avoiding div by 0, maxpoints is %1").arg(maxPoints));
      return 0.0;
   }

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
   {
      Brewtarget::logW(QString("calculated OG will be off because of bad expected volume into bk %1").arg(expectedVol));
      return 0.0;
   }

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

