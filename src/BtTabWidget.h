/*
 * BtTabWidget.h is part of Brewtarget, and is Copyright the following
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

#ifndef BTTABWDIGET_H
#define BTTABWDIGET_H

class Recipe;
class Equipment;
class Style;
class Fermentable;
class Hop;
class Misc;
class Yeast;
#include <QTabWidget>

/*!
 * \class BtTabWdiget
 * \author Mik Firestone
 *
 * \brief To implement drag'n'drop, you need to subclass the dropEvent. I
 * think I need to implement this as a dragMoveEvent, the dragEnterEvent and
 * the dropEvent.
 *
 * Some implementation notes. The acceptMime string is used to determine which
 * kind of drop a given pane will accept. I don't want (for example) the
 * recipe pane accepting ingredient drops. This parameter is actually set
 * as a dynamic property on the UI object.
 *
 */
class BtTabWidget : public QTabWidget
{
   Q_OBJECT

public:
      BtTabWidget(QWidget* parent=0);

signals:
      void setRecipe(Recipe* rec);
      void setEquipment(Equipment* kit);
      void setStyle(Style* kit);
      void setFermentables(QList<Fermentable*>ferms);
      void setHops(QList<Hop*>hops);
      void setMiscs(QList<Misc*>miscs);
      void setYeasts(QList<Yeast*>yeasts);

protected:
      void dropEvent(QDropEvent *dpEvent);
      // void dragMoveEvent(QDragMoveEvent *dmEvent);
      virtual void dragEnterEvent(QDragEnterEvent *deEvent);

protected:
      QString acceptMime;

};

#endif /* BTTABWIDGET_H */
