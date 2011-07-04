/*
* RecipeFormatter.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QPrinter>
#include <QPrintDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "instruction.h"

RecipeFormatter::RecipeFormatter()
{
   textSeparator = 0;
   rec = 0;

   //===Construct a print-preview dialog.===
   docDialog = new QDialog(Brewtarget::mainWindow);
   docDialog->setWindowTitle("Print Preview");
   if( docDialog->layout() == 0 )
      docDialog->setLayout(new QVBoxLayout(docDialog));
   doc = new QWebView(docDialog);
   docDialog->layout()->addWidget(doc);
   /*
   // Add a print button at the bottom.
   QHBoxLayout* buttonBox = new QHBoxLayout(docDialog);
   QPushButton* print = new QPushButton(QObject::tr("Print"), docDialog);
   connect(print, SLOT(clicked()), Brewtarget::mainWindow, SLOT(printRecipe()));
   buttonBox->addStretch();
   buttonBox->addWidget(print);
   docDialog->layout()->addItem(buttonBox);
   */
}

RecipeFormatter::~RecipeFormatter()
{
   delete doc;
   delete docDialog; 
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
   ret += rec->getName();
   if( style != 0 && style->getName() != "" )
      ret += (" - " + style->getName());
   ret += "\n";
   
   ret += getTextSeparator();
   
   ret += QObject::tr("Batch Size: %1\n").arg(Brewtarget::displayAmount(rec->getBatchSize_l(), Units::liters));
   ret += QObject::tr("Boil Size: %1\n").arg(Brewtarget::displayAmount(rec->getBoilSize_l(), Units::liters));
   ret += QObject::tr("Boil Time: %1\n").arg( (rec->getEquipment() == 0)?
                                              Brewtarget::displayAmount(0, Units::minutes)
                                            : Brewtarget::displayAmount( (rec->getEquipment())->getBoilTime_min(), Units::minutes));
   ret += QObject::tr("Efficiency: %1%%\n").arg(rec->getEfficiency_pct(), 0, 'f', 0);
   ret += QObject::tr("OG: %1\n").arg( Brewtarget::displayOG(rec->getOg(), true) );
   ret += QObject::tr("FG: %1\n").arg( Brewtarget::displayFG(rec->getFg(), rec->getOg(), true) );
   ret += QObject::tr("ABV: %1%%\n").arg( rec->getABV_pct(), 0, 'f', 1 );
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
         names.append( ferm->getName() );
         types.append( ferm->getTypeStringTr() );
         amounts.append( Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms) );
         masheds.append( ferm->getIsMashed() ? QObject::tr("Yes") : QObject::tr("No") );
         lates.append( ferm->getAddAfterBoil() ? QObject::tr("Yes") : QObject::tr("No") );
         yields.append( QString("%1%%").arg(ferm->getYield_pct(), 0, 'f', 0) );
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

      ret += QObject::tr("Total grain: %1\n").arg(Brewtarget::displayAmount(rec->getGrains_kg(), Units::kilograms));
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
         
         names.append( hop->getName() );
         alphas.append( QString("%1%%").arg(hop->getAlpha_pct(), 0, 'f', 1) );
         amounts.append( Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms) );
         uses.append( hop->getUseStringTr() );
         times.append( Brewtarget::displayAmount(hop->getTime_min(), Units::minutes) );
         forms.append( hop->getFormStringTr() );
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
         names.append(misc->getName());
         types.append(misc->getTypeStringTr());
         uses.append(misc->getUseStringTr());
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
         names.append( y->getName() );
         types.append( y->getTypeStringTr() );
         forms.append( y->getFormStringTr() );
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
      QStringList names, types, amounts, temps, targets, times;
      ret += "\n";
      ret += QObject::tr("Mash\n");
      ret += getTextSeparator();
      
      names.append(QObject::tr("Name"));
      types.append(QObject::tr("Type"));
      amounts.append(QObject::tr("Amount"));
      temps.append(QObject::tr("Temp"));
      targets.append(QObject::tr("Target"));
      times.append(QObject::tr("Time"));
      
      size = mash->getNumMashSteps();
      for( i = 0; i < size; ++i )
      {
         MashStep* s = mash->getMashStep(i);
         names.append(s->getName());
         types.append(s->getTypeStringTr());
         if( s->getType() == MashStep::TYPEINFUSION )
         {
            amounts.append( Brewtarget::displayAmount( s->getInfuseAmount_l(), Units::liters ) );
            temps.append( Brewtarget::displayAmount( s->getInfuseTemp_c(), Units::celsius ) );
         }
         else if( s->getType() == MashStep::TYPEDECOCTION )
         {
            amounts.append( Brewtarget::displayAmount( s->getDecoctionAmount_l(), Units::liters ) );
            temps.append("---");
         }
         else
         {
            amounts.append( "---" );
            temps.append("---");
         }
         targets.append( Brewtarget::displayAmount( s->getStepTemp_c(), Units::celsius ) );
         times.append( Brewtarget::displayAmount( s->getStepTime_min(), Units::minutes ) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&temps);
      padAllToMaxLength(&targets);
      padAllToMaxLength(&times);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + amounts.at(i) + temps.at(i) + targets.at(i) + times.at(i) + "\n";
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
   QString pDoc;

   pDoc = buildTitleTable();
   pDoc += buildFermentableTable();
   pDoc += buildHopsTable();
   pDoc += buildMiscTable();
   pDoc += buildYeastTable();
   pDoc += buildMashTable();
   pDoc += buildNotes();
   pDoc += buildInstructionTable();

   pDoc += "</body></html>";

   return pDoc;
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
   int i;
   int maxlen = 0;

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

QString RecipeFormatter::getCSS()
{
   if ( cssName == NULL )
       cssName = QString(":/css/recipe.css");

   QFile cssInput(cssName);
   QString css;

   if (cssInput.open(QFile::ReadOnly)) {
      QTextStream inStream(&cssInput);
      while ( ! inStream.atEnd() )
      {
         css += inStream.readLine();
      }
   }
   return css;
}

QString RecipeFormatter::buildTitleTable()
{
   QString header;
   QString body;
   QString color;
   QString bitterness;
   Style* style = 0;

   if ( rec == 0 )
      return "";

   style = rec->getStyle();

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";
   //body += QString("<h1>%1</h1>").arg(rec->getName()());
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"header\">");
   body += QString("<caption>%1 - %2 (%3%4)</caption>")
         .arg( rec->getName())
         .arg( style ? style->getName() : tr("unknown style"))
                   .arg( style ? style->getCategoryNumber() : tr("N/A") )
                   .arg( style ? style->getStyleLetter() : "" );

   body += QString("<tr><td class=\"label\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Brewer"))
         .arg(rec->getBrewer());
   body += QString("<tr><td class=\"label\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Date"))
         .arg(rec->getDate());
   body += "</table>";

   // Build the top table
   // Build the first row: Batch Size and Boil Size
   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Batch Size"))
           .arg(Brewtarget::displayAmount(rec->getBatchSize_l(), Units::liters));
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Boil Size"))
           .arg(Brewtarget::displayAmount(rec->getBoilSize_l(), Units::liters));
   // Second row: Boil Time and Efficiency
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Boil Time"))
           .arg( (rec->getEquipment() == 0)?
                   Brewtarget::displayAmount(0, Units::minutes)
                 : Brewtarget::displayAmount( (rec->getEquipment())->getBoilTime_min(), Units::minutes));
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2%</td></tr>")
           .arg(tr("Efficiency"))
           .arg(rec->getEfficiency_pct(), 0, 'f', 0);

   // Third row: OG and FG
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("OG"))
           .arg(Brewtarget::displayOG(rec->getOg(), true ));
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("FG"))
           .arg(Brewtarget::displayFG(rec->getFg(), rec->getOg(), true));

   // Fourth row: ABV and Bitterness.  We need to set the bitterness string up first
   bitterness = QString("%1 IBU (%2)")
         .arg( rec->getIBU(), 0, 'f', 1);
   switch ( Brewtarget::ibuFormula )
   {
      case Brewtarget::TINSETH:
         bitterness = bitterness.arg("Tinseth");
         break;
      case Brewtarget::RAGER:
         bitterness = bitterness.arg("Rager");
         break;
       default:
           bitterness = bitterness.arg(tr("Unknown"));
   }
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td>")
           .arg(tr("ABV"))
           .arg(rec->getABV_pct(), 0, 'f', 1);
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Bitterness"))
           .arg(bitterness);
   // Fifth row: Color and calories.  Set up the color string first
   color = tr("%1 SRM (%2)").arg(rec->getColor_srm(), 0, 'f', 0);
   switch( Brewtarget::colorFormula )
   {
      case Brewtarget::MOREY:
         color = color.arg("Morey");
         break;
      case Brewtarget::DANIEL:
         color = color.arg("Daniels");
         break;
      case Brewtarget::MOSHER:
         color = color.arg("Mosher");
         break;
   }
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Color"))
           .arg(color);
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Calories (per 12 oz.)"))
           .arg(rec->estimateCalories(), 0, 'f', 0);

   body += "</table>";

   return header + body;

}

