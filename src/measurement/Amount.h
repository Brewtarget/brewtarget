/*
 * measurement/Amount.h is part of Brewtarget, and is copyright the following
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
#ifndef MEASUREMENT_AMOUNT_H
#define MEASUREMENT_AMOUNT_H
#pragma once

// Note that we cannot #include "measurement/Unit.h" because that header file already includes this one

namespace Measurement {
   class Unit;

   /**
    * \brief When passing amounts to and from functions, it's useful to combine the quantity and the units, eg
    *        5.5 US gallons is \b quantity 5.5 and \b unit \c Measurement::Units::us_gallons.  Even where we think we
    *        know what units we're getting back from a function (eg \c qstringToSI), it helps reduce bugs to have
    *        quantity and units together in a single struct.
    */
   class Amount {
   public:
      //! Regular constructor
      Amount(double quantity, Unit const & unit);

      //! Copy constructor
      Amount(Amount const & other);

      //! Assignment operator
      Amount & operator=(Amount const & other);

      //! Move constructor.
      Amount(Amount && other);

      //! Move assignment.
      Amount & operator=(Amount && other);

      double       quantity() const;
      Unit const * unit    () const;

   private:
      void setQuantity(double const   val);
      void setUnit    (Unit   const & val);

   protected:
      double       m_quantity;
      Unit const * m_unit;
   };
}

/**
 * \brief A version of \c Measurement::Amount that is "constrained" to be either a
 *        \c Measurement::PhysicalQuantity::Mass or a \c Measurement::PhysicalQuantity::Volume.
 *        The constraint is not bullet-proof but you will get an assert (on a debug build) if you try to construct /
 *        assign / move it with a \c Measurement::Unit of the wrong \c Measurement::PhysicalQuantity
 */
class MassOrVolumeAmt : public Measurement::Amount {
public:
   /**
    * \brief Default constructor is needed so we can store in \c QVariant which is needed to use this type in the Qt
    *        Properties system.  The default-constructed type will be an invalid amount (eg a negative mass).
    */
   MassOrVolumeAmt();

   //! Regular constructor
   MassOrVolumeAmt(double quantity, Measurement::Unit const & unit);

   //! Copy constructor
   MassOrVolumeAmt(Measurement::Amount const & other);

   //! Assignment operator
   MassOrVolumeAmt & operator=(Measurement::Amount const & other);

   //! Move constructor.
   MassOrVolumeAmt(Measurement::Amount && other);

   //! Move assignment.
   MassOrVolumeAmt & operator=(Measurement::Amount && other);

   bool isMass() const;
private:
   bool wasConstructAssignOrMoveOK();
};

bool operator<(Measurement::Amount const & lhs, Measurement::Amount const & rhs);
bool operator==(Measurement::Amount const & lhs, Measurement::Amount const & rhs);

/**
 * \brief Convenience function to allow output of \c Measurement::Amount to \c QDebug or \c QTextStream stream etc
 *
 *        Implementation is in the .cpp file because we need to #include "measurement/Unit.h", which we can't do in this
 *        header, per comment above.
 */
template<class S> S & operator<<(S & stream, Measurement::Amount const amount);

#endif
