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
#include <QColor>
#include <QSize>
#include "BeerColorWidget.h"

// TODO: make the size adjust inside the container.
BeerColorWidget::BeerColorWidget()
{
   setFixedSize(QSize(100,100));
}

// TODO: color seems too dark.
void BeerColorWidget::paintEvent(QPaintEvent *)
{
   QPainter painter(this);
   QRect rect;

   rect.setCoords(0,0,100,100);
   painter.setBrush(color);
   painter.drawRect(rect);
}

void BeerColorWidget::setColor( QColor newColor )
{
   color = QColor(newColor);
   
   repaint();
}
