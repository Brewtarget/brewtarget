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
#ifndef _PAGEOCHILDBJECT_H
#define _PAGEOCHILDBJECT_H
#include <QPoint>
#include <QPainter>
#include <QObject>
#include <QFont>
#include <QRect>
#include <QPrinter>
#include "BtEnumFlags.h"

namespace nBtPage
{
   class BtPage;

   class PageChildObject
   {
   public:
      QFont Font;
      BtPage *parent;

      //All sub classes from PageChildObject should know how to render them selves.
      virtual void render(QPainter *painter) = 0;
      virtual QSize getSize() = 0;
      virtual void calculateBoundingBox() = 0;

      void setBoundingBox(QRect rect);
      void setBoundingBox(int x, int y, int width, int height);
      void setBoundingBox(QPoint p, int width, int height);
      QRect getBoundingBox() { return _boundingBox; }
      void moveBoundingBox(QPoint point) { _boundingBox.moveTopLeft(point); }
      void setPosition(QPoint point);
      QPoint position() { return _position; }

   protected:
      QRect _boundingBox;
      QPoint _position;
   };
}
#endif /* _PAGECHILDOBJECT_H */