/*
 * recipe.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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
#include <QList>
#include <iostream>
#include <cmath>
#include <algorithm>
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
#include <QObject>
#include "Algorithms.h"
#include "IbuMethods.h"
#include "ColorMethods.h"
#include <QDate>
#include "HeatCalculations.h"
#include <ctime>

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
   tmpText = doc.createTextNode(name);
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(text(version));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(type);
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   if( style != 0 )
      style->toXml(doc, recipeNode);
   
   tmpNode = doc.createElement("BREWER");
   tmpText = doc.createTextNode(brewer);
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

   tmpNode = doc.createElement("BREWNOTES");
   size = brewNotes.size();
   for(i=0; i < size; ++i)
      brewNotes[i]->toXml(doc,tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ASST_BREWER");
   tmpText = doc.createTextNode(asstBrewer);
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   if( equipment )
      equipment->toXml(doc, recipeNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes);
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TASTE_NOTES");
   tmpText = doc.createTextNode(tasteNotes);
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
   tmpText = doc.createTextNode(date);
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
   tmpText = doc.createTextNode(primingSugarName);
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

void Recipe::clear()
{
   QString name = getName();
   setDefaults();
   setName(name);
   hasChanged();
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
   hops = QVector<Hop*>();
   fermentables = QVector<Fermentable*>();
   miscs = QVector<Misc*>();
   yeasts = QVector<Yeast*>();
   waters = QVector<Water*>();
   brewNotes = QVector<BrewNote*>();
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
   date = QDate::currentDate().toString("MM/dd/yyyy");
   carbonation_vols = 0.0;
   forcedCarbonation = false;
   primingSugarName = "";
   carbonationTemp_c = 0.0;
   primingSugarEquiv = 0.0;
   kegPrimingFactor = 0.0;
   
   hops.clear();
}

Recipe::Recipe() : BeerXMLElement()
{
}

Recipe::Recipe(const QDomNode& recipeNode)
{
   fromNode(recipeNode);
}

Recipe::Recipe( Recipe const& other ) : BeerXMLElement(other)
{
}

void Recipe::fromNode(const QDomNode& recipeNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = recipeNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
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
         setName(value);
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("RECIPE says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "TYPE" )
      {
         if( value.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("Error at line %1.").arg(textNode.lineNumber()) );
            continue;
         }
         
         if( isValidType(value) )
            setType(value);
         else
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid type for RECIPE. Line %2").arg(value).arg(textNode.lineNumber()));
      }
      else if( property == "BREWER" )
      {
         if( value.isNull() )
         {
            continue;
         }
         
         setBrewer(value);
      }
      else if( property == "STYLE" )
      {
         setStyle(new Style(node));
      }
      else if( property == "BATCH_SIZE" )
      {
         if( textNode.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("Error at line %1.").arg(node.lineNumber()) );
            continue;
         }
         setBatchSize_l(getDouble(textNode));
      }
      else if( property == "BOIL_SIZE" )
      {
         if( textNode.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("Error at line %1.").arg(node.lineNumber()) );
            continue;
         }
         setBoilSize_l(getDouble(textNode));
      }
      else if( property == "BOIL_TIME" )
      {
         if( textNode.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("Error at line %1.").arg(node.lineNumber()) );
            continue;
         }
         setBoilTime_min(getDouble(textNode));
      }
      else if( property == "EFFICIENCY" )
      {
         if( textNode.isNull() )
         {
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("Error at line %1.").arg(node.lineNumber()) );
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
      else if( property == "BREWNOTES" )
      {
         QDomNode bnNode;
         for( bnNode = child; ! bnNode.isNull(); bnNode = bnNode.nextSibling() )
            addBrewNote(new BrewNote(this,bnNode));
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
         setAsstBrewer(value);
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
         setNotes(value);
      }
      else if( property == "TASTE_NOTES" )
      {
         if( value.isNull() )
         {
            continue;
         }
         setTasteNotes(value);
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
         setDate(value);
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
         setPrimingSugarName(value);
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
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported RECIPE property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
}

void Recipe::addInstruction(Instruction* ins)
{
   if( ins == 0 )
      return;
   
   Database::instance().addToRecipe( this, ins );
}

void Recipe::removeInstruction(Instruction* ins)
{
   Database::instance().removeFromRecipe( this, ins );
}

void Recipe::swapInstructions(unsigned int j, unsigned int k)
{
   if( j == k || static_cast<int>(j) >= instructions().size() || static_cast<int>(k) >= instructions().size() )
      return;
   
   instructions()[j]->setNumber(k);
   instructions()[k]->setNumber(j);
}

void Recipe::clearInstructions()
{
   instructions.clear();
   hasChanged(QVariant(Recipe::INSTRUCTION));
}

void Recipe::insertInstruction(Instruction* ins, int pos)
{
   int i;

   if( ins == 0 )
      return;

   for( i = pos; i < instructions().size(); ++i )
   {
      instructions()[i]->setNumber(i+1);
   }

   ins->setNumber(pos);
   Database::instance().addToRecipe( this, ins );
}

int Recipe::numInstructions()
{
   return instructions().size();
}

Instruction* Recipe::getMashFermentable() const
{
   Instruction* ins;
   QString str,tmp;
   unsigned int i;

    /*** Add grains ***/
     ins = new Instruction();
     ins->setName(QObject::tr("Add grains"));
     str = QObject::tr("Add ");
     for( i = 0; static_cast<int>(i) < fermentables.size(); ++i )
     {
        if( fermentables[i]->getIsMashed() )
          tmp = QString("%1 %2, ")
            .arg(Brewtarget::displayAmount(fermentables[i]->getAmount_kg(), Units::kilograms))
            .arg(fermentables[i]->getName());
           str += tmp;
           ins->setReagent(tmp);
     }
     str += QObject::tr("to the mash tun.");
     ins->setDirections(str);

     return ins;

}

