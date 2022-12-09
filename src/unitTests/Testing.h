/*
 * Testing.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Mattias Måhl <mattias@kejsarsten.com>
 * - Philip G. Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TESTING_H
#define TESTING_H
#pragma once

#include <cstdint>
#include <memory>

#include <QDir>
#include <QObject>
#include <QtTest/QtTest>

class Equipment;
class Hop;
class Fermentable;

#include "Application.h"
#include "Logging.h"

class Testing : public QObject {
   Q_OBJECT

public:
   Testing();
   virtual ~Testing();

private:
   //! \brief Where we write database and log files etc
   QDir tempDir;

   std::shared_ptr<Equipment> equipFiveGalNoLoss;
   std::shared_ptr<Hop>       cascade_4pct;
   //! \brief 70% yield, no moisture, 2 SRM
   std::shared_ptr<Fermentable> twoRow;

private slots:

   // Run once before all test cases
   void initTestCase();

   // Run once after all test cases
   void cleanupTestCase();

   //! \brief Verify pstdint.h is sane
   void pstdintTest();

   //! \brief Unit test: verify Brewtarget runs
   void runTest();

   //! \brief Verify standard all-grain recipe calculates properly
   void recipeCalcTest_allGrain();

   //! \brief Verify post-boil losses do not affect OG
   void postBoilLossOgTest();

   //! \brief Verify conversion between US Customary & Metric units etc
   void testUnitConversions();

   /**
    * \brief Verify other conversions that warrant their own algorithms.
    *
    *        This is usually things where we have a formula in one direction but do some root-finding for the inverse
    *        function because there isn't a non-horrible formula in that direction.
    */
   void testAlgorithms();

   //! \brief Verify Log rotation is working
   void testLogRotation();

};

#endif
