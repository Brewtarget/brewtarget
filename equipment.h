/*
 * equipment.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _EQUIPMENT_H
#define	_EQUIPMENT_H

#include <string>
#include <exception>
#include <ostream>

#include "xmlnode.h"

class Equipment;
class EquipmentException;

class Equipment
{
public:
   
   Equipment();
   Equipment(XmlNode *node);
   
   std::string toXml();

   // Operators
   friend bool operator<(Equipment &e1, Equipment &e2);

   // Set
   void setName( const std::string &var );
   void setBoilSize_l( double var );
   void setBatchSize_l( double var );
   void setTunVolume_l( double var );
   void setTunWeight_kg( double var );
   void setTunSpecificHeat_calGC( double var );
   void setTopUpWater_l( double var );
   void setTrubChillerLoss_l( double var );
   void setEvapRate_pctHr( double var );
   void setBoilTime_hrs( double var );
   void setCalcBoilVolume( bool var );
   void setLauterDeadspace_l( double var );
   void setTopUpKettle_l( double var );
   void setHopUtilization_pct( double var );
   void setNotes( const std::string &var );

   // Get
   std::string getName() const;
   double getBoilSize_l() const;
   double getBatchSize_l() const;
   double getTunVolume_l() const;
   double getTunWeight_kg() const;
   double getTunSpecificHeat_calGC() const;
   double getTopUpWater_l() const;
   double getTrubChillerLoss_l() const;
   double getEvapRate_pctHr() const;
   double getBoilTime_hrs() const;
   bool getCalcBoilVolume() const;
   double getLauterDeadspace_l() const;
   double getTopUpKettle_l() const;
   double getHopUtilization_pct() const;
   std::string getNotes() const;

private:
   std::string name;
   static const int version = 1;
   double boilSize_l;
   double batchSize_l;
   double tunVolume_l;
   double tunWeight_kg;
   double tunSpecificHeat_calGC;
   double topUpWater_l;
   double trubChillerLoss_l;
   double evapRate_pctHr;
   double boilTime_hrs;
   bool calcBoilVolume;
   double lauterDeadspace_l;
   double topUpKettle_l;
   double hopUtilization_pct;
   std::string notes;

   void setDefaults();
};

class EquipmentException : public std::exception
{
public:
   
   virtual const char* what() const throw()
   {
      // Note: this temporary object might get destroyed too early.
      // I'm not really sure.
      return std::string("BeerXml EQUIPMENT error: " + _err + "\n").c_str();
   }
   
   EquipmentException( std::string message )
   {
      _err = message;
   }
   
   ~EquipmentException() throw() {}
   
private:
   
   std::string _err;
};

struct Equipment_ptr_cmp
{
   bool operator()( Equipment* lhs, Equipment* rhs)
   {
      return *lhs < *rhs;
   }
};

#endif	/* _EQUIPMENT_H */

