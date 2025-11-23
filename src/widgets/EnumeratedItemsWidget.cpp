/*======================================================================================================================
 * widgets/EnumeratedItemsWidget.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "widgets/EnumeratedItemsWidget.h"

#include "PersistentSettings.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_EnumeratedItemsWidget.cpp"
#endif

EnumeratedItemsWidget::EnumeratedItemsWidget(QWidget * parent) :
   QWidget{parent},
   m_horizontalLayout_main  {std::make_unique<QHBoxLayout>()},
   m_tableView_items        {std::make_unique<QTableView >()},
   m_verticalLayout_buttons {std::make_unique<QVBoxLayout>()},
   m_pushButton_addItem     {std::make_unique<QPushButton>()},
   m_pushButton_removeItem  {std::make_unique<QPushButton>()},
   m_pushButton_moveItemUp  {std::make_unique<QPushButton>()},
   m_pushButton_moveItemDown{std::make_unique<QPushButton>()},
   m_pushButton_editItem    {std::make_unique<QPushButton>()},
   m_icon_addItem     {":/images/smallPlus.svg"     },
   m_icon_removeItem  {":/images/smallMinus.svg"    },
   m_icon_moveItemUp  {":/images/smallUpArrow.svg"  },
   m_icon_moveItemDown{":/images/smallDownArrow.svg"},
   m_icon_editItem    {":/images/edit.svg"          } {
   //
   // Do the generic bits of layout.  Subclasses (via ItemsWidgetBase) need to do text that refers to Mash/Boil/etc.
   //
   // Setting the object names helps with debugging if we get warnings or errors logged from Qt base classes
   //
   m_horizontalLayout_main  ->setObjectName("m_horizontalLayout_main"  );
   m_tableView_items        ->setObjectName("m_tableView_items"        );
   m_verticalLayout_buttons ->setObjectName("m_verticalLayout_buttons" );
   m_pushButton_addItem     ->setObjectName("m_pushButton_addItem"     );
   m_pushButton_removeItem  ->setObjectName("m_pushButton_removeItem"  );
   m_pushButton_moveItemUp  ->setObjectName("m_pushButton_moveItemUp"  );
   m_pushButton_moveItemDown->setObjectName("m_pushButton_moveItemDown");
   m_pushButton_editItem    ->setObjectName("m_pushButton_editItem"    );
   this->setLayout(m_horizontalLayout_main.get());
   m_horizontalLayout_main->addWidget(m_tableView_items.get());
   m_horizontalLayout_main->addLayout(m_verticalLayout_buttons.get());
   m_verticalLayout_buttons->addWidget(m_pushButton_addItem     .get());
   m_verticalLayout_buttons->addWidget(m_pushButton_removeItem  .get());
   m_verticalLayout_buttons->addWidget(m_pushButton_moveItemUp  .get());
   m_verticalLayout_buttons->addWidget(m_pushButton_moveItemDown.get());
   m_verticalLayout_buttons->addWidget(m_pushButton_editItem    .get());
   m_pushButton_addItem     ->setIcon(m_icon_addItem     );
   m_pushButton_removeItem  ->setIcon(m_icon_removeItem  );
   m_pushButton_moveItemUp  ->setIcon(m_icon_moveItemUp  );
   m_pushButton_moveItemDown->setIcon(m_icon_moveItemDown);
   m_pushButton_editItem    ->setIcon(m_icon_editItem    );
   return;
}

EnumeratedItemsWidget::~EnumeratedItemsWidget() = default;

void EnumeratedItemsWidget::saveUiState(BtStringConst const & property, BtStringConst const & section) const {
   PersistentSettings::saveUiState(property, *this->m_tableView_items->horizontalHeader(), section);
   return;
}

bool EnumeratedItemsWidget::restoreUiState(BtStringConst const & property, BtStringConst const & section) {
   return PersistentSettings::restoreUiState(property, *this->m_tableView_items->horizontalHeader(), section);
}

//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
#include "editors/StockUseIngredientEditor.h"

#include "model/StockPurchaseFermentable.h"

ENUMERATED_ITEMS_WIDGET_COMMON_CODE(MashStep)
ENUMERATED_ITEMS_WIDGET_COMMON_CODE(BoilStep)
ENUMERATED_ITEMS_WIDGET_COMMON_CODE(FermentationStep)
ENUMERATED_ITEMS_WIDGET_COMMON_CODE(StockUseFermentable)
ENUMERATED_ITEMS_WIDGET_COMMON_CODE(StockUseHop)
ENUMERATED_ITEMS_WIDGET_COMMON_CODE(StockUseMisc)
ENUMERATED_ITEMS_WIDGET_COMMON_CODE(StockUseSalt)
ENUMERATED_ITEMS_WIDGET_COMMON_CODE(StockUseYeast)
