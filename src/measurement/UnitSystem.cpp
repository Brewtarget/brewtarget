/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/UnitSystem.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "measurement/UnitSystem.h"

#include <QApplication>
#include <QDebug>

#include "Localization.h"
#include "measurement/Unit.h"
#include "utils/EnumStringMapping.h"

namespace {
   /**
    * \brief This is just a small function to create a QMap from an ordered list.  TODO We should probably get rid of
    *        the whole map and just replace it with a vector, but that needs a bit more refactoring.
    */
   QMap<Measurement::UnitSystem::RelativeScale, Measurement::Unit const *> makeScaleToUnit(std::initializer_list<Measurement::Unit const *> unitEntriesSmallestToLargest) {
      QMap<Measurement::UnitSystem::RelativeScale, Measurement::Unit const *> scaleToUnit;
      uint8_t index = 0;
      for (Measurement::Unit const * unitEntry : unitEntriesSmallestToLargest) {
         scaleToUnit.insert(static_cast<Measurement::UnitSystem::RelativeScale>(index++), unitEntry);
      }
      return scaleToUnit;
   }

   int const fieldWidth = 0;
   char const format = 'f';
   int const defaultPrecision = 3;

   QMultiMap<Measurement::PhysicalQuantity, Measurement::UnitSystem const *> physicalQuantityToUnitSystems;

   // Used by UnitSystem::getInstanceByName()
   QMap<QString, Measurement::UnitSystem const *> nameToUnitSystem;

   // .:TBD:. See if we can eliminate all this and get compile-time checking benefits
   //
   // We sometimes want to be able to access RelativeScale enum values via a string name (eg in persistent settings
   // files) so it's useful to be able to map between them.  There is some functionality built in to Qt to do this via
   // QMetaEnum, but this is at the cost of inheriting from QObject, which seems overkill here.  Alternatively, we could
   // use Magic Enum C++ -- see https://github.com/Neargye/magic_enum -- at the cost of having to convert between
   // std::string and QString.  For the moment, this more manual approach seems appropriate to the scale of what we
   // need.
   EnumStringMapping const relativeScaleToName {
      {Measurement::UnitSystem::RelativeScale::ExtraSmall, "scaleExtraSmall"},
      {Measurement::UnitSystem::RelativeScale::Small     , "scaleSmall"     },
      {Measurement::UnitSystem::RelativeScale::Medium    , "scaleMedium"    },
      {Measurement::UnitSystem::RelativeScale::Large     , "scaleLarge"     },
      {Measurement::UnitSystem::RelativeScale::ExtraLarge, "scaleExtraLarge"},
      {Measurement::UnitSystem::RelativeScale::Huge      , "scaleHuge"      },

   };
}

// This private implementation class holds all private non-virtual members of UnitSystem
class Measurement::UnitSystem::impl {
public:
   /**
    * Constructor
    */
   impl(Measurement::UnitSystem & self,
        Measurement::PhysicalQuantity const physicalQuantity,
        Measurement::Unit const * const thickness,
        Measurement::Unit const * const defaultUnit,
        std::initializer_list<Measurement::Unit const *> unitEntriesSmallestToLargest) :
      self            {self},
      physicalQuantity{physicalQuantity},
      thickness       {thickness},
      defaultUnit     {defaultUnit},
      scaleToUnit     {makeScaleToUnit(unitEntriesSmallestToLargest)} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * \brief Maps from a Measurement::UnitSystem::RelativeScale to a concrete Unit - eg in the US weight UnitSystem,
    *        Measurement::Unit::scaleExtraSmall maps to Measurement::Units::ounces and Measurement::Unit::scaleSmall
    *        maps to Measurement::Units::pounds
    */
   Unit const * getUnitForRelativeScale(Measurement::UnitSystem::RelativeScale const relativeScale) const {
      return this->scaleToUnit.value(relativeScale, nullptr);
   }

