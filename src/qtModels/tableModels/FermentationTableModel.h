/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/FermentationTableModel.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef TABLEMODELS_FERMENTATIONTABLEMODEL_H
#define TABLEMODELS_FERMENTATIONTABLEMODEL_H
#pragma once

#include <QItemDelegate>

#include "model/Fermentation.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

TABLE_MODEL_TRAITS(Fermentation, Name,
                                 NumSteps)

/*!
 * \class FermentationTableModel
 *
 * \brief Model class for a list of boils.
 */
class FermentationTableModel : public BtTableModel, public TableModelBase<FermentationTableModel, Fermentation> {
   Q_OBJECT
   TABLE_MODEL_COMMON_DECL(Fermentation)
};

/**
 * \class FermentationItemDelegate
 *
 * \brief An item delegate for hop tables.
 * \sa FermentationTableModel
 */
class FermentationItemDelegate : public QItemDelegate, public ItemDelegate<FermentationItemDelegate, FermentationTableModel> {
   Q_OBJECT
   ITEM_DELEGATE_COMMON_DECL(Fermentation)
};

#endif
