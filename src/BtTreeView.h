/*
 * BtTreeView.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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

#ifndef BTTREEVIEW_H_
#define BTTREEVIEW_H_

class BtTreeView;

#include <QTreeView>
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include "BtTreeItem.h"
#include "BtTreeFilterProxyModel.h"

// Forward declarations.
class BtTreeModel;
class Recipe;
class Equipment;
class Fermentable;
class Hop;
class Misc;
class Yeast;
class BrewNote;
class Style;
class Water;

/*!
 * \class BtTreeItem
 * \author Mik Firestone
 *
 * \brief View class for BtTreeModel.
 */
class BtTreeView : public QTreeView
{
   Q_OBJECT
public:
   //! \brief The standard contructor
   BtTreeView(QWidget *parent = nullptr, BtTreeModel::TypeMasks mask = BtTreeModel::RECIPEMASK);
   //! \brief returns the model associated with this tree
   BtTreeModel* model();
   //! \brief returns the context menu associated with the \c selected item
   QMenu* contextMenu(QModelIndex selected);

   //! \brief removes \c index item from the tree returns true if the remove works
   bool removeRow(const QModelIndex &index);
   //! \brief returns true if \c parent is the parent of \c child
   bool isParent(const QModelIndex& parent, const QModelIndex& child);

   //! \brief returns the parent of \c child
   QModelIndex parent(const QModelIndex& child);
   //! \brief returns the first \c type element in the tree
   QModelIndex first();

   QModelIndex findElement(Ingredient* thing);

   //! \brief returns the recipe at \c index
   Recipe* recipe(const QModelIndex &index) const;
   //! \brief returns the equipment at \c index
   Equipment* equipment(const QModelIndex &index) const;
   //! \brief returns the fermentable at \c index
   Fermentable* fermentable(const QModelIndex &index) const;
   //! \brief returns the hop at \c index
   Hop* hop(const QModelIndex &index) const;
   //! \brief returns the misc at \c index
   Misc* misc(const QModelIndex &index) const;
   //! \brief returns the yeast at \c index
   Yeast* yeast(const QModelIndex &index) const;
   //! \brief returns the yeast at \c index
   Style* style(const QModelIndex &index) const;
   //! \brief returns the brewnote at \c index
   BrewNote* brewNote(const QModelIndex &index) const;
   //! \brief returns the water at \c index
   Water* water(const QModelIndex &index) const;

   //! \brief returns the folder at \c index
   BtFolder* folder(const QModelIndex &index) const;
   //! \brief finds the index of the \c folder in the tree,but does not create
   QModelIndex findFolder( BtFolder* folder);
   //! \brief adds a folder to the tree
   void addFolder( QString folder);
   //! \brief renames a folder and all of its subitems
   void renameFolder(BtFolder* victim, QString newName);
   QString folderName(QModelIndex starter);

   //! \brief gets the type of the item at \c index.
   int type(const QModelIndex &index);

   //! \brief returns true if a recipe and an ingredient (hop, equipment, etc.) are selected at the same time
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
   void setupContextMenu(QWidget* top, QWidget* editor );

   //! \brief sets a new filter
   void setFilter(BtTreeFilterProxyModel* newFilter);
   //! \brief gets the current filter
   BtTreeFilterProxyModel* filter() const;

   void deleteSelected(QModelIndexList selected);
   void copySelected(QModelIndexList selected);
   // Friend classes. For the most part, the children don't do much beyond
   // contructors and context menus. So far :/
   friend class RecipeTreeView;
   friend class EquipmentTreeView;
   friend class FermentableTreeView;
   friend class HopTreeView;
   friend class MiscTreeView;
   friend class YeastTreeView;
   friend class StyleTreeView;
   friend class WaterTreeView;

public slots:
   void newIngredient();

private slots:
   void expandFolder(BtTreeModel::TypeMasks kindaThing, QModelIndex fIdx);

private:
   BtTreeModel* _model;
   BtTreeFilterProxyModel* _filter;
   BtTreeModel::TypeMasks _type;
   QMenu* _contextMenu, *subMenu;
   QPoint dragStart;
   QWidget* _editor;

   bool doubleClick;

   int verifyDelete(int confirmDelete, QString tag, QString name);
   QString verifyCopy(QString tag, QString name, bool *abort);
   QMimeData *mimeData(QModelIndexList indexes);
};

//!
// \class RecipeTreeView
// \brief subclasses BtTreeView to only show recipes.
class RecipeTreeView : public BtTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   RecipeTreeView(QWidget *parent = nullptr);

};

//!
// \class EquipmentTreeView
// \brief subclasses BtTreeView to only show equipment.
class EquipmentTreeView : public BtTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   EquipmentTreeView(QWidget *parent = nullptr);
};

//!
// \class FermentableTreeView
// \brief subclasses BtTreeView to only show fermentables.
class FermentableTreeView : public BtTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   FermentableTreeView(QWidget *parent = nullptr);

};

//!
// \class HopTreeView
// \brief subclasses BtTreeView to only show hops.
class HopTreeView : public BtTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   HopTreeView(QWidget *parent = nullptr);

};

//!
// \class MiscTreeView
// \brief subclasses BtTreeView to only show miscs.
class MiscTreeView : public BtTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   MiscTreeView(QWidget *parent = nullptr);
};

//!
// \class YeastTreeView
// \brief subclasses BtTreeView to only show yeasts.
class YeastTreeView : public BtTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   YeastTreeView(QWidget *parent = nullptr);

};

//!
// \class StyleTreeView
// \brief subclasses BtTreeView to only show styles.
class StyleTreeView : public BtTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   StyleTreeView(QWidget *parent = nullptr);

};

//!
// \class WaterTreeView
// \brief subclasses BtTreeView to only show waters.
class WaterTreeView : public BtTreeView
{
   Q_OBJECT
public:
   //! \brief Constructs the tree view, sets up the filter proxy and sets a
   // few options on the tree that can only be set after the model
   WaterTreeView(QWidget *parent = nullptr);

};
#endif /* BREWTARGETTREEVIEW_H_ */
