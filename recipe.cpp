/*
 * recipe.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "instruction.h"
#include "brewtarget.h"
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "xmlnode.h"
#include "stringparsing.h"
#include "recipe.h"
#include "style.h"
#include "misc.h"
#include "mash.h"
#include "hop.h"
#include "fermentable.h"
#include "equipment.h"
#include "yeast.h"
#include "water.h"
#include "hoputilization.h"
#include "PreInstruction.h"

bool operator<(Recipe &r1, Recipe &r2 )
{
   return r1.name < r2.name;
}

bool operator==(Recipe &r1, Recipe &r2 )
{
   return r1.name == r2.name;
}

std::string Recipe::toXml()
{
   unsigned int i, size;
   std::string ret = "<RECIPE>\n";
   
   ret += "<NAME>"+name+"</NAME>\n";
   ret += "<VERSION>"+intToString(version)+"</VERSION>\n";
   ret += "<TYPE>"+type+"</TYPE>\n";
   ret += "<BREWER>"+brewer+"</BREWER>\n";
   ret += style->toXml();
   ret += "<BATCH_SIZE>"+doubleToString(batchSize_l)+"</BATCH_SIZE>\n";
   ret += "<BOIL_SIZE>"+doubleToString(boilSize_l)+"</BOIL_SIZE>\n";
   ret += "<BOIL_TIME>"+doubleToString(boilTime_min)+"</BOIL_TIME>\n";
   ret += "<EFFICIENCY>"+doubleToString(efficiency_pct)+"</EFFICIENCY>\n";
   ret += "<HOPS>\n"; size = hops.size();
   for(i = 0; i < size; ++i )
      ret += hops[i]->toXml();
   ret += "</HOPS>\n";
   ret += "<FERMENTABLES>\n"; size = fermentables.size();
   for(i = 0; i < size; ++i )
      ret += fermentables[i]->toXml();
   ret += "</FERMENTABLES>\n";
   ret += "<MISCS>\n"; size = miscs.size();
   for( i = 0; i < size; ++i )
      ret += miscs[i]->toXml();
   ret += "</MISCS>\n";
   ret += "<YEASTS>\n"; size = yeasts.size();
   for( i = 0; i < size; ++i )
      ret += yeasts[i]->toXml();
   ret += "</YEASTS>\n";
   ret += "<WATERS>\n"; size = waters.size();
   for( i = 0; i < size; ++i )
      ret += waters[i]->toXml();
   ret += "</WATERS>\n";
   ret += mash->toXml();
   
   ret += "<ASST_BREWER>"+asstBrewer+"</ASST_BREWER>\n";
   if( equipment )
      ret += equipment->toXml();
   ret += "<NOTES>"+notes+"</NOTES>\n";
   ret += "<TASTE_NOTES>"+tasteNotes+"</TASTE_NOTES>\n";
   ret += "<TASTE_RATING>"+doubleToString(tasteRating)+"</TASTE_RATING>\n";
   ret += "<OG>"+doubleToString(og)+"</OG>\n";
   ret += "<FG>"+doubleToString(fg)+"</FG>\n";
   ret += "<FERMENTATION_STAGES>"+intToString(fermentationStages)+"</FERMENTATION_STAGES>\n";
   ret += "<PRIMARY_AGE>"+doubleToString(primaryAge_days)+"</PRIMARY_AGE>\n";
   ret += "<PRIMARY_TEMP>"+doubleToString(primaryTemp_c)+"</PRIMARY_TEMP>\n";
   ret += "<SECONDARY_AGE>"+doubleToString(secondaryAge_days)+"</SECONDARY_AGE>\n";
   ret += "<SECONDARY_TEMP>"+doubleToString(secondaryTemp_c)+"</SECONDARY_TEMP>\n";
   ret += "<TERTIARY_AGE>"+doubleToString(tertiaryAge_days)+"</TERTIARY_AGE>\n";
   ret += "<TERTIARY_TEMP>"+doubleToString(tertiaryTemp_c)+"</TERTIARY_TEMP>\n";
   ret += "<AGE>"+doubleToString(age_days)+"</AGE>\n";
   ret += "<AGE_TEMP>"+doubleToString(ageTemp_c)+"</AGE_TEMP>\n";
   ret += "<DATE>"+date+"</DATE>\n";
   ret += "<CARBONATION>"+doubleToString(carbonation_vols)+"</CARBONATION>\n";
   ret += "<FORCED_CARBONATION>"+boolToString(forcedCarbonation)+"</FORCED_CARBONATION>\n";
   ret += "<PRIMING_SUGAR_NAME>"+primingSugarName+"</PRIMING_SUGAR_NAME>\n";
   ret += "<CARBONATION_TEMP>"+doubleToString(carbonationTemp_c)+"</CARBONATION_TEMP>\n";
   ret += "<PRIMING_SUGAR_EQUIV>"+doubleToString(primingSugarEquiv)+"</PRIMING_SUGAR_EQUIV>\n";
   ret += "<KEG_PRIMING_FACTOR>"+doubleToString(kegPrimingFactor)+"</KEG_PRIMING_FACTOR>\n";
   
   ret += "</RECIPE>\n";
   return ret;
}

//=================================CONSTRUCTORS=================================
void Recipe::setDefaults()
{
   name = "";
   type = "All Grain";
   brewer = "Nobody";
   style = new Style();
   batchSize_l = 0.0;
   boilSize_l = 0.0;
   boilTime_min = 0.0;
   efficiency_pct = 0.0;
   hops = std::vector<Hop*>();
   fermentables = std::vector<Fermentable*>();
   miscs = std::vector<Misc*>();
   yeasts = std::vector<Yeast*>();
   waters = std::vector<Water*>();
   mash = new Mash();

   asstBrewer = "";
   equipment = NULL;
   notes = "";
   tasteNotes = "";
   tasteRating = 0.0;
   og = 0.0;
   fg = 0.0;
   fermentationStages = 0;
   primaryAge_days = 0.0;
   primaryTemp_c = 0.0;
   secondaryAge_days = 0.0;
   secondaryTemp_c = 0.0;
   tertiaryAge_days = 0.0;
   tertiaryTemp_c = 0.0;
   age_days = 0.0;
   ageTemp_c = 0.0;
   date = "";
   carbonation_vols = 0.0;
   forcedCarbonation = false;
   primingSugarName = "";
   carbonationTemp_c = 0.0;
   primingSugarEquiv = 0.0;
   kegPrimingFactor = 0.0;
}

Recipe::Recipe()
{
   setDefaults();
}

Recipe::Recipe(const XmlNode *node)
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   std::string leafText;
   XmlNode* leaf;
   unsigned int i, childrenSize;
   bool hasVersion=false;
   
   setDefaults();
   
   if( node->getTag() != "RECIPE" )
      throw RecipeException("initializer not passed a RECIPE node.");
   
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
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setName(leafText);
      }
      else if( tag == "VERSION" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         if( parseInt(leafText) != version )
            std::cerr << "Warning: XML RECIPE version is not " << version << std::endl;
         hasVersion=true;
      }
      else if( tag == "TYPE" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setType(leafText);
      }
      else if( tag == "BREWER" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setBrewer(leafText);
      }
      else if( tag == "STYLE" )
      {
         setStyle(new Style(children[i]));
      }
      else if( tag == "BATCH_SIZE" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setBatchSize_l(parseDouble(leafText));
      }
      else if( tag == "BOIL_SIZE" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setBoilSize_l(parseDouble(leafText));
      }
      else if( tag == "BOIL_TIME" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setBoilTime_min(parseDouble(leafText));
      }
      else if( tag == "EFFICIENCY" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setEfficiency_pct(parseDouble(leafText));
      }
      else if( tag == "HOPS" )
      {
         Hop *h;
         unsigned int j;
         
         for( j = 0; j < tmpVec.size(); ++j )
         {
            h = new Hop(tmpVec[j]);
            addHop(h);
         }
      }
      else if( tag == "FERMENTABLES" )
      {
         unsigned int j;
         
         for( j = 0; j < tmpVec.size(); ++j )
            addFermentable(new Fermentable(tmpVec[j]));
      }
      else if( tag == "MISCS" )
      {
         unsigned int j;
         
         for( j = 0; j < tmpVec.size(); ++j )
            addMisc(new Misc(tmpVec[j]));
      }
      else if( tag == "YEASTS" )
      {
         unsigned int j;
         
         for( j = 0; j < tmpVec.size(); ++j )
            addYeast(new Yeast(tmpVec[j]));
      }
      else if( tag == "WATERS" )
      {
         unsigned int j;
         
         for( j = 0; j < tmpVec.size(); ++j )
            addWater(new Water(tmpVec[j]));
      }
      else if( tag == "MASH" )
      {
         setMash(new Mash(children[i]));
      }
      else if( tag == "ASST_BREWER" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setAsstBrewer(leafText);
      }
      else if( tag == "EQUIPMENT" )
      {
         setEquipment(new Equipment(children[i]));
      }
      else if( tag == "NOTES" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setNotes(leafText);
      }
      else if( tag == "TASTE_NOTES" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setTasteNotes(leafText);
      }
      else if( tag == "TASTE_RATING" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setTasteRating(parseDouble(leafText));
      }
      else if( tag == "OG" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setOg(parseDouble(leafText));
      }
      else if( tag == "FG" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setFg(parseDouble(leafText));
      }
      else if( tag == "FERMENTATION_STAGES" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setFermentationStages(parseInt(leafText));
      }
      else if( tag == "PRIMARY_AGE" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setPrimaryAge_days(parseDouble(leafText));
      }
      else if( tag == "PRIMARY_TEMP" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setPrimaryTemp_c(parseDouble(leafText));
      }
      else if( tag == "SECONDARY_AGE" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setSecondaryAge_days(parseDouble(leafText));
      }
      else if( tag == "SECONDARY_TEMP" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setSecondaryTemp_c(parseDouble(leafText));
      }
      else if( tag == "TERTIARY_AGE")
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setTertiaryAge_days(parseDouble(leafText));
      }
      else if( tag == "TERTIARY_TEMP" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setTertiaryTemp_c(parseDouble(leafText));
      }
      else if( tag == "AGE" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setAge_days(parseDouble(leafText));
      }
      else if( tag == "AGE_TEMP" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setAgeTemp_c(parseDouble(leafText));
      }
      else if( tag == "DATE" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setDate(leafText);
      }
      else if( tag == "CARBONATION" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setCarbonation_vols(parseDouble(leafText));
      }
      else if( tag == "FORCED_CARBONATION" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setForcedCarbonation(parseBool(leafText));
      }
      else if( tag == "PRIMING_SUGAR_NAME" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setPrimingSugarName(leafText);
      }
      else if( tag == "CARBONATION_TEMP" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setCarbonationTemp_c(parseDouble(leafText));
      }
      else if( tag == "PRIMING_SUGAR_EQUIV" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setPrimingSugarEquiv(parseDouble(leafText));
      }
      else if( tag == "KEG_PRIMING_FACTOR" )
      {
         if( !leaf->isLeaf())
            throw RecipeException("expected a leaf");
         setKegPrimingFactor(parseDouble(leafText));
      }
      else
         std::cerr << "Warning: unsupported RECIPE tag: " << tag << std::endl;
   }// end for()

   // I am purposely being slack about checking for all required fields.
   // Seems to me BeerXML is a little too strict with Recipes.
   if( ! hasVersion )
      throw RecipeException("Recipe lacks version tag.");
} // end Recipe()

void Recipe::addInstruction(Instruction* ins)
{
   if( ins == 0 )
      return;
   
   instructions.push_back(ins);
   hasChanged(QVariant(Recipe::INSTRUCTION));
}

void Recipe::removeInstruction(Instruction* ins)
{
   std::vector<Instruction*>::iterator it;

   for( it = instructions.begin(); it != instructions.end(); it++ )
   {
      if( *it == ins )
      {
         instructions.erase(it);
         hasChanged(QVariant(Recipe::INSTRUCTION));
         return;
      }
   }
}

void Recipe::clearInstructions()
{
   instructions.clear();
   hasChanged(QVariant(Recipe::INSTRUCTION));
}

void Recipe::insertInstruction(Instruction* ins, int pos)
{
   std::vector<Instruction*>::iterator it;
   int i;

   if( ins == 0 )
      return;

   it = instructions.begin();
   for( i = 0; i < pos && it != instructions.end(); i++ )
   {
      it++;
   }

   instructions.insert(it, ins);
   hasChanged(QVariant(Recipe::INSTRUCTION));
}

int Recipe::getNumInstructions()
{
   return instructions.size();
}

Instruction* Recipe::getInstruction(int i)
{
   if( 0 <= i && i < instructions.size() )
      return instructions[i];
   else
      return 0;
}

void Recipe::generateInstructions()
{
   Instruction* ins;
   MashStep* mstep;
   instructions.clear();
   QString str;
   int i, j, size;
   double timeRemaining;
   std::vector<PreInstruction> preinstructions;

   // Mash instructions
   if( mash != 0 && mash->getNumMashSteps() > 0 )
   {
      size = mash->getNumMashSteps();

      /*** Add grains ***/
      ins = new Instruction();
      ins->setName(QString("Add grains"));
      str = "Add ";
      for( j = 0; j < fermentables.size(); ++j )
      {
         if( fermentables[j]->getRecommendMash() )
            str += QString("%1 %2, ")
            .arg(Brewtarget::displayAmount(fermentables[j]->getAmount_kg(), Units::kilograms))
            .arg(fermentables[j]->getName().c_str());
      }
      str += "to the mash tun.";
      ins->setDirections(str);
      instructions.push_back(ins);
      /*** END Add grains ***/

      /*** Prepare water additions ***/
      ins = new Instruction();
      ins->setName(QString("Heat water"));
      str = "Bring ";
      for( i = 0; i < size; ++i )
      {
         mstep = mash->getMashStep(i);
         if( mstep->getType() != "Infusion" )
            continue;
         
         str += QString("%1 water to %2, ")
                .arg(Brewtarget::displayAmount(mstep->getInfuseAmount_l(), Units::liters))
                .arg(Brewtarget::displayAmount(mstep->getInfuseTemp_c(), Units::celsius));
      }
      str += "for upcoming infusions.";
      ins->setDirections(str);
      instructions.push_back(ins);
      /*** END prepare water additions ***/

      timeRemaining = 0.0;
      for( i = 0; i < size; ++i )
      {
         mstep = mash->getMashStep(i);
         timeRemaining += mstep->getStepTime_min();
      }

      /*** Do each mash step ***/
      preinstructions.clear();
      for( i = 0; i < size; ++i )
      {
         mstep = mash->getMashStep(i);
         
         if( mstep->getType() == "Infusion")
         {
            str = QString("Add %1 water at %2 to mash to bring it to %3.")
                  .arg(Brewtarget::displayAmount(mstep->getInfuseAmount_l(), Units::liters))
                  .arg(Brewtarget::displayAmount(mstep->getInfuseTemp_c(), Units::celsius))
                  .arg(Brewtarget::displayAmount(mstep->getStepTemp_c(), Units::celsius));
         }
         else if( mstep->getType() == "Temperature" )
         {
            str = QString("Heat mash to %1.").arg(Brewtarget::displayAmount(mstep->getStepTemp_c(), Units::celsius));
         }
         else if( mstep->getType() == "Decoction" )
         {
            str = QString("Bring %1 of the mash to a boil and return to the mash tun to bring it to %2.")
                  .arg(Brewtarget::displayAmount(mstep->getDecoctionAmount_l(), Units::liters))
                  .arg(Brewtarget::displayAmount(mstep->getStepTemp_c(), Units::celsius));
         }

         str += QString(" Hold for %1.").arg(Brewtarget::displayAmount(mstep->getStepTime_min(), Units::minutes));

         preinstructions.push_back(PreInstruction(str, mstep->getType().c_str(),timeRemaining));
         timeRemaining -= mstep->getStepTime_min();
      }
      /*** END do each mash step ***/

      /*** Hops mash additions ***/
      for( j = 0; j < hops.size(); ++j )
      {
         Hop* hop = hops[j];
         if( hop->getUse() == "Mash" )
         {
            str = QString("Put %1 %2 into mash for %3.")
                  .arg(Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms))
                  .arg(hop->getName().c_str())
                  .arg(Brewtarget::displayAmount(hop->getTime_min(), Units::minutes));
            preinstructions.push_back(PreInstruction(str, "Mash hop addition", hop->getTime_min()));
         }
      }
      /*** END hop mash additions ***/

      /*** Misc mash additions ***/
      for( j = 0; j < miscs.size(); ++j )
      {
         Misc* misc = miscs[j];
         if( misc->getUse() == "Mash" )
         {
            str = QString("Put %1 %2 into mash for %3.")
                  .arg(Brewtarget::displayAmount(misc->getAmount(), ((misc->getAmountIsWeight()) ? (Unit*)(Units::kilograms) : (Unit*)(Units::liters) )))
                  .arg(misc->getName().c_str())
                  .arg(Brewtarget::displayAmount(misc->getTime(), Units::minutes));
            preinstructions.push_back(PreInstruction(str, "Mash misc addition", misc->getTime()));
         }
      }
      /*** END misc mash additions ***/

      // Add instructions in descending mash time order.
      std::sort(preinstructions.begin(), preinstructions.end());
      for( j = preinstructions.size()-1; j >= 0; --j )
      {
         PreInstruction pi = preinstructions[j];
         ins = new Instruction();
         ins->setName(pi.getTitle());
         ins->setDirections(pi.getText());
         instructions.push_back(ins);
      }
   } // END mash instructions.

   // First wort hopping
   bool hasHop = false;
   str = QString("Do first wort hopping with ");
   for( i = 0; i < hops.size(); ++i )
   {
      Hop* hop = hops[i];
      if( hop->getUse() == "First Wort")
      {
         str += QString("%1 %2,")
                .arg(Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms))
                .arg(hop->getName().c_str());
         hasHop = true;
      }
   }
   str += ".";
   if( hasHop )
   {
      ins = new Instruction();
      ins->setName("First wort hopping");
      ins->setDirections(str);
      instructions.push_back(ins);
   }
   // END first wort hopping

   // Boil instructions
   preinstructions.clear();
   
   // Find boil time.
   // TODO: fix this.
   if( equipment != 0 )
      timeRemaining = equipment->getBoilTime_min();
   else
      timeRemaining = 60.0;

   /*** Get fermentables we haven't added yet ***/
   bool hasFerms = false;
   str = QString("Add ");
   for( i = 0; i < fermentables.size(); ++i )
   {
      Fermentable* ferm = fermentables[i];
      if( ferm->getRecommendMash() || ferm->getAddAfterBoil() )
         continue;

      hasFerms = true;
      str += QString("%1 %2, ")
             .arg(Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms))
             .arg(ferm->getName().c_str());
   }
   str += "to the boil.";
   if( hasFerms )
   {
      preinstructions.push_back(PreInstruction(str, "Boil fermentables", timeRemaining));
   }
   /*** END Get fermentables we haven't added yet ***/
   
   /*** Boiled hops ***/
   for( i = 0; i < hops.size(); ++i )
   {
      Hop* hop = hops[i];
      if( hop->getUse() == "Boil" )
      {
         str = QString("Put %1 %2 into boil for %3.")
               .arg(Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms))
               .arg(hop->getName().c_str())
               .arg(Brewtarget::displayAmount(hop->getTime_min(), Units::minutes));
         preinstructions.push_back(PreInstruction(str, "Boil hop addition", hop->getTime_min()));
      }
   }
   /*** END boiled hops***/

   /*** Boiled miscs ***/
   for( i = 0; i < miscs.size(); ++i )
   {
      Misc* misc = miscs[i];
      if( misc->getUse() == "Boil" )
      {
         str = QString("Put %1 %2 into boil for %3.")
               .arg(Brewtarget::displayAmount(misc->getAmount(), ((misc->getAmountIsWeight()) ? (Unit*)(Units::kilograms) : (Unit*)(Units::liters) )))
               .arg(misc->getName().c_str())
               .arg(Brewtarget::displayAmount(misc->getTime(), Units::minutes));
         preinstructions.push_back(PreInstruction(str, "Mash misc addition", misc->getTime()));
      }
   }
   /*** End boiled miscs ***/
   // END boil instructions.

   // Add instructions in descending mash time order.
   std::sort(preinstructions.begin(), preinstructions.end());
   for( j = preinstructions.size()-1; j >= 0; --j )
   {
      PreInstruction pi = preinstructions[j];
      ins = new Instruction();
      ins->setName(pi.getTitle());
      ins->setDirections(pi.getText());
      instructions.push_back(ins);
   }

   // Fermentation instructions
   preinstructions.clear();

   /*** Fermentables added after boil ***/
   hasFerms = false;
   str = QString("Add ");
   for( i = 0; i < fermentables.size(); ++i )
   {
      Fermentable* ferm = fermentables[i];
      if( ! ferm->getAddAfterBoil() )
         continue;

      hasFerms = true;
      str += QString("%1 %2, ")
             .arg(Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms))
             .arg(ferm->getName().c_str());
   }
   str += "to the boil at knockout.";
   if( hasFerms )
   {
      ins = new Instruction();
      ins->setName("Knockout additions");
      ins->setDirections(str);
      instructions.push_back(ins);
   }
   /*** END fermentables added after boil ***/

   /*** Primary yeast ***/
   str = QString("Cool wort and pitch ");
   for( i = 0; i < yeasts.size(); ++i )
   {
      Yeast* yeast = yeasts[i];
      if( ! yeast->getAddToSecondary() )
         str += QString("%1 %2 yeast, ").arg(yeast->getName().c_str()).arg(yeast->getType().c_str());
   }
   str += "to the primary.";
   ins = new Instruction();
   ins->setName("Pitch yeast");
   ins->setDirections(str);
   instructions.push_back(ins);
   /*** End primary yeast ***/

   /*** Primary misc ***/
   str = QString("Add ");
   bool hasMisc = false;
   for( i = 0; i < miscs.size(); ++i )
   {
      Misc* misc = miscs[i];
      if( misc->getUse() == "Primary" )
      {
         str += QString("%1 %2, ")
                .arg(Brewtarget::displayAmount(misc->getAmount(), (misc->getAmountIsWeight()) ? ((Unit*)Units::kilograms) : ((Unit*)Units::liters) ))
                .arg(misc->getName().c_str());
         hasMisc = true;
      }
   }
   str += "to primary.";
   if( hasMisc )
   {
      ins = new Instruction();
      ins->setName("Additions to primary");
      ins->setDirections(str);
      instructions.push_back(ins);
   }
   /*** END primary misc ***/

   str = QString("Let ferment until FG is %1.")
         .arg(Brewtarget::displayAmount(fg));
   ins = new Instruction();
   ins->setName("Ferment");
   ins->setDirections(str);
   instructions.push_back(ins);

   str = QString("Transfer beer to secondary.");
   ins = new Instruction();
   ins->setName("Transfer to secondary");
   ins->setDirections(str);
   instructions.push_back(ins);

   /*** Secondary misc ***/
   for( i = 0; i < miscs.size(); ++i )
   {
      Misc* misc = miscs[i];
      if( misc->getUse() == "Secondary" )
      {
         str = QString("Add %1 %2 to secondary for %3.")
               .arg(Brewtarget::displayAmount(misc->getAmount(), (misc->getAmountIsWeight()) ? ((Unit*)Units::kilograms) : ((Unit*)Units::liters) ))
               .arg(misc->getName().c_str())
               .arg(Brewtarget::displayAmount(misc->getTime(), Units::minutes));

         ins = new Instruction();
         ins->setName("Secondary addition");
         ins->setDirections(str);
         instructions.push_back(ins);
      }
   }
   /*** END secondary misc ***/

   /*** Dry hopping ***/
   for( i = 0; i < hops.size(); ++i )
   {
      Hop* hop = hops[i];
      if( hop->getUse() == "Dry Hop" )
      {
         str = QString("Dry hop %1 %2 for %3.")
               .arg(Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms))
               .arg(hop->getName().c_str())
               .arg(Brewtarget::displayAmount(hop->getTime_min(), Units::minutes));
         ins = new Instruction();
         ins->setName("Dry hop");
         ins->setDirections(str);
         instructions.push_back(ins);
      }
   }
   /*** END dry hopping ***/

   // END fermentation instructions
   hasChanged(QVariant(Recipe::INSTRUCTION));
}

