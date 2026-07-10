/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * RadarChart.cpp is part of Brewtarget, and is copyright the following authors 2021-2026:
 *   • Matt Young <mfsy@yahoo.com>
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
#include "RadarChart.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QHash>

// Internal constants
namespace {
   struct LineStyleAndObject{
      QColor color;
      Qt::PenStyle penStyle;
      NamedEntity const * object;
   };

   // Qt measures angles either in degrees or sixteenths of a degree, measured clockwise starting from 12 o'clock as 0°
   //
   // For <cmath> functions, angles are all in radians, and the "natural" coordinate system measures anti-clockwise
   // starting from 3 o'clock as 0rad.  There are 2π radians in a circle.
   double constexpr RadiansInACircle = 2 * std::numbers::pi;

   double constexpr StartingAngleInRadians = RadiansInACircle / 4;

   // QFonts are specified in point size, so the hard-coded numbers are fine here, even on HDPI displays
   // Note that if QFont::Black is specified as third parameter to QFont constructor, it is a weight (beyond ExtraBold)
   // not a colour!
   QFont  const axisLabelFont{"Arial Narrow",
                              12,
                              QFont::Medium};
   QColor const axisColor{Qt::black};
   QPen   const axisPen{axisColor};

   QFont  const axisValueFont{"Arial",
                              11,
                              QFont::Normal};

   // The colors differ for different series, but the other pen attributes are the same
   QBrush const allSeriesBrush{Qt::SolidPattern};
   int constexpr allSeriesLineWidth = 4;
   QPen const allSeriesPen{allSeriesBrush, allSeriesLineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
}

// This private implementation class holds all private non-virtual members of RadarChart
class RadarChart::impl {
public:

   /**
    * Constructor
    */
   impl() {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * (Re)calculate the maximum value in all the series, so we can scale the chart
    */
   void updateMaxAxisValue() {
      double maxInAllSeries = 0.0;
      for (auto const & currSeries : std::as_const(this->m_allSeries)) {
         // It's a coding error if we have a series with a null pointer.  (If the object has gone away, we should remove
         // the series.)
         Q_ASSERT(currSeries.object);
         auto const maxVariableInThisSeries = std::ranges::max_element(
            this->m_variableNames,
            [currSeries] (RadarChart::VariableName const & lhs, RadarChart::VariableName const & rhs) {
               return (currSeries.object->property(*lhs.propertyName).toDouble() <
                  currSeries.object->property(*rhs.propertyName).toDouble());
            }
         );

         double const maxValueInThisSeries =
            currSeries.object->property(*maxVariableInThisSeries->propertyName).toDouble();
         maxInAllSeries = std::max(maxInAllSeries, maxValueInThisSeries);
      }

      // Now round up maxInAllSeries to the nearest multiple of this->axisMarkInterval.  If the result is zero (because
      // maxInAllSeries was zero), then set this->maxAxisValue to this->axisMarkInterval
      this->m_maxAxisValue =
         std::max(ceil(maxInAllSeries / this->m_axisMarkInterval) * this->m_axisMarkInterval, this->m_axisMarkInterval);

      // It's a programming error if we haven't managed to set a positive max axis value
      Q_ASSERT(this->m_maxAxisValue > 0);
      return;
   }

   /**
    * @brief Convert canonical polar coordinates to Qt Cartesian coordinates (which NB are upside-down from canonical
    *        Cartesian coordinates: in Qt, the x-axis grows to the right and the y-axis grows downwards).
    */
   static QPointF polarToQtCartesian(double const radial, double const angular) {
      return QPointF(radial * cos(angular), -radial * sin(angular));
   }

   /**
    * @brief Convert radians to degrees.  NB this does not convert the origin.
    */
   double radiansToDegrees(double radians) {
      return 360.0 * radians / RadiansInACircle;
   }

   //============================================== impl member variables ==============================================
   QString m_unitsName = "";
   double  m_axisMarkInterval = 0.0;
   QVector<RadarChart::VariableName> m_variableNames;
   QHash<QString, LineStyleAndObject > m_allSeries;

   // Angle in radians between the radii (aka spokes aka axes)
   // Calculated in RadarChart::init()
   double m_angleInRadiansBetweenAxes = 0.0;

