/*======================================================================================================================
 * widgets/CommonContextMenus.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef WIDGETS_COMMONCONTEXTMENUS_H
#define WIDGETS_COMMONCONTEXTMENUS_H
#pragma once

#include <concepts>
#include <type_traits>

#include <QAction>
#include <QDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

#include "database/ObjectStoreUtils.h"
#include "model/Recipe.h"
#include "serialization/ImportExport.h"
#include "undoRedo/Undoable.h"
#include "utils/BtStringStream.h"
#include "utils/PropertyHelper.h"

class BrewNote;
class Folder;
class Ingredient;
class Salt;
class StockPurchase;
class TreeView;

namespace CommonContextMenuHelper {
   // See .cpp file for implementation.
   template<class NE> void doAddToOrSetForRecipe(std::shared_ptr<NE> selected);

   /**
    * \brief Used to improve readability when passing counts around.
    */
   template<class NE, class SNE = void>
   struct Selected {
      struct Empty { };
      int numPrimary = 0;
      int numSecondary = 0;
      int numFolders = 0;
      std::shared_ptr<NE> firstPrimary = nullptr;
      [[no_unique_address]] std::conditional_t<!std::is_same_v<NE, Recipe>, Empty, bool> fpAncestorsVisible {};
      [[no_unique_address]] std::conditional_t<IsVoid<SNE>, Empty, std::shared_ptr<SNE>> firstSecondary {};
   };

}

/**
 * @brief This class holds and manages common context menus, currently used by \c TreeViewBase and \c CatalogBase
 *
 * @tparam Derived The GUI class using this context menu -- eg \c FermentationTreeView, \c StyleCatalog
 * @tparam NE  The main type of NamedEntity being shown in this tree or catalog -- eg \c Equipment, \c Recipe, \c Hop.
 * @tparam SNE If not \c void then the secondary type of NamedEntity being shown in this tree -- eg \c BrewNote,
 *             \c MashStep.
 * @tparam showFolderOptions Whether to show options such as "new folder".  Essentially the same as
 *                           \c std::derived_from<Derived, TreeView> but we can't use that as \c Derived is not a
 *                           complete type at this point
 */
template<class Derived, class NE, class SNE, bool showFolderOptions>
struct CommonContextMenus {

