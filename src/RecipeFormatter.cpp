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
#include <QObject>
#include "instruction.h"

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
   
   ret += QObject::tr("Batch Size: %1\n").arg(Brewtarget::displayAmount(rec->getBatchSize_l(), Units::liters));
   ret += QObject::tr("Boil Size: %1\n").arg(Brewtarget::displayAmount(rec->getBoilSize_l(), Units::liters));
   ret += QObject::tr("Boil Time: %1\n").arg(Brewtarget::displayAmount(rec->getBoilTime_min(), Units::minutes));
   ret += QObject::tr("Efficiency: %1\%\n").arg(rec->getEfficiency_pct(), 0, 'f', 0);
   ret += QObject::tr("OG: %1\n").arg( rec->getOg(), 0, 'f', 3 );
   ret += QObject::tr("FG: %1\n").arg( rec->getFg(), 0, 'f', 3 );
   ret += QObject::tr("ABV: %1\%\n").arg( rec->getABV_pct(), 0, 'f', 1 );
   bitternessString = QObject::tr("Bitterness: %1 IBUs (%2)\n").arg( rec->getIBU(), 0, 'f', 1 );
   switch( Brewtarget::ibuFormula )
   {
      case Brewtarget::TINSETH:
         bitternessString = bitternessString.arg("Tinseth");
         break;
      case Brewtarget::RAGER:
         bitternessString = bitternessString.arg("Rager");
         break;
   }
   ret += bitternessString;
   colorString = QObject::tr("Color: %1 SRM (%2)\n").arg( rec->getColor_srm(), 0, 'f', 0 );
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
   ret += colorString;
   
   if( rec->getNumFermentables() > 0 )
   {
      QStringList names, types, amounts, masheds, lates, yields, colors;
      ret += "\n";
      ret += QObject::tr("Fermentables\n");
      ret += getTextSeparator();
      
      //ret += "Name\t\tType\t\tAmount\t\tMashed\t\tYield\t\tColor\n";
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      amounts.append(QObject::tr("Amount"));
      masheds.append(QObject::tr("Mashed"));
      lates.append(QObject::tr("Late"));
      yields.append(QObject::tr("Yield"));
      colors.append(QObject::tr("Color"));
      
      size = rec->getNumFermentables();
      for( i = 0; i < size; ++i )
      {
         Fermentable* ferm =  rec->getFermentable(i);
         names.append( ferm->getName().c_str() );
         types.append( ferm->getType().c_str() );
         amounts.append( Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms) );
         masheds.append( ferm->getIsMashed() ? QObject::tr("Yes") : QObject::tr("No") );
         lates.append( ferm->getAddAfterBoil() ? QObject::tr("Yes") : QObject::tr("No") );
         yields.append( QString("%1\%").arg(ferm->getYield_pct(), 0, 'f', 0) );
         colors.append( QString("%1 L").arg(ferm->getColor_srm(), 0, 'f', 0) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&masheds);
      padAllToMaxLength(&lates);
      padAllToMaxLength(&yields);
      padAllToMaxLength(&colors);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + amounts.at(i) + masheds.at(i) + lates.at(i) + yields.at(i) + colors.at(i) + "\n";
   }
   
   if( rec->getNumHops() > 0 )
   {
      QStringList names, alphas, amounts, uses, times, forms, ibus;
      ret += "\n";
      ret += QObject::tr("Hops\n");
      ret += getTextSeparator();
      
      //ret += "Name\t\tAlpha\t\tAmount\t\tUse\t\tTime\t\tIBU\n";
      
      names.append( QObject::tr("Name") );
      alphas.append( QObject::tr("Alpha") );
      amounts.append( QObject::tr("Amount") );
      uses.append( QObject::tr("Use") );
      times.append( QObject::tr("Time") );
      forms.append( QObject::tr("Form") );
      ibus.append( QObject::tr("IBU") );
      
      size = rec->getNumHops();
      for( i = 0; i < size; ++i )
      {
         Hop* hop = rec->getHop(i);
         
         names.append( hop->getName().c_str() );
         alphas.append( QString("%1\%").arg(hop->getAlpha_pct(), 0, 'f', 1) );
         amounts.append( Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms) );
         uses.append( hop->getUse().c_str() );
         times.append( Brewtarget::displayAmount(hop->getTime_min(), Units::minutes) );
         forms.append( hop->getForm().c_str() );
         ibus.append( QString("%1").arg( rec->getIBUFromHop(i), 0, 'f', 1 ) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&alphas);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&uses);
      padAllToMaxLength(&times);
      padAllToMaxLength(&forms);
      padAllToMaxLength(&ibus);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + alphas.at(i) + amounts.at(i) + uses.at(i) + times.at(i) + forms.at(i) + ibus.at(i) + "\n";
   }
   
   if( rec->getNumMiscs() > 0 )
   {
      QStringList names, types, uses, amounts, times;
      ret += "\n";
      ret += QObject::tr("Misc\n");
      ret += getTextSeparator();
      
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      uses.append(QObject::tr("Use"));
      amounts.append(QObject::tr("Amount"));
      times.append(QObject::tr("Time"));
      
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
      ret += QObject::tr("Yeast\n");
      ret += getTextSeparator();
      
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      forms.append(QObject::tr("Form"));
      amounts.append(QObject::tr("Amount"));
      stages.append(QObject::tr("Stage"));
      
      size = rec->getNumYeasts();
      for( i = 0; i < size; ++i )
      {
         Yeast* y = rec->getYeast(i);
         names.append( y->getName().c_str() );
         types.append( y->getType().c_str() );
         forms.append( y->getForm().c_str() );
         amounts.append( Brewtarget::displayAmount( y->getAmount(), y->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
         stages.append( y->getAddToSecondary() ? QObject::tr("Secondary") : QObject::tr("Primary") );
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
      ret += QObject::tr("Mash\n");
      ret += getTextSeparator();
      
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      amounts.append(QObject::tr("Amount"));
      targets.append(QObject::tr("Target"));
      times.append(QObject::tr("Time"));
      
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
   
   if( rec->getNotes() != "" )
   {
      ret += "\n";
      ret += QObject::tr("Notes\n") + getTextSeparator() + "\n" + rec->getNotes();
   }
   
   if( (rec->instructions).size() > 0 )
   {
      ret += "\n";
      ret += QObject::tr("Instructions\n") + getTextSeparator();
      size = (rec->instructions).size();
      Instruction* ins;
      for( i = 0; i < size; ++i )
      {
         ins = rec->instructions[i];
         ret += QString("%1) %2\n\n").arg(i).arg(ins->getDirections());
      }
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
