/*
 * UnitSystem.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
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
#include "UnitSystem.h"

#include <QDebug>
#include <QLocale>
#include <QRegExp>

#include "brewtarget.h"
#include "Unit.h"

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
}

UnitSystem::UnitSystem(Unit::UnitType type,
                       Unit const * thickness,
                       Unit const * defaultUnit,
                       std::initializer_list<std::pair<Unit::unitScale, Unit const *> > scaleToUnitEntries,
                       std::initializer_list<std::pair<QString, Unit const *> > qstringToUnitEntries,
                       char const * name) :
   type{type},
   thickness{thickness},
   defaultUnit{defaultUnit},
   scaleToUnit{scaleToUnitEntries},
   qstringToUnit{qstringToUnitEntries},
   name{name} {
   return;
}

double UnitSystem::qstringToSI(QString qstr, Unit const * defUnit, bool force, Unit::unitScale scale) const {
   Unit const * u = defUnit;
   Unit const * found = 0;

   // make sure we can parse the string
   if (amtUnit.indexIn(qstr) == -1) {
      return 0.0;
   }

   double amt = Brewtarget::toDouble( amtUnit.cap(1), "UnitSystem::qstringToSI()");

   QString unit = amtUnit.cap(2);

   // Look first in this unit system. If you can't find it here, find it
   // globally. I *think* this finally has all the weird magic right. If the
   // field is marked as "Imperial" and you enter "3 qt" you get 3 imperial
   // qts, 3.6 US qts, 3.41L. If you enter 3L, you get 2.64 imperial qts,
   // 3.17 US qt. If you mean 3 US qt, you are SOL unless you mark the field
   // as US Customary.

   if (!unit.isEmpty()) {
      found = this->qstringToUnit.value(unit);
   } else if (scale != Unit::noScale) {
      found = this->scaleToUnit.value(scale);
   }

   if (!found) {
      found = Unit::getUnit(unit, false);
   }

   // If the calling method isn't overriding the search and we actually found something, use it
   if (!force && found) {
      u = found;
   }

   // It is possible for u to be NULL at this point, so make sure we handle that case
   if (u == nullptr) {
      return -1.0;
   }

   return u->toSI(amt);
}

QString UnitSystem::displayAmount(double amount, Unit const * units, int precision, Unit::unitScale scale) const {
   // If the precision is not specified, we take the default one
   if (precision < 0) {
      precision = defaultPrecision;
   }

   auto result = this->displayableAmount(amount, units, scale);

   if (result.second.isEmpty()) {
      return QString("%L1").arg(this->amountDisplay(result.first, units, scale), fieldWidth, format, precision);
   }

   return QString("%L1 %2").arg(result.first, fieldWidth, format, precision).arg(result.second);
}

double UnitSystem::amountDisplay(double amount, Unit const * units, Unit::unitScale scale) const {
   // Essentially we're just returning the numeric part of the displayable amount
   return this->displayableAmount(amount, units, scale).first;
}

Unit const * UnitSystem::scaleUnit(Unit::unitScale scale) const {
   return this->scaleToUnit.contains(scale) ? this->scaleToUnit.value(scale) : nullptr;
}

Unit const * UnitSystem::thicknessUnit() const {
   return this->thickness;
}

Unit const * UnitSystem::unit() const {
   return this->defaultUnit;
}

QString const & UnitSystem::unitType() const {
   return this->name;
}

std::pair<double, QString> UnitSystem::displayableAmount(double amount, Unit const * units, Unit::unitScale scale) const {
   // Special cases
   if (units == nullptr || units->getUnitType() != this->type) {
      return std::pair(amount, "");
   }

   // Short circuit if the 'without' key is defined
   if (this->scaleToUnit.contains(Unit::scaleWithout)) {
      scale = Unit::scaleWithout;
   }

   double SIAmount = units->toSI( amount );

   // If a specific scale is provided, just use that and don't loop.
   if (this->scaleToUnit.contains(scale) ) {
      Unit const * bb = this->scaleToUnit.value(scale);
      return std::pair(bb->fromSI(SIAmount), bb->getUnitName());
   }

   // Search for the smallest measure in this system that's not too big to show the supplied value
   // QMap guarantees that we iterate in the order of its keys, thus here we'll loop from smallest to largest scale
   // (e.g., mg, g, kg).
   Unit const * last  = nullptr;
   for (auto it : this->scaleToUnit) {
      if (last != nullptr && qAbs(SIAmount) < it->toSI(it->boundary())) {
         // Stop looping as we've found a unit that's too big to use (so we'll return the last one, ie the one smaller,
         // below)
         break;
      }
      last = it;
   }

   // It is a programming error if the map was empty (ie we didn't go through the loop at all)
   Q_ASSERT(last != nullptr);
   return std::pair(last->fromSI(SIAmount), last->getUnitName());
}

//---------------------------------------------------------------------------------------------------------------------
//
// This is where we actually define all the different unit systems
//
//---------------------------------------------------------------------------------------------------------------------
UnitSystem const UnitSystems::usWeightUnitSystem = UnitSystem(Unit::Mass,
                                                              &Units::pounds,
                                                              &Units::pounds,
                                                              {{Unit::scaleExtraSmall, &Units::ounces},
                                                              {Unit::scaleSmall,      &Units::pounds}},
                                                              {{"oz", &Units::ounces},
                                                               {"lb", &Units::pounds}},
                                                              "USCustomary");

UnitSystem const UnitSystems::siWeightUnitSystem = UnitSystem(Unit::Mass,
                                                              &Units::kilograms,
                                                              &Units::kilograms,
                                                              {{Unit::scaleExtraSmall, &Units::milligrams},
                                                               {Unit::scaleSmall,      &Units::grams     },
                                                               {Unit::scaleMedium,     &Units::kilograms }},
                                                              {{"mg", &Units::milligrams},
                                                              { "g", &Units::grams     },
                                                              {"kg", &Units::kilograms }},
                                                              "SI");

UnitSystem const UnitSystems::imperialVolumeUnitSystem = UnitSystem(Unit::Volume,
                                                                    &Units::imperial_quarts,
                                                                    &Units::imperial_gallons,
                                                                    {{Unit::scaleExtraSmall, &Units::imperial_teaspoons  },
                                                                     {Unit::scaleSmall,      &Units::imperial_tablespoons},
                                                                     {Unit::scaleMedium,     &Units::imperial_cups       },
                                                                     {Unit::scaleLarge,      &Units::imperial_quarts     },
                                                                     {Unit::scaleExtraLarge, &Units::imperial_gallons    },
                                                                     {Unit::scaleHuge,       &Units::imperial_barrels    }},
                                                                    {{"tsp", &Units::imperial_teaspoons  },
                                                                     {"tbs", &Units::imperial_tablespoons},
                                                                     {"cup", &Units::imperial_cups       },
                                                                     {"qt",  &Units::imperial_quarts     },
                                                                     {"gal", &Units::imperial_gallons    },
                                                                     {"bbl", &Units::imperial_barrels    }},
                                                                    "Imperial");

UnitSystem const UnitSystems::usVolumeUnitSystem = UnitSystem(Unit::Volume,
                                                              &Units::us_quarts,
                                                              &Units::us_gallons,
                                                              {{Unit::scaleExtraSmall, &Units::us_teaspoons  },
                                                               {Unit::scaleSmall,      &Units::us_tablespoons},
                                                               {Unit::scaleMedium,     &Units::us_cups       },
                                                               {Unit::scaleLarge,      &Units::us_quarts     },
                                                               {Unit::scaleExtraLarge, &Units::us_gallons    },
                                                               {Unit::scaleHuge,       &Units::us_barrels    }},
                                                              {{"tsp", &Units::us_teaspoons  },
                                                               {"tbs", &Units::us_tablespoons},
                                                               {"cup", &Units::us_cups       },
                                                               {"qt",  &Units::us_quarts     },
                                                               {"gal", &Units::us_gallons    },
                                                               {"bbl", &Units::us_barrels    }},
                                                              "USCustomary");

UnitSystem const UnitSystems::siVolumeUnitSystem = UnitSystem(Unit::Volume,
                                                              &Units::liters,
                                                              &Units::liters,
                                                              {{Unit::scaleExtraSmall, &Units::milliliters},
                                                               {Unit::scaleSmall,      &Units::liters     }},
                                                              {{"mL", &Units::milliliters},
                                                               {"ml", &Units::milliliters},
                                                               {"L",  &Units::liters     },
                                                               {"l",  &Units::liters     }},
                                                              "SI");

UnitSystem const UnitSystems::celsiusTempUnitSystem = UnitSystem(Unit::Temp,
                                                                 nullptr,
                                                                 &Units::celsius,
                                                                 {{Unit::scaleWithout, &Units::celsius}},
                                                                 {{"C", &Units::celsius}},
                                                                 "SI");

UnitSystem const UnitSystems::fahrenheitTempUnitSystem = UnitSystem(Unit::Temp,
                                                                    nullptr,
                                                                    &Units::fahrenheit,
                                                                    {{Unit::scaleWithout, &Units::fahrenheit}},
                                                                    {{"F", &Units::fahrenheit}},
                                                                    "Fahrenheit");

UnitSystem const UnitSystems::timeUnitSystem = UnitSystem(Unit::Time,
                                                          nullptr,
                                                          &Units::minutes,
                                                          {{Unit::scaleExtraSmall, &Units::seconds},
                                                           {Unit::scaleSmall,      &Units::minutes},
                                                           {Unit::scaleMedium,     &Units::hours  },
                                                           {Unit::scaleLarge,      &Units::days   }},
                                                          {{"s", &Units::seconds},
                                                           {"m", &Units::minutes},
                                                           {"h", &Units::hours  },
                                                           {"d", &Units::days   }},
                                                          "entropy");

UnitSystem const UnitSystems::ebcColorUnitSystem = UnitSystem(Unit::Color,
                                                              nullptr,
                                                              &Units::ebc,
                                                              {{Unit::scaleWithout, &Units::ebc}},
                                                              {{"ebc", &Units::ebc}},
                                                              "Color");

UnitSystem const UnitSystems::srmColorUnitSystem = UnitSystem(Unit::Color,
                                                              nullptr,
                                                              &Units::srm,
                                                              {{Unit::scaleWithout, &Units::srm}},
                                                              {{"srm", &Units::srm}},
                                                              "Color");

UnitSystem const UnitSystems::sgDensityUnitSystem = UnitSystem(Unit::Density,
                                                               nullptr,
                                                               &Units::sp_grav,
                                                               {{Unit::scaleWithout, &Units::sp_grav}},
                                                               {{"sg", &Units::sp_grav}},
                                                               "Density");

UnitSystem const UnitSystems::platoDensityUnitSystem = UnitSystem(Unit::Density,
                                                                  nullptr,
                                                                  &Units::plato,
                                                                  {{Unit::scaleWithout, &Units::plato}},
                                                                  {{"P", &Units::plato}},
                                                                  "Density");

UnitSystem const UnitSystems::lintnerDiastaticPowerUnitSystem = UnitSystem(Unit::DiastaticPower,
                                                                           nullptr,
                                                                           &Units::lintner,
                                                                           {{Unit::scaleWithout, &Units::lintner}},
                                                                           {{"lintner", &Units::lintner}},
                                                                           "DiastaticPower");

UnitSystem const UnitSystems::wkDiastaticPowerUnitSystem = UnitSystem(Unit::DiastaticPower,
                                                                      nullptr,
                                                                      &Units::wk,
                                                                      {{Unit::scaleWithout, &Units::wk}},
                                                                      {{"wk", &Units::wk}},
                                                                      "DiastaticPower");
