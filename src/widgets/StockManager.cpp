/*======================================================================================================================
 * widgets/StockManager.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "widgets/StockManager.h"


StockManager::StockManager(QWidget * parent) :
   QWidget{parent},
   // NB layouts other than the top one shouldn't have a parent
   m_vLayout_invMgr  {new QVBoxLayout(this)},
   m_hLayout_controls{new QHBoxLayout()    },
   m_searchIcon      {new QToolButton(this)},
   m_searchBox_filter{new QLineEdit  (this)},
   m_button_new      {new QPushButton(this)},
   m_button_edit     {new QPushButton(this)},
   m_button_delete   {new QPushButton(this)} {

   this->m_searchIcon->setStyleSheet("QToolButton {border-style: outset; border-width: 0px;}");
   this->m_searchIcon->setIcon(QIcon{QStringLiteral(":/images/iconSearch.svg")});

   this->m_button_new   ->setIcon(QIcon{QStringLiteral(":/images/smallPlus.svg" )});
   this->m_button_edit  ->setIcon(QIcon{QStringLiteral(":/images/edit.svg"      )});
   this->m_button_delete->setIcon(QIcon{QStringLiteral(":/images/smallMinus.svg")});

   this->m_button_new   ->setAutoDefault(false);
   this->m_button_edit  ->setAutoDefault(false);
   this->m_button_delete->setAutoDefault(false);

   this->m_searchBox_filter->setMaxLength(30);

   this->m_hLayout_controls->addWidget(this->m_searchIcon      );
   this->m_hLayout_controls->addWidget(this->m_searchBox_filter);
   this->m_hLayout_controls->addWidget(this->m_button_new      );
   this->m_hLayout_controls->addWidget(this->m_button_edit     );
   this->m_hLayout_controls->addWidget(this->m_button_delete   );

   //
   // Remaining layout work for derived classes etc is done in StockManagerBase
   //

   this->retranslateUi();

   return;
}

StockManager::~StockManager() = default;

void StockManager::retranslateUi() {
   this->m_searchBox_filter->setPlaceholderText(QObject::tr("Enter filter"));
   return;
}
