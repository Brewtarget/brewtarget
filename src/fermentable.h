/*
 * fermentable.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _FERMENTABLE_H
#define _FERMENTABLE_H

#include <QDomNode>
#include <QStringList>
#include <QString>
#include "BeerXMLElement.h"

// Forward declarations.
class Fermentable;
bool operator<(Fermentable &f1, Fermentable &f2);
bool operator==(Fermentable &f1, Fermentable &f2);

class Fermentable : public BeerXMLElement
{
   Q_OBJECT
   friend class Brewtarget;
   friend class Database;
public:

   enum Type { TYPEGRAIN=0, TYPESUGAR, TYPEEXTRACT, TYPEDRY_EXTRACT, TYPEADJUNCT, NUMTYPES };
   Q_ENUMS( TYPE )
   
   virtual ~Fermentable() {}
   
   // New Q_PROPERTIES
   Q_PROPERTY( QString name                  READ name                   WRITE setName                   NOTIFY changed /*changedName*/ )
   Q_PROPERTY( Type type                     READ type                   WRITE setType                   NOTIFY changed /*changedType*/ )
   Q_PROPERTY( QString typeString            READ typeString             /*WRITE*/                       NOTIFY changed /*changedTypeString*/             STORED false )
   Q_PROPERTY( QString typeStringTr          READ typeStringTr           /*WRITE*/                       NOTIFY changed /*changedTypeStringTr*/           STORED false )
   Q_PROPERTY( double amount_kg              READ amount_kg              WRITE setAmount_kg              NOTIFY changed /*changedAmount_kg*/ )
   Q_PROPERTY( double yield_pct              READ yield_pct              WRITE setYield_pct              NOTIFY changed /*changedYield_pct*/ )
   Q_PROPERTY( double color_srm              READ color_srm              WRITE setColor_srm              NOTIFY changed /*changedColor_srm*/ )
   Q_PROPERTY( bool addAfterBoil             READ addAfterBoil           WRITE setAddAfterBoil           NOTIFY changed /*changedAddAfterBoil*/ )
   Q_PROPERTY( QString origin                READ origin                 WRITE setOrigin                 NOTIFY changed /*changedOrigin*/ )
   Q_PROPERTY( QString supplier              READ supplier               WRITE setSupplier               NOTIFY changed /*changedSupplier*/ )
   Q_PROPERTY( QString notes                 READ notes                  WRITE setNotes                  NOTIFY changed /*changedNotes*/ )
   Q_PROPERTY( double coarseFineDiff_pct     READ coarseFineDiff_pct     WRITE setCoarseFineDiff_pct     NOTIFY changed /*changedCoarseFineDiff_pct*/ )
   Q_PROPERTY( double moisture_pct           READ moisture_pct           WRITE setMoisture_pct           NOTIFY changed /*changedMoisture_pct*/ )
   Q_PROPERTY( double diastaticPower_lintner READ diastaticPower_lintner WRITE setDiastaticPower_lintner NOTIFY changed /*changedDiastaticPower_lintner*/ )
   Q_PROPERTY( double protein_pct            READ protein_pct            WRITE setProtein_pct            NOTIFY changed /*changedProtein_pct*/ )
   Q_PROPERTY( double maxInBatch_pct         READ maxInBatch_pct         WRITE setMaxInBatch_pct         NOTIFY changed /*changedMaxInBatch_pct*/ )
   Q_PROPERTY( bool recommendMash            READ recommendMash          WRITE setRecommendMash          NOTIFY changed /*changedRecommendMash*/ )
   Q_PROPERTY( double ibuGalPerLb            READ ibuGalPerLb            WRITE setIbuGalPerLb            NOTIFY changed /*changedIbuGalPerLb*/ )
   Q_PROPERTY( double equivSucrose_kg        READ equivSucrose_kg        /*WRITE*/                       NOTIFY changed /*changedEquivSucrose_kg*/        STORED false )
   Q_PROPERTY( bool isMashed                 READ isMashed               WRITE setIsMashed               NOTIFY changed /*changedIsMashed*/ )
   
   const QString name() const;
   const Type type() const;
   const QString typeString() const;
   //! Returns a translated type string.
   const QString typeStringTr() const;
   double amount_kg() const;
   double yield_pct() const;
   double color_srm() const;
   bool addAfterBoil() const;
   const QString origin() const;
   const QString supplier() const;
   const QString notes() const;
   double coarseFineDiff_pct() const;
   double moisture_pct() const;
   double diastaticPower_lintner() const;
   double protein_pct() const;
   double maxInBatch_pct() const;
   bool recommendMash() const;
   double ibuGalPerLb() const;

   // Calculated getters.
   //! Get the maximum kg of equivalent sucrose that will come out of this ferm.
   double equivSucrose_kg() const;

   void setName( const QString& str );
   void setType( Type t );
   void setAmount_kg( double num );
   void setYield_pct( double num );
   void setColor_srm( double num );
   
   void setAddAfterBoil( bool b );
   void setOrigin( const QString& str );
   void setSupplier( const QString& str);
   void setNotes( const QString& str );
   void setCoarseFineDiff_pct( double num );
   void setMoisture_pct( double num );
   void setDiastaticPower_lintner( double num );
   void setProtein_pct( double num );
   void setMaxInBatch_pct( double num );
   void setRecommendMash( bool b );
   void setIbuGalPerLb( double num );
   
   /*** My extensions ***/
   bool isMashed() const;
   void setIsMashed(bool var);
   /*** END my extensions ***/
   
signals:
   
   /*
   void changedName( QString newName );
   void changedType( Type newType );
   void changedTypeString( QString newTypeString );
   void changedTypeStringTr( QString newTypeStringTr );
   void changedAmount_kg( double newAmount_kg );
   void changedYield_pct( double newYield_pct );
   void changedColor_srm( double newColor_srm );
   void changedAddAfterBoil( bool newAddAfterBoil );
   void changedOrigin( QString newOrigin );
   void changedSupplier( QString newSupplier );
   void changedNotes( QString newNotes );
   void changedCoarseFineDiff_pct( double newCoarseFineDiff_pct );
   void changedMoisture_pct( double newMoisture_pct );
   void changedDiastaticPower_lintner( double newDiastaticPower_lintner );
   void changedProtein_pct( double newProtein_pct );
   void changedMaxInBatch_pct( double newMaxInBatch_pct );
   void changedRecommendMash( bool newRecommendMash );
   void changedIbuGalPerLb( double newIbuGalPerLb );
   void changedIsMashed( bool newIsMashed );
   */
   
private:
   Fermentable();
   Fermentable( Fermentable const& other );
   static bool isValidType( const QString& str );
   static QStringList types;
   
   void setDefaults();
};

inline bool FermentablePtrLt( Fermentable* lhs, Fermentable* rhs)
{
   return *lhs < *rhs;
}

inline bool FermentablePtrEq( Fermentable* lhs, Fermentable* rhs)
{
   return *lhs == *rhs;
}

struct Fermentable_ptr_cmp
{
   bool operator()( Fermentable* lhs, Fermentable* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Fermentable_ptr_equals
{
   bool operator()( Fermentable* lhs, Fermentable* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif
