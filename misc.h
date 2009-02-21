/*
 * misc.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MISC_H
#define	_MISC_H

#include <string>
#include <exception>
#include "xmlnode.h"
#include "observable.h"

class Misc;
class MiscException;

class Misc : public Observable
{
public:
   Misc();
   Misc(Misc& other);
   Misc( const XmlNode *node );

   friend bool operator<(Misc &m1, Misc &m2);

   std::string toXml();
   
   // Set
   void setName( const std::string &var );
   void setType( const std::string &var );
   void setUse( const std::string &var );
   void setAmount( double var );
   void setTime( double var );
   void setAmountIsWeight( bool var );
   void setUseFor( const std::string &var );
   void setNotes( const std::string &var );
   
   // Get
   std::string getName() const;
   std::string getType() const;
   std::string getUse() const;
   double getAmount() const;
   double getTime() const;
   bool getAmountIsWeight() const;
   std::string getUseFor() const;
   std::string getNotes() const;
   
private:
   
   // Required fields.
   std::string name;
   static const int version = 1;
   std::string type;
   std::string use;
   double time;
   double amount;
   
   // Optional fields.
   bool amountIsWeight;
   std::string useFor;
   std::string notes;

   // Private methods
   void setDefaults();
   bool isValidType( const std::string &var );
   bool isValidUse( const std::string &var );
};

class MiscException : public std::exception
{
public:
   
   virtual const char* what() const throw()
   {
      // Note: this temporary object might get destroyed too early.
      // I'm not really sure.
      return std::string("BeerXml MISC error: " + _err + "\n").c_str();
   }
   
   MiscException( std::string message )
   {
      _err = message;
   }
   
   ~MiscException() throw() {}
   
private:
   
   std::string _err;
};

struct Misc_ptr_cmp
{
   bool operator()( Misc* lhs, Misc* rhs)
   {
      return *lhs < *rhs;
   }
};

#endif	/* _MISC_H */
