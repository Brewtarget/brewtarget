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

QString BrewLog::localisedName                              () { return tr("Brew Log"                          ); }
QString BrewLog::localisedName_brewDate                     () { return tr("Brew Date"                         ); }
QString BrewLog::localisedName_computedAlcoholByVolume_pct  () { return tr("ABV"                               ); }
QString BrewLog::localisedName_computedAttenuation_pct      () { return tr("Attenuation"                       ); }
QString BrewLog::localisedName_computedEfficiency_pct       () { return tr("Brewhouse Efficiency"              ); }
QString BrewLog::localisedName_computedPreBoilEfficiency_pct() { return tr("Efficiency Into Boil Kettle"       ); }
QString BrewLog::localisedName_expectedAlcoholByVolume_pct  () { return tr("Projected ABV"                     ); }
QString BrewLog::localisedName_expectedAttenuation_pct      () { return tr("Projected Attenuation"             ); }
QString BrewLog::localisedName_expectedBoilOff_l            () { return tr("Expected Boil-Off"                 ); }
QString BrewLog::localisedName_expectedEfficiency_pct       () { return tr("Projected Efficiency"              ); }
QString BrewLog::localisedName_expectedFinalGravity_sg      () { return tr("Projected FG"                      ); }
QString BrewLog::localisedName_expectedMashFinalTemp_c      () { return tr("Projected Mash Final Temperature"  ); }
QString BrewLog::localisedName_expectedOriginalGravity_sg   () { return tr("Planned OG"                        ); }
QString BrewLog::localisedName_expectedPreBoilGravity_sg    () { return tr("Expected pre-boil specific gravity"); }
QString BrewLog::localisedName_expectedPreBoilVolume_l      () { return tr("Projected Volume Into Boil Kettle" ); }
QString BrewLog::localisedName_expectedStrikeTemp_c         () { return tr("Projected Strike Temperature"      ); }
QString BrewLog::localisedName_expectedVolumeIntoFermentor_l() { return tr("Projected Volume Into Fermentor"   ); }
QString BrewLog::localisedName_fermentDate                  () { return tr("Ferment Date"                      ); }
QString BrewLog::localisedName_forecastOriginalGravity_sg   () { return tr("Projected OG"                      ); }
QString BrewLog::localisedName_measuredFinalGravity_sg      () { return tr("Measured FG"                       ); }
QString BrewLog::localisedName_measuredFinalVolume_l        () { return tr("Final Volume"                      ); }
QString BrewLog::localisedName_measuredMashFinalTemp_c      () { return tr("Mash Final Temperature"            ); }
QString BrewLog::localisedName_measuredOriginalGravity_sg   () { return tr("OG"                                ); }
QString BrewLog::localisedName_measuredPitchTemp_c          () { return tr("Measured Pitch Temperature"        ); }
QString BrewLog::localisedName_measuredPostBoilVolume_l     () { return tr("Post-Boil Volume"                  ); }
QString BrewLog::localisedName_measuredPreBoilGravity_sg    () { return tr("Measured pre-boil specific gravity"); }
QString BrewLog::localisedName_measuredPreBoilVolume_l      () { return tr("Volume Into Boil Kettle"           ); }
QString BrewLog::localisedName_measuredStrikeTemp_c         () { return tr("Strike Temperature"                ); }
QString BrewLog::localisedName_measuredVolumeIntoFermentor_l() { return tr("Volume Into Fermentor"             ); }
QString BrewLog::localisedName_notes                        () { return tr("Notes"                             ); }
QString BrewLog::localisedName_projFermPoints               () { return tr("Projected Fermentation Points"     ); }

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
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedBoilOff_l            , m_expectedBoilOff_l            , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, brewDate                     , m_brewDate                     ,           NonPhysicalQuantity::Date       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, computedAlcoholByVolume_pct  , m_computedAlcoholByVolume_pct  ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, computedAttenuation_pct      , m_computedAttenuation_pct      ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, computedEfficiency_pct       , m_computedEfficiency_pct       ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, computedPreBoilEfficiency_pct, m_computedPreBoilEfficiency_pct,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedAlcoholByVolume_pct  , m_expectedAlcoholByVolume_pct  ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedAttenuation_pct      , m_expectedAttenuation_pct      ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedEfficiency_pct       , m_expectedEfficiency_pct       ,           NonPhysicalQuantity::Percentage ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedFinalGravity_sg      , m_expectedFinalGravity_sg      , Measurement::PhysicalQuantity::Gravity    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedMashFinalTemp_c      , m_expectedMashFinalTemp_c      , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedOriginalGravity_sg   , m_expectedOriginalGravity_sg   , Measurement::PhysicalQuantity::Gravity    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedPreBoilGravity_sg    , m_expectedPreBoilGravity_sg    , Measurement::PhysicalQuantity::Gravity    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedPreBoilVolume_l      , m_expectedPreBoilVolume_l      , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedStrikeTemp_c         , m_expectedStrikeTemp_c         , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, expectedVolumeIntoFermentor_l, m_expectedVolumeIntoFermentor_l, Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, fermentDate                  , m_fermentDate                  ,           NonPhysicalQuantity::Date       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, forecastOriginalGravity_sg   , m_forecastOriginalGravity_sg   , Measurement::PhysicalQuantity::Gravity    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredFinalGravity_sg      , m_measuredFinalGravity_sg      , Measurement::PhysicalQuantity::Gravity    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredFinalVolume_l        , m_measuredFinalVolume_l        , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredMashFinalTemp_c      , m_measuredMashFinalTemp_c      , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredOriginalGravity_sg   , m_measuredOriginalGravity_sg   , Measurement::PhysicalQuantity::Gravity    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredPitchTemp_c          , m_measuredPitchTemp_c          , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredPostBoilVolume_l     , m_measuredPostBoilVolume_l     , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredPreBoilGravity_sg    , m_measuredPreBoilGravity_sg    , Measurement::PhysicalQuantity::Gravity    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredPreBoilVolume_l      , m_measuredPreBoilVolume_l      , Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredStrikeTemp_c         , m_measuredStrikeTemp_c         , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, measuredVolumeIntoFermentor_l, m_measuredVolumeIntoFermentor_l, Measurement::PhysicalQuantity::Volume     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(BrewLog, notes                        , m_notes                        ,           NonPhysicalQuantity::String     ),
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
   m_fermentDate      {dateNow} {

   CONSTRUCTOR_END
   return;
}

