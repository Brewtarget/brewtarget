/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/MashStepTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef TABLEMODELS_MASHSTEPTABLEMODEL_H
#define TABLEMODELS_MASHSTEPTABLEMODEL_H
#pragma once

#include <QItemDelegate>

#include "model/MashStep.h"
#include "model/Mash.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/StepTableModelBase.h"
#include "qtModels/tableModels/TableModelBase.h"

// Define the columns on this table
TABLE_MODEL_TRAITS(MashStep, Name      ,
                             Type      ,
                             Amount    ,
                             Temp      ,
                             TargetTemp,
                             Time      ,)

/*!
 * \class MashStepTableModel
 *
 * \brief Model for the list of mash steps in a mash.
 */
class MashStepTableModel : public BtTableModel,
                           public TableModelBase<MashStepTableModel, MashStep>,
                           public StepTableModelBase<MashStepTableModel, MashStep, Mash> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(MashStep)
   STEP_TABLE_MODEL_COMMON_DECL(Mash)

};

//============================================ CLASS MashStepItemDelegate ==============================================

/**
 * \class MashStepItemDelegate
 *
 * \brief An item delegate for hop tables.
 * \sa MashStepTableModel
 */
class MashStepItemDelegate : public QItemDelegate,
                             public ItemDelegate<MashStepItemDelegate, MashStepTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(MashStep)
};

#endif
