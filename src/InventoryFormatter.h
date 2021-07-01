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

#include <QList>
#include <QStringList>
#include <QMap>
#include "brewtarget.h"
#include "database.h"
#include "model/Hop.h"
namespace InventoryFormatter
{
   enum HTMLgenerationFlags
   {
      NOOPERATION           = 0,
      FERMENTABLESFLAG      = (1<<0),
      HOPSFLAG              = (1<<1),
      YEASTFLAG             = (1<<2),
      MISCELLANEOUSFLAG     = (1<<3)
   };

   inline HTMLgenerationFlags operator|(HTMLgenerationFlags a, HTMLgenerationFlags b)
   {
      return static_cast<HTMLgenerationFlags>(static_cast<int>(a) | static_cast<int>(b));
   }

   inline bool operator&(HTMLgenerationFlags a, HTMLgenerationFlags b)
   {
      return (static_cast<int>(a) & static_cast<int>(b));
   }

   /**
    * @brief Create a Inventory H T M L for export
    *
    * @return QString containing the HTML code for the inventory tables.
    *
    */
   QString createInventoryHTML(HTMLgenerationFlags flags);

   /**
    * @brief this will call the appropriate database function to get the type specified item by id.
    * in short this wraps the "Database::instance().yeast(id)" behind getEntity<Yeast>(34);
    *
    * @tparam S
    * @param id
    * @return S*
    */
   template <class S> S* getEntity(int id);

   /**
    * @brief Get the Table Row of the specified type and returns a QStringlist with the 'columns' in the row.
    * This is implemented per type as the row data is gathered somewhat different for each type.
    * i.e. a row for Hop do not look the same as a row for Fermentables.
    *
    * @tparam S
    * @return const QStringList
    */
   template <class S> const QStringList getTableRow(S*, double);

   /**
    * @brief Create a Inventory List of the specified Type, i.e. Yeast or Fermentable and so on.
    *
    * @tparam T
    * @param table
    * @return QList<QStringList>
    */
   template <class T> QList<QStringList> createInventoryList(Brewtarget::DBTable table)
   {
      QList<QStringList> result;
      const QMap<int, double> inventory =
            Database::instance().getInventory(table);

      if (!inventory.empty())
      {
         QStringList row;
         // We check if T is the Hop class, because that one has a special set of dataheaders.
         // all others have the same set of headers.
         if (sizeof(T) == sizeof(Hop))
            row << QObject::tr("Name") << QObject::tr("Alpha %") << QObject::tr("Amount");
         else
            row << QObject::tr("Name") << QObject::tr("Amount");
         result.append(row);
         row.clear();
         for (auto itor = inventory.begin(); itor != inventory.end(); ++itor)
         {
            T* entity = getEntity<T>(itor.key());
            //const T* entity = Database::instance().getAll<T>();

            if (!entity)
            {
               qCritical() << QString("The ingredient %1 has a record in the "
                                       "inventory, but does not exist.")
                                    .arg(itor.key());
               continue;
            }
            result.append(getTableRow(entity, itor.value()));
            row.clear();
         }
      }
      return result;
   }
} // InventoryFormatter

#endif
