/*======================================================================================================================
 * widgets/StockManager.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef WIDGETS_STOCKMANAGER_H
#define WIDGETS_STOCKMANAGER_H
#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

/**
 * \brief Used in each of the tabs of \c StockWindow
 */
class StockManager : public QWidget {
public:
   StockManager(QWidget * parent);
   ~StockManager();

   void retranslateUi();

protected:
   //! \name UI Variables
   //! @{

   //! Top level widget needs a layout, otherwise its contents will not expand to fill the available space
   QVBoxLayout * m_vLayout_invMgr;

   //! For the controls at the bottom
   QHBoxLayout * m_hLayout_controls;

   //! See comments in \c catalogs/CatalogBase.h for why we use \c QToolButton rather than \c QLabel here
   QToolButton * m_searchIcon;
   QLineEdit   * m_searchBox_filter;
   QPushButton * m_button_new      ;
   QPushButton * m_button_edit     ;
   QPushButton * m_button_delete   ;

   //! @}
};

#endif
