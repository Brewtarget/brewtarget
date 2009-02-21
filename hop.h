/*
 * hop.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _HOP_H
#define _HOP_H

#include <string>
#include <exception>
#include "xmlnode.h"
#include "observable.h"

using namespace std;

class Hop;
class HopException;

class Hop : public Observable
{
   
   public:
      Hop();
      Hop( Hop& other );
      Hop( const XmlNode *node );

      friend bool operator<( Hop &h1, Hop &h2 );

      std::string toXml();
      
      const string& getName() const;
      int getVersion() const;
      double getAlpha_pct() const;
      double getAmount_kg() const;
      const string& getUse() const;
      double getTime_min() const;
      
      const string& getNotes() const;
      const string& getType() const;
      const string& getForm() const;
      double getBeta_pct() const;
      double getHsi_pct() const;
      const string& getOrigin() const;
      const string& getSubstitutes() const;
      double getHumulene_pct() const;
      double getCaryophyllene_pct() const;
      double getCohumulone_pct() const;
      double getMyrcene_pct() const;
      
      //set
      void setName( const string &str );
      void setAlpha_pct( double num );
      void setAmount_kg( double num );
      bool setUse( const string &str );
      void setTime_min( double num );
      
      void setNotes( const string &str );
      bool setType( const string &str );
      bool setForm( const string &str );
      void setBeta_pct( double num );
      void setHsi_pct( double num );
      void setOrigin( const string &str );
      void setSubstitutes( const string &str );
      void setHumulene_pct( double num );
      void setCaryophyllene_pct( double num );
      void setCohumulone_pct( double num );
      void setMyrcene_pct( double num );
      
   private:
      // Mandatory members.
      string name;
      const static int version = 1;
      double alpha_pct;
      double amount_kg;
      string use;
      double time_min;
      
      // Optional members.
      string notes;
      string type;
      string form;
      double beta_pct;
      double hsi_pct;
      string origin;
      string substitutes;
      double humulene_pct;
      double caryophyllene_pct;
      double cohumulone_pct;
      double myrcene_pct;
      
      // Sets every member to zero or blank or whatever.
      void setDefaults();
      // Misc functions.
      static bool isValidUse(const string &str);
      static bool isValidType(const string &str);
      static bool isValidForm(const string &str);
};

class HopException : public std::exception
{
public:
   
   virtual const char* what() const throw()
   {
      // Note: this temporary object might get destroyed too early.
      // I'm not really sure.
      return std::string("BeerXml HOP error: " + _err + "\n").c_str();
   }
   
   HopException( std::string message )
   {
      _err = message;
   }
   
   ~HopException() throw() {}
   
private:
   
   std::string _err;
};

struct Hop_ptr_cmp
{
   bool operator()( Hop* lhs, Hop* rhs)
   {
      return *lhs < *rhs;
   }
};

#endif // _HOP_H
