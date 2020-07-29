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

BtDigitWidget::BtDigitWidget(QWidget *parent) : QLabel(parent),
   m_rgblow(0x0000d0),
   m_rgbgood(0x008000),
   m_rgbhigh(0xd00000),
   m_lowLim(0.0),
   m_highLim(1.0),
   m_styleSheet(QString("QLabel { font-weight: bold; color: #%1 }")),
   m_constantColor(false),
   m_lastNum(1.5),
   m_lastPrec(3),
   m_low_msg(tr("Too low for style.")),
   m_good_msg(tr("In range for style.")),
   m_high_msg(tr("Too high for style."))
{
   setStyleSheet(m_styleSheet.arg(0,6,16,QChar('0')));
   setFrameStyle(QFrame::Box);
   setFrameShadow(QFrame::Sunken);
}

void BtDigitWidget::display(QString str)
{
   static bool converted;
  

   m_lastNum = Brewtarget::toDouble(str,&converted);
   m_lastPrec = str.length() - str.lastIndexOf(QLocale().decimalPoint()) - 1;
   if( converted )
      display(m_lastNum,m_lastPrec);
   else
   {
      Brewtarget::logW( QString( "BtDigitWidget::display(QString) could not convert %1 to double").arg(str));
      setText("-");
   }
}

void BtDigitWidget::display(double num, int prec)
{
   QString str = QString("%L1").arg(num,0,'f',prec);
   QString style = m_styleSheet;

   m_lastNum = num;
   m_lastPrec = prec;

   if( (!m_constantColor && (num < m_lowLim)) || (m_constantColor && m_color == LOW))
   {
      style = m_styleSheet.arg(m_rgblow,6,16,QChar('0'));
      setToolTip(m_constantColor? "" : m_low_msg);
   }
   else if( (!m_constantColor && (num <= m_highLim)) || (m_constantColor && m_color == GOOD))
   {
      style = m_styleSheet.arg(m_rgbgood,6,16,QChar('0'));
      setToolTip(m_constantColor? "" : m_good_msg);
   }
   else
   {
      if( m_constantColor && m_color == BLACK )
         style = m_styleSheet.arg(0,6,16,QChar('0'));
      else
      {
         style = m_styleSheet.arg(m_rgbhigh,6,16,QChar('0'));
         setToolTip(m_high_msg);
      }
   }

   setStyleSheet(style);
   setText(str);
}

void BtDigitWidget::setLowLim(double num)
{
   if( num < m_highLim )
      m_lowLim = num;
   display(m_lastNum, m_lastPrec);
}

void BtDigitWidget::setHighLim(double num)
{
   if( num > m_lowLim )
      m_highLim = num;
   display(m_lastNum, m_lastPrec);
}

void BtDigitWidget::setConstantColor(ColorType c)
{
   m_constantColor = (c == LOW || c == GOOD || c == HIGH || c == BLACK );
   m_color = c;
   update(); // repaint.
}

void BtDigitWidget::setLimits(double low, double high)
{
   if( low <  high ) {
      m_lowLim = low;
      m_highLim = high;
   }
   display(m_lastNum, m_lastPrec);
}

void BtDigitWidget::setLowMsg( QString msg )  { m_low_msg = msg; }
void BtDigitWidget::setGoodMsg( QString msg ) { m_good_msg = msg; }
void BtDigitWidget::setHighMsg( QString msg ) { m_high_msg = msg; }

void BtDigitWidget::setMessages( QStringList msgs )
{
   if ( msgs.size() != 3 ) {
      Brewtarget::logW("Wrong number of messages");
      return;
   }
   m_low_msg = msgs[0];
   m_good_msg = msgs[1];
   m_high_msg = msgs[2];
}

