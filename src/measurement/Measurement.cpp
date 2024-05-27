/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/Measurement.cpp is part of Brewtarget, and is copyright the following authors 2010-2023:
 *   • Mark de Wever <koraq@xs4all.nl>
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
#include "measurement/Measurement.h"

#include <QDebug>
#include <QMap>
#include <QString>

#include "Algorithms.h"
#include "Localization.h"
#include "measurement/PhysicalQuantity.h"
#include "measurement/UnitSystem.h"
#include "model/NamedEntity.h"
#include "model/Style.h" // For PropertyNames::Style::colorMin_srm, PropertyNames::Style::colorMax_srm
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"
#include "utils/OptionalHelpers.h"
#include "utils/TypeLookup.h"

namespace {

   int const fieldWidth = 0;
   char const format = 'f';

   /**
    * \brief Stores the current \c Measurement::UnitSystem being used for \b input and \b display for each
    *        \c Measurement::PhysicalQuantity.  Note that we always convert to a standard ("canonical")
    *        \c Measurement::Unit (usually Metric/SI where that's an option) for storing in the DB.
    */
   QMap<Measurement::PhysicalQuantity, Measurement::UnitSystem const *> physicalQuantityToDisplayUnitSystem;

   //
   // Load the previous stored setting for which UnitSystem we use for a particular physical quantity
   //
   void loadDisplayScale(Measurement::PhysicalQuantity const physicalQuantity,
                         BtStringConst const &               settingName,
                         Measurement::UnitSystem    const &  defaultUnitSystem) {
      QString unitSystemName = PersistentSettings::value(settingName, defaultUnitSystem.uniqueName).toString();
      Measurement::UnitSystem const * unitSystem = Measurement::UnitSystem::getInstanceByUniqueName(unitSystemName);
      if (nullptr == unitSystem) {
         qWarning() <<
            Q_FUNC_INFO << "Unrecognised unit system," << unitSystemName << "for" << physicalQuantity <<
            ", defaulting to" << defaultUnitSystem.uniqueName << "(" <<
            Measurement::getDisplayName(defaultUnitSystem.systemOfMeasurement) << ")";
         unitSystem = &defaultUnitSystem;
      }
      Measurement::setDisplayUnitSystem(physicalQuantity, *unitSystem);
      return;
   }

}

//
// There isn't a generic version of this function, just the specialisations below
//
// Note that Qt conversion functions are generally not very accepting of extra characters.  Eg
// \c QString::toInt() will give 0 when parsing "12.34" as it barfs on the decimal point, whereas
// \c std::stoi() will give 12 on the same string input.  Nonetheless, we want to use Localization for decimal
// separators etc.  So, we always convert everything to double first and then, if needed, convert the double
// to an int / unsigned int, as this will give the behaviour we want.
//
template<typename T> [[nodiscard]] T Measurement::extractRawFromString(QString const & input, bool * ok) {
   // This compile-time assert relies on the fact that no type has size 0
   static_assert(sizeof(T) == 0, "Only specializations of stringTo() can be used");
}
template<> [[nodiscard]] int          Measurement::extractRawFromString<int>         (QString const & input, bool * ok) { return static_cast<int         >(Measurement::Unit::splitAmountString(input, ok).first); }
template<> [[nodiscard]] unsigned int Measurement::extractRawFromString<unsigned int>(QString const & input, bool * ok) { return static_cast<unsigned int>(Measurement::Unit::splitAmountString(input, ok).first); }
template<> [[nodiscard]] double       Measurement::extractRawFromString<double>      (QString const & input, bool * ok) { return                           Measurement::Unit::splitAmountString(input, ok).first ; }

