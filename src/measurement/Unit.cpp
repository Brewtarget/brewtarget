/*
 * measurement/Unit.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2022:
 * - Mark de Wever <koraq@xs4all.nl>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
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
#include "measurement/Unit.h"

#include <string>
#include <iostream>

#include <QStringList>
#include <QRegExp>
#include <QDebug>

#include "Algorithms.h"
#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/UnitSystem.h"

namespace {

   QRegExp getRegExpToMatchAmountPlusUnits() {
      QRegExp amtUnit;

      // Make sure we get the right decimal point (. or ,) and the right grouping
      // separator (, or .). Some locales write 1.000,10 and other write
      // 1,000.10. We need to catch both
      QString decimal =  QRegExp::escape(QLocale::system().decimalPoint());
      QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

      amtUnit.setPattern("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
      amtUnit.setCaseSensitivity(Qt::CaseInsensitive);

      return amtUnit;
   }

   QString unitFromString(QString qstr) {
      QRegExp amtUnit = getRegExpToMatchAmountPlusUnits();

      // if the regex dies, return ?
      if (amtUnit.indexIn(qstr) == -1) {
         return QString("?");
      }

      // Get the unit from the second capture
      return amtUnit.cap(2);
   }

   double valueFromString(QString qstr) {
      QRegExp amtUnit = getRegExpToMatchAmountPlusUnits();

      // if the regex dies, return 0.0
      if (amtUnit.indexIn(qstr) == -1) {
         return 0.0;
      }

      return Localization::toDouble(amtUnit.cap(1), Q_FUNC_INFO);
   }

   // Note that, although Unit names (ie abbreviations) are unique within a UnitSystem, they are not globally unique. Eg
   // "L" is the abbreviation/name of both Liters and Lintner; "gal" is the abbreviation/name of the Imperial gallon and
   // the US Customary one; etc.
   QMultiMap<QString, Measurement::Unit const *> nameToUnit;

   // NB: There is no canonical unit for Measurement::PhysicalQuantity::Mixed
   QMap<Measurement::PhysicalQuantity, Measurement::Unit const *> const physicalQuantityToCanonicalUnit {
      {Measurement::PhysicalQuantity::Mass,            &Measurement::Units::kilograms},
      {Measurement::PhysicalQuantity::Volume,          &Measurement::Units::liters},
      {Measurement::PhysicalQuantity::Time,            &Measurement::Units::minutes},
      {Measurement::PhysicalQuantity::Temperature,     &Measurement::Units::celsius},
      {Measurement::PhysicalQuantity::Color,           &Measurement::Units::srm},
      {Measurement::PhysicalQuantity::Density,         &Measurement::Units::sp_grav},
      {Measurement::PhysicalQuantity::DiastaticPower,  &Measurement::Units::lintner}
   };
}

// This private implementation class holds all private non-virtual members of Unit
class Measurement::Unit::impl {
public:
   /**
    * Constructor
    */
   impl(Unit & self,
        UnitSystem const & unitSystem,
        std::function<double(double)> convertToCanonical,
        std::function<double(double)> convertFromCanonical,
        double boundaryValue,
        Measurement::Unit const & canonical) :
      self                {self},
      unitSystem          {unitSystem},
      convertToCanonical  {convertToCanonical},
      convertFromCanonical{convertFromCanonical},
      boundaryValue       {boundaryValue},
      canonical           {canonical} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   // Member variables for impl
   Unit & self;
   UnitSystem const & unitSystem;

   std::function<double(double)> convertToCanonical;
   std::function<double(double)> convertFromCanonical;
   double boundaryValue;
   Measurement::Unit const & canonical; //,:TODO:. Get rid of this
};


Measurement::Unit::Unit(UnitSystem const & unitSystem,
                        QString const unitName,
                        std::function<double(double)> convertToCanonical,
                        std::function<double(double)> convertFromCanonical,
                        double boundaryValue,
                        Measurement::Unit const * canonical) :
   name{unitName},
   pimpl{std::make_unique<impl>(*this,
                                unitSystem,
                                convertToCanonical,
                                convertFromCanonical,
                                boundaryValue,
                                canonical ? *canonical : *this)} {
   nameToUnit.insert(this->name, this);
   return;
}

Measurement::Unit::~Unit() = default;

bool Measurement::Unit::operator==(Unit const & other) const {
   // Since we're not intending to create multiple instances of any given UnitSystem, it should be enough to check
   // the addresses are equal, but, as belt-and-braces, we'll check the names & physical quantities are equal as a
   // fall-back.
   return (this == &other ||
           (this->name == other.name && this->getPhysicalQuantity() == other.getPhysicalQuantity()));
}

