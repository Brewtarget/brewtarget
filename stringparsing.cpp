/*
 * stringparsing.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "stringparsing.h"

int parseInt( const std::string &str )
{
   return atoi(str.c_str());
}

long parseLong( const std::string &str )
{
   char *endptr;
   long ret = 0.0;
   const char *begptr = str.c_str();
   
   ret = strtol( begptr, &endptr, 10 );
   
   if( endptr == begptr && str.size() > 0 )
   {
      std::cerr << "parseLong() could not convert " + str << std::endl;
      throw ParseException("parseLong() could not convert " + str);
   }
   
   return ret;
}

double parseDouble( const std::string &str )
{
   char *endptr;
   double ret = 0.0;
   const char *begptr = str.c_str();
   
   ret = strtod( begptr, &endptr );
   
   if( endptr == begptr && str.size() > 0 )
   {
      std::cerr << "parseDouble() could not convert " + str << std::endl;
      throw ParseException("parseDouble() could not convert " + str);
   }
   
   return ret;
}

bool parseBool( const std::string &str )
{
   if( str == "TRUE" )
      return true;
   else if( str == "FALSE" )
      return false;
   else
   {
      std::cerr << "parseBool() could not convert " + str << std::endl;
      throw ParseException("parseBool() could not convert " + str);
   }
}

std::string boolToString( bool b )
{
   if( b )
      return "TRUE";
   else
      return "FALSE";
}

std::string doubleToString( double num )
{
   static char s[32];
   
   sprintf( s, "%.3lf", num );
   
   return std::string(s);
}

std::string doubleToStringPrec( double num, unsigned int prec )
{
   static char s[32];
   static char format[32];

   sprintf( format, "%%.%dlf", prec );
   sprintf( s, format, num );

   return std::string(s);
}

std::string intToString( int num )
{
   static char s[32];
   
   sprintf( s, "%d", num );
   
   return std::string(s);
}

std::string& trim( std::string &str )
{
   static const std::string white(" \f\n\r\t\v");
   std::string::size_type lastNonSpace, firstNonSpace;

   firstNonSpace = str.find_first_not_of(white);
   if( firstNonSpace == std::string::npos )
      return str;
   str.erase( 0, firstNonSpace );

   lastNonSpace = str.find_last_not_of(white);
   if( lastNonSpace == std::string::npos )
      return str;
   str.erase( lastNonSpace+1, str.size()-lastNonSpace-1 );

   return str;
}

bool isWhiteSpace( char c )
{
   return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t'
           || c == '\v' );
}

void iterateUntilDelimiter( std::string::const_iterator &iter )
{
   while( !(isWhiteSpace(*iter) || *iter == '=' || *iter == '>') )
          ++iter;
}

void iterateUntilNotDelimiter( std::string::const_iterator &iter )
{
   while( isWhiteSpace(*iter) || *iter == '=' || *iter == '>' )
          ++iter;
}

void iterateUntilCharFound( std::string::const_iterator &iter, char c )
{
   while( *iter != c )
      ++iter;
}
