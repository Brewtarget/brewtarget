/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeNode.h is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#ifndef TREES_TREENODE_H
#define TREES_TREENODE_H
#pragma once

#include <QList>
#include <QObject>
#include <QModelIndex>
#include <QVariant>

#include "Localization.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Folder.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/NamedEntity.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/EnumStringMapping.h"

class TreeModel;

template<class NE> class TreeFolderNode;

/**
 * \brief See comment in qtModels/tableModels/TableModelBase.h for why we use a traits class to allow the following attributes
 *        from each \c Derived class to be accessible in \c TreeNodeBase:
 *           - \c ColumnIndex        = class enum for the columns of this node type
 *           - \c Info               = class enum holding just NumberOfColumns = number of entries in the above
 *           - \c ParentType         = type of the parent node
 *           - \c ChildPtrTypes      = std::variant of unique_ptrs to valid child types (or
 *                                     std::variant<std::monostate> if no children are allowed).
 */
template<class Derived>
struct TreeItemTraits;

/**
 * \class TreeNodeBase Curiously Recurring Template Base for NewTreeNode subclasses
 *
 *        NOTE: This is still mostly an idea at the moment - would require a rework of TreeView and TreeModel to be
 *              useful.  For now we just use ColumnIndex and Info.
 *
 *        Class structure
 *                                         TreeNodeBase
 *                                          /        \ .
 *                             TreeFolderNode<NE>    TreeItemNode<NE>
 *
 *        Tree structure:
 *
 *           TreeModel
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
 */
template<class Derived, class NE>
class TreeNodeBase : public CuriouslyRecurringTemplateBase<TreeItemTraits, Derived> {
public:
   using ColumnIndex        = typename TreeItemTraits<Derived>::ColumnIndex;
   using Info               = typename TreeItemTraits<Derived>::Info;
   using ParentType         = typename TreeItemTraits<Derived>::ParentType;
   using ChildPtrTypes      = typename TreeItemTraits<Derived>::ChildPtrTypes;

   TreeNodeBase(ParentType * parent = nullptr) :
      m_parent{parent} {
      return;
   }
   ~TreeNodeBase() = default;

   static QVariant header(int section) {
      if (section < 0 || section >= static_cast<int>(Info::NumberOfColumns)) {
         return QVariant();
      }
      return QVariant(Derived::columnDisplayNames[section]);
   }

   static bool lessThan(TreeModel const & model,
                        QModelIndex const & left,
                        QModelIndex const & right,
                        NE const & lhs,
                        NE const & rhs) {
      return Derived::isLessThan(model, left, right, static_cast<ColumnIndex>(left.column()), lhs, rhs);
   }

   //! \brief flag this node to override display() or not
   void setShowMe(bool val) {
      this->m_showMe = val;
      return;
   }

   //! \brief does the node want to be shown regardless of display()
   bool showMe() const {
      return this->m_showMe;
   }

   /**
    * \brief returns item's parent
    */
   ParentType * parent() const {
      return this->m_parent;
   }

   std::shared_ptr<NE> modelItem() const {
      return this->m_modelItem;
   }

   void setModelItem(std::shared_ptr<NE> val) {
      this->m_modelItem = val;
      return;
   }

   /**
    * \return Name of the item stored at this node (or "" if there is none).
    */
   QString name() const {
      if (this->m_modelItem) {
         return this->m_modelItem->name();
      }
      return "";
   }

   /**
    * \brief inserts \c count new items starting at \c position
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool insertChildren(std::size_t position, int count) requires IsSubstantiveVariant<ChildPtrTypes> {
      if (position > this->m_children.size()) {
         // This is probably a coding error, but we can probably recover by just not doing the insert
         qWarning() << Q_FUNC_INFO << "Position" << position << "outside range (0, " << this->m_children.size() << ")";
         return false;
      }

      for (int row = 0; row < count; ++row) {
         this->m_children.emplace(this->m_children.begin() + position + row);
      }

      return true;
   }

   bool insertChildren(std::size_t position, int count) requires IsNullVariant<ChildPtrTypes> {
      qWarning() << Q_FUNC_INFO << "Should not be called!";
      return false;
   }

   /**
    * \brief removes \c count items starting at \c position
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool removeChildren(int position, int count) requires IsSubstantiveVariant<ChildPtrTypes> {
      if (position < 0  || position > this->m_children.size()) {
         // This is probably a coding error, but we can probably recover by just not doing the remove
         qWarning() << Q_FUNC_INFO << "Position" << position << "outside range (0, " << this->m_children.size() << ")";
         return false;
      }

      // The range for erase is inclusive of the first element, and exclusive of the last, so the second parameter is
      // one beyond where we want to erase (and can legitimately be cend()).
      this->m_children.erase(this->m_children.cbegin() + position, this->m_children.cbegin() + position + count);
      return true;
   }

   bool removeChildren(int position, int count) requires IsNullVariant<ChildPtrTypes> {
      qWarning() << Q_FUNC_INFO << "Should not be called!";
      return false;
   }

   //! \brief returns the number of children of the folder
   int childCount() const requires IsSubstantiveVariant<ChildPtrTypes> {
      return m_children.size();
   }

   int childCount() const requires IsNullVariant<ChildPtrTypes> {
      return 0;
   }

   /**
    * \brief Return specified child.
    */
   ChildPtrTypes child(int number) const requires IsSubstantiveVariant<ChildPtrTypes> {
      return this->m_children.at(number);
   }