Instruction* Recipe::getMashWater(unsigned int size) const
{
   Instruction* ins;
   MashStep* mstep;
   QString str, tmp;
   unsigned int i;

    ins = new Instruction();
    ins->setName(QObject::tr("Heat water"));
    str = QObject::tr("Bring ");
    for( i = 0; i < size; ++i )
    {
       mstep = mash->getMashStep(i);
       if( mstep->getType() != MashStep::TYPEINFUSION )
          continue;

       tmp = QObject::tr("%1 water to %2, ")
              .arg(Brewtarget::displayAmount(mstep->getInfuseAmount_l(), Units::liters))
              .arg(Brewtarget::displayAmount(mstep->getInfuseTemp_c(), Units::celsius));
       str += tmp;
       ins->setReagent(tmp);
    }
    str += QObject::tr("for upcoming infusions.");
    ins->setDirections(str);

    return ins;
}

QVector<PreInstruction> Recipe::getMashInstructions(double timeRemaining, double totalWaterAdded_l, unsigned int size) const
{
   QVector<PreInstruction> preins;
   MashStep* mstep;
   QString str;

   unsigned int i;

   preins.clear();
    for( i = 0; i < size; ++i )
    {
       mstep = mash->getMashStep(i);

       if( mstep->getType() == MashStep::TYPEINFUSION)
       {
          str = QObject::tr("Add %1 water at %2 to mash to bring it to %3.")
                .arg(Brewtarget::displayAmount(mstep->getInfuseAmount_l(), Units::liters))
                .arg(Brewtarget::displayAmount(mstep->getInfuseTemp_c(), Units::celsius))
                .arg(Brewtarget::displayAmount(mstep->getStepTemp_c(), Units::celsius));

          totalWaterAdded_l += mstep->getInfuseAmount_l();
       }
       else if( mstep->getType() == MashStep::TYPETEMPERATURE )
       {
          str = QObject::tr("Heat mash to %1.").arg(Brewtarget::displayAmount(mstep->getStepTemp_c(), Units::celsius));
       }
       else if( mstep->getType() == MashStep::TYPEDECOCTION )
       {
          str = QObject::tr("Bring %1 of the mash to a boil and return to the mash tun to bring it to %2.")
                .arg(Brewtarget::displayAmount(mstep->getDecoctionAmount_l(), Units::liters))
                .arg(Brewtarget::displayAmount(mstep->getStepTemp_c(), Units::celsius));
       }

       str += QObject::tr(" Hold for %1.").arg(Brewtarget::displayAmount(mstep->getStepTime_min(), Units::minutes));

       preins.push_back(PreInstruction(str, QString("%1 - %2").arg(mstep->getTypeStringTr()).arg(mstep->getName()),
                   timeRemaining));
       timeRemaining -= mstep->getStepTime_min();
    }
    return preins;
}

QVector<PreInstruction> Recipe::getHopSteps(Hop::Use type) const
{
   QVector<PreInstruction> preins;
   QString str;
   unsigned int i;

   preins.clear();

    for( i = 0; static_cast<int>(i) < hops.size(); ++i )
    {
       Hop* hop = hops[i];
       if( hop->getUse() == type )
       {
          if( type == Hop::USEBOIL )
             str = QObject::tr("Put %1 %2 into boil for %3.");
          else if( type == Hop::USEDRY_HOP )
             str = QObject::tr("Put %1 %2 into fermenter for %3.");
          else if( type == Hop::USEFIRST_WORT )
             str = QObject::tr("Put %1 %2 into first wort for %3.");
          else if( type == Hop::USEMASH )
             str = QObject::tr("Put %1 %2 into mash for %3.");
          else if( type == Hop::USEAROMA )
             str = QObject::tr("Steep %1 %2 in wort for %3.");
          else
          {
             Brewtarget::logW("Recipe::getHopSteps(): Unrecognized hop use.");
             str = QObject::tr("Use %1 %2 for %3");
          }

          str = str.arg(Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms))
                   .arg(hop->getName())
                   .arg(Brewtarget::displayAmount(hop->getTime_min(), Units::minutes));

          preins.push_back(PreInstruction(str, QObject::tr("Hop addition"), hop->getTime_min()));
       }
    }
   return preins;
}

QVector<PreInstruction> Recipe::getMiscSteps(Misc::Use type) const
{
   QVector<PreInstruction> preins;
   QString str;
   unsigned int i;

    for( i = 0; static_cast<int>(i) < miscs.size(); ++i )
    {
       Misc* misc = miscs[i];
       if( misc->getUse() == type )
       {
          if( type == Misc::USEBOIL )
             str = QObject::tr("Put %1 %2 into boil for %3.");
          else if( type == Misc::USEBOTTLING )
             str = QObject::tr("Use %1 %2 at bottling for %3.");
          else if( type == Misc::USEMASH )
             str = QObject::tr("Put %1 %2 into mash for %3.");
          else if( type == Misc::USEPRIMARY )
             str = QObject::tr("Put %1 %2 into primary for %3.");
          else if( type == Misc::USESECONDARY )
             str = QObject::tr("Put %1 %2 into secondary for %3.");
          else
          {
             Brewtarget::logW("Recipe::getMiscSteps(): Unrecognized misc use.");
             str = QObject::tr("Use %1 %2 for %3.");
          }

          str = str .arg(Brewtarget::displayAmount(misc->getAmount(), ((misc->getAmountIsWeight()) ? (Unit*)(Units::kilograms) : (Unit*)(Units::liters) )))
                    .arg(misc->getName())
                    .arg(Brewtarget::displayAmount(misc->getTime(), Units::minutes));

          preins.push_back(PreInstruction(str, QObject::tr("Misc addition"), misc->getTime()));
       }
    }
    return preins;
}