   CommonContextMenus(Derived & derived) :
      m_derived{derived} {

      //
      // Although we could add all actions directly, having separate QAction objects allows us to disable them, eg if
      // nothing is selected.  Some things (eg new) don't need to be disabled, but it seems neater to do all the actions
      // the same way.
      //

      this->m_menu_primary.addMenu(&this->m_menu_new);
      this->addAndConnect(this->m_menu_new, this->m_action_newItem, &Derived::newItem);
      // We only show the new folder option in tree views
      if constexpr (showFolderOptions) {
         this->addAndConnect(this->m_menu_new, this->m_action_newFolder, &Derived::newFolder);
      }
      if constexpr (std::derived_from<NE, Ingredient>) {
         this->addAndConnect(this->m_menu_new, this->m_action_newStockPurchase, &Derived::newStockPurchase);
      }

      if constexpr (!std::is_same_v<NE, Recipe>) {
         this->m_menu_primary.addSeparator();
         this->addAndConnect(this->m_menu_primary, this->m_action_addToRecipe, &Derived::addSelectedToRecipe);
      }

      this->m_menu_primary.addSeparator();
      if constexpr (!std::is_same_v<NE, Recipe>) {
         this->addAndConnect(this->m_menu_primary, this->m_action_edit, &Derived::editSelected);
      }
      this->addAndConnect(this->m_menu_primary, this->m_action_copy  , &Derived::  copySelected);
      this->addAndConnect(this->m_menu_primary, this->m_action_delete, &Derived::deleteSelected);
      this->addAndConnect(this->m_menu_primary, this->m_action_rename, &Derived::renameSelected);

      if constexpr (std::derived_from<NE, Ingredient>) {
         this->m_menu_primary.addSeparator();
         this->addAndConnect(this->m_menu_primary, this->m_action_showStockPurchases, &Derived::showStockPurchases);
      }

      if constexpr (!std::is_same_v<NE, Salt>) {
         this->m_menu_primary.addSeparator();

         // TBD: Export is sub-menu because we used to have export to HTML as an option.  We should look at whether/when
         //      to bring that back.  Perhaps add export to PDF too?  Obvious link with printing...
         this->m_menu_primary.addMenu(&this->m_menu_export);
         this->addAndConnect(this->m_menu_export, this->m_action_exportToXmlOrJson, &Derived::exportSelected);

         this->addAndConnect(this->m_menu_primary, this->m_action_import, &Derived::importFromFiles);
      }

      //
      // Although "merge" logically belongs with New/Copy/Edit/Delete, it's a bit of an edge case (mostly for people
      // migrating from old versions of the software), so we put it at the bottom.
      //
      if constexpr (!std::disjunction_v<std::is_same<NE, Recipe>, std::is_base_of<StockPurchase, NE>>) {
         this->m_menu_primary.addSeparator();
         this->addAndConnect(this->m_menu_primary, this->m_action_merge, &Derived::mergeSelected);
      }

      if constexpr (std::is_same_v<NE, Recipe>) {
         this->m_menu_primary.addSeparator();
         this->m_menu_primary.addMenu(&this->m_menu_versioning);
         this->addAndConnect(this->m_menu_versioning, this->m_action_showAncestors, &Derived::showAncestors);
         this->addAndConnect(this->m_menu_versioning, this->m_action_hideAncestors, &Derived::hideAncestors);
         this->addAndConnect(this->m_menu_versioning, this->m_action_orphanRecipe , &Derived::orphanRecipe );
         this->addAndConnect(this->m_menu_versioning, this->m_action_spawnRecipe  , &Derived::spawnRecipe  );

         this->addAndConnect(this->m_menu_primary, this->m_action_brewIt, &Derived::brewItHelper);
      }

      if constexpr (!IsVoid<SNE>) {
         if constexpr (std::is_same_v<SNE, BrewNote>) {
            this->addAndConnect(this->m_menu_secondary, this->m_action_brewAgain       , &Derived::brewAgainHelper);
            this->addAndConnect(this->m_menu_secondary, this->m_action_changeBrewDate  , &Derived::changeBrewDate );
            this->addAndConnect(this->m_menu_secondary, this->m_action_recalcEfficiency, &Derived::fixBrewNote    );
         }
         this->addAndConnect(this->m_menu_secondary, this->m_action_deleteSecondary    , &Derived::deleteSelected);
      }

      this->retranslateUi();

      // Setting Qt::CustomContextMenu here causes the signal customContextMenuRequested() to be emitted when the user
      // requests the context menu (by right-clicking).
      this->m_derived.setContextMenuPolicy(Qt::CustomContextMenu);
      //
      // Note that caller/owner should NOT do this connection, otherwise the signal will get sent twice and the menu
      // won't exit until you've done two actions (or the same action twice).
      //
      this->m_derived.connect(&this->m_derived,
                              &QWidget::customContextMenuRequested,
                              &this->m_derived,
                              &Derived::contextMenu);

      return;
   }

   ~CommonContextMenus() = default;

private:
   template<typename Action, typename Slot>
   void addAndConnect(QMenu & menu, Action & action, Slot slot) {
      menu.addAction(&action);
      this->m_derived.connect(&action, &QAction::triggered, &this->m_derived, slot);
      return;
   }

public:

