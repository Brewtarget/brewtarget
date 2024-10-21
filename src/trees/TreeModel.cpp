/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "trees/TreeModel.h"

#include <cstring>

#include <QAbstractItemModel>
#include <QList>
#include <QMessageBox>
#include <QMimeData>
#include <QModelIndex>
#include <QObject>
#include <QRegularExpression>
#include <QStringBuilder>
#include <Qt>
#include <QVariant>

#include "AncestorDialog.h"
#include "model/Folder.h"
#include "trees/TreeView.h"
#include "RecipeFormatter.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Yeast.h"
#include "model/BrewNote.h"
#include "model/Style.h"
#include "model/Water.h"
#include "utils/BtStringConst.h"
#include "PersistentSettings.h"

namespace {
   NamedEntity * getElement(TreeNode::Type oType, int id) {
      switch (oType) {
         // .:TODO:. For now we just pull the raw pointer out of the shared pointer, but the rest of this code needs refactoring
         case TreeNode::Type::Recipe     : return ObjectStoreWrapper::getById<Recipe     >(id).get();
         case TreeNode::Type::Equipment  : return ObjectStoreWrapper::getById<Equipment  >(id).get();
         case TreeNode::Type::Fermentable: return ObjectStoreWrapper::getById<Fermentable>(id).get();
         case TreeNode::Type::Hop        : return ObjectStoreWrapper::getById<Hop        >(id).get();
         case TreeNode::Type::Misc       : return ObjectStoreWrapper::getById<Misc       >(id).get();
         case TreeNode::Type::Style      : return ObjectStoreWrapper::getById<Style      >(id).get();
         case TreeNode::Type::Yeast      : return ObjectStoreWrapper::getById<Yeast      >(id).get();
         case TreeNode::Type::Water      : return ObjectStoreWrapper::getById<Water      >(id).get();
         case TreeNode::Type::Folder     : break;
         default:
            return nullptr;
      }

      return nullptr;
   }
}

namespace FolderUtils {

   std::optional<QString> getFolder(NamedEntity const * ne) {
      if (!ne) {
         return std::nullopt;
      }

      QMetaObject const * neMetaObject = ne->metaObject();
      int propertyIndex = neMetaObject->indexOfProperty(*PropertyNames::FolderBase::folder);
      if (propertyIndex >= 0) {
         QMetaProperty neMetaProperty = neMetaObject->property(propertyIndex);
         Q_ASSERT(neMetaProperty.isReadable());
         QVariant val = ne->property(*PropertyNames::FolderBase::folder);
         return val.toString();
      }
      return std::nullopt;
   }

   void setFolder(NamedEntity * ne, QString const & val) {
      if (!ne) {
         return;
      }
      QMetaObject const * neMetaObject = ne->metaObject();
      int propertyIndex = neMetaObject->indexOfProperty(*PropertyNames::FolderBase::folder);
      if (propertyIndex >= 0) {
         QMetaProperty neMetaProperty = neMetaObject->property(propertyIndex);
         Q_ASSERT(neMetaProperty.isReadable());

         bool succeeded = ne->setProperty(*PropertyNames::FolderBase::folder, val);
         if (!succeeded) {
            qCritical() <<
               Q_FUNC_INFO << "Error trying to set" << PropertyNames::FolderBase::folder << "on" <<
               neMetaObject->className() << "to" << val << "; property type =" <<
               neMetaProperty.typeName() << "; writable =" << neMetaProperty.isWritable();
         }
      }
      return;
   }

}

// =========================================================================
// ============================ CLASS STUFF ================================
// =========================================================================

TreeModel::TreeModel(TreeView * parent, TreeModel::TypeMasks types) :
   QAbstractItemModel(parent) {
   // Initialize the tree structure
   int items = 0;
   this->rootItem = new TreeNode();

   if (types.testFlag(TreeModel::TypeMask::Recipe)) {
      rootItem->insertChildren(items, 1, TreeNode::Type::Recipe);
      connect(&ObjectStoreTyped<Recipe>::getInstance(), &ObjectStoreTyped<Recipe>::signalObjectInserted, this, &TreeModel::elementAddedRecipe);
      connect(&ObjectStoreTyped<Recipe>::getInstance(), &ObjectStoreTyped<Recipe>::signalObjectDeleted,  this, &TreeModel::elementRemovedRecipe);
      // Brewnotes need love too!
      connect(&ObjectStoreTyped<BrewNote>::getInstance(), &ObjectStoreTyped<BrewNote>::signalObjectInserted, this, &TreeModel::elementAddedBrewNote);
      connect(&ObjectStoreTyped<BrewNote>::getInstance(), &ObjectStoreTyped<BrewNote>::signalObjectDeleted,  this, &TreeModel::elementRemovedBrewNote);
      // And some versioning stuff, because why not?
      connect(&ObjectStoreTyped<Recipe>::getInstance(), &ObjectStoreTyped<Recipe>::signalPropertyChanged, this, &TreeModel::recipePropertyChanged);
      this->nodeType = TreeNode::Type::Recipe;
      m_mimeType = "application/x-brewtarget-recipe";
      m_maxColumns = static_cast<int>(TreeItemNode<Recipe>::Info::NumberOfColumns);
   } else if (types.testFlag(TreeModel::TypeMask::Equipment)) {
      rootItem->insertChildren(items, 1, TreeNode::Type::Equipment);
      connect(&ObjectStoreTyped<Equipment>::getInstance(), &ObjectStoreTyped<Equipment>::signalObjectInserted, this, &TreeModel::elementAddedEquipment);
      connect(&ObjectStoreTyped<Equipment>::getInstance(), &ObjectStoreTyped<Equipment>::signalObjectDeleted,  this, &TreeModel::elementRemovedEquipment);
      this->nodeType = TreeNode::Type::Equipment;
      m_mimeType = "application/x-brewtarget-recipe";
      m_maxColumns = static_cast<int>(TreeItemNode<Equipment>::Info::NumberOfColumns);
   } else if (types.testFlag(TreeModel::TypeMask::Fermentable)) {
      rootItem->insertChildren(items, 1, TreeNode::Type::Fermentable);
      connect(&ObjectStoreTyped<Fermentable>::getInstance(), &ObjectStoreTyped<Fermentable>::signalObjectInserted, this, &TreeModel::elementAddedFermentable);
      connect(&ObjectStoreTyped<Fermentable>::getInstance(), &ObjectStoreTyped<Fermentable>::signalObjectDeleted,  this, &TreeModel::elementRemovedFermentable);
      this->nodeType = TreeNode::Type::Fermentable;
      m_mimeType = "application/x-brewtarget-ingredient";
      m_maxColumns = static_cast<int>(TreeItemNode<Fermentable>::Info::NumberOfColumns);
   } else if (types.testFlag(TreeModel::TypeMask::Hop)) {
      rootItem->insertChildren(items, 1, TreeNode::Type::Hop);
      connect(&ObjectStoreTyped<Hop>::getInstance(), &ObjectStoreTyped<Hop>::signalObjectInserted, this, &TreeModel::elementAddedHop);
      connect(&ObjectStoreTyped<Hop>::getInstance(), &ObjectStoreTyped<Hop>::signalObjectDeleted,  this, &TreeModel::elementRemovedHop);
      this->nodeType = TreeNode::Type::Hop;
      m_mimeType = "application/x-brewtarget-ingredient";
      m_maxColumns = static_cast<int>(TreeItemNode<Hop>::Info::NumberOfColumns);
   } else if (types.testFlag(TreeModel::TypeMask::Misc)) {
      rootItem->insertChildren(items, 1, TreeNode::Type::Misc);
      connect(&ObjectStoreTyped<Misc>::getInstance(), &ObjectStoreTyped<Misc>::signalObjectInserted, this, &TreeModel::elementAddedMisc);
      connect(&ObjectStoreTyped<Misc>::getInstance(), &ObjectStoreTyped<Misc>::signalObjectDeleted,  this, &TreeModel::elementRemovedMisc);
      this->nodeType = TreeNode::Type::Misc;
      m_mimeType = "application/x-brewtarget-ingredient";
      m_maxColumns = static_cast<int>(TreeItemNode<Misc>::Info::NumberOfColumns);
   } else if (types.testFlag(TreeModel::TypeMask::Style)) {
      rootItem->insertChildren(items, 1, TreeNode::Type::Style);
      connect(&ObjectStoreTyped<Style>::getInstance(), &ObjectStoreTyped<Style>::signalObjectInserted, this, &TreeModel::elementAddedStyle);
      connect(&ObjectStoreTyped<Style>::getInstance(), &ObjectStoreTyped<Style>::signalObjectDeleted,  this, &TreeModel::elementRemovedStyle);
      this->nodeType = TreeNode::Type::Style;
      m_mimeType = "application/x-brewtarget-recipe";
      m_maxColumns = static_cast<int>(TreeItemNode<Style>::Info::NumberOfColumns);
   } else if (types.testFlag(TreeModel::TypeMask::Yeast)) {
      rootItem->insertChildren(items, 1, TreeNode::Type::Yeast);
      connect(&ObjectStoreTyped<Yeast>::getInstance(), &ObjectStoreTyped<Yeast>::signalObjectInserted, this, &TreeModel::elementAddedYeast);
      connect(&ObjectStoreTyped<Yeast>::getInstance(), &ObjectStoreTyped<Yeast>::signalObjectDeleted,  this, &TreeModel::elementRemovedYeast);
      this->nodeType = TreeNode::Type::Yeast;
      m_mimeType = "application/x-brewtarget-ingredient";
      m_maxColumns = static_cast<int>(TreeItemNode<Yeast>::Info::NumberOfColumns);
   } else if (types.testFlag(TreeModel::TypeMask::Water)) {
      rootItem->insertChildren(items, 1, TreeNode::Type::Water);
      connect(&ObjectStoreTyped<Water>::getInstance(), &ObjectStoreTyped<Water>::signalObjectInserted, this, &TreeModel::elementAddedWater);
      connect(&ObjectStoreTyped<Water>::getInstance(), &ObjectStoreTyped<Water>::signalObjectDeleted,  this, &TreeModel::elementRemovedWater);
      this->nodeType = TreeNode::Type::Water;
      m_mimeType = "application/x-brewtarget-ingredient";
      m_maxColumns = static_cast<int>(TreeItemNode<Water>::Info::NumberOfColumns);
   } else {
      qWarning() << Q_FUNC_INFO << "Invalid treemask:" << types;
   }

   this->m_treeMask = types;
   this->parentTree = parent;
   this->loadTreeModel();
   return;
}

