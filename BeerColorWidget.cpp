/*
 * BeerColorWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QWidget>
#include <QPixmap>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QPoint>
#include <QColor>
#include <QSize>
#include "BeerColorWidget.h"
#include "config.h"

// TODO: make the size adjust inside the container.
BeerColorWidget::BeerColorWidget()
{
   setFixedSize(QSize(90,130));
}

void BeerColorWidget::paintEvent(QPaintEvent *)
{
   // === ORIG ===
   QPainter painter(this);
   QRect rect;

   rect.setCoords(0,0,90,130);
   painter.setBrush(color);
   painter.drawRect(rect);
   // === END ORIG ===

   QImage glass = QImage(GLASS);
   painter.drawImage( QPoint(0,0), glass );
}

void BeerColorWidget::setColor( QColor newColor )
{
   color = QColor(newColor);
   
   repaint();
}
