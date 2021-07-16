/*
 * InventoryFormatter.cpp is part of Brewtarget, and is Copyright the following
 * authors 2016-2021
 * - Mark de Wever <koraq@xs4all.nl>
 * - Matt Young <mfsy@yahoo.com>
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

namespace InventoryFormatter
{
   /**
    * @brief Create Inventory HTML Header
    *
    * @return QString
    */
   static QString createInventoryHeaderHTML()
   {
      return Html::createHeader(QObject::tr("Inventory"), ":css/inventory.css") +
            QString("<h1>%1 &mdash; %2</h1>")
                  .arg(QObject::tr("Inventory"))
                  .arg(Brewtarget::displayDateUserFormated(QDate::currentDate()));
   }

   /**
    * @brief Create Inventory HTML Table Fermentables
    *
    * @return QString
    */
   static QString createInventoryTableFermentableHTML()
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
               qCritical() << QString("The fermentable %1 has a record in the "
                                       "inventory, but does not exist.")
                                    .arg(itor.key());
               continue;
            }

            result += QString("<tr>"
                              "<td>%1</td>"
                              "<td>%2</td>"
                              "</tr>")
                           .arg(fermentable->name())
                           .arg(Brewtarget::displayAmount(itor.value(),
                                 "fermentableTable", "inventory_kg",
                                 &Units::kilograms));
         }
         result += "</table>";
      }
      return result;
   }

   /**
    * @brief Create Inventory HTML Table of Hops
    *
    * @return QString
    */
   static QString createInventoryTableHopHTML()
   {
      QString result;
      const QMap<int, double> inventory =
            Database::instance().getInventory(Brewtarget::HOPTABLE);

      if (!inventory.empty())
      {

         result += QString("<h2>%1</h2>").arg(QObject::tr("Hops"));
         result += "<table id=\"hops\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"30%\">%1</th>"
                           "<th align=\"left\" width=\"20%\">%2</th>"
                           "<th align=\"left\" width=\"50%\">%3</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Alpha %"))
                        .arg(QObject::tr("Amount"));

         for (auto itor = inventory.begin(); itor != inventory.end(); ++itor)
         {
            const Hop* hop = Database::instance().hop(itor.key());

            if (!hop)
            {
               qCritical() << QString("The hop %1 has a record in the "
                                       "inventory, but does not exist.")
                                    .arg(itor.key());
               continue;
            }

            result += QString("<tr>"
                              "<td>%1</td>"
                              "<td>%2</td>"
                              "<td>%3</td>"
                              "</tr>")
                           .arg(hop->name())
                           .arg(hop->alpha_pct())
                           .arg(Brewtarget::displayAmount(itor.value(),
                                 "hopTable", "inventory_kg", &Units::kilograms));
         }
         result += "</table>";
      }
      return result;
   }

   /**
    * @brief Create Inventory HTML Table of Miscellaneous
    *
    * @return QString
    */
   static QString createInventoryTableMiscellaneousHTML()
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
               qCritical() << QString("The miscellaneous %1 has a record in the "
                                       "inventory, but does not exist.")
                                    .arg(itor.key());
               continue;
            }

            const QString displayAmount =
                  Brewtarget::displayAmount(itor.value(), "miscTable", "amount",
                        miscellaneous->amountIsWeight() ? (Unit const *)&Units::kilograms
                                                      : (Unit const *)&Units::liters);
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


   /**
    * @brief Create Inventory HTML Table Yeasts
    *
    * @return QString
    */
   static QString createInventoryTableYeastHTML()
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
               qCritical() << QString("The yeast %1 has a record in the "
                                       "inventory, but does not exist.")
                                    .arg(itor.key());
               continue;
            }

            const QString displayAmount =
                  Brewtarget::displayAmount(itor.value(), "yeastTable", "quanta",
                        yeast->amountIsWeight() ? (Unit const *)&Units::kilograms
                                                : (Unit const *)&Units::liters);

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

   /**
    * @brief Create a Inventory H T M L Body object
    *
    * @param flags HTMLgeneretionFlags stacked to generate
    * @return QString
    */
   static QString createInventoryHTMLBody(HTMLgenerationFlags flags)
   {
      //Only generate users selection of Ingredient inventory.
      QString result =
            ((HTMLgenerationFlags::FERMENTABLESFLAG  & flags) ? createInventoryTableFermentableHTML() : "") +
            ((HTMLgenerationFlags::HOPSFLAG          & flags) ? createInventoryTableHopHTML() : "") +
            ((HTMLgenerationFlags::MISCELLANEOUSFLAG & flags) ? createInventoryTableMiscellaneousHTML() : "") +
            ((HTMLgenerationFlags::YEASTFLAG         & flags) ? createInventoryTableYeastHTML() : "");
      //If users selects no printout or if there are no inventory for the selected ingredients.
      if (result.size() == 0)
      {
         result = QObject::tr("No inventory available.");
      }

      return result;
   }

   /**
    * @brief Create Inventory HTML Footer
    *
    * @return QString
    */
   static QString createInventoryFooterHTML()
   {
      return Html::createFooter();
   }

   /**
    * @brief Create Inventory HTML string
    *
    * @return QString
    */
   QString createInventoryHTML(HTMLgenerationFlags flags)
   {
      return createInventoryHeaderHTML() +
               createInventoryHTMLBody(flags) +
               createInventoryFooterHTML();
   }


   /**
    * @brief These implementations are to get the specific entity with the supplies ID.
    *
    * @param id
    * @return templates Type pointer.
    */
   template <> Yeast* getEntity(int id) { return Database::instance().yeast(id); }
   template <> Hop* getEntity(int id) { return Database::instance().hop(id); }
   template <> Fermentable* getEntity(int id) { return Database::instance().fermentable(id); }
   template <> Misc* getEntity(int id) { return Database::instance().misc(id); }

   template<> const QStringList getTableRow(Hop *obj, double value)
   {
      return QStringList()
         << obj->name()
         << QString("%1").arg(obj->alpha_pct())
         << Brewtarget::displayAmount(value, "hopTable", "inventory_kg", &Units::kilograms);
   }

   template<> const QStringList getTableRow(Fermentable *obj, double value)
   {
      return QStringList()
         << obj->name()
         << Brewtarget::displayAmount(value, "fermentableTable", "inventory_kg", &Units::kilograms);
   }

   template<> const QStringList getTableRow(Yeast *obj, double value)
   {
      const QString displayAmount =
                  Brewtarget::displayAmount(value, "yeastTable", "quanta",
                        obj->amountIsWeight() ? (Unit const *)&Units::kilograms
                                                : (Unit const *)&Units::liters);

      return QStringList() << obj->name() << displayAmount;
   }

   template<> const QStringList getTableRow(Misc *obj, double value)
   {
      const QString displayAmount =
                  Brewtarget::displayAmount(value, "miscTable", "amount",
                        obj->amountIsWeight() ? (Unit const *)&Units::kilograms
                                                      : (Unit const *)&Units::liters);
      return QStringList() << obj->name() << displayAmount;
   }

} // InventoryFormatter