TreeModel::~TreeModel() {
   // Qt automatically handles the disconnection of any signals we were listening to
   delete rootItem;
   rootItem = nullptr;
}

// =========================================================================
// =================== AbstractItemModel STUFF =============================
// =========================================================================

TreeNode * TreeModel::item(const QModelIndex & index) const {
   if (index.isValid()) {
      TreeNode * item = static_cast<TreeNode *>(index.internalPointer());
      if (item) {
         return item;
      }
   }

   return rootItem;
}

int TreeModel::rowCount(const QModelIndex & parent) const {
   if (! parent.isValid()) {
//      qDebug() << Q_FUNC_INFO << "No parent. Root item has" << this->rootItem->childCount() << "children";
      return this->rootItem->childCount();
   }

//   qDebug() << Q_FUNC_INFO << "Parent has" << this->item(parent)->childCount() << "children";
   return this->item(parent)->childCount();
}

int TreeModel::columnCount(const QModelIndex & parent) const {
   Q_UNUSED(parent)
   return m_maxColumns;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex & index) const {
   if (!index.isValid()) {
      return Qt::ItemIsDropEnabled;
   }

   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled |
          Qt::ItemIsDropEnabled;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex & parent) const {
   TreeNode * pItem, *cItem;

   if (parent.isValid() && parent.column() >= columnCount(parent)) {
      return QModelIndex();
   }

   pItem = item(parent);
   cItem = pItem->child(row);

   if (cItem) {
      return createIndex(row, column, cItem);
   } else {
      return QModelIndex();
   }
}

QModelIndex TreeModel::parent(QModelIndex const & index) const {
   if (!index.isValid()) {
      return QModelIndex();
   }

   TreeNode * cItem = item(index);

   if (cItem == nullptr) {
      return QModelIndex();
   }

   TreeNode * pItem = cItem->parent();

   if (pItem == rootItem || pItem == nullptr) {
      return QModelIndex();
   }

   return createIndex(pItem->childNumber(), 0, pItem);
}

QModelIndex TreeModel::first() {

   // get the first item in the list, which is the place holder
   TreeNode * pItem = rootItem->child(0);
   if (pItem->childCount() > 0) {
      return this->createIndex(0, 0, pItem->child(0));
   }

   return QModelIndex();
}

QVariant TreeModel::data(const QModelIndex & index, int role) const {
   int maxColumns;

   if (m_treeMask.testFlag(TreeModel::TypeMask::Folder)) {
      // All folders have the same columns, so it's a bit arbitrary which one we use here
      maxColumns = static_cast<int>(TreeFolderNode<Hop>::Info::NumberOfColumns);
   } else {
      maxColumns = m_maxColumns;
   }

   if (!rootItem || !index.isValid() || index.column() < 0 || index.column() >= maxColumns) {
      return QVariant();
   }

   TreeNode * itm = item(index);

   QFont font;
   switch (role) {
      case Qt::ToolTipRole:
         return toolTipData(index);
      case Qt::DisplayRole:
         return itm->data(index.column());
      case Qt::DecorationRole:
         if (index.column() == 0 && itm->type() == TreeNode::Type::Folder) {
            return QIcon(":images/folder.png");
         }
         break;
      default:
         break;
   }

   return QVariant();
}

QVariant TreeModel::toolTipData(const QModelIndex & index) const {
   // .:TBD:. This looks like a memory leak...
   RecipeFormatter * rf = new RecipeFormatter();

   if (m_treeMask.testFlag(TreeModel::TypeMask::Recipe     )) { return rf->getToolTip(qobject_cast<Recipe *     >(thing(index))); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Style      )) { return rf->getToolTip(qobject_cast<Style *      >(thing(index))); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Equipment  )) { return rf->getToolTip(qobject_cast<Equipment *  >(thing(index))); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Fermentable)) { return rf->getToolTip(qobject_cast<Fermentable *>(thing(index))); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Hop        )) { return rf->getToolTip(qobject_cast<Hop *        >(thing(index))); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Misc       )) { return rf->getToolTip(qobject_cast<Misc *       >(thing(index))); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Yeast      )) { return rf->getToolTip(qobject_cast<Yeast *      >(thing(index))); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Water      )) { return rf->getToolTip(qobject_cast<Water *      >(thing(index))); }

   return item(index)->name();
}

