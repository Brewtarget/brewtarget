/*======================================================================================================================
 * trees/TreeNodeBase.h is part of Brewtarget, and is copyright the following authors 2024-2025:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef TREES_TREENODEBASE_H
#define TREES_TREENODEBASE_H
#pragma once

#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

#include "trees/TreeNode.h"
#include "trees/TreeNodeTraits.h"
#include "utils/ColumnInfo.h"
#include "utils/ColumnOwnerTraits.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/PropertyHelper.h"

/**
 * \class TreeNodeBase Curiously Recurring Template Base for NewTreeNode subclasses
 *
 *        NOTE: This is still mostly an idea at the moment - would require a rework of TreeView and TreeModel to be
 *              useful.  For now we just use ColumnIndex and Info.
 *
 *        Class structure:
 *        ----------------
 *                                           TreeNode
 *                                              |
 *                                         TreeNodeBase
 *                                          /        \ .
 *                             TreeFolderNode<NE>    TreeItemNode<NE>
 *
 *        Note that we have a simpler structure here than in a lot of places where we use CRTP.  This is because these
 *        classes do not need to inherit from QObject, so we don't need to jump around to ensure the Q_OBJECT pseudo
 *        macro works etc.
 *
 *        Tree structure:
 *        ---------------
 *
 *           TreeModel<Recipe>
 *             │
 *           TreeFolderNode<Recipe>
 *             ├── TreeFolderNode<Recipe>
 *             │   ├── TreeItemNode<Recipe>
 *             │   │   └── TreeItemNode<BrewNote>
 *             │   └── TreeItemNode<Recipe>
 *             ├── TreeFolderNode<Recipe>
 *             │   └── TreeItemNode<Recipe>
 *             ├── TreeItemNode<Recipe>
 *             └── TreeItemNode<Recipe>
 *
 *        A folder node in a Hop tree can contain only hop nodes or other Hop Folder nodes.  A Hop node cannot contain
 *        other nodes.
 *
 *        In a Recipe tree it's a bit more complicated:
 *           - A Folder node can contain only Recipe nodes or other Recipe Folder nodes
 *           - A Recipe node can contain only BrewNote nodes or Recipe nodes (when using ancestor versioning)
 *           - A BrewNote node cannot contain other nodes
 *
 *        So, in general, depending on the type of node, it can contain:
 *           - No other nodes
 *           - Nodes of one other type
 *           - Nodes of its own type and nodes of one other type
 *
 *        This means, depending on the type of node, its parent can be:
 *           - A node of its own type
 *           - A node of one other type
 *           - Either of the above
 *
 *        And, similarly, in a given tree, there are either two or three types of node:
 *           - Folders
 *           - Primary item - eg Recipe - which is also the type of the tree
 *           - Secondary item - eg BrewNote in the Recipe tree, but not present in the Hop tree
 *        This is a helpful classification for code that is traversing or manipulating the tree, so we have an enum for
 *        it: NodeClassifier
 *
 */
template<class Derived> class TreeNodeBasePhantom;
template<class Derived, class NE, class TreeType>
class TreeNodeBase : public TreeNode, public CuriouslyRecurringTemplateBase<TreeNodeBasePhantom, Derived> {
public:
   using ColumnIndex        = typename TreeNodeTraits<NE, TreeType>::ColumnIndex;
   static constexpr TreeNodeClassifier NodeClassifier  = TreeNodeTraits<NE, TreeType>::NodeClassifier;
   using ParentPtrTypes     = typename TreeNodeTraits<NE, TreeType>::ParentPtrTypes;
   using ChildPtrTypes      = typename TreeNodeTraits<NE, TreeType>::ChildPtrTypes;
   static constexpr char const *       DragNDropMimeType = TreeNodeTraits<NE, TreeType>::DragNDropMimeType;