[[nodiscard]] QVariant Measurement::extractRawFromString(QString const & input, TypeInfo const & typeInfo, bool * ok) {
   // Optional values are allowed to be blank
   if (typeInfo.isOptional() && Optional::isEmptyOrBlank(input)) {
      return Optional::makeNullOpt(typeInfo);
   }

   bool myOk = false;
   double valueAsDouble = Measurement::Unit::splitAmountString(input, &myOk).first;
   if (ok) {
      *ok = myOk;
   }

   if (typeInfo.isOptional()) {
      // If we couldn't parse an optional value, we'll unset it.  I think this is better than setting it to 0 (which is
      // about the only, ah, option for non-optional values).
      if (!myOk) {
         return Optional::makeNullOpt(typeInfo);
      }

      // We do have something to return, so just make sure it's the right type
      if (typeInfo.typeIndex == typeid(double      )) { return QVariant::fromValue<std::optional<double      >>(                          valueAsDouble ); }
      if (typeInfo.typeIndex == typeid(int         )) { return QVariant::fromValue<std::optional<int         >>(static_cast<int         >(valueAsDouble)); }
      if (typeInfo.typeIndex == typeid(unsigned int)) { return QVariant::fromValue<std::optional<unsigned int>>(static_cast<unsigned int>(valueAsDouble)); }
   } else {
      if (typeInfo.typeIndex == typeid(double      )) { return QVariant::fromValue(                          valueAsDouble ); }
      if (typeInfo.typeIndex == typeid(int         )) { return QVariant::fromValue(static_cast<int         >(valueAsDouble)); }
      if (typeInfo.typeIndex == typeid(unsigned int)) { return QVariant::fromValue(static_cast<unsigned int>(valueAsDouble)); }
   }

   // It's a coding error if we reached here
   qCritical().noquote() <<
      Q_FUNC_INFO << "Unexpected type" << typeInfo << ".  Call stack:" << Logging::getStackTrace();
   Q_ASSERT(false);
   return QVariant{};
}


void Measurement::loadDisplayScales() {
   for (auto const & ii : Measurement::physicalQuantityStringMapping) {
      auto const physicalQuantity = static_cast<Measurement::PhysicalQuantity>(ii.native);
      loadDisplayScale(physicalQuantity,
                       Measurement::getSettingsName(physicalQuantity),
                       Measurement::Unit::getCanonicalUnit(physicalQuantity).getUnitSystem());
   }
   return;
}

void Measurement::saveDisplayScales() {
   for (auto const & ii : Measurement::physicalQuantityStringMapping) {
      auto const physicalQuantity = static_cast<Measurement::PhysicalQuantity>(ii.native);
      PersistentSettings::insert(Measurement::getSettingsName(physicalQuantity),
                                 Measurement::getDisplayUnitSystem(physicalQuantity).uniqueName);
   }
   return;
}

void Measurement::setDisplayUnitSystem(Measurement::PhysicalQuantity physicalQuantity,
                                       Measurement::UnitSystem const & unitSystem) {
   // It's a coding error if we try to store a UnitSystem against a PhysicalQuantity to which it does not relate!
   Q_ASSERT(physicalQuantity == unitSystem.getPhysicalQuantity());
   qDebug() << Q_FUNC_INFO << "Setting UnitSystem for" << physicalQuantity << "to" << unitSystem.uniqueName;
   physicalQuantityToDisplayUnitSystem.insert(physicalQuantity, &unitSystem);
   return;
}

void Measurement::setDisplayUnitSystem(UnitSystem const & unitSystem) {
   // It's a coding error if we try to store a UnitSystem against a PhysicalQuantity to which it does not relate!
   auto physicalQuantity = unitSystem.getPhysicalQuantity();
   Measurement::setDisplayUnitSystem(physicalQuantity, unitSystem);
   return;
}

Measurement::UnitSystem const & Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity physicalQuantity) {
   // It is a coding error if physicalQuantityToDisplayUnitSystem has not had data loaded into it by the time this
   // function is called.
   Q_ASSERT(!physicalQuantityToDisplayUnitSystem.isEmpty());

   Measurement::UnitSystem const * unitSystem = physicalQuantityToDisplayUnitSystem.value(physicalQuantity, nullptr);
   if (nullptr == unitSystem) {
      // This is a coding error
      qCritical() << Q_FUNC_INFO << "Unable to find display unit system for physical quantity" << physicalQuantity;
      Q_ASSERT(false);
   }
   return *unitSystem;
}

QString Measurement::displayQuantity(double quantity, int precision) {
   return QString("%L1").arg(quantity, fieldWidth, format, precision);
}

QString Measurement::displayQuantity(double quantity, int precision, NonPhysicalQuantity const nonPhysicalQuantity) {
   // For percentages, we'd like to show the % symbol after the number
   QString symbol{""};
   if (NonPhysicalQuantity::Percentage == nonPhysicalQuantity) {
      symbol = " %";
   }

   return Measurement::displayQuantity(quantity, precision) + symbol;
}

