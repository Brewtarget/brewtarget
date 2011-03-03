/*
 * yeast.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _YEAST_H
#define	_YEAST_H

#include <string>
#include <exception>
#include "observable.h"
#include "BeerXMLElement.h"
#include <QDomNode>
#include <QString>
#include <QStringList>

class Yeast;

class Yeast : public Observable, public BeerXMLElement
{
public:
   enum Type {TYPEALE, TYPELAGER, TYPEWHEAT, TYPEWINE, TYPECHAMPAGNE};
   enum Form {FORMLIQUID, FORMDRY, FORMSLANT, FORMCULTURE};
   enum Flocculation {FLOCLOW, FLOCMEDIUM, FLOCHIGH, FLOCVERY_HIGH};

   Yeast();
   Yeast(Yeast& other);
   Yeast( const QDomNode& yeastNode );

   friend bool operator<(Yeast &y1, Yeast &y2);
   friend bool operator==(Yeast &y1, Yeast &y2);

   virtual void fromNode(const QDomNode& node); // From BeerXMLElement
   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
   
   // Set
   void setName( const QString& var );
   void setType( Type t );
   void setForm( Form f );
   void setAmount( double var );
   void setAmountIsWeight( bool var );
   void setLaboratory( const QString& var );
   void setProductID( const QString& var );
   void setMinTemperature_c( double var );
   void setMaxTemperature_c( double var );
   void setFlocculation( Flocculation f );
   void setAttenuation_pct( double var );
   void setNotes( const QString& var );
   void setBestFor( const QString& var );
   void setTimesCultured( int var );
   void setMaxReuse( int var );
   void setAddToSecondary( bool var );
   
   // Get
   QString getName() const;
   Type getType() const;
   const QString getTypeString() const;
   //! Returns a translated type string.
   const QString getTypeStringTr() const;
   Form getForm() const;
   const QString getFormString() const;
   //! Returns a translated form string.
   const QString getFormStringTr() const;
   double getAmount() const;
   bool getAmountIsWeight() const;
   QString getLaboratory() const;
   QString getProductID() const;
   double getMinTemperature_c() const;
   double getMaxTemperature_c() const;
   Flocculation getFlocculation() const;
   const QString getFlocculationString() const;
   //! Returns a flocculation string.
   const QString getFlocculationStringTr() const;
   double getAttenuation_pct() const;
   QString getNotes() const;
   QString getBestFor() const;
   int getTimesCultured() const;
   int getMaxReuse() const;
   bool getAddToSecondary() const;
   
private:
   // Required fields.
   QString name;
   static const int version = 1;
   Type type;
   Form form;
   double amount;
   
   // Optional fields.
   bool amountIsWeight;
   QString laboratory;
   QString productID;
   double minTemperature_c;
   double maxTemperature_c;
   Flocculation flocculation;
   double attenuation_pct;
   QString notes;
   QString bestFor;
   int timesCultured;
   int maxReuse;
   bool addToSecondary;
   
   static QStringList types;
   static QStringList forms;
   static QStringList flocculations;

   // Methods
   bool isValidType(const QString& str) const;
   bool isValidForm(const QString& str) const;
   bool isValidFlocculation(const QString& str) const;
   void setDefaults();
};

inline bool YeastPtrLt( Yeast* lhs, Yeast* rhs)
{
   return *lhs < *rhs;
}

inline bool YeastPtrEq( Yeast* lhs, Yeast* rhs)
{
   return *lhs == *rhs;
}

struct Yeast_ptr_cmp
{
   bool operator()( Yeast* lhs, Yeast* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Yeast_ptr_equals
{
   bool operator()( Yeast* lhs, Yeast* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif	/* _YEAST_H */

