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
#ifndef _PAGEIMAGE_H
#define _PAGEIMAGE_H
#include <QImage>
#include <QPainter>
#include <QRect>
#include <QPoint>

#include "PageChildObject.h"

namespace BtPage
{
   /* \!brief
 * Class PageImage
 * Class to handle image printout on the BtPage object.
 */
   class PageImage : public PageChildObject
   {
   public:
      PageImage(){};
      PageImage(QPoint pos, QImage image)
      {
         setImage(image);
         position = pos;
      };

      PageImage(int x, int y, char *filepath) : PageImage(QPoint(x, y), QImage()) {}

      void setImage(QImage image);
      void setImageSize(int width, int height);
      void render(QPainter *painter);

      QImage image();

   private:
      QImage *_image = nullptr;
      int _width = -1, _height = -1;
   };
}
#endif /* _PAGEIMAGE */
