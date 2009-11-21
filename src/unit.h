/*
 * unit.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

#include <QString>
#include <string>
#include <map>

enum UnitSystem
{
    SI,
    USCustomary,
    Imperial
};

enum TempScale
{
    Celsius,
    Fahrenheit
};

inline QString unitSystemToString(UnitSystem us)
{
   switch (us)
   {
      case SI: return "SI";
      case USCustomary: return "USCustomary";
      case Imperial: return "Imperial";
      default: return 0;
   }
}

inline QString tempScaleToString(TempScale ts)
{
   switch (ts)
   {
      case Celsius: return "Celsius";
      case Fahrenheit: return "Fahrenheit";
      default: return 0;
   }
}
// TODO: implement ppm, percent, diastatic power, ibuGalPerLb, gravity, srm, volumes.

class Unit
{
   public:
      // Seems these can't be PURE virtuals b/c of some issue with std::map.
      virtual double toSI( double amt ) const { return amt; };
      virtual double fromSI( double amt ) const { return amt; };
      // The unit name will be the singular of the commonly used abbreviation.
      virtual const std::string& getUnitName() const { return 0; };
      virtual const std::string& getSIUnitName() const { return 0; };

      static double convert( double amount, const std::string& fromUnit, const std::string& toUnit );
      static QString convert( QString qstr, QString toUnit );
      static double qstringToSI( QString qstr );

   private:
      static std::map<std::string, Unit*> nameToUnit;
      static bool isMapSetup;
      static void setupMap();
};

// ================ Weight/Mass ================
class KilogramUnit : public Unit
{
   public:
      KilogramUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class GramUnit : public Unit
{
   public:
      GramUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class MilligramUnit : public Unit
{
   public:
      MilligramUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class PoundUnit : public Unit
{
   public:
      PoundUnit();
      
      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }
      
   private:
      std::string unitName;
      std::string SIUnitName;
};

class OunceUnit : public Unit
{
   public:
      OunceUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

// ================ Volume ================
class LiterUnit : public Unit
{
   public:
      LiterUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class MilliliterUnit : public Unit
{
   public:
      MilliliterUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class USGallonUnit : public Unit
{
   public:
      USGallonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class USQuartUnit : public Unit
{
   public:
      USQuartUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class USCupUnit : public Unit
{
   public:
      USCupUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class ImperialGallonUnit : public Unit
{
   public:
      ImperialGallonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class ImperialQuartUnit : public Unit
{
   public:
      ImperialQuartUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class ImperialCupUnit : public Unit
{
   public:
      ImperialCupUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};


class TablespoonUnit : public Unit
{
   public:
      TablespoonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class TeaspoonUnit : public Unit
{
   public:
      TeaspoonUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

// ================ Time ================
class SecondUnit : public Unit
{
   public:
      SecondUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class MinuteUnit : public Unit
{
   public:
      MinuteUnit();
      
      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }
      
   private:
      std::string unitName;
      std::string SIUnitName;
};

class HourUnit : public Unit
{
   public:
      HourUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

// ================ Temperature ================

class CelsiusUnit : public Unit
{
   public:
      CelsiusUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class KelvinUnit : public Unit
{
   public:
      KelvinUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
};

class FahrenheitUnit : public Unit
{
   public:
      FahrenheitUnit();

      // Inherited methods.
      double toSI( double amt ) const;
      double fromSI( double amt ) const;
      const std::string& getUnitName() const { return unitName; }
      const std::string& getSIUnitName() const { return SIUnitName; }

   private:
      std::string unitName;
      std::string SIUnitName;
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
   static USGallonUnit *us_gallons;
   static USQuartUnit *us_quarts;
   static USCupUnit *us_cups;
   static ImperialGallonUnit *imperial_gallons;
   static ImperialQuartUnit *imperial_quarts;
   static ImperialCupUnit *imperial_cups;
   static TablespoonUnit *tablespoons;
   static TeaspoonUnit *teaspoons;
   // === Time ===
   static SecondUnit *seconds;
   static MinuteUnit *minutes;
   static HourUnit *hours;
   // === Temperature ===
   static CelsiusUnit *celsius;
   static FahrenheitUnit *fahrenheit;
   static KelvinUnit *kelvin;
};

#endif // _UNIT_H
