/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/FermentationTableModel.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "qtModels/tableModels/FermentationTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QTableView>
#include <QVariant>
#include <QWidget>

#include "model/Fermentation.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_FermentationTableModel.cpp"

FermentationTableModel::FermentationTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(Fermentation, Name    , tr("Name"           ), PropertyNames::  NamedEntity::name    ),
         TABLE_MODEL_HEADER(Fermentation, NumSteps, tr("Number of Steps"), PropertyNames::StepOwnerBase::numSteps),
      }
   },
   TableModelBase<FermentationTableModel, Fermentation>{} {

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &FermentationTableModel::contextMenu);
   return;
}

FermentationTableModel::~FermentationTableModel() = default;

void FermentationTableModel::added  ([[maybe_unused]] std::shared_ptr<Fermentation> item) { return; }
void FermentationTableModel::removed([[maybe_unused]] std::shared_ptr<Fermentation> item) { return; }
void FermentationTableModel::updateTotals()                                               { return; }

QVariant FermentationTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

Qt::ItemFlags FermentationTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<FermentationTableModel>(index, this->m_editable);
}

bool FermentationTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Fermentation, fermentation, PropertyNames::Recipe::fermentationId)

//=============================================== CLASS FermentationItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Fermentation)
