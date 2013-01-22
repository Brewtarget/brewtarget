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
class QPaintEvent;

/*!
 * \brief Widget to display a recipe statistic with "in-range" context from the style.
 * \author Philip G. Lee
 */
class StyleRangeWidget : public QWidget
{
   Q_OBJECT
public:
   StyleRangeWidget(QWidget* parent=0);
   
   //! \brief Set the \c min and \c max values for the style.
   void setStyleRange( double min, double max );
   //! \brief Set the \c min and \c max values for the whole widget.
   void setRange( double min, double max );
   //! \brief Set the \c precision for displaying values.
   void setPrecision(int precision);
   
   //! \brief Reimplemented from QWidget.
   virtual QSize sizeHint() const;
   
public slots:
   //! \brief Set the \c value for the indicator.
   void setValue(double value);
   //! \brief Set the \c value for the indicator. Must be convertible to double.
   void setValue(QString const& value);
   
protected:
   //! \brief Reimplemented from QWidget.
   virtual void paintEvent(QPaintEvent* event);
   
private:
   double _min;
   double _max;
   double _styleMin;
   double _styleMax;
   double _val;
   QString _valText;
   int _prec;
};

#endif /*STYLERANGEWIDGET_H*/
