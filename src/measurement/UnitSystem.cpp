/*
 * measurement/UnitSystem.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2022:
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Théophane Martin <theophane.m@gmail.com>
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
#include "measurement/UnitSystem.h"

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QRegExp>

#include "Localization.h"
#include "measurement/Unit.h"
#include "utils/EnumStringMapping.h"

namespace {
   int const fieldWidth = 0;
   char const format = 'f';
   int const defaultPrecision = 3;

   // All functions in QRegExp are reentrant, so it should be safe to use as a shared const in multi-threaded code.
   QRegExp const amtUnit {
      // Make sure we get the right decimal point (. or ,) and the right grouping separator (, or .).  Some locales
      // write 1.000,10 and others write 1,000.10.  We need to catch both.
      "((?:\\d+" + QRegExp::escape(QLocale::system().groupSeparator()) + ")?\\d+(?:" +
      QRegExp::escape(QLocale::system().decimalPoint()) + "\\d+)?|" +
      QRegExp::escape(QLocale::system().decimalPoint()) + "\\d+)\\s*(\\w+)?",
      Qt::CaseInsensitive
   };

   QMultiMap<Measurement::PhysicalQuantity, Measurement::UnitSystem const *> physicalQuantityToUnitSystems;

   // Used by UnitSystem::getInstanceByName()
   QMap<QString, Measurement::UnitSystem const *> nameToUnitSystem;

   // We sometimes want to be able to access RelativeScale enum values via a string name (eg code generated from .ui
   // files) so it's useful to be able to map between them.  There is some functionality built in to Qt to do this via
   // QMetaEnum, but this is at the cost of inheriting from QObject, which seems overkill here.  Alternatively, we could
   // use Magic Enum C++ -- see https://github.com/Neargye/magic_enum -- at the cost of having to convert between
   // std::string and QString.  For the moment, this more manual approach seems appropriate to the scale of what we
   // need.
   EnumStringMapping const relativeScaleToName {
      {"scaleExtraSmall", Measurement::UnitSystem::RelativeScale::scaleExtraSmall},
      {"scaleSmall"     , Measurement::UnitSystem::RelativeScale::scaleSmall     },
      {"scaleMedium"    , Measurement::UnitSystem::RelativeScale::scaleMedium    },
      {"scaleLarge"     , Measurement::UnitSystem::RelativeScale::scaleLarge     },
      {"scaleExtraLarge", Measurement::UnitSystem::RelativeScale::scaleExtraLarge},
      {"scaleHuge"      , Measurement::UnitSystem::RelativeScale::scaleHuge      },
      {"scaleWithout"   , Measurement::UnitSystem::RelativeScale::scaleWithout   }
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
        std::initializer_list<std::pair<Measurement::UnitSystem::RelativeScale const,
                                        Measurement::Unit const *> > scaleToUnit) :
      self            {self},
      physicalQuantity{physicalQuantity},
      thickness       {thickness},
      defaultUnit     {defaultUnit},
      scaleToUnit     {scaleToUnit} {
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
    * \brief Maps from unit name (in this \c UnitSystem) to \c Unit
    */
   Unit const * getUnitFromName(QString const & name) const {
      auto const unitsForThisSystem = this->scaleToUnit.values();
      auto matchingUnit = std::find_if(
         unitsForThisSystem.begin(),
         unitsForThisSystem.end(),
         [name](Measurement::Unit const * unit) {return unit->name == name;}
      );
      if (matchingUnit != unitsForThisSystem.end()) {
         return *matchingUnit;
      }
      return nullptr;
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
      if (amount.unit.getPhysicalQuantity() != this->physicalQuantity) {
         return std::pair(amount.quantity, "");
      }

      auto siAmount = amount.unit.toSI(amount.quantity);

      // Short circuit if there is only one Unit in this UnitSystem or if a specific scale is requested
      if (this->scaleToUnit.size() == 1 || forcedScale) {
         Measurement::Unit const * bb;
         if (this->scaleToUnit.size() == 1) {
            bb = this->scaleToUnit.first();
         } else {
            // It's a coding error to specify a forced scale that is not in the UnitSystem
            Q_ASSERT(this->scaleToUnit.contains(*forcedScale));
            bb = this->scaleToUnit.value(*forcedScale);
         }
         return std::pair(bb->fromSI(siAmount.quantity), bb->name);
      }

      // Search for the smallest measure in this system that's not too big to show the supplied value
      // QMap guarantees that we iterate in the order of its keys, thus here we'll loop from smallest to largest scale
      // (e.g., mg, g, kg).
      Measurement::Unit const * last  = nullptr;
      for (auto it : this->scaleToUnit) {
         if (last != nullptr && qAbs(siAmount.quantity) < it->toSI(it->boundary()).quantity) {
            // Stop looping as we've found a unit that's too big to use (so we'll return the last one, ie the one smaller,
            // below)
            break;
         }
         last = it;
      }

      // It is a programming error if the map was empty (ie we didn't go through the loop at all)
      Q_ASSERT(last != nullptr);
      return std::pair(last->fromSI(siAmount.quantity), last->name);
   }

   // Member variables for impl
   Measurement::UnitSystem & self;
   Measurement::PhysicalQuantity const physicalQuantity;
   Measurement::Unit const * const thickness;
   Measurement::Unit const * const defaultUnit;

   QMap<Measurement::UnitSystem::RelativeScale, Measurement::Unit const *> const scaleToUnit;
};

