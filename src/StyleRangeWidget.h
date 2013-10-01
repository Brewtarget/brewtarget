/*
 * StyleRangeWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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

#ifndef STYLERANGEWIDGET_H
#define STYLERANGEWIDGET_H

#include <QWidget>
#include <QSize>
#include <QString>
#include "RangedSlider.h"
class QPaintEvent;
class QMouseEvent;

/*!
 * \brief Widget to display a recipe statistic with "in-range" context from the style.
 * \author Philip G. Lee
 */
class StyleRangeWidget : public RangedSlider
{
   Q_OBJECT
public:
   StyleRangeWidget(QWidget* parent=0);
   
//public slots:
   //! \brief Set the \c value for the indicator. Must be convertible to double.
   //void setValue(QString const& value);
   
private:
   double _min;
   double _max;
   double _styleMin;
   double _styleMax;
   double _val;
   QString _valText;
   int _prec;
   double _tickInterval;
   int _secondaryTicks;
   QString _tooltipText;
};

#endif /*STYLERANGEWIDGET_H*/
