/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/BrewLog.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
 =====================================================================================================================*/
#include "model/BrewLog.h"

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
   #include "moc_BrewLog.cpp"
#endif

QString BrewLog::localisedName                  () { return tr("Brew Log"                         ); }
QString BrewLog::localisedName_abv              () { return tr("ABV"                              ); }
QString BrewLog::localisedName_attenuation      () { return tr("Attenuation"                      ); }
QString BrewLog::localisedName_boilOff_l        () { return tr("Boil-Off"                         ); }
QString BrewLog::localisedName_brewDate         () { return tr("Brew Date"                        ); }
QString BrewLog::localisedName_brewhouseEff_pct () { return tr("Brewhouse Efficiency"             ); }
QString BrewLog::localisedName_effIntoBK_pct    () { return tr("Efficiency Into Boil Kettle"      ); }
QString BrewLog::localisedName_fermentDate      () { return tr("Ferment Date"                     ); }
QString BrewLog::localisedName_fg               () { return tr("FG"                               ); }
QString BrewLog::localisedName_finalVolume_l    () { return tr("Final Volume"                     ); }
QString BrewLog::localisedName_mashFinTemp_c    () { return tr("Mash Final Temperature"           ); }
QString BrewLog::localisedName_notes            () { return tr("Notes"                            ); }
QString BrewLog::localisedName_og               () { return tr("OG"                               ); }
QString BrewLog::localisedName_pitchTemp_c      () { return tr("Pitch Temp"                       ); }
QString BrewLog::localisedName_postBoilVolume_l () { return tr("Post-Boil Volume"                 ); }
QString BrewLog::localisedName_projABV_pct      () { return tr("Projected ABV"                    ); }
QString BrewLog::localisedName_projAtten        () { return tr("Projected Attenuation"            ); }
QString BrewLog::localisedName_projBoilGrav     () { return tr("Projected Boil Gravity"           ); }
QString BrewLog::localisedName_projEff_pct      () { return tr("Projected Efficiency"             ); }
QString BrewLog::localisedName_projFermPoints   () { return tr("Projected Fermentation Points"    ); }
QString BrewLog::localisedName_projFg           () { return tr("Projected FG"                     ); }
QString BrewLog::localisedName_projMashFinTemp_c() { return tr("Projected Mash Final Temperature" ); }
QString BrewLog::localisedName_projOg           () { return tr("Projected OG"                     ); }
QString BrewLog::localisedName_projPoints       () { return tr("Projected Points"                 ); }
QString BrewLog::localisedName_projStrikeTemp_c () { return tr("Projected Strike Temperature"     ); }
QString BrewLog::localisedName_projVolIntoBK_l  () { return tr("Projected Volume Into Boil Kettle"); }
QString BrewLog::localisedName_projVolIntoFerm_l() { return tr("Projected Volume Into Fermentor"  ); }
QString BrewLog::localisedName_sg               () { return tr("SG"                               ); }
QString BrewLog::localisedName_strikeTemp_c     () { return tr("Strike Temperature"               ); }
QString BrewLog::localisedName_volumeIntoBK_l   () { return tr("Volume Into Boil Kettle"          ); }
QString BrewLog::localisedName_volumeIntoFerm_l () { return tr("Volume Into Fermentor"            ); }

std::strong_ordering BrewLog::operator<=>(BrewLog const & other) const {
   // If two BrewLogs have the same Date, then we use name (ie batch number) to break the tie
   return Utils::Auto3WayCompare(this->m_brewDate, other.m_brewDate,
                                 this->name()     , other.name());
}

bool BrewLog::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   BrewLog const & rhs = static_cast<BrewLog const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_brewDate   , PropertyNames::BrewLog::brewDate   , propertiesThatDiffer)
   );
}

ObjectStore & BrewLog::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<BrewLog>::getInstance();
}

