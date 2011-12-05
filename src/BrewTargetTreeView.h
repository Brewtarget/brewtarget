/*
 * BrewTargetTreeView.h is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#ifndef BREWTARGETTREEVIEW_H_
#define BREWTARGETTREEVIEW_H_

class BrewTargetTreeView;

#include <QTreeView>
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include "BrewTargetTreeItem.h"

// Forward declarations.
class BrewTargetTreeModel;
class Recipe;
class Equipment;
class Fermentable;
class Hop;
class Misc;
class Yeast;
class BrewNote;

class BrewTargetTreeView : public QTreeView
{
   Q_OBJECT
public:
   BrewTargetTreeView(QWidget *parent = 0);
   virtual ~BrewTargetTreeView();
   BrewTargetTreeModel* getModel();
   QMenu* getContextMenu(QModelIndex selected);

   bool removeRow(const QModelIndex &index);
   bool isParent(const QModelIndex& parent, const QModelIndex& child);
   QModelIndex getParent(const QModelIndex& child);

   QModelIndex getFirst(int type = BrewTargetTreeItem::NUMTYPES);

   // Ugh
   Recipe* getRecipe(const QModelIndex &index) const;
   QModelIndex findRecipe(Recipe* rec);

   Equipment* getEquipment(const QModelIndex &index) const;
   QModelIndex findEquipment(Equipment* kit);

   Fermentable* getFermentable(const QModelIndex &index) const;
   QModelIndex findFermentable(Fermentable* ferm);

   Hop* getHop(const QModelIndex &index) const;
   QModelIndex findHop(Hop* hop);

   Misc* getMisc(const QModelIndex &index) const;
   QModelIndex findMisc(Misc* misc);

   Yeast* getYeast(const QModelIndex &index) const;
   QModelIndex findYeast(Yeast* yeast);

   BrewNote* getBrewNote(const QModelIndex &index) const;
   QModelIndex findBrewNote( BrewNote* bNote);

   int getType(const QModelIndex &index);
   bool multiSelected();

   // Another try at drag and drop
   void mousePressEvent(QMouseEvent *event);
   void mouseMoveEvent(QMouseEvent *event);
   void mouseDoubleClickEvent(QMouseEvent *event);

   // The abstraction kind of hurts
   void setupContextMenu(QWidget* top, QWidget* editor, QMenu* sMenu,int type = BrewTargetTreeItem::RECIPE);

   // Friend classes. For the most part, the children don't do much beyond
   // contructors and context menus. So far :/
   friend class RecipeTreeView;
   friend class EquipmentTreeView;
   friend class FermentableTreeView;
   friend class HopTreeView;
   friend class MiscTreeView;
   friend class YeastTreeView;

private:
   BrewTargetTreeModel* model;
   QMenu* contextMenu, *subMenu;
   QPoint dragStart;

   bool doubleClick;

   QMimeData *mimeData(QModelIndexList indexes);
};

// RecipeTreeView subclasses BrewTargetTreeView to only show recipes.
class RecipeTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   RecipeTreeView(QWidget *parent = 0);
   virtual ~RecipeTreeView();

};

// EquipmentTreeView only shows equipment. I think you can see where this is headed?
class EquipmentTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   EquipmentTreeView(QWidget *parent = 0);
   virtual ~EquipmentTreeView();
};

class FermentableTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   FermentableTreeView(QWidget *parent = 0);
   virtual ~FermentableTreeView();
};

class HopTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   HopTreeView(QWidget *parent = 0);
   virtual ~HopTreeView();
};

class MiscTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   MiscTreeView(QWidget *parent = 0);
   virtual ~MiscTreeView();
};

class YeastTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   YeastTreeView(QWidget *parent = 0);
   virtual ~YeastTreeView();
};

#endif /* BREWTARGETTREEVIEW_H_ */
