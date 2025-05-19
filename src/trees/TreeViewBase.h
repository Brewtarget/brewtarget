/*======================================================================================================================
 * trees/TreeViewBase.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#ifndef TREES_TREEVIEWBASE_H
#define TREES_TREEVIEWBASE_H
#pragma once

#include <QDebug>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QString>
#include <QWidget>

#include "utils/CuriouslyRecurringTemplateBase.h"
#include "trees/NamedEntityTreeSortFilterProxyModel.h"

/**
 * \brief CRTP base for \c TreeView subclasses.  See comment on \c TreeView class for more info.
 *
 * \param Derived - The derived class
 * \param NeTreeModel - The corresponding sub-class of \c TreeModel and \c TreeModelBase (eg \c RecipeTreeModel)
 * \param NeTreeSortFilterProxyModel - The corresponding sub-class of \c QSortFilterProxyModel and
 *                                 \c TreeSortFilterProxyModelBase (eg \c RecipeTreeSortFilterProxyModel)
 * \param NeEditor - The corresponding subclass of \c EditorBase for creating/editing \c NE item, \b unless \c NE is
 *                   \c Recipe (for which no separate editor exists), in which case this type should be \c MainWindow.
 * \param NE - The primary \c NamedEntity subclass (besides \c Folder) shown in this tree (eg \c Recipe for
 *             \c RecipeTreeView)
 * \param SNE - The optional secondary \c NamedEntity subclass shown in this tree (eg \c BrewNote for
 *               \c RecipeTreeView, or \c MashStep for \c MashTreeView).  This class must have:
 *                 • an \c owner() member function that does the obvious thing (eg \c BrewNote::owner() returns a
 *                   \c Recipe; \c MashStep::owner returns a \c Mash);
 *                 • a static \c ownedBy() member function that returns all the \c BrewNote objects owned by a given
 *                   \c Recipe or all the \c MashStep objects owned by a given \c Mash, etc.
 */
template<class Derived> class TreeViewPhantom;
template<class Derived,
         class NeTreeModel,
         class NeTreeSortFilterProxyModel,
         class NeEditor,
         class NE,
         typename SNE = void>
class TreeViewBase : public CuriouslyRecurringTemplateBase<TreeViewPhantom, Derived> {
   friend Derived;
private:
   /**
    * \brief Constructor
    *
    *        Note that we pass nullptr as parent to m_model and m_treeSortFilterProxy constructors because they are value
    *        members of this class and we do not want them put in the QObject tree (which would try to call their
    *        destructors when parent is destroyed).
    */
   TreeViewBase() :
      m_model          {nullptr},
      m_treeSortFilterProxy{nullptr} {
      this->m_treeSortFilterProxy.setSourceModel(&this->m_model);
      this->m_treeSortFilterProxy.setDynamicSortFilter(true);
      this->m_treeSortFilterProxy.setFilterKeyColumn(1);
      this->derived().setModel(&this->m_treeSortFilterProxy);
      this->derived().connect(&this->m_model, &NeTreeModel::expandFolder, &this->derived(), &Derived::expandFolder);

      this->derived().setAllColumnsShowFocus(true);
      this->derived().setContextMenuPolicy(Qt::CustomContextMenu);

///      this->derived().setDragEnabled(true); // Already done in TreeView::TreeView
///      this->derived().setAcceptDrops(true); // Already done in TreeView::TreeView
///      this->derived().setDropIndicatorShown(true); // Already done in TreeView::TreeView
///      this->derived().setSelectionMode(QAbstractItemView::ExtendedSelection); // Already done in TreeView::TreeView

      this->derived().setExpanded(this->derived().getRootIndex(), true);
      this->derived().setSortingEnabled(true);
      this->derived().sortByColumn(0, Qt::AscendingOrder);
      this->derived().resizeColumnToContents(0);

      return;
   }

public:
   ~TreeViewBase() = default;

