/*
 * RangedSlider.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip G. Lee <rocketman768@gmail.com>
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

#include "RangedSlider.h"
#include "brewtarget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QColor>
#include <QPalette>
#include <QApplication>
#include <QRectF>
#include <QFont>
#include <QFontMetrics>
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
     _markerTextIsValue(false),
     valueTextFont("Arial",
                   14,             // QFonts are specified in point size, so the hard-coded number is fine here.
                   QFont::Black),  // Note that QFont::Black is a weight (more bold than ExtraBold), not a colour.
     indicatorTextFont("Arial",
                       10,
                       QFont::Normal) // Previously we just did the indicator text in 'default' font
{
   // Ensure this->heightInPixels is properly initialised
   this->recalculateHeightInPixels();

   this->setMinimumSize();

   // Setting the horizontal policy here
   this->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );

   // There no particular reason to limit our horizontal size, so, in principle, this call asks that there be no such
   // (practical) limit.  However, if a maximumSize property has been set in a Designer UI File (eg MainWindow.ui) then
   // that setting will override this one, because it will be applied later (in fact pretty much straight after this
   // constructor returns).
   this->setMaximumWidth(QWIDGETSIZE_MAX);

   // Generate mouse move events whenever mouse movers over widget.
   this->setMouseTracking(true);

   this->repaint();
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

void RangedSlider::setPreferredRange(QPair<double,double> minmax)
{
   setPreferredRange( minmax.first, minmax.second );
}

void RangedSlider::setRange( double min, double max )
{
   _min = min;
   _max = max;
   update();
}

void RangedSlider::setRange(QPair<double,double> minmax)
{
   setRange( minmax.first, minmax.second );
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

void RangedSlider::recalculateHeightInPixels() const {
   //
   // We need to be able to tell Qt about our minimum and preferred size in pixels.  This is something that's going
   // to depend on the dots-per-inch (DPI) resolution of the monitor we're being displayed on.  Advice at
   // https://doc.qt.io/qt-5/highdpi.html is that, for best High DPI display support, we should replace hard-coded
   // sizes in layouts and drawing code with values calculated from font metrics or screen size.  For this widget, we
   // use font sizes (as described in more detail later in this comment) as it seems simpler and the height of the
   // widget really is determined by the size of the text it contains.
   //
   // In theory, someone might be running the app on a system with multiple screens with different DPI resolutions,
   // so, in principle we ought to do redo size calculations every time the widget is moved, in case it moves from one
   // screen to another and the resolution changes.  In practice, I'm not sure how big a requirement this is.  So, for
   // now, have just tried to organise things so that it would, in principle, be possible to implement such behaviour
   // in future.
   //
   // We want height to be fixed, as the slider does not get more readable if you make it taller.  So minimum and
   // preferred height are the same.
   //
   // For width, We are OK for them to expand and contract horizontally, within reason, as the size of the main window
   // changes.  Minimum and preferred widths are a bit of a rule-of-thumb, but 2× and 4× height are a sensible stab.
   //
   // Firstly we have to determine what the fixed height is.  We could query the DPI resolution and size of the current
   // screen, but the simplest thing is to use font sizes.  The slider is basically two lines of characters high.  Top
   // line is the "indicator" text that sits above the slider visual in the middle of the "preferred range".  Bottom
   // line is the slider visual and the "value" text that sits to the right of the slider visual.  The fonts for both
   // bits of text are set in device-independent points in the constructor, and we can just query their height etc in
   // pixels.
   //
   // Secondly, the way we tell Qt about minimum and preferred sizes is slightly different:
   //  • setMinimumSize() tells Qt not to make us smaller than the specified size when resizing windows etc, HOWEVER
   //    it does not determine what size we are initially drawn
   //  • instead, Qt calls sizeHint() when doing initial layout, and we must override this to supply our desired
   //    initial dimensions.  NB: Although it makes no sense, there is nothing to stop this method returning dimensions
   //    below the minimums already set via setMinimumSize().  (AIUI, sizeHint() is also called on resize events to
   //    find our preferred dimensions.)
   //
   // The final wrinkle is that the height of the font sort of depends what you mean.  Strictly, using the inter-line
   // spacing (= height plus leading, though the latter is often 0) gives you enough space to show any character of the
   // font.  But, for value text font, we only ever show digits and decimal points, we might not need all this space.
   // We could consider using the height of a capital letter - via capHeight().  For now, we stick with line-spacing.
   //
   QFontMetrics valueTextFontMetrics(this->valueTextFont);
   QFontMetrics indicatorTextFontMetrics(this->indicatorTextFont);
   this->heightInPixels = valueTextFontMetrics.lineSpacing() + indicatorTextFontMetrics.lineSpacing();
   return;
}

void RangedSlider::setMinimumSize() {
   // Caller's responsibility to have recently called this->recalculateHeightInPixels();
   // We just need to call the parent class method with suitable parameters
   this->setMinimumSize(2 * this->heightInPixels, this->heightInPixels);
   return;
}

QSize RangedSlider::sizeHint() const
{
   this->recalculateHeightInPixels();
   return QSize(4 * this->heightInPixels, this->heightInPixels);
}

void RangedSlider::mouseMoveEvent(QMouseEvent* event)
{
   event->accept();

   QPoint tipPoint( mapToGlobal(QPoint(0,0)) );
   QToolTip::showText( tipPoint, _tooltipText, this );
}

void RangedSlider::paintEvent(QPaintEvent* event)
{
   //
   // Simplistically, the high-level layout of the slider is:
   //
   //    |-------------------------------------------------------------|
   //    |                   Indicator text                            |
   //    |------------------------------------------------+------------|
   //    | <--------------- Graphical Area -------------> | Value text |
   //    |-------------------------------------------------------------|
   //
   // The graphical area has:
   //  - a background rectangle of the full width of the area, representing the range from this->_min to this->_max
   //  - a foreground rectangle showing the sub-range of this background from this->_prefMin to this->_prefMax
   //  - a line ("the indicator") showing where this->_val lies in the (this->_min to this->_max) range
   //
   // The indicator text sits above the indicator line and shows either its value (this->_valText) or some textual
   // description (eg "Slightly Malty" on the IBU/GU scale) which comes from this->_markerText.
   //
   // The value text also shows this->_valText
   //

   QFontMetrics indicatorTextFontMetrics(this->indicatorTextFont);
   int indicatorTextHeight = indicatorTextFontMetrics.lineSpacing();

   // The slider graphic and the value text are the same height
   QFontMetrics valueTextFontMetrics(this->valueTextFont);
   int graphicalAreaHeight = valueTextFontMetrics.lineSpacing();


   static const QPalette palette(QApplication::palette());
   static const int indicatorLineWidth   = 4;
   static const QColor fgRectColor(0,127,0);
   static const QColor indicatorTextColor(0,0,0);
   static const QColor valueTextColor(0,127,0);

   // We need to allow for the width of the text that displays to the right of the slider showing the current value.
   // If there were just one slider, we might ask Qt for the width of this text with one of the following calls:
   //    const int valueTextWidth = valueTextFontMetrics.width(_valText);              // Pre Qt 5.13
   //    const int valueTextWidth = valueTextFontMetrics.horizontalAdvance(_valText);  // Since Qt 5.13
   // However, we want all the sliders to have exact same width, so we choose some representative text to measure the
   // width of - viz "1.000".
   int valueTextWidth =
#if QT_VERSION < QT_VERSION_CHECK(5,13,0)
      valueTextFontMetrics.width("1.000");
#else
      valueTextFontMetrics.horizontalAdvance("1.000");
#endif

   QLinearGradient glassGrad( QPointF(0,0), QPointF(0,graphicalAreaHeight) );
   glassGrad.setColorAt( 0, QColor(255,255,255,127) );
   glassGrad.setColorAt( 1, QColor(255,255,255,0) );
   QBrush glassBrush(glassGrad);

   // Per https://doc.qt.io/qt-5/highdpi.html, for best High DPI display support, we need to:
   //  • Always use the qreal versions of the QPainter drawing API
   //  • Size windows and dialogs in relation to the corresponding screen size
   //  • Replace hard-coded sizes in layouts and drawing code with values calculated from font metrics or screen size
   QPainter painter(this);

   // Work out the left-to-right (ie x-coordinate) positions of things in the graphical area
   double graphicalAreaWidth  = this->width() - valueTextWidth;
   double range               = this->_max - this->_min;
   double fgRectLeft          = graphicalAreaWidth * ((this->_prefMin - this->_min    )/range);
   double fgRectWidth         = graphicalAreaWidth * ((this->_prefMax - this->_prefMin)/range);
   double indicatorLineMiddle = graphicalAreaWidth * ((this->_val     - this->_min    )/range);
   double indicatorLineLeft   = indicatorLineMiddle - (indicatorLineWidth / 2);

   // Make sure all coordinates are valid.
   fgRectLeft          = qBound(0.0, fgRectLeft,          graphicalAreaWidth);
   fgRectWidth         = qBound(0.0, fgRectWidth,         graphicalAreaWidth - fgRectLeft);
   indicatorLineMiddle = qBound(0.0, indicatorLineMiddle, graphicalAreaWidth - (indicatorLineWidth / 2));
   indicatorLineLeft   = qBound(0.0, indicatorLineLeft,   graphicalAreaWidth - indicatorLineWidth);

   // The left-to-right position of the indicator text (also known as marker text) depends on where the slider is.
   // First we ask the painter what size rectangle it will need to display this text
   painter.setPen(indicatorTextColor);
   painter.setFont(this->indicatorTextFont);
   QRectF indicatorTextRect = painter.boundingRect(QRectF(),
                                                   Qt::AlignCenter | Qt::AlignBottom,
                                                   this->_markerTextIsValue ? this->_valText : this->_markerText);

   // Then we use the size of this rectangle to try to make the middle of the text sit over the indicator marker on
   // the slider - but bounding things so that the text doesn't go off the edge of the slider.
   double indicatorTextLeft = qBound(0.0,
                                    indicatorLineMiddle - (indicatorTextRect.width() / 2),
                                    graphicalAreaWidth - indicatorTextRect.width());

   // Now we can draw the indicator text
   painter.drawText(
      indicatorTextLeft, 0,
      indicatorTextRect.width(), indicatorTextRect.height(),
      Qt::AlignCenter | Qt::AlignBottom,
      this->_markerTextIsValue ? this->_valText : this->_markerText
   );

   // The position of the value text is trivial to work out, so just draw it
   painter.setPen(valueTextColor);
   painter.setFont(this->valueTextFont);
   painter.drawText(graphicalAreaWidth, indicatorTextHeight,
                    valueTextWidth, this->heightInPixels - indicatorTextHeight,
                    Qt::AlignRight | Qt::AlignVCenter,
                    this->_valText );

   painter.save();
      painter.translate(0, indicatorTextRect.height());

      painter.setPen(Qt::NoPen);

      // Make sure anything we draw "inside" the "glass rectangle" stays inside.
      QPainterPath clipRect;
      clipRect.addRoundedRect( QRectF(0, 0, graphicalAreaWidth, graphicalAreaHeight), 8, 8 );
      painter.setClipPath(clipRect);

      // Draw the background rectangle.
      painter.setBrush(_bgBrush);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.drawRoundedRect( QRectF(0, 0, graphicalAreaWidth, graphicalAreaHeight), 8, 8 );
      painter.setRenderHint(QPainter::Antialiasing,false);

      // Draw the style "foreground" rectangle.
      painter.save();
         painter.setBrush(_prefRangeBrush);
         painter.setPen(_prefRangePen);
         painter.setRenderHint(QPainter::Antialiasing);
         //painter.drawRect( QRectF(fgRectLeft, 0, fgRectWidth, graphicalAreaHeight) );
         painter.drawRoundedRect( QRectF(static_cast<qreal>(fgRectLeft), 0, static_cast<qreal>(fgRectWidth), graphicalAreaHeight), 8,8 );
      painter.restore();

      // Draw the indicator.
      painter.setBrush(_markerBrush);
      painter.drawRect( QRectF(static_cast<double>(indicatorLineLeft), 0, indicatorLineWidth, graphicalAreaHeight) );

      // Draw a white to clear gradient to suggest "glassy."
      painter.setBrush(glassBrush);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.drawRoundedRect( QRectF(0, 0, graphicalAreaWidth, graphicalAreaHeight), 8, 8 );
      painter.setRenderHint(QPainter::Antialiasing,false);

      // Draw the ticks.
      painter.setPen(Qt::black);
      if( _tickInterval > 0.0 )
      {
         int secTick = 1;
         for( double currentTick = _min+_tickInterval; _max - currentTick > _tickInterval-1e-6; currentTick += _tickInterval )
         {
            painter.translate( graphicalAreaWidth/(_max-_min) * _tickInterval, 0);
            if( secTick == _secondaryTicks )
            {
               painter.drawLine( QPointF(0,0.25*graphicalAreaHeight), QPointF(0,0.75*graphicalAreaHeight) );
               secTick = 1;
            }
            else
            {
               painter.drawLine( QPointF(0,0.333*graphicalAreaHeight), QPointF(0,0.666*graphicalAreaHeight) );
               ++secTick;
            }
         }
      }
   painter.restore();

   return;
}


void RangedSlider::moveEvent(QMoveEvent *event) {
   // If we've moved, we might be on a new screen with a different DPI resolution
   // .:TBD:. This almost certainly needs further work and further testing.  It's far from clear whether our font size
   //         querying will give different answers just because the app has been moved from one screen to another.
   this->recalculateHeightInPixels();

   QWidget::moveEvent(event);
   return;
}
