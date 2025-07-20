/*======================================================================================================================
 * widgets/StepsWidget.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef WIDGETS_STEPSWIDGET_H
#define WIDGETS_STEPSWIDGET_H
#pragma once

#include <memory>

#include <QHBoxLayout>
#include <QModelIndexList>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

#include "widgets/StepsWidgetBase.h"

/**
 * \brief A class to show (and allow editing of) \c MashStep items in a \c Mash, \c BoilStep items in a \c Boil, etc.
 *
 *        Used both in the \c MainWindow tabs (\c mashStepsTab etc) and the relevant editors (\c MashEditor etc).
 */
class StepsWidget : public QWidget {
   Q_OBJECT

public:
   StepsWidget(QWidget * parent);
   virtual ~StepsWidget();


   //! Save the UI state of the table against the supplied property
   void    saveUiState(BtStringConst const & property, BtStringConst const & section) const;
   void restoreUiState(BtStringConst const & property, BtStringConst const & section);

   //! UI elements are public
   std::unique_ptr<QHBoxLayout> m_horizontalLayout_main;
   std::unique_ptr<QTableView > m_tableView_steps;
   std::unique_ptr<QVBoxLayout> m_verticalLayout_buttons;
   std::unique_ptr<QPushButton> m_pushButton_addStep;
   std::unique_ptr<QPushButton> m_pushButton_removeStep;
   std::unique_ptr<QPushButton> m_pushButton_moveStepUp;
   std::unique_ptr<QPushButton> m_pushButton_moveStepDown;
   std::unique_ptr<QPushButton> m_pushButton_editStep;

   QIcon m_icon_addStep;
   QIcon m_icon_removeStep;
   QIcon m_icon_moveStepUp;
   QIcon m_icon_moveStepDown;
   QIcon m_icon_editStep;
};


//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "editors/MashStepEditor.h"
#include "model/Mash.h"
#include "qtModels/tableModels/MashStepTableModel.h"

class MashStepsWidget : public StepsWidget,
                        public StepsWidgetBase<MashStepsWidget, Mash> {
   Q_OBJECT

   STEPS_WIDGET_COMMON_DECL(Mash)
};

//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "editors/BoilStepEditor.h"
#include "model/Boil.h"
#include "qtModels/tableModels/BoilStepTableModel.h"

class BoilStepsWidget : public StepsWidget,
                        public StepsWidgetBase<BoilStepsWidget, Boil> {
   Q_OBJECT

   STEPS_WIDGET_COMMON_DECL(Boil)
};

//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "editors/FermentationStepEditor.h"
#include "model/Fermentation.h"
#include "qtModels/tableModels/FermentationStepTableModel.h"

class FermentationStepsWidget : public StepsWidget,
                                public StepsWidgetBase<FermentationStepsWidget, Fermentation> {
   Q_OBJECT

   STEPS_WIDGET_COMMON_DECL(Fermentation)
};


#endif
