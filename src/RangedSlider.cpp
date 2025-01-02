/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * RangedSlider.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#include "RangedSlider.h"

#include <algorithm>

#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPalette>
#include <QRectF>
#include <QToolTip>

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_RangedSlider.cpp"

RangedSlider::RangedSlider(QWidget* parent)
   : QWidget(parent),
     m_min(0.0),
     m_max(1.0),
     m_prefMin(0.25),
     m_prefMax(0.75),
     m_val(0.5),
     m_valText("0.500"),
     m_prec(3),
     m_tickInterval(0),
     m_secondaryTicks(1),
     m_tooltipText(""),
     m_bgBrush(QColor(255,255,255)),
     m_prefRangeBrush(QColor(0,0,0)),
     m_prefRangePen(Qt::NoPen),
     m_markerBrush(QColor(255,255,255)),
     m_markerTextIsValue(false),
     m_valueTextFont("Arial",
                     14,             // QFonts are specified in point size, so the hard-coded number is fine here.
                     QFont::Black),  // Note that QFont::Black is a weight (more bold than ExtraBold), not a colour.
     m_indicatorTextFont("Arial",
                         10,
                         QFont::Normal) // Previously we just did the indicator text in 'default' font
{
   // Ensure this->m_heightInPixels is properly initialised
   this->recalculateHeightInPixels();

   // In principle we want to set our min/max sizes etc here.  However, if, say, a maximumSize property has been set
   // for this object in a Designer UI File (eg ui/mainWindow.ui) then that setting will override this one, because it
   // will be applied later (in fact pretty much straight after this constructor returns).  So we also make this call
   // inside setValue(), which will be invoked m_afterm_ the setter calls that were auto-generated from the Designer UI
   // File.
   this->setSizes();

   // Generate mouse move events whenever mouse movers over widget.
   this->setMouseTracking(true);

   this->repaint();
   return;
}

double RangedSlider::value() const { return m_val; }

void RangedSlider::setPreferredRange(double min, double max) {
   m_prefMin = min;
   m_prefMax = max;

   // Only show tooltips if the range has nonzero size.
   setMouseTracking(min < max);

   m_tooltipText = QString("%1 - %2").arg(min, 0, 'f', m_prec).arg(max, 0, 'f', m_prec);

   update();
   return;
}

void RangedSlider::setPreferredRange(QPair<double,double> minmax) {
   setPreferredRange( minmax.first, minmax.second );
   return;
}

void RangedSlider::setRange(double min, double max) {
   m_min = min;
   m_max = max;
   update();
   return;
}

void RangedSlider::setRange(QPair<double,double> minmax) {
   setRange( minmax.first, minmax.second );
   return;
}

void RangedSlider::setValue(double value) {
   m_val = value;
   m_valText = QString("%1").arg(m_val, 0, 'f', m_prec);
   update();

   // See comment in constructor for why we call this here
   this->setSizes();
   return;
}

void RangedSlider::setPrecision(int precision) {
   m_prec = precision;
   update();
   return;
}

void RangedSlider::setBackgroundBrush(QBrush const & brush) {
   m_bgBrush = brush;
   update();
   return;
}

void RangedSlider::setPreferredRangeBrush(QBrush const & brush) {
   m_prefRangeBrush = brush;
   update();
   return;
}

void RangedSlider::setPreferredRangePen(QPen const & pen) {
   m_prefRangePen = pen;
   update();
   return;
}

void RangedSlider::setMarkerBrush(QBrush const & brush) {
   m_markerBrush = brush;
   update();
   return;
}

void RangedSlider::setMarkerText(QString const & text) {
   m_markerText = text;
   update();
   return;
}

void RangedSlider::setMarkerTextIsValue(bool val) {
   m_markerTextIsValue = val;
   update();
   return;
}

void RangedSlider::setTickMarks(double primaryInterval, int secondaryTicks) {
   m_secondaryTicks = (secondaryTicks<1)? 1 : secondaryTicks;
   m_tickInterval = primaryInterval/m_secondaryTicks;

   update();
   return;
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
   // font.  It is helpful for the indicator text to have a bit of space below it before we draw the graphical bit, and
   // it's not a large font in any case.  However, for large value text font we don't necessarily need all this space
   // because we currently only show digits and decimal points, which don't require space below the baseline.  However,
   // assumptions about space below the baseline are locale-specific, so, say, using ascent() instead of lineSpacing()
   // could end up painting us into a corner.
   //
   QFontMetrics indicatorTextFontMetrics(this->m_indicatorTextFont);
   QFontMetrics valueTextFontMetrics(this->m_valueTextFont);
   this->m_heightInPixels = indicatorTextFontMetrics.lineSpacing() + valueTextFontMetrics.lineSpacing();
   return;
}