TypeLookup const BrewLog::typeLookup {
   "BrewLog",
   {
      // Note that we need Enums to be treated as ints for the purposes of type lookup
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, abv              , m_abv              ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, attenuation      , m_attenuation      ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, boilOff_l        , m_boilOff_l        , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, brewDate         , m_brewDate         ,           NonPhysicalQuantity::Date       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, brewhouseEff_pct , m_brewhouseEff_pct ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, effIntoBK_pct    , m_effIntoBK_pct    ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, fermentDate      , m_fermentDate      ,           NonPhysicalQuantity::Date       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, fg               , m_fg               , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, finalVolume_l    , m_finalVolume_l    , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, mashFinTemp_c    , m_mashFinTemp_c    , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, notes            , m_notes            ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, og               , m_og               , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, pitchTemp_c      , m_pitchTemp_c      , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, postBoilVolume_l , m_postBoilVolume_l , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projABV_pct      , m_projABV_pct      ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projAtten        , m_projAtten        ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projBoilGrav     , m_projBoilGrav     , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projEff_pct      , m_projEff_pct      ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projFermPoints   , m_projFermPoints                                               ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projFg           , m_projFg           , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projMashFinTemp_c, m_projMashFinTemp_c, Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projOg           , m_projOg           , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projPoints       , m_projPoints                                                   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projStrikeTemp_c , m_projStrikeTemp_c , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projVolIntoBK_l  , m_projVolIntoBK_l  , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, projVolIntoFerm_l, m_projVolIntoFerm_l, Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, sg               , m_sg               , Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, strikeTemp_c     , m_strikeTemp_c     , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, volumeIntoBK_l   , m_volumeIntoBK_l   , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, volumeIntoFerm_l , m_volumeIntoFerm_l , Measurement::PhysicalQuantity::Volume     ),
   },
   // Parent class lookup
   {&OwnedByRecipe::typeLookup}
};

// Initializers
BrewLog::BrewLog(QString name) :
   BrewLog(QDate::currentDate(), name) {

   CONSTRUCTOR_END
   return;
}

BrewLog::BrewLog(Recipe const & recipe) :
   BrewLog(QDate::currentDate(), "") {
   this->m_recipeId = recipe.key();

   CONSTRUCTOR_END
   return;
}

BrewLog::BrewLog(QDate dateNow, QString const & name) :
   OwnedByRecipe      {name},
   loading            {false  },
   m_brewDate         {dateNow},
   m_fermentDate      {dateNow},
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

BrewLog::BrewLog(NamedParameterBundle const & namedParameterBundle) :
   OwnedByRecipe{namedParameterBundle},
   loading            {false},
   SET_REGULAR_FROM_NPB (m_brewDate         , namedParameterBundle, PropertyNames::BrewLog::brewDate         ),
   SET_REGULAR_FROM_NPB (m_fermentDate      , namedParameterBundle, PropertyNames::BrewLog::fermentDate      ),
   SET_REGULAR_FROM_NPB (m_notes            , namedParameterBundle, PropertyNames::BrewLog::notes            ),
   SET_REGULAR_FROM_NPB (m_sg               , namedParameterBundle, PropertyNames::BrewLog::sg               ),
   SET_REGULAR_FROM_NPB (m_abv              , namedParameterBundle, PropertyNames::BrewLog::abv              ),
   SET_REGULAR_FROM_NPB (m_effIntoBK_pct    , namedParameterBundle, PropertyNames::BrewLog::effIntoBK_pct    ),
   SET_REGULAR_FROM_NPB (m_brewhouseEff_pct , namedParameterBundle, PropertyNames::BrewLog::brewhouseEff_pct ),
   SET_REGULAR_FROM_NPB (m_volumeIntoBK_l   , namedParameterBundle, PropertyNames::BrewLog::volumeIntoBK_l   ),
   SET_REGULAR_FROM_NPB (m_strikeTemp_c     , namedParameterBundle, PropertyNames::BrewLog::strikeTemp_c     ),
   SET_REGULAR_FROM_NPB (m_mashFinTemp_c    , namedParameterBundle, PropertyNames::BrewLog::mashFinTemp_c    ),
   SET_REGULAR_FROM_NPB (m_og               , namedParameterBundle, PropertyNames::BrewLog::og               ),
   SET_REGULAR_FROM_NPB (m_postBoilVolume_l , namedParameterBundle, PropertyNames::BrewLog::postBoilVolume_l ),
   SET_REGULAR_FROM_NPB (m_volumeIntoFerm_l , namedParameterBundle, PropertyNames::BrewLog::volumeIntoFerm_l ),
   SET_REGULAR_FROM_NPB (m_pitchTemp_c      , namedParameterBundle, PropertyNames::BrewLog::pitchTemp_c      ),
   SET_REGULAR_FROM_NPB (m_fg               , namedParameterBundle, PropertyNames::BrewLog::fg               ),
   SET_REGULAR_FROM_NPB (m_attenuation      , namedParameterBundle, PropertyNames::BrewLog::attenuation      ),
   SET_REGULAR_FROM_NPB (m_finalVolume_l    , namedParameterBundle, PropertyNames::BrewLog::finalVolume_l    ),
   SET_REGULAR_FROM_NPB (m_boilOff_l        , namedParameterBundle, PropertyNames::BrewLog::boilOff_l        ),
   SET_REGULAR_FROM_NPB (m_projBoilGrav     , namedParameterBundle, PropertyNames::BrewLog::projBoilGrav     ),
   SET_REGULAR_FROM_NPB (m_projVolIntoBK_l  , namedParameterBundle, PropertyNames::BrewLog::projVolIntoBK_l  ),
   SET_REGULAR_FROM_NPB (m_projStrikeTemp_c , namedParameterBundle, PropertyNames::BrewLog::projStrikeTemp_c ),
   SET_REGULAR_FROM_NPB (m_projMashFinTemp_c, namedParameterBundle, PropertyNames::BrewLog::projMashFinTemp_c),
   SET_REGULAR_FROM_NPB (m_projOg           , namedParameterBundle, PropertyNames::BrewLog::projOg           ),
   SET_REGULAR_FROM_NPB (m_projVolIntoFerm_l, namedParameterBundle, PropertyNames::BrewLog::projVolIntoFerm_l),
   SET_REGULAR_FROM_NPB (m_projFg           , namedParameterBundle, PropertyNames::BrewLog::projFg           ),
   SET_REGULAR_FROM_NPB (m_projEff_pct      , namedParameterBundle, PropertyNames::BrewLog::projEff_pct      ),
   SET_REGULAR_FROM_NPB (m_projABV_pct      , namedParameterBundle, PropertyNames::BrewLog::projABV_pct      ),
   SET_REGULAR_FROM_NPB (m_projPoints       , namedParameterBundle, PropertyNames::BrewLog::projPoints       ),
   SET_REGULAR_FROM_NPB (m_projFermPoints   , namedParameterBundle, PropertyNames::BrewLog::projFermPoints   ),
   SET_REGULAR_FROM_NPB (m_projAtten        , namedParameterBundle, PropertyNames::BrewLog::projAtten        ) {

   CONSTRUCTOR_END
   return;
}

