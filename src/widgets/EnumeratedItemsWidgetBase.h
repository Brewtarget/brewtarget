/*======================================================================================================================
 * widgets/EnumeratedItemsWidgetBase.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef WIDGETS_ENUMERATEDITEMSWIDGETBASE_H
#define WIDGETS_ENUMERATEDITEMSWIDGETBASE_H
#pragma once

#include <QAbstractButton>
#include <QTableView>

#include "model/Recipe.h"
#include "undoRedo/Undoable.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/WindowDistributor.h"

/**
 * \brief CRTP base class for \c EnumeratedItemsWidget subclasses
 */
template<class Derived> class EnumeratedItemsWidgetPhantom;
template<class Derived, class Item>
class EnumeratedItemsWidgetBase : public CuriouslyRecurringTemplateBase<EnumeratedItemsWidgetPhantom, Derived> {
public:
   //
   // These aliases save us having to use typename in a lot of places
   //
   using Owner               = Item::OwnerClass;
   using NeItemDelegateClass = Item::ItemDelegateClass;
   using NeTableModelClass   = Item::TableModelClass;
   using ItemEditorClass     = Item::EditorClass;

   EnumeratedItemsWidgetBase() {
      this->derived().m_pushButton_addItem     ->setToolTip(Owner::tr("Add %1 item"               ).arg(Owner::localisedName()));
      this->derived().m_pushButton_removeItem  ->setToolTip(Owner::tr("Remove selected %1 item"   ).arg(Owner::localisedName()));
      this->derived().m_pushButton_moveItemUp  ->setToolTip(Owner::tr("Move selected %1 item up"  ).arg(Owner::localisedName()));
      this->derived().m_pushButton_moveItemDown->setToolTip(Owner::tr("Move selected %1 item down").arg(Owner::localisedName()));
      this->derived().m_pushButton_editItem    ->setToolTip(Owner::tr("Edit selected %1 item"     ).arg(Owner::localisedName()));

      this->m_itemTableModel = std::make_unique<NeTableModelClass>(this->derived().m_tableView_items.get());
      this->derived().m_tableView_items->setItemDelegate(
         new NeItemDelegateClass(this->derived().m_tableView_items.get(), *this->m_itemTableModel)
      );
      this->derived().m_tableView_items->setModel(this->m_itemTableModel.get());
      //
      // Connect all the buttons
      //
      this->derived().connect(this->derived().m_pushButton_addItem.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->newItem(); return; } );
      this->derived().connect(this->derived().m_pushButton_removeItem.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->removeSelectedItem(); return; } );
      this->derived().connect(this->derived().m_pushButton_moveItemUp.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->moveSelectedItemUp(); return; } );
      this->derived().connect(this->derived().m_pushButton_moveItemDown.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->moveSelectedItemDown(); return; } );
      this->derived().connect(this->derived().m_pushButton_editItem.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->editSelectedItem(); return; } );

      // Double-clicking on the name of a item also edits it
      this->derived().connect(
         this->derived().m_tableView_items.get(),
         &QTableView::doubleClicked,
         &this->derived(),
         [&](QModelIndex const & idx) {
            if (idx.column() == 0) {
               this->editSelectedItem();
            }
            return;
         }
      );


      return;
   }
   ~EnumeratedItemsWidgetBase() = default;

   /**
    * \brief Call this when the \c EnumeratedItemsWidget is being used in an editor (eg \c MashStepsWidget in \c MashEditor).
    */
   void setOwner(std::shared_ptr<Owner> owner) {
      this->m_owner = owner;
      this->m_itemTableModel->setEnumeratedItemOwner(owner);
      return;
   }

   /**
    * \brief Call this when the \c EnumeratedItemsWidget is being used in MainWindow (eg \c MashEnumeratedItemsWidget in \c mashStepsTab).
    */
   void setRecipe(Recipe * recipe) {
      if (this->m_recipe) {
         this->derived().disconnect(this->m_recipe, nullptr, &this->derived(), nullptr);
      }

      this->m_recipe = recipe;
      if (this->m_recipe) {
         this->derived().connect(
            this->m_recipe,
            &NamedEntity::changed,
            &this->derived(),
            [&](QMetaProperty prop, [[maybe_unused]] QVariant val) {
               if (prop.name() == Recipe::propertyNameFor<Owner>()) {
                  // See comment in buttons/RecipeAttributeButton.h for why we need template keyword here for Clang
                  this->setOwner(this->m_recipe->template get<Owner>());
               }
               return;
            }
         );
         // See comment in buttons/RecipeAttributeButton.h for why we need template keyword here for Clang
         this->setOwner(this->m_recipe->template get<Owner>());
      } else {
         this->setOwner(nullptr);
      }
      return;
   }

   //! \return -1 if no row is selected or more than one row is selected
   [[nodiscard]] int getSelectedRowNum() const {
      QModelIndexList selected = this->derived().m_tableView_items->selectionModel()->selectedIndexes();
      int size = selected.size();
      if (size == 0) {
         return -1;
      }

      // Make sure only one row is selected.
      int const row = selected[0].row();
      for (int ii = 1; ii < size; ++ii) {
         if (selected[ii].row() != row) {
            return -1;
         }
      }

      return row;
   }

   void showItemEditor(std::shared_ptr<Item> item) {
      ItemEditorClass & itemEditor = WindowDistributor::get<ItemEditorClass>();
      itemEditor.setOwner(this->m_owner);
      itemEditor.setEditItem(item);
      itemEditor.setVisible(true);
      return;
   }

   void editSelectedItem() {
      if (!this->m_owner) {
         return;
      }

      int const row = this->getSelectedRowNum();
      if (row < 0) {
         return;
      }

      auto item = this->m_itemTableModel->getRow(static_cast<unsigned int>(row));
      this->showItemEditor(item);

      return;
   }

   void newItem() {
      if (!this->m_owner) {
         return;
      }

      // This ultimately gets stored in Undoable::addStepToStepOwner() etc
      auto item = std::make_shared<Item>("");
      this->showItemEditor(item);
      return;
   }

   void removeSelectedItem() {
      if (!this->m_owner) {
         return;
      }

      int const row = this->getSelectedRowNum();
      if (row < 0) {
         return;
      }

      auto item = this->m_itemTableModel->getRow(static_cast<unsigned int>(row));
      Undoable::doOrRedoUpdate(
         newUndoableAddOrRemove(*this->m_owner,
                                &Item::EnumeratedItemOwnerClass::remove,
                                item,
                                &Item::EnumeratedItemOwnerClass::add,
                                static_cast<void (*)(std::shared_ptr<Item>)>(nullptr),
                                static_cast<void (*)(std::shared_ptr<Item>)>(nullptr),
                                Derived::tr("Remove %1").arg(Item::localisedName()))
      );

      return;
   }

   void moveSelectedItemUp() {
      int const row = this->getSelectedRowNum();

      // Make sure row is valid and we can actually move it up.
      if (row < 1) {
         return;
      }

      this->m_itemTableModel->moveItemUp(row);
      return;
   }

   void moveSelectedItemDown() {
      int const row = this->getSelectedRowNum();

      // Make sure row is valid and it's not the last row so we can move it down.
      if (row < 0 || row >= this->m_itemTableModel->rowCount() - 1) {
         return;
      }

      this->m_itemTableModel->moveItemDown(row);
      return;
   }

protected:
   Recipe *                           m_recipe         = nullptr;
   std::shared_ptr<Owner>             m_owner          = nullptr;
   std::unique_ptr<NeTableModelClass> m_itemTableModel = nullptr;
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define ENUMERATED_ITEMS_WIDGET_COMMON_DECL(ItemName)                             \
   /* This allows EnumeratedItemsWidgetBase to call protected and private members of Derived */ \
   friend class EnumeratedItemsWidgetBase<ItemName##sWidget, ItemName>;  \
                                                                         \
   public:                                                               \
      ItemName##sWidget(QWidget * parent);                               \
      virtual ~ItemName##sWidget();                                      \
                                                                         \


/**
 * \brief Derived classes should include this in their implementation file
 */
#define ENUMERATED_ITEMS_WIDGET_COMMON_CODE(ItemName) \
   ItemName##sWidget::ItemName##sWidget(QWidget * parent) :      \
      EnumeratedItemsWidget{parent},                             \
      EnumeratedItemsWidgetBase<ItemName##sWidget, ItemName>{} { \
      return;                                                    \
   }                                                             \
   ItemName##sWidget::~ItemName##sWidget() = default;            \

#endif