   /**
    * \brief This does most of the work for displayAmount() and amountDisplay()
    *
    * \param amount the amount to display
    * \param forcedScale  if specified, the \c RelativeScale from this \c UnitSystem to use for displaying
    *
    * \return
    */
   std::pair<double, QString> displayableAmount(Measurement::Amount const & amount,
                                                std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) const {
      // Special cases
      if (amount.unit->getPhysicalQuantity() != this->physicalQuantity) {
         return std::pair(amount.quantity, "");
      }

      auto siAmount = amount.unit->toCanonical(amount.quantity);

      // If there is only one unit in this unit system, then the scale to unit mapping will be empty as there's nothing
      // to choose from
      if (this->scaleToUnit.size() == 0) {
         return std::pair(this->defaultUnit->fromCanonical(siAmount.quantity), this->defaultUnit->name);
      }

      // Conversely, if we have a non-empty mapping then it's a coding error if it only has one entry!
      Q_ASSERT(this->scaleToUnit.size() >  1);

      // Short circuit if a specific scale is requested
      if (forcedScale) {
         // It's a coding error to specify a forced scale that is not in the UnitSystem
         Q_ASSERT(this->scaleToUnit.contains(*forcedScale));
         Measurement::Unit const * bb = this->scaleToUnit.value(*forcedScale);
         return std::pair(bb->fromCanonical(siAmount.quantity), bb->name);
      }

      // Search for the smallest measure in this system that's not too big to show the supplied value
      // QMap guarantees that we iterate in the order of its keys, thus here we'll loop from smallest to largest scale
      // (e.g., mg, g, kg).
      Measurement::Unit const * last  = nullptr;
      for (auto it : this->scaleToUnit) {
         if (last != nullptr && qAbs(siAmount.quantity) < it->toCanonical(it->boundary()).quantity) {
            // Stop looping as we've found a unit that's too big to use (so we'll return the last one, ie the one smaller,
            // below)
            break;
         }
         last = it;
      }

      // It is a programming error if the map was empty (ie we didn't go through the loop at all)
      Q_ASSERT(last != nullptr);
      return std::pair(last->fromCanonical(siAmount.quantity), last->name);
   }

   // Member variables for impl
   Measurement::UnitSystem & self;
   Measurement::PhysicalQuantity const physicalQuantity;
   Measurement::Unit const * const thickness;
   Measurement::Unit const * const defaultUnit;

   QMap<Measurement::UnitSystem::RelativeScale, Measurement::Unit const *> const scaleToUnit;
};

Measurement::UnitSystem::UnitSystem(Measurement::PhysicalQuantity const physicalQuantity,
                                    Measurement::Unit const * const defaultUnit,
                                    char const * const uniqueName,
                                    SystemOfMeasurement const systemOfMeasurement,
                                    std::initializer_list<Measurement::Unit const *> unitEntriesSmallestToLargest,
                                    Measurement::Unit const * const thickness) :
   uniqueName{uniqueName},
   systemOfMeasurement{systemOfMeasurement},
   pimpl{std::make_unique<impl>(*this,
                                physicalQuantity,
                                thickness,
                                defaultUnit,
                                unitEntriesSmallestToLargest)} {
   // We assert that no other UnitSystem has the same name as this one
   Q_ASSERT(!nameToUnitSystem.contains(uniqueName));
   nameToUnitSystem.insert(uniqueName, this);
   // Conversely, it is more often than not the case that there will be more than one UnitSystem per PhysicalQuantity
   physicalQuantityToUnitSystems.insert(physicalQuantity, this);
   return;
}

Measurement::UnitSystem::~UnitSystem() = default;

bool Measurement::UnitSystem::operator==(UnitSystem const & other) const {
   // Since we're not intending to create multiple instances of any given UnitSystem, it should be enough to check
   // the addresses are equal, but, as belt-and-braces, we'll check the names are equal as a fall-back.
   return (this == &other || this->uniqueName == other.uniqueName);
}

