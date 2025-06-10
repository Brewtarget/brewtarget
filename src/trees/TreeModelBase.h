/*======================================================================================================================
 * trees/TreeModelBase.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Mattias Måhl <mattias@kejsarsten.com>
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
 =====================================================================================================================*/
#ifndef TREES_TREEMODELBASE_H
#define TREES_TREEMODELBASE_H
#pragma once

#include <memory>
#include <optional>
#include <type_traits>
#include <variant>
#include <utility>

#include <QDebug>
#include <QMimeData>
#include <QModelIndex>
#include <QQueue>
#include <QString>
#include <QStringBuilder> // Needed for efficient QString concatenation operator (%)
#include <QVariant>

#include "database/ObjectStoreWrapper.h"
#include "trees/TreeNode.h"
#include "trees/TreeModel.h"
#include "trees/TreeModelChangeGuard.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeTraits.h"

namespace {
   // This is used as a parameter to findFolder to tell it what to do if it doesn't find the requested folder
   enum class IfNotFound {
      Create,
      ReturnInvalid
   };
}

template <typename T>
concept HasNodeClassifier = requires {
   { T::NodeClassifier } -> std::convertible_to<TreeNodeClassifier>;
};

/**
 * \brief CRTP base for \c TreeModel subclasses.  See comment on \c TreeModel class for more info.
 *
 * \param Derived - The derived class
 * \param NE  - The primary \c NamedEntity subclass (besides \c Folder) shown in this tree (eg \c Recipe for
 *              \c RecipeTreeModel)
 * \param SNE - The optional secondary \c NamedEntity subclass shown in this tree (eg \c BrewNote for
 *              \c RecipeTreeModel, or \c MashStep for \c MashTreeModel).  This class must have:
 *                 • an \c owner() member function that does the obvious thing (eg \c BrewNote::owner() returns a
 *                   \c Recipe; \c MashStep::owner returns a \c Mash);
 *                 • a static \c ownedBy() member function that returns all the \c BrewNote objects owned by a given
 *                   \c Recipe or all the \c MashStep objects owned by a given \c Mash, etc.
 */
template<class Derived> class TreeModelPhantom;
template<class Derived, class NE, typename SNE = void>
class TreeModelBase : public CuriouslyRecurringTemplateBase<TreeModelPhantom, Derived> {
   friend Derived;
private:
   /**
    * \brief Derived classes should also call \c TreeModelBase::connectSignalsAndSlots from their own constructor (see
    *        TREE_MODEL_COMMON_CODE).
    */
   TreeModelBase() :
   m_rootNode{std::make_unique<TreeFolderNode<NE>>(this->derived())} {
      return;
   }

public:
   ~TreeModelBase() = default;

   /**
    * \brief Returns the class name of \c NE
    *
    *        Note that we deliberately return QString rather than char const * here, because we want callers to be able
    *        to do an == comparison (eg `if (model->treeForClassName() == Recipe::staticMetaObject.className())`) rather
    *        than have to manually call strcmp.
    *
    *        TBD: We could do something clever with a templated function instead of comparing class names...
    */
   QString doTreeForClassName() const {
      return NE::staticMetaObject.className();
   }

   /**
    * \brief Returns the localised name of the main thing stored in the tree
    */
   QString doTreeForLocalisedClassName() const {
      return NE::localisedName();
   }

   std::optional<TreeNodeClassifier> classifier(QModelIndex const & index) const {
      return index.isValid() ? std::optional<TreeNodeClassifier>{this->doTreeNode(index)->classifier()} : std::nullopt;
   }

   /**
    * \brief Return the \c TreeNode for a given index
    */
   TreeNode * doTreeNode(QModelIndex const & index) const {
      if (index.isValid()) {
         TreeNode * item = static_cast<TreeNode *>(index.internalPointer());
         if (item) {
            return item;
         }
      }

      return this->m_rootNode.get();
   }

   //! \brief Get the upper-left index for the tree
   QModelIndex doFirst() const {
      // get the first item in the list, which is the place holder
      if (this->m_rootNode->childCount() > 0) {
         return this->derived().createIndex(0, 0, this->m_rootNode.get());
      }

      return QModelIndex();
   }

   QVariant doData(QModelIndex const & index, int const role) const {
      TreeNode * treeNode = this->doTreeNode(index);
      if (treeNode) {
         return treeNode->data(index.column(), role);
      }

      return QVariant();
   }

   QVariant doHeaderData(int const section, Qt::Orientation const orientation, int const role) const {
      if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
         return QVariant();
      }

