/*======================================================================================================================
 * measurement/ConstrainedAmount.h is part of Brewken, and is copyright the following authors 2022-2023:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewken is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewken is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#ifndef MEASUREMENT_CONSTRAINEDAMOUNT_H
#define MEASUREMENT_CONSTRAINEDAMOUNT_H
#pragma once

#include <QDebug>

#include "measurement/Amount.h"
#include "measurement/Unit.h"

namespace Measurement {
   /**
    * \brief A version of \c Measurement::Amount that is "constrained" to be hold units of one of the two specified
    *        physical quantities.  The constraint is not bullet-proof but you will get an assert (on a debug build) if
    *        you try to construct / assign / move it with a \c Measurement::Unit of the wrong
    *        \c Measurement::PhysicalQuantity
    */
   template<PhysicalQuantity PQ1, PhysicalQuantity PQ2>
   class ConstrainedAmount : public Measurement::Amount {
   public:
      /**
       * \brief Default constructor is needed so we can store in \c QVariant which is needed to use this type in the Qt
       *        Properties system.  The default-constructed type will be, as far as possible, an invalid amount (a
       *        negative quantity of PQ1).
       */
      ConstrainedAmount() : Measurement::Amount{-999.999, Measurement::Unit::getCanonicalUnit(PQ1)} {
         return;
      }

      ~ConstrainedAmount() = default;

      //! Regular constructor
      ConstrainedAmount(double quantity, Measurement::Unit const & unit) : Measurement::Amount{quantity, unit} {
         checkConstructAssignOrMoveOK("construct");
         return;
      }

      //! Copy constructor
      ConstrainedAmount(Measurement::Amount const & other) : Measurement::Amount{other} {
         checkConstructAssignOrMoveOK("construct");
         return;
      }

      //! Assignment operator
      ConstrainedAmount & operator=(Measurement::Amount const & other) {
         Measurement::Amount::operator=(other);
         checkConstructAssignOrMoveOK("assign");
         return *this;
      }

      //! Move constructor.
      ConstrainedAmount(Measurement::Amount && other)  : Measurement::Amount{other} {
         checkConstructAssignOrMoveOK("construct");
         return;
      }

      //! Move assignment.
      ConstrainedAmount & operator=(Measurement::Amount && other) {
         Measurement::Amount::operator=(other);
         checkConstructAssignOrMoveOK("move assign");
         return *this;
      }

      /**
       * \brief Returns \c true if the unit is of type \c PQ1, \c false otherwise
       */
      bool isFirst() const {
         return this->Measurement::Amount::m_unit->getPhysicalQuantity() == PQ1;
      }

   private:
      /**
       * \brief Checks the object is valid after a construction, assignment or move.  If not, logs an error and asserts.
       */
      void checkConstructAssignOrMoveOK(char const * const operation) {
         auto const currentPhysicalQuantity = this->Measurement::Amount::m_unit->getPhysicalQuantity();
         if (currentPhysicalQuantity != PQ1 && currentPhysicalQuantity != PQ2) {
            qCritical() <<
               Q_FUNC_INFO << "Trying to" << operation << "ConstrainedAmount<" << PQ1 << "," << PQ2 << "> with " <<
               this->m_unit->name << "which is" << currentPhysicalQuantity;
            Q_ASSERT(false);
         }
         return;
      }
   };
}

using MassOrVolumeAmtBase = Measurement::ConstrainedAmount<Measurement::PhysicalQuantity::Mass,
                                                           Measurement::PhysicalQuantity::Volume>;
/**
 * \brief A version of \c Measurement::Amount that is "constrained" to be either a
 *        \c Measurement::PhysicalQuantity::Mass or a \c Measurement::PhysicalQuantity::Volume.
 */
class MassOrVolumeAmt : public MassOrVolumeAmtBase {
public:
   using MassOrVolumeAmtBase::MassOrVolumeAmtBase;
   bool isMass() const;
};

using MassOrVolumeConcentrationAmtBase =
   Measurement::ConstrainedAmount<Measurement::PhysicalQuantity::MassConcentration,
                                  Measurement::PhysicalQuantity::VolumeConcentration>;
/**
 * \brief A version of \c Measurement::Amount that is "constrained" to be either a
 *        \c Measurement::PhysicalQuantity::MassConcentration or a \c Measurement::PhysicalQuantity::VolumeConcentration.
 */
class MassOrVolumeConcentrationAmt : public MassOrVolumeConcentrationAmtBase {
public:
   using MassOrVolumeConcentrationAmtBase::MassOrVolumeConcentrationAmtBase;
   bool isMassConcentration() const;
};


#endif
