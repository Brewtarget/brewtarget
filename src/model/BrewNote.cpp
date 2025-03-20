/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/BrewNote.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "model/BrewNote.h"

#include <algorithm>
#include <QDebug>
#include <QObject>
#include <QString>

#include "Algorithms.h"
#include "database/ObjectStoreWrapper.h"
#include "Localization.h"
#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Fermentation.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionYeast.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BrewNote.cpp"
#endif

QString BrewNote::localisedName() { return tr("Brew Note"); }

// BrewNote doesn't use its name field, so we sort by brew date
// TBD: Could consider copying date into name field and leaving the default ordering
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

TypeLookup const BrewNote::typeLookup {
   "BrewNote",
   {
      // Note that we need Enums to be treated as ints for the purposes of type lookup
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::abv              , BrewNote::m_abv              ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::attenuation      , BrewNote::m_attenuation      ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::boilOff_l        , BrewNote::m_boilOff_l        , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::brewDate         , BrewNote::m_brewDate         ,           NonPhysicalQuantity::Date       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::brewhouseEff_pct , BrewNote::m_brewhouseEff_pct ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::effIntoBK_pct    , BrewNote::m_effIntoBK_pct    ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::fermentDate      , BrewNote::m_fermentDate      ,           NonPhysicalQuantity::Date       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::fg               , BrewNote::m_fg               , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::finalVolume_l    , BrewNote::m_finalVolume_l    , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::mashFinTemp_c    , BrewNote::m_mashFinTemp_c    , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::notes            , BrewNote::m_notes                                                        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::og               , BrewNote::m_og               , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::pitchTemp_c      , BrewNote::m_pitchTemp_c      , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::postBoilVolume_l , BrewNote::m_postBoilVolume_l , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projABV_pct      , BrewNote::m_projABV_pct      ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projAtten        , BrewNote::m_projAtten        ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projBoilGrav     , BrewNote::m_projBoilGrav     , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projEff_pct      , BrewNote::m_projEff_pct      ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projFermPoints   , BrewNote::m_projFermPoints                                               ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projFg           , BrewNote::m_projFg           , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projMashFinTemp_c, BrewNote::m_projMashFinTemp_c, Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projOg           , BrewNote::m_projOg           , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projPoints       , BrewNote::m_projPoints                                                   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projStrikeTemp_c , BrewNote::m_projStrikeTemp_c , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projVolIntoBK_l  , BrewNote::m_projVolIntoBK_l  , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::projVolIntoFerm_l, BrewNote::m_projVolIntoFerm_l, Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::sg               , BrewNote::m_sg               , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::strikeTemp_c     , BrewNote::m_strikeTemp_c     , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::volumeIntoBK_l   , BrewNote::m_volumeIntoBK_l   , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BrewNote::volumeIntoFerm_l , BrewNote::m_volumeIntoFerm_l , Measurement::PhysicalQuantity::Volume     ),
   },
   // Parent class lookup
   {&OwnedByRecipe::typeLookup}
};

// Initializers
BrewNote::BrewNote(QString name) :
   BrewNote(QDate(), name) {

   CONSTRUCTOR_END
   return;
}

BrewNote::BrewNote(Recipe const & recipe) :
   BrewNote(QDate(), "") {
   this->m_recipeId = recipe.key();

   CONSTRUCTOR_END
   return;
}

BrewNote::BrewNote(QDate dateNow, QString const & name) :
   OwnedByRecipe      {name},
   loading            {false  },
   m_brewDate         {dateNow},
   m_fermentDate      {       },
   m_notes            {""     },
   m_sg               {0.0    },
   m_abv              {0.0    },
   m_effIntoBK_pct    {0.0    },
   m_brewhouseEff_pct {0.0    },
   m_volumeIntoBK_l   {0.0    },
   m_strikeTemp_c     {0.0    },
   m_mashFinTemp_c    {0.0    },
   m_og               {0.0    },
   m_postBoilVolume_l {0.0    },
   m_volumeIntoFerm_l {0.0    },
   m_pitchTemp_c      {0.0    },
   m_fg               {0.0    },
   m_attenuation      {0.0    },
   m_finalVolume_l    {0.0    },
   m_boilOff_l        {0.0    },
   m_projBoilGrav     {0.0    },
   m_projVolIntoBK_l  {0.0    },
   m_projStrikeTemp_c {0.0    },
   m_projMashFinTemp_c{0.0    },
   m_projOg           {0.0    },
   m_projVolIntoFerm_l{0.0    },
   m_projFg           {0.0    },
   m_projEff_pct      {0.0    },
   m_projABV_pct      {0.0    },
   m_projPoints       {0.0    },
   m_projFermPoints   {0.0    },
   m_projAtten        {0.0    } {

   CONSTRUCTOR_END
   return;
}

