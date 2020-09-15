/*
 * water.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#ifndef _WATER_H
#define _WATER_H

#include <QString>
#include "ingredient.h"

// Forward declarations.
class Water;
bool operator<(Water &w1, Water &w2);
bool operator==(Water &w1, Water &w2);

/*!
 * \class Water
 * \author Philip G. Lee
 *
 * \brief Model for water records in the database.
 */
class Water : public Ingredient
{
   Q_OBJECT
   Q_CLASSINFO("signal", "waters")

   friend class Database;
   friend class BeerXML;
   friend class WaterDialog;
   friend class WaterEditor;
public:

   enum Types {
      NONE,
      BASE,
      TARGET
   };

   enum Ions {
      Ca,
      Cl,
      HCO3,
      Mg,
      Na,
      SO4,
      numIons
   };

   Q_ENUM(Types Ions)

   virtual ~Water() {}

   // On a base or target profile, bicarbonate and alkalinity cannot both be used. I'm gonna have fun figuring that out
   //! \brief The amount in liters.
   Q_PROPERTY( double amount READ amount WRITE setAmount /*NOTIFY changed*/ /*changedAmount_l*/ )
   //! \brief The ppm of calcium.
   Q_PROPERTY( double calcium_ppm READ calcium_ppm WRITE setCalcium_ppm /*NOTIFY changed*/ /*changedCalcium_ppm*/ )
   //! \brief The ppm of bicarbonate.
   Q_PROPERTY( double bicarbonate_ppm READ bicarbonate_ppm WRITE setBicarbonate_ppm /*NOTIFY changed*/ /*changedBicarbonate_ppm*/ )
   //! \brief The ppm of sulfate.
   Q_PROPERTY( double sulfate_ppm READ sulfate_ppm WRITE setSulfate_ppm /*NOTIFY changed*/ /*changedSulfate_ppm*/ )
   //! \brief The ppm of chloride.
   Q_PROPERTY( double chloride_ppm READ chloride_ppm WRITE setChloride_ppm /*NOTIFY changed*/ /*changedChloride_ppm*/ )
   //! \brief The ppm of sodium.
   Q_PROPERTY( double sodium_ppm READ sodium_ppm WRITE setSodium_ppm /*NOTIFY changed*/ /*changedSodium_ppm*/ )
   //! \brief The ppm of magnesium.
   Q_PROPERTY( double magnesium_ppm READ magnesium_ppm WRITE setMagnesium_ppm /*NOTIFY changed*/ /*changedMagnesium_ppm*/ )
   //! \brief The pH.
   Q_PROPERTY( double ph READ ph WRITE setPh /*NOTIFY changed*/ /*changedPh*/ )
   //! \brief The residual alkalinity
   Q_PROPERTY( double alkalinity READ alkalinity WRITE setAlkalinity /*NOTIFY changed*/ /*changedAlkalinity*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief What kind of water is this
   Q_PROPERTY( Water::Types type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief percent of the mash water that is RO
   Q_PROPERTY( double mashRO READ mashRO WRITE setMashRO /*NOTIFY changed*/ /*changedMashRO*/ )
   //! \brief percent of the sparge water that is RO
   Q_PROPERTY( double spargeRO READ spargeRO WRITE setSpargeRO /*NOTIFY changed*/ /*changedSpargeRO*/ )
   //! \brief is the alkalinity measured as HCO3 or CO3?
   Q_PROPERTY( bool alkalinityAsHCO3 READ alkalinityAsHCO3 WRITE setAlkalinityAsHCO3 /*NOTIFY changed*/ /*changedSpargeRO*/ )

   double amount() const;
   double calcium_ppm() const;
   double bicarbonate_ppm() const;
   double sulfate_ppm() const;
   double chloride_ppm() const;
   double sodium_ppm() const;
   double magnesium_ppm() const;
   double ph() const;
   double alkalinity() const;
   QString notes() const;
   bool cacheOnly() const;
   Water::Types type() const;
   double mashRO() const;
   double spargeRO() const;
   bool alkalinityAsHCO3() const;

   double ppm( Water::Ions ion );
   void setAmount( double var );
   void setCalcium_ppm( double var );
   void setSulfate_ppm( double var );
   void setBicarbonate_ppm( double var );
   void setChloride_ppm( double var );
   void setSodium_ppm( double var );
   void setMagnesium_ppm( double var );
   void setPh( double var );
   void setAlkalinity(double var);
   void setNotes( const QString &var );
   void setCacheOnly( bool cache );
   void setType(Types type);
   void setMashRO(double var);
   void setSpargeRO(double var);
   void setAlkalinityAsHCO3(bool var);

   static QString classNameStr();

signals:


private:
   Water(Brewtarget::DBTable table, int key);
   Water(Brewtarget::DBTable table, int key, QSqlRecord rec);
   Water(Water const& other, bool cache = true);
   Water(QString name, bool cache = true);

   double m_amount;
   double m_calcium_ppm;
   double m_bicarbonate_ppm;
   double m_sulfate_ppm;
   double m_chloride_ppm;
   double m_sodium_ppm;
   double m_magnesium_ppm;
   double m_ph;
   double m_alkalinity;
   QString m_notes;
   bool m_cacheOnly;
   Water::Types m_type;
   double m_mash_ro;
   double m_sparge_ro;
   bool m_alkalinity_as_hco3;

};

Q_DECLARE_METATYPE( QList<Water*> )

inline bool WaterPtrLt( Water* lhs, Water* rhs)
{
   return *lhs < *rhs;
}

inline bool WaterPtrEq( Water* lhs, Water* rhs)
{
   return *lhs == *rhs;
}

struct Water_ptr_cmp
{
   bool operator()( Water* lhs, Water* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Water_ptr_equals
{
   bool operator()( Water* lhs, Water* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif   /* _WATER_H */