QString RecipeFormatter::buildFermentableTable()
{
   QString ftable;
   if ( rec == 0 || rec->getNumFermentables() < 1 )
      return "";

   ftable = QString("<h3>%1</h3>").arg(tr("Fermentables"));
   ftable += QString("<table id=\"fermentables\">");
   ftable += QString("<caption>%1 %2</caption>")
                  .arg(tr("Total grain:"))
                  .arg(Brewtarget::displayAmount(rec->getGrains_kg(), Units::kilograms));
   // Set up the header row.
   ftable += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th><th>%6</th><th>%7</th></tr>")
         .arg(tr("Name"))
         .arg(tr("Type"))
         .arg(tr("Amount"))
         .arg(tr("Mashed"))
         .arg(tr("Late"))
         .arg(tr("Yield"))
         .arg(tr("Color"));
   // Now add a row for each fermentable
   for(unsigned int i=0; i < rec->getNumFermentables(); ++i)
   {
      Fermentable* ferm = rec->getFermentable(i);
      ftable += "<tr>";
      ftable += QString("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6%</td><td>%7 L</td>")
            .arg( ferm->getName())
            .arg( ferm->getTypeStringTr())
            .arg( Brewtarget::displayAmount(ferm->getAmount_kg(), Units::kilograms))
            .arg( ferm->getIsMashed() ? tr("Yes") : tr("No") )
            .arg( ferm->getAddAfterBoil() ? tr("Yes") : tr("No"))
            .arg( ferm->getYield_pct(), 0, 'f', 0)
            .arg( ferm->getColor_srm(), 0, 'f', 0);
      ftable += "</tr>";
   }
   ftable += "</table>";
   return ftable;
}

