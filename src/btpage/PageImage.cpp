/*
 * PageImage.cpp is part of Brewtarget, and is Copyright the following
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

#include "PageImage.h"

#include <QPainter>
namespace BtPage
{
   void PageImage::setImage(QImage image)
   {
      _image = new QImage(image);
      setBoundingbox(_image->rect());
   }

   QImage PageImage::image()
   {
      return *_image;
   }

   void PageImage::setImageSize(int width, int height)
   {
      _width = width;
      _height = height;
      setBoundingbox(QRect(position.x(), position.y(), width, height));
   }

   void PageImage::render(QPainter *painter)
   {
      if (_width >= 0 and _height >= 0)
      {
         painter->drawImage(QRect(position, QSize(_width, _height)), *_image);
      }
      else
      {
         painter->drawImage(position, *_image);
      }
   }
}