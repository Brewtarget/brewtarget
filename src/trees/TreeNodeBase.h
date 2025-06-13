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

#include "trees/TreeNode.h"
#include "trees/TreeNodeTraits.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

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
   // We _could_ use size_t for NumberOfColumns, since it's obviously never negative.  However, various Qt functions for
   // column number use int (and -1 means "invalid"), so we can spare ourselves compiler warnings about comparing signed
   // and unsigned types by sticking to int ourselves.
   static constexpr int                NumberOfColumns = TreeNodeTraits<NE, TreeType>::NumberOfColumns;
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

   virtual QVariant data(int const column, int const role) const override {
      if (column < 0 || column >= NumberOfColumns) {
         return QVariant{};
      }

      // Check above means this cast is valid
      auto const typedColumn = static_cast<ColumnIndex>(column);

      switch (role) {
         case Qt::ToolTipRole:
            if (this->m_underlyingItem) {
               if constexpr (NodeClassifier == TreeNodeClassifier::Folder) {
                  // Tooltip for folders is just the name of the tree - eg "Recipes" for the Recipe tree
                  return QVariant(TreeNodeTraits<TreeType>::getRootName());
               } else {
                  return this->derived().getToolTip();
               }
            }
            break;

         case Qt::DisplayRole:
            if (this->m_underlyingItem) {
               return TreeNodeTraits<NE, TreeType>::data(*this->m_underlyingItem, typedColumn);
            }
            // Special handling for the root node
            if (!this->rawParent()) {
               // For the root node, we display the name of the tree in the first column
               // Root node is always a folder
               if constexpr (NodeClassifier == TreeNodeClassifier::Folder) {
                  if (typedColumn == ColumnIndex::Name) {
                     return QVariant(TreeNodeTraits<TreeType>::getRootName());
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
      if (/*section < 0 || */section >= NumberOfColumns) {
         return QVariant();
      }
      return QVariant(Derived::columnDisplayNames[section]);
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

   static EnumStringMapping const columnDisplayNames;
   bool columnIsLessThan(TreeFolderNode<NE> const & other, TreeNodeTraits<Folder, NE>::ColumnIndex column) const {
      auto const & lhs = *this->m_underlyingItem;
      auto const & rhs = *other.m_underlyingItem;
      switch (column) {
         case TreeNodeTraits<Folder, NE>::ColumnIndex::Name     : return lhs.name    () < rhs.name    ();
         case TreeNodeTraits<Folder, NE>::ColumnIndex::Path     : return lhs.path    () < rhs.path    ();
         case TreeNodeTraits<Folder, NE>::ColumnIndex::FullPath : return lhs.fullPath() < rhs.fullPath();
      }
//      std::unreachable();
   }

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

   static EnumStringMapping const columnDisplayNames;
   bool columnIsLessThan(TreeItemNode<NE> const & other,
                         TreeNodeTraits<NE, typename TreeTypeDeducer<NE>::TreeType>::ColumnIndex column) const;

   QString getToolTip() const;

   virtual std::shared_ptr<Folder> folder() const override {

      if constexpr (TreeNodeTraits<NE, typename TreeTypeDeducer<NE>::TreeType>::NodeClassifier == TreeNodeClassifier::PrimaryItem) {
         //
         // We are assuming here that all PrimaryItem nodes hold subclasses of NamedEntity that also inherit from
         // FolderBase.  This saves us chasing up the node tree to try to find a TreeFolderNode.
         //
         // TODO: This is a temporary hack to return a Folder object!
         return std::make_shared<Folder>(this->underlyingItem()->folderPath());
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

#endif
