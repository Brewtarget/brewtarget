/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/BoilStepTableModel.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#include "qtModels/tableModels/BoilStepTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QTableView>
#include <QVariant>
#include <QWidget>

#include "model/BoilStep.h"
#include "qtModels/tableModels/BtTableModel.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BoilStepTableModel.cpp"
#endif

BoilStepTableModel::BoilStepTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(BoilStep, Name        , tr("Name"         ), PropertyNames:: NamedEntity::name           ),
         TABLE_MODEL_HEADER(BoilStep, StepTime    , tr("Step Time"    ), PropertyNames::    StepBase::stepTime_mins  /*, PrecisionInfo{0}*/),
         TABLE_MODEL_HEADER(BoilStep, StartTemp   , tr("Start Temp"   ), PropertyNames::    StepBase::startTemp_c    /*, PrecisionInfo{1}*/),
         TABLE_MODEL_HEADER(BoilStep, RampTime    , tr("Ramp Time"    ), PropertyNames::    StepBase::rampTime_mins  /*, PrecisionInfo{0}*/),
         TABLE_MODEL_HEADER(BoilStep, EndTemp     , tr("End Temp"     ), PropertyNames::        Step::endTemp_c      /*, PrecisionInfo{1}*/),
         TABLE_MODEL_HEADER(BoilStep, StartAcidity, tr("Start Acidity"), PropertyNames::        Step::startAcidity_pH/*, PrecisionInfo{1}*/),
         TABLE_MODEL_HEADER(BoilStep, EndAcidity  , tr("End Acidity"  ), PropertyNames::        Step::endAcidity_pH  /*, PrecisionInfo{1}*/),
         TABLE_MODEL_HEADER(BoilStep, StartGravity, tr("Start Gravity"), PropertyNames::StepExtended::startGravity_sg),
         TABLE_MODEL_HEADER(BoilStep, EndGravity  , tr("End Gravity"  ), PropertyNames::StepExtended::  endGravity_sg),
         TABLE_MODEL_HEADER(BoilStep, ChillingType, tr("Chilling Type"), PropertyNames::    BoilStep::chillingType   /*, EnumInfo{BoilStep::chillingTypeStringMapping,
                                                                                                                                BoilStep::chillingTypeDisplayNames}*/),
      }
   },
   TableModelBase<BoilStepTableModel, BoilStep>{},
   StepTableModelBase<BoilStepTableModel, BoilStep, Boil>{} {

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &BoilStepTableModel::contextMenu);
   //
   // Whilst, in principle, we could connect to ObjectStoreTyped<BoilStep>::getInstance() to listen for signals
   // &ObjectStoreTyped<BoilStep>::signalObjectInserted and &ObjectStoreTyped<BoilStep>::signalObjectDeleted, this is
   // less useful in practice because (a) we get updates about BoilSteps in Boiles other than the one we are watching
   // (so we have to filter them out) and (b) when a new BoilStep is created, it doesn't have a Boil, so it's not useful
   // for us to receive a signal about it until after it has been added to a Boil.  Fortunately, all we have to do is
   // connect to the Boil we are watching and listen for Boil::mashStepsChanged, which we'll get whenever a BoilStep is
   // added to, or removed from, the Boil, as well as when the BoilStep order changes.  We then just reread all the
   // BoilSteps from the Boil which gives us simplicity for a miniscule overhead (because the number of BoilSteps in a
   // Boil is never going to be enormous).
   //
   return;
}

BoilStepTableModel::~BoilStepTableModel() = default;

void BoilStepTableModel::added  ([[maybe_unused]] std::shared_ptr<BoilStep> item) { return; }
void BoilStepTableModel::removed([[maybe_unused]] std::shared_ptr<BoilStep> item) { return; }
void BoilStepTableModel::updateTotals()                                           { return; }

QVariant BoilStepTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_stepOwnerObs) {
      return QVariant();
   }
   return this->doDataDefault(index, role);
}

Qt::ItemFlags BoilStepTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<BoilStepTableModel>(index, this->m_editable);
}

bool BoilStepTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_stepOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(BoilStep, boilStep, PropertyNames::Recipe::boilId)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
STEP_TABLE_MODEL_COMMON_CODE(Boil)
//=============================================== CLASS BoilStepItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(BoilStep)
