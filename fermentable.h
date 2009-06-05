/*
 * fermentable.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _FERMENTABLE_H
#define _FERMENTABLE_H

#include <string>
#include <exception>
#include "xmlnode.h"
#include "observable.h"
#include <QDomNode>
#include "BeerXMLElement.h"

class Fermentable;

class Fermentable : public Observable, public BeerXMLElement
{
public:
   Fermentable();
   Fermentable( Fermentable& other );
   Fermentable( const XmlNode* node );
   Fermentable(const QDomNode& fermentableNode);
   
   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
   //std::string toXml();

   // Operators
   friend bool operator<(Fermentable &f1, Fermentable &f2);
   friend bool operator==(Fermentable &f1, Fermentable &f2);

   // Get
   const std::string& getName() const;
   int getVersion() const;
   const std::string& getType() const;
   double getAmount_kg() const;
   double getYield_pct() const;
   double getColor_srm() const;
   
   bool getAddAfterBoil() const;
   const std::string& getOrigin() const;
   const std::string& getSupplier() const;
   const std::string& getNotes() const;
   double getCoarseFineDiff_pct() const;
   double getMoisture_pct() const;
   double getDiastaticPower_lintner() const;
   double getProtein_pct() const;
   double getMaxInBatch_pct() const;
   bool getRecommendMash() const;
   double getIbuGalPerLb() const;
   
   // Set
   void setName( const std::string& str );
   void setVersion( int num );
   void setType( const std::string& str );
   void setAmount_kg( double num );
   void setYield_pct( double num );
   void setColor_srm( double num );
   
   void setAddAfterBoil( bool b );
   void setOrigin( const std::string& str );
   void setSupplier( const std::string& str);
   void setNotes( const std::string& str );
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
   
private:
   std::string name;
   static const int version = 1;
   std::string type;
   double amount_kg;
   double yield_pct;
   double color_srm;

   bool addAfterBoil;
   std::string origin;
   std::string supplier;
   std::string notes;
   double coarseFineDiff_pct;
   double moisture_pct;
   double diastaticPower_lintner;
   double protein_pct;
   double maxInBatch_pct;
   bool recommendMash;
   double ibuGalPerLb;
   /*** My extensions ***/
   bool isMashed;
   /*** END my extensions ***/

   static bool isValidType( const std::string& str );
   void setDefaults();
};

class FermentableException : public std::exception
{
public:
   
   virtual const char* what() const throw()
   {
      // Note: this temporary object might get destroyed too early.
      // I'm not really sure.
      return std::string("BeerXml FERMENTABLE error: " + _err + "\n").c_str();
   }
   
   FermentableException( std::string message )
   {
      _err = message;
   }
   
   ~FermentableException() throw() {}
   
private:
   
   std::string _err;
};

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
