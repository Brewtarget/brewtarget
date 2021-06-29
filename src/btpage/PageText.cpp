/*
 * PageText.cpp is part of Brewtarget, and is Copyright the following
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

#include "PageText.h"
#include "Page.h"
namespace BtPage
{

   PageText::PageText(Page *parent, QString value, QFont font)
   {
      this->parent = parent;
      Value = value;
      Font = QFont( font, parent->printer );
      calculateBoundingBox();
   }

   void PageText::render(QPainter *painter)
   {
      painter->setFont(Font);
      painter->drawText(getBoundingBox(), Value, Options);
   }

   int PageText::count()
   {
      return Value.count();
   }

   QSize PageText::getSize()
   {
      QFontMetrics fm(Font);
      return QSize(getFontHorizontalAdvance(fm, Value), fm.height());
   }

   void PageText::calculateBoundingBox( double scalex, double scaley)
   {
      int tW, w, pageLogicalWidth;
      QRect pagePaintRect, ri;
      QFontMetrics fm(Font, parent->printer);
      tW = getFontHorizontalAdvance(fm, Value);
      pagePaintRect = parent->printer->pageLayout().paintRectPixels(parent->printer->logicalDpiX());
      pageLogicalWidth = pagePaintRect.width();

      // if the string is longer than what will fit on the page, create a box that will fit and auto line brake.
      if (tW > pageLogicalWidth - position().x())
      {
         //QMarginsF margins = parent->printer->pageLayout().marginsPixels(parent->printer->logicalDpiX())
         w = pageLogicalWidth - position().x();
         ri = fm.boundingRect(QRect(0,0, w, pagePaintRect.height()), Qt::TextWordWrap, Value);
         qDebug() << Q_FUNC_INFO << "bounding rect for text" << ri;
      }
      else
      {
         ri = fm.boundingRect(Value);
         ri.setWidth( tW * 1.05 );
      }

      //now check if we need to PageBrake!!!

      if (pagePaintRect.height() - position().y() < ri.height())
      {
         //set the new height of the bounding bpx for this object
         ri.setHeight((pagePaintRect.height() - position().y() + pagePaintRect.top()));
         int RowsInBoundingBox = ri.height() / fm.height();
         QString currentRow;
         QString nextSectionText = Value;
         for(int i = 0; i < RowsInBoundingBox; i++)
         {
            currentRow += fm.elidedText(nextSectionText, Qt::ElideRight, ri.width(), Qt::TextWordWrap);
            currentRow.chop(currentRow.length() - currentRow.lastIndexOf(" "));
            i += currentRow.count("\n\n");
            nextSectionText = Value.right(Value.length() - currentRow.length());
         }
         qDebug() << Q_FUNC_INFO << "nextSectionText: " << nextSectionText;
         //setting this flag to tell the rendering method that the data is splitted.
         //If this is set to true the pointer to the next section will be called when rendering the pages.
         needPageBreak = true;
         //Create the next section of the text.
         //this will most often happen when there are long notes or brewing instructions.
         PageText *tnextSection = new PageText(parent, nextSectionText, Font);
         //put the next section at the top of next page keeping the x position to make it consistent.
         tnextSection->setPosition(QPoint(position().x(), 0));
         //Copy over the text options to the next section.
         tnextSection->Options = Options;
         //Calculate the bounding box for the new section.
         tnextSection->calculateBoundingBox();

         nextSection = tnextSection;
         //Keep only the first section in this object.
         //Value = Value.left(charsInFirstSection);
         Value = currentRow;
         qDebug() << Q_FUNC_INFO << "Value after split: " << Value;
      }
      if (needPageBreak) qDebug() << Q_FUNC_INFO << "NeedPageBrake == true :: ri = " << ri;
      itemBoundingBox = ri;
      moveBoundingBox(position());
   }
}