/*
 * measurement/Amount.cpp is part of Brewtarget, and is copyright the following
 * authors 2022-2023:
 * - Matt Young <mfsy@yahoo.com>
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
#include "measurement/Amount.h"

#include <QDebug>
#include <QTextStream>

#include "measurement/Unit.h"

namespace Measurement {

   Amount::Amount(double const quantity, Unit const & unit) : m_quantity{quantity}, m_unit{&unit} {
      return;
   }

   //! Copy constructor
   Amount::Amount(Amount const & other) = default;

   //! Assignment operator
   Amount & Amount::operator=(Amount const & other) = default;

   //! Move constructor.
   Amount::Amount(Amount && other) = default;

   //! Move assignment.
   Amount & Amount::operator=(Amount && other) = default;

   double       Amount::quantity() const { return this->m_quantity; }
   Unit const * Amount::unit()     const { return this->m_unit    ; }

   void Amount::setQuantity(double const   val) { this->m_quantity =  val; return; }
   void Amount::setUnit    (Unit   const & val) { this->m_unit     = &val; return; }

}

bool operator<(Measurement::Amount const & lhs, Measurement::Amount const & rhs) {
   // Amounts in the same units are trivial to compare
   if (lhs.unit() == rhs.unit()) {
      return lhs.quantity() < rhs.quantity();
   }

   // It's a coding error if we try to compare two things that aren't a measure of the same physical quantity (because
   // it's meaningless to compare a temperature to a mass, etc
   Q_ASSERT(lhs.unit()->getPhysicalQuantity() == rhs.unit()->getPhysicalQuantity());

   return lhs.unit()->toCanonical(lhs.quantity()).quantity() < rhs.unit()->toCanonical(lhs.quantity()).quantity();
}

bool operator==(Measurement::Amount const & lhs, Measurement::Amount const & rhs) {
   // Amounts in the same units are trivial to compare
   if (lhs.unit() == rhs.unit()) {
      return lhs.quantity() == rhs.quantity();
   }

   // It's a coding error if we try to compare two things that aren't a measure of the same physical quantity (because
   // it's meaningless to compare a temperature to a mass, etc
   Q_ASSERT(lhs.unit()->getPhysicalQuantity() == rhs.unit()->getPhysicalQuantity());

   return lhs.unit()->toCanonical(lhs.quantity()).quantity() == rhs.unit()->toCanonical(lhs.quantity()).quantity();
}

template<class S>
S & operator<<(S & stream, Measurement::Amount const amount) {
   stream << amount.quantity() << " " << amount.unit()->name;
   return stream;
}

//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// means, amongst other things, that we can reference member functions of Measurement::Unit.)
//
template QDebug      & operator<<(QDebug      & stream, Measurement::Amount const amount);
template QTextStream & operator<<(QTextStream & stream, Measurement::Amount const amount);
