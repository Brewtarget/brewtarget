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

#include <functional>

#include <QColor>
#include <QHash>
#include <QList>
#include <QMetaObject>
#include <QStringList>
#include <QStyledItemDelegate>

#include "model/MashStep.h"
#include "MashAutoAdjuster.h"
#include "model/Mash.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/EnumeratedItemTableModelBase.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

class Recipe;

// Define the columns on this table
COLUMN_NAMES(MashStepTableModel, Name        ,
                                 Type        ,
                                 Amount      ,
                                 Temp        ,
                                 TargetTemp  ,
                                 EndTemp     ,
                                 RampTime    ,
                                 Time        ,
                                 StartAcidity,
                                 EndAcidity  ,
                                 WaterToGrain,)

/*!
 * \class MashStepTableModel
 *
 * \brief Model for the list of mash steps in a mash.
 */
class MashStepTableModel : public BtTableModel,
                           public TableModelBase<MashStepTableModel, MashStep>,
                           public EnumeratedItemTableModelBase<MashStepTableModel, MashStep, Mash> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(MashStep)
   ENUMERATED_ITEM_TABLE_MODEL_COMMON_DECL(MashStep, Mash)

public slots:
   /**
    * \brief Context menu for a cell in the table.  For the amount and infusion temperature columns, this allows the user
    *        to toggle whether the value is "pinned" (aka "fixed"), ie not changed when adjusting the mash (see \c MashAutoAdjuster).
    */
   void cellContextMenu(QPoint const & point);

   /**
    * \brief Re-check the steps (see \c validate) the next time we get back to the event loop.  Multiple calls before
    *        then result in only one check.
    */
   void scheduleValidation();

signals:
   //! Emitted when the results of checking the steps (see \c validate) change
   void validationChanged();

public:
   //
   // Colors used to highlight problems found by checking the steps (see \c validate).  They are semi-transparent, so
   // that they work with both light and dark themes.  A row with a problem is tinted with the "row" color, and, where
   // we know which value in the row is the cause of the problem, that cell is tinted with the stronger "cell" color.
   //
   // MashEditor uses the same colors for highlighting problems with fields of the mash itself.
   //
   static QColor const errorRowColor;
   static QColor const errorCellColor;
   static QColor const warningRowColor;
   static QColor const warningCellColor;

   /**
    * \return \c true if the steps are being checked, ie the mash is used by exactly one recipe
    */
   bool isValidating() const;

   //! \return \c true if checking the steps found any problems, either with individual steps or the mash as a whole
   bool hasProblems() const;

   /**
    * \return Description of all the problems found when checking the steps, one per line, plus any overall problems
    *         with the mash, for showing to the user
    */
   [[nodiscard]] QString allProblemsDisplayable() const;

   //! \return Problems found with the mash as a whole (eg the water not adding up to what the recipe needs)
   [[nodiscard]] QStringList const & mashOverallProblems() const;

   /**
    * \return Problems found with particular fields of the mash itself (eg initial grain temperature), keyed by property
    *         name (eg \c PropertyNames::Mash::grainTemp_c), so that an editor can highlight the relevant fields
    */
   [[nodiscard]] QHash<QString, QStringList> const & mashFieldProblems() const;

   /**
    * \brief Normally we check the steps using the mash's stored values (eg initial grain temperature).  The Mash Editor
    *        uses this to substitute the values the user has entered but not yet saved, so the checks reflect what the
    *        user sees.  (It should call \c scheduleValidation when those values change.)
    */
   void setInputsAdjuster(std::function<void(MashAutoAdjuster::Inputs &)> adjuster);

private:
   /**
    * \brief If the mash is used by exactly one recipe, check whether its steps make sense for that recipe (see
    *        \c MashAutoAdjuster::check), so we can highlight the ones that don't.  (If the mash is used by more than one
    *        recipe, it might make sense for some and not others.)
    *
    *        We re-check whenever the steps, the mash, the recipe or the recipe's equipment change.
    */
   void validate();

   struct StepProblems {
      bool        isError  = false;
      QStringList messages = {};
      /**
       * For problems where we know which value in the step is the cause (see
       * \c MashAutoAdjuster::Problem::StepScope::stepProperty), the name of the property, mapped to whether any of the
       * problems it causes is an error (rather than a warning).
       */
      QHash<QString, bool> propertyIsError = {};
      bool operator==(StepProblems const & other) const = default;
   };
   QHash<MashStep const *, StepProblems> m_stepProblems;
   QStringList                           m_mashProblems;
   QHash<QString, QStringList>           m_mashFieldProblems;
   std::function<void(MashAutoAdjuster::Inputs &)> m_inputsAdjuster;
   Recipe *                              m_validationRecipe    = nullptr;
   bool                                  m_validationScheduled = false;
   QList<QMetaObject::Connection>        m_validationConnections;
};

//============================================ CLASS MashStepItemDelegate ==============================================

/**
 * \class MashStepItemDelegate
 *
 * \brief An item delegate for hop tables.
 * \sa MashStepTableModel
 */
class MashStepItemDelegate : public QStyledItemDelegate,
                             public ItemDelegate<MashStepItemDelegate, MashStepTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(MashStep)
};

#endif