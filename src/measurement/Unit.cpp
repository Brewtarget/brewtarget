/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/Unit.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
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
#include "measurement/Unit.h"

#include <iostream>
#include <mutex>    // For std::once_flag etc
#include <string>

#include <QStringList>
#include <QRegularExpression>
#include <QDebug>

#include "Algorithms.h"
#include "Localization.h"
#include "Logging.h"
#include "measurement/Measurement.h"
#include "measurement/UnitSystem.h"

namespace {

   /**
    * \brief This is useful to allow us to initialise \c unitNameLookup and \c physicalQuantityToCanonicalUnit after
    *        all \c Unit and \c UnitSystem objects have been created.
    */
   QVector<Measurement::Unit const *> listOfAllUnits;

   /**
    * \brief This allows us to ensure that \c Unit::initialiseLookups is called exactly once
    */
   std::once_flag initFlag_Lookups;

   //
   // Note that, although Unit names (ie abbreviations) are unique within an individual UnitSystem, some are are not
   // globally unique, and some are not even unique within a PhysicalQuantity.   For example:
   //    - "L" is the abbreviation/name of both Liters and Lintner
   //    - "gal" is the abbreviation/name of the Imperial gallon and the US Customary one
   //
   // Almost all of the time when we are doing look-ups, we know the PhysicalQuantity (and it is not meaningful for the
   // user to specify units relating to a different PhysicalQuantity) so it makes sense to group look-ups by that.
   //
   struct NameLookupKey {
      Measurement::PhysicalQuantity physicalQuantity;
      QString                       lowerCaseUnitName;
      //
      // We need an ordering on the struct to use it as a key of QMap / QMultiMap.  AIUI, these two operators suffice
      // for the compiler to deduce all the others.
      //
      friend auto operator==(NameLookupKey const & lhs, NameLookupKey const & rhs);
      friend auto operator<=>(NameLookupKey const & lhs, NameLookupKey const & rhs);

   };

   auto operator==(NameLookupKey const & lhs, NameLookupKey const & rhs) {
      return lhs.physicalQuantity == rhs.physicalQuantity && lhs.lowerCaseUnitName == rhs.lowerCaseUnitName;
   }

   auto operator<=>(NameLookupKey const & lhs, NameLookupKey const & rhs) {
      if (lhs.physicalQuantity < rhs.physicalQuantity) { return std::strong_ordering::less; }
      if (lhs.physicalQuantity > rhs.physicalQuantity) { return std::strong_ordering::greater; }
      if (lhs.lowerCaseUnitName < rhs.lowerCaseUnitName) { return std::strong_ordering::less; }
      if (lhs.lowerCaseUnitName > rhs.lowerCaseUnitName) { return std::strong_ordering::greater; }
      return std::strong_ordering::equal;
   }

   QMultiMap<NameLookupKey, Measurement::Unit const *> unitNameLookup;

   QMap<Measurement::PhysicalQuantity, Measurement::Unit const *> physicalQuantityToCanonicalUnit;

   /**
    * \brief Get all units matching a given name and physical quantity
    *
    * \param name
    * \param physicalQuantity
    * \param caseInensitiveMatching If \c true, do a case-insensitive search.  Eg, match "ml" for milliliters, even
    *                               though the correct name is "mL".  This should always be safe to do, as AFAICT there
    *                               are no current or foreseeable units that _we_ use whose names only differ by case --
    *                               or, at least, that's the case in English...
    */
   QList<Measurement::Unit const *> getUnitsByNameAndPhysicalQuantity(QString const & name,
                                                                      Measurement::PhysicalQuantity const & physicalQuantity,
                                                                      bool const caseInensitiveMatching) {
      // Need this before we reference unitNameLookup or physicalQuantityToCanonicalUnit
      std::call_once(initFlag_Lookups, &Measurement::Unit::initialiseLookups);

      auto matches = unitNameLookup.values(NameLookupKey{physicalQuantity, name.toLower()});
      // Commented out this log statement as it otherwise takes up a lot of log space
//      qDebug() << Q_FUNC_INFO << name << "has" << matches.length() << "case-insensitive match(es)";

      if (caseInensitiveMatching) {
         return matches;
      }

      // If we ever want to do case insensitive matching (which we think should be rare), the simplest thing is just to
      // go through all the case-insensitive matches and exclude those that aren't an exact match
      decltype(matches) filteredMatches;
      for (auto match : matches) {
         if (match->name == name) {
            filteredMatches.append(match);
         }
      }
      qDebug() << Q_FUNC_INFO << name << "has" << filteredMatches.length() << "case-sensitive match(es)";
      return filteredMatches;
   }

