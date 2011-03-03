/*
 * equipment.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _EQUIPMENT_H
#define	_EQUIPMENT_H

#include <string>
#include <exception>
#include <ostream>
#include <QDomNode>
#include "observable.h"
#include "BeerXMLElement.h"

class Equipment;
class EquipmentException;

class Equipment : public Observable, public BeerXMLElement
{
public:
   
   Equipment();
   Equipment(const QDomNode& equipmentNode);
   
   virtual void fromNode(const QDomNode& node); // From BeerXMLElement
   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement

   // Operators
   friend bool operator<(Equipment &e1, Equipment &e2);
   friend bool operator==(Equipment &e1, Equipment &e2);

   // Set
   void setName( const QString &var );
   void setBoilSize_l( double var );
   void setBatchSize_l( double var );
   void setTunVolume_l( double var );
   void setTunWeight_kg( double var );
   void setTunSpecificHeat_calGC( double var );
   void setTopUpWater_l( double var );
   void setTrubChillerLoss_l( double var );
   void setEvapRate_pctHr( double var );
   void setEvapRate_lHr( double var ); // Use this one.
   void setBoilTime_min( double var );
   void setCalcBoilVolume( bool var );
   void setLauterDeadspace_l( double var );
   void setTopUpKettle_l( double var );
   void setHopUtilization_pct( double var );
   void setNotes( const QString &var );
   void setGrainAbsorption_LKg(double var);

   // Get
   QString getName() const;
   double getBoilSize_l() const;
   double getBatchSize_l() const;
   double getTunVolume_l() const;
   double getTunWeight_kg() const;
   double getTunSpecificHeat_calGC() const;
   double getTopUpWater_l() const;
   double getTrubChillerLoss_l() const;
   double getEvapRate_pctHr() const;
   double getEvapRate_lHr() const; // Use this one.
   double getBoilTime_min() const;
   bool getCalcBoilVolume() const;
   double getLauterDeadspace_l() const;
   double getTopUpKettle_l() const;
   double getHopUtilization_pct() const;
   QString getNotes() const;
   double getGrainAbsorption_LKg();

   double wortEndOfBoil_l( double kettleWort_l ) const; // Calculate how much wort is left immediately at knockout.

private:
   QString name;
   static const int version = 1;
   double boilSize_l;
   double batchSize_l;
   double tunVolume_l;
   double tunWeight_kg;
   double tunSpecificHeat_calGC;
   double topUpWater_l;
   double trubChillerLoss_l;
   double evapRate_pctHr;
   double evapRate_lHr;
   double boilTime_min;
   bool calcBoilVolume;
   double lauterDeadspace_l;
   double topUpKettle_l;
   double hopUtilization_pct;
   QString notes;
   // My extensions below.
   double absorption_LKg;

   void setDefaults();
   void doCalculations();
};

inline bool EquipmentPtrLt( Equipment* lhs, Equipment* rhs)
{
   return *lhs < *rhs;
}

inline bool EquipmentPtrEq( Equipment* lhs, Equipment* rhs)
{
   return *lhs == *rhs;
}

struct Equipment_ptr_cmp
{
   bool operator()( Equipment* lhs, Equipment* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Equipment_ptr_equals
{
   bool operator()( Equipment* lhs, Equipment* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif	/* _EQUIPMENT_H */

