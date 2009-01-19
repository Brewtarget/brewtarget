/*
 * yeast.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "yeast.h"
#include "stringparsing.h"
#include "xmlnode.h"

std::string Yeast::toXml()
{
   std::string ret = "<YEAST>\n";
   
   ret += "<NAME>"+name+"</NAME>\n";
   ret += "<VERSION>"+intToString(version)+"</VERSION>\n";
   ret += "<TYPE>"+type+"</TYPE>\n";
   ret += "<FORM>"+form+"</FORM>\n";
   ret += "<AMOUNT>"+doubleToString(amount)+"</AMOUNT>\n";
   ret += "<AMOUNT_IS_WEIGHT>"+boolToString(amountIsWeight)+"</AMOUNT_IS_WEIGHT>\n";
   ret += "<LABORATORY>"+laboratory+"</LABORATORY>\n";
   ret += "<PRODUCT_ID>"+productID+"</PRODUCT_ID>\n";
   ret += "<MIN_TEMPERATURE>"+doubleToString(minTemperature_c)+"</MIN_TEMPERATURE>\n";
   ret += "<MAX_TEMPERATURE>"+doubleToString(maxTemperature_c)+"</MAX_TEMPERATURE>\n";
   ret += "<FLOCCULATION>"+flocculation+"</FLOCCULATION>\n";
   ret += "<ATTENUATION>"+doubleToString(attenuation_pct)+"</ATTENUATION>\n";
   ret += "<NOTES>"+notes+"</NOTES>\n";
   ret += "<BEST_FOR>"+bestFor+"</BEST_FOR>";
   ret += "<TIMES_CULTURED>"+intToString(timesCultured)+"</TIMES_CULTURED>\n";
   ret += "<MAX_REUSE>"+intToString(maxReuse)+"</MAX_REUSE>\n";
   ret += "<ADD_TO_SECONDARY>"+boolToString(addToSecondary)+"</ADD_TO_SECONDARY>\n";
   
   ret += "</YEAST>\n";
   return ret;
}

void Yeast::setDefaults()
{
   // Required fields.
   name = "";
   type = "Ale";
   form = "Liquid";
   amount = 0.0;
   
   // Optional fields.
   amountIsWeight = false;
   laboratory = "";
   productID = "";
   minTemperature_c = 0.0;
   maxTemperature_c = 0.0;
   flocculation = "";
   attenuation_pct = 0.0;
   notes = "";
   bestFor = "";
   timesCultured = 0;
   maxReuse = 0;
   addToSecondary = false;
}

Yeast::Yeast()
{
   setDefaults();
}

Yeast::Yeast(Yeast& other)
        : Observable()
{
   name = other.name;
   type = other.type;
   form = other.form;
   amount = other.amount;

   amountIsWeight = other.amountIsWeight;
   laboratory = other.laboratory;
   productID = other.productID;
   minTemperature_c = other.minTemperature_c;
   maxTemperature_c = other.maxTemperature_c;
   flocculation = other.flocculation;
   attenuation_pct = other.attenuation_pct;
   notes = other.notes;
   bestFor = other.bestFor;
   timesCultured = other.timesCultured;
   maxReuse = other.maxReuse;
   addToSecondary = other.addToSecondary;
}

Yeast::Yeast( XmlNode *node )
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   std::string leafText;
   XmlNode* leaf;
   unsigned int i, childrenSize;
   bool hasName=false, hasVersion=false, hasType=false, hasForm=false, hasAmount=false;
   
   setDefaults();
   
   if( node->getTag() != "YEAST" )
      throw YeastException("initializer not passed a YEAST node.");
   
   node->getChildren( children );
   childrenSize = children.size();
   
   for( i = 0; i < childrenSize; ++i )
   {
      tag = children[i]->getTag();
      children[i]->getChildren( tmpVec );
      
      // All valid children of YEAST only have zero or one child.
      if( tmpVec.size() > 1 )
         throw YeastException("Tag \""+tag+"\" has more than one child.");

      // Have to deal with the fact that this node might not have
      // and children at all.
      if( tmpVec.size() == 1 )
         leaf = tmpVec[0];
      else
         leaf = &XmlNode();
      
      // It must be a leaf if it is a valid BeerXML entry.
      if( ! leaf->isLeaf() )
         throw YeastException("Should have been a leaf but is not.");
      
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
            std::cerr << "Warning: XML YEAST version is not " << version << std::endl;
      }
      else if( tag == "TYPE" )
      {
         setType(leafText);
         hasType = true;
      }
      else if( tag == "FORM" )
      {
         setForm(leafText);
         hasForm = true;
      }
      else if( tag == "AMOUNT" )
      {
         setAmount(parseDouble(leafText));
         hasAmount = true;
      }
      else if( tag == "AMOUNT_IS_WEIGHT" )
         setAmountIsWeight(parseBool(leafText));
      else if( tag == "LABORATORY" )
         setLaboratory(leafText);
      else if( tag == "PRODUCT_ID" )
         setProductID(leafText);
      else if( tag == "MIN_TEMPERATURE" )
         setMinTemperature_c(parseDouble(leafText));
      else if( tag == "MAX_TEMPERATURE" )
         setMaxTemperature_c(parseDouble(leafText));
      else if( tag == "FLOCCULATION" )
         setFlocculation(leafText);
      else if( tag == "ATTENUATION" )
         setAttenuation_pct(parseDouble(leafText));
      else if( tag == "NOTES" )
         setNotes(leafText);
      else if( tag == "BEST_FOR" )
         setBestFor(leafText);
      else if( tag == "TIMES_CULTURED" )
         setTimesCultured(parseInt(leafText));
      else if( tag == "MAX_REUSE" )
         setMaxReuse(parseInt(leafText));
      else if( tag == "ADD_TO_SECONDARY" )
         setAddToSecondary(parseBool(leafText));
      else
      {
         std::cerr << "Warning: " << tag << " is not a recognized YEAST tag." << std::endl;
      } // end if..else
   } // end for(...)
   
   if( !hasName || !hasVersion || !hasType || !hasForm || !hasAmount )
      throw YeastException("missing required fields.");
} // end Yeast()

//============================="SET" METHODS====================================
void Yeast::setName( const std::string& var )
{
   name = std::string(var);
   hasChanged();
}

void Yeast::setType( const std::string& var )
{
   if( !isValidType(var) )
      throw YeastException("invalid type \"" + var + "\".");
   else
   {
      type = std::string(var);
      hasChanged();
   }
}

void Yeast::setForm( const std::string& var )
{
   if( ! isValidForm(var) )
      throw YeastException("invalid form \"" + var + "\".");
   else
   {
      form = std::string(var);
      hasChanged();
   }
}

void Yeast::setAmount( double var )
{
   if( var < 0.0 )
      throw YeastException("amount cannot be negative: " + doubleToString(var) );
   else
   {
      amount = var;
      hasChanged();
   }
}

void Yeast::setAmountIsWeight( bool var )
{
   amountIsWeight = var;
   hasChanged();
}

void Yeast::setLaboratory( const std::string& var )
{
   laboratory = std::string(var);
   hasChanged();
}

void Yeast::setProductID( const std::string& var )
{
   productID = std::string(var);
   hasChanged();
}

void Yeast::setMinTemperature_c( double var )
{
   if( var < -273.15 )
      throw YeastException("Temperature below absolute zero: " + doubleToString(var));
   else
   {
      minTemperature_c = var;
      hasChanged();
   }
}

void Yeast::setMaxTemperature_c( double var )
{
   if( var < -273.15 )
      throw YeastException("Temperature below absolute zero: " + doubleToString(var));
   else
   {
      maxTemperature_c = var;
      hasChanged();
   }
}

void Yeast::setFlocculation( const std::string& var )
{
   if( ! isValidFlocculation(var) )
      throw YeastException("invalid flocculation \"" + var + "\".");
   else
   {
      flocculation = std::string(var);
      hasChanged();
   }
}

void Yeast::setAttenuation_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      throw YeastException("invalid attenuation percentage: " + doubleToString(var) );
   else
   {
      attenuation_pct = var;
      hasChanged();
   }
}

void Yeast::setNotes( const std::string& var )
{
   notes = std::string(var);
   hasChanged();
}

void Yeast::setBestFor( const std::string& var )
{
   bestFor = std::string(var);
   hasChanged();
}

void Yeast::setTimesCultured( int var )
{
   if( var < 0 )
      throw YeastException("times cultured cannot be negative: " +  intToString(var) );
   else
   {
      timesCultured = var;
      hasChanged();
   }
}
void Yeast::setMaxReuse( int var )
{
   if( var < 0 )
      throw YeastException("max reuse cannot be negative: " +  intToString(var) );
   else
   {
      maxReuse = var;
      hasChanged();
   }
}

void Yeast::setAddToSecondary( bool var )
{
   addToSecondary = var;
   hasChanged();
}
   
//============================="GET" METHODS====================================
std::string Yeast::getName() const { return name; }
std::string Yeast::getType() const { return type; }
std::string Yeast::getForm() const { return form; }
double Yeast::getAmount() const { return amount; }
bool Yeast::getAmountIsWeight() const { return amountIsWeight; }
std::string Yeast::getLaboratory() const { return laboratory; }
std::string Yeast::getProductID() const { return productID; }
double Yeast::getMinTemperature_c() const { return minTemperature_c; }
double Yeast::getMaxTemperature_c() const { return maxTemperature_c; }
std::string Yeast::getFlocculation() const { return flocculation; }
double Yeast::getAttenuation_pct() const { return attenuation_pct; }
std::string Yeast::getNotes() const { return notes; }
std::string Yeast::getBestFor() const { return bestFor; }
int Yeast::getTimesCultured() const { return timesCultured; }
int Yeast::getMaxReuse() const { return maxReuse; }
bool Yeast::getAddToSecondary() const { return addToSecondary; }

bool Yeast::isValidType(const std::string& str) const
{
   static const std::string types[] = {"Ale", "Lager", "Wheat", "Wine", "Champagne"};
   unsigned int i, size = 5;
   
   for( i = 0; i < size; ++i )
      if( str == types[i] )
         return true;
   
   return false;
}

bool Yeast::isValidForm(const std::string& str) const
{
   static const std::string forms[] = {"Liquid", "Dry", "Slant", "Culture"};
   unsigned int i, size=4;
   
   for( i = 0; i < size; ++i )
      if( str == forms[i] )
         return true;
   
   return false;
}

bool Yeast::isValidFlocculation(const std::string& str) const
{
   static const std::string floc[] = {"Low", "Medium", "High", "Very High"};
   unsigned int i, size=4;
   
   for( i = 0; i < size; ++i )
      if( str == floc[i] )
         return true;
   
   return false;
}