   void init(NeEditor & editor) {
      this->m_editor = &editor;

      this->m_contextMenu.addMenu(&this->m_contextMenu_new);
      this->m_contextMenu_new.setTitle(Derived::tr("New"));
      this->m_contextMenu_new.addAction(NE::localisedName()  , &this->derived(), &Derived::newItem);
      this->m_contextMenu_new.addAction(Derived::tr("Folder"), &this->derived(), &Derived::newFolder);
      this->m_contextMenu.addSeparator();
      // Copy
      this->m_copyAction   = m_contextMenu.addAction(Derived::tr("Copy"  ), &this->derived(), &Derived::copySelected  );
      // m_deleteAction makes it easier to find this later to disable it
      this->m_deleteAction = m_contextMenu.addAction(Derived::tr("Delete"), &this->derived(), &Derived::deleteSelected);
      // export and import
      this->m_contextMenu.addSeparator();
      this->m_exportMenu.setTitle(Derived::tr("Export"));
      this->m_exportMenu.addAction(Derived::tr("To File (BeerXML or BeerJSON)"), &this->derived(), &Derived::exportSelected);
//    this->m_exportMenu.addAction(Derived::tr("To HTML"), &this->derived(), &Derived::exportSelectedHtml());
      this->m_contextMenu.addMenu(&this->m_exportMenu);
      this->m_contextMenu.addAction(Derived::tr("Import"), &this->derived(), &Derived::importFiles);

      this->derived().connect(&this->derived(), &QAbstractItemView::doubleClicked   , &this->derived(), &Derived::activated  );
      this->derived().connect(&this->derived(), &QWidget::customContextMenuRequested, &this->derived(), &Derived::contextMenu);

      return;
   }

protected:
   TreeModel & doTreeModel() {
      return this->m_model;
   }

   TreeNode * doTreeNode(QModelIndex const & viewIndex) const {
      QModelIndex const modelIndex = this->m_treeSortFilterProxy.mapToSource(viewIndex);
      return this->m_model.treeNode(modelIndex);
   }

   void doActivated(QModelIndex const & viewIndex) {
      TreeNode * node = this->m_model.treeNode(this->m_treeSortFilterProxy.mapToSource(viewIndex));
      if (!node) {
         qWarning() << Q_FUNC_INFO << "No node at viewIndex" << viewIndex;
         return;
      }

      if (node->classifier() == TreeNodeClassifier::Folder) {
         // default behavior is fine, but no warning
         qDebug() << Q_FUNC_INFO << "Folder";
         return;
      }

      if constexpr (std::same_as<NE, Recipe>) {
         //
         // Recipe trees can hold Recipes and BrewNotes
         //
         if (node->classifier() == TreeNodeClassifier::PrimaryItem) {
            auto recipe = this->getItem<Recipe>(viewIndex);
            this->m_editor->setRecipe(recipe.get());
            this->derived().setCurrentIndex(viewIndex);
         } else {
            Q_ASSERT(node->classifier() == TreeNodeClassifier::SecondaryItem);
            this->m_editor->setBrewNoteByIndex(viewIndex);
         }
      } else {
         auto item = this->getItem<NE>(viewIndex);
         if (item) {
            this->m_editor->setEditItem(item);
            this->m_editor->show();
         }
      }
      return;
   }

   void doContextMenu(QPoint const & point) {
      QModelIndex selectedViewIndex = this->derived().indexAt(point);
      if (!selectedViewIndex.isValid()) {
         return;
      }

      QMenu * tempMenu = this->derived().getContextMenu(selectedViewIndex);
      if (tempMenu) {
         tempMenu->exec(this->derived().mapToGlobal(point));
      }
      return;

   }

public:
   template<class T>
   std::shared_ptr<T> getItem(QModelIndex const & viewIndex) const {
      if (!viewIndex.isValid()) {
         return nullptr;
      }
      QModelIndex const modelIndex = this->m_treeSortFilterProxy.mapToSource(viewIndex);
      TreeNode * treeNode = this->m_model.treeNode(modelIndex);
      if (treeNode->classifier() == TreeNodeClassifier::Folder) {
         return nullptr;
      }
      if constexpr (std::same_as<NE, T>) {
         if (treeNode->classifier() != TreeNodeClassifier::PrimaryItem) {
            return nullptr;
         }
         TreeItemNode<NE> const & primaryTreeNode = static_cast<TreeItemNode<NE> &>(*treeNode);
         return primaryTreeNode.underlyingItem();
      } else if constexpr (!IsVoid<SNE>) {
         static_assert(std::same_as<SNE, T>);
         if (treeNode->classifier() != TreeNodeClassifier::SecondaryItem) {
            return nullptr;
         }
         TreeItemNode<SNE> const & primaryTreeNode = static_cast<TreeItemNode<SNE> &>(*treeNode);
         return primaryTreeNode.underlyingItem();
      }
   }

