/*======================================================================================================================
 * unitTests/MeasurementTests.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#include "unitTests/MeasurementTests.h"

#include <cmath>
#include <utility>

#include <QDebug>
#include <QString>
#include <QTextStream>
#include <QtTest/QtTest>
#include <QVector>

#include "Algorithms.h"
#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "unitTests/TestData.h"
#include "unitTests/TestRegistry.h"

REGISTER_TEST_CLASS(MeasurementTests)

namespace {
   using UnitTests::fuzzyComp;

   struct SgBrixEquivalance {
      double sg;
      double brix;
      double errorMarginSgToBrix;
      double errorMarginBrixToSg;
   };

   //
   // We test conversion in both directions Brix -> SG and SG -> Brix.  As commented elsewhere, I don't think many
   // brewers use Brix.  We need Brix -> SG at least to be able to read any BeerJSON file that uses Brix.  I think our
   // method (of interpolating the USDA published data) is likely more accurate than the best-fit cubic equation that
   // most folks use.  However, for now, we're just testing here that our method is not hugely different than those
   // used to generate tables on popular homebrew sites.
   //
   QVector<SgBrixEquivalance> const sgBrixEquivalances {
      //
      // With a few exceptions, the data below come from
      // https://winning-homebrew.com/wp-content/uploads/2021/03/sgtobrix_conv_table.pdf (linked to from
      // https://winning-homebrew.com/specific-gravity-to-brix.html).  It's not clear exactly how those figures were
      // obtained, so it's not necessarily an error if we get slightly different results.  Still, it would be surprising
      // if we were a long way adrift from these numbers.
      //

      //                Error Margins
      // SG    Brix   SG->Brix  Brix->SG
      {1.00156,  0.4, 0.00001, 0.00001},
      {0.990,  0.00,  0.005,    0.0110},  // ===========================================================================
      {0.991,  0.00,  0.005,    0.0100},  //
      {0.992,  0.00,  0.005,    0.0090},  // For SG < 1.0, the conversion between Brix and SG is not very meaningful:
      {0.993,  0.00,  0.005,    0.0080},  //  - SG to Brix will always give 0
      {0.994,  0.00,  0.005,    0.0070},  //  - Brix to SG will always give 1.0
      {0.995,  0.00,  0.005,    0.0060},  //
      {0.996,  0.00,  0.005,    0.0050},  // We leave these test cases in for now, but with larger error bars then the
      {0.997,  0.00,  0.005,    0.0040},  // more meaningful conversions below
      {0.998,  0.00,  0.005,    0.0030},  //
      {0.999,  0.00,  0.005,    0.0020},  // ===========================================================================
      {1.000,  0.00,  0.005,    0.0002},
      {1.001,  0.26,  0.010,    0.0002},
      {1.002,  0.51,  0.010,    0.0002},
      {1.003,  0.77,  0.010,    0.0002},
      {1.004,  1.03,  0.010,    0.0002},
      {1.005,  1.28,  0.010,    0.0002},
      {1.006,  1.54,  0.010,    0.0002},
      {1.007,  1.80,  0.010,    0.0002},
      {1.008,  2.05,  0.010,    0.0002},
      {1.009,  2.31,  0.010,    0.0002},
      {1.010,  2.56,  0.010,    0.0002},
      {1.011,  2.81,  0.010,    0.0002},
      {1.012,  3.07,  0.010,    0.0002},
      {1.013,  3.32,  0.010,    0.0002},
      {1.014,  3.57,  0.010,    0.0002},
      {1.015,  3.82,  0.010,    0.0002},
      {1.016,  4.08,  0.010,    0.0002},
      {1.017,  4.33,  0.010,    0.0002},
      {1.018,  4.58,  0.010,    0.0002},
      {1.019,  4.83,  0.010,    0.0002},
      {1.020,  5.08,  0.010,    0.0002},
      {1.021,  5.33,  0.010,    0.0002},
      {1.022,  5.57,  0.020,    0.0002},
      {1.023,  5.82,  0.020,    0.0002},
      {1.024,  6.07,  0.020,    0.0002},
      {1.025,  6.32,  0.020,    0.0002},
      {1.026,  6.57,  0.020,    0.0002},
      {1.027,  6.81,  0.020,    0.0002},
      {1.028,  7.06,  0.020,    0.0002},
      {1.029,  7.30,  0.020,    0.0002},
      {1.030,  7.55,  0.020,    0.0002},
      {1.031,  7.80,  0.020,    0.0002},
      {1.032,  8.04,  0.020,    0.0002},
      {1.033,  8.28,  0.020,    0.0002},
      {1.034,  8.53,  0.020,    0.0002},
      {1.035,  8.77,  0.020,    0.0002},
      {1.036,  9.01,  0.020,    0.0002},
      {1.037,  9.26,  0.020,    0.0002},
      {1.038,  9.50,  0.020,    0.0002},
      {1.039,  9.74,  0.020,    0.0002},
      {1.040,  9.98,  0.020,    0.0002},
      {1.041, 10.22,  0.020,    0.0002},
      {1.042, 10.46,  0.020,    0.0002},
      {1.043, 10.70,  0.020,    0.0002},
      {1.044, 10.94,  0.020,    0.0002},
      {1.045, 11.18,  0.020,    0.0002},
      {1.046, 11.42,  0.020,    0.0002},
      {1.047, 11.66,  0.020,    0.0002},
      {1.048, 11.90,  0.020,    0.0002},
      {1.049, 12.14,  0.020,    0.0002},
      {1.050, 12.37,  0.030,    0.0002},
      {1.051, 12.61,  0.030,    0.0002},
      {1.052, 12.85,  0.030,    0.0002},
      {1.053, 13.08,  0.030,    0.0002},
      {1.054, 13.32,  0.030,    0.0002},
      {1.055, 13.55,  0.030,    0.0002},
      {1.056, 13.79,  0.030,    0.0002},
      {1.057, 14.02,  0.030,    0.0002},
      {1.058, 14.26,  0.030,    0.0002},
      {1.059, 14.49,  0.030,    0.0002},
      {1.060, 14.72,  0.030,    0.0002},
      {1.061, 14.96,  0.030,    0.0002},
      {1.062, 15.19,  0.030,    0.0002},
      {1.063, 15.42,  0.030,    0.0002},
      {1.064, 15.65,  0.030,    0.0002},
      {1.065, 15.88,  0.030,    0.0002},
      {1.066, 16.11,  0.030,    0.0002},
      {1.067, 16.34,  0.030,    0.0002},
      {1.068, 16.57,  0.030,    0.0002},
      {1.069, 16.80,  0.030,    0.0002},
      {1.070, 17.03,  0.030,    0.0002},
      {1.071, 17.26,  0.030,    0.0002},
      {1.072, 17.49,  0.030,    0.0002},
      {1.073, 17.72,  0.030,    0.0002},
      {1.074, 17.95,  0.030,    0.0002},
      {1.075, 18.18,  0.030,    0.0002},
      {1.076, 18.40,  0.030,    0.0002},
      {1.077, 18.63,  0.030,    0.0002},
      {1.078, 18.86,  0.030,    0.0002},
      {1.079, 19.08,  0.030,    0.0002},
      {1.080, 19.31,  0.030,    0.0002},
      {1.081, 19.53,  0.030,    0.0002},
      {1.082, 19.76,  0.030,    0.0002},
      {1.083, 19.98,  0.030,    0.0002},
      {1.084, 20.21,  0.030,    0.0002},
      {1.085, 20.43,  0.030,    0.0002},
      {1.086, 20.65,  0.030,    0.0002},
      {1.087, 20.88,  0.030,    0.0002},
      {1.088, 21.10,  0.030,    0.0002},
      {1.089, 21.32,  0.030,    0.0002},
      {1.090, 21.54,  0.032,    0.0002},  // <- Wider difference on SG->Brix
      {1.091, 21.77,  0.030,    0.0002},
      {1.092, 21.99,  0.030,    0.0002},
      {1.093, 22.21,  0.030,    0.0002},
      {1.094, 22.43,  0.030,    0.0002},
      {1.095, 22.65,  0.030,    0.0002},
      {1.096, 22.87,  0.030,    0.0002},
      {1.097, 23.09,  0.030,    0.0002},
      {1.098, 23.31,  0.030,    0.0002},
      {1.099, 23.53,  0.030,    0.0002},
      {1.100, 23.75,  0.030,    0.0002},
      {1.101, 23.96,  0.035,    0.0002},
      {1.102, 24.18,  0.035,    0.0002},
      {1.103, 24.40,  0.035,    0.0002},
      {1.104, 24.62,  0.035,    0.0002},
      {1.105, 24.83,  0.035,    0.0002},
      {1.106, 25.05,  0.035,    0.0002},
      {1.107, 25.27,  0.035,    0.0002},
      {1.108, 25.48,  0.035,    0.0002},
      {1.109, 25.70,  0.035,    0.0002},
      {1.110, 25.91,  0.035,    0.0002},
      {1.111, 26.13,  0.035,    0.0002},
      {1.112, 26.34,  0.035,    0.0002},
      {1.113, 26.56,  0.035,    0.0002},
      {1.114, 26.77,  0.035,    0.0002},
      {1.115, 26.98,  0.035,    0.0002},
      {1.116, 27.20,  0.035,    0.0002},
      {1.117, 27.41,  0.035,    0.0002},
      {1.118, 27.62,  0.035,    0.0002},
      {1.119, 27.83,  0.036,    0.0002},  // <- Wider difference on SG->Brix
      {1.120, 28.05,  0.035,    0.0002},
      {1.121, 28.26,  0.035,    0.0002},
      {1.122, 28.47,  0.035,    0.0002},
      {1.123, 28.68,  0.035,    0.0002},
      {1.124, 28.89,  0.035,    0.0002},
      {1.125, 29.10,  0.035,    0.0002},
      {1.126, 29.31,  0.035,    0.0002},
      {1.127, 29.52,  0.035,    0.0002},
      {1.128, 29.73,  0.035,    0.0002},
      {1.129, 29.94,  0.035,    0.0002},
      {1.130, 30.15,  0.035,    0.0002},
   };
}

class MeasurementTests::impl {
public:
   impl() = default;
   ~impl() = default;
};

MeasurementTests::MeasurementTests() : UnitTests::TestClassBase(), pimpl{std::make_unique<impl>()} {
   return;
}

MeasurementTests::~MeasurementTests() = default;

void MeasurementTests::testUnitConversions() {
   //
   // Originally, some of these tests assumed '.' is the decimal separator and ',' is the digit group separator.  This
   // meant they would fail on locales where this is not the case.  Plan A was just to force the locale to be one that
   // makes the tests work (because we are testing conversions rather than number parsing, eg via:
   //    QLocale::setDefault(QLocale::C);
   // However, this didn't seem to have any effect on a French locale Windows.
   // So Plan B is to construct test data based on the current locale settings.
   //
   QString const decimalSeparator   = Localization::getLocale().decimalPoint();
   QString const thousandsSeparator = Localization::getLocale().groupSeparator();
   qDebug() <<
      Q_FUNC_INFO << "Decimal separator is " << decimalSeparator << " | Thousands separator is " << thousandsSeparator;
   QString testInput{};
   QTextStream testInputAsStream{&testInput};
   // "5.500 gal"
   testInput.clear();
   testInputAsStream << "5" << decimalSeparator << "500 gal";
   QVERIFY2(fuzzyComp(Measurement::UnitSystems::volume_UsCustomary.qstringToSI(testInput, // "5.500 gal"
                                                                               Measurement::Units::liters).quantity,
                      20.820,
                      0.001),
            "Unit conversion error (US gallons to Litres v1)");
   // "5.500"
   testInput.clear();
   testInputAsStream << "5" << decimalSeparator << "500";
   QVERIFY2(fuzzyComp(Measurement::UnitSystems::volume_UsCustomary.qstringToSI(testInput, // "5.500"
                                                                               Measurement::Units::us_gallons).quantity,
                      20.820,
                      0.001),
            "Unit conversion error (US gallons to Litres v2)");
   // "5.500 gal"
   // This is an ambiguous case where the units could be US gallons or Imperial gallons.  At the moment, we say either
   // interpretation is valid (in the absence of any other context).
   // 5.5 US gallons       ≈ 20.820 litres
   // 5.5 Imperial gallons ≈ 25.004 litres
   testInput.clear();
   testInputAsStream << "5" << decimalSeparator << "500 gal";
   auto converted = Measurement::qStringToSI(testInput, // "5.500 gal"
                                             Measurement::PhysicalQuantity::Volume);
   qDebug() << Q_FUNC_INFO << testInput << "converted to" << converted;
   QVERIFY2(fuzzyComp(converted.quantity, 20.820, 0.001) ||
            fuzzyComp(converted.quantity, 25.004, 0.001),
            "Unit conversion error (US or Imperial gallons to Litres v3)");
   // "9.994 P"
   testInput.clear();
   testInputAsStream << "9" << decimalSeparator << "994 P";
   QVERIFY2(fuzzyComp(Measurement::UnitSystems::density_Plato.qstringToSI(testInput, // "9.994 P"
                                                                          Measurement::Units::specificGravity).quantity,
                      1.040,
                      0.001),
            "Unit conversion error (Plato to SG)");
   // "1,083 ebc"
   testInput.clear();
   testInputAsStream << "1" << thousandsSeparator << "083 ebc";
   QVERIFY2(
      fuzzyComp(Measurement::UnitSystems::color_StandardReferenceMethod.qstringToSI(testInput, // "1,083 ebc"
                                                                                    Measurement::Units::srm).quantity,
                550,
                1),
      "Unit conversion error (EBC to SRM)"
   );

   return;
}

void MeasurementTests::testUnitInverses() {
   //
   // There isn't a list of all units, so we have to make one.  (If you add a new Unit, please add it here!)
   //
   std::initializer_list<std::pair<char const *, Measurement::Unit const *>> const allUnits {
      {"kilograms"               , &Measurement::Units::kilograms               },
      {"grams"                   , &Measurement::Units::grams                   },
      {"milligrams"              , &Measurement::Units::milligrams              },
      {"pounds"                  , &Measurement::Units::pounds                  },
      {"ounces"                  , &Measurement::Units::ounces                  },
      {"imperial_pounds"         , &Measurement::Units::imperial_pounds         },
      {"imperial_ounces"         , &Measurement::Units::imperial_ounces         },
      {"liters"                  , &Measurement::Units::liters                  },
      {"milliliters"             , &Measurement::Units::milliliters             },
      {"us_barrels"              , &Measurement::Units::us_barrels              },
      {"us_gallons"              , &Measurement::Units::us_gallons              },
      {"us_quarts"               , &Measurement::Units::us_quarts               },
      {"us_pints"                , &Measurement::Units::us_pints                },
      {"us_cups"                 , &Measurement::Units::us_cups                 },
      {"us_fluidOunces"          , &Measurement::Units::us_fluidOunces          },
      {"us_tablespoons"          , &Measurement::Units::us_tablespoons          },
      {"us_teaspoons"            , &Measurement::Units::us_teaspoons            },
      {"imperial_barrels"        , &Measurement::Units::imperial_barrels        },
      {"imperial_gallons"        , &Measurement::Units::imperial_gallons        },
      {"imperial_quarts"         , &Measurement::Units::imperial_quarts         },
      {"imperial_pints"          , &Measurement::Units::imperial_pints          },
      {"imperial_cups"           , &Measurement::Units::imperial_cups           },
      {"imperial_fluidOunces"    , &Measurement::Units::imperial_fluidOunces    },
      {"imperial_tablespoons"    , &Measurement::Units::imperial_tablespoons    },
      {"imperial_teaspoons"      , &Measurement::Units::imperial_teaspoons      },
      {"millimeters"             , &Measurement::Units::millimeters             },
      {"centimeters"             , &Measurement::Units::centimeters             },
      {"meters"                  , &Measurement::Units::meters                  },
      {"inches"                  , &Measurement::Units::inches                  },
      {"feet"                    , &Measurement::Units::feet                    },
      {"numberOf"                , &Measurement::Units::numberOf                },
      {"celsius"                 , &Measurement::Units::celsius                 },
      {"fahrenheit"              , &Measurement::Units::fahrenheit              },
      {"minutes"                 , &Measurement::Units::minutes                 },
      {"weeks"                   , &Measurement::Units::weeks                   },
      {"days"                    , &Measurement::Units::days                    },
      {"hours"                   , &Measurement::Units::hours                   },
      {"seconds"                 , &Measurement::Units::seconds                 },
      {"srm"                     , &Measurement::Units::srm                     },
      {"ebc"                     , &Measurement::Units::ebc                     },
      {"lovibond"                , &Measurement::Units::lovibond                },
      {"specificGravity"         , &Measurement::Units::specificGravity         },
      {"gravityPoints"           , &Measurement::Units::gravityPoints           },
      {"plato"                   , &Measurement::Units::plato                   },
      {"brix"                    , &Measurement::Units::brix                    },
      {"lintner"                 , &Measurement::Units::lintner                 },
      {"wk"                      , &Measurement::Units::wk                      },
      {"pH"                      , &Measurement::Units::pH                      },
      {"ibu"                     , &Measurement::Units::ibu                     },
      {"carbonationVolumes"      , &Measurement::Units::carbonationVolumes      },
      {"carbonationGramsPerLiter", &Measurement::Units::carbonationGramsPerLiter},
      {"partsPerMillionMass"     , &Measurement::Units::partsPerMillionMass     },
      {"partsPerBillionMass"     , &Measurement::Units::partsPerBillionMass     },
      {"milligramsPerLiter"      , &Measurement::Units::milligramsPerLiter      },
      {"centipoise"              , &Measurement::Units::centipoise              },
      {"millipascalSecond"       , &Measurement::Units::millipascalSecond       },
   };
   // Values in the range that makes sense for most units, including ones (eg Plato, Brix) where we interpolate
   for (auto const & [unitName, unit] : allUnits) {
      for (double const value : {1.05, 5.0, 12.3, 25.0}) {
         double const roundTrip = unit->fromCanonical(unit->toCanonical(value).quantity);
         QVERIFY2(fuzzyComp(roundTrip, value, value * 1e-6),
                  qPrintable(QString{"%1: %2 -> %3 (canonical) -> %4"}.arg(unitName)
                                                                      .arg(value)
                                                                      .arg(unit->toCanonical(value).quantity)
                                                                      .arg(roundTrip)));
      }
   }
   return;
}

void MeasurementTests::testNumberDisplayAndParsing() {
   // Per comment above, we should be seeing number formats in French locale here
   // Eg: 1,234.56 in US locale = 1.234,56 in French locale
   QVERIFY(fuzzyComp(1.23, Measurement::extractRawFromString<double>("1,23 %"                  ), 0.0000000001));
   QVERIFY(fuzzyComp(3.45, Measurement::extractRawFromString<double>("  03,45 srm  "           ), 0.0000000001));
   QVERIFY(fuzzyComp(6.78, Measurement::extractRawFromString<double>("\t6,78000000    bananas!"), 0.0000000001));
   QVERIFY(1    == Measurement::extractRawFromString<int>   ("1,23 %"));
   QVERIFY(3    == Measurement::extractRawFromString<int>   ("  03,45 srm  "));
   QVERIFY(6    == Measurement::extractRawFromString<int>   ("\t6,78000000    bananas!"));
   return;
}

void MeasurementTests::testAlgorithms() {
   for (auto const & ii : sgBrixEquivalances) {
      qDebug() <<
         Q_FUNC_INFO << "Testing conversions between" << ii.sg << "SG = " << ii.brix << "Brix; SG->Brix=" <<
         Algorithms::SgAt20CToBrix(ii.sg) << "(" << std::abs(ii.brix - Algorithms::SgAt20CToBrix(ii.sg)) <<
         "); Brix->SG=" << Algorithms::BrixToSgAt20C(ii.brix) << "(" <<
         std::abs(Algorithms::BrixToSgAt20C(ii.brix) - ii.sg) << ")";
      QVERIFY2(
         fuzzyComp(Algorithms::BrixToSgAt20C(ii.brix), ii.sg, ii.errorMarginBrixToSg),
         "Error converting Brix to Specific Gravity"
      );
      QVERIFY2(
         fuzzyComp(Algorithms::SgAt20CToBrix(ii.sg), ii.brix, ii.errorMarginSgToBrix),
         "Error converting Specific Gravity to Brix"
      );
   }
   return;
}