QString Recipe::nextAddToBoil(double& time)
{
   int i, size;
   double max = 0;
   bool foundSomething = false;
   Hop* h;
   Misc* m;
   QString ret;

   // Search hops
   size = hops.size();
   for( i = 0; i < size; ++i )
   {
      h = hops[i];
      if( h->getUse() != "Boil" )
         continue;
      if( h->getTime_min() < time && h->getTime_min() > max )
      {
         ret = QString("Add %1 %2 to boil at %3.")
               .arg(Brewtarget::displayAmount(h->getAmount_kg(), Units::kilograms))
               .arg(h->getName().c_str())
               .arg(Brewtarget::displayAmount(h->getTime_min(), Units::minutes));

         max = h->getTime_min();
         foundSomething = true;
      }
   }

   // Search miscs
   size = miscs.size();
   for( i = 0; i < size; ++i )
   {
      m = miscs[i];
      if( m->getUse() != "Boil" )
         continue;
      if( m->getTime() < time && m->getTime() > max )
      {
         ret = QString("Add %1 %2 to boil at %3.");
         if( m->getAmountIsWeight() )
            ret = ret.arg(Brewtarget::displayAmount(m->getAmount(), Units::kilograms));
         else
            ret = ret.arg(Brewtarget::displayAmount(m->getAmount(), Units::liters));

         ret = ret.arg(m->getName().c_str());
         ret = ret.arg(Brewtarget::displayAmount(m->getTime(), Units::minutes));
         max = m->getTime();
         foundSomething = true;
      }
   }
   
   time = foundSomething ? max : -1.0;
   return ret;
}

