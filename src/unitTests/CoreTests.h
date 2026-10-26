/*======================================================================================================================
 * unitTests/CoreTests.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#ifndef UNITTESTS_CORETESTS_H
#define UNITTESTS_CORETESTS_H
#pragma once

#include <memory>

#include "unitTests/TestClassBase.h"

/**
 * \brief Unit tests for core infrastructure (type lookups, NamedParameterBundle, MultiVector, logging, etc)
 *
 *        See comment in \c unitTests/TestRegistry.h for how to add new tests.
 */
class CoreTests : public UnitTests::TestClassBase {
   Q_OBJECT

public:
   CoreTests();
   virtual ~CoreTests();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

private slots:
   //! \brief Verify pstdint.h is sane
   void pstdintTest();

   //! \brief Test that NamedParameterBundle is behaving as we expect
   void testNamedParameterBundle();

   /**
    * \brief Verify the mechanism we use for looking up type info about a parameter in the "model" classes (ie
    *        \c NamedEntity and subclasses thereof).
    */
   void testTypeLookups();

   //! \brief Check for off-by-one errors etc in the implementation of \c MultiVector
   void testMultiVector();

   //! \brief Verify Log rotation is working
   void testLogRotation();
};

#endif
