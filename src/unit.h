/*
 * unit.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
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

#ifndef _UNIT_H
#define _UNIT_H

class Unit;
class Units; // A container of instances.
class KilogramUnit;
class GramUnit;
class MilligramUnit;
class PoundUnit;
class OunceUnit;
class LiterUnit;
class MilliliterUnit;
class USBarrelUnit;
class USGallonUnit;
class USQuartUnit;
class USCupUnit;
class ImperialGallonUnit;
class ImperialQuartUnit;
class ImperialCupUnit;
class TablespoonUnit;
class TeaspoonUnit;
class SecondUnit;
class MinuteUnit;
class HourUnit;
class CelsiusUnit;
class FahrenheitUnit;
class KelvinUnit;
class EBCUnit;
class SRMUnit;
class PlatoUnit;
class SgUnit;
class LintnerUnit;
class WKUnit;

#include <QString>
#include <QObject>
#include <string>
#include <map>
#include <QMultiMap>
#include <QRegExp>

enum iUnitSystem
{
    SI = 0,
    USCustomary = 1,
    Imperial = 2,
    ImperialAndUS = 3,
    Any = 4
};

enum TempScale
{
    Celsius,
    Fahrenheit,
    Kelvin
};

// TODO: implement ppm, percent, ibuGalPerLb,

/*!
 * \class Unit
 * \author Philip G. Lee
 *
 * \brief Interface for arbitrary physical units and their formatting.
 */
class Unit : public QObject
{
   Q_OBJECT

   Q_ENUMS(unitDisplay)
   Q_ENUMS(unitScale)
   Q_ENUMS(UnitType)

   public:
      // Did you know you need these to be *INSIDE* the class definition for
      // Qt to see them?
      enum unitDisplay
      {
         noUnit         = -1,
         displayDef     = 0x000,
         displaySI      = 0x100,
         displayUS      = 0x101,
         displayImp     = 0x102,
         displaySrm     = 0x200,
         displayEbc     = 0x201,
         displaySg      = 0x300,
         displayPlato   = 0x301,
         displayLintner = 0x400,
         displayWK      = 0x401
      };

      enum unitScale
      {
         noScale = -1,
         scaleExtraSmall = 0,
         scaleSmall = 1,
         scaleMedium = 2,
         scaleLarge = 3,
         scaleExtraLarge = 4,
         scaleHuge = 5,
         scaleWithout=1000
      };

      enum UnitType
      {
         Mass           = 0x100000,
         Volume         = 0x200000,
         Time           = 0x300000,
         Temp           = 0x400000,
         Color          = 0x500000,
         Density        = 0x600000,
         String         = 0x700000,
         Mixed          = 0x800000,
         DiastaticPower = 0x900000,
         None           = 0x000000
      };

      virtual ~Unit() {}
      virtual double toSI( double amt ) const =0;// { return amt; }
      virtual double fromSI( double amt ) const =0;// { return amt; }
      // The unit name will be the singular of the commonly used abbreviation.

      const QString getUnitName() const { return unitName; }
      const QString getSIUnitName() const { return SIUnitName; }

      const Unit::UnitType getUnitType() const { return _type; };
      const int getUnitOrTempSystem() const { return _unitSystem; };

      const double boundary() const { return 1.0; };

      static Unit* getUnit(QString& name, bool matchCurrentSystem = true);
      static QString convert(QString qstr, QString toUnit);


   protected:
      UnitType _type;
      int _unitSystem;
      QString unitName;
      QString SIUnitName;

   private:

      static QMultiMap<QString, Unit*> nameToUnit;
      static bool isMapSetup;
      static void setupMap();
      static QString unitFromString(QString qstr);
      static double valueFromString(QString qstr);

};

// ================ Weight/Mass ================
class KilogramUnit : public Unit
{
   public:
      KilogramUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;

};