   // The smallest multiple of axisMarkInterval that is greater than or equal to the maximum value of any point on the
   // graph.  Used to size/scale the axes (aka radii aka spokes)
   double m_maxAxisValue = 0.0;

};

RadarChart::RadarChart(QWidget * parent) : QWidget(parent),
                                           pimpl{std::make_unique<impl>()} {
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
RadarChart::~RadarChart() = default;


void RadarChart::init(QString const & unitsName,
                      double const axisMarkInterval,
                      QVector<RadarChart::VariableName> const variableNames) {
   this->pimpl->m_unitsName = unitsName;
   this->pimpl->m_axisMarkInterval = axisMarkInterval;
   this->pimpl->m_maxAxisValue     = axisMarkInterval; // Starting assumption before we have any data
   this->pimpl->m_variableNames = variableNames;

   // It's a programming error to supply a zero or negative axis interval
   Q_ASSERT(axisMarkInterval > 0);

   // It's a programming error to supply an empty vector to this function
   Q_ASSERT(variableNames.size() > 0);

   this->pimpl->m_angleInRadiansBetweenAxes = RadiansInACircle / variableNames.size();

   qDebug() <<
      Q_FUNC_INFO << "axisMarkInterval:" << this->pimpl->m_axisMarkInterval << "; maxAxisValue:" <<
      this->pimpl->m_maxAxisValue;
   return;
}

void RadarChart::addSeries(QString const & name,
                           NamedEntity const & object,
                           QColor const color,
                           Qt::PenStyle const penStyle) {

   // Can't store an object or a reference in a hash, but we can store a pointer
   // (Still good to have the object passed in by reference as it saves having to check/assert for null pointers.)
   this->pimpl->m_allSeries.insert(name, {color, penStyle, &object});
   qDebug() <<
      Q_FUNC_INFO << "Added" << name << object.metaObject()->className() << "#" << object.key() << ":" << object.name();
   // We deliberately don't replot here because the caller might need to make further calls before it is OK to redraw
   // the graph.
   return;
}

void RadarChart::removeSeries(QString const & name) {
   // It doesn't matter if the series is not present - QHash remove will just return false, but nothing bad happens
   this->pimpl->m_allSeries.remove(name);
   qDebug() << Q_FUNC_INFO << "Removed" << name;
   // We deliberately don't replot here because the caller might need to make further calls before it is OK to redraw
   // the graph.
   return;
}


void RadarChart::replot() {
   this->pimpl->updateMaxAxisValue();

   // Tell Qt to repaint the widget next time it returns to the main event loop
   this->update();
   return;
}

void RadarChart::paintEvent(QPaintEvent *event) {
   Q_UNUSED(event);

   // This is a bit of a rule of thumb.  Allowing the exact space for the chart and its axis labels would be a hard
   // sum.  This works for short labels.
   double const axisLengthInPixels = std::min(this->width(), this->height()) / 3;

   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing);
   painter.translate(this->rect().center());

   //
   // Draw and label the axes lines
   //
   QPointF constexpr origin{};
   painter.setFont(axisLabelFont);
   painter.setPen(axisPen);
   for (int ii = 0; ii < this->pimpl->m_variableNames.size(); ++ii) {
      // The <cmath> functions won't mind the angle being more than 2π
      double const angleInRadians = StartingAngleInRadians + ii * this->pimpl->m_angleInRadiansBetweenAxes;

      QPointF axisEnd = this->pimpl->polarToQtCartesian(axisLengthInPixels, angleInRadians);
      painter.drawLine(origin, axisEnd);

      // Where exactly to put the text label on the end of the axis depends which direction it's pointing in.
      // Essentially there are four possibilities depending on which of the four quadrants of the circle we're in.
      // Fortunately, having the coordinate origin at the centre of the rectangle makes life easy for us:
      //
      //                      |
      //       X < 0, Y <= 0  |  X >= 0, Y <= 0
      //                      |
      //    ------------------+------------------
      //                      |
      //       X < 0, Y > 0   |  X >= 0, Y > 0
      //                      |
      //
      // If we think of the origin as a magnet, that tells us how to align the text in each quadrant.  (Where we are at
      // 0, then our bias is towards the top right quadrant.)
      //
      int alignFlags = (axisEnd.x() < 0 ? Qt::AlignRight : Qt::AlignLeft) |
                       (axisEnd.y() <= 0 ? Qt::AlignBottom : Qt::AlignTop);
      // Get a rectangle of the size necessary to contain the label text
      QRectF axisLabelRect = painter.boundingRect(QRectF(),
                                                  alignFlags,
                                                  this->pimpl->m_variableNames[ii].displayName);
      // If we're in the bottom-right quadrant, then the coordinates of the axis end are also good for the text
      // rectangle.  If not, it needs correcting.
      double const labelX = axisEnd.x() - (axisEnd.x() <  0 ? axisLabelRect.width() : 0);
      double const labelY = axisEnd.y() - (axisEnd.y() <= 0 ? axisLabelRect.height() : 0);
      axisLabelRect.moveTo(labelX, labelY);
      painter.drawText(axisLabelRect, alignFlags, this->pimpl->m_variableNames[ii].displayName);
   }

   // TODO Maybe different weight for outer circle?
   //
   // Instead of tick marks on the axes we draw circles.  In principle, we want a circle at every multiple of
   // this->pimpl->axisMarkInterval.  In practice, we need to check whether this would make the circles too close
   // together and, if so, use a suitable multiple of this->pimpl->axisMarkInterval.  We use the height of the
   // value labelling font as a guide to what "too close together" would be.  From trial-and-error, it seems like 1.5×
   // the height of the font is a sensible minimum spacing for the circles, otherwise the value marks look too squashed
   // in.
   //
   // Although we are ultimately looking for an integer, we need to do the sums in floating point to handle extreme
   // cases, such as where pimpl->m_maxAxisValue is so large that
   // `axisLengthInPixels * pimpl->m_axisMarkInterval / pimpl->m_maxAxisValue` would round to 0 in integer arithmetic.
   //
   // In principle we want the smallest positive integer n, such that:
   //
   //    n × (axisLengthInPixels × pimpl->m_axisMarkInterval / pimpl->m_maxAxisValue) >= minPixelDistanceBetweenCircles
   //
   // In other words:
   //
   //           minPixelDistanceBetweenCircles × pimpl->m_maxAxisValue
   //    n  >=  ------------------------------------------------------
   //              axisLengthInPixels × pimpl->m_axisMarkInterval
   //
   // Once we found this n, then:
   //                                  n × axisLengthInPixels × pimpl->m_axisMarkInterval
   //    pixelDistanceBetweenCircles = --------------------------------------------------
   //                                               pimpl->m_maxAxisValue
   //
   // We use the variable 'multiplier' in the code to represent 'n' in the equations above.
   //
   QFontMetrics const axisValueFontMetrics(axisValueFont);
   int const minPixelDistanceBetweenCircles = 1.5 * axisValueFontMetrics.lineSpacing();
   double const multiplier = std::ceil(
      (static_cast<double>(minPixelDistanceBetweenCircles) * this->pimpl->m_maxAxisValue) /
         (static_cast<double>(axisLengthInPixels) * this->pimpl->m_axisMarkInterval)
   );
   Q_ASSERT(multiplier >= 1);
   int const pixelDistanceBetweenCircles =
      multiplier * static_cast<double>(axisLengthInPixels) * this->pimpl->m_axisMarkInterval /
      this->pimpl->m_maxAxisValue;
   int const valueDifferenceBetweenCircles = multiplier * this->pimpl->m_axisMarkInterval;
   int const numCircles = axisLengthInPixels / pixelDistanceBetweenCircles;

   //
   // We want to rotate the canvas to put the value markers on the circle at a jaunty angle.  This has to be
   // done in degrees rather than radians.  The specific jaunty angle we want is half-way between two of the axes.
   //
   painter.save();
   painter.rotate(this->pimpl->radiansToDegrees(this->pimpl->m_angleInRadiansBetweenAxes / 2));
   painter.setFont(axisValueFont);

   for (int jj = 1; jj <= numCircles; ++jj) {
      double const radius = jj * pixelDistanceBetweenCircles;
      // Drawing a circle in Qt is a bit convoluted because you have to start with the bounding rectangle
      // Then you have to draw an arc whose angle is measured in 16ths of a degree
      QRectF boundingRectangle{origin.x() - radius, origin.y() - radius, 2 * radius, 2 * radius};
      painter.drawArc(boundingRectangle, 0, 16 * 360);

      QString circleLabel = QStringLiteral("%1").arg(std::round(jj * valueDifferenceBetweenCircles));
      painter.drawText(0, -radius, circleLabel);
   }
//   painter.drawText(0, axisLengthInPixels, this->pimpl->unitsName);
   painter.restore();

   //
   //
   // Now plot the actual data
   //
   QPen seriesPen{allSeriesPen};

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
   for (auto const & [name, lineStyleAndObject] : this->pimpl->m_allSeries.asKeyValueRange()) {
//      qDebug() << Q_FUNC_INFO << "Plotting series:" << name << "in" << lineStyleAndObject.color.name();
#else
   for (auto const & lineStyleAndObject : std::as_const(this->pimpl->m_allSeries)) {
#endif
      seriesPen.setColor(lineStyleAndObject.color);
      seriesPen.setStyle(lineStyleAndObject.penStyle);
      painter.setPen(seriesPen);
      QVector<QPointF> seriesPoints(this->pimpl->m_variableNames.size());
      for (int ii = 0; ii < this->pimpl->m_variableNames.size(); ++ii) {
         double const angleInRadians {StartingAngleInRadians + ii * this->pimpl->m_angleInRadiansBetweenAxes};
         double const value {
            lineStyleAndObject.object->property(*this->pimpl->m_variableNames[ii].propertyName).toDouble()
         };

         seriesPoints[ii] = this->pimpl->polarToQtCartesian(
            axisLengthInPixels * value / this->pimpl->m_maxAxisValue,
            angleInRadians
         );

         // Join the dots
         if (ii > 0) {
            painter.drawLine(seriesPoints[ii - 1], seriesPoints[ii]);
            if (ii == this->pimpl->m_variableNames.size() - 1) {
               painter.drawLine(seriesPoints[ii], seriesPoints[0]);
            }
         }
      }
   }

   return;
}