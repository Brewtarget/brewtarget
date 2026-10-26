/*======================================================================================================================
 * unitTests/RecipeCalcTests.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef UNITTESTS_RECIPECALCTESTS_H
#define UNITTESTS_RECIPECALCTESTS_H
#pragma once

#include <memory>

#include "unitTests/TestClassBase.h"

/**
 * \brief Unit tests for recipe calculations, inventory, etc
 *
 *        See comment in \c unitTests/TestRegistry.h for how to add new tests.
 */
class RecipeCalcTests : public UnitTests::TestClassBase {
   Q_OBJECT

public:
   RecipeCalcTests();
   virtual ~RecipeCalcTests();

protected:
   //! \brief Run once before all test functions in this class.  Creates fixtures used by the test functions.
   void initTestClass() override;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

private slots:
   //! \brief Verify standard all-grain recipe calculates properly
   void recipeCalcTest_allGrain();

   //! \brief Verify post-boil losses do not affect OG
   void postBoilLossOgTest();

   /**
    * \brief Verify that the timing of boil additions (including post-boil/whirlpool additions and additions with a
    *        step but no time) is handled correctly in IBU calculations and in generated instructions.  See
    *        https://github.com/Brewtarget/brewtarget/issues/1167 and
    *        https://github.com/Brewtarget/brewtarget/issues/1168
    */
   void testBoilAdditionTiming();

   //! \brief Unit test: verify we can set and read inventory via Qt Properties
   void testInventory();
};

#endif
