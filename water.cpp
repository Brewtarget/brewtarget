/*
 * water.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "stringparsing.h"
#include "water.h"

bool operator<(Water &w1, Water &w2)
{
   return w1.name < w2.name;
}

std::string Water::toXml()
{
   std::string ret = "<WATER>\n";
   
   ret += "<NAME>"+name+"</NAME>\n";
   ret += "<VERSION>"+intToString(version)+"</VERSION>\n";
   ret += "<AMOUNT>"+doubleToString(amount_l)+"</AMOUNT>\n";
   ret += "<CALCIUM>"+doubleToString(calcium_ppm)+"</CALCIUM>\n";
   ret += "<BICARBONATE>"+doubleToString(bicarbonate_ppm)+"</BICARBONATE>\n";
   ret += "<SULFATE>"+doubleToString(sulfate_ppm)+"</SULFATE>\n";
   ret += "<CHLORIDE>"+doubleToString(chloride_ppm)+"</CHLORIDE>\n";
   ret += "<SODIUM>"+doubleToString(sodium_ppm)+"</SODIUM>\n";
   ret += "<MAGNESIUM>"+doubleToString(magnesium_ppm)+"</MAGNESIUM>\n";
   ret += "<PH>"+doubleToString(ph)+"</PH>\n";
   ret += "<NOTES>"+notes+"</NOTES>\n";
   
   ret += "</WATER>\n";
   return ret;
}

//================================CONSTRUCTORS==================================
void Water::setDefaults()
{
   name = "";
   amount_l = 0.0;
   calcium_ppm = 0.0;
   bicarbonate_ppm = 0.0;
   chloride_ppm = 0.0;
   sodium_ppm = 0.0;
   magnesium_ppm = 0.0;
   ph = 7.0;
   notes = "";
}

Water::Water()
{
   setDefaults();
}

Water::Water( XmlNode *node )
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   std::string leafText;
   XmlNode* leaf;
   unsigned int i, childrenSize;
   bool hasName=false, hasVersion=false, hasAmount=false, hasCa=false, hasBic=false,
           hasSulf=false, hasChl=false, hasSodium=false, hasMag=false;
   
   setDefaults();
   
   if( node->getTag() != "WATER" )
      throw WaterException("initializer not passed a WATER node.");
   
   node->getChildren( children );
   childrenSize = children.size();
   
   for( i = 0; i < childrenSize; ++i )
   {
      tag = children[i]->getTag();
      children[i]->getChildren( tmpVec );
      
      // All valid children of WATER only have one child.
      if( tmpVec.size() != 1 )
         throw WaterException("Tag \""+tag+"\" has more than one child.");
      
      leaf = tmpVec[0];
      // It must be a leaf if it is a valid BeerXML entry.
      if( ! leaf->isLeaf() )
         throw WaterException("Should have been a leaf but is not.");
      
      leafText = leaf->getLeafText();
      
      if( tag == "NAME" )
      {
         setName(leafText);
         hasName = true;
      }
      else if( tag == "VERSION" )
      {
         if( parseInt(leafText) != version )
            std::cerr << "Warning: WATER version is not " << version << std::endl;
         
         hasVersion = true;
      }
      else if( tag == "AMOUNT" )
      {
         setAmount_l(parseDouble(leafText));
         hasAmount = true;
      }
      else if( tag == "CALCIUM" )
      {
         setCalcium_ppm(parseDouble(leafText));
         hasCa = true;
      }
      else if( tag == "BICARBONATE" )
      {
         setBicarbonate_ppm(parseDouble(leafText));
         hasBic = true;
      }
      else if( tag == "SULFATE" )
      {
         setSulfate_ppm(parseDouble(leafText));
         hasSulf = true;
      }
      else if( tag == "CHLORIDE" )
      {
         setChloride_ppm(parseDouble(leafText));
         hasChl = true;
      }
      else if( tag == "SODIUM" )
      {
         setSodium_ppm(parseDouble(leafText));
         hasSodium = true;
      }
      else if( tag == "MAGNESIUM" )
      {
         setMagnesium_ppm(parseDouble(leafText));
         hasMag = true;
      }
      else if( tag == "PH" )
      {
         setPh(parseDouble(leafText));
      }
      else if( tag == "NOTES" )
      {
         setNotes(leafText);
      }
      else
         std::cerr << "Warning: \"" << tag << "\" is not a supported WATER tag." << std::endl;
      
   } // end for(...)
   
   if( !hasName || !hasVersion || !hasAmount || !hasCa || !hasBic ||
           !hasSulf || !hasChl || !hasSodium || !hasMag )
      throw WaterException("missing required fields.");
} // end Water()

//================================"SET" METHODS=================================
void Water::setName( const std::string &var )
{
   name = std::string(var);
   hasChanged();
}

void Water::setAmount_l( double var )
{
   if( var < 0.0 )
      throw WaterException("amount cannot be negative: " + doubleToString(var) );
   else
   {
      amount_l = var;
      hasChanged();
   }
}

void Water::setCalcium_ppm( double var )
{
   if( var < 0.0 )
      throw WaterException("calcium cannot be negative: " + doubleToString(var) );
   else
   {
      calcium_ppm = var;
      hasChanged();
   }
}

void Water::setBicarbonate_ppm( double var )
{
   if( var < 0.0 )
      throw WaterException("bicarbonate cannot be negative: " + doubleToString(var) );
   else
   {
      bicarbonate_ppm = var;
      hasChanged();
   }
}

void Water::setChloride_ppm( double var )
{
   if( var < 0.0 )
      throw WaterException("chloride cannot be negative: " + doubleToString(var) );
   else
   {
      chloride_ppm = var;
      hasChanged();
   }
}

void Water::setSodium_ppm( double var )
{
   if( var < 0.0 )
      throw WaterException("sodium cannot be negative: " + doubleToString(var) );
   else
   {
      sodium_ppm = var;
      hasChanged();
   }
}

void Water::setMagnesium_ppm( double var )
{
   if( var < 0.0 )
      throw WaterException("magnesium cannot be negative: " + doubleToString(var) );
   else
   {
      magnesium_ppm = var;
      hasChanged();
   }
}

void Water::setPh( double var )
{
   if( var < 0.0 || var > 14.0 )
      throw WaterException("pH was not in [0,14]: " + doubleToString(var) );
   else
   {
      ph = var;
      hasChanged();
   }
}

void Water::setSulfate_ppm( double var )
{
   if( var < 0.0 )
      throw WaterException("sulfate cannot be negative: " + doubleToString(var));
   else
   {
      sulfate_ppm = var;
      hasChanged();
   }
}

void Water::setNotes( const std::string &var )
{
   notes = std::string(var);
   hasChanged();
}

//=========================="GET" METHODS=======================================
std::string Water::getName() const
{
   return name;
}

double Water::getSulfate_ppm() const
{
   return sulfate_ppm;
}

double Water::getAmount_l() const
{
   return amount_l;
}

double Water::getCalcium_ppm() const
{
   return calcium_ppm;
}

double Water::getBicarbonate_ppm() const
{
   return bicarbonate_ppm;
}

double Water::getChloride_ppm() const
{
   return chloride_ppm;
}

double Water::getSodium_ppm() const
{
   return sodium_ppm;
}

double Water::getMagnesium_ppm() const
{
   return magnesium_ppm;
}

double Water::getPh() const
{
   return ph;
}

std::string Water::getNotes() const
{
   return notes;
}
