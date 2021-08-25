/*
 * RecipeFormatter.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
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

#include <QClipboard>
#include <QPrinter>
#include <QStringList>

#include "brewtarget.h"
#include "database/Database.h" // Needed only for tooltip that displays database connection parameters
#include "Html.h"
#include "MainWindow.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"
#include "Unit.h"

namespace {
   //! Get the maximum number of characters in a list of strings.
   unsigned int getMaxLength( QStringList* list ) {
      int maxlen = 0;
      for (int ii = 0; ii < list->count(); ++ii) {
         if (list->at(ii).size() > maxlen) {
            maxlen = list->at(ii).size();
         }
      }

      return static_cast<unsigned int>(maxlen);
   }

   //! Prepend a string with spaces until its final length is the given length.
   QString padToLength( const QString &str, unsigned int length ) {
      // The 0 is redundant. It makes me feel better
      return QString("%1").arg(str, 0 - static_cast<int>(length), ' ');
   }

   //! Same as \b padToLength but with multiple strings.
   void padAllToMaxLength( QStringList* list, unsigned int padding = 2) {
      unsigned int maxlen = getMaxLength( list );
      int size = list->count();
      for (int ii = 0; ii < size; ++ii) {
         // Add a padding so that text doesn't run together.
         list->replace(ii, padToLength( list->at(ii), maxlen + padding ) );
      }
      return;
   }

   //! Return the text wrapped with the given length
   QString wrapText( const QString &text, int wrapLength ) {
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
            if ( currentLine.at(splitPos) == ' ' ) //String without whitespace won't be split
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

   QList<Hop*> sortHopsByTime(Recipe* rec) {
      QList<Hop*> sorted = rec->hops();

      std::sort(sorted.begin(), sorted.end(), hopLessThanByTime);
      return sorted;
   }

   QList<Fermentable*> sortFermentablesByWeight(Recipe* rec) {
      QList<Fermentable*> sorted = rec->fermentables();

      std::sort(sorted.begin(), sorted.end(), fermentablesLessThanByWeight);
      return sorted;
   }

}


// This private implementation class holds all private non-virtual members of RecipeFormatter
class RecipeFormatter::impl {
public:

   /**
    * Constructor
    */
   impl() : textSeparator{nullptr},
            rec{nullptr} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;


   //! Get a plaintext view.
   QString getTextFormat() {
      QString ret = "";
      QString tmp = "";
      if (this->rec == nullptr) {
         return "";
      }

      Style* style = this->rec->style();

      ret += QString("%1 - %2 (%3%4)\n").arg( rec->name())
            .arg( style ? style->name() : tr("unknown style"))
            .arg( style ? style->categoryNumber() : tr("N/A"))
            .arg( style ? style->styleLetter() : "");
      ret += this->getTextSeparator();
      ret += this->buildStatTableTxt();
      if ((tmp = this->buildFermentableTableTxt()) != "") {
         ret += "\n" + tr("Fermentables") + "\n";
         ret += this->getTextSeparator();
         ret += tmp;
      }
      if ((tmp = this->buildHopsTableTxt()) != "") {
         ret += "\n" + tr("Hops") + "\n";
         ret += this->getTextSeparator();
         ret += tmp;
      }
      if ((tmp = this->buildMiscTableTxt()) != "") {
         ret += "\n" + tr("Miscs") + "\n";
         ret += this->getTextSeparator();
         ret += tmp;
      }
      if ((tmp = this->buildYeastTableTxt()) != "") {
         ret += "\n" + tr("Yeasts") + "\n";
         ret += this->getTextSeparator();
         ret += tmp;
      }
      if ((tmp = this->buildMashTableTxt()) != "") {
         ret += "\n" + tr("Mash") + "\n";
         ret += this->getTextSeparator();
         ret += tmp;
      }
      if ((tmp = rec->notes()) != "" ) {
         ret += "\n" + tr("Notes") + "\n";
         ret += getTextSeparator();
         ret += tmp;
      }
      if ((tmp = this->buildInstructionTableTxt()) != "") {
         ret += "\n" + tr("Instructions") + "\n";
         ret += this->getTextSeparator();
         ret += tmp;
      }

      return ret;
   }

   //! Get an HTML view.
   QString getHTMLFormat() {
      QString pDoc = this->buildHTMLHeader();
      pDoc += this->buildStatTableHtml();
      pDoc += this->buildFermentableTableHtml();
      pDoc += this->buildHopsTableHtml();
      pDoc += this->buildMiscTableHtml();
      pDoc += this->buildYeastTableHtml();
      pDoc += this->buildMashTableHtml();
      pDoc += this->buildNotesHtml();
      pDoc += this->buildInstructionTableHtml();
      pDoc += this->buildBrewNotesHtml();

      pDoc += this->buildHTMLFooter();

      return pDoc;
   }

   QString getTextSeparator() {
      if (this->textSeparator.get() != nullptr) {
         return *this->textSeparator;
      }

      this->textSeparator.reset(new QString{});
      for(int ii = 0; ii < 80; ++ii ) {
         this->textSeparator->append('=');
      }

      this->textSeparator->append('\n');
      return *this->textSeparator;
   }

   QString buildHTMLHeader() {
      return Html::createHeader(RecipeFormatter::tr("Recipe"), ":css/recipe.css");
   }

   QString buildStatTableHtml() {
      QString header;
      QString body;

      if (this->rec == nullptr) {
         return "";
      }

      Style* style = rec->style();

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
            .arg(Brewtarget::displayAmount(rec->finalVolume_l(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::finalVolume_l, &Units::liters));
      body += QString("<td width=\"40%\" align=\"right\" class=\"right\">%1</td>"
                     "<td class=\"value\">%2</td>"
                     "</tr>")
            .arg(tr("Boil Size"))
            .arg(Brewtarget::displayAmount(rec->boilVolume_l(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::boilVolume_l, &Units::liters));
      // Second row: Boil Time and Efficiency
      body += QString("<tr>"
                     "<td align=\"left\" class=\"left\">%1</td>"
                     "<td class=\"value\">%2</td>")
            .arg(tr("Boil Time"))
            .arg( (rec->equipment() == nullptr)?
                     Brewtarget::displayAmount(0, PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::boilTime_min, &Units::minutes)
                  : Brewtarget::displayAmount( (rec->equipment())->boilTime_min(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::boilTime_min, &Units::minutes));
      body += QString("<td align=\"right\" class=\"right\">%1</td>"
                     "<td class=\"value\">%2</td></tr>")
            .arg(tr("Efficiency"))
            .arg(rec->efficiency_pct(), 0, 'f', 0);

      // Third row: OG and FG
      body += QString("<tr>"
                     "<td align=\"left\" class=\"left\">%1</td>"
                     "<td class=\"value\">%2</td>")
            .arg(tr("OG"))
            .arg(Brewtarget::displayAmount(rec->og(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::og, &Units::sp_grav, 3));
      body += QString("<td align=\"right\" class=\"right\">%1</td>"
                     "<td class=\"value\">%2</td></tr>")
            .arg(tr("FG"))
            .arg(Brewtarget::displayAmount(rec->fg(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::fg, &Units::sp_grav, 3));

      // Fourth row: ABV and Bitterness.  We need to set the bitterness string up first
      body += QString("<tr>"
                     "<td align=\"left\" class=\"left\">%1</td>"
                     "<td class=\"value\">%2%</td>")
            .arg(tr("ABV"))
            .arg(Brewtarget::displayAmount(rec->ABV_pct(), nullptr, 1));
      body += QString("<td align=\"right\" class=\"right\">%1</td>"
                     "<td class=\"value\">%2 (%3)</td></tr>")
            .arg(tr("IBU"))
            .arg(Brewtarget::displayAmount(rec->IBU(), nullptr, 1))
            .arg(Brewtarget::ibuFormulaName() );

      // Fifth row: Color and calories.  Set up the color string first
      body += QString("<tr>"
                     "<td align=\"left\" class=\"left\">%1</td>"
                     "<td class=\"value\">%2 (%3)</td>")
            .arg(tr("Color"))
            .arg(Brewtarget::displayAmount(rec->color_srm(),PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::color_srm, &Units::srm, 1))
            .arg(Brewtarget::colorFormulaName());

      body += QString("<td align=\"right\" class=\"right\">%1</td>"
                     "<td class=\"value\">%2</td></tr>")
            .arg( Brewtarget::getVolumeUnitSystem() == SI ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
            .arg( Brewtarget::displayAmount(Brewtarget::getVolumeUnitSystem() == SI ? rec->calories33cl() : rec->calories12oz(),nullptr,0) );

      body += "</table>";

      return header + body;

   }

   QString buildStatTableTxt() {
      const int nbLines = 9;

      if (this->rec == nullptr) {
         return "";
      }

      QStringList entry, value;

      entry.append(tr("Batch Size"));
      value.append(QString("%1").arg(Brewtarget::displayAmount(rec->finalVolume_l(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::finalVolume_l, &Units::liters)));
      entry.append(tr("Boil Size"));
      value.append(QString("%1").arg(Brewtarget::displayAmount(rec->boilVolume_l(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::boilVolume_l, &Units::liters)));
      entry.append(tr("Boil Time"));
      value.append(QString("%1").arg((rec->equipment() == nullptr)?
                           Brewtarget::displayAmount(0, PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::boilTime_min, &Units::minutes)
                        : Brewtarget::displayAmount( (rec->equipment())->boilTime_min(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::boilTime_min, &Units::minutes)));
      entry.append(tr("Efficiency"));
      value.append(QString("%1%").arg(rec->efficiency_pct(), 0, 'f', 0));
      entry.append(tr("OG"));
      value.append(QString("%1").arg(Brewtarget::displayAmount(rec->og(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::og, &Units::sp_grav, 3)));
      entry.append(tr("FG"));
      value.append(QString("%1").arg(Brewtarget::displayAmount(rec->fg(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::fg, &Units::sp_grav, 3)));
      entry.append(tr("ABV"));
      value.append(QString("%1%").arg(Brewtarget::displayAmount(rec->ABV_pct(), nullptr, 1)));
      entry.append(tr("Bitterness"));
      value.append(QString("%1 %2 (%3)").arg(Brewtarget::displayAmount(rec->IBU(), nullptr, 1))
                                 .arg(tr("IBU"))
                                 .arg(Brewtarget::ibuFormulaName()));
      entry.append(tr("Color"));
      value.append(QString("%1 (%2)").arg(Brewtarget::displayAmount(rec->color_srm(),PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::color_srm, &Units::srm, 1))
                              .arg(Brewtarget::colorFormulaName()));

      padAllToMaxLength(&entry);
      padAllToMaxLength(&value);

      QString ret = "";
      for(int ii = 0; ii < nbLines; ++ii) {
         ret += entry.at(ii) + value.at(ii) + "\n";
      }

      return ret;
   }

   QString buildFermentableTableHtml() {
      if (this->rec == nullptr) {
         return "";
      }

      QString ftable;
      QList<Fermentable*> ferms = sortFermentablesByWeight(this->rec);

      int size = ferms.size();
      if ( size < 1 ) {
         return "";
      }

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
      for(int ii = 0; ii < size; ++ii) {
         Fermentable* ferm = ferms[ii];
         ftable += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6%</td><td>%7</td></tr>")
               .arg( ferm->name())
               .arg( ferm->typeStringTr())
               .arg( Brewtarget::displayAmount(ferm->amount_kg(), PersistentSettings::Sections::fermentableTable, PropertyNames::Fermentable::amount_kg, &Units::kilograms))
               .arg( ferm->isMashed() ? tr("Yes") : tr("No") )
               .arg( ferm->addAfterBoil() ? tr("Yes") : tr("No"))
               .arg( Brewtarget::displayAmount(ferm->yield_pct(), nullptr, 0) )
               .arg( Brewtarget::displayAmount(ferm->color_srm(), PersistentSettings::Sections::fermentableTable, PropertyNames::Fermentable::color_srm, &Units::srm, 1));
      }
      // One row for the total grain (QTextBrowser does not know the caption tag)
      ftable += QString("<tr><td><b>%1</b></td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
               .arg(tr("Total"))
               .arg("&mdash;" )
               .arg(Brewtarget::displayAmount(rec->grains_kg(), PersistentSettings::Sections::fermentableTable, PropertyNames::Fermentable::amount_kg, &Units::kilograms))
               .arg("&mdash;")
               .arg("&mdash;")
               .arg("&mdash;")
               .arg("&mdash;");
      ftable += "</table>";
      return ftable;
   }

   QString buildFermentableTableTxt() {
      if (this->rec == nullptr) {
         return "";
      }

      QString ret = "";
      QList<Fermentable*> ferms = sortFermentablesByWeight(this->rec);
      int size = ferms.size();
      if (size > 0) {
         QStringList names, types, amounts, masheds, lates, yields, colors;

         names.append(tr("Name"));
         types.append(tr("Type"));
         amounts.append(tr("Amount"));
         masheds.append(tr("Mashed"));
         lates.append(tr("Late"));
         yields.append(tr("Yield"));
         colors.append(tr("Color"));

         for (int ii = 0; ii < size; ++ii) {
            Fermentable* ferm =  ferms[ii];
            names.append( ferm->name() );
            types.append( ferm->typeStringTr() );
            amounts.append(Brewtarget::displayAmount(ferm->amount_kg(), PersistentSettings::Sections::fermentableTable, PropertyNames::Fermentable::amount_kg, &Units::kilograms));
            masheds.append( ferm->isMashed() ? tr("Yes") : tr("No"));
            lates.append( ferm->addAfterBoil() ? tr("Yes") : tr("No"));
            yields.append( QString("%1%").arg(Brewtarget::displayAmount(ferm->yield_pct(), nullptr, 0) ) );
            colors.append( QString("%1").arg(Brewtarget::displayAmount(ferm->color_srm(), PersistentSettings::Sections::fermentableTable, PropertyNames::Fermentable::color_srm, &Units::srm, 1)));
         }

         padAllToMaxLength(&names);
         padAllToMaxLength(&types);
         padAllToMaxLength(&amounts);
         padAllToMaxLength(&masheds);
         padAllToMaxLength(&lates);
         padAllToMaxLength(&yields);
         padAllToMaxLength(&colors);

         for (int ii = 0; ii < size+1; ++ii) {
            ret += names.at(ii) + types.at(ii) + amounts.at(ii) + masheds.at(ii) + lates.at(ii) + yields.at(ii) + colors.at(ii) + "\n";
         }

         ret += QString("%1 %2\n").arg(tr("Total grain:")).arg(Brewtarget::displayAmount(rec->grains_kg(), PersistentSettings::Sections::fermentableTable, PropertyNames::Fermentable::amount_kg, &Units::kilograms));
      }
      return ret;
   }

   QString buildHopsTableHtml() {
      if (this->rec == nullptr) {
         return "";
      }

      QList<Hop*> hops = sortHopsByTime(rec);

      int size = hops.size();
      if ( size < 1 ) {
         return "";
      }

      QString hTable = QString("<h3>%1</h3>").arg(tr("Hops"));
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

      for(int ii = 0; ii < size; ++ii) {
         Hop *hop = hops[ii];
         hTable += QString("<tr><td>%1</td><td>%2%</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
               .arg( hop->name())
               .arg( Brewtarget::displayAmount(hop->alpha_pct(),nullptr,1) )
               .arg( Brewtarget::displayAmount(hop->amount_kg(), PersistentSettings::Sections::hopTable, PropertyNames::Hop::amount_kg, &Units::kilograms))
               .arg( hop->useStringTr())
               .arg( Brewtarget::displayAmount(hop->time_min(), PersistentSettings::Sections::hopTable, PropertyNames::Hop::time_min, &Units::minutes))
               .arg( hop->formStringTr())
               .arg( Brewtarget::displayAmount(rec->ibuFromHop(hop), nullptr, 1) );
      }
      hTable += "</table>";
      return hTable;
   }

   QString buildHopsTableTxt() {
      if (this->rec == nullptr) {
         return "";
      }

      QString ret = "";
      QList<Hop*> hops = sortHopsByTime(rec);
      int size = hops.size();
      if ( size > 0 ) {
         QStringList names, alphas, amounts, uses, times, forms, ibus;

         names.append(tr("Name"));
         alphas.append(("Alpha"));
         amounts.append(tr("Amount"));
         uses.append(tr("Use"));
         times.append(tr("Time"));
         forms.append(tr("Form"));
         ibus.append(tr("IBU"));

         for(int ii = 0; ii < size; ++ii) {
            Hop* hop = hops[ii];

            names.append(hop->name());
            alphas.append(QString("%1%").arg(Brewtarget::displayAmount(hop->alpha_pct(), nullptr, 1)));
            amounts.append(Brewtarget::displayAmount(hop->amount_kg(), PersistentSettings::Sections::hopTable, PropertyNames::Hop::amount_kg, &Units::kilograms));
            uses.append(hop->useStringTr());
            times.append(Brewtarget::displayAmount(hop->time_min(), PersistentSettings::Sections::hopTable, PropertyNames::Hop::time_min, &Units::minutes));
            forms.append(hop->formStringTr());
            ibus.append(QString("%1").arg( Brewtarget::displayAmount(rec->ibuFromHop(hop), nullptr, 1)));
         }

         padAllToMaxLength(&names);
         padAllToMaxLength(&alphas);
         padAllToMaxLength(&amounts);
         padAllToMaxLength(&uses);
         padAllToMaxLength(&times);
         padAllToMaxLength(&forms);
         padAllToMaxLength(&ibus);

         for(int ii = 0; ii < size+1; ++ii) {
            ret += names.at(ii) + alphas.at(ii) + amounts.at(ii) + uses.at(ii) + times.at(ii) + forms.at(ii) + ibus.at(ii) + "\n";
         }
      }
      return ret;
   }

   QString buildMiscTableHtml() {
      if (this->rec == nullptr) {
         return "";
      }

      QList<Misc*> miscs = rec->miscs();
      int size = miscs.size();
      if ( size < 1 ) {
         return "";
      }

      QString mtable = QString("<h3>%1</h3>").arg(tr("Misc"));
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
      for (int ii = 0; ii < size; ++ii) {
         Misc *misc = miscs[ii];
         Unit const * kindOf = misc->amountIsWeight() ? &Units::kilograms : &Units::liters;

         mtable += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
               .arg( misc->name())
               .arg( misc->typeStringTr())
               .arg( misc->useStringTr())
               .arg( Brewtarget::displayAmount(misc->amount(), PersistentSettings::Sections::miscTableModel, PropertyNames::Misc::amount, kindOf, 3))
               .arg( Brewtarget::displayAmount(misc->time(), PersistentSettings::Sections::miscTableModel, PropertyNames::Misc::time, &Units::minutes));
      }
      mtable += "</table>";
      return mtable;

   }

   QString buildMiscTableTxt() {
      if (this->rec == nullptr) {
         return "";
      }
      QString ret = "";

      QList<Misc*> miscs = this->rec->miscs();
      int size = miscs.size();
      if( size > 0 ) {
         QStringList names, types, uses, amounts, times;

         names.append(tr("Name"));
         types.append(tr("Type"));
         uses.append(tr("Use"));
         amounts.append(tr("Amount"));
         times.append(tr("Time"));

         for (int ii = 0; ii < size; ++ii) {
            Misc* misc = miscs[ii];
            Unit const * kindOf = misc->amountIsWeight() ? &Units::kilograms : &Units::liters;
            names.append(misc->name());
            types.append(misc->typeStringTr());
            uses.append(misc->useStringTr());
            amounts.append(Brewtarget::displayAmount(misc->amount(), PersistentSettings::Sections::miscTableModel, PropertyNames::Misc::amount, kindOf, 3));
            times.append(Brewtarget::displayAmount(misc->time(), PersistentSettings::Sections::miscTableModel, PropertyNames::Misc::time, &Units::minutes));
         }

         padAllToMaxLength(&names);
         padAllToMaxLength(&types);
         padAllToMaxLength(&uses);
         padAllToMaxLength(&amounts);
         padAllToMaxLength(&times);

         for (int ii = 0; ii < size+1; ++ii) {
            ret += names.at(ii) + types.at(ii) + uses.at(ii) + amounts.at(ii) + times.at(ii) + "\n";
         }
      }
      return ret;
   }

   QString buildYeastTableHtml() {
      if (this->rec == nullptr) {
         return "";
      }

      QList<Yeast*> yeasts = this->rec->yeasts();
      int size = yeasts.size();

      if( size < 1 ) {
         return "";
      }

      QString ytable = QString("<h3>%1</h3>").arg(tr("Yeast"));
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
      for(int ii = 0; ii < size; ++ii) {
         Yeast* y = yeasts[ii];
         Unit const * kindOf = y->amountIsWeight() ? &Units::kilograms : &Units::liters;

         ytable += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
               .arg( y->name())
               .arg( y->typeStringTr())
               .arg( y->formStringTr())
               .arg( Brewtarget::displayAmount( y->amount(), PersistentSettings::Sections::yeastTableModel, PropertyNames::Yeast::amount, kindOf, 2) )
               .arg( y->addToSecondary() ? tr("Secondary") : tr("Primary"));
      }
      ytable += "</table>";
      return ytable;
   }

   QString buildYeastTableTxt() {
      if (this->rec == nullptr) {
         return "";
      }

      QString ret = "";
      QList<Yeast*> yeasts = rec->yeasts();
      int size = yeasts.size();
      if (size > 0) {
         QStringList names, types, forms, amounts, stages;

         names.append(tr("Name"));
         types.append(tr("Type"));
         forms.append(tr("Form"));
         amounts.append(tr("Amount"));
         stages.append(tr("Stage"));

         for (int ii = 0; ii < size; ++ii) {
            Yeast* y = yeasts[ii];
            Unit const * kindOf = y->amountIsWeight() ? &Units::kilograms : &Units::liters;
            names.append(y->name());
            types.append(y->typeStringTr());
            forms.append(y->formStringTr());
            amounts.append(Brewtarget::displayAmount( y->amount(), PersistentSettings::Sections::yeastTableModel, PropertyNames::Yeast::amount, kindOf, 2));
            stages.append(y->addToSecondary() ? tr("Secondary") : tr("Primary"));
         }

         padAllToMaxLength(&names);
         padAllToMaxLength(&types);
         padAllToMaxLength(&forms);
         padAllToMaxLength(&amounts);
         padAllToMaxLength(&stages);

         for (int ii = 0; ii < size+1; ++ii) {
            ret += names.at(ii) + types.at(ii) + forms.at(ii) + amounts.at(ii) + stages.at(ii) + "\n";
         }
      }
      return ret;
   }

   QString buildMashTableHtml() {
      if (this->rec == nullptr || this->rec->mash() == nullptr) {
         return "";
      }

      MashStep* ms;
      Mash* m = rec->mash();
      QList<MashStep*> mashSteps = m->mashSteps();
      int size = mashSteps.size();

      if (size <= 0) {
         return "";
      }

      QString mtable = QString("<h3>%1</h3>").arg(tr("Mash"));
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
      for (int ii = 0; ii < size; ++ii) {
         QString tmp = "<tr>";
         ms = mashSteps[ii];
         tmp += QString("<td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td>")
               .arg(ms->name())
               .arg(ms->typeStringTr());

         if (ms->isInfusion()) {
            tmp = tmp.arg(Brewtarget::displayAmount(ms->infuseAmount_l(), PersistentSettings::Sections::mashStepTableModel, PropertyNames::MashStep::infuseAmount_l, &Units::liters))
                     .arg(Brewtarget::displayAmount(ms->infuseTemp_c(),   PersistentSettings::Sections::mashStepTableModel, PropertyNames::MashStep::infuseTemp_c, &Units::celsius));
         } else if (ms->isDecoction()) {
            tmp = tmp.arg( Brewtarget::displayAmount( ms->decoctionAmount_l(), PersistentSettings::Sections::mashStepTableModel, PropertyNames::MashStep::decoctionAmount_l, &Units::liters ) )
                  .arg("---");
         } else {
            tmp = tmp.arg( "---" ).arg("---");
         }

         tmp = tmp.arg( Brewtarget::displayAmount(ms->stepTemp_c(), PersistentSettings::Sections::mashStepTableModel, PropertyNames::MashStep::stepTemp_c, &Units::celsius) );
         tmp = tmp.arg( Brewtarget::displayAmount(ms->stepTime_min(), PersistentSettings::Sections::mashStepTableModel, PropertyNames::Misc::time, &Units::minutes, 0) );

         mtable += tmp + "</tr>";
      }

      mtable += "</table>";

      return mtable;
   }

   QString buildMashTableTxt() {
      if (this->rec == nullptr) {
         return "";
      }

      QString ret = "";

      Mash* mash = rec->mash();

      QList<MashStep*> mashSteps;
      if ( mash ) {
         mashSteps = mash->mashSteps();
      }

      int size = mashSteps.size();
      if (size > 0) {
         QStringList names, types, amounts, temps, targets, times;

         names.append(tr("Name"));
         types.append(tr("Type"));
         amounts.append(tr("Amount"));
         temps.append(tr("Temp"));
         targets.append(tr("Target"));
         times.append(tr("Time"));

         for(int ii = 0; ii < size; ++ii) {
            MashStep* s = mashSteps[ii];
            names.append(s->name());
            types.append(s->typeStringTr());
            if ( s->isInfusion() ) {
               amounts.append(Brewtarget::displayAmount(s->infuseAmount_l(), PersistentSettings::Sections::mashStepTableModel, PropertyNames::MashStep::infuseAmount_l, &Units::liters));
               temps.append(Brewtarget::displayAmount(s->infuseTemp_c(),   PersistentSettings::Sections::mashStepTableModel, PropertyNames::MashStep::infuseTemp_c, &Units::celsius));
            } else if( s->isDecoction() ) {
               amounts.append(Brewtarget::displayAmount(s->decoctionAmount_l(), PersistentSettings::Sections::mashStepTableModel, PropertyNames::MashStep::decoctionAmount_l, &Units::liters));
               temps.append("---");
            } else {
               amounts.append( "---" );
               temps.append("---");
            }
            targets.append(Brewtarget::displayAmount(s->stepTemp_c(), PersistentSettings::Sections::mashStepTableModel, PropertyNames::MashStep::stepTemp_c, &Units::celsius));
            times.append(Brewtarget::displayAmount(s->stepTime_min(), PersistentSettings::Sections::mashStepTableModel, PropertyNames::Misc::time, &Units::minutes, 0));
         }

         padAllToMaxLength(&names);
         padAllToMaxLength(&types);
         padAllToMaxLength(&amounts);
         padAllToMaxLength(&temps);
         padAllToMaxLength(&targets);
         padAllToMaxLength(&times);

         for (int ii = 0; ii < size+1; ++ii) {
            ret += names.at(ii) + types.at(ii) + amounts.at(ii) + temps.at(ii) + targets.at(ii) + times.at(ii) + "\n";
         }
      }
      return ret;
   }

   QString buildNotesHtml() {
      if (this->rec == nullptr || rec->notes() == "") {
         return "";
      }

      QString notes = QString("<h3>%1</h3>").arg(tr("Notes"));
      // NOTE: (heh) Using the QTextDocument.toHtml() method doesn't really work
      // here. So we cheat and use some newer functionality
      notes += rec->notes().toHtmlEscaped();

      return notes;
   }

   QString buildInstructionTableHtml() {
      if (this->rec == nullptr) {
         return "";
      }

      QList<Instruction*> instructions = this->rec->instructions();
      int size = instructions.size();
      if ( size < 1 ) {
         return "";
      }

      QString itable = QString("<h3>%1</h3>").arg(tr("Instructions"));
      itable += "<ol id=\"instruction\">";

      for (int ii = 0; ii < size; ++ii) {
         Instruction* ins = instructions[ii];
         itable += QString("<li>%1</li>").arg( ins->directions());
      }

      itable += "</ol>";

      return itable;
   }

   QString buildInstructionTableTxt() {
      if (this->rec == nullptr) {
         return "";
      }

      QString ret = "";

      QStringList num, text;

      QList<Instruction*> instructions = rec->instructions();
      int size = instructions.size();
      if ( size > 0 ) {
         Instruction* ins;
         for (int ii = 0; ii < size; ++ii) {
            ins = instructions[ii];
            num.append(QString("%1").arg(ii));
            //Wrap instruction text to 75 ( 80 (text separator length) - 5 (num colunm lenght) )
            text.append(QString("- %1").arg(wrapText(ins->directions(), 75)));
         }
         padAllToMaxLength(&num, 1);
         //Set a margin to align multiple line instructions
         QString leftMargin = QString("").leftJustified(num.at(0).size() + 2, ' ');
         for (int ii = 0; ii < size; ++ii) {
            QString tmp = text.at(ii);
            tmp.replace("\n", "\n" + leftMargin);
            ret += num.at(ii) + tmp + "\n";
         }
      }
      return ret;
   }

   /* I am not sure how I want to implement these yet.
    * I might just include the salts in the instructions table. Until I decide
    * these stay commented out
   QString buildWaterTableHtml();
   QString buildWaterTableTxt();
   QString buildSaltTableHtml();
   QString buildSaltTableTxt();
   */

   QString buildBrewNotesHtml() {
      if (this->rec == nullptr) {
         return "";
      }

      QString bnTable = "";
      QList<BrewNote*> brewNotes = rec->brewNotes();
      int size = brewNotes.size();
      if ( size < 1 ) {
         return bnTable;
      }

      for(int ii = 0; ii < size; ++ii) {
         BrewNote* note = brewNotes[ii];

         bnTable += QString("<h2>%1 %2</h2>").arg(tr("Brew Date")).arg(note->brewDate_short());

         // PREBOIL, done two-by-two
         bnTable += "<table id=\"brewnote\">";
         bnTable += QString("<caption>%1</caption>").arg(tr("Preboil"));
         bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                  .arg(tr("SG"))
                  .arg(Brewtarget::displayAmount(note->sg(), PersistentSettings::Sections::page_preboil, PropertyNames::BrewNote::sg, &Units::sp_grav, 3))
                  .arg(tr("Volume into BK"))
                  .arg(Brewtarget::displayAmount(note->volumeIntoBK_l(), PersistentSettings::Sections::page_preboil, PropertyNames::BrewNote::volumeIntoBK_l, &Units::liters));

         bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                  .arg(tr("Strike Temp"))
                  .arg(Brewtarget::displayAmount(note->strikeTemp_c(), PersistentSettings::Sections::page_preboil, PropertyNames::BrewNote::strikeTemp_c, &Units::celsius))
                  .arg(tr("Final Temp"))
                  .arg(Brewtarget::displayAmount(note->mashFinTemp_c(), PersistentSettings::Sections::page_preboil, PropertyNames::BrewNote::mashFinTemp_c, &Units::celsius));

         bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                  .arg(tr("Eff into BK"))
                  .arg(Brewtarget::displayAmount(note->calculateEffIntoBK_pct(), nullptr, 2))
                  .arg(tr("Projected OG"))
                  .arg(Brewtarget::displayAmount(note->calculateOg(), PersistentSettings::Sections::page_preboil, PropertyNames::BrewNote::projOg, &Units::sp_grav, 3));
         bnTable += "</table>";

         // POSTBOIL
         bnTable += "<table id=\"brewnote\">";
         bnTable += QString("<caption>%1</caption>").arg(tr("Postboil"));
         bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                  .arg(tr("OG"))
                  .arg(Brewtarget::displayAmount(note->og(),PersistentSettings::Sections::page_postboil, PropertyNames::BrewNote::og, &Units::sp_grav, 3))
                  .arg(tr("Postboil Volume"))
                  .arg(Brewtarget::displayAmount(note->postBoilVolume_l(), PersistentSettings::Sections::page_postboil, PropertyNames::BrewNote::postBoilVolume_l, &Units::liters));
         bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                  .arg(tr("Volume Into Fermenter"))
                  .arg(Brewtarget::displayAmount(note->volumeIntoFerm_l(), PersistentSettings::Sections::page_postboil, PropertyNames::BrewNote::volumeIntoFerm_l, &Units::liters))
                  .arg(tr("Brewhouse Eff"))
                  .arg(Brewtarget::displayAmount(note->calculateBrewHouseEff_pct(), nullptr, 2));
         bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td></tr>")
                  .arg(tr("Projected ABV"))
                  .arg(Brewtarget::displayAmount(note->calculateABV_pct(), nullptr, 2));
         bnTable += "</table>";


         // POSTFERMENT
         bnTable += "<table id=\"brewnote\">";
         bnTable += QString("<caption>%1</caption>").arg(tr("Postferment"));
         bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                  .arg(tr("FG"))
                  .arg(Brewtarget::displayAmount(note->fg(),PersistentSettings::Sections::page_postferment, PropertyNames::BrewNote::fg,&Units::sp_grav, 3))
                  .arg(tr("Volume"))
                  .arg(Brewtarget::displayAmount(note->finalVolume_l(), PersistentSettings::Sections::page_postferment, PropertyNames::BrewNote::finalVolume_l, &Units::liters));

         bnTable += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
                  .arg(tr("Date"))
                  .arg(note->fermentDate_short())
                  .arg(tr("ABV"))
                  .arg(Brewtarget::displayAmount(note->calculateActualABV_pct(), nullptr, 2));
         bnTable += "</table>";

      }

      return bnTable;
   }

   QString buildHTMLFooter() {
      return "</div></body></html>";
   }

   std::unique_ptr<QString> textSeparator;
   Recipe* rec;

};


RecipeFormatter::RecipeFormatter(QWidget* parent) : BtPrintPreview{parent},
                                                    pimpl{new impl{}} {
   this->setWindowTitle(tr("Print Preview"));
   /*
   // Add a print button at the bottom.
   QHBoxLayout* buttonBox = new QHBoxLayout(docDialog);
   QPushButton* print = new QPushButton(QObject::tr("Print"), docDialog);
   connect(print, SLOT(clicked()), Brewtarget::mainWindow, SLOT(printRecipe()));
   buttonBox->addStretch();
   buttonBox->addWidget(print);
   docDialog->layout()->addItem(buttonBox);
   */
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
RecipeFormatter::~RecipeFormatter() = default;


void RecipeFormatter::setRecipe(Recipe* recipe) {
   this->pimpl->rec = recipe;
   return;
}



QString RecipeFormatter::getHTMLFormat( QList<Recipe*> recipes ) {
   Recipe *current = this->pimpl->rec;

   QString hDoc = this->pimpl->buildHTMLHeader();

   // build a toc -- why do I do this to myself?
   hDoc += "<ul>";
   foreach ( Recipe* foo, recipes ) {
       hDoc += QString("<li><a href=\"#%1\">%1</a></li>").arg(foo->name());
   }
   hDoc += "</ul>";

   foreach (Recipe* foo, recipes) {
      this->pimpl->rec = foo;
      hDoc += QString("<a name=\"%1\"></a>").arg(foo->name());
      hDoc += this->pimpl->buildStatTableHtml();
      hDoc += this->pimpl->buildFermentableTableHtml();
      hDoc += this->pimpl->buildHopsTableHtml();
      hDoc += this->pimpl->buildMiscTableHtml();
      hDoc += this->pimpl->buildYeastTableHtml();
      hDoc += this->pimpl->buildMashTableHtml();
      hDoc += this->pimpl->buildNotesHtml();
      hDoc += this->pimpl->buildInstructionTableHtml();
      hDoc += this->pimpl->buildBrewNotesHtml();
      hDoc += "<p></p>";
   }
   hDoc += this->pimpl->buildHTMLFooter();

   this->pimpl->rec = current;
   return hDoc;
}

QString RecipeFormatter::getBBCodeFormat() {
   if (this->pimpl->rec == nullptr) {
      return "";
   }

   QString tmp = "";
   QRegExp regexp("(^[^\n]*\n)(.*$)"); //Regexp to match the first line of tables

   Style* style = this->pimpl->rec->style();

   QString ret = "[size=150][color=#004080][b][u]";
   ret += QString("%1 - %2 (%3%4)").arg(this->pimpl->rec->name())
         .arg( style ? style->name() : tr("unknown style"))
         .arg( style ? style->categoryNumber() : tr("N/A"))
         .arg( style ? style->styleLetter() : "");
   ret += "[/b][/u][/color][/size]\n\n";
   ret += "[pre]" + this->pimpl->buildStatTableTxt() + "[/pre]";
   if ((tmp = this->pimpl->buildFermentableTableTxt()) != "") {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Fermentables") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if ((tmp = this->pimpl->buildHopsTableTxt()) != "") {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Hops") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if ((tmp = this->pimpl->buildMiscTableTxt()) != "") {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Miscs") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if ((tmp = this->pimpl->buildYeastTableTxt()) != "") {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Yeasts") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if ((tmp = this->pimpl->buildMashTableTxt()) != "") {
      tmp.replace(regexp, "[b]\\1[/b]\\2");
      ret += "\n[color=#004080][b]" + tr("Mash") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if ((tmp = this->pimpl->rec->notes()) != "") {
      ret += "\n[color=#004080][b]" + tr("Notes") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }
   if ((tmp = this->pimpl->buildInstructionTableTxt()) != "") {
      ret += "\n[color=#004080][b]" + tr("Instructions") + "[/b][/color]\n";
      ret += "[pre]" + tmp + "[/pre]";
   }

   return ret;
}

QString RecipeFormatter::getToolTip(Recipe* rec) {
   if (rec == nullptr) {
      return "";
   }

   Style* style = rec->style();

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";
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
           .arg(Brewtarget::displayAmount(rec->og(), &Units::sp_grav, 3));
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("FG"))
           .arg(Brewtarget::displayAmount(rec->fg(), &Units::sp_grav, 3));

   // Fourth row: Color and Bitterness.
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td>")
           .arg(tr("Color"))
           .arg(Brewtarget::displayAmount(rec->color_srm(),&Units::srm, 1))
           .arg(Brewtarget::colorFormulaName());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2 (%3)</td></tr>")
           .arg(tr("IBU"))
           .arg(Brewtarget::displayAmount(rec->IBU(), nullptr, 1))
           .arg(Brewtarget::ibuFormulaName() );

   body += "</table></body></html>";

   return header + body;
}

QString RecipeFormatter::getToolTip(Style* style) {
   if (style == nullptr) {
      return "";
   }

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";

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

QString RecipeFormatter::getToolTip(Equipment* kit) {
   if (kit == nullptr) {
      return "";
   }

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( kit->name() );

   // First row -- batchsize and boil time
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Preboil"))
           .arg(Brewtarget::displayAmount(kit->boilSize_l(), &Units::liters) );
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("BoilTime"))
           .arg(Brewtarget::displayAmount(kit->boilTime_min(), &Units::minutes) );

   body += "</table></body></html>";

   return header + body;
}

// Once we do inventory, this needs to be fixed to show amount on hand
QString RecipeFormatter::getToolTip(Fermentable* ferm) {
   if (ferm == nullptr) {
      return "";
   }

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";

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
           .arg(Brewtarget::displayAmount(ferm->color_srm(), &Units::srm, 1));
   // Second row -- isMashed and yield?
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Mashed"))
           .arg( ferm->isMashed() ? tr("Yes") : tr("No") );
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Yield"))
           .arg(Brewtarget::displayAmount(ferm->yield_pct(), nullptr));

   body += "</table></body></html>";

   return header + body;
}

QString RecipeFormatter::getToolTip(Hop* hop) {
   if (hop == nullptr) {
      return "";
   }

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( hop->name() );

   // First row -- alpha and beta
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Alpha"))
           .arg(Brewtarget::displayAmount(hop->alpha_pct(), nullptr));
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Beta"))
           .arg(Brewtarget::displayAmount(hop->beta_pct(), nullptr));

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

QString RecipeFormatter::getToolTip(Misc* misc) {
   if (misc == nullptr) {
      return "";
   }

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";

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

QString RecipeFormatter::getToolTip(Yeast* yeast) {
   if (yeast == nullptr) {
      return "";
   }

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";

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
           .arg(Brewtarget::displayAmount(yeast->attenuation_pct(), nullptr));

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

QString RecipeFormatter::getToolTip(Water* water) {
   if (water == nullptr) {
      return "";
   }

   // Do the style sheet first
   QString header = "<html><head><style type=\"text/css\">";
   header += Html::getCss(":/css/tooltip.css");
   header += "</style></head>";

   QString body   = "<body>";

   body += QString("<div id=\"headerdiv\">");
   body += QString("<table id=\"tooltip\">");
   body += QString("<caption>%1</caption>")
         .arg( water->name() );

   // First row -- Ca and Mg
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Ca"))
           .arg(water->calcium_ppm());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Mg"))
           .arg(water->magnesium_ppm());
   // Second row -- SO4 and Na
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("SO<sub>4</sub>"))
           .arg(water->sulfate_ppm());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("Na"))
           .arg(water->sodium_ppm());
   // third row -- Cl and HCO3
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td>")
           .arg(tr("Cl"))
           .arg(water->chloride_ppm());
   body += QString("<td class=\"left\">%1</td><td class=\"value\">%2</td></tr>")
           .arg(tr("HCO<sub>3</sub>"))
           .arg( water->bicarbonate_ppm());


   body += "</table></body></html>";

   return header + body;
}

void RecipeFormatter::toTextClipboard() {
   QApplication::clipboard()->setText(this->pimpl->getTextFormat());
}

QString RecipeFormatter::getLabelToolTip() {
   Database const & database = Database::instance();
   QString toolTip{};
   QTextStream toolTipAsStream{&toolTip};
   toolTipAsStream <<
      "<html><head><style type=\"text/css\">" << Html::getCss(":/css/tooltip.css") << "</style></head>"
      "<body>"
      "<div id=\"headerdiv\">"
      "<table id=\"tooltip\">"
      "<caption>Using " << DatabaseHelper::getNameFromDbTypeName(database.dbType()) << "</caption>";
   auto connectionParms = database.displayableConnectionParms();
   for (auto parm : connectionParms) {
      toolTipAsStream <<
         "<tr><td class=\"left\">" << parm.first << ": </td><td class=\"value\">" << parm.second << "</td>";
   }
   toolTipAsStream << "</table></body></html>";
   return toolTip;
}

void RecipeFormatter::printPreview() {
   this->setContent(this->pimpl->getHTMLFormat());
   this->show();
   return;
}

void RecipeFormatter::print(QPrinter* printer) {
   this->setContent(this->pimpl->getHTMLFormat());
   this->BtPrintPreview::print(printer);
   return;
}

void RecipeFormatter::exportHtml(QFile* file) {
   this->setContent(this->pimpl->getHTMLFormat());
   this->BtPrintPreview::exportHtml(file);
   return;
}