Measurement::Amount Measurement::UnitSystem::qstringToSI(QString qstr, Unit const & defUnit) const {
   // Structured binding declarations are a pretty neat feature from C++17 that make it easier to have a function return
   // more than one thing.
   auto const [amt, unitName] = Measurement::Unit::splitAmountString(qstr);

   // Look first in this unit system. If you can't find it here, find it
   // globally. I *think* this finally has all the weird magic right. If the
   // field is marked as "Imperial" and you enter "3 qt" you get 3 imperial
   // qts, 3.6 US qts, 3.41L. If you enter 3L, you get 2.64 imperial qts,
   // 3.17 US qt. If you mean 3 US qt, you are SOL unless you mark the field
   // as US Customary.

   Unit const * unitToUse = nullptr;
   if (!unitName.isEmpty()) {
      // Unit::getUnit() will, by preference, match to a unit in the current UnitSystem if possible.  If not, it will
      // match to a unit in another UnitSystem for the same PhysicalQuantity.  If there are no matches that way, it will
      // return nullptr;
      unitToUse = Unit::getUnit(unitName, *this, true);
//      if (unitToUse) {
//         qDebug() << Q_FUNC_INFO << this->uniqueName << ":" << unitName << "interpreted as" << unitToUse->name;
//      } else {
//         qDebug() <<
//            Q_FUNC_INFO << this->uniqueName << ":" << unitName << "not recognised for" << this->pimpl->physicalQuantity;
//      }
   }

   if (!unitToUse) {
//      qDebug() << Q_FUNC_INFO << "Defaulting to" << defUnit;
      unitToUse = &defUnit;
   }

   Measurement::Amount siAmount = unitToUse->toCanonical(amt);
//   qDebug() <<
//      Q_FUNC_INFO << this->uniqueName << ": " << qstr << "is" << amt << " " << unitToUse->name << "=" <<
//      siAmount.quantity << "in" << siAmount.unit->name;

   return siAmount;
}

QString Measurement::UnitSystem::displayAmount(Measurement::Amount const & amount,
                                               int precision,
                                               std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) const {
   // If the precision is not specified, we take the default one
   if (precision < 0) {
      precision = defaultPrecision;
   }

   auto result = this->pimpl->displayableAmount(amount, forcedScale);

   if (result.second.isEmpty()) {
      return QString("%L1").arg(this->amountDisplay(Measurement::Amount{result.first, *amount.unit}, forcedScale),
                                fieldWidth,
                                format,
                                precision);
   }

   return QString("%L1 %2").arg(result.first, fieldWidth, format, precision).arg(result.second);
}

double Measurement::UnitSystem::amountDisplay(Measurement::Amount const & amount,
                                              std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) const {
   // Essentially we're just returning the numeric part of the displayable amount
   return this->pimpl->displayableAmount(amount, forcedScale).first;
}

QList<Measurement::UnitSystem::RelativeScale> Measurement::UnitSystem::getRelativeScales() const {
   return this->pimpl->scaleToUnit.keys();;
}

Measurement::Unit const * Measurement::UnitSystem::scaleUnit(Measurement::UnitSystem::RelativeScale scale) const {
   return this->pimpl->getUnitForRelativeScale(scale);
}

Measurement::Unit const * Measurement::UnitSystem::thicknessUnit() const {
   return this->pimpl->thickness;
}

Measurement::Unit const * Measurement::UnitSystem::unit() const {
   return this->pimpl->defaultUnit;
}

Measurement::PhysicalQuantity Measurement::UnitSystem::getPhysicalQuantity() const {
   return this->pimpl->physicalQuantity;
}

Measurement::UnitSystem const * Measurement::UnitSystem::getInstanceByUniqueName(QString const & uniqueName) {
   return nameToUnitSystem.value(uniqueName, nullptr);
}

Measurement::UnitSystem const & Measurement::UnitSystem::getInstance(SystemOfMeasurement const systemOfMeasurement,
                                                                     PhysicalQuantity const physicalQuantity) {
   auto systemsForThisPhysicalQuantity = Measurement::UnitSystem::getUnitSystems(physicalQuantity);

   // Per the comment in the header, if there is _only_ one UnitSystem for the given PhysicalQuantity then we return
   // that, without trying to match SystemOfMeasurement.
   if (systemsForThisPhysicalQuantity.size() == 1) {
      return **systemsForThisPhysicalQuantity.begin();
   }

   auto result = std::find_if(
      systemsForThisPhysicalQuantity.begin(),
      systemsForThisPhysicalQuantity.end(),
      [systemOfMeasurement](Measurement::UnitSystem const * unitSystem) {
         return unitSystem->systemOfMeasurement == systemOfMeasurement;
      }
   );

   if (systemsForThisPhysicalQuantity.end() == result) {
      // At this point, it's a coding error if we didn't find a match
      qCritical() <<
         Q_FUNC_INFO << "Unable to find a UnitSystem for SystemOfMeasurement" <<
         Measurement::getDisplayName(systemOfMeasurement) << "and PhysicalQuantity" <<
         Measurement::physicalQuantityStringMapping[physicalQuantity] << "(Searched" <<
         systemsForThisPhysicalQuantity.size() << "option(s).)";
      qCritical().noquote() << Q_FUNC_INFO << "Stacktrace:" << Logging::getStackTrace();
      Q_ASSERT(false); // Stop here on a debug build
   }

   return **result;
}

