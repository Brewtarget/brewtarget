/*======================================================================================================================
 * unitTests/MeasurementTests.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#ifndef UNITTESTS_MEASUREMENTTESTS_H
#define UNITTESTS_MEASUREMENTTESTS_H
#pragma once

#include <memory>

#include "unitTests/TestClassBase.h"

/**
 * \brief Unit tests for measurement units, unit conversions and number parsing
 *
 *        See comment in \c unitTests/TestRegistry.h for how to add new tests.
 */
class MeasurementTests : public UnitTests::TestClassBase {
   Q_OBJECT

public:
   MeasurementTests();
   virtual ~MeasurementTests();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

private slots:
   //! \brief Verify conversion between US Customary & Metric units etc
   void testUnitConversions();

   /**
    * \brief Verify that, for every \c Measurement::Unit, \c fromCanonical is the inverse of \c toCanonical.  (These
    *        are coded separately for non-linear units, so it's easy to get one of them wrong.)
    */
   void testUnitInverses();

   //! \brief Verify various number extractions and conversions, including with localisation.
   void testNumberDisplayAndParsing();

   /**
    * \brief Verify other conversions that warrant their own algorithms.
    *
    *        This is usually things where we have a formula in one direction but do some root-finding for the inverse
    *        function because there isn't a non-horrible formula in that direction.
    */
   void testAlgorithms();
};

#endif
