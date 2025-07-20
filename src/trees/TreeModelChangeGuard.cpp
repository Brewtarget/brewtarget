/*======================================================================================================================
 * trees/TreeModelChangeGuard.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#include "trees/TreeModelChangeGuard.h"

#include <QDebug>

TreeModelChangeGuard::TreeModelChangeGuard(TreeModelChangeType const changeType,
                                           TreeModel & model,
                                           QModelIndex const & parent,
                                           int const first,
                                           int const last) :
   m_changeType{changeType},
   m_model{model} {
   Q_ASSERT(first <= last);
   // Normally leave this debug statement commented out as otherwise it generates too much logging
//   qDebug() <<
//      Q_FUNC_INFO << "Prepare to" << this->m_changeType << ":" << first << "-" << last << "for parent" << parent;
   switch (this->m_changeType) {
      case TreeModelChangeType::InsertRows  : emit this->m_model.beginInsertRows(parent, first, last); break;
      case TreeModelChangeType::RemoveRows  : emit this->m_model.beginRemoveRows(parent, first, last); break;
      case TreeModelChangeType::ChangeLayout: emit this->m_model.layoutAboutToBeChanged(); break;
      // NB: No default clause, as we want compiler to warn us if we missed a case above
   }
   return;
}

TreeModelChangeGuard::~TreeModelChangeGuard() {
   // Normally leave this debug statement commented out as otherwise it generates too much logging
//   qDebug() << Q_FUNC_INFO << "End of" << this->m_changeType;
   switch (this->m_changeType) {
      case TreeModelChangeType::InsertRows  : emit this->m_model.endInsertRows(); break;
      case TreeModelChangeType::RemoveRows  : emit this->m_model.endRemoveRows(); break;
      case TreeModelChangeType::ChangeLayout: emit this->m_model.layoutChanged(); break;
      // NB: No default clause, as we want compiler to warn us if we missed a case above
   }
   return;
}