Measurement::Unit const & Measurement::Unit::getCanonical() const {
   return Measurement::Unit::getCanonicalUnit(this->getPhysicalQuantity());
}

Measurement::Amount Measurement::Unit::toSI(double amt) const {
   return Measurement::Amount{this->pimpl->convertToCanonical(amt), this->getCanonical()};
}

double Measurement::Unit::fromSI(double amt) const {
   return this->pimpl->convertFromCanonical(amt);
}

Measurement::PhysicalQuantity Measurement::Unit::getPhysicalQuantity() const {
   // The PhysicalQuantity for this Unit is already stored in its UnitSystem, so we don't store it separately here
   return this->pimpl->unitSystem.getPhysicalQuantity();
}

Measurement::UnitSystem const & Measurement::Unit::getUnitSystem() const {
   return this->pimpl->unitSystem;
}

double Measurement::Unit::boundary() const {
   return this->pimpl->boundaryValue;
}

Measurement::Unit const & Measurement::Unit::getCanonicalUnit(Measurement::PhysicalQuantity const physicalQuantity) {
   // It's a coding error if there is no canonical unit for a real physical quantity (ie not Mixed).  (And of course
   // there should be no Unit or UnitSystem for Mixed.)
   Q_ASSERT(physicalQuantityToCanonicalUnit.contains(physicalQuantity));
   auto canonical = physicalQuantityToCanonicalUnit.value(physicalQuantity);
   return *canonical;
}

QString Measurement::Unit::convert(QString qstr, QString toUnit) {

   QString fName = unitFromString(qstr);
   Unit const * f = Measurement::Unit::getUnit(fName);

   double si;
   if (f) {
      double amt = valueFromString(qstr);
      si = f->toSI(amt).quantity;
   } else {
      si = 0.0;
   }

   Unit const * u = Measurement::Unit::getUnit(toUnit);

   // If we couldn't find either unit, or the two units don't match (eg, you
   // cannot convert L to lb)
   if (u == nullptr || f == nullptr || u->getPhysicalQuantity() != f->getPhysicalQuantity()) {
      return QString("%1 ?").arg(Measurement::displayQuantity(si, 3));
   }

   return QString("%1 %2").arg(Measurement::displayQuantity(u->fromSI(si), 3)).arg(toUnit);
}

Measurement::Unit const * Measurement::Unit::getUnit(QString const & name,
                                                     std::optional<Measurement::PhysicalQuantity> physicalQuantity) {
   auto const numMatches = nameToUnit.count(name);
   if (0 == numMatches) {
      qDebug() << Q_FUNC_INFO << name << "does not match any Unit";
      return nullptr;
   }

   qDebug() << Q_FUNC_INFO << name << "has" << numMatches << "match(es)";

   // Under most circumstances, there is a one-to-one relationship between unit string and Unit. C will only map to
   // Measurement::Unit::Celsius, for example. If there's only one match, just return it.
   if (1 == numMatches) {
      Measurement::Unit const * unitToReturn = nameToUnit.value(name);
      if (physicalQuantity && unitToReturn->getPhysicalQuantity() != *physicalQuantity) {
         qWarning() <<
            Q_FUNC_INFO << "Unit" << name << "matches a unit of type" <<
            Measurement::getDisplayName(unitToReturn->getPhysicalQuantity()) << "but caller specified" <<
            Measurement::getDisplayName(*physicalQuantity);
         return nullptr;
      }
      return unitToReturn;
   }

   // That solved something like 99% of the use cases. Now we have to handle those pesky volumes.
   // Loop through the found Units, like Measurement::Unit::us_quart and
   // Measurement::Unit::imperial_quart, and try to find one that matches the global default.
   Measurement::Unit const * defUnit = nullptr;
   for (auto ii = nameToUnit.find(name); ii != nameToUnit.end() && ii.key() == name; ++ii) {
      Measurement::Unit const * unit = ii.value();
      auto const & displayUnitSystem = Measurement::getDisplayUnitSystem(unit->getPhysicalQuantity());
      qDebug() <<
         Q_FUNC_INFO << "Look at" << *unit << "from" << unit->getUnitSystem() << "(Display Unit System for" <<
         unit->getPhysicalQuantity() << "is" << displayUnitSystem << ")";
      if (physicalQuantity && unit->getPhysicalQuantity() != *physicalQuantity) {
         // If the caller knows the amount is, say, a Volume, don't bother trying to match against units for any other
         // physical quantity.
         qDebug() << Q_FUNC_INFO << "Ignoring match in" << unit->getPhysicalQuantity() << "as not" << *physicalQuantity;
         continue;
      }

      if (displayUnitSystem == unit->getUnitSystem()) {
         // We found a match that belongs to one of the global default unit systems
         return unit;
      }

      // Save this for later if we need it - ie if we don't find a better match
      defUnit = unit;
   }

   // If we got here, we couldn't find a match. Unless something weird has
   // happened, that means you entered "qt" into a field and the system
   // default is SI. At that point, just use the USCustomary
   return defUnit;
}