   TreeNodeBase(TreeModel & model,
                ParentPtrTypes parent = nullptr,
                std::shared_ptr<NE> underlyingItem = nullptr) :
      TreeNode{model},
      m_parent{parent},
      m_underlyingItem{underlyingItem} {
      return;
   }
   TreeNodeBase(TreeModel & model,
                TreeNode * parent,
                std::shared_ptr<NE> underlyingItem) :
      TreeNode{model},
      m_parent{
         [parent]() -> ParentPtrTypes {
            //
            // Because we've made everything strongly typed (yay), there are some things we _have_ to do at compile time
            // to avoid asking the compiler to generate meaningless code.
            //
            if constexpr (NodeClassifier == TreeNodeClassifier::Folder) {
               // Folder can only have folder as parent
               return static_cast<TreeFolderNode<TreeType> *>(parent);
            } else if constexpr (NodeClassifier == TreeNodeClassifier::PrimaryItem) {
               if constexpr (std::variant_size_v<ParentPtrTypes> == 1) {
                  // If there's only one possibility for parent type, then it will be folder
                  return static_cast<TreeFolderNode<TreeType> *>(parent);
               } else {
                  //
                  // This is the only case where we have to decide at run-time -- ie where a primary item could have either
                  // a folder or another primary item as parent.  At the moment, it's only needed in the Recipe tree (to
                  // handle Recipe versioning).
                  //
                  if (!parent || parent->classifier() == TreeNodeClassifier::Folder) {
                     return static_cast<TreeFolderNode<TreeType> *>(parent);
                  }
                  return static_cast<TreeItemNode<TreeType> *>(parent);
               }
            } else {
               static_assert(NodeClassifier == TreeNodeClassifier::SecondaryItem);
               // Secondary Item (eg BrewNote) can only have primary item (eg Recipe) as parent
               return static_cast<TreeItemNode<TreeType> *>(parent);
            }
         }()
      },
      m_underlyingItem{underlyingItem} {
      return;
   }
   virtual ~TreeNodeBase() = default;

   virtual TreeNodeClassifier classifier() const override {
      return NodeClassifier;
   }

   /**
    * \brief Similar to \c TreeModelBase::get_ColumnInfo
    */
   ColumnInfo const & get_ColumnInfo(ColumnIndex const columnIndex) const {
      return ColumnOwnerTraits<Derived>::getColumnInfo(static_cast<size_t>(columnIndex));
   }

   QVariant readDataFromModel(ColumnInfo const & columnInfo, int const role) const {
      QVariant modelData = columnInfo.propertyPath.getValue(*this->m_underlyingItem);
      if (!modelData.isValid()) {
         //
         // You might think it's a programming error if we couldn't read a property modelData, but there are
         // circumstances where this is expected -- eg reading "style/name" from a Recipe that does not have style set.
         //
         qWarning() <<
            Q_FUNC_INFO <<
               "Unable to read" << this->derived() << "property" << columnInfo.propertyPath << "(Got" << modelData <<
               ")";
         return QVariant{};
      }

      TypeInfo const & typeInfo = columnInfo.typeInfo;

      // Uncomment this log statement if asserts in PropertyHelper::readDataFromPropertyValue are firing
//      qDebug() <<
//         Q_FUNC_INFO << columnInfo.columnFqName << ", propertyPath:" << columnInfo.propertyPath << "TypeInfo:" <<
//         typeInfo << ", modelData:" << modelData;

      return PropertyHelper::readDataFromPropertyValue(modelData,
                                                       typeInfo,
                                                       role,
                                                       columnInfo.extras.has_value(),
                                                       columnInfo.getForcedSystemOfMeasurement(),
                                                       columnInfo.getForcedRelativeScale());
   }

   /**
    * \brief Similar to \c TreeModelBase::readDataFromModel
    */
   QVariant readDataFromModel(ColumnIndex const columnIndex, int const role) const {

      ColumnInfo const & columnInfo = this->get_ColumnInfo(columnIndex);
      return this->readDataFromModel(columnInfo, role);

   }

   /**
    *
    */
   bool columnIsLessThan(Derived const & other, ColumnIndex const columnIndex) const {
      ColumnInfo const & columnInfo = this->get_ColumnInfo(columnIndex);
      QVariant  leftItem = this->readDataFromModel(columnInfo, Qt::UserRole);
      QVariant rightItem = other.readDataFromModel(columnInfo, Qt::UserRole);

      return PropertyHelper::isLessThan(leftItem, rightItem, columnInfo.typeInfo);
   }

