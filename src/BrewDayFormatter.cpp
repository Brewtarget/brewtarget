/*
 * BrewDayFormatter.cpp is part of Brewtarget, and is Copyright the following
 * authors 2021
 * - Mattias Måhl <mattias@kejsarsten.com>
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
#include "BrewDayFormatter.h"
#include "brewtarget.h"
#include <QList>
#include <QStringList>
#include "Html.h"
#include "model/Style.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/Instruction.h"

/**
 * @brief Construct a new Brew Day Formatter:: Brew Day Formatter object
 *
 * @param recipe
 */
BrewDayFormatter::BrewDayFormatter(QObject *parent)
   : QObject(parent)
{
   recObs = nullptr;
}

/**
 * @brief Sets the recipe pointer.
 *
 * @param recipe
 */
void BrewDayFormatter::setRecipe(Recipe *recipe)
{
   recObs = recipe;
}

/**
 * @brief Builds the whole HTML page for Brewday instructions
 *
 * @return QString
 */
QString BrewDayFormatter::buildHTML()
{
   return buildTitleHTML() + buildInstructionHTML() + buildFooterHTML();
}

/**
 * @brief generates a table with the basic information about the recipe.
 *
 * @param includeImage
 * @return QString
 */
QString BrewDayFormatter::buildTitleHTML(bool includeImage)
{
   QString header;
   QString body;

   // Do the style sheet first
   if (cssName == nullptr)
      cssName = ":/css/brewday.css";

   header = Html::createHeader(tr("Brewday"), cssName);

   body = QString("<h1>%1</h1>").arg(recObs->name());
   if ( includeImage )
      body += QString("<img src=\"%1\" />").arg("qrc:/images/title.svg");

   // Build the top table
   // Build the first row: Style and Date
   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td>")
         .arg(tr("Style"));
   body += QString("<td class=\"value\">%1</td>")
           .arg( (recObs->style()) ? recObs->style()->name() : "unknown" );
   body += QString("<td class=\"right\">%1</td>")
         .arg(tr("Date"));
   body += QString("<td class=\"value\">%1</td></tr>")
           .arg(QDate::currentDate().toString());

   // second row:  boil time and efficiency.
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Time"))
            .arg((recObs->equipment()) ? Brewtarget::displayAmount(recObs->equipment()->boilTime_min(), "tab_recipe", "boilTime_min", &Units::minutes) : "unknown" )
            .arg(tr("Efficiency"))
            .arg(Brewtarget::displayAmount(recObs->efficiency_pct(),nullptr,0));

   // third row: pre-Boil Volume and Preboil Gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Volume"))
            .arg(Brewtarget::displayAmount(recObs->boilVolume_l(), "tab_recipe", "boilVolume_l", &Units::liters,2))
            .arg(tr("Preboil Gravity"))
            .arg(Brewtarget::displayAmount(recObs->boilGrav(), "tab_recipe", "og", &Units::sp_grav, 3));

   // fourth row: Final volume and starting gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Final Volume"))
            .arg(Brewtarget::displayAmount(recObs->finalVolume_l(), "tab_recipe", "finalVolume_l", &Units::liters,2))
            .arg(tr("Starting Gravity"))
            .arg(Brewtarget::displayAmount(recObs->og(), "tab_recipe", "og", &Units::sp_grav, 3));

   // fifth row: IBU and Final gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("IBU"))
            .arg( Brewtarget::displayAmount(recObs->IBU(),nullptr,1))
            .arg(tr("Final Gravity"))
            .arg(Brewtarget::displayAmount(recObs->fg(), "tab_recipe", "fg", &Units::sp_grav, 3));

   // sixth row: ABV and estimate calories
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("ABV"))
            .arg( Brewtarget::displayAmount(recObs->ABV_pct(),nullptr,1) )
            .arg( Brewtarget::getVolumeUnitSystem() == SI ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
            .arg( Brewtarget::displayAmount(Brewtarget::getVolumeUnitSystem() == SI ? recObs->calories33cl() : recObs->calories12oz(),nullptr,0) );

   body += "</table>";

   return header + body;

}

