/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/FermentationStepTableModel.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "tableModels/FermentationStepTableModel.h"

#include <QAbstractTableModel>
#include <QComboBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QModelIndex>
#include <QObject>
#include <QTableView>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/FermentationStep.h"
#include "PersistentSettings.h"

FermentationStepTableModel::FermentationStepTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(FermentationStep, Name        , tr("Name"         ), PropertyNames::     NamedEntity::name           ),
         TABLE_MODEL_HEADER(FermentationStep, StepTime    , tr("Step Time"    ), PropertyNames::            Step::stepTime_mins  , PrecisionInfo{0}),
         TABLE_MODEL_HEADER(FermentationStep, StartTemp   , tr("Start Temp"   ), PropertyNames::            Step::startTemp_c    ),
         TABLE_MODEL_HEADER(FermentationStep, EndTemp     , tr("End Temp"     ), PropertyNames::            Step::endTemp_c      ),
         TABLE_MODEL_HEADER(FermentationStep, StartAcidity, tr("Start Acidity"), PropertyNames::            Step::startAcidity_pH),
         TABLE_MODEL_HEADER(FermentationStep, EndAcidity  , tr("End Acidity"  ), PropertyNames::            Step::endAcidity_pH  ),
         TABLE_MODEL_HEADER(FermentationStep, StartGravity, tr("Start Gravity"), PropertyNames::    StepExtended::startGravity_sg),
         TABLE_MODEL_HEADER(FermentationStep, EndGravity  , tr("End Gravity"  ), PropertyNames::    StepExtended::  endGravity_sg),
         TABLE_MODEL_HEADER(FermentationStep, FreeRise    , tr("Free Rise"    ), PropertyNames::FermentationStep::freeRise       , BoolInfo{tr("No"), tr("Yes")}),
         TABLE_MODEL_HEADER(FermentationStep, Vessel      , tr("Vessel"       ), PropertyNames::FermentationStep::vessel         ),
      }
   },
   TableModelBase<FermentationStepTableModel, FermentationStep>{},
   StepTableModelBase<FermentationStepTableModel, FermentationStep, Fermentation>{} {
   this->setObjectName("fermentationStepTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &FermentationStepTableModel::contextMenu);

   //
   // Whilst, in principle, we could connect to ObjectStoreTyped<FermentationStep>::getInstance() to listen for signals
   // &ObjectStoreTyped<FermentationStep>::signalObjectInserted and &ObjectStoreTyped<FermentationStep>::signalObjectDeleted, this is
   // less useful in practice because (a) we get updates about FermentationSteps in Fermentationes other than the one we are watching
   // (so we have to filter them out) and (b) when a new FermentationStep is created, it doesn't have a Fermentation, so it's not useful
   // for us to receive a signal about it until after it has been added to a Fermentation.  Fortunately, all we have to do is
   // connect to the Fermentation we are watching and listen for Fermentation::mashStepsChanged, which we'll get whenever a FermentationStep is
   // added to, or removed from, the Fermentation, as well as when the FermentationStep order changes.  We then just reread all the
   // FermentationSteps from the Fermentation which gives us simplicity for a miniscule overhead (because the number of FermentationSteps in a
   // Fermentation is never going to be enormous).
   //
   return;
}

FermentationStepTableModel::~FermentationStepTableModel() = default;

void FermentationStepTableModel::added  ([[maybe_unused]] std::shared_ptr<FermentationStep> item) { return; }
void FermentationStepTableModel::removed([[maybe_unused]] std::shared_ptr<FermentationStep> item) { return; }
void FermentationStepTableModel::updateTotals()                                      { return; }

QVariant FermentationStepTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_stepOwnerObs || !this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   // No special handling required for any of our columns
   return this->readDataFromModel(index, role);
}

Qt::ItemFlags FermentationStepTableModel::flags(const QModelIndex& index ) const {
   auto const columnIndex = static_cast<FermentationStepTableModel::ColumnIndex>(index.column());
   if (columnIndex == FermentationStepTableModel::ColumnIndex::Name) {
      return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool FermentationStepTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_stepOwnerObs || !this->indexAndRoleOk(index, role)) {
      return false;
   }

   // No special handling required for any of our columns
   return this->writeDataToModel(index, value, role);
}

/////==========================CLASS FermentationStepItemDelegate===============================

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(FermentationStep, fermentationStep, PropertyNames::Recipe::fermentationId)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
STEP_TABLE_MODEL_COMMON_CODE(Fermentation)
//=============================================== CLASS FermentationStepItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(FermentationStep)