BrewNote::BrewNote(NamedParameterBundle const & namedParameterBundle) :
   OwnedByRecipe{namedParameterBundle},
   loading            {false},
   SET_REGULAR_FROM_NPB (m_brewDate         , namedParameterBundle, PropertyNames::BrewNote::brewDate         ),
   SET_REGULAR_FROM_NPB (m_fermentDate      , namedParameterBundle, PropertyNames::BrewNote::fermentDate      ),
   SET_REGULAR_FROM_NPB (m_notes            , namedParameterBundle, PropertyNames::BrewNote::notes            ),
   SET_REGULAR_FROM_NPB (m_sg               , namedParameterBundle, PropertyNames::BrewNote::sg               ),
   SET_REGULAR_FROM_NPB (m_abv              , namedParameterBundle, PropertyNames::BrewNote::abv              ),
   SET_REGULAR_FROM_NPB (m_effIntoBK_pct    , namedParameterBundle, PropertyNames::BrewNote::effIntoBK_pct    ),
   SET_REGULAR_FROM_NPB (m_brewhouseEff_pct , namedParameterBundle, PropertyNames::BrewNote::brewhouseEff_pct ),
   SET_REGULAR_FROM_NPB (m_volumeIntoBK_l   , namedParameterBundle, PropertyNames::BrewNote::volumeIntoBK_l   ),
   SET_REGULAR_FROM_NPB (m_strikeTemp_c     , namedParameterBundle, PropertyNames::BrewNote::strikeTemp_c     ),
   SET_REGULAR_FROM_NPB (m_mashFinTemp_c    , namedParameterBundle, PropertyNames::BrewNote::mashFinTemp_c    ),
   SET_REGULAR_FROM_NPB (m_og               , namedParameterBundle, PropertyNames::BrewNote::og               ),
   SET_REGULAR_FROM_NPB (m_postBoilVolume_l , namedParameterBundle, PropertyNames::BrewNote::postBoilVolume_l ),
   SET_REGULAR_FROM_NPB (m_volumeIntoFerm_l , namedParameterBundle, PropertyNames::BrewNote::volumeIntoFerm_l ),
   SET_REGULAR_FROM_NPB (m_pitchTemp_c      , namedParameterBundle, PropertyNames::BrewNote::pitchTemp_c      ),
   SET_REGULAR_FROM_NPB (m_fg               , namedParameterBundle, PropertyNames::BrewNote::fg               ),
   SET_REGULAR_FROM_NPB (m_attenuation      , namedParameterBundle, PropertyNames::BrewNote::attenuation      ),
   SET_REGULAR_FROM_NPB (m_finalVolume_l    , namedParameterBundle, PropertyNames::BrewNote::finalVolume_l    ),
   SET_REGULAR_FROM_NPB (m_boilOff_l        , namedParameterBundle, PropertyNames::BrewNote::boilOff_l        ),
   SET_REGULAR_FROM_NPB (m_projBoilGrav     , namedParameterBundle, PropertyNames::BrewNote::projBoilGrav     ),
   SET_REGULAR_FROM_NPB (m_projVolIntoBK_l  , namedParameterBundle, PropertyNames::BrewNote::projVolIntoBK_l  ),
   SET_REGULAR_FROM_NPB (m_projStrikeTemp_c , namedParameterBundle, PropertyNames::BrewNote::projStrikeTemp_c ),
   SET_REGULAR_FROM_NPB (m_projMashFinTemp_c, namedParameterBundle, PropertyNames::BrewNote::projMashFinTemp_c),
   SET_REGULAR_FROM_NPB (m_projOg           , namedParameterBundle, PropertyNames::BrewNote::projOg           ),
   SET_REGULAR_FROM_NPB (m_projVolIntoFerm_l, namedParameterBundle, PropertyNames::BrewNote::projVolIntoFerm_l),
   SET_REGULAR_FROM_NPB (m_projFg           , namedParameterBundle, PropertyNames::BrewNote::projFg           ),
   SET_REGULAR_FROM_NPB (m_projEff_pct      , namedParameterBundle, PropertyNames::BrewNote::projEff_pct      ),
   SET_REGULAR_FROM_NPB (m_projABV_pct      , namedParameterBundle, PropertyNames::BrewNote::projABV_pct      ),
   SET_REGULAR_FROM_NPB (m_projPoints       , namedParameterBundle, PropertyNames::BrewNote::projPoints       ),
   SET_REGULAR_FROM_NPB (m_projFermPoints   , namedParameterBundle, PropertyNames::BrewNote::projFermPoints   ),
   SET_REGULAR_FROM_NPB (m_projAtten        , namedParameterBundle, PropertyNames::BrewNote::projAtten        ) {

   CONSTRUCTOR_END
   return;
}