//================================"SET" METHODS=================================
void Recipe::setName( const std::string &var )
{
   name = std::string(var);
   hasChanged();
}

void Recipe::setType( const std::string &var )
{
   if( ! isValidType(var) )
      throw RecipeException("not a valid type: " + var );
   else
   {
      type = std::string(var);
      hasChanged();
   }
}

void Recipe::setBrewer( const std::string &var )
{
   brewer = std::string(var);
   hasChanged();
}

void Recipe::setStyle( Style *var )
{
   if( var == NULL )
      throw RecipeException("null pointer for setStyle");
   else
   {
      style = var;
      hasChanged();
   }
}

void Recipe::setBatchSize_l( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      batchSize_l = var;
      hasChanged();
   }
}

void Recipe::setBoilSize_l( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      boilSize_l = var;
      hasChanged();
   }
}

void Recipe::setBoilTime_min( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      boilTime_min = var;
      hasChanged();
   }
}

void Recipe::setEfficiency_pct( double var )
{
   if( var < 0.0  || var > 100.0 )
      throw RecipeException("invalid percent");
   else
   {
      efficiency_pct = var;
      hasChanged();
   }
}

void Recipe::addHop( Hop *var )
{
   if( var == NULL )
      throw RecipeException("null object");
   else
   {
      hops.push_back(var);
      addObserved(var);
      hasChanged();
   }
}

