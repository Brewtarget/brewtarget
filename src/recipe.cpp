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
#include "PreInstruction.h"
#include <QDomElement>
#include <QDomText>
#include <QInputDialog>
#include "Algorithms.h"
#include "IbuMethods.h"
#include "ColorMethods.h"

bool operator<(Recipe &r1, Recipe &r2 )
{
   return r1.name < r2.name;
}

bool operator==(Recipe &r1, Recipe &r2 )
{
   return r1.name == r2.name;
}

void Recipe::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement recipeNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   unsigned int i, size;
   
   recipeNode = doc.createElement("RECIPE");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(name.c_str());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(text(version));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(type.c_str());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   if( style != 0 )
      style->toXml(doc, recipeNode);
   
   tmpNode = doc.createElement("BREWER");
   tmpText = doc.createTextNode(brewer.c_str());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BATCH_SIZE");
   tmpText = doc.createTextNode(text(batchSize_l));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BOIL_SIZE");
   tmpText = doc.createTextNode(text(boilSize_l));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BOIL_TIME");
   tmpText = doc.createTextNode(text(boilTime_min));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("EFFICIENCY");
   tmpText = doc.createTextNode(text(efficiency_pct));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HOPS");
   size = hops.size();
   for( i = 0; i < size; ++i )
      hops[i]->toXml(doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FERMENTABLES");
   size = fermentables.size();
   for( i = 0; i < size; ++i )
      fermentables[i]->toXml(doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MISCS");
   size = miscs.size();
   for( i = 0; i < size; ++i )
      miscs[i]->toXml(doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("YEASTS");
   size = yeasts.size();
   for( i = 0; i < size; ++i )
      yeasts[i]->toXml(doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("WATERS");
   size = waters.size();
   for( i = 0; i < size; ++i )
      waters[i]->toXml(doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   if( mash != 0 )
      mash->toXml(doc, recipeNode);
   
   tmpNode = doc.createElement("INSTRUCTIONS");
   size = instructions.size();
   for( i = 0; i < size; ++i )
      instructions[i]->toXml(doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ASST_BREWER");
   tmpText = doc.createTextNode(asstBrewer.c_str());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   if( equipment )
      equipment->toXml(doc, recipeNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes.c_str());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TASTE_NOTES");
   tmpText = doc.createTextNode(tasteNotes.c_str());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TASTE_RATING");
   tmpText = doc.createTextNode(text(tasteRating));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("OG");
   tmpText = doc.createTextNode(text(og));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("FG");
   tmpText = doc.createTextNode(text(fg));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("FERMENTATION_STAGES");
   tmpText = doc.createTextNode(text(fermentationStages));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PRIMARY_AGE");
   tmpText = doc.createTextNode(text(primaryAge_days));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PRIMARY_TEMP");
   tmpText = doc.createTextNode(text(primaryTemp_c));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SECONDARY_AGE");
   tmpText = doc.createTextNode(text(secondaryAge_days));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SECONDARY_TEMP");
   tmpText = doc.createTextNode(text(secondaryTemp_c));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TERTIARY_AGE");
   tmpText = doc.createTextNode(text(tertiaryAge_days));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TERTIARY_TEMP");
   tmpText = doc.createTextNode(text(tertiaryTemp_c));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AGE");
   tmpText = doc.createTextNode(text(age_days));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AGE_TEMP");
   tmpText = doc.createTextNode(text(ageTemp_c));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("DATE");
   tmpText = doc.createTextNode(date.c_str());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("CARBONATION");
   tmpText = doc.createTextNode(text(carbonation_vols));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("FORCED_CARBONATION");
   tmpText = doc.createTextNode(text(forcedCarbonation));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PRIMING_SUGAR_NAME");
   tmpText = doc.createTextNode(primingSugarName.c_str());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("CARBONATION_TEMP");
   tmpText = doc.createTextNode(text(carbonationTemp_c));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PRIMING_SUGAR_EQUIV");
   tmpText = doc.createTextNode(text(primingSugarEquiv));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("KEG_PRIMING_FACTOR");
   tmpText = doc.createTextNode(text(kegPrimingFactor));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   parent.appendChild(recipeNode);
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
   addObserved(mash);

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
   
   hops.clear();
}

Recipe::Recipe()
{
   setDefaults();
}

Recipe::Recipe(const QDomNode& recipeNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;

   setDefaults();

   for( node = recipeNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QString("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }

      child = node.firstChild();
      if( child.isNull() )
         continue;

      property = node.nodeName();
      if( child.isText() )
      {
         textNode = child.toText();
         value = textNode.nodeValue();
      }
      else
      {
         textNode = QDomText();
         value = QString();
      }

      if( property == "NAME" )
      {
         setName(value.toStdString());
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QString("RECIPE says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "TYPE" )
      {
         if( value.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QString("Error at line %2.").arg(textNode.lineNumber()) );
            continue;
         }

         if( isValidType(value.toStdString()) )
            setType(value.toStdString());
         else
             Brewtarget::log(Brewtarget::ERROR, QString("%1 is not a valid type for RECIPE. Line %2").arg(value).arg(textNode.lineNumber()));
      }
      else if( property == "BREWER" )
      {
         if( value.isNull() )
         {
            continue;
         }

         setBrewer(value.toStdString());
      }
      else if( property == "STYLE" )
      {
         setStyle(new Style(node));
      }
      else if( property == "BATCH_SIZE" )
      {
         if( textNode.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QString("Error at line %2.").arg(node.lineNumber()) );
            continue;
         }
         setBatchSize_l(getDouble(textNode));
      }
      else if( property == "BOIL_SIZE" )
      {
         if( textNode.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QString("Error at line %2.").arg(node.lineNumber()) );
            continue;
         }
         setBoilSize_l(getDouble(textNode));
      }
      else if( property == "BOIL_TIME" )
      {
         if( textNode.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QString("Error at line %2.").arg(node.lineNumber()) );
            continue;
         }
         setBoilTime_min(getDouble(textNode));
      }
      else if( property == "EFFICIENCY" )
      {
         if( textNode.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QString("Error at line %2.").arg(node.lineNumber()) );
            continue;
         }
         setEfficiency_pct(getDouble(textNode));
      }
      else if( property == "HOPS" )
      {
         QDomNode hopNode;
         for( hopNode = child; ! hopNode.isNull(); hopNode = hopNode.nextSibling() )
            addHop(new Hop(hopNode));
      }
      else if( property == "FERMENTABLES" )
      {
         QDomNode fermNode;
         for( fermNode = child; ! fermNode.isNull(); fermNode = fermNode.nextSibling() )
            addFermentable(new Fermentable(fermNode));
      }
      else if( property == "MISCS" )
      {
         QDomNode miscNode;
         for( miscNode = child; ! miscNode.isNull(); miscNode = miscNode.nextSibling() )
            addMisc(new Misc(miscNode));
      }
      else if( property == "YEASTS" )
      {
         QDomNode yeastNode;
         for( yeastNode = child; ! yeastNode.isNull(); yeastNode = yeastNode.nextSibling() )
            addYeast(new Yeast(yeastNode));
      }
      else if( property == "WATERS" )
      {
         QDomNode waterNode;
         for( waterNode = child; ! waterNode.isNull(); waterNode = waterNode.nextSibling() )
            addWater(new Water(waterNode));
      }
      else if( property == "INSTRUCTIONS" )
      {
         QDomNode instructionNode;
         for( instructionNode = child; ! instructionNode.isNull(); instructionNode = instructionNode.nextSibling() )
            addInstruction(new Instruction(instructionNode));
      }
      else if( property == "MASH" )
      {
         setMash(new Mash(node));
      }
      else if( property == "ASST_BREWER" )
      {
         if( value.isNull() )
         {
            continue;
         }
         setAsstBrewer(value.toStdString());
      }
      else if( property == "EQUIPMENT" )
      {
         setEquipment(new Equipment(node));
      }
      else if( property == "NOTES" )
      {
         if( value.isNull() )
         {
            continue;
         }
         setNotes(value.toStdString());
      }
      else if( property == "TASTE_NOTES" )
      {
         if( value.isNull() )
         {
            continue;
         }
         setTasteNotes(value.toStdString());
      }
      else if( property == "TASTE_RATING" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setTasteRating(getDouble(textNode));
      }
      else if( property == "OG" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setOg(getDouble(textNode));
      }
      else if( property == "FG" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setFg(getDouble(textNode));
      }
      else if( property == "FERMENTATION_STAGES" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setFermentationStages(getInt(textNode));
      }
      else if( property == "PRIMARY_AGE" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setPrimaryAge_days(getDouble(textNode));
      }
      else if( property == "PRIMARY_TEMP" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setPrimaryTemp_c(getDouble(textNode));
      }
      else if( property == "SECONDARY_AGE" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setSecondaryAge_days(getDouble(textNode));
      }
      else if( property == "SECONDARY_TEMP" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setSecondaryTemp_c(getDouble(textNode));
      }
      else if( property == "TERTIARY_AGE" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setTertiaryAge_days(getDouble(textNode));
      }
      else if( property == "TERTIARY_TEMP" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setTertiaryTemp_c(getDouble(textNode));
      }
      else if( property == "AGE" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setAge_days(getDouble(textNode));
      }
      else if( property == "AGE_TEMP" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setAgeTemp_c(getDouble(textNode));
      }
      else if( property == "DATE" )
      {
         if( value.isNull() )
         {
            continue;
         }
         setDate(value.toStdString());
      }
      else if( property == "CARBONATION" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setCarbonation_vols(getDouble(textNode));
      }
      else if( property == "FORCED_CARBONATION" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setForcedCarbonation(getBool(textNode));
      }
      else if( property == "PRIMING_SUGAR_NAME" )
      {
         if( value.isNull() )
         {
            continue;
         }
         setPrimingSugarName(value.toStdString());
      }
      else if( property == "CARBONATION_TEMP" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setCarbonationTemp_c(getDouble(textNode));
      }
      else if( property == "PRIMING_SUGAR_EQUIV" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setPrimingSugarEquiv(getDouble(textNode));
      }
      else if( property == "KEG_PRIMING_FACTOR" )
      {
         if( textNode.isNull() )
         {
            continue;
         }
         setKegPrimingFactor(getDouble(textNode));
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QString("Unsupported RECIPE property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
}

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

void Recipe::swapInstructions(unsigned int j, unsigned int k)
{
   if( j == k || j >= instructions.size() || k >= instructions.size() )
      return;
   
   Instruction* tmp;
   tmp = instructions[j];
   instructions[j] = instructions[k];
   instructions[k] = tmp;
   
   hasChanged(QVariant(Recipe::INSTRUCTION));
   
   return;
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
   double totalWaterAdded_l = 0.0;
   double wortInBoil_l = 0.0;
   double wort_l = 0.0;
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
         if( fermentables[j]->getIsMashed() )
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
		  
	    totalWaterAdded_l += mstep->getInfuseAmount_l();
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

         preinstructions.push_back(PreInstruction(str, QString("%1 - %2").arg(mstep->getType().c_str()).arg(mstep->getName().c_str()), timeRemaining));
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

   // Need to top up the kettle before boil?
   if( equipment != 0 )
   {
      wortInBoil_l = estimateWortFromMash_l() - equipment->getLauterDeadspace_l();
      str = QString("You should now have %1 wort.")
      .arg(Brewtarget::displayAmount( wortInBoil_l, Units::liters));
      wortInBoil_l += equipment->getTopUpKettle_l();
      str += QString(" Add %1 water to the kettle, bringing pre-boil volume to %2.")
	    .arg(Brewtarget::displayAmount(equipment->getTopUpKettle_l(), Units::liters))
	    .arg(Brewtarget::displayAmount(wortInBoil_l, Units::liters));
	    
      ins = new Instruction();
      ins->setName("Pre-boil");
      ins->setDirections(str);
      instructions.push_back(ins);
   }

   // Boil instructions
   preinstructions.clear();   
   
   // Find boil time.
   if( equipment != 0 )
      timeRemaining = equipment->getBoilTime_min();
   else
   {
      timeRemaining = Unit::qstringToSI(QInputDialog::getText(0,
					     "Boil time",
					     "You did not configure an equipment (which you really should), so tell me the boil time."));
   }
   
   str = QString("Bring the wort to a boil and hold for %1.").arg(Brewtarget::displayAmount( timeRemaining, Units::minutes));
   ins = new Instruction();
   ins->setName("Start boil");
   ins->setDirections(str);
   instructions.push_back(ins);
   
   /*** Get fermentables we haven't added yet ***/
   bool hasFerms = false;
   str = QString("Add ");
   for( i = 0; i < fermentables.size(); ++i )
   {
      Fermentable* ferm = fermentables[i];
      if( ferm->getIsMashed() || ferm->getAddAfterBoil() )
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

   /*** post boil ***/
   if( equipment != 0 )
   {
      wort_l = equipment->wortEndOfBoil_l(wortInBoil_l);
      str = QString("You should have %1 wort post-boil.")
            .arg(Brewtarget::displayAmount( wort_l, Units::liters));
      str += QString("\nYou anticipate losing %1 to trub and chiller loss.")
	     .arg(Brewtarget::displayAmount( equipment->getTrubChillerLoss_l(), Units::liters));
      wort_l -= equipment->getTrubChillerLoss_l();
      if( equipment->getTopUpWater_l() > 0.0 )
	 str += QString("\nAdd %1 top up water into primary.")
	        .arg(Brewtarget::displayAmount( equipment->getTopUpWater_l(), Units::liters));
      wort_l += equipment->getTopUpWater_l();
      str += QString("\nThe final volume in the primary is %1.")
             .arg(Brewtarget::displayAmount(wort_l, Units::liters));
	     
      ins = new Instruction();
      ins->setName("Post boil");
      ins->setDirections(str);
      instructions.push_back(ins);
   }
   /*** end post boil ***/
   
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

void Recipe::setMash( Mash *var )
{
   if( var == NULL )
      throw RecipeException("null object");
   else
   {
      mash = var;
      addObserved(mash);
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
         //sugar_kg_ignoreEfficiency += (ferm->getYield_pct()/100.0)*ferm->getAmount_kg();
         sugar_kg_ignoreEfficiency += ferm->getEquivSucrose_kg();
      else
         //sugar_kg += (ferm->getYield_pct()/100.0)*ferm->getAmount_kg();
         sugar_kg += ferm->getEquivSucrose_kg();
   }

   // We might lose some sugar in the form of Trub/Chiller loss and lauter deadspace.
   if( equipment != 0 )
   {
      // First, lauter deadspace.
      double ratio = (estimateWortFromMash_l() - equipment->getLauterDeadspace_l()) / (estimateWortFromMash_l());
      if( ratio > 1.0 ) // Usually happens when we don't have a mash yet.
         ratio = 1.0;
      else if( ratio < 0.0 ) // Only happens if the user is stupid with lauter deadspace.
         ratio = 0.0;
      else if( isnan(ratio) )
         ratio = 1.0; // Need this in case we have no mash, and therefore end up with NaN.
         
      sugar_kg *= ratio;
      // Don't consider this one since nobody adds sugar or extract to the mash.
      //sugar_kg_ignoreEfficiency *= ratio;
      
      // Next, trub/chiller loss.
      double kettleWort_l = (estimateWortFromMash_l() - equipment->getLauterDeadspace_l()) + equipment->getTopUpKettle_l();
      double postBoilWort_l = equipment->wortEndOfBoil_l(kettleWort_l);
      ratio = (postBoilWort_l - equipment->getTrubChillerLoss_l()) / postBoilWort_l;
      if( ratio > 1.0 ) // Usually happens when we don't have a mash yet.
         ratio = 1.0;
      else if( ratio < 0.0 )
         ratio = 0.0;
      else if( isnan(ratio) )
         ratio = 1.0;
      sugar_kg *= ratio;
      sugar_kg_ignoreEfficiency *= ratio;
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

   return ColorMethods::mcuToSrm(mcu);
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

   // We might lose some sugar in the form of lauter deadspace.
   if( equipment != 0 )
   {
      // First, lauter deadspace.
      double ratio = (estimateWortFromMash_l() - equipment->getLauterDeadspace_l()) / (estimateWortFromMash_l());
      if( ratio > 1.0 )
         ratio = 1.0;
      else if( isnan(ratio) )
         ratio = 1.0; // Need this in case we have no mash, and therefore end up with NaN.
      sugar_kg *= ratio;
      // Don't consider this one since nobody adds sugar or extract to the mash.
      //sugar_kg_ignoreEfficiency *= ratio;
   }

   // Conversion factor for lb/gal to kg/l = 8.34538.
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
      ibus += getIBUFromHop(i);

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

double Recipe::getIBUFromHop( unsigned int i )
{
   double ibus = 0.0;
   
   if( i >= hops.size() )
      return 0.0;
   
   if( hops[i]->getUse() == "Boil")
      ibus = IbuMethods::getIbus( hops[i]->getAlpha_pct()/100.0, hops[i]->getAmount_kg()*1000.0,
                   batchSize_l, getWortGrav(), hops[i]->getTime_min() );
   else if( hops[i]->getUse() == "First Wort" )
      ibus = 1.10 * IbuMethods::getIbus( hops[i]->getAlpha_pct()/100.0,
                          hops[i]->getAmount_kg()*1000.0,
                          batchSize_l,
                          getWortGrav(),
                          20 ); // I am estimating First wort hops give 10% more ibus than a 20 minute addition.

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

double Recipe::estimateWortFromMash_l() const
{
   if( mash == 0 )
      return 0.0;
   
   double waterAdded_l = mash->totalMashWater_l();
   double absorption_lKg = 0.13 * (3.78541178)/(0.45359237); // 0.13 gal/lb
   
   //std::cerr << "estimateWortFromMash_l(): " << waterAdded_l - absorption_lKg*getGrainsInMash_kg() << std::endl;

   return (waterAdded_l - absorption_lKg*getGrainsInMash_kg());
}

double Recipe::getGrainsInMash_kg() const
{
   unsigned int i, size;
   double grains_kg = 0.0;
   Fermentable* ferm;
   
   size = fermentables.size();
   for( i = 0; i < size; ++i )
   {
      ferm = fermentables[i];
      
      if( ferm->getIsMashed() )
	 grains_kg += ferm->getAmount_kg();
   }
   
   return grains_kg;
}

double Recipe::estimateBoilVolume_l() const
{
   double mashVol_l;
   double ret = 0.0;
   
   mashVol_l = estimateWortFromMash_l();
   
   if( mashVol_l <= 0.0 ) // Give up.
      return boilSize_l;
   
   if( equipment != 0 )
      ret = mashVol_l - equipment->getLauterDeadspace_l() + equipment->getTopUpKettle_l();
   else
      ret = mashVol_l;
   
   return ret;
}

double Recipe::estimateFinalVolume_l() const
{
   double boilVol_l;
   double ret;
   
   boilVol_l = estimateBoilVolume_l();
   
   if( equipment != 0 )
      ret = equipment->wortEndOfBoil_l(boilVol_l) - equipment->getTrubChillerLoss_l() + equipment->getTopUpWater_l();
   else
      ret = boilVol_l - 4.0; // This is just shooting in the dark. Can't do much without an equipment.
      
   return ret;
}