// This is much better, assuming the rest can be made to work
QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
      return QVariant();
   }

   if (m_treeMask.testFlag(TreeModel::TypeMask::Recipe     )) { return TreeItemNode<Recipe     >::header(section); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Equipment  )) { return TreeItemNode<Equipment  >::header(section); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Fermentable)) { return TreeItemNode<Fermentable>::header(section); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Hop        )) { return TreeItemNode<Hop        >::header(section); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Misc       )) { return TreeItemNode<Misc       >::header(section); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Yeast      )) { return TreeItemNode<Yeast      >::header(section); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Style      )) { return TreeItemNode<Style      >::header(section); }
   // All folders have the same columns, so it's a bit arbitrary which one we use here
   if (m_treeMask.testFlag(TreeModel::TypeMask::Folder     )) { return TreeFolderNode<Hop      >::header(section); }
   if (m_treeMask.testFlag(TreeModel::TypeMask::Water      )) { return TreeItemNode<Water      >::header(section); }

   return QVariant();
}

bool TreeModel::insertRow(int row,
                            QModelIndex const & parent,
                            QObject * victim,
                            std::optional<TreeNode::Type> victimType) {
   if (! parent.isValid()) {
      return false;
   }

   TreeNode * pItem = item(parent);
   auto type = pItem->type();

   bool success = true;

   beginInsertRows(parent, row, row);
   success = pItem->insertChildren(row, 1, type);
   if (victim && success) {
      type = (victimType ? *victimType : type);
      TreeNode * added = pItem->child(row);
      added->setData(type, victim);
   }
   endInsertRows();

   return success;
}

bool TreeModel::removeRows(int row, int count, const QModelIndex & parent) {
   TreeNode * pItem = item(parent);

   this->beginRemoveRows(parent, row, row + count - 1);
   bool success = pItem->removeChildren(row, count);
   this->endRemoveRows();

   return success;
}

// One find method for all things. This .. is nice
QModelIndex TreeModel::findElement(NamedEntity * thing, TreeNode * parent) {
   TreeNode * pItem = parent ? parent : this->rootItem->child(0);


   if (! thing) {
      return createIndex(0, 0, pItem);
   }

   if (pItem) {
      qDebug() << Q_FUNC_INFO << "Find" << *thing << "in" << pItem->name();
   }

   QList<TreeNode *> folders;
   folders.append(pItem);

   // Recursion. Wonderful.
   while (! folders.isEmpty()) {
      TreeNode * target = folders.takeFirst();
      for (int i = 0; i < target->childCount(); ++i) {
         // If we've found what we are looking for, return
         if (target->child(i)->thing() == thing) {
            qDebug() << Q_FUNC_INFO << "Found at" << i;
            return createIndex(i, 0, target->child(i));
         }

         // If we have a folder, or we are looking for a brewnote and have a
         // recipe in hand, push the child onto the stack
         if (target->child(i)->type() == TreeNode::Type::Folder ||
             (qobject_cast<BrewNote *>(thing) && target->child(i)->type() == TreeNode::Type::Recipe)) {
            folders.append(target->child(i));
         }
      }
   }
   return QModelIndex();
}

QList<NamedEntity *> TreeModel::elements() {
   QList<NamedEntity *> elements;
   //
   // .:TBD:. This code assumes no more than one flag is set in the mask.  If that's true, why bother having a set of
   //         flags and not just an enum?
   //
   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Recipe)) {
      for (NamedEntity * elem : ObjectStoreWrapper::getAllDisplayableRaw<Recipe>()) {
         elements.append(elem);
      }

   } else if (this->m_treeMask.testFlag(TreeModel::TypeMask::Equipment)) {
      for (NamedEntity * elem : ObjectStoreWrapper::getAllDisplayableRaw<Equipment>()) {
         elements.append(elem);
      }
   } else if (this->m_treeMask.testFlag(TreeModel::TypeMask::Fermentable)) {
      for (NamedEntity * elem : ObjectStoreWrapper::getAllDisplayableRaw<Fermentable>()) {
         elements.append(elem);
      }
   } else if (this->m_treeMask.testFlag(TreeModel::TypeMask::Hop)) {
      for (NamedEntity * elem : ObjectStoreWrapper::getAllDisplayableRaw<Hop>()) {
         elements.append(elem);
      }
   } else if (this->m_treeMask.testFlag(TreeModel::TypeMask::Misc)) {
      for (NamedEntity * elem : ObjectStoreWrapper::getAllDisplayableRaw<Misc>()) {
         elements.append(elem);
      }
   } else if (this->m_treeMask.testFlag(TreeModel::TypeMask::Yeast)) {
      for (NamedEntity * elem : ObjectStoreWrapper::getAllDisplayableRaw<Yeast>()) {
         elements.append(elem);
      }
   } else if (this->m_treeMask.testFlag(TreeModel::TypeMask::Style)) {
      for (NamedEntity * elem : ObjectStoreWrapper::getAllDisplayableRaw<Style>()) {
         elements.append(elem);
      }
   } else if (this->m_treeMask.testFlag(TreeModel::TypeMask::Water)) {
      for (NamedEntity * elem : ObjectStoreWrapper::getAllDisplayableRaw<Water>()) {
         elements.append(elem);
      }
   } else {
      qWarning() << Q_FUNC_INFO << "Invalid treemask:" << this->m_treeMask;
   }
   return elements;
}

void TreeModel::loadTreeModel() {
   int i;

   QModelIndex ndxLocal;
   TreeNode * local = nullptr;
   QList<NamedEntity *> elems = this->elements();

   qDebug() << Q_FUNC_INFO << "Got " << elems.length() << "elements matching type mask" << this->m_treeMask;

   for (NamedEntity * elem : elems) {
      // TODO: At some point we should refactor this code so that we have separate handling for objects that have
      //       folders from ones that don't.
      auto folder = FolderUtils::getFolder(elem);
      if (folder && !folder->isEmpty()) {
         ndxLocal = findFolder(*folder, rootItem->child(0), true);
         // I cannot imagine this failing, but what the hell
         if (! ndxLocal.isValid()) {
            qWarning() << Q_FUNC_INFO << "Invalid return from findFolder in loadTreeModel()";
            continue;
         }
         local = item(ndxLocal);
         i = local->childCount();
      } else {
         local = rootItem->child(0);
         i = local->childCount();
         ndxLocal = createIndex(i, 0, local);
      }

      if (!this->insertRow(i, ndxLocal, elem, this->nodeType)) {
         qWarning() << Q_FUNC_INFO << "Insert failed in loadTreeModel()";
         continue;
      }

      // If we have brewnotes, set them up here.
      if (m_treeMask & TreeModel::TypeMask::Recipe) {
         Recipe * holdmebeer = qobject_cast<Recipe *>(elem);
         if (PersistentSettings::value(PersistentSettings::Names::showsnapshots, false).toBool() && holdmebeer->hasAncestors()) {
            setShowChild(ndxLocal, true);
            addAncestoralTree(holdmebeer, i, local);
            addBrewNoteSubTree(holdmebeer, i, local, false);
         } else {
            addBrewNoteSubTree(holdmebeer, i, local);
         }
      }
      observeElement(elem);
   }
   return;
}

