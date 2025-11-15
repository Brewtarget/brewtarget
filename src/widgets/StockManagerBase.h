/*======================================================================================================================
 * widgets/StockManagerBase.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef WIDGETS_STOCKMANAGERBASE_H
#define WIDGETS_STOCKMANAGERBASE_H
#pragma once

#include <memory>

#include "PersistentSettings.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \class StockManagerBase
 *
 * \brief This is one of the base classes for \c StockManagerHop, \c StockManagerFermentable, etc.  Essentially each of
 *        these classes is a UI element that shows a list of all stock purchases and uses for a certain type of
 *        ingredient.
 *
 *        See catalog/CatalogBase.h for the idea behind what we're doing with the class structure here.
 *
 *              QWidget
 *                  \
 *          StockManager     StockManagerBase<StockManagerHop,
 *                      \           /         StockPurchaseHop,
 *                       \         /          StockPurchaseHopTreeView,
 *                        \       /           StockPurchaseHopEditor>
 *                         \     /
 *                     StockManagerHop
 *
 *        Classes inheriting from this one need to include the STOCK_MGR_COMMON_DECL macro in their header file and
 *        the STOCK_MGR_COMMON_CODE macro in their .cpp file.
 */
template<class Derived> class StockManagerPhantom;
template<class Derived,
         class StockPurchaseClass,
         class SpTreeViewClass>
class StockManagerBase : public CuriouslyRecurringTemplateBase<StockManagerPhantom, Derived> {
public:
   using StockPurchaseEditorClass = StockPurchaseClass::EditorClass;
   using StockUseClass            = StockPurchaseClass::StockUseClass;
   using StockUseEditorClass      = StockUseClass::EditorClass;
   StockManagerBase([[maybe_unused]] QWidget * parent) :
      m_treeView            {new SpTreeViewClass(&this->derived())},
      m_stockPurchaseEditor {std::make_unique<StockPurchaseEditorClass>(&this->derived())},
      m_stockUseEditor      {std::make_unique<StockUseEditorClass     >(&this->derived())} {

      this->derived().m_vLayout_invMgr->addWidget(this->m_treeView);
      this->derived().m_vLayout_invMgr->addLayout(this->derived().m_hLayout_controls);

      this->m_treeView->init(*this->m_stockPurchaseEditor);

      this->derived().connect(this->derived().m_searchBox_filter, &QLineEdit::textEdited   , &this->derived(), &Derived::filterItems   );
      this->derived().connect(this->derived().m_button_new      , &QAbstractButton::clicked, &this->derived(), &Derived::newItem       );
      this->derived().connect(this->derived().m_button_edit     , &QAbstractButton::clicked, &this->derived(), &Derived::editSelected  );
      this->derived().connect(this->derived().m_button_delete   , &QAbstractButton::clicked, &this->derived(), &Derived::deleteSelected);

      return;
   }

   ~StockManagerBase() = default;

   StockPurchaseEditorClass & getPurchaseEditor() const {
      return *this->m_stockPurchaseEditor;
   }

   StockUseEditorClass & getUseEditor() const {
      return *this->m_stockUseEditor;
   }

   void saveUiState(BtStringConst const & property,
                    BtStringConst const & section = PersistentSettings::Sections::StockWindow) const {
      PersistentSettings::saveUiState(property, *this->m_treeView->header(), section);
      return;
   }

   bool restoreUiState(BtStringConst const & property,
                       BtStringConst const & section = PersistentSettings::Sections::StockWindow) {
      return PersistentSettings::restoreUiState(property, *this->m_treeView->header(), section);
   }

   //================================================ MEMBER VARIABLES =================================================
   SpTreeViewClass * m_treeView;
   std::unique_ptr<StockPurchaseEditorClass> m_stockPurchaseEditor;
   std::unique_ptr<StockUseEditorClass>      m_stockUseEditor;
};


/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define STOCK_MGR_COMMON_DECL(NeName) \
   /* This allows StockManagerBase to call protected and private members of Derived */ \
   friend class StockManagerBase<StockManager##NeName,             \
                                 StockPurchase##NeName,            \
                                 StockPurchase##NeName##TreeView>; \
                                                                   \
   public:                                                         \
      StockManager##NeName(QWidget * parent);                      \
      virtual ~StockManager##NeName();                             \
                                                                   \
   public slots:                                                   \
      void newItem();                                              \
      void editSelected();                                         \
      void deleteSelected();                                       \
      void filterItems(QString searchExpression);                  \


/**
 * \brief Derived classes should include this in their implementation file
 */
#define STOCK_MGR_COMMON_CODE(NeName) \
   StockManager##NeName::StockManager##NeName(QWidget* parent) :   \
      StockManager{parent},                                        \
      StockManagerBase<StockManager##NeName,                       \
                       StockPurchase##NeName,                      \
                       StockPurchase##NeName##TreeView>{parent} {  \
      return;                                                      \
   }                                                               \
                                                                   \
   StockManager##NeName::~StockManager##NeName() = default;        \
                                                                   \
   void StockManager##NeName::newItem()                             { this->m_treeView->doNewItem();        return; } \
   void StockManager##NeName::editSelected()                        { this->m_treeView->doEditSelected();   return; } \
   void StockManager##NeName::deleteSelected()                      { this->m_treeView->doDeleteSelected(); return; } \
   void StockManager##NeName::filterItems(QString searchExpression) { this->m_treeView->filter(searchExpression); return; } \

#endif
