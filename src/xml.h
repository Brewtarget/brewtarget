/*
 * xml.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _XML_H
#define _XML_H

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include "stringparsing.h"

using namespace std;

bool beginsWith( const string &str1, const string &str2 );
// Returns string between first and second quotes or first and second apostrophes
// whichever is found first. Returns the original string if there are no quotes
// or apos. Returns a blank string if there is no matching second quote or apo.
std::string removeQuotes( const std::string &str );
std::string replaceXmlCodes( const std::string &str );

struct ToLower
{
   char operator()( char c ) const
   {
      return std::tolower(c);
   }
};

class Xml;

class Xml
{
   public:
      Xml( istream &in );
      string nextTag();
      string nextNonTag();
      void resetIter();
      bool isAtEnd();
      string getStringVal( const string &closeTag, bool &success, const string &errMsg );
      double getDoubleVal( const string &closeTag, bool &success, const string &errMsg );
      long int getLongVal( const string &closeTag, bool &success, const string &errMsg );
      
   private:
      string xmlText;
      string::const_iterator iter;
      string::const_iterator end;
};

class BeerXmlException : public std::exception
{
public:

   virtual const char* what() const throw()
   {
      // Note: this temporary object might get destroyed too early.
      // I'm not really sure.
      return std::string("BeerXml error: " + _err + "\n").c_str();
   }

   BeerXmlException( std::string message )
   {
      _err = message;
   }

   ~BeerXmlException() throw() {}

private:

   std::string _err;
};

#endif // _XML_H