   /**
    * \brief Get all units matching a given name, but without knowing the physical quantity.  Pretty much the only time
    *        we need this is in \c ConverterTool to do contextless conversions.
    *
    * \param name
    * \param caseInensitiveMatching If \c true, do a case-insensitive search.  Eg, match "ml" for milliliters, even
    *                               though the correct name is "mL".
    */
   QList<Measurement::Unit const *> getUnitsOnlyByName(QString const & name) {
      // Need this before we reference unitNameLookup or physicalQuantityToCanonicalUnit
      std::call_once(initFlag_Lookups, &Measurement::Unit::initialiseLookups);

      QList<Measurement::Unit const *> allMatches;
      auto const & keys {unitNameLookup.uniqueKeys()};
      for (auto const & key : keys) {

         if (name.toLower() == key.lowerCaseUnitName) {
            auto matches = unitNameLookup.values(key);
            allMatches.append(matches);
            qDebug() <<
               Q_FUNC_INFO << "Added" << matches.length() << "matches for" << key.lowerCaseUnitName << "/" <<
               key.physicalQuantity << ":";
            for (auto & match : matches) {
               qDebug() << Q_FUNC_INFO << " - " << match;
            }
         }
      }
      return allMatches;
   }

   QString displayableConvert(Measurement::Unit const & fromUnit,
                              Measurement::Unit const &   toUnit,
                              double const fromQuantity) {
      double const canonicalQuantity = fromUnit.toCanonical(fromQuantity).quantity;
      double const toQuantity        = toUnit.fromCanonical(canonicalQuantity);
      return QString("%1 %2").arg(Measurement::displayQuantity(toQuantity, 3)).arg(toUnit.name);
   }
}

// This private implementation class holds all private non-virtual members of Unit
class Measurement::Unit::impl {
public:
   /**
    * Simple case constructor -- conversion to/from canonical units is just multiplication/division
    */
   impl(Unit const & self,
        UnitSystem const & unitSystem,
        double const multiplierToCanonical,
        double const boundaryValue,
        bool const isCanonical) :
      m_self                 {self},
      m_unitSystem           {unitSystem},
      m_multiplierToCanonical{multiplierToCanonical},
      m_convertToCanonical   {},
      m_convertFromCanonical {},
      m_boundaryValue        {boundaryValue},
      m_isCanonical          {isCanonical} {
      // If this is a canonical unit then, by definition, its multiplier should be 1.0.  Usually we wouldn't compare
      // doubles, but I'm pretty sure comparing against 1.0 is safe in this context because there will never be a
      // rounding error from the `1.0` literal.
      //
      // Note, however, that it _can_ be valid for a non-canonical unit to have a 1.0 multiplier to and from canonical
      // units (eg Lovibond is a no-op conversion to/from SRM).
      Q_ASSERT((isCanonical && 1.0 == multiplierToCanonical) || !isCanonical);

      // It's a coding error for the multiplier to be zero.  Again, I think this is an OK comparison to do since we're
      // checking for source code error rather than "value is so close to zero it might as well be zero".
      Q_ASSERT(0.0 != multiplierToCanonical);

      return;
   }

   /**
    * Complex-case constructor -- conversion to/from canonical units requires formulae (via lambda functions).  By
    * definition, this cannot be a canonical unit.
    */
   impl(Unit const & self,
        UnitSystem const & unitSystem,
        std::function<double(double)> const convertToCanonical,
        std::function<double(double)> const convertFromCanonical,
        double const boundaryValue) :
      m_self                 {self},
      m_unitSystem           {unitSystem},
      m_multiplierToCanonical{std::nullopt},
      m_convertToCanonical   {convertToCanonical},
      m_convertFromCanonical {convertFromCanonical},
      m_boundaryValue        {boundaryValue},
      m_isCanonical          {false} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   // Member variables for impl
   Unit const & m_self;
   UnitSystem const & m_unitSystem;

   std::optional<double> const m_multiplierToCanonical = std::nullopt;
   std::function<double(double)> const m_convertToCanonical = {};
   std::function<double(double)> const m_convertFromCanonical = {};
   double const m_boundaryValue;
   bool const m_isCanonical;
   //
   // TBD: We could store a pointer to the canonical Unit here, since we have it in the constructors below
   //
};

Measurement::Unit::Unit(UnitSystem const & unitSystem,
                        QString const unitName,
                        double const multiplierToCanonical,
                        Measurement::Unit const * canonical,
                        double const boundaryValue) :
   name{unitName},
   pimpl{std::make_unique<impl>(*this,
                                unitSystem,
                                multiplierToCanonical,
                                boundaryValue,
                                (canonical == nullptr))} {
   //
   // You might think here would be a neat place to add the Unit we are constructing to unitNameLookup and, if
   // appropriate, physicalQuantityToCanonicalUnit.  However, there is not guarantee that unitSystem is constructed at
   // this point, so unitSystem.getPhysicalQuantity() could result in a core dump.
   //
   // What we can do safely is add ourselves to listOfAllUnits
   //
   listOfAllUnits.append(this);
   return;
}

Measurement::Unit::Unit(UnitSystem const & unitSystem,
                        QString const unitName,
                        std::function<double(double)> convertToCanonical,
                        std::function<double(double)> convertFromCanonical,
                        Measurement::Unit const * canonical,
                        double const boundaryValue) :
   name{unitName},
   pimpl{std::make_unique<impl>(*this,
                                unitSystem,
                                convertToCanonical,
                                convertFromCanonical,
                                boundaryValue)} {
   // It's a coding error if we used this version of the constructor for a canonical unit
   Q_ASSERT(canonical);

   // See comment in other version of constructor above
   listOfAllUnits.append(this);
   return;
}