Instruction* Recipe::getFirstWortHops() const
{
   Instruction* ins;
   QString str,tmp;
   unsigned int i;
   bool hasHop = false;

   str = QObject::tr("Do first wort hopping with ");

   for( i = 0; static_cast<int>(i) < hops.size(); ++i )
   {
     Hop* hop = hops[i];
     if( hop->getUse() == Hop::USEFIRST_WORT )
     {
       tmp = QString("%1 %2,")
            .arg(Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms))
            .arg(hop->getName());
       str += tmp;
       hasHop = true;
     }
   }
   str += ".";
   if( hasHop )
   {
     ins = new Instruction();
     ins->setName(QObject::tr("First wort hopping"));
     ins->setDirections(str);
     ins->setReagent(tmp);
     return ins;
   }
   return 0;
}

Instruction* Recipe::getTopOff() const
{
   double wortInBoil_l = 0.0;
   QString str,tmp;
   Instruction* ins;

   if( equipment != 0 )
   {
      wortInBoil_l = estimateWortFromMash_l() - equipment->getLauterDeadspace_l();
      str = QObject::tr("You should now have %1 wort.")
         .arg(Brewtarget::displayAmount( wortInBoil_l, Units::liters));
      if ( equipment->getTopUpKettle_l() != 0 )
      {
         wortInBoil_l += equipment->getTopUpKettle_l();
         tmp = QObject::tr(" Add %1 water to the kettle, bringing pre-boil volume to %2.")
            .arg(Brewtarget::displayAmount(equipment->getTopUpKettle_l(), Units::liters))
            .arg(Brewtarget::displayAmount(wortInBoil_l, Units::liters));

         str += tmp;

         ins = new Instruction();
         ins->setName(QObject::tr("Pre-boil"));
         ins->setDirections(str);
         ins->setReagent(tmp);
         return ins;
      }
   }
   return 0;
}

bool Recipe::hasBoilFermentable()
{
   unsigned int i;
   for ( i = 0; static_cast<int>(i) < fermentables().size(); ++i )
   {
      Fermentable* ferm = fermentables()[i];
      if( ferm->isMashed() || ferm->addAfterBoil() )
         continue;
      else
         return true;
   }
   return false;
}

PreInstruction Recipe::getBoilFermentables(double timeRemaining) const
{
   bool hasFerms = false;
   QString str;
   unsigned int i;

   str = QObject::tr("Boil or steep ");
   for( i = 0; static_cast<int>(i) < fermentables.size(); ++i )
   {
     Fermentable* ferm = fermentables[i];
     if( ferm->getIsMashed() || ferm->getAddAfterBoil() )
       continue;

     hasFerms = true;
     str += QString("%1 %2, ")
          .arg(Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms))
          .arg(ferm->getName());
   }
   str += ".";

   return PreInstruction(str, QObject::tr("Boil/steep fermentables"), timeRemaining);
}

Instruction* Recipe::getPostboilFermentables()
{
   Instruction* ins;
   QString str,tmp;
   unsigned int i;
   bool hasFerms = false;

   str = QObject::tr("Add ");
   for( i = 0; static_cast<int>(i) < fermentables.size(); ++i )
   {
      Fermentable* ferm = fermentables[i];
      if( ! ferm->getAddAfterBoil() )
         continue;

      hasFerms = true;
      tmp = QString("%1 %2, ")
             .arg(Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms))
             .arg(ferm->getName());
      str += tmp;
   }
   str += QObject::tr("to the boil at knockout.");

   if( hasFerms )
   {
      ins = new Instruction();
      ins->setName(QObject::tr("Knockout additions"));
      ins->setDirections(str);
      ins->setReagent(tmp);
      return ins;
   }
   else
   {
      return 0;
   }
}

Instruction* Recipe::getPostboilSteps()
{
   QString str;
   Instruction* ins;
   double wort_l = 0.0;
   double wortInBoil_l = 0.0;

   if( equipment != 0 )
   {
      wortInBoil_l = estimateWortFromMash_l() - equipment->getLauterDeadspace_l();
      if ( equipment->getTopUpKettle_l() != 0 )
         wortInBoil_l += equipment->getTopUpKettle_l();

      wort_l = equipment->wortEndOfBoil_l(wortInBoil_l);
      str = QObject::tr("You should have %1 wort post-boil.")
            .arg(Brewtarget::displayAmount( wort_l, Units::liters));
      str += QObject::tr("\nYou anticipate losing %1 to trub and chiller loss.")
            .arg(Brewtarget::displayAmount( equipment->getTrubChillerLoss_l(), Units::liters));
      wort_l -= equipment->getTrubChillerLoss_l();
      if( equipment->getTopUpWater_l() > 0.0 )
          str += QObject::tr("\nAdd %1 top up water into primary.")
               .arg(Brewtarget::displayAmount( equipment->getTopUpWater_l(), Units::liters));
      wort_l += equipment->getTopUpWater_l();
      str += QObject::tr("\nThe final volume in the primary is %1.")
             .arg(Brewtarget::displayAmount(wort_l, Units::liters));

      ins = new Instruction();
      ins->setName(QObject::tr("Post boil"));
      ins->setDirections(str);
      return ins;
   }
   else
   {
      return 0;
   }
}

void Recipe::addPreinstructions( QVector<PreInstruction> preins )
{
   unsigned int i;
   Instruction* ins;

    // Add instructions in descending mash time order.
    qSort( preins.begin(), preins.end(), qGreater<PreInstruction>() );
    for( i=0; static_cast<int>(i) < preins.size(); ++i )
    {
       PreInstruction pi = preins[i];
       ins = new Instruction();
       ins->setName(pi.getTitle());
       ins->setDirections(pi.getText());
       ins->setInterval(pi.getTime());
       instructions.push_back(ins);
    }
}