class GramUnit : public Unit
{
   public:
      GramUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class MilligramUnit : public Unit
{
   public:
      MilligramUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class PoundUnit : public Unit
{
   public:
      PoundUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class OunceUnit : public Unit
{
   public:
      OunceUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

// ================ Volume ================
class LiterUnit : public Unit
{
   public:
      LiterUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class MilliliterUnit : public Unit
{
   public:
      MilliliterUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class USBarrelUnit : public Unit
{
   public:
      USBarrelUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class USGallonUnit : public Unit
{
   public:
      USGallonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class USQuartUnit : public Unit
{
   public:
      USQuartUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class USCupUnit : public Unit
{
   public:
      USCupUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const double boundary() const { return 0.25; } // override the default

};

class ImperialBarrelUnit : public Unit
{
   public:
      ImperialBarrelUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class ImperialGallonUnit : public Unit
{
   public:
      ImperialGallonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class ImperialQuartUnit : public Unit
{
   public:
      ImperialQuartUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class ImperialCupUnit : public Unit
{
   public:
      ImperialCupUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const double boundary() const { return 0.25; }

};

class ImperialTablespoonUnit : public Unit
{
   public:
      ImperialTablespoonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class ImperialTeaspoonUnit : public Unit
{
   public:
      ImperialTeaspoonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class USTablespoonUnit : public Unit
{
   public:
      USTablespoonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class USTeaspoonUnit : public Unit
{
   public:
      USTeaspoonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

// ================ Time ================
class SecondUnit : public Unit
{
   public:
      SecondUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const double boundary() const { return 90.0; };

};

class MinuteUnit : public Unit
{
   public:
      MinuteUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class HourUnit : public Unit
{
   public:
      HourUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const double boundary() const { return 2.0; }

};

class DayUnit: public Unit
{
public:
   DayUnit();

   // Inherited methods.
   double toSI( double amt ) const;
   double fromSI( double amt ) const;
};

// ================ Temperature ================

class CelsiusUnit : public Unit
{
   public:
      CelsiusUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class KelvinUnit : public Unit
{
   public:
      KelvinUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

class FahrenheitUnit : public Unit
{
   public:
      FahrenheitUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
};

// ================ Color =======================
// I will consider the standard unit of color
// to be SRM.
class SRMUnit : public Unit
{
public:
   SRMUnit();

   // Inherited methods.
   double toSI( double amt ) const;
   double fromSI( double amt ) const;
};

class EBCUnit : public Unit
{
public:
   EBCUnit();

   // Inherited methods.
   double toSI( double amt ) const;
   double fromSI( double amt ) const;
};

// ================ Density =======================
// Specific gravity (aka, Sg) will be the standard unit, since that is how we
// store things in teh database.
//
class SgUnit : public Unit
{
public:
   SgUnit();

   // Inherited methods.
   double toSI( double amt ) const;
   double fromSI( double amt ) const;
};

class PlatoUnit : public Unit
{
public:
   PlatoUnit();

   // Inherited methods.
   double toSI( double amt ) const;
   double fromSI( double amt ) const;
};

// == diastatic power ==
// Lintner will be the standard unit, since that is how we store things in the
// database.
//
class LintnerUnit : public Unit
{
public:
   LintnerUnit();

   // Inherited methods.
   double toSI( double amt ) const;
   double fromSI( double amt ) const;
};

class WKUnit : public Unit
{
public:
   WKUnit();

   // Inherited methods.
   double toSI( double amt ) const;
   double fromSI( double amt ) const;
};


class Units
{
public:
   Units();

   // === Mass ===
   static KilogramUnit *kilograms;
   static GramUnit *grams;
   static MilligramUnit *milligrams;
   static PoundUnit *pounds;
   static OunceUnit *ounces;
   // === Volume ===
   static LiterUnit *liters;
   static MilliliterUnit *milliliters;
   static USBarrelUnit *us_barrels;
   static USGallonUnit *us_gallons;
   static USQuartUnit *us_quarts;
   static USCupUnit *us_cups;
   static ImperialBarrelUnit *imperial_barrels;
   static ImperialGallonUnit *imperial_gallons;
   static ImperialQuartUnit *imperial_quarts;
   static ImperialCupUnit *imperial_cups;
   static USTablespoonUnit *us_tablespoons;
   static USTeaspoonUnit *us_teaspoons;
   static ImperialTablespoonUnit *imperial_tablespoons;
   static ImperialTeaspoonUnit *imperial_teaspoons;
   // === Time ===
   static SecondUnit *seconds;
   static MinuteUnit *minutes;
   static HourUnit *hours;
   static DayUnit *days;
   // === Temperature ===
   static CelsiusUnit *celsius;
   static FahrenheitUnit *fahrenheit;
   static KelvinUnit *kelvin;
   // === Color ===
   static SRMUnit *srm;
   static EBCUnit *ebc;
   // == Density ===
   static SgUnit *sp_grav;
   static PlatoUnit *plato;
   // == diastatic power ==
   static LintnerUnit *lintner;
   static WKUnit *wk;
};

#endif // _UNIT_H