Measurement::Unit::~Unit() = default;

void Measurement::Unit::initialiseLookups() {
   for (auto const unit : listOfAllUnits) {
      Measurement::PhysicalQuantity const physicalQuantity = unit->pimpl->m_unitSystem.getPhysicalQuantity();
      unitNameLookup.insert(NameLookupKey{physicalQuantity, unit->name.toLower()}, unit);
      if (unit->pimpl->m_isCanonical) {
         physicalQuantityToCanonicalUnit.insert(physicalQuantity, unit);
      }
   }
   return;
}

std::pair<double, QString> Measurement::Unit::splitAmountString(QString const & inputString, bool * ok) {
   // Assume it didn't work until it did.  It's less code this way!
   if (ok) {
      *ok = false;
   }

   // All functions in QRegularExpression are reentrant, so it should be safe to use as a shared const in multi-threaded
   // code.
   static QRegularExpression const amtUnit {
      //
      // For the numeric part (the quantity) we need to make sure we get the right decimal point (. or ,) and the right
      // grouping separator (, or .).  Some locales write 1.000,10 and others write 1,000.10.  We need to catch both.
      //
      // For the units, we have to be a bit careful.  We used to use "\\w" to match "word characters" for the unit name.
      // This was fine when we had "simple" unit names such as "kg" and "floz", but it breaks for names containing
      // symbols, such as "L/kg" or "c/g·C".  Instead, we have to match non-space characters
      //
      "((?:\\d+" + QRegularExpression::escape(Localization::getLocale().groupSeparator()) + ")?\\d+(?:" +
      QRegularExpression::escape(Localization::getLocale().decimalPoint()) + "\\d+)?|" +
      QRegularExpression::escape(Localization::getLocale().decimalPoint()) + "\\d+)\\s*([^\\s]+)?",
      QRegularExpression::CaseInsensitiveOption
   };

   // Make sure we can parse the string
   QRegularExpressionMatch match = amtUnit.match(inputString);
   if (!match.hasMatch()) {
      qDebug() << Q_FUNC_INFO << "Unable to parse" << inputString << "so treating as 0.0";
      return std::pair<double, QString>{0.0, ""};
   }

   QString const unitName = match.captured(2);

   double quantity = 0.0;
   QString numericPartOfInput{match.captured(1)};
   try {
      quantity = Localization::toDouble(numericPartOfInput, Q_FUNC_INFO);
      // If we didn't throw an exception then all must finally be OK!
      if (ok) {
         *ok = true;
      }
   } catch (std::invalid_argument const & ex) {
      // If we get this error it's most probably either a bug in our regular expression or a problem with
      // Localization::getLocale().
      qWarning() << Q_FUNC_INFO << "Could not parse" << numericPartOfInput << "as number:" << ex.what();
   } catch(std::out_of_range const & ex) {
      // This one is more likely user error!
      qWarning() << Q_FUNC_INFO << "Out of range parsing" << numericPartOfInput << "as number:" << ex.what();
   }

   return std::pair<double, QString>{quantity, unitName};
}

bool Measurement::Unit::operator==(Unit const & other) const {
   // Since we're not intending to create multiple instances of any given UnitSystem, it should be enough to check
   // the addresses are equal, but, as belt-and-braces, we'll check the names & physical quantities are equal as a
   // fall-back.
   return (this == &other ||
           (this->name == other.name && this->getPhysicalQuantity() == other.getPhysicalQuantity()));
}

QString Measurement::Unit::getOutputForStream() const {
   QString output;
   QTextStream outputAsStream{&output};
   outputAsStream <<
      this->name << " (" << this->getPhysicalQuantity() << ", " << this->getUnitSystem().uniqueName << ")";
   return output;
}


Measurement::Unit const & Measurement::Unit::getCanonical() const {
   return Measurement::Unit::getCanonicalUnit(this->getPhysicalQuantity());
}

bool Measurement::Unit::isCanonical() const {
   Q_ASSERT(this->pimpl->m_isCanonical == (&this->getCanonical() == this));
   return this->pimpl->m_isCanonical;
}

Measurement::Amount Measurement::Unit::toCanonical(double amt) const {
   double const convertedQuantity{
      this->pimpl->m_multiplierToCanonical ? amt * (*this->pimpl->m_multiplierToCanonical) :
                                             this->pimpl->m_convertToCanonical(amt)
   };
   return Measurement::Amount{convertedQuantity, this->getCanonical()};
}

double Measurement::Unit::fromCanonical(double amt) const {
   if (this->pimpl->m_multiplierToCanonical) {
      return amt / (*this->pimpl->m_multiplierToCanonical);
   }
   return this->pimpl->m_convertFromCanonical(amt);
}

Measurement::PhysicalQuantity Measurement::Unit::getPhysicalQuantity() const {
   // The PhysicalQuantity for this Unit is already stored in its UnitSystem, so we don't store it separately here
   return this->pimpl->m_unitSystem.getPhysicalQuantity();
}

Measurement::UnitSystem const & Measurement::Unit::getUnitSystem() const {
   return this->pimpl->m_unitSystem;
}

double Measurement::Unit::boundary() const {
   return this->pimpl->m_boundaryValue;
}

