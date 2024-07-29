/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeModel.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
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
#ifndef TREES_TREEMODEL_H
#define TREES_TREEMODEL_H
#pragma once

#include <memory>
#include <optional>

#include <QAbstractItemModel>
#include <QFlags> // For Q_DECLARE_FLAGS
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QObject>
//#include <QSqlRelationalTableModel>
#include <QVariant>

#include "trees/TreeNode.h"

// Forward declarations
class BrewNote;
class Folder;
class BtStringConst;
class TreeView;
class Equipment;
class Fermentable;
class Hop;
class Misc;
class NamedEntity;
class Recipe;
class Style;
class Water;
class Yeast;

/*!
 * \class TreeModel
 *
 * \brief Model for a tree of Recipes, Equipments, Fermentables, Hops, Miscs and Yeasts
 *
 * Provides the necessary model so we can build the trees. It extends the
 * QAbstractItemModel, so it has to implement some of the virtual methods
 * required.
 */
class TreeModel : public QAbstractItemModel {
   Q_OBJECT

public:
   //! \brief Describes what items this tree will show.
   enum class TypeMask {
      Recipe      = (1 << 0),
      Equipment   = (1 << 1),
      Fermentable = (1 << 2),
      Hop         = (1 << 3),
      Misc        = (1 << 4),
      Yeast       = (1 << 5),
      BrewNote    = (1 << 6),
      Style       = (1 << 7),
      Folder      = (1 << 8),
      Water       = (1 << 9),
   };
   Q_DECLARE_FLAGS(TypeMasks, TypeMask)

   TreeModel(TreeView * parent = nullptr, TypeMasks type = TypeMask::Recipe);
   virtual ~TreeModel();

   //! \brief Reimplemented from QAbstractItemModel
   virtual QVariant data(const QModelIndex & index, int role) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual Qt::ItemFlags flags(const QModelIndex & index) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual int columnCount(const QModelIndex & index = QModelIndex()) const;

