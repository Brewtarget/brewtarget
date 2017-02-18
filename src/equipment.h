/*
 * equipment.h is part of Brewtarget, and is Copyright the following
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

#ifndef _EQUIPMENT_H
#define _EQUIPMENT_H

#include <QDomNode>
#include "BeerXMLElement.h"

/*!
 * \class Equipment
 * \author Philip G. Lee
 *
 * \brief Model representing a single equipment record.
 */
class Equipment : public BeerXMLElement
{
   Q_OBJECT

   Q_CLASSINFO("signal", "equipments")
   Q_CLASSINFO("prefix", "equipment")
   
   friend class Database;
public:

   virtual ~Equipment() {}
   
   //! \brief The boil size in liters.
   Q_PROPERTY( double boilSize_l            READ boilSize_l            WRITE setBoilSize_l            NOTIFY changedBoilSize_l )
   //! \brief The batch size in liters.
   Q_PROPERTY( double batchSize_l           READ batchSize_l           WRITE setBatchSize_l           NOTIFY changedBatchSize_l )
   //! \brief The tun volume in liters.
   Q_PROPERTY( double tunVolume_l           READ tunVolume_l           WRITE setTunVolume_l           NOTIFY changedTunVolume_l )
   //! \brief Set the tun mass in kg.
   Q_PROPERTY( double tunWeight_kg          READ tunWeight_kg          WRITE setTunWeight_kg          NOTIFY changedTunWeight_kg )
   //! \brief Set the tun specific heat in kcal/(g*C)
   Q_PROPERTY( double tunSpecificHeat_calGC READ tunSpecificHeat_calGC WRITE setTunSpecificHeat_calGC NOTIFY changedTunSpecificHeat_calGC )
   //! \brief Set the top-up water in liters.
   Q_PROPERTY( double topUpWater_l          READ topUpWater_l          WRITE setTopUpWater_l          NOTIFY changedTopUpWater_l )
   //! \brief Set the loss to trub and chiller in liters.
   Q_PROPERTY( double trubChillerLoss_l     READ trubChillerLoss_l     WRITE setTrubChillerLoss_l     NOTIFY changedTrubChillerLoss_l )
   //! \brief Set the evaporation rate in percent of the boil size per hour. DO NOT USE. Only for BeerXML compatibility.
   Q_PROPERTY( double evapRate_pctHr        READ evapRate_pctHr        WRITE setEvapRate_pctHr        NOTIFY changedEvapRate_pctHr )
   //! \brief Set the evaporation rate in liters/hr.
   Q_PROPERTY( double evapRate_lHr          READ evapRate_lHr          WRITE setEvapRate_lHr          NOTIFY changedEvapRate_lHr )
   //! \brief Set the boil time in minutes.
   Q_PROPERTY( double boilTime_min          READ boilTime_min          WRITE setBoilTime_min          NOTIFY changedBoilTime_min )
   //! \brief Set whether you want the boil volume to be automatically calculated.
   Q_PROPERTY( bool calcBoilVolume          READ calcBoilVolume        WRITE setCalcBoilVolume        NOTIFY changedCalcBoilVolume )
   //! \brief Set the lauter tun's deadspace in liters.
   Q_PROPERTY( double lauterDeadspace_l     READ lauterDeadspace_l     WRITE setLauterDeadspace_l     NOTIFY changedLauterDeadspace_l )
   //! \brief Set the kettle top up in liters.
   Q_PROPERTY( double topUpKettle_l         READ topUpKettle_l         WRITE setTopUpKettle_l         NOTIFY changedTopUpKettle_l )
   //! \brief Set the hop utilization factor. I do not believe this is used.
   Q_PROPERTY( double hopUtilization_pct    READ hopUtilization_pct    WRITE setHopUtilization_pct    NOTIFY changedHopUtilization_pct )
   //! \brief Set the notes.
   Q_PROPERTY( QString notes                READ notes                 WRITE setNotes                 NOTIFY changedNotes )
   //! \brief Set how much water the grains absorb in liters/kg.
   Q_PROPERTY( double grainAbsorption_LKg   READ grainAbsorption_LKg   WRITE setGrainAbsorption_LKg   NOTIFY changedGrainAbsorption_LKg )
   //! \brief Set the boiling point of water in Celsius.
   Q_PROPERTY( double boilingPoint_c        READ boilingPoint_c        WRITE setBoilingPoint_c        NOTIFY changedBoilingPoint_c )

   // Set
   void setBoilSize_l( double var );
   void setBatchSize_l( double var );
   void setTunVolume_l( double var );
   void setTunWeight_kg( double var );
   void setTunSpecificHeat_calGC( double var );
   void setTopUpWater_l( double var );
   void setTrubChillerLoss_l( double var );
   void setEvapRate_pctHr( double var );
   void setEvapRate_lHr( double var );
   void setBoilTime_min( double var );
   void setCalcBoilVolume( bool var );
   void setLauterDeadspace_l( double var );
   void setTopUpKettle_l( double var );
   void setHopUtilization_pct( double var );
   void setNotes( const QString &var );
   void setGrainAbsorption_LKg(double var);
   void setBoilingPoint_c(double var);

   // Get
   double boilSize_l() const;
   double batchSize_l() const;
   double tunVolume_l() const;
   double tunWeight_kg() const;
   double tunSpecificHeat_calGC() const;
   double topUpWater_l() const;
   double trubChillerLoss_l() const;
   double evapRate_pctHr() const;
   double evapRate_lHr() const;
   double boilTime_min() const;
   bool calcBoilVolume() const;
   double lauterDeadspace_l() const;
   double topUpKettle_l() const;
   double hopUtilization_pct() const;
   QString notes() const;
   double grainAbsorption_LKg();
   double boilingPoint_c() const;

   //! \brief Calculate how much wort is left immediately at knockout.
   double wortEndOfBoil_l( double kettleWort_l ) const;

   static QString classNameStr();

signals:
   
   void changedName(QString);
   void changedBoilSize_l(double);
   void changedBatchSize_l(double);
   void changedTunVolume_l(double);
   void changedTunWeight_kg(double);
   void changedTunSpecificHeat_calGC(double);
   void changedTopUpWater_l(double);
   void changedTrubChillerLoss_l(double);
   void changedEvapRate_pctHr(double);
   void changedEvapRate_lHr(double);
   void changedBoilTime_min(double);
   void changedCalcBoilVolume(bool);
   void changedLauterDeadspace_l(double);
   void changedTopUpKettle_l(double);
   void changedHopUtilization_pct(double);
   void changedNotes(QString);
   void changedGrainAbsorption_LKg(double);
   void changedBoilingPoint_c(double);
   
private:
   Equipment(Brewtarget::DBTable table, int key);
   Equipment( Equipment const& other);
   
   // Calculate the boil size.
   void doCalculations();
   
   static QHash<QString,QString> tagToProp;
   static QHash<QString,QString> tagToPropHash();
};

Q_DECLARE_METATYPE( Equipment* )

bool operator<(Equipment &e1, Equipment &e2);
bool operator==(Equipment &e1, Equipment &e2);

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

#endif   /* _EQUIPMENT_H */

