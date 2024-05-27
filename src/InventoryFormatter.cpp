/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * InventoryFormatter.cpp is part of Brewtarget, and is copyright the following authors 2016-2024:
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
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
#include "InventoryFormatter.h"

#include <QList>
#include <QMap>
#include <QStringList>

#include "database/ObjectStoreWrapper.h"
#include "Html.h"
#include "Localization.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/InventoryFermentable.h"
#include "model/InventoryHop.h"
#include "model/InventoryMisc.h"
#include "model/InventoryYeast.h"
#include "model/Misc.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"

namespace {
   /**
    * @brief Create Inventory HTML Header
    *
    * @return QString
    */
   QString createInventoryHeader() {
      return Html::createHeader(QObject::tr("Inventory"), ":css/inventory.css") +
            QString("<h1>%1 &mdash; %2</h1>")
                  .arg(QObject::tr("Inventory"))
                  .arg(Localization::displayDateUserFormated(QDate::currentDate()));
   }

   /**
    * \brief Create Inventory HTML Table of \c Fermentable
    */
   QString createInventoryTableFermentable() {
      QString result;

      // Find all the parent Fermentables whose inventory is > 0
      // (We don't want children because they are just usages of the parents in recipes.)
///      auto inventory = ObjectStoreWrapper::findAllMatching<Fermentable>(
///         [](std::shared_ptr<Fermentable> ff) { return (ff->getParent() == nullptr && ff->inventory() > 0.0); }
///      );
      auto fermentableInventory = ObjectStoreWrapper::findAllMatching<InventoryFermentable>(
         [](std::shared_ptr<InventoryFermentable> val) { return val->quantity() > 0.0; }
      );
      if (!fermentableInventory.empty()) {
         result += QString("<h2>%1</h2>").arg(QObject::tr("Fermentables"));
         result += "<table id=\"fermentables\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

///         for (auto fermentable : inventory) {
///            result += QString("<tr>"
///                              "<td>%1</td>"
///                              "<td>%2</td>"
///                              "</tr>")
///                           .arg(fermentable->name())
///                           .arg(Measurement::displayAmount(Measurement::Amount{fermentable->inventory(),
///                                                                               Measurement::Units::kilograms}));
///         }
         for (auto ii : fermentableInventory) {
            result += QString("<tr>"
                              "<td>%1</td>"
                              "<td>%2</td>"
                              "</tr>")
                           .arg(ii->fermentable()->name())
                           .arg(Measurement::displayAmount(ii->amount()));
         }
         result += "</table>";
      }
      return result;
   }

   /**
    * \brief Create Inventory HTML Table of \c Hop
    */
   QString createInventoryTableHop() {
      QString result;

///      auto inventory = ObjectStoreWrapper::findAllMatching<Hop>(
///         [](std::shared_ptr<Hop> hh) { return (hh->getParent() == nullptr && hh->inventory() > 0.0); }
///      );
      auto hopInventory = ObjectStoreWrapper::findAllMatching<InventoryHop>(
         [](std::shared_ptr<InventoryHop> val) { return val->quantity() > 0.0; }
      );

      if (!hopInventory.empty()) {
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

///         for (auto hop : inventory) {
///            result += QString("<tr>"
///                              "<td>%1</td>"
///                              "<td>%2</td>"
///                              "<td>%3</td>"
///                              "</tr>")
///                           .arg(hop->name())
///                           .arg(hop->alpha_pct())
///                           .arg(Measurement::displayAmount(Measurement::Amount{hop->inventory(),
///                                                                               Measurement::Units::kilograms}));
///         }
         for (auto ii : hopInventory) {
            result += QString("<tr>"
                              "<td>%1</td>"
                              "<td>%2</td>"
                              "<td>%3</td>"
                              "</tr>")
                           .arg(ii->hop()->name())
                           .arg(ii->hop()->alpha_pct())
                           .arg(Measurement::displayAmount(ii->amount()));
         }
         result += "</table>";
      }
      return result;
   }