   void retranslateUi() {

      this->m_menu_new.setTitle(Derived::tr("New"));
      this->m_action_newItem.setText(NE::localisedName());
      if constexpr (showFolderOptions) {
         this->m_action_newFolder.setText(Derived::tr("Folder"));
      }
      if constexpr (std::derived_from<NE, Ingredient>) {
         this->m_action_newStockPurchase.setText(Derived::tr("Stock Purchase"));
      }

      if constexpr (!std::is_same_v<NE, Recipe>) {
         if constexpr (std::derived_from<NE, Ingredient>) {
            this->m_action_addToRecipe.setText(Derived::tr("Add to recipe"));
         } else {
            this->m_action_addToRecipe.setText(Derived::tr("Set for recipe"));
         }
      }

      if constexpr (!std::is_same_v<NE, Recipe>) {
         this->m_action_edit.setText(Derived::tr("Edit"));
      }
      this->m_action_copy.setText(Derived::tr("Copy"));
      this->m_action_delete.setText(Derived::tr("Delete"));
      this->m_action_rename.setText(Derived::tr("Rename"));

      if constexpr (std::derived_from<NE, Ingredient>) {
         this->m_action_showStockPurchases.setText(Derived::tr("Show stock purchases"));
      }

      if constexpr (!std::is_same_v<NE, Salt>) {
         this->m_menu_export.setTitle(Derived::tr("Export"));
         this->m_action_exportToXmlOrJson.setText(Derived::tr("To File (BeerXML or BeerJSON)"));
      }

      this->m_action_import.setText(Derived::tr("Import (from BeerXML or BeerJSON)"));

      if constexpr (!std::disjunction_v<std::is_same<NE, Recipe>, std::is_base_of<StockPurchase, NE>>) {
         this->m_action_merge.setText(Derived::tr("Merge selected"));
      }

      if constexpr (std::is_same_v<NE, Recipe>) {
         this->m_menu_versioning.setTitle(Derived::tr("Snapshots"));
         this->m_action_showAncestors.setText(Derived::tr("Show Snapshots" ));
         this->m_action_hideAncestors.setText(Derived::tr("Hide Snapshots" ));
         this->m_action_orphanRecipe .setText(Derived::tr("Detach Recipe"  ));
         this->m_action_spawnRecipe  .setText(Derived::tr("Snapshot Recipe"));

         this->m_action_brewIt.setText(Derived::tr("Brew It!"));
      }

      if constexpr (!IsVoid<SNE>) {
         if constexpr (std::is_same_v<SNE, BrewNote>) {
            this->m_action_brewAgain       .setText(Derived::tr("Brew again"            ));
            this->m_action_changeBrewDate  .setText(Derived::tr("Change date"           ));
            this->m_action_recalcEfficiency.setText(Derived::tr("Recalculate efficiency"));
         }
         this->m_action_deleteSecondary.setText(Derived::tr("Delete"));
      }

      return;
   }

   /**
    * \brief Displays the relevant context menu (either primary or secondary) with relevant options enabled
    *
    * @param point
    * @param selected
    */
   QAction * showContextMenu(QPoint const & point, CommonContextMenuHelper::Selected<NE, SNE> const & selected) {
      if constexpr (std::same_as<NE, Recipe>) {
         // You cannot delete a locked recipe
         auto const & recipe{selected.firstPrimary};
         this->m_action_delete.setEnabled(!recipe->locked() && selected.numPrimary >= 1);
      } else {
         this->m_action_delete.setEnabled(selected.numPrimary >= 1);
      }

      //
      // If you have more than one type of thing enabled, we'll give precedence to primary items over secondary items
      // over folders.
      //
      if constexpr (std::same_as<NE, Recipe>) {
         if (selected.numPrimary > 0) {
            auto const & recipe{selected.firstPrimary};

            // if we have ancestors and are showing them but are not an actual
            // ancestor, then enable hide
            this->m_action_hideAncestors.setEnabled(recipe->hasAncestors() && selected.fpAncestorsVisible);
            // if we have ancestors and are not showing them, enable showAncestors
            this->m_action_showAncestors.setEnabled(recipe->hasAncestors() && !selected.fpAncestorsVisible);
            // If we have ancestors and are not locked, then we are a leaf node and allow orphaning
            this->m_action_orphanRecipe.setEnabled(recipe->hasAncestors() && !recipe->locked());
            // We don't want to be able to spawn ancestors directly.
            this->m_action_spawnRecipe.setEnabled(!recipe->hasDescendants());
            // If user has clicked the top-level Item 'Recipes' Once this menu Item will be forever disabled if we don't enable it.
            this->m_menu_export.setEnabled(true);
            this->m_action_copy.setEnabled(true);
            this->m_action_brewIt.setEnabled(true);
         } else if (selected.numFolders > 0) {
            this->m_action_hideAncestors.setEnabled(false);
            this->m_action_showAncestors.setEnabled(false);
            this->m_action_orphanRecipe.setEnabled(false);
            this->m_action_spawnRecipe.setEnabled(false);
            this->m_menu_export.setEnabled(false);
            this->m_action_copy.setEnabled(false);
            this->m_action_brewIt.setEnabled(false);
         }
      }

      //
      // TBD: For the moment, we don't allow multiple selections either to be deleted or to be added to the recipe.  But
      //      it would not be huge work to fix that if there is user demand for it.
      //
      if constexpr (!std::is_same_v<NE, Recipe>) {
         this->m_action_addToRecipe.setEnabled(selected.numPrimary == 1);
         this->m_action_edit       .setEnabled(selected.numPrimary == 1);
      }
      if constexpr (!std::disjunction_v<std::is_same<NE, Recipe>, std::is_base_of<StockPurchase, NE>>) {
         this->m_action_merge      .setEnabled(selected.numPrimary  > 1);
      }

      if constexpr (std::derived_from<NE, Ingredient>) {
         //
         // Doesn't make sense to offer option to show stock purchases unless we have exactly one item selected.  (This
         // is because we just do a text filtering based on the selected item's name.  We'd need a different approach if
         // we wanted to, eg, show the stock purchases for N selected items.)
         //
         // We deliberately do not try to disable this option just because there are no stock purchases, as it will not
         // be obvious to the user -- eg an item could have stock purchases and current zero inventory.
         //
         this->m_action_showStockPurchases.setEnabled(selected.numPrimary == 1);
      }

      this->m_action_rename.setEnabled(selected.numPrimary == 1);

      QAction * result = this->m_menu_primary.exec(point);
      qDebug() << Q_FUNC_INFO << "Result was:" << result;
      return result;
   }

