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

#include <QDate>
#include <QDialog>

#include "BtPrintPreview.h"
#include "Html.h"
#include "MainWindow.h"
#include "brewtarget.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Inventory.h"
#include "model/Misc.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"

namespace {
   QString createInventoryHeader() {
      return Html::createHeader(QObject::tr("Inventory"), ":css/inventory.css") +
            QString("<h1>%1 &mdash; %2</h1>")
                  .arg(QObject::tr("Inventory"))
                  .arg(Brewtarget::displayDateUserFormated(QDate::currentDate()));
   }

   /**
    * Fermentables
    */
   QString createInventoryTableFermentable() {
      QString result;

      // Find all the parent Fermentables whose inventory is > 0
      // (We don't want children because they are just usages of the parents in recipes.)
      auto inventory = ObjectStoreWrapper::findAllMatching<Fermentable>(
         [](std::shared_ptr<Fermentable> ff) { return (ff->getParent() == nullptr && ff->inventory() > 0.0); }
      );
      if (!inventory.empty()) {
         result += QString("<h2>%1</h2>").arg(QObject::tr("Fermentables"));
         result += "<table id=\"fermentables\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

         for (auto fermentable : inventory) {
            result += QString("<tr>"
                              "<td>%1</td>"
                              "<td>%2</td>"
                              "</tr>")
                           .arg(fermentable->name())
                           .arg(Brewtarget::displayAmount(fermentable->inventory(),
                                                       PersistentSettings::Sections::fermentableTable,
                                                       PropertyNames::NamedEntityWithInventory::inventory,
                                                       &Units::kilograms));
         }
         result += "</table>";
      }
      return result;
   }

   /**
    * Hops
    */
   QString createInventoryTableHop() {
      QString result;

      auto inventory = ObjectStoreWrapper::findAllMatching<Hop>(
         [](std::shared_ptr<Hop> hh) { return (hh->getParent() == nullptr && hh->inventory() > 0.0); }
      );
      if (!inventory.empty()) {

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

         for (auto hop : inventory) {
            result += QString("<tr>"
                              "<td>%1</td>"
                              "<td>%2</td>"
                              "<td>%3</td>"
                              "</tr>")
                           .arg(hop->name())
                           .arg(hop->alpha_pct())
                           .arg(Brewtarget::displayAmount(hop->inventory(),
                                                       PersistentSettings::Sections::hopTable,
                                                       PropertyNames::NamedEntityWithInventory::inventory,
                                                       &Units::kilograms));
         }
         result += "</table>";
      }
      return result;
   }

   /**
    * Misc
    */
   QString createInventoryTableMiscellaneous() {
      QString result;

      auto inventory = ObjectStoreWrapper::findAllMatching<Misc>(
         [](std::shared_ptr<Misc> mm) { return (mm->getParent() == nullptr && mm->inventory() > 0.0); }
      );
      if (!inventory.empty()) {

         result += QString("<h2>%1</h2>").arg(QObject::tr("Miscellaneous"));
         result += "<table id=\"misc\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

         for (auto miscellaneous : inventory) {
            const QString displayAmount =
                  Brewtarget::displayAmount(miscellaneous->inventory(),
                                         PersistentSettings::Sections::miscTable,
                                         PropertyNames::NamedEntityWithInventory::inventory,
                        miscellaneous->amountIsWeight() ? (Unit*)&Units::kilograms
                                                      : (Unit*)&Units::liters);
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
    * Yeast
    */
   QString createInventoryTableYeast() {
      QString result;
      auto inventory = ObjectStoreWrapper::findAllMatching<Yeast>(
         [](std::shared_ptr<Yeast> yy) { return (yy->getParent() == nullptr && yy->inventory() > 0.0); }
      );
      if (!inventory.empty()) {
         result += QString("<h2>%1</h2>").arg(QObject::tr("Yeast"));
         result += "<table id=\"yeast\">";
         result += QString("<tr>"
                           "<th align=\"left\" width=\"40%\">%1</th>"
                           "<th align=\"left\" width=\"60%\">%2</th>"
                           "</tr>")
                        .arg(QObject::tr("Name"))
                        .arg(QObject::tr("Amount"));

         for (auto yeast : inventory) {
            const QString displayAmount =
                  Brewtarget::displayAmount(yeast->inventory(),
                                         PersistentSettings::Sections::yeastTable,
                                         PropertyNames::NamedEntityWithInventory::inventory,
                        yeast->amountIsWeight() ? (Unit*)&Units::kilograms
                                                : (Unit*)&Units::liters);

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

   QString createInventoryBody() {
      QString result =
            createInventoryTableFermentable() + createInventoryTableHop() +
            createInventoryTableMiscellaneous() + createInventoryTableYeast();

      if (result.size() == 0) {
         result = QObject::tr("No inventory available.");
      }

      return result;
   }

   QString createInventoryFooter() {
      return Html::createFooter();
   }


   BtPrintPreview * dialog;

   void createOrUpdateDialog() {
      if (nullptr == dialog) {
         dialog = new BtPrintPreview(Brewtarget::mainWindow());
         dialog->setWindowTitle(QObject::tr("Inventory Print Preview"));
      }
      dialog->setContent(createInventoryHeader() +
                         createInventoryBody() +
                         createInventoryFooter());
      return;
   }

}


void InventoryFormatter::printPreview() {
   createOrUpdateDialog();
   dialog->show();
   return;
}

void InventoryFormatter::print(QPrinter* printer) {
   createOrUpdateDialog();
   dialog->print(printer);
   return;
}

void InventoryFormatter::exportHtml(QFile* file) {
   createOrUpdateDialog();
   dialog->exportHtml(file);
   return;
}