QString RecipeFormatter::buildHopsTable()
{
   QString hTable;

   if ( rec == 0 || rec->getNumHops() < 1 )
      return "";

   hTable = QString("<h3>%1</h3>").arg(tr("Hops"));
   hTable += QString("<table id=\"hops\">");
   // Set up the header row.
   hTable += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th><th>%6</th><th>%7</th></tr>")
         .arg(tr("Name"))
         .arg(tr("Alpha"))
         .arg(tr("Amount"))
         .arg(tr("Use"))
         .arg(tr("Time"))
         .arg(tr("Form"))
         .arg(tr("IBU"));
   for( unsigned int i = 0; i < rec->getNumHops(); ++i)
   {
      Hop *hop = rec->getHop(i);
      hTable += QString("<td>%1</td><td>%2%</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td>")
            .arg( hop->getName())
            .arg( hop->getAlpha_pct(), 0, 'f', 0)
            .arg( Brewtarget::displayAmount(hop->getAmount_kg(), Units::kilograms))
            .arg( hop->getUseStringTr())
            .arg( Brewtarget::displayAmount(hop->getTime_min(), Units::minutes) )
            .arg( hop->getFormStringTr())
            .arg( rec->getIBUFromHop(i), 0, 'f', 1);
      hTable += "</tr>";
   }
   hTable += "</table>";
   return hTable;
}

QString RecipeFormatter::buildMiscTable()
{
   QString mtable;

   if ( rec == 0 || rec->getNumMiscs() < 1 )
      return "";

   mtable = QString("<h3>%1</h3>").arg(tr("Misc"));
   mtable += QString("<table id=\"misc\">");
   // Set up the header row.
   mtable += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>")
         .arg(tr("Name"))
         .arg(tr("Type"))
         .arg(tr("Use"))
         .arg(tr("Amount"))
         .arg(tr("Time"));
   for( unsigned int i = 0; i < rec->getNumMiscs(); ++i)
   {
      Misc *misc = rec->getMisc(i);
      mtable += QString("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>")
            .arg( misc->getName())
            .arg( misc->getTypeStringTr())
            .arg( misc->getUseStringTr())
            .arg( Brewtarget::displayAmount(misc->getAmount(), misc->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters))
            .arg( Brewtarget::displayAmount(misc->getTime(), Units::minutes) );
      mtable += "</tr>";
   }
   mtable += "</table>";
   return mtable;

}

