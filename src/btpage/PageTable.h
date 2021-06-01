/*
 * PageTable.h is part of Brewtarget, and is Copyright the following
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
#ifndef _PAGETABLE_H
#define _PAGETABLE_H

#include "PageChildObject.h"
#include "PageText.h"
#include "PageTableColumn.h"
#include <QFont>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPainter>

namespace BtPage
{
   /* \!brief
   * class PageTable
   * This is meant to represent a table to render/print onto a Paper or PDF.
   */
   class PageTable : public PageChildObject
   {
   public:
      PageTable(PageText *th, QList<QStringList> td, QFont tableDataFont, QFont *columnHeaderFont = nullptr, QPoint pos = QPoint(), QRect rect = QRect());
      PageTable(QString title, QList<QStringList> tabledata, QPoint pos = QPoint(), QRect rect = QRect());

      PageText *tableHeader;
      QList<PageTableColumn *> columnHeaders;
      QList<QList<PageText>> tableData;
      int columnPadding = 20;
      int rowPadding = 2;
      QFont columnHeadersFont;

      //Enforced by PageChildObject
      void render(QPainter *painter);
      void setColumnAlignment(int colindex, Qt::AlignmentFlag a);
   };
}
#endif /* _PAGETABLE_H */