/*
 * mash.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef _MASH_H
#define _MASH_H

#include "model/NamedEntity.h"
namespace PropertyNames::Mash { static char const * const ph = "ph"; /* previously kpropPH */ }
namespace PropertyNames::Mash { static char const * const tunSpecificHeat_calGC = "tunSpecificHeat_calGC"; /* previously kpropTunSpecHeat */ }
namespace PropertyNames::Mash { static char const * const tunWeight_kg = "tunWeight_kg"; /* previously kpropTunWeight */ }
namespace PropertyNames::MashStep { static char const * const typeString = "typeString"; /* previously kpropTypeString */ }
namespace PropertyNames::MashStep { static char const * const type = "type"; /* previously kpropType */ }
namespace PropertyNames::Mash { static char const * const notes = "notes"; /* previously kpropNotes */ }
namespace PropertyNames::Mash { static char const * const equipAdjust = "equipAdjust"; /* previously kpropEquipAdjust */ }
namespace PropertyNames::Mash { static char const * const spargeTemp_c = "spargeTemp_c"; /* previously kpropSpargeTemp */ }
namespace PropertyNames::Mash { static char const * const tunTemp_c = "tunTemp_c"; /* previously kpropTunTemp */ }
namespace PropertyNames::Mash { static char const * const grainTemp_c = "grainTemp_c"; /* previously kpropGrainTemp */ }

// Forward declarations.
class MashStep;

/*!
 * \class Mash
 *
 * \brief Model class for a mash record in the database.
 */
class Mash : public NamedEntity
{
   Q_OBJECT
   Q_CLASSINFO("signal", "mashs")

   friend class Database;
   friend class BeerXML;
   friend class MashDesigner;
   friend class MashEditor;
public:

   virtual ~Mash() {}

   //! \brief The initial grain temp in Celsius.
   Q_PROPERTY( double grainTemp_c READ grainTemp_c WRITE setGrainTemp_c /*NOTIFY changed*/ /*changedGrainTemp_c*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief The initial tun temp in Celsius.
   Q_PROPERTY( double tunTemp_c READ tunTemp_c WRITE setTunTemp_c /*NOTIFY changed*/ /*changedTunTemp_c*/ )
   //! \brief The sparge temp in C.
   Q_PROPERTY( double spargeTemp_c READ spargeTemp_c WRITE setSpargeTemp_c /*NOTIFY changed*/ /*changedSpargeTemp_c*/ )
   //! \brief The pH.
   Q_PROPERTY( double ph READ ph WRITE setPh /*NOTIFY changed*/ /*changedPh*/ )
   //! \brief The mass of the tun in kg.
   Q_PROPERTY( double tunWeight_kg READ tunWeight_kg WRITE setTunWeight_kg /*NOTIFY changed*/ /*changedTunWeight_kg*/ )
   //! \brief The tun's specific heat in kcal/(g*C).
   Q_PROPERTY( double tunSpecificHeat_calGC READ tunSpecificHeat_calGC WRITE setTunSpecificHeat_calGC /*NOTIFY changed*/ /*changedTunSpecificHeat_calGC*/ )
   //! \brief Whether to adjust strike temperatures to account for the tun.
   Q_PROPERTY( bool equipAdjust READ equipAdjust WRITE setEquipAdjust /*NOTIFY changed*/ /*changedEquipAdjust*/ )
   //! \brief The total water that went into the mash in liters. Calculated.
   Q_PROPERTY( double totalMashWater_l READ totalMashWater_l /*WRITE*/ /*NOTIFY changed*/ /*changedTotalMashWater_l*/ STORED false )
   //! \brief The total mash time in minutes. Calculated.
   Q_PROPERTY( double totalTime READ totalTime /*NOTIFY changed*/ /*changedTotalTime*/ STORED false )
  // Q_PROPERTY( double tunMass_kg READ tunMass_kg  WRITE setTunMass_kg /*NOTIFY changed*/ /*changedTotalTime*/ )
   //! \brief The individual mash steps.
   Q_PROPERTY( QList<MashStep*> mashSteps  READ mashSteps /*WRITE*/ /*NOTIFY changed*/ /*changedTotalTime*/ STORED false )

   // Setters
   void setGrainTemp_c( double var );
   void setNotes( const QString &var );
   void setTunTemp_c( double var );
   void setSpargeTemp_c( double var );
   void setPh( double var );
   void setTunWeight_kg( double var );
   void setTunSpecificHeat_calGC( double var );
   void setEquipAdjust( bool var );
   void setCacheOnly( bool cache );

   // Getters
   double grainTemp_c() const;
   unsigned int numMashSteps() const;
   QString notes() const;
   double tunTemp_c() const;
   double spargeTemp_c() const;
   double ph() const;
   double tunWeight_kg() const;
   double tunSpecificHeat_calGC() const;
   bool equipAdjust() const;
   bool cacheOnly() const;

   // Calculated getters
   //! \brief all the mash water, sparge and strike
   double totalMashWater_l();
   //! \brief all the infusion water, excluding sparge
   double totalInfusionAmount_l() const;
   //! \brief all the sparge water
   double totalSpargeAmount_l() const;
   double totalTime();

   bool hasSparge() const;

   // Relational getters
   QList<MashStep*> mashSteps() const;

   // NOTE: should this be completely in Database?
   void removeAllMashSteps();

   static QString classNameStr();

   // Mash objects do not have parents
   NamedEntity * getParent() { return nullptr; }
   virtual int insertInDatabase();
   virtual void removeFromDatabase();

public slots:
   void acceptMashStepChange(QMetaProperty, QVariant);
   MashStep * addMashStep(MashStep * mashStep);
   MashStep * removeMashStep(MashStep * mashStep);

signals:
   // Emitted when the number of steps change, or when you should call mashSteps() again.
   void mashStepsChanged();

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

private:
   Mash(Brewtarget::DBTable table, int key);
   Mash(Brewtarget::DBTable table, int key, QSqlRecord rec);
   Mash( Mash const& other );
public:
   Mash( QString name, bool cache = true );

private:
   double m_grainTemp_c;
   QString m_notes;
   double m_tunTemp_c;
   double m_spargeTemp_c;
   double m_ph;
   double m_tunWeight_kg;
   double m_tunSpecificHeat_calGC;
   bool m_equipAdjust;
   bool m_cacheOnly;

   QList<MashStep*> m_mashSteps;

};

Q_DECLARE_METATYPE( Mash* )
/*
inline bool MashPtrLt( Mash* lhs, Mash* rhs)
{
   return *lhs < *rhs;
}

inline bool MashPtrEq( Mash* lhs, Mash* rhs)
{
   return *lhs == *rhs;
}

struct Mash_ptr_cmp
{
   bool operator()( Mash* lhs, Mash* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Mash_ptr_equals
{
   bool operator()( Mash* lhs, Mash* rhs )
   {
      return *lhs == *rhs;
   }
};
*/
#endif //_MASH_H