void TreeModel::addAncestoralTree(Recipe * rec, int i, TreeNode * parent) {
   TreeNode * temp = parent->child(i);
   int j = 0;

   for (Recipe * stor : rec->ancestors()) {
      // insert the ancestor. This is most of magic. One day, I understood it.
      // Now I simply copy/paste it
      if (! insertRow(j, createIndex(i, 0, temp), stor, TreeNode::Type::Recipe)) {
         qWarning() << Q_FUNC_INFO << "Ancestor insert failed in loadTreeModel()";
         continue;
      }
      // we need to find the index of what we just inserted
      QModelIndex cIndex = findElement(stor, temp);
      // and set showChild on it
      setShowChild(cIndex, true);

      // finally, add this ancestors brewnotes but do not recurse
      addBrewNoteSubTree(stor, j, temp, false);
      observeElement(stor);
      ++j;
   }
   return;
}

void TreeModel::addBrewNoteSubTree(Recipe * rec, int i, TreeNode * parent, bool recurse) {
   auto notes = recurse ? RecipeHelper::brewNotesForRecipeAndAncestors(*rec) : rec->brewNotes();
   TreeNode * temp = parent->child(i);

   int j = 0;

   for (auto note : notes) {
      // In previous insert loops, we ignore the error and soldier on. So we
      // will do that here too
      if (! insertRow(j, createIndex(i, 0, temp), note.get(), TreeNode::Type::BrewNote)) {
         qWarning() << Q_FUNC_INFO << "Brewnote insert failed in loadTreeModel()";
         continue;
      }
      observeElement(note.get());
      ++j;
   }
   return;
}

template<class T>
T * TreeModel::getItem(QModelIndex const & index) const {
   return index.isValid() ? this->item(index)->getData<T>() : nullptr;
}
//
// Instantiate the above template function for the types that are going to use it
//
template Recipe      * TreeModel::getItem<Recipe     >(QModelIndex const & index) const;
template Equipment   * TreeModel::getItem<Equipment  >(QModelIndex const & index) const;
template Fermentable * TreeModel::getItem<Fermentable>(QModelIndex const & index) const;
template Hop         * TreeModel::getItem<Hop        >(QModelIndex const & index) const;
template Misc        * TreeModel::getItem<Misc       >(QModelIndex const & index) const;
template Yeast       * TreeModel::getItem<Yeast      >(QModelIndex const & index) const;
template BrewNote    * TreeModel::getItem<BrewNote   >(QModelIndex const & index) const;
template Style       * TreeModel::getItem<Style      >(QModelIndex const & index) const;
template Folder      * TreeModel::getItem<Folder     >(QModelIndex const & index) const;
template Water       * TreeModel::getItem<Water      >(QModelIndex const & index) const;

NamedEntity * TreeModel::thing(const QModelIndex & index) const {
   return index.isValid() ? this->item(index)->thing() : nullptr;
}

template<class T>
bool TreeModel::itemIs(QModelIndex const & index) const {
   return this->type(index) == TreeNode::typeOf<T>();
}
//
// Instantiate the above template function for the types that are going to use it
//
template bool TreeModel::itemIs<Recipe     >(QModelIndex const & index) const;
template bool TreeModel::itemIs<Equipment  >(QModelIndex const & index) const;
template bool TreeModel::itemIs<Fermentable>(QModelIndex const & index) const;
template bool TreeModel::itemIs<Hop        >(QModelIndex const & index) const;
template bool TreeModel::itemIs<Misc       >(QModelIndex const & index) const;
template bool TreeModel::itemIs<Yeast      >(QModelIndex const & index) const;
template bool TreeModel::itemIs<BrewNote   >(QModelIndex const & index) const;
template bool TreeModel::itemIs<Style      >(QModelIndex const & index) const;
template bool TreeModel::itemIs<Folder     >(QModelIndex const & index) const;
template bool TreeModel::itemIs<Water      >(QModelIndex const & index) const;

std::optional<TreeNode::Type> TreeModel::type(const QModelIndex & index) const {
   return index.isValid() ? std::optional<TreeNode::Type>{item(index)->type()} : std::nullopt;
}

QString TreeModel::name(const QModelIndex & idx) {
   return idx.isValid() ? item(idx)->name() : QString();
}

int TreeModel::mask() {
   return m_treeMask;
}

void TreeModel::copySelected(QList< QPair<QModelIndex, QString>> toBeCopied) {
   bool failed = false;
   while (! toBeCopied.isEmpty()) {
      QPair<QModelIndex, QString> thisPair = toBeCopied.takeFirst();
      QModelIndex ndx = thisPair.first;
      QString name = thisPair.second;

      //
      // Note that the type of the local variable copy is different in each case statement (std::shared_ptr<Equipment>,
      // std::shared_ptr<Fermentable>, etc) which is why the setName and insert calls cannot be pulled out of the
      // switch.
      //
      auto theType = type(ndx);
      if (!theType) {
         qWarning() << Q_FUNC_INFO << "Unknown type for ndx" << ndx;
      } else {
         switch (*theType) {
            case TreeNode::Type::Equipment: {
                  auto copy = ObjectStoreWrapper::copy(*this->getItem<Equipment>(ndx)); // Create a deep copy.
                  copy->setName(name);
                  ObjectStoreWrapper::insert(copy);
                  this->folderChanged(copy.get());
               }
               break;
            case TreeNode::Type::Fermentable: {
                  auto copy = ObjectStoreWrapper::copy(*this->getItem<Fermentable>(ndx)); // Create a deep copy.
                  copy->setName(name);
                  ObjectStoreWrapper::insert(copy);
                  this->folderChanged(copy.get());
               }
               break;
            case TreeNode::Type::Hop: {
                  auto copy = ObjectStoreWrapper::copy(*this->getItem<Hop>(ndx)); // Create a deep copy.
                  copy->setName(name);
                  ObjectStoreWrapper::insert(copy);
                  this->folderChanged(copy.get());
               }
               break;
            case TreeNode::Type::Misc: {
                  auto copy = ObjectStoreWrapper::copy(*this->getItem<Misc>(ndx)); // Create a deep copy.
                  copy->setName(name);
                  ObjectStoreWrapper::insert(copy);
                  this->folderChanged(copy.get());
               }
               break;
            case TreeNode::Type::Recipe: {
                  auto copy = ObjectStoreWrapper::copy(*this->getItem<Recipe>(ndx)); // Create a deep copy.
                  qDebug() <<
                     Q_FUNC_INFO << "display:" <<  copy->display() << "isLocked:" << copy->locked() <<
                     "hasDescendants:" << copy->hasDescendants();
                  copy->setName(name);
                  ObjectStoreWrapper::insert(copy);
                  this->folderChanged(copy.get());
               }
               break;
            case TreeNode::Type::Style: {
                  auto copy = ObjectStoreWrapper::copy(*this->getItem<Style>(ndx)); // Create a deep copy.
                  copy->setName(name);
                  ObjectStoreWrapper::insert(copy);
                  this->folderChanged(copy.get());
               }
               break;
            case TreeNode::Type::Yeast: {
                  auto copy = ObjectStoreWrapper::copy(*this->getItem<Yeast>(ndx)); // Create a deep copy.
                  copy->setName(name);
                  ObjectStoreWrapper::insert(copy);
                  this->folderChanged(copy.get());
               }
               break;
            case TreeNode::Type::Water: {
                  auto copy = ObjectStoreWrapper::copy(*this->getItem<Water>(ndx)); // Create a deep copy.
                  copy->setName(name);
                  ObjectStoreWrapper::insert(copy);
                  this->folderChanged(copy.get());
               }
               break;
            case TreeNode::Type::BrewNote:
            case TreeNode::Type::Folder:
               // These cases shouldn't arise (I think!) but the compiler will emit a warning if we don't explicitly
               // have code to handle them (which is good!).
               qWarning() << Q_FUNC_INFO << "Unexpected item type" << static_cast<int>(*theType);
               break;
         }
      }
      if (failed) {
         QMessageBox::warning(nullptr,
                              tr("Could not copy"),
                              tr("There was an unexpected error creating %1").arg(name));
         return;
      }
   }
}

