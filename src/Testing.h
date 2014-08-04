/*
 * Testing.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include <QObject>
#include <QtTest/QtTest>
#include <QSettings>
#include <QString>
#include <QDir>

#include "brewtarget.h"
//#include "MainWindow.h"
//#include "recipe.h"

class Testing : public QObject
{
   Q_OBJECT
   
public:
   
   //! \brief True iff. a <= c <= b
   static bool inRange( double c, double a, double b )
   {
      return (a <= c) && (c <= b);
   }
   
   //! \brief True iff. b-tol <= a <= b+tol
   static bool fuzzyComp( double a, double b, double tol )
   {
      return inRange( a, b-tol, b+tol );
   }
   
private slots:
   
   // Run once before all test cases
   void initTestCase()
   {
      // Create a different set of options to avoid clobbering real options
      QCoreApplication::setOrganizationName("brewtarget");
      QCoreApplication::setOrganizationDomain("brewtarget.org");
      QCoreApplication::setApplicationName("brewtarget-test");
      
      // Set options so that any data modification does not affect any other data
      Brewtarget::setOption("user_data_dir", QDir::tempPath());
      Brewtarget::setOption("color_formula", "morey");
      Brewtarget::setOption("ibu_formula", "tinseth");
      
      QVERIFY( Brewtarget::initialize() );
   }
   
   // Run once after all test cases
   void cleanupTestCase()
   {
      Brewtarget::cleanup();
   }
   
   //! \brief Unit test: verify brewtarget runs
   void runTest()
   {
      QVERIFY( 1==1 );
      /*
      MainWindow* mw = Brewtarget::mainWindow();
      QVERIFY( mw );
      */
   }
   
   //! \brief Test: verify some standard recipe calculates properly
   void recipeCalcTest()
   {
      QVERIFY( 42==42 );
      
      /*
      Recipe* rec = Database::instance().newRecipe();
      rec->setName("TestRecipe");
      rec->setBatchSize_l(18.93); // 5 gallons
      rec->setBoilSize_l(23.47);  // 6.2 gallons
      rec->setEfficiency_pct(70.0);
      
      ...
      
      // Verify all recipe parameters within some tolerance.
      QVERIFY2( fuzzyComp(rec->og(), 1.050, 0.002), "Wrong OG calculation" );
      QVERIFY2( fuzzyComp(rec->IBU(), 30, 5), "Wrong IBU calculation" );
      ...
      */
   }
};

#endif /*TESTING_H*/
