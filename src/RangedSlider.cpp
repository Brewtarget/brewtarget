/*
 * RangedSlider.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "RangedSlider.h"
#include <QPaintEvent>
#include <QPainter>
#include <QColor>
#include <QPalette>
#include <QApplication>
#include <QRectF>
#include <QFont>
#include <QMouseEvent>
#include <QLabel>
#include <QToolTip>
#include <QLinearGradient>
#include <QPainterPath>

#include <QDebug>

RangedSlider::RangedSlider(QWidget* parent)
   : QWidget(parent),
     _min(0.0),
     _max(1.0),
     _prefMin(0.25),
     _prefMax(0.75),
     _val(0.5),
     _valText("0.500"),
     _prec(3),
     _tickInterval(0),
     _secondaryTicks(1),
     _tooltipText(""),
     _bgBrush(QColor(255,255,255)),
     _prefRangeBrush(QColor(0,0,0)),
     _prefRangePen(Qt::NoPen),
     _markerBrush(QColor(255,255,255)),
     _markerTextIsValue(false)
{
   setMinimumSize( 32, 32 );
   setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
   
   // Generate mouse move events whenever mouse movers over widget.
   setMouseTracking(true);
   
   repaint();
}

void RangedSlider::setPreferredRange( double min, double max )
{
   _prefMin = min;
   _prefMax = max;
   
   // Only show tooltips if the range has nonzero size.
   setMouseTracking(min < max);
   
   _tooltipText = QString("%1 - %2").arg(min, 0, 'f', _prec).arg(max, 0, 'f', _prec);
   
   update();
}

void RangedSlider::setRange( double min, double max )
{
   _min = min;
   _max = max;
   update();
}

void RangedSlider::setValue(double value)
{
   _val = value;
   _valText = QString("%1").arg(_val, 0, 'f', _prec);
   update();
}

void RangedSlider::setPrecision(int precision)
{
   _prec = precision;
   update();
}

void RangedSlider::setBackgroundBrush( QBrush const& brush )
{
   _bgBrush = brush;
   update();
}

void RangedSlider::setPreferredRangeBrush( QBrush const& brush )
{
   _prefRangeBrush = brush;
   update();
}

void RangedSlider::setPreferredRangePen( QPen const& pen )
{
   _prefRangePen = pen;
   update();
}

void RangedSlider::setMarkerBrush( QBrush const& brush )
{
   _markerBrush = brush;
   update();
}

void RangedSlider::setMarkerText( QString const& text )
{
   _markerText = text;
   update();
}

void RangedSlider::setMarkerTextIsValue(bool val)
{
   _markerTextIsValue = val;
   update();
}

void RangedSlider::setTickMarks( double primaryInterval, int secondaryTicks )
{
   _secondaryTicks = (secondaryTicks<1)? 1 : secondaryTicks;
   _tickInterval = primaryInterval/_secondaryTicks;
   
   update();
}

QSize RangedSlider::sizeHint() const
{
   static const QSize hint(64,32);
   
   return hint;
}

void RangedSlider::mouseMoveEvent(QMouseEvent* event)
{
   event->accept();
   
   QPoint tipPoint( mapToGlobal(QPoint(0,0)) );
   QToolTip::showText( tipPoint, _tooltipText, this );
}

void RangedSlider::paintEvent(QPaintEvent* event)
{
   static const QPalette palette(QApplication::palette());
   static const int indTextHeight=16;
   static const int rectHeight = 16;
   static const int textWidth  = 48;
   static const int indWidth   = 4;
   static const QColor fgRectColor(0,127,0);
   static const QColor textColor(0,127,0);
   static const QFont textFont("Arial", 14, QFont::Black);
   
   QLinearGradient glassGrad( QPointF(0,0), QPointF(0,rectHeight) );
   glassGrad.setColorAt( 0, QColor(255,255,255,127) );
   glassGrad.setColorAt( 1, QColor(255,255,255,0) );
   QBrush glassBrush(glassGrad);
   
   QPainter painter(this);
   float rectWidth   = 512;
   float fgRectLeft  = rectWidth/(_max-_min) * (_prefMin-_min);
   float fgRectWidth = rectWidth/(_max-_min) * (_prefMax-_prefMin);
   float indX        = rectWidth/(_max-_min) * (_val-_min);
   float indLeft;
   
   // Make sure all coordinates are valid.
   fgRectLeft  = qBound( 0.f, fgRectLeft, rectWidth);
   fgRectWidth = qBound( 0.f, fgRectWidth, rectWidth-fgRectLeft);
   indX        = qBound( 0.f, indX, rectWidth-indWidth/2 );
   indLeft     = qBound( 0.f, indX-indWidth/2, rectWidth );
   
   painter.save();

      // Indicator text.
      QRectF markerTextRect = painter.boundingRect( QRectF(), Qt::AlignCenter | Qt::AlignBottom, _markerTextIsValue? _valText : _markerText );
      float markerTextLeft = qBound( 0.f, static_cast<float>(indLeft*(width()-textWidth-2)/rectWidth - markerTextRect.width()/2), static_cast<float>(width()-textWidth-2-markerTextRect.width()));
      painter.drawText(
         markerTextLeft, 0,
         markerTextRect.width(), 16,
         Qt::AlignCenter | Qt::AlignBottom,
         _markerTextIsValue? _valText : _markerText
      );

      // Scale coordinates so that 'rectWidth' units == width()-textWidth-2 pixels.
      painter.scale( (width()-textWidth-2)/rectWidth, 1.0 );
      painter.translate(0, indTextHeight);
      
      painter.setPen(Qt::NoPen);
      
      // Make sure anything we draw "inside" the "glass rectangle" stays inside.
      QPainterPath clipRect;
      clipRect.addRoundedRect( QRectF(0, 0, rectWidth, rectHeight), 8, 8 );
      painter.setClipPath(clipRect);
      
      // Draw the background rectangle.
      painter.setBrush(_bgBrush);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.drawRoundedRect( QRectF(0, 0, rectWidth, rectHeight), 8, 8 );
      painter.setRenderHint(QPainter::Antialiasing,false);
      
      // Draw the style "foreground" rectangle.
      painter.save();
         painter.setBrush(_prefRangeBrush);
         painter.setPen(_prefRangePen);
         painter.setRenderHint(QPainter::Antialiasing);
         //painter.drawRect( QRectF(fgRectLeft, 0, fgRectWidth, rectHeight) );
         painter.drawRoundedRect( QRectF(fgRectLeft, 0, fgRectWidth, rectHeight), 8,8 );
      painter.restore();
      
      // Draw the indicator.
      painter.setBrush(_markerBrush);
      painter.drawRect( QRectF(indLeft, 0, indWidth, rectHeight) );
      
      // Draw a white to clear gradient to suggest "glassy."
      painter.setBrush(glassBrush);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.drawRoundedRect( QRectF(0, 0, rectWidth, rectHeight), 8, 8 );
      painter.setRenderHint(QPainter::Antialiasing,false);
      
      // Draw the ticks.
      painter.setPen(Qt::black);
      if( _tickInterval > 0.0 )
      {
         int secTick = 1;
         for( double currentTick = _min+_tickInterval; _max - currentTick > _tickInterval-1e-6; currentTick += _tickInterval )
         {
            painter.translate( rectWidth/(_max-_min) * _tickInterval, 0);
            if( secTick == _secondaryTicks )
            {
               painter.drawLine( QPointF(0,0.25*rectHeight), QPointF(0,0.75*rectHeight) );
               secTick = 1;
            }
            else
            {
               painter.drawLine( QPointF(0,0.333*rectHeight), QPointF(0,0.666*rectHeight) );
               ++secTick;
            }
         }
      }
   painter.restore();
   
   painter.translate( width() - textWidth, indTextHeight );
   // Draw the text.
   painter.setPen(textColor);
   painter.setFont(textFont);
   painter.drawText( 0, 0, textWidth, 16, Qt::AlignRight | Qt::AlignVCenter, _valText );
}
