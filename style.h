/*
 * style.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _STYLE_H
#define _STYLE_H
#include <string>
#include <exception>
#include "xmlnode.h"
#include "observable.h"

class Style;
class StyleException;

class Style : public Observable
{
public:

   Style();
   Style(XmlNode *node);

   std::string toXml();
   
   void setName( const std::string& var );
   void setCategory( const std::string& var );
   void setCategoryNumber( const std::string& var );
   void setStyleLetter( const std::string& var );
   void setStyleGuide( const std::string& var );
   void setType( const std::string& var );
   void setOgMin( double var );
   void setOgMax( double var );
   void setFgMin( double var );
   void setFgMax( double var );
   void setIbuMin( double var );
   void setIbuMax( double var );
   void setColorMin_srm( double var );
   void setColorMax_srm( double var );
   void setCarbMin_vol( double var );
   void setCarbMax_vol( double var );
   void setAbvMin_pct( double var );
   void setAbvMax_pct( double var );
   void setNotes( const std::string& var );
   void setProfile( const std::string& var );
   void setIngredients( const std::string& var );
   void setExamples( const std::string& var );

   std::string getName() const;
   std::string getCategory() const;
   std::string getCategoryNumber() const;
   std::string getStyleLetter() const;
   std::string getStyleGuide() const;
   std::string getType() const;
   double getOgMin() const;
   double getOgMax() const;
   double getFgMin() const;
   double getFgMax() const;
   double getIbuMin() const;
   double getIbuMax() const;
   double getColorMin_srm() const;
   double getColorMax_srm() const;
   double getCarbMin_vol() const;
   double getCarbMax_vol() const;
   double getAbvMin_pct() const;
   double getAbvMax_pct() const;
   std::string getNotes() const;
   std::string getProfile() const;
   std::string getIngredients() const;
   std::string getExamples() const;

private:

   // Mandatory fields.
   std::string name;
   static const int version = 1;
   std::string category;
   std::string categoryNumber;
   std::string styleLetter;
   std::string styleGuide;
   std::string type;
   double ogMin;
   double ogMax;
   double fgMin;
   double fgMax;
   double ibuMin;
   double ibuMax;
   double colorMin_srm;
   double colorMax_srm;
   
   // Optional fields
   double carbMin_vol;
   double carbMax_vol;
   double abvMin_pct;
   double abvMax_pct;
   std::string notes;
   std::string profile;
   std::string ingredients;
   std::string examples;

   void setDefaults();
   bool isValidType( const std::string &str );
};

class StyleException : public std::exception
{
public:

   virtual const char* what() const throw()
   {
      return std::string("BeerXML STYLE error: " + _err + "\n").c_str();
   }

   StyleException( const std::string& message )
   {
      _err = message;
   }

   ~StyleException() throw() {}

private:

   std::string _err;
};

#endif //_STYLE_H
