/*======================================================================================================================
 * widgets/StepsWidget.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "widgets/StepsWidget.h"

#include "PersistentSettings.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StepsWidget.cpp"
#endif

StepsWidget::StepsWidget(QWidget * parent) :
   QWidget{parent},
   m_horizontalLayout_main  {new QHBoxLayout{this}},
   m_tableView_steps        {new QTableView{this}},
   m_verticalLayout_buttons {new QVBoxLayout{this}},
   m_pushButton_addStep     {new QPushButton{this}},
   m_pushButton_removeStep  {new QPushButton{this}},
   m_pushButton_moveStepUp  {new QPushButton{this}},
   m_pushButton_moveStepDown{new QPushButton{this}},
   m_pushButton_editStep    {new QPushButton{this}},
   m_icon_addStep     {":/images/smallPlus.svg"     },
   m_icon_removeStep  {":/images/smallMinus.svg"    },
   m_icon_moveStepUp  {":/images/smallUpArrow.svg"  },
   m_icon_moveStepDown{":/images/smallDownArrow.svg"},
   m_icon_editStep    {":/images/edit.svg"          } {
   //
   // Do the generic bits of layout.  Subclasses (via StepsWidgetBase) need to do text that refers to Mash/Boil/etc.
   //
   m_horizontalLayout_main->addWidget(m_tableView_steps);
   m_horizontalLayout_main->addLayout(m_verticalLayout_buttons);
   m_verticalLayout_buttons->addWidget(m_pushButton_addStep     );
   m_verticalLayout_buttons->addWidget(m_pushButton_removeStep  );
   m_verticalLayout_buttons->addWidget(m_pushButton_moveStepUp  );
   m_verticalLayout_buttons->addWidget(m_pushButton_moveStepDown);
   m_verticalLayout_buttons->addWidget(m_pushButton_editStep    );
   m_pushButton_addStep     ->setIcon(m_icon_addStep     );
   m_pushButton_removeStep  ->setIcon(m_icon_removeStep  );
   m_pushButton_moveStepUp  ->setIcon(m_icon_moveStepUp  );
   m_pushButton_moveStepDown->setIcon(m_icon_moveStepDown);
   m_pushButton_editStep    ->setIcon(m_icon_editStep    );
   return;
}

StepsWidget::~StepsWidget() = default;

void StepsWidget::saveUiState(BtStringConst const & property, BtStringConst const & section) const {
   PersistentSettings::insert(property, this->m_tableView_steps->horizontalHeader()->saveState(), section);
   return;
}

void StepsWidget::restoreUiState(BtStringConst const & property, BtStringConst const & section) {
   if (PersistentSettings::contains(property, section)) {
      this->m_tableView_steps->horizontalHeader()->restoreState(
         PersistentSettings::value(property, QVariant(), section).toByteArray()
      );
   }
   return;
}

//»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
STEPS_WIDGET_COMMON_CODE(Mash)
STEPS_WIDGET_COMMON_CODE(Boil)
STEPS_WIDGET_COMMON_CODE(Fermentation)
