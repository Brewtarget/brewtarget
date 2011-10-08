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

#include <string>
#include <exception>
#include "observable.h"
#include <QDomNode>
#include "BeerXMLElement.h"
#include <QStringList>
#include <QString>

class Fermentable;

// Should this inherit an item model? E.g. Fermentable -> BeerXMLTreeModel -> QAbstractItemModel
class Fermentable : public QObject
{
   Q_OBJECT
   Q_CLASSINFO("version","1")
   friend class Brewtarget;
   
public:

   enum Type { TYPEGRAIN=0, TYPESUGAR, TYPEEXTRACT, TYPEDRY_EXTRACT, TYPEADJUNCT, NUMTYPES };

   Fermentable( const QString& table, const int& key ) : table(table), key(key) {}
   Fermentable( Fermentable& other ) : table(other.table), key(other.key) {}
   
   // QUESTION: will we still need this?
   Fermentable(const QDomNode& fermentableNode);

   virtual ~Fermentable() {}
   
   // QUESTION: still need these?
   virtual void fromNode(const QDomNode& node); // From BeerXMLElement
   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement

   friend bool operator<(Fermentable &f1, Fermentable &f2);
   friend bool operator==(Fermentable &f1, Fermentable &f2);

   // Obsolete
   /*
   enum{ DONOTUSE, NAME, TYPE, AMOUNT, YIELD, COLOR, AFTERBOIL, ORIGIN, SUPPLIER, NOTES, COARSEFINEDIFF, MOISTURE,
         DIASTATICPOWER, PROTEIN, MAXINBATCH, ISMASHED };
   */
   
   
   // New Q_PROPERTIES
   Q_PROPERTY( QString name                  READ name                   WRITE setName                   NOTIFY changedName )
   Q_PROPERTY( Type type                     READ type                   WRITE setType                   NOTIFY changedType )
   Q_PROPERTY( QString typeString            READ typeString             /*WRITE*/                       NOTIFY changedTypeString             STORED false )
   Q_PROPERTY( QString typeStringTr          READ typeStringTr           /*WRITE*/                       NOTIFY changedTypeStringTr           STORED false )
   Q_PROPERTY( double amount_kg              READ amount_kg              WRITE setAmount_kg              NOTIFY changedAmount_kg )
   Q_PROPERTY( double yield_pct              READ yield_pct              WRITE setYield_pct              NOTIFY changedYield_pct )
   Q_PROPERTY( double color_srm              READ color_srm              WRITE setColor_srm              NOTIFY changedColor_srm )
   Q_PROPERTY( bool addAfterBoil             READ addAfterBoil           WRITE setAddAfterBoil           NOTIFY changedAddAfterBoil )
   Q_PROPERTY( QString origin                READ origin                 WRITE setOrigin                 NOTIFY changedOrigin )
   Q_PROPERTY( QString supplier              READ supplier               WRITE setSupplier               NOTIFY changedSupplier )
   Q_PROPERTY( QString notes                 READ notes                  WRITE setNotes                  NOTIFY changedNotes )
   Q_PROPERTY( double coarseFineDiff_pct     READ coarseFineDiff_pct     WRITE setCoarseFineDiff_pct     NOTIFY changedCoarseFineDiff_pct )
   Q_PROPERTY( double moisture_pct           READ moisture_pct           WRITE setMoisture_pct           NOTIFY changedMoisture_pct )
   Q_PROPERTY( double diastaticPower_lintner READ diastaticPower_lintner WRITE setDiastaticPower_lintner NOTIFY changedDiastaticPower_lintner )
   Q_PROPERTY( double protein_pct            READ protein_pct            WRITE setProtein_pct            NOTIFY changedProtein_pct )
   Q_PROPERTY( double maxInBatch_pct         READ maxInBatch_pct         WRITE setMaxInBatch_pct         NOTIFY changedMaxInBatch_pct )
   Q_PROPERTY( bool recommendMash            READ recommendMash          WRITE setRecommendMash          NOTIFY changedRecommendMash )
   Q_PROPERTY( double ibuGalPerLb            READ ibuGalPerLb            WRITE setIbuGalPerLb            NOTIFY changedIbuGalPerLb )
   Q_PROPERTY( double equivSucrose_kg        READ equivSucrose_kg        /*WRITE*/                       NOTIFY changedEquivSucrose_kg        STORED false )
   Q_PROPERTY( bool isMashed                 READ isMashed               WRITE setIsMashed               NOTIFY changedIsMashed )
   
   // Getters. These will do a query on the corresponding database table entry.
   const QString name() const
   {
      //Example pseudocode
      
      //Query q = constructGetterQuery( table, key, "name" );
      Query q = constructGetterQuery( this, "name" );
      QVariant result = Database::instance().execQuery( q );
      return result.toString();
   }
   // Obsolete. See Q_CLASSINFO("version","1")
   //int version() const;
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

   // Calculated property getters.
   //! Get the maximum kg of equivalent sucrose that will come out of this ferm.
   double equivSucrose_kg() const;

   // Setters. These will set the corresponding database table entry.
   void setName( const QString& str )
   {
      // Example pseudocode.
      
      // SetterCommand implements QUndoCommand. Pass a reference to this object
      // so that if we ever execute q.undo(), we will know to emit changedName() again.
      SetterCommand c = constructSetterCommand( this, "name", str );
      Database::instance().pushSetterCommandOntoStackToBeExecutedInTheNearFuture( c );
      
      // The SetterCommand HAS TO emit this signal when it is actually executed somehow.
      // Otherwise, we can have concurrency issues where a getter is called before the
      // value has actually been changed in the database. I think we can take advantage
      // of the Q_PROPERTY stuff here.
      
      //emit changedName(str);
   }
   // Obsolete.
   //void setVersion( int num );
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
   bool getIsMashed() const;
   void setIsMashed(bool var);
   /*** END my extensions ***/
   
signals:
   
   // New notification signals.
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
   
private:
   
   // The key of this Fermentable in the database table.
   const int keyValue;
   // Which table this Fermentable is in.
   const QString table;
   
   // All obsolete. The data is stored only in the database.
   /*
   QString name;
   //See Q_CLASSINFO("version","1")
   static const int version = 1;
   Type type;
   double amount_kg;
   double yield_pct;
   double color_srm;

   bool addAfterBoil;
   QString origin;
   QString supplier;
   QString notes;
   double coarseFineDiff_pct;
   double moisture_pct;
   double diastaticPower_lintner;
   double protein_pct;
   double maxInBatch_pct;
   bool recommendMash;
   double ibuGalPerLb;
   bool isMashed;
   */

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
