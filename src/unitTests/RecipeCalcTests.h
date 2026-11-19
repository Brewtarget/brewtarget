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

   /**
    * \brief Verify that a boil step with no start/end temperature is treated as being at boiling.  See
    *        https://github.com/Brewtarget/brewtarget/issues/1165
    */
   void testBoilStepsWithoutTemperatures();

   /**
    * \brief Verify that boil steps are classified as pre-boil, boil proper and post-boil by their temperatures and
    *        order, per the dotBeer rules (see \c Boil::stepPhases).
    */
   void testBoilStepPhases();

   /**
    * \brief Verify that changes to a recipe's boil, and to its boil steps, are reflected in the recipe's calculated
    *        values (IBUs, volumes, gravities) without an explicit recalculation.
    */
   void testBoilChangesRecalculate();

   /**
    * \brief Verify IBU calculations for hops in hot wort after flameout -- eg flameout, whirlpool and hop stand
    *        additions, and boil additions while the wort cools.  See https://github.com/Brewtarget/brewtarget/issues/774
    */
   void testPostBoilIbus();

   /**
    * \brief Verify that the ramp time of a post-boil step is handled correctly in the post-boil temperature profile,
    *        the timing of post-boil additions and the resulting IBUs.  Also verify that mIBU uses the kettle volume at
    *        flameout for the rate of natural cooling.
    */
   void testPostBoilRampTime();

   /**
    * \brief Verify the Garetz, Daniels and SMPH IBU formulae, including against John-Paul Hosom's published SMPH
    *        worked examples.
    */
   void testIbuFormulas();

   //! \brief Unit test: verify we can set and read inventory via Qt Properties
   void testInventory();

   /**
    * \brief Check the water volume calculations for the mash, in particular that:
    *          - mash tun foundation volume (eg space below the malt pipe in an all-in-one system) is added to the strike
    *            water, but is recovered (ie does not reduce the wort that reaches the boil, nor change the total water)
    *          - mash tun and lauter tun losses are allowed for exactly once in the total water, and are lost
    *          - the above works the same whether or not the brewer has a separate lauter tun
    */
   void testMashWaterVolumes();

   /**
    * \brief Check that the BeerXML-style evaporation rate (percent per hour) is relative to the boil size, and is kept
    *        consistent with the evaporation rate we actually use (liters per hour).
    */
   void testEvaporationRate();

   /**
    * \brief Check that vessel capacities are kept separate from default batch size and pre-boil volume, including the
    *        fallbacks we use when a file only gives us some of them.
    */
   void testEquipmentDefaultVolumes();

   /**
    * \brief Check where we get the mash tun's heat capacity and temperature from for strike water calculations: the
    *        equipment in preference to the (deprecated) fields on the Mash, and nothing at all if the mash happens in
    *        the kettle.
    */
   void testMashTunHeatCapacity();

   /**
    * \brief Verify that water alkalinity (used for residual alkalinity in the water adjuster) comes from bicarbonate and
    *        carbonate when alkalinity itself is not set (as is the case for all imported water profiles)
    */
   void testWaterAlkalinity();

   /**
    * \brief Verify that fermentables added after the boil proper (in a post-boil step, during fermentation or at
    *        packaging) do not contribute to the boil gravity (and hence do not affect IBUs), whereas ones added during
    *        the boil proper do.  See https://github.com/Brewtarget/brewtarget/issues/438
    */
   void testLateFermentableAdditions();

   /**
    * \brief Verify that the gravity used in IBU calculations is the average over the time each hop addition spends in
    *        the boil, taking into account evaporation and fermentables added part-way through the boil (eg late
    *        extract additions), whereas the pre-boil gravity only counts what is in the wort at the start of the boil.
    */
   void testTimeWeightedBoilGravity();

   /**
    * \brief Verify that taking a snapshot of a recipe (ie copying it and making the copy the original's ancestor, as
    *        the "Snapshot Recipe" context menu action and automatic versioning both do) works for a recipe with first
    *        wort, boil and post-boil hop additions.  In particular:
    *          - The copy constructor can do its calculations, even though the copy's additions do not yet know which
    *            recipe they belong to (because the copy does not yet have an ID).  See
    *            https://github.com/Brewken/brewken/pull/169
    *          - The copy is a deep copy of the additions, but shares the boil, equipment etc, and its calculated values
    *            match the original's both before and after it is stored
    *          - The ancestry is set up correctly, and changes to the copy do not affect the original
    *          - The same is true when the snapshot is made by automatic versioning
    */
   void testRecipeSnapshot();
};

#endif