   //! \brief returns the index of the item in its parents list
   int childNumber() const {
      if (!this->m_parent) {
         return 0;
      }

      return this->m_parent->childNumber(this);
   }

   //================================================ Member variables =================================================
   ParentType * m_parent;
   bool m_showMe = true;
   // [[no_unique_address]] Here allows the compiler to optimise away the variable storage when it's an empty class type
   struct Empty { };
   [[no_unique_address]] std::conditional_t<IsSubstantiveVariant<ChildPtrTypes>,
                                            std::vector<ChildPtrTypes>,
                                            Empty>  m_children;
   std::shared_ptr<NE> m_modelItem = nullptr;
};

// This can't be a member function otherwise we end up with circular definitions
template<class TN>
int childNumber(TN const & tn) {
   if (tn.parent()) {
      return tn.parent()->childNumber(&tn);
   }
   return 0;
}


////////////// TreeFolderNode

template<class NE> class TreeFolderNode;
template<class NE> class TreeItemNode;

template <class NE> struct TreeItemTraits<TreeFolderNode<NE>> {
   enum class ColumnIndex {
      Name    ,
      Path    ,
      FullPath,
   };
   enum class Info { NumberOfColumns = 3 };
   using ParentType = TreeFolderNode<NE>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeFolderNode<NE>>, std::shared_ptr<TreeItemNode<NE>>>;
};

/**
 * \brief Besides other folders of the same type, a given type of folders can only only contain one type of thing (eg
 *        FermentableTreeItem, HopTreeItem, etc).
 */
template<class NE>
class TreeFolderNode : public TreeNodeBase<TreeFolderNode<NE>, NE> {
public:
   TreeFolderNode();
   ~TreeFolderNode();
   static EnumStringMapping const columnDisplayNames;
   static bool isLessThan(TreeModel const & model,
                          QModelIndex const & left,
                          QModelIndex const & right,
                          TreeNodeBase<TreeFolderNode<NE>, NE>::ColumnIndex section,
                          Folder const & lhs,
                          Folder const & rhs);

};
///////////////////////////////////////////

////////////// TreeItemNode
template<class NE>
class TreeItemNode : public TreeNodeBase<TreeItemNode<NE>, NE> {
public:
   TreeItemNode();
   virtual ~TreeItemNode();
   static EnumStringMapping const columnDisplayNames;
   static bool isLessThan(TreeModel const & model,
                          QModelIndex const & left,
                          QModelIndex const & right,
                          TreeItemTraits<TreeItemNode<NE>>::ColumnIndex section,
                          NE const & lhs,
                          NE const & rhs);
};

class BrewNote;

template<> struct TreeItemTraits<TreeItemNode<BrewNote>> {
   enum class ColumnIndex {
      BrewDate,
   };
   enum class Info { NumberOfColumns = 1 };
   using ParentType = TreeItemNode<Recipe>;
   using ChildPtrTypes = std::variant<std::monostate>;
};

template<> struct TreeItemTraits<TreeItemNode<Recipe>> {
   enum class ColumnIndex {
      Name             ,
      NumberOfAncestors,
      BrewDate         ,
      Style            ,
   };
   enum class Info { NumberOfColumns = 4 };
   using ParentType = TreeFolderNode<Recipe>;
   using ChildPtrTypes = std::variant<std::shared_ptr<TreeItemNode<BrewNote>>, std::shared_ptr<TreeItemNode<Recipe>>>;
};

template<> struct TreeItemTraits<TreeItemNode<Equipment>> {
   enum class ColumnIndex {
      Name    ,
      BoilTime,
   };
   enum class Info { NumberOfColumns = 2 };
   using ParentType = TreeFolderNode<Equipment>;
   using ChildPtrTypes = std::variant<std::monostate>;
};

template<> struct TreeItemTraits<TreeItemNode<Fermentable>> {
   enum class ColumnIndex {
      Name ,
      Type ,
      Color,
   };
   enum class Info { NumberOfColumns = 3 };
   using ParentType = TreeFolderNode<Fermentable>;
   using ChildPtrTypes = std::variant<std::monostate>;
};

template<> struct TreeItemTraits<TreeItemNode<Hop>> {
   enum class ColumnIndex {
      Name    ,
      Form    ,
      AlphaPct, // % Alpha Acid
      Origin  , // Country of origin
   };
   enum class Info { NumberOfColumns = 4 };
   using ParentType = TreeFolderNode<Hop>;
   using ChildPtrTypes = std::variant<std::monostate>;
};

template<> struct TreeItemTraits<TreeItemNode<Misc>> {
   enum class ColumnIndex {
      Name,
      Type,
   };
   enum class Info { NumberOfColumns = 2 };
   using ParentType = TreeFolderNode<Misc>;
   using ChildPtrTypes = std::variant<std::monostate>;
};