BrewNote::BrewNote(BrewNote const & other) :
   OwnedByRecipe      {other                    },
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

   CONSTRUCTOR_END
   return;
}

BrewNote::~BrewNote() = default;

void BrewNote::populateNote(Recipe * parent) {
   this->m_recipeId = parent->key();

   // Since we have the recipe, lets set some defaults The order in which
   // these are done is very specific. Please do not modify them without some
   // serious testing.

   // Everything needs volumes of one type or another. But the individual
   // volumes are fairly independent of anything. Do them all first.
   double const boilSize_l = parent->boil() ? parent->boil()->preBoilSize_l().value_or(0.0) : 0.0;
   this->setProjVolIntoBK_l  (boilSize_l);
   this->setVolumeIntoBK_l   (boilSize_l);
   this->setPostBoilVolume_l (parent->postBoilVolume_l());
   this->setProjVolIntoFerm_l(parent->finalVolume_l());
   this->setVolumeIntoFerm_l (parent->finalVolume_l());
   this->setFinalVolume_l    (parent->finalVolume_l());

   auto equip = parent->equipment();
   if (equip) {
      double const boilTime_mins = parent->boil() ? parent->boil()->boilTime_mins() : Equipment::default_boilTime_mins;
      this->setBoilOff_l(
         equip->kettleEvaporationPerHour_l().value_or(Equipment::default_kettleEvaporationPerHour_l) * (boilTime_mins/60.0)
      );
   }

   auto sugars = parent->calcTotalPoints();
   this->setProjPoints(sugars.sugar_kg + sugars.sugar_kg_ignoreEfficiency);

   this->setProjFermPoints(sugars.sugar_kg + sugars.sugar_kg_ignoreEfficiency);

   // Out of the gate, we expect projected to be the measured.
   this->setSg( parent->boilGrav() );
   this->setProjBoilGrav(parent->boilGrav() );

   if (parent->mash()) {
      auto steps = parent->mash()->mashSteps();
      if (!steps.isEmpty()) {
         auto mStep = steps.at(0);

         if (mStep) {
            double const strikeTemp = mStep->infuseTemp_c().value_or(mStep->startTemp_c().value_or(0.0));
            this->setStrikeTemp_c(strikeTemp);
            this->setProjStrikeTemp_c(strikeTemp);

            double const endTemp = mStep->endTemp_c().value_or(mStep->startTemp_c().value_or(0.0));
            this->setMashFinTemp_c(endTemp);
            this->setProjMashFinTemp_c(endTemp);
         }

         if (steps.size() > 2) {
            // NOTE: Qt will complain that steps.size()-2 is always positive,
            // and therefore the internal assert that the index is positive is
            // bunk. This is OK, as we just checked that we will not underflow.
            mStep = steps.at( steps.size() - 2 );
            double const endTemp = mStep->endTemp_c().value_or(mStep->startTemp_c().value_or(0.0));
            this->setMashFinTemp_c(endTemp);
            this->setProjMashFinTemp_c(endTemp);
         }
      }
   }

   this->setOg( parent->og());
   this->setProjOg(parent->og());

   auto fermentation = parent->fermentation();
   if (fermentation &&
       fermentation->primary() &&
       fermentation->primary()->startTemp_c()) {
      this->setPitchTemp_c(*fermentation->primary()->startTemp_c()); // Replaces parent->primaryTemp_c()
   }

   this->setFg( parent->fg());
   this->setProjFg( parent->fg() );

   this->setProjEff_pct(parent->efficiency_pct());
   this->setProjABV_pct( parent->ABV_pct());

   double atten_pct = -1.0;
   auto const yeastAdditions = parent->yeastAdditions();
   for (auto const & yeastAddition : yeastAdditions) {
      if (yeastAddition->attenuation_pct() > atten_pct ) {
         atten_pct = yeastAddition->yeast()->attenuationTypical_pct();
      }
   }

   if (atten_pct < 0.0) {
      atten_pct = Yeast::DefaultAttenuation_pct; // Use an average attenuation;
   }
   this->setProjAtten(atten_pct);
   return;
}

