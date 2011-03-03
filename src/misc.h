/*
 * misc.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MISC_H
#define	_MISC_H

#include <string>
#include <exception>
#include <QDomNode>
#include <QDomText>
#include <QString>
#include "observable.h"
#include "BeerXMLElement.h"

class Misc;

class Misc : public Observable, public BeerXMLElement
{
public:
  
   enum Type{ TYPESPICE, TYPEFINING, TYPEWATER_AGENT, TYPEHERB, TYPEFLAVOR, TYPEOTHER };
   enum Use{ USEBOIL, USEMASH, USEPRIMARY, USESECONDARY, USEBOTTLING };
   
   Misc();
   Misc(Misc& other);
   Misc( const QDomNode& miscNode );

   friend bool operator<(Misc &m1, Misc &m2);
   friend bool operator==(Misc &m1, Misc &m2);

   virtual void fromNode(const QDomNode& node); // From BeerXMLElement
   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
   //QString toXml();
   
   // Set
   void setName( const QString &var );
   void setType( Type t );
   void setUse( Use u );
   void setAmount( double var );
   void setTime( double var );
   void setAmountIsWeight( bool var );
   void setUseFor( const QString &var );
   void setNotes( const QString &var );
   
   // Get
   QString getName() const;
   Type getType() const;
   const QString getTypeString() const;
   //! Returns a translated type string.
   const QString getTypeStringTr() const;
   Use getUse() const;
   const QString getUseString() const;
   //! Returns a translated use string.
   const QString getUseStringTr() const;
   double getAmount() const;
   double getTime() const;
   bool getAmountIsWeight() const;
   QString getUseFor() const;
   QString getNotes() const;
   
private:
   
   // Required fields.
   QString name;
   static const int version = 1;
   Type type;
   Use use;
   double time;
   double amount;
   
   // Optional fields.
   bool amountIsWeight;
   QString useFor;
   QString notes;

   // Private methods
   void setDefaults();
   bool isValidType( const QString &var );
   bool isValidUse( const QString &var );
   
   static QStringList types;
   static QStringList uses;
};

inline bool MiscPtrLt( Misc* lhs, Misc* rhs)
{
   return *lhs < *rhs;
}

inline bool MiscPtrEq( Misc* lhs, Misc* rhs)
{
   return *lhs == *rhs;
}

struct Misc_ptr_cmp
{
   bool operator()( Misc* lhs, Misc* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Misc_ptr_equals
{
   bool operator()( Misc* lhs, Misc* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif	/* _MISC_H */
