/*
 * BtDigitWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include "BtDigitWidget.h"
#include "brewtarget.h"
#include <QFrame>
#include <iostream>
#include <QLocale>

BtDigitWidget::BtDigitWidget(QWidget *parent) : QLabel(parent)
{
   //rgblow = 208; // r = 0, g = 0, b = 208
   //rgbgood = 128 << 8; // r = 0, g = 128, b = 0
   //rgbhigh = 208 << 16; // r = 208, g = 0, b = 0
   rgblow = 0x0000d0;
   rgbgood = 0x008000;
   rgbhigh = 0xd00000;
   //styleSheet = QString("QLabel { font: normal bold 12 px \"Arial\"; color: #%1 }");
   styleSheet = QString("QLabel { font-weight: bold; color: #%1 }");
   setStyleSheet(styleSheet.arg(0,6,16,QChar('0')));
   setFrameStyle(QFrame::Box);
   setFrameShadow(QFrame::Sunken);
   lowLim = 0;
   highLim = 1;
   lastNum = 1.5;
   lastPrec = 3;
   constantColor = false;
}

void BtDigitWidget::display(QString str)
{
   static bool converted;
  

   lastNum = Brewtarget::toDouble(str,&converted);
   lastPrec = str.length() - str.lastIndexOf(QLocale().decimalPoint()) - 1;
   if( converted )
      display(lastNum,lastPrec);
   else
   {
      Brewtarget::logW( QString( "BtDigitWidget::display(QString) could not convert %1 to double").arg(str));
      setText("-");
   }
}

void BtDigitWidget::display(double num, int prec)
{
   QString str = QString("%L1").arg(num,0,'f',prec);
   QString style = styleSheet;

   lastNum = num;
   lastPrec = prec;

   if( (!constantColor && (num < lowLim)) || (constantColor && color == LOW))
   {
      style = styleSheet.arg(rgblow,6,16,QChar('0'));
      setToolTip(constantColor? "" : tr("Too low for style."));
   }
   else if( (!constantColor && (num <= highLim)) || (constantColor && color == GOOD))
   {
      style = styleSheet.arg(rgbgood,6,16,QChar('0'));
      setToolTip(constantColor? "" : tr("In range for style."));
   }
   else
   {
      if( constantColor && color == BLACK )
         style = styleSheet.arg(0,6,16,QChar('0'));
      else
      {
         style = styleSheet.arg(rgbhigh,6,16,QChar('0'));
         setToolTip(tr("Too high for style."));
      }
   }

   setStyleSheet(style);
   setText(str);
   //update(); // Calls for a repaint.
}

void BtDigitWidget::setLowLim(double num)
{
   if( num < highLim )
      lowLim = num;
   display(lastNum, lastPrec);
}

void BtDigitWidget::setHighLim(double num)
{
   if( num > lowLim )
      highLim = num;
   display(lastNum, lastPrec);
}

void BtDigitWidget::setConstantColor(ColorType c)
{
   constantColor = (c == LOW || c == GOOD || c == HIGH || c == BLACK );
   color = c;
   update(); // repaint.
}

void BtDigitWidget::unsetConstantColor()
{
   constantColor = false;
   update(); // repaint
}
