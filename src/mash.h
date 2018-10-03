/*
 * mash.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Kregg K <gigatropolis@yahoo.com>
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

#include "BeerXMLElement.h"

// Forward declarations.
class Mash;
class MashStep;
bool operator<(Mash &m1, Mash &m2);
bool operator==(Mash &m1, Mash &m2);

/*!
 * \class Mash
 * \author Philip G. Lee
 *
 * \brief Model class for a mash record in the database.
 */
class Mash : public BeerXMLElement
{
   Q_OBJECT
   Q_CLASSINFO("signal", "mashs")
   Q_CLASSINFO("prefix", "mash")
   
   friend class Database;
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
   
   // Calculated getters
   double totalMashWater_l();
   double totalTime();
   
   // Relational getters
   QList<MashStep*> mashSteps() const;
   
   // NOTE: should this be completely in Database?
   void removeAllMashSteps();

   static QString classNameStr();

public slots:
   void acceptMashStepChange(QMetaProperty, QVariant);
   
signals:
   //! \brief Emitted when \c name() changes.
   void changedName(QString);
   
   // Emitted when the number of steps change, or when you should call mashSteps() again.
   void mashStepsChanged();
   
private:
   Mash(Brewtarget::DBTable table, int key);
   Mash( Mash const& other );
   
   // Get via the relational relationship.
   //QVector<MashStep *> mashSteps;
   
   static QHash<QString,QString> tagToProp;
   static QHash<QString,QString> tagToPropHash();

};

Q_DECLARE_METATYPE( Mash* )

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

#endif //_MASH_H