template<> struct TreeItemTraits<TreeItemNode<Yeast>> {
   enum class ColumnIndex {
      // It's tempting to put Laboratory first, and have it at the first column, but it messes up the way the folders
      // work if the first column isn't Name
      Name,
      Laboratory,
      ProductId,
      Type,
      Form,
   };
   enum class Info { NumberOfColumns = 5 };
   using ParentType = TreeFolderNode<Yeast>;
   using ChildPtrTypes = std::variant<std::monostate>;
};

template<> struct TreeItemTraits<TreeItemNode<Style>> {
   enum class ColumnIndex {
      Name          ,
      Category      ,
      CategoryNumber,
      CategoryLetter,
      StyleGuide    ,
   };
   enum class Info { NumberOfColumns = 5 };
   using ParentType = TreeFolderNode<Style>;
   using ChildPtrTypes = std::variant<std::monostate>;
};

template<> struct TreeItemTraits<TreeItemNode<Water>> {
   enum class ColumnIndex {
      Name       ,
      Calcium    ,
      Bicarbonate,
      Sulfate    ,
      Chloride   ,
      Sodium     ,
      Magnesium  ,
      pH         ,
   };
   enum class Info { NumberOfColumns = 8 };
   using ParentType = TreeFolderNode<Water>;
   using ChildPtrTypes = std::variant<std::monostate>;
};

//
// Check the concepts we use above are working as we intend
//
static_assert(IsSubstantiveVariant<TreeFolderNode<Equipment>::ChildPtrTypes>);
static_assert(IsSubstantiveVariant<TreeFolderNode<Style>::ChildPtrTypes>);
static_assert(IsNullVariant<TreeItemNode<Equipment>::ChildPtrTypes>);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \class TreeNode
 *
 * \brief Model for an item in a tree.
 *
 * This provides a generic item from which the trees are built. Since most of
 * the actions required are the same regardless of the item being stored (e.g.
 * hop or equipment), this class considers them all the same.
 *
 * It does assume that everything being stored can be cast into a QObject.
 */
class TreeNode {

public:

   /*!
    * This enum lists the different things that we can store in an item
    */
   enum class Type {
      Recipe,
      Equipment,
      Fermentable,
      Hop,
      Misc,
      Yeast,
      BrewNote,
      Style,
      Folder,
      Water
   };

   /**
    * \brief This templated function will convert a class to its \c TreeNode::Type. Eg \c typeOf<Hop>() returns
    *        \c TreeNode::Type::Hop
    */
   template<class T>
   static TreeNode::Type typeOf();

   friend bool operator==(TreeNode & lhs, TreeNode & rhs);

   //! \brief A constructor that sets the \c type of the TreeNode and
   // the \c parent
   TreeNode(TreeNode::Type nodeType = TreeNode::Type::Folder, TreeNode * parent = nullptr);
   virtual ~TreeNode();

   //! \brief returns the child at \c number
   TreeNode * child(int number);
   //! \brief returns item's parent
   TreeNode * parent();

   //! \brief returns item's type
   TreeNode::Type type() const;
   //! \brief returns the number of the item's children
   int childCount() const;
   //! \brief returns number of columns associated with the item's \c type
   int columnCount(TreeNode::Type nodeType) const;
   //! \brief returns the data of the item of \c type at \c column
   QVariant data(/*TreeNode::Type nodeType, */int column);
   //! \brief returns the index of the item in it's parents list
   int childNumber() const;

   //! \brief sets the \c t type of the object and the \c d data
   void setData(TreeNode::Type t, QObject * d);

   //! \brief returns the data as a T
   template<class T> T * getData();

   //! \brief returns the data as a NamedEntity
   NamedEntity * thing();

   //! \brief inserts \c count new items of \c type, starting at \c position
   bool insertChildren(int position, int count, TreeNode::Type nodeType = TreeNode::Type::Recipe);
   //! \brief removes \c count items starting at \c position
   bool removeChildren(int position, int count);

   //! \brief returns the name.
   QString name();
   //! \brief flag this node to override display() or not
   void setShowMe(bool val);
   //! \brief does the node want to be shown regardless of display()
   bool showMe() const;

private:
   /*!  Keep a pointer to the parent tree item. */
   TreeNode * parentItem;
   /*!  The list of children associated with this item */
   QList<TreeNode *> childItems;

   /*! the type of this item */
   TreeNode::Type nodeType;

   /*! the data associated with this item */
   QObject * m_thing;
   //! \b overrides the display()
   bool m_showMe;

   /*! helper functions to get the information from the item */
   QVariant dataRecipe(int column);
   QVariant dataEquipment(int column);
   QVariant dataFermentable(int column);
   QVariant dataHop(int column);
   QVariant dataMisc(int column);
   QVariant dataYeast(int column);
   QVariant dataBrewNote(int column);
   QVariant dataStyle(int column);
   QVariant dataFolder(int column);
   QVariant dataWater(int column);
};

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, TreeNode::Type const treeNodeType);

#endif
