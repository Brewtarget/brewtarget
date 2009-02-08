/*
 * stringparsing.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _STRINGPARSING_H
#define	_STRINGPARSING_H

#include <string>
#include <exception>
#include <stdlib.h>

class ParseException;

int parseInt( const std::string &str );
long parseLong( const std::string &str );
double parseDouble( const std::string &str );
bool parseBool( const std::string &str );
std::string doubleToString( double num );
std::string doubleToStringPrec( double num, unsigned int prec );
std::string intToString( int num );
std::string boolToString( bool b );

std::string& trim( std::string &str );
void iterateUntilDelimiter( std::string::const_iterator &iter );
void iterateUntilNotDelimiter( std::string::const_iterator &iter );
void iterateUntilCharFound( std::string::const_iterator &iter, char c );
bool isWhiteSpace( char c );

//==========================ParseException======================================
class ParseException : std::exception
{
public:
   
   virtual const char* what() const throw()
   {
      return "Error in parsing.";
   }
   
   ParseException(){};
};

#endif	/* _STRINGPARSING_H */