   /**
    * \brief Create Inventory HTML Table of \c Misc
    */
   QString createInventoryTableMiscellaneous() {
      QString result;

///      auto inventory = ObjectStoreWrapper::findAllMatching<Misc>(
///         [](std::shared_ptr<Misc> mm) { return (mm->getParent() == nullptr && mm->inventory() > 0.0); }
///      );
      auto inventoryMisc = ObjectStoreWrapper::findAllMatching<InventoryMisc>(
         [](std::shared_ptr<InventoryMisc> val) { return val->quantity() > 0.0; }
      );
      if (!inventoryMisc.empty()) {

         result += QString("<h2>%1</h2>").arg(QObject::tr("Miscellaneous"));
         result += "<table id=\"misc\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

         for (auto ii : inventoryMisc) {
///            QString const displayAmount = Measurement::displayAmount(
///               Measurement::Amount{
///                  ii->inventory(),
///                  ii->amountIsWeight() ? Measurement::Units::kilograms : Measurement::Units::liters
///               }
///            );
            result += QString("<tr>"
                              "<td>%1</td>"
                              "<td>%2</td>"
                              "</tr>")
                           .arg(ii->name())
                           .arg(Measurement::displayAmount(ii->amount()));
         }
         result += "</table>";
      }
      return result;
   }

   /**
    * \brief Create Inventory HTML Table of \c Yeast
    */
   QString createInventoryTableYeast() {
      QString result;
///      auto inventory = ObjectStoreWrapper::findAllMatching<Yeast>(
///         [](std::shared_ptr<Yeast> yy) { return (yy->getParent() == nullptr && yy->inventory() > 0.0); }
///      );
      auto inventoryYeast = ObjectStoreWrapper::findAllMatching<InventoryYeast>(
         [](std::shared_ptr<InventoryYeast> val) { return val->quantity() > 0.0; }
      );
      if (!inventoryYeast.empty()) {
         result += QString("<h2>%1</h2>").arg(QObject::tr("Yeast"));
         result += "<table id=\"yeast\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

         for (auto ii : inventoryYeast) {
///            QString const displayAmount = Measurement::displayAmount(
///               Measurement::Amount{
///                  yeast->inventory(),
///                  yeast->amountIsWeight() ? Measurement::Units::kilograms : Measurement::Units::liters
///               }
///            );

            result += QString("<tr>"
                              "<td>%1</td>"
                              "<td>%2</td>"
                              "</tr>")
                           .arg(ii->name())
                           .arg(Measurement::displayAmount(ii->amount()));
         }
         result += "</table>";
      }
      return result;
   }


   /**
    * Create Inventory HTML Body
    */
   QString createInventoryBody(InventoryFormatter::HtmlGenerationFlags flags) {
      // Only generate users selection of Ingredient inventory.
      QString result =
         ((flags & InventoryFormatter::HtmlGenerationFlag::FERMENTABLES ) ? createInventoryTableFermentable() : "") +
         ((flags & InventoryFormatter::HtmlGenerationFlag::HOPS         ) ? createInventoryTableHop() : "") +
         ((flags & InventoryFormatter::HtmlGenerationFlag::MISCELLANEOUS) ? createInventoryTableMiscellaneous() : "") +
         ((flags & InventoryFormatter::HtmlGenerationFlag::YEAST        ) ? createInventoryTableYeast() : "");

      // If user selects no printout or if there are no inventory for the selected ingredients
      if (result.size() == 0) {
         result = QObject::tr("No inventory available.");
      }

      return result;
   }

   /**
    * Create Inventory HTML Footer
    */
   QString createInventoryFooter() {
      return Html::createFooter();
   }

}

///InventoryFormatter::HtmlGenerationFlags InventoryFormatter::operator|(InventoryFormatter::HtmlGenerationFlags a,
///                                                                      InventoryFormatter::HtmlGenerationFlags b) {
///   return static_cast<HtmlGenerationFlags>(static_cast<int>(a) | static_cast<int>(b));
///}
///
///bool InventoryFormatter::operator&(InventoryFormatter::HtmlGenerationFlags a,
///                                   InventoryFormatter::HtmlGenerationFlags b) {
///   return (static_cast<int>(a) & static_cast<int>(b));
///}

QString InventoryFormatter::createInventoryHtml(HtmlGenerationFlags flags) {
   return createInventoryHeader() +
          createInventoryBody(flags) +
          createInventoryFooter();
}