   QModelIndex parentIndex(QModelIndex const & viewIndex) {
      if (!viewIndex.isValid()) {
         return QModelIndex();
      }
      QModelIndex const modelIndex = this->m_treeSortFilterProxy.mapToSource(viewIndex);
      if (modelIndex.isValid()) {
         qDebug() << Q_FUNC_INFO << "modelIndex:" << modelIndex;
         return this->m_treeSortFilterProxy.mapFromSource(this->m_model.parent(modelIndex));
      }

      return QModelIndex();
   }

   QModelIndex getRootIndex() {
      return this->m_treeSortFilterProxy.mapFromSource(this->m_model.getRootIndex());
   }

   QModelIndex findElement(NE const * ne) {
      qDebug() << Q_FUNC_INFO << *ne;
      return this->m_treeSortFilterProxy.mapFromSource(this->m_model.findElement(ne));
   }

   //
   // See comment on TreeModelBase::findElement for why we cannot just use `SNE const &` as the parameter type
   //
   QModelIndex findElement(SNE const * sne) requires (!IsVoid<SNE>) {
      qDebug() << Q_FUNC_INFO << *sne;
      return this->m_treeSortFilterProxy.mapFromSource(this->m_model.findElement(sne));
   }

   /**
    * \brief Ask the user for a new name for an item we are about to copy
    *
    * \return The new name for the copy, or empty string if this item is to be skipped or \c std::nullopt to abort
    *         copying all selected items.
    */
   std::optional<QString> askCopyConfirmation(QString currentItemName) {
      QInputDialog copyConfirmationDialog;

      // Gotta build this hard, so we can say "cancel all"
      copyConfirmationDialog.setWindowTitle(Derived::tr("Copy %1").arg(NE::localisedName().toStdString().c_str()));
      copyConfirmationDialog.setLabelText(Derived::tr("Enter a unique name for the copy of %1.").arg(currentItemName));
      copyConfirmationDialog.setToolTip(Derived::tr("An empty name will skip copying this %1.").arg(NE::localisedName().toStdString().c_str()));
      copyConfirmationDialog.setCancelButtonText(Derived::tr("Cancel All"));

      if (copyConfirmationDialog.exec() == QDialog::Accepted) {
         return copyConfirmationDialog.textValue();
      }

      return std::nullopt;
   }

   /**
    * \brief Some subclasses need to override this, but we don't need to make it virtual because we're never calling it
    *        from a base class pointer/reference.
    */
   void doSetSelected(QModelIndex const & viewIndex) {
      QModelIndex parentIndex = this->parentIndex(viewIndex);
      this->derived().setCurrentIndex(viewIndex);
      QModelIndex modelIndex = this->m_treeSortFilterProxy.mapToSource(viewIndex);
      TreeNode * treeNode = this->m_model.treeNode(modelIndex);
      if (treeNode->classifier() == TreeNodeClassifier::Folder && !this->derived().isExpanded(parentIndex)) {
         this->derived().setExpanded(parentIndex, true);
      }
      this->derived().scrollTo(viewIndex, QAbstractItemView::PositionAtCenter);
      return;
   }

   /**
    * \brief Copy selected items
    *
    *        Note that we only copy primary items.  It doesn't make sense to copy secondary items (because they belong
    *        to primary items -- eg you wouldn't copy a \c BrewNote or a \c MashStep in the tree view).  We also don't
    *        support copying folders.  (It could be a future enhancement, but I'm not sure there's much need for it.)
    */
   void doCopy(QModelIndexList const & selectedViewIndexes) {
      QList<std::pair<QModelIndex, QString>> modelIndexToNewName;

      for (QModelIndex viewIndex : selectedViewIndexes) {
         QModelIndex modelIndex = this->m_treeSortFilterProxy.mapToSource(viewIndex);
         TreeNode * treeNode = this->m_model.treeNode(modelIndex);
         if (treeNode->classifier() != TreeNodeClassifier::PrimaryItem) {
            // Only support copying primary items, so skip any secondary ones or folders
            continue;
         }

         std::optional<QString> newItemName = this->askCopyConfirmation(treeNode->name());

         if (!newItemName) {
            // Null return means user clicked "Cancel All"
            return;
         }

         // Empty name just means skip current item
         if (newItemName->isEmpty()) {
            continue;
         }

         modelIndexToNewName.append(std::make_pair(modelIndex, *newItemName));
      }

      // If we get here, call the model to do the copy
      this->m_model.copyItems(modelIndexToNewName);
      return;
   }

