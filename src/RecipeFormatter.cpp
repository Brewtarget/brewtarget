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
   QString colorString;
   QString bitternessString;
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
   ret += "ABV: " + QString("%1\%").arg( rec->getABV_pct(), 0, 'f', 1 ) + "\n";
   ret += "Bitterness: ";
   bitternessString = QString("%1 IBUs (%2)").arg( rec->getIBU(), 0, 'f', 1 );
   switch( Brewtarget::ibuFormula )
   {
      case Brewtarget::TINSETH:
         bitternessString = bitternessString.arg("Tinseth");
         break;
      case Brewtarget::RAGER:
         bitternessString = bitternessString.arg("Rager");
         break;
   }
   ret += bitternessString + "\n";
   ret += "Color: ";
   colorString = QString("%1 SRM (%2)").arg( rec->getColor_srm(), 0, 'f', 0 );
   switch( Brewtarget::colorFormula )
   {
      case Brewtarget::MOREY:
         colorString = colorString.arg("Morey");
         break;
      case Brewtarget::DANIEL:
         colorString = colorString.arg("Daniels");
         break;
      case Brewtarget::MOSHER:
         colorString = colorString.arg("Mosher");
         break;
   }
   ret += colorString + "\n";
   
   if( rec->getNumFermentables() > 0 )
   {
      QStringList names, types, amounts, masheds, yields, colors;
      ret += "\n";
      ret += "Fermentables\n";
      ret += getTextSeparator();
      
      //ret += "Name\t\tType\t\tAmount\t\tMashed\t\tYield\t\tColor\n";
      names.append("Name");
      types.append("Type");
      amounts.append("Amount");
      masheds.append("Mashed");
      yields.append("Yield");
      colors.append("Color");
      
      size = rec->getNumFermentables();
      for( i = 0; i < size; ++i )
      {
         Fermentable* ferm =  rec->getFermentable(i);
         names.append( ferm->getName().c_str() );
         types.append( ferm->getType().c_str() );
         amounts.append( Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms) );
         masheds.append( ferm->getIsMashed() ? "Yes" : "No" );
         yields.append( QString("%1\%").arg(ferm->getYield_pct(), 0, 'f', 0) );
         colors.append( QString("%1 L").arg(ferm->getColor_srm(), 0, 'f', 0) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&masheds);
      padAllToMaxLength(&yields);
      padAllToMaxLength(&colors);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + amounts.at(i) + masheds.at(i) + yields.at(i) + colors.at(i) + "\n";
   }
   
   if( rec->getNumHops() > 0 )
   {
      QStringList names, alphas, amounts, uses, times, ibus;
      ret += "\n";
      ret += "Hops\n";
      ret += getTextSeparator();
      
      //ret += "Name\t\tAlpha\t\tAmount\t\tUse\t\tTime\t\tIBU\n";
      
      names.append( "Name" );
      alphas.append( "Alpha" );
      amounts.append( "Amount" );
      uses.append( "Use" );
      times.append( "Time" );
      ibus.append( "IBU" );
      
      size = rec->getNumHops();
      for( i = 0; i < size; ++i )
      {
         Hop* hop = rec->getHop(i);
         
         names.append( hop->getName().c_str() );
         alphas.append( QString("%1\%").arg(hop->getAlpha_pct(), 0, 'f', 1) );
         amounts.append( Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms) );
         uses.append( hop->getUse().c_str() );
         times.append( Brewtarget::displayAmount(hop->getTime_min(), Units::minutes) );
         ibus.append( QString("%1").arg( rec->getIBUFromHop(i), 0, 'f', 1 ) );
         //ret += (hop->getName() + "\t\t").c_str();
         //ret += QString("%1").arg(hop->getAlpha_pct(), 0, 'f', 1) + "\t\t";
         //ret += Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms) + "\t\t";
         //ret += (hop->getUse() + "\t\t").c_str();
         //ret += Brewtarget::displayAmount(hop->getTime_min(), Units::minutes) + "\t\t";
         //ret += QString("%1").arg( rec->getIBUFromHop(i), 0, 'f', 1 ) + "\n";
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&alphas);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&uses);
      padAllToMaxLength(&times);
      padAllToMaxLength(&ibus);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + alphas.at(i) + amounts.at(i) + uses.at(i) + times.at(i) + ibus.at(i) + "\n";
   }
   
   if( rec->getNumMiscs() > 0 )
   {
      QStringList names, types, uses, amounts, times;
      ret += "\n";
      ret += "Misc\n";
      ret += getTextSeparator();
      
      names.append("Name");
      types.append("Type");
      uses.append("Use");
      amounts.append("Amount");
      times.append("Time");
      
      size = rec->getNumMiscs();
      for( i = 0; i < size; ++i )
      {
         Misc* misc = rec->getMisc(i);
         names.append(misc->getName().c_str());
         types.append(misc->getType().c_str());
         uses.append(misc->getUse().c_str());
         amounts.append(Brewtarget::displayAmount(misc->getAmount(), misc->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters));
         times.append( Brewtarget::displayAmount(misc->getTime(), Units::minutes) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&uses);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&times);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + uses.at(i) + amounts.at(i) + times.at(i) + "\n";
   }
   
   if( rec->getNumYeasts() > 0 )
   {
      QStringList names, types, forms, amounts, stages;
      ret += "\n";
      ret += "Yeast\n";
      ret += getTextSeparator();
      
      names.append("Name");
      types.append("Type");
      forms.append("Form");
      amounts.append("Amount");
      stages.append("Stage");
      
      size = rec->getNumYeasts();
      for( i = 0; i < size; ++i )
      {
         Yeast* y = rec->getYeast(i);
         names.append( y->getName().c_str() );
         types.append( y->getType().c_str() );
         forms.append( y->getForm().c_str() );
         amounts.append( Brewtarget::displayAmount( y->getAmount(), y->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
         stages.append( y->getAddToSecondary() ? "Secondary" : "Primary" );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&forms);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&stages);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + forms.at(i) + amounts.at(i) + stages.at(i) + "\n";
   }
   
   if( mash && mash->getNumMashSteps() > 0 )
   {
      QStringList names, types, amounts, targets, times;
      ret += "\n";
      ret += "Mash\n";
      ret += getTextSeparator();
      
      names.append("Name");
      types.append("Type");
      amounts.append("Amount");
      targets.append("Target");
      times.append("Time");
      
      size = mash->getNumMashSteps();
      for( i = 0; i < size; ++i )
      {
         MashStep* s = mash->getMashStep(i);
         names.append(s->getName().c_str());
         types.append(s->getType().c_str());
         if( s->getType() == "Infusion" )
            amounts.append( Brewtarget::displayAmount( s->getInfuseAmount_l(), Units::liters ) );
         else if( s->getType() == "Decoction" )
            amounts.append( Brewtarget::displayAmount( s->getDecoctionAmount_l(), Units::liters ) );
         else
            amounts.append( "---" );
         targets.append( Brewtarget::displayAmount( s->getStepTemp_c(), Units::celsius ) );
         times.append( Brewtarget::displayAmount( s->getStepTime_min(), Units::minutes ) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&targets);
      padAllToMaxLength(&times);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + amounts.at(i) + targets.at(i) + times.at(i) + "\n";
   }
   
   return ret;
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

unsigned int RecipeFormatter::getMaxLength( QStringList* list )
{
   unsigned int i;
   unsigned int maxlen = 0;

   for( i = 0; i < list->count(); ++i )
   {
      if( list->at(i).size() > maxlen )
         maxlen = list->at(i).size();
   }
   
   return maxlen;
}

QString RecipeFormatter::padToLength( QString str, unsigned int length )
{
   return QString("%1").arg(str, length, ' ');
}

void RecipeFormatter::padAllToMaxLength( QStringList* list )
{
   unsigned int maxlen = getMaxLength( list );
   unsigned int i, size;
   
   size = list->count();
   for( i = 0; i < size; ++i )
      list->replace( i, padToLength( list->at(i), maxlen+1 ) ); // Add one so that text doesn't run together.
}