QString Measurement::displayAmount(Measurement::Amount const & amount,
                                   int precision,
                                   std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement,
                                   std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) {
   // Check for insane values.
   if (Algorithms::isNan(amount.quantity) || Algorithms::isInf(amount.quantity)) {
      return "-";
   }

   // If the caller told us (via forced system of measurement) what UnitSystem to use, use that, otherwise get whatever
   // one we're using generally for related physical property.
   PhysicalQuantity const physicalQuantity = amount.unit->getPhysicalQuantity();
   Measurement::UnitSystem const & displayUnitSystem =
      forcedSystemOfMeasurement ? UnitSystem::getInstance(*forcedSystemOfMeasurement, physicalQuantity) :
                                  Measurement::getDisplayUnitSystem(physicalQuantity);
   return displayUnitSystem.displayAmount(amount, precision, forcedScale);
}

double Measurement::amountDisplay(Measurement::Amount const & amount,
                                  std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement,
                                  std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) {

   // Check for insane values.
   if (Algorithms::isNan(amount.quantity) || Algorithms::isInf(amount.quantity)) {
      return -1.0;
   }

   // If the caller told us (via forced system of measurement) what UnitSystem to use, use that, otherwise get whatever
   // one we're using generally for related physical property.
   PhysicalQuantity const physicalQuantity = amount.unit->getPhysicalQuantity();
   Measurement::UnitSystem const & displayUnitSystem =
      forcedSystemOfMeasurement ? UnitSystem::getInstance(*forcedSystemOfMeasurement, physicalQuantity) :
                                  Measurement::getDisplayUnitSystem(physicalQuantity);

   return displayUnitSystem.amountDisplay(amount, forcedScale);
}

void Measurement::getThicknessUnits(Unit const ** volumeUnit, Unit const ** weightUnit) {
   *volumeUnit = Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Volume).thicknessUnit();
   *weightUnit = Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Mass).thicknessUnit();
   return;
}

QString Measurement::displayThickness( double thick_lkg, bool showUnits ) {
   int fieldWidth = 0;
   char format = 'f';
   int precision = 2;

   Measurement::Unit const * volUnit;
   Measurement::Unit const * weightUnit;
   Measurement::getThicknessUnits(&volUnit, &weightUnit);

   double num = volUnit->fromCanonical(thick_lkg);
   double den = weightUnit->fromCanonical(1.0);

   if (showUnits) {
      return QString("%L1 %2/%3").arg(num/den, fieldWidth, format, precision).arg(volUnit->name).arg(weightUnit->name);
   }

   return QString("%L1").arg(num/den, fieldWidth, format, precision).arg(volUnit->name).arg(weightUnit->name);
}

Measurement::Amount Measurement::qStringToSI(QString qstr,
                                             Measurement::PhysicalQuantity const physicalQuantity,
                                             std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement,
                                             std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) {
   // Commented out this log statement as it otherwise takes up a lot of log space
//   qDebug() <<
//      Q_FUNC_INFO << "Input" << qstr << "of" << physicalQuantity << "; forcedSystemOfMeasurement=" <<
//      forcedSystemOfMeasurement << "; forcedScale=" << forcedScale;

   //
   // If the caller told us that the SystemOfMeasurement and/or RelativeScale on the input (qstr) are "forced" then that
   // information can be used to interpret a case where no (valid) unit is supplied in the input (ie it's just a number
   // rather than number plus units) or where the supplied unit is ambiguous (eg US pints are different than Imperial
   // pints).  Otherwise, just otherwise get whatever UnitSystem we're using generally for related physical property.
   //
   Measurement::UnitSystem const & displayUnitSystem {
      forcedSystemOfMeasurement ? UnitSystem::getInstance(*forcedSystemOfMeasurement, physicalQuantity) :
                                  Measurement::getDisplayUnitSystem(physicalQuantity)
   };
   Measurement::Unit const * defaultUnit {
      forcedScale ? displayUnitSystem.scaleUnit(*forcedScale) : displayUnitSystem.unit()
   };
   // It's a coding error if defaultUnit is null, because it means previousScaleInfo.oldForcedScale was not valid for
   // oldUnitSystem.  However, we can recover.
   if (!defaultUnit) {
      qWarning() << Q_FUNC_INFO << "forcedScale invalid?" << forcedScale;
      defaultUnit = &Measurement::Unit::getCanonicalUnit(physicalQuantity);
   }

   return displayUnitSystem.qstringToSI(qstr, *defaultUnit);
}