// the v2 release had some bugs in the efficiency calcs. They have been fixed.
// This should allow the users to redo those calculations
void BrewNote::recalculateEff(Recipe* parent) {
   this->m_recipeId = parent->key();

   auto const sugars = parent->calcTotalPoints();
   this->setProjPoints(sugars.sugar_kg + sugars.sugar_kg_ignoreEfficiency);

//   sugars = parent->calcTotalPoints();
   this->setProjFermPoints(sugars.sugar_kg + sugars.sugar_kg_ignoreEfficiency);

   this->calculateEffIntoBK_pct();
   this->calculateBrewHouseEff_pct();
   return;
}

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
QDate   BrewNote::brewDate         () const { return this->m_brewDate; }
QString BrewNote::brewDate_str     () const { return this->m_brewDate.toString(); }
QString BrewNote::brewDate_short   () const { return Localization::displayDateUserFormated(this->m_brewDate); }
QDate   BrewNote::fermentDate      () const { return this->m_fermentDate; }
QString BrewNote::fermentDate_str  () const { return this->m_fermentDate.toString(); }
QString BrewNote::fermentDate_short() const { return Localization::displayDateUserFormated(this->m_fermentDate); }
QString BrewNote::notes            () const { return this->m_notes            ; }
double  BrewNote::sg               () const { return this->m_sg               ; }
double  BrewNote::abv              () const { return this->m_abv              ; }
double  BrewNote::attenuation      () const { return this->m_attenuation      ; }
double  BrewNote::volumeIntoBK_l   () const { return this->m_volumeIntoBK_l   ; }
double  BrewNote::effIntoBK_pct    () const { return this->m_effIntoBK_pct    ; }
double  BrewNote::brewhouseEff_pct () const { return this->m_brewhouseEff_pct ; }
double  BrewNote::strikeTemp_c     () const { return this->m_strikeTemp_c     ; }
double  BrewNote::mashFinTemp_c    () const { return this->m_mashFinTemp_c    ; }
double  BrewNote::og               () const { return this->m_og               ; }
double  BrewNote::volumeIntoFerm_l () const { return this->m_volumeIntoFerm_l ; }
double  BrewNote::postBoilVolume_l () const { return this->m_postBoilVolume_l ; }
double  BrewNote::pitchTemp_c      () const { return this->m_pitchTemp_c      ; }
double  BrewNote::fg               () const { return this->m_fg               ; }
double  BrewNote::finalVolume_l    () const { return this->m_finalVolume_l    ; }
double  BrewNote::projBoilGrav     () const { return this->m_projBoilGrav     ; }
double  BrewNote::projVolIntoBK_l  () const { return this->m_projVolIntoBK_l  ; }
double  BrewNote::projStrikeTemp_c () const { return this->m_projStrikeTemp_c ; }
double  BrewNote::projMashFinTemp_c() const { return this->m_projMashFinTemp_c; }
double  BrewNote::projOg           () const { return this->m_projOg           ; }
double  BrewNote::projVolIntoFerm_l() const { return this->m_projVolIntoFerm_l; }
double  BrewNote::projFg           () const { return this->m_projFg           ; }
double  BrewNote::projEff_pct      () const { return this->m_projEff_pct      ; }
double  BrewNote::projABV_pct      () const { return this->m_projABV_pct      ; }
double  BrewNote::projPoints       () const { return this->m_projPoints       ; }
double  BrewNote::projFermPoints   () const { return this->m_projFermPoints   ; }
double  BrewNote::projAtten        () const { return this->m_projAtten        ; }
double  BrewNote::boilOff_l        () const { return this->m_boilOff_l        ; }

// Setters=====================================================================
void BrewNote::setBrewDate(QDate const & date) {
   SET_AND_NOTIFY(PropertyNames::BrewNote::brewDate, this->m_brewDate, date);
   if (this->key() > 0) {
      // .:TBD:. Do we really need this special signal when we could use the generic changed one?
      emit brewDateChanged(date);
   }
   return;
}

