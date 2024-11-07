/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BrewDayFormatter.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "BrewDayFormatter.h"

#include <QList>
#include <QStringList>

#include "Html.h"
#include "measurement/Measurement.h"
#include "model/Equipment.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/Style.h"
#include "PersistentSettings.h"

BrewDayFormatter::BrewDayFormatter(QObject * parent)
   : QObject(parent) {
   recObs = nullptr;
}

void BrewDayFormatter::setRecipe(Recipe * recipe) {
   recObs = recipe;
}

QString BrewDayFormatter::buildHtml() {
   return buildTitleHtml() + buildInstructionHtml() + buildFooterHtml();
}

QString BrewDayFormatter::buildTitleHtml(bool includeImage) {

   // Do the style sheet first
   if (cssName == nullptr) {
      cssName = ":/css/brewday.css";
   }

   QString header = Html::createHeader(tr("Brewday"), cssName);

   QString body = QString("<h1>%1</h1>").arg(recObs->name());
   if (includeImage) {
      body += QString("<img src=\"%1\" />").arg("qrc:/images/title.svg");
   }

   // Build the top table
   // Build the first row: Style and Date
   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td>")
           .arg(tr("Style"));
   body += QString("<td class=\"value\">%1</td>")
           .arg((recObs->style()) ? recObs->style()->name() : "unknown");
   body += QString("<td class=\"right\">%1</td>")
           .arg(tr("Date"));
   body += QString("<td class=\"value\">%1</td></tr>")
           .arg(QDate::currentDate().toString());

   // second row:  boil time and efficiency.
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Boil Time"))
           .arg(
              recObs->equipment() ? Measurement::displayAmount(Measurement::Amount{recObs->equipment()->boilTime_min().value_or(Equipment::default_boilTime_mins),
                                                                                   Measurement::Units::minutes}) : "unknown"
           )
           .arg(tr("Efficiency"))
           .arg(Measurement::displayQuantity(recObs->efficiency_pct(), 0));

   // third row: pre-Boil Volume and Preboil Gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Boil Volume"))
           .arg(Measurement::displayAmount(Measurement::Amount{recObs->boilVolume_l(), Measurement::Units::liters}, 2))
           .arg(tr("Preboil Gravity"))
           .arg(Measurement::displayAmount(Measurement::Amount{recObs->boilGrav(), Measurement::Units::specificGravity}, 3));

   // fourth row: Final volume and starting gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Final Volume"))
           .arg(Measurement::displayAmount(Measurement::Amount{recObs->finalVolume_l(), Measurement::Units::liters}, 2))
           .arg(tr("Starting Gravity"))
           .arg(Measurement::displayAmount(Measurement::Amount{recObs->og(), Measurement::Units::specificGravity}, 3));

   // fifth row: IBU and Final gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
           .arg(tr("IBU"))
           .arg(Measurement::displayQuantity(recObs->IBU(), 1))
           .arg(tr("Final Gravity"))
           .arg(Measurement::displayAmount(Measurement::Amount{recObs->fg(), Measurement::Units::specificGravity}, 3));

   // sixth row: ABV and estimate calories
   bool metricVolume =
      Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Volume) ==
      Measurement::UnitSystems::volume_Metric;
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
           .arg(tr("ABV"))
           .arg(Measurement::displayQuantity(recObs->ABV_pct(), 1))
           .arg(metricVolume ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
           .arg(Measurement::displayQuantity(metricVolume ? recObs->caloriesPer33cl() : recObs->caloriesPerUs12oz(), 0));

   body += "</table>";

   return header + body;
}

QList<QStringList> BrewDayFormatter::buildTitleList() {
   QList<QStringList> ret;

   if (! recObs) {
      return ret;
   }

   QString body = "";
   QStringList row;
   row.append(tr("Style"));
   row.append((recObs->style()) ? recObs->style()->name() : "unknown");
   row.append(tr("Date"));
   row.append(QDate::currentDate().toString());
   ret.append(row);
   row.clear();

   // second row:  boil time and efficiency.
   row.append(tr("Boil Time"));
   row.append(
      recObs->equipment() ? Measurement::displayAmount(Measurement::Amount{recObs->equipment()->boilTime_min().value_or(Equipment::default_boilTime_mins),
                                                                           Measurement::Units::minutes}) : "unknown"
   );
   row.append(tr("Efficiency"));
   row.append(Measurement::displayQuantity(recObs->efficiency_pct(), 0));
   ret.append(row);
   row.clear();

   // third row: pre-Boil Volume and Preboil Gravity
   row.append(tr("Boil Volume"));
   row.append(Measurement::displayAmount(Measurement::Amount{recObs->boilVolume_l(), Measurement::Units::liters}, 2));
   row.append(tr("Preboil Gravity"));
   row.append(Measurement::displayAmount(Measurement::Amount{recObs->boilGrav(), Measurement::Units::specificGravity}, 3));
   ret.append(row);
   row.clear();
   ret.append(row);
   row.clear();

   // fourth row: Final volume and starting gravity
   row.append(tr("Final Volume"));
   row.append(Measurement::displayAmount(Measurement::Amount{recObs->finalVolume_l(), Measurement::Units::liters}, 2));
   row.append(tr("Starting Gravity"));
   row.append(Measurement::displayAmount(Measurement::Amount{recObs->og(), Measurement::Units::specificGravity}, 3));
   ret.append(row);
   row.clear();

   // fifth row: IBU and Final gravity
   row.append(tr("IBU"));
   row.append(Measurement::displayQuantity(recObs->IBU(), 1));
   row.append(tr("Final Gravity"));
   row.append(Measurement::displayAmount(Measurement::Amount{recObs->fg(), Measurement::Units::specificGravity}, 3));
   ret.append(row);
   row.clear();

   // sixth row: ABV and estimate calories
   row.append(tr("ABV"));
   row.append(Measurement::displayQuantity(recObs->ABV_pct(), 1));
   bool metricVolume =
      Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Volume) ==
      Measurement::UnitSystems::volume_Metric;

   row.append(metricVolume ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"));
   row.append(Measurement::displayQuantity(metricVolume ? recObs->caloriesPer33cl() : recObs->caloriesPerUs12oz(), 0));
   ret.append(row);
   row.clear();

   return ret;
}