BrewLog::BrewLog(NamedParameterBundle const & namedParameterBundle) :
   OwnedByRecipe{namedParameterBundle},
   loading            {false},
   SET_REGULAR_FROM_NPB (m_brewDate                     , namedParameterBundle, PropertyNames::BrewLog::brewDate                     ),
   SET_REGULAR_FROM_NPB (m_fermentDate                  , namedParameterBundle, PropertyNames::BrewLog::fermentDate                  ),
   SET_REGULAR_FROM_NPB (m_notes                        , namedParameterBundle, PropertyNames::BrewLog::notes                        ),
   SET_REGULAR_FROM_NPB (m_measuredPreBoilGravity_sg    , namedParameterBundle, PropertyNames::BrewLog::measuredPreBoilGravity_sg    ),
   SET_REGULAR_FROM_NPB (m_computedAlcoholByVolume_pct  , namedParameterBundle, PropertyNames::BrewLog::computedAlcoholByVolume_pct  ),
   SET_REGULAR_FROM_NPB (m_computedPreBoilEfficiency_pct, namedParameterBundle, PropertyNames::BrewLog::computedPreBoilEfficiency_pct),
   SET_REGULAR_FROM_NPB (m_computedEfficiency_pct       , namedParameterBundle, PropertyNames::BrewLog::computedEfficiency_pct       ),
   SET_REGULAR_FROM_NPB (m_measuredPreBoilVolume_l      , namedParameterBundle, PropertyNames::BrewLog::measuredPreBoilVolume_l      ),
   SET_REGULAR_FROM_NPB (m_measuredStrikeTemp_c         , namedParameterBundle, PropertyNames::BrewLog::measuredStrikeTemp_c         ),
   SET_REGULAR_FROM_NPB (m_measuredMashFinalTemp_c      , namedParameterBundle, PropertyNames::BrewLog::measuredMashFinalTemp_c      ),
   SET_REGULAR_FROM_NPB (m_measuredOriginalGravity_sg   , namedParameterBundle, PropertyNames::BrewLog::measuredOriginalGravity_sg   ),
   SET_REGULAR_FROM_NPB (m_measuredPostBoilVolume_l     , namedParameterBundle, PropertyNames::BrewLog::measuredPostBoilVolume_l     ),
   SET_REGULAR_FROM_NPB (m_measuredVolumeIntoFermentor_l, namedParameterBundle, PropertyNames::BrewLog::measuredVolumeIntoFermentor_l),
   SET_REGULAR_FROM_NPB (m_measuredPitchTemp_c          , namedParameterBundle, PropertyNames::BrewLog::measuredPitchTemp_c          ),
   SET_REGULAR_FROM_NPB (m_measuredFinalGravity_sg      , namedParameterBundle, PropertyNames::BrewLog::measuredFinalGravity_sg      ),
   SET_REGULAR_FROM_NPB (m_computedAttenuation_pct      , namedParameterBundle, PropertyNames::BrewLog::computedAttenuation_pct      ),
   SET_REGULAR_FROM_NPB (m_measuredFinalVolume_l        , namedParameterBundle, PropertyNames::BrewLog::measuredFinalVolume_l        ),
   SET_REGULAR_FROM_NPB (m_expectedBoilOff_l            , namedParameterBundle, PropertyNames::BrewLog::expectedBoilOff_l            ),
   SET_REGULAR_FROM_NPB (m_expectedPreBoilGravity_sg    , namedParameterBundle, PropertyNames::BrewLog::expectedPreBoilGravity_sg    ),
   SET_REGULAR_FROM_NPB (m_expectedPreBoilVolume_l      , namedParameterBundle, PropertyNames::BrewLog::expectedPreBoilVolume_l      ),
   SET_REGULAR_FROM_NPB (m_expectedStrikeTemp_c         , namedParameterBundle, PropertyNames::BrewLog::expectedStrikeTemp_c         ),
   SET_REGULAR_FROM_NPB (m_expectedMashFinalTemp_c      , namedParameterBundle, PropertyNames::BrewLog::expectedMashFinalTemp_c      ),
   SET_REGULAR_FROM_NPB (m_expectedOriginalGravity_sg   , namedParameterBundle, PropertyNames::BrewLog::expectedOriginalGravity_sg   ),
   SET_REGULAR_FROM_NPB (m_forecastOriginalGravity_sg   , namedParameterBundle, PropertyNames::BrewLog::forecastOriginalGravity_sg   ),
   SET_REGULAR_FROM_NPB (m_expectedVolumeIntoFermentor_l, namedParameterBundle, PropertyNames::BrewLog::expectedVolumeIntoFermentor_l),
   SET_REGULAR_FROM_NPB (m_expectedFinalGravity_sg      , namedParameterBundle, PropertyNames::BrewLog::expectedFinalGravity_sg      ),
   SET_REGULAR_FROM_NPB (m_expectedEfficiency_pct       , namedParameterBundle, PropertyNames::BrewLog::expectedEfficiency_pct       ),
   SET_REGULAR_FROM_NPB (m_expectedAlcoholByVolume_pct  , namedParameterBundle, PropertyNames::BrewLog::expectedAlcoholByVolume_pct  ),
   SET_REGULAR_FROM_NPB (m_expectedAttenuation_pct      , namedParameterBundle, PropertyNames::BrewLog::expectedAttenuation_pct      ) {

   CONSTRUCTOR_END
   return;
}