   /**
    * \brief Delete the selected nodes (and their contents) and return the closest node that could become the new
    *        selected one.  Eg, if nodes [n, m] are to be deleted then, if node m+1 exists, it will become node n after
    *        the deletion and thus the new current selection.  If there is no node m+1, then node n-1, if it exists,
    *        will become the selected one.  (The actual logic is a bit more complicated than this because we have to
    *        account for folders, but this gives the idea.)
    *
    * \return \c std::nullopt if the user cancelled the deletion
    */
   std::optional<QModelIndex> doDeleteItems(QModelIndexList const & selectedViewIndexes) {
      // QModelIndexList is a synonym for QList<QModelIndex>
      // We have to grab this info here, as it won't necessarily be valid after we've modified the structure of the tree
      int firstRowToDelete = selectedViewIndexes.at(0).row();
      int firstColToDelete = selectedViewIndexes.at(0).column();

      QModelIndexList modelIndexesToDelete;

      auto confirmDelete = QMessageBox::NoButton;

      // Time to lay down the boogie
      for (QModelIndex viewIndex : selectedViewIndexes) {
         QModelIndex modelIndex = m_treeSortFilterProxy.mapToSource(viewIndex);
         TreeNode * treeNode = this->m_model.treeNode(modelIndex);
         if (!treeNode->rawParent()) {
            // You can't delete the root element
            continue;
         }

         // If we have already said "Yes To All", we skip asking for subsequent items
         if (confirmDelete != QMessageBox::YesToAll) {
            confirmDelete = QMessageBox::question(
               &this->derived(),
               Derived::tr("Delete %1").arg(treeNode->localisedClassName()),
               Derived::tr("Delete %1 %2?").arg(treeNode->localisedClassName()).arg(treeNode->name()),
               QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::Cancel,
               QMessageBox::No
            );
         }

         if (confirmDelete == QMessageBox::Cancel) {
            // Cancel means abort the entire operation
            return std::nullopt;
         }

         if (confirmDelete == QMessageBox::No) {
            // No just means skip the current item
            continue;
         }

         modelIndexesToDelete.append(modelIndex);
      }

      // If we get here, call the model to delete the victims
      this->m_model.deleteItems(modelIndexesToDelete);

      //
      // This is a stab at giving a new "currently selected" item after the previously selected items were deleted.
      //
      // TODO: There is a bit more logic to add here to cover folders and sub-types (via TreeNode::classifier()).  Original
      //       comments from MainWindow::deleteSelected() below
      //
      // Now that we deleted the selected recipe, we don't want it to appear in the main window any more, so let's select
      // another one.
      //
      // Most of the time, after deleting the nth recipe, the new nth item is also a recipe.  If there isn't an nth item
      // (eg because the recipe(s) we deleted were at the end of the list) then let's go back to the 1st item.  But then
      // we have to make sure to skip over folders.
      //
      // .:TBD:. This works if you have plenty of recipes outside folders.  If all your recipes are inside folders, then
      // we should so a proper search through the tree to find the first recipe and then expand the folder that it's in.
      // Doesn't feel like that logic belongs here.  Would be better to create TreeView::firstNonFolder() or similar.
      //
      //
      QModelIndex newSelectediewIndex = this->m_treeSortFilterProxy.index(firstRowToDelete, firstColToDelete);
      if (!newSelectediewIndex.isValid()) {
         newSelectediewIndex = this->m_treeSortFilterProxy.index(0, 0);
      }
      return newSelectediewIndex;
   }