void RangedSlider::setSizes() {
   // Caller's responsibility to have recently called this->recalculateHeightInPixels().  (See comment in that function
   // for how we choose minimum width.)
   this->setMinimumSize(2 * this->m_heightInPixels, this->m_heightInPixels);

   this->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );

   // There no particular reason to limit our horizontal size, so, in principle, this call asks that there be no such
   // (practical) limit.
   this->setMaximumWidth(QWIDGETSIZE_MAX);

   return;
}

QSize RangedSlider::sizeHint() const {
   this->recalculateHeightInPixels();
   return QSize(4 * this->m_heightInPixels, this->m_heightInPixels);
}

void RangedSlider::mouseMoveEvent(QMouseEvent* event) {
   event->accept();

   QPoint tipPoint( mapToGlobal(QPoint(0,0)) );
   QToolTip::showText( tipPoint, m_tooltipText, this );
   return;
}

void RangedSlider::paintEvent([[maybe_unused]] QPaintEvent * event) {
   //
   // Simplistically, the high-level layout of the slider is:
   //
   //    ╔════════════════════════════════════════════════════════════════╗
   //    ║                  Indicator text                   ┊ B L A N K  ║
   //    ║┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈║
   //    ║ ╭────────────────┲━━━━━━━━━━━━━━━━━━━━┱─────────╮ ┊            ║
   //    ║ │                ┃            ┋       ┃         │ ┊ Value text ║
   //    ║ ╰────────────────┺━━━━━━━━━━━━━━━━━━━━┹─────────╯ ┊            ║
   //    ║ <---------------- Graphical Area ---------------> ┊            ║
   //    ╚════════════════════════════════════════════════════════════════╝
   //      ↑                ↑            ↑       ↑         ↑
   //      m_min            m_prefMin    m_val   m_prefMax m_max
   //      │                │            │       │         │
   //      0.0              fgRectLeft   │       │         graphicalAreaWidth
   //                                    │       │
   //                                    │       fgRectLeft + fgRectWidth
   //                                    │
   //                                    indicatorLineMiddle
   //
   // The graphical area has:
   //  - a background rectangle of the full width of the area, representing the range from this->m_min to this->m_max
   //  - a foreground rectangle showing the sub-range of this background from this->m_prefMin to this->m_prefMax
   //  - a line ("the indicator") showing where this->m_val lies in the (this->m_min to this->m_max) range
   //
   // The indicator text sits above the indicator line and shows either its value (this->m_valText) or some textual
   // description (eg "Slightly Malty" on the IBU/GU scale) which comes from this->m_markerText.
   //
   // In principle, we could have the value text a slightly different height than the graphical area - eg to help
   // squeeze into smaller available vertical space on small screens (as we know there is blank space of
   // indicatorTextHeight pixels above the space for the value text).
   //
   // The value text also shows this->m_valText.
   //

   QFontMetrics indicatorTextFontMetrics(this->m_indicatorTextFont);
   int indicatorTextHeight = indicatorTextFontMetrics.lineSpacing();

   // The heights of the slider graphic and the value text are usually the same, but we calculate them differently in
   // case, in future, we want to squeeze things up a bit.
   QFontMetrics valueTextFontMetrics(this->m_valueTextFont);
   int const valueTextHeight = valueTextFontMetrics.lineSpacing();

   int const graphicalAreaHeight = this->height() - indicatorTextHeight;

   // Although the Qt calls take an x- and a y- radius, we want the radius on the rectangle corners to be the same
   // vertically and horizontally, so only define one measure here.
   int const rectangleCornerRadius = graphicalAreaHeight / 4;

   static const QPalette palette(QApplication::palette());
   static const int indicatorLineWidth   = 4;
   static const QColor fgRectColor(0,127,0);
   static const QColor indicatorTextColor(0,0,0);
   static const QColor valueTextColor(0,127,0);

   // We need to allow for the width of the text that displays to the right of the slider showing the current value.
   // If there were just one slider, we might ask Qt for the width of this text with the following call:
   //    const int valueTextWidth = valueTextFontMetrics.horizontalAdvance(m_valText);  // Since Qt 5.13
   // However, we want all the sliders to have exact same width, so we choose some representative text to measure the
   // width of.  We assume that all sliders show no more than 4 digits and a decimal point, and then add a space to
   // ensure a gap between the value text and the graphical area.  (Note that digits are all the same width in the font
   // we are using.
   int valueTextWidth = valueTextFontMetrics.horizontalAdvance(" 1.000");

   QLinearGradient glassGrad( QPointF(0, 0), QPointF(0, graphicalAreaHeight) );
   glassGrad.setColorAt(0, QColor(255, 255, 255, 127));
   glassGrad.setColorAt(1, QColor(255, 255, 255,   0));
   QBrush glassBrush(glassGrad);

   // Per https://doc.qt.io/qt-5/highdpi.html, for best High DPI display support, we need to:
   //  • Always use the qreal versions of the QPainter drawing API
   //  • Size windows and dialogs in relation to the corresponding screen size
   //  • Replace hard-coded sizes in layouts and drawing code with values calculated from font metrics or screen size
   QPainter painter(this);

   // Usually leave this debug log commented out unless trouble-shooting as it generates a lot of logging
//   qDebug() <<
//      Q_FUNC_INFO << "Width:" << this->width() << "; valueTextWidth:" << valueTextWidth << "; m_min:" << this->m_min <<
//      "; m_max:" << this->m_max << "; m_prefMin:" << this->m_prefMin << "; m_prefMax:" << this->m_prefMax <<
//      "; m_val" << this->m_val;

   //
   // Work out the left-to-right (ie x-coordinate) positions of things in the graphical area.  Note that we have to
   // gracefully handle extreme cases -- eg where we do not have enough space to paint ourselves.  In particular, we
   // have to be careful about using calculated values as the min and/or max parameters to qBound, because that function
   // (quite reasonably) asserts at runtime that min < max, and it's easy to break that if you're doing simple
   // calculations for how to draw the graphical area when there isn't really enough space to draw it.
   //
   double const graphicalAreaWidth  = static_cast<double>(std::max(0, this->width() - valueTextWidth));
   double const range               = this->m_max - this->m_min;
   double fgRectLeft          = std::max(0.0, graphicalAreaWidth * ((this->m_prefMin - this->m_min    )/range));
   double fgRectWidth         = std::max(0.0, graphicalAreaWidth * ((this->m_prefMax - this->m_prefMin)/range));
   double indicatorLineMiddle = std::max(0.0, graphicalAreaWidth * ((this->m_val     - this->m_min    )/range));
   double indicatorLineLeft   = std::max(0.0, indicatorLineMiddle - (static_cast<double>(indicatorLineWidth) / 2.0));
   // Usually leave this debug log commented out unless trouble-shooting as it generates a lot of logging
//   qDebug() <<
//      Q_FUNC_INFO << "graphicalAreaWidth:" << graphicalAreaWidth << "; range:" << range << "; fgRectLeft:" <<
//      fgRectLeft << "; fgRectWidth:" << fgRectWidth << "; indicatorLineMiddle:" << indicatorLineMiddle <<
//      "; indicatorLineLeft:" << indicatorLineLeft;

   //
   // Make sure all coordinates are valid.
   //
   fgRectLeft          = std::min(fgRectLeft,          graphicalAreaWidth);
   fgRectWidth         = std::max(0.0, std::min(fgRectWidth,         graphicalAreaWidth - fgRectLeft));
   indicatorLineMiddle = std::max(0.0, std::min(indicatorLineMiddle, graphicalAreaWidth - (indicatorLineWidth / 2)));
   indicatorLineLeft   = std::max(0.0, std::min(indicatorLineLeft,   graphicalAreaWidth - indicatorLineWidth));

   // The left-to-right position of the indicator text (also known as marker text) depends on where the slider is.
   // First we ask the painter what size rectangle it will need to display this text
   painter.setPen(indicatorTextColor);
   painter.setFont(this->m_indicatorTextFont);
   QRectF const indicatorTextRect =
      painter.boundingRect(QRectF(),
                           Qt::AlignCenter | Qt::AlignBottom,
                           this->m_markerTextIsValue ? this->m_valText : this->m_markerText);

   //
   // Then we use the size of this rectangle to try to make the middle of the text sit over the indicator marker on
   // the slider - but bounding things so that the text doesn't go off the edge of the slider.
   //
   // This is a more robust way of writing
   //
   //    double indicatorTextLeft = qBound(0.0,
   //                                     indicatorLineMiddle - (indicatorTextRect.width() / 2),
   //                                     graphicalAreaWidth - indicatorTextRect.width());
   //
   double indicatorTextLeft = std::max(0.0, indicatorLineMiddle - (indicatorTextRect.width() / 2));
   indicatorTextLeft = std::max(0.0, std::min(indicatorTextLeft, graphicalAreaWidth - indicatorTextRect.width()));

   // Now we can draw the indicator text
   painter.drawText(
      indicatorTextLeft, 0,
      indicatorTextRect.width(), indicatorTextRect.height(),
      Qt::AlignCenter | Qt::AlignBottom,
      this->m_markerTextIsValue ? this->m_valText : this->m_markerText
   );

   // Next draw the value text
   // We work out its vertical position relative to the bottom of the graphical area in case (in future) we want to be
   // able to use some of the blank space above it (to the right of the indicator text).
   painter.setPen(valueTextColor);
   painter.setFont(this->m_valueTextFont);
   painter.drawText(graphicalAreaWidth, this->height() - valueTextHeight,
                    valueTextWidth, valueTextHeight,
                    Qt::AlignRight | Qt::AlignVCenter,
                    this->m_valText );

   // All the rest of what we need to do is inside the graphical area, so move the origin to the top-left corner of it
   painter.translate(0, indicatorTextRect.height());
   painter.setPen(Qt::NoPen);

   // Make sure anything we draw "inside" the "glass rectangle" stays inside.
   QPainterPath clipRect;
   clipRect.addRoundedRect( QRectF(0, 0, graphicalAreaWidth, graphicalAreaHeight),
                            rectangleCornerRadius,
                            rectangleCornerRadius );
   painter.setClipPath(clipRect);

   // Draw the background rectangle.
   painter.setBrush(m_bgBrush);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.drawRoundedRect( QRectF(0, 0, graphicalAreaWidth, graphicalAreaHeight),
                            rectangleCornerRadius,
                            rectangleCornerRadius );
   painter.setRenderHint(QPainter::Antialiasing, false);

   // Draw the style "foreground" rectangle.
   painter.save();
      painter.setBrush(m_prefRangeBrush);
      painter.setPen(m_prefRangePen);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.drawRoundedRect( QRectF(fgRectLeft, 0, fgRectWidth, graphicalAreaHeight),
                               rectangleCornerRadius,
                               rectangleCornerRadius );
   painter.restore();

   // Draw the indicator.
   painter.setBrush(m_markerBrush);
   painter.drawRect( QRectF(indicatorLineLeft, 0, indicatorLineWidth, graphicalAreaHeight) );

   // Draw a white-to-clear gradient to suggest "glassy."
   painter.setBrush(glassBrush);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.drawRoundedRect( QRectF(0, 0, graphicalAreaWidth, graphicalAreaHeight),
                            rectangleCornerRadius,
                            rectangleCornerRadius );
   painter.setRenderHint(QPainter::Antialiasing, false);

   // Draw the ticks.
   painter.setPen(Qt::black);
   if( m_tickInterval > 0.0 )
   {
      int secTick = 1;
      for( double currentTick = m_min+m_tickInterval; m_max - currentTick > m_tickInterval-1e-6; currentTick += m_tickInterval )
      {
         painter.translate( graphicalAreaWidth/(m_max-m_min) * m_tickInterval, 0);
         if( secTick == m_secondaryTicks )
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

   return;
}


void RangedSlider::moveEvent(QMoveEvent *event) {
   // If we've moved, we might be on a new screen with a different DPI resolution...
   // .:TBD:. This almost certainly needs further work and further testing.  It's far from clear whether our font size
   //         querying will give different answers just because the app has been moved from one screen to another.
   this->recalculateHeightInPixels();

   QWidget::moveEvent(event);
   return;
}
