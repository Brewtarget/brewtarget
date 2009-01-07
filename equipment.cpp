/*
 * equipment.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "equipment.h"

std::string Equipment::toXml()
{
   std::string ret = "<EQUIPMENT>\n";
   
   ret += "<NAME>"+name+"</NAME>\n";
   ret += "<VERSION>"+intToString(version)+"</VERSION>\n";
   ret += "<BOIL_SIZE>"+doubleToString(boilSize_l)+"</BOIL_SIZE>\n";
   ret += "<BATCH_SIZE>"+doubleToString(batchSize_l)+"</BATCH_SIZE>\n";
   ret += "<TUN_VOLUME>"+doubleToString(tunVolume_l)+"</TUN_VOLUME>\n";
   ret += "<TUN_WEIGHT>"+doubleToString(tunWeight_kg)+"</TUN_WEIGHT>\n";
   ret += "<TUN_SPECIFIC_HEAT>"+doubleToString(tunSpecificHeat_calGC)+"</TUN_SPECIFIC_HEAT>\n";
   ret += "<TOP_UP_WATER>"+doubleToString(topUpWater_l)+"</TOP_UP_WATER>\n";
   ret += "<TRUB_CHILLER_LOSS>"+doubleToString(trubChillerLoss_l)+"</TRUB_CHILLER_LOSS>\n";
   ret += "<EVAP_RATE>"+doubleToString(evapRate_pctHr)+"</EVAP_RATE>\n";
   ret += "<BOIL_TIME>"+doubleToString(boilTime_hrs)+"</BOIL_TIME>\n";
   ret += "<CALC_BOIL_VOLUME>"+boolToString(calcBoilVolume)+"</CALC_BOIL_VOLUME>\n";
   ret += "<LAUTER_DEADSPACE>"+doubleToString(lauterDeadspace_l)+"</LAUTER_DEADSPACE>\n";
   ret += "<TOP_UP_KETTLE>"+doubleToString(topUpKettle_l)+"</TOP_UP_KETTLE>\n";
   ret += "<HOP_UTILIZATION>"+doubleToString(hopUtilization_pct)+"</HOP_UTILIZATION>\n";
   ret += "<NOTES>"+notes+"</NOTES>\n";
   
   ret += "</EQUIPMENT>\n";
   
   return ret;
}

//=============================CONSTRUCTORS=====================================

void Equipment::setDefaults()
{
   name = "";
   boilSize_l = 0.0;
   batchSize_l = 0.0;
   tunVolume_l = 0.0;
   tunWeight_kg = 0.0;
   tunSpecificHeat_calGC = 0.0;
   topUpWater_l = 0.0;
   trubChillerLoss_l = 0.0;
   evapRate_pctHr = 0.0;
   boilTime_hrs = 0.0;
   calcBoilVolume = false;
   lauterDeadspace_l = 0.0;
   topUpKettle_l = 0.0;
   hopUtilization_pct = 0.0;
   notes = "";
}

Equipment::Equipment()
{
   setDefaults();
}

Equipment::Equipment(XmlNode *node)
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   std::string leafText;
   XmlNode* leaf;
   unsigned int i, childrenSize;
   bool hasName=false, hasVersion=false, hasBoilSize=false, hasBatchSize=false;
   
   setDefaults();
   
   if( node->getTag() != "EQUIPMENT" )
      throw EquipmentException("initializer not passed an EQUIPMENT node.");
   
   node->getChildren( children );
   childrenSize = children.size();
   
   for( i = 0; i < childrenSize; ++i )
   {
      tag = children[i]->getTag();
      children[i]->getChildren( tmpVec );
      
      // All valid children of EQUIPMENT only have one child.
      if( tmpVec.size() != 1 )
         throw EquipmentException("Tag \""+tag+"\" has more than one child.");
      
      leaf = tmpVec[0];
      // It must be a leaf if it is a valid BeerXML entry.
      if( ! leaf->isLeaf() )
         throw EquipmentException("Should have been a leaf but is not.");
      
      leafText = leaf->getLeafText();
      
      if( tag == "NAME" )
      {
         setName(leafText);
         hasName = true;
      }
      else if( tag == "VERSION" )
      {
         if( parseInt(leafText) != version )
            std::cerr << "Warning: XML EQUIPMENT version is not " << version << std::endl;
      }
      else if( tag == "BOIL_SIZE" )
      {
         setBoilSize_l(parseDouble(leafText));
         hasBoilSize=true;
      }
      else if( tag == "BATCH_SIZE" )
      {
         setBatchSize_l(parseDouble(leafText));
         hasBatchSize=true;
      }
      else if( tag == "TUN_VOLUME" )
         setTunVolume_l(parseDouble(leafText));
      else if( tag == "TUN_WEIGHT" )
         setTunWeight_kg(parseDouble(leafText));
      else if( tag == "TUN_SPECIFIC_HEAT" )
         setTunSpecificHeat_calGC(parseDouble(leafText));
      else if( tag == "TOP_UP_WATER" )
         setTopUpWater_l(parseDouble(leafText));
      else if( tag == "TRUB_CHILLER_LOSS" )
         setTrubChillerLoss_l(parseDouble(leafText));
      else if( tag == "EVAP_RATE" )
         setEvapRate_pctHr(parseDouble(leafText));
      else if( tag == "BOIL_TIME" )
         setBoilTime_hrs(parseDouble(leafText));
      else if( tag == "CALC_BOIL_VOLUME" )
         setCalcBoilVolume(parseBool(leafText));
      else if( tag == "LAUTER_DEADSPACE" )
         setLauterDeadspace_l(parseDouble(leafText));
      else if( tag == "TOP_UP_KETTLE" )
         setTopUpKettle_l(parseDouble(leafText));
      else if( tag == "HOP_UTILIZATION" )
         setHopUtilization_pct(parseDouble(leafText));
      else if( tag == "NOTES" )
         setNotes(leafText);
      else
         std::cerr << "Warning: Unsupported EQUIPMENT tag: " << tag << std::endl;
   } // end for
   
   if( !hasName || !hasVersion || !hasBoilSize || !hasBatchSize )
      throw EquipmentException("missing required tag.");
} // end Equipment()

//============================"SET" METHODS=====================================

void Equipment::setName( const std::string &var )
{
   name = std::string(var);
}

void Equipment::setBoilSize_l( double var )
{
   if( var < 0.0 )
      throw EquipmentException("boil size cannot be negative: " + doubleToString(var) );
   else
      boilSize_l = var;
}

void Equipment::setBatchSize_l( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "batch size cannot be negative: " + doubleToString(var) );
   else
      batchSize_l = var;
}

void Equipment::setTunVolume_l( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "tun volume cannot be negative: " + doubleToString(var) );
   else
      tunVolume_l = var;
}

void Equipment::setTunWeight_kg( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "tun weight cannot be negative: " + doubleToString(var) );
   else
      tunWeight_kg = var;
}

void Equipment::setTunSpecificHeat_calGC( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "tun specific heat cannot be negative: " + doubleToString(var) );
   else
      tunSpecificHeat_calGC = var;
}

void Equipment::setTopUpWater_l( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "top up water cannot be negative: " + doubleToString(var) );
   else
      topUpWater_l = var;
}

void Equipment::setTrubChillerLoss_l( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "trub chiller loss cannot be negative: " + doubleToString(var) );
   else
      trubChillerLoss_l = var;
}

void Equipment::setEvapRate_pctHr( double var )
{
   if( var < 0.0 || var > 100.0)
      throw EquipmentException( "evap rate must be a percent: " + doubleToString(var) );
   else
      evapRate_pctHr = var;
}

void Equipment::setBoilTime_hrs( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "boil time cannot be negative: " + doubleToString(var) );
   else
      boilTime_hrs = var;
}

void Equipment::setCalcBoilVolume( bool var )
{
   calcBoilVolume = var;
}

void Equipment::setLauterDeadspace_l( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "lauter deadspace cannot be negative: " + doubleToString(var) );
   else
      lauterDeadspace_l = var;
}

void Equipment::setTopUpKettle_l( double var )
{
   if( var < 0.0 )
      throw EquipmentException( "top up kettle cannot be negative: " + doubleToString(var) );
   else
      topUpKettle_l = var;
}

void Equipment::setHopUtilization_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      throw EquipmentException( "hop utilization must be a percentage: " + doubleToString(var) );
   else
      ;
}

void Equipment::setNotes( const std::string &var )
{
   notes = std::string(var);
}

//============================"GET" METHODS=====================================

std::string Equipment::getName() const
{
   return name;
}

double Equipment::getBoilSize_l() const
{
   return boilSize_l;
}

double Equipment::getBatchSize_l() const
{
   return batchSize_l;
}

double Equipment::getTunVolume_l() const
{
   return tunVolume_l;
}

double Equipment::getTunWeight_kg() const
{
   return tunWeight_kg;
}

double Equipment::getTunSpecificHeat_calGC() const
{
   return tunSpecificHeat_calGC;
}

double Equipment::getTopUpWater_l() const
{
   return topUpWater_l;
}

double Equipment::getTrubChillerLoss_l() const
{
   return trubChillerLoss_l;
}

double Equipment::getEvapRate_pctHr() const
{
   return evapRate_pctHr;
}

double Equipment::getBoilTime_hrs() const
{
   return boilTime_hrs;
}

bool Equipment::getCalcBoilVolume() const
{
   return calcBoilVolume;
}

double Equipment::getLauterDeadspace_l() const
{
   return lauterDeadspace_l;
}

double Equipment::getTopUpKettle_l() const
{
   return topUpKettle_l;
}

double Equipment::getHopUtilization_pct() const
{
   return hopUtilization_pct;
}

std::string Equipment::getNotes() const
{
   return notes;
}
