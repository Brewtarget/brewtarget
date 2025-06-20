/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BeerColorWidget.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "BeerColorWidget.h"

#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QSizePolicy>

#include "config.h"
#include "measurement/ColorMethods.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BeerColorWidget.cpp"
#endif

// TODO: make the size adjust inside the container.
BeerColorWidget::BeerColorWidget(QWidget* parent) : QWidget(parent) {
   //setFixedSize(QSize(90,130));
   setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
   setMinimumSize(90, 130);

   glass = QImage(":/images/glass2.png");
   recObs = 0;
   return;
}

void BeerColorWidget::setRecipe(Recipe* rec) {
   if (recObs) {
      disconnect(recObs, &Recipe::changed, this, &BeerColorWidget::parseChanges);
   }

   recObs = rec;
   if (recObs) {
      connect(recObs, &Recipe::changed, this, &BeerColorWidget::parseChanges);
      this->setColor(ColorMethods::srmToDisplayColor(recObs->color_srm()));
   }
   return;
}

void BeerColorWidget::parseChanges(QMetaProperty, QVariant) {
   // For now, don't check to see what QMetaProperty is, just get the color.
   if (recObs) {
      this->setColor(ColorMethods::srmToDisplayColor(recObs->color_srm()));
   }
   return;
}

void BeerColorWidget::paintEvent(QPaintEvent *) {

   int x1 = (size().width() - 90) / 2;
   int y1 = 0;
   QRect rect;
   rect.setCoords(x1, y1, x1+87, y1+130);

   QPainter painter(this);
   painter.setBrush(color);
   painter.drawRect(rect);

   painter.drawImage(QPoint(x1,y1), glass);
   return;
}

void BeerColorWidget::setColor(QColor newColor) {
   color = QColor(newColor);

   repaint();
   return;
}