   void doCopySelected() {
      QModelIndexList selected = this->derived().selectionModel()->selectedRows();
      this->doCopy(selected);
      return;
   }

   void doDeleteSelected() {
      QModelIndexList selected = this->derived().selectionModel()->selectedRows();

      QModelIndex start = selected.first();
      qDebug() << Q_FUNC_INFO << "Delete starting from row" << start.row();
      auto newSelected = this->doDeleteItems(selected);

      if (newSelected && newSelected->isValid()) {
         TreeNode * node = this->m_model.doTreeNode(*newSelected);
         qDebug() << Q_FUNC_INFO << "Row" << start.row() << "is" << node->className();
         this->derived().setSelected(*newSelected);
      }

      return;
   }

   void doRenameSelected() {
      // I don't think I can figure out what the behavior will be if you select many items
      QModelIndexList indexes = this->derived().selectionModel()->selectedRows();
      if (indexes.size() == 0) {
         return;
      }

      QModelIndex startIndex = indexes.at(0);

      TreeNode * treeNode = this->m_model.treeNode(startIndex);
      if (!treeNode->rawParent()) {
         // You can't rename the root element
         return;
      }

      // Don't rename anything other than a folder
      if (treeNode->classifier() != TreeNodeClassifier::Folder) {
         return;
      }

      TreeFolderNode<NE> & treeFolderNode = static_cast<TreeFolderNode<NE> &>(*treeNode);

      auto folder = treeFolderNode.underlyingItem();
      QString newName = QInputDialog::getText(&this->derived(),
                                              Derived::tr("Folder name"),
                                              Derived::tr("Folder name:"),
                                              QLineEdit::Normal,
                                              folder->name());

      // User clicks cancel
      if (newName.isEmpty()) {
         return;
      }
      // Do some input validation here.

      // Nice little builtin to collapse leading and following white space
      newName = newName.simplified();
      if (newName.isEmpty()) {
         QMessageBox::critical(&this->derived(),
                               Derived::tr("Bad Name"),
                               Derived::tr("A folder name must have at least one non-whitespace character in it"));
         return;
      }

      Qt::SplitBehaviorFlags const skip = Qt::SkipEmptyParts;

      if (newName.split("/", skip).isEmpty()) {
         QMessageBox::critical(&this->derived(),
                               Derived::tr("Bad Name"),
                               Derived::tr("A folder name must have at least one non-/ character in it"));
         return;
      }
      newName = folder->path() % "/" % newName;

      // Delegate this work to the tree.
      this->m_model.renameFolder(*folder, newName);

      return;
   }

   void doAddFolder(QString const & folder) {
      this->m_model.addFolder(folder);
      return;
   }

   QString doFolderName(QModelIndex const & viewIndex) const {
      QModelIndex const modelIndex{this->m_treeSortFilterProxy.mapToSource(viewIndex)};
      return this->m_model.folderPath(modelIndex);
   }

   //! \brief Create a new folder
   void doNewFolder() {
      QModelIndexList indexes = this->derived().selectionModel()->selectedRows();
      QModelIndex starter = indexes.at(0);

      // Where to start from
      QString dPath = this->doFolderName(starter);
      QString name = QInputDialog::getText(&this->derived(),
                                           Derived::tr("Folder name"),
                                           Derived::tr("Folder name:"),
                                           QLineEdit::Normal,
                                           dPath);
      // User clicks cancel
      if (name.isEmpty()) {
         return;
      }
      // Do some input validation here.

      // Nice little builtin to collapse leading and following white space
      name = name.simplified();
      if (name.isEmpty()) {
         QMessageBox::critical(&this->derived(),
                               Derived::tr("Bad Name"),
                               Derived::tr("A folder name must have at least one non-whitespace character in it"));
         return;
      }

      Qt::SplitBehaviorFlags const skip = Qt::SkipEmptyParts;
      if (name.split("/", skip).isEmpty()) {
         QMessageBox::critical(&this->derived(),
                               Derived::tr("Bad Name"),
                               Derived::tr("A folder name must have at least one non-/ character in it"));
         return;
      }
      this->doAddFolder(name);
      return;
   }