/**
 * @brief Creates and returns a list of StringList rows with data to be used in Page.
 * @author Mattias Måhl
 *
 * @param includeImage
 * @return QList<QStringList>
 */
QList<QStringList> BrewDayFormatter::buildTitleList()
{
   QList<QStringList> ret;

   if ( ! recObs )
   {
      return ret;
   }
   QString body = "";
   QStringList row;
   row.append(tr("Style"));
   row.append( (recObs->style()) ? recObs->style()->name() : "unknown" );
   row.append(tr("Date"));
   row.append(QDate::currentDate().toString());
   ret.append(row);
   row.clear();

   // second row:  boil time and efficiency.
   row.append(tr("Boil Time"));
   row.append((recObs->equipment()) ? Brewtarget::displayAmount(recObs->equipment()->boilTime_min(), "tab_recipe", "boilTime_min", &Units::minutes) : "unknown");
   row.append(tr("Efficiency"));
   row.append(Brewtarget::displayAmount(recObs->efficiency_pct(),nullptr,0));
   ret.append(row);
   row.clear();

   // third row: pre-Boil Volume and Preboil Gravity
   row.append(tr("Boil Volume"));
   row.append(Brewtarget::displayAmount(recObs->boilVolume_l(), "tab_recipe", "boilVolume_l", &Units::liters,2));
   row.append(tr("Preboil Gravity"));
   row.append(Brewtarget::displayAmount(recObs->boilGrav(), "tab_recipe", "og", &Units::sp_grav, 3));
   ret.append(row);
   row.clear();
   ret.append(row);
   row.clear();

   // fourth row: Final volume and starting gravity
   row.append( tr("Final Volume") );
   row.append( Brewtarget::displayAmount(recObs->finalVolume_l(), "tab_recipe", "finalVolume_l", &Units::liters,2) );
   row.append( tr("Starting Gravity") );
   row.append( Brewtarget::displayAmount( recObs->og(), "tab_recipe", "og", &Units::sp_grav, 3 ) );
   ret.append(row);
   row.clear();

   // fifth row: IBU and Final gravity
   row.append( tr("IBU") );
   row.append( Brewtarget::displayAmount( recObs->IBU(), nullptr, 1 ) );
   row.append( tr("Final Gravity") );
   row.append( Brewtarget::displayAmount( recObs->fg(), "tab_recipe", "fg", &Units::sp_grav, 3 ) );
   ret.append(row);
   row.clear();

   // sixth row: ABV and estimate calories
   row.append(tr("ABV"));
   row.append( Brewtarget::displayAmount(recObs->ABV_pct(),nullptr,1) );
   row.append( Brewtarget::getVolumeUnitSystem() == SI ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"));
   row.append( Brewtarget::displayAmount(Brewtarget::getVolumeUnitSystem() == SI ? recObs->calories33cl() : recObs->calories12oz(),nullptr,0) );
   ret.append(row);
   row.clear();

   return ret;

}

/**
 * @brief Builds the InstructionsTable in HTML and returns a string with the content.
 *
 * @return QString
 */
QString BrewDayFormatter::buildInstructionHTML()
{
   QString middle;
   int i, j, size;

   middle += QString("<h2>%1</h2>").arg(tr("Instructions"));
   middle += QString("<table id=\"steps\">");
   middle += QString("<tr><th class=\"check\">%1</th><th class=\"time\">%2</th><th class=\"step\">%3</th></tr>")
         .arg(tr("Completed"))
         .arg(tr("Time"))
         .arg(tr("Step"));

   QList<Instruction*> instructions = recObs->instructions();
   QList<MashStep*> mashSteps = recObs->mash()->mashSteps();
   size = instructions.size();
   for( i = 0; i < size; ++i )
   {
      QString stepTime, tmp;
      QList<QString> reagents;

      Instruction* ins = instructions[i];

      if (ins->interval() > 0.0 )
         stepTime = Brewtarget::displayAmount(ins->interval(), &Units::minutes, 0);
      else
         stepTime = "--";

      tmp = "";

      // TODO: comparing ins->name() with these untranslated strings means this
      // doesn't work in other languages. Find a better way.
      if ( ins->name() == tr("Add grains") )
         reagents = recObs->getReagents( recObs->fermentables() );
      else if ( ins->name() == tr("Heat water") )
         reagents = recObs->getReagents( recObs->mash()->mashSteps() );
      else
         reagents = ins->reagents();

      if ( reagents.size() > 1 )
      {
         tmp = QString("<ul>");
         for ( j = 0; j < reagents.size(); j++ )
         {
            tmp += QString("<li>%1</li>")
                   .arg(reagents.at(j));
         }
         tmp += QString("</ul>");
      }
      else if ( reagents.size() == 1 )
      {
         tmp = reagents.at(0);
      }
      else
      {
         tmp = ins->directions();
      }

      QString altTag = i % 2 ? "alt" : "norm";

      middle += QString("<tr class=\"%1\"><td class=\"check\"></td><td class=\"time\">%2</td><td align=\"step\">%3 : %4</td></tr>")
               .arg(altTag)
               .arg(stepTime)
               .arg(ins->name())
               .arg(tmp);
   }
   middle += "</table>";

   return middle;
}


/**
 * @brief Create a list of string-lists that contain the instructions on how to brew the recipe.
 *
 * @return QList<QStringList>
 */
QList<QStringList> BrewDayFormatter::buildInstructionList()
{
   QList<QStringList> ret;

   QStringList row;
   int i, size;

   row.append(tr("Completed"));
   row.append(tr("Time"));
   row.append(tr("Step"));
   ret.append(row);
   row.clear();

   QList<Instruction*> instructions = recObs->instructions();
   QList<MashStep*> mashSteps = recObs->mash()->mashSteps();
   size = instructions.size();
   for( i = 0; i < size; ++i )
   {
      QString stepTime, tmp;
      QList<QString> reagents;

      Instruction* ins = instructions[i];

      if (ins->interval() > 0.0 )
         stepTime = Brewtarget::displayAmount(ins->interval(), &Units::minutes, 0);
      else
         stepTime = "--";

      // TODO: comparing ins->name() with these untranslated strings means this
      // doesn't work in other languages. Find a better way.
      if ( ins->name() == tr("Add grains") )
         reagents = recObs->getReagents( recObs->fermentables() );
      else if ( ins->name() == tr("Heat water") )
         reagents = recObs->getReagents( recObs->mash()->mashSteps() );
      else
         reagents = ins->reagents();

      tmp = "";
      if ( reagents.size() > 0 )
      {
         foreach(QString reagent, reagents)
         {
            tmp += QString("\t%1\n").arg(reagent);
         }
      }
      else
      {
         tmp = ins->directions();
      }

      row.append(stepTime);
      row.append(QString("%1 : %2").arg(ins->name()).arg(tmp));
      ret.append(row);
      row.clear();
   }

   return ret;
}

/**
 * @brief Builds and returns the Boil notes section for the bottom of the HTML page.
 *
 * @return QString
 */
QString BrewDayFormatter::buildFooterHTML()
{
   QString bottom;

   bottom = QString("<table id=\"notes\">");
   bottom += QString("<tr><td class=\"left\">%1:</td><td class=\"value\"></td><td class=\"right\">%2:</td><td class=\"value\"></td></tr>")
         .arg(tr("Actual PreBoil Volume"))
         .arg(tr("Actual PreBoil Gravity"));

   bottom += QString("<tr><td class=\"left\">%1:</td><td class=\"value\"></td><td class=\"right\">%2:</td><td class=\"value\"></td></tr>")
         .arg(tr("PostBoil Volume"))
         .arg(tr("PostBoil Gravity"));

   bottom += QString("<tr><td class=\"left\">%1:</td><td class=\"value\"></tr>")
         .arg(tr("Volume into fermenter"));
   bottom += "</table>";

   return bottom;
}
