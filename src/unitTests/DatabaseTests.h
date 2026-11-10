/*======================================================================================================================
 * unitTests/DatabaseTests.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef UNITTESTS_DATABASETESTS_H
#define UNITTESTS_DATABASETESTS_H
#pragma once

#include "unitTests/TestClassBase.h"

/**
 * \brief Unit tests for the database layer, in particular schema upgrades
 *
 *        See comment in \c unitTests/TestRegistry.h for how to add new tests.
 */
class DatabaseTests : public UnitTests::TestClassBase {
   Q_OBJECT

public:
   DatabaseTests();
   virtual ~DatabaseTests();

private slots:
   /**
    * \brief Take a copy of the default database from Brewtarget 2.3.1 (which has schema version "2.1.0", ie 3), and
    *        check that we can upgrade it to the latest schema version.  As well as checking that the migration
    *        succeeds, we check that the upgraded database has all the tables and columns of a database created from
    *        scratch at the latest schema version, and that its data is self-consistent.
    */
   void testUpgradeBrewtarget231Database();

   /**
    * \brief Check the logic for things (eg Mash) that can be shared between Recipes and locked -- see
    *        \c model/LockablePropertyBase.h and the related functions in \c model/RecipeUtils.h.
    */
   void testLockingAndSharing();

   /**
    * \brief Check automatic Recipe versioning (see \c RecipeUtils::prepareForPropertyChange): changing a brewed Recipe
    *        creates one new version (to which the BrewLogs move), and nothing else does.
    */
   void testAutomaticVersioning();
};

#endif
