/*
 * BtTreeModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef BTTREEMODEL_H
#define BTTREEMODEL_H
#pragma once

#include <memory>
#include <optional>

#include <QAbstractItemModel>
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QObject>
#include <QSqlRelationalTableModel>
#include <QVariant>

#include "BtTreeItem.h"

// Forward declarations
class BrewNote;
class BtFolder;
class BtStringConst;
class BtTreeView;
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
 * \class BtTreeModel
 *
 * \brief Model for a tree of Recipes, Equipments, Fermentables, Hops, Miscs and Yeasts
 *
 * Provides the necessary model so we can build the trees. It extends the
 * QAbstractItemModel, so it has to implement some of the virtual methods
 * required.
 */
class BtTreeModel : public QAbstractItemModel {
   Q_OBJECT

public:
   //! \brief Describes what items this tree will show.
   enum TypeMasks {
      //! Show recipes
      RECIPEMASK        = 1,
      //! Show equipments
      EQUIPMASK         = 2,
      //! Show fermentables
      FERMENTMASK       = 4,
      //! Show hops
      HOPMASK           = 8,
      //! Show miscs
      MISCMASK          = 16,
      //! Show yeasts
      YEASTMASK         = 32,
      //! Show brewnotes
      BREWNOTEMASK      = 64,
      //! Show styles
      STYLEMASK         = 128,
      //! folders. This may actually have worked better than expected.
      FOLDERMASK        = 256,
      //! waters.
      WATERMASK         = 512,
   };

   BtTreeModel(BtTreeView * parent = nullptr, TypeMasks type = RECIPEMASK);
   virtual ~BtTreeModel();

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
                  std::optional<BtTreeItem::Type> victimType = std::nullopt);
   //! \brief Reimplemented from QAbstractItemModel
   virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

   //! \brief Get the upper-left index for the tree
   QModelIndex first();
   //! \brief returns the BtTreeItem at \c index
   BtTreeItem * item(const QModelIndex & index) const;

   /**
    * \brief Test type at \c index.
    *        Valid for \c Recipe, \c Equipment, \c Fermentable, \c Hop, \c Misc, \c Yeast, \c Style, \c BrewNote,
    *        \c Water, \c BtFolder.
    */
   template<class T>
   bool itemIs(QModelIndex const & index) const;

   //! \brief Gets the type of item at \c index
   std::optional<BtTreeItem::Type> type(const QModelIndex & index) const;

   //! \brief Return the type mask for this tree. \sa BtTreeModel::TypeMasks
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
    *        \c Water, \c BtFolder.
    */
   template<class T>
   T * getItem(QModelIndex const & index) const;

   //! \brief Get NamedEntity at \c index.
   NamedEntity * thing(const QModelIndex & index) const;

   //! \brief one find method to find them all, and in darkness bind them
   QModelIndex findElement(NamedEntity * thing, BtTreeItem * parent = nullptr);

   //! \brief Get index of \c Folder
   QModelIndex findFolder(QString folder, BtTreeItem * parent = nullptr, bool create = false);
   //! \brief a new folder .
   bool addFolder(QString name);
   //! \brief renames a folder
   bool renameFolder(BtFolder * victim, QString name);
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
   void elementAddedRecipe(int victimId);
   void elementAddedEquipment(int victimId);
   void elementAddedFermentable(int victimId);
   void elementAddedHop(int victimId);
   void elementAddedMisc(int victimId);
   void elementAddedStyle(int victimId);
   void elementAddedYeast(int victimId);
   void elementAddedBrewNote(int victimId);
   void elementAddedWater(int victimId);

   void elementChanged();

   void elementRemovedRecipe(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedEquipment(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedFermentable(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedHop(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedMisc(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedStyle(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedYeast(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedBrewNote(int victimId, std::shared_ptr<QObject> victim);
   void elementRemovedWater(int victimId, std::shared_ptr<QObject> victim);

   void recipePropertyChanged(int recipeId, BtStringConst const & propertyName);

signals:
   void expandFolder(BtTreeModel::TypeMasks kindofThing, QModelIndex fIdx);
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
   //! \brief returns the \c section header from a recipe
   QVariant recipeHeader(int section) const;
   //! \brief returns the \c section header from an equipment
   QVariant equipmentHeader(int section) const;
   //! \brief returns the \c section header from a fermentable
   QVariant fermentableHeader(int section) const;
   //! \brief returns the \c section header from a hop
   QVariant hopHeader(int section) const;
   //! \brief returns the \c section header from a misc
   QVariant miscHeader(int section) const;
   //! \brief returns the \c section header from a yeast
   QVariant yeastHeader(int section) const;
   //! \brief returns the \c section header from a style
   QVariant styleHeader(int section) const;
   //! \brief returns the \c section header for a folder.
   QVariant folderHeader(int section) const;
   //! \brief returns the \c section header for a folder.
   QVariant waterHeader(int section) const;

   //! \brief get a tooltip
   QVariant toolTipData(const QModelIndex & index) const;

   //! \brief Returns the list of things in a tree (e.g., recipes) as a list
   //! of NamedEntitys. It's a convenience method to make loadTree()
   //! cleaner
   QList<NamedEntity *> elements();
   //! \brief creates a folder tree. It's mostly a helper function.
   QModelIndex createFolderTree(QStringList dirs, BtTreeItem * parent, QString pPath);

   //! \brief convenience function to add brewnotes to a recipe as a subtree
   void addBrewNoteSubTree(Recipe * rec, int i, BtTreeItem * parent, bool recurse = true);
   //! \b flip the switch to show descendants
   void setShowChild(QModelIndex child, bool val);
   void addAncestoralTree(Recipe * rec, int i, BtTreeItem * parent);

   BtTreeItem * rootItem;
   BtTreeView * parentTree;
   TypeMasks treeMask;
   BtTreeItem::Type itemType;
   int m_maxColumns;
   QString _mimeType;

};

#endif
