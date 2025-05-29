/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeNode.h is part of Brewtarget, and is copyright the following authors 2009-2025:
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

#include <QDebug>
#include <QIcon>
#include <QList>
#include <QObject>
#include <QModelIndex>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include "Localization.h"
#include "RecipeFormatter.h"
#include "config.h"
#include "measurement/Measurement.h"
#include "model/BrewNote.h"
#include "model/Folder.h"
#include "model/Recipe.h"
#include "utils/EnumStringMapping.h"
#include "utils/NoCopy.h"

/**
 * \brief Each tree has one primary type of object that it stores.  However, some trees (eg Recipe, Mash) can hold
 *        secondary items (eg Recipe tree holds Recipes and BrewNotes owned by those Recipes).  It's useful to have a
 *        compile-time mapping from object type to show which class belongs in which tree.  The rule here is that things
 *        belong in their own tree (eg Equipment is in Equipment tree) unless there's a specialisation that says
 *        otherwise.
 *
 *        If a secondary item \c FooBar is omitted from this list, we'll get compile errors along the lines of `invalid
 *        use of incomplete type ‘struct TreeNodeTraits<FooBar, FooBar>’`.
 */
template <class NE> struct TreeTypeDeducer                   { using TreeType = NE          ; };
template<>          struct TreeTypeDeducer<BrewNote        > { using TreeType = Recipe      ; };
template<>          struct TreeTypeDeducer<MashStep        > { using TreeType = Mash        ; };
template<>          struct TreeTypeDeducer<BoilStep        > { using TreeType = Boil        ; };
template<>          struct TreeTypeDeducer<FermentationStep> { using TreeType = Fermentation; };

class TreeModel;


//! \brief See comment in \c trees/TreeNodeBase.h for explanation of this
enum class TreeNodeClassifier {
   Folder        = 0,
   PrimaryItem   = 1,
   SecondaryItem = 2,
};
//! \brief Convenience function for logging
template<class S> S & operator<<(S & stream, TreeNodeClassifier const treeNodeClassifier) {
   switch (treeNodeClassifier) {
      case TreeNodeClassifier::Folder       : stream << "Folder"       ; break;
      case TreeNodeClassifier::PrimaryItem  : stream << "PrimaryItem"  ; break;
      case TreeNodeClassifier::SecondaryItem: stream << "SecondaryItem"; break;
   }
   return stream;
}


class TreeNode {
protected:
   TreeNode(TreeModel & model);

public:
   ~TreeNode();

   /**
    * \brief Returns total number of nodes of specified type in this node's subtree.
    */
   int nodeCount(TreeNodeClassifier const classifier) const;

   /**
    * \brief Returns a string representation of this node's subtree, useful for logging/debugging.
    *        The parameters are used for recursive calling.  The original caller can just leave them defaulted.
    *
    * \param indent A string of spaces and "│" characters to indent the current node in the output
    * \param prefix The characters to prefix the node with (blank for root node, "├──", "└──"
    */
   QString subTreeToString(QString const indent = "", QString const prefix = "") const;

   /**
    * \brief Used by \c subTreeToString.  Saves us creating lots of QTextStream objects when we're ultimately sending
    *        all the output to the same one.
    */
   void subTreeToStream(QTextStream & outputStream, QString const & indent, QString const & prefix) const;

   /**
    * \brief Derived classes implement this function, which then makes it easy for us to cast from TreeNode * to the
    *        actual type.
    */
   virtual TreeNodeClassifier classifier() const = 0;

   /**
    * \brief Called from \c TreeModelBase::doData to obtain what to show in the specified column for the given role
    *
    *        See https://doc.qt.io/qt-6/qt.html#ItemDataRole-enum for possible values for \c role
    */
   virtual QVariant data(int const column, int const role) const = 0;

   virtual int childCount() const = 0;

   virtual TreeNode * rawChild(int number) const = 0;

   virtual TreeNode * rawParent() const = 0;

   /**
    * \brief NOTE that this is not currently supported if the underlying item is a \c Folder
    */
   virtual NamedEntity * rawUnderlyingItem() const = 0;

   virtual int numberOfChild(TreeNode const * childToCheck) const = 0;

   virtual int childNumber() const = 0;

   virtual bool removeChildren(int position, int count) = 0;

   /**
    * \brief Class name of whatever type of object is stored in this node (eg "Recipe", "Hop, etc)
    */
   virtual QString className() const = 0;

   /**
    * \brief Localised name of whatever type of object is stored in this node (eg
    *        "Recipe" -> "Recette" / "Rezept" / "Receta" / etc)
    */
   virtual QString localisedClassName() const = 0;

   //! \brief Name of individual object stored in this node (eg "Oatmeal Stout")
   virtual QString name() const = 0;

   //! \brief ID of individual object stored in this node.  NB: Will currently return 0 for a \c Folder
   virtual int underlyingItemKey() const = 0;

   virtual QString dragAndDropMimeType() const = 0;

   /**
    * \brief For a \c TreeFolderNode, this should return the folder held by the node.
    *        For a \c TreeItemNode, this should return the closest containing folder, or \c nullptr otherwise.
    */
   virtual std::shared_ptr<Folder> folder() const = 0;

   //! \brief flag this node to override display() or not
   void setShowMe(bool val);

   //! \brief does the node want to be shown regardless of display()
   bool showMe() const;

protected:
   /**
    * \brief This is currently used only in the Recipe tree.
    *
    *        If this is true then the node is to be displayed regardless of the return value of display for the item it
    *        contains.
    */
   bool m_showMe = true;

   /**
    * \brief The model to which this node belongs.
    *
    *        Every \c TreeNode object belongs to a \c TreeModel object.  Eg \c TreeFolderNode<Recipe>,
    *        \c TreeItemNode<Recipe> and \c TreeItemNode<BrewNote> belong to \c RecipeTreeModel.  Strictly speaking, we
    *        do not need to store a reference to the model here because it can be determined from the type of the node.
    *        (There is only one \c RecipeTreeModel object, and all \c TreeItemNode<Recipe> nodes belong to it, etc.)
    *        However, for the moment, it is convenient to have the reference to hand.  And no tree is going to have
    *        millions of nodes, so it's not a huge memory overhead in absolute terms.
    */
   TreeModel & m_model;

private:
   // Insert all the usual boilerplate to prevent copy/assignment/move
   // Since a TreeNode owns its children, we don't want nodes to be copied.
   NO_COPY_DECLARATIONS(TreeNode)
};

//! \brief Convenience function for logging
template<class S> S & operator<<(S & stream, TreeNode const & treeNode) {
   stream <<
      treeNode.className() << "TreeNode (" << treeNode.classifier() << " #" << treeNode.underlyingItemKey() << "): " <<
      treeNode.name() << " (" << treeNode.childCount() << " children)";
   return stream;
}
template<class S> S & operator<<(S & stream, TreeNode const * treeNode) {
   if (treeNode) {
      stream << *treeNode;
   } else {
      stream << "NULL";
   }
   return stream;
}

#endif
