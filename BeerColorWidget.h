/*
 * BeerColorWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _BEERCOLORWIDGET_H
#define	_BEERCOLORWIDGET_H

class BeerColorWidget;

#include <QWidget>
#include <QColor>
#include <QPaintEvent>
#include <QImage>
#include "observable.h"
#include "recipe.h"

class BeerColorWidget : public QWidget, public Observer
{
   Q_OBJECT

public:
   BeerColorWidget();
   void setColor( QColor newColor );
   void setRecipe( Recipe* rec );
protected:
   virtual void paintEvent(QPaintEvent *);
   QColor color;
private:
   QImage glass;
   void showColor();
   
   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer
   
   Recipe* recObs;
};

#endif	/* _BEERCOLORWIDGET_H */

