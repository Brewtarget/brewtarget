/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * InventoryFormatter.h is part of Brewtarget, and is copyright the following authors 2016-2023:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Mark de Wever <koraq@xs4all.nl>
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
#ifndef INVENTORY_FORMATTER_H
#define INVENTORY_FORMATTER_H
#pragma once

#include <cstdint>

#include <QFlags> // For Q_DECLARE_FLAGS

class QString;

namespace InventoryFormatter {

   //
   // TBD: In theory we can use
   //
   enum class HtmlGenerationFlag {
      NO_OPERATION          = 0,
      FERMENTABLES          = (1 << 0),
      HOPS                  = (1 << 1),
      YEAST                 = (1 << 2),
      MISCELLANEOUS         = (1 << 3)
   };
   Q_DECLARE_FLAGS(HtmlGenerationFlags, HtmlGenerationFlag)


   /**
    * @brief ORs the HtmlGenerationFlags implementation.
    *
    * @param a
    * @param b
    * @return HtmlGenerationFlags
    */
//   HtmlGenerationFlags operator|(HtmlGenerationFlags a, HtmlGenerationFlags b);

   /**
    * @brief ANDs the HtmlGenerationFlags
    *
    * @param a
    * @param b
    * @return true
    * @return false
    */
//   bool operator&(HtmlGenerationFlags a, HtmlGenerationFlags b);

   /**
    * @brief Create a Inventory HTML for export
    *
    * @return QString containing the HTML code for the inventory tables.
    */
   QString createInventoryHtml(HtmlGenerationFlags flags);

}

Q_DECLARE_OPERATORS_FOR_FLAGS(InventoryFormatter::HtmlGenerationFlags)

#endif
