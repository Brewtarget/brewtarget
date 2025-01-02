/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/BoilTableModel.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "qtModels/tableModels/BoilTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QTableView>
#include <QVariant>
#include <QWidget>

#include "model/Boil.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BoilTableModel.cpp"
#endif

BoilTableModel::BoilTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(Boil, Name        , tr("Name"         ), PropertyNames:: NamedEntity::name           ),
         TABLE_MODEL_HEADER(Boil, PreBoilSize , tr("Pre-Boil Size"), PropertyNames::        Boil::preBoilSize_l  , PrecisionInfo{1}),
      }
   },
   TableModelBase<BoilTableModel, Boil>{} {

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &BoilTableModel::contextMenu);
   return;
}

BoilTableModel::~BoilTableModel() = default;

void BoilTableModel::added  ([[maybe_unused]] std::shared_ptr<Boil> item) { return; }
void BoilTableModel::removed([[maybe_unused]] std::shared_ptr<Boil> item) { return; }
void BoilTableModel::updateTotals()                                       { return; }

QVariant BoilTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

Qt::ItemFlags BoilTableModel::flags(QModelIndex const & index) const {
   return TableModelHelper::doFlags<BoilTableModel>(index, this->m_editable);
}

bool BoilTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Boil, boil, PropertyNames::Recipe::boilId)

//=============================================== CLASS BoilItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Boil)