BrewLog::BrewLog(BrewLog const & other) :
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

BrewLog::~BrewLog() = default;

void BrewLog::populateNote(Recipe * parent) {
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

   if (auto const equip = parent->equipment()) {
      double const boilTime_mins = parent->boil() ? parent->boil()->boilTime_mins() : Equipment::default_boilTime_mins;
      this->setBoilOff_l(
         equip->kettleEvaporationPerHour_l().value_or(Equipment::default_kettleEvaporationPerHour_l) * (boilTime_mins/60.0)
      );
   }

   auto const sugars = parent->calcTotalPoints();
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
            double const strikeTemp = mStep->infuseTemp_c().value_or(mStep->startTemp_c());
            this->setStrikeTemp_c(strikeTemp);
            this->setProjStrikeTemp_c(strikeTemp);

            double const endTemp = mStep->endTemp_c().value_or(mStep->startTemp_c());
            this->setMashFinTemp_c(endTemp);
            this->setProjMashFinTemp_c(endTemp);
         }

         if (steps.size() > 2) {
            // NOTE: Qt will complain that steps.size()-2 is always positive,
            // and therefore the internal assert that the index is positive is
            // bunk. This is OK, as we just checked that we will not underflow.
            mStep = steps.at(steps.size() - 2);
            double const endTemp = mStep->endTemp_c().value_or(mStep->startTemp_c());
            this->setMashFinTemp_c(endTemp);
            this->setProjMashFinTemp_c(endTemp);
         }
      }
   }

   this->setOg(parent->og());
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
void BrewLog::recalculateEff(Recipe* parent) {
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
QDate   BrewLog::brewDate         () const { return this->m_brewDate; }
QString BrewLog::brewDate_str     () const { return this->m_brewDate.toString(); }
QString BrewLog::brewDate_short   () const { return Localization::displayDateUserFormated(this->m_brewDate); }
QDate   BrewLog::fermentDate      () const { return this->m_fermentDate; }
QString BrewLog::fermentDate_str  () const { return this->m_fermentDate.toString(); }
QString BrewLog::fermentDate_short() const { return Localization::displayDateUserFormated(this->m_fermentDate); }
QString BrewLog::notes            () const { return this->m_notes            ; }
double  BrewLog::sg               () const { return this->m_sg               ; }
double  BrewLog::abv              () const { return this->m_abv              ; }
double  BrewLog::attenuation      () const { return this->m_attenuation      ; }
double  BrewLog::volumeIntoBK_l   () const { return this->m_volumeIntoBK_l   ; }
double  BrewLog::effIntoBK_pct    () const { return this->m_effIntoBK_pct    ; }
double  BrewLog::brewhouseEff_pct () const { return this->m_brewhouseEff_pct ; }
double  BrewLog::strikeTemp_c     () const { return this->m_strikeTemp_c     ; }
double  BrewLog::mashFinTemp_c    () const { return this->m_mashFinTemp_c    ; }
double  BrewLog::og               () const { return this->m_og               ; }
double  BrewLog::volumeIntoFerm_l () const { return this->m_volumeIntoFerm_l ; }
double  BrewLog::postBoilVolume_l () const { return this->m_postBoilVolume_l ; }
double  BrewLog::pitchTemp_c      () const { return this->m_pitchTemp_c      ; }
double  BrewLog::fg               () const { return this->m_fg               ; }
double  BrewLog::finalVolume_l    () const { return this->m_finalVolume_l    ; }
double  BrewLog::projBoilGrav     () const { return this->m_projBoilGrav     ; }
double  BrewLog::projVolIntoBK_l  () const { return this->m_projVolIntoBK_l  ; }
double  BrewLog::projStrikeTemp_c () const { return this->m_projStrikeTemp_c ; }
double  BrewLog::projMashFinTemp_c() const { return this->m_projMashFinTemp_c; }
double  BrewLog::projOg           () const { return this->m_projOg           ; }
double  BrewLog::projVolIntoFerm_l() const { return this->m_projVolIntoFerm_l; }
double  BrewLog::projFg           () const { return this->m_projFg           ; }
double  BrewLog::projEff_pct      () const { return this->m_projEff_pct      ; }
double  BrewLog::projABV_pct      () const { return this->m_projABV_pct      ; }
double  BrewLog::projPoints       () const { return this->m_projPoints       ; }
double  BrewLog::projFermPoints   () const { return this->m_projFermPoints   ; }
double  BrewLog::projAtten        () const { return this->m_projAtten        ; }
double  BrewLog::boilOff_l        () const { return this->m_boilOff_l        ; }

// Setters=====================================================================
void BrewLog::setBrewDate(QDate const & date) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::brewDate, this->m_brewDate, date);
   if (this->key() > 0) {
      // .:TBD:. Do we really need this special signal when we could use the generic changed one?
      emit brewDateChanged(date);
   }
   return;
}

