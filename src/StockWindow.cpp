/*======================================================================================================================
 * StockWindow.cpp is part of Brewtarget, and is copyright the following authors 2025-2026:
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
#include "StockWindow.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include "PersistentSettings.h"
#include "widgets/StockManagerIngredient.h"

// This private implementation class holds all private non-virtual members of StockWindow
class StockWindow::impl {
public:
   impl(StockWindow & self) :
      m_self{self},
      m_vLayout_Outermost            {new QVBoxLayout(&m_self)},
      m_tabWidget_StockPurchaseTrees {new QTabWidget (&m_self)},

      // TBD: Start ownership with us and then pass it to m_tabWidget_StockPurchaseTrees via addTab below
      m_stockManagerFermentable{new StockManagerFermentable(&m_self)},
      m_stockManagerHop        {new StockManagerHop        (&m_self)},
      m_stockManagerMisc       {new StockManagerMisc       (&m_self)},
      m_stockManagerSalt       {new StockManagerSalt       (&m_self)},
      m_stockManagerYeast      {new StockManagerYeast      (&m_self)} {

      this->m_self.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      //
      // For the moment at least, I think it's a nicer UI if each tab (Fermentables, Hops, Misc, etc) has its own search
      // box and controls.  The alternative would be to take the controls etc outside the tabs and have them work out
      // which tab is visible, but that means you can't have separate search filters active on each tab, and the button
      // tool-tips become more generic.
      //
      // NOTE: You might think we can share icons between layouts.  However, this is not the case.  Calling addWidget
      // calls addItem, which transfers ownership of the supplied item to the layout, making it the layout's
      // responsibility to delete.
      //

      this->m_tabWidget_StockPurchaseTrees->setTabPosition(QTabWidget::North);
      this->m_tabWidget_StockPurchaseTrees->setCurrentIndex(0);
      //
      // NB: QTabWidget::addTab passes ownership of the QWidget in the first parameter to the QTabWidget, hence why we
      //     use raw pointers.
      //
      this->m_tabWidget_StockPurchaseTrees->addTab(this->m_stockManagerFermentable, QIcon{QStringLiteral(":/images/smallBarley.svg"  )}, QObject::tr("Fermentables" ));
      this->m_tabWidget_StockPurchaseTrees->addTab(this->m_stockManagerHop        , QIcon{QStringLiteral(":/images/smallHop.svg"     )}, QObject::tr("Hops"         ));
      this->m_tabWidget_StockPurchaseTrees->addTab(this->m_stockManagerMisc       , QIcon{QStringLiteral(":/images/smallQuestion.svg")}, QObject::tr("Miscellaneous"));
      this->m_tabWidget_StockPurchaseTrees->addTab(this->m_stockManagerSalt       , QIcon{QStringLiteral(":/images/iconSalt.svg"     )}, QObject::tr("Salts"        ));
      this->m_tabWidget_StockPurchaseTrees->addTab(this->m_stockManagerYeast      , QIcon{QStringLiteral(":/images/smallYeast.svg"   )}, QObject::tr("Yeasts"       ));

      this->m_vLayout_Outermost->addWidget(this->m_tabWidget_StockPurchaseTrees);

      this->retranslateUi();

      return;
   }

   ~impl() = default;

   void retranslateUi() {
      this->m_self.setWindowTitle(QObject::tr("Stock Purchases (Inventory)"));
      this->m_tabWidget_StockPurchaseTrees->setTabText(0, QObject::tr("Fermentables" ));
      this->m_tabWidget_StockPurchaseTrees->setTabText(1, QObject::tr("Hops"         ));
      this->m_tabWidget_StockPurchaseTrees->setTabText(2, QObject::tr("Miscellaneous"));
      this->m_tabWidget_StockPurchaseTrees->setTabText(3, QObject::tr("Salts"        ));
      this->m_tabWidget_StockPurchaseTrees->setTabText(4, QObject::tr("Yeasts"       ));

      return;
   }

   // There are no general cases for these member functions, only specialisations -- which have to be outside the class
   // definition (see below).
   template<class Ingrd> void selectTab();
   template<class Ingrd> StockManager & getStockManager();

   //================================================ MEMBER VARIABLES =================================================
   StockWindow & m_self;

   //! \name UI Variables
   //! @{

   //! Top level widget needs a layout, otherwise its contents will not expand to fill the available space
   QVBoxLayout * m_vLayout_Outermost;
   QTabWidget  * m_tabWidget_StockPurchaseTrees;

   StockManagerFermentable * m_stockManagerFermentable;
   StockManagerHop         * m_stockManagerHop        ;
   StockManagerMisc        * m_stockManagerMisc       ;
   StockManagerSalt        * m_stockManagerSalt       ;
   StockManagerYeast       * m_stockManagerYeast      ;
   //! @}

};

template<> void StockWindow::impl::selectTab<Fermentable>() { this->m_tabWidget_StockPurchaseTrees->setCurrentIndex(0); return; }
template<> void StockWindow::impl::selectTab<Hop        >() { this->m_tabWidget_StockPurchaseTrees->setCurrentIndex(1); return; }
template<> void StockWindow::impl::selectTab<Misc       >() { this->m_tabWidget_StockPurchaseTrees->setCurrentIndex(2); return; }
template<> void StockWindow::impl::selectTab<Salt       >() { this->m_tabWidget_StockPurchaseTrees->setCurrentIndex(3); return; }
template<> void StockWindow::impl::selectTab<Yeast      >() { this->m_tabWidget_StockPurchaseTrees->setCurrentIndex(4); return; }

template<> StockManager & StockWindow::impl::getStockManager<Fermentable>() { return *this->m_stockManagerFermentable; }
template<> StockManager & StockWindow::impl::getStockManager<Hop        >() { return *this->m_stockManagerHop        ; }
template<> StockManager & StockWindow::impl::getStockManager<Misc       >() { return *this->m_stockManagerMisc       ; }
template<> StockManager & StockWindow::impl::getStockManager<Salt       >() { return *this->m_stockManagerSalt       ; }
template<> StockManager & StockWindow::impl::getStockManager<Yeast      >() { return *this->m_stockManagerYeast      ; }

StockWindow::StockWindow(QWidget * parent) :
   QDialog{parent},
   pimpl{std::make_unique<impl>(*this)} {

   return;
}

StockWindow::~StockWindow() = default;

void StockWindow::retranslateUi() {
   this->pimpl->retranslateUi();
   return;
}

//
// There is no general case for StockWindow::getPurchaseEditor(), only specialisations.
//
template<> StockPurchaseFermentableEditor & StockWindow::getPurchaseEditor<Fermentable>() const { return this->pimpl->m_stockManagerFermentable->getPurchaseEditor(); }
template<> StockPurchaseHopEditor         & StockWindow::getPurchaseEditor<Hop        >() const { return this->pimpl->m_stockManagerHop        ->getPurchaseEditor(); }
template<> StockPurchaseMiscEditor        & StockWindow::getPurchaseEditor<Misc       >() const { return this->pimpl->m_stockManagerMisc       ->getPurchaseEditor(); }
template<> StockPurchaseSaltEditor        & StockWindow::getPurchaseEditor<Salt       >() const { return this->pimpl->m_stockManagerSalt       ->getPurchaseEditor(); }
template<> StockPurchaseYeastEditor       & StockWindow::getPurchaseEditor<Yeast      >() const { return this->pimpl->m_stockManagerYeast      ->getPurchaseEditor(); }

template<> StockUseFermentableEditor & StockWindow::getUseEditor<Fermentable>() const { return this->pimpl->m_stockManagerFermentable->getUseEditor(); }
template<> StockUseHopEditor         & StockWindow::getUseEditor<Hop        >() const { return this->pimpl->m_stockManagerHop        ->getUseEditor(); }
template<> StockUseMiscEditor        & StockWindow::getUseEditor<Misc       >() const { return this->pimpl->m_stockManagerMisc       ->getUseEditor(); }
template<> StockUseSaltEditor        & StockWindow::getUseEditor<Salt       >() const { return this->pimpl->m_stockManagerSalt       ->getUseEditor(); }
template<> StockUseYeastEditor       & StockWindow::getUseEditor<Yeast      >() const { return this->pimpl->m_stockManagerYeast      ->getUseEditor(); }

template<class Ingrd> void StockWindow::showStockPurchasesFor(Ingrd const * ingredient) {
   this->pimpl->selectTab<Ingrd>();
   this->pimpl->getStockManager<Ingrd>().setSearchFilter(ingredient ? ingredient->name() : "");
   this->show();
   this->activateWindow();
   this->raise();
   return;
}
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// allows us to access private details in the implementation.)
template void StockWindow::showStockPurchasesFor(Fermentable const * ingredient);
template void StockWindow::showStockPurchasesFor(Hop         const * ingredient);
template void StockWindow::showStockPurchasesFor(Misc        const * ingredient);
template void StockWindow::showStockPurchasesFor(Salt        const * ingredient);
template void StockWindow::showStockPurchasesFor(Yeast       const * ingredient);


void StockWindow::saveUiState() const {
   static auto const & section = PersistentSettings::Sections::StockWindow;
   this->pimpl->m_stockManagerFermentable->saveUiState(PersistentSettings::Names::uiState_stockManagerFermentable, section);
   this->pimpl->m_stockManagerHop        ->saveUiState(PersistentSettings::Names::uiState_stockManagerHop        , section);
   this->pimpl->m_stockManagerMisc       ->saveUiState(PersistentSettings::Names::uiState_stockManagerMisc       , section);
   this->pimpl->m_stockManagerSalt       ->saveUiState(PersistentSettings::Names::uiState_stockManagerSalt       , section);
   this->pimpl->m_stockManagerYeast      ->saveUiState(PersistentSettings::Names::uiState_stockManagerYeast      , section);
   return;
}

bool StockWindow::restoreUiState() {
   static auto const & section = PersistentSettings::Sections::StockWindow;
   bool restored =
      this->pimpl->m_stockManagerFermentable->restoreUiState(PersistentSettings::Names::uiState_stockManagerFermentable, section) |
      this->pimpl->m_stockManagerHop        ->restoreUiState(PersistentSettings::Names::uiState_stockManagerHop        , section) |
      this->pimpl->m_stockManagerMisc       ->restoreUiState(PersistentSettings::Names::uiState_stockManagerMisc       , section) |
      this->pimpl->m_stockManagerSalt       ->restoreUiState(PersistentSettings::Names::uiState_stockManagerSalt       , section) |
      this->pimpl->m_stockManagerYeast      ->restoreUiState(PersistentSettings::Names::uiState_stockManagerYeast      , section);
   return restored;
}