void Recipe::generateInstructions()
{
   Instruction* ins;
   instructions.clear();
   QString str, tmp;
   unsigned int i, size;
   double timeRemaining;
   double totalWaterAdded_l = 0.0;

   QVector<PreInstruction> preinstructions;

   // Mash instructions

   if( mash != 0 && mash->getNumMashSteps() > 0 )
   {
     size = mash->getNumMashSteps();

     /*** prepare mashed fermentables ***/
     instructions += getMashFermentable();

     /*** Prepare water additions ***/
     instructions += getMashWater(size);

     timeRemaining = mash->getTotalTime();

     /*** Generate the mash instructions ***/
     preinstructions = getMashInstructions(timeRemaining, totalWaterAdded_l, size);

      /*** Hops mash additions ***/
     preinstructions += getHopSteps(Hop::USEMASH);

      /*** Misc mash additions ***/
     preinstructions += getMiscSteps(Misc::USEMASH);

     /*** Add the preinstructions into the instructions ***/
     addPreinstructions(preinstructions);

   } // END mash instructions.

   // First wort hopping
   ins = getFirstWortHops();
   if ( ins )
      instructions += ins;

   // Need to top up the kettle before boil?
   ins = getTopOff();
   if ( ins )
      instructions += ins;

   // Boil instructions
   preinstructions.clear();   
   
   // Find boil time.
   if( equipment != 0 )
      timeRemaining = equipment->getBoilTime_min();
   else
   {
      timeRemaining = Brewtarget::timeQStringToSI(QInputDialog::getText(0,
                                        QObject::tr("Boil time"),
                                        QObject::tr("You did not configure an equipment (which you really should), so tell me the boil time.")));
   }
   
   str = QObject::tr("Bring the wort to a boil and hold for %1.").arg(Brewtarget::displayAmount( timeRemaining, Units::minutes));
   ins = new Instruction();
   ins->setName(QObject::tr("Start boil"));
   ins->setInterval(timeRemaining);
   ins->setDirections(str);
   instructions.push_back(ins);
   
   /*** Get fermentables we haven't added yet ***/
   if ( hasBoilFermentable() )
      preinstructions.push_back(getBoilFermentables(timeRemaining));
   
   /*** Boiled hops ***/
   preinstructions += getHopSteps(Hop::USEBOIL);

   /*** Boiled miscs ***/
   preinstructions += getMiscSteps(Misc::USEBOIL);

   // END boil instructions.

   // Add instructions in descending mash time order.
   addPreinstructions(preinstructions);

   // FLAMEOUT
   instructions += new Instruction(QObject::tr("Flameout"),
                                   QObject::tr("Stop boiling the wort."));

   // Steeped aroma hops
   preinstructions.clear();
   preinstructions += getHopSteps(Hop::USEAROMA);
   addPreinstructions(preinstructions);
   
   // Fermentation instructions
   preinstructions.clear();

   /*** Fermentables added after boil ***/
   ins = getPostboilFermentables();
   if ( ins )
      instructions += ins;

   /*** post boil ***/
   ins = getPostboilSteps();
   if ( ins )
      instructions += ins;
   
   /*** Primary yeast ***/
   str = QObject::tr("Cool wort and pitch ");
   for( i = 0; static_cast<int>(i) < yeasts.size(); ++i )
   {
      Yeast* yeast = yeasts[i];
      if( ! yeast->getAddToSecondary() )
         str += QObject::tr("%1 %2 yeast, ").arg(yeast->getName()).arg(yeast->getTypeString());
   }
   str += QObject::tr("to the primary.");
   ins = new Instruction();
   ins->setName(QObject::tr("Pitch yeast"));
   ins->setDirections(str);
   instructions.push_back(ins);
   /*** End primary yeast ***/

   /*** Primary misc ***/
   addPreinstructions( getMiscSteps(Misc::USEPRIMARY));
//   instructions += getMiscSteps(Misc::USEPRIMARY);

   str = QObject::tr("Let ferment until FG is %1.")
         .arg(Brewtarget::displayAmount(fg));
   ins = new Instruction();
   ins->setName(QObject::tr("Ferment"));
   ins->setDirections(str);
   instructions.push_back(ins);

   str = QObject::tr("Transfer beer to secondary.");
   ins = new Instruction();
   ins->setName(QObject::tr("Transfer to secondary"));
   ins->setDirections(str);
   instructions.push_back(ins);

   /*** Secondary misc ***/
   addPreinstructions( getMiscSteps(Misc::USESECONDARY));

   /*** Dry hopping ***/
   addPreinstructions(getHopSteps(Hop::USEDRY_HOP));

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
      if( h->getUse() != Hop::USEBOIL )
         continue;
      if( h->getTime_min() < time && h->getTime_min() > max )
      {
         ret = QObject::tr("Add %1 %2 to boil at %3.")
               .arg(Brewtarget::displayAmount(h->getAmount_kg(), Units::kilograms))
               .arg(h->getName())
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
      if( m->getUse() != Misc::USEBOIL )
         continue;
      if( m->getTime() < time && m->getTime() > max )
      {
         ret = QObject::tr("Add %1 %2 to boil at %3.");
         if( m->getAmountIsWeight() )
            ret = ret.arg(Brewtarget::displayAmount(m->getAmount(), Units::kilograms));
         else
            ret = ret.arg(Brewtarget::displayAmount(m->getAmount(), Units::liters));

         ret = ret.arg(m->getName());
         ret = ret.arg(Brewtarget::displayAmount(m->getTime(), Units::minutes));
         max = m->getTime();
         foundSomething = true;
      }
   }
   
   time = foundSomething ? max : -1.0;
   return ret;
}

//============================Relational Setters===============================

