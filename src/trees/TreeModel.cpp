/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "trees/TreeModel.h"

#include <QAbstractItemModel>
#include <Qt>

#include "model/Folder.h"
#include "trees/TreeView.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_TreeModel.cpp"
#endif

TreeModel::TreeModel(TreeView * parent) :
   QAbstractItemModel{parent} {
   return;
}

TreeModel::~TreeModel() = default;

Qt::ItemFlags TreeModel::flags(QModelIndex const & index) const {
   if (!index.isValid()) {
      return Qt::ItemIsDropEnabled;
   }

   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled |
          Qt::ItemIsDropEnabled;
}

bool TreeModel::insertRows([[maybe_unused]] int row,
                           [[maybe_unused]] int count,
                           [[maybe_unused]] QModelIndex const & parent) {
   // It's a coding error to call this function!
   qCritical() << Q_FUNC_INFO << "Error: Rows not added via TreeModelBase::insertChild";
   Q_ASSERT(false);
   return false;
}

bool TreeModel::removeRows([[maybe_unused]] int row,
                           [[maybe_unused]] int count,
                           [[maybe_unused]] QModelIndex const & parent) {
   // It's a coding error to call this function!
   qCritical() << Q_FUNC_INFO << "Error: Rows not added via TreeModelBase::removeChildren";
   Q_ASSERT(false);
   return false;
}

Qt::DropActions TreeModel::supportedDropActions() const {
   return Qt::CopyAction | Qt::MoveAction;
}

//============================================== TreeModelRowInsertGuard ===============================================

TreeModelRowInsertGuard::TreeModelRowInsertGuard(TreeModel & model,
                                                 QModelIndex const & parent,
                                                 int const first,
                                                 int const last) :
   m_model{model} {
   Q_ASSERT(first <= last);
   this->m_model.beginInsertRows(parent, first, last);
   return;
}

TreeModelRowInsertGuard::~TreeModelRowInsertGuard() {
   this->m_model.endInsertRows();
   return;
}
