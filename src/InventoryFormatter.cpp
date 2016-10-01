/*
 * InventoryFormatter.cpp is part of Brewtarget, and was written by
 * Mark de Wever (koraq@xs4all.nl), copyright 2016
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

#include "InventoryFormatter.h"

#include "Html.h"
#include "MainWindow.h"
#include "brewtarget.h"
#include "database.h"

#include <QDate>
#include <QDialog>
#include <QTextBrowser>
#include <QVBoxLayout>

namespace InventoryFormatter
{

static QString createInventoryHeader()
{
   return Html::createHeader(QObject::tr("Inventory"), ":css/inventory.css") +
          QString("<h1>%1 &mdash; %2</h1>")
                .arg(QObject::tr("Inventory"))
                .arg(Brewtarget::displayDateUserFormated(QDate::currentDate()));
}

static QString createInventoryTableFermentable()
{
   QString result;

   const QMap<int, double> inventory =
         Database::instance().getInventory(Brewtarget::FERMTABLE);

   if (!inventory.empty())
   {

      result += QString("<h2>%1</h2>").arg(QObject::tr("Fermentables"));
      result += "<table id=\"fermentables\">";
      result += QString("<tr>"
                        "<th align=\"left\" width=\"40%\">%1</th>"
                        "<th align=\"left\" width=\"60%\">%2</th>"
                        "</tr>")
                      .arg(QObject::tr("Name"))
                      .arg(QObject::tr("Amount"));

      for (auto itor = inventory.begin(); itor != inventory.end(); ++itor)
      {
         const Fermentable* fermentable =
               Database::instance().fermentable(itor.key());

         if (!fermentable)
         {
            Brewtarget::logE(QString("The fermentable %1 has a record in the "
                                     "inventory, but does not exist.")
                                   .arg(itor.key()));
            continue;
         }

         result += QString("<tr>"
                           "<td>%1</td>"
                           "<td>%2</td>"
                           "</tr>")
                         .arg(fermentable->name())
                         .arg(Brewtarget::displayAmount(itor.value(),
                               "fermentableTable", "inventory_kg",
                               Units::kilograms));
      }
      result += "</table>";
   }
   return result;
}

static QString createInventoryTableHop()
{
   QString result;
   const QMap<int, double> inventory =
         Database::instance().getInventory(Brewtarget::HOPTABLE);

   if (!inventory.empty())
   {

      result += QString("<h2>%1</h2>").arg(QObject::tr("Hops"));
      result += "<table id=\"hops\">";
      result += QString("<tr>"
                        "<th align=\"left\" width=\"40%\">%1</th>"
                        "<th align=\"left\" width=\"60%\">%2</th>"
                        "</tr>")
                      .arg(QObject::tr("Name"))
                      .arg(QObject::tr("Amount"));

      for (auto itor = inventory.begin(); itor != inventory.end(); ++itor)
      {
         const Hop* hop = Database::instance().hop(itor.key());

         if (!hop)
         {
            Brewtarget::logE(QString("The hop %1 has a record in the "
                                     "inventory, but does not exist.")
                                   .arg(itor.key()));
            continue;
         }

         result += QString("<tr>"
                           "<td>%1</td>"
                           "<td>%2</td>"
                           "</tr>")
                         .arg(hop->name())
                         .arg(Brewtarget::displayAmount(itor.value(),
                               "hopTable", "inventory_kg", Units::kilograms));
      }
      result += "</table>";
   }
   return result;
}

static QString createInventoryTableMiscellaneous()
{
   QString result;
   const QMap<int, double> inventory =
         Database::instance().getInventory(Brewtarget::MISCTABLE);

   if (!inventory.empty())
   {

      result += QString("<h2>%1</h2>").arg(QObject::tr("Miscellaneous"));
      result += "<table id=\"misc\">";
      result += QString("<tr>"
                        "<th align=\"left\" width=\"40%\">%1</th>"
                        "<th align=\"left\" width=\"60%\">%2</th>"
                        "</tr>")
                      .arg(QObject::tr("Name"))
                      .arg(QObject::tr("Amount"));

      for (auto itor = inventory.begin(); itor != inventory.end(); ++itor)
      {
         const Misc* miscellaneous = Database::instance().misc(itor.key());

         if (!miscellaneous)
         {
            Brewtarget::logE(QString("The miscellaneous %1 has a record in the "
                                     "inventory, but does not exist.")
                                   .arg(itor.key()));
            continue;
         }

         const QString displayAmount =
               Brewtarget::displayAmount(itor.value(), "miscTable", "amount",
                     miscellaneous->amountIsWeight() ? (Unit*)Units::kilograms
                                                     : (Unit*)Units::liters);
         result += QString("<tr>"
                           "<td>%1</td>"
                           "<td>%2</td>"
                           "</tr>")
                         .arg(miscellaneous->name())
                         .arg(displayAmount);
      }
      result += "</table>";
   }
   return result;
}

static QString createInventoryTableYeast()
{
   QString result;
   const QMap<int, double> inventory =
         Database::instance().getInventory(Brewtarget::YEASTTABLE);

   if (!inventory.empty())
   {
      result += QString("<h2>%1</h2>").arg(QObject::tr("Yeast"));
      result += "<table id=\"yeast\">";
      result += QString("<tr>"
                        "<th align=\"left\" width=\"40%\">%1</th>"
                        "<th align=\"left\" width=\"60%\">%2</th>"
                        "</tr>")
                      .arg(QObject::tr("Name"))
                      .arg(QObject::tr("Amount"));

      for (auto itor = inventory.begin(); itor != inventory.end(); ++itor)
      {
         const Yeast* yeast = Database::instance().yeast(itor.key());

         if (!yeast)
         {
            Brewtarget::logE(QString("The yeast %1 has a record in the "
                                     "inventory, but does not exist.")
                                   .arg(itor.key()));
            continue;
         }

         const QString displayAmount =
               Brewtarget::displayAmount(itor.value(), "yeastTable", "quanta",
                     yeast->amountIsWeight() ? (Unit*)Units::kilograms
                                             : (Unit*)Units::liters);

         result += QString("<tr>"
                           "<td>%1</td>"
                           "<td>%2</td>"
                           "</tr>")
                         .arg(yeast->name())
                         .arg(displayAmount);
      }
      result += "</table>";
   }
   return result;
}

static QString createInventoryBody()
{
   QString result =
         createInventoryTableFermentable() + createInventoryTableHop() +
         createInventoryTableMiscellaneous() + createInventoryTableYeast();

   if (result.size() == 0)
   {
      result = QObject::tr("No inventory available.");
   }

   return result;
}

static QString createInventoryFooter()
{
   return Html::createFooter();
}

static QString createInventory()
{
   return createInventoryHeader() + createInventoryBody() +
          createInventoryFooter();
}

static std::tuple<QDialog*, QTextBrowser*>& previewDialogue()
{
   static auto result = []() -> std::tuple<QDialog*, QTextBrowser*> {
      QDialog* dialog = new QDialog(Brewtarget::mainWindow());
      dialog->setWindowTitle(QObject::tr("Print Preview"));
      if (!dialog->layout())
      {
         dialog->setLayout(new QVBoxLayout);
      }
      QTextBrowser* text = new QTextBrowser(dialog);
      dialog->layout()->addWidget(text);
      return std::make_tuple(dialog, text);
   }();

   return result;
}

void printPreview()
{
   auto& dialogue = previewDialogue();
   std::get<1>(dialogue)->setHtml(createInventory());
   std::get<0>(dialogue)->show();
}

void print(QPrinter* printer)
{
   QTextBrowser& text = *std::get<1>(previewDialogue());
   text.setHtml(createInventory());
   text.print(printer);
}

void exportHTML(QFile* file)
{
   QTextStream(file) << createInventory();
}

} // InventoryFormatter