   /**
    * \brief Does what it says on the tin (including no-op if NE is Recipe).
    *
    * @param selected
    */
   void addToOrSetForRecipe(std::shared_ptr<NE> selected) const {
      // Note that we cannot put the substance of the function here as it would create circular dependencies.
      if constexpr (!std::disjunction_v<std::is_same<NE, Recipe>, std::is_base_of<StockPurchase, NE>>) {
         CommonContextMenuHelper::doAddToOrSetForRecipe(selected);
      }
      return;
   }

   /**
    * \brief Ask the user for a new name for an item we are about to copy
    *
    * \return The new name for the copy, or empty string if this item is to be skipped or \c std::nullopt to abort
    *         copying all selected items.
    */
   std::optional<QString> askCopyConfirmation(QString currentItemName) {
      QInputDialog copyConfirmationDialog;
      copyConfirmationDialog.setWindowTitle(Derived::tr("Copy %1").arg(NE::localisedName()));
      copyConfirmationDialog.setLabelText(Derived::tr("Enter a unique name for the copy of %1.").arg(currentItemName));
      copyConfirmationDialog.setToolTip(Derived::tr("An empty name will skip copying this %1.").arg(NE::localisedName()));
      copyConfirmationDialog.setCancelButtonText(Derived::tr("Cancel All"));

      //
      // We already know how to generate a default new unique name, so we offer that to the user as a suggestion
      //
      copyConfirmationDialog.setTextValue(ObjectStoreUtils::normaliseName<NE>(currentItemName));

      if (copyConfirmationDialog.exec() == QDialog::Accepted) {
         //
         // QString::simplified not only trims trailing and leading spaces but also converts tabs etc to spaces and
         // condenses double spaces to single ones.
         //
         // NOTE we do not currently _enforce_ that names are unique.  The code should all work fine if there are, say,
         // two Hop objects called "Fuggle".  The only reason we try to ensure names are unique when we generate them is
         // to avoid confusing the user.
         //
         return copyConfirmationDialog.textValue().simplified();
      }

      return std::nullopt;
   }

   /**
    * \brief As \c askCopyConfirmation, but for multiple items
    */
   QList<std::pair<NE *, QString>> getConfirmedToCopy(QList<std::shared_ptr<NE>> const & selectedItems) {
      QList<std::pair<NE *, QString>> confirmedToCopy;
      for (auto const & item : selectedItems) {
         std::optional<QString> newItemName = this->askCopyConfirmation(item->name());
         if (!newItemName) {
            // std::nullopt means user clicked "Cancel All"
            return QList<std::pair<NE *, QString>>{};
         }

         // Empty name just means skip current item
         if (newItemName->isEmpty()) {
            continue;
         }

         confirmedToCopy.append(std::make_pair(item.get(), *newItemName));
      }

      return confirmedToCopy;
   }

