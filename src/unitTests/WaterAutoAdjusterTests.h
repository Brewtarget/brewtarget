/*======================================================================================================================
 * unitTests/WaterAutoAdjusterTests.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef UNITTESTS_WATERAUTOADJUSTERTESTS_H
#define UNITTESTS_WATERAUTOADJUSTERTESTS_H
#pragma once

#include "unitTests/TestClassBase.h"

/**
 * \brief Unit tests for \c WaterAutoAdjuster.  These use \c WaterAutoAdjuster::Inputs set directly, so they don't need any
 *        model objects and we can check the results against figures worked out by hand.
 *
 *        See comment in \c unitTests/TestRegistry.h for how to add new tests.
 */
class WaterAutoAdjusterTests : public UnitTests::TestClassBase {
   Q_OBJECT

public:
   WaterAutoAdjusterTests();
   virtual ~WaterAutoAdjusterTests();

private slots:
   //! \brief Ions added per gram per liter of each salt, checked against figures worked out by hand
   void testSaltChemistry();

   //! \brief With only two salts, a target that they can hit exactly gives back the amounts used to make it
   void testExactAmounts();

   //! \brief With all the default salts, a target that can be hit is hit (even though the amounts aren't unique)
   void testDefaultSalts();

   //! \brief Base water, RO dilution and additions we aren't changing are all taken into account
   void testBaseWaterAndFixedAdditions();

   //! \brief Where the target can't be hit, we get as close as we can, with sensible warnings
   void testUnreachableTarget();

   //! \brief Amounts too small to weigh out are dropped
   void testMinimumAddition();

   //! \brief Things we can't do
   void testErrors();

   //! \brief Acid strengths, densities and milliequivalents, checked against figures worked out by hand
   void testAcidChemistry();

   //! \brief Mash pH model, checked against figures worked out by hand
   void testMashPh();

   //! \brief Amount of acid needed to get a target mash pH
   void testAcidAmounts();

   /**
    * \brief Auto-adjusting a recipe, undo and redo, and how these interact with keep in sync (see
    *        \c UndoableAutoAdjust)
    */
   void testRecipeUndoAndKeepInSync();

   //! \brief Suggested target mash pH from the pH set on the recipe's mash steps
   void testSuggestedTargetMashPh();

   /**
    * \brief The water profiles in \c data/DefaultContent006-MashAndWaterProfiles.beer import, and Auto-Adjust can
    *        match all the style target profiles from RO water using its default salts
    */
   void testExampleProfiles();
};

#endif