QString RecipeFormatter::buildYeastTable()
{
   QString ytable;

   if ( rec == 0 || rec->getNumYeasts() < 1 )
      return "";

   ytable = QString("<h3>%1</h3>").arg(tr("Yeast"));
   ytable += QString("<table id=\"yeast\">");
   // Set up the header row.
   ytable += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>")
         .arg(tr("Name"))
         .arg(tr("Type"))
         .arg(tr("Form"))
         .arg(tr("Amount"))
         .arg(tr("Stage"));
   for( unsigned int i = 0; i < rec->getNumYeasts(); ++i)
   {
      Yeast *y = rec->getYeast(i);
      ytable += QString("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>")
            .arg( y->getName())
            .arg( y->getTypeStringTr())
            .arg( y->getFormStringTr())
            .arg( Brewtarget::displayAmount( y->getAmount(), y->getAmountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters ) )
            .arg( y->getAddToSecondary() ? tr("Secondary") : tr("Primary"));
      ytable += "</tr>";
   }
   ytable += "</table>";
   return ytable;
}

QString RecipeFormatter::buildMashTable()
{
   QString mtable;
   Mash* m;
   MashStep* ms;
   unsigned int i;

   if( rec == 0 || rec->getMash() == 0 || rec->getMash()->getNumMashSteps() <= 0 )
      return "";

   m = rec->getMash();

   mtable = QString("<h3>%1</h3>").arg(tr("Mash"));
   mtable += "<table id=\"mash\">";

   // Header row.
   mtable += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th><th>%6</th></tr>")
             .arg( tr("Name") )
             .arg(tr("Type"))
             .arg(tr("Amount"))
             .arg(tr("Temp"))
             .arg(tr("Target Temp"))
             .arg(tr("Time"));
   for( i = 0; i < m->getNumMashSteps(); ++i )
   {
      QString tmp = "<tr>";
      ms = m->getMashStep(i);
      tmp += QString("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td>")
             .arg(ms->getName())
             .arg(ms->getTypeStringTr());

      if( ms->getType() == MashStep::TYPEINFUSION )
      {
         tmp = tmp.arg(Brewtarget::displayAmount(ms->getInfuseAmount_l(), Units::liters))
               .arg(Brewtarget::displayAmount(ms->getInfuseTemp_c(), Units::celsius));
      }
      else if( ms->getType() == MashStep::TYPEDECOCTION )
      {
         tmp = tmp.arg( Brewtarget::displayAmount( ms->getDecoctionAmount_l(), Units::liters ) )
               .arg("---");
      }
      else
         tmp = tmp.arg( "---" ).arg("---");

      tmp = tmp.arg( Brewtarget::displayAmount(ms->getStepTemp_c(), Units::celsius) );
      tmp = tmp.arg( Brewtarget::displayAmount(ms->getStepTime_min(), Units::minutes) );

      mtable += tmp + "</tr>";
   }

   mtable += "</table>";

   return mtable;
}

QString RecipeFormatter::buildNotes()
{
   QString notes;

   if ( rec == 0 || rec->getNotes() == "" )
      return "";

   notes = QString("<h3>%1</h3>").arg(tr("Notes"));
   notes += QString("%1").arg( rec->getNotes());

   return notes;
}

QString RecipeFormatter::buildInstructionTable()
{
   QString itable;

   if ( rec == 0 || rec->getNumInstructions() < 1 )
      return "";

   itable = QString("<h3>%1</h3>").arg(tr("Instructions"));
   itable += "<ol id=\"instruction\">";

   for(int i = 0; i < rec->getNumInstructions(); ++i )
   {
      Instruction* ins = rec->getInstruction(i);
      itable += QString("<li>%1</li>").arg( ins->getDirections());
   }

   itable += "</table>";

   return itable;
}

bool RecipeFormatter::loadComplete(bool ok)
{
   doc->print(printer);
   disconnect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );
   return ok;
}

void RecipeFormatter::print(QPrinter* mainPrinter, QPrintDialog *dialog, 
      int action, QFile* outFile)
{
   if( rec == 0 )
      return;

   // Short cut if we are saving to HTML
   if ( action == HTML )
   {
      QTextStream out(outFile);
      out << getHTMLFormat();
      outFile->close();
      return;
   }
   // We are printing hard copy
   if ( action == PRINT )
   {
      printer = mainPrinter;
      dialog->setWindowTitle(tr("Print Document"));
      if (dialog->exec() != QDialog::Accepted)
         return;
      connect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );
   }

   doc->setHtml(getHTMLFormat());
   if ( action == PREVIEW )
      docDialog->show();
}