   void doExpandFolder(NeTreeModel const * const sender, QModelIndex const & index) {
      if (sender != &this->m_model) {
         // This shouldn't happen, because we should only be listening for signals from our model, but it's harmless if
         // it does.
         qWarning() << Q_FUNC_INFO << "Ignoring expandFolder signal not sent from our model";
         return;
      }

      if (index.isValid()) {
         qDebug() << Q_FUNC_INFO << "index:" << index;
         QModelIndex const translatedIndex{this->m_treeSortFilterProxy.mapFromSource(index)};
         if (translatedIndex.isValid() && !this->derived().isExpanded(translatedIndex)) {
            this->derived().expand(translatedIndex);
         }
      }

      return;
   }

   void doNewItem(QString const & folderPath) {
      if constexpr (std::same_as<NE, Recipe>) {
         // MainWindow::newRecipeInFolder
         this->m_editor->newRecipeInFolder(folderPath);
      } else {
         this->m_editor->newEditItem(folderPath);
      }
      return;
   }

   void doNewItem() {
      QModelIndexList const indexes = this->derived().selectionModel()->selectedRows();
      // This is a little weird. There is an edge case where nothing is selected and you click the big blue + button.
      QString folderPath;
      if (indexes.size() > 0) {
         folderPath = this->doFolderName(indexes.at(0));
      }

      this->doNewItem(folderPath);
      return;
   }

   std::pair<QMenu *, TreeNode *> getContextMenuPair(QModelIndex const & selectedViewIndex) {
      QModelIndex const selectedModelIndex{this->m_treeSortFilterProxy.mapToSource(selectedViewIndex)};
      TreeNode * selectedNode = this->m_model.treeNode(selectedModelIndex);
      if constexpr (!IsVoid<SNE>) {
         if (selectedNode->classifier() == TreeNodeClassifier::SecondaryItem) {
            return std::make_pair(&this->m_secondaryContextMenu, selectedNode);
         }
      }

      return std::make_pair(&this->m_contextMenu, selectedNode);
   }

   /**
    * \brief Some subclasses need to override this, but we don't need to make it virtual because we're never calling it
    *        from a base class pointer/reference.
    */
   QMenu * doGetContextMenu(QModelIndex const & selectedViewIndex) {
      auto [menu, selectedNode] = this->getContextMenuPair(selectedViewIndex);
      return menu;
   }

   //================================================ Member Variables =================================================
   NeTreeModel            m_model;
   NeTreeSortFilterProxyModel m_treeSortFilterProxy;
   NeEditor             * m_editor = nullptr;
   // These are the common parts of the right-click menu, initialised in doInit()
   QMenu   m_contextMenu     = QMenu{};
   QMenu   m_contextMenu_new = QMenu{};
   QMenu   m_exportMenu      = QMenu{};
   QAction * m_copyAction   = nullptr;
   QAction * m_deleteAction = nullptr;

   // We use the same trick here as in TreeNodeBase to have a member variable that only exists when there is a secondary
   // item (eg BrewNote in RecipeTreeView or MashStep in MashTreeView).
   struct Empty { };
   [[no_unique_address]] std::conditional_t<IsVoid<SNE>, Empty, QMenu> m_secondaryContextMenu;
};

class MainWindow;
//
// This is a horrible trick to make the macros below work
//
using RecipeEditor = MainWindow;

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define TREE_VIEW_COMMON_DECL(NeName, ...) \
   /* This allows TableViewBase to call protected and private members of Derived */   \
   friend class TreeViewBase<NeName##TreeView,                                        \
                             NeName##TreeModel,                                       \
                             NeName##TreeSortFilterProxyModel,                        \
                             NeName##Editor,                                          \
                             NeName __VA_OPT__(, __VA_ARGS__)>;                       \
                                                                                      \
   public:                                                                            \
      NeName##TreeView(QWidget * parent = nullptr);                                   \
      virtual ~NeName##TreeView();                                                    \
                                                                                      \
      virtual void setSelected(QModelIndex const & index) override;                   \
      virtual QMenu * getContextMenu(QModelIndex const & selectedViewIndex) override; \
      virtual TreeModel & treeModel() override;                                       \
      virtual TreeNode * treeNode(QModelIndex const & index) const override;          \
      virtual void copy(QModelIndexList const & selectedViewIndexes) override;        \
      virtual std::optional<QModelIndex> deleteItems(QModelIndexList const & selectedViewIndexes) override; \
      virtual void   copySelected() override;                                         \
      virtual void deleteSelected() override;                                         \
      virtual void renameSelected() override;                                         \
      virtual void addFolder(QString const & folder) override;                        \
      virtual QString folderName(QModelIndex const & viewIndex) const override;       \
                                                                                      \
   public slots:                                                                      \
      virtual void activated(QModelIndex const & viewIndex) override;                 \
      void contextMenu(QPoint const & point);                                         \
                                                                                      \
   private slots:                                                                     \
      void newFolder();                                                               \
      void expandFolder(QModelIndex const & viewIndex);                               \
      void newItem();                                                                 \

