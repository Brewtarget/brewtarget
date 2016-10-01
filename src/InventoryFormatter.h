/*
 * InventoryFormatter.h is part of Brewtarget, and was written by
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

#ifndef INVENTORY_FORMATTER_H
#define INVENTORY_FORMATTER_H

class QFile;
class QPrinter;

namespace InventoryFormatter
{

/*!
 * \brief Shows the print preview dialogue for the inventory.
 */
void printPreview();

/*!
 * \brief Prints the inventory.
 * \param printer The printer to print to, should not be @c NULL.
 */
void print(QPrinter* printer);

/*!
 * \brief Exports the inventory to a HTML document.
 * \param file The output file opened for writing.
 */
void exportHTML(QFile* file);

} // InventoryFormatter

#endif
