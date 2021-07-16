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
      _image = image;
      _width = _image.width();
      _height = _image.height();
      setBoundingBox(_image.rect());
   }

   void PageImage::getDPI(int &xdpi, int &ydpi)
   {
      xdpi = _image.dotsPerMeterX() / 39.37;
      ydpi = _image.dotsPerMeterY() / 39.37;
   }

   void PageImage::setDPI(int xdpi, int ydpi)
   {
      _image.setDotsPerMeterX(int(xdpi * 39.37));
      _image.setDotsPerMeterY(int(ydpi * 39.37));
   }

   QImage PageImage::image()
   {
      return _image;
   }

   void PageImage::setImageSize(int width, int height)
   {
      setImage(_image.scaled(width, height, Qt::AspectRatioMode::KeepAspectRatio));
   }

   void PageImage::setImageSizeMM(int width, int height)
   {
      setDPI(parent->printer->logicalDpiX(), parent->printer->logicalDpiY());
      int dpmmX = _image.dotsPerMeterX() / 1000;
      int dpmmY = _image.dotsPerMeterY() / 1000;
      int cWidth = width * dpmmX;
      int cHeight = height * dpmmY;
      setImageSize(cWidth, cHeight);
   }

   void PageImage::render(QPainter *painter)
   {
      painter->drawImage(itemPosition, _image);
   }

   QSize PageImage::getSize()
   {
      return QSize(_image.width(), _image.height());
   }

   void PageImage::calculateBoundingBox( double scalex, double scaley )
   {
      setBoundingBox(position(), _image.width(), _image.height());
   }
}