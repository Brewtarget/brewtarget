/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/MashStepTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "qtModels/tableModels/MashStepTableModel.h"

#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QModelIndex>
#include <QTableView>
#include <QTimer>
#include <QVariant>
#include <QWidget>

#include "MashAutoAdjuster.h"
#include "model/Equipment.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "model/RecipeUtils.h"
#include "qtModels/tableModels/BtTableModel.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MashStepTableModel.cpp"
#endif

COLUMN_INFOS(
   MashStepTableModel,
   TABLE_MODEL_HEADER(MashStep, Name        , PropertyNames::NamedEntity::name               ), // "Name"
   TABLE_MODEL_HEADER(MashStep, Type        , PropertyNames::MashStep::type                  ), // "Type"
   TABLE_MODEL_HEADER(MashStep, Amount      , PropertyNames::MashStep::amount_l              ), // "Amount"
   TABLE_MODEL_HEADER(MashStep, Temp        , PropertyNames::MashStep::infuseTemp_c          ), // "Infusion Temp"
   TABLE_MODEL_HEADER(MashStep, TargetTemp  , PropertyNames::StepBase::startTemp_c           ), // "Target Temp"
   TABLE_MODEL_HEADER(MashStep, EndTemp     , PropertyNames::Step::endTemp_c                 ), // "End Temp"
   TABLE_MODEL_HEADER(MashStep, RampTime    , PropertyNames::StepBase::rampTime_mins         ), // "Ramp Time"
   TABLE_MODEL_HEADER(MashStep, Time        , PropertyNames::StepBase::stepTime_mins         ), // "Time"
   TABLE_MODEL_HEADER(MashStep, StartAcidity, PropertyNames::Step::startAcidity_pH           ), // "Start pH"
   TABLE_MODEL_HEADER(MashStep,   EndAcidity, PropertyNames::Step::  endAcidity_pH           ), // "End pH"
   TABLE_MODEL_HEADER(MashStep, WaterToGrain, PropertyNames::MashStep::liquorToGristRatio_lKg), // "Water:Grain"
)

// See comment in header.  The fourth parameter is alpha (ie 0 = fully transparent, 255 = fully opaque).
QColor const MashStepTableModel::errorRowColor   {230,  40,  40,  70};
QColor const MashStepTableModel::errorCellColor  {230,  40,  40, 160};
QColor const MashStepTableModel::warningRowColor {240, 170,   0,  70};
QColor const MashStepTableModel::warningCellColor{240, 170,   0, 160};