BrewLog::BrewLog(BrewLog const & other) :
   OwnedByRecipe                  {other                                },
   m_brewDate                     {other.m_brewDate                     },
   m_fermentDate                  {other.m_fermentDate                  },
   m_notes                        {other.m_notes                        },
   m_measuredPreBoilGravity_sg    {other.m_measuredPreBoilGravity_sg    },
   m_computedAlcoholByVolume_pct  {other.m_computedAlcoholByVolume_pct  },
   m_computedPreBoilEfficiency_pct{other.m_computedPreBoilEfficiency_pct},
   m_computedEfficiency_pct       {other.m_computedEfficiency_pct       },
   m_measuredPreBoilVolume_l      {other.m_measuredPreBoilVolume_l      },
   m_measuredStrikeTemp_c         {other.m_measuredStrikeTemp_c         },
   m_measuredMashFinalTemp_c      {other.m_measuredMashFinalTemp_c      },
   m_measuredOriginalGravity_sg   {other.m_measuredOriginalGravity_sg   },
   m_measuredPostBoilVolume_l     {other.m_measuredPostBoilVolume_l     },
   m_measuredVolumeIntoFermentor_l{other.m_measuredVolumeIntoFermentor_l},
   m_measuredPitchTemp_c          {other.m_measuredPitchTemp_c          },
   m_measuredFinalGravity_sg      {other.m_measuredFinalGravity_sg      },
   m_computedAttenuation_pct      {other.m_computedAttenuation_pct      },
   m_measuredFinalVolume_l        {other.m_measuredFinalVolume_l        },
   m_expectedBoilOff_l            {other.m_expectedBoilOff_l            },
   m_expectedPreBoilGravity_sg    {other.m_expectedPreBoilGravity_sg    },
   m_expectedPreBoilVolume_l      {other.m_expectedPreBoilVolume_l      },
   m_expectedStrikeTemp_c         {other.m_expectedStrikeTemp_c         },
   m_expectedMashFinalTemp_c      {other.m_expectedMashFinalTemp_c      },
   m_expectedOriginalGravity_sg   {other.m_expectedOriginalGravity_sg   },
   m_forecastOriginalGravity_sg   {other.m_forecastOriginalGravity_sg   },
   m_expectedVolumeIntoFermentor_l{other.m_expectedVolumeIntoFermentor_l},
   m_expectedFinalGravity_sg      {other.m_expectedFinalGravity_sg      },
   m_expectedEfficiency_pct       {other.m_expectedEfficiency_pct       },
   m_expectedAlcoholByVolume_pct  {other.m_expectedAlcoholByVolume_pct  },
   m_expectedAttenuation_pct      {other.m_expectedAttenuation_pct      } {

   CONSTRUCTOR_END
   return;
}