Measurement::Unit const & Measurement::Unit::getCanonicalUnit(Measurement::PhysicalQuantity const physicalQuantity) {
   // Need this before we reference unitNameLookup or physicalQuantityToCanonicalUnit
   std::call_once(initFlag_Lookups, &Measurement::Unit::initialiseLookups);

   // It's a coding error if there is no canonical unit for a real physical quantity (ie not Mixed).  (And of course
   // there should be no Unit or UnitSystem for Mixed.)
   Q_ASSERT(physicalQuantityToCanonicalUnit.contains(physicalQuantity));
   auto canonical = physicalQuantityToCanonicalUnit.value(physicalQuantity);
   return *canonical;
}

QString Measurement::Unit::convertWithoutContext(QString const & qstr, QString const & toUnitName) {

   qDebug() << Q_FUNC_INFO << "Trying to convert" << qstr << "to" << toUnitName;
   double const fromQuantity = Measurement::Unit::splitAmountString(qstr).first;

   QString const fromUnitName =  Measurement::Unit::splitAmountString(qstr).second;
   auto const fromUnits = getUnitsOnlyByName(fromUnitName);
   auto const toUnits   = getUnitsOnlyByName(toUnitName);
   qDebug() <<
      Q_FUNC_INFO << "Found" << fromUnits.length() << "matches for" << fromUnitName << "and" << toUnits.length() <<
      "matches for" << toUnitName;

   if (fromUnits.length() > 0 && toUnits.length() > 0) {
      //
      // We found at least one match for both "from" and "to" unit names.  We need to search amongst these to find one
      // where both units relate to the same physical quantity.  If we find multiple matches, we see if the global
      // settings can help us disambiguate.  (This is mostly for the case of Imperial vs US Customary units, where many
      // of the units, such as pints and pounds, have the same names in both unit systems.  If the user's global
      // preferences for a unit system are set to Imperial or US Customary then we can use that to disambiguate between,
      // say, Imperial pints and US Customary pints for a field where the user has entered "pt".
      //
      Unit const * fromUnitBestMatch = nullptr;
      Unit const *   toUnitBestMatch = nullptr;
      for (auto const fromUnit : fromUnits) {
         for (auto const toUnit : toUnits) {
            if (fromUnit->getPhysicalQuantity() == toUnit->getPhysicalQuantity()) {
               Measurement::UnitSystem const & displayUnitSystem =
                  Measurement::getDisplayUnitSystem(fromUnit->getPhysicalQuantity());
               //
               // If both the from and to units match the user's preferred unit system (eg from is "gal", to is "pt",
               // and display unit system is "US Customary"), then that's a "perfect match" and has to be the best
               // answer we could give, so stop here.
               //
               if (fromUnit->getUnitSystem() == toUnit->getUnitSystem() &&
                   fromUnit->getUnitSystem() == displayUnitSystem) {
                  return displayableConvert(*fromUnit, *toUnit, fromQuantity);
               }

               //
               // Otherwise, if either the from or the to units match the user's preferred unit system (eg from is
               // "gal", to is "L", and display unit system is "US Customary", then that's a good candidate answer,
               // which should override any previous match (other than a perfect one which we would have already
               // returned).  We don't return yet, because, conceivably, we might find a "perfect match" in the rest of
               // the search.
               //
               if (fromUnit->getUnitSystem() == displayUnitSystem ||
                     toUnit->getUnitSystem() == displayUnitSystem) {
                  fromUnitBestMatch = fromUnit;
                    toUnitBestMatch =   toUnit;
                  continue;
               }

               //
               // If neither of the above conditions are true (eg from is "gal", to is "pt", and display unit system is
               // "Metric"), then this is a potential match that's worth using if we don't already have one.
               //
               if (!fromUnit && !toUnit) {
                  fromUnitBestMatch = fromUnit;
                    toUnitBestMatch =   toUnit;
               }
            }
         }
      }

      if (fromUnitBestMatch && toUnitBestMatch) {
         return displayableConvert(*fromUnitBestMatch, *toUnitBestMatch, fromQuantity);
      }
   }

   // If we didn't recognise from or to units, or we couldn't find a pair for the same PhysicalQuantity, the we return
   // the original amount with a question mark.
   return QString("%1 ?").arg(Measurement::displayQuantity(fromQuantity, 3));
}