void TreeModel::deleteSelected(QModelIndexList victims) {
   QModelIndexList toBeDeleted = victims; // trust me

   // There are black zones of shadow close to our daily paths,
   // and now and then some evil soul breaks a passage through.

   while (! toBeDeleted.isEmpty()) {
      QModelIndex ndx = toBeDeleted.takeFirst();
      auto theType = type(ndx);
      if (!theType) {
         qWarning() << Q_FUNC_INFO << "Unknown type for ndx" << ndx;
         continue;
      }
      switch (*theType) {
         case TreeNode::Type::Equipment:
            ObjectStoreWrapper::softDelete(*this->getItem<Equipment>(ndx));
            break;
         case TreeNode::Type::Fermentable:
            ObjectStoreWrapper::softDelete(*this->getItem<Fermentable>(ndx));
            break;
         case TreeNode::Type::Hop:
            ObjectStoreWrapper::softDelete(*this->getItem<Hop>(ndx));
            break;
         case TreeNode::Type::Misc:
            ObjectStoreWrapper::softDelete(*this->getItem<Misc>(ndx));
            break;
         case TreeNode::Type::Recipe:
            {
               auto rec = this->getItem<Recipe>(ndx);
               int deletewhat = PersistentSettings::value(PersistentSettings::Names::deletewhat, Recipe::DESCENDANT).toInt();
               if (deletewhat == Recipe::DESCENDANT) {
                  this->revertRecipeToPreviousVersion(ndx);
               } else {
                  for (auto ancestor : rec->ancestors()) {
                     ObjectStoreWrapper::softDelete(*ancestor);
                  }
               }
               ObjectStoreWrapper::softDelete(*rec);
            }
            break;
         case TreeNode::Type::Style:
            ObjectStoreWrapper::softDelete(*this->getItem<Style>(ndx));
            break;
         case TreeNode::Type::Yeast:
            ObjectStoreWrapper::softDelete(*this->getItem<Yeast>(ndx));
            break;
         case TreeNode::Type::BrewNote:
            ObjectStoreWrapper::softDelete(*this->getItem<BrewNote>(ndx));
            break;
         case TreeNode::Type::Water:
            ObjectStoreWrapper::softDelete(*this->getItem<Water>(ndx));
            break;
         case TreeNode::Type::Folder:
            // We want to delete the contents of the folder (and remove it from from the model) before remove the folder
            // itself, otherwise the QModelIndex values for the contents will not be valid.
            this->deleteSelected(allChildren(ndx));
            this->removeFolder(ndx);
            break;
      }
   }
   return;
}

// =========================================================================
// ============================ FOLDER STUFF ===============================
// =========================================================================

// The actual magic shouldn't be hard. Once we trap the signal, find the
// recipe, remove it from the parent and add it to the target folder.
// It is not easy. Indexes are ephemeral things. We MUST calculate the insert
// index after we have removed the recipe. BAD THINGS happen otherwise.
//
void TreeModel::folderChanged(QString name) {
   qDebug() << Q_FUNC_INFO << name;

   NamedEntity * test = qobject_cast<NamedEntity *>(sender());
   if (test) {
      this->folderChanged(test);
   }

   return;
}

void TreeModel::folderChanged(NamedEntity * test) {
   qDebug() << Q_FUNC_INFO << test;

   // Find it.
   QModelIndex ndx = findElement(test);
   if (! ndx.isValid()) {
      qWarning() << Q_FUNC_INFO << "Could not find element";
      return;
   }

   QModelIndex pIndex;
   bool expand = true;

   pIndex = parent(ndx); // Get the parent
   // If the parent isn't valid, its the root
   if (!pIndex.isValid()) {
      pIndex = createIndex(0, 0, rootItem->child(0));
   }

   int ii = item(ndx)->childNumber();
   // Remove it
   if (!this->removeRows(ii, 1, pIndex)) {
      qWarning() << Q_FUNC_INFO << "Could not remove row" << ii;
      return;
   }

   // Find the new parent
   // That's awkward, but dropping a folder prolly does need a the folder
   // created.
   //
   // TODO: At some point we should refactor this code so that we have separate handling for objects that have
   //       folders from ones that don't.
   auto folder = FolderUtils::getFolder(test);
   if (!folder || folder->isEmpty()) {
      return;
   }
   QModelIndex newNdx = findFolder(*folder, rootItem->child(0), true);
   if (! newNdx.isValid()) {
      newNdx = createIndex(0, 0, rootItem->child(0));
      expand = false;
   }

   TreeNode * local = item(newNdx);
   int jj = local->childCount();

   if (!insertRow(jj, newNdx, test, this->nodeType)) {
      qWarning() << Q_FUNC_INFO << "Could not insert row" << jj;
      return;
   }
   // If we have brewnotes, set them up here.
   if (m_treeMask & TreeModel::TypeMask::Recipe) {
      addBrewNoteSubTree(qobject_cast<Recipe *>(test), jj, local);
   }

   if (expand) {
      emit expandFolder(m_treeMask, newNdx);
   }
   return;
}

bool TreeModel::addFolder(QString name) {
   return findFolder(name, rootItem->child(0), true).isValid();
}

bool TreeModel::removeFolder(QModelIndex ndx) {
   if (! ndx.isValid()) {
      return false;
   }

   int i = -1;
   QModelIndex pInd = parent(ndx);

   if (! pInd.isValid()) {
      return false;
   }

   TreeNode * start = item(ndx);

   // Remove the victim.
   i = start->childNumber();
   return removeRows(i, 1, pInd);
}

QModelIndexList TreeModel::allChildren(QModelIndex ndx) {
   QModelIndexList leafNodes;

   // Don't send an invalid index or something that isn't a folder
   if (! ndx.isValid() || type(ndx) != TreeNode::Type::Folder) {
      return leafNodes;
   }

   TreeNode * start = item(ndx);
   QList<TreeNode *> folders;
   folders.append(start);

   while (! folders.isEmpty()) {
      TreeNode * target = folders.takeFirst();

      for (int i = 0; i < target->childCount(); ++i) {
         TreeNode * next = target->child(i);
         // If a folder, push it onto the folders stack for later processing
         if (next->type() == TreeNode::Type::Folder) {
            folders.append(next);
         } else { // Leafnode
            leafNodes.append(createIndex(i, 0, next));
         }
      }
   }
   return leafNodes;
}