BrewLog::~BrewLog() = default;

void BrewLog::populateNote(Recipe * recipe) {
   this->m_recipeId = recipe->key();

   // Since we have the recipe, lets set some defaults The order in which
   // these are done is very specific. Please do not modify them without some
   // serious testing.

   // Everything needs volumes of one type or another. But the individual
   // volumes are fairly independent of anything. Do them all first.
   double const boilSize_l = recipe->boil() ? recipe->boil()->preBoilSize_l().value_or(0.0) : 0.0;
   this->setExpectedPreBoilVolume_l      (boilSize_l);
   this->setMeasuredPreBoilVolume_l      (boilSize_l);
   this->setMeasuredPostBoilVolume_l     (recipe->postBoilVolume_l());
   this->setExpectedVolumeIntoFermentor_l(recipe->finalVolume_l());
   this->setMeasuredVolumeIntoFermentor_l(recipe->finalVolume_l());
   this->setMeasuredFinalVolume_l        (recipe->finalVolume_l());

   if (auto const equip = recipe->equipment()) {
      double const boilTime_mins = recipe->boil() ? recipe->boil()->boilTime_mins() : Boil::default_boilTime_mins;
      this->setExpectedBoilOff_l(
         equip->kettleEvaporationPerHour_l().value_or(Equipment::default_kettleEvaporationPerHour_l) * (boilTime_mins/60.0)
      );
   }

   // Out of the gate, we expect projected to be the measured.
   this->setMeasuredPreBoilGravity_sg( recipe->boilGrav() );
   this->setExpectedPreBoilGravity_sg(recipe->boilGrav() );

   if (recipe->mash()) {
      if (auto const steps = recipe->mash()->mashSteps();
          !steps.isEmpty()) {
         auto mStep = steps.at(0);

         //
         // TODO: Need a slightly more sophisticated way to obtain final temperature prior to mash out (if any).
         //       Probably should be delegated to Mash.
         //
         if (mStep) {
            double const strikeTemp = mStep->infuseTemp_c().value_or(mStep->startTemp_c());
            this->setMeasuredStrikeTemp_c(strikeTemp);
            this->setExpectedStrikeTemp_c(strikeTemp);

            double const endTemp = mStep->endTemp_c().value_or(mStep->startTemp_c());
            this->setMeasuredMashFinalTemp_c(endTemp);
            this->setExpectedMashFinalTemp_c(endTemp);
         }

         if (steps.size() > 2) {
            // NOTE: Qt will complain that steps.size()-2 is always positive,
            // and therefore the internal assert that the index is positive is
            // bunk. This is OK, as we just checked that we will not underflow.
            mStep = steps.at(steps.size() - 2);
            double const endTemp = mStep->endTemp_c().value_or(mStep->startTemp_c());
            this->setMeasuredMashFinalTemp_c(endTemp);
            this->setExpectedMashFinalTemp_c(endTemp);
         }
      }
   }

   this->setMeasuredOriginalGravity_sg(recipe->og());
   this->setExpectedOriginalGravity_sg(recipe->og());
   this->setForecastOriginalGravity_sg(recipe->og());

   auto const fermentation = recipe->fermentation();
   if (fermentation &&
       fermentation->primary() &&
       fermentation->primary()->startTemp_c()) {
      this->setMeasuredPitchTemp_c(*fermentation->primary()->startTemp_c()); // Replaces parent->primaryTemp_c()
   }

   this->setMeasuredFinalGravity_sg( recipe->fg());
   this->setExpectedFinalGravity_sg( recipe->fg() );

   this->setExpectedEfficiency_pct(recipe->efficiency_pct());
   this->setComputedEfficiency_pct(recipe->efficiency_pct());
   this->setExpectedAlcoholByVolume_pct( recipe->ABV_pct());

   double atten_pct = -1.0;
   auto const yeastAdditions = recipe->yeastAdditions();
   for (auto const & yeastAddition : yeastAdditions) {
      if (yeastAddition->attenuation_pct() > atten_pct ) {
         atten_pct = yeastAddition->yeast()->attenuationTypical_pct();
      }
   }

   if (atten_pct < 0.0) {
      atten_pct = Yeast::DefaultAttenuation_pct; // Use an average attenuation;
   }
   this->setExpectedAttenuation_pct(atten_pct);
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
double  BrewLog::measuredPreBoilGravity_sg    () const { return this->m_measuredPreBoilGravity_sg    ; }
double  BrewLog::computedAlcoholByVolume_pct  () const { return this->m_computedAlcoholByVolume_pct  ; }
double  BrewLog::computedAttenuation_pct      () const { return this->m_computedAttenuation_pct      ; }
double  BrewLog::measuredPreBoilVolume_l      () const { return this->m_measuredPreBoilVolume_l      ; }
double  BrewLog::computedPreBoilEfficiency_pct() const { return this->m_computedPreBoilEfficiency_pct; }
double  BrewLog::computedEfficiency_pct       () const { return this->m_computedEfficiency_pct       ; }
double  BrewLog::measuredStrikeTemp_c         () const { return this->m_measuredStrikeTemp_c         ; }
double  BrewLog::measuredMashFinalTemp_c      () const { return this->m_measuredMashFinalTemp_c      ; }
double  BrewLog::measuredOriginalGravity_sg   () const { return this->m_measuredOriginalGravity_sg   ; }
double  BrewLog::measuredVolumeIntoFermentor_l() const { return this->m_measuredVolumeIntoFermentor_l; }
double  BrewLog::measuredPostBoilVolume_l     () const { return this->m_measuredPostBoilVolume_l     ; }
double  BrewLog::measuredPitchTemp_c          () const { return this->m_measuredPitchTemp_c          ; }
double  BrewLog::measuredFinalGravity_sg      () const { return this->m_measuredFinalGravity_sg      ; }
double  BrewLog::measuredFinalVolume_l        () const { return this->m_measuredFinalVolume_l        ; }
double  BrewLog::expectedPreBoilGravity_sg    () const { return this->m_expectedPreBoilGravity_sg    ; }
double  BrewLog::expectedPreBoilVolume_l      () const { return this->m_expectedPreBoilVolume_l      ; }
double  BrewLog::expectedStrikeTemp_c         () const { return this->m_expectedStrikeTemp_c         ; }
double  BrewLog::expectedMashFinalTemp_c      () const { return this->m_expectedMashFinalTemp_c      ; }
double  BrewLog::expectedOriginalGravity_sg   () const { return this->m_expectedOriginalGravity_sg   ; }
double  BrewLog::forecastOriginalGravity_sg   () const { return this->m_forecastOriginalGravity_sg   ; }
double  BrewLog::expectedVolumeIntoFermentor_l() const { return this->m_expectedVolumeIntoFermentor_l; }
double  BrewLog::expectedFinalGravity_sg      () const { return this->m_expectedFinalGravity_sg      ; }
double  BrewLog::expectedEfficiency_pct       () const { return this->m_expectedEfficiency_pct       ; }
double  BrewLog::expectedAlcoholByVolume_pct  () const { return this->m_expectedAlcoholByVolume_pct  ; }
double  BrewLog::expectedAttenuation_pct      () const { return this->m_expectedAttenuation_pct      ; }
double  BrewLog::expectedBoilOff_l            () const { return this->m_expectedBoilOff_l            ; }

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
void BrewLog::setMeasuredPreBoilGravity_sg(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::measuredPreBoilGravity_sg, this->m_measuredPreBoilGravity_sg, var);

   // write the value to the DB if requested
   if (!this->loading) {
      this->calculateEffIntoBK_pct();
      this->calculateOg();
   }
   return;
}

