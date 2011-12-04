/*
 * mash.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#ifndef _MASH_H
#define _MASH_H

#include "mashstep.h"
#include <QDomNode>
#include "BeerXMLElement.h"

// Forward declarations.
class Mash;
bool operator<(Mash &m1, Mash &m2);
bool operator==(Mash &m1, Mash &m2);

class Mash : public BeerXMLElement
{
   Q_OBJECT
   
   friend class Database;
public:

   virtual ~Mash() {}

   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
   
   Q_PROPERTY( QString name READ name WRITE setName NOTIFY changed /*changedName*/ )
   Q_PROPERTY( double grainTemp_c READ grainTemp_c WRITE setGrainTemp_c NOTIFY changed /*changedGrainTemp_c*/ )
   Q_PROPERTY( QString notes READ notes WRITE setNotes NOTIFY changed /*changedNotes*/ )
   Q_PROPERTY( double tunTemp_c READ tunTemp_c WRITE setTunTemp_c NOTIFY changed /*changedTunTemp_c*/ )
   Q_PROPERTY( double spargeTemp_c READ spargeTemp_c WRITE setSpargeTemp_c NOTIFY changed /*changedSpargeTemp_c*/ )
   Q_PROPERTY( double ph READ ph WRITE setPh NOTIFY changed /*changedPh*/ )
   Q_PROPERTY( double tunWeight_kg READ tunWeight_kg WRITE setTunWeight_kg NOTIFY changed /*changedTunWeight_kg*/ )
   Q_PROPERTY( double tunSpecificHeat_calGC READ tunSpecificHeat_calGC WRITE setTunSpecificHeat_calGC NOTIFY changed /*changedTunSpecificHeat_calGC*/ )
   Q_PROPERTY( bool equipAdjust READ equipAdjust WRITE setEquipAdjust NOTIFY changed /*changedEquipAdjust*/ )
   Q_PROPERTY( double totalMashWater_l READ totalMashWater_l /*WRITE*/ NOTIFY changed /*changedTotalMashWater_l*/ STORED false )
   Q_PROPERTY( double totalTime READ totalTime /*WRITE*/ NOTIFY changed /*changedTotalTime*/ STORED false )
   
   Q_PROPERTY( QList<MashStep*> mashSteps  READ mashSteps /*WRITE*/ NOTIFY changed /*changedTotalTime*/ STORED false )
   
   // Setters
   void setName( const QString &var );
   void setGrainTemp_c( double var );
   void setNotes( const QString &var );
   void setTunTemp_c( double var );
   void setSpargeTemp_c( double var );
   void setPh( double var );
   void setTunWeight_kg( double var );
   void setTunSpecificHeat_calGC( double var );
   void setEquipAdjust( bool var );

   // Getters
   QString name() const;
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
   double totalMashWater_l() const; // Total amount of water that went INTO the mash.
   double getTotalTime();
   
   // Relational getters
   QList<MashStep*> mashSteps();
   
   // TODO: is this the right place for these? Probably move to Database.
   void addMashStep(MashStep* step);
   void removeMashStep(MashStep* step);
   void removeAllMashSteps();
   void swapSteps( unsigned int i, unsigned int j );

signals:
   /*
   void changedName(QString);
   void changedGrainTemp_c(double);
   void changedTunTemp_c(double);
   void changedSpargeTemp_c(double);
   void changedPh(double);
   void changedTunWeight_kg(double);
   void changedTunSpecificHeat_calGC(double);
   void changedEquipAdjust(bool);
   void changedTotalMashWater_l(double);
   void changedTotalTime(double);
   */
   
private:
   Mash();
   Mash( Mash const& other );
   
   // Get via the relational relationship.
   //QVector<MashStep *> mashSteps;

};

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
