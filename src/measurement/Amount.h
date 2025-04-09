/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/Amount.h is part of Brewtarget, and is copyright the following authors 2022-2025:
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
#ifndef MEASUREMENT_AMOUNT_H
#define MEASUREMENT_AMOUNT_H
#pragma once

#include <compare>

#include "measurement/PhysicalQuantity.h"

// Note that we cannot #include "measurement/Unit.h" because that header file already includes this one

namespace Measurement {
   class Unit;

   /**
    * \brief When passing amounts to and from functions, it's useful to combine the quantity and the units, eg
    *        5.5 US gallons is \b quantity 5.5 and \b unit \c Measurement::Units::us_gallons.  Even where we think we
    *        know what units we're getting back from a function (eg \c qstringToSI), it helps reduce bugs to have
    *        quantity and units together in a single struct.
    *
    *        NOTE: Although, in everyday conversation, "quantity" and "amount" are often used interchangeably, in this
    *              code base it is useful to us to make the distinction between:
    *
    *                 \b quantity = a numerical value without units; the units not being present because either:
    *                                  - there are no units, or
    *                                  - the units are stored in another field, or
    *                                  - the units are "fixed" for a certain field (eg it's always a percentage or it's
    *                                    always kilograms)
    *
    *                 \b amount = a quantity plus units -- ie an instance of \c Amount
    *
    *              Maybe one day we can come up with some better names.  Eg, I don't like that "quantity" and "physical
    *              quantity" are somewhat unrelated concepts.
    */
   struct Amount {
      double       quantity;
      Unit const * unit;

      //! Regular constructor
      Amount(double const quantity, Unit const & unit);

      /**
       * Constructs quantity amount in the default units for the given \c PhysicalQuantity
       *
       * NOTE It is the caller's responsibility to ensure that the defaulted or supplied quantity is valid.  (Eg for
       * mass or volume, it is safe to assume that 0.0 means the same for all units, but, for temperature, this is not
       * the case.)
       */
      Amount(PhysicalQuantity const physicalQuantity, double const quantity = 0.0);

      /**
       * Default constructor is required if we are passing things through the Qt Property system.
       * NOTE that this will construct an invalid amount
       */
      Amount();

      //! Copy constructor
      Amount(Amount const & other);

      //! Copy assignment operator
      Amount & operator=(Amount const & other);

      //! Move constructor
      Amount(Amount && other) noexcept;

      //! Move assignment operator
      Amount & operator=(Amount && other) noexcept;

      //! Equality operator does fuzzy comparison
      bool operator==(Amount const & other) const;

      //! Inequality operator implemented in terms of equality operator
      bool operator!=(Amount const & other) const;

      //! Three-way comparison (aka spaceship) operator
      std::partial_ordering operator<=>(Amount const & other) const;

      //! Checks for an uninitialised (or badly initialised) amount
      bool isValid() const;

      //! Convenience function for converting this Amount to canonical units (via \c Unit::toCanonical)
      Amount toCanonical() const;
   };

}

/**
 * \brief Convenience function to allow output of \c Measurement::Amount to \c QDebug or \c QTextStream stream etc
 *
 *        Implementation is in the .cpp file because we need to #include "measurement/Unit.h", which we can't do in this
 *        header, per comment above.
 */
template<class S> S & operator<<(S & stream, Measurement::Amount const amount);

#endif
