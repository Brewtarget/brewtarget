/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/FermentationStepTableModel.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef TABLEMODELS_FERMENTATIONSTEPTABLEMODEL_H
#define TABLEMODELS_FERMENTATIONSTEPTABLEMODEL_H
#pragma once

#include <QItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include "measurement/Unit.h"
#include "model/FermentationStep.h"
#include "model/Fermentation.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/StepTableModelBase.h"
#include "qtModels/tableModels/TableModelBase.h"

// You have to get the order of everything right with traits classes, but the end result is that we can refer to
// HopTableModel::ColumnIndex::Alpha etc.
class FermentationStepTableModel;
template <> struct TableModelTraits<FermentationStepTableModel> {
   enum class ColumnIndex {
      // NB: FermentationStep does not support rampTime_mins -- see comment in model/Step.h
      Name        ,
      StepTime    ,
      StartTemp   ,
      EndTemp     ,
      StartAcidity,
      EndAcidity  ,
      StartGravity,
      EndGravity  ,
      FreeRise    ,
      Vessel      ,
   };
};

/*!
 * \class FermentationStepTableModel
 *
 * \brief Model for the list of fermentation steps in a fermentation.
 */
class FermentationStepTableModel : public BtTableModel,
                                   public TableModelBase<FermentationStepTableModel, FermentationStep>,
                                   public StepTableModelBase<FermentationStepTableModel, FermentationStep, Fermentation> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(FermentationStep)
   STEP_TABLE_MODEL_COMMON_DECL(Fermentation)
};

//============================================ CLASS FermentationStepItemDelegate ==============================================

/**
 * \class FermentationStepItemDelegate
 *
 * \brief An item delegate for hop tables.
 * \sa FermentationStepTableModel
 */
class FermentationStepItemDelegate : public QItemDelegate,
                                     public ItemDelegate<FermentationStepItemDelegate, FermentationStepTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(FermentationStep)
};

#endif
