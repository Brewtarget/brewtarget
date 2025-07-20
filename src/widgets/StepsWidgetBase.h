/*======================================================================================================================
 * widgets/StepsWidgetBase.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef WIDGETS_STEPSWIDGETBASE_H
#define WIDGETS_STEPSWIDGETBASE_H
#pragma once

#include <QAbstractButton>
#include <QTableView>

#include "model/Recipe.h"
#include "undoRedo/Undoable.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \brief CRTP base class for \c StepsWidget subclasses
 */
template<class Derived> class StepsWidgetPhantom;
template<class Derived, class NE>
class StepsWidgetBase : public CuriouslyRecurringTemplateBase<StepsWidgetPhantom, Derived> {
public:
   using NeStepClass         = NE::StepClass;
   using NeItemDelegateClass = NeStepClass::ItemDelegateClass;
   using NeStepEditorClass   = NeStepClass::EditorClass;
   using NeTableModelClass   = NeStepClass::TableModelClass;

   StepsWidgetBase() {
      this->derived().m_pushButton_addStep     ->setToolTip(NE::tr("Add %1 step"               ).arg(NE::localisedName()));
      this->derived().m_pushButton_removeStep  ->setToolTip(NE::tr("Remove selected %1 step"   ).arg(NE::localisedName()));
      this->derived().m_pushButton_moveStepUp  ->setToolTip(NE::tr("Move selected %1 step up"  ).arg(NE::localisedName()));
      this->derived().m_pushButton_moveStepDown->setToolTip(NE::tr("Move selected %1 step down").arg(NE::localisedName()));
      this->derived().m_pushButton_editStep    ->setToolTip(NE::tr("Edit selected %1 step"     ).arg(NE::localisedName()));

      this->m_stepTableModel = std::make_unique<NeTableModelClass>(this->derived().m_tableView_steps.get());
      this->derived().m_tableView_steps->setItemDelegate(
         new NeItemDelegateClass(this->derived().m_tableView_steps.get(), *this->m_stepTableModel)
      );
      this->derived().m_tableView_steps->setModel(this->m_stepTableModel.get());
      //
      // Connect all the buttons
      //
      this->derived().connect(this->derived().m_pushButton_addStep.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->newStep(); return; } );
      this->derived().connect(this->derived().m_pushButton_removeStep.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->removeSelectedStep(); return; } );
      this->derived().connect(this->derived().m_pushButton_moveStepUp.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->moveSelectedStepUp(); return; } );
      this->derived().connect(this->derived().m_pushButton_moveStepDown.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->moveSelectedStepDown(); return; } );
      this->derived().connect(this->derived().m_pushButton_editStep.get(),
                              &QAbstractButton::clicked,
                              &this->derived(),
                              [&]([[maybe_unused]] bool checked) { this->editSelectedStep(); return; } );

      // Double-clicking on the name of a step also edits it
      this->derived().connect(
         this->derived().m_tableView_steps.get(),
         &QTableView::doubleClicked,
         &this->derived(),
         [&](QModelIndex const & idx) {
            if (idx.column() == 0) {
               this->editSelectedStep();
            }
            return;
         }
      );


      return;
   }
   ~StepsWidgetBase() = default;

   /**
    * \brief Call this when the \c StepsWidget is being used in an editor (eg \c MashStepsWidget in \c MashEditor).
    */
   void setStepOwner(std::shared_ptr<NE> stepOwner) {
      this->m_stepOwner = stepOwner;
      this->m_stepTableModel->setStepOwner(stepOwner);
      return;
   }

   /**
    * \brief Call this when the \c StepsWidget is being used in MainWindow (eg \c MashStepsWidget in \c mashStepsTab).
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
               if (prop.name() == Recipe::propertyNameFor<NE>()) {
                  // See comment in buttons/RecipeAttributeButton.h for why we need template keyword here for Clang
                  this->setStepOwner(this->m_recipe->template get<NE>());
               }
               return;
            }
         );
         // See comment in buttons/RecipeAttributeButton.h for why we need template keyword here for Clang
         this->setStepOwner(this->m_recipe->template get<NE>());
      } else {
         this->setStepOwner(nullptr);
      }
      return;
   }

   //! \return -1 if no row is selected or more than one row is selected
   [[nodiscard]] int getSelectedRowNum() const {
      QModelIndexList selected = this->derived().m_tableView_steps->selectionModel()->selectedIndexes();
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

   void showStepEditor(std::shared_ptr<NeStepClass> step) {
      NeStepEditorClass & stepEditor = NeStepClass::getEditor();
      stepEditor.setStepOwner(this->m_stepOwner);
      stepEditor.setEditItem(step);
      stepEditor.setVisible(true);
      return;
   }

   void editSelectedStep() {
      if (!this->m_stepOwner) {
         return;
      }

      int const row = this->getSelectedRowNum();
      if (row < 0) {
         return;
      }

      auto step = this->m_stepTableModel->getRow(static_cast<unsigned int>(row));
      this->showStepEditor(step);

      return;
   }

   void newStep() {
      if (!this->m_stepOwner) {
         return;
      }

      // This ultimately gets stored in Undoable::addStepToStepOwner() etc
      auto step = std::make_shared<NeStepClass>("");
      this->showStepEditor(step);
      return;
   }

   void removeSelectedStep() {
      if (!this->m_stepOwner) {
         return;
      }

      int const row = this->getSelectedRowNum();
      if (row < 0) {
         return;
      }

      auto step = this->m_stepTableModel->getRow(static_cast<unsigned int>(row));
      Undoable::doOrRedoUpdate(
         newUndoableAddOrRemove(*this->m_stepOwner,
                                &NeStepClass::StepOwnerClass::remove,
                                step,
                                &NeStepClass::StepOwnerClass::add,
                                static_cast<void (*)(std::shared_ptr<NeStepClass>)>(nullptr),
                                static_cast<void (*)(std::shared_ptr<NeStepClass>)>(nullptr),
                                Derived::tr("Remove %1").arg(NeStepClass::localisedName()))
      );

      return;
   }

   void moveSelectedStepUp() {
      int const row = this->getSelectedRowNum();

      // Make sure row is valid and we can actually move it up.
      if (row < 1) {
         return;
      }

      this->m_stepTableModel->moveStepUp(row);
      return;
   }

   void moveSelectedStepDown() {
      int const row = this->getSelectedRowNum();

      // Make sure row is valid and it's not the last row so we can move it down.
      if (row < 0 || row >= this->m_stepTableModel->rowCount() - 1) {
         return;
      }

      this->m_stepTableModel->moveStepDown(row);
      return;
   }

protected:
   Recipe *                           m_recipe         = nullptr;
   std::shared_ptr<NE>                m_stepOwner      = nullptr;
   std::unique_ptr<NeTableModelClass> m_stepTableModel = nullptr;
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define STEPS_WIDGET_COMMON_DECL(NeName)                                              \
   /* This allows StepsWidgetBase to call protected and private members of Derived */ \
   friend class StepsWidgetBase<NeName##StepsWidget, NeName>;                         \
                                                                                      \
   public:                                                                            \
      NeName##StepsWidget(QWidget * parent);                                          \
      virtual ~NeName##StepsWidget();                                                 \
                                                                                      \


/**
 * \brief Derived classes should include this in their implementation file
 */
#define STEPS_WIDGET_COMMON_CODE(NeName) \
   NeName##StepsWidget::NeName##StepsWidget(QWidget * parent) :   \
      StepsWidget{parent},                                        \
      StepsWidgetBase<NeName##StepsWidget, NeName>{} {            \
      return;                                                     \
   }                                                              \
   NeName##StepsWidget::~NeName##StepsWidget() = default;         \

#endif