void Recipe::addFermentable( Fermentable* var )
{
   if( var == NULL )
      throw RecipeException("null object");
   else
   {
      fermentables.push_back(var);
      addObserved(var);
      hasChanged();
   }
}
void Recipe::addMisc( Misc* var )
{
   if( var == NULL )
      throw RecipeException("null object");
   else
   {
      miscs.push_back(var);
      addObserved(var);
      hasChanged();
   }
}

void Recipe::addYeast( Yeast* var )
{
   if( var == NULL )
      throw RecipeException("null object");
   else
   {
      yeasts.push_back(var);
      addObserved(var);
      hasChanged();
   }
}

void Recipe::addWater( Water* var )
{
   if( var == NULL )
      throw RecipeException("null object");
   else
   {
      waters.push_back(var);
      addObserved(var);
      hasChanged();
   }
}

// TODO: need to make mash an observable and do the addObserved() call here.
void Recipe::setMash( Mash *var )
{
   if( var == NULL )
      throw RecipeException("null object");
   else
   {
      mash = var;
      hasChanged();
   }
}


void Recipe::setAsstBrewer( const std::string &var )
{
   asstBrewer = std::string(var);
   hasChanged();
}

void Recipe::setEquipment( Equipment *var )
{
   if( var == NULL )
      throw RecipeException("null object");
   else
   {
      equipment = var;
      hasChanged();
   }
}