void Recipe::addHop( Hop *var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addFermentable( Fermentable* var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addMisc( Misc* var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addYeast( Yeast* var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addWater( Water* var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addBrewNote(BrewNote* var)
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::setMash( Mash *var )
{
   if( var == NULL )
      return;

   mash = var;
   addObserved(mash);
   hasChanged(QVariant(MASH));
   
   if( var )
   {
      if( mash() )
         disconnect( mash(), SIGNAL(changed(QMetaProperty,QVariant)), this, 0 );
      
      connect( var, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(parseChanges(QMetaProperty,QVariant)) );
      set( "mash", "mash", var->key );
   }
}

void Recipe::setEquipment( Equipment *var )
{
   if( var )
      set( "equipment", "equipment", var->key );
}

void Recipe::setStyle( Style *var )
{
   if( var )
      set( "style", "style", var->key );
}

//==============================="SET" METHODS=================================
void Recipe::setName( const QString &var )
{
   set( "name", "name", var );
}

void Recipe::setType( const QString &var )
{
   QString tmp;
   if( ! isValidType(var) )
   {
      Brewtarget::logW( QString("Recipe: invalid type: %1").arg(var) );
      tmp = "All Grain";
   }
   else
   {
      tmp = QString(var);
   }

   set( "type", "type", tmp );
}

void Recipe::setBrewer( const QString &var )
{
   set( "brewer", "brewer", var );
}

void Recipe::setBatchSize_l( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: batch size < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "batchSize_l", "batch_size", tmp );
}

void Recipe::setBoilSize_l( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: boil size < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "boilSize_l", "boil_size", tmp );
}

void Recipe::setBoilTime_min( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: boil time < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "boilTime_min", "boil_time", tmp );
}

void Recipe::setEfficiency_pct( double var )
{
   double tmp;
   if( var < 0.0  || var > 100.0 )
   {
      Brewtarget::logW( QString("Recipe: 0 < efficiency < 100: %1").arg(var) );
      tmp = 70;
   }
   else
   {
      tmp = var;
   }

   set( "efficiency_pct", "efficiency", tmp );
}

void Recipe::setAsstBrewer( const QString &var )
{
   set( "asstBrewer", "assistant_brewer", var );
}

void Recipe::setNotes( const QString &var )
{
   set( "notes", "notes", var );
}

void Recipe::setTasteNotes( const QString &var )
{
   set( "tasteNotes", "taste_notes", var );
}

void Recipe::setTasteRating( double var )
{
   double tmp;
   if( var < 0.0 || var > 50.0 )
   {
      Brewtarget::logW( QString("Recipe: 0 < taste rating < 50: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "tasteRating", "taste_rating", var );
}

void Recipe::setOg( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: og < 0: %1").arg(var) );
      tmp = 1.0;
   }
   else
   {
      tmp = var;
   }

   set( "og", "og", tmp );
}

void Recipe::setFg( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: fg < 0: %1").arg(var) );
      tmp = 1.0;
   }
   else
   {
      tmp = var;
   }

   set( "fg", "fg", tmp );
}

void Recipe::setFermentationStages( int var )
{
   int tmp;
   if( var < 0 )
   {
      Brewtarget::logW( QString("Recipe: stages < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "fermentationStages", "fermentation_stages", tmp );
}

void Recipe::setPrimaryAge_days( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: primary age < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "primaryAge_days", "primary_age", tmp );
}

void Recipe::setPrimaryTemp_c( double var )
{
   set( "primaryTemp_c", "primary_temp", var );
}

void Recipe::setSecondaryAge_days( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: secondary age < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "secondaryAge_days", "secondary_age", tmp );
}

void Recipe::setSecondaryTemp_c( double var )
{
   secondaryTemp_c = var;
   hasChanged();
}

void Recipe::setTertiaryAge_days( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: tertiary age < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "tertiaryAge_days", "tertiary_age", tmp );
}

void Recipe::setTertiaryTemp_c( double var )
{
   set( "tertiaryTemp_c", "tertiary_temp", var );
}

void Recipe::setAge_days( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: age < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "age_days", "age_days", tmp );
}

void Recipe::setAgeTemp_c( double var )
{
   set( "ageTemp_c", "age_temp", var );
}

void Recipe::setDate( const QDate &var )
{
   set( "date", "brew_date", var.toString("yyyy-MM-dd") );
}

void Recipe::setCarbonation_vols( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: carb < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "carbonation_vols", "carb_volume", var );
}

void Recipe::setForcedCarbonation( bool var )
{
   set( "forcedCarbonation", "forced_carb", var );
}

void Recipe::setPrimingSugarName( const QString &var )
{
   set( "primingSugarName", "priming_sugar_name", var );
}

void Recipe::setCarbonationTemp_c( double var )
{
   set( "carbonationTemp", "carbonation_temp", var );
}

void Recipe::setPrimingSugarEquiv( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: primingsugarequiv < 0: %1").arg(var) );
      tmp = 1;
   }
   else
   {
      tmp = var;
   }

   set( "primingSugarEquiv", "priming_sugar_equiv", tmp );
}

void Recipe::setKegPrimingFactor( double var )
{
   double tmp;
   
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: keg priming factor < 0: %1").arg(var) );
      tmp = 1;
   }
   else
   {
      tmp = var;
   }

   set( "kegPrimingFactor", "keg_priming_factor", tmp );
}

//==========================Calculated Getters============================

double Recipe::getOg() const
{
   return og;
}

double Recipe::getFg() const
{
   return fg;
}

double Recipe::color_srm()
{
   return color_srm;
}

double Recipe::ABV_pct()
{
   return ABV_pct;
}

//=========================Relational Getters=============================

Style* Recipe::getStyle() const
{
   return Database::instance().style( get("style").toInt() );
}

Mash* Recipe::getMash() const
{
   return Database::instance().mash( get("mash").toInt() );
}

Equipment* Recipe::getEquipment() const
{
   return Database::instance().equipment( get("equipment").toInt() );
}

// Should these "num" methods exist?
unsigned int Recipe::numHops() const
{
   return hops().size();
}

unsigned int Recipe::numFermentables() const
{
   return fermentables().size();
}

unsigned int Recipe::numMiscs() const
{
   return miscs().size();
}

unsigned int Recipe::numYeasts() const
{
   return yeasts().size();
}

unsigned int Recipe::numWaters() const
{
   return waters().size();
}

unsigned int Recipe::numBrewNotes() const
{
   return brewNotes().size();
}

//==============================Getters===================================
QString Recipe::name() const
{
   return get("name").toString();
}

QString Recipe::type() const
{
   return get("type").toString();
}

QString Recipe::brewer() const
{
   return get("brewer").toString();
}

