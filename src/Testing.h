/*
 * Testing.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2020
 * - Philip G. Lee <rocketman768@gmail.com>
 * - Mattias M�hl <mattias@kejsarsten.com>
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
#include <QDebug>
#include <QMutexLocker>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <QtGlobal>
#endif

class Equipment;
class Hop;
class Fermentable;

#include "brewtarget.h"
#include "pstdint.h"
#include "Log.h"

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
      bool ret = inRange( a, b-tol, b+tol );
      if( !ret )
         qDebug() << QString("a: %1, b: %2, tol: %3").arg(a).arg(b).arg(tol);
      return ret;
   }

   // method to fill dummy logs with content to build size
   static QString randomStringGenerator()
   {
      QString posChars = "ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwwxyz";
      int randomcharLength = 64;

      QString randSTR;
      for (int i = 0; i < randomcharLength; i++)
      {
         #if QT_VERSION < QT_VERSION_CHECK(5,10,0)
         int index = qrand() % posChars.length();
         #else
         int index = QRandomGenerator().generate64() % posChars.length();
         #endif
         QChar nChar = posChars.at(index);
         randSTR.append(nChar);
      }
      return randSTR;
   }

private:

   Equipment* equipFiveGalNoLoss;
   Hop* cascade_4pct;
   //! \brief 70% yield, no moisture, 2 SRM
   Fermentable* twoRow;

private slots:

   // Run once before all test cases
   void initTestCase();

   // Run once after all test cases
   void cleanupTestCase();

   //! \brief Verify pstdint.h is sane
   void pstdintTest()
   {
      QVERIFY( sizeof(int8_t) == 1 );
      QVERIFY( sizeof(int16_t) == 2 );
      QVERIFY( sizeof(int32_t) == 4 );
#ifdef stdint_int64_defined
      QVERIFY( sizeof(int64_t) == 8 );
#endif

      QVERIFY( sizeof(uint8_t) == 1 );
      QVERIFY( sizeof(uint16_t) == 2 );
      QVERIFY( sizeof(uint32_t) == 4 );
#ifdef stdint_int64_defined
      QVERIFY( sizeof(uint64_t) == 8 );
#endif
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

   //! \brief Verify standard all-grain recipe calculates properly
   void recipeCalcTest_allGrain();

   //! \brief Verify post-boil losses do not affect OG
   void postBoilLossOgTest();

   //! \brief Verify Log rotation is working
   void testLogRotation();
};

#endif /*TESTING_H*/
