/*
 * mash.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "mash.h"
#include "mashstep.h"

std::string Mash::toXml()
{
   unsigned int i, size = mashSteps.size();
   std::string ret = "<MASH>\n";
   
   ret += "<NAME>"+name+"</NAME>\n";
   ret += "<VERSION>"+intToString(version)+"</VERSION>\n";
   ret += "<GRAIN_TEMP>"+doubleToString(grainTemp_c)+"</GRAIN_TEMP>\n";
   ret += "<MASH_STEPS>\n";
   for( i = 0; i < size; ++i )
      ret += mashSteps[i]->toXml();
   ret += "</MASH_STEPS>\n";
   ret += "<NOTES>"+notes+"</NOTES>\n";
   ret += "<TUN_TEMP>"+doubleToString(tunTemp_c)+"</TUN_TEMP>\n";
   ret += "<SPARGE_TEMP>"+doubleToString(spargeTemp_c)+"</SPARGE_TEMP>\n";
   ret += "<PH>"+doubleToString(ph)+"</PH>\n";
   ret += "<TUN_WEIGHT>"+doubleToString(tunWeight_kg)+"</TUN_WEIGHT>\n";
   ret += "<TUN_SPECIFIC_HEAT>"+doubleToString(tunSpecificHeat_calGC)+"</TUN_SPECIFIC_HEAT>\n";
   ret += "<EQUIP_ADJUST>"+boolToString(equipAdjust)+"</EQUIP_ADJUST>\n";
   
   ret += "</MASH>\n";
   
   return ret;
}

void Mash::setDefaults()
{
   name = "";
   grainTemp_c = 0.0;
   mashSteps = std::vector<MashStep *>();
   notes = "";
   tunTemp_c = 0.0;
   spargeTemp_c = 0.0;
   ph = 7.0;
   tunWeight_kg = 0.0;
   tunSpecificHeat_calGC = 0.0;
   equipAdjust = false;
}

Mash::Mash()
{
   setDefaults();
}

Mash::Mash(const XmlNode *node)
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   std::string leafText;
   XmlNode* leaf;
   unsigned int i, childrenSize;
   bool hasName=false, hasVersion=false, hasGrainTemp=false, hasMashStep=false;
   
   setDefaults();
   
   if( node->getTag() != "MASH" )
      throw MashException("initializer not passed a MASH node.");
   
   node->getChildren( children );
   childrenSize = children.size();
   
   for( i = 0; i < childrenSize; ++i )
   {
      tag = children[i]->getTag();
      children[i]->getChildren( tmpVec );
      
      if( tmpVec.size() == 0 )
         leaf = &XmlNode();
      else
         leaf = tmpVec[0]; // May not really be a leaf.
      
      if( leaf->isLeaf() )
         leafText = leaf->getLeafText();

      if( tag == "NAME" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         setName(leaf->getLeafText());
         hasName = true;
      }
      else if( tag == "VERSION" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         if( parseInt(leaf->getLeafText()) != version )
            std::cerr << "Warning: XML MASH is not version " << version << std::endl;
         
         hasVersion = true;
      }
      else if( tag == "GRAIN_TEMP" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         setGrainTemp_c(parseDouble(leaf->getLeafText()));
         hasGrainTemp=true;
      }
      else if( tag == "MASH_STEPS" )
      {
         MashStep *a;
         unsigned int j;
         for( j = 0; j < tmpVec.size(); ++j )
         {
            a = new MashStep(tmpVec[j]);
            // TODO: need to check to make sure tmpVec[j] is a Mash node.
            mashSteps.push_back(a);
         }
         hasMashStep=true;
      }
      else if( tag == "NOTES" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         setNotes(leafText);
      }
      else if( tag == "TUN_TEMP" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         setTunTemp_c(parseDouble(leafText));
      }
      else if( tag == "SPARGE_TEMP" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         setSpargeTemp_c(parseDouble(leafText));
      }
      else if( tag == "PH" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         setPh(parseDouble(leafText));
      }
      else if( tag == "TUN_WEIGHT" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         setTunWeight_kg(parseDouble(leafText));
      }
      else if( tag == "TUN_SPECIFIC_HEAT" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         setTunSpecificHeat_calGC(parseDouble(leafText));
      }
      else if( tag == "EQUIP_ADJUST" )
      {
         if( ! leaf->isLeaf() )
            throw MashException("Should have been a leaf but is not.");
         
         setEquipAdjust(parseBool(leafText));
      }
      else
         std::cerr << "Warning: non-standard tag: " << tag << std::endl;
   } // end for()
   
   if( !hasName || !hasVersion || !hasGrainTemp || !hasMashStep )
      throw MashException("missing required field.");
}// end Mash()

void Mash::setName( const std::string& var )
{
   name = std::string(var);
}

void Mash::setGrainTemp_c( double var )
{
   grainTemp_c = var;
}

void Mash::setNotes( const std::string& var )
{
   notes = std::string(var);
}

void Mash::setTunTemp_c( double var )
{
   tunTemp_c = var;
}

void Mash::setSpargeTemp_c( double var )
{
   spargeTemp_c = var;
}

void Mash::setPh( double var )
{
   if( var < 0.0 || var > 14.0 )
      throw MashException("invalid PH: " + doubleToString(var) );
   else
      ph = var;
}

void Mash::setTunWeight_kg( double var )
{
   if( var < 0.0 )
      throw MashException("invalid weight: " + doubleToString(var) );
   else
      tunWeight_kg = var;
}

void Mash::setTunSpecificHeat_calGC( double var )
{
   if( var < 0.0 )
      throw MashException("invalid specific heat: " + doubleToString(var) );
   else
      tunSpecificHeat_calGC = var;
}

void Mash::setEquipAdjust( bool var )
{
   equipAdjust = var;
}

//============================="GET" METHODS====================================
std::string Mash::getName() const
{
   return name;
}

double Mash::getGrainTemp_c() const
{
   return grainTemp_c;
}

unsigned int Mash::getNumMashSteps() const
{
   return mashSteps.size();
}

MashStep* Mash::getMashStep( unsigned int i )
{
   if( i >= mashSteps.size() )
      throw MashException("getMashStep(): index too large: " + intToString(i));
   else
      return mashSteps[i];
}

std::string Mash::getNotes() const
{
   return notes;
}

double Mash::getTunTemp_c() const
{
   return tunTemp_c;
}

double Mash::getSpargeTemp_c() const
{
   return spargeTemp_c;
}

double Mash::getPh() const
{
   return ph;;
}

double Mash::getTunWeight_kg() const
{
   return tunWeight_kg;
}

double Mash::getTunSpecificHeat_calGC() const
{
   return tunSpecificHeat_calGC;
}

bool Mash::getEquipAdjust() const
{
   return equipAdjust;
}
