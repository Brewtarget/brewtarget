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
#include "Page.h"
#include <QTextOption>
namespace BtPage
{
   PageTable::PageTable(Page *parent, PageText *th, QList<QStringList> td, QFont tableDataFont, QFont *columnHeaderFont, QPoint pos, QRect rect)
   {
      this->parent = parent;
      setPosition(pos);
      setBoundingBox(rect);
      tableHeader = th;
      qDebug() << Q_FUNC_INFO << "Tableheader Value" << tableHeader->Value;
      Font = QFont(tableDataFont, parent->printer);
      columnHeadersFont = (columnHeaderFont != nullptr) ? QFont(*columnHeaderFont, parent->printer) : Font;

      //recalculate the column- and Rowpadding to printers DPI.
      columnPadding *= (parent->printer->logicalDpiX() / 25.4);
      rowPadding    *= (parent->printer->logicalDpiY() / 25.4);

      //Get the Font mterics to calculate the row height and text lenght and so on.
      QFontMetrics fm( Font, parent->printer );
      QFontMetrics fm_colHeaders( columnHeadersFont, parent->printer );
      QFontMetrics fm_tableheader(tableHeader->Font, parent->printer);
      tableHeight = fm_tableheader.height() + rowPadding;
      // Check to see if the data is empty, if so save an empty list.
      // Pop off the first row as it contains all the headers for the columns.
      if ( ! td.isEmpty() )
      {
         tableHeight += fm_colHeaders.height();
         foreach (QString st, td.takeFirst())
         {
            int fm_width = getFontHorizontalAdvance(fm_colHeaders, st);
            columnHeaders.append(new PageTableColumn {
                  //set the initial Column width to the fonts horizontal advance as a startingpoint for calculating column widths below.
                  fm_width,
                  PageText
                     (
                        parent,
                        st,
                        columnHeadersFont
                     )});
         }
      }
      else
      {
         tableHeader->Value = QString("No %1 available").arg(tableHeader->Value.toLower());
         tableWidth = (int)qMax(getFontHorizontalAdvance(fm_tableheader, tableHeader->Value)*1.05, (double)tableWidth);
         columnHeaders = QList<PageTableColumn *>();
      }

      // Search and see if there is any text in the table that is larger than the Columnheader, if so ajust it accordingly.
      // Maybe there is a better way to do this, but this will have to do for now.
      QList<PageText> current_row;
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
            columnHeaders.at(col)->ColumnWidth = (columnHeaders.at(col)->ColumnWidth < getFontHorizontalAdvance(fm, row.at(col))) ? getFontHorizontalAdvance(fm, row.at(col)) : columnHeaders.at(col)->ColumnWidth;
            current_row.append(PageText{
               parent,
               row.at(col),
               QFont(tableDataFont, parent->printer)
               });
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
      tableWidth = (int)qMax(getFontHorizontalAdvance(fm_tableheader, tableHeader->Value)*1.05, (double)tableWidth);
      calculateBoundingBox();
   }

   PageTable::PageTable(Page *parent, QString title, QList<QStringList> tabledata, QPoint pos, QRect rect) : PageTable(
                                                                                                   parent,
                                                                                                   //Create the Table header for the document
                                                                                                   new PageText{
                                                                                                      parent,
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
         tableHeader->setPosition(position());
         tableHeader->calculateBoundingBox();
         tableHeader->render(painter);
         return;
      }

      QPoint currentPosition = position();

      //Draw the Header text onto the document.
      tableHeader->setPosition(position());
      tableHeader->calculateBoundingBox();
      tableHeader->render(painter);

      QFontMetrics fm(tableHeader->Font, parent->printer);
      currentPosition.setY(currentPosition.y() + fm.height() + rowPadding);

      //Set the colum positions and draw the Column headers onto the painter.
      int x = currentPosition.x();
      foreach (PageTableColumn *col, columnHeaders)
      {
         col->Text.setPosition(QPoint(x, currentPosition.y()));
         col->Text.calculateBoundingBox();
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
            currentText.setPosition(QPoint(xcur, ycur));
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

   void PageTable::calculateBoundingBox( double scalex, double scaley ) {
      setBoundingBox(position(), tableWidth, tableHeight);
   }
}