bool TreeModel::renameFolder(Folder * victim, QString newName) {
   QModelIndex ndx = findFolder(victim->fullPath(), nullptr, false);
   QModelIndex pInd;
   QString targetPath = newName % "/" % victim->name();
   QPair<QString, TreeNode *> f;
   QList<QPair<QString, TreeNode *>> folders;
   // This space is important       ^
   int i, kids, src;

   if (! ndx.isValid()) {
      return false;
   }

   pInd = parent(ndx);
   if (! pInd.isValid()) {
      return false;
   }

   TreeNode * start = item(ndx);
   f.first  = targetPath;
   f.second = start;

   folders.append(f);

   while (! folders.isEmpty()) {
      // This looks weird, but it is needed for later
      f = folders.takeFirst();
      targetPath = f.first;
      TreeNode * target = f.second;

      // As we move things, childCount changes. This makes sure we loop
      // through all of the kids
      kids = target->childCount();
      src = 0;
      // Ok. We have a start and an index.
      for (i = 0; i < kids; ++i) {
         // This looks weird and it is. As we move children out, the 0 items
         // changes to the next child. In the case of a folder, though, we
         // don't move it, so we need to get the item beyond that.
         TreeNode * next = target->child(src);
         // If a folder, push it onto the folders stack for latter processing
         if (next->type() == TreeNode::Type::Folder) {
            QPair<QString, TreeNode *> newTarget;
            newTarget.first = targetPath % "/" % next->name();
            newTarget.second = next;
            folders.append(newTarget);
            src++;
         } else {
            // Leafnode
            //
            // TODO: At some point we should refactor this code so that we have separate handling for objects that have
            //       folders from ones that don't.
            auto item = next->thing();
            auto folder = FolderUtils::getFolder(item);
            if (folder) {
               FolderUtils::setFolder(item, targetPath);
            }
         }
      }
   }
   // Last thing is to remove the victim.
   i = start->childNumber();
   return removeRows(i, 1, pInd);
}

QModelIndex TreeModel::createFolderTree(QStringList dirs, TreeNode * parent, QString pPath) {
   TreeNode * pItem = parent;

   // Start the loop. We are going to return ndx at the end,
   // so we need to declare and initialize outside of the loop
   QModelIndex ndx = createIndex(pItem->childCount(), 0, pItem);

   // Need to call this because we are adding different things with different
   // column counts. Just using the rowsAboutToBeAdded throws ugly errors and
   // then a sigsegv
   emit layoutAboutToBeChanged();
   for (QString cur : dirs) {
      QString fPath;
      Folder * temp = new Folder();

      // If the parent item is a folder, use its full path
      if (pItem->type() == TreeNode::Type::Folder) {
         fPath = pItem->getData<Folder>()->fullPath() % "/" % cur;
      } else {
         fPath = pPath % "/" % cur;   // If it isn't we need the parent path
      }

      fPath.replace(QRegularExpression("//"), "/");

      // Set the full path, which will set the name and the path
      temp->setfullPath(fPath);
      int i = pItem->childCount();

      pItem->insertChildren(i, 1, TreeNode::Type::Folder);
      pItem->child(i)->setData(TreeNode::Type::Folder, temp);

      // Set the parent item to point to the newly created tree
      pItem = pItem->child(i);

      // And this for the return
      ndx = createIndex(pItem->childCount(), 0, pItem);
   }
   emit layoutChanged();

   // May K&R have mercy on my soul
   return ndx;
}

QModelIndex TreeModel::findFolder(QString name, TreeNode * parent, bool create) {
   TreeNode * pItem;
   QStringList dirs;
   QString current, fullPath, targetPath;
   int i;

   pItem = parent ? parent : rootItem->child(0);

   // Upstream interfaces should handle this for me, but I like belt and
   // suspenders
   name = name.simplified();
   // I am assuming asking me to find an empty name means find the root of the
   // tree.
   if (name.isEmpty()) {
      return createIndex(0, 0, pItem);
   }

   // Prepare all the variables for the first loop

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   dirs = name.split("/", QString::SkipEmptyParts);
#else
   dirs = name.split("/", Qt::SkipEmptyParts);
#endif

   if (dirs.isEmpty()) {
      return QModelIndex();
   }

   current = dirs.takeFirst();
   fullPath = "/";
   targetPath = fullPath % current;

   i = 0;

   // Time to get funky with no recursion!
   while (i < pItem->childCount()) {
      TreeNode * kid = pItem->child(i);
      // The kid is a folder
      if (kid->type() == TreeNode::Type::Folder) {
         // The folder name matches the part we are looking at
         if (kid->getData<Folder>()->isFolder(targetPath)) {
            // If there are no more subtrees to look for, we found it
            if (dirs.isEmpty()) {
               return createIndex(i, 0, kid);
            }
            // Otherwise, we found a parent folder in our path
            else {
               // get the next folder in the path
               current = dirs.takeFirst();
               // append that to the fullPath we are looking for
               fullPath = targetPath;
               targetPath = fullPath % "/" % current;

               // Set the parent to the folder
               pItem = kid;
               // Reset the counter
               i = 0;
               // And do the time warp again!
               continue;
            }
         }
      }
      // If we got this far, it wasn't a folder or it wasn't a match.
      i++;
   }
   // If we get here, we found no match.

   // If we are supposed to create something, then lets get busy
   if (create) {
      // push the current dir back on the stack
      dirs.prepend(current);
      // And start with the madness
      return createFolderTree(dirs, pItem, fullPath);
   }

   // If we weren't supposed to create, we drop to here and return an empty
   // index.
   return QModelIndex();
}

// =========================================================================
// ============================ SLOT STUFF ===============================
// =========================================================================

void TreeModel::elementChanged() {
   NamedEntity * d = qobject_cast<NamedEntity *>(sender());
   if (!d) {
      return;
   }

   QModelIndex ndxLeft = findElement(d);
   if (! ndxLeft.isValid()) {
      return;
   }

   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft) - 1, ndxLeft.internalPointer());
   emit dataChanged(ndxLeft, ndxRight);
}

/* I don't like this part, but Qt's signal/slot mechanism are pretty
 * simplistic and do a string compare on signatures. Each one of these one
 * liners is required to give the right signature and to be able to call
 * addElement() properly
 */
void TreeModel::elementAddedRecipe(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<Recipe     >(victimId)));
}
void TreeModel::elementAddedEquipment(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<Equipment  >(victimId)));
}
void TreeModel::elementAddedFermentable(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<Fermentable>(victimId)));
}
void TreeModel::elementAddedHop(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<Hop        >(victimId)));
}
void TreeModel::elementAddedMisc(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<Misc       >(victimId)));
}
void TreeModel::elementAddedStyle(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<Style      >(victimId)));
}
void TreeModel::elementAddedYeast(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<Yeast      >(victimId)));
}
void TreeModel::elementAddedBrewNote(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<BrewNote   >(victimId)));
}
void TreeModel::elementAddedWater(int victimId) {
   this->elementAdded(qobject_cast<NamedEntity *>(ObjectStoreWrapper::getByIdRaw<Water      >(victimId)));
}

// I guess this isn't too bad. Better than this same function copied 7 times
void TreeModel::elementAdded(NamedEntity * victim) {

   if (!victim->display()) {
      return;
   }

   QModelIndex pIdx;
   auto lType = this->nodeType;
   if (qobject_cast<BrewNote *>(victim)) {
      auto brewNote = qobject_cast<BrewNote *>(victim);
      Recipe * recipe = ObjectStoreWrapper::getByIdRaw<Recipe>(brewNote->recipeId());
      pIdx = findElement(recipe);
      lType = TreeNode::Type::BrewNote;
   } else {
      pIdx = createIndex(0, 0, rootItem->child(0));
   }

   if (!pIdx.isValid()) {
      return;
   }

   int breadth = rowCount(pIdx);
   if (!insertRow(breadth, pIdx, victim, lType)) {
      return;
   }

   // We need some special processing here to add brewnotes on a recipe import
   if (qobject_cast<Recipe *>(victim)) {
      Recipe * parent = qobject_cast<Recipe *>(victim);
      auto notes = parent->brewNotes();

      if (notes.size()) {
         pIdx = findElement(parent);
         lType = TreeNode::Type::BrewNote;
         int row = 0;
         for (auto note : notes) {
            insertRow(row++, pIdx, note.get(), lType);
         }
      }
   }
   observeElement(victim);
   return;
}

