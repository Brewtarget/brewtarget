/*
 * style.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <vector>
#include "xmlnode.h"
#include "stringparsing.h"
#include "style.h"

std::string Style::toXml()
{
   std::string ret = "<STYLE>\n";
   
   ret += "<NAME>"+name+"</NAME>\n";
   ret += "<VERSION>"+intToString(version)+"</VERSION>\n";
   ret += "<CATEGORY>"+category+"</CATEGORY>\n";
   ret += "<CATEGORY_NUMBER>"+categoryNumber+"</CATEGORY_NUMBER>\n";
   ret += "<STYLE_LETTER>"+styleLetter+"</STYLE_LETTER>\n";
   ret += "<STYLE_GUIDE>"+styleGuide+"</STYLE_GUIDE>\n";
   ret += "<TYPE>"+type+"</TYPE>\n";
   ret += "<OG_MIN>"+doubleToString(ogMin)+"</OG_MIN>\n";
   ret += "<OG_MAX>"+doubleToString(ogMax)+"</OG_MAX>\n";
   ret += "<FG_MIN>"+doubleToString(fgMin)+"</FG_MIN>\n";
   ret += "<FG_MAX>"+doubleToString(fgMax)+"</FG_MAX>\n";
   ret += "<IBU_MIN>"+doubleToString(ibuMin)+"</IBU_MIN>\n";
   ret += "<IBU_MAX>"+doubleToString(ibuMax)+"</IBU_MAX>\n";
   ret += "<COLOR_MIN>"+doubleToString(colorMin_srm)+"</COLOR_MIN>\n";
   ret += "<COLOR_MAX>"+doubleToString(colorMax_srm)+"</COLOR_MAX>\n";
   ret += "<CARB_MIN>"+doubleToString(carbMin_vol)+"</CARB_MIN>\n";
   ret += "<CARB_MAX>"+doubleToString(carbMax_vol)+"</CARB_MAX>\n";
   ret += "<ABV_MIN>"+doubleToString(abvMin_pct)+"</ABV_MIN>\n";
   ret += "<ABV_MAX>"+doubleToString(abvMax_pct)+"</ABV_MAX>\n";
   ret += "<NOTES>"+notes+"</NOTES>\n";
   ret += "<PROFILE>"+profile+"</PROFILE>\n";
   ret += "<INGREDIENTS>"+ingredients+"</INGREDIENTS>\n";
   ret += "<EXAMPLES>"+examples+"</EXAMPLES>\n";
   
   ret += "</STYLE>\n";
   return ret;
}

//===========================CONSTRUCTORS=======================================

void Style::setDefaults()
{
   name = "";
   category = "";
   categoryNumber = "";
   styleLetter = "";
   styleGuide = "";
   type = "";
   ogMin = 0.0;
   ogMax = 0.0;
   fgMin = 0.0;
   fgMax = 0.0;
   ibuMin = 0.0;
   ibuMax = 0.0;
   colorMin_srm = 0.0;
   colorMax_srm = 0.0;
   
   // Optional fields
   carbMin_vol = 0.0;
   carbMax_vol = 0.0;
   abvMin_pct = 0.0;
   abvMax_pct = 0.0;
   notes = "";
   profile = "";
   ingredients = "";
   examples = "";
}

Style::Style()
{
   setDefaults();
}

Style::Style(XmlNode *node)
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   std::string leafText;
   XmlNode* leaf;
   unsigned int i, childrenSize;
   bool hasName=false, hasVersion=false, hasCat=false, hasCatNum=false, hasStyleLetter=false,
           hasStyleGuide=false, hasType=false, hasOgMin=false, hasOgMax=false,
           hasFgMin=false, hasFgMax=false, hasIbuMin=false, hasIbuMax=false,
           hasColorMin=false, hasColorMax=false;
   
   setDefaults();
   
   if( node->getTag() != "STYLE" )
      throw StyleException("initializer not passed a STYLE node.");
   
   node->getChildren( children );
   childrenSize = children.size();
   
   for( i = 0; i < childrenSize; ++i )
   {
      tag = children[i]->getTag();
      children[i]->getChildren( tmpVec );
      
      // All valid children of YEAST only have one child.
      if( tmpVec.size() != 1 )
         throw StyleException("Tag \""+tag+"\" has more than one child.");
      
      leaf = tmpVec[0];
      // It must be a leaf if it is a valid BeerXML entry.
      if( ! leaf->isLeaf() )
         throw StyleException("Should have been a leaf but is not.");
      
      leafText = leaf->getLeafText();
      
      if( tag == "NAME" )
      {
         setName(leafText);
         hasName=true;
      }
      else if( tag == "VERSION" )
      {
         if( parseInt(leafText) != version )
            std::cerr << "Warning: XML STYLE is not version " << version << std::endl;
         hasVersion=true;
      }
      else if( tag == "CATEGORY" )
      {
         setCategory(leafText);
         hasCat=true;
      }
      else if( tag == "CATEGORY_NUMBER" )
      {
         setCategoryNumber(leafText);
         hasCatNum=true;
      }
      else if( tag == "STYLE_LETTER" )
      {
         setStyleLetter(leafText);
         hasStyleLetter=true;
      }
      else if( tag == "STYLE_GUIDE" )
      {
         setStyleGuide(leafText);
         hasStyleGuide=true;
      }
      else if( tag == "TYPE" )
      {
         setType(leafText);
         hasType=true;
      }
      else if( tag == "OG_MIN" )
      {
         setOgMin(parseDouble(leafText));
         hasOgMin=true;
      }
      else if( tag == "OG_MAX" )
      {
         setOgMax(parseDouble(leafText));
         hasOgMax=true;
      }
      else if( tag == "FG_MIN" )
      {
         setFgMin(parseDouble(leafText));
         hasFgMin=true;
      }
      else if( tag == "FG_MAX" )
      {
         setFgMax(parseDouble(leafText));
         hasFgMax=true;
      }
      else if( tag == "IBU_MIN" )
      {
         setIbuMin(parseDouble(leafText));
         hasIbuMin=true;
      }
      else if( tag == "IBU_MAX" )
      {
         setIbuMax(parseDouble(leafText));
         hasIbuMax=true;
      }
      else if( tag == "COLOR_MIN" )
      {
         setColorMin_srm(parseDouble(leafText));
         hasColorMin=true;
      }
      else if( tag == "COLOR_MAX" )
      {
         setColorMax_srm(parseDouble(leafText));
         hasColorMax=true;
      }
      else if( tag == "CARB_MIN" )
         setCarbMin_vol(parseDouble(leafText));
      else if( tag == "CARB_MAX" )
         setCarbMax_vol(parseDouble(leafText));
      else if( tag == "ABV_MIN" )
         setAbvMin_pct(parseDouble(leafText));
      else if( tag == "ABV_MAX" )
         setAbvMax_pct(parseDouble(leafText));
      else if( tag == "NOTES" )
         setNotes(leafText);
      else if( tag == "PROFILE" )
         setProfile(leafText);
      else if( tag == "INGREDIENTS" )
         setIngredients(leafText);
      else if( tag == "EXAMPLES" )
         setExamples(leafText);
      else
         std::cerr << "Warning: Style tag " << tag << " is not supported." << std::endl;
   } // end for()
   
   if( !hasName || !hasVersion || !hasCat || !hasCatNum || !hasStyleLetter ||
           !hasStyleGuide || !hasType || !hasOgMin || !hasOgMax ||
           !hasFgMin || !hasFgMax || !hasIbuMin || !hasIbuMax ||
           !hasColorMin || !hasColorMax )
      throw StyleException("missing a required field.");
}// end Style()

//==============================="SET" METHODS==================================
void Style::setName( const std::string& var )
{
   name = std::string(var);
   hasChanged();
}

void Style::setCategory( const std::string& var )
{
   category = std::string(var);
   hasChanged();
}

void Style::setCategoryNumber( const std::string& var )
{
   categoryNumber = std::string(var);
   hasChanged();
}

void Style::setStyleLetter( const std::string& var )
{
   styleLetter= std::string(var);
   hasChanged();
}

void Style::setStyleGuide( const std::string& var )
{
   styleGuide = std::string(var);
   hasChanged();
}

void Style::setType( const std::string& var )
{
   if( ! isValidType(var) )
      throw StyleException("invalid style: " + var );
   else
   {
      type = std::string(var);
      hasChanged();
   }
}

void Style::setOgMin( double var )
{
   if( var < 0.0 )
      throw StyleException("bad OG: " + doubleToString(var));
   else
   {
      ogMin = var;
      hasChanged();
   }
}

void Style::setOgMax( double var )
{
   if( var < 0.0 )
      throw StyleException("bad OG: " + doubleToString(var));
   else
   {
      ogMax = var;
      hasChanged();
   }
}

void Style::setFgMin( double var )
{
   if( var < 0.0 )
      throw StyleException("bad FG: " + doubleToString(var));
   else
   {
      fgMin = var;
      hasChanged();
   }
}

void Style::setFgMax( double var )
{
   if( var < 0.0 )
      throw StyleException("bad FG: " + doubleToString(var));
   else
   {
      fgMax = var;
      hasChanged();
   }
}

void Style::setIbuMin( double var )
{
   if( var < 0.0 )
      throw StyleException("bad IBU: " + doubleToString(var));
   else
   {
      ibuMin = var;
      hasChanged();
   }
}

void Style::setIbuMax( double var )
{
   if( var < 0.0 )
      throw StyleException("bad IBU: " + doubleToString(var));
   else
   {
      ibuMax = var;
      hasChanged();
   }
}

void Style::setColorMin_srm( double var )
{
   if( var < 0.0 )
      throw StyleException("bad color: " + doubleToString(var));
   else
   {
      colorMin_srm = var;
      hasChanged();
   }
}

void Style::setColorMax_srm( double var )
{
   if( var < 0.0 )
      throw StyleException("bad color: " + doubleToString(var));
   else
   {
      colorMax_srm = var;
      hasChanged();
   }
}

void Style::setCarbMin_vol( double var )
{
   if( var < 0.0 )
      throw StyleException("bad carb: " + doubleToString(var));
   else
   {
      carbMin_vol = var;
      hasChanged();
   }
}

void Style::setCarbMax_vol( double var )
{
   if( var < 0.0 )
      throw StyleException("bad carb: " + doubleToString(var));
   else
   {
      carbMax_vol = var;
      hasChanged();
   }
}

void Style::setAbvMin_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      throw StyleException("bad ABV: " + doubleToString(var));
   else
   {
      abvMin_pct = var;
      hasChanged();
   }
}

void Style::setAbvMax_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      throw StyleException("bad ABV: " + doubleToString(var));
   else
   {
      abvMax_pct = var;
      hasChanged();
   }
}

void Style::setNotes( const std::string& var )
{
   notes = std::string(var);
   hasChanged();
}

void Style::setProfile( const std::string& var )
{
   profile = std::string(var);
   hasChanged();
}

void Style::setIngredients( const std::string& var )
{
   ingredients = std::string(var);
   hasChanged();
}

void Style::setExamples( const std::string& var )
{
   examples = std::string(var);
   hasChanged();
}

//============================="GET" METHODS====================================
std::string Style::getName() const
{
   return name;
}

std::string Style::getCategory() const
{
   return category;
}

std::string Style::getCategoryNumber() const
{
   return categoryNumber;
}

std::string Style::getStyleLetter() const
{
   return styleLetter;
}

std::string Style::getStyleGuide() const
{
   return styleGuide;
}

std::string Style::getType() const
{
   return type;
}

double Style::getOgMin() const
{
   return ogMin;
}

double Style::getOgMax() const
{
   return ogMax;
}

double Style::getFgMin() const
{
   return fgMin;
}

double Style::getFgMax() const
{
   return fgMax;
}

double Style::getIbuMin() const
{
   return ibuMin;
}

double Style::getIbuMax() const
{
   return ibuMax;
}

double Style::getColorMin_srm() const
{
   return colorMin_srm;
}

double Style::getColorMax_srm() const
{
   return colorMax_srm;
}

double Style::getCarbMin_vol() const
{
   return carbMin_vol;
}

double Style::getCarbMax_vol() const
{
   return carbMax_vol;
}

double Style::getAbvMin_pct() const
{
   return abvMin_pct;
}

double Style::getAbvMax_pct() const
{
   return abvMax_pct;
}

std::string Style::getNotes() const
{
   return notes;
}

std::string Style::getProfile() const
{
   return profile;
}

std::string Style::getIngredients() const
{
   return ingredients;
}

std::string Style::getExamples() const
{
   return examples;
}

bool Style::isValidType( const std::string &str )
{
   static const std::string types[] = {"Lager", "Ale", "Mead", "Wheat", "Mixed", "Cider"};
   static const unsigned int size = 6;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( str == types[i] )
         return true;
   
   return false;
}