void BrewLog::setMeasuredPreBoilVolume_l(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::measuredPreBoilVolume_l, this->m_measuredPreBoilVolume_l, var);

   if (!this->loading) {
      this->calculateEffIntoBK_pct();
      this->calculateOg();
      this->calculateBrewHouseEff_pct();
   }
   return;
}

void BrewLog::setMeasuredOriginalGravity_sg(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::measuredOriginalGravity_sg, this->m_measuredOriginalGravity_sg, var);

   if (!this->loading) {
      this->calculateBrewHouseEff_pct();
      this->calculateABV_pct();
      this->calculateActualABV_pct();
      this->calculateAttenuation_pct();
   }
   return;
}

void BrewLog::setMeasuredVolumeIntoFermentor_l(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::measuredVolumeIntoFermentor_l, this->m_measuredVolumeIntoFermentor_l, var);

   if (!this->loading) {
      this->calculateBrewHouseEff_pct();
   }
   return;
}

void BrewLog::setMeasuredFinalGravity_sg(double var) {
   SET_AND_NOTIFY(PropertyNames::BrewLog::measuredFinalGravity_sg, this->m_measuredFinalGravity_sg, var);

   if (!this->loading) {
      this->calculateActualABV_pct();
      this->calculateAttenuation_pct();
   }
   return;
}

