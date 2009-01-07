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
   long ret;
   const char *begptr = str.c_str();
   
   ret = strtol( begptr, &endptr, 10 );
   
   if( endptr == begptr )
      throw ParseException();
   
   return ret;
}

double parseDouble( const std::string &str )
{
   char *endptr;
   double ret;
   const char *begptr = str.c_str();
   
   ret = strtod( begptr, &endptr );
   
   if( endptr == begptr )
      throw ParseException();
   
   return ret;
}

bool parseBool( const std::string &str )
{
   if( str == "TRUE" )
      return true;
   else if( str == "FALSE" )
      return false;
   else
      throw ParseException();
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

std::string intToString( int num )
{
   static char s[32];
   
   sprintf( s, "%d", num );
   
   return std::string(s);
}
