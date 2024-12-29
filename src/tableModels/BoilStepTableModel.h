/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/BoilStepTableModel.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef TABLEMODELS_BOILSTEPTABLEMODEL_H
#define TABLEMODELS_BOILSTEPTABLEMODEL_H
#pragma once

#include <QItemDelegate>

#include "model/BoilStep.h"
#include "tableModels/BtTableModel.h"
#include "tableModels/ItemDelegate.h"
#include "tableModels/StepTableModelBase.h"
#include "tableModels/TableModelBase.h"

TABLE_MODEL_TRAITS(BoilStep, Name        ,
                             StepTime    ,
                             StartTemp   ,
                             RampTime    ,
                             EndTemp     ,
                             StartAcidity,
                             EndAcidity  ,
                             StartGravity,
                             EndGravity  ,
                             ChillingType,)

/*!
 * \class BoilStepTableModel
 *
 * \brief Model for the list of boil steps in a boil.
 */
class BoilStepTableModel : public BtTableModel,
                           public TableModelBase<BoilStepTableModel, BoilStep>,
                           public StepTableModelBase<BoilStepTableModel, BoilStep, Boil> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(BoilStep)
   STEP_TABLE_MODEL_COMMON_DECL(Boil)
};

//============================================ CLASS BoilStepItemDelegate ==============================================

/**
 * \class BoilStepItemDelegate
 *
 * \brief An item delegate for hop tables.
 * \sa BoilStepTableModel
 */
class BoilStepItemDelegate : public QItemDelegate,
                             public ItemDelegate<BoilStepItemDelegate, BoilStepTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(BoilStep)
};

#endif