   virtual QVariant data(int const column, int const role) const override {
      if (column < 0 || column >= ColumnOwnerTraits<Derived>::numColumns()) {
         return QVariant{};
      }

      // Check above means this cast is valid
      auto const columnIndex = static_cast<ColumnIndex>(column);
      ColumnInfo const & columnInfo = this->get_ColumnInfo(columnIndex);

      switch (role) {
         case Qt::ToolTipRole:
            if (this->m_underlyingItem) {
               if constexpr (NodeClassifier == TreeNodeClassifier::Folder) {
                  // Tooltip for folders is just the name of the tree - eg "Recipes" for the Recipe tree
                  // We only ever care about this for primary items where, by definition, the tree type is the same as
                  // the primary item type.
                  return QVariant(TreeNodeTraits<TreeType, TreeType>::getRootName());
               } else {
                  return this->derived().getToolTip();
               }
            }
            break;

         case Qt::DisplayRole:
            if (this->m_underlyingItem) {
               return this->readDataFromModel(columnInfo, Qt::DisplayRole);
            }
            // Special handling for the root node
            if (!this->rawParent()) {
               // For the root node, we display the name of the tree in the first column
               // Root node is always a folder
               if constexpr (NodeClassifier == TreeNodeClassifier::Folder) {
                  if (columnIndex == ColumnIndex::Name) {
                     return QVariant(TreeNodeTraits<TreeType, TreeType>::getRootName());
                  }
               }
               return QVariant{};
            }
            break;

         case Qt::DecorationRole:
            if (column == 0 && NodeClassifier == TreeNodeClassifier::Folder) {
               return QIcon(":images/folder.png");
            }
            break;

         default:
            break;
      }

      return QVariant();
   }

   static QVariant header(size_t const section) {
      auto const numColumns = static_cast<size_t>(ColumnOwnerTraits<Derived>::numColumns());
      if (section >= numColumns) {
         return QVariant();
      }
      return ColumnOwnerTraits<Derived>::getColumnLabel(section);
   }

   /**
    * \brief returns item's parent
    */
   ParentPtrTypes parent() const {
      return this->m_parent;
   }

   virtual TreeNode * rawParent() const override {
      // Every substantive member of ParentPtrTypes is always a raw pointer to some subclass of TreeNode, so this
      // generic lambda suffices to obtain whatever member the variant holds.
      auto theParent{this->parent()};
      return std::visit([](auto&& arg) { return static_cast<TreeNode *>(arg); }, theParent);
   }

   std::shared_ptr<NE> underlyingItem() const {
      return this->m_underlyingItem;
   }

   virtual NamedEntity * rawUnderlyingItem() const override {
      if constexpr (std::same_as<NE, Folder>) {
         // This function is not currently supported on Folder nodes, for the simple reason that Folder does not (yet)
         // inherit from NamedEntity.
         Q_ASSERT(false);
         return nullptr;
      } else {
         return this->m_underlyingItem.get();
      }
   }

   void setUnderlyingItem(std::shared_ptr<NE> val) {
      this->m_underlyingItem = val;
      return;
   }

   /**
    * \brief inserts a new item at \c position.  Note that it is the caller's responsibility to call
    *        \c QAbstractItemModel::beginInsertRows etc (via \c TreeModelChangeGuard).
    *
    *        TBD: We could probably simplify the code in a number of places by always adding a new child at the end of
    *             our list (this->m_children).  The order of child nodes in the TreeModel classes is somewhat irrelevant
    *             because they get sorted by the TreeSortFilterProxyModel classes.
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool insertChild(std::size_t position, ChildPtrTypes child) requires IsSubstantiveVariant<ChildPtrTypes> {
      if (position > this->m_children.size()) {
         // This is probably a coding error, but we can probably recover by just not doing the insert
         qWarning() << Q_FUNC_INFO << "Position" << position << "outside range (0, " << this->m_children.size() << ")";
         return false;
      }

      this->m_children.insert(this->m_children.begin() + position, child);

      return true;
   }

   /**
    * \brief Removes \c count items starting at \c position.  NB: This just removes the nodes from the tree structure;
    *        it does not delete the contents of the nodes (m_underlyingItem).  Similarly, it is not recursive, so it is
    *        the caller's responsibility to do any processing of children's children etc.
    *
    *        Note that it is the caller's responsibility to call \c QAbstractItemModel::beginRemoveRows etc (via
    *        \c TreeModelChangeGuard).
    *
    * \return \c true if succeeded, \c false otherwise
    */
   virtual bool removeChildren(int position, int count) override {
      if constexpr (IsSubstantiveVariant<ChildPtrTypes>) {
         if (position < 0  || position > static_cast<int>(this->m_children.size())) {
            // This is probably a coding error, but we can probably recover by just not doing the remove
            qWarning() <<
               Q_FUNC_INFO << "Position" << position << "outside range (0, " << this->m_children.size() << ")";
            return false;
         }

         // The range for erase is inclusive of the first element, and exclusive of the last, so the second parameter is
         // one beyond where we want to erase (and can legitimately be cend()).
         this->m_children.erase(this->m_children.cbegin() + position, this->m_children.cbegin() + position + count);
         return true;
      }

      // Shouldn't ever get here
      qWarning() << Q_FUNC_INFO << "This function should not be called on nodes that cannot have children!";
      return false;
   }