void Recipe::setNotes( const std::string &var )
{
   notes = std::string(var);
   hasChanged();
}

void Recipe::setTasteNotes( const std::string &var )
{
   tasteNotes = std::string(var);
   hasChanged();
}

void Recipe::setTasteRating( double var )
{
   if( var < 0.0 || var > 50.0 )
      throw RecipeException("bad taste rating: " + doubleToString(var) );
   else
   {
      tasteRating = var;
      hasChanged();
   }
}

void Recipe::setOg( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      og = var;
      hasChanged();
   }
}

void Recipe::setFg( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      fg = var;
      hasChanged();
   }
}

void Recipe::setFermentationStages( int var )
{
   if( var < 0 )
      throw RecipeException("negative quantity");
   else
   {
      fermentationStages = var;
      hasChanged();
   }
}

void Recipe::setPrimaryAge_days( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      primaryAge_days = var;
      hasChanged();
   }
}

void Recipe::setPrimaryTemp_c( double var )
{
   primaryTemp_c = var;
   hasChanged();
}

void Recipe::setSecondaryAge_days( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      secondaryAge_days = var;
      hasChanged();
   }
}

void Recipe::setSecondaryTemp_c( double var )
{
   secondaryTemp_c = var;
   hasChanged();
}

void Recipe::setTertiaryAge_days( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      tertiaryAge_days = var;
      hasChanged();
   }
}

