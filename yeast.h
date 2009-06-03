/*
 * yeast.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _YEAST_H
#define	_YEAST_H

#include <string>
#include <exception>
#include "xmlnode.h"
#include "observable.h"
#include "BeerXMLElement.h"
#include <QDomNode>

class Yeast;
class YeastException;

class Yeast : public Observable, public BeerXMLElement
{
public:
   Yeast();
   Yeast(Yeast& other);
   Yeast( XmlNode *node );
   Yeast( const QDomNode& yeastNode );

   friend bool operator<(Yeast &y1, Yeast &y2);
   friend bool operator==(Yeast &y1, Yeast &y2);

   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement

   //std::string toXml();
   
   // Set
   void setName( const std::string& var );
   void setType( const std::string& var );
   void setForm( const std::string& var );
   void setAmount( double var );
   void setAmountIsWeight( bool var );
   void setLaboratory( const std::string& var );
   void setProductID( const std::string& var );
   void setMinTemperature_c( double var );
   void setMaxTemperature_c( double var );
   void setFlocculation( const std::string& var );
   void setAttenuation_pct( double var );
   void setNotes( const std::string& var );
   void setBestFor( const std::string& var );
   void setTimesCultured( int var );
   void setMaxReuse( int var );
   void setAddToSecondary( bool var );
   
   // Get
   std::string getName() const;
   std::string getType() const;
   std::string getForm() const;
   double getAmount() const;
   bool getAmountIsWeight() const;
   std::string getLaboratory() const;
   std::string getProductID() const;
   double getMinTemperature_c() const;
   double getMaxTemperature_c() const;
   std::string getFlocculation() const;
   double getAttenuation_pct() const;
   std::string getNotes() const;
   std::string getBestFor() const;
   int getTimesCultured() const;
   int getMaxReuse() const;
   bool getAddToSecondary() const;
   
private:
   // Required fields.
   std::string name;
   static const int version = 1;
   std::string type;
   std::string form;
   double amount;
   
   // Optional fields.
   bool amountIsWeight;
   std::string laboratory;
   std::string productID;
   double minTemperature_c;
   double maxTemperature_c;
   std::string flocculation;
   double attenuation_pct;
   std::string notes;
   std::string bestFor;
   int timesCultured;
   int maxReuse;
   bool addToSecondary;
   
   // Methods
   bool isValidType(const std::string& str) const;
   bool isValidForm(const std::string& str) const;
   bool isValidFlocculation(const std::string& str) const;
   void setDefaults();
};

class YeastException : public std::exception
{
public:
   
   virtual const char* what() const throw()
   {
      // Note: this temporary object might get destroyed too early.
      // I'm not really sure.
      return std::string("BeerXml YEAST error: " + _err + "\n").c_str();
   }
   
   YeastException( std::string message )
   {
      _err = message;
   }
   
   ~YeastException() throw() {}
   
private:
   
   std::string _err;
};

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