void BrewNote::setFermentDate(QDate const & date) {
   SET_AND_NOTIFY(PropertyNames::BrewNote::fermentDate, this->m_fermentDate, date);
   return;
}

void BrewNote::setNotes(QString const& var) {
   SET_AND_NOTIFY(PropertyNames::BrewNote::notes, this->m_notes, var);
   return;
}

void BrewNote::setLoading(bool flag) { this->loading = flag; }

// These five items cause the calculated fields to change. I should do this
// with signals/slots, likely, but the *only* slot for the signal will be
// the brewnote.
void BrewNote::setSg(double var) {
   // I REALLY dislike this logic. It is too bloody intertwined
   SET_AND_NOTIFY(PropertyNames::BrewNote::sg, this->m_sg, var);

   // write the value to the DB if requested
   if ( ! this->loading ) {
      this->calculateEffIntoBK_pct();
      this->calculateOg();
   }
   return;
}

void BrewNote::setVolumeIntoBK_l(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewNote::volumeIntoBK_l, this->m_volumeIntoBK_l, var);

   if ( ! loading ) {
      this->calculateEffIntoBK_pct();
      this->calculateOg();
      this->calculateBrewHouseEff_pct();
   }
   return;
}

void BrewNote::setOg(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewNote::og, this->m_og, var);

   if ( ! loading ) {
      this->calculateBrewHouseEff_pct();
      this->calculateABV_pct();
      this->calculateActualABV_pct();
      this->calculateAttenuation_pct();
   }
   return;
}

void BrewNote::setVolumeIntoFerm_l(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewNote::volumeIntoFerm_l, this->m_volumeIntoFerm_l, var);

   if ( ! loading ) {
      this->calculateBrewHouseEff_pct();
   }
   return;
}

void BrewNote::setFg(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewNote::fg, this->m_fg, var);

   if ( !loading ) {
      this->calculateActualABV_pct();
      this->calculateAttenuation_pct();
   }
   return;
}

// This one is a bit of an odd ball. We need to convert to pure glucose points
// before we store it in the database.
// DO NOT ignore the loading flag. Just. Don't.
void BrewNote::setProjPoints(double var) {
   if ( loading ) {
      this->m_projPoints = var;
   } else {
      double const plato = Algorithms::getPlato(var, m_projVolIntoBK_l);
      double const total_g = Algorithms::PlatoToSG_20C20C( plato );
      double const convertPnts = (total_g - 1.0 ) * 1000;

      SET_AND_NOTIFY(PropertyNames::BrewNote::projPoints, this->m_projPoints, convertPnts);
   }
   return;
}

void BrewNote::setProjFermPoints(double var) {
   if ( loading ) {
      this->m_projFermPoints = var;
   } else {
      double const plato = Algorithms::getPlato(var, m_projVolIntoFerm_l);
      double const total_g = Algorithms::PlatoToSG_20C20C( plato );
      double const convertPnts = (total_g - 1.0 ) * 1000;

      SET_AND_NOTIFY(PropertyNames::BrewNote::projFermPoints, this->m_projFermPoints, convertPnts);
   }
   return;
}