void TreeModel::elementRemovedRecipe([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}
void TreeModel::elementRemovedEquipment([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}
void TreeModel::elementRemovedFermentable([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}
void TreeModel::elementRemovedHop([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}
void TreeModel::elementRemovedMisc([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}
void TreeModel::elementRemovedStyle([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}
void TreeModel::elementRemovedYeast([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}
void TreeModel::elementRemovedBrewNote([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}
void TreeModel::elementRemovedWater([[maybe_unused]] int victimId, std::shared_ptr<QObject> victim) {
   this->elementRemoved(qobject_cast<NamedEntity *>(victim.get()));
}

void TreeModel::elementRemoved(NamedEntity * victim) {

   if (!victim) {
      return;
   }

   QModelIndex index = findElement(victim);
   if (!index.isValid()) {
      return;
   }

   QModelIndex pIndex = parent(index);
   if (!pIndex.isValid()) {
      return;
   }

   if (!this->removeRows(index.row(), 1, pIndex)) {
      return;
   }

   disconnect(victim, nullptr, this, nullptr);
   return;
}

void TreeModel::recipePropertyChanged(int recipeId, BtStringConst const & propertyName) {
   // If a Recipe's ancestor ID has changed then it might be because a new ancestor has been created
   // .:TBD:. We could probably get away with propertyName == PropertyNames::Recipe::ancestorId here because
   // we always use the same constants for property names.
   if (propertyName != PropertyNames::Recipe::ancestorId) {
      qDebug() << Q_FUNC_INFO << "Ignoring change to" << propertyName << "on Recipe" << recipeId;
      return;
   }

   Recipe * descendant = ObjectStoreWrapper::getByIdRaw<Recipe>(recipeId);
   if (!descendant) {
      qCritical() << Q_FUNC_INFO << "Unable to find Recipe" << recipeId << "to check its change to" << propertyName;
      return;
   }

   int ancestorId = descendant->getAncestorId();

   if (ancestorId <= 0 || ancestorId == descendant->key()) {
      qDebug() << Q_FUNC_INFO << "No ancestor (" << ancestorId << ") on Recipe" << recipeId;
      return;
   }

   Recipe * ancestor = ObjectStoreWrapper::getByIdRaw<Recipe>(ancestorId);
   if (!ancestor) {
      qCritical() << Q_FUNC_INFO << "Unable to find Recipe" << ancestorId << "set as ancestor to Recipe" << recipeId;
      return;
   }

   this->versionedRecipe(ancestor, descendant);

   return;
}


void TreeModel::observeElement(NamedEntity * d) {
   if (! d) {
      return;
   }

   if (qobject_cast<BrewNote *>(d)) {
      connect(qobject_cast<BrewNote *>(d), &BrewNote::brewDateChanged, this, &TreeModel::elementChanged);
   } else {
      connect(d, &NamedEntity::changedName,   this, &TreeModel::elementChanged);
      connect(d, &NamedEntity::changedFolder, this, static_cast<void (TreeModel::*)(QString)>(&TreeModel::folderChanged));
   }
}


// =========================================================================
// ===================== DRAG AND DROP STUFF ===============================
// =========================================================================
bool TreeModel::dropMimeData(QMimeData const * data,
                               [[maybe_unused]] Qt::DropAction action,
                               [[maybe_unused]] int row,
                               [[maybe_unused]] int column,
                               QModelIndex const & parent) {
   // See https://en.wikipedia.org/wiki/Media_type for more on MIME types (now called media types)
   qDebug() <<
      Q_FUNC_INFO << "MIME Data:" << (data ? data->text() : "NULL") << ".  "
      "Parent" << (parent.isValid() ? "valid" : "invalid");

   QByteArray encodedData;

   if (data->hasFormat(this->m_mimeType)) {
      encodedData = data->data(this->m_mimeType);
   } else if (data->hasFormat("application/x-brewtarget-folder")) {
      encodedData = data->data("application/x-brewtarget-folder");
   } else {
      qDebug() << Q_FUNC_INFO << "Unrecognised MIME type";
      return false;   // Don't know what we got, but we don't want it
   }

   if (!parent.isValid()) {
      return false;
   }

   qDebug() << Q_FUNC_INFO << "Parent row:" << parent.row() << ", column:" << parent.column();

   QString target = "";
   if (this->itemIs<Folder>(parent)) {
      target = this->getItem<Folder>(parent)->fullPath();
   } else {
      NamedEntity * something = this->thing(parent);

      // Did you know there's a space between elements in a tree, and you can
      // actually drop things there? If somebody drops something there, don't
      // do anything
      if (! something) {
         qDebug() << Q_FUNC_INFO << "Invalid drop location";
         return false;
      }
      auto folder = FolderUtils::getFolder(something);
      if (!folder) {
         qDebug() << Q_FUNC_INFO << "Dragged element" << something << "cannot have folder";
         return false;
      }
      target = *folder;
   }

   qDebug() << Q_FUNC_INFO << "Target:" << target;

   // Pull the stream apart and do that which needs done. Late binding ftw!
   for (QDataStream stream{&encodedData, QIODevice::ReadOnly}; !stream.atEnd(); ) {
      int oTypeRaw;
      int id;
      QString name = "";
      stream >> oTypeRaw >> id >> name;
      TreeNode::Type oType = static_cast<TreeNode::Type>(oTypeRaw);
      qDebug() << Q_FUNC_INFO << "Name:" << name << ", ID:" << id << ", Type:" << oTypeRaw;

      auto item = getElement(oType, id);
      auto folder = FolderUtils::getFolder(item);
      if (!folder) {
         qDebug() << Q_FUNC_INFO << "No matching element";
         return false;
      }

      // this is the work.
      if (oType != TreeNode::Type::Folder) {
         qDebug() <<
            Q_FUNC_INFO << "Moving" << item << "from folder" << folder << "to folder" << target;
         // Dropping an item in a folder just means setting the folder name on that item
         FolderUtils::setFolder(item, target);
         // Now we have to update our own model (ie that of TreeModel) so that the display will update!
         this->folderChanged(item);

      } else {
         // I need the actual folder object that got dropped.
         Folder * victim = new Folder;
         victim->setfullPath(name);

         renameFolder(victim, target);
      }
   }

   return true;
}

void TreeModel::revertRecipeToPreviousVersion(QModelIndex ndx) {
   TreeNode * node = item(ndx);
   TreeNode * pNode = node->parent();
   QModelIndex pIndex = parent(ndx);

   // The recipe referred to by the index is the one that's about to be deleted
   auto recipeToRevert = this->getItem<Recipe>(ndx);

   auto ancestor = recipeToRevert->revertToPreviousVersion();

   // don't do anything if there is nothing to do
   if (!ancestor) {
      return;
   }

   // Remove all the rows associated with the about-to-be-deleted Recipe
   removeRows(0, node->childCount(), ndx);

   // Put the ancestor into the tree
   if (! insertRow(pIndex.row(), pIndex, ancestor, TreeNode::Type::Recipe)) {
      qWarning() << Q_FUNC_INFO << "Could not add ancestor to tree";
   }

   // Find the ancestor in the tree
   QModelIndex ancNdx = findElement(ancestor);
   if (! ancNdx.isValid()) {
      qWarning() << Q_FUNC_INFO << "Couldn't find the ancestor";
   }

   // Add the ancestor's brewnotes to the descendant
   addBrewNoteSubTree(ancestor, ancNdx.row(), pNode);

   return;
}

// This is detaching a Recipe from its previous versions
void TreeModel::orphanRecipe(QModelIndex ndx) {
   TreeNode* node = item(ndx);
   TreeNode* pNode = node->parent();
   QModelIndex pIndex = parent(ndx);

   // I need the recipe referred to by the index
   auto orphan = this->getItem<Recipe>(ndx);

   // don't do anything if there is nothing to do
   if ( ! orphan->hasAncestors() ) {
      return;
   }

   // And I need its immediate ancestor
   auto ancestor = orphan->ancestors().at(0);

   // Deal with the soon-to-be orphan first
   // Remove all the rows associated with the orphan
   removeRows(0,node->childCount(),ndx);

   // This looks weird, but I think it will do what I need -- set
   // the ancestor_id to itself and reload the ancestors array. setAncestor
   // handles the locked and display flags
   orphan->setAncestor(*orphan);
   // Display all of its brewnotes
   addBrewNoteSubTree(orphan, ndx.row(), pNode, false);

   // set the ancestor to visible. Not sure this is required?
   ancestor->setDisplay(true);
   ancestor->setLocked(false);

   // Put the ancestor into the tree
   if ( ! insertRow(pIndex.row(), pIndex, ancestor, TreeNode::Type::Recipe) ) {
      qWarning() << Q_FUNC_INFO << "Could not add ancestor to tree";
   }

   // Find the ancestor in the tree
   QModelIndex ancNdx = findElement(ancestor);
   if ( ! ancNdx.isValid() ) {
      qWarning() << Q_FUNC_INFO << "Couldn't find the ancestor";
   }

   // Add the ancestor's brewnotes to the descendant
   addBrewNoteSubTree(ancestor,ancNdx.row(),pNode);

   return;
}

void TreeModel::spawnRecipe(QModelIndex ndx) {
   Q_ASSERT(ndx.isValid());
   auto ancestor = this->getItem<Recipe>(ndx);
   Q_ASSERT(ancestor);
   std::shared_ptr<Recipe> descendant = std::make_shared<Recipe>(*ancestor);
   // We want to store the new Recipe first so that it gets an ID...
   ObjectStoreWrapper::insert<Recipe>(descendant);
   // ...then we can connect it to the one it was copied from
   descendant->setAncestor(*ancestor);
   qDebug() <<
      Q_FUNC_INFO << "Created descendant Recipe" << descendant->key() << "of Recipe" << ancestor->key() <<
      "(at position" << ndx.row() << ")";

   //
   // First, we remove the ancestor from the tree
   //
   // NB: Inserting the descendant in the database will have generated a signal resulting in a call to
   // TreeModel::elementAddedRecipe(), so we can't assume that ndx is still valid, hence the reassignement here.
   //
   ndx = this->findElement(ancestor);
   if (!removeRows(ndx.row(), 1, this->parent(ndx))) {
      qCritical() << Q_FUNC_INFO << "Could not find Recipe" << ancestor->key() << "in display tree";
   }

   // Now we need to find the descendant in the tree. This has to be done
   // after we removed the ancestor row, otherwise the index will be wrong.
   QModelIndex decNdx = findElement(descendant.get());
   this->showAncestors(decNdx);

   emit dataChanged(decNdx, decNdx);
   emit recipeSpawn(descendant.get());

   return;
}

void TreeModel::versionedRecipe(Recipe * ancestor, Recipe * descendant) {
   qDebug() << Q_FUNC_INFO << "Updating tree now that Recipe" << descendant->key() << "has ancestor Recipe" << ancestor->key();

   // like before, remove the ancestor
   QModelIndex ndx = findElement(ancestor);
   if (!removeRows(ndx.row(), 1, this->parent(ndx))) {
      qCritical() << Q_FUNC_INFO << "Could not find Recipe" << ancestor->key() << "in display tree";
   }

   // add the descendant in, but get the index only after we removed the
   // ancestor
   QModelIndex decNdx = findElement(descendant);
   this->showAncestors(decNdx);

   // do not mess with this order. We have to signal the data is in the tree
   // first (dataChanged), then put it in the right folder (folderChanged) and
   // finally tell MainWindow something happened (recipeSpawn).
   // Any other order doesn't work, or dumps core
   emit dataChanged(decNdx, decNdx);
   this->folderChanged(descendant);
   emit recipeSpawn(descendant);
   return;
}


QStringList TreeModel::mimeTypes() const {
   QStringList types;
   // accept whatever type we like, and folders
   types << m_mimeType << "application/x-brewtarget-folder";

   return types;
}

Qt::DropActions TreeModel::supportedDropActions() const {
   return Qt::CopyAction | Qt::MoveAction;
}


// =========================================================================
// ===================== RECIPE VERSION STUFF ==============================
// =========================================================================
//
bool TreeModel::showChild(QModelIndex child) const {
   TreeNode * node = item(child);
   return node->showMe();
}

void TreeModel::setShowChild(QModelIndex child, bool val) {
   TreeNode * node = item(child);
   return node->setShowMe(val);
}

void TreeModel::showAncestors(QModelIndex ndx) {

   if (! ndx.isValid()) {
      return;
   }

   TreeNode * node = item(ndx);
   auto descendant = this->getItem<Recipe>(ndx);
   QList<Recipe *> ancestors = descendant->ancestors();

   removeRows(0, node->childCount(), ndx);

   // add the brewnotes for this version back
   addBrewNoteSubTree(descendant, ndx.row(), node->parent(), false);

   // set showChild on the leaf node. I use this for drawing menus
   setShowChild(ndx, true);

   // Now loop through the ancestors. The nature of the beast is nearest
   // ancestors are first
   for (auto ancestor : ancestors) {
      int j = node->childCount();
      if (! insertRow(j, ndx, ancestor, TreeNode::Type::Recipe)) {
         qWarning() << "Could not add ancestoral brewnotes";
      }
      QModelIndex cIndex = findElement(ancestor, node);
      setShowChild(cIndex, true);
      // ew, but apparently this has to happen here.
      emit dataChanged(cIndex, cIndex);

      // add the brewnotes to the ancestors, but make sure we don't recurse
      addBrewNoteSubTree(ancestor, j, node, false);
   }
   return;
}

void TreeModel::hideAncestors(QModelIndex ndx) {
   TreeNode * node = item(ndx);

   // This has no potential to be clever. None.
   if (! ndx.isValid()) {
      return;
   }

   // remove all the currently shown children
   removeRows(0, node->childCount(), ndx);
   auto descendant = this->getItem<Recipe>(ndx);

   // put the brewnotes back, including those from the ancestors.
   addBrewNoteSubTree(descendant, ndx.row(), node->parent());

   // This is for menus
   setShowChild(ndx, false);

   // Now we just need to mark each ancestor invisible again
   for (auto ancestor : descendant->ancestors()) {
      QModelIndex aIndex = findElement(ancestor, node);
      setShowChild(aIndex, false);
      emit dataChanged(aIndex, aIndex);
   }
   return;
}

// more cleverness must happen. Wonder if I can figure it out.
void TreeModel::catchAncestors(bool showem) {
   QModelIndex ndxLocal;
   TreeNode * local = nullptr;
   QList<NamedEntity *> elems = elements();

   foreach (NamedEntity * elem, elems) {
      Recipe * rec = qobject_cast<Recipe *>(elem);

      local = rootItem->child(0);
      ndxLocal = findElement(elem, local);

      if (rec->hasAncestors()) {
         showem ? showAncestors(ndxLocal) : hideAncestors(ndxLocal);
      }
   }
   return;
}
