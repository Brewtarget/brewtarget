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

PageTable::PageTable(PageText *th, QList<QStringList> td, QFont tableDataFont, QFont *columnHeaderFont)
{
   tableHeader = th;
   Font = tableDataFont;
   columnHeadersFont = (columnHeaderFont != nullptr) ? *columnHeaderFont : tableDataFont;
   // Check to see if the data is empty, if so save an empty list.
   // Pop off the first row as it contains all the headers for the columns.
   QFontMetrics fm(Font);
   QFontMetrics fm_colHeaders(columnHeadersFont);

   if ( ! td.isEmpty() ) {
      foreach(QString st, td.takeFirst())
      {
         columnHeaders.append( new PageTableColumn {
            fm_colHeaders.horizontalAdvance(st), //set the initial Column width to the number of characters.
            PageText {
               st,
               (columnHeaderFont != nullptr) ? *columnHeaderFont : tableDataFont
            }
         });
      }
   }
   else
   {
      columnHeaders = QList<PageTableColumn *>();
   }
   // Search and see if there is any text in the table that is larger than the Columnheader, if so ajust it accordingly.
   // Maybe there is a better way to do this, but this will have to do for now.
   QList<PageText> current_row;
   foreach(QStringList row, td)
   {
      //Clear out any data in the current_data to make sure we have an emtpy list for the for_loop below.
      current_row.clear();
      /*
      we do this forloop like this because I need to know what index I'm at to compare the width
      Comparing the width between the texts as they would take space on the paper and storing the larger value.
      Any padding between the columns is done in the rendering method.
      We also populate the tableData at the same time.
      */
      for (int col = 0; col < row.count(); col++ ) {
         columnHeaders.at(col)->ColumnWidth = (columnHeaders.at(col)->ColumnWidth < fm.horizontalAdvance(row.at(col))) ? fm.horizontalAdvance(row.at(col)) : columnHeaders.at(col)->ColumnWidth;
         current_row.append( PageText {
            row.at(col),
            tableDataFont
         });
      }
      tableData.append(current_row);
   }
}

PageTable::PageTable(QString title, QList<QStringList> tabledata) :
   PageTable (
            //Create the Table header for the document
            new PageText {
               title,
               QFont("Arial", 14, QFont::Bold)
               },
            // Send in the data including the columnheaders, the first row is assumed to be column headers.
            tabledata,
            // set the default Font for the hopsTable. i.e. the contents Font.
            QFont("Arial", 12),
            // set the Columnheaders font.
            new QFont("Arial", 14, QFont::Bold)
            ) {}

void PageTable::setColumnAlignment(int colindex, Qt::AlignmentFlag a) {
   if (columnHeaders.isEmpty()) return;
   columnHeaders.at(colindex)->setAlignment(a);
}

//This inherits from the PageChildObject abstract class.
//This should render itself onto a painter object according to the data within it.
//Although the position has tp be set in order to not paint in the 0,0 coordinates.
void PageTable::render(QPainter *painter) {
   QPoint currentPosition = QPoint(position);

   //Draw the Header text onto the document.
   tableHeader->position = position;
   tableHeader->render(painter);

   //Move our curent drawposition
   QFontMetrics fm(tableHeader->Font);
   currentPosition.setY(currentPosition.y() + fm.height() + rowPadding);

   //Set the colum positions and draw the Column headers onto the painter.
   int x = currentPosition.x();
   foreach(PageTableColumn *col, columnHeaders)
   {
      col->Text.position = QPoint(x, currentPosition.y());
      x += col->ColumnWidth + columnPadding;
      col->Text.render(painter);
   }
   QFontMetrics fm_colHeaders = QFontMetrics(columnHeadersFont);
   currentPosition.setY(currentPosition.y() + fm_colHeaders.height() + rowPadding);

   // get the metrics for the Tablefont so we can get the Fonts height when drawing the table contents.
   QFontMetrics fmtable = QFontMetrics(Font);
   int font_height = fmtable.height();

   foreach(QList<PageText> row, tableData)
   {
      for(int col_index=0; col_index < row.count(); col_index++)
      {
         PageText currentText = row.at(col_index);
         currentText.boundingRectangle = new QRectF(columnHeaders.at(col_index)->Text.position.x(), currentPosition.y(), columnHeaders.at(col_index)->ColumnWidth, fmtable.height());
         currentText.Options = QTextOption(columnHeaders.at(col_index)->Text.Options);
         currentText.render(painter);
      }
      currentPosition.setY(currentPosition.y() + font_height + rowPadding);
   }

}