void BrewLog::setComputedAlcoholByVolume_pct  (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::computedAlcoholByVolume_pct  , this->m_computedAlcoholByVolume_pct  , var); }
void BrewLog::setComputedAttenuation_pct      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::computedAttenuation_pct      , this->m_computedAttenuation_pct      , var); }
void BrewLog::setComputedPreBoilEfficiency_pct(double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::computedPreBoilEfficiency_pct, this->m_computedPreBoilEfficiency_pct, var); }
void BrewLog::setComputedEfficiency_pct       (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::computedEfficiency_pct       , this->m_computedEfficiency_pct       , var); }
void BrewLog::setMeasuredStrikeTemp_c         (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::measuredStrikeTemp_c         , this->m_measuredStrikeTemp_c         , var); }
void BrewLog::setMeasuredMashFinalTemp_c      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::measuredMashFinalTemp_c      , this->m_measuredMashFinalTemp_c      , var); }
void BrewLog::setMeasuredPostBoilVolume_l     (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::measuredPostBoilVolume_l     , this->m_measuredPostBoilVolume_l     , var); }
void BrewLog::setMeasuredPitchTemp_c          (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::measuredPitchTemp_c          , this->m_measuredPitchTemp_c          , var); }
void BrewLog::setMeasuredFinalVolume_l        (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::measuredFinalVolume_l        , this->m_measuredFinalVolume_l        , var); }
void BrewLog::setExpectedPreBoilGravity_sg    (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedPreBoilGravity_sg    , this->m_expectedPreBoilGravity_sg    , var); }
void BrewLog::setExpectedPreBoilVolume_l      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedPreBoilVolume_l      , this->m_expectedPreBoilVolume_l      , var); }
void BrewLog::setExpectedStrikeTemp_c         (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedStrikeTemp_c         , this->m_expectedStrikeTemp_c         , var); }
void BrewLog::setExpectedMashFinalTemp_c      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedMashFinalTemp_c      , this->m_expectedMashFinalTemp_c      , var); }
void BrewLog::setExpectedOriginalGravity_sg   (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedOriginalGravity_sg   , this->m_expectedOriginalGravity_sg   , var); }
void BrewLog::setForecastOriginalGravity_sg   (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::forecastOriginalGravity_sg   , this->m_forecastOriginalGravity_sg   , var); }
void BrewLog::setExpectedVolumeIntoFermentor_l(double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedVolumeIntoFermentor_l, this->m_expectedVolumeIntoFermentor_l, var); }
void BrewLog::setExpectedFinalGravity_sg      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedFinalGravity_sg      , this->m_expectedFinalGravity_sg      , var); }
void BrewLog::setExpectedEfficiency_pct       (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedEfficiency_pct       , this->m_expectedEfficiency_pct       , var); }
void BrewLog::setExpectedAlcoholByVolume_pct  (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedAlcoholByVolume_pct  , this->m_expectedAlcoholByVolume_pct  , var); }
void BrewLog::setExpectedAttenuation_pct      (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedAttenuation_pct      , this->m_expectedAttenuation_pct      , var); }
void BrewLog::setExpectedBoilOff_l            (double const var) { SET_AND_NOTIFY(PropertyNames::BrewLog::expectedBoilOff_l            , this->m_expectedBoilOff_l            , var); }