      return TreeItemNode<NE>::header(section);
   }

   int doRowCount(QModelIndex const & parent) const {
      if (!parent.isValid()) {
         return this->m_rootNode ? this->m_rootNode->childCount() : 0;
      }

      return this->doTreeNode(parent)->childCount();
   }

   int doColumnCount([[maybe_unused]] QModelIndex const & parent) const {
      return TreeItemNode<NE>::NumberOfColumns;
   }

   QModelIndex doIndex(int const row, int const column, QModelIndex const & parent) const {
      if (parent.isValid() && parent.column() >= TreeItemNode<NE>::NumberOfColumns) {
         return QModelIndex();
      }

      TreeNode * parentNode = this->doTreeNode(parent);
      TreeNode * childNode = parentNode->rawChild(row);

      if (childNode) {
         return this->derived().createIndex(row, column, childNode);
      }

      return QModelIndex();
   }

   QModelIndex indexOfNode(TreeNode * node) const {
      if (!node || node == this->m_rootNode.get()) {
         return QModelIndex();
      }

      return this->derived().createIndex(node->childNumber(), 0, node);
   }

   //! \return Index of the top of the tree
   QModelIndex getRootIndex() {
      //
      // The index for the root node must always be the default (invalid) QModelIndex.  If you try to use
      // this->derived().createIndex to create a valid QModelIndex, then you'll get a crash inside Qt in certain
      // circumstances when you try to insert a child of the root.
      //
      return QModelIndex();
   }

   QModelIndex doParent(QModelIndex const & index) const {
      if (!index.isValid()) {
         return QModelIndex();
      }

      TreeNode * childNode = this->doTreeNode(index);
      if (!childNode) {
         return QModelIndex();
      }

      TreeNode * parentNode = childNode->rawParent();
      return this->indexOfNode(parentNode);
   }

   QString folderPath(QModelIndex const & index) const {

      TreeNode const * treeNode = this->doTreeNode(index);
      if (!treeNode) {
         return "";
      }
      std::shared_ptr<Folder> folder = treeNode->folder();
      if (!folder) {
         return "";
      }
      return folder->path();
   }

   /**
    * \brief Returns "the list of allowed MIME types", which is to say the MIME types that can be dropped on this model
    */
   QStringList doMimeTypes() const {
      QStringList mimeTypesWeAccept;
      //
      // We only accept the primary items stored in this tree and folders.  (It doesn't make sense to allow drag and
      // drop of secondary items such as BrewNotes or MashSteps.)
      //
      mimeTypesWeAccept << TreeItemNode<NE>::DragNDropMimeType;
      mimeTypesWeAccept << TreeFolderNode<NE>::DragNDropMimeType;
      return mimeTypesWeAccept;
   }

   /**
    * \brief Returns a heap-allocated object that contains serialized items of MIME data corresponding to the list of
    *        indexes specified
    */
   QMimeData * doMimeData(QModelIndexList const & indexes) const {

      QByteArray  encodedData;
      QDataStream encodedDataStream(&encodedData, QIODevice::WriteOnly);
      QString mimeType;

      // From what I've been able to tell, the drop events are homogeneous -- a
      // single drop event will be all equipment or all recipe or ...
      for (QModelIndex const & modelIndex : indexes) {

         if (!modelIndex.isValid()) {
            continue;
         }

         //
         // Note below that we have to be careful about character encoding.  If, eg, we write
         //
         //    encodedDataStream << NE::staticMetaObject.className()
         //
         // we would get strange results, because QObject::staticMetaObject.className() returns a C-style string which
         // will give us garbage when we try to read it as a QString at the other end.  (Eg "Fermentable" ends up as
         // "䙥牭敮瑡扬攀"!
         //
         TreeNode * treeNode = this->doTreeNode(modelIndex);
         qDebug() << Q_FUNC_INFO << *treeNode;
         switch (treeNode->classifier()) {
            case TreeNodeClassifier::Folder:
               {
                  auto & folderTreeNode = static_cast<TreeFolderNode<NE> &>(*treeNode);
                  auto folder = folderTreeNode.underlyingItem();
                  // For the moment, Folder does not inherit from NamedEntity, and does not have an ID
                  encodedDataStream <<
                     QString{Folder::staticMetaObject.className()} << -1 << folder->fullPath();
                  mimeType = TreeFolderNode<NE>::DragNDropMimeType;
               }
               break;
            case TreeNodeClassifier::PrimaryItem:
               {
                  auto & itemTreeNode = static_cast<TreeItemNode<NE> &>(*treeNode);
                  auto item = itemTreeNode.underlyingItem();
                  encodedDataStream << QString{NE::staticMetaObject.className()} << item->key() << item->name();
                  mimeType = TreeItemNode<NE>::DragNDropMimeType;
               }
               break;
            case TreeNodeClassifier::SecondaryItem:
               if constexpr (!IsVoid<SNE>) {
                  // Even though we don't currently support drag and drop of secondary items, we still do all the mime
                  // type stuff for them here.  If we decide in future that we want to allow them to be dropped
                  // somewhere we just have to implement a widget that accepts drops of this mime type.
                  auto & secondaryItemTreeNode = static_cast<TreeItemNode<SNE> &>(*treeNode);
                  auto secondaryItem = secondaryItemTreeNode.underlyingItem();
                  encodedDataStream <<
                     QString{SNE::staticMetaObject.className()} << secondaryItem->key() << secondaryItem->name();
                  mimeType = TreeItemNode<SNE>::DragNDropMimeType;
               } else {
                  // If this tree does not support secondary items, then it should not be possible to attempt to drag
                  // and drop them!
                  Q_ASSERT(false);
               }
               break;
         }
      }

      //
      // Much as it grates to do a direct call to new and return the results, this is the required behaviour, since
      // that's what the function we are overriding (QAbstractItemModel::mimeData) does -- see
      // https://github.com/qt/qtbase/blob/dev/src/corelib/itemmodels/qabstractitemmodel.cpp
      //
      QMimeData * mimeData = new QMimeData();
      mimeData->setData(mimeType, encodedData);
      return mimeData;
   }

   //! \brief Accept a drop action
   bool doDropMimeData(QMimeData const * mimeData,
                       [[maybe_unused]] Qt::DropAction action,
                       [[maybe_unused]] int row,
                       [[maybe_unused]] int column,
                       QModelIndex const & parentIndex) {
      // See https://en.wikipedia.org/wiki/Media_type for more on MIME types (now called media types)
      qDebug() <<
         Q_FUNC_INFO << "MIME Data:" << (mimeData ? mimeData->text() : "NULL") << ".  "
         "Parent" << (parentIndex.isValid() ? "valid" : "invalid");

      if (!parentIndex.isValid()) {
         return false;
      }

      qDebug() << Q_FUNC_INFO << "Parent row:" << parentIndex.row() << ", column:" << parentIndex.column();

      QByteArray encodedData;

      if (mimeData->hasFormat(TreeItemNode<NE>::DragNDropMimeType)) {
         encodedData = mimeData->data(TreeItemNode<NE>::DragNDropMimeType);
      } else if (mimeData->hasFormat(TreeFolderNode<NE>::DragNDropMimeType)) {
         encodedData = mimeData->data(TreeFolderNode<NE>::DragNDropMimeType);
      } else {
         qDebug() << Q_FUNC_INFO << "Unrecognised MIME type";
         return false;   // Don't know what we got, but we don't want it
      }

      TreeNode * parentNode = this->doTreeNode(parentIndex);
      if (!parentNode) {
         // Did you know there's a space between elements in a tree, and you can
         // actually drop things there? If somebody drops something there, don't
         // do anything
         qDebug() << Q_FUNC_INFO << "Invalid drop location";
         return false;
      }

      QString targetFolderPath = "";
      if (parentNode->classifier() == TreeNodeClassifier::Folder) {
         auto & folderParentNode = static_cast<TreeFolderNode<NE> &>(*parentNode);
         targetFolderPath = folderParentNode.underlyingItem()->fullPath();
      } else {
         Q_ASSERT(parentNode->classifier() == TreeNodeClassifier::PrimaryItem);
         auto & itemParentNode = static_cast<TreeItemNode<NE> &>(*parentNode);
         targetFolderPath = itemParentNode.underlyingItem()->folderPath();
      }

      qDebug() << Q_FUNC_INFO << "Target:" << targetFolderPath;

      // Pull the stream apart and do that which needs done. Late binding ftw!
      for (QDataStream stream{&encodedData, QIODevice::ReadOnly}; !stream.atEnd(); ) {
         //
         // Obviously the format of what we read here has to align with what we write in TreeViewBase::doMimeData.
         //
         QString className = "";
         int id = -1;
         QString name = "";
         stream >> className >> id >> name;
         qDebug() << Q_FUNC_INFO << "Class:" << className << ", Name:" << name << ", ID:" << id;

         if (className == NE::staticMetaObject.className()) {
            auto item = ObjectStoreWrapper::getById<NE>(id);
            if (!item) {
               qDebug() << Q_FUNC_INFO << "Could not find" << NE::staticMetaObject.className() << "with ID" << id;
               return false;
            }
            auto folder = item->folderPath();
            if (folder.isEmpty()) {
               qDebug() << Q_FUNC_INFO << item << "has no folder";
               return false;
            }
            qDebug() <<
               Q_FUNC_INFO << "Moving" << item << "from folder" << folder << "to folder" << targetFolderPath;
            // Dropping an item in a folder just means setting the folder name on that item
            item->setFolderPath(targetFolderPath);
            // Now we have to update our own model (ie that of TreeModel) so that the display will update!
            this->doFolderChanged(item.get());
         } else if (className == Folder::staticMetaObject.className()) {
            // I need the actual folder object that got dropped.
            auto newFolder = std::make_shared<Folder>();
            newFolder->setfullPath(name);

            this->renameFolder(*newFolder, targetFolderPath);
         }
      }

      return true;
   }

   /**
    * \brief Call this in derived class's constructor.
    */
   void connectSignalsAndSlots() {
      //
      // We want to know about additions or deletions of objects of the type(s) used in our tree
      //
      this->derived().connect(&ObjectStoreTyped<NE>::getInstance(), &ObjectStoreTyped<NE>::signalObjectInserted, &this->derived(), &Derived::elementAdded  );
      this->derived().connect(&ObjectStoreTyped<NE>::getInstance(), &ObjectStoreTyped<NE>::signalObjectDeleted , &this->derived(), &Derived::elementRemoved);
      // For the moment at least, we don't support more than one secondary subclass
      if constexpr (!IsVoid<SNE>) {
         this->derived().connect(&ObjectStoreTyped<SNE>::getInstance(),
                                 &ObjectStoreTyped<SNE>::signalObjectInserted,
                                 &this->derived(),
                                 &Derived::secondaryElementAdded  );
         this->derived().connect(&ObjectStoreTyped<SNE>::getInstance(),
                                 &ObjectStoreTyped<SNE>::signalObjectDeleted ,
                                 &this->derived(),
                                 &Derived::secondaryElementRemoved);
      }
      return;
   }

   /**
    * \brief Add an item to the tree
    */
   void insertPrimaryItem(std::shared_ptr<NE> item) {
      //
      // When we call insertChild below, it results in beginInsertRows() and endInsertRows() signals being emitted.
      // However, for reasons I didn't get to the bottom of, this doesn't result in the display getting updated.
      // Neither does emitting the dataChanged() signal for either the newly-inserted item or its parent.  What does
      // work is the layoutAboutToBeChanged() and layoutChanged() signals.  It seems like a bit of a sledgehammer
      // solution, but it works, and so is useful unless and until we find a better approach.
      //
      TreeModelChangeGuard treeModelChangeGuard(TreeModelChangeType::ChangeLayout, this->derived());

      QModelIndex parentIndex;
      int childNumber;
      QString const folderPath = item->folderPath();
      if (!folderPath.isEmpty()) {
         parentIndex = this->findFolder(folderPath, this->m_rootNode.get(), IfNotFound::Create);
         // I cannot imagine this failing, but what the hell
         if (!parentIndex.isValid()) {
            qCritical() << Q_FUNC_INFO << "Invalid return from findFolder";
            return;
         }
         childNumber = this->doTreeNode(parentIndex)->childCount();
      } else {
         childNumber = this->m_rootNode->childCount();
         parentIndex = this->getRootIndex();
      }

      // Get the parent node here, because, in principle at least, parentIndex will no longer be valid after we call
      // insertChild.
      TreeNode * parentNode = this->doTreeNode(parentIndex);

      qDebug() << Q_FUNC_INFO << "Inserting" << *item << "as child #" << childNumber << "of" << parentIndex;
      if (!this->insertChild(parentIndex, childNumber, item)) {
         qCritical() << Q_FUNC_INFO << "Insert failed";
         return;
      }

      TreeNode * itemRawNode = parentNode->rawChild(childNumber);
      // We know what we just inserted, so this should be a safe cast
      auto & itemNode = static_cast<TreeItemNode<NE> &>(*itemRawNode);

      // Depending on the tree type, there might be secondary items (eg BrewNote items on RecipeTreeModel) or other
      // primary items (eg ancestor Recipes on RecipeTreeModel) under this one.
      this->addSubTreeIfNeeded(*item, itemNode);

      this->observeElement(item);
      return;
   }

   /**
    * \brief Call this at the end of derived class's constructor.
    */
   void loadTreeModel() {
      auto primaryItems = ObjectStoreWrapper::getAllDisplayable<NE>();
      qDebug() << Q_FUNC_INFO << "Got " << primaryItems.length() << NE::staticMetaObject.className() << "items";

      for (auto item : primaryItems) {
         this->insertPrimaryItem(item);
      }

      int const numPrimaryItems = this->m_rootNode->nodeCount(TreeNodeClassifier::PrimaryItem);
      qDebug() <<
         Q_FUNC_INFO << NE::staticMetaObject.className() << "tree now has" <<
         numPrimaryItems << "primary items";
      qDebug().noquote() << Q_FUNC_INFO << "Tree:\n" << this->m_rootNode->subTreeToString();
      if (numPrimaryItems != primaryItems.length()) {
         qCritical() <<
            Q_FUNC_INFO << "Inserting" << primaryItems.length() << NE::staticMetaObject.className() << "items in tree "
            "only resulted in" << numPrimaryItems << "primary items";
      }

      return;
   }

   void observeElement(std::shared_ptr<NE> observed) {
      if (observed) {
         this->derived().connect(observed.get(), &NamedEntity::changedName  , &this->derived(), &Derived::elementChanged);
         // .:TBD:. AFAICT nothing emits NamedEntity::changedFolder...
         this->derived().connect(observed.get(), &NamedEntity::changedFolder, &this->derived(), &Derived::folderChanged );
      }
      return;
   }

   void observeElement(std::shared_ptr<SNE> observed) requires (!IsVoid<SNE>) {
      if (observed) {
         if constexpr (std::same_as<NE, BrewNote>) {
            // For a BrewNote, it's the date, not the name, that we're interested in
            this->derived().connect(observed.get(), &BrewNote::brewDateChanged, &this->derived(), &Derived::secondaryElementChanged);
         } else {
            this->derived().connect(observed.get(), &NamedEntity::changedName , &this->derived(), &Derived::secondaryElementChanged);
         }
      }
      return;
   }

   void unObserveElement(std::shared_ptr<NE> observed) {
      if (observed) {
         this->derived().disconnect(observed.get(), nullptr, &this->derived(), nullptr);
      }
      return;
   }

   void unObserveElement(std::shared_ptr<SNE> observed) requires (!IsVoid<SNE>) {
      if (observed) {
         this->derived().disconnect(observed.get(), nullptr, &this->derived(), nullptr);
      }
      return;
   }

   /**
    * \brief Find the given \c NE (eg given \c Recipe) in the tree.  In most trees (eg \c Equipment, \c Hop,
    *        \c Fermentable, \c Yeast, etc), primary elements can only be inside folders.  But in the \c Recipe tree,
    *        primary elements can also be inside other primary elements (ie a \c Recipe can contain another \c Recipe)
    *        as this is used to show ancestorship (ie prior versions of a \c Recipe).  In all trees, folders can also
    *        contain other folders.  Caller can provide a starting node, otherwise we'll start at the root of the tree.
    *
    * \param ne The primary element (eg \c Recipe) we are looking for.
    */
   QModelIndex findElement(NE const * ne, TreeNode * parent = nullptr) {
      Q_ASSERT(ne);
      if (!parent) {
         parent = this->m_rootNode.get();
      }

      //
      // We do a breadth-first search of the tree.  It seems as good as anything, given we don't have any a priori
      // reason to prefer one search order over another.  An obvious alternative would be a depth-first search using
      // recursion.
      //
      QQueue<TreeNode *> queue;
      queue.enqueue(parent);

      while (!queue.isEmpty()) {
         auto nodeToSearchIn = queue.dequeue();
         qDebug() << Q_FUNC_INFO << "Find" << ne << "in" << *nodeToSearchIn;
         //
         // This is a compile-time check whether it's possible in this tree for primary items to have other primary
         // items as children.  (If not, which is the case in most trees, we can skip over looking for children of
         // primary items.)
         //
         if constexpr (std::is_constructible_v<typename TreeItemNode<NE>::ChildPtrTypes,
                                               std::shared_ptr<TreeItemNode<NE>>>) {
            //
            // This is the case, eg in the Recipe tree, where a primary item (eg a Recipe) can hold other primary
            // items.
            //
            if (nodeToSearchIn->classifier() == TreeNodeClassifier::PrimaryItem) {
               //
               // We assume that nodeToSearchIn doesn't itself match what we're looking for, otherwise we wouldn't have
               // put it on the queue.
               //
               auto & searchInItem = static_cast<TreeItemNode<NE> &>(*nodeToSearchIn);
               for (int childNumInPrimaryItem = 0; childNumInPrimaryItem < searchInItem.childCount(); ++childNumInPrimaryItem) {
                  auto child = searchInItem.child(childNumInPrimaryItem);
                  if (std::holds_alternative<std::shared_ptr<TreeItemNode<NE>>>(child)) {
                     auto itemNode = std::get<std::shared_ptr<TreeItemNode<NE>>>(child);
                     if (itemNode->underlyingItem().get() == const_cast<NE *>(ne)) {
                        // We found what we were looking for
                        qDebug() << Q_FUNC_INFO << "Found as child #" << childNumInPrimaryItem << "of" << searchInItem;
                        return this->derived().createIndex(childNumInPrimaryItem, 0, itemNode.get());
                     }
                     //
                     // The child primary item wasn't a match, but it itself might contain other primary items, so throw
                     // it on the queue.
                     //
                     queue.enqueue(itemNode.get());
                  }
                  // Primary items can never contain folders, so if the child wasn't another primary item, we can ignore
                  // it
               }
               //
               // We processed this nodeToSearchIn item, so go to the next item in the queue.
               //
               continue;
            }
         }

         //
         // If we got here, either nodeToSearchIn cannot be a primary item (because either that's not allowed in this
         // tree or it's handled above), so it's a coding error if we're trying to look inside anything other than a
         // folder.
         //
         Q_ASSERT(nodeToSearchIn->classifier() == TreeNodeClassifier::Folder);
         auto & folderNodeToSearchIn = static_cast<TreeFolderNode<NE> &>(*nodeToSearchIn);
         for (int childNumInFolder = 0; childNumInFolder < folderNodeToSearchIn.childCount(); ++childNumInFolder) {
            auto child = folderNodeToSearchIn.child(childNumInFolder);
            if (std::holds_alternative<std::shared_ptr<TreeItemNode<NE>>>(child)) {
               auto itemNode = std::get<std::shared_ptr<TreeItemNode<NE>>>(child);
               // Normally leave the next line commented out as it generates quite a bit of logging
//               qDebug() << Q_FUNC_INFO << "itemNode:" << *itemNode;
               if (itemNode->underlyingItem().get() == const_cast<NE *>(ne)) {
                  // We found what we were looking for
                  qDebug() << Q_FUNC_INFO << "Found as child #" << childNumInFolder << "of" << folderNodeToSearchIn;
                  return this->derived().createIndex(childNumInFolder, 0, itemNode.get());
               }
               if constexpr (!std::is_constructible_v<typename TreeItemNode<NE>::ChildPtrTypes,
                                                      std::shared_ptr<TreeItemNode<NE>>>) {
                  // We're in a tree where primary items can contain other primary items, and the child primary item
                  // wasn't a match, so throw it on the queue.
                  queue.enqueue(itemNode.get());
               }
            } else if (std::holds_alternative<std::shared_ptr<TreeFolderNode<NE>>>(child)) {
               // We found another folder to look in.  Add it to the list.
               auto folderNode = std::get<std::shared_ptr<TreeFolderNode<NE>>>(child);
               qDebug() << Q_FUNC_INFO << "folderNode:" << *folderNode;
               queue.enqueue(folderNode.get());
            } else {
               // It should be impossible to get here, as folders only contain either primary items or other folders
               Q_ASSERT(false);
            }
         }
      }

      // If we got here, we didn't find a match
      return QModelIndex();
   }

   //
   // Notwithstanding the "requires" condition, GCC 13.3 gives an error about "forming reference to void" when we write:
   //       QModelIndex findElement(SNE const & sne) requires (!IsVoid<SNE>) {
   // Tried using std::add_lvalue_reference_t to construct a type that is:
   //    `SNE &` when SNE is not void
   //    `void`  when SNE is void
   // But GCC still complains.  For now, just use raw pointers, which is slightly less satisfactory.
   //
   QModelIndex findElement(SNE const * sne) requires (!IsVoid<SNE>) {
      Q_ASSERT(sne);
      //
      // Secondary elements are owned by primary ones -- eg BrewNotes are owned by Recipes.  (If they weren't they'd
      // have their own tree -- eg Mash has separate tree from Recipe because Mash is not owned by Recipe.)
      // So, first we find the owner of the supplied element.
      //
      std::shared_ptr<NE> const owner = sne->owner();

      QModelIndex ownerIndex = this->findElement(owner.get());
      //
      // Secondary elements can only be stored inside of primary ones (eg BrewNote cannot live directly in a folder or
      // in another BrewNote), so this cast is safe.
      //
      auto ownerNode = static_cast<TreeItemNode<NE> *>(this->doTreeNode(ownerIndex));
      for (int ii = 0; ii < ownerNode->childCount(); ++ii) {
         auto childNodeVar = ownerNode->child(ii);
         if (std::holds_alternative<std::shared_ptr<TreeItemNode<SNE>>>(childNodeVar)) {
            auto childNode = std::get<std::shared_ptr<TreeItemNode<SNE>>>(childNodeVar);
            std::shared_ptr<SNE> const child = childNode->underlyingItem();
            if (child->key() == sne->key()) {
               // See comment in trees/TreeModel.h for how QModelIndex is used in trees.
               return this->derived().createIndex(ii, 0, childNode.get());
            }
         }
      }

      // If we got here, we didn't find a match
      return QModelIndex();
   }

   /**
    * \param name       Folder name to search for
    * \param parent     Where in the tree to start searching
    * \param ifNotFound Whether to create the folder if it is not found
    * \param folderIsNewlyCreated If supplied, will be set to \c true if the folder was newly created or \c false
    *                             otherwise
    */
   QModelIndex findFolder(QString name,
                          TreeFolderNode<NE> * parent,
                          IfNotFound const ifNotFound,
                          bool * folderIsNewlyCreated = nullptr) {
      auto pItem = parent ? parent : this->m_rootNode.get();

      // Upstream interfaces should handle this for me, but I like belt and suspenders
      name = name.simplified();
      // I am assuming asking me to find an empty name means find the root of the tree.
      if (name.isEmpty()) {
         return this->derived().createIndex(0, 0, pItem);
      }

      // Prepare all the variables for the first loop
      QStringList dirs = name.split("/", Qt::SkipEmptyParts);
      if (dirs.isEmpty()) {
         return QModelIndex();
      }

      QString current = dirs.takeFirst();
      QString fullPath = "/";
      QString targetPath = fullPath % current;

      int ii = 0;

      // Time to get funky with no recursion!
      while (ii < pItem->childCount()) {
         auto child = pItem->child(ii);
         if (std::holds_alternative<std::shared_ptr<TreeFolderNode<NE>>>(child)) {
            // Child is a folder
            auto folderNode = std::get<std::shared_ptr<TreeFolderNode<NE>>>(child);
            auto folder = folderNode->underlyingItem();
            if (folder->isFolder(targetPath)) {
               // The folder name matches the part we are looking at
               if (dirs.isEmpty()) {
                  // There are no more subtrees to look for, we found it
                  return this->derived().createIndex(ii, 0, folderNode.get());
               }
               // Otherwise, we found a parent folder in our path
               // get the next folder in the path
               current = dirs.takeFirst();
               // append that to the fullPath we are looking for
               fullPath = targetPath;
               targetPath = fullPath % "/" % current;

               // Set the parent to the folder
               pItem = folderNode.get();
               // Reset the counter
               ii = 0;
               // And do the time warp again!
               continue;
            }
         }

         // If we got this far, it wasn't a folder or it wasn't a match.
         ii++;
      }
      // If we get here, we found no match.

      // If we are supposed to create something, then let's get busy
      if (ifNotFound == IfNotFound::Create) {
         if (folderIsNewlyCreated) {
            *folderIsNewlyCreated = true;
         }

         // push the current dir back on the stack
         dirs.prepend(current);
         // And start with the madness
         return this->createFolderTree(dirs, pItem, fullPath);
      }

      // If we weren't supposed to create, we drop to here and return an empty index.
      if (folderIsNewlyCreated) {
         *folderIsNewlyCreated = false;
      }
      return QModelIndex();
   }

   auto makeNode(TreeNode * parentNode, std::shared_ptr<NE> element) {
      return std::make_shared<TreeItemNode<NE>>(this->derived(), parentNode, element);
   }
   auto makeNode(TreeNode * parentNode, std::shared_ptr<SNE> element) requires (!IsVoid<SNE>) {
      return std::make_shared<TreeItemNode<SNE>>(this->derived(), parentNode, element);
   }

   template<class ElementType, HasNodeClassifier ParentNodeType>
   bool insertChild(ParentNodeType & parentNode,
                    QModelIndex const & parentIndex,
                    int const row,
                    std::shared_ptr<ElementType> element) requires (IsSubstantiveVariant<typename ParentNodeType::ChildPtrTypes>) {
      // It's a coding error if the child insert position is more than one place beyond the end of the list of current
      // children.  Eg, if there are 4 children, then (because numbering starts from 0), the maximum position at which
      // a child can be inserted is 4.
      int const numChildrenBeforeInsert = parentNode.childCount();
      Q_ASSERT(row <= numChildrenBeforeInsert);
      // It's also a coding error if the child insert position is less than zero!
      Q_ASSERT(row >= 0);

      // Any time we change the tree structure, we need to call functions such beginInsertRows() and endInsertRows() to
      // notify other components that the model has changed.  TreeModelChangeGuard handles the details of this for us.
      TreeModelChangeGuard treeModelChangeGuard(TreeModelChangeType::InsertRows,
                                                this->derived(),
                                                parentIndex,
                                                row,
                                                row);

      auto childNode = std::make_shared<TreeItemNode<ElementType>>(this->derived(), &parentNode, element);
      qDebug() << Q_FUNC_INFO << "Inserting new node " << *childNode << "as child #" << row << "of" << parentNode;

      // Parent node can only be one of two types. (It cannot be SecondaryItem because, although we allow Recipes to
      // contain Recipes -- for Recipe versioning -- we don't allow BrewNotes to contain BrewNotes etc.)
      bool succeeded;
      if constexpr (ParentNodeType::NodeClassifier == TreeNodeClassifier::Folder) {
         auto & parentFolderNode = static_cast<TreeFolderNode<NE> &>(parentNode);
         succeeded = parentFolderNode.insertChild(row, childNode);
      } else {
         Q_ASSERT(ParentNodeType::NodeClassifier == TreeNodeClassifier::PrimaryItem);
         auto & parentItemNode = static_cast<TreeItemNode<NE> &>(parentNode);
         succeeded = parentItemNode.insertChild(row, childNode);
      }

      qDebug() << Q_FUNC_INFO << "Insert" << (succeeded ? "succeeded" : "failed");

      // It's a coding error if the parent node into which we just inserted a child doesn't now have one more child than
      // before!
      Q_ASSERT(parentNode.childCount() == numChildrenBeforeInsert + 1);

      return true;
   }

   /**
    * \brief Use this instead of \c QAbstractItemModel::insertRow or \c QAbstractItemModel::insertRows to add nodes to
    *        the tree.
    *
    * \param parentIndex   The index of the parent tree node of the one we are creating.  (This parent owns the newly-
    *                      created tree node.)
    * \param row           The child number at which to insert the newly-created tree node.
    * \param element       The NamedEntity (primary item, folder or secondary item) we are creating a tree node for
    */
   template<class T>
   bool insertChild(QModelIndex const & parentIndex, int const row, std::shared_ptr<T> element) {
      TreeNode * parentNode = this->doTreeNode(parentIndex);
      if (!parentNode) {
         return false;
      }

      //
      // We don't want to try to compile things that aren't supported.  Eg, none of our models support putting secondary
      // items in anything other than primary items, so we don't want to even compile a code call for putting a
      // secondary item directly in a folder, etc.  Hence why we need the `else` on this `if` statement.
      //
      if constexpr (!IsVoid<SNE> && std::same_as<T, SNE>) {
         //
         // We are inserting a secondary item, so the parent can only be primary item
         //
         Q_ASSERT(parentNode->classifier() == TreeNodeClassifier::PrimaryItem);
         return this->insertChild(static_cast<TreeItemNode<NE> &>(*parentNode), parentIndex, row, element);
      } else {
         //
         // For the moment at least, folders are handled by createFolderTree.
         //
         // So, we know we are inserting a primary item.  This means the parent could be a folder or, in some trees, it
         // could be another primary item (eg an ancestor Recipe in RecipeTreeModel).  To determine at compile time
         // whether the tree supports holding primary items inside other primary items, we look simply at the number of
         // alternatives in ParentPtrTypes.  This will be 1 in all trees that only allow primary items in folders, and
         // it will be 2 for trees that also allow primary items inside other primary items.  (There are no other
         // possibilities.)
         //
         if constexpr (std::variant_size_v<typename TreeItemNode<NE>::ParentPtrTypes> == 2) {
            // This is a tree in which primary items inside each other are supported, so see if parent is primary item
            if (TreeNodeClassifier::PrimaryItem == parentNode->classifier()) {
               return this->insertChild(static_cast<TreeItemNode<NE> &>(*parentNode), parentIndex, row, element);
            }
         }

         //
         // The only remaining valid possibility is that we are inserting the primary item child in a folder
         //
         Q_ASSERT(TreeNodeClassifier::Folder == parentNode->classifier());
         return this->insertChild(static_cast<TreeFolderNode<NE> &>(*parentNode), parentIndex, row, element);
      }

//      std::unreachable();
   }

   template<std::derived_from<TreeNode> TreeNodeType>
   bool removeChildren(int const firstRow,
                       int const count,
                       QModelIndex const & parentIndex,
                       TreeNodeType & parentNode) {
      if (0 == count) {
         // No children to remove = no work to do = succeeded
         return true;
      }

      int const lastRow = firstRow + count - 1;

      // Comment in insertChild applies here too
      TreeModelChangeGuard treeModelChangeGuard(TreeModelChangeType::RemoveRows,
                                                this->derived(),
                                                parentIndex,
                                                firstRow,
                                                lastRow);
      qDebug() << Q_FUNC_INFO << "Removing children" << firstRow << "to" << lastRow << "from" << parentNode;
      return parentNode.removeChildren(firstRow, count);
   }

   /**
    * \brief Use this instead of \c QAbstractItemModel::removeRow or \c QAbstractItemModel::removeRows to remove nodes
    *        from the tree.
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool removeChildren(int row, int count, QModelIndex const & parentIndex) {
      if (0 == count) {
         // No children to remove = no work to do = succeeded
         return true;
      }

      TreeNode * parentNode = this->doTreeNode(parentIndex);
      if (!parentNode) {
         return false;
      }

      //
      // Parent node is usually a folder, though in Recipe tree it can also be a primary item (ie Recipe).  It can never
      // be a secondary item.
      //
      if (parentNode->classifier() == TreeNodeClassifier::Folder) {
         return this->removeChildren(row, count, parentIndex, static_cast<TreeFolderNode<NE> &>(*parentNode));
      }
      Q_ASSERT(parentNode->classifier() == TreeNodeClassifier::PrimaryItem);
      return this->removeChildren(row, count, parentIndex, static_cast<TreeItemNode<NE>   &>(*parentNode));
   }

private:
   QModelIndex createFolderTree(QStringList const & dirs,
                                TreeFolderNode<NE> * parentNode,
                                QString const & parentPath) {
      // Start the loop. We are going to return newIndex at the end,
      // so we need to declare and initialize outside of the loop
      QModelIndex newIndex = this->derived().createIndex(parentNode->childCount(), 0, parentNode);

      // Need to call layoutAboutToBeChanged because we are adding different things with different
      // column counts. Just using the rowsAboutToBeAdded throws ugly errors and
      // then a sigsegv
      TreeModelChangeGuard treeModelChangeGuard(TreeModelChangeType::ChangeLayout, this->derived());
      for (QString cur : dirs) {

         // If we have a parent folder, use its full path.  Otherwise, use the parent path
         QString folderPath =
            parentNode->underlyingItem() ? QString{parentNode->underlyingItem()->fullPath() % "/" % cur} :
                                           QString{parentPath % "/" % cur};

         folderPath.replace(QRegularExpression("//"), "/");

         // Set the full path, which will set the name and the path
         auto newFolder = std::make_shared<Folder>();
         newFolder->setfullPath(folderPath);

         auto newFolderNode = std::make_shared<TreeFolderNode<NE>>(this->derived(), parentNode, newFolder);
         int const numChildren = parentNode->childCount();

         parentNode->insertChild(numChildren, newFolderNode);

         // Set the parent item to point to the newly created tree
         parentNode = newFolderNode.get();

         // And this for the return
         newIndex = this->derived().createIndex(parentNode->childCount(), 0, parentNode);
      }

      // May K&R have mercy on my soul
      return newIndex;
   }

protected:
   void doElementAdded(int itemId) {
      std::shared_ptr<NE> item = ObjectStoreWrapper::getById<NE>(itemId);
      this->insertPrimaryItem(item);
      return;
   }

   //! No-op version
   void doSecondaryElementAdded([[maybe_unused]] int elementId) requires (IsVoid<SNE>) {
      // It's a coding error if this ever gets called!
      Q_ASSERT(false);
      return;
   }
   //! Substantive version
   void doSecondaryElementAdded(int elementId) requires (!IsVoid<SNE>) {
      auto element = ObjectStoreWrapper::getById<SNE>(elementId);
      if (element->deleted()) {
         return;
      }

      std::shared_ptr<NE> owner = element->owner();
      if (!owner) {
         //
         // It is possible for a secondary element to get added to the DB before its "owner" is stored in the DB -- eg
         // see OwnedSet copy constructor.  We'll receive ObjectStoreTyped::signalObjectInserted, but we won't be able
         // to find the owner in the database.  This is OK: we just bail out here.  If and when the owner to gets added
         // to the database, we'll get ObjectStoreTyped::signalObjectInserted for that and update the tree accordingly.
         //
         return;
      }

      QModelIndex parentIndex = this->findElement(owner.get());
      if (!parentIndex.isValid()) {
         return;
      }

      int breadth = this->doRowCount(parentIndex);
      if (!this->insertChild(parentIndex, breadth, element)) {
         return;
      }

      this->observeElement(element);
      return;
   }

private:
   /**
    * \brief For removing an element, the code is largely the same, regardless of the element type
    */
   template<typename T>
   void implElementRemoved(std::shared_ptr<T> element) {
      if (!element) {
         return;
      }

      qDebug() << Q_FUNC_INFO << *element << "was deleted";
      QModelIndex index = this->findElement(element.get());
      if (!index.isValid()) {
         // This is probably a coding error, but we can recover
         qWarning() << Q_FUNC_INFO << "Could not find" << *element << "in the tree";
         return;
      }

      this->removeItemByIndex(index);
      return;
   }