/**
 * @brief Builds the InstructionsTable in HTML and returns a string with the content.
 *
 * @return QString
 */
QString BrewDayFormatter::buildInstructionHtml() {
   QString middle = QString("<h2>%1</h2>").arg(tr("Instructions"));
   middle += QString("<table id=\"steps\">");
   middle += QString("<tr><th class=\"check\">%1</th><th class=\"time\">%2</th><th class=\"step\">%3</th></tr>")
             .arg(tr("Completed"))
             .arg(tr("Time"))
             .arg(tr("Step"));

   bool useAlt = true;
   for (auto instruction : recObs->instructions()) {
      useAlt = !useAlt;
      QString stepTime, tmp;
      QList<QString> reagents;

      if (instruction->interval() > 0.0) {
         stepTime = Measurement::displayAmount(Measurement::Amount{instruction->interval(), Measurement::Units::minutes}, 0);
      } else {
         stepTime = "--";
      }

      tmp = "";

      // TODO: comparing instruction->name() with these untranslated strings means this
      // doesn't work in other languages. Find a better way.
      if (instruction->name() == tr("Add grains")) {
         reagents = recObs->getReagents(recObs->fermentableAdditions());
      } else if (instruction->name() == tr("Heat water")) {
         if (recObs->mash()) {
            reagents = recObs->getReagents(recObs->mash()->mashSteps());
         }
      } else {
         reagents = instruction->reagents();
      }

      if (reagents.size() > 1) {
         tmp = QString("<ul>");
         for (int j = 0; j < reagents.size(); j++) {
            tmp += QString("<li>%1</li>")
                   .arg(reagents.at(j));
         }
         tmp += QString("</ul>");
      } else if (reagents.size() == 1) {
         tmp = reagents.at(0);
      } else {
         tmp = instruction->directions();
      }

      QString altTag = useAlt ? "alt" : "norm";

      middle += QString("<tr class=\"%1\"><td class=\"check\"></td><td class=\"time\">%2</td><td align=\"step\">%3 : %4</td></tr>")
                .arg(altTag)
                .arg(stepTime)
                .arg(instruction->name())
                .arg(tmp);
   }
   middle += "</table>";

   return middle;
}

QList<QStringList> BrewDayFormatter::buildInstructionList() {
   QList<QStringList> ret;

   QStringList row;

   row.append(tr("Completed"));
   row.append(tr("Time"));
   row.append(tr("Step"));
   ret.append(row);
   row.clear();

   for (auto instruction : recObs->instructions()) {
      QString stepTime, tmp;
      QList<QString> reagents;

      if (instruction->interval() > 0.0) {
         stepTime = Measurement::displayAmount(Measurement::Amount{instruction->interval(), Measurement::Units::minutes}, 0);
      } else {
         stepTime = "--";
      }

      // TODO: comparing instruction->name() with these untranslated strings means this
      // doesn't work in other languages. Find a better way.
      if (instruction->name() == tr("Add grains")) {
         reagents = recObs->getReagents(recObs->fermentableAdditions());
      } else if (instruction->name() == tr("Heat water")) {
         if (recObs->mash()) {
            reagents = recObs->getReagents(recObs->mash()->mashSteps());
         }
      } else {
         reagents = instruction->reagents();
      }

      tmp = "";
      if (reagents.size() > 0) {
         for (QString reagent : reagents) {
            tmp += QString("\t%1\n").arg(reagent);
         }
      } else {
         tmp = instruction->directions();
      }

      row.append(stepTime);
      row.append(QString("%1 : %2").arg(instruction->name()).arg(tmp));
      ret.append(row);
      row.clear();
   }

   return ret;
}

QString BrewDayFormatter::buildFooterHtml() {
   QString bottom = QString("<table id=\"notes\">");
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