MashStepTableModel::MashStepTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<MashStepTableModel, MashStep>{},
   EnumeratedItemTableModelBase<MashStepTableModel, MashStep, Mash>{} {
   this->setObjectName("mashStepTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &MashStepTableModel::contextMenu);
   m_parentTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(m_parentTableWidget, &QWidget::customContextMenuRequested, this, &MashStepTableModel::cellContextMenu);

   //
   // Re-check the steps whenever they change.  (Changes to the mash, recipe and equipment are handled in validate().)
   // Note that validate() itself emits dataChanged if the results change, which means we'll check one more time, but
   // that will find nothing has changed, so won't emit anything.
   //
   connect(this, &QAbstractItemModel::modelReset  , this, &MashStepTableModel::scheduleValidation);
   connect(this, &QAbstractItemModel::rowsInserted, this, &MashStepTableModel::scheduleValidation);
   connect(this, &QAbstractItemModel::rowsRemoved , this, &MashStepTableModel::scheduleValidation);
   connect(this, &QAbstractItemModel::rowsMoved   , this, &MashStepTableModel::scheduleValidation);
   connect(this, &QAbstractItemModel::dataChanged , this, &MashStepTableModel::scheduleValidation);
   //
   // See comment in qtModels/tableModels/BoilStepTableModel.cpp for why we don't listen directly to signals from
   // ObjectStore.
   //
   return;
}

MashStepTableModel::~MashStepTableModel() = default;

void MashStepTableModel::added  ([[maybe_unused]] std::shared_ptr<MashStep> item) { return; }
void MashStepTableModel::removed([[maybe_unused]] std::shared_ptr<MashStep> item) { return; }
void MashStepTableModel::updateTotals()                                      { return; }

QVariant MashStepTableModel::data(QModelIndex const & index, int role) const {
   //
   // Show a pin icon on amounts and infusion temperatures that are "pinned" (aka "fixed"), ie that will not be changed when
   // adjusting the mash (see MashAutoAdjuster and MashEditor::calculate).
   //
   // Also, if a step doesn't make sense (see validate()), tint its row and explain why in the tool tip.  Where we know
   // which value in the step causes the problem, we tint that cell more strongly.
   //
   if (this->m_itemOwnerObs &&
       (role == Qt::DecorationRole || role == Qt::ToolTipRole || role == Qt::BackgroundRole) &&
       this->indexOk(index)) {
      auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
      auto const & row = this->m_rows[index.row()];
      bool const isFixed =
         (columnIndex == MashStepTableModel::ColumnIndex::Amount && row->amountFixed()) ||
         (columnIndex == MashStepTableModel::ColumnIndex::Temp   && row->infuseTempFixed() &&
                                                                    row->type() != MashStep::Type::Decoction);
      auto const problems = this->m_stepProblems.constFind(row.get());
      bool const hasProblems = (problems != this->m_stepProblems.cend());
      switch (role) {
         case Qt::DecorationRole:
            if (isFixed) {
               static QIcon const pinIcon{":/images/smallPin.svg"};
               return pinIcon;
            }
            break;
         case Qt::BackgroundRole:
            if (hasProblems) {
               QString const cellProperty = this->get_ColumnInfo(columnIndex).propertyPath.asXPath();
               if (auto const cellProblem = problems->propertyIsError.constFind(cellProperty);
                   cellProblem != problems->propertyIsError.cend()) {
                  return QBrush{cellProblem.value() ? errorCellColor : warningCellColor};
               }
               return QBrush{problems->isError ? errorRowColor : warningRowColor};
            }
            break;
         case Qt::ToolTipRole:
            {
               QStringList toolTip;
               if (isFixed) {
                  toolTip.append(tr("Pinned: not changed when adjusting the mash"));
               }
               if (hasProblems) {
                  toolTip.append(problems->messages);
               }
               if (!toolTip.isEmpty()) {
                  return toolTip.join("\n");
               }
            }
            break;
         default:
            break;
      }
      return QVariant();
   }

   if (!this->m_itemOwnerObs || !this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
   if (MashStepTableModel::ColumnIndex::Temp == columnIndex) {
      auto row = this->m_rows[index.row()];
      if (row->type() == MashStep::Type::Decoction) {
         return QVariant("---");
      }
   }

   // No other special handling required for any of our other columns
   return this->readDataFromModel(index, role);
}

void MashStepTableModel::cellContextMenu(QPoint const & point) {
   if (!this->m_itemOwnerObs) {
      return;
   }
   QModelIndex const index = this->m_parentTableWidget->indexAt(point);
   if (!index.isValid() || index.row() >= static_cast<int>(this->m_rows.size())) {
      return;
   }
   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
   std::shared_ptr<MashStep> const step = this->m_rows[index.row()];

   QString actionText;
   bool isFixed = false;
   if (columnIndex == MashStepTableModel::ColumnIndex::Amount) {
      actionText = tr("Pinned amount (not changed when adjusting the mash)");
      isFixed = step->amountFixed();
   } else if (columnIndex == MashStepTableModel::ColumnIndex::Temp &&
              step->type() != MashStep::Type::Decoction &&
              step->type() != MashStep::Type::Temperature) {
      actionText = tr("Pinned infusion temperature (not changed when adjusting the mash)");
      isFixed = step->infuseTempFixed();
   } else {
      // No menu for other cells
      return;
   }

   QMenu menu{this->m_parentTableWidget};
   QAction * fixedAction = menu.addAction(QIcon{":/images/smallPin.svg"}, actionText);
   fixedAction->setCheckable(true);
   fixedAction->setChecked(isFixed);
   // Same rules as for editing the cell
   bool const canChange =
      this->m_editable &&
      RecipeUtils::lockReason(*this->m_itemOwnerObs) == RecipeUtils::LockReason::NotLocked;
   fixedAction->setEnabled(canChange);

   if (menu.exec(this->m_parentTableWidget->viewport()->mapToGlobal(point)) != fixedAction) {
      return;
   }
   if (columnIndex == MashStepTableModel::ColumnIndex::Amount) {
      step->setAmountFixed(!isFixed);
   } else {
      step->setInfuseTempFixed(!isFixed);
   }
   return;
}

void MashStepTableModel::scheduleValidation() {
   if (!this->m_validationScheduled) {
      this->m_validationScheduled = true;
      QTimer::singleShot(0, this, [this]() {
         this->m_validationScheduled = false;
         this->validate();
         return;
      });
   }
   return;
}

void MashStepTableModel::validate() {
   for (auto const & connection : this->m_validationConnections) {
      QObject::disconnect(connection);
   }
   this->m_validationConnections.clear();

   QHash<MashStep const *, StepProblems> stepProblems;
   QStringList mashProblems;
   QHash<QString, QStringList> mashFieldProblems;
   Recipe * recipe = nullptr;
   if (this->m_itemOwnerObs) {
      Mash const & mash = *this->m_itemOwnerObs;
      auto reValidate = [this]([[maybe_unused]] QMetaProperty prop, [[maybe_unused]] QVariant val) {
         this->scheduleValidation();
         return;
      };
      // The mash could get used by another recipe (or stop being used by one), and its own fields (eg grain temperature)
      // affect the checks.
      this->m_validationConnections.append(
         connect(this->m_itemOwnerObs.get(), &NamedEntity::changed, this, reValidate)
      );
      QList<Recipe *> const recipes = RecipeUtils::recipesUsing(mash);
      if (recipes.size() == 1) {
         recipe = recipes.first();
         this->m_validationConnections.append(connect(recipe, &NamedEntity::changed, this, reValidate));
         if (auto const equipment = recipe->equipment()) {
            this->m_validationConnections.append(connect(equipment.get(), &NamedEntity::changed, this, reValidate));
         }

         MashAutoAdjuster::Inputs inputs = MashAutoAdjuster::Inputs::fromRecipe(*recipe, mash);
         if (this->m_inputsAdjuster) {
            this->m_inputsAdjuster(inputs);
         }
         MashAutoAdjuster::Result const result = MashAutoAdjuster::check(inputs, MashAutoAdjuster::stepsFrom(mash));
         QList<std::shared_ptr<MashStep>> const mashSteps = mash.mashSteps();
         for (auto const & problem : result.problems) {
            bool const isError = problem.severity == MashAutoAdjuster::Problem::Severity::Error;

            std::visit(
               [&](auto const & scope) {
                  using T = std::decay_t<decltype(scope)>;

                  if constexpr (std::is_same_v<T, MashAutoAdjuster::Problem::StepScope>) {
                     StepProblems & entry = stepProblems[mashSteps.at(scope.stepIndex).get()];
                     entry.isError |= isError;
                     entry.messages.append(problem.message);
                     if (!scope.stepProperty.isEmpty()) {
                        // Note that QHash::operator[] default-constructs bool to false if the key isn't already present
                        entry.propertyIsError[scope.stepProperty] |= isError;
                     }
                  } else if constexpr (std::is_same_v<T, MashAutoAdjuster::Problem::MashPropertyScope>) {
                     mashFieldProblems[scope.mashProperty].append(problem.message);
                  } else if constexpr (std::is_same_v<T, MashAutoAdjuster::Problem::MashScope>) {
                     mashProblems.append(problem.message);
                  }
               },
               problem.scope
            );
         }
      }
   }

   if (stepProblems      != this->m_stepProblems      ||
       mashProblems      != this->m_mashProblems      ||
       mashFieldProblems != this->m_mashFieldProblems ||
       recipe            != this->m_validationRecipe) {
      this->m_stepProblems      = stepProblems;
      this->m_mashProblems      = mashProblems;
      this->m_mashFieldProblems = mashFieldProblems;
      this->m_validationRecipe  = recipe;
      if (this->rowCount() > 0) {
         emit this->dataChanged(this->index(0, 0), this->index(this->rowCount() - 1, this->columnCount() - 1));
      }
      emit this->validationChanged();
   }
   return;
}

bool MashStepTableModel::isValidating() const {
   return this->m_validationRecipe != nullptr;
}

bool MashStepTableModel::hasProblems() const {
   return !this->m_stepProblems.isEmpty() || !this->m_mashProblems.isEmpty() || !this->m_mashFieldProblems.isEmpty();
}

[[nodiscard]] QHash<QString, QStringList> const & MashStepTableModel::mashFieldProblems() const {
   return this->m_mashFieldProblems;
}

void MashStepTableModel::setInputsAdjuster(std::function<void(MashAutoAdjuster::Inputs &)> adjuster) {
   this->m_inputsAdjuster = std::move(adjuster);
   this->scheduleValidation();
   return;
}

[[nodiscard]] QStringList const & MashStepTableModel::mashOverallProblems() const {
   return this->m_mashProblems;
}

[[nodiscard]] QString MashStepTableModel::allProblemsDisplayable() const {
   QStringList lines;
   for (auto const & messages : this->m_mashFieldProblems) {
      lines.append(messages);
   }
   for (auto const & row : this->m_rows) {
      auto const problems = this->m_stepProblems.constFind(row.get());
      if (problems != this->m_stepProblems.cend()) {
         for (auto const & message : problems->messages) {
            lines.append(tr("⚠ %1: %2").arg(row->name(), message));
         }
      }
   }
   for (auto const & mashProblem : this->m_mashProblems) {
      lines.append(tr("⚠ %1").arg(mashProblem));
   }
   return lines.join("\n");
}

bool MashStepTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_itemOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(MashStep, mashStep, PropertyNames::Recipe::mashId)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
ENUMERATED_ITEM_TABLE_MODEL_COMMON_CODE(MashStep, Mash)
//============================================= CLASS MashStepItemDelegate =============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(MashStep)