   /**
    * \brief
    * @param selectedItems
    */
   void copyItems(QList<std::shared_ptr<NE>> const & selectedItems) {
      QList<std::pair<NE *, QString>> confirmedToCopy = this->getConfirmedToCopy(selectedItems);
      for (auto [item, newName] : confirmedToCopy) {
         auto itemCopy = ObjectStoreWrapper::insertCopyOf(*item);
         itemCopy->setName(newName);
         qDebug() << Q_FUNC_INFO << "Copied" << *item << "to" << *itemCopy;
         //
         // NOTE that caller typically does NOT need to manually add the item to its table or tree.  It will be
         // connected to the ObjectStore::signalObjectInserted signal, so its doElementAdded() member function will
         // already have been called.  So the new item will already be in its table or tree.
         //
      }
      return;
   }

   /**
    *
    * @param items
    */
   void exportItems(QList<NE const *> const & items) const {
      //
      // Neither salts nor inventory records form any part of BeerXML or BeerJSON
      //
      if constexpr (!std::disjunction_v<std::is_same<NE, Salt>, std::is_base_of<StockPurchase, NE>>) {
         bool const success = ImportExport::exportToFile(items);
         QMessageBox messageBox;
         messageBox.setIcon(success ? QMessageBox::Information : QMessageBox::Warning);
         messageBox.setWindowTitle(success ? Derived::tr("Export Succeeded") : Derived::tr("Export Failed"));
         messageBox.exec();
      }
      return;
   }

   /**
    *
    * @param items
    */
   void deletePrimaryItems(QList<std::shared_ptr<NE>> const & items) const {
      QList<int> itemIdsToDelete;

      auto confirmDelete = QMessageBox::NoButton;

      for (auto const & item : items) {
         //
         // If someone tries to delete something that's used in one or more Recipes then we just say it's not allowed.
         // (The alternative would be to check that they are sure and then run through every Recipe removing the item
         // about to be deleted, which could perhaps leave that Recipe in a weird state.)
         //
         // We dont need to check this in the following cases:
         //    - deleting a Recipe (because a Recipe is not used in another Recipe);
         //    - deleting a StockPurchase (because StockPurchase is not used in a Recipe)
         //    - deleting a secondary item (eg BrewNote), because it is only used by its parent.
         //
         // TODO: We have similar logic in TreeViewBase which we should ideally unify somewhere.
         //
         QString confirmationMessage = Derived::tr("Delete %1 #%2 \"%3\"?").arg(
                                          NE::localisedName()
                                       ).arg(
                                          item->key()
                                       ).arg(
                                          item->name()
                                       );
         if constexpr (!std::is_same_v<NE, Recipe> && !std::is_base_of_v<StockPurchase, NE>) {
            int const numRecipesUsedIn = item->numRecipesUsedIn();
            if (numRecipesUsedIn > 0) {
               QMessageBox::warning(&this->m_derived,
                                    Derived::tr("%1 in use").arg(NE::localisedName()),
                                    Derived::tr("Cannot delete this %1, as it is used in %n recipe(s)",
                                                "",
                                                numRecipesUsedIn).arg(NE::localisedName()),
                                    QMessageBox::Ok);
               // Skip the current item
               continue;
            }

            //
            // When the item is not used in any recipes, we show that as reassurance
            //
            confirmationMessage.append(QString{" (%1)"}.arg(numRecipesUsedIn));
         }

         if (confirmDelete != QMessageBox::YesToAll) {
            confirmDelete = QMessageBox::question(
               &this->m_derived,
               Derived::tr("Delete %1").arg(NE::localisedName()),
               confirmationMessage,
               QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::Cancel,
               QMessageBox::No
            );

            if (confirmDelete == QMessageBox::Cancel) {
               // Cancel means abort the entire operation
               return;
            }

            if (confirmDelete == QMessageBox::No) {
               // "No" just means skip the current item
               continue;
            }
         }

         itemIdsToDelete.append(item->key());
      }

      ObjectStoreWrapper::softDelete<NE>(itemIdsToDelete);

      return;
   }

