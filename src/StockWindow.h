/*======================================================================================================================
 * StockWindow.h is part of Brewtarget, and is copyright the following authors 2025-2026:
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
#ifndef STOCKWINDOW_H
#define STOCKWINDOW_H
#pragma once

#include <memory> // For PImpl

#include <QDialog>

/**
 * \brief A window showing stock of Fermentables, Hops, Miscs, Salts, Yeasts
 *
 */
class StockWindow : public QDialog {
   Q_OBJECT

public:
   //! \brief Default constructor.
   StockWindow(QWidget * parent = nullptr);
   ~StockWindow();

   void retranslateUi();

   /**
    * \brief Get the \c Ingredient class's stock purchase editor instance, eg \c StockPurchaseHopEditor for \c Hop,
    *        \c StockPurchaseMiscEditor for \c Misc, etc.
    */
   template<class Ingrd> typename Ingrd::StockPurchaseClass::EditorClass & getPurchaseEditor() const;

   /**
    * \brief Get the \c Ingredient class's stock use editor instance, eg \c StockUseHopEditor for \c Hop,
    *        \c StockUseMiscEditor for \c Misc, etc.
    */
   template<class Ingrd> typename Ingrd::StockPurchaseClass::StockUseClass::EditorClass & getUseEditor() const;

   /**
    * \brief In other parts of the UI, we show Total Inventory for each ingredient.  It's natural for the user to want
    *        to see the detail behind this, so this function helps with that.  The \c ingredient parameter is used to
    *        preselect the appropriate tab and prepopulate the search filter.  (This isn't perfect as you might have
    *        one ingredient whose name is a substring of another one.  But it should be good enough for most people most
    *        of the time.)
    *
    * \param ingredient If null, show stock purchases for \b all ingredients of this type
    */
   template<class Ingrd> void showStockPurchasesFor(Ingrd const * ingredient);

   void saveUiState() const;
   bool restoreUiState();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
