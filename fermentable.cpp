/*
 * fermentable.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "fermentable.h"
#include "stringparsing.h"

bool operator<(Fermentable &f1, Fermentable &f2)
{
   return f1.name < f2.name;
}

bool operator==(Fermentable &f1, Fermentable &f2)
{
   return f1.name == f2.name;
}

std::string Fermentable::toXml()
{
   std::string ret = "<FERMENTABLE>\n";
   
   ret += "<NAME>"+name+"</NAME>\n";
   ret += "<VERSION>"+intToString(version)+"</VERSION>\n";
   ret += "<TYPE>"+type+"</TYPE>\n";
   ret += "<AMOUNT>"+doubleToString(amount_kg)+"</AMOUNT>\n";
   ret += "<YIELD>"+doubleToString(yield_pct)+"</YIELD>\n";
   ret += "<COLOR>"+doubleToString(color_srm)+"</COLOR>\n";
   ret += "<ADD_AFTER_BOIL>"+boolToString(addAfterBoil)+"</ADD_AFTER_BOIL>\n";
   ret += "<ORIGIN>"+origin+"</ORIGIN>\n";
   ret += "<SUPPLIER>"+supplier+"</SUPPLIER>\n";
   ret += "<NOTES>"+notes+"</NOTES>\n";
   ret += "<COARSE_FINE_DIFF>"+doubleToString(coarseFineDiff_pct)+"</COARSE_FINE_DIFF>\n";
   ret += "<MOISTURE>"+doubleToString(moisture_pct)+"</MOISTURE>\n";
   ret += "<DIASTATIC_POWER>"+doubleToString(diastaticPower_lintner)+"</DIASTATIC_POWER>\n";
   ret += "<PROTEIN>"+doubleToString(protein_pct)+"</PROTEIN>\n";
   ret += "<MAX_IN_BATCH>"+doubleToString(maxInBatch_pct)+"</MAX_IN_BATCH>\n";
   ret += "<RECOMMEND_MASH>"+boolToString(recommendMash)+"</RECOMMEND_MASH>\n";
   ret += "<IBU_GAL_PER_LB>"+doubleToString(ibuGalPerLb)+"</IBU_GAL_PER_LB>\n";
   
   ret += "</FERMENTABLE>\n";
   
   return ret;
}

Fermentable::Fermentable()
{
   setDefaults();
}

Fermentable::Fermentable( Fermentable& other )
        : Observable()
{
   name = other.name;
   type = other.type;
   amount_kg = other.amount_kg;
   yield_pct = other.yield_pct;
   color_srm = other.color_srm;

   addAfterBoil = other.addAfterBoil;
   origin = other.origin;
   supplier = other.supplier;
   notes = other.notes;
   coarseFineDiff_pct = other.coarseFineDiff_pct;
   moisture_pct = other.moisture_pct;
   diastaticPower_lintner = other.diastaticPower_lintner;
   protein_pct = other.protein_pct;
   maxInBatch_pct = other.maxInBatch_pct;
   recommendMash = other.recommendMash;
   ibuGalPerLb = other.ibuGalPerLb;
}

Fermentable::Fermentable( const XmlNode* node )
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   XmlNode* leaf;
   unsigned int i, childrenSize;
   bool hasName=false, hasVersion=false, hasType=false, hasAmount=false, hasYield=false, hasColor=false;
   
   setDefaults();
   
   if( node->getTag() != "FERMENTABLE" )
      throw FermentableException("FERMENTABLE initializer not passed a FERMENTABLE node.");
   
   node->getChildren( children );
   childrenSize = children.size();
   
   for( i = 0; i < childrenSize; ++i )
   {
      tag = children[i]->getTag();
      children[i]->getChildren( tmpVec );
      
      // All valid children of FERMENTABLE only have zero or one child.
      if( tmpVec.size() > 1 )
      {
         children[i]->printParentTags();
         throw FermentableException("Tag \""+tag+"\" has more than one child.");
      }

      if( tmpVec.size() == 1 )
         leaf = tmpVec[0];
      else
         leaf = &XmlNode();
      
      // It must be a leaf if it is a valid BeerXML entry.
      if( ! leaf->isLeaf() )
         throw FermentableException("Should have been a leaf but is not.");
      
      if( tag == "NAME" )
      {
         setName(leaf->getLeafText());
         hasName=true;
      }
      else if( tag == "VERSION" )
      {
         if( parseInt(leaf->getLeafText()) != version )
            std::cerr << "Warning: FERMENTABLE is not version " << version << std::endl;
         hasVersion=true;
      }
      else if( tag == "TYPE" )
      {
         setType(leaf->getLeafText());
         hasType=true;
      }
      else if( tag == "AMOUNT" )
      {
         setAmount_kg(parseDouble(leaf->getLeafText()));
         hasAmount=true;
      }
      else if( tag == "YIELD" )
      {
         setYield_pct(parseDouble(leaf->getLeafText()));
         hasYield=true;
      }
      else if( tag == "COLOR" )
      {
         setColor_srm(parseDouble(leaf->getLeafText()));
         hasColor=true;
      }
      else if( tag == "ADD_AFTER_BOIL" )
         setAddAfterBoil(parseBool(leaf->getLeafText()));
      else if( tag == "ORIGIN" )
         setOrigin(leaf->getLeafText());
      else if( tag == "SUPPLIER" )
         setSupplier(leaf->getLeafText());
      else if( tag == "NOTES" )
         setNotes(leaf->getLeafText());
      else if( tag == "COARSE_FINE_DIFF" )
         setCoarseFineDiff_pct(parseDouble(leaf->getLeafText()));
      else if( tag == "MOISTURE" )
         setMoisture_pct(parseDouble(leaf->getLeafText()));
      else if( tag == "DIASTATIC_POWER" )
         setDiastaticPower_lintner(parseDouble(leaf->getLeafText()));
      else if( tag == "PROTEIN" )
         setProtein_pct(parseDouble(leaf->getLeafText()));
      else if( tag == "MAX_IN_BATCH" )
         setMaxInBatch_pct(parseDouble(leaf->getLeafText()));
      else if( tag == "RECOMMEND_MASH" )
         setRecommendMash( parseBool(leaf->getLeafText()) );
      else if( tag == "IBU_GAL_PER_LB" )
         setIbuGalPerLb(parseDouble(leaf->getLeafText()));
      else
      {
         std::cerr << "Warning: \"" << tag << "\" is not a recognized FERMENTABLE tag." << std::endl;
      } // end if..else
   } // end for(...)
   
   if( !hasName || !hasVersion || !hasType || !hasAmount || !hasYield || !hasColor )
      throw FermentableException("missing required fields.");
   
} // end Fermentable(...)

void Fermentable::setDefaults()
{
   name = "";
   type = "Grain";
   amount_kg = 0.0;
   yield_pct = 0.0;
   color_srm = 0.0;

   addAfterBoil = false;
   origin = "";
   supplier = "";
   notes = "";
   coarseFineDiff_pct = 0.0;
   moisture_pct = 0.0;
   diastaticPower_lintner = 0.0;
   protein_pct = 0.0;
   maxInBatch_pct = 0.0;
   recommendMash = false;
   ibuGalPerLb = 0.0;
}

// Get
const std::string& Fermentable::getName() const { return name; }
int Fermentable::getVersion() const { return version; }
const std::string& Fermentable::getType() const { return type; }
double Fermentable::getAmount_kg() const { return amount_kg; }
double Fermentable::getYield_pct() const { return yield_pct; }
double Fermentable::getColor_srm() const { return color_srm; }

bool Fermentable::getAddAfterBoil() const { return addAfterBoil; }
const std::string& Fermentable::getOrigin() const { return origin; }
const std::string& Fermentable::getSupplier() const { return supplier; }
const std::string& Fermentable::getNotes() const { return notes; }
double Fermentable::getCoarseFineDiff_pct() const { return coarseFineDiff_pct; }
double Fermentable::getMoisture_pct() const { return moisture_pct; }
double Fermentable::getDiastaticPower_lintner() const { return diastaticPower_lintner; }
double Fermentable::getProtein_pct() const { return protein_pct; }
double Fermentable::getMaxInBatch_pct() const { return maxInBatch_pct; }
bool Fermentable::getRecommendMash() const { return recommendMash; }
double Fermentable::getIbuGalPerLb() const { return ibuGalPerLb; }

// Set
void Fermentable::setName( const std::string& str ) { name = std::string(str); }
void Fermentable::setType( const std::string& str )
{
   if( isValidType( str ) )
   {
      type = std::string(str);
      hasChanged();
   }
   else
      throw FermentableException( "invalid type." );
}
void Fermentable::setAmount_kg( double num )
{
   if( num < 0.0 )
      throw FermentableException( "amount cannot be negative" );
   else
   {
      amount_kg = num;
      hasChanged();
   }
}
void Fermentable::setYield_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      yield_pct = num;
      hasChanged();
   }
   else
      throw FermentableException( "wrong range for a percent: " + doubleToString(num) );
}
void Fermentable::setColor_srm( double num )
{
   if( num < 0.0 )
      throw FermentableException( "color cannot be negative" );
   else
   {
      color_srm = num;
      hasChanged();
   }
}

void Fermentable::setAddAfterBoil( bool b ) { addAfterBoil = b; hasChanged();}
void Fermentable::setOrigin( const std::string& str ) { origin = std::string(str); hasChanged();}
void Fermentable::setSupplier( const std::string& str) { supplier = std::string(str); hasChanged();}
void Fermentable::setNotes( const std::string& str ) { notes = std::string(str); hasChanged();}
void Fermentable::setCoarseFineDiff_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      coarseFineDiff_pct = num;
      hasChanged();
   }
   else
      throw FermentableException( "wrong range for a percent: " + doubleToString(num) );
}
void Fermentable::setMoisture_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      moisture_pct = num;
      hasChanged();
   }
   else
      throw FermentableException( "wrong range for a percent: " + doubleToString(num) );
}
void Fermentable::setDiastaticPower_lintner( double num )
{
   if( num < 0.0 )
      throw FermentableException( "DP cannot be negative");
   else
   {
      diastaticPower_lintner = num;
      hasChanged();
   }
}
void Fermentable::setProtein_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      protein_pct = num;
      hasChanged();
   }
   else
      throw FermentableException( "wrong range for a percent: " + doubleToString(num) );
}
void Fermentable::setMaxInBatch_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      maxInBatch_pct = num;
      hasChanged();
   }
   else
      throw FermentableException( "wrong range for a percent: " + doubleToString(num) );
}
void Fermentable::setRecommendMash( bool b ) { recommendMash = b; hasChanged();}
void Fermentable::setIbuGalPerLb( double num ) { ibuGalPerLb = num; hasChanged();}

bool Fermentable::isValidType( const std::string& str )
{
   static const std::string validTypes[] = {"Grain", "Sugar", "Extract", "Dry Extract", "Adjunct"};
   unsigned int i, size = 5;
   
   for( i = 0; i < size; ++i )
      if( str == validTypes[i] )
         return true;
   
   return false;
}