public:
   bool addFolder(QString name) {
      //
      // Since findFolder can also create a folder that doesn't exist, there's pretty much no additional work for us to
      // do here.
      //
      return this->findFolder(name, this->m_rootNode.get(), IfNotFound::Create).isValid();
   }

   /**
    * \brief Delete the contents of the specified node (and of any child nodes) and remove the node from the tree
    */
   bool deleteNode(TreeNode & nodeToDelete) {
      if (this->m_rootNode.get() == &nodeToDelete) {
         // We don't allow you to delete the top-level folder
         return false;
      }

      // Doing the deletion recursively here means the leaves of the tree get deleted first, which is what we want
      for (int childNum = 0; childNum < nodeToDelete.childCount(); ++childNum) {
         if (!this->deleteNode(*nodeToDelete.rawChild(childNum))) {
            return false;
         }
      }

      // Remove the node from the tree structure before we delete its contents
      TreeNode & parentNodeToDelete = *nodeToDelete.rawParent();
      parentNodeToDelete.removeChildren(nodeToDelete.childNumber(), 1);

      if constexpr (!IsVoid<SNE>) {
         // For a secondary item (eg BrewNote, MashStep), we don't delete the item itself because it is owned by a
         // primary item (Recipe, Mash), which will take care of the deletion itself.
         if (nodeToDelete.classifier() != TreeNodeClassifier::SecondaryItem) {
            return true;
         }
      }

      // TBD: For the moment, Folder objects do not have an existence in the database, so there is nothing to delete
      if (nodeToDelete.classifier() == TreeNodeClassifier::PrimaryItem) {
         auto itemNode = static_cast<TreeItemNode<NE> &>(nodeToDelete);
         std::shared_ptr<NE> item = itemNode.underlyingItem();
         qDebug() << Q_FUNC_INFO << "Deleting" << *item;
         ObjectStoreWrapper::softDelete(*item);
      }

      return true;
   }

   bool deleteNode(QModelIndex modelIndex) {
      if (!modelIndex.isValid()) {
         return false;
      }

      return this->deleteNode(*this->doTreeNode(modelIndex));
   }

   QModelIndexList allChildren(QModelIndex const & modelIndex) {
      QModelIndexList leafNodeIndexes;
      if (!modelIndex.isValid()) {
         return leafNodeIndexes;
      }

      TreeNode * treeNode = this->doTreeNode(modelIndex);
      if (treeNode->classifier() != TreeNodeClassifier::Folder) {
         return leafNodeIndexes;
      }

      auto folderNodes = QList<TreeNode *>{};
      folderNodes.append(treeNode);
      while (!folderNodes.isEmpty()) {
         treeNode = folderNodes.takeFirst();

         for (int ii = 0; ii < treeNode->childCount(); ++ii) {
            TreeNode * childNode = treeNode->rawChild(ii);
            if (childNode->classifier() == TreeNodeClassifier::Folder) {
               // Folder: push it onto the folderNodes stack for later processing
               folderNodes.append(childNode);
            } else {
               // TBD: We're calling this leaf node, but it's not necessarily that
               leafNodeIndexes.append(this->derived().createIndex(ii, 0, childNode));
            }
         }
      }
      return leafNodeIndexes;
   }

   /**
    * \brief Remove the item with supplied index from the tree
    */
   bool removeItemByIndex(QModelIndex index) {
      if (!index.isValid()) {
         qWarning() << Q_FUNC_INFO << "Could not find node with index " << index << "in display tree";
         return false;
      }

      // Note that parentIndex.isValid() being false just implies the parent is the root node
      QModelIndex parentIndex = this->derived().parent(index);

      //
      // Comment in insertPrimaryItem() about insertChild equally applies here to removeChildren, hence this guard to
      // emit the layoutAboutToBeChanged() and layoutChanged() signals.
      //
      TreeModelChangeGuard treeModelChangeGuard(TreeModelChangeType::ChangeLayout, this->derived());

      TreeNode * treeNode = this->doTreeNode(index);
      if (treeNode->classifier() == TreeNodeClassifier::PrimaryItem) {
         auto & neTreeNode = static_cast<TreeItemNode<NE> &>(*treeNode);
         std::shared_ptr<NE> neItem = neTreeNode.underlyingItem();
         this->unObserveElement(neItem);
      }

      if constexpr (!IsVoid<SNE>) {
         if (treeNode->classifier() == TreeNodeClassifier::SecondaryItem) {
            auto & sneTreeNode = static_cast<TreeItemNode<SNE> &>(*treeNode);
            std::shared_ptr<SNE> sneItem = sneTreeNode.underlyingItem();
            this->unObserveElement(sneItem);
         }
      }

      int const childNumber = treeNode->childNumber();
      return this->removeChildren(childNumber, 1, parentIndex);
   }

   bool removeElement(NE const & element) {
      qDebug() << Q_FUNC_INFO << "Removing" << element;
      QModelIndex elementIndex = this->findElement(&element);
      return this->removeItemByIndex(elementIndex);
   }

   bool renameFolder(Folder & folder, QString newName) {
      QModelIndex folderIndex = findFolder(folder.fullPath(), nullptr, IfNotFound::ReturnInvalid);
      if (!folderIndex.isValid()) {
         return false;
      }

      QModelIndex parentIndex = this->derived().parent(folderIndex);
      if (!parentIndex.isValid()) {
         return false;
      }

      QString targetPath = newName % "/" % folder.name();
      TreeNode * folderNode = this->doTreeNode(folderIndex);
      QList<QPair<QString, TreeNode *>> folderPathsAndNodes;
      folderPathsAndNodes.append(QPair<QString, TreeNode *>{targetPath, folderNode });

      while (! folderPathsAndNodes.isEmpty()) {
         // This looks weird, but it is needed for later
         auto folderPathAndNode = folderPathsAndNodes.takeFirst();

         targetPath = folderPathAndNode.first;
         TreeNode * target = folderPathAndNode.second;

         // As we move things, childCount changes. This makes sure we loop through all of the kids.
         int src = 0;
         // Ok. We have a folderNode and an index.
         for (int ii = 0; ii < target->childCount(); ++ii) {
            // This looks weird and it is. As we move children out, the 0 items
            // changes to the next child. In the case of a folder, though, we
            // don't move it, so we need to get the item beyond that.
            TreeNode * next = target->rawChild(src);
            // If a folder, append it to the queue for latter processing
            if (next->classifier() == TreeNodeClassifier::Folder) {
               QPair<QString, TreeNode *> newTarget;
               newTarget.first = targetPath % "/" % next->name();
               newTarget.second = next;
               folderPathsAndNodes.append(newTarget);
               src++;
            } else {
               // Secondary items don't have folders, because they belong to exactly one primary item.
               if (next->classifier() == TreeNodeClassifier::PrimaryItem) {
                  auto & primaryNode = static_cast<TreeItemNode<NE> &>(*next);
                  auto item = primaryNode.underlyingItem();
                  item->setFolderPath(targetPath);
               }
            }
         }
      }
      // Last thing is to remove the folder.
      return this->removeChildren(folderNode->childNumber(), 1, parentIndex);
   }

   void doElementRemoved(int elementId) {
      auto element = ObjectStoreWrapper::getById<NE>(elementId);
      this->implElementRemoved(element);
      return;
   }

   //! No-op version
   void doSecondaryElementRemoved([[maybe_unused]] int elementId) requires (IsVoid<SNE>) {
      // It's a coding error if this ever gets called!
      Q_ASSERT(false);
      return;
   }
   //! Substantive version
   void doSecondaryElementRemoved(int elementId) requires (!IsVoid<SNE>) {
      auto element = ObjectStoreWrapper::getById<SNE>(elementId);
      this->implElementRemoved(element);
      return;
   }

   /**
    * \brief
    *
    * \param selectedModelIndexes
    */
   void deleteItems(QModelIndexList const & selectedModelIndexes) {
      //
      // In the loop below, the calls to ObjectStoreWrapper::softDelete will, via the
      // ObjectStoreTyped::signalObjectDeleted signal, result in implElementRemoved being called, so we don't need to
      // remove the object from the tree here, other than in the case that it is a folder (and thus does not have an
      // entity in the DB).
      //
      for (QModelIndex const & modelIndex : selectedModelIndexes) {
         TreeNode * treeNode = this->doTreeNode(modelIndex);
         if (treeNode->classifier() == TreeNodeClassifier::PrimaryItem) {
            auto & neTreeNode = static_cast<TreeItemNode<NE> &>(*treeNode);
            std::shared_ptr<NE> neItem = neTreeNode.underlyingItem();
            ObjectStoreWrapper::softDelete<NE>(*neItem);
            continue;
         }

         if constexpr (!IsVoid<SNE>) {
            if (treeNode->classifier() == TreeNodeClassifier::SecondaryItem) {
               auto & sneTreeNode = static_cast<TreeItemNode<SNE> &>(*treeNode);
               std::shared_ptr<SNE> sneItem = sneTreeNode.underlyingItem();
               ObjectStoreWrapper::softDelete(*sneItem);
               continue;
            }
         }

         Q_ASSERT(treeNode->classifier() == TreeNodeClassifier::Folder);
         // We want to delete the contents of the folder (and remove it from from the model) before remove the folder
         // itself, otherwise the QModelIndex values for the contents will not be valid.
         this->deleteItems(this->allChildren(modelIndex));
         this->removeItemByIndex(modelIndex);
      }
      return;
   }

   /**
    * \brief Copy the supplied items
    *
    * \param toBeCopied - list of pairs of {\c index, \c newName } where \c index is the item to be copied and
    *                     \c newName is the name to give the copy.
    */
   void copyItems(QList<std::pair<QModelIndex, QString>> const & toBeCopied) {
      qDebug() << Q_FUNC_INFO << "Copying" << toBeCopied.length() << "item(s)";
      //
      // We make a list of the things we are going to insert before we insert them, as all the QModelIndex objects will
      // be invalidated by the first insert
      //
      QList<std::pair<TreeNode *, QString>> rawToBeCopied;
      for (auto [modelIndex, newName] : toBeCopied) {
         TreeNode * treeNode = this->doTreeNode(modelIndex);
         rawToBeCopied.append(std::make_pair(treeNode, newName));
      }

      for (auto [treeNode, newName] : rawToBeCopied) {
         if (treeNode->classifier() == TreeNodeClassifier::PrimaryItem) {
            auto & neTreeNode = static_cast<TreeItemNode<NE> &>(*treeNode);
            std::shared_ptr<NE> neItem = neTreeNode.underlyingItem();
            std::shared_ptr<NE> neItemCopy = ObjectStoreWrapper::insertCopyOf(*neItem);
            neItemCopy->setName(newName);
            qDebug() << Q_FUNC_INFO << "Copied" << *neItem << "to" << *neItemCopy;
            //
            // NOTE that we do NOT need to manually add the item to the tree.  Because we are connected to the
            // ObjectStore::signalObjectInserted signal, our doElementAdded() member function will already have been
            // called.  So the new item will already be in our tree.
            //
         } else {
            // It's a coding error if we ask this function to copy either a folder or a secondary item (eg BrewNote or
            // MashStep).  However, we can recover by just not doing the copy.
            qWarning() << Q_FUNC_INFO << "Unexpected item type" << static_cast<int>(treeNode->classifier());
         }
      }
      int const numPrimaryItems = this->m_rootNode->nodeCount(TreeNodeClassifier::PrimaryItem);
      qDebug() <<
         Q_FUNC_INFO << NE::staticMetaObject.className() << "tree now has" << numPrimaryItems << "primary items";
      qDebug().noquote() << Q_FUNC_INFO << "Tree:\n" << this->m_rootNode->subTreeToString();
      return;
   }

