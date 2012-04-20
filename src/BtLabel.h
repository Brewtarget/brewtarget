/*
 * BtLabel.h is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
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

#ifndef BTLABEL_H
#define BTLABEL_H


#include <QLabel>
#include <QHash>
#include <QMenu>
#include <QAction>
#include <QPoint>

#include "UnitSystem.h"

class BtLabel;
class BtMassLabel;
class BtVolumeLabel;
class BtGravityLabel;

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
   enum LabelType{ NONE, GRAVITY, MASS, VOLUME };

   BtLabel(QWidget* parent = 0, LabelType lType = NONE);

public slots:
   void popContextMenu(const QPoint &point);
   //! these work for mass and volume, but not so well for gravity
   void setSI();
   void setUsTraditional();
   void setBritishImperial();
   //! these work for gravity
   void setPlato();
   void setSg();

   //! I need to stop using this trick?
   friend class BtMassLabel;
   friend class BtVolumeLabel;
   friend class BtGravityLabel;

signals:
   void labelChanged(QString field);

private:
   LabelType whatAmI;
   QMenu* cachedMenu;
   iUnitSystem selected;
   QString propertyName;
   QWidget *btParent;

   //! Only need one for mass or volume. Gravity is odd
   QMenu* setupGravityMenu();
   QMenu* setupMassVolumeMenu();
};

class BtMassLabel : public BtLabel
{
   Q_OBJECT
public:
   BtMassLabel(QWidget* parent = 0);
};

class BtVolumeLabel : public BtLabel
{
   Q_OBJECT
public:
   BtVolumeLabel(QWidget* parent = 0);
};

class BtGravityLabel : public BtLabel
{
   Q_OBJECT
public:
   BtGravityLabel(QWidget* parent = 0);
};

#endif