Measurement::Unit const * Measurement::Unit::getUnit(QString const & name,
                                                     Measurement::PhysicalQuantity const & physicalQuantity,
                                                     bool const caseInensitiveMatching) {
   auto matches = getUnitsByNameAndPhysicalQuantity(name, physicalQuantity, caseInensitiveMatching);

   auto const numMatches = matches.length();
   if (0 == numMatches) {
      return nullptr;
   }

   // Under most circumstances, there is a one-to-one relationship between unit string and Unit. C will only map to
   // Measurement::Unit::Celsius, for example. If there's only one match, just return it.
   if (1 == numMatches) {
      auto unitToReturn = matches.at(0);
      if (unitToReturn->getPhysicalQuantity() != physicalQuantity) {
         qWarning() <<
            Q_FUNC_INFO << "Unit" << name << "matches a unit of type" << unitToReturn->getPhysicalQuantity() <<
            "but caller specified" << physicalQuantity;
         return nullptr;
      }
      return unitToReturn;
   }

   // That solved something like 99% of the use cases. Now we have to handle those pesky volumes.
   // Loop through the found Units, like Measurement::Unit::us_quart and
   // Measurement::Unit::imperial_quart, and try to find one that matches the global default.
   Measurement::Unit const * defUnit = nullptr;
   for (auto const unit : matches) {
      auto const & displayUnitSystem = Measurement::getDisplayUnitSystem(unit->getPhysicalQuantity());
      qDebug() <<
         Q_FUNC_INFO << "Look at" << *unit << "from" << unit->getUnitSystem() << "(Display Unit System for" <<
         unit->getPhysicalQuantity() << "is" << displayUnitSystem << ")";
      if (unit->getPhysicalQuantity() != physicalQuantity) {
         // If the caller knows the amount is, say, a Volume, don't bother trying to match against units for any other
         // physical quantity.
         qDebug() << Q_FUNC_INFO << "Ignoring match in" << unit->getPhysicalQuantity() << "as not" << physicalQuantity;
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

Measurement::Unit const * Measurement::Unit::getUnit(QString const & name,
                                                     Measurement::UnitSystem const & unitSystem,
                                                     bool const caseInensitiveMatching) {
   auto matches = getUnitsByNameAndPhysicalQuantity(name, unitSystem.getPhysicalQuantity(), caseInensitiveMatching);

   //
   // At this point, matches is a list of all units matching the supplied name and the PhysicalQuantity of the supplied
   // UnitSystem.  If we have more than one match, then we prefer the first one we find (if any) in the supplied
   // UnitSystem, otherwise, first in the list will have to do.
   //

   auto const numMatches = matches.length();
   if (0 == numMatches) {
      return nullptr;
   }

   if (1 == numMatches) {
      return matches.at(0);
   }

   for (auto const match : matches) {
      if (match->getUnitSystem() == unitSystem) {
         return match;
      }
   }

   return matches.at(0);
}

// This is where we actually define all the different units and how to convert them to/from their canonical equivalents
// Previously this was done with a huge number of subclasses, but lambdas mean that's no longer necessary
// Note that we always need to define the canonical Unit for a given PhysicalQuantity before any others
//
namespace Measurement::Units {
   // :NOTE FOR TRANSLATORS: The abbreviated name of each unit (eg "kg" for kilograms, "g" for grams, etc) must be
   //                        unique for that type of unit.  Eg you cannot have two units of weight with the same
   //                        abbreviated name.  Ideally, this should also be true on a case insensitive basis, eg it is
   //                        undesirable for "foo" and "Foo" to be the abbreviated names of two different units of
   //                        the same type.
   //

   // === Mass ===
   // See comment in measurement/UnitSystem.cpp for why we have separate entities for US Customary pounds/ounces and Imperials ones, even though they are, in fact, the same
   Unit const kilograms      {Measurement::UnitSystems::mass_Metric     , QObject::tr("kg")};
   Unit const grams          {Measurement::UnitSystems::mass_Metric     , QObject::tr("g" ), 1.0/1000.0   , &kilograms};
   Unit const milligrams     {Measurement::UnitSystems::mass_Metric     , QObject::tr("mg"), 1.0/1000000.0, &kilograms};
   Unit const pounds         {Measurement::UnitSystems::mass_UsCustomary, QObject::tr("lb"), 0.45359237   , &kilograms};
   Unit const ounces         {Measurement::UnitSystems::mass_UsCustomary, QObject::tr("oz"), 0.0283495231 , &kilograms};
   Unit const imperial_pounds{Measurement::UnitSystems::mass_Imperial   , QObject::tr("lb"), 0.45359237   , &kilograms};
   Unit const imperial_ounces{Measurement::UnitSystems::mass_Imperial   , QObject::tr("oz"), 0.0283495231 , &kilograms};

   // === Volume ===
   // Where possible, the multipliers for going to and from litres come from www.conversion-metric.org as it seems to offer the most decimal places on its conversion tables
   Unit const liters              {Measurement::UnitSystems::volume_Metric     , QObject::tr("L"   )};
   Unit const milliliters         {Measurement::UnitSystems::volume_Metric     , QObject::tr("mL"  ), 1.0/1000.0          , &liters};
   Unit const us_barrels          {Measurement::UnitSystems::volume_UsCustomary, QObject::tr("bbl" ), 117.34777           , &liters};
   Unit const us_gallons          {Measurement::UnitSystems::volume_UsCustomary, QObject::tr("gal" ),   3.7854117840007   , &liters};
   Unit const us_quarts           {Measurement::UnitSystems::volume_UsCustomary, QObject::tr("qt"  ),   0.94635294599999  , &liters};
   Unit const us_pints            {Measurement::UnitSystems::volume_UsCustomary, QObject::tr("pt"  ),   0.473176473       , &liters};
   Unit const us_cups             {Measurement::UnitSystems::volume_UsCustomary, QObject::tr("cup" ),   0.23658823648491  , &liters, 0.25};
   Unit const us_fluidOunces      {Measurement::UnitSystems::volume_UsCustomary, QObject::tr("floz"),   0.029573529564112 , &liters};
   Unit const us_tablespoons      {Measurement::UnitSystems::volume_UsCustomary, QObject::tr("tbsp"),   0.014786764782056 , &liters};
   Unit const us_teaspoons        {Measurement::UnitSystems::volume_UsCustomary, QObject::tr("tsp" ),   0.0049289215940186, &liters};
   Unit const imperial_barrels    {Measurement::UnitSystems::volume_Imperial   , QObject::tr("bbl" ), 163.659             , &liters};
   Unit const imperial_gallons    {Measurement::UnitSystems::volume_Imperial   , QObject::tr("gal" ),   4.5460899999997   , &liters};
   Unit const imperial_quarts     {Measurement::UnitSystems::volume_Imperial   , QObject::tr("qt"  ),   1.1365225         , &liters};
   Unit const imperial_pints      {Measurement::UnitSystems::volume_Imperial   , QObject::tr("pt"  ),   0.56826125        , &liters};
   Unit const imperial_cups       {Measurement::UnitSystems::volume_Imperial   , QObject::tr("cup" ),   0.284130625       , &liters, 0.25};
   Unit const imperial_fluidOunces{Measurement::UnitSystems::volume_Imperial   , QObject::tr("floz"),   0.028413075003383 , &liters};
   Unit const imperial_tablespoons{Measurement::UnitSystems::volume_Imperial   , QObject::tr("tbsp"),   0.0177581714      , &liters};
   Unit const imperial_teaspoons  {Measurement::UnitSystems::volume_Imperial   , QObject::tr("tsp" ),   0.00591939047     , &liters};

   // === Length ===
   // I suppose we could use the other official abbreviations for feet and inches -- either the real ones (see
   // en.wikipedia.org/wiki/Prime_(symbol)) that no-one can type (′ and ″) or what people would actually type
   // (' and ") -- but I fear it's a bit more hassle than it's worth.
   Unit const centimeters{Measurement::UnitSystems::length_Metric     , QObject::tr("cm")};
   Unit const millimeters{Measurement::UnitSystems::length_Metric     , QObject::tr("mm"),   0.1, &centimeters};
   Unit const meters     {Measurement::UnitSystems::length_Metric     , QObject::tr("m" ), 100.0, &centimeters};
   Unit const inches     {Measurement::UnitSystems::length_UsCustomary, QObject::tr("in"),  2.54, &centimeters};
   Unit const feet       {Measurement::UnitSystems::length_UsCustomary, QObject::tr("ft"), 30.48, &centimeters};

   // === Count ===
   // The choice of abbreviation here is a bit of a compromise, in English at least, because it's a bit unnatural to say
   // things such as "Cinnamon sticks: 2.5 count" or "Cinnamon sticks: 2.5 number of" instead of "2.5 cinnamon sticks"
   // or "2.5 × cinnamon sticks".  "Cinnamon sticks: 2.5" would be more natural, but I'm reluctant to have no
   // abbreviation, as there are, arguably, circumstances where it could lead to ambiguity or confusion.  At very least
   // if we are showing an abbreviation for "number of" then we are showing that the units haven't been forgotten.
   Unit const numberOf{Measurement::UnitSystems::count_NumberOf, QObject::tr("(№)")};

   // === Temperature ===
   Unit const celsius   {Measurement::UnitSystems::temperature_MetricIsCelsius        , QObject::tr("C")};
   Unit const fahrenheit{Measurement::UnitSystems::temperature_UsCustomaryIsFahrenheit, QObject::tr("F"), [](double x){return (x - 32.0) * 5.0/9.0;},
                                                                                                          [](double y){return (y * 9.0/5.0) + 32.0;}, &celsius};

   // === Time ===
   // Added weeks because BeerJSON has it
   // TBD I've put days and weeks in plural here, because in practice it looks jarring to have them in singular, but
   //     maybe we should decide on abbreviations for them.
   Unit const minutes{Measurement::UnitSystems::time_CoordinatedUniversalTime, QObject::tr("min"  )};
   Unit const weeks  {Measurement::UnitSystems::time_CoordinatedUniversalTime, QObject::tr("weeks"), (7.0*24.0*60.0), &minutes};
   Unit const days   {Measurement::UnitSystems::time_CoordinatedUniversalTime, QObject::tr("days" ), (24.0*60.0)    , &minutes};
   Unit const hours  {Measurement::UnitSystems::time_CoordinatedUniversalTime, QObject::tr("hr"   ), 60.0           , &minutes,  2.0};
   Unit const seconds{Measurement::UnitSystems::time_CoordinatedUniversalTime, QObject::tr("s"    ), 1.0/60.0       , &minutes, 90.0};

   // === Color ===
   // Not sure how many people use Lovibond scale these days, but BeerJSON supports it, so we need to be able to read
   // it.  https://en.wikipedia.org/wiki/Beer_measurement#Colour= says "The Standard Reference Method (SRM) ... [gives]
   // results approximately equal to the °L."
   Unit const srm     {Measurement::UnitSystems::color_StandardReferenceMethod  , QObject::tr("srm"     )};
   Unit const ebc     {Measurement::UnitSystems::color_EuropeanBreweryConvention, QObject::tr("ebc"     ), 12.7/25.0, &srm};
   Unit const lovibond{Measurement::UnitSystems::color_Lovibond                 , QObject::tr("lovibond"), 1.0      , &srm};

   // == Density ===
   // Brix isn't much used in beer brewing, but BeerJSON supports it, so we have it here.
   // Per https://en.wikipedia.org/wiki/Beer_measurement, Plato and Brix are "essentially ... the same ([both based on
   // mass fraction of sucrose) [and only] differ in their conversion from weight percentage to specific gravity in the
   // fifth and sixth decimal places"
   Unit const specificGravity{Measurement::UnitSystems::density_SpecificGravity, QObject::tr("sg"),};
   Unit const plato          {Measurement::UnitSystems::density_Plato,
                              QObject::tr("P"),
                                     [](double x){return x == 0.0 ? 0.0 : Algorithms::PlatoToSG_20C20C(x);},
                                     [](double y){return y == 0.0 ? 0.0 : Algorithms::SG_20C20C_toPlato(y);},
                                     &specificGravity};
   Unit const brix           {Measurement::UnitSystems::density_Brix,
                              QObject::tr("brix"),
                              [](double x){return x == 0.0 ? 0.0 : Algorithms::BrixToSgAt20C(x);},
                              [](double y){return y == 0.0 ? 0.0 : Algorithms::SgAt20CToBrix(y);},
                              &specificGravity};

   // == Diastatic power ==
   Unit const lintner{Measurement::UnitSystems::diastaticPower_Lintner        , QObject::tr("L" )};
   Unit const wk     {Measurement::UnitSystems::diastaticPower_WindischKolbach, QObject::tr("WK"), [](double x){return (x + 16.0) / 3.5;},
                                                                                                   [](double y){return (3.5 * y) - 16.0;}, &lintner};

   // == Acidity ==
   Unit const pH{Measurement::UnitSystems::acidity_pH, QObject::tr("pH")};

   // == Bitterness ==
   Unit const ibu{Measurement::UnitSystems::bitterness_InternationalBitternessUnits, QObject::tr("IBU")};

   // == Carbonation ==
   // Per http://www.uigi.com/co2_conv.html, 1 cubic metre (aka 1000 litres) of CO2 at 1 atmosphere pressure and 0°C
   // temperature weighs 1.9772 kg, so 1 litre weighs 1.9772 g at this pressure and temperature.  Not clear however
   // whether we should use 0°C or 20°C or some other temperature for the conversion from volumes to grams per litre.
   // A brewing-specific source, https://byo.com/article/master-the-action-carbonation/, gives the conversion factor as
   // 1.96, so we use that.
   Unit const carbonationVolumes      {Measurement::UnitSystems::carbonation_Volumes      , QObject::tr("vol" )};
   Unit const carbonationGramsPerLiter{Measurement::UnitSystems::carbonation_MassPerVolume, QObject::tr("mg/L"), 1.0/1.96, &carbonationVolumes};

   // == Mass Fraction & Mass Concentration ==
   Unit const partsPerMillionMass {Measurement::UnitSystems::massFractionOrConc_Brewing, QObject::tr("ppm" )};
   Unit const partsPerBillionMass {Measurement::UnitSystems::massFractionOrConc_Brewing, QObject::tr("ppb" ), 1000.0, &partsPerMillionMass};
   Unit const milligramsPerLiter  {Measurement::UnitSystems::massFractionOrConc_Brewing, QObject::tr("mg/L"),    1.0, &partsPerMillionMass};

   // == Viscosity ==
   // Yes, 1 centipoise = 1 millipascal-second.  See comment in measurement/Unit.h for more info
   Unit const centipoise       {Measurement::UnitSystems::viscosity_Metric         , QObject::tr("cP"   )};
   Unit const millipascalSecond{Measurement::UnitSystems::viscosity_MetricAlternate, QObject::tr("mPa-s"), 1.0,  &centipoise};

   // == Specific heat capacity ==
   // See comment in measurement/Unit.h for why the non-metric units are the canonical ones
   Unit const caloriesPerCelsiusPerGram{Measurement::UnitSystems::specificHeatCapacity_Calories, QObject::tr("c/g·C"   )};
   Unit const joulesPerKelvinPerKg     {Measurement::UnitSystems::specificHeatCapacity_Joules  , QObject::tr("J/kg·K"  ), 1.0/4184.0, &caloriesPerCelsiusPerGram};
   Unit const btuPerFahrenheitPerPound {Measurement::UnitSystems::specificHeatCapacity_Btus    , QObject::tr("BTU/lb·F"), 1.0       , &caloriesPerCelsiusPerGram};

   // == Specific Volume ==
   Unit const litresPerKilogram     {Measurement::UnitSystems::specificVolume_Metric     , QObject::tr("L/kg"   )};
   Unit const litresPerGram         {Measurement::UnitSystems::specificVolume_Metric     , QObject::tr("L/g"    ), 1000.0             , &litresPerKilogram};
   Unit const cubicMetersPerKilogram{Measurement::UnitSystems::specificVolume_Metric     , QObject::tr("m^3/kg" ), 1000.0             , &litresPerKilogram};
   Unit const us_fluidOuncesPerOunce{Measurement::UnitSystems::specificVolume_UsCustomary, QObject::tr("floz/oz"),   66.7632356142    , &litresPerKilogram};
   Unit const us_gallonsPerPound    {Measurement::UnitSystems::specificVolume_UsCustomary, QObject::tr("gal/lb" ),    8.34540445177617, &litresPerKilogram};
   Unit const us_quartsPerPound     {Measurement::UnitSystems::specificVolume_UsCustomary, QObject::tr("qt/lb"  ),    2.08635111294   , &litresPerKilogram};
   Unit const us_gallonsPerOunce    {Measurement::UnitSystems::specificVolume_UsCustomary, QObject::tr("gal/oz" ),    0.521587778236  , &litresPerKilogram};
   Unit const cubicFeetPerPound     {Measurement::UnitSystems::specificVolume_UsCustomary, QObject::tr("ft^3/lb"),   62.4279605755126 , &litresPerKilogram};

   ObjectAddressStringMapping<Unit> const unitStringMapping {
      {
         // === Mass ===
         {kilograms                     , "kilograms"                    },
         {grams                         , "grams"                        },
         {milligrams                    , "milligrams"                   },
         {pounds                        , "pounds"                       },
         {ounces                        , "ounces"                       },
         {imperial_pounds               , "imperial_pounds"              },
         {imperial_ounces               , "imperial_ounces"              },
         // === Volume ===
         {liters                        , "liters"                       },
         {milliliters                   , "milliliters"                  },
         {us_barrels                    , "us_barrels"                   },
         {us_gallons                    , "us_gallons"                   },
         {us_quarts                     , "us_quarts"                    },
         {us_pints                      , "us_pints"                     },
         {us_cups                       , "us_cups"                      },
         {us_fluidOunces                , "us_fluid_ounces"              },
         {us_tablespoons                , "us_tablespoons"               },
         {us_teaspoons                  , "us_teaspoons"                 },
         {imperial_barrels              , "imperial_barrels"             },
         {imperial_gallons              , "imperial_gallons"             },
         {imperial_quarts               , "imperial_quarts"              },
         {imperial_pints                , "imperial_pints"               },
         {imperial_cups                 , "imperial_cups"                },
         {imperial_fluidOunces          , "imperial_fluid_ounces"        },
         {imperial_tablespoons          , "imperial_tablespoons"         },
         {imperial_teaspoons            , "imperial_teaspoons"           },
         // === Length ===
         {centimeters                   , "centimeters"                  },
         {millimeters                   , "millimeters"                  },
         {meters                        , "meters"                       },
         {inches                        , "inches"                       },
         {feet                          , "feet"                         },
         // === Count ===
         {numberOf                      , "number_of"                    },
         // === Temperature ===
         {celsius                       , "celsius"                      },
         {fahrenheit                    , "fahrenheit"                   },
         // === Time ===
         {minutes                       , "minutes"                      },
         {weeks                         , "weeks"                        },
         {days                          , "days"                         },
         {hours                         , "hours"                        },
         {seconds                       , "seconds"                      },
         // === Color ===
         {srm                           , "srm"                          },
         {ebc                           , "ebc"                          },
         {lovibond                      , "lovibond"                     },
         // == Density ===
         {specificGravity               , "specific_gravity"             },
         {plato                         , "plato"                        },
         {brix                          , "brix"                         },
         // == Diastatic power ==
         {lintner                       , "lintner"                      },
         {wk                            , "wk"                           },
         // == Acidity ==
         {pH                            , "ph"                           },
         // == Bitterness ==
         {ibu                           , "ibu"                          },
         // == Carbonation ==
         {carbonationVolumes            , "carbonation_volumes"          },
         {carbonationGramsPerLiter      , "carbonation_grams_per_liter"  },
         // == Mass Fraction & Mass Concentration ==
         {partsPerMillionMass           , "parts_per_million_mass"       },
         {partsPerBillionMass           , "parts_per_billion_mass"       },
         {milligramsPerLiter            , "milligrams_per_liter"         },
         // == Viscosity ==
         {centipoise                    , "centipoise"                   },
         {millipascalSecond             , "millipascal_second"           },
         // == Specific Heat Capacity ==
         {caloriesPerCelsiusPerGram     , "calories_per_celsius_per_gram"},
         {joulesPerKelvinPerKg          , "joules_per_kelvin_per_kg"     },
         {btuPerFahrenheitPerPound      , "btu_per_fahrenheit_per_pound" },
         // == Specific Volume ==
         {litresPerKilogram             , "litres_per_kilogram"          },
         {litresPerGram                 , "litres_per_gram"              },
         {cubicMetersPerKilogram        , "cubic_meters_per_kilogram"    },
         {us_quartsPerPound             , "us_quarts_per_pound"          },
         {us_gallonsPerPound            , "us_gallons_per_pound"         },
         {us_gallonsPerOunce            , "us_gallons_per_ounce"         },
         {us_fluidOuncesPerOunce        , "us_fluid_ounces_per_ounce"    },
         {cubicFeetPerPound             , "cubic_feet_per_pound"         },
      }
   };

}