double Recipe::batchSize_l() const
{
   return get("batch_size").toDouble();
}

double Recipe::boilSize_l() const
{
   return get("boil_size").toDouble();
}

double Recipe::boilTime_min() const
{
   return get("boil_time").toDouble();
}

double Recipe::efficiency_pct() const
{
   return get("efficiency").toDouble();
}

QString Recipe::asstBrewer() const
{
   return get("assistant_brewer").toString();
}

QString Recipe::notes() const
{
   return get("notes").toString();
}

QString Recipe::tasteNotes() const
{
   return get("taste_notes").toString();
}

double Recipe::tasteRating() const
{
   return get("taste_rating").toDouble();
}

int Recipe::fermentationStages() const
{
   return get("fermentation_stages").toInt();
}

double Recipe::primaryAge_days() const
{
   return get("primary_age").toDouble();
}

double Recipe::primaryTemp_c() const
{
   return get("primary_temp").toDouble();
}

double Recipe::secondaryAge_days() const
{
   return get("secondary_age").toDouble();
}

double Recipe::secondaryTemp_c() const
{
   return get("secondary_temp").toDouble();
}

double Recipe::tertiaryAge_days() const
{
   return get("tertiary_age").toDouble();
}

double Recipe::tertiaryTemp_c() const
{
   return get("tertiary_temp").toDouble();
}

double Recipe::age_days() const
{
   return get("age").toDouble();
}

double Recipe::ageTemp_c() const
{
   return get("age_temp").toDouble();
}

QDate Recipe::date() const
{
   return QDate::fromString( get("brew_date").toString(), "yyyy-MM-dd" );
}

double Recipe::carbonation_vols() const
{
   return get("carb_volume").toDouble();
}

bool Recipe::forcedCarbonation() const
{
   return get("forced_carb").toBool();
}

QString Recipe::primingSugarName() const
{
   return get("priming_sugar_name").toString();
}

double Recipe::carbonationTemp_c() const
{
   return get("carbonation_temp").toDouble();
}

double Recipe::primingSugarEquiv() const
{
   return get("priming_sugar_equiv").toDouble();
}

double Recipe::kegPrimingFactor() const
{
   return get("keg_priming_factor").toDouble();
}

//=============================Removers========================================

// Returns true if var is found and removed.
void Recipe::removeHop( Hop *var )
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeFermentable(Fermentable* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeMisc(Misc* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeWater(Water* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeYeast(Yeast* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeBrewNote(BrewNote* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeBrewNote(QList<BrewNote*> var)
{
   Database::instance().removeFromRecipe( this, var );
}

//==============================Recalculators==================================

// The theoretical maximum yield without any non-mashed anything.  This
// will need to be communicated somewhere.
void Recipe::recalcPoints(double volume)
{
   unsigned int i;
   Fermentable* ferm;
   double sugar_kg = 0.0;
   double sugar_kg_ignoreEfficiency = 0.0;
   Fermentable::Type type;

   // Calculate OG
   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];
      if( ferm->addAfterBoil() )
         continue;

      // If we have some sort of non-grain, we have to ignore efficiency.
      type = ferm->type();
      if( type==Fermentable::TYPESUGAR || type==Fermentable::TYPEEXTRACT || type==Fermentable::TYPEDRY_EXTRACT )
         sugar_kg_ignoreEfficiency += (ferm->yield_pct()/100.0)*ferm->amount_kg();
      else
         sugar_kg += (ferm->yield_pct()/100.0)*ferm->amount_kg();
   }

   points = 1000 * (  Algorithms::Instance().PlatoToSG_20C20C( Algorithms::Instance().getPlato(sugar_kg,volume)) - 1);
   
   emit changed( metaObject().property( metaObject().indexOfProperty("points") ), points );
}

void Recipe::recalcABV_pct()
{
    // Alcohol by weight.  This is a different formula than used
    // when calculating the calories.
    //abw = 76.08 * (og-fg)/(1.775-og);
    //return abw * (fg/0.794);

    // George Fix: Brewing Science and Practice, page 686.
    // The multiplicative factor actually varies from
    // 125 for weak beers to 135 for strong beers.
    ABV_pct = 130*(og()-fg());

    // From http://en.wikipedia.org/w/index.php?title=Alcohol_by_volume&oldid=414661414
    // Has no citations, so I don't know how trustworthy it is.
    // It seems to be in conflict with Fix's method above, because
    // if the beer is weak, it should have a low fg, meaning the
    // multiplicative factor is higher.
    // return 132.9*(og - fg)/fg;
    
    emit changed( metaObject().property( metaObject().indexOfProperty("ABV_pct") ), ABV_pct );
}

void Recipe::recalcColor_srm()
{
   Fermentable *ferm;
   double mcu = 0.0;
   unsigned int i;

   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];
      // Conversion factor for lb/gal to kg/l = 8.34538.
      mcu += ferm->color_srm()*8.34538 * ferm->amount_kg()/finalVolume_l;
   }

   color_srm = ColorMethods::mcuToSrm(mcu);
   
   emit changed( metaObject().property( metaObject().indexOfProperty("color_srm") ), color_srm );
}

void Recipe::recalcBoilGrav()
{
   unsigned int i;
   Fermentable* ferm;
   double sugar_kg = 0.0;
   double sugar_kg_ignoreEfficiency = 0.0;
   Fermentable::Type type;

   // Calculate OG
   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];
      if( ferm->addAfterBoil() )
         continue;

      // If we have some sort of non-grain, we have to ignore efficiency.
      type = ferm->type();
      if( type==Fermentable::TYPESUGAR || type==Fermentable::TYPEEXTRACT || type==Fermentable::TYPEDRY_EXTRACT )
         sugar_kg_ignoreEfficiency += (ferm->yield_pct()/100.0)*ferm->amount_kg();
      else
         sugar_kg += (ferm->yield_pct()/100.0)*ferm->amount_kg();
   }

   // We might lose some sugar in the form of lauter deadspace.
   /*** Forget lauter deadspace...this loss is included in efficiency ***/

   // Since the efficiency refers to how much sugar we get into the fermenter,
   // we need to adjust for that here.
   sugar_kg = (efficiency_pct()/100.0 * sugar_kg + sugar_kg_ignoreEfficiency);
   if( equipment() )
      sugar_kg = sugar_kg / (1 - equipment()->getTrubChillerLoss_l()/estimatePostBoilVolume_l());

   boilGrav = Algorithms::Instance().PlatoToSG_20C20C( Algorithms::Instance().getPlato(sugar_kg, estimateBoilVolume_l()) );
   
   emit changed( metaObject().property( metaObject().indexOfProperty("boilGrav") ), boilGrav );
}

