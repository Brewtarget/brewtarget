/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeView.h is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#ifndef TREES_TREEVIEW_H
#define TREES_TREEVIEW_H
#pragma once

#include <QTreeView>
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>

#include "trees/TreeNode.h"
#include "trees/TreeFilterProxyModel.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

// Forward declarations.
class TreeModel;
class Recipe;
class Equipment;
class Fermentable;
class Hop;
class Mash;
class Misc;
class Yeast;
class BrewNote;
class Style;
class Water;

/*!
 * \class TreeView
 *
 * \brief View class for \c TreeModel.
 */
class TreeView : public QTreeView {
   Q_OBJECT
public:
   //! \brief The standard constructor
   TreeView(QWidget * parent = nullptr, TreeModel::TypeMasks mask = TreeModel::TypeMask::Recipe);
   //! \brief returns the model associated with this tree
   TreeModel * model();
   //! \b returns the filter associated with this model
   TreeFilterProxyModel * filter();

   //! \brief returns the context menu associated with the \c selected item
   QMenu * contextMenu(QModelIndex selected);

   //! \brief removes \c index item from the tree returns true if the remove works
   bool removeRow(const QModelIndex & index);
   //! \brief returns true if \c parent is the parent of \c child
   bool isParent(const QModelIndex & parent, const QModelIndex & child);

   //! \brief returns the parent of \c child
   QModelIndex parent(const QModelIndex & child);
   //! \brief returns the first \c type element in the tree
   QModelIndex first();

   QModelIndex findElement(NamedEntity * thing);

   /**
    * \brief returns the item at \c index
    *        Valid for \c Recipe, \c Equipment, \c Fermentable, \c Hop, \c Misc, \c Yeast, \c Style, \c BrewNote,
    *        \c Water, \c Folder.
    */
   template<class T>
   T * getItem(QModelIndex const & index) const;

   //! \brief finds the index of the \c folder in the tree,but does not create
   QModelIndex findFolder(Folder * folder);
   //! \brief adds a folder to the tree
   void addFolder(QString folder);
   //! \brief renames a folder and all of its subitems
   void renameFolder(Folder * victim, QString newName);
   QString folderName(QModelIndex starter);

   //! \brief gets the type of the item at \c index.
   std::optional<TreeNode::Type> type(QModelIndex const & index) const;

   //! \brief returns true if the recipe at ndx is showing its ancestors
   bool ancestorsAreShowing(QModelIndex ndx);
   //! \brief enables or disables the delete action when a recipe is unlocked/locked
   void enableDelete(bool enable);
   //! \brief enables or disables showing ancestors
   void enableShowAncestor(bool enable);
   //! \brief enables or disables hiding ancestors
   void enableHideAncestor(bool enable);
   //! \brief make a recipe its own ancestor
   void enableOrphan(bool enable);
   //! \brief do we breed, or not
   void enableSpawn(bool enable);

   //! \brief returns true if a recipe and an ingredient (hop, equipment, etc.) are selected at the same time
   bool multiSelected();

   // Another try at drag and drop
   //! \brief starts a drag and drop event
   void mousePressEvent(QMouseEvent * event);
   //! \brief distinguishes between a move event and a double click
   void mouseMoveEvent(QMouseEvent * event);
   //! \brief recognizes a double click event
   void mouseDoubleClickEvent(QMouseEvent * event);

   //! \brief catches a key stroke in a tree
   void keyPressEvent(QKeyEvent * event);

   //! \brief creates a context menu based on the type of tree
   void setupContextMenu(QWidget * top, QWidget * editor);

   //! \brief sets a new filter
   void setFilter(TreeFilterProxyModel * newFilter);
   //! \brief gets the current filter
   TreeFilterProxyModel * filter() const;

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
   void newNamedEntity();
   void versionedRecipe(Recipe * descendant);

private slots:
   void expandFolder(TreeModel::TypeMasks kindaThing, QModelIndex fIdx);

   void showAncestors();
   void hideAncestors();
   void revertRecipeToPreviousVersion();
   void orphanRecipe();
   void spawnRecipe();

signals:
   void recipeSpawn(Recipe * descendant);

private:
   TreeModel * m_model;
   TreeFilterProxyModel * m_filter;
   TreeModel::TypeMasks m_type;
   QMenu * m_contextMenu,
         * subMenu,
         * m_versionMenu,
         * m_exportMenu;
   QAction * m_deleteAction,
           * m_showAncestorAction,
           * m_hideAncestorAction,
           * m_orphanAction,
           * m_spawnAction,
           * m_copyAction,
           * m_brewItAction;
   QPoint dragStart;
   QWidget * m_editor;

   bool doubleClick;

   int verifyDelete(int confirmDelete, QString tag, QString name);
   QString verifyCopy(QString tag, QString name, bool * abort);
   QMimeData * mimeData(QModelIndexList indexes);
};
//======================================================================================================================
template<class Derived> class TreeViewPhantom;
template<class Derived, class NE>
class TreeViewBase : public CuriouslyRecurringTemplateBase<TreeViewPhantom, Derived> {
public:
   TreeViewBase() {
      return;
   }
   ~TreeViewBase() = default;

   void doConnections() requires std::same_as<Recipe, Derived> {
      this->derived().connect(this->derived().m_model, &TreeModel::recipeSpawn, &this->derived(), &TreeView::versionedRecipe);
      return;
   }

   void doConnections() requires (!std::same_as<Recipe, Derived>) {
      return;
   }


};
//======================================================================================================================
#define TREE_VIEW_COMMON_DECL(NeName) \
class NeName##TreeView : public TreeView,                                            \
                         public TreeViewBase<NeName##TreeView, NeName> {             \
   Q_OBJECT                                                                          \
                                                                                     \
   /* This allows TreeViewBase to call protected and private members of Derived */   \
   friend class TreeViewBase<NeName##TreeView, NeName>;                              \
                                                                                     \
   public:                                                                           \
      /* Constructs the tree view, sets up the filter proxy and sets a */            \
      /* few options on the tree that can only be set after the model  */            \
      NeName##TreeView(QWidget * parent = nullptr);                                  \
      virtual ~NeName##TreeView();                                                   \
};                                                                                   \

TREE_VIEW_COMMON_DECL(Recipe)
TREE_VIEW_COMMON_DECL(Equipment)
TREE_VIEW_COMMON_DECL(Fermentable)
TREE_VIEW_COMMON_DECL(Hop)
TREE_VIEW_COMMON_DECL(Misc)
TREE_VIEW_COMMON_DECL(Yeast)
TREE_VIEW_COMMON_DECL(Style)
TREE_VIEW_COMMON_DECL(Water)

#define TREE_VIEW_COMMON_CODE(NeName) \
   NeName##TreeView::NeName##TreeView(QWidget * parent) : \
      TreeView(parent, TreeModel::TypeMask::NeName),      \
      TreeViewBase<NeName##TreeView, NeName>() {          \
      this->doConnections();                              \
      return;                                             \
   }                                                      \
                                                          \
   NeName##TreeView::~NeName##TreeView() = default;       \


#endif
