/*======================================================================================================================
 * unitTests/SerializationTests.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#ifndef UNITTESTS_SERIALIZATIONTESTS_H
#define UNITTESTS_SERIALIZATIONTESTS_H
#pragma once

#include <memory>

#include "unitTests/TestClassBase.h"

/**
 * \brief Unit tests for reading and writing files -- dotBeer, BeerJSON, BeerXML
 *
 *        See comment in \c unitTests/TestRegistry.h for how to add new tests.
 */
class SerializationTests : public UnitTests::TestClassBase {
   Q_OBJECT

public:
   SerializationTests();
   virtual ~SerializationTests();

protected:
   /**
    * \brief Run once before all test functions in this class.  Imports the dotBeer example files from
    *        testData/dotBeer/examples (which checks that all the valid ones can be read), so that all the test
    *        functions in this class have them to work with.
    */
   void initTestClass() override;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

private slots:
   //! \brief Check calculated values on recipes imported from the dotBeer examples
   void testDotBeerImportedRecipeCalcs();

   //! \brief Check we correct (rather than reject) recipe additions for steps that don't exist
   void testDotBeerAdditionStepCorrection();

   /**
    * \brief Check brew log calculations don't give nonsense (or NaN) when gravities have not all been measured.  (This
    *        is here rather than in \c RecipeCalcTests because it uses a recipe from the dotBeer examples.)
    */
   void testBrewLogCalcs();

   //! \brief Check we give a helpful message for files written with a newer version of dotBeer than we support
   void testDotBeerVersionHandling();

   /**
    * \brief Add some things that possible with our internal data structures, but which used to give an invalid dotBeer
    *        file on export, then export the database to dotBeer, check the result, and check it can be read back in
    *        (which includes validating it against the schema).  Set environment variable DOTBEER_EXPORT_FILE to keep
    *        the exported file (eg to check it with DotBeer's validate.py).
    */
   void testDotBeerExport();

   //! \brief Check \c Equipment::clearUnusedOptionalVessels (which the equipment editor calls on save)
   void testClearUnusedOptionalVessels();

   /**
    * \brief YIELD is required in BeerXML but \c Fermentable::fineGrindYield_pct is optional for us.  Check we write
    *        YIELD of 0 if it's not set and, when reading, treat YIELD of 0 as not set.  Also check potential yield
    *        survives a BeerXML round trip.
    */
   void testBeerXmlNullYield();

   /**
    * \brief Check that a null sentinel substituted on write survives the "remove optional wrapper" step that
    *        \c JsonRecord does straight afterwards.
    */
   void testNullSentinel();

   /**
    * \brief Export the database to BeerJSON and check the result can be read back in (which includes validating it
    *        against the BeerJSON schema).  Set environment variable BEERJSON_EXPORT_FILE to keep the exported file.
    */
   void testBeerJsonRoundTrip();
};

#endif