   void renamePrimaryItem(NE & item) const {
      //
      // TODO: Once we make Folders first-class NamedEntity objects, we can do their renaming here, instead of in
      //       TreeModelBase.
      //
      if constexpr (!std::is_same_v<NE, Folder>) {
         QString newName = QInputDialog::getText(&this->m_derived,
                                                 Derived::tr("%1 name").arg(NE::localisedName()),
                                                 Derived::tr("%1 name:").arg(NE::localisedName()),
                                                 QLineEdit::Normal,
                                                 item.name()).simplified();
         if (newName.isEmpty()) {
            return;
         }

         Undoable::doOrRedoUpdate(item,
                                  TYPE_INFO(NE, NamedEntity, name),
                                  newName,
                                  Derived::tr("Change %1 Name").arg(NE::localisedName()));

      }
      return;
   }

   void mergeItems(QList<std::shared_ptr<NE>> selectedItems) const {
      //
      // It's somewhat harder to merge Recipes than other things, and I don't think there is the same need for it.
      //
      // It would be relatively straightforward to support merge of StockPurchases but, again, TBD whether this is
      // needed.
      //
      if constexpr (!std::disjunction_v<std::is_same<NE, Recipe>, std::is_base_of<StockPurchase, NE>>) {
         if (selectedItems.size() < 2) {
            return;
         }

         auto const confirmMerge = QMessageBox::question(
            &this->m_derived,
            // It's a bit of a cheat here to use "records" to avoid having to pluralise NE::localisedName()...
            Derived::tr("Merge %1 records").arg(NE::localisedName()),
            Derived::tr("Attempt to merge %1 %2 records? (NOTE: <b>This action cannot be undone!</b>  It is intended for "
                        "use where the records are identical or as near as makes no difference.  "
                        "Use on non-identical records risks data loss.)"
                        "<br><br><b>Please make sure you have a backup of your database file before using this "
                        "function!</b>").arg(
               selectedItems.size()
            ).arg(
               NE::localisedName()
            ),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::No
         );

         if (confirmMerge != QMessageBox::Yes) {
            return;
         }

         //
         // We're not doing anything clever here, just repeatedly merging the first and last items on the list until either
         // we hit an error or the list only has one item.
         //
         for (; selectedItems.size() >= 2; selectedItems.pop_back()) {
            //
            // The "merge" of two records is simply the replacement of one by the other.  Specifically we will delete one
            // of the items and replace all references to it with the other one.
            //
            // To decide which direction to do the merge, look at which is used in more recipes (and prefer to retain that
            // one).
            //
            // Note that we deliberately want references to shared pointers here, so we can swap the actual elements of the
            // list if needed.
            //
            std::shared_ptr<NE> & firstItem = selectedItems.first();
            std::shared_ptr<NE> & lastItem  = selectedItems.last ();
            if (lastItem->numRecipesUsedIn() > firstItem->numRecipesUsedIn()) {
               //
               // The items being swapped are shared pointers, so this should be relatively efficient
               //
               std::swap(firstItem, lastItem);
            }
            qInfo() <<
               Q_FUNC_INFO << "Attempting to merge" << *lastItem << "(used in" << lastItem->numRecipesUsedIn() <<
               "recipes) into" << *firstItem << "(used in" << firstItem->numRecipesUsedIn() << "recipes)";

            //
            // We are going to delete lastItem and replace all references to it with references to firstItem.
            //
            // Before we do that, check whether the items are identical and, if not, give the user a chance to bail.
            //
            QList<BtStringConst const *> propertiesThatDiffer = firstItem->getPropertiesThatDiffer(*lastItem);
            if (int const numDifferences = propertiesThatDiffer.size(); numDifferences > 0) {
               //
               // Show the user the list of properties that differ, and their values, and ask them if they really really
               // want to force the merge.  We'll show them how many fields differ in the main message, and then we'll list
               // each of the differences in the "detailed text" that can be shown by clicking "Show details..."
               //
               // Normally this is exactly what we'd use QMessageBox::setDetailedText for.  However, we really want to show
               // the data in tabular format using HTML, and QMessageBox does not support this for its detailed text field.
               // So we have to use BtMessageBox instead. TODO Create that class and replace QMessageBox below!
               //
               QMessageBox diffsFoundMessageBox;
               diffsFoundMessageBox.setWindowTitle(Derived::tr("%1 records differ").arg(NE::localisedName()));
               diffsFoundMessageBox.setText(
                  Derived::tr("WARNING: %1 records to be merged have %n difference(s).  "
                              "Do you want to merge anyway?", "0", numDifferences).arg(NE::localisedName())
               );
               BtStringStream detailedText;
               detailedText << "<table>"
                               "<tr>"
                                  "<th>Field</th>"
                                  "<th>" << firstItem->name() << " (#" << firstItem->key() << ")" << "</th>"
                                  "<th>" <<  lastItem->name() << " (#" <<  lastItem->key() << ")" << "</th>"
                               "</tr>"
                               "<tr>"
                                  "<td>№ Recipes</td>"
                                  "<td>" << firstItem->numRecipesUsedIn() << "</td>"
                                  "<td>" <<  lastItem->numRecipesUsedIn() << "</td>"
                               "</tr>";
               for (BtStringConst const * propertyName : propertiesThatDiffer) {
                  TypeInfo const & typeInfo = NE::typeLookup.getType(*propertyName);

                  detailedText << "<tr>"
                                  "<td>" << typeInfo.localisedName() << "</td>"
                                  "<td>" << PropertyHelper::readDataFromPropertyValue(
                                               firstItem->property(**propertyName),
                                               typeInfo,
                                               Qt::DisplayRole
                                            ).toString().toHtmlEscaped() << "</td>"
                                  "<td>" << PropertyHelper::readDataFromPropertyValue(
                                               lastItem->property(**propertyName),
                                               typeInfo,
                                               Qt::DisplayRole
                                            ).toString().toHtmlEscaped() << "</td>"
                                  "</tr>";
               }
               detailedText << "</table>";

               diffsFoundMessageBox.setInformativeText(
                  Derived::tr(
                     "%n field(s) differ between %1 #%2 and %1 #%3.  If you continue, %1 #%3 will be deleted "
                     "and all uses of it will be replaced by %1 #%2.  This <b>cannot</b> be undone.<br><br>%4",
                     "0",
                     numDifferences
                  ).arg(NE::localisedName()).arg(firstItem->key()).arg(lastItem->key()).arg(detailedText.asString())
               );

               diffsFoundMessageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No |QMessageBox::Cancel);
               diffsFoundMessageBox.setDefaultButton(QMessageBox::No);

               int const ret = diffsFoundMessageBox.exec();
               if (ret == QMessageBox::Cancel) {
                  //
                  // Cancel means stop trying to merge further records, so the user will investigate further and then
                  // reselect the records s/he wants.
                  //
                  return;
               }
               if (ret == QMessageBox::No) {
                  //
                  // No means skip this record pair
                  //
                  continue;
               }
            }

            //
            // Find all the uses of the last item on the list and replace it with the first one
            //
            int const lastItemKey = lastItem->key();
            if constexpr (std::is_base_of_v<Ingredient, NE>) {
               QList<std::shared_ptr<typename NE::RecipeAdditionClass>> recipeAdditions =
                  ObjectStoreWrapper::findAllMatching<typename NE::RecipeAdditionClass>(
                     [lastItemKey](std::shared_ptr<typename NE::RecipeAdditionClass> ra) {
                        return ra->ingredientId() == lastItemKey;
                     }
                  );
               qInfo() <<
                  Q_FUNC_INFO << "Replacing" << recipeAdditions.size() << "uses of" << lastItem << "with" << firstItem;
               for (auto const & recipeAddition : recipeAdditions) {
                  recipeAddition->setIngredientId(firstItem->key());
               }

               //
               // Now we have to do the same thing for inventory entries.  The code is almost the same because, in both
               // cases, the class we're dealing with inherits from IngredientAmount, which is the class that holds the
               // field we care about.
               //
               QList<std::shared_ptr<typename NE::StockPurchaseClass>> inventoryEntries =
                  ObjectStoreWrapper::findAllMatching<typename NE::StockPurchaseClass>(
                     [lastItemKey](std::shared_ptr<typename NE::StockPurchaseClass> inv) {
                        return inv->ingredientId() == lastItemKey;
                     }
                  );
               qInfo() <<
                  Q_FUNC_INFO << "Assigning" << inventoryEntries.size() << "inventory entries for" << lastItem << "to" <<
                  firstItem;
               for (auto const & inventoryEntry : inventoryEntries) {
                  inventoryEntry->setIngredientId(firstItem->key());
               }

            } else {
               QList<std::shared_ptr<Recipe>> recipes = ObjectStoreWrapper::findAllMatching<Recipe>(
                  [&lastItem](std::shared_ptr<Recipe> rec) { return rec->uses(*lastItem); }
               );
               qInfo() <<
                  Q_FUNC_INFO << "Replacing" << recipes.size() << "uses of" << lastItem << "with" << firstItem;
               for (auto const & recipe : recipes) {
                  recipe->set(firstItem);
               }
            }

            //
            // Item we are about to delete should no longer be used in any recipes
            //
            Q_ASSERT(0 == lastItem->numRecipesUsedIn());

            qInfo() << Q_FUNC_INFO << "Deleting" << lastItem;
            ObjectStoreWrapper::softDelete(*lastItem);
         }
      }

