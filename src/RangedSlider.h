/*
 * RangedSlider.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2020
 * - Matt Young <mfsy@yahoo.com>
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

#ifndef RANGEDSLIDER_H
#define RANGEDSLIDER_H

#include <QWidget>
#include <QSize>
#include <QString>
#include <QBrush>
#include <QPen>
class QPaintEvent;
class QMouseEvent;

/*!
 * \brief Widget to display a number with an optional range on a type of read-only slider.
 * \author Philip G. Lee
 */
class RangedSlider : public QWidget
{
   Q_OBJECT

public:
   RangedSlider(QWidget* parent=0);

   Q_PROPERTY( double value READ value WRITE setValue )

   double value() const { return _val; }

   //! \brief Set the background brush for the widget.
   void setBackgroundBrush( QBrush const& brush );
   //! \brief Set the brush for the preffered range.
   void setPreferredRangeBrush( QBrush const& brush );
   //! \brief Set the pen for the preferred range
   void setPreferredRangePen( QPen const& pen );
   //! \brief Set the brush for the marker.
   void setMarkerBrush( QBrush const& brush );
   //! \brief Set the text displayed above the marker.
   void setMarkerText( QString const& text );
   //! \brief If true, the marker text will always be updated to the value given by \c setValue().
   void setMarkerTextIsValue(bool val);

   /*!
    * \brief Set the tick mark intervals.
    *
    * If either parameter is <= 0, then the tick marks are not drawn.
    *
    * \param primaryInterval How often to draw big tick marks.
    * \param secondaryTicks Number of secondary ticks per primary tick.
    */
   void setTickMarks( double primaryInterval, int secondaryTicks = 1 );

   //! \brief Set the \c precision for displaying values.
   void setPrecision(int precision);

   //! \brief Reimplemented from QWidget.
   virtual QSize sizeHint() const;

public slots:

   //! \brief Set the \c value for the indicator.
   void setValue(double value);

   /*!
    * \brief Set the range of values considered to be *best*
    *
    * \param range \c range.first and \c range.second are the min and max
    *        values for the preferred range resp.
    */
   void setPreferredRange(QPair<double,double> range);

   /*!
    * \brief Set the range of values that the widget displays
    *
    * \param range \c range.first and \c range.second are the min and max
    *        values for the preferred range resp.
    */
   void setRange(QPair<double,double> range);

   //! \brief Convenience method for setting the widget range
   void setRange( double min, double max );

   //! \brief Convenience method for setting the preferred range
   //         Note that this is completely unrelated to "preferred size".
   void setPreferredRange( double min, double max );

protected:
   //! \brief Reimplemented from QWidget.
   virtual void paintEvent(QPaintEvent* event);
   //! \brief Reimplemented from QWidget for popup on mouseover.
   virtual void mouseMoveEvent(QMouseEvent* event);
   //! \brief Reimplemented from QWidget.
   virtual void moveEvent(QMoveEvent *event);

private:
   /**
    * Sets minimum / maximum sizes and resize policy
    */
   void setSizes();
   void recalculateHeightInPixels() const;

   /**
    * Minimum value the widget displays
    */
   double _min;
   /**
    * Maximum value the widget displays
    */
   double _max;

   /**
    * Minimum value for the "best" sub-range
    */
   double _prefMin;
   /**
    * Maximum value for the "best" sub-range
    */
   double _prefMax;
   double _val;
   QString _valText;
   QString _markerText;
   int _prec;
   double _tickInterval;
   int _secondaryTicks;
   QString _tooltipText;
   QBrush _bgBrush;
   QBrush _prefRangeBrush;
   QPen _prefRangePen;
   QBrush _markerBrush;
   bool _markerTextIsValue;

   /**
    * The font used for showing the value at the right-hand side of the slider
    */
   QFont const valueTextFont;

   /**
    * The font used for showing the indicator above the "needle" on the slider.  Often this is just showing the same as
    * the value - eg OG, FG, ABV - but sometimes it's something else - eg descriptive text such as "slightly hoppy" for
    * the IBU/GU reading.
    */
   QFont const indicatorTextFont;

   /**
    * Since preferred and minimum dimensions are all based off a height we need to calculate based on the resolution of
    * the current display (see more detailed comment in implementation of recalculateSizes()), it is useful to store
    * that height here.  However, its value is not really part of the current value/state of the object, hence mutable
    * (ie OK to change in a const function).
    */
   mutable int heightInPixels;
};

#endif /*RANGEDSLIDER_H*/