void BrewLog::setFermentDate(QDate const & date) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::fermentDate, this->m_fermentDate, date);
   return;
}

void BrewLog::setNotes(QString const& var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::notes, this->m_notes, var);
   return;
}

void BrewLog::setLoading(bool flag) { this->loading = flag; }

// These five items cause the calculated fields to change. I should do this
// with signals/slots, likely, but the *only* slot for the signal will be
// the BrewLog.
void BrewLog::setSg(double var) {
   // I REALLY dislike this logic. It is too bloody intertwined
   SET_AND_NOTIFY(PropertyNames::BrewLog::sg, this->m_sg, var);

   // write the value to the DB if requested
   if ( ! this->loading ) {
      this->calculateEffIntoBK_pct();
      this->calculateOg();
   }
   return;
}

void BrewLog::setVolumeIntoBK_l(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::volumeIntoBK_l, this->m_volumeIntoBK_l, var);

   if ( ! loading ) {
      this->calculateEffIntoBK_pct();
      this->calculateOg();
      this->calculateBrewHouseEff_pct();
   }
   return;
}

void BrewLog::setOg(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::og, this->m_og, var);

   if ( ! loading ) {
      this->calculateBrewHouseEff_pct();
      this->calculateABV_pct();
      this->calculateActualABV_pct();
      this->calculateAttenuation_pct();
   }
   return;
}

void BrewLog::setVolumeIntoFerm_l(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::volumeIntoFerm_l, this->m_volumeIntoFerm_l, var);

   if ( ! loading ) {
      this->calculateBrewHouseEff_pct();
   }
   return;
}

void BrewLog::setFg(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::fg, this->m_fg, var);

   if ( !loading ) {
      this->calculateActualABV_pct();
      this->calculateAttenuation_pct();
   }
   return;
}

// This one is a bit of an odd ball. We need to convert to pure glucose points
// before we store it in the database.
// DO NOT ignore the loading flag. Just. Don't.
void BrewLog::setProjPoints(double var) {
   if ( loading ) {
      this->m_projPoints = var;
   } else {
      double const plato = Algorithms::getPlato(var, m_projVolIntoBK_l);
      double const total_g = Algorithms::PlatoToSG_20C20C( plato );
      double const convertPnts = (total_g - 1.0 ) * 1000;

      SET_AND_NOTIFY(PropertyNames::BrewLog::projPoints, this->m_projPoints, convertPnts);
   }
   return;
}