      return;
   }

   //================================================ Member Variables =================================================
   //
   // Note that member variables are intentionally public.  Owner classes will want to reuse translations from actions
   // and/or extend bits of the menu structure.
   //

   Derived & m_derived;

   // We use the same trick below as in TreeNodeBase to have member variables that only exists in certain circumstances.
   // We don't care too much about saving the space, but it's useful to have the compiler tell us if we try to use
   // something that has no meaning.  Eg, there aren't any stock purchases for a Style or a Recipe.
   struct Empty { };

   QMenu m_menu_primary = QMenu{};

   QMenu m_menu_new = QMenu{};
   QAction m_action_newItem;
   [[no_unique_address]] std::conditional_t<!showFolderOptions, Empty, QAction> m_action_newFolder;
   [[no_unique_address]] std::conditional_t<!std::derived_from<NE, Ingredient>, Empty, QAction> m_action_newStockPurchase;

   [[no_unique_address]] std::conditional_t<std::is_same_v<NE, Recipe>, Empty, QAction> m_action_edit;
   QAction m_action_copy;
   QAction m_action_delete;
   QAction m_action_rename;
   [[no_unique_address]] std::conditional_t<!std::derived_from<NE, Ingredient>, Empty, QAction> m_action_showStockPurchases;
   [[no_unique_address]] std::conditional_t<std::is_same_v<NE, Recipe>, Empty, QAction> m_action_addToRecipe;

   QMenu m_menu_export;
   // Salts aren't part of the BeerXML or BeerJSON standards, so they are the one ingredient you can't export
   [[no_unique_address]] std::conditional_t<std::is_same_v<NE, Salt>, Empty, QAction> m_action_exportToXmlOrJson;

   QAction m_action_import;
   [[no_unique_address]] std::conditional_t<
      std::disjunction_v<std::is_same<NE, Recipe>, std::is_base_of<StockPurchase, NE>>, Empty, QAction
   > m_action_merge;

   [[no_unique_address]] std::conditional_t<!std::is_same_v<NE, Recipe>, Empty, QMenu> m_menu_versioning;
   [[no_unique_address]] std::conditional_t<!std::is_same_v<NE, Recipe>, Empty, QAction> m_action_showAncestors;
   [[no_unique_address]] std::conditional_t<!std::is_same_v<NE, Recipe>, Empty, QAction> m_action_hideAncestors;
   [[no_unique_address]] std::conditional_t<!std::is_same_v<NE, Recipe>, Empty, QAction> m_action_orphanRecipe ;
   [[no_unique_address]] std::conditional_t<!std::is_same_v<NE, Recipe>, Empty, QAction> m_action_spawnRecipe  ;

   [[no_unique_address]] std::conditional_t<!std::is_same_v<NE, Recipe>, Empty, QAction> m_action_brewIt;

   [[no_unique_address]] std::conditional_t<IsVoid<SNE>, Empty, QMenu> m_menu_secondary;
   [[no_unique_address]] std::conditional_t<!std::is_same_v<SNE, BrewNote>, Empty, QAction> m_action_brewAgain;
   [[no_unique_address]] std::conditional_t<!std::is_same_v<SNE, BrewNote>, Empty, QAction> m_action_changeBrewDate;
   [[no_unique_address]] std::conditional_t<!std::is_same_v<SNE, BrewNote>, Empty, QAction> m_action_recalcEfficiency;
   [[no_unique_address]] std::conditional_t<IsVoid<SNE>, Empty, QAction> m_action_deleteSecondary;

};

#endif