QList<Measurement::UnitSystem const *> Measurement::UnitSystem::getUnitSystems(Measurement::PhysicalQuantity const physicalQuantity) {
   return physicalQuantityToUnitSystems.values(physicalQuantity);
}

QString Measurement::UnitSystem::getUniqueName(Measurement::UnitSystem::RelativeScale relativeScale) {
   // It's a coding error if we define a name for a RelativeScale (in which case EnumStringMapping::enumToString will
   // log an error and throw an exception).
   return relativeScaleToName.enumToString(relativeScale);
}

std::optional<Measurement::UnitSystem::RelativeScale> Measurement::UnitSystem::getScaleFromUniqueName(QString relativeScaleAsString) {
   return relativeScaleToName.stringToEnumOrNull<Measurement::UnitSystem::RelativeScale>(relativeScaleAsString);
}

template<class S>
S & operator<<(S & stream, Measurement::UnitSystem const & unitSystem) {
   stream << unitSystem.uniqueName;
   return stream;
}

template<class S>
S & operator<<(S & stream, Measurement::UnitSystem const * unitSystem) {
   if (unitSystem) {
      stream << *unitSystem;
   } else {
      stream << "NULL";
   }
   return stream;
}

template<class S>
S & operator<<(S & stream, Measurement::UnitSystem::RelativeScale const relativeScale) {
   stream << Measurement::UnitSystem::getUniqueName(relativeScale);
   return stream;
}

//
// Instantiate the above template functions for the types that are going to use them
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template QDebug &      operator<<(QDebug &      stream, Measurement::UnitSystem const & unitSystem);
template QTextStream & operator<<(QTextStream & stream, Measurement::UnitSystem const & unitSystem);
template QDebug &      operator<<(QDebug &      stream, Measurement::UnitSystem const * unitSystem);
template QTextStream & operator<<(QTextStream & stream, Measurement::UnitSystem const * unitSystem);
template QDebug &      operator<<(QDebug &      stream, Measurement::UnitSystem::RelativeScale const relativeScale);
template QTextStream & operator<<(QTextStream & stream, Measurement::UnitSystem::RelativeScale const relativeScale);


//---------------------------------------------------------------------------------------------------------------------
//
// This is where we actually define all the different unit systems
//
//---------------------------------------------------------------------------------------------------------------------
namespace Measurement::UnitSystems {
   //
   // NB: For the mass_Xxxx and volume_Xxxx unit systems, to make PhysicalQuantity::ChoiceOfPhysicalQuantity work, we
   //     rely on them sharing systemOfMeasurement (Imperial, UsCustomary or Metric).  However, this trick can't work
   //     with PhysicalQuantity::Count, so there is special handling for that.
   //
   UnitSystem const mass_Metric{PhysicalQuantity::Mass,
                                &Measurement::Units::kilograms,
                                "mass_Metric",
                                Measurement::SystemOfMeasurement::Metric,
                                {&Measurement::Units::milligrams,
                                 &Measurement::Units::grams     ,
                                 &Measurement::Units::kilograms },
                                &Measurement::Units::kilograms};

   UnitSystem const mass_UsCustomary{PhysicalQuantity::Mass,
                                     &Measurement::Units::pounds,
                                     "mass_UsCustomary",
                                     Measurement::SystemOfMeasurement::UsCustomary,
                                     {&Measurement::Units::ounces,
                                      &Measurement::Units::pounds},
                                     &Measurement::Units::pounds};

   UnitSystem const mass_Imperial{PhysicalQuantity::Mass,
                                  &Measurement::Units::imperial_pounds,
                                  "mass_Imperial",
                                  Measurement::SystemOfMeasurement::Imperial,
                                  {&Measurement::Units::imperial_ounces,
                                   &Measurement::Units::imperial_pounds},
                                  &Measurement::Units::imperial_pounds};

   UnitSystem const volume_Metric{PhysicalQuantity::Volume,
                                  &Measurement::Units::liters,
                                  "volume_Metric",
                                  Measurement::SystemOfMeasurement::Metric,
                                  {&Measurement::Units::milliliters,
                                   &Measurement::Units::liters     },
                                  &Measurement::Units::liters};

