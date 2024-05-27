/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * unitTests/Testing.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef TESTING_H
#define TESTING_H
#pragma once

#include <memory>

#include <QObject>



class Testing : public QObject {
   Q_OBJECT

public:
   Testing();
   virtual ~Testing();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

private slots:

   // Run once before all test cases
   void initTestCase();

   // Run once after all test cases
   void cleanupTestCase();

   //! \brief Verify pstdint.h is sane
   void pstdintTest();

   //! \brief Unit test: verify we can set and read inventory via Qt Properties
   void testInventory();

   //! \brief Verify standard all-grain recipe calculates properly
   void recipeCalcTest_allGrain();

   //! \brief Verify post-boil losses do not affect OG
   void postBoilLossOgTest();

   //! \brief Verify conversion between US Customary & Metric units etc
   void testUnitConversions();

   //! \brief Test that NamedParameterBundle is behaving as we expect
   void testNamedParameterBundle();

   /**
    * \brief Verify various number extractions and conversions, including with localisation.
    */
   void testNumberDisplayAndParsing();

   /**
    * \brief Verify other conversions that warrant their own algorithms.
    *
    *        This is usually things where we have a formula in one direction but do some root-finding for the inverse
    *        function because there isn't a non-horrible formula in that direction.
    */
   void testAlgorithms();

   /**
    * \brief Verify the mechanism we use for looking up type info about a parameter in the "model" classes (ie
    *        \c NamedEntity and subclasses thereof).
    */
   void testTypeLookups();

   //! \brief Verify Log rotation is working
   void testLogRotation();

};

#endif
