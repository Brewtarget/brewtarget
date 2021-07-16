/*
 * PageImage.h is part of Brewtarget, and is Copyright the following
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
#ifndef BTPAGE_PAGEIMAGE_H
#define BTPAGE_PAGEIMAGE_H
#include <QImage>
#include <QPainter>
#include <QRect>
#include <QPoint>
#include "Page.h"
#include "PageChildObject.h"

namespace BtPage
{
   /* \!brief
 * Class PageImage
 * Class to handle image printout on the Page object.
 */
   class PageImage
      : public PageChildObject
   {
   public:
      PageImage(){};
      PageImage(Page *parent, QPoint pos, QImage image)
      {
         this->parent = parent;
         setPosition(pos);
         setImage(image);
      };

      void setImage(QImage image);
      void setImageSize(int width, int height);
      void setImageSizeMM(int width, int height);
      void getDPI(int &xdpi, int &ydpi);
      void setDPI(int xdpi, int ydpi);
      void setDPI(int dpi) { setDPI(dpi, dpi); }

      //Virtual members implementation
      void render(QPainter *painter);
      QSize getSize();
      void calculateBoundingBox( double scalex = 0.0, double scaley = 0.0 );

      QImage image();

   private:
      QImage _image;
      int _width = -1, _height = -1;
   };
}
#endif /* BTPAGE_PAGEIMAGE */