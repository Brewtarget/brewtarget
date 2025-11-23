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
 =====================================================================================================================*/
#include "StockFormatter.h"

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
#include "model/Misc.h"
#include "model/StockPurchaseFermentable.h"
#include "model/StockPurchaseHop.h"
#include "model/StockPurchaseMisc.h"
#include "model/StockPurchaseYeast.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"

namespace {
   /**
    * @brief Create Stock HTML Header
    *
    * @return QString
    */
   QString createStockHeader() {
      return Html::createHeader(QObject::tr("Stock"), ":css/stock.css") +
            QString("<h1>%1 &mdash; %2</h1>")
                  .arg(QObject::tr("Stock"))
                  .arg(Localization::displayDateUserFormated(QDate::currentDate()));
   }

   /**
    * \brief Create Stock HTML Table of \c Fermentable
    */
   QString createStockTableFermentable() {
      QString result;

      // Find all the parent Fermentables whose stock is > 0
      auto fermentableStock = ObjectStoreWrapper::findAllMatching<StockPurchaseFermentable>(
         [](std::shared_ptr<StockPurchaseFermentable> val) { return val->quantity() > 0.0; }
      );
      if (!fermentableStock.empty()) {
         result += QString("<h2>%1</h2>").arg(QObject::tr("Fermentables"));
         result += "<table id=\"fermentables\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

         for (auto ii : fermentableStock) {
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
    * \brief Create Stock HTML Table of \c Hop
    */
   QString createStockTableHop() {
      QString result;

      auto hopStock = ObjectStoreWrapper::findAllMatching<StockPurchaseHop>(
         [](std::shared_ptr<StockPurchaseHop> val) { return val->quantity() > 0.0; }
      );

      if (!hopStock.empty()) {
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

         for (auto ii : hopStock) {
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
    * \brief Create Stock HTML Table of \c Misc
    */
   QString createStockTableMiscellaneous() {
      QString result;

      auto stockMisc = ObjectStoreWrapper::findAllMatching<StockPurchaseMisc>(
         [](std::shared_ptr<StockPurchaseMisc> val) { return val->quantity() > 0.0; }
      );
      if (!stockMisc.empty()) {

         result += QString("<h2>%1</h2>").arg(QObject::tr("Miscellaneous"));
         result += "<table id=\"misc\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

         for (auto ii : stockMisc) {
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
    * \brief Create Stock HTML Table of \c Yeast
    */
   QString createStockTableYeast() {
      QString result;
      auto stockYeast = ObjectStoreWrapper::findAllMatching<StockPurchaseYeast>(
         [](std::shared_ptr<StockPurchaseYeast> val) { return val->quantity() > 0.0; }
      );
      if (!stockYeast.empty()) {
         result += QString("<h2>%1</h2>").arg(QObject::tr("Yeast"));
         result += "<table id=\"yeast\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

         for (auto ii : stockYeast) {

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
    * Create Stock HTML Body
    */
   QString createStockBody(StockFormatter::HtmlGenerationFlags flags) {
      // Only generate users selection of Ingredient stock.
      QString result =
         ((flags & StockFormatter::HtmlGenerationFlag::FERMENTABLES ) ? createStockTableFermentable() : "") +
         ((flags & StockFormatter::HtmlGenerationFlag::HOPS         ) ? createStockTableHop() : "") +
         ((flags & StockFormatter::HtmlGenerationFlag::MISCELLANEOUS) ? createStockTableMiscellaneous() : "") +
         ((flags & StockFormatter::HtmlGenerationFlag::YEAST        ) ? createStockTableYeast() : "");

      // If user selects no printout or if there are no stock for the selected ingredients
      if (result.size() == 0) {
         result = QObject::tr("No stock available.");
      }

      return result;
   }

   /**
    * Create Stock HTML Footer
    */
   QString createStockFooter() {
      return Html::createFooter();
   }

}

QString StockFormatter::createStockHtml(HtmlGenerationFlags flags) {
   return createStockHeader() +
          createStockBody(flags) +
          createStockFooter();
}