void Recipe::setTertiaryTemp_c( double var )
{
   tertiaryTemp_c = var;
   hasChanged();
}

void Recipe::setAge_days( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      age_days = var;
      hasChanged();
   }
}

void Recipe::setAgeTemp_c( double var )
{
   ageTemp_c = var;
   hasChanged();
}

void Recipe::setDate( const std::string &var )
{
   date = std::string(var);
   hasChanged();
}

void Recipe::setCarbonation_vols( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      carbonation_vols = var;
      hasChanged();
   }
}

void Recipe::setForcedCarbonation( bool var )
{
   forcedCarbonation = var;
   hasChanged();
}

void Recipe::setPrimingSugarName( const std::string &var )
{
   primingSugarName = std::string(var);
   hasChanged();
}

void Recipe::setCarbonationTemp_c( double var )
{
   carbonationTemp_c = var;
   hasChanged();
}

void Recipe::setPrimingSugarEquiv( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      primingSugarEquiv = var;
      hasChanged();
   }
}

void Recipe::setKegPrimingFactor( double var )
{
   if( var < 0.0 )
      throw RecipeException("negative quantity");
   else
   {
      kegPrimingFactor = var;
      hasChanged();
   }
}

//=============================="GET" METHODS===================================
std::string Recipe::getName() const
{
   return name;
}

std::string Recipe::getType() const
{
   return type;
}

std::string Recipe::getBrewer() const
{
   return brewer;
}

Style* Recipe::getStyle() const
{
   return style;
}

double Recipe::getBatchSize_l() const
{
   return batchSize_l;
}

double Recipe::getBoilSize_l() const
{
   return boilSize_l;
}

double Recipe::getBoilTime_min() const
{
   return boilTime_min;
}

double Recipe::getEfficiency_pct() const
{
   return efficiency_pct;
}

unsigned int Recipe::getNumHops() const
{
   return hops.size();
}

Hop* Recipe::getHop(unsigned int i)
{
   if( i >= hops.size() )
      throw RecipeException("bad index: " + intToString(i) );
   else
      return hops[i];
}

unsigned int Recipe::getNumFermentables() const
{
   return fermentables.size();
}

Fermentable* Recipe::getFermentable(unsigned int i)
{
   if( i >= fermentables.size() )
      throw RecipeException("bad index: " + intToString(i) );
   return fermentables[i];
}

unsigned int Recipe::getNumMiscs() const
{
   return miscs.size();
}

Misc* Recipe::getMisc(unsigned int i)
{
   if( i >= miscs.size() )
      throw RecipeException("bad index: " + intToString(i) );
   return miscs[i];
}