   //! \brief returns the number of children of the folder (or recipe)
   virtual int childCount() const override {
      if constexpr (IsSubstantiveVariant<ChildPtrTypes>) {
         return this->m_children.size();
      } else {
         return 0;
      }
   }

   /**
    * \brief Return specified child.
    *
    *        TODO: One day it would be neat to have an iterator for looping over children etc.
    */
   ChildPtrTypes child(int number) const {
      if constexpr (IsSubstantiveVariant<ChildPtrTypes>) {
         Q_ASSERT(number < static_cast<int>(this->m_children.size()));
         return this->m_children.at(number);
      } else {
         // For the moment, it is simpler to allow calls to this function even when there can be no children.  We just
         // always return the null variant in such cases.
         return std::variant<std::monostate>{};
      }
   }

   virtual TreeNode * rawChild(int number) const override {
      // If this node type does not support children, there are never any to return.  (It is in fact unlikely we'd get
      // called in such circumstances, but we can't have a requires clause on a virtual function, so it's easier just to
      // cover the case here.)
      if constexpr (IsNullVariant<ChildPtrTypes>) {
         return nullptr;
      } else {
         // Every substantive member of ChildPtrTypes is always a shared pointer to some subclass of TreeNode, so this
         // generic lambda suffices to give us a raw pointer that can be cast to TreeNode *.
         auto theChild{this->child(number)};
         return std::visit([](auto&& arg) { return static_cast<TreeNode *>(arg.get()); }, theChild);
      }
   }

   virtual QList<TreeNode *> rawChildren() const override {
      QList<TreeNode *> rawChildren;
      if constexpr (!IsNullVariant<ChildPtrTypes>) {
         for (ChildPtrTypes const & childPtr : this->m_children) {
            rawChildren.append(std::visit([](auto&& arg) { return static_cast<TreeNode *>(arg.get()); }, childPtr));
         }
      }
      return rawChildren;
   }

   /**
    * \brief Return a raw pointer to specified child, suitable for call to \c QAbstractItemModel::createIndex
    */
   void const * voidChild(std::size_t number) const requires IsSubstantiveVariant<ChildPtrTypes> {
      if (number < this->m_children.size()) {
         auto const & theChild = this->m_children.at(number);
         return std::visit([](auto visited){ return static_cast<void *>(visited.get()); }, theChild);
      }
      return nullptr;
   }

   /**
    * \brief If the supplied parameter is a pointer to one of the children of this node, then return the number of
    *        that child in our list.  Otherwise, return -1.
    */
   virtual int numberOfChild(TreeNode const * childToCheck) const override {
      // Comment from rawChild above applies equally here
      if constexpr (IsNullVariant<ChildPtrTypes>) {
         qCritical() << Q_FUNC_INFO << "TreeNode::numberOfChild() called on node type that has no children!";
         Q_ASSERT(false);
      } else {
         for (int childNumber = 0; childNumber < static_cast<int>(this->m_children.size()); ++childNumber) {
            auto const & currentChild = this->m_children.at(childNumber);
            if (std::visit([&](auto visited){ return (visited.get() == childToCheck); }, currentChild)) {
               return childNumber;
            }
         }
      }

      // Usually it's a coding error if we get here
      qCritical() << Q_FUNC_INFO << "Unable to find child";
      qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      return -1;
   }

   /**
    * \brief returns the index of the item in its parent's list.  This is needed for constructing \c QModelIndex
    *        objects.
    */
   virtual int childNumber() const override {
      TreeNode * rawParent = this->rawParent();
      if (!rawParent) {
         return 0;
      }

      return rawParent->numberOfChild(this);
   }

   virtual QString className() const override {
      return NE::staticMetaObject.className();
   }

   virtual QString localisedClassName() const override {
      return NE::localisedName();
   }

   virtual QString name() const override {
      if (!this->m_underlyingItem) {
         return QObject::tr("None!");
      }
      return this->m_underlyingItem->name();
   }

   virtual int underlyingItemKey() const override {
      //
      // For the moment, Folders don't have IDs, so return 0
      //
      if constexpr (NodeClassifier == TreeNodeClassifier::Folder) {
         return 0;
      } else {
         // We need this code inside the else so that the compiler doesn't try to call m_underlyingItem->key() on Folder
         if (!this->m_underlyingItem) {
            //
            // I don't think we ever have things in the tree that aren't in the DB (ie with ID -1), but we might as well
            // return a different negative number for "null pointer" (which should also be rare-to-never).
            //
            return -2;
         }
         return this->m_underlyingItem->key();
      }
   }