   UnitSystem const volume_UsCustomary{PhysicalQuantity::Volume,
                                       &Measurement::Units::us_gallons,
                                       "volume_UsCustomary",
                                       Measurement::SystemOfMeasurement::UsCustomary,
                                       {&Measurement::Units::us_teaspoons  ,
                                        &Measurement::Units::us_tablespoons,
                                        &Measurement::Units::us_cups       ,
                                        &Measurement::Units::us_quarts     ,
                                        &Measurement::Units::us_gallons    ,
                                        &Measurement::Units::us_barrels    },
                                       &Measurement::Units::us_quarts};

   UnitSystem const volume_Imperial{PhysicalQuantity::Volume,
                                    &Measurement::Units::imperial_gallons,
                                    "volume_Imperial",
                                    Measurement::SystemOfMeasurement::Imperial,
                                    {&Measurement::Units::imperial_teaspoons  ,
                                     &Measurement::Units::imperial_tablespoons,
                                     &Measurement::Units::imperial_cups       ,
                                     &Measurement::Units::imperial_quarts     ,
                                     &Measurement::Units::imperial_gallons    ,
                                     &Measurement::Units::imperial_barrels    },
                                    &Measurement::Units::imperial_quarts};

   UnitSystem const length_Metric{PhysicalQuantity::Length,
                                  &Measurement::Units::centimeters,
                                  "length_Metric"     ,
                                  Measurement::SystemOfMeasurement::Metric,
                                  {&Measurement::Units::millimeters,
                                   &Measurement::Units::centimeters,
                                   &Measurement::Units::meters     }};

   UnitSystem const length_UsCustomary{PhysicalQuantity::Length,
                                       &Measurement::Units::inches     ,
                                       "length_UsCustomary",
                                       Measurement::SystemOfMeasurement::UsCustomary,
                                       {&Measurement::Units::inches,
                                        &Measurement::Units::feet}};

   UnitSystem const count_NumberOf{PhysicalQuantity::Count,
                                   &Measurement::Units::numberOf,
                                   "count_NumberOf",
                                   Measurement::SystemOfMeasurement::UniversalStandard};

   UnitSystem const temperature_MetricIsCelsius{PhysicalQuantity::Temperature,
                                                &Measurement::Units::celsius,
                                                "temperature_MetricIsCelsius",
                                                Measurement::SystemOfMeasurement::Metric};

   UnitSystem const temperature_UsCustomaryIsFahrenheit{PhysicalQuantity::Temperature,
                                                        &Measurement::Units::fahrenheit,
                                                        "temperature_UsCustomaryIsFahrenheit",
                                                        Measurement::SystemOfMeasurement::UsCustomary};

   UnitSystem const time_CoordinatedUniversalTime{PhysicalQuantity::Time,
                                                  &Measurement::Units::minutes,
                                                  "time_CoordinatedUniversalTime",
                                                  Measurement::SystemOfMeasurement::UniversalStandard,
                                                  {&Measurement::Units::seconds,
                                                   &Measurement::Units::minutes,
                                                   &Measurement::Units::hours  ,
                                                   &Measurement::Units::days   }};

   UnitSystem const color_EuropeanBreweryConvention{PhysicalQuantity::Color,
                                                    &Measurement::Units::ebc,
                                                    "color_EuropeanBreweryConvention",
                                                    Measurement::SystemOfMeasurement::EuropeanBreweryConvention};

   UnitSystem const color_StandardReferenceMethod{PhysicalQuantity::Color,
                                                  &Measurement::Units::srm,
                                                  "color_StandardReferenceMethod",
                                                  Measurement::SystemOfMeasurement::StandardReferenceMethod};
   UnitSystem const color_Lovibond{PhysicalQuantity::Color,
                                   &Measurement::Units::lovibond,
                                   "color_Lovibond",
                                   Measurement::SystemOfMeasurement::Lovibond};

   UnitSystem const density_SpecificGravity{PhysicalQuantity::Density,
                                            &Measurement::Units::specificGravity,
                                            "density_SpecificGravity",
                                            Measurement::SystemOfMeasurement::SpecificGravity};

   UnitSystem const density_Plato{PhysicalQuantity::Density,
                                  &Measurement::Units::plato,
                                  "density_Plato",
                                  Measurement::SystemOfMeasurement::Plato};

   UnitSystem const density_Brix{PhysicalQuantity::Density,
                                  &Measurement::Units::brix,
                                  "density_Brix",
                                  Measurement::SystemOfMeasurement::Brix};

