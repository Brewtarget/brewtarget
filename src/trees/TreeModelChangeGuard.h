/*======================================================================================================================
 * trees/TreeModelChangeGuard.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#ifndef TREES_TREEMODELCHANGEGUARD_H
#define TREES_TREEMODELCHANGEGUARD_H
#pragma once

#include "trees/TreeModel.h"

//! The types of \c QAbstractItemModel change that \c TreeModelChangeGuard supports
enum class TreeModelChangeType {
   InsertRows,
   RemoveRows
};

/**
 * \brief Any time we change the tree structure, we need to call beginInsertRows() and endInsertRows() to notify
 *        other components that the model has changed.  This RAII class handles that for us.
 *
 *        NOTE that because \c QAbstractItemModel::beginInsertRows, \c QAbstractItemModel::endInsertRows,
 *        \c QAbstractItemModel::beginRemoveRows, \c QAbstractItemModel::endRemoveRows etc are protected, this class
 *        needs to be a friend of \c TreeModel.  (And this is why I didn't template this class on
 *        \c TreeModelChangeType, even though it's a constant known at compile-time -- because we'd end up with circular
 *        dependencies which would be a bit tiresome to eliminate.)
 */
class TreeModelChangeGuard {
public:
   TreeModelChangeGuard(TreeModelChangeType const changeType,
                        TreeModel & model,
                        QModelIndex const & parent,
                        int const first,
                        int const last);
   ~TreeModelChangeGuard();
private:
   TreeModelChangeType const m_changeType;
   TreeModel & m_model;
};

#endif
