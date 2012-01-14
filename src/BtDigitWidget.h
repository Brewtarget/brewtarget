/*
 * BtDigitWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2010-2012.
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

#ifndef BTDIGITWIDGET_H
#define BTDIGITWIDGET_H

class BtDigitWidget;

#include <QLabel>
#include <QWidget>
#include <QString>

/*!
 * \class BtDigitWidget
 * \author Philip G. Lee
 *
 * \brief Widget that displays colored numbers, depending on if the number is ok, high, or low.
 */
class BtDigitWidget : public QLabel
{
   Q_OBJECT
public:
   enum ColorType{ LOW, GOOD, HIGH, BLACK };

   BtDigitWidget(QWidget* parent = 0);
   ~BtDigitWidget();

   //void display(const QString& str);
   //! Displays the given \b num with precision \b prec.
   void display( double num, int prec );
   //! Display a QString.
   void display(QString str);

   //! Set the lower limit of the "good" range.
   void setLowLim(double num);
   //! Set the upper limit of the "good" range.
   void setHighLim(double num);
   //! Always use a constant color.
   void setConstantColor( ColorType c );
   //! Automatically choose color.
   void unsetConstantColor();

private:
   unsigned int rgblow;
   unsigned int rgbgood;
   unsigned int rgbhigh;
   double lowLim;
   double highLim;
   QString styleSheet;
   bool constantColor;
   ColorType color;
   double lastNum;
   int lastPrec;
};

#endif // BTDIGITWIDGET_H