unsigned int Recipe::getNumYeasts() const
{
   return yeasts.size();
}

Yeast* Recipe::getYeast(unsigned int i)
{
   if( i >= yeasts.size() )
      throw RecipeException("bad index: " + intToString(i) );
   return yeasts[i];
}

unsigned int Recipe::getNumWaters() const
{
   return waters.size();
}

Water* Recipe::getWater(unsigned int i)
{
   if( i >= waters.size() )
      throw RecipeException("bad index: " + intToString(i) );
   return waters[i];
}

Mash* Recipe::getMash() const
{
   return mash;
}

std::string Recipe::getAsstBrewer() const
{
   return asstBrewer;
}

Equipment* Recipe::getEquipment() const
{
   return equipment;
}

std::string Recipe::getNotes() const
{
   return notes;
}

std::string Recipe::getTasteNotes() const
{
   return tasteNotes;
}

double Recipe::getTasteRating() const
{
   return tasteRating;
}

double Recipe::getOg() const
{
   return og;
}

double Recipe::getFg() const
{
   return fg;
}

int Recipe::getFermentationStages() const
{
   return fermentationStages;
}

double Recipe::getPrimaryAge_days() const
{
   return primaryAge_days;
}

double Recipe::getPrimaryTemp_c() const
{
   return primaryTemp_c;
}

double Recipe::getSecondaryAge_days() const
{
   return secondaryAge_days;
}

double Recipe::getSecondaryTemp_c() const
{
   return secondaryTemp_c;
}

double Recipe::getTertiaryAge_days() const
{
   return tertiaryAge_days;
}

double Recipe::getTertiaryTemp_c() const
{
   return tertiaryTemp_c;
}

double Recipe::getAge_days() const
{
   return age_days;
}

double Recipe::getAgeTemp_c() const
{
   return ageTemp_c;
}

std::string Recipe::getDate() const
{
   return date;
}

double Recipe::getCarbonation_vols() const
{
   return carbonation_vols;
}

bool Recipe::getForcedCarbonation() const
{
   return forcedCarbonation;
}

std::string Recipe::getPrimingSugarName() const
{
   return primingSugarName;
}

double Recipe::getCarbonationTemp_c() const
{
   return carbonationTemp_c;
}

double Recipe::getPrimingSugarEquiv() const
{
   return primingSugarEquiv;
}

double Recipe::getKegPrimingFactor() const
{
   return kegPrimingFactor;
}

bool Recipe::isValidType( const std::string &str )
{
   static const std::string types[] = {"Extract", "Partial Mash", "All Grain"};
   static const unsigned int size = 3;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( str == types[i] )
         return true;
   
   return false;
}

void Recipe::recalculate()
{
   unsigned int i;
   double points = 0;
   double sugar_kg = 0;
   double sugar_kg_ignoreEfficiency = 0.0;
   std::string fermtype;
   double attenuation_pct = 0.0;
   Fermentable* ferm;
   Yeast* yeast;

   // Calculate OG
   for( i = 0; i < fermentables.size(); ++i )
   {
      ferm = fermentables[i];

      // If we have some sort of non-grain, we have to ignore efficiency.
      fermtype = ferm->getType();
      if( fermtype.compare("Sugar") == 0 || fermtype.compare("Extract") == 0 || fermtype.compare("Dry Extract") == 0 )
         sugar_kg_ignoreEfficiency += (ferm->getYield_pct()/100.0)*ferm->getAmount_kg();
      else
         sugar_kg += (ferm->getYield_pct()/100.0)*ferm->getAmount_kg();
   }

   // Conversion factor for lb/gal to kg/l = 8.34538.
   points = (383.89 * sugar_kg / getBatchSize_l()) * getEfficiency_pct()/100.0;
   points += 383.89 * sugar_kg_ignoreEfficiency / getBatchSize_l();
   og = 1 + points/1000.0;

   // Calculage FG
   for( i = 0; i < yeasts.size(); ++i )
   {
      yeast = yeasts[i];
      // Get the yeast with the greatest attenuation.
      if( yeast->getAttenuation_pct() > attenuation_pct )
         attenuation_pct = yeast->getAttenuation_pct();
   }
   if( yeasts.size() > 0 && attenuation_pct <= 0.0 ) // This means we have yeast, but they neglected to provide attenuation percentages.
      attenuation_pct = 75.0; // 75% is an average attenuation.

   points = points*(1.0 - attenuation_pct/100.0);
   fg =  1 + points/1000.0;

   // Calculate ABV
   /* No need, just call getABV_pct() */

   // Calculate color
   /* No need, just call getColor_srm() */

   // Calculate IBUs
   /* No need, just call getIBU() */
}

double Recipe::getColor_srm()
{
   Fermentable *ferm;
   double mcu = 0.0;
   unsigned int i;

   for( i = 0; i < fermentables.size(); ++i )
   {
      ferm = fermentables[i];
      // Conversion factor for lb/gal to kg/l = 8.34538.
      mcu += ferm->getColor_srm()*8.34538 * ferm->getAmount_kg()/getBatchSize_l();
   }
   // Morey color calculation.
   return 1.4922 * pow( mcu, 0.6859 );
}

double Recipe::getABV_pct()
{
   return 0.130*((getOg()-1)-(getFg()-1))*1000.0;
}

void Recipe::notify(Observable* /*notifier*/, QVariant info)
{
   recalculate();
   hasChanged();
}

