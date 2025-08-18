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

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_FermentationStepTableModel.cpp"
#endif

FermentationStepTableModel::FermentationStepTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         //
         // As noted elsewhere, we store the step time for fermentation times in minutes, so we can reuse code with
         // mash steps, boil steps etc, but the Measurement system will automatically show them in days (because
         // sufficiently large numbers of minutes will get shown as days).
         //
         // TODO: What is not great is that, currently, if step time is "30 days" and you edit to replace it with "29",
         //       the system will convert this to "29 mins", which is almost certainly not what the user means.  Even if
         //       you type "29 day", it will still get converted to "29 mins", which is definitely wrong.
         //
         TABLE_MODEL_HEADER(FermentationStep, Name        , tr("Name"         ), PropertyNames::     NamedEntity::name           ),
         TABLE_MODEL_HEADER(FermentationStep, StepTime    , tr("Step Time"    ), PropertyNames::        StepBase::stepTime_mins  /*, PrecisionInfo{0}*/),
         TABLE_MODEL_HEADER(FermentationStep, StartTemp   , tr("Start Temp"   ), PropertyNames::        StepBase::startTemp_c    /*, PrecisionInfo{1}*/),
         TABLE_MODEL_HEADER(FermentationStep, EndTemp     , tr("End Temp"     ), PropertyNames::            Step::endTemp_c      /*, PrecisionInfo{1}*/),
         TABLE_MODEL_HEADER(FermentationStep, StartAcidity, tr("Start Acidity"), PropertyNames::            Step::startAcidity_pH/*, PrecisionInfo{1}*/),
         TABLE_MODEL_HEADER(FermentationStep, EndAcidity  , tr("End Acidity"  ), PropertyNames::            Step::endAcidity_pH  /*, PrecisionInfo{1}*/),
         TABLE_MODEL_HEADER(FermentationStep, StartGravity, tr("Start Gravity"), PropertyNames::    StepExtended::startGravity_sg),
         TABLE_MODEL_HEADER(FermentationStep, EndGravity  , tr("End Gravity"  ), PropertyNames::    StepExtended::  endGravity_sg),
         TABLE_MODEL_HEADER(FermentationStep, FreeRise    , tr("Free Rise"    ), PropertyNames::FermentationStep::freeRise       /*, BoolInfo{tr("No"), tr("Yes")}*/),
         TABLE_MODEL_HEADER(FermentationStep, Vessel      , tr("Vessel"       ), PropertyNames::FermentationStep::vessel         ),
      }
   },
   TableModelBase<FermentationStepTableModel, FermentationStep>{},
   StepTableModelBase<FermentationStepTableModel, FermentationStep, Fermentation>{} {

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
   if (!this->m_stepOwnerObs) {
      return QVariant();
   }
   return this->doDataDefault(index, role);
}

Qt::ItemFlags FermentationStepTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<FermentationStepTableModel>(index, this->m_editable);
}

bool FermentationStepTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_stepOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

/////==========================CLASS FermentationStepItemDelegate===============================

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(FermentationStep, fermentationStep, PropertyNames::Recipe::fermentationId)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
STEP_TABLE_MODEL_COMMON_CODE(Fermentation)
//=============================================== CLASS FermentationStepItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(FermentationStep)
