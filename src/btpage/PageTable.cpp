/*
 * PageTable.cpp is part of Brewtarget, and is Copyright the following
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
#include "PageTable.h"
#include <QTextOption>
namespace nBtPage
{
   PageTable::PageTable(PageText *th, QList<QStringList> td, QFont tableDataFont, QFont *columnHeaderFont, QPoint pos, QRect rect)
   {
      setPosition(pos);
      setBoundingBox(rect);
      tableHeader = th;
      Font = tableDataFont;
      columnHeadersFont = (columnHeaderFont != nullptr) ? *columnHeaderFont : tableDataFont;

      // Check to see if the data is empty, if so save an empty list.
      // Pop off the first row as it contains all the headers for the columns.
      QFontMetrics fm(Font);
      QFontMetrics fm_colHeaders(columnHeadersFont);

      if (!td.isEmpty())
      {
         foreach (QString st, td.takeFirst())
         {
            columnHeaders.append(
               new PageTableColumn {
                  //set the initial Column width to the fonts horizontal advance as a startingpoint for calculating column widths below.
                  fm_colHeaders.horizontalAdvance(st),
                  PageText
                     (
                        st,
                        (columnHeaderFont != nullptr) ? *columnHeaderFont : tableDataFont
                     )});
         }
      }
      else
      {
         columnHeaders = QList<PageTableColumn *>();
      }
      // Search and see if there is any text in the table that is larger than the Columnheader, if so ajust it accordingly.
      // Maybe there is a better way to do this, but this will have to do for now.
      QList<PageText> current_row;
      tableHeight = fm_colHeaders.height() + rowPadding;
      foreach (QStringList row, td)
      {
         //Clear out any data in the current_data to make sure we have an emtpy list for the for_loop below.
         current_row.clear();
         /*
         Comparing the width between the texts as they would take space on the paper and storing the larger value.
         Any padding between the columns is done in the rendering method.
         We also populate the tableData at the same time.
         */

         for (int col = 0; col < row.count(); col++)
         {
            columnHeaders.at(col)->ColumnWidth = (columnHeaders.at(col)->ColumnWidth < fm.horizontalAdvance(row.at(col))) ? fm.horizontalAdvance(row.at(col)) : columnHeaders.at(col)->ColumnWidth;
            current_row.append(PageText{
                row.at(col),
                tableDataFont});
         }

         tableData.append(current_row);
         tableHeight += fm.height() + rowPadding;
      }

      // Storing the tableWidth to the object for later reference.
      tableWidth = 0;
      foreach (PageTableColumn *col, columnHeaders)
      {
         tableWidth += col->ColumnWidth + columnPadding;
      }
      setBoundingBox(position(), tableWidth, tableHeight);
   }

   PageTable::PageTable(QString title, QList<QStringList> tabledata, QPoint pos, QRect rect) : PageTable(
                                                                                                   //Create the Table header for the document
                                                                                                   new PageText{
                                                                                                       title,
                                                                                                       QFont("Arial", 10, QFont::Bold)},
                                                                                                   // Send in the data including the columnheaders, the first row is assumed to be column headers.
                                                                                                   tabledata,
                                                                                                   // set the default Font for the hopsTable. i.e. the contents Font.
                                                                                                   QFont("Arial", 8),
                                                                                                   // set the Columnheaders font.
                                                                                                   new QFont("Arial", 10, QFont::Bold),
                                                                                                   pos,
                                                                                                   rect)
   {
   }

   void PageTable::setColumnAlignment(int colindex, Qt::AlignmentFlag aFlag)
   {
      if (columnHeaders.isEmpty())
         return;
      columnHeaders.at(colindex)->setAlignment(aFlag);
   }

   void PageTable::render(QPainter *painter)
   {
      if (tableData.size() == 0)
      {
         tableHeader->Value = QString("No %1 in this Recipe").arg(tableHeader->Value.toLower());
         tableHeader->setPosition(position());
         tableHeader->render(painter);
         return;
      }

      QPoint currentPosition = position();

      //Draw the Header text onto the document.
      tableHeader->setPosition(position());
      tableHeader->render(painter);
      //Move our curent drawposition

      QFontMetrics fm(tableHeader->Font);
      currentPosition.setY(currentPosition.y() + fm.height() + rowPadding);

      //Set the colum positions and draw the Column headers onto the painter.
      int x = currentPosition.x();
      foreach (PageTableColumn *col, columnHeaders)
      {
         col->Text.setPosition(QPoint(x, currentPosition.y()));
         col->Text.render(painter);
         x += col->ColumnWidth + columnPadding;
      }
      QFontMetrics fm_colHeaders = QFontMetrics(columnHeadersFont);
      currentPosition.setY(currentPosition.y() + fm_colHeaders.height() + rowPadding);

      // get the metrics for the Tablefont so we can get the Fonts height when drawing the table contents.
      QFontMetrics fmtable = QFontMetrics(Font);
      int font_height = fmtable.height();

      foreach (QList<PageText> row, tableData)
      {
         for (int col_index = 0; col_index < row.count(); col_index++)
         {
            PageText currentText = row.at(col_index);
            int xcur = columnHeaders.at(col_index)->Text.position().x();
            int ycur = currentPosition.y();
            int wcur = columnHeaders.at(col_index)->ColumnWidth;
            int hcur = fmtable.height();
            currentText.setBoundingBox(xcur, ycur, wcur, hcur);
            currentText.Options = QTextOption(columnHeaders.at(col_index)->Text.Options);
            currentText.render(painter);
         }
         currentPosition.setY(currentPosition.y() + font_height + rowPadding);
      }

   }

   QSize PageTable::getSize()
   {
      QRect r = getBoundingBox();
      return QSize(r.width(), r.height());
   }

   void PageTable::calculateBoundingBox() {
      setBoundingBox(position(), tableWidth, tableHeight);
   }
}