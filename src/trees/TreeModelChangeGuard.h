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
   InsertRows  ,
   RemoveRows  ,
   ChangeLayout,
};

/**
 * \brief Any time we change the tree structure, we need to call such things as \c beginInsertRows() and
 *        \c endInsertRows() to notify other components about the change to the model has changed.  This RAII class
 *        handles that for us.
 *
 *        There are several different types of change that can be handled - per \c TreeModelChangeType.  NOTE that we
 *        have to be careful only to make these calls when they are needed.  Eg, if you notify about layout change and
 *        row insertion together, you can put Qt internals in a funny state that results in crashes.
 *
 * \param parent is not required for \c changeType of \c ChangeLayout
 * \param first  is not required for \c changeType of \c ChangeLayout
 * \param last   is not required for \c changeType of \c ChangeLayout
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
                        QModelIndex const & parent = QModelIndex(),
                        int const first = -1,
                        int const last  = -1);
   ~TreeModelChangeGuard();
private:
   TreeModelChangeType const m_changeType;
   TreeModel & m_model;
};

/**
 * \brief Convenience functions for logging
 */
/**@{*/
template<class S> S & operator<<(S & stream, TreeModelChangeType const val);

template<class S>
S & operator<<(S & stream, TreeModelChangeType const val) {
   switch (val) {
      case TreeModelChangeType::InsertRows  : stream << "insert rows"; break;
      case TreeModelChangeType::RemoveRows  : stream << "remove rows"; break;
      case TreeModelChangeType::ChangeLayout: stream << "change layout"; break;
      // NB: No default clause, as we want compiler to warn us if we missed a case above
   }
   return stream;
}
/**@}*/

#endif