private:
   std::shared_ptr<NE> const senderToElement(QObject * sender) {
      NE * elementRaw = qobject_cast<NE *>(sender);
      if (!elementRaw) {
         return nullptr;
      }
      return ObjectStoreWrapper::getSharedFromRaw(elementRaw);
   }

   std::shared_ptr<SNE> const senderToSecondaryElement(QObject * sender) requires (!IsVoid<SNE>) {
      auto elementRaw = qobject_cast<SNE *>(sender);
      if (!elementRaw) {
         return nullptr;
      }
      return ObjectStoreWrapper::getSharedFromRaw(elementRaw);
   }

protected:
   void doElementChanged(QObject * sender) {
      std::shared_ptr<NE> const element = this->senderToElement(sender);
      if (!element) {
         return;
      }

      QModelIndex indexLeft = this->findElement(element.get());
      if (!indexLeft.isValid()) {
         return;
      }

      QModelIndex indexRight = this->derived().createIndex(
         indexLeft.row(),
         this->derived().columnCount(indexLeft) - 1,
         indexLeft.internalPointer()
      );

      emit this->derived().dataChanged(indexLeft, indexRight);
      return;
   }

   void doSecondaryElementChanged(QObject * sender) {
      if constexpr (!IsVoid<SNE>) {
         std::shared_ptr<SNE> const element = this->senderToSecondaryElement(sender);
         if (!element) {
            return;
         }

      } else {
         // It's a coding error for the function to be called when there is no secondary element
         // Static assert would be best here, but need to wait until we're not supporting Ubuntu 22.04
//         static_assert(false);
         Q_ASSERT(false);
      }
      return;
   }

   void addSubTreeIfNeeded(NE const & element, TreeItemNode<NE> & elementNode) {
      //
      // There are two possible reasons a tree could support nodes with sub-trees:
      //    - the tree supports secondary items; and/or
      //    - it's possible in this tree for primary items to have other primary items as children.
      //
      if constexpr (!IsVoid<SNE> ||
                    std::is_constructible_v<typename TreeItemNode<NE>::ChildPtrTypes,
                                             std::shared_ptr<TreeItemNode<NE>>>) {
         //
         // The rules for adding subtrees are class-specific.  Eg for Recipes we need to take account of Ancestors.
         //
         // However, since the logic for MashTreeModel, BoilTreeModel, FermentationTreeModel is the same, we put it here
         // rather than either duplicate it in those three classes or make yet another base class.
         //
         if constexpr (std::same_as<SNE,         MashStep> ||
                       std::same_as<SNE,         BoilStep> ||
                       std::same_as<SNE, FermentationStep>) {
            auto secondaries = SNE::ownedBy(element);
            if (!secondaries.empty()) {
               QModelIndex parentIndex = this->findElement(&element);
               int row = 0;
               for (auto secondary : secondaries) {
                  this->insertChild(parentIndex, row++, secondary);
               }
            }
         } else {
            this->derived().addSubTree(element, elementNode);
         }
      }
      return;
   }

   void doFolderChanged(QObject * sender) {
      std::shared_ptr<NE> const element = this->senderToElement(sender);
      if (!element) {
         return;
      }
      qDebug() << Q_FUNC_INFO << *element;

      // Find the sending item in the existing tree
      QModelIndex elementIndex = this->findElement(element.get());
      if (!elementIndex.isValid()) {
         qWarning() << Q_FUNC_INFO << "Could not find element" << element;
         return;
      }
      auto elementNode = static_cast<TreeItemNode<NE> *>(this->doTreeNode(elementIndex));

      //
      // Remove the sending item from its current parent folder (which will be the root node if it had no folder)
      //
      QModelIndex parentIndex = this->derived().parent(elementIndex);
      // It's a coding error if a PrimaryItem in the tree does not have a parent
      Q_ASSERT(parentIndex.isValid());


      int const elementChildNumber = this->doTreeNode(elementIndex)->childNumber();
      // Remove it
      if (!this->removeChildren(elementChildNumber, 1, parentIndex)) {
         qWarning() << Q_FUNC_INFO << "Could not remove row" << elementChildNumber;
         return;
      }

      // Find the new parent folder.  Note that findFolder() will give us the root node if folderPath is empty, so we
      // don't have to handle that here.  Similarly, we can ask it to create the folder if it (is non-empty and) does
      // not exist.
      QString const & folderPath = element->folderPath();
      bool folderIsNewlyCreated;

      QModelIndex newParentIndex = this->findFolder(folderPath,
                                                    this->m_rootNode.get(),
                                                    IfNotFound::Create,
                                                    &folderIsNewlyCreated);
      // Root node is TreeFolderNode<NE> so, regardless of whether the item has a folder, so this cast should always be
      // valid.
      auto parentFolderNode = static_cast<TreeFolderNode<NE> *>(this->doTreeNode(newParentIndex));

      int const numElementsInFolder = parentFolderNode->childCount();
      if (!this->insertChild(newParentIndex, numElementsInFolder, element)) {
         qWarning() << Q_FUNC_INFO << "Could not insert row" << numElementsInFolder;
         return;
      }

      // If we have brewnotes, set them up here.
      this->addSubTreeIfNeeded(*element, *elementNode);

      if (folderIsNewlyCreated) {
         emit this->derived().expandFolder(newParentIndex);
      }

      return;
   }

   //================================================ Member Variables =================================================
   std::unique_ptr<TreeFolderNode<NE>> m_rootNode;

};