   virtual QString dragAndDropMimeType() const override {
      return QString{DragNDropMimeType};
   }

   //================================================ Member variables =================================================
   ParentPtrTypes m_parent;
   //
   // Although it's easy to have conditional member functions (ie ones that only exist when certain template constraints
   // are satisfied), there isn't (yet) a straightforward way to do the equivalent for member variables.  About as close
   // as we can get is using [[no_unique_address]] here, which allows the compiler to optimise away the variable storage
   // when it's an empty class type
   //
   struct Empty { };
   [[no_unique_address]] std::conditional_t<IsSubstantiveVariant<ChildPtrTypes>,
                                            std::vector<ChildPtrTypes>,
                                            Empty>  m_children;
   //
   // The underlying item stored in this tree node -- eg the Recipe object stored in a particular TreeItemNode<Recipe>
   // object.
   //
   std::shared_ptr<NE> m_underlyingItem = nullptr;
};

//=================================================== TreeFolderNode ===================================================

/**
 * \brief Besides other folders of the same type, a given type of folders can only only contain one type of thing (eg
 *        FermentableTreeItem, HopTreeItem, etc).
 */
template<class NE>
class TreeFolderNode : public TreeNodeBase<TreeFolderNode<NE>, Folder, NE> {
public:
   using TreeNodeBase<TreeFolderNode<NE>, Folder, NE>::TreeNodeBase;
   ~TreeFolderNode() = default;

   // Have to override the version in \c TreeNodeBase as that will give Folder::staticMetaObject.className() rather
   // than NE::staticMetaObject.className()
   virtual QString className() const override {
      return NE::staticMetaObject.className();
   }

   virtual std::shared_ptr<Folder> folder() const override {
      return this->underlyingItem();
   }
};

//==================================================== TreeItemNode ====================================================

template<class NE>
class TreeItemNode : public TreeNodeBase<TreeItemNode<NE>, NE, typename TreeTypeDeducer<NE>::TreeType> {
public:
   using TreeNodeBase<TreeItemNode<NE>, NE, typename TreeTypeDeducer<NE>::TreeType>::TreeNodeBase;
   virtual ~TreeItemNode() = default;

   QString getToolTip() const;

   virtual std::shared_ptr<Folder> folder() const override {

      if constexpr (TreeNodeTraits<NE, typename TreeTypeDeducer<NE>::TreeType>::NodeClassifier == TreeNodeClassifier::PrimaryItem) {
         if constexpr (HasNoFolder<NE>) {
            //
            // For elements that don't support folders (eg Inventory) we just want the root folder, which will also be
            // the parent node.
            //
            return this->rawParent()->folder();
         } else {
            //
            // It's quicker to get the folder directly than chase up the node tree to try to find a TreeFolderNode.
            //
            // TODO: This is a temporary hack to return a Folder object!
            return std::make_shared<Folder>(this->underlyingItem()->folderPath());
         }


      } else {
         //
         // For a SecondaryItem node, it must, by definition, have a parent node, so we just defer to that.
         //
         static_assert (TreeNodeTraits<NE, typename TreeTypeDeducer<NE>::TreeType>::NodeClassifier == TreeNodeClassifier::SecondaryItem);
         return this->rawParent()->folder();
      }
   }

};

//
// Check the concepts we use above are working as we intend
//
static_assert(IsSubstantiveVariant<TreeFolderNode<Equipment>::ChildPtrTypes>);
static_assert(IsSubstantiveVariant<TreeFolderNode<Style>::ChildPtrTypes>);
static_assert(IsNullVariant<TreeItemNode<Equipment>::ChildPtrTypes>);


//
// All folders have the same columns, so we do a partial specialisation here.
//
template<class NE>
struct ColumnOwnerTraitsData<TreeFolderNode<NE>> {
   static std::vector<ColumnInfo> const & getColumnInfos() {
      // Meyers singleton
      static std::vector<ColumnInfo> const columnInfos {
         TREE_NODE_HEADER(TreeFolderNode, Folder, Name    , tr("Name"     ), PropertyNames::NamedEntity::name),
         TREE_NODE_HEADER(TreeFolderNode, Folder, Path    , tr("Path"     ), PropertyNames::Folder::path    ),
         TREE_NODE_HEADER(TreeFolderNode, Folder, FullPath, tr("Full Path"), PropertyNames::Folder::fullPath),
      };
      return columnInfos;
   }
};


#endif