void Recipe::recalcIBU()
{
   unsigned int i;
   double ibus = 0.0;
   
   // Bitterness due to hops...
   QList<Hop*> hops = hops();
   for( i = 0; static_cast<int>(i) < hops.size(); ++i )
      ibus += ibuFromHop(hops[i]);

   // Bitterness due to hopped extracts...
   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      // Conversion factor for lb/gal to kg/l = 8.34538.
      ibus +=
              fermentables[i]->ibuGalPerLb() *
              (fermentables[i]->amount_kg() / batchSize_l()) / 8.34538;
   }

   IBU = ibus;
   
   emit changed( metaObject().property( metaObject().indexOfProperty("IBU") ), IBU );
}

void Recipe::recalcVolumeEstimates()
{
   // wortFromMash_l ==========================
   double waterAdded_l;
   double absorption_lKg;
   
   if( mash == 0 )
      wortFromMash_l = 0.0;
   else
   {
   
       waterAdded_l = mash->totalMashWater_l();
       if( equipment != 0 )
          absorption_lKg = equipment->grainAbsorption_LKg();
       else
          absorption_lKg = HeatCalculations::absorption_LKg;

       wortFromMash_l = (waterAdded_l - absorption_lKg * grainsInMash_kg);
   }
   
   // boilVolume_l ==============================
   double mashVol_l;
   double tmp = 0.0;
   
   //if( mashVol_l <= 0.0 ) // Give up.
   //   return boilSize_l;
   
   if( equipment() != 0 )
      tmp = wortFromMash_l - equipment()->lauterDeadspace_l() + equipment()->topUpKettle_l();
   else
      tmp = wortFromMash_l;
   
   if( tmp <= 0.0 )
      tmp = boilSize_l(); // Give up.
   
   boilVolume_l = tmp;
   
   // finalVolume_l ==============================
   
   if( equipment() != 0 )
      finalVolume_l = equipment()->wortEndOfBoil_l(boilVolume_l) - equipment->trubChillerLoss_l() + equipment->topUpWater_l();
   else
      finalVolume_l = boilVolume_l - 4.0; // This is just shooting in the dark. Can't do much without an equipment.
   
   // postBoilVolume_l ===========================

   if( equipment() != 0 )
      postBoilVolume_l = equipment->wortEndOfBoil_l( boilVolume_l );
   else
      postBoilVolume_l = batchSize_l(); // Give up.
      
   // Emit changes.
   emit changed( metaObject().property( metaObject().indexOfProperty("wortFromMash_l") ), wortFromMash_l );
   emit changed( metaObject().property( metaObject().indexOfProperty("boilVolume_l") ), boilVolume_l );
   emit changed( metaObject().property( metaObject().indexOfProperty("finalVolume_l") ), finalVolume_l );
   emit changed( metaObject().property( metaObject().indexOfProperty("postBoilVolume_l") ), postBoilVolume_l );
}

void Recipe::recalcGrainsInMash_kg()
{
   unsigned int i, size;
   double ret = 0.0;
   Fermentable* ferm;
   
   QList<Fermentable*> ferms = fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
   {
      ferm = ferms[i];
      
      if( ferm->type() == Fermentable::TYPEGRAIN && ferm->isMashed() )
         ret += ferm->amount_kg();
   }
   
   grainsInMash_kg = ret;
   
   emit changed( metaObject().property( metaObject().indexOfProperty("grainsInMash_kg") ), grainsInMash_kg );
}

void Recipe::recalcGrains_kg()
{
   unsigned int i, size;
   double ret = 0.0;

   QList<Fermentable*> ferms = fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
      ret += ferms[i]->amount_kg();

   grains_kg = ret;
   
   emit changed( metaObject().property( metaObject().indexOfProperty("grains_kg") ), grains_kg );
}

