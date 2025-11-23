/*======================================================================================================================
 * widgets/EnumeratedItemsWidget.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef WIDGETS_ENUMERATEDITEMSWIDGET_H
#define WIDGETS_ENUMERATEDITEMSWIDGET_H
#pragma once

#include <memory>

#include <QHBoxLayout>
#include <QModelIndexList>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

#include "widgets/EnumeratedItemsWidgetBase.h"

/**
 * \brief A class to show (and allow editing of) \c MashStep items in a \c Mash, \c BoilStep items in a \c Boil, etc.
 *
 *        Used both in the \c MainWindow tabs (\c mashStepsTab etc) and the relevant editors (\c MashEditor etc).
 */
class EnumeratedItemsWidget : public QWidget {
   Q_OBJECT

public:
   EnumeratedItemsWidget(QWidget * parent);
   virtual ~EnumeratedItemsWidget();


   //! Save the UI state of the table against the supplied property
   void    saveUiState(BtStringConst const & property, BtStringConst const & section) const;
   bool restoreUiState(BtStringConst const & property, BtStringConst const & section);

   //! UI elements are public
   std::unique_ptr<QHBoxLayout> m_horizontalLayout_main;
   std::unique_ptr<QTableView > m_tableView_items;
   std::unique_ptr<QVBoxLayout> m_verticalLayout_buttons;
   std::unique_ptr<QPushButton> m_pushButton_addItem;
   std::unique_ptr<QPushButton> m_pushButton_removeItem;
   std::unique_ptr<QPushButton> m_pushButton_moveItemUp;
   std::unique_ptr<QPushButton> m_pushButton_moveItemDown;
   std::unique_ptr<QPushButton> m_pushButton_editItem;

   QIcon m_icon_addItem;
   QIcon m_icon_removeItem;
   QIcon m_icon_moveItemUp;
   QIcon m_icon_moveItemDown;
   QIcon m_icon_editItem;
};


//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "editors/MashStepEditor.h"
#include "model/Mash.h"
#include "qtModels/tableModels/MashStepTableModel.h"

class MashStepsWidget : public EnumeratedItemsWidget,
                        public EnumeratedItemsWidgetBase<MashStepsWidget, MashStep> {
   Q_OBJECT

   ENUMERATED_ITEMS_WIDGET_COMMON_DECL(MashStep)
};

//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "editors/BoilStepEditor.h"
#include "model/Boil.h"
#include "qtModels/tableModels/BoilStepTableModel.h"

class BoilStepsWidget : public EnumeratedItemsWidget,
                        public EnumeratedItemsWidgetBase<BoilStepsWidget, BoilStep> {
   Q_OBJECT

   ENUMERATED_ITEMS_WIDGET_COMMON_DECL(BoilStep)
};

//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "editors/FermentationStepEditor.h"
#include "model/Fermentation.h"
#include "qtModels/tableModels/FermentationStepTableModel.h"

class FermentationStepsWidget : public EnumeratedItemsWidget,
                                public EnumeratedItemsWidgetBase<FermentationStepsWidget,
                                                                 FermentationStep> {
   Q_OBJECT

   ENUMERATED_ITEMS_WIDGET_COMMON_DECL(FermentationStep)
};

//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "editors/StockUseIngredientEditor.h"
#include "model/StockUseIngredient.h"
#include "qtModels/tableModels/StockUseFermentableTableModel.h"

class StockUseFermentablesWidget : public EnumeratedItemsWidget,
                                          public EnumeratedItemsWidgetBase<StockUseFermentablesWidget,
                                                                           StockUseFermentable> {
   Q_OBJECT

   ENUMERATED_ITEMS_WIDGET_COMMON_DECL(StockUseFermentable)
};

//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "qtModels/tableModels/StockUseHopTableModel.h"

class StockUseHopsWidget : public EnumeratedItemsWidget,
                                  public EnumeratedItemsWidgetBase<StockUseHopsWidget,
                                                                   StockUseHop> {
   Q_OBJECT

   ENUMERATED_ITEMS_WIDGET_COMMON_DECL(StockUseHop)
};
//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "qtModels/tableModels/StockUseMiscTableModel.h"

class StockUseMiscsWidget : public EnumeratedItemsWidget,
                                   public EnumeratedItemsWidgetBase<StockUseMiscsWidget,
                                                                    StockUseMisc> {
   Q_OBJECT

   ENUMERATED_ITEMS_WIDGET_COMMON_DECL(StockUseMisc)
};
//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "qtModels/tableModels/StockUseSaltTableModel.h"

class StockUseSaltsWidget : public EnumeratedItemsWidget,
                                   public EnumeratedItemsWidgetBase<StockUseSaltsWidget,
                                                                    StockUseSalt> {
   Q_OBJECT

   ENUMERATED_ITEMS_WIDGET_COMMON_DECL(StockUseSalt)
};
//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "qtModels/tableModels/StockUseYeastTableModel.h"

class StockUseYeastsWidget : public EnumeratedItemsWidget,
                                    public EnumeratedItemsWidgetBase<StockUseYeastsWidget,
                                                                     StockUseYeast> {
   Q_OBJECT

   ENUMERATED_ITEMS_WIDGET_COMMON_DECL(StockUseYeast)
};

#endif