   //! \brief Reimplemented from QAbstractItemModel
   virtual QModelIndex index(int row, int col, const QModelIndex & parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractItemModel
   virtual QModelIndex parent(const QModelIndex & index) const;

   //! \brief Reimplemented from QAbstractItemModel
   bool insertRow(int row,
                  QModelIndex const & parent = QModelIndex(),
                  QObject * victim = nullptr,
                  std::optional<TreeNode::Type> victimType = std::nullopt);
   //! \brief Reimplemented from QAbstractItemModel
   virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

   //! \brief Get the upper-left index for the tree
   QModelIndex first();
   //! \brief returns the TreeNode at \c index
   TreeNode * item(const QModelIndex & index) const;

   /**
    * \brief Test type at \c index.
    *        Valid for \c Recipe, \c Equipment, \c Fermentable, \c Hop, \c Misc, \c Yeast, \c Style, \c BrewNote,
    *        \c Water, \c Folder.
    */
   template<class T>
   bool itemIs(QModelIndex const & index) const;

   //! \brief Gets the type of item at \c index
   std::optional<TreeNode::Type> type(const QModelIndex & index) const;

   //! \brief Return the type mask for this tree. \sa TreeModel::TypeMasks
   int mask();

   // I'm trying to shove some complexity down a few layers.
   // \!brief returns the name of whatever is at idx
   QString name(const QModelIndex & idx);
   //! \brief delete things from the tree/db
   void deleteSelected(QModelIndexList victims);

   void copySelected(QList< QPair<QModelIndex, QString>> toBeCopied);

   /**
    * \brief Get T at \c index.
    *        Valid for \c Recipe, \c Equipment, \c Fermentable, \c Hop, \c Misc, \c Yeast, \c Style, \c BrewNote,
    *        \c Water, \c Folder.
    */
   template<class T>
   T * getItem(QModelIndex const & index) const;

   //! \brief Get NamedEntity at \c index.
   NamedEntity * thing(const QModelIndex & index) const;

   //! \brief one find method to find them all, and in darkness bind them
   QModelIndex findElement(NamedEntity * thing, TreeNode * parent = nullptr);

   //! \brief Get index of \c Folder
   QModelIndex findFolder(QString folder, TreeNode * parent = nullptr, bool create = false);
   //! \brief a new folder .
   bool addFolder(QString name);
   //! \brief renames a folder
   bool renameFolder(Folder * victim, QString name);
   //! \brief removes a folder. This could get weird if you don't remove
   //! everything from it first. This is *intended* to be called from
   //! deleteSelected().
   bool removeFolder(QModelIndex ndx);

   QModelIndexList allChildren(QModelIndex parent);
   // !\brief accept a drop action.
   bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
   // !\brief what our supported drop actions are. Don't know if I need the drag option or not?
   Qt::DropActions supportedDropActions() const;
   QStringList mimeTypes() const;

   //! \b show the versions of the recipe at index
   void showAncestors(QModelIndex ndx);
   //! \b show the child of the recipe at index
   bool showChild(QModelIndex ndx) const;
   //! \b hide the ancestors
   void hideAncestors(QModelIndex ndx);
   void revertRecipeToPreviousVersion(QModelIndex ndx);
   //! \b orphan a recipe
   void orphanRecipe(QModelIndex ndx);
   //! \b spawns a recipe
   void spawnRecipe(QModelIndex ndx);

public slots:
   void versionedRecipe(Recipe * ancestor, Recipe * descendant);
   void catchAncestors(bool showem);

private slots:
   //! \brief slot to catch a changed folder signal. Folders are odd, because they
   // can hold .. anything, including other folders. So I need the most generic
   // pointer I can get. I hope this works.
   void folderChanged(QString name);

   //! \brief This is as best as I can see to do it. Qt signaling mechanism is
   //   doing, as I recall, string compares on the signatures. Sigh.
   void elementAddedRecipe     (int victimId);
   void elementAddedEquipment  (int victimId);
   void elementAddedFermentable(int victimId);
   void elementAddedHop        (int victimId);
   void elementAddedMisc       (int victimId);
   void elementAddedStyle      (int victimId);
   void elementAddedYeast      (int victimId);
   void elementAddedBrewNote   (int victimId);
   void elementAddedWater      (int victimId);

   void elementChanged();

   void elementRemovedRecipe     (int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedEquipment  (int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedFermentable(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedHop        (int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedMisc       (int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedStyle      (int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedYeast      (int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedBrewNote   (int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedWater      (int victimId, std::shared_ptr<QObject> victim);

   void recipePropertyChanged(int recipeId, BtStringConst const & propertyName);

signals:
   void expandFolder(TreeModel::TypeMasks kindofThing, QModelIndex fIdx);
   void recipeSpawn(Recipe * descendant);

private:
   //! \brief Loads the tree.
   void loadTreeModel();

   //! \brief add and remove an element from the, respectively. All of the
   //slots actually call these two methods
   void elementAdded(NamedEntity * victim);
   void elementRemoved(NamedEntity * victim);

   //! \brief connects the changedName() signal and changedFolder() signals to
   //! the proper methods for most things, and the same for changedBrewDate
   //! and brewNotes
   void observeElement(NamedEntity *);

   void folderChanged(NamedEntity * test);

   //! \brief get a tooltip
   QVariant toolTipData(const QModelIndex & index) const;

   //! \brief Returns the list of things in a tree (e.g., recipes) as a list
   //! of NamedEntitys. It's a convenience method to make loadTree()
   //! cleaner
   QList<NamedEntity *> elements();
   //! \brief creates a folder tree. It's mostly a helper function.
   QModelIndex createFolderTree(QStringList dirs, TreeNode * parent, QString pPath);

   //! \brief convenience function to add brewnotes to a recipe as a subtree
   void addBrewNoteSubTree(Recipe * rec, int i, TreeNode * parent, bool recurse = true);
   //! \b flip the switch to show descendants
   void setShowChild(QModelIndex child, bool val);
   void addAncestoralTree(Recipe * rec, int i, TreeNode * parent);

   TreeNode * rootItem;
   TreeView * parentTree;
   TypeMasks m_treeMask;
   TreeNode::Type nodeType;
   int m_maxColumns;
   QString m_mimeType;

};

// This is a bit ugly, but will ultimately be refactored away, once we stop having to decide at runtime whether
// something can have a folder.
namespace FolderUtils {
   std::optional<QString> getFolder(NamedEntity const * ne);
   void setFolder(NamedEntity * ne, QString const & val);
}

#endif
