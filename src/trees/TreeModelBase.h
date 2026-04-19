/*======================================================================================================================
 * trees/TreeModelBase.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#include <set>
#include <type_traits>
#include <variant>
#include <vector>
#include <utility>

#include <QDebug>
#include <QMimeData>
#include <QModelIndex>
#include <QQueue>
#include <QString>
#include <QStringBuilder> // Needed for efficient QString concatenation operator (%)
#include <QVariant>
#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE
#include <QMessageBox>

#include "database/ObjectStoreWrapper.h"
#include "trees/TreeNode.h"
#include "trees/TreeModel.h"
#include "trees/TreeModelChangeGuard.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeTraits.h"

/**
 * \brief For logging \c std::set
 *
 *        This probably belongs somewhere else!
 */
template<class S, class T>
S & operator<<(S & stream, std::set<T> const & val) {
   QString output;
   QTextStream outputAsStream{&output};
   for (T const & item : val) {
      if (!output.isEmpty()) {
         outputAsStream << ", ";
      }
      outputAsStream << item;
   }
   stream << "std::set {" << output << "}";
   return stream;
}


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
 * \param SNE - The optional secondary \c NamedEntity subclass shown in this tree (eg \c BrewLog for
 *              \c RecipeTreeModel, or \c MashStep for \c MashTreeModel).  This class must have:
 *                 • an \c owner() member function that does the obvious thing (eg \c BrewLog::owner() returns a
 *                   \c Recipe; \c MashStep::owner returns a \c Mash);
 *                 • a static \c ownedBy() member function that returns all the \c BrewLog objects owned by a given
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
      return ColumnOwnerTraits<TreeItemNode<NE>>::numColumns();
   }

   QModelIndex doIndex(int const row, int const column, QModelIndex const & parent) const {
      if (parent.isValid() && parent.column() >= ColumnOwnerTraits<TreeItemNode<NE>>::numColumns()) {
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
         // See comment in getRootIndex() for why we must return a default QModelIndex for the root node.
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

   /**
    * \brief Gets the folder (or containing folder), if any, for the supplied index.  Thus, if the item at the  supplied
    *        index is a folder, that is returned.  Otherwise, the item's containing folder, if any, is returned.
    *
    *        NB: Not all trees support folders.  Specifically, the StockPurchase/StockUse trees do not have folders.
    */
   std::shared_ptr<Folder<NE>> folder(TreeNode const * selectedNode) const requires (HasFolder<NE>) {
      switch (selectedNode->classifier()) {
         case TreeNodeClassifier::PrimaryItem  : {
            auto const & primaryNode = static_cast<TreeItemNode<NE> const &>(*selectedNode);
            return primaryNode.underlyingItem()->containedInFolder();
         }

         case TreeNodeClassifier::SecondaryItem: {
            if constexpr (!IsVoid<SNE>) {
               auto const & secondaryNode = static_cast<TreeItemNode<SNE> const &>(*selectedNode);
               return secondaryNode.underlyingItem()->owner()->containedInFolder();
            }
            break;
         }

         case TreeNodeClassifier::Folder       : {
            auto const & folderNode = static_cast<TreeFolderNode<NE> const &>(*selectedNode);
            return folderNode.underlyingItem();
         }

         // No default as we want the compiler to warn us if we missed an option above
      }
      return nullptr;
   }

   std::shared_ptr<Folder<NE>> folder(QModelIndex const & index) const requires (HasFolder<NE>) {
      TreeNode const * treeNode = this->doTreeNode(index);
      if (!treeNode) {
         return nullptr;
      }
      return this->folder(treeNode);
   }

   Folder<NE> const * folderRaw(QModelIndex const & index) const requires (HasFolder<NE>) {
      return this->folder(index).get();
   }

   /**
    * \brief Returns "the list of allowed MIME types", which is to say the MIME types that can be dropped on this model
    */
   [[nodiscard]] QStringList doMimeTypes() const {
      QStringList mimeTypesWeAccept;
      //
      // We only accept the primary items stored in this tree and folders.  (It doesn't make sense to allow drag and
      // drop of secondary items such as BrewLogs or MashSteps.)
      //
      mimeTypesWeAccept << TreeItemNode<NE>::dragAndDropMimeType();
      if constexpr (HasFolder<NE>) {
         mimeTypesWeAccept << TreeFolderNode<NE>::dragAndDropMimeType();
      }
      return mimeTypesWeAccept;
   }

   /**
    * \brief Returns a heap-allocated object that contains serialized items of MIME data corresponding to the list of
    *        indexes specified
    */
   [[nodiscard]] QMimeData * doMimeData(QModelIndexList const & indexes) const {

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
         // What we encode here has to correspond with what we look for below in doDropMimeData(), specifically:
         //    - Classname of the object being dragged
         //    - ID of the object
         //    - Name or other descriptor (only used for debugging)
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
         qDebug() << Q_FUNC_INFO << "Dragging" << *treeNode;
         switch (treeNode->classifier()) {
            case TreeNodeClassifier::Folder:
               if constexpr (HasFolder<NE>) {
                  auto & folderTreeNode = static_cast<TreeFolderNode<NE> &>(*treeNode);
                  auto folder = folderTreeNode.underlyingItem();
                  // Folder::className returns QString, so no need to wrap it.  And, for folders, fullPath is better for
                  // debugging than just name.
                  encodedDataStream << Folder<NE>::staticClassName() << folder->key() << folder->fullPath();
                  mimeType = TreeFolderNode<NE>::dragAndDropMimeType();
               } else {
                  // It's a coding error if we have a Folder node in a tree that does not support folders
                  qCritical() << Q_FUNC_INFO << "Unexpected tree node:" << *treeNode;
               }
               break;
            case TreeNodeClassifier::PrimaryItem:
               {
                  auto & itemTreeNode = static_cast<TreeItemNode<NE> &>(*treeNode);
                  auto item = itemTreeNode.underlyingItem();
                  encodedDataStream << QString{NE::staticMetaObject.className()} << item->key() << item->name();
                  mimeType = TreeItemNode<NE>::dragAndDropMimeType();
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
                  mimeType = TreeItemNode<SNE>::dragAndDropMimeType();
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

   /**
    * \brief Accept a drop action
    *
    * \return This is implementing an override of QAbstractItemModel::dropMimeData, so returns \c true if the data and
    *         action were handled by the model; otherwise returns \c false.
    */
   bool doDropMimeData(QMimeData const * mimeData,
                       [[maybe_unused]] Qt::DropAction action,
                       [[maybe_unused]] int row,
                       [[maybe_unused]] int column,
                       QModelIndex const & parentIndex) {
      // See https://en.wikipedia.org/wiki/Media_type for more on MIME types (now called media types)
      qDebug() <<
         Q_FUNC_INFO << "MIME Data:" << (mimeData ? mimeData->text() : "NULL") << ".  "
         "Parent" << (parentIndex.isValid() ? "valid" : "invalid");

      //
      // TBD For the moment, we skip most folder handling on StockPurchase items (which don't have folders).  We still
      // have to allow the existence of TreeFolderNode for them to have the root node, but we turn off everything else.
      //
      if constexpr (HasNoFolder<NE>) {
         return false;
      } else {

         if (!parentIndex.isValid()) {
            return false;
         }

         qDebug() << Q_FUNC_INFO << "Parent row:" << parentIndex.row() << ", column:" << parentIndex.column();

         QByteArray encodedData;

         if (mimeData->hasFormat(TreeItemNode<NE>::dragAndDropMimeType())) {
            encodedData = mimeData->data(TreeItemNode<NE>::dragAndDropMimeType());
         } else if (mimeData->hasFormat(TreeFolderNode<NE>::dragAndDropMimeType())) {
            encodedData = mimeData->data(TreeFolderNode<NE>::dragAndDropMimeType());
         } else {
            qDebug() << Q_FUNC_INFO << "Unrecognised MIME type";
            return false;   // Don't know what we got, but we don't want it
         }

         TreeNode const * parentNode = this->doTreeNode(parentIndex);
         if (!parentNode) {
            // Did you know there's a space between elements in a tree, and you can
            // actually drop things there? If somebody drops something there, don't
            // do anything
            qDebug() << Q_FUNC_INFO << "Invalid drop location";
            return false;
         }

         auto targetFolder = this->folder(parentNode);
         qDebug() << Q_FUNC_INFO << "Target:" << targetFolder;

         //
         // Pull the stream apart and do that which needs done. Late binding ftw!
         //
         for (QDataStream stream{&encodedData, QIODevice::ReadOnly}; !stream.atEnd(); ) {
            //
            // Obviously the format of what we read here has to align with what we write above doMimeData().
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
               auto folder = item->containedInFolder();
               qDebug() <<
                  Q_FUNC_INFO << "Moving" << item << "from folder" << folder << "to folder" << targetFolder;
               // Dropping an item in a folder just means setting the folder name on that item
               item->setContainedInFolder(targetFolder);
               // Now we have to update our own model (ie that of TreeModel) so that the display will update!
               this->movedFolder(item);
            } else if (className == Folder<NE>::staticClassName()) {
               auto droppedFolder = ObjectStoreWrapper::getById<Folder<NE>>(id);
               auto folder = droppedFolder->containedInFolder();
               qDebug() <<
                  Q_FUNC_INFO << "Moving" << droppedFolder << "from folder" << folder << "to folder" << targetFolder;
               droppedFolder->setContainedInFolder(targetFolder);
               this->movedFolder(droppedFolder);
            }
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
                                 &ObjectStoreTyped<SNE>::signalObjectDeleted,
                                 &this->derived(),
                                 &Derived::secondaryElementRemoved);
      }
      if constexpr (HasFolder<NE>) {
         this->derived().connect(&ObjectStoreTyped<Folder<NE>>::getInstance(),
                                 &ObjectStoreTyped<Folder<NE>>::signalObjectInserted,
                                 &this->derived(),
                                 &Derived::folderAdded);
         this->derived().connect(&ObjectStoreTyped<Folder<NE>>::getInstance(),
                                 &ObjectStoreTyped<Folder<NE>>::signalObjectDeleted,
                                 &this->derived(),
                                 &Derived::folderRemoved);
      }

      return;
   }

   /**
    * \brief Add an item to the tree
    */
   void insertPrimaryItem(std::shared_ptr<NE> item) {
      QModelIndex parentIndex;
      int childNumber = -1;

      if constexpr (HasFolder<NE>) {
         if (auto const folder = item->containedInFolderRaw();
             folder) {
            parentIndex = this->findFolder(folder, IfNotFound::Create);
            // I cannot imagine this failing, but what the hell
            if (!parentIndex.isValid()) {
               qCritical() << Q_FUNC_INFO << "Invalid return from findFolder";
               return;
            }
            childNumber = this->doTreeNode(parentIndex)->childCount();
         }
      }

      // This is slightly clunky, but minimises duplicating code for the HasNoFolder case
      if (childNumber < 0) {
         childNumber = this->m_rootNode->childCount();
         parentIndex = this->getRootIndex();
      }

      // Get the parent node here, because, in principle at least, parentIndex will no longer be valid after we call
      // insertChild.
      TreeNode const * parentNode = this->doTreeNode(parentIndex);

      // Normally leave this debug statement commented out as otherwise it generates too much logging
//      qDebug() << Q_FUNC_INFO << "Inserting" << *item << "as child #" << childNumber << "of" << parentIndex;
      if (!this->insertChild(parentIndex, childNumber, item)) {
         qCritical() << Q_FUNC_INFO << "Insert failed";
         return;
      }

      TreeNode * itemRawNode = parentNode->rawChild(childNumber);
      // We know what we just inserted, so this should be a safe cast
      auto & itemNode = static_cast<TreeItemNode<NE> &>(*itemRawNode);

      // Depending on the tree type, there might be secondary items (eg BrewLog items on RecipeTreeModel) or other
      // primary items (eg ancestor Recipes on RecipeTreeModel) under this one.
      this->addSubTreeIfNeeded(*item, itemNode);

      this->observe(item);
      return;
   }

   /**
    * \brief Call this at the end of derived class's constructor.
    */
   void loadTreeModel() {
      auto primaryItems = ObjectStoreWrapper::getAllDisplayable<NE>();
      qDebug() << Q_FUNC_INFO << "Got " << primaryItems.length() << NE::staticMetaObject.className() << "items";

      // Loading all the primary items gives us all the non-empty folders for free
      for (auto item : primaryItems) {
         this->insertPrimaryItem(item);
      }

      if constexpr (HasFolder<NE>) {
         // In theory, we just need to add the empty folders.  In practice, it's easier just to give all the folders to
         // findFolder than work out which folders are empty.
         auto allFolders = ObjectStoreWrapper::getAllDisplayableRaw<Folder<NE>>();
         qDebug() << Q_FUNC_INFO << "Got " << allFolders.length() << NE::staticMetaObject.className() << "folders";
         for (auto folder : allFolders) {
            this->findFolder(folder, IfNotFound::Create);
         }
      }

      int const numPrimaryItems = this->m_rootNode->nodeCount(TreeNodeClassifier::PrimaryItem);
      qDebug() <<
         Q_FUNC_INFO << NE::staticMetaObject.className() << "tree now has" <<
         numPrimaryItems << "primary items";
      qDebug().noquote() << Q_FUNC_INFO << "Tree:\n" << this->m_rootNode->subTreeToString();
      //
      // It's possible for the tree to have _more_ primary items than we inserted (because, eg in a Recipe tree, we add
      // the ancestors of each recipe), but it should never have fewer.
      //
      if (numPrimaryItems < primaryItems.length()) {
         qCritical() <<
            Q_FUNC_INFO << "Inserting" << primaryItems.length() << NE::staticMetaObject.className() << "items in tree "
            "only resulted in" << numPrimaryItems << "primary items";
      }

      return;
   }

   void observe(std::shared_ptr<NE> observed) {
      if (observed) {
         this->derived().connect(observed.get(), &NamedEntity::changed, &this->derived(), &Derived::elementChanged);

         if constexpr (!IsVoid<SNE>) {
            // Normally leave this commented out as otherwise it generates too much logging
//            qDebug() <<
//               Q_FUNC_INFO << "Connecting ownedItemsChanged signal from" << NE::staticMetaObject.className() << "#" <<
//               observed->key() << "to" << Derived::staticMetaObject.className();
            this->derived().connect(observed.get(), &NE::ownedItemsChanged, &this->derived(), &Derived::secondaryElementsChanged);
         }
      }
      return;
   }

   void observe(std::shared_ptr<SNE> observed) requires (!IsVoid<SNE>) {
      if (observed) {
         if constexpr (std::same_as<NE, BrewLog>) {
            // For a BrewLog, it's the date, not the name, that we're interested in
            this->derived().connect(observed.get(), &BrewLog::brewDateChanged, &this->derived(), &Derived::secondaryElementChanged);
         } else {
            this->derived().connect(observed.get(), &NamedEntity::changed, &this->derived(), &Derived::secondaryElementChanged);
         }
      }
      return;
   }

   void observe(std::shared_ptr<Folder<NE>> observed) requires (HasFolder<NE>) {
      if (observed) {
         this->derived().connect(observed.get(), &NamedEntity::changed, &this->derived(), &Derived::folderChanged);
      }
      return;
   }

   void unObserve(std::shared_ptr<NE> observed) {
      if (observed) {
         this->derived().disconnect(observed.get(), nullptr, &this->derived(), nullptr);
      }
      return;
   }

   void unObserve(std::shared_ptr<SNE> observed) requires (!IsVoid<SNE>) {
      if (observed) {
         this->derived().disconnect(observed.get(), nullptr, &this->derived(), nullptr);
      }
      return;
   }

   void unObserve(std::shared_ptr<Folder<NE>> observed) requires (HasFolder<NE>) {
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
    * \param parent
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
         // Normally leave the next line commented out as it generates quite a bit of logging
//         qDebug() << Q_FUNC_INFO << "Find" << ne << "(at" << static_cast<void const *>(ne) << ") in" << *nodeToSearchIn;
         if (nodeToSearchIn->classifier() == TreeNodeClassifier::PrimaryItem) {
            //
            // This is a compile-time check whether it's possible in this tree for primary items to have other primary
            // items as children.  (If not, which is the case in most trees, we can skip looking for children of primary
            // items.)
            //
            if constexpr (std::is_constructible_v<typename TreeItemNode<NE>::ChildPtrTypes,
                                                  std::shared_ptr<TreeItemNode<NE>>>) {
               //
               // Primary items can be inside other primary items in this tree.  This is the case, eg in the Recipe
               // tree.
               //
               // We assume that nodeToSearchIn doesn't itself match what we're looking for, otherwise we wouldn't have
               // put it on the queue.
               //
               auto & searchInItem = static_cast<TreeItemNode<NE> &>(*nodeToSearchIn);
               for (int childNumInPrimaryItem = 0; childNumInPrimaryItem < searchInItem.childCount(); ++childNumInPrimaryItem) {
                  auto child = searchInItem.child(childNumInPrimaryItem);
                  if (std::holds_alternative<std::shared_ptr<TreeItemNode<NE>>>(child)) {
                     auto itemNode = std::get<std::shared_ptr<TreeItemNode<NE>>>(child);
                     // Normally leave the next line commented out as it can generate quite a bit of logging
//                     qDebug() <<
//                        Q_FUNC_INFO << "itemNode:" << *itemNode << "(at " <<
//                        static_cast<void *>(itemNode->underlyingItem().get()) << ")";
                     if (itemNode->underlyingItem().get() == const_cast<NE *>(ne)) {
                        // We found what we were looking for
//                        qDebug() << Q_FUNC_INFO << "Found as child #" << childNumInPrimaryItem << "of" << searchInItem;
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
            }

            //
            // We either processed this nodeToSearchIn item or skipped over it (because this tree does not support
            // primary items inside other primary items), so go to the next item in the queue.
            //
            continue;
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
//               qDebug() <<
//                  Q_FUNC_INFO << "itemNode:" << *itemNode << "(at " <<
//                  static_cast<void *>(itemNode->underlyingItem().get()) << ")";
               if (itemNode->underlyingItem().get() == const_cast<NE *>(ne)) {
                  // We found what we were looking for
//                  qDebug() << Q_FUNC_INFO << "Found as child #" << childNumInFolder << "of" << folderNodeToSearchIn;
                  return this->derived().createIndex(childNumInFolder, 0, itemNode.get());
               }
               if constexpr (std::is_constructible_v<typename TreeItemNode<NE>::ChildPtrTypes,
                                                     std::shared_ptr<TreeItemNode<NE>>>) {
                  // We're in a tree where primary items can contain other primary items, and the child primary item
                  // wasn't a match, so throw it on the queue.
                  queue.enqueue(itemNode.get());
               }
            } else if (std::holds_alternative<std::shared_ptr<TreeFolderNode<NE>>>(child)) {
               // We found another folder to look in.  Add it to the list.
               auto folderNode = std::get<std::shared_ptr<TreeFolderNode<NE>>>(child);
//               qDebug() << Q_FUNC_INFO << "folderNode:" << *folderNode;
               queue.enqueue(folderNode.get());
            } else {
               // It should be impossible to get here, as folders only contain either primary items or other folders
               Q_ASSERT(false);
            }
         }
      }

      // If we got here, we didn't find a match
      return {};
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
      // Secondary elements are owned by primary ones -- eg BrewLogs are owned by Recipes.  (If they weren't they'd
      // have their own tree -- eg Mash has separate tree from Recipe because Mash is not owned by Recipe.)
      // So, first we find the owner of the supplied element.
      //
      std::shared_ptr<NE> const owner = sne->owner();

      QModelIndex const ownerIndex = this->findElement(owner.get());
      //
      // Secondary elements can only be stored inside of primary ones (eg BrewLog cannot live directly in a folder or
      // in another BrewLog), so this cast is safe.
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
      return {};
   }

   QModelIndex findElement(Folder<NE> const * folder, TreeNode * parent = nullptr) requires (HasFolder<NE>) {
      //
      // You might think we can just call findFolder here, but that isn't going to work if the folder's path has changed
      // since it was inserted in the tree -- which is the case if we are being called from movedFolder.
      //
      // The logic here is similar to (but simpler than) the overload where we are looking for a primary element.  At
      // the moment, I think there are enough differences that it's not worth trying to combine the two cases.
      //
      Q_ASSERT(folder);
      if (!parent) {
         parent = this->m_rootNode.get();
      }

      QQueue<TreeNode *> queue;
      queue.enqueue(parent);

      while (!queue.isEmpty()) {
         auto nodeToSearchIn = queue.dequeue();

         // We should only be looking inside folders
         Q_ASSERT(nodeToSearchIn->classifier() == TreeNodeClassifier::Folder);
         auto & folderNodeToSearchIn = static_cast<TreeFolderNode<NE> &>(*nodeToSearchIn);
         for (int childNumInFolder = 0; childNumInFolder < folderNodeToSearchIn.childCount(); ++childNumInFolder) {
            if (auto child = folderNodeToSearchIn.child(childNumInFolder);
                std::holds_alternative<std::shared_ptr<TreeFolderNode<NE>>>(child)) {

               auto folderNode = std::get<std::shared_ptr<TreeFolderNode<NE>>>(child);
               if (folderNode->underlyingItem().get() == const_cast<Folder<NE> *>(folder)) {
                  // We found what we were looking for
                  return this->derived().createIndex(childNumInFolder, 0, folderNode.get());
               }
               // Add the folder we just found to the list of folders to search in
               queue.enqueue(folderNode.get());
            }
         }

      }

      // If we got here, we didn't find a match
      return {};
   }

   /**
    * Find a folder in the tree, and, if not present, optionally create it.
    *
    * NOTE that this will only find a folder that is at its correct location in the tree.  If a folder is in the wrong
    * place in the tree, eg because its path has just changed, the relevant overload of findElement is needed instead.
    *
    * \param targetFolder         Folder<NE> to search for -- OK to pass nullptr
    * \param ifNotFound           Whether to create the folder if it is not found
    * \param folderIsNewlyCreated If supplied, will be set to \c true if the folder was newly created or \c false
    *                             otherwise
    */
   QModelIndex findFolder(Folder<NE> const * targetFolder,
                          IfNotFound const ifNotFound,
                          bool * folderIsNewlyCreated = nullptr) requires HasFolder<NE> {
      if (targetFolder) {
         qDebug() << Q_FUNC_INFO << "Searching for" << targetFolder;
         TreeFolderNode<NE> * parentNode = this->m_rootNode.get();

         //
         // We follow the path of the Folder that we're seeking and, at each stage, look for the corresponding
         // TreeFolderNode.  If we've been asked to create the (display tree) folder in the event that no match was
         // found then, as soon as we don't find a match we know we need to create TreeFolderNodes for all the remaining
         // Folders in the path.
         //
         // Eg, if we are searching for /foo/bar/hum/bug/cat/dog, and we find /foo/bar but not /foo/bar/hum, then we
         // know we need to create:
         //   /foo/bar/hum
         //   /foo/bar/hum/bug
         //   /foo/bar/hum/bug/cat
         //   /foo/bar/hum/bug/cat/dog
         //
         // The way we do this is to have two "modes" in which the outer loop can run.  If we are in "searching" mode,
         // then we are looking for matches.  If not, then we are just creating missing folders.  (If we are not
         // required to create folders when no match is found, we just break out of the loop.)
         //
         bool searching = true;
         for (QList<Folder<NE> *> targetPath = targetFolder->folderPath();
              Folder<NE> * segment : targetPath) {

//            qDebug() << Q_FUNC_INFO << "segment" << *segment;
            bool found = false;
            if (searching) {
               for (int ii = 0; ii < parentNode->childCount(); ++ii) {
                  if (auto childNode = parentNode->child(ii);
                      std::holds_alternative<std::shared_ptr<TreeFolderNode<NE>>>(childNode)) {
                     // Child is a folder
                     auto childFolderNode = std::get<std::shared_ptr<TreeFolderNode<NE>>>(childNode);
//                     qDebug() << Q_FUNC_INFO << "Checking" << *childFolderNode;
                     if (auto folder = childFolderNode->underlyingItem();
                         folder->key() == segment->key()) {
//                        qDebug() << Q_FUNC_INFO << "Matched segment";
                        if (folder->key() == targetFolder->key()) {
                           // Reached the end of the chain
//                           qDebug() << Q_FUNC_INFO << "Found target folder";
                           return this->derived().createIndex(ii, 0, childFolderNode.get());
                        }

                        parentNode = childFolderNode.get();
                        // Jump out of the inner for loop
                        found = true;
                        break;
                     }
                  }
               }
            }
            if (!found) {
               //
               // Inner for loop didn't find a match (or we didn't run it because we already found no match on a
               // previous iteration of the outer loop).
               //
               if (ifNotFound == IfNotFound::Create) {
                  searching = false;

                  // Need to call layoutAboutToBeChanged because we are adding different things with different column
                  // counts. Just using the rowsAboutToBeAdded throws ugly errors and then a sigsegv.
                  TreeModelChangeGuard treeModelChangeGuard(TreeModelChangeType::ChangeLayout, this->derived());

                  std::shared_ptr<Folder<NE>> folder = ObjectStoreWrapper::getSharedFromRaw(segment);
                  auto newFolderNode = std::make_shared<TreeFolderNode<NE>>(
                     this->derived(),
                     parentNode,
                     folder
                  );
                  int const numChildren = parentNode->childCount();

                  parentNode->insertChild(numChildren, newFolderNode);
                  qDebug() << Q_FUNC_INFO << "Created" << *newFolderNode << "inside" << parentNode;
                  this->observe(folder);

                  // Parent TreeFolderNode now owns the one we just created, so we don't need to worry about the shared
                  // pointer going out of scope below.

                  if (segment->key() == targetFolder->key()) {
                     // Reached the end of the chain
                     if (folderIsNewlyCreated) {
                        *folderIsNewlyCreated = true;
                     }
                     return this->derived().createIndex(numChildren + 1, 0, newFolderNode.get());
                  }

                  // Get ready for the next go round the outer loop
                  parentNode = newFolderNode.get();
               } else {
                  //
                  // We don't need to create the remainder of the chain, so break out of the outer loop
                  //
                  break;
               }
            }
         }
      }

      // If we get here, we found no match, and we weren't asked to create node(s) in that event, so return an empty
      // index.
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
   auto makeNode(TreeNode * parentNode, std::shared_ptr<Folder<NE>> element) requires (HasFolder<NE>) {
      return std::make_shared<TreeFolderNode<NE>>(this->derived(), parentNode, element);
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

      auto childNode = this->makeNode(&parentNode, element);
      // Normally leave this debug statement commented out as otherwise it generates too much logging
//      qDebug() << Q_FUNC_INFO << "Inserting new node " << *childNode << "as child #" << row << "of" << parentNode;

      // Parent node can only be one of two types. (It cannot be SecondaryItem because, although we allow Recipes to
      // contain Recipes -- for Recipe versioning -- we don't allow BrewLogs to contain BrewLogs etc.)
      bool succeeded;
      if constexpr (ParentNodeType::NodeClassifier == TreeNodeClassifier::Folder) {
         auto & parentFolderNode = static_cast<TreeFolderNode<NE> &>(parentNode);
         succeeded = parentFolderNode.insertChild(row, childNode);
      } else {
         static_assert(ParentNodeType::NodeClassifier == TreeNodeClassifier::PrimaryItem);
         // If parent is a Primary Item then child cannot be a folder -- ie must be primary or secondary item
         static_assert(std::is_same_v<ElementType, NE> || std::is_same_v<ElementType, SNE>);
         auto & parentItemNode = static_cast<TreeItemNode<NE> &>(parentNode);
         succeeded = parentItemNode.insertChild(row, childNode);
      }

      // Normally leave this debug statement commented out as otherwise it generates too much logging
//      qDebug() << Q_FUNC_INFO << "Insert" << (succeeded ? "succeeded" : "failed");

      // It's a coding error if the parent node into which we just inserted a child doesn't now have one more child than
      // before!
      Q_ASSERT(parentNode.childCount() == numChildrenBeforeInsert + 1);

      return true;
   }

   /**
    * \brief Use this instead of \c QAbstractItemModel::insertRow or \c QAbstractItemModel::insertRows to add nodes to
    *        the tree.
    *
    * \param parentIndex   The index of the parent tree node of the one we are creating.  (This parent owns the
    *                      newly-created tree node.)
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
      } else if constexpr (HasFolder<NE> && std::same_as<T, Folder<NE>>) {
         //
         // We are inserting a folder, so parent can only be another folder
         //
         Q_ASSERT(parentNode->classifier() == TreeNodeClassifier::Folder);
         return this->insertChild(static_cast<TreeFolderNode<NE> &>(*parentNode), parentIndex, row, element);
      } else {
         //
         // Remaining possibility is that we are inserting a primary item.  This means the parent could be a folder or,
         // in some trees, it could be another primary item (eg an ancestor Recipe in RecipeTreeModel).  To determine at
         // compile time whether the tree supports holding primary items inside other primary items, we look simply at
         // the number of alternatives in ParentPtrTypes.  This will be 1 in all trees that only allow primary items in
         // folders, and it will be 2 for trees that also allow primary items inside other primary items.  (There are no
         // other possibilities.)
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

      Q_UNREACHABLE(); // We should never get here
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

protected:
   void doElementAdded(int itemId) {
      std::shared_ptr<NE> item = ObjectStoreWrapper::getById<NE>(itemId);
      qDebug() << Q_FUNC_INFO << "Added" << *item;
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
   void doSecondaryElementAdded(int const elementId) requires (!IsVoid<SNE>) {
      auto element = ObjectStoreWrapper::getById<SNE>(elementId);
      if (element->deleted()) {
         return;
      }

      std::shared_ptr<NE> owner = element->owner();
      qDebug() << Q_FUNC_INFO << "Added" << *element << "with owner" << owner;

      if (!owner) {
         //
         // It is possible for a secondary element to get added to the DB before its "owner" is stored in the DB -- eg
         // see OwnedSet copy constructor.  We'll receive ObjectStoreTyped::signalObjectInserted, but we won't be able
         // to find the owner in the database.  This is OK: we just bail out here.  When the secondary element gets
         // added to its owner (ultimately via OwnedSet::extend), the owner will emit NamedEntity::changed and
         // NE::ownedItemsChanged.  We will pick up the latter and call doSecondaryElementsChanged.
         //
         return;
      }

      QModelIndex parentIndex = this->findElement(owner.get());
      if (!parentIndex.isValid()) {
         return;
      }

      if (int breadth = this->doRowCount(parentIndex);
          !this->insertChild(parentIndex, breadth, element)) {
         return;
      }

      this->observe(element);
      return;
   }

   /**
    * This is no-op for trees that don't support folders
    *
    * @param itemId
    */
   void doFolderAdded(int const folderId) {
      if constexpr (HasFolder<NE>) {
         auto folder = ObjectStoreWrapper::getByIdRaw<Folder<NE>>(folderId);
         qDebug() << Q_FUNC_INFO << "Added" << *folder;
         this->findFolder(folder, IfNotFound::Create);
      }
      return;
   }

   void doSecondaryElementsChanged(QObject * sender) {
      if constexpr (IsVoid<SNE>) {
         // It's a coding error if this ever gets called for a tree with no secondary elements!
         Q_ASSERT(false);
      } else if constexpr (!std::same_as<NE, Recipe>) {
         //
         // For Mash, Boil, Fermentation and StockPurchase subclasses, the logic is the same here
         //
         // For Recipe, things are a bit different and thus handled in RecipeTreeModel.
         //
         std::shared_ptr<NE> const owner = this->senderToPrimaryElement(sender);
         if (!owner) {
            return;
         }

         QModelIndex ownerIndex = this->findElement(owner.get());
         if (!ownerIndex.isValid()) {
            return;
         }

         auto ownerNode = static_cast<TreeItemNode<NE> *>(this->doTreeNode(ownerIndex));

         QList<TreeNode *> rawChildNodes = ownerNode->rawChildren();
         QList<std::shared_ptr<SNE>> children = owner->ownedItems();
//         qDebug() <<
//            Q_FUNC_INFO << "Num child nodes:" << rawChildNodes.size() << "; num children:" << children.size();

         //
         // We want to find:
         //    - child elements that are in the tree and should no longer be, so we can remove them
         //    - child elements that are not in the tree and should be, so we can add them
         //
         // It's probably overkill to use std::set here.  The number of secondary elements owned by any primary element
         // is typically just a handful, so nested loops would suffice.  However, using the standard library means we
         // don't have to roll our own algorithm, so, hopefully it's less error-prone.
         //
         std::set<int> currentlyInTree;
         for (TreeNode * rawChildNode : rawChildNodes) {
            if (rawChildNode->classifier() == TreeNodeClassifier::SecondaryItem) {
               //
               // Any child that is no longer in the DB should be removed.  We can't remove it with
               // doSecondaryElementRemoved because that's for items that are still in the DB.
               //
               int const key = rawChildNode->underlyingItemKey();
               if (key < 0) {
                  QModelIndex index = this->indexOfNode(rawChildNode);
                  this->removeItemByIndex(index);
               } else {
                  currentlyInTree.insert(key);
               }
            }
         }

//         qDebug() << Q_FUNC_INFO << "currentlyInTree:" << currentlyInTree;

         std::set<int> shouldBeInTree;
         for (auto const & child : children) {
            shouldBeInTree.insert(child->key());
         }

//         qDebug() << Q_FUNC_INFO << "shouldBeInTree:" << shouldBeInTree;

         std::vector<int> toBeCorrected;
         std::set_symmetric_difference(currentlyInTree.begin(), currentlyInTree.end(),
                                       shouldBeInTree.begin(),  shouldBeInTree.end(),
                                       std::back_inserter(toBeCorrected));

//         qDebug() << Q_FUNC_INFO << "toBeCorrected:" << toBeCorrected;

         //
         // Once we have the IDs, correcting the tree is easy as the hard work is already implemented elsewhere
         //
         for (int secondaryElementId : toBeCorrected) {
            if (currentlyInTree.contains(secondaryElementId)) {
               this->doSecondaryElementRemoved(secondaryElementId);
            } else {
               Q_ASSERT(shouldBeInTree.contains(secondaryElementId));
               this->doSecondaryElementAdded(secondaryElementId);
            }
         }
      }
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
      QModelIndex const index = this->findElement(element.get());
      if (!index.isValid()) {
         // This is probably a coding error, but we can recover
         qWarning() << Q_FUNC_INFO << "Could not find" << *element << "in the tree";
         return;
      }

      this->removeItemByIndex(index);
      return;
   }

public:

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
         // For a secondary item (eg BrewLog, MashStep), we don't delete the item itself because it is owned by a
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

      TreeNode * treeNode = this->doTreeNode(index);
      if (treeNode->classifier() == TreeNodeClassifier::PrimaryItem) {
         auto & neTreeNode = static_cast<TreeItemNode<NE> &>(*treeNode);
         std::shared_ptr<NE> neItem = neTreeNode.underlyingItem();
         this->unObserve(neItem);
      }

      if constexpr (!IsVoid<SNE>) {
         if (treeNode->classifier() == TreeNodeClassifier::SecondaryItem) {
            auto & sneTreeNode = static_cast<TreeItemNode<SNE> &>(*treeNode);
            std::shared_ptr<SNE> sneItem = sneTreeNode.underlyingItem();
            this->unObserve(sneItem);
         }
      }

      if constexpr (HasFolder<NE>) {
         if (treeNode->classifier() == TreeNodeClassifier::Folder) {
            auto & treeFolderNode = static_cast<TreeFolderNode<NE> &>(*treeNode);
            auto folderItem = treeFolderNode.underlyingItem();
            this->unObserve(folderItem);
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

   void doElementRemoved(int const elementId) {
      auto element = ObjectStoreWrapper::getById<NE>(elementId);
      this->implElementRemoved(element);
      return;
   }

   //! No-op version
   void doSecondaryElementRemoved([[maybe_unused]] int const elementId) requires (IsVoid<SNE>) {
      // It's a coding error if this ever gets called!
      Q_ASSERT(false);
      return;
   }
   //! Substantive version
   void doSecondaryElementRemoved(int const elementId) requires (!IsVoid<SNE>) {
      auto element = ObjectStoreWrapper::getById<SNE>(elementId);
      this->implElementRemoved(element);
      return;
   }

   void doFolderRemoved(int const folderId) {
      // We shouldn't get called on trees that don't support folders, but it's easier to just have a no-op in such cases
      // then remove the function calls etc.
      if constexpr (HasFolder<NE>) {
         auto folder = ObjectStoreWrapper::getById<Folder<NE>>(folderId);
         this->implElementRemoved(folder);
      }
      return;
   }

   /**
    * \brief
    *
    * \param selectedModelIndexes
    */
   void deleteItems(QModelIndexList const & selectedModelIndexes) {
      //
      // As explained below, we're going to delete objects one by one.  After the first deletion, none of the indexes in
      // selectedModelIndexes is guaranteed to still be valid.  So, before we do any deletion, we get a list of the
      // nodes whose objects we are deleting.
      //
      QList<TreeNode *> nodesToDelete;
      for (QModelIndex const & modelIndex : selectedModelIndexes) {
         nodesToDelete.append(this->doTreeNode(modelIndex));
      }
      this->deleteItems(nodesToDelete);
      return;
   }


   void deleteItems(QList<TreeNode *> const & nodesToDelete) {

      //
      // In the loop below, the calls to ObjectStoreWrapper::softDelete will, via the
      // ObjectStoreTyped::signalObjectDeleted signal, result in implElementRemoved being called, so we don't need to
      // remove the object from the tree here, other than in the case that it is a folder (and thus does not have an
      // entity in the DB).
      //
      for (TreeNode * treeNode : nodesToDelete) {
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

         // We want to delete the contents of the folder (and remove it from the model) before we remove the folder
         // itself, otherwise the QModelIndex values for the contents will not be valid.
         Q_ASSERT(treeNode->classifier() == TreeNodeClassifier::Folder);
         this->deleteItems(treeNode->rawChildren());
         //
         // For the moment, folders don't exist in the database, so we just remove directly from the tree
         //
         QModelIndex folderIndex = this->indexOfNode(treeNode);
         this->removeItemByIndex(folderIndex);
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
            // It's a coding error if we ask this function to copy either a folder or a secondary item (eg BrewLog or
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
   std::shared_ptr<NE> senderToPrimaryElement(QObject * sender) const {
      auto elementRaw = qobject_cast<NE *>(sender);
      if (!elementRaw) {
         return nullptr;
      }
      return ObjectStoreWrapper::getSharedFromRaw(elementRaw);
   }

   std::shared_ptr<SNE> senderToSecondaryElement(QObject * sender) const requires (!IsVoid<SNE>) {
      auto elementRaw = qobject_cast<SNE *>(sender);
      if (!elementRaw) {
         return nullptr;
      }
      return ObjectStoreWrapper::getSharedFromRaw(elementRaw);
   }

   std::shared_ptr<Folder<NE>> senderToFolder(QObject * sender) const {
      // Note that we can't use qobject_cast here as Folder<NE> does not have the Q_OBJECT Macro (because it is a
      // templated class).  Using dynamic_cast achieves the result we need, but is just slightly slower at runtime than
      // if we could use qobject_cast.
      auto folderRaw = dynamic_cast<Folder<NE> *>(sender);
      if (!folderRaw) {
         return nullptr;
      }
      return ObjectStoreWrapper::getSharedFromRaw(folderRaw);
   }

   /**
    * Called from doElementChanged, doSecondaryElementChanged and doFolderChanged below, as the logic is pretty much the
    * same for all three cases.
    *
    * @tparam ElementType
    * @param element
    * @param property
    * @param value
    */
   template<class ElementType>
   void implElementChanged(std::shared_ptr<ElementType> const & element,
                           QMetaProperty const & property,
                           [[maybe_unused]] QVariant const & value) {
      if (!element) {
         return;
      }

      QString const propertyName(property.name());
//      qDebug() << Q_FUNC_INFO << "Property name:" << propertyName;
      if constexpr (HasFolder<NE> && !std::is_same_v<ElementType, SNE>) {
         //
         // Note that, although there are also folder and folderRaw properties on FolderPropertyBase, it is always
         // folderId that we get notified about.
         //
         if (propertyName == PropertyNames::FolderPropertyBase::containedInFolderId) {
            qDebug() << Q_FUNC_INFO << "Folder changed on" << *element;
            this->movedFolder(element);
         }
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

protected:
   /**
    * Handles the NamedEntity::changed signal sent by primary elements
    * @param sender
    * @param property
    * @param value
    */
   void doElementChanged(QObject * sender, QMetaProperty const & property, QVariant const & value) {
      std::shared_ptr<NE> const element = this->senderToPrimaryElement(sender);
      this->implElementChanged(element, property, value);
      return;
   }

   /**
    * Handles the NamedEntity::changed signal sent by secondary elements
    * @param sender
    * @param property
    * @param value
    */
   void doSecondaryElementChanged(QObject * sender, QMetaProperty const & property, QVariant const & value) requires (!IsVoid<SNE>) {
      std::shared_ptr<SNE> const element = this->senderToSecondaryElement(sender);
      this->implElementChanged(element, property, value);
      return;
   }

   /**
    * Handles the NamedEntity::changed signal sent by folders
    * @param sender
    * @param property
    * @param value
    */
   void doFolderChanged(QObject * sender, QMetaProperty const & property, QVariant const & value) {
      // We don't need this function at all in trees that don't support folders but, for now, it's simpler to make it a
      // no-op in such cases.
      if constexpr (HasFolder<NE>) {
         std::shared_ptr<Folder<NE>> const folder = this->senderToFolder(sender);
         this->implElementChanged(folder, property, value);
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
         // In principle, the rules for adding subtrees are class-specific.  Eg for Recipes we need to take account of
         // Ancestors.
         //
         // In practice, it's mostly the case that Recipe is the special case, so we can put the generic logic here for
         // other cases.
         //
         // However, since the logic for MashTreeModel, BoilTreeModel, FermentationTreeModel is the same, we put it here
         // rather than either duplicate it in those three classes or make yet another base class.
         //
         if constexpr (!std::same_as<NE, Recipe>) {
            if (auto secondaries = SNE::ownedBy(element);
                !secondaries.empty()) {
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

private:
   /**
    * If a Primary Element or a Folder was moved to a different folder, then we need to call this function to update the
    * display.
    *
    * @tparam ElementType
    * @param element
    */
   template<class ElementType>
   void movedFolder(std::shared_ptr<ElementType> const & element) requires (HasFolder<NE>) {
      if (!element) {
         return;
      }
      qDebug() << Q_FUNC_INFO << *element;

      // Find the sending item in the existing tree
      QModelIndex const elementIndex = this->findElement(element.get());
      if (!elementIndex.isValid()) {
         qWarning() << Q_FUNC_INFO << "Could not find element" << *element;
         return;
      }

      //
      // Remove the sending item from its current parent folder, which will be the root node if it had no folder.  (In
      // that case, where parent is root node, parentIndex will be default, ie invalid, QModelIndex.  This is what
      // various bits of Qt code expect, so it's OK.)
      //
      QModelIndex const parentIndex = this->derived().parent(elementIndex);

      //
      // Remove the element from its old location.  Note that this removes the subtree of which the element is the root
      // (ie removes any children of the element and their children etc).
      //
      if (int const elementChildNumber = this->doTreeNode(elementIndex)->childNumber();
          !this->removeChildren(elementChildNumber, 1, parentIndex)) {
         qWarning() << Q_FUNC_INFO << "Could not remove row" << elementChildNumber;
         return;
      }

      //
      // Insert the element where it belongs in the tree
      //
      this->insert(element);

      return;
   }

   /**
    * Insert a primary element or folder in its rightful place in the tree
    *
    * @tparam ElementType
    * @param element
    */
   template<class ElementType>
   void insert(std::shared_ptr<ElementType> const & element) requires (HasFolder<NE>) {
      //
      // Find the element's folder.  Note that findFolder() will give us the root node if folder is null, so we don't
      // have to handle that here.  Similarly, we can ask it to create the folder if it (is non-empty and) does not
      // exist.
      //
      auto const folder = element->containedInFolderRaw();
      bool folderIsNewlyCreated;
      QModelIndex newParentIndex = this->findFolder(folder,
                                                    IfNotFound::Create,
                                                    &folderIsNewlyCreated);

      TreeNode * parentFolderNode = this->doTreeNode(newParentIndex);

      // Root node is TreeFolderNode<NE> so, regardless of whether the item has a folder, so this cast should always be
      // valid.
      if (int const numElementsInFolder = static_cast<TreeFolderNode<NE> *>(parentFolderNode)->childCount();
          !this->insertChild(newParentIndex, numElementsInFolder, element)) {
         qWarning() << Q_FUNC_INFO << "Could not insert row" << numElementsInFolder;
         return;
      }

      //
      // Now we inserted the element in the tree, we can get its node's index
      //
      QModelIndex const newElementIndex = this->findElement(element.get(), parentFolderNode);
      if (!newElementIndex.isValid()) {
         qWarning() << Q_FUNC_INFO << "Could not find element" << element << "after inserting!";
         return;
      }

      //
      // We now need to insert the element's children (if any).
      //
      if constexpr (!std::is_base_of_v<FolderCommon, ElementType>) {
         //
         // For a primary element (eg Recipe), addSubTreeIfNeeded() will handle both secondary element children (eg
         // BrewLogs) and primary element ones (eg ancestor Recipes).
         //
         auto elementNode = static_cast<TreeItemNode<ElementType> *>(this->doTreeNode(newElementIndex));
         this->addSubTreeIfNeeded(*element, *elementNode);
      } else {
         //
         // For a folder, we need to add any primary element children and any folder children, but we can rely on
         // recursion to handle other descendents (ie children's children etc).
         //
         // TBD: At the moment, this is inserting folders after any existing content.  I think we just need to tell
         //      the filter to refresh.
         //
         for (auto const & primaryChild : element->childItems()) {
            this->insert(primaryChild);
         }
         for (auto const & folderChild : element->childFolders()) {
            this->insert(folderChild);
         }
      }

      if (folderIsNewlyCreated) {
         emit this->derived().expandFolder(newParentIndex);
      }

      return;
   }

protected:

   //================================================ Member Variables =================================================
   //
   // TBD: It's slightly ugly that we need to have TreeFolderNode even for trees that don't support folders
   //
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
      void secondaryElementAdded  (int elementId);    \
      void secondaryElementRemoved(int elementId);    \
      void secondaryElementChanged(QMetaProperty property, QVariant value); \

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
      void elementChanged(QMetaProperty property, QVariant value);                          \
      TREE_MODEL_COMMON_DECL_SNE(NeName __VA_OPT__(,) __VA_ARGS__)                          \
                                                                                            \
      /* Strictly speaking, we do not want these functions in trees that do not support */  \
      /* folders.  However, to avoid even more macro jiggery pokery, we let them exist */   \
      /* as no-ops in such cases. */                                                        \
      void folderAdded  (int folderId);                                                     \
      void folderRemoved(int folderId);                                                     \
      void folderChanged(QMetaProperty property, QVariant value);                           \
                                                                                            \
      void secondaryElementsChanged();                                                      \


//
// These macros are used by TREE_MODEL_COMMON_CODE (see below) to handle its optional second parameter
//
#define TREE_MODEL_COMMON_CODE_SNE_1(NeName)
#define TREE_MODEL_COMMON_CODE_SNE_2(NeName, SneName) \
   void NeName##TreeModel::secondaryElementAdded  (int elementId) { this->doSecondaryElementAdded  (elementId)     ; return; } \
   void NeName##TreeModel::secondaryElementRemoved(int elementId) { this->doSecondaryElementRemoved(elementId)     ; return; } \
   void NeName##TreeModel::secondaryElementChanged(QMetaProperty property, QVariant value) { \
      this->doSecondaryElementChanged(this->sender(), property, value); return; \
   } \

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
   void NeName##TreeModel::elementAdded  (int const elementId) { this->doElementAdded  (elementId)     ; return; } \
   void NeName##TreeModel::elementRemoved(int const elementId) { this->doElementRemoved(elementId)     ; return; } \
   void NeName##TreeModel::elementChanged(QMetaProperty property, QVariant value) { \
      this->doElementChanged(this->sender(), property, value);                      \
      return;                                                                       \
   }                                                                                \
   TREE_MODEL_COMMON_CODE_SNE(NeName __VA_OPT__(,) __VA_ARGS__)                                           \
   void NeName##TreeModel::folderAdded  (int const folderId) { this->doFolderAdded  (folderId); return; } \
   void NeName##TreeModel::folderRemoved(int const folderId) { this->doFolderRemoved(folderId); return; } \
   void NeName##TreeModel::folderChanged(QMetaProperty property, QVariant value) { \
      this->doFolderChanged (this->sender(), property, value);                     \
      return;                                                                      \
   }                                                                               \
   void NeName##TreeModel::secondaryElementsChanged() { this->doSecondaryElementsChanged(this->sender()); return; } \

#endif