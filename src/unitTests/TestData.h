/*======================================================================================================================
 * unitTests/TestData.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#ifndef UNITTESTS_TESTDATA_H
#define UNITTESTS_TESTDATA_H
#pragma once

#include <memory>

#include <QList>
#include <QString>

#include "database/ObjectStoreWrapper.h"

class Equipment;
class Fermentable;
class Hop;

/**
 * \brief Helper functions and test data shared between more than one test class
 */
namespace UnitTests {

   //! \brief True iff a <= c <= b
   constexpr bool inRange(double const c, double const a, double const b) {
      return (a <= c) && (c <= b);
   }

   //! \brief True iff b - tolerance <= a <= b + tolerance.  Logs the values if not.
   bool fuzzyComp(double const a, double const b, double const tolerance);

   //! \return Cascade pellets at 4% alpha acid (stored in the database)
   std::shared_ptr<Hop> makeHop_Cascade();

   //! \return 5 gallon equipment with no losses (not stored in the database)
   std::shared_ptr<Equipment> makeEquipment_FiveGalNoLoss();

   //! \return Two-row malt: 70% yield, no moisture, 2 SRM (not stored in the database)
   std::shared_ptr<Fermentable> makeFermentable_TwoRow();

   /**
    * \brief Import all the dotBeer example files from \c testData/dotBeer/examples, including the invalid ones (some
    *        of which we accept, eg after correcting them).  The examples come from the examples directory of the
    *        dotBeer repository (https://github.com/Brewken/DotBeer), so they need to be updated when we upgrade to
    *        newer versions of dotBeer.
    *
    * \param errorMessage Set to describe the problem if we return \c false
    *
    * \return \c true if all the valid examples were imported and at least one invalid one was found, \c false otherwise
    */
   bool importDotBeerExamples(QString & errorMessage);

   //! \return All the objects of type \c NE in the database, as a list of const pointers (as needed for exporting)
   template<class NE>
   QList<NE const *> allConst() {
      QList<NE const *> result;
      for (NE * ne : ObjectStoreWrapper::getAllRaw<NE>()) {
         result.append(ne);
      }
      return result;
   }

   //! \return The first object of type \c NE in the database with the supplied name, or \c nullptr if there is none
   template<class NE>
   std::shared_ptr<NE> findByName(QString const & name) {
      return ObjectStoreWrapper::findFirstMatching<NE>(
         [&name](std::shared_ptr<NE> ne) { return ne->name() == name; }
      );
   }

}

#endif
