/*
 * BtLabel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#ifndef BTLABEL_H
#define BTLABEL_H


#include <QLabel>
#include <QHash>
#include <QMenu>
#include <QAction>
#include <QPoint>

#include "UnitSystem.h"

class BtLabel;
class BtColorLabel;
class BtDensityLabel;
class BtMassLabel;
class BtTemperatureLabel;
class BtVolumeLabel;
class BtTimeLabel;
class BtMixedLabel;
class BtDateLabel;
class BtDiastaticPowerLabel;

/*!
 * \class BtLabel
 * \author Mik Firestone
 *
 * \brief Performs the necessary magic to select display units for any label.
 * It will need to gracefully handle labels for which no unit is set..
 */
class BtLabel : public QLabel
{
   Q_OBJECT
   Q_ENUMS(LabelType)

public:
   //! What kinds of units are available for labels
   enum LabelType{
      NONE,
      COLOR,
      DENSITY,
      MASS,
      TEMPERATURE,
      VOLUME,
      TIME,
      MIXED,
      DATE,
      DIASTATIC_POWER
   };

   BtLabel(QWidget* parent = 0, LabelType lType = NONE);

public slots:
   void popContextMenu(const QPoint &point);


signals:
   void labelChanged(Unit::unitDisplay oldUnit, Unit::unitScale oldScale);

// Using protected instead of private allows me to not use the friends
// declaration
protected:
   LabelType whatAmI;
   QString propertyName;
   QString _section;
   QWidget *btParent;
   QMenu* _menu;

   void initializeSection();
   void initializeProperty();
   void initializeMenu();

};

class BtColorLabel : public BtLabel
{
   Q_OBJECT
public:
   BtColorLabel(QWidget* parent = 0);
};

class BtDensityLabel : public BtLabel
{
   Q_OBJECT
public:
   BtDensityLabel(QWidget* parent = 0);
};

class BtMassLabel : public BtLabel
{
   Q_OBJECT
public:
   BtMassLabel(QWidget* parent = 0);
};

class BtTemperatureLabel : public BtLabel
{
   Q_OBJECT
public:
   BtTemperatureLabel(QWidget* parent = 0);
};

class BtVolumeLabel : public BtLabel
{
   Q_OBJECT
public:
   BtVolumeLabel(QWidget* parent = 0);
};

class BtTimeLabel : public BtLabel
{
   Q_OBJECT
public:
   BtTimeLabel(QWidget* parent = 0);
};

class BtMixedLabel : public BtLabel
{
   Q_OBJECT
public:
   BtMixedLabel(QWidget* parent = 0);
};

class BtDateLabel : public BtLabel
{
   Q_OBJECT
public:
   BtDateLabel(QWidget* parent = 0);
};

class BtDiastaticPowerLabel : public BtLabel
{
   Q_OBJECT
public:
   BtDiastaticPowerLabel(QWidget* parent = 0);
};

#endif
