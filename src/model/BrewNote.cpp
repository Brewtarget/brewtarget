/*
 * model/BrewNote.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/BrewNote.h"

#include <algorithm>
#include <QDebug>
#include <QLocale>
#include <QObject>
#include <QRegExp>
#include <QString>

#include "Algorithms.h"
#include "database/ObjectStoreWrapper.h"
#include "Localization.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "model/Yeast.h"

// These belong here, because they really just are constant strings for
// reaching into a hash
static const QString kSugarKg("sugar_kg");
static const QString kSugarKg_IgnoreEff("sugar_kg_ignoreEfficiency");


// BrewNote doesn't use its name field, so we sort by brew date
// TBD: Could consider copying date into name field and leaving the default ordering
// MAF: In all the databases, names are strings but any date is the proper
//      date/timestamp type. Swapping a name for a date field would require
//      much conversion between them, would cause endless confusion given that
//      Americans don't believe in normal date formats and will end up
//      generating a LOT of work so we can avoid two lines of code.
//
//      I mean. Suppose you could make the brewnote name property a time/date
//      and set it to the brewdate when the object is loaded/created. That
//      would generate a lot of confusion for me at least.
//
//      Personally, I like two lines of code to avoid lots of confusion.
bool BrewNote::operator<(BrewNote const & other) const { return this->m_brewDate < other.m_brewDate; }
bool BrewNote::operator>(BrewNote const & other) const { return this->m_brewDate > other.m_brewDate; }

bool BrewNote::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   BrewNote const & rhs = static_cast<BrewNote const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_brewDate == rhs.m_brewDate
   );
}

ObjectStore & BrewNote::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<BrewNote>::getInstance();
}

// Initializers
BrewNote::BrewNote(QString name) : BrewNote(QDate(), name) {
   return;
}

BrewNote::BrewNote(Recipe const & recipe) :
   BrewNote(QDate(), "") {
   this->m_recipeId = recipe.key();
   return;
}

BrewNote::BrewNote(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   loading            {false},
   m_brewDate         {namedParameterBundle(PropertyNames::BrewNote::brewDate         ).toDate()},
   m_fermentDate      {namedParameterBundle(PropertyNames::BrewNote::fermentDate      ).toDate()},
   m_notes            {namedParameterBundle(PropertyNames::BrewNote::notes            ).toString()},
   m_sg               {namedParameterBundle(PropertyNames::BrewNote::sg               ).toDouble()},
   m_abv              {namedParameterBundle(PropertyNames::BrewNote::abv              ).toDouble()},
   m_effIntoBK_pct    {namedParameterBundle(PropertyNames::BrewNote::effIntoBK_pct    ).toDouble()},
   m_brewhouseEff_pct {namedParameterBundle(PropertyNames::BrewNote::brewhouseEff_pct ).toDouble()},
   m_volumeIntoBK_l   {namedParameterBundle(PropertyNames::BrewNote::volumeIntoBK_l   ).toDouble()},
   m_strikeTemp_c     {namedParameterBundle(PropertyNames::BrewNote::strikeTemp_c     ).toDouble()},
   m_mashFinTemp_c    {namedParameterBundle(PropertyNames::BrewNote::mashFinTemp_c    ).toDouble()},
   m_og               {namedParameterBundle(PropertyNames::BrewNote::og               ).toDouble()},
   m_postBoilVolume_l {namedParameterBundle(PropertyNames::BrewNote::postBoilVolume_l ).toDouble()},
   m_volumeIntoFerm_l {namedParameterBundle(PropertyNames::BrewNote::volumeIntoFerm_l ).toDouble()},
   m_pitchTemp_c      {namedParameterBundle(PropertyNames::BrewNote::pitchTemp_c      ).toDouble()},
   m_fg               {namedParameterBundle(PropertyNames::BrewNote::fg               ).toDouble()},
   m_attenuation      {namedParameterBundle(PropertyNames::BrewNote::attenuation      ).toDouble()},
   m_finalVolume_l    {namedParameterBundle(PropertyNames::BrewNote::finalVolume_l    ).toDouble()},
   m_boilOff_l        {namedParameterBundle(PropertyNames::BrewNote::boilOff_l        ).toDouble()},
   m_projBoilGrav     {namedParameterBundle(PropertyNames::BrewNote::projBoilGrav     ).toDouble()},
   m_projVolIntoBK_l  {namedParameterBundle(PropertyNames::BrewNote::projVolIntoBK_l  ).toDouble()},
   m_projStrikeTemp_c {namedParameterBundle(PropertyNames::BrewNote::projStrikeTemp_c ).toDouble()},
   m_projMashFinTemp_c{namedParameterBundle(PropertyNames::BrewNote::projMashFinTemp_c).toDouble()},
   m_projOg           {namedParameterBundle(PropertyNames::BrewNote::projOg           ).toDouble()},
   m_projVolIntoFerm_l{namedParameterBundle(PropertyNames::BrewNote::projVolIntoFerm_l).toDouble()},
   m_projFg           {namedParameterBundle(PropertyNames::BrewNote::projFg           ).toDouble()},
   m_projEff_pct      {namedParameterBundle(PropertyNames::BrewNote::projEff_pct      ).toDouble()},
   m_projABV_pct      {namedParameterBundle(PropertyNames::BrewNote::projABV_pct      ).toDouble()},
   m_projPoints       {namedParameterBundle(PropertyNames::BrewNote::projPoints       ).toDouble()},
   m_projFermPoints   {namedParameterBundle(PropertyNames::BrewNote::projFermPoints   ).toDouble()},
   m_projAtten        {namedParameterBundle(PropertyNames::BrewNote::projAtten        ).toDouble()},
   m_recipeId         {namedParameterBundle(PropertyNames::BrewNote::recipeId         ).toInt()} {
   return;
}


BrewNote::BrewNote(QDate dateNow, QString const & name) :
   NamedEntity        {name, true},
   loading            {false},
   m_brewDate         {dateNow},
   m_fermentDate      {},
   m_notes            {""},
   m_sg               {0.0},
   m_abv              {0.0},
   m_effIntoBK_pct    {0.0},
   m_brewhouseEff_pct {0.0},
   m_volumeIntoBK_l   {0.0},
   m_strikeTemp_c     {0.0},
   m_mashFinTemp_c    {0.0},
   m_og               {0.0},
   m_postBoilVolume_l {0.0},
   m_volumeIntoFerm_l {0.0},
   m_pitchTemp_c      {0.0},
   m_fg               {0.0},
   m_attenuation      {0.0},
   m_finalVolume_l    {0.0},
   m_boilOff_l        {0.0},
   m_projBoilGrav     {0.0},
   m_projVolIntoBK_l  {0.0},
   m_projStrikeTemp_c {0.0},
   m_projMashFinTemp_c{0.0},
   m_projOg           {0.0},
   m_projVolIntoFerm_l{0.0},
   m_projFg           {0.0},
   m_projEff_pct      {0.0},
   m_projABV_pct      {0.0},
   m_projPoints       {0.0},
   m_projFermPoints   {0.0},
   m_projAtten        {0.0} {
   return;
}

void BrewNote::populateNote(Recipe* parent)
{
   this->m_recipeId = parent->key();

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

   auto equip = parent->equipment();
   if (equip) {
      setBoilOff_l( equip->evapRate_lHr() * ( parent->boilTime_min()/60));
   }

   QHash<QString,double> sugars;
   sugars = parent->calcTotalPoints();
   setProjPoints(sugars.value(kSugarKg) + sugars.value(kSugarKg_IgnoreEff));

   setProjFermPoints(sugars.value(kSugarKg) + sugars.value(kSugarKg_IgnoreEff));

   // Out of the gate, we expect projected to be the measured.
   setSg( parent->boilGrav() );
   setProjBoilGrav(parent->boilGrav() );

   auto mash = parent->mash();
   if (mash) {
      auto steps = mash->mashSteps();
      if (!steps.isEmpty()) {
         auto mStep = steps.at(0);

         if (mStep) {
            double endTemp = mStep->endTemp_c() > 0.0 ? mStep->endTemp_c() : mStep->stepTemp_c();

            setStrikeTemp_c(mStep->infuseTemp_c());
            setProjStrikeTemp_c(mStep->infuseTemp_c());

            setMashFinTemp_c(endTemp);
            setProjMashFinTemp_c(endTemp);
         }

         if (steps.size() > 2) {
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

   double atten_pct = -1.0;
   auto yeasts = parent->yeasts();
   for (auto yeast : yeasts) {
      if ( yeast->attenuation_pct() > atten_pct ) {
         atten_pct = yeast->attenuation_pct();
      }
   }

   if ( yeasts.size() == 0 || atten_pct < 0.0 ) {
      atten_pct = 75;
   }
   setProjAtten(atten_pct);
   return;
}

// the v2 release had some bugs in the efficiency calcs. They have been fixed.
// This should allow the users to redo those calculations
void BrewNote::recalculateEff(Recipe* parent)
{
   this->m_recipeId = parent->key();

   QHash<QString,double> sugars;

   sugars = parent->calcTotalPoints();
   setProjPoints(sugars.value(kSugarKg) + sugars.value(kSugarKg_IgnoreEff));

   sugars = parent->calcTotalPoints();
   setProjFermPoints(sugars.value(kSugarKg) + sugars.value(kSugarKg_IgnoreEff));

   calculateEffIntoBK_pct();
   calculateBrewHouseEff_pct();
}

BrewNote::BrewNote(BrewNote const& other) :
   NamedEntity        {other                    },
   m_brewDate         {other.m_brewDate         },
   m_fermentDate      {other.m_fermentDate      },
   m_notes            {other.m_notes            },
   m_sg               {other.m_sg               },
   m_abv              {other.m_abv              },
   m_effIntoBK_pct    {other.m_effIntoBK_pct    },
   m_brewhouseEff_pct {other.m_brewhouseEff_pct },
   m_volumeIntoBK_l   {other.m_volumeIntoBK_l   },
   m_strikeTemp_c     {other.m_strikeTemp_c     },
   m_mashFinTemp_c    {other.m_mashFinTemp_c    },
   m_og               {other.m_og               },
   m_postBoilVolume_l {other.m_postBoilVolume_l },
   m_volumeIntoFerm_l {other.m_volumeIntoFerm_l },
   m_pitchTemp_c      {other.m_pitchTemp_c      },
   m_fg               {other.m_fg               },
   m_attenuation      {other.m_attenuation      },
   m_finalVolume_l    {other.m_finalVolume_l    },
   m_boilOff_l        {other.m_boilOff_l        },
   m_projBoilGrav     {other.m_projBoilGrav     },
   m_projVolIntoBK_l  {other.m_projVolIntoBK_l  },
   m_projStrikeTemp_c {other.m_projStrikeTemp_c },
   m_projMashFinTemp_c{other.m_projMashFinTemp_c},
   m_projOg           {other.m_projOg           },
   m_projVolIntoFerm_l{other.m_projVolIntoFerm_l},
   m_projFg           {other.m_projFg           },
   m_projEff_pct      {other.m_projEff_pct      },
   m_projABV_pct      {other.m_projABV_pct      },
   m_projPoints       {other.m_projPoints       },
   m_projFermPoints   {other.m_projFermPoints   },
   m_projAtten        {other.m_projAtten        } {
   return;
}

// Setters=====================================================================
void BrewNote::setBrewDate(QDate const & date) {
   this->setAndNotify(PropertyNames::BrewNote::brewDate, this->m_brewDate, date);
   if (this->key() > 0) {
      // .:TBD:. Do we really need this special signal when we could use the generic changed one?
      emit brewDateChanged(date);
   }
}

void BrewNote::setFermentDate(QDate const & date) {
   this->setAndNotify(PropertyNames::BrewNote::fermentDate, this->m_fermentDate, date);
}

void BrewNote::setNotes(QString const& var) {
   this->setAndNotify(PropertyNames::BrewNote::notes, this->m_notes, var);
}

void BrewNote::setLoading(bool flag) { this->loading = flag; }

// These five items cause the calculated fields to change. I should do this
// with signals/slots, likely, but the *only* slot for the signal will be
// the brewnote.
void BrewNote::setSg(double var) {
   // I REALLY dislike this logic. It is too bloody intertwined
   this->setAndNotify(PropertyNames::BrewNote::sg, this->m_sg, var);

   // write the value to the DB if requested
   if ( ! this->loading ) {
      calculateEffIntoBK_pct();
      calculateOg();
   }
}

void BrewNote::setVolumeIntoBK_l(double var) {
   this->setAndNotify(PropertyNames::BrewNote::volumeIntoBK_l, this->m_volumeIntoBK_l, var);

   if ( ! loading ) {
      calculateEffIntoBK_pct();
      calculateOg();
      calculateBrewHouseEff_pct();
   }
}

void BrewNote::setOg(double var) {
   this->setAndNotify(PropertyNames::BrewNote::og, this->m_og, var);

   if ( ! loading ) {
      calculateBrewHouseEff_pct();
      calculateABV_pct();
      calculateActualABV_pct();
      calculateAttenuation_pct();
   }
}

void BrewNote::setVolumeIntoFerm_l(double var) {
   this->setAndNotify(PropertyNames::BrewNote::volumeIntoFerm_l, this->m_volumeIntoFerm_l, var);

   if ( ! loading ) {
      calculateBrewHouseEff_pct();
   }
}

void BrewNote::setFg(double var) {
   this->setAndNotify(PropertyNames::BrewNote::fg, this->m_fg, var);

   if ( !loading ) {
      calculateActualABV_pct();
      calculateAttenuation_pct();
   }
}

// This one is a bit of an odd ball. We need to convert to pure glucose points
// before we store it in the database.
// DO NOT ignore the loading flag. Just. Don't.
void BrewNote::setProjPoints(double var) {
   if ( loading ) {
      this->m_projPoints = var;
   } else {
      double plato = Algorithms::getPlato(var, m_projVolIntoBK_l);
      double total_g = Algorithms::PlatoToSG_20C20C( plato );
      double convertPnts = (total_g - 1.0 ) * 1000;

      this->setAndNotify(PropertyNames::BrewNote::projPoints, this->m_projPoints, convertPnts);
   }
}

void BrewNote::setProjFermPoints(double var) {
   if ( loading ) {
      this->m_projFermPoints = var;
   } else {
      double plato = Algorithms::getPlato(var, m_projVolIntoFerm_l);
      double total_g = Algorithms::PlatoToSG_20C20C( plato );
      double convertPnts = (total_g - 1.0 ) * 1000;

      this->setAndNotify(PropertyNames::BrewNote::projFermPoints, this->m_projFermPoints, convertPnts);
   }
}

void BrewNote::setABV(double var) {
   this->setAndNotify(PropertyNames::BrewNote::abv, this->m_abv, var);
}

void BrewNote::setAttenuation(double var) {
   this->setAndNotify(PropertyNames::BrewNote::attenuation, this->m_attenuation, var);
}

void BrewNote::setEffIntoBK_pct(double var) {
   this->setAndNotify(PropertyNames::BrewNote::effIntoBK_pct, this->m_effIntoBK_pct, var);
}

void BrewNote::setBrewhouseEff_pct(double var) {
   this->setAndNotify(PropertyNames::BrewNote::brewhouseEff_pct, this->m_brewhouseEff_pct, var);
}

void BrewNote::setStrikeTemp_c(double var) {
   this->setAndNotify(PropertyNames::BrewNote::strikeTemp_c, this->m_strikeTemp_c, var);
}

void BrewNote::setMashFinTemp_c(double var) {
   this->setAndNotify(PropertyNames::BrewNote::mashFinTemp_c, this->m_mashFinTemp_c, var);
}

void BrewNote::setPostBoilVolume_l(double var) {
   this->setAndNotify(PropertyNames::BrewNote::postBoilVolume_l, this->m_postBoilVolume_l, var);
}

void BrewNote::setPitchTemp_c(double var) {
   this->setAndNotify(PropertyNames::BrewNote::pitchTemp_c, this->m_pitchTemp_c, var);
}

void BrewNote::setFinalVolume_l(double var) {
   this->setAndNotify(PropertyNames::BrewNote::finalVolume_l, this->m_finalVolume_l, var);
}

void BrewNote::setProjBoilGrav(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projBoilGrav, this->m_projBoilGrav, var);
}

void BrewNote::setProjVolIntoBK_l(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projVolIntoBK_l, this->m_projVolIntoBK_l, var);
}

void BrewNote::setProjStrikeTemp_c(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projStrikeTemp_c, this->m_projStrikeTemp_c, var);
}

void BrewNote::setProjMashFinTemp_c(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projMashFinTemp_c, this->m_projMashFinTemp_c, var);
}

void BrewNote::setProjOg(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projOg, this->m_projOg, var);
}

void BrewNote::setProjVolIntoFerm_l(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projVolIntoFerm_l, this->m_projVolIntoFerm_l, var);
}

void BrewNote::setProjFg(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projFg, this->m_projFg, var);
}

void BrewNote::setProjEff_pct(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projEff_pct, this->m_projEff_pct, var);
}

void BrewNote::setProjABV_pct(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projABV_pct, this->m_projABV_pct, var);
}

void BrewNote::setProjAtten(double var) {
   this->setAndNotify(PropertyNames::BrewNote::projAtten, this->m_projAtten, var);
}

void BrewNote::setBoilOff_l(double var) {
   this->setAndNotify(PropertyNames::BrewNote::boilOff_l, this->m_boilOff_l, var);
}

void BrewNote::setRecipeId(int recipeId) { this->m_recipeId = recipeId; }
void BrewNote::setRecipe(Recipe * recipe) {
   Q_ASSERT(nullptr != recipe);
   this->m_recipeId = recipe->key();
   return;
}

Recipe * BrewNote::getOwningRecipe() {
   // getById will return nullptr if the recipe ID is invalid, which is what we want here
   return ObjectStoreWrapper::getByIdRaw<Recipe>(this->m_recipeId);
}


// Getters
QDate BrewNote::brewDate() const { return m_brewDate; }
QString BrewNote::brewDate_str() const { return m_brewDate.toString(); }
QString BrewNote::brewDate_short() const { return Localization::displayDateUserFormated(m_brewDate); }

QDate BrewNote::fermentDate() const { return m_fermentDate; }
QString BrewNote::fermentDate_str() const { return m_fermentDate.toString(); }
QString BrewNote::fermentDate_short() const { return Localization::displayDateUserFormated(m_fermentDate); }

QString BrewNote::notes() const { return m_notes; }

double BrewNote::sg() const { return m_sg; }
double BrewNote::abv() const { return m_abv; }
double BrewNote::attenuation() const { return m_attenuation; }
double BrewNote::volumeIntoBK_l() const { return m_volumeIntoBK_l; }
double BrewNote::effIntoBK_pct() const { return m_effIntoBK_pct; }
double BrewNote::brewhouseEff_pct() const { return m_brewhouseEff_pct; }
double BrewNote::strikeTemp_c() const { return m_strikeTemp_c; }
double BrewNote::mashFinTemp_c() const { return m_mashFinTemp_c; }
double BrewNote::og() const { return m_og; }
double BrewNote::volumeIntoFerm_l() const { return m_volumeIntoFerm_l; }
double BrewNote::postBoilVolume_l() const { return m_postBoilVolume_l; }
double BrewNote::pitchTemp_c() const { return m_pitchTemp_c; }
double BrewNote::fg() const { return m_fg; }
double BrewNote::finalVolume_l() const { return m_finalVolume_l; }
double BrewNote::projBoilGrav() const { return m_projBoilGrav; }
double BrewNote::projVolIntoBK_l() const { return m_projVolIntoFerm_l; }
double BrewNote::projStrikeTemp_c() const { return m_projStrikeTemp_c; }
double BrewNote::projMashFinTemp_c() const { return m_projMashFinTemp_c; }
double BrewNote::projOg() const { return m_projOg; }
double BrewNote::projVolIntoFerm_l() const { return m_projVolIntoFerm_l; }
double BrewNote::projFg() const { return m_projFg; }
double BrewNote::projEff_pct() const { return m_projEff_pct; }
double BrewNote::projABV_pct() const { return m_projABV_pct; }
double BrewNote::projPoints() const { return m_projPoints; }
double BrewNote::projFermPoints() const { return m_projFermPoints; }
double BrewNote::projAtten() const { return m_projAtten; }
double BrewNote::boilOff_l() const { return m_boilOff_l; }
int    BrewNote::getRecipeId() const { return this->m_recipeId; }

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewNote::calculateEffIntoBK_pct()
{
   double effIntoBK;
   double maxPoints, actualPoints;

   // I don't think we need a lot of math here. Points has already been
   // translated from SG into pure glucose points
   maxPoints = m_projPoints * m_projVolIntoBK_l;

   actualPoints = (m_sg - 1) * 1000 * m_volumeIntoBK_l;

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

   points = (m_sg-1) * 1000;
   expectedVol = m_projVolIntoBK_l - m_boilOff_l;
   actualVol   = m_volumeIntoBK_l;

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

   expectedPoints = m_projFermPoints * m_projVolIntoFerm_l;
   actualPoints = (m_og-1.0) * 1000.0 * m_volumeIntoFerm_l;

   brewhouseEff = actualPoints/expectedPoints * 100.0;
   setBrewhouseEff_pct(brewhouseEff);

   return brewhouseEff;
}

// Need to do some work here to figure out what the expected FG will be based
// on the actual OG, not the calculated.
double BrewNote::calculateABV_pct()
{
   double atten_pct = m_projAtten;
   double calculatedABV;
   double estFg;

   // This looks weird, but the math works. (Yes, I am showing my work)
   // 1 + [(og-1) * 1000 * (1.0 - %/100)] / 1000  =
   // 1 + [(og - 1) * (1.0 - %/100)]
   estFg = 1 + ((m_og-1.0)*(1.0 - atten_pct/100.0));

   calculatedABV = (m_og-estFg)*130;
   setProjABV_pct(calculatedABV);

   return calculatedABV;
}

double BrewNote::calculateActualABV_pct()
{
   double abv;

   abv = (m_og - m_fg) * 130;
   setABV(abv);

   return abv;
}

double BrewNote::calculateAttenuation_pct()
{
    // Calculate measured attenuation based on user-reported values for
    // post-boil OG and post-ferment FG
    double attenuation = ((m_og - m_fg) / (m_og - 1)) * 100;

    setAttenuation(attenuation);

    return attenuation;
}
