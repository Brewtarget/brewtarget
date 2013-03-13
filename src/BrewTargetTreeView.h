/*
 * BrewTargetTreeView.h is part of Brewtarget and was written by Mik Firestone
 * (mikfire@gmail.com).  Copyright is granted to Philip G. Lee
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

#ifndef BREWTARGETTREEVIEW_H_
#define BREWTARGETTREEVIEW_H_

class BrewTargetTreeView;

#include <QTreeView>
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include "BrewTargetTreeItem.h"
#include "BtTreeFilterProxyModel.h"

// Forward declarations.
class BrewTargetTreeModel;
class Recipe;
class Equipment;
class Fermentable;
class Hop;
class Misc;
class Yeast;
class BrewNote;
class Style;

/*!
 * \class BrewTargetTreeItem
 * \author Mik Firestone
 *
 * \brief View class for BrewTargetTreeModel.
 */
class BrewTargetTreeView : public QTreeView
{
   Q_OBJECT
public:
   //! \brief The standard contructor
   BrewTargetTreeView(QWidget *parent = 0);
   //! \brief returns the model associated with this tree
   BrewTargetTreeModel* getModel();
   //! \brief returns the context menu associated with the \c selected item
   QMenu* getContextMenu(QModelIndex selected);

   //! \brief removes \c index item from the tree returns true if the remove works
   bool removeRow(const QModelIndex &index);
   //! \brief returns true if \c parent is the parent of \c child
   bool isParent(const QModelIndex& parent, const QModelIndex& child);
   //! \brief returns the parent of \c child
   QModelIndex getParent(const QModelIndex& child);

   //! \brief returns the first \c type element in the tree
   QModelIndex getFirst();

   //! \brief returns the recipe at \c index 
   Recipe* getRecipe(const QModelIndex &index) const;
   //! \brief finds the index of the \c recipe in the tree
   QModelIndex findRecipe(Recipe* rec);

   //! \brief returns the equipment at \c index 
   Equipment* getEquipment(const QModelIndex &index) const;
   //! \brief finds the index of the \c equipment in the tree
   QModelIndex findEquipment(Equipment* kit);

   //! \brief returns the fermentable at \c index 
   Fermentable* getFermentable(const QModelIndex &index) const;
   //! \brief finds the index of the \c fermentable in the tree
   QModelIndex findFermentable(Fermentable* ferm);

   //! \brief returns the hop at \c index 
   Hop* getHop(const QModelIndex &index) const;
   //! \brief finds the index of the \c hop in the tree
   QModelIndex findHop(Hop* hop);

   //! \brief returns the misc at \c index 
   Misc* getMisc(const QModelIndex &index) const;
   //! \brief finds the index of the \c misc in the tree
   QModelIndex findMisc(Misc* misc);

   //! \brief returns the yeast at \c index 
   Yeast* getYeast(const QModelIndex &index) const;
   //! \brief finds the index of the \c yeast in the tree
   QModelIndex findYeast(Yeast* yeast);

   //! \brief returns the yeast at \c index 
   Style* getStyle(const QModelIndex &index) const;
   //! \brief finds the index of the \c yeast in the tree
   QModelIndex findStyle(Style* style);

   //! \brief returns the brewnote at \c index 
   BrewNote* getBrewNote(const QModelIndex &index) const;
   //! \brief finds the index of the \c brewnote in the tree
   QModelIndex findBrewNote( BrewNote* bNote);

   //! \brief gets the type of the item at \c index. 
   int getType(const QModelIndex &index);

   //! returns true if a recipe and an ingredient (hop, equipment, etc.) are selected at the same time
   bool multiSelected();

   // Another try at drag and drop
   //! \brief starts a drag and drop event
   void mousePressEvent(QMouseEvent *event);
   //! \brief distinguishes between a move event and a double click
   void mouseMoveEvent(QMouseEvent *event);
   //! \brief recognizes a double click event
   void mouseDoubleClickEvent(QMouseEvent *event);

   //! \brief catches a key stroke in a tree
   void keyPressEvent(QKeyEvent* event);

   //! \brief creates a context menu based on the type of tree
   void setupContextMenu(QWidget* top, QWidget* editor, QMenu* sMenu,int type = BrewTargetTreeItem::RECIPE);

   // Friend classes. For the most part, the children don't do much beyond
   // contructors and context menus. So far :/
   friend class RecipeTreeView;
   friend class EquipmentTreeView;
   friend class FermentableTreeView;
   friend class HopTreeView;
   friend class MiscTreeView;
   friend class YeastTreeView;
   friend class StyleTreeView;

private:
   BrewTargetTreeModel* model;
   BtTreeFilterProxyModel* filter;
   QMenu* contextMenu, *subMenu;
   QPoint dragStart;

   bool doubleClick;

   QMimeData *mimeData(QModelIndexList indexes);
};

//!
// \class RecipeTreeView 
// \brief subclasses BrewTargetTreeView to only show recipes.
class RecipeTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   RecipeTreeView(QWidget *parent = 0);

};

//! 
// \class EquipmentTreeView 
// \brief subclasses BrewTargetTreeView to only show equipment.
class EquipmentTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   EquipmentTreeView(QWidget *parent = 0);
};

//!
// \class FermentableTreeView 
// \brief subclasses BrewTargetTreeView to only show fermentables.
class FermentableTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   FermentableTreeView(QWidget *parent = 0);

};

//!
// \class HopTreeView 
// \brief subclasses BrewTargetTreeView to only show hops.
class HopTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   HopTreeView(QWidget *parent = 0);

};

//!
// \class MiscTreeView 
// \brief subclasses BrewTargetTreeView to only show miscs.
class MiscTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   MiscTreeView(QWidget *parent = 0);
};

//!
// \class YeastTreeView 
// \brief subclasses BrewTargetTreeView to only show yeasts.
class YeastTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   YeastTreeView(QWidget *parent = 0);

};

//!
// \class StyleTreeView 
// \brief subclasses BrewTargetTreeView to only show styles.
class StyleTreeView : public BrewTargetTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   StyleTreeView(QWidget *parent = 0);

};

#endif /* BREWTARGETTREEVIEW_H_ */
