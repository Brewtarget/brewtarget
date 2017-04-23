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
#include "Html.h"
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
      docDialog->setLayout(new QVBoxLayout);
   doc = new QTextBrowser(docDialog);
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
   QString tmp = "";
   if( rec == 0 )
      return "";
   
   Style* style = rec->style();
   
   ret += QString("%1 - %2 (%3%4)\n").arg( rec->name())
         .arg( style ? style->name() : tr("unknown style"))
         .arg( style ? style->categoryNumber() : tr("N/A"))
         .arg( style ? style->styleLetter() : "");
   ret += getTextSeparator();
   ret += buildStatTableTxt();
   if((tmp = buildFermentableTableTxt()) != "")
   {
      ret += "\n" + tr("Fermentables") + "\n";
      ret += getTextSeparator();
      ret += tmp;
   }
   if((tmp = buildHopsTableTxt()) != "")
   {
      ret += "\n" + tr("Hops") + "\n";
      ret += getTextSeparator();
      ret += tmp;
   }
   if((tmp = buildMiscTableTxt()) != "")
   {
      ret += "\n" + tr("Miscs") + "\n";
      ret += getTextSeparator();
      ret += tmp;
   }
   if((tmp = buildYeastTableTxt()) != "")
   {
      ret += "\n" + tr("Yeasts") + "\n";
      ret += getTextSeparator();
      ret += tmp;
   }
   if((tmp = buildMashTableTxt()) != "")
   {
      ret += "\n" + tr("Mash") + "\n";
      ret += getTextSeparator();
      ret += tmp;
   }
   if( (tmp = rec->notes()) != "" )
   {
      ret += "\n" + tr("Notes") + "\n";
      ret += getTextSeparator();
      ret += tmp;
   }
   if((tmp = buildInstructionTableTxt()) != "")
   {
      ret += "\n" + tr("Instructions") + "\n";
      ret += getTextSeparator();
      ret += tmp;
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

QString RecipeFormatter::buildHTMLHeader()
{
   return Html::createHeader(RecipeFormatter::tr("Recipe"), ":css/recipe.css");
}

QString RecipeFormatter::buildHTMLFooter() {
    return "</div></body></html>";
}

QString RecipeFormatter::getHTMLFormat( QList<Recipe*> recipes ) {
   Recipe *current = rec;
   QString hDoc;

   hDoc = buildHTMLHeader();

   // build a toc -- why do I do this to myself?
   hDoc += "<ul>";
   foreach ( Recipe* foo, recipes ) {
       hDoc += QString("<li><a href=\"#%1\">%1</a></li>").arg(foo->name());
   }
   hDoc += "</ul>";

   foreach (Recipe* foo, recipes) {
      rec = foo;
      hDoc += QString("<a name=\"%1\"></a>").arg(foo->name());
      hDoc += buildStatTableHtml();
      hDoc += buildFermentableTableHtml();
      hDoc += buildHopsTableHtml();
      hDoc += buildMiscTableHtml();
      hDoc += buildYeastTableHtml();
      hDoc += buildMashTableHtml();
      hDoc += buildNotesHtml();
      hDoc += buildInstructionTableHtml();
      hDoc += buildBrewNotesHtml();
      hDoc += "<p></p>";
   }
   hDoc += buildHTMLFooter();

   rec = current;
   return hDoc;
}

QString RecipeFormatter::getHTMLFormat()
{
   QString pDoc;

   pDoc = buildHTMLHeader();
   pDoc += buildStatTableHtml();
   pDoc += buildFermentableTableHtml();
   pDoc += buildHopsTableHtml();
   pDoc += buildMiscTableHtml();
   pDoc += buildYeastTableHtml();
   pDoc += buildMashTableHtml();
   pDoc += buildNotesHtml();
   pDoc += buildInstructionTableHtml();
   pDoc += buildBrewNotesHtml();

   pDoc += buildHTMLFooter();

   return pDoc;
}

QString RecipeFormatter::getBBCodeFormat()
{
   QString ret = "";
   QString tmp = "";
   QRegExp regexp("(^[^\n]*\n)(.*$)"); //Regexp to match the first line of tables

   if( rec == 0 )
      return "";

   Style* style = rec->style();

   ret += "[size=150][color=#004080][b][u]";
   ret += QString("%1 - %2 (%3%4)").arg( rec->name())
         .arg( style ? style->name() : tr("unknown style"))
         .arg( style ? style->categoryNumber() : tr("N/A"))
         .arg( style ? style->styleLetter() : "");
   ret += "[/b][/u][/color][/size]\n\n";
   ret += "[pre]" + buildStatTableTxt() + "[/pre]";
   if((tmp = buildFermentableTableTxt()) != "")
   {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Fermentables") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if((tmp = buildHopsTableTxt()) != "")
   {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Hops") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if((tmp = buildMiscTableTxt()) != "")
   {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Miscs") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if((tmp = buildYeastTableTxt()) != "")
   {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Yeasts") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if((tmp = buildMashTableTxt()) != "")
   {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Mash") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if( (tmp = rec->notes()) != "" )
   {
      ret += "\n[color=#004080][b]" + tr("Notes") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if((tmp = buildInstructionTableTxt()) != "")
   {
      ret += "\n[color=#004080][b]" + tr("Instructions") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }

   return ret;
}

QString RecipeFormatter::getToolTip(Recipe* rec)
{
   QString header;
   QString body;

   Style* style = 0;

   if ( rec == 0 )
      return "";

   style = rec->style();

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
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
           .arg(Brewtarget::displayAmount(rec->og(), Units::sp_grav, 3));
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("FG"))
           .arg(Brewtarget::displayAmount(rec->fg(), Units::sp_grav, 3));

   // Fourth row: Color and Bitterness.  
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td>")
           .arg(tr("Color"))
           .arg(Brewtarget::displayAmount(rec->color_srm(),Units::srm, 1))
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

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
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

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
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

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
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
           .arg(Brewtarget::displayAmount(ferm->color_srm(), Units::srm, 1));
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

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
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

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
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

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
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

QString RecipeFormatter::padToLength( const QString &str, unsigned int length )
{
   return QString("%1").arg(str, -length, ' ');
}

void RecipeFormatter::padAllToMaxLength( QStringList* list, unsigned int padding )
{
   unsigned int maxlen = getMaxLength( list );
   unsigned int i, size;
   
   size = list->count();
   for( i = 0; i < size; ++i )
      list->replace( i, padToLength( list->at(i), maxlen + padding ) ); // Add a padding so that text doesn't run together.
}

QString RecipeFormatter::wrapText( const QString &text, int wrapLength )
{
   QStringList line = text.split("\n");
   QString wrappedText("");

   int nbLines = line.size();
   for (int i = 0; i < nbLines; ++i)
   {
       QString currentLine = line.at(i);
       int len = currentLine.length();
       int pos = (wrapLength > len-1) ? len-1 : wrapLength;

       while( pos < len-1 )
       {
          int splitPos = pos;
          while( currentLine.at(splitPos) != ' ' && splitPos > 0 )
             splitPos--;
          if ( currentLine.at(splitPos) == ' ' ) //String without whitespace won't be splited
          {
             currentLine.replace(splitPos, 1, '\n');
          }
          else //If the first part of the string doesn't contain a whitspace, look for the next one to split the line
          {
             splitPos = pos;
             while( currentLine.at(splitPos) != ' ' && splitPos < len-1 )
                splitPos++;
             if ( currentLine.at(splitPos) == ' ' )
             {
                currentLine.replace(splitPos, 1, '\n');
             }
          }
          pos = splitPos + wrapLength;
       }

       wrappedText += currentLine;
       if (i < nbLines - 1)
          wrappedText += "\n";
   }
   return wrappedText;
}

QString RecipeFormatter::buildStatTableHtml()
{
   QString header;
   QString body;
   Style* style = 0;

   if ( rec == 0 )
      return "";

   style = rec->style();

   body += QString("<div id=\"headerdiv\">");
   // NOTE: QTextBrowser does not support the caption tag
   body += QString("<h1>%1 - %2 (%3%4)</h1>")
         .arg( rec->name())
         .arg( style ? style->name() : tr("unknown style"))
         .arg( style ? style->categoryNumber() : tr("N/A") )
         .arg( style ? style->styleLetter() : "" );

   body += QString("<table id=\"header\">");
   body += QString("<tr>"
                   "<td class=\"label\">%1</td>"
                   "<td class=\"value\">%2</td>"
                   "</tr>")
           .arg(tr("Brewer"))
           .arg(rec->brewer());
   body += QString("<tr>"
                   "<td class=\"label\">%1</td>"
                   "<td class=\"value \">%2</td>"
                   "</tr>")
           .arg(tr("Date"))
           .arg(Brewtarget::displayDate(rec->date()));
   body += "</table>";

   // Build the top table
   // Build the first row: Batch Size and Boil Size.
   // NOTE: using getBatchSize_l() and/or getBoilSize_l() only gives the
   // *target* batch and boil size.  I think we want the actual (aka,
   // estimated) sizes

   body += "<table id=\"title\">";
   body += QString("<tr>"
                   "<td align=\"left\" class=\"left\">%1</td>"
                   "<td width=\"20%\" class=\"value\">%2</td>")
           .arg(tr("Batch Size"))
           .arg(Brewtarget::displayAmount(rec->finalVolume_l(), "tab_recipe", "finalVolume_l", Units::liters));
   body += QString("<td width=\"40%\" align=\"right\" class=\"right\">%1</td>"
                   "<td class=\"value\">%2</td>"
                   "</tr>")
           .arg(tr("Boil Size"))
           .arg(Brewtarget::displayAmount(rec->boilVolume_l(), "tab_recipe", "boilVolume_l", Units::liters));
   // Second row: Boil Time and Efficiency
   body += QString("<tr>"
                   "<td align=\"left\" class=\"left\">%1</td>"
                   "<td class=\"value\">%2</td>")
           .arg(tr("Boil Time"))
           .arg( (rec->equipment() == 0)?
                   Brewtarget::displayAmount(0, "tab_recipe", "boilTime_min", Units::minutes)
                 : Brewtarget::displayAmount( (rec->equipment())->boilTime_min(), "tab_recipe", "boilTime_min", Units::minutes));
   body += QString("<td align=\"right\" class=\"right\">%1</td>"
                   "<td class=\"value\">%2</td></tr>")
           .arg(tr("Efficiency"))
           .arg(rec->efficiency_pct(), 0, 'f', 0);

   // Third row: OG and FG
   body += QString("<tr>"
                   "<td align=\"left\" class=\"left\">%1</td>"
                   "<td class=\"value\">%2</td>")
           .arg(tr("OG"))
           .arg(Brewtarget::displayAmount(rec->og(), "tab_recipe", "og", Units::sp_grav, 3));
   body += QString("<td align=\"right\" class=\"right\">%1</td>"
                   "<td class=\"value\">%2</td></tr>")
           .arg(tr("FG"))
           .arg(Brewtarget::displayAmount(rec->fg(), "tab_recipe", "fg", Units::sp_grav, 3));

   // Fourth row: ABV and Bitterness.  We need to set the bitterness string up first
   body += QString("<tr>"
                   "<td align=\"left\" class=\"left\">%1</td>"
                   "<td class=\"value\">%2%</td>")
           .arg(tr("ABV"))
           .arg(Brewtarget::displayAmount(rec->ABV_pct(), 0, 1));
   body += QString("<td align=\"right\" class=\"right\">%1</td>"
                   "<td class=\"value\">%2 (%3)</td></tr>")
           .arg(tr("IBU"))
           .arg(Brewtarget::displayAmount(rec->IBU(), 0, 1))
           .arg(Brewtarget::ibuFormulaName() );

   // Fifth row: Color and calories.  Set up the color string first
   body += QString("<tr>"
                   "<td align=\"left\" class=\"left\">%1</td>"
                   "<td class=\"value\">%2 (%3)</td>")
           .arg(tr("Color"))
           .arg(Brewtarget::displayAmount(rec->color_srm(),"tab_recipe", "color_srm", Units::srm, 1))
           .arg(Brewtarget::colorFormulaName());
           
   body += QString("<td align=\"right\" class=\"right\">%1</td>"
                   "<td class=\"value\">%2</td></tr>")
           .arg( Brewtarget::getVolumeUnitSystem() == SI ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
           .arg( Brewtarget::displayAmount(Brewtarget::getVolumeUnitSystem() == SI ? rec->calories33cl() : rec->calories12oz(),0,0) );

   body += "</table>";

   return header + body;

}

QString RecipeFormatter::buildStatTableTxt()
{
   QString ret = "";
   const int nbLines = 9;
   int i;

   if( rec == 0 )
      return "";

   QStringList entry, value;

   entry.append(tr("Batch Size"));
   value.append(QString("%1").arg(Brewtarget::displayAmount(rec->finalVolume_l(), "tab_recipe", "finalVolume_l", Units::liters)));
   entry.append(tr("Boil Size"));
   value.append(QString("%1").arg(Brewtarget::displayAmount(rec->boilVolume_l(), "tab_recipe", "boilVolume_l", Units::liters)));
   entry.append(tr("Boil Time"));
   value.append(QString("%1").arg((rec->equipment() == 0)?
                         Brewtarget::displayAmount(0, "tab_recipe", "boilTime_min", Units::minutes)
                       : Brewtarget::displayAmount( (rec->equipment())->boilTime_min(), "tab_recipe", "boilTime_min", Units::minutes)));
   entry.append(tr("Efficiency"));
   value.append(QString("%1%").arg(rec->efficiency_pct(), 0, 'f', 0));
   entry.append(tr("OG"));
   value.append(QString("%1").arg(Brewtarget::displayAmount(rec->og(), "tab_recipe", "og", Units::sp_grav, 3)));
   entry.append(tr("FG"));
   value.append(QString("%1").arg(Brewtarget::displayAmount(rec->fg(), "tab_recipe", "fg", Units::sp_grav, 3)));
   entry.append(tr("ABV"));
   value.append(QString("%1%").arg(Brewtarget::displayAmount(rec->ABV_pct(), 0, 1)));
   entry.append(tr("Bitterness"));
   value.append(QString("%1 %2 (%3)").arg(Brewtarget::displayAmount(rec->IBU(), 0, 1))
                              .arg(tr("IBU"))
                              .arg(Brewtarget::ibuFormulaName()));
   entry.append(tr("Color"));
   value.append(QString("%1 (%2)").arg(Brewtarget::displayAmount(rec->color_srm(),"tab_recipe", "color_srm", Units::srm, 1))
                           .arg(Brewtarget::colorFormulaName()));

   padAllToMaxLength(&entry);
   padAllToMaxLength(&value);

   for(i = 0; i < nbLines; ++i)
      ret += entry.at(i) + value.at(i) + "\n";

   return ret;
}

QString RecipeFormatter::buildFermentableTableHtml()
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
   // Set up the header row.
   ftable += QString("<tr>"
                     "<th align=\"left\" width=\"20%\">%1</th>"
                     "<th align=\"left\" width=\"10%\">%2</th>"
                     "<th align=\"left\" width=\"10%\">%3</th>"
                     "<th align=\"left\" width=\"10%\">%4</th>"
                     "<th align=\"left\" width=\"10%\">%5</th>"
                     "<th align=\"left\" width=\"10%\">%6</th>"
                     "<th align=\"left\" width=\"10%\">%7</th>"
                     "</tr>")
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
      ftable += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6%</td><td>%7</td></tr>")
            .arg( ferm->name())
            .arg( ferm->typeStringTr())
            .arg( Brewtarget::displayAmount(ferm->amount_kg(), "fermentableTable", "amount_kg", Units::kilograms))
            .arg( ferm->isMashed() ? tr("Yes") : tr("No") )
            .arg( ferm->addAfterBoil() ? tr("Yes") : tr("No"))
            .arg( Brewtarget::displayAmount(ferm->yield_pct(), 0, 0) )
            .arg( Brewtarget::displayAmount(ferm->color_srm(), "fermentableTable", "color_srm", Units::srm, 1));
   }
   // One row for the total grain (QTextBrowser does not know the caption tag)
   ftable += QString("<tr><td><b>%1</b></td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
            .arg(tr("Total"))
            .arg("&mdash;" )
            .arg(Brewtarget::displayAmount(rec->grains_kg(), "fermentableTable", "amount_kg", Units::kilograms))
            .arg("&mdash;")
            .arg("&mdash;")
            .arg("&mdash;")
            .arg("&mdash;");
   ftable += "</table>";
   return ftable;
}

QString RecipeFormatter::buildFermentableTableTxt()
{
   QString ret = "";
   int i, size;

   if( rec == 0 )
      return "";

   QList<Fermentable*> ferms = sortFermentablesByWeight(rec);
   size = ferms.size();
   if( size > 0 )
   {
      QStringList names, types, amounts, masheds, lates, yields, colors;

      names.append(tr("Name"));
      types.append(tr("Type"));
      amounts.append(tr("Amount"));
      masheds.append(tr("Mashed"));
      lates.append(tr("Late"));
      yields.append(tr("Yield"));
      colors.append(tr("Color"));

      for( i = 0; i < size; ++i )
      {
         Fermentable* ferm =  ferms[i];
         names.append( ferm->name() );
         types.append( ferm->typeStringTr() );
         amounts.append(Brewtarget::displayAmount(ferm->amount_kg(), "fermentableTable", "amount_kg", Units::kilograms));
         masheds.append( ferm->isMashed() ? tr("Yes") : tr("No"));
         lates.append( ferm->addAfterBoil() ? tr("Yes") : tr("No"));
         yields.append( QString("%1%").arg(Brewtarget::displayAmount(ferm->yield_pct(), 0, 0) ) );
         colors.append( QString("%1").arg(Brewtarget::displayAmount(ferm->color_srm(), "fermentableTable", "color_srm", Units::srm, 1)));
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

      ret += QString("%1 %2\n").arg(tr("Total grain:")).arg(Brewtarget::displayAmount(rec->grains_kg(), "fermentableTable", "amount_kg", Units::kilograms));
   }
   return ret;
}

QString RecipeFormatter::buildHopsTableHtml()
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
   hTable += QString("<tr>"
                     "<th align=\"left\" width=\"20%\">%1</th>"
                     "<th align=\"left\" width=\"10%\">%2</th>"
                     "<th align=\"left\" width=\"10%\">%3</th>"
                     "<th align=\"left\" width=\"10%\">%4</th>"
                     "<th align=\"left\" width=\"10%\">%5</th>"
                     "<th align=\"left\" width=\"10%\">%6</th>"
                     "<th align=\"left\" width=\"10%\">%7</th>"
                     "</tr>")
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
            .arg( Brewtarget::displayAmount(hop->time_min(), "hopTable", "time_min", Units::minutes))
            .arg( hop->formStringTr())
            .arg( Brewtarget::displayAmount(rec->ibuFromHop(hop), 0, 1) );
   }
   hTable += "</table>";
   return hTable;
}

QString RecipeFormatter::buildHopsTableTxt()
{
   QString ret = "";
   int i, size;

   if( rec == 0 )
      return "";

   QList<Hop*> hops = sortHopsByTime(rec);
   size = hops.size();
   if( size > 0 )
   {
      QStringList names, alphas, amounts, uses, times, forms, ibus;

      names.append(tr("Name"));
      alphas.append(("Alpha"));
      amounts.append(tr("Amount"));
      uses.append(tr("Use"));
      times.append(tr("Time"));
      forms.append(tr("Form"));
      ibus.append(tr("IBU"));

      for( i = 0; i < size; ++i )
      {
         Hop* hop = hops[i];

         names.append(hop->name());
         alphas.append(QString("%1%").arg(Brewtarget::displayAmount(hop->alpha_pct(), 0, 1)));
         amounts.append(Brewtarget::displayAmount(hop->amount_kg(), "hopTable", "amount_kg", Units::kilograms));
         uses.append(hop->useStringTr());
         times.append(Brewtarget::displayAmount(hop->time_min(), "hopTable", "time_min", Units::minutes));
         forms.append(hop->formStringTr());
         ibus.append(QString("%1").arg( Brewtarget::displayAmount(rec->ibuFromHop(hop), 0, 1)));
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
   return ret;
}

QString RecipeFormatter::buildMiscTableHtml()
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
   mtable += QString("<tr>"
                     "<th align=\"left\" width=\"20%\">%1</th>"
                     "<th align=\"left\" width=\"10%\">%2</th>"
                     "<th align=\"left\" width=\"10%\">%3</th>"
                     "<th align=\"left\" width=\"10%\">%4</th>"
                     "<th align=\"left\" width=\"10%\">%5</th>"
                     "</tr>")
         .arg(tr("Name"))
         .arg(tr("Type"))
         .arg(tr("Use"))
         .arg(tr("Amount"))
         .arg(tr("Time"));
   for( i = 0; i < size; ++i)
   {
      Misc *misc = miscs[i];
      kindOf = misc->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters;

      mtable += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
            .arg( misc->name())
            .arg( misc->typeStringTr())
            .arg( misc->useStringTr())
            .arg( Brewtarget::displayAmount(misc->amount(), "miscTableModel", "amount_kg", kindOf, 3))
            .arg( Brewtarget::displayAmount(misc->time(), "miscTableModel", "time", Units::minutes));
   }
   mtable += "</table>";
   return mtable;

}

QString RecipeFormatter::buildMiscTableTxt()
{
   QString ret = "";
   int i, size;
   Unit* kindOf;

   if( rec == 0 )
      return "";

   QList<Misc*> miscs = rec->miscs();
   size = miscs.size();
   if( size > 0 )
   {
      QStringList names, types, uses, amounts, times;

      names.append(tr("Name"));
      types.append(tr("Type"));
      uses.append(tr("Use"));
      amounts.append(tr("Amount"));
      times.append(tr("Time"));

      for( i = 0; i < size; ++i )
      {
         Misc* misc = miscs[i];
         kindOf = misc->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters;
         names.append(misc->name());
         types.append(misc->typeStringTr());
         uses.append(misc->useStringTr());
         amounts.append(Brewtarget::displayAmount(misc->amount(), "miscTableModel", "amount_kg", kindOf, 3));
         times.append(Brewtarget::displayAmount(misc->time(), "miscTableModel", "time", Units::minutes));
      }

      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&uses);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&times);

      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + uses.at(i) + amounts.at(i) + times.at(i) + "\n";
   }
   return ret;
}

QString RecipeFormatter::buildYeastTableHtml()
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
   ytable += QString("<tr>"
                     "<th width=\"20%\" align=\"left\">%1</th>"
                     "<th width=\"10%\" align=\"left\">%2</th>"
                     "<th width=\"10%\" align=\"left\">%3</th>"
                     "<th width=\"10%\" align=\"left\">%4</th>"
                     "<th width=\"10%\" align=\"left\">%5</th></tr>")
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
            .arg( Brewtarget::displayAmount( y->amount(), "yeastTableModel", "amount_kg", kindOf, 2) )
            .arg( y->addToSecondary() ? tr("Secondary") : tr("Primary"));
   }
   ytable += "</table>";
   return ytable;
}

QString RecipeFormatter::buildYeastTableTxt()
{
   QString ret = "";
   int i, size;
   Unit* kindOf;

   if( rec == 0 )
      return "";

   QList<Yeast*> yeasts = rec->yeasts();
   size = yeasts.size();
   if( size > 0 )
   {
      QStringList names, types, forms, amounts, stages;

      names.append(tr("Name"));
      types.append(tr("Type"));
      forms.append(tr("Form"));
      amounts.append(tr("Amount"));
      stages.append(tr("Stage"));

      for( i = 0; i < size; ++i )
      {
         Yeast* y = yeasts[i];
         kindOf = y->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters;
         names.append(y->name());
         types.append(y->typeStringTr());
         forms.append(y->formStringTr());
         amounts.append(Brewtarget::displayAmount( y->amount(), "yeastTableModel", "amount_kg", kindOf, 2));
         stages.append(y->addToSecondary() ? tr("Secondary") : tr("Primary"));
      }

      padAllToMaxLength(&names);
      padAllToMaxLength(&types);
      padAllToMaxLength(&forms);
      padAllToMaxLength(&amounts);
      padAllToMaxLength(&stages);

      for( i = 0; i < size+1; ++i )
         ret += names.at(i) + types.at(i) + forms.at(i) + amounts.at(i) + stages.at(i) + "\n";
   }
   return ret;
}

QString RecipeFormatter::buildMashTableHtml()
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
   mtable += QString("<tr>"
                     "<th align=\"left\" width=\"20%\">%1</th>"
                     "<th align=\"left\" width=\"10%\">%2</th>"
                     "<th align=\"left\" width=\"10%\">%3</th>"
                     "<th align=\"left\" width=\"10%\">%4</th>"
                     "<th align=\"left\" width=\"10%\">%5</th>"
                     "<th align=\"left\" width=\"10%\">%6</th>"
                     "</tr>")
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

      if( ms->isInfusion() )
      {
         tmp = tmp.arg(Brewtarget::displayAmount(ms->infuseAmount_l(), "mashStepTableModel", "amount", Units::liters))
                  .arg(Brewtarget::displayAmount(ms->infuseTemp_c(),   "mashStepTableModel", "infuseTemp_c", Units::celsius));
      }
      else if( ms->isDecoction() )
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

QString RecipeFormatter::buildMashTableTxt()
{
   QString ret = "";
   int i, size;

   if( rec == 0 )
      return "";

   Mash* mash = rec->mash();

   QList<MashStep*> mashSteps;
   if( mash )
      mashSteps = mash->mashSteps();
   size = mashSteps.size();
   if( size > 0 )
   {
      QStringList names, types, amounts, temps, targets, times;

      names.append(tr("Name"));
      types.append(tr("Type"));
      amounts.append(tr("Amount"));
      temps.append(tr("Temp"));
      targets.append(tr("Target"));
      times.append(tr("Time"));

      for( i = 0; i < size; ++i )
      {
         MashStep* s = mashSteps[i];
         names.append(s->name());
         types.append(s->typeStringTr());
         if( s->isInfusion() )
         {
            amounts.append(Brewtarget::displayAmount(s->infuseAmount_l(), "mashStepTableModel", "amount", Units::liters));
            temps.append(Brewtarget::displayAmount(s->infuseTemp_c(),   "mashStepTableModel", "infuseTemp_c", Units::celsius));
         }
         else if( s->isDecoction() )
         {
            amounts.append(Brewtarget::displayAmount(s->decoctionAmount_l(), "mashStepTableModel", "amount", Units::liters));
            temps.append("---");
         }
         else
         {
            amounts.append( "---" );
            temps.append("---");
         }
         targets.append(Brewtarget::displayAmount(s->stepTemp_c(), "mashStepTableModel", "stepTemp_c", Units::celsius));
         times.append(Brewtarget::displayAmount(s->stepTime_min(), "mashStepTableModel", "time", Units::minutes, 0));
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
   return ret;
}

QString RecipeFormatter::buildNotesHtml()
{
   QString notes;

   if ( rec == 0 || rec->notes() == "" )
      return "";

   notes = QString("<h3>%1</h3>").arg(tr("Notes"));
   // NOTE: (heh) Using the QTextDocument.toHtml() method doesn't really work
   // here. So we cheat and use some newer functionality
   notes += rec->notes().toHtmlEscaped();

   return notes;
}

QString RecipeFormatter::buildInstructionTableHtml()
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

QString RecipeFormatter::buildInstructionTableTxt()
{
   QString ret = "";
   int i, size;

   if( rec == 0 )
      return "";

   QStringList num, text;

   QList<Instruction*> instructions = rec->instructions();
   size = instructions.size();
   if( size > 0 )
   {
      Instruction* ins;
      for( i = 0; i < size; ++i )
      {
         ins = instructions[i];
         num.append(QString("%1").arg(i));
         //Wrap instruction text to 75 ( 80 (text separator length) - 5 (num colunm lenght) )
         text.append(QString("- %1").arg(wrapText(ins->directions(), 75)));
      }
      padAllToMaxLength(&num, 1);
      //Set a margin to align multiple line instructions
      QString leftMargin = QString("").leftJustified(num.at(0).size() + 2, ' ');
      for( i = 0; i < size; ++i)
      {
         QString tmp = text.at(i);
         tmp.replace("\n", "\n" + leftMargin);
         ret += num.at(i) + tmp + "\n";
      }
   }
   return ret;
}

QString RecipeFormatter::buildBrewNotesHtml()
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
                 .arg(Brewtarget::displayAmount(note->sg(), section, "sg", Units::sp_grav, 3))
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
                 .arg(Brewtarget::displayAmount(note->calculateOg(), section, "projOg", Units::sp_grav, 3));
      bnTable += "</table>";

      // POSTBOIL
      section = "page_postboil";
      bnTable += "<table id=\"brewnote\">";
      bnTable += QString("<caption>%1</caption>").arg(tr("Postboil"));
      bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                 .arg(tr("OG"))
                 .arg(Brewtarget::displayAmount(note->og(),section, "og", Units::sp_grav, 3))
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
                 .arg(Brewtarget::displayAmount(note->fg(),section,"fg",Units::sp_grav, 3))
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

QString RecipeFormatter::getLabelToolTip() {
   QString header;
   QString body;

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   body   = "<body>";
   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( "Using PostgreSQL");

   // First row -- hostname and port
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
         .arg(tr("Hostname"))
         .arg(Brewtarget::option("dbHostname").toString());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
         .arg(tr("Port"))
         .arg(Brewtarget::option("dbPortnum").toInt());
   // Second row -- schema and database
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
         .arg(tr("Schema"))
         .arg(Brewtarget::option("dbSchema").toString());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
         .arg(tr("Database"))
         .arg(Brewtarget::option("dbName").toString());

   // third row -- username and is the password saved (NOTE: NOT THE
   // PASSWORD ITSELF)
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
         .arg(tr("Username"))
         .arg(Brewtarget::option("dbUsername").toString());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
         .arg(tr("Saved Password"))
         .arg( Brewtarget::hasOption("dbPassword") ? "Yes" : "No");


   body += "</table></body></html>";

   return header + body;
}


bool RecipeFormatter::loadComplete(bool ok)
{
   doc->print(printer);
   return ok;
}

void RecipeFormatter::print(QPrinter* mainPrinter,
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
      doc->setHtml(getHTMLFormat());
      loadComplete(true);
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