/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        NB: Mostly I have tried to make these macro-included function bodies trivial.  Macros are a bit clunky, so we
 *            only really want to use them for the things that are hard to do other ways.
 */
#define TREE_VIEW_COMMON_CODE(NeName, ...) \
   NeName##TreeView::NeName##TreeView(QWidget * parent) :                            \
      TreeView{parent},                                                              \
      TreeViewBase<NeName##TreeView,                                                 \
                   NeName##TreeModel,                                                \
                   NeName##TreeSortFilterProxyModel,                                 \
                   NeName##Editor,                                                   \
                   NeName __VA_OPT__(, __VA_ARGS__)>{} {                             \
         /*this->connectSignalsAndSlots();*/                                         \
         return;                                                                     \
      }                                                                              \
   NeName##TreeView::~NeName##TreeView() = default;                                  \
                                                                                     \
   void NeName##TreeView::setSelected(QModelIndex const & index) {                   \
      this->doSetSelected(index);                                                    \
      return;                                                                        \
   }                                                                                 \
   QMenu * NeName##TreeView::getContextMenu(QModelIndex const & selectedViewIndex) { \
      return this->doGetContextMenu(selectedViewIndex);                              \
   }                                                                                 \
   TreeModel & NeName##TreeView::treeModel() { return this->doTreeModel(); }         \
   TreeNode * NeName##TreeView::treeNode(QModelIndex const & index) const {          \
      return this->doTreeNode(index);                                                \
   }                                                                                 \
   void NeName##TreeView::copy(QModelIndexList const & selectedViewIndexes) {        \
      this->doCopy(selectedViewIndexes);                                             \
      return;                                                                        \
   }                                                                                 \
   std::optional<QModelIndex> NeName##TreeView::deleteItems(QModelIndexList const & selectedViewIndexes) {  \
      return this->doDeleteItems(selectedViewIndexes);                               \
   }                                                                                 \
   void NeName##TreeView::  copySelected() { this->  doCopySelected(); return; }     \
   void NeName##TreeView::deleteSelected() { this->doDeleteSelected(); return; }     \
   void NeName##TreeView::renameSelected() { this->doRenameSelected(); return; }     \
   void NeName##TreeView::addFolder(QString const & folder) {                        \
      this->doAddFolder(folder);                                                     \
      return;                                                                        \
      }                                                                              \
   QString NeName##TreeView::folderName(QModelIndex const & viewIndex) const {       \
      return this->doFolderName(viewIndex);                                          \
   }                                                                                 \
                                                                                     \
   void NeName##TreeView::activated(QModelIndex const & viewIndex) {                 \
      this->doActivated(viewIndex);                                                  \
      return;                                                                        \
   }                                                                                 \
   void NeName##TreeView::contextMenu(QPoint const & point) {                        \
      this->doContextMenu(point);                                                    \
      return;                                                                        \
   }                                                                                 \
                                                                                     \
   void NeName##TreeView::newFolder() {                                                    \
      this->doNewFolder();                                                                 \
      return;                                                                              \
   }                                                                                       \
   void NeName##TreeView::expandFolder(QModelIndex const & viewIndex) {                    \
      this->doExpandFolder(qobject_cast<NeName##TreeModel *>(this->sender()), viewIndex);  \
      return;                                                                              \
   }                                                                                       \
   void NeName##TreeView::newItem() { this->doNewItem(); return; }                         \

#endif
