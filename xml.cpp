/*
 * xml.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <iostream>
#include <string>
#include <algorithm>
#include "xml.h"

using namespace std;

// Returns true if str1 begins with str2
bool beginsWith( const string &str1, const string &str2 )
{
   if( str1.size() < str2.size() )
      return false;

   return ( str1.compare( 0, str2.size(), str2 ) == 0 );
}

std::string replaceXmlCodes( const std::string &str )
{
   std::string::size_type ndx;
   std::string ret(str);
   
   // This iterative approach doesn't exactly conform to standards. During
   // processing, it may create other false XML codes which will get translated.
   while( true )
   {
      ndx = ret.find("&amp;");
      if( ndx >=  ret.size() || ndx == std::string::npos )
         break;
      ret.erase( ndx, 5 );
      ret.insert( ndx, "&" );
   }
   while( true )
   {
      ndx = ret.find("&lt;");
      if( ndx >=  ret.size() || ndx == std::string::npos )
         break;
      ret.erase( ndx, 4 );
      ret.insert( ndx, "<" );
   }
   while( true )
   {
      ndx = ret.find("&gt;");
      if( ndx >=  ret.size() || ndx == std::string::npos )
         break;
      ret.erase( ndx, 4 );
      ret.insert( ndx, ">" );
   }
   while( true )
   {
      ndx = ret.find("&quot;");
      if( ndx >=  ret.size() || ndx == std::string::npos )
         break;
      ret.erase( ndx, 6 );
      ret.insert( ndx, "\"" );
   }
   while( true )
   {
      ndx = ret.find("&apos;");
      if( ndx >=  ret.size() || ndx == std::string::npos )
         break;
      ret.erase( ndx, 6 );
      ret.insert( ndx, "\'" );
   }
   
   return ret;
}

std::string removeQuotes( const std::string &str )
{
   std::string::size_type quoteNdx, quot, apos;
   char c;
   std::string ret(str);
   
   quot = ret.find( "\"", 0 );
   apos = ret.find( "\'", 0 );
   quoteNdx = quot < apos? quot : apos; // Minimum
   
   if( quoteNdx == std::string::npos )
      return ret; // There were no quotes or apostrophes, so, don't process.
   
   c = ret.at(quoteNdx);
   
   ret.erase( 0, quoteNdx+1 );
   
   quoteNdx = ret.find(c);
   if( quoteNdx == std::string::npos )
      return ""; // There was a beginning quote or apostrophe, but no matching one...error.
   
   ret.erase( quoteNdx, ret.size() - quoteNdx );
   
   return ret;
}

Xml::Xml( istream &in )
{
   string tmp;

   while( getline( cin, tmp ) )
      //xmlTextVec.push_back(tmp);
      xmlText += tmp;

   //iter = xmlTextVec.begin();

   // Transform everything to lowercase
   // so that comparing will be easier.
   transform( xmlText.begin(), xmlText.end(), xmlText.begin(), ToLower() );

   iter = xmlText.begin();
   end = xmlText.end();
}

void Xml::resetIter()
{
   iter = xmlText.begin();
}

string Xml::nextTag()
{
   string::const_iterator b, e;
   string ret;

   do
   {
      // Find next "<"
      while( *iter != '<' && iter != end )
         iter++;

      b = iter;
      b++;

      while( *iter != '>' && iter != end )
         iter++;

      e = iter;
      ret = string(b,e);

      // Leave the iterator on the character after
      // the ">".
      if( iter != end )
         ++iter;
   } while( beginsWith( ret, "!--" ) ); // This excludes comment lines.

   return trim(ret);
}

string Xml::nextNonTag()
{
   string::const_iterator b, e;

   b = iter;
   e = iter;

   while( *e != '<' && *e != '>' )
      ++e;

   iter = e;

   string ret(b,e);
   return trim(ret);
}

string Xml::getStringVal( const string &closeTag, bool &success, const string &errMsg )
{
   string ret = nextNonTag();
   if( ret.size() >= 0 )
      success = true;
   else
      success = false;

   if( ! beginsWith( nextTag(), closeTag ) )
      cerr << errMsg << endl;

   return ret;
}

double Xml::getDoubleVal( const string &closeTag, bool &success, const string &errMsg )
{
   double ret;
   char *endptr;
   const char *txt = nextNonTag().c_str();

   ret = strtod( txt, &endptr );

   if( endptr == txt )
   {
      success = false;
      cerr << errMsg << endl;
   }
   else
      success = true;

   if( ! beginsWith( nextTag(), closeTag ) )
      cerr << errMsg << endl;

   return ret;
}

long int Xml::getLongVal( const string &closeTag, bool &success, const string &errMsg )
{
   long int ret;
   char *endptr;
   const char *txt = nextNonTag().c_str();

   ret = strtol( txt, &endptr, 10 );

   if( endptr == txt )
   {
      success = false;
      cerr << errMsg << endl;
   }
   else
      success = true;

   if( ! beginsWith( nextTag(), closeTag ) )
      cerr << errMsg << endl;

   return ret;
}

bool Xml::isAtEnd()
{
   return (iter == end);
}