// Returns true if var is found and removed.
bool Recipe::removeHop( Hop *var )
{
   std::vector<Hop*>::iterator iter;

   for( iter = hops.begin(); iter != hops.end(); iter++ )
   {
      if( *iter == var )
      {
         hops.erase(iter);
         removeObserved(var);
         hasChanged();
         return true;
      }
   }

   return false;
}

bool Recipe::removeFermentable(Fermentable* var)
{
   std::vector<Fermentable*>::iterator iter;

   for( iter = fermentables.begin(); iter != fermentables.end(); iter++ )
   {
      if( *iter == var )
      {
         fermentables.erase(iter);
         removeObserved(var);
         hasChanged();
         return true;
      }
   }

   return false;
}

bool Recipe::removeMisc(Misc* var)
{
   std::vector<Misc*>::iterator iter;

   for( iter = miscs.begin(); iter != miscs.end(); iter++ )
   {
      if( *iter == var )
      {
         miscs.erase(iter);
         removeObserved(var);
         hasChanged();
         return true;
      }
   }

   return false;
}

bool Recipe::removeWater(Water* var)
{
   std::vector<Water*>::iterator iter;

   for( iter = waters.begin(); iter != waters.end(); iter++ )
   {
      if( *iter == var )
      {
         waters.erase(iter);
         removeObserved(var);
         hasChanged();
         return true;
      }
   }

   return false;
}

bool Recipe::removeYeast(Yeast* var)
{
   std::vector<Yeast*>::iterator iter;

   for( iter = yeasts.begin(); iter != yeasts.end(); iter++ )
   {
      if( *iter == var )
      {
         yeasts.erase(iter);
         removeObserved(var);
         hasChanged();
         return true;
      }
   }

   return false;
}

double Recipe::getWortGrav()
{
   unsigned int i;
   Fermentable* ferm;
   double sugar_kg = 0.0;
   double sugar_kg_ignoreEfficiency = 0.0;
   double points = 0.0;
   std::string type;

   // Calculate OG
   for( i = 0; i < fermentables.size(); ++i )
   {
      ferm = fermentables[i];
      if( ferm->getAddAfterBoil() )
         continue;

      // If we have some sort of non-grain, we have to ignore efficiency.
      type = ferm->getType();
      if( type.compare("Sugar") == 0 || type.compare("Extract") == 0 || type.compare("Dry Extract") == 0 )
         sugar_kg_ignoreEfficiency += (ferm->getYield_pct()/100.0)*ferm->getAmount_kg();
      else
         sugar_kg += (ferm->getYield_pct()/100.0)*ferm->getAmount_kg();
   }

   // Conversion factor for lb/gal to kg/l = 8.34538.
   /*
   points = (383.89 * sugar_kg / getBatchSize_l()) * getEfficiency_pct()/100.0;
   points += 383.89 * sugar_kg_ignoreEfficiency / getBatchSize_l();
    */
   points = (383.89 * sugar_kg / getBoilSize_l()) * getEfficiency_pct()/100.0;
   points += 383.89 * sugar_kg_ignoreEfficiency / getBoilSize_l();
   return (1.0 + points/1000.0);
}

double Recipe::getIBU()
{
   unsigned int i;
   double ibus = 0.0;

   // Bitterness due to hops...
   for( i = 0; i < hops.size(); ++i )
      ibus += IBU( hops[i]->getAlpha_pct()/100.0, hops[i]->getAmount_kg()*1000.0,
                   batchSize_l, getWortGrav(), hops[i]->getTime_min() );

   // Bitterness due to hopped extracts...
   for( i = 0; i < fermentables.size(); ++i )
   {
      // Conversion factor for lb/gal to kg/l = 8.34538.
      ibus +=
              fermentables[i]->getIbuGalPerLb() *
              (fermentables[i]->getAmount_kg() / batchSize_l) / 8.34538;
   }

   return ibus;
}

void Recipe::clear()
{
   std::string name = getName();
   setDefaults();
   setName(name);
   hasChanged();
}

QColor Recipe::getSRMColor()
{
   /**** Original method from a website: Came out dark. ***
   double SRM = getColor_srm();
   
   // Luminance Y
   double Y = 94.6914*exp(-0.131272*SRM);
   // Chroma x
   double x = 0.73978 - 0.25442*exp(-0.037865*SRM) - 0.017511*exp(-0.24307*SRM);
   // Chroma y
   double y = 0.197785 + 0.260472*exp( -pow( (x-0.491021)/.214194, 2)   );
   // Chroma z
   double z = 1 - x - y;

   double X = (Y/y)*x;
   double Z = (Y/y)*z;

   // Get [0,255] RGB values.
   int R = (int)ceil(1.910*X  - 0.533*Y - 0.288*Z);
   R = (R<0)?0:((R>255)?255:R);

   int G = (int)ceil(-0.985*X + 2.000*Y - 0.0280*Z);
   G = (G<0)?0:((G>255)?255:G);

   int B = (int)ceil(0.058*X -0.118*Y + 0.896*Z);
   B = (B<0)?0:((B>255)?255:B);

   QColor ret;

   ret.setRgb( R, G, B, 255 );

   return ret;
   ***********/

   //==========My approximation from a photo and spreadsheet===========
   double SRM = getColor_srm();

   double red = 232.9 * pow( (double)0.93, SRM );
   double green = (double)-106.25 * log(SRM) + 280.9;

   int r = (red < 0)? 0 : ((red > 255)? 255 : (int)round(red));
   int g = (green < 0)? 0 : ((green > 255)? 255 : (int)round(green));
   int b = 0;

   QColor ret;
   ret.setRgb( r, g, b );

   return ret;
}