void Recipe::recalcSRMColor()
{
   /**** Original method from a website: Came out dark. ***
   
   // Luminance Y
   double Y = 94.6914*exp(-0.131272*color_srm);
   // Chroma x
   double x = 0.73978 - 0.25442*exp(-0.037865*color_srm) - 0.017511*exp(-0.24307*color_srm);
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

   double red = 232.9 * pow( (double)0.93, color_srm );
   double green = (double)-106.25 * log(color_srm) + 280.9;

   int r = (red < 0)? 0 : ((red > 255)? 255 : (int)Algorithms::Instance().round(red));
   int g = (green < 0)? 0 : ((green > 255)? 255 : (int)Algorithms::Instance().round(green));
   int b = 0;

   SRMColor.setRgb( r, g, b );

   emit changed( metaObject().property( metaObject().indexOfProperty("SRMColor") ), SRMColor );
}

// the formula in here are taken from http://hbd.org/ensmingr/
void Recipe::recalcCalories()
{
    double startPlato, finishPlato, RE, abw, og, fg;

    og = og();
    fg = fg();

    // Need to translate OG and FG into plato
    startPlato  = -463.37 + ( 668.72 * og ) - (205.35 * og * og);
    finishPlato = -463.37 + ( 668.72 * fg ) - (205.35 * fg * fg);

    // RE (real extract)
    RE = (0.1808 * startPlato) + (0.8192 * finishPlato);

    // Alcohol by weight?
    abw = (startPlato-RE)/(2.0665 - (0.010665 * startPlato));

    calories = ((6.9*abw) + 4.0 * (RE-0.1)) * fg * 3.55;

    emit changed( metaObject().property( metaObject().indexOfProperty("calories") ), calories );
}

void Recipe::recalcOgFg()
{
   unsigned int i;
   double kettleWort_l;
   double postBoilWort_l;
   double plato;
   double points = 0;
   double ratio = 0;
   double sugar_kg = 0;
   double sugar_kg_ignoreEfficiency = 0.0;
   Fermentable::Type fermtype;
   double attenuation_pct = 0.0;
   Fermentable* ferm;
   Yeast* yeast;
   
   QList<Fermentable*> ferms = fermentables();
   
   // Calculate OG
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];

      // If we have some sort of non-grain, we have to ignore efficiency.
      fermtype = ferm->type();
      if( fermtype==Fermentable::TYPESUGAR || fermtype==Fermentable::TYPEEXTRACT || fermtype==Fermentable::TYPEDRY_EXTRACT )
         sugar_kg_ignoreEfficiency += ferm->equivSucrose_kg();
      else
         sugar_kg += ferm->equivSucrose_kg();
   }

   // We might lose some sugar in the form of Trub/Chiller loss and lauter deadspace.
   if( equipment() != 0 )
   {
      /* Ignore lauter deadspace since it should be included in efficiency ***
      // First, lauter deadspace.
      ratio = (estimateWortFromMash_l() - equipment->getLauterDeadspace_l()) / (estimateWortFromMash_l());
      if( ratio > 1.0 ) // Usually happens when we don't have a mash yet.
         ratio = 1.0;
      else if( ratio < 0.0 ) // Only happens if the user is stupid with lauter deadspace.
         ratio = 0.0;
      else if( isnan(ratio) )
         ratio = 1.0; // Need this in case we have no mash, and therefore end up with NaN.
      
      sugar_kg *= ratio;
      // Don't consider this one since nobody adds sugar or extract to the mash.
      //sugar_kg_ignoreEfficiency *= ratio;
      */
      
      // Next, trub/chiller loss.
      kettleWort_l = (wortFromMash_l - equipment()->lauterDeadspace_l()) + equipment()->topUpKettle_l();
      postBoilWort_l = equipment->wortEndOfBoil_l(kettleWort_l);
      ratio = (postBoilWort_l - equipment()->trubChillerLoss_l()) / postBoilWort_l;
      if( ratio > 1.0 ) // Usually happens when we don't have a mash yet.
         ratio = 1.0;
      else if( ratio < 0.0 )
         ratio = 0.0;
      else if( Algorithms::Instance().isnan(ratio) )
         ratio = 1.0;
      // Ignore this again since it should be included in efficiency.
      //sugar_kg *= ratio;
      sugar_kg_ignoreEfficiency *= ratio;
   }

   // Combine the two sugars.
   sugar_kg = sugar_kg * getEfficiency_pct()/100.0 + sugar_kg_ignoreEfficiency;
   plato = Algorithms::Instance().getPlato( sugar_kg, estimateFinalVolume_l());

   og = Algorithms::Instance().PlatoToSG_20C20C( plato );
   points = (og-1)*1000.0;

   // Calculage FG
   for( i = 0; static_cast<int>(i) < yeasts.size(); ++i )
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
   
   emit changed( metaObject().property( metaObject().indexOfProperty("og") ), og );
   emit changed( metaObject().property( metaObject().indexOfProperty("fg") ), fg );
}

//====================================Helpers===========================================

double Recipe::ibuFromHop(Hop const* hop)
{
   double ibus = 0.0;
   
   if( hop == 0 )
      return 0.0;
   
   double AArating = hop->alpha_pct()/100.0;
   double grams = hop->amount_kg()*1000.0;
   double minutes = hop->time_min();
   //double water_l = estimateFinalVolume_l();
   //double boilVol_l = estimateBoilVolume_l();
   //double boilGrav = boilGrav();
   double boilGrav_final = boilGrav; 
   double avgBoilGrav;
   
   if( equipment )
      boilGrav_final = boilVolume_l / equipment->wortEndOfBoil_l( boilVolume_l ) * (boilGrav-1) + 1;
   
   avgBoilGrav = (boilGrav + boilGrav_final) / 2;
   //avgBoilGrav = boilGrav;
   
   if( hops[i]->getUse() == Hop::USEBOIL)
      ibus = IbuMethods::getIbus( AArating, grams, finalVolume_l, avgBoilGrav, minutes );
   else if( hops[i]->getUse() == Hop::USEFIRST_WORT )
      ibus = 1.10 * IbuMethods::getIbus( AArating, grams, finalVolume_l, avgBoilGrav, 20 ); // I am estimating First wort hops give 10% more ibus than a 20 minute addition.

   // Adjust for hop form.
   if( hops[i]->getForm() == Hop::FORMLEAF )
      ibus *= 0.90;
   else if( hops[i]->getForm() == Hop::FORMPLUG )
      ibus *= 0.92;
   
   return ibus;
}

bool Recipe::isValidType( const QString &str )
{
   static const QString types[] = {"Extract", "Partial Mash", "All Grain"};
   static const unsigned int size = 3;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( str == types[i] )
         return true;
   
   return false;
}

void Recipe::parseChanges(QMetaProperty prop, QVariant val)
{
   QObject sender = sender();
   QString senderClass(sender.className());
   
   // Pass along the signal if it's one of our ingredients.
   // I don't know really what to emit here...
   /*
   if( senderClass == "Hop" )
      emit changed(...);
   else if( senderClass == "Fermentable" )
      emit changed(...);
   else if( senderClass == "Misc" )
      emit changed(...);
   else if( senderClass == "Yeast" )
      emit changed(...);
   else if( senderClass == "Water" )
      emit changed(...);
   else if( senderClass == "BrewNote" )
      emit changed(...);
   else if( senderClass == "Instruction" )
      emit changed(...);
   */
}