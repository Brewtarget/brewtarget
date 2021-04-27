/*
 * RadarChart.h is part of Brewtarget, and is Copyright the following
 * authors 2021
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef _RADARCHART_H
#define _RADARCHART_H

#include <memory> // For PImpl

#include <QWidget>
#include <QString>
#include <QVector>


/**
 * @brief Plots radar charts (see https://en.wikipedia.org/wiki/Radar_chart) of the specified Qt properties of one or
 *        more QObject.  Each axis of the chart represents a different Qt property.
 */
class RadarChart: public QWidget {
public:
   /**
    * @param parent
    */
   RadarChart(QWidget *parent = nullptr);
   virtual ~RadarChart();

   /**
    * Identifies a variable to plot on the chart.  Each variable in a series is a Qt property, identified by its
    * internal property name.  We also have a localised display name to label the axis for this variable.
    */
   struct VariableName {
      char const * propertyName;
      QString displayName;
   };

   /**
    * @brief Initialise the chart
    * @param unitsName name of the units being measured - eg "PPM" for salt concentrations in water
    * @param axisMarkInterval spacing between ticks on axes - eg 50 means put ticks at 50, 100, 150, 200, etc
    * @param propertyToDisplayNames an ordered list of all the Qt property names to plot with, for each one, the
    *                               (localised) axis name to display ¥¥¥¥of the radii (aka spokes, axes) in the order
    *                               you want them displayed.  (First item in the list will be the axis at 12 o'clock
    *                               and the others will follow anti-clockwise.)
    */
   void init(QString const unitsName,
             double axisMarkInterval,
             QVector<RadarChart::VariableName> const variableNames);

   /**
    * @brief Add a series to the chart
    * @param name  Unique displayable name for the series.  If the chart already has a series with this name, it will
    *              be replaced by the one supplied here.
    * @param color  Color in which to plot the series
    * @param values  The object whose properties are to be plotted for this series
    */
   void addSeries(QString name, QColor color, QObject const & object);

   /**
    * @brief (Re)plot the graph.  Call this when there's a change to a property on an object being plotted, so that the
    *        graph can be replotted
    */
   void replot();


protected:
   void paintEvent(QPaintEvent *event);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