//
// This is a standard macro "trick" used below to allow us to have overloads of the same macro.
//
#define TREE_MODEL_GET_OVERLOAD(param1, param2, NAME, ...) NAME

//
// These macros are used by TREE_MODEL_COMMON_DECL (see below) to handle its optional second parameter
//
#define TREE_MODEL_COMMON_DECL_SNE_1(NeName)
#define TREE_MODEL_COMMON_DECL_SNE_2(NeName, SneName) \
      void secondaryElementAdded  (int elementId); \
      void secondaryElementRemoved(int elementId); \
      void secondaryElementChanged();              \

#define TREE_MODEL_COMMON_DECL_SNE(...) \
   TREE_MODEL_GET_OVERLOAD(__VA_ARGS__ __VA_OPT__(,) \
                           TREE_MODEL_COMMON_DECL_SNE_2, \
                           TREE_MODEL_COMMON_DECL_SNE_1)(__VA_ARGS__)

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define TREE_MODEL_COMMON_DECL(NeName, ...) \
   /* This allows TableModelBase to call protected and private members of Derived */        \
   friend class TreeModelBase<NeName##TreeModel, NeName __VA_OPT__(, __VA_ARGS__)>;         \
                                                                                            \
   public:                                                                                  \
      NeName##TreeModel(TreeView * parent = nullptr);                                       \
      virtual ~NeName##TreeModel();                                                         \
                                                                                            \
      QModelIndex first() const;                                                            \
                                                                                            \
      /** \brief Overrides for TreeModel */                                                 \
      virtual TreeNode * treeNode(QModelIndex const & index) const override;                \
      virtual QString treeForClassName() const override;                                    \
      virtual QString treeForLocalisedClassName() const override;                           \
                                                                                            \
      /** \brief These are all overrides for QAbstractItemModel */                          \
      virtual QVariant data(QModelIndex const & index, int role) const override;            \
      virtual QVariant headerData(int section,                                              \
                                  Qt::Orientation orientation,                              \
                                  int role = Qt::DisplayRole) const override;               \
      virtual int rowCount(QModelIndex const & parent = QModelIndex()) const override;      \
      virtual int columnCount(QModelIndex const & index = QModelIndex()) const override;    \
      virtual QModelIndex index(int row,                                                    \
                                int column,                                                 \
                                QModelIndex const & parent = QModelIndex()) const override; \
      virtual QModelIndex parent(QModelIndex const & index) const override;                 \
      virtual QStringList mimeTypes() const override;                                       \
      virtual QMimeData * mimeData(QModelIndexList const & indexes) const override;         \
      virtual bool dropMimeData(QMimeData const * data,                                     \
                                Qt::DropAction action,                                      \
                                int row,                                                    \
                                int column,                                                 \
                                QModelIndex const & parent) override;                       \
                                                                                            \
   private slots:                                                                           \
      void elementAdded  (int elementId);                                                   \
      void elementRemoved(int elementId);                                                   \
      void elementChanged();                                                                \
      TREE_MODEL_COMMON_DECL_SNE(NeName __VA_OPT__(,) __VA_ARGS__)                          \
      void folderChanged ();                                                                \


