/*======================================================================================================================
 * unitTests/MashFermentabilityTests.h is part of Brewtarget, and is copyright the following authors 2026:
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
 =====================================================================================================================*/
#ifndef UNITTESTS_MASHFERMENTABILITYTESTS_H
#define UNITTESTS_MASHFERMENTABILITYTESTS_H
#pragma once

#include "unitTests/TestClassBase.h"

/**
 * \brief Unit tests for \c MashFermentability (the mash fermentability model used when \c Recipe::fgCalcUsesMash is
 *        set).  These don't need any model objects.
 *
 *        See comment in \c unitTests/TestRegistry.h for how to add new tests.
 */
class MashFermentabilityTests : public UnitTests::TestClassBase {
   Q_OBJECT

public:
   MashFermentabilityTests();
   virtual ~MashFermentabilityTests();

private slots:
   //! \brief We reproduce the paper's own model results for all nine of its experiments
   void testReproducesPaper();

   //! \brief The reference mash gives a multiplier of exactly 1, and the effects of temperature and time are as expected
   void testTemperatureAndTime();

   //! \brief Building temperature profiles from mash steps: ramps, end temperatures, sparge and souring steps, mash-out
   void testBuildProfile();

   //! \brief Step mashes give sensible results
   void testStepMashes();

   //! \brief Small changes to a mash step never make the result jump
   void testNoDiscontinuities();

   //! \brief Enzyme levels from diastatic power and mash thickness, including clamping to the tested ranges
   void testEnzymeLevels();

   //! \brief Cases where we decline to adjust, and the limits on the adjustment
   void testLimits();
};

#endif
