/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/Amount.cpp is part of Brewtarget, and is copyright the following authors 2022-2025:
 *   • Matt Young <mfsy@yahoo.com>
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
#include "measurement/Amount.h"

#include <QDebug>
#include <QTextStream>

#include "measurement/Unit.h"
#include "utils/FuzzyCompare.h"

namespace Measurement {

   Amount::Amount(double const quantity, Unit const & unit) : quantity{quantity}, unit{&unit} {
      return;
   }

   Amount::Amount(PhysicalQuantity const physicalQuantity, double const quantity) :
      quantity{quantity},
      unit{&Measurement::Unit::getCanonicalUnit(physicalQuantity)} {
      return;
   }

   Amount::Amount() : quantity{-1.0}, unit{nullptr} {
      return;
   }

   //! Copy constructor
   Amount::Amount(Amount const & other) :
      quantity{other.quantity},
      unit    {other.unit    } {
      return;
   }

   //! Copy assignment operator
   Amount & Amount::operator=(Amount const & other) {
      *this = Amount{other};
      return *this;
   }

   //! Move constructor
   Amount::Amount(Amount && other) noexcept :
      quantity{other.quantity},
      unit    {other.unit    } {
      return;
   }

   //! Move assignment operator
   Amount & Amount::operator=(Amount && other) noexcept {
      std::swap(this->quantity, other.quantity);
      std::swap(this->unit    , other.unit    );
      return *this;
   }

   bool Amount::operator==(Amount const & other) const {
      // Most of the time, everything will be in canonical units, but if two amounts for the same physical quantity are
      // in different units, they should still be comparable.
      auto const lhsCanonical {this->unit->toCanonical(this->quantity)};
      auto const rhsCanonical {other.unit->toCanonical(other.quantity)};
      // Unit classes are singletons so, once everything is in canonical units, it's OK to compare just the pointers
      return lhsCanonical.unit == rhsCanonical.unit && Utils::FuzzyCompare(lhsCanonical.quantity, rhsCanonical.quantity);
   }

   bool Amount::operator!=(Amount const & other) const {
      // Don't reinvent the wheel '!=' should just be the opposite of '=='
      return !(*this == other);
   }

   std::partial_ordering Amount::operator<=>(Amount const & other) const {
      // Comments above in operator== also apply here
      auto const lhsCanonical {this->unit->toCanonical(this->quantity)};
      auto const rhsCanonical {other.unit->toCanonical(other.quantity)};
      if (lhsCanonical.unit != rhsCanonical.unit) {
         return std::partial_ordering::unordered;
      }
      if (Utils::FuzzyCompare(lhsCanonical.quantity, rhsCanonical.quantity)) {
         return std::partial_ordering::equivalent;
      }
      // Now we did the fuzzy compare, anything that's not a fuzzy match is safe to compare "as normal"
      return (lhsCanonical.quantity < rhsCanonical.quantity) ? std::partial_ordering::less :
                                                               std::partial_ordering::greater;
   };

   bool Amount::isValid() const {
      return (this->unit && this->quantity >= 0.0);
   }

   Amount Amount::toCanonical() const {
      return this->unit->toCanonical(this->quantity);
   }

   Amount Amount::operator+(Amount const & other) const {
      PhysicalQuantity const physicalQuantity = this->unit->getPhysicalQuantity();
      if (physicalQuantity != other.unit->getPhysicalQuantity()) {
         qWarning() << Q_FUNC_INFO << "Meaninless to add" << *this << "to" << other << "as physical quantities differ!";
      }
      return Amount{physicalQuantity, this->toCanonical().quantity + other.toCanonical().quantity};
   }

}

template<class S>
S & operator<<(S & stream, Measurement::Amount const & amount) {
   // QDebug puts extra spaces around each thing you output but QTextStream does not (I think), so, to get the right gap
   // between the quantity and the unit, we need to be a bit heavy-handed.
   stream << QString("%1 %2").arg(amount.quantity).arg(amount.unit->name);
   return stream;
}

//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// means, amongst other things, that we can reference member functions of Measurement::Unit.)
//
template QDebug      & operator<<(QDebug      & stream, Measurement::Amount const & amount);
template QTextStream & operator<<(QTextStream & stream, Measurement::Amount const & amount);