Measurement::UnitSystem::UnitSystem(Measurement::PhysicalQuantity const physicalQuantity,
                                    Measurement::Unit const * const thickness,
                                    Measurement::Unit const * const defaultUnit,
                                    std::initializer_list<std::pair<Measurement::UnitSystem::RelativeScale const,
                                                                    Measurement::Unit const *> > scaleToUnit,
                                    char const * const uniqueName,
                                    SystemOfMeasurement const systemOfMeasurement) :
   uniqueName{uniqueName},
   systemOfMeasurement{systemOfMeasurement},
   pimpl{std::make_unique<impl>(*this,
                                physicalQuantity,
                                thickness,
                                defaultUnit,
                                scaleToUnit)} {
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

   // make sure we can parse the string
   if (amtUnit.indexIn(qstr) == -1) {
      qDebug() << Q_FUNC_INFO << "Unable to parse" << qstr;
      return Amount{0.0, Measurement::Unit::getCanonicalUnit(this->pimpl->physicalQuantity)};
   }

   double const amt = Localization::toDouble(amtUnit.cap(1), Q_FUNC_INFO);

   QString const unitName = amtUnit.cap(2);

   // Look first in this unit system. If you can't find it here, find it
   // globally. I *think* this finally has all the weird magic right. If the
   // field is marked as "Imperial" and you enter "3 qt" you get 3 imperial
   // qts, 3.6 US qts, 3.41L. If you enter 3L, you get 2.64 imperial qts,
   // 3.17 US qt. If you mean 3 US qt, you are SOL unless you mark the field
   // as US Customary.

   Unit const * unitToUse = nullptr;
   if (!unitName.isEmpty()) {
      // The supplied string specifies units, so see if they are ones we recognise in this unit system
      unitToUse = this->pimpl->getUnitFromName(unitName);
      // If we didn't find the specified units in this UnitSystem, broaden the search and look in all units
      if (!unitToUse) {
         unitToUse = Measurement::Unit::getUnit(unitName, this->pimpl->physicalQuantity);
      }
      if (unitToUse) {
         qDebug() << Q_FUNC_INFO << this->uniqueName << ":" << unitName << "interpreted as" << unitToUse->name;
      } else {
         qDebug() << Q_FUNC_INFO << this->uniqueName << ":" << unitName << "not recognised";
      }
   }

   if (!unitToUse) {
      unitToUse = &defUnit;
   }

   // It is possible for unitToUse to be NULL at this point, so make sure we handle that case
   if (!unitToUse) {
      qDebug() << Q_FUNC_INFO << this->uniqueName << ": Couldn't determine units";
      return Amount{-1.0, Measurement::Unit::getCanonicalUnit(this->pimpl->physicalQuantity)};
   }

   Measurement::Amount siAmount = unitToUse->toSI(amt);
   qDebug() <<
      Q_FUNC_INFO << this->uniqueName << ": " << qstr << "is" << amt << " " << unitToUse->name << "=" << siAmount.quantity <<
      "in" << siAmount.unit.name;

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
      return QString("%L1").arg(this->amountDisplay(Measurement::Amount{result.first, amount.unit}, forcedScale),
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
   auto result = std::find_if(
      systemsForThisPhysicalQuantity.begin(),
      systemsForThisPhysicalQuantity.end(),
      [systemOfMeasurement](Measurement::UnitSystem const * unitSystem) {
         return unitSystem->systemOfMeasurement == systemOfMeasurement;
      }
   );

   if (systemsForThisPhysicalQuantity.end() == result) {
      // It's a coding error if we didn't find a match
      qCritical() <<
         Q_FUNC_INFO << "Unable to find a UnitSystem for SystemOfMeasurement" <<
         Measurement::getDisplayName(systemOfMeasurement) << "and PhysicalQuantity" <<
         Measurement::getDisplayName(physicalQuantity);
      Q_ASSERT(false); // Stop here on a debug build
   }

   return **result;
}

QList<Measurement::UnitSystem const *> Measurement::UnitSystem::getUnitSystems(Measurement::PhysicalQuantity const physicalQuantity) {
   return physicalQuantityToUnitSystems.values(physicalQuantity);
}

QString Measurement::UnitSystem::getUniqueName(Measurement::UnitSystem::RelativeScale relativeScale) {
   auto result = relativeScaleToName.enumToString(relativeScale);
   if (!result) {
      // It's a coding error if we didn't define a name for a RelativeScale
      qCritical() << Q_FUNC_INFO << "Unable to find a name for RelativeScale #" << relativeScale;
      Q_ASSERT(false); // Stop here on a debug build
   }

   return *result;
}

std::optional<Measurement::UnitSystem::RelativeScale> Measurement::UnitSystem::getScaleFromUniqueName(QString relativeScaleAsString) {
   return relativeScaleToName.stringToEnum<Measurement::UnitSystem::RelativeScale>(relativeScaleAsString);
}


//---------------------------------------------------------------------------------------------------------------------
//
// This is where we actually define all the different unit systems
//
//---------------------------------------------------------------------------------------------------------------------
namespace Measurement::UnitSystems {
   //
   // NB: For the mass_Xxxx and volume_Xxxx unit systems, to make Measurement::PhysicalQuantity::Mixed work, we rely on
   //     systemOfMeasurementName being identical for identical systems of measurement (because, for reasons explained
   //     in comments in measurement/PhysicalQuantity.h, we don't explicitly model system of measurement).
   //
   UnitSystem const mass_Metric = UnitSystem(PhysicalQuantity::Mass,
                                             &Measurement::Units::kilograms,
                                             &Measurement::Units::kilograms,
                                             {{UnitSystem::scaleExtraSmall, &Measurement::Units::milligrams},
                                              {UnitSystem::scaleSmall,      &Measurement::Units::grams     },
                                              {UnitSystem::scaleMedium,     &Measurement::Units::kilograms }},
                                             "mass_Metric",
                                             Measurement::SystemOfMeasurement::Metric);

   UnitSystem const mass_UsCustomary = UnitSystem(PhysicalQuantity::Mass,
                                                  &Measurement::Units::pounds,
                                                  &Measurement::Units::pounds,
                                                  {{UnitSystem::scaleExtraSmall, &Measurement::Units::ounces},
                                                   {UnitSystem::scaleSmall,      &Measurement::Units::pounds}},
                                                  "mass_UsCustomary",
                                                  Measurement::SystemOfMeasurement::UsCustomary);

   UnitSystem const mass_Imperial = UnitSystem(PhysicalQuantity::Mass,
                                               &Measurement::Units::imperial_pounds,
                                               &Measurement::Units::imperial_pounds,
                                               {{UnitSystem::scaleExtraSmall, &Measurement::Units::imperial_ounces},
                                                {UnitSystem::scaleSmall,      &Measurement::Units::imperial_pounds}},
                                               "mass_Imperial",
                                               Measurement::SystemOfMeasurement::Imperial);

   UnitSystem const volume_Metric = UnitSystem(PhysicalQuantity::Volume,
                                               &Measurement::Units::liters,
                                               &Measurement::Units::liters,
                                               {{UnitSystem::scaleExtraSmall, &Measurement::Units::milliliters},
                                                {UnitSystem::scaleSmall,      &Measurement::Units::liters     }},
                                               "volume_Metric",
                                               Measurement::SystemOfMeasurement::Metric);

   UnitSystem const volume_UsCustomary = UnitSystem(PhysicalQuantity::Volume,
                                                    &Measurement::Units::us_quarts,
                                                    &Measurement::Units::us_gallons,
                                                    {{UnitSystem::scaleExtraSmall, &Measurement::Units::us_teaspoons  },
                                                     {UnitSystem::scaleSmall,      &Measurement::Units::us_tablespoons},
                                                     {UnitSystem::scaleMedium,     &Measurement::Units::us_cups       },
                                                     {UnitSystem::scaleLarge,      &Measurement::Units::us_quarts     },
                                                     {UnitSystem::scaleExtraLarge, &Measurement::Units::us_gallons    },
                                                     {UnitSystem::scaleHuge,       &Measurement::Units::us_barrels    }},
                                                    "volume_UsCustomary",
                                                    Measurement::SystemOfMeasurement::UsCustomary);

   UnitSystem const volume_Imperial = UnitSystem(PhysicalQuantity::Volume,
                                                 &Measurement::Units::imperial_quarts,
                                                 &Measurement::Units::imperial_gallons,
                                                 {{UnitSystem::scaleExtraSmall, &Measurement::Units::imperial_teaspoons  },
                                                  {UnitSystem::scaleSmall,      &Measurement::Units::imperial_tablespoons},
                                                  {UnitSystem::scaleMedium,     &Measurement::Units::imperial_cups       },
                                                  {UnitSystem::scaleLarge,      &Measurement::Units::imperial_quarts     },
                                                  {UnitSystem::scaleExtraLarge, &Measurement::Units::imperial_gallons    },
                                                  {UnitSystem::scaleHuge,       &Measurement::Units::imperial_barrels    }},
                                                 "volume_Imperial",
                                                 Measurement::SystemOfMeasurement::Imperial);

   UnitSystem const temperature_MetricIsCelsius = UnitSystem(PhysicalQuantity::Temperature,
                                                             nullptr,
                                                             &Measurement::Units::celsius,
                                                             {{UnitSystem::scaleWithout, &Measurement::Units::celsius}},
                                                             "temperature_MetricIsCelsius",
                                                             Measurement::SystemOfMeasurement::Metric);

   UnitSystem const temperature_UsCustomaryIsFahrenheit = UnitSystem(PhysicalQuantity::Temperature,
                                                                     nullptr,
                                                                     &Measurement::Units::fahrenheit,
                                                                     {{UnitSystem::scaleWithout, &Measurement::Units::fahrenheit}},
                                                                     "temperature_UsCustomaryIsFahrenheit",
                                                                     Measurement::SystemOfMeasurement::UsCustomary);

   UnitSystem const time_CoordinatedUniversalTime = UnitSystem(PhysicalQuantity::Time,
                                                               nullptr,
                                                               &Measurement::Units::minutes,
                                                               {{UnitSystem::scaleExtraSmall, &Measurement::Units::seconds},
                                                                {UnitSystem::scaleSmall,      &Measurement::Units::minutes},
                                                                {UnitSystem::scaleMedium,     &Measurement::Units::hours  },
                                                                {UnitSystem::scaleLarge,      &Measurement::Units::days   }},
                                                               "time_CoordinatedUniversalTime",
                                                               Measurement::SystemOfMeasurement::StandardTimeUnits);

   UnitSystem const color_EuropeanBreweryConvention = UnitSystem(PhysicalQuantity::Color,
                                                                 nullptr,
                                                                 &Measurement::Units::ebc,
                                                                 {{UnitSystem::scaleWithout, &Measurement::Units::ebc}},
                                                                 "color_EuropeanBreweryConvention",
                                                                 Measurement::SystemOfMeasurement::EuropeanBreweryConvention);

   UnitSystem const color_StandardReferenceMethod = UnitSystem(PhysicalQuantity::Color,
                                                               nullptr,
                                                               &Measurement::Units::srm,
                                                               {{UnitSystem::scaleWithout, &Measurement::Units::srm}},
                                                               "color_StandardReferenceMethod",
                                                               Measurement::SystemOfMeasurement::StandardReferenceMethod);

   UnitSystem const density_SpecificGravity = UnitSystem(PhysicalQuantity::Density,
                                                         nullptr,
                                                         &Measurement::Units::sp_grav,
                                                         {{UnitSystem::scaleWithout, &Measurement::Units::sp_grav}},
                                                         "density_SpecificGravity",
                                                         Measurement::SystemOfMeasurement::SpecificGravity);

   UnitSystem const density_Plato = UnitSystem(PhysicalQuantity::Density,
                                               nullptr,
                                               &Measurement::Units::plato,
                                               {{UnitSystem::scaleWithout, &Measurement::Units::plato}},
                                               "density_Plato",
                                               Measurement::SystemOfMeasurement::Plato);

   UnitSystem const diastaticPower_Lintner = UnitSystem(PhysicalQuantity::DiastaticPower,
                                                        nullptr,
                                                        &Measurement::Units::lintner,
                                                        {{UnitSystem::scaleWithout, &Measurement::Units::lintner}},
                                                        "diastaticPower_Lintner",
                                                        Measurement::SystemOfMeasurement::Lintner);

   UnitSystem const diastaticPower_WindischKolbach = UnitSystem(PhysicalQuantity::DiastaticPower,
                                                                nullptr,
                                                                &Measurement::Units::wk,
                                                                {{UnitSystem::scaleWithout, &Measurement::Units::wk}},
                                                                "diastaticPower_WindischKolbach",
                                                                Measurement::SystemOfMeasurement::WindischKolbach);

}
