/*
 * RecipeFormatter.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RecipeFormatter.h"
#include "style.h"
#include "brewnote.h"
#include "fermentable.h"
#include "equipment.h"
#include "hop.h"
#include "instruction.h"
#include "misc.h"
#include "yeast.h"
#include "mash.h"
#include "mashstep.h"
#include "unit.h"
#include "brewtarget.h"
#include "MainWindow.h"
#include <QClipboard>
#include <QObject>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

RecipeFormatter::RecipeFormatter(QObject* parent)
   : QObject(parent)
{
   textSeparator = 0;
   rec = 0;

   //===Construct a print-preview dialog.===
   docDialog = new QDialog(Brewtarget::mainWindow());
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
   delete textSeparator;
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
   
   int i, size;
   
   Style* style = rec->style();
   Mash* mash = rec->mash();
   
   // Vital statistics.
   ret += rec->name();
   if( style != 0 && style->name() != "" )
      ret += (" - " + style->name());
   ret += "\n";
   
   ret += getTextSeparator();
   
   ret += QObject::tr("Batch Size: %1\n").arg(Brewtarget::displayAmount(rec->finalVolume_l(), Units::liters));
   ret += QObject::tr("Boil Size: %1\n").arg(Brewtarget::displayAmount(rec->boilVolume_l(), Units::liters));
   ret += QObject::tr("Boil Time: %1\n").arg( (rec->equipment() == 0)?
                                              Brewtarget::displayAmount(0, Units::minutes)
                                            : Brewtarget::displayAmount( rec->equipment()->boilTime_min(), Units::minutes));
   ret += QObject::tr("Efficiency: %1%\n").arg(rec->efficiency_pct(), 0, 'f', 0);
   ret += QObject::tr("OG: %1\n").arg( Brewtarget::displayAmount(rec->og(), Units::sp_grav, 0) );
   ret += QObject::tr("FG: %1\n").arg( Brewtarget::displayAmount(rec->fg(), Units::sp_grav, 0) );
   ret += QObject::tr("ABV: %1%\n").arg( Brewtarget::displayAmount(rec->ABV_pct(), 0, 1) );
   bitternessString = QObject::tr("Bitterness: %1 IBUs (%2)\n").arg( Brewtarget::displayAmount(rec->IBU(), 0, 1) );
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
   colorString = QObject::tr("Color: %1 SRM (%2)\n").arg( Brewtarget::displayAmount(rec->color_srm(), 0, 0) );
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
   
   QList<Fermentable*> ferms = rec->fermentables();
   size = ferms.size();
   if( size > 0 )
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
      
      for( i = 0; i < size; ++i )
      {
         Fermentable* ferm =  ferms[i];
         names.append( ferm->name() );
         types.append( ferm->typeStringTr() );
         amounts.append( Brewtarget::displayAmount(ferm->amount_kg(), Units::kilograms) );
         masheds.append( ferm->isMashed() ? QObject::tr("Yes") : QObject::tr("No") );
         lates.append( ferm->addAfterBoil() ? QObject::tr("Yes") : QObject::tr("No") );
         yields.append( QString("%1%").arg( Brewtarget::displayAmount(ferm->yield_pct(), 0, 0) ) );
         colors.append( QString("%1 L").arg( Brewtarget::displayAmount(ferm->color_srm(), 0, 0) ) );
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

      ret += QObject::tr("Total grain: %1\n").arg(Brewtarget::displayAmount(rec->grains_kg(), Units::kilograms));
   }
   
   QList<Hop*> hops = rec->hops();
   QList<double> ibuList = rec->IBUs();
   size = hops.size();
   if( size > 0 )
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
      
      for( i = 0; i < size; ++i )
      {
         Hop* hop = hops[i];
         
         names.append( hop->name() );
         alphas.append( QString("%1%").arg(Brewtarget::displayAmount(hop->alpha_pct(), 0, 1) ) );
         amounts.append( Brewtarget::displayAmount(hop->amount_kg(), Units::kilograms) );
         uses.append( hop->useStringTr() );
         times.append( Brewtarget::displayAmount(hop->time_min(), Units::minutes) );
         forms.append( hop->formStringTr() );
         ibus.append( QString("%1").arg( Brewtarget::displayAmount(ibuList[i], 0, 1) ) );
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
   
   QList<Misc*> miscs = rec->miscs();
   size = miscs.size();
   if( size > 0 )
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
      
      for( i = 0; i < size; ++i )
      {
         Misc* misc = miscs[i];
         names.append(misc->name());
         types.append(misc->typeStringTr());
         uses.append(misc->useStringTr());
         amounts.append(Brewtarget::displayAmount(misc->amount(), misc->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters));
         times.append( Brewtarget::displayAmount(misc->time(), Units::minutes) );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&uses);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&times);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + uses.at(i) + amounts.at(i) + times.at(i) + "\n";
   }
   
   QList<Yeast*> yeasts = rec->yeasts();
   size = yeasts.size();
   if( size > 0 )
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
      
      for( i = 0; i < size; ++i )
      {
         Yeast* y = yeasts[i];
         names.append( y->name() );
         types.append( y->typeStringTr() );
         forms.append( y->formStringTr() );
         amounts.append( Brewtarget::displayAmount( y->amount(), y->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters ) );
         stages.append( y->addToSecondary() ? QObject::tr("Secondary") : QObject::tr("Primary") );
      }
      
      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&forms);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&stages);
      
      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + forms.at(i) + amounts.at(i) + stages.at(i) + "\n";
   }
   
   QList<MashStep*> mashSteps;
   if( mash )
      mashSteps = mash->mashSteps();
   size = mashSteps.size();
   if( size > 0 )
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
      
      for( i = 0; i < size; ++i )
      {
         MashStep* s = mashSteps[i];
         names.append(s->name());
         types.append(s->typeStringTr());
         if( s->type() == MashStep::Infusion )
         {
            amounts.append( Brewtarget::displayAmount( s->infuseAmount_l(), Units::liters ) );
            temps.append( Brewtarget::displayAmount( s->infuseTemp_c(), Units::celsius ) );
         }
         else if( s->type() == MashStep::Decoction )
         {
            amounts.append( Brewtarget::displayAmount( s->decoctionAmount_l(), Units::liters ) );
            temps.append("---");
         }
         else
         {
            amounts.append( "---" );
            temps.append("---");
         }
         targets.append( Brewtarget::displayAmount( s->stepTemp_c(), Units::celsius ) );
         times.append( Brewtarget::displayAmount( s->stepTime_min(), Units::minutes ) );
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
   
   if( rec->notes() != "" )
   {
      ret += "\n";
      ret += QObject::tr("Notes\n") + getTextSeparator() + "\n" + rec->notes();
   }
   
   QList<Instruction*> instructions = rec->instructions();
   size = instructions.size();
   if( size > 0 )
   {
      ret += "\n";
      ret += QObject::tr("Instructions\n") + getTextSeparator();
      Instruction* ins;
      for( i = 0; i < size; ++i )
      {
         ins = instructions[i];
         ret += QString("%1) %2\n\n").arg(i).arg(ins->directions());
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
   pDoc += buildBrewNotes();

   pDoc += "</div></body></html>";

   return pDoc;
}

QString RecipeFormatter::getBBCodeFormat()
{
   return "";
}

QString RecipeFormatter::getToolTip(Recipe* rec)
{
   QString header;
   QString body;

   Style* style = 0;

   if ( rec == 0 )
      return "";

   cssName = QString(":/css/tooltip.css");
   style = rec->style();

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";
   //body += QString("<h1>%1</h1>").arg(rec->getName()());
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1 (%2%3)</caption>")
         .arg( style ? style->name() : tr("unknown style"))
         .arg( style ? style->categoryNumber() : tr("N/A") )
         .arg( style ? style->styleLetter() : "" );

   // Third row: OG and FG
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("OG"))
           .arg(Brewtarget::displayAmount(rec->og(), Units::sp_grav, 0 ));
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("FG"))
           .arg(Brewtarget::displayAmount(rec->fg(), Units::sp_grav, 0));

   // Fourth row: Color and Bitterness.  
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td>")
           .arg(tr("Color"))
           .arg(Brewtarget::displayAmount(rec->color_srm(),Units::srm,0))
           .arg(Brewtarget::colorFormulaName());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td></tr>")
           .arg(tr("IBU"))
           .arg(Brewtarget::displayAmount(rec->IBU(), 0, 1))
           .arg(Brewtarget::ibuFormulaName() );

   body += "</table></body></html>";

   return header + body;

}

QString RecipeFormatter::getToolTip(Style* style)
{
   QString header;
   QString body;

   if ( style == 0 )
      return "";

   cssName = QString(":/css/tooltip.css");

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( style->name() );

   // First row -- category and number (letter)
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Category"))
           .arg(style->category());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2%3</td></tr>")
           .arg(tr("Code"))
           .arg(style->categoryNumber())
           .arg(style->styleLetter());

   // Second row: guide and type
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Guide"))
           .arg(style->styleGuide());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Type"))
           .arg(style->typeString());

   body += "</table></body></html>";

   return header + body;

}

QString RecipeFormatter::getToolTip(Equipment* kit)
{
   QString header;
   QString body;

   if ( kit == 0 )
      return "";

   cssName = QString(":/css/tooltip.css");

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( kit->name() );

   // First row -- batchsize and boil time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Preboil"))
           .arg(Brewtarget::displayAmount(kit->boilSize_l(), Units::liters) );
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("BoilTime"))
           .arg(Brewtarget::displayAmount(kit->boilTime_min(), Units::minutes) );

   body += "</table></body></html>";

   return header + body;

}

// Once we do inventory, this needs to be fixed to show amount on hand
QString RecipeFormatter::getToolTip(Fermentable* ferm)
{
   QString header;
   QString body;

   if ( ferm == 0 )
      return "";

   cssName = QString(":/css/tooltip.css");

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( ferm->name() );

   // First row -- type and color
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Type"))
           .arg(ferm->typeStringTr());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Color"))
           .arg(Brewtarget::displayAmount(ferm->color_srm(), Units::srm, 0));
   // Second row -- isMashed and yield?
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Mashed"))
           .arg( ferm->isMashed() ? tr("Yes") : tr("No") );
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Yield"))
           .arg(Brewtarget::displayAmount(ferm->yield_pct(), 0));

   body += "</table></body></html>";

   return header + body;

}

QString RecipeFormatter::getToolTip(Hop* hop)
{
   QString header;
   QString body;

   if ( hop == 0 )
      return "";

   cssName = QString(":/css/tooltip.css");

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( hop->name() );

   // First row -- alpha and beta
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Alpha"))
           .arg(Brewtarget::displayAmount(hop->alpha_pct(), 0));
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Beta"))
           .arg(Brewtarget::displayAmount(hop->beta_pct(), 0));

   // Second row -- form and use
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Form"))
           .arg( hop->formStringTr() );
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Use"))
           .arg( hop->useStringTr() );

   body += "</table></body></html>";

   return header + body;

}

QString RecipeFormatter::getToolTip(Misc* misc)
{
   QString header;
   QString body;

   if ( misc == 0 )
      return "";

   cssName = QString(":/css/tooltip.css");

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( misc->name() );

   // First row -- type and use
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Type"))
           .arg(misc->typeStringTr());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Use"))
           .arg(misc->useStringTr());

   body += "</table></body></html>";

   return header + body;

}

QString RecipeFormatter::getToolTip(Yeast* yeast)
{
   QString header;
   QString body;

   if ( yeast == 0 )
      return "";

   cssName = QString(":/css/tooltip.css");

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( yeast->name() );

   // First row -- type and form
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Type"))
           .arg(yeast->typeStringTr());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Form"))
           .arg(yeast->formStringTr());
   // Second row -- lab and prod id
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Lab"))
           .arg(yeast->laboratory());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Attenuation"))
           .arg(Brewtarget::displayAmount(yeast->attenuation_pct(), 0));

   // third row -- atten and floc
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Id"))
           .arg(yeast->productID());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Flocculation"))
           .arg( yeast->flocculationStringTr());


   body += "</table></body></html>";

   return header + body;

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
   Style* style = 0;

   if ( rec == 0 )
      return "";

   style = rec->style();

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";
   //body += QString("<h1>%1</h1>").arg(rec->getName()());
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"header\">");
   body += QString("<caption>%1 - %2 (%3%4)</caption>")
         .arg( rec->name())
         .arg( style ? style->name() : tr("unknown style"))
         .arg( style ? style->categoryNumber() : tr("N/A") )
         .arg( style ? style->styleLetter() : "" );

   body += QString("<tr><td class=\"label\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Brewer"))
           .arg(rec->brewer());
   body += QString("<tr><td class=\"label\">%1</td><td class=\"value \">%2</td></tr>")
           .arg(tr("Date"))
           .arg(Brewtarget::displayDate(rec->date()));
   body += "</table>";

   // Build the top table
   // Build the first row: Batch Size and Boil Size.
   // NOTE: using getBatchSize_l() and/or getBoilSize_l() only gives the
   // *target* batch and boil size.  I think we want the actual (aka,
   // estimated) sizes

   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Batch Size"))
           .arg(Brewtarget::displayAmount(rec->finalVolume_l(), "tab_recipe", "finalVolume_l", Units::liters));
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Boil Size"))
           .arg(Brewtarget::displayAmount(rec->boilVolume_l(), "tab_recipe", "boilVolume_l", Units::liters));
   // Second row: Boil Time and Efficiency
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Boil Time"))
           .arg( (rec->equipment() == 0)?
                   Brewtarget::displayAmount(0, Units::minutes)
                 : Brewtarget::displayAmount( (rec->equipment())->boilTime_min(), Units::minutes));
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2%</td></tr>")
           .arg(tr("Efficiency"))
           .arg(rec->efficiency_pct(), 0, 'f', 0);

   // Third row: OG and FG
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("OG"))
           .arg(Brewtarget::displayAmount(rec->og(), "tab_recipe", "og", Units::sp_grav, 0 ));
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("FG"))
           .arg(Brewtarget::displayAmount(rec->fg(), "tab_recipe", "fg", Units::sp_grav, 0));

   // Fourth row: ABV and Bitterness.  We need to set the bitterness string up first
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td>")
           .arg(tr("ABV"))
           .arg(Brewtarget::displayAmount(rec->ABV_pct(), 0, 1));
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2 (%3)</td></tr>")
           .arg(tr("IBU"))
           .arg(Brewtarget::displayAmount(rec->IBU(), 0, 1))
           .arg(Brewtarget::ibuFormulaName() );

   // Fifth row: Color and calories.  Set up the color string first
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td>")
           .arg(tr("Color"))
           .arg(Brewtarget::displayAmount(rec->color_srm(),"tab_recipe", "color_srm", Units::srm,0))
           .arg(Brewtarget::colorFormulaName());
           
   body += QString("<td class=\"right\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Calories (per 12 oz.)"))
           .arg(Brewtarget::displayAmount(rec->calories(), 0, 0));

   body += "</table>";

   return header + body;

}

QString RecipeFormatter::buildFermentableTable()
{
   if( rec == 0 )
      return "";
   
   QString ftable;
   QList<Fermentable*> ferms = sortFermentablesByWeight(rec);
   int i, size;

   size = ferms.size();
   if ( size < 1 )
      return "";

   ftable = QString("<h3>%1</h3>").arg(tr("Fermentables"));
   ftable += QString("<table id=\"fermentables\">");
   ftable += QString("<caption>%1 %2</caption>")
                  .arg(tr("Total grain:"))
                  .arg(Brewtarget::displayAmount(rec->grains_kg(), "fermentableTable", "amount_kg", Units::kilograms));
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
   for(i=0; i < size; ++i)
   {
      Fermentable* ferm = ferms[i];
      ftable += "<tr>";
      ftable += QString("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6%</td><td>%7</td>")
            .arg( ferm->name())
            .arg( ferm->typeStringTr())
            .arg( Brewtarget::displayAmount(ferm->amount_kg(), "fermentableTable", "amount_kg", Units::kilograms))
            .arg( ferm->isMashed() ? tr("Yes") : tr("No") )
            .arg( ferm->addAfterBoil() ? tr("Yes") : tr("No"))
            .arg( Brewtarget::displayAmount(ferm->yield_pct(), 0, 0) )
            .arg( Brewtarget::displayAmount(ferm->color_srm(), "fermentableTable", "color_srm", Units::srm,0));
      ftable += "</tr>";
   }
   ftable += "</table>";
   return ftable;
}

QString RecipeFormatter::buildHopsTable()
{
   if( rec == 0 )
      return "";
   
   QString hTable;
   QList<Hop*> hops = sortHopsByTime(rec);
   int i, size;

   size = hops.size();
   if ( size < 1 )
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
   for( i = 0; i < size; ++i)
   {
      Hop *hop = hops[i];
      hTable += QString("<tr><td>%1</td><td>%2%</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
            .arg( hop->name())
            .arg( Brewtarget::displayAmount(hop->alpha_pct(),0,1) )
            .arg( Brewtarget::displayAmount(hop->amount_kg(), "hopTable", "amount_kg", Units::kilograms))
            .arg( hop->useStringTr())
            .arg( Brewtarget::displayAmount(hop->time_min(), Units::minutes) )
            .arg( hop->formStringTr())
            .arg( Brewtarget::displayAmount(rec->ibuFromHop(hop), 0, 1) );
      hTable += "</tr>";
   }
   hTable += "</table>";
   return hTable;
}

QString RecipeFormatter::buildMiscTable()
{
   if( rec == 0 )
      return "";
   
   QString mtable;
   int i, size;
   QList<Misc*> miscs = rec->miscs();
   size = miscs.size();
   Unit* kindOf;

   if ( size < 1 )
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
   for( i = 0; i < size; ++i)
   {
      Misc *misc = miscs[i];
      kindOf = misc->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters;

      mtable += QString("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td>")
            .arg( misc->name())
            .arg( misc->typeStringTr())
            .arg( misc->useStringTr())
            .arg( Brewtarget::displayAmount(misc->amount(), "miscTableModel", "amount_kg", kindOf, 3))
            .arg( Brewtarget::displayAmount(misc->time(), Units::minutes) );
      mtable += "</tr>";
   }
   mtable += "</table>";
   return mtable;

}

QString RecipeFormatter::buildYeastTable()
{
   if( rec == 0 )
      return "";
   
   QString ytable;
   int i, size;
   QList<Yeast*> yeasts = rec->yeasts();
   Unit* kindOf;
   size = yeasts.size();
   
   if( size < 1 )
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
   for( i = 0; i < size; ++i)
   {
      Yeast* y = yeasts[i];
      kindOf = y->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters;

      ytable += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
            .arg( y->name())
            .arg( y->typeStringTr())
            .arg( y->formStringTr())
            .arg( Brewtarget::displayAmount( y->amount(), "yeastTableModel", "amount_kg", kindOf, 0) )
            .arg( y->addToSecondary() ? tr("Secondary") : tr("Primary"));
      ytable += "</tr>";
   }
   ytable += "</table>";
   return ytable;
}

QString RecipeFormatter::buildMashTable()
{
   if( rec == 0 || rec->mash() == 0 )
      return "";
      
   QString mtable;
   
   MashStep* ms;
   int i, size;
   Mash* m = rec->mash();
   QList<MashStep*> mashSteps = m->mashSteps();
   size = mashSteps.size();
   
   if( size <= 0 )
      return "";

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
   for( i = 0; i < size; ++i )
   {
      QString tmp = "<tr>";
      ms = mashSteps[i];
      tmp += QString("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td>")
             .arg(ms->name())
             .arg(ms->typeStringTr());

      if( ms->type() == MashStep::Infusion )
      {
         tmp = tmp.arg(Brewtarget::displayAmount(ms->infuseAmount_l(), "mashStepTableModel", "amount", Units::liters))
                  .arg(Brewtarget::displayAmount(ms->infuseTemp_c(),   "mashStepTableModel", "infuseTemp_c", Units::celsius));
      }
      else if( ms->type() == MashStep::Decoction )
      {
         tmp = tmp.arg( Brewtarget::displayAmount( ms->decoctionAmount_l(), "mashStepTableModel", "amount", Units::liters ) )
               .arg("---");
      }
      else
         tmp = tmp.arg( "---" ).arg("---");

      tmp = tmp.arg( Brewtarget::displayAmount(ms->stepTemp_c(), "mashStepTableModel", "stepTemp_c", Units::celsius) );
      tmp = tmp.arg( Brewtarget::displayAmount(ms->stepTime_min(), "mashStepTableModel", "time", Units::minutes, 0) );

      mtable += tmp + "</tr>";
   }

   mtable += "</table>";

   return mtable;
}

QString RecipeFormatter::buildNotes()
{
   QString notes;

   if ( rec == 0 || rec->notes() == "" )
      return "";

   notes = QString("<h3>%1</h3>").arg(tr("Notes"));
   notes += QString("%1").arg( QTextDocument(rec->notes()).toHtml());

   return notes;
}

QString RecipeFormatter::buildInstructionTable()
{
   if( rec == 0 )
      return "";
   
   QString itable;
   int i, size;
   QList<Instruction*> instructions = rec->instructions();
   size = instructions.size();
   
   if ( size < 1 )
      return "";

   itable = QString("<h3>%1</h3>").arg(tr("Instructions"));
   itable += "<ol id=\"instruction\">";

   for( i = 0; i < size; ++i )
   {
      Instruction* ins = instructions[i];
      itable += QString("<li>%1</li>").arg( ins->directions());
   }

   itable += "</ol>";

   return itable;
}

QString RecipeFormatter::buildBrewNotes()
{
   if( rec == 0 )
      return "";
   
   QString bnTable = "";
   int i, size;
   QList<BrewNote*> brewNotes = rec->brewNotes();
   size = brewNotes.size();
   
   if ( size < 1 )
      return bnTable;

   for( i = 0; i < size; ++i )
   {
      BrewNote* note = brewNotes[i];
      QString section;

      bnTable += QString("<h2>%1 %2</h2>").arg(tr("Brew Date")).arg(note->brewDate_short());
      
      // PREBOIL, done two-by-two
      section = "page_preboil";
      bnTable += "<table id=\"brewnote\">";
      bnTable += QString("<caption>%1</caption>").arg(tr("Preboil"));
      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                 .arg(tr("SG"))
                 .arg(Brewtarget::displayAmount(note->sg(), section, "sg", Units::sp_grav,0))
                 .arg(tr("Volume into BK"))
                 .arg(Brewtarget::displayAmount(note->volumeIntoBK_l(), section, "volumeIntoBK_l", Units::liters));

      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                 .arg(tr("Strike Temp"))
                 .arg(Brewtarget::displayAmount(note->strikeTemp_c(), section, "strikeTemp_c", Units::celsius))
                 .arg(tr("Final Temp"))
                 .arg(Brewtarget::displayAmount(note->mashFinTemp_c(), section, "mashFinTemp_c", Units::celsius));

      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                 .arg(tr("Eff into BK"))
                 .arg(Brewtarget::displayAmount(note->calculateEffIntoBK_pct(), 0, 2))
                 .arg(tr("Projected OG"))
                 .arg(Brewtarget::displayAmount(note->calculateOg(), section, "projOg", Units::sp_grav, 0));
      bnTable += "</table>";

      // POSTBOIL
      section = "page_postboil";
      bnTable += "<table id=\"brewnote\">";
      bnTable += QString("<caption>%1</caption>").arg(tr("Postboil"));
      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                 .arg(tr("OG"))
                 .arg(Brewtarget::displayAmount(note->og(),section, "og", Units::sp_grav,0))
                 .arg(tr("Postboil Volume"))
                 .arg(Brewtarget::displayAmount(note->postBoilVolume_l(), section, "postBoilVolume_l", Units::liters));
      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                 .arg(tr("Volume Into Fermenter"))
                 .arg(Brewtarget::displayAmount(note->volumeIntoFerm_l(), section, "volumeIntoFerm_l", Units::liters))
                 .arg(tr("Brewhouse Eff"))
                 .arg(Brewtarget::displayAmount(note->calculateBrewHouseEff_pct(), 0, 2));
      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td></tr>")
                 .arg(tr("Projected ABV"))
                 .arg(Brewtarget::displayAmount(note->calculateABV_pct(), 0, 2));
      bnTable += "</table>";


      // POSTFERMENT
      section = "page_postferment";
      bnTable += "<table id=\"brewnote\">";
      bnTable += QString("<caption>%1</caption>").arg(tr("Postferment"));
      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                 .arg(tr("FG"))
                 .arg(Brewtarget::displayAmount(note->fg(),section,"fg",Units::sp_grav,0))
                 .arg(tr("Volume"))
                 .arg(Brewtarget::displayAmount(note->finalVolume_l(), section, "finalVolume_l", Units::liters));

      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                 .arg(tr("Date"))
                 .arg(note->fermentDate_short())
                 .arg(tr("ABV"))
                 .arg(Brewtarget::displayAmount(note->calculateActualABV_pct(), 0, 2));
      bnTable += "</table>";

   }

   return bnTable;
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

QList<Hop*> RecipeFormatter::sortHopsByTime(Recipe* rec)
{
   QList<Hop*> sorted = rec->hops();
   
   qSort(sorted.begin(), sorted.end(), hopLessThanByTime);
   return sorted;
}

QList<Fermentable*> RecipeFormatter::sortFermentablesByWeight(Recipe* rec)
{
   QList<Fermentable*> sorted = rec->fermentables();
   
   qSort(sorted.begin(), sorted.end(), fermentablesLessThanByWeight);
   return sorted;
}