// calculators -- these kind of act as both setters and getters.  Likely bad
// form
double BrewLog::calculateEffIntoBK_pct() {
   double const expectedPreBoilGravityPoints = Measurement::Units::gravityPoints.fromCanonical(m_expectedPreBoilGravity_sg);
   double const maxPoints = expectedPreBoilGravityPoints * m_expectedPreBoilVolume_l;
   qDebug() <<
      Q_FUNC_INFO << "expectedPreBoilGravityPoints: " << expectedPreBoilGravityPoints << ", m_expectedPreBoilVolume_l:" << m_expectedPreBoilVolume_l <<
      ", maxPoints:" << maxPoints;

   double const measuredPoints = Measurement::Units::gravityPoints.fromCanonical(m_measuredPreBoilGravity_sg);
   double const actualPoints = measuredPoints * m_measuredPreBoilVolume_l;
   qDebug() <<
      Q_FUNC_INFO << "m_sg:" << m_measuredPreBoilGravity_sg << ", m_measuredPreBoilVolume_l:" << m_measuredPreBoilVolume_l <<
      ", actualPoints:" << actualPoints;
   // this can happen under normal circumstances (eg, load)
   if (maxPoints <= 0.0) {
      return 0.0;
   }

   double const efficiencyIntoBoilKettle = actualPoints/maxPoints * 100;
   qDebug() << Q_FUNC_INFO << "efficiencyIntoBoilKettle:" << efficiencyIntoBoilKettle;
   this->setComputedPreBoilEfficiency_pct(efficiencyIntoBoilKettle);

   return efficiencyIntoBoilKettle;
}