void BrewNote::setABV              (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::abv              , this->m_abv              , var); }
void BrewNote::setAttenuation      (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::attenuation      , this->m_attenuation      , var); }
void BrewNote::setEffIntoBK_pct    (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::effIntoBK_pct    , this->m_effIntoBK_pct    , var); }
void BrewNote::setBrewhouseEff_pct (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::brewhouseEff_pct , this->m_brewhouseEff_pct , var); }
void BrewNote::setStrikeTemp_c     (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::strikeTemp_c     , this->m_strikeTemp_c     , var); }
void BrewNote::setMashFinTemp_c    (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::mashFinTemp_c    , this->m_mashFinTemp_c    , var); }
void BrewNote::setPostBoilVolume_l (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::postBoilVolume_l , this->m_postBoilVolume_l , var); }
void BrewNote::setPitchTemp_c      (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::pitchTemp_c      , this->m_pitchTemp_c      , var); }
void BrewNote::setFinalVolume_l    (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::finalVolume_l    , this->m_finalVolume_l    , var); }
void BrewNote::setProjBoilGrav     (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projBoilGrav     , this->m_projBoilGrav     , var); }
void BrewNote::setProjVolIntoBK_l  (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projVolIntoBK_l  , this->m_projVolIntoBK_l  , var); }
void BrewNote::setProjStrikeTemp_c (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projStrikeTemp_c , this->m_projStrikeTemp_c , var); }
void BrewNote::setProjMashFinTemp_c(double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projMashFinTemp_c, this->m_projMashFinTemp_c, var); }
void BrewNote::setProjOg           (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projOg           , this->m_projOg           , var); }
void BrewNote::setProjVolIntoFerm_l(double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projVolIntoFerm_l, this->m_projVolIntoFerm_l, var); }
void BrewNote::setProjFg           (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projFg           , this->m_projFg           , var); }
void BrewNote::setProjEff_pct      (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projEff_pct      , this->m_projEff_pct      , var); }
void BrewNote::setProjABV_pct      (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projABV_pct      , this->m_projABV_pct      , var); }
void BrewNote::setProjAtten        (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::projAtten        , this->m_projAtten        , var); }
void BrewNote::setBoilOff_l        (double var) { SET_AND_NOTIFY(PropertyNames::BrewNote::boilOff_l        , this->m_boilOff_l        , var); }

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewNote::calculateEffIntoBK_pct() {
   // I don't think we need a lot of math here. Points has already been
   // translated from SG into pure glucose points
   double const maxPoints = m_projPoints * m_projVolIntoBK_l;
   qDebug() <<
      Q_FUNC_INFO << "m_projPoints: " << m_projPoints << ", m_projVolIntoBK_l:" << m_projVolIntoBK_l <<
      ", maxPoints:" << maxPoints;

   double const actualPoints = (m_sg - 1) * 1000 * m_volumeIntoBK_l;
   qDebug() <<
      Q_FUNC_INFO << "m_sg:" << m_sg << ", m_volumeIntoBK_l:" << m_volumeIntoBK_l << ", actualPoints:" << actualPoints;
   // this can happen under normal circumstances (eg, load)
   if (maxPoints <= 0.0) {
      return 0.0;
   }

   double const effIntoBK = actualPoints/maxPoints * 100;
   qDebug() << Q_FUNC_INFO << "effIntoBK:" << effIntoBK;
   setEffIntoBK_pct(effIntoBK);

   return effIntoBK;
}

// The idea is that based on the preboil gravity, estimate what the actual OG will be.
double BrewNote::calculateOg() {
   double const points = (m_sg-1) * 1000;
   double const expectedVol = m_projVolIntoBK_l - m_boilOff_l;
   if (expectedVol <= 0.0) {
      return 0.0;
   }
   double const actualVol   = m_volumeIntoBK_l;
   double const cOG = 1+ ((points * actualVol / expectedVol) / 1000);
   this->setProjOg(cOG);
   return cOG;
}

double BrewNote::calculateBrewHouseEff_pct() {
   double const expectedPoints = m_projFermPoints * m_projVolIntoFerm_l;
   double const actualPoints = (m_og-1.0) * 1000.0 * m_volumeIntoFerm_l;

   double const brewhouseEff = actualPoints/expectedPoints * 100.0;
   this->setBrewhouseEff_pct(brewhouseEff);
   return brewhouseEff;
}

// Need to do some work here to figure out what the expected FG will be based
// on the actual OG, not the calculated.
double BrewNote::calculateABV_pct() {
   double const atten_pct = m_projAtten;

   // This looks weird, but the math works. (Yes, I am showing my work)
   // 1 + [(og-1) * 1000 * (1.0 - %/100)] / 1000  =
   // 1 + [(og - 1) * (1.0 - %/100)]
   double const estFg = 1 + ((m_og-1.0)*(1.0 - atten_pct/100.0));

   double const calculatedAbv_pct = Algorithms::abvFromOgAndFg(this->m_og, estFg);

   this->setProjABV_pct(calculatedAbv_pct);

   return calculatedAbv_pct;
}

double BrewNote::calculateActualABV_pct() {
   double const abv_pct = Algorithms::abvFromOgAndFg(this->m_og, this->m_fg);
   this->setABV(abv_pct);
   return abv_pct;
}

double BrewNote::calculateAttenuation_pct() {
    // Calculate measured attenuation based on user-reported values for
    // post-boil OG and post-ferment FG
    double const attenuation = ((m_og - m_fg) / (m_og - 1)) * 100;
    this->setAttenuation(attenuation);
    return attenuation;
}

QList<std::shared_ptr<BrewNote>> BrewNote::ownedBy(Recipe const & recipe) {
   return recipe.brewNotes();
}