void BrewLog::setProjFermPoints(double var) {
   if ( loading ) {
      this->m_projFermPoints = var;
   } else {
      double const plato = Algorithms::getPlato(var, m_projVolIntoFerm_l);
      double const total_g = Algorithms::PlatoToSG_20C20C( plato );
      double const convertPnts = (total_g - 1.0 ) * 1000;

      SET_AND_NOTIFY(PropertyNames::BrewLog::projFermPoints, this->m_projFermPoints, convertPnts);
   }
   return;
}

void BrewLog::setABV              (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::abv              , this->m_abv              , var); }
void BrewLog::setAttenuation      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::attenuation      , this->m_attenuation      , var); }
void BrewLog::setEffIntoBK_pct    (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::effIntoBK_pct    , this->m_effIntoBK_pct    , var); }
void BrewLog::setBrewhouseEff_pct (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::brewhouseEff_pct , this->m_brewhouseEff_pct , var); }
void BrewLog::setStrikeTemp_c     (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::strikeTemp_c     , this->m_strikeTemp_c     , var); }
void BrewLog::setMashFinTemp_c    (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::mashFinTemp_c    , this->m_mashFinTemp_c    , var); }
void BrewLog::setPostBoilVolume_l (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::postBoilVolume_l , this->m_postBoilVolume_l , var); }
void BrewLog::setPitchTemp_c      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::pitchTemp_c      , this->m_pitchTemp_c      , var); }
void BrewLog::setFinalVolume_l    (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::finalVolume_l    , this->m_finalVolume_l    , var); }
void BrewLog::setProjBoilGrav     (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projBoilGrav     , this->m_projBoilGrav     , var); }
void BrewLog::setProjVolIntoBK_l  (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projVolIntoBK_l  , this->m_projVolIntoBK_l  , var); }
void BrewLog::setProjStrikeTemp_c (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projStrikeTemp_c , this->m_projStrikeTemp_c , var); }
void BrewLog::setProjMashFinTemp_c(double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projMashFinTemp_c, this->m_projMashFinTemp_c, var); }
void BrewLog::setProjOg           (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projOg           , this->m_projOg           , var); }
void BrewLog::setProjVolIntoFerm_l(double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projVolIntoFerm_l, this->m_projVolIntoFerm_l, var); }
void BrewLog::setProjFg           (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projFg           , this->m_projFg           , var); }
void BrewLog::setProjEff_pct      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projEff_pct      , this->m_projEff_pct      , var); }
void BrewLog::setProjABV_pct      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projABV_pct      , this->m_projABV_pct      , var); }
void BrewLog::setProjAtten        (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::projAtten        , this->m_projAtten        , var); }
void BrewLog::setBoilOff_l        (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::boilOff_l        , this->m_boilOff_l        , var); }

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewLog::calculateEffIntoBK_pct() {
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
double BrewLog::calculateOg() {
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

double BrewLog::calculateBrewHouseEff_pct() {
   double const expectedPoints = m_projFermPoints * m_projVolIntoFerm_l;
   double const actualPoints = (m_og-1.0) * 1000.0 * m_volumeIntoFerm_l;

   double const brewhouseEff = actualPoints/expectedPoints * 100.0;
   this->setBrewhouseEff_pct(brewhouseEff);
   return brewhouseEff;
}

// Need to do some work here to figure out what the expected FG will be based
// on the actual OG, not the calculated.
double BrewLog::calculateABV_pct() {
   double const atten_pct = m_projAtten;

   // This looks weird, but the math works. (Yes, I am showing my work)
   // 1 + [(og-1) * 1000 * (1.0 - %/100)] / 1000  =
   // 1 + [(og - 1) * (1.0 - %/100)]
   double const estFg = 1 + ((m_og-1.0)*(1.0 - atten_pct/100.0));

   double const calculatedAbv_pct = Algorithms::abvFromOgAndFg(this->m_og, estFg);

   this->setProjABV_pct(calculatedAbv_pct);

   return calculatedAbv_pct;
}

double BrewLog::calculateActualABV_pct() {
   double const abv_pct = Algorithms::abvFromOgAndFg(this->m_og, this->m_fg);
   this->setABV(abv_pct);
   return abv_pct;
}

double BrewLog::calculateAttenuation_pct() {
    // Calculate measured attenuation based on user-reported values for
    // post-boil OG and post-ferment FG
    double const attenuation = ((m_og - m_fg) / (m_og - 1)) * 100;
    this->setAttenuation(attenuation);
    return attenuation;
}

QList<std::shared_ptr<BrewLog>> BrewLog::ownedBy(Recipe const & recipe) {
   return recipe.brewLogs();
}
