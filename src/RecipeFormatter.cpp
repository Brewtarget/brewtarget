/*
* RecipeFormatter.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "RecipeFormatter.h"
#include "style.h"
#include "fermentable.h"
#include "hop.h"
#include "misc.h"
#include "yeast.h"
#include "mash.h"
#include "unit.h"
#include "brewtarget.h"
#include <QClipboard>

RecipeFormatter::RecipeFormatter()
{
   textSeparator = 0;
   rec = 0;
}

void RecipeFormatter::setRecipe(Recipe* recipe)
{
   rec = recipe;
}

QString RecipeFormatter::getTextFormat()
{
   QString ret = "";
   if( rec == 0 )
      return ret;
   
   unsigned int i, size;
   
   Style* style = rec->getStyle();
   Mash* mash = rec->getMash();
   
   // Vital statistics.
   ret += rec->getName().c_str();
   if( style != 0 && style->getName() != "" )
      ret += (" - " + style->getName()).c_str();
   ret += "\n";
   
   ret += getTextSeparator();
   
   ret += "Batch Size: " + Brewtarget::displayAmount(rec->getBatchSize_l(), Units::liters) + "\n";
   ret += "Boil Size: " + Brewtarget::displayAmount(rec->getBoilSize_l(), Units::liters) + "\n";
   ret += "Boil Time: " + Brewtarget::displayAmount(rec->getBoilTime_min(), Units::minutes) + "\n";
   ret += "Efficiency: " + QString("%1\%").arg(rec->getEfficiency_pct(), 0, 'f', 0) + "\n";
   ret += "OG: " + QString("%1").arg( rec->getOg(), 0, 'f', 3 ) + "\n";
   ret += "FG: " + QString("%1").arg( rec->getFg(), 0, 'f', 3 ) + "\n";
   ret += "ABV: " + QString("%1\%").arg( rec->getABV_pct(), 0, 'f', 0 ) + "\n";
   ret += "Bitterness: " + QString("%1 IBUs").arg( rec->getIBU(), 0, 'f', 1 ) + "\n";
   ret += "Color: " + QString("%1 SRM").arg( rec->getColor_srm(), 0, 'f', 0 ) + "\n";
   
   if( rec->getNumFermentables() > 0 )
   {
      ret += "\n";
      ret += "Fermentables\n";
      ret += getTextSeparator();
      
      ret += "Name\t\tType\t\tAmount\t\tMashed\t\tYield\t\tColor\n";
      
      size = rec->getNumFermentables();
      for( i = 0; i < size; ++i )
      {
	 Fermentable* ferm =  rec->getFermentable(i);
	 ret += (ferm->getName() + "\t\t").c_str();
	 ret += (ferm->getType() + "\t\t").c_str();
	 ret += Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms) + "\t\t";
	 ret += ferm->getIsMashed() ? "Yes\t\t" : "No\t\t";
	 ret += QString("%1\%").arg(ferm->getYield_pct(), 0, 'f', 0) + "\t\t";
	 ret += QString("%1 L").arg(ferm->getColor_srm(), 0, 'f', 0) + "\n";
      }
   }
   
   if( rec->getNumHops() > 0 )
   {
      ret += "\n";
      ret += "Hops\n";
      ret += getTextSeparator();
      
      ret += "Name\t\tAlpha\t\tAmount\t\tUse\t\tTime\t\tIBU\n";
      
      size = rec->getNumHops();
      for( i = 0; i < size; ++i )
      {
	 Hop* hop = rec->getHop(i);
	 ret += (hop->getName() + "\t\t").c_str();
	 ret += QString("%1").arg(hop->getAlpha_pct(), 0, 'f', 1) + "\t\t";
	 ret += Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms) + "\t\t";
	 ret += (hop->getUse() + "\t\t").c_str();
	 ret += Brewtarget::displayAmount(hop->getTime_min(), Units::minutes) + "\t\t";
	 ret += QString("%1").arg( rec->getIBUFromHop(i), 0, 'f', 1 ) + "\n";
      }
   }
   
   if( rec->getNumMiscs() > 0 )
   {
      ret += "\n";
      ret += "Misc\n";
      ret += getTextSeparator();
      
      ret += "Name\t\tType\t\tUse\t\tAmount\t\tTime\n";
      
      size = rec->getNumMiscs();
      for( i = 0; i < size; ++i )
      {
	 Misc* misc = rec->getMisc(i);
	 ret += (misc->getName() + "\t\t").c_str();
	 ret += (misc->getType() + "\t\t").c_str();
	 ret += (misc->getUse() + "\t\t").c_str();
	 ret += Brewtarget::displayAmount(misc->getTime(), Units::minutes) + "\n";
      }
   }
   
   if( rec->getNumYeasts() > 0 )
   {
      ret += "\n";
      ret += "Yeast\n";
      ret += getTextSeparator();
      
      ret += "Name\t\tType\t\tForm\t\tAmount\t\tStage\n";
      
      size = rec->getNumYeasts();
      for( i = 0; i < size; ++i )
      {
         Yeast* y = rec->getYeast(i);
	 ret += (y->getName() + "\t\t").c_str();
	 ret += (y->getType() + "\t\t").c_str();
	 ret += (y->getForm() + "\t\t").c_str();
         ret += Brewtarget::displayAmount( y->getAmount(), y->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters ) + "\t\t";
         ret += (y->getAddToSecondary() ? "Secondary\n" : "Primary\n");
      }
   }
   
   if( mash && mash->getNumMashSteps() > 0 )
   {
      ret += "\n";
      ret += "Mash\n";
      ret += getTextSeparator();
      
      ret += "Name\t\tType\t\tAmount\t\tTarget\t\tTime\n";
      
      size = mash->getNumMashSteps();
      for( i = 0; i < size; ++i )
      {
	 MashStep* s = mash->getMashStep(i);
	 ret += (s->getName() + "\t\t").c_str();
	 ret += (s->getType() + "\t\t").c_str();
	 if( s->getType() == "Infusion" )
	    ret += Brewtarget::displayAmount( s->getInfuseAmount_l(), Units::liters ) + "\t\t";
	 else if( s->getType() == "Decoction" )
	    ret += Brewtarget::displayAmount( s->getDecoctionAmount_l(), Units::liters ) + "\t\t";
	 else
	    ret += "\t\t";
	 ret += Brewtarget::displayAmount( s->getStepTemp_c(), Units::celsius ) + "\t\t";
	 ret += Brewtarget::displayAmount( s->getStepTime_min(), Units::minutes ) + "\n";
      }
   }
}

QString RecipeFormatter::getTextSeparator()
{
   if( textSeparator != 0 )
      return *textSeparator;
   
   int i;
   textSeparator = new QString();
   for( i = 0; i < 80; ++i )
      textSeparator->append('=');
   
   textSeparator->append('\n');
   return *textSeparator;
}

QString RecipeFormatter::getHTMLFormat()
{
   return "";
}

QString RecipeFormatter::getBBCodeFormat()
{
   return "";
}

void RecipeFormatter::toTextClipboard()
{
   QApplication::clipboard()->setText(getTextFormat());
}