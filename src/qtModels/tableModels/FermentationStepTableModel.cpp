/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/FermentationStepTableModel.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#include "qtModels/tableModels/FermentationStepTableModel.h"

#include <QAbstractTableModel>
#include <QComboBox>
#include <QHeaderView>
#include <QStyledItemDelegate>
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

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_FermentationStepTableModel.cpp"
#endif

COLUMN_INFOS(
   FermentationStepTableModel,
   //
   // As noted elsewhere, we store the step time for fermentation times in minutes, so we can reuse code with
   // mash steps, boil steps etc, but the Measurement system will automatically show them in days (because
   // sufficiently large numbers of minutes will get shown as days).
   //
   // TODO: What is not great is that, currently, if step time is "30 days" and you edit to replace it with "29",
   //       the system will convert this to "29 mins", which is almost certainly not what the user means.  Even if
   //       you type "29 day", it will still get converted to "29 mins", which is definitely wrong.
   //
   TABLE_MODEL_HEADER(FermentationStep, Name        , PropertyNames::     NamedEntity::name           ), // "Name"
   TABLE_MODEL_HEADER(FermentationStep, StepTime    , PropertyNames::        StepBase::stepTime_mins  ), // "Step Time"
   TABLE_MODEL_HEADER(FermentationStep, StartTemp   , PropertyNames::        StepBase::startTemp_c    ), // "Start Temp"
   TABLE_MODEL_HEADER(FermentationStep, EndTemp     , PropertyNames::            Step::endTemp_c      ), // "End Temp"
   TABLE_MODEL_HEADER(FermentationStep, StartAcidity, PropertyNames::            Step::startAcidity_pH), // "Start Acidity"
   TABLE_MODEL_HEADER(FermentationStep, EndAcidity  , PropertyNames::            Step::endAcidity_pH  ), // "End Acidity"
   TABLE_MODEL_HEADER(FermentationStep, StartGravity, PropertyNames::    StepExtended::startGravity_sg), // "Start Gravity"
   TABLE_MODEL_HEADER(FermentationStep, EndGravity  , PropertyNames::    StepExtended::  endGravity_sg), // "End Gravity"
   TABLE_MODEL_HEADER(FermentationStep, FreeRise    , PropertyNames::FermentationStep::freeRise       ), // "Free Rise"
   TABLE_MODEL_HEADER(FermentationStep, Vessel      , PropertyNames::FermentationStep::vessel         ), // "Vessel"
)

FermentationStepTableModel::FermentationStepTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<FermentationStepTableModel, FermentationStep>{},
   EnumeratedItemTableModelBase<FermentationStepTableModel, FermentationStep, Fermentation>{} {

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &FermentationStepTableModel::contextMenu);

   //
   // See comment in qtModels/tableModels/BoilStepTableModel.cpp for why we don't listen directly to signals from
   // ObjectStore.
   //
   return;
}

FermentationStepTableModel::~FermentationStepTableModel() = default;

void FermentationStepTableModel::added  ([[maybe_unused]] std::shared_ptr<FermentationStep> item) { return; }
void FermentationStepTableModel::removed([[maybe_unused]] std::shared_ptr<FermentationStep> item) { return; }
void FermentationStepTableModel::updateTotals()                                      { return; }

QVariant FermentationStepTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_itemOwnerObs) {
      return QVariant();
   }
   return this->doDataDefault(index, role);
}

bool FermentationStepTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_itemOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

//========================================= CLASS FermentationStepItemDelegate =========================================

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(FermentationStep, fermentationStep, PropertyNames::Recipe::fermentationId)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
ENUMERATED_ITEM_TABLE_MODEL_COMMON_CODE(FermentationStep, Fermentation)
//========================================= CLASS FermentationStepItemDelegate =========================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(FermentationStep)