// This is where we actually define all the different units and how to convert them to/from their canonical equivalents
// Previously this was done with a huge number of subclasses, but lambdas mean that's no longer necessary
// Note that we always need to define the canonical Unit for a given PhysicalQuantity before any others
//
namespace Measurement::Units {
   //: NOTE FOR TRANSLATORS: The abbreviated name of each unit (eg "kg" for kilograms, "g" for grams, etc) must be
   //  unique for that type of unit.  Eg you cannot have two units of weight with the same abbreviated name.

   // === Mass ===
   // See comment in measurement/UnitSystem.cpp for why we have separate entities for US Customary pounds/ounces and Imperials ones, even though they are, in fact, the same
   Unit const kilograms            = Unit{Measurement::UnitSystems::mass_Metric,                         QObject::tr("kg"),   [](double x){return x;},               [](double y){return y;},                1.0};
   Unit const grams                = Unit{Measurement::UnitSystems::mass_Metric,                         QObject::tr("g"),    [](double x){return x/1000.0;},        [](double y){return y*1000.0;},         1.0,  &kilograms};
   Unit const milligrams           = Unit{Measurement::UnitSystems::mass_Metric,                         QObject::tr("mg"),   [](double x){return x/1000000.0;},     [](double y){return y*1000000.0;},      1.0,  &kilograms};
   Unit const pounds               = Unit{Measurement::UnitSystems::mass_UsCustomary,                    QObject::tr("lb"),   [](double x){return x*0.45359237;},    [](double y){return y/0.45359237;},     1.0,  &kilograms};
   Unit const ounces               = Unit{Measurement::UnitSystems::mass_UsCustomary,                    QObject::tr("oz"),   [](double x){return x*0.0283495231;},  [](double y){return y/0.0283495231;},   1.0,  &kilograms};
   Unit const imperial_pounds      = Unit{Measurement::UnitSystems::mass_Imperial,                       QObject::tr("lb"),   [](double x){return x*0.45359237;},    [](double y){return y/0.45359237;},     1.0,  &kilograms};
   Unit const imperial_ounces      = Unit{Measurement::UnitSystems::mass_Imperial,                       QObject::tr("oz"),   [](double x){return x*0.0283495231;},  [](double y){return y/0.0283495231;},   1.0,  &kilograms};
   // === Volume ===
   Unit const liters               = Unit{Measurement::UnitSystems::volume_Metric,                       QObject::tr("L"),    [](double x){return x;},               [](double y){return y;},                1.0};
   Unit const milliliters          = Unit{Measurement::UnitSystems::volume_Metric,                       QObject::tr("mL"),   [](double x){return x/1000.0;},        [](double y){return y*1000.0;},         1.0,  &liters};
   Unit const us_barrels           = Unit{Measurement::UnitSystems::volume_UsCustomary,                  QObject::tr("bbl"),  [](double x){return x*117.34777;},     [](double y){return y/117.34777;},      1.0,  &liters};
   Unit const us_gallons           = Unit{Measurement::UnitSystems::volume_UsCustomary,                  QObject::tr("gal"),  [](double x){return x*3.78541178;},    [](double y){return y/3.78541178;},     1.0,  &liters};
   Unit const us_quarts            = Unit{Measurement::UnitSystems::volume_UsCustomary,                  QObject::tr("qt"),   [](double x){return x*0.946352946;},   [](double y){return y/0.946352946;},    1.0,  &liters};
   Unit const us_cups              = Unit{Measurement::UnitSystems::volume_UsCustomary,                  QObject::tr("cup"),  [](double x){return x*0.236588236;},   [](double y){return y/0.236588236;},    0.25, &liters};
   Unit const us_tablespoons       = Unit{Measurement::UnitSystems::volume_UsCustomary,                  QObject::tr("tbsp"), [](double x){return x*0.0147867648;},  [](double y){return y/0.0147867648;},   1.0,  &liters};
   Unit const us_teaspoons         = Unit{Measurement::UnitSystems::volume_UsCustomary,                  QObject::tr("tsp"),  [](double x){return x*0.00492892159;}, [](double y){return y/0.00492892159;},  1.0,  &liters};
   Unit const imperial_barrels     = Unit{Measurement::UnitSystems::volume_Imperial,                     QObject::tr("bbl"),  [](double x){return x*163.659;},       [](double y){return y/163.659;},        1.0,  &liters};
   Unit const imperial_gallons     = Unit{Measurement::UnitSystems::volume_Imperial,                     QObject::tr("gal"),  [](double x){return x*4.54609;},       [](double y){return y/4.54609;},        1.0,  &liters};
   Unit const imperial_quarts      = Unit{Measurement::UnitSystems::volume_Imperial,                     QObject::tr("qt"),   [](double x){return x*1.1365225;},     [](double y){return y/1.1365225;},      1.0,  &liters};
   Unit const imperial_cups        = Unit{Measurement::UnitSystems::volume_Imperial,                     QObject::tr("cup"),  [](double x){return x*0.284130625;},   [](double y){return y/0.284130625;},    0.25, &liters};
   Unit const imperial_tablespoons = Unit{Measurement::UnitSystems::volume_Imperial,                     QObject::tr("tbsp"), [](double x){return x*0.0177581714;},  [](double y){return y/0.0177581714;},   1.0,  &liters};
   Unit const imperial_teaspoons   = Unit{Measurement::UnitSystems::volume_Imperial,                     QObject::tr("tsp"),  [](double x){return x*0.00591939047;}, [](double y){return y/0.00591939047;},  1.0,  &liters};
   // === Time ===
   Unit const minutes              = Unit{Measurement::UnitSystems::time_CoordinatedUniversalTime,       QObject::tr("min"),  [](double x){return x;},               [](double y){return y;},                1.0};
   Unit const seconds              = Unit{Measurement::UnitSystems::time_CoordinatedUniversalTime,       QObject::tr("s"),    [](double x){return x/60.0;},          [](double y){return y*60.0;},           90.0, &minutes};
   Unit const hours                = Unit{Measurement::UnitSystems::time_CoordinatedUniversalTime,       QObject::tr("hr"),   [](double x){return x*60.0;},          [](double y){return y/60.0;},           2.0,  &minutes};
   Unit const days                 = Unit{Measurement::UnitSystems::time_CoordinatedUniversalTime,       QObject::tr("day"),  [](double x){return x*1440.0;},        [](double y){return y/1440.0;},         1.0,  &minutes};
   // === Temperature ===
   Unit const celsius              = Unit{Measurement::UnitSystems::temperature_MetricIsCelsius,         QObject::tr("C"),    [](double x){return x;},               [](double y){return y;},                1.0};
   Unit const fahrenheit           = Unit{Measurement::UnitSystems::temperature_UsCustomaryIsFahrenheit, QObject::tr("F"),    [](double x){return (x-32)*5.0/9.0;},  [](double y){return y * 9.0/5.0 + 32;}, 1.0,  &celsius};
   // === Color ===
   Unit const srm                  = Unit{Measurement::UnitSystems::color_StandardReferenceMethod,       QObject::tr("srm"),  [](double x){return x;},               [](double y){return y;},                1.0};
   Unit const ebc                  = Unit{Measurement::UnitSystems::color_EuropeanBreweryConvention,     QObject::tr("ebc"),  [](double x){return x * 12.7/25.0;},   [](double y){return y * 25.0/12.7;},    1.0,  &srm};
   // == Density ===
   Unit const sp_grav              = Unit{Measurement::UnitSystems::density_SpecificGravity,             QObject::tr("sg"),   [](double x){return x;},               [](double y){return y;},                1.0};
   Unit const plato                = Unit{Measurement::UnitSystems::density_Plato,
                                          QObject::tr("P"),
                                          [](double x){return x == 0.0 ? 0.0 : Algorithms::PlatoToSG_20C20C(x);},
                                          [](double y){return y == 0.0 ? 0.0 : Algorithms::SG_20C20C_toPlato(y);},
                                          1.0,
                                          &sp_grav};
   // == Diastatic power ==
   Unit const lintner              = Unit{Measurement::UnitSystems::diastaticPower_Lintner,              QObject::tr("L"),    [](double x){return x;},               [](double y){return y;},                1.0};
   Unit const wk                   = Unit{Measurement::UnitSystems::diastaticPower_WindischKolbach,      QObject::tr("WK"),   [](double x){return (x + 16) / 3.5;},  [](double y){return 3.5 * y - 16;},     1.0,  &lintner};
}