//
// These macros are used by TREE_MODEL_COMMON_CODE (see below) to handle its optional second parameter
//
#define TREE_MODEL_COMMON_CODE_SNE_1(NeName)
#define TREE_MODEL_COMMON_CODE_SNE_2(NeName, SneName) \
   void NeName##TreeModel::secondaryElementAdded  (int elementId) { this->doSecondaryElementAdded  (elementId)     ; return; } \
   void NeName##TreeModel::secondaryElementRemoved(int elementId) { this->doSecondaryElementRemoved(elementId)     ; return; } \
   void NeName##TreeModel::secondaryElementChanged()              { this->doSecondaryElementChanged(this->sender()); return; } \

#define TREE_MODEL_COMMON_CODE_SNE(...) \
   TREE_MODEL_GET_OVERLOAD(__VA_ARGS__ __VA_OPT__(,) \
                           TREE_MODEL_COMMON_CODE_SNE_2, \
                           TREE_MODEL_COMMON_CODE_SNE_1)(__VA_ARGS__)

/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        NB: Mostly I have tried to make these macro-included function bodies trivial.  Macros are a bit clunky, so we
 *            only really want to use them for the things that are hard to do other ways.
 */
#define TREE_MODEL_COMMON_CODE(NeName, ...) \
   NeName##TreeModel::NeName##TreeModel(TreeView * parent) :                     \
      TreeModel{parent},                                                         \
      TreeModelBase<NeName##TreeModel, NeName __VA_OPT__(, __VA_ARGS__)>{} {     \
         this->connectSignalsAndSlots();                                         \
         this->loadTreeModel();                                                  \
         return;                                                                 \
      }                                                                          \
   NeName##TreeModel::~NeName##TreeModel() = default;                            \
                                                                                 \
   QModelIndex NeName##TreeModel::first() const { return this->doFirst(); }      \
                                                                                 \
   TreeNode * NeName##TreeModel::treeNode(QModelIndex const & index) const {     \
      return this->doTreeNode(index);                                            \
   }                                                                             \
                                                                                 \
   QVariant NeName##TreeModel::data(QModelIndex const & index, int role) const { \
      return this->doData(index, role);                                          \
   }                                                                             \
   QVariant NeName##TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {     \
      return this->doHeaderData(section, orientation, role);                                              \
   }                                                                                                      \
   int NeName##TreeModel::rowCount(QModelIndex const & parent) const { return this->doRowCount(parent); } \
   int NeName##TreeModel::columnCount(QModelIndex const & parent) const {                                 \
      return this->doColumnCount(parent);                                                                 \
   }                                                                                                      \
   QModelIndex NeName##TreeModel::index(int row, int column, QModelIndex const & parent) const {             \
      return this->doIndex(row, column, parent);                                                             \
   }                                                                                                         \
   QModelIndex NeName##TreeModel::parent(QModelIndex const & index) const { return this->doParent(index); }  \
   QStringList NeName##TreeModel::mimeTypes() const {                  \
      return this->doMimeTypes();                                      \
   }                                                                   \
   QMimeData * NeName##TreeModel::mimeData(QModelIndexList const & indexes) const { \
      return this->doMimeData(indexes);                                             \
   }                                                                                \
   bool NeName##TreeModel::dropMimeData(QMimeData const * data,        \
                                        Qt::DropAction action,         \
                                        int row,                       \
                                        int column,                    \
                                        QModelIndex const & parent) {  \
      return this->doDropMimeData(data, action, row, column, parent);  \
   }                                                                   \
                                                                       \
   QString NeName##TreeModel::treeForClassName         () const { return this->doTreeForClassName(); }          \
   QString NeName##TreeModel::treeForLocalisedClassName() const { return this->doTreeForLocalisedClassName(); } \
                                                                                                                \
   void NeName##TreeModel::elementAdded  (int elementId) { this->doElementAdded  (elementId)     ; return; }    \
   void NeName##TreeModel::elementRemoved(int elementId) { this->doElementRemoved(elementId)     ; return; }    \
   void NeName##TreeModel::elementChanged()              { this->doElementChanged(this->sender()); return; }    \
   TREE_MODEL_COMMON_CODE_SNE(NeName __VA_OPT__(,) __VA_ARGS__)                                                 \
   void NeName##TreeModel::folderChanged ()              { this->doFolderChanged (this->sender()); return; }    \


#endif