// The idea is that based on the preboil gravity, estimate what the actual OG will be.
double BrewLog::calculateOg() {
   double const measuredPreBoilPoints = Measurement::Units::gravityPoints.fromCanonical(this->m_measuredPreBoilGravity_sg);
   double const expectedVolume_l = m_expectedPreBoilVolume_l - m_expectedBoilOff_l;
   if (expectedVolume_l <= 0.0) {
      return 0.0;
   }
   double const calculatedPoints = measuredPreBoilPoints * m_measuredPreBoilVolume_l / expectedVolume_l;
   double const calculatedOriginalGravity_sg = Measurement::Units::gravityPoints.toCanonical(calculatedPoints).quantity;
   this->setForecastOriginalGravity_sg(calculatedOriginalGravity_sg);
   return calculatedOriginalGravity_sg;
}

double BrewLog::calculateBrewHouseEff_pct() {

   Recipe const & recipe = *this->recipe();

   Recipe::Sugars const sugars = recipe.sugarTotals();

   // The maximum possible OG is if all the starches are converted to sugar by the mash
   double const allPossibleSugar_kg = sugars.existingSugars_all_kg + sugars.sugarsFromStarch_all_kg;
   // Since °Plato is the mass fraction of sucrose in the wort, the sums here are relatively easy.  We assume 1 liter of
   // wort weighs 1 kilogram.  So, 1 degree Plato represents 10g of sugar in 1 liter of wort.
   double const maxGravity_plato = 100.0 * allPossibleSugar_kg / this->m_expectedVolumeIntoFermentor_l;
   double const measuredGravity_plato = 100.0 * Measurement::Units::plato.fromCanonical(this->m_measuredOriginalGravity_sg);


   //
   // TBD: Not sure this is exactly the right calculation here...
   //
   // The standard formula is:
   //
   // Brewhouse efficiency (%) = [(OG – 1) × 1000 × volume into fermenter (litres)] ÷ [grain weight (kg) × maximum extract points per kg] × 100
   //
   double const brewhouseEfficiency = measuredGravity_plato / maxGravity_plato;



   this->setComputedEfficiency_pct(brewhouseEfficiency);
   return brewhouseEfficiency;
}

// Need to do some work here to figure out what the expected FG will be based
// on the actual OG, not the calculated.
double BrewLog::calculateABV_pct() {
   double const attenuation_pct = m_expectedAttenuation_pct;

   // This looks weird, but the math works. (Yes, I am showing my work)
   // 1 + [(og-1) * 1000 * (1.0 - %/100)] / 1000  =
   // 1 + [(og - 1) * (1.0 - %/100)]
   double const estFg = 1 + ((m_measuredOriginalGravity_sg-1.0)*(1.0 - attenuation_pct/100.0));

   double const calculatedAlcoholByVolume_pct = Algorithms::abvFromOgAndFg(this->m_measuredOriginalGravity_sg, estFg);

   this->setExpectedAlcoholByVolume_pct(calculatedAlcoholByVolume_pct);

   return calculatedAlcoholByVolume_pct;
}

double BrewLog::calculateActualABV_pct() {
   double const abv_pct = this->m_measuredOriginalGravity_sg > this->m_measuredFinalGravity_sg ?
      Algorithms::abvFromOgAndFg(this->m_measuredOriginalGravity_sg, this->m_measuredFinalGravity_sg) : std::numeric_limits<double>::quiet_NaN();
   this->setComputedAlcoholByVolume_pct(abv_pct);
   return abv_pct;
}

double BrewLog::calculateAttenuation_pct() {
    // Calculate measured attenuation based on user-reported values for
    // post-boil OG and post-ferment FG
    double const attenuation = ((m_measuredOriginalGravity_sg - m_measuredFinalGravity_sg) / (m_measuredOriginalGravity_sg - 1)) * 100;
    this->setComputedAttenuation_pct(attenuation);
    return attenuation;
}

QList<std::shared_ptr<BrewLog>> BrewLog::ownedBy(Recipe const & recipe) {
   return recipe.brewLogs();
}