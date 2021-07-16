/*
 * PageChildObject.h is part of Brewtarget, and is Copyright the following
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
#ifndef BTPAGE_PAGEOCHILDBJECT_H
#define BTPAGE_PAGEOCHILDBJECT_H
#include <QPoint>
#include <QPainter>
#include <QObject>
#include <QDebug>
#include <QFont>
#include <QRect>
#include <QPrinter>
#include "BtEnumFlags.h"

namespace BtPage
{
   class Page;

   class PageChildObject
   {
   public:
      //All sub classes from PageChildObject should know how to render them selves.
      virtual void render(QPainter *painter) = 0;
      virtual QSize getSize() = 0;
      virtual void calculateBoundingBox( double scalex = 0.0, double scaley = 0.0 ) = 0;

      QFont Font;
      Page *parent;
      //poor mans singularly linked list for outputting to multiple pages.
      PageChildObject *nextSection = nullptr;
      bool needPageBreak = false;

      void setBoundingBox(QRect rect);
      void setBoundingBox(int x, int y, int width, int height);
      void setBoundingBox(QPoint p, int width, int height);
      QRect getBoundingBox() {
         return itemBoundingBox;
      }
      void moveBoundingBox(QPoint point) { itemBoundingBox.moveTopLeft(point); }
      void setPosition(QPoint point);
      void setPositionMM(int x, int y);
      QPoint position() { return itemPosition; }

      /**
       * @brief Get the Font Horizontal Advance for a given string.
       * Thsi is QT version sensitive, using different methods depanding on QT version.
       * Since Qt-version 5.13 they introdued QFontMetrics::horizontalAdvance(const QString &text, int len = -1) to get the width of a given string.
       * Before that there was the QFontMetrics::width(const QString &text, int len = -1) function to do the same.
       *
       * @param fontMetrics
       * @param text
       * @return int
       */
      int getFontHorizontalAdvance(QFontMetrics fm, QString text);

   protected:
      QRect itemBoundingBox;
      QPoint itemPosition;
   };
}
#endif /* BTPAGE_PAGECHILDOBJECT_H */