   UnitSystem const diastaticPower_Lintner{PhysicalQuantity::DiastaticPower,
                                           &Measurement::Units::lintner,
                                           "diastaticPower_Lintner",
                                           Measurement::SystemOfMeasurement::Lintner};

   UnitSystem const diastaticPower_WindischKolbach{PhysicalQuantity::DiastaticPower,
                                                   &Measurement::Units::wk,
                                                   "diastaticPower_WindischKolbach",
                                                   Measurement::SystemOfMeasurement::WindischKolbach};

   UnitSystem const acidity_pH{PhysicalQuantity::Acidity,
                               &Measurement::Units::pH,
                               "acidity_pH"};

   UnitSystem const bitterness_InternationalBitternessUnits{PhysicalQuantity::Bitterness,
                                                            &Measurement::Units::ibu,
                                                            "bitterness_InternationalBitternessUnits"};


   UnitSystem const carbonation_Volumes{PhysicalQuantity::Carbonation,
                                        &Measurement::Units::carbonationVolumes,
                                        "carbonation_Volumes",
                                        Measurement::SystemOfMeasurement::CarbonationVolumes};

   UnitSystem const carbonation_MassPerVolume{PhysicalQuantity::Carbonation,
                                              &Measurement::Units::carbonationGramsPerLiter,
                                              "carbonation_MassPerVolume",
                                              Measurement::SystemOfMeasurement::CarbonationMassPerVolume};

   UnitSystem const massFractionOrConc_Brewing{PhysicalQuantity::MassFractionOrConc,
                                               &Measurement::Units::partsPerMillionMass,
                                               "massFractionOrConc_Brewing",
                                               Measurement::SystemOfMeasurement::BrewingConcentration,
                                               {&Measurement::Units::partsPerBillionMass,
                                                &Measurement::Units::partsPerMillionMass,
                                                &Measurement::Units::milligramsPerLiter }};

   UnitSystem const viscosity_Metric{PhysicalQuantity::Viscosity,
                                     &Measurement::Units::centipoise,
                                     "viscosity_Metric",
                                     Measurement::SystemOfMeasurement::Metric};

   UnitSystem const viscosity_MetricAlternate{PhysicalQuantity::Viscosity,
                                              &Measurement::Units::millipascalSecond,
                                              "viscosity_MetricAlternate",
                                              Measurement::SystemOfMeasurement::MetricAlternate};

   UnitSystem const specificHeatCapacity_Calories{PhysicalQuantity::SpecificHeatCapacity,
                                                  &Measurement::Units::caloriesPerCelsiusPerGram,
                                                  "specificHeatCapacity_Calories",
                                                  Measurement::SystemOfMeasurement::SpecificHeatCapacityCalories};

   UnitSystem const specificHeatCapacity_Joules{PhysicalQuantity::SpecificHeatCapacity,
                                                &Measurement::Units::joulesPerKelvinPerKg,
                                                "specificHeatCapacity_Joules",
                                                Measurement::SystemOfMeasurement::SpecificHeatCapacityJoules};

   UnitSystem const specificHeatCapacity_Btus{PhysicalQuantity::SpecificHeatCapacity,
                                              &Measurement::Units::btuPerFahrenheitPerPound,
                                              "specificHeatCapacity_Btus",
                                              Measurement::SystemOfMeasurement::SpecificHeatCapacityBtus};

   UnitSystem const specificVolume_Metric{PhysicalQuantity::SpecificVolume,
                                          &Measurement::Units::litresPerKilogram,
                                          "specificVolume_Metric",
                                          Measurement::SystemOfMeasurement::Metric,
                                          {&Measurement::Units::litresPerKilogram     ,
                                           &Measurement::Units::cubicMetersPerKilogram,
                                           &Measurement::Units::litresPerGram         }};

   UnitSystem const specificVolume_UsCustomary{PhysicalQuantity::SpecificVolume,
                                               &Measurement::Units::us_quartsPerPound,
                                               "specificVolume_UsCustomary",
                                               Measurement::SystemOfMeasurement::UsCustomary,
                                               {&Measurement::Units::us_fluidOuncesPerOunce,
                                                &Measurement::Units::cubicFeetPerPound     ,
                                                &Measurement::Units::us_gallonsPerPound    ,
                                                &Measurement::Units::us_quartsPerPound     ,
                                                &Measurement::Units::us_gallonsPerOunce    }};
}
