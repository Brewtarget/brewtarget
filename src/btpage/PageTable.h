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
#ifndef BTPAGE_PAGETABLE_H
#define BTPAGE_PAGETABLE_H

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
   class Page;
   /* \!brief
   * class PageTable
   * This is meant to represent a table to render/print onto a Paper or PDF.
   */
   class PageTable
      : public PageChildObject
   {
   public:
      /**
       * @brief PageTable Contructor
       * @author Mattias Måhl <mattias@kejsarsten.com>
       *
       * @param th Table header
       * @param td Table data including columnheaders as the first item in the QList
       * @param tableDataFont Font to be used for the Table data
       * @param columnheaderFont Font to be used for the Columnheaders, if nullptr it will default to tableDataFont
       * @param pos Location for the table on the page
       * @param rect QRect bounding box for the table, if set it will override the automatic sizing of the table.
       */
      PageTable(Page *parent, PageText *th, QList<QStringList> td, QFont tableDataFont, QFont *columnHeaderFont = nullptr, QPoint pos = QPoint(), QRect rect = QRect());

      /**
       * @brief Constuctor PageTable
       * This is a wrapper contructor for the PageTable setting the fonts to a standars value.
       * the standardvalue is hardcoded but may be a subject for future change to be included in the options.
       * @param title Table Header
       * @param tabledata list of lists with the first row being the column headers
       * @param pos Position of the table on the printout
       * @param rect Bounding box for the table, this overrides the automatic sizing of the table.
       */
      PageTable(Page *parent, QString title, QList<QStringList> tabledata, QPoint pos = QPoint(), QRect rect = QRect());

      PageText *tableHeader;
      QList<PageTableColumn *> columnHeaders;
      QList<QList<PageText>> tableData;

      // set these in Millimeters, will be recalculated to dpi in constructor.
      int columnPadding = 10;
      int rowPadding = 2;

      QFont columnHeadersFont;

      /**
       * @brief setColumnAlignment
       * this will set the text alignment for the column at a specific column index.
       *
       * @param colindex
       * @param aFlag
       */
      void setColumnAlignment(int colindex, Qt::AlignmentFlag aFlag);

      //Enforced by PageChildObject
      /**
       * @brief Renders the object to the supplied Painter.
       *
       * @param painter QPaintDevice to render the object to.
       */
      void render(QPainter *painter);

      /**
       * @brief Get the Size object
       *
       * @return QSize
       */
      QSize getSize();

      /**
       * @brief Calculates the bounding box for the object and updates the boundingBox member.
       *
       */
      void calculateBoundingBox( double scalex = 0.0, double scaley = 0.0 );

      /**
       * @brief Set the Table Style object
       *
       * @param ts
       */
      void setTableStyle(PageTableStyle ts);

      /**
       * @brief Get the Table Style object
       *
       * @return PageTableStyle
       */
      PageTableStyle getTableStyle() { return tableStyle; }
   private:
      PageTableStyle tableStyle;
      int tableHeight = -1;
      int tableWidth = -1;
   };
}
#endif /* BTPAGE_PAGETABLE_H */