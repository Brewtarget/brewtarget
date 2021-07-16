/*
 * PageTableColumn.h is part of Brewtarget, and is Copyright the following
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
#ifndef BTPAGE_PAGETABLECOLUMN_H
#define BTPAGE_PAGETABLECOLUMN_H
#include <QString>
#include <QFont>
#include "PageText.h"
#include "PageTableStyle.h"

namespace BtPage
{
   class Page;

   class PageTableColumn
   {
   public:
      int ColumnWidth;
      PageText Text;

      void setAlignment(Qt::AlignmentFlag a);
   };
}
#endif /* BTPAGE_PAGETABLECOLUMN_H */