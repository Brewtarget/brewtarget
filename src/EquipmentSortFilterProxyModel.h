/*
 * StyleSortFilterProxyModel.h is part of Brewtarget, and is Copyright the following
 * authors 2016
 * - Rodrigo Farias Andriolo <rondonctba@outlook.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EQUIPMENTSORTFILTERPROXYMODEL_H
#define EQUIPMENTSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/*!
 * \class EquipmentSortFilterProxyModel
 * \author Rodrigo F. Andriolo
 *
 * \brief Proxy model for sorting/filtering Equipments.
 */
class EquipmentSortFilterProxyModel : public QSortFilterProxyModel
{
   Q_OBJECT

public:

   EquipmentSortFilterProxyModel(QObject* parent = 0);

protected:
  // Magic fancy stuff can be done in here!
  // bool filterAcceptsRow( int source_row, const QModelIndex &source_parent) const;
};

#endif // EQUIPMENTSORTFILTERPROXYMODEL_H
