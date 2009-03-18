/*
 * misc.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "misc.h"
#include "stringparsing.h"
#include "xmlnode.h"

bool operator<(Misc &m1, Misc &m2)
{
   return m1.name < m2.name;
}

bool operator==(Misc &m1, Misc &m2)
{
   return m1.name == m2.name;
}

std::string Misc::toXml()
{
   std::string ret = "<MISC>\n";
   
   ret += "<NAME>"+name+"</NAME>\n";
   ret += "<VERSION>"+intToString(version)+"</VERSION>\n";
   ret += "<TYPE>"+type+"</TYPE>\n";
   ret += "<USE>"+use+"</USE>\n";
   ret += "<TIME>"+doubleToString(time)+"</TIME>\n";
   ret += "<AMOUNT>"+doubleToString(amount)+"</AMOUNT>\n";
   ret += "<AMOUNT_IS_WEIGHT>"+boolToString(amountIsWeight)+"</AMOUNT_IS_WEIGHT>\n";
   ret += "<USE_FOR>"+useFor+"</USE_FOR>\n";
   ret += "<NOTES>"+notes+"</NOTES>\n";
   
   ret += "</MISC>\n";
   return ret;
}

//============================CONSTRUCTORS======================================
void Misc::setDefaults()
{
   name = "";
   type = "Other";
   use = "Boil";
   amount = 0.0;
   time = 0.0;
   
   amountIsWeight=false;
   useFor = "";
   notes = "";
}

Misc::Misc() : Observable()
{
   setDefaults();
}

Misc::Misc(Misc& other)
        : Observable()
{
   name = other.name;
   type = other.type;
   use = other.use;
   time = other.time;
   amount = other.amount;

   amountIsWeight = other.amountIsWeight;
   useFor = other.useFor;
   notes = other.notes;
}

Misc::Misc( const XmlNode * node ) : Observable()
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   std::string leafText;
   XmlNode* leaf;
   unsigned int i, childrenSize;
   bool hasName=false, hasVersion=false, hasType=false, hasUse=false, hasAmount=false, hasTime=false;
   
   setDefaults();
   
   if( node->getTag() != "MISC" )
      throw MiscException("initializer not passed a MISC node.");
   
   node->getChildren( children );
   childrenSize = children.size();
   
   for( i = 0; i < childrenSize; ++i )
   {
      tag = children[i]->getTag();
      children[i]->getChildren( tmpVec );
      
      // All valid children of MISC only have zero or one child.
      if( tmpVec.size() > 1 )
         throw MiscException("Tag \""+tag+"\" has more than one child.");
      
      // Have to deal with the fact that this node might not have
      // and children at all.
      if( tmpVec.size() == 1 )
         leaf = tmpVec[0];
      else
         leaf = &XmlNode();

      // It must be a leaf if it is a valid BeerXML entry.
      if( ! leaf->isLeaf() )
         throw MiscException("Should have been a leaf but is not.");
      
      leafText = leaf->getLeafText();
      
      if( tag == "NAME" )
      {
         setName(leafText);
         hasName = true;
      }
      else if( tag == "VERSION" )
      {
         hasVersion = true;
         if( parseInt(leafText) != version )
            std::cerr << "Warning: XML MISC version is not " << version << std::endl;
      }
      else if( tag == "TYPE" )
      {
         setType(leafText);
         hasType = true;
      }
      else if( tag == "USE" )
      {
         setUse(leafText);
         hasUse = true;
      }
      else if( tag == "TIME" )
      {
         setTime(parseDouble(leafText));
         hasTime = true;
      }
      else if( tag == "AMOUNT" )
      {
         setAmount(parseDouble(leafText));
         hasAmount = true;
      }
      else if( tag == "AMOUNT_IS_WEIGHT" )
         setAmountIsWeight(parseBool(leafText));
      else if( tag == "USE_FOR" )
         setUseFor(leafText);
      else if( tag == "NOTES" )
         setNotes(leafText);
      else
         std::cerr << "Warning: MISC tag \"" << tag << "\" is not recognized." << std::endl;
   } // end for(...)
   
   if( !hasName || !hasVersion || !hasType || !hasUse || !hasAmount || !hasTime )
      throw MiscException("one of the required fields is missing.");
} //end Misc()

//============================"GET" METHODS=====================================
std::string Misc::getName() const
{
   return name;
}

std::string Misc::getType() const
{
   return type;
}

std::string Misc::getUse() const
{
   return use;
}

double Misc::getAmount() const
{
   return amount;
}

double Misc::getTime() const
{
   return time;
}

bool Misc::getAmountIsWeight() const
{
   return amountIsWeight;
}

std::string Misc::getUseFor() const
{
   return useFor;
}

std::string Misc::getNotes() const
{
   return notes;
}

//============================"SET" METHODS=====================================
void Misc::setName( const std::string &var )
{
   name = std::string(var);
   hasChanged();
}

void Misc::setType( const std::string &var )
{
   if( ! isValidType(var) )
      throw MiscException("\""+var+"\" is not a valid type.");
   else
   {
      type = std::string(var);
      hasChanged();
   }
}

void Misc::setUse( const std::string &var )
{
   if( ! isValidUse(var) )
      throw MiscException("\""+var+"\" is not a valid use.");
   else
   {
      use = std::string(var);
      hasChanged();
   }
}

void Misc::setAmount( double var )
{
   if( var < 0.0 )
      throw MiscException("amount cannot be negative: " + doubleToString(var) );
   else
   {
      amount = var;
      hasChanged();
   }
}

void Misc::setTime( double var )
{
   if( var < 0.0 )
      throw MiscException("time cannot be negative: " + doubleToString(var) );
   else
   {
      time = var;
      hasChanged();
   }
}

void Misc::setAmountIsWeight( bool var )
{
   amountIsWeight = var;
   hasChanged();
}

void Misc::setUseFor( const std::string &var )
{
   useFor = std::string(var);
   hasChanged();
}

void Misc::setNotes( const std::string &var )
{
   notes = std::string(var);
   hasChanged();
}

//========================OTHER METHODS=========================================

bool Misc::isValidUse( const std::string &var )
{
   static const std::string uses[] = {"Boil", "Mash", "Primary", "Secondary", "Bottling"};
   static const unsigned int size = 5;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( var == uses[i] )
         return true;
   
   return false;
}

bool Misc::isValidType( const std::string &var )
{
   static const std::string types[] = {"Spice", "Fining", "Water Agent", "Herb", "Flavor", "Other"};
   static const unsigned int size = 6;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( var == types[i] )
         return true;
   
   return false;
}
