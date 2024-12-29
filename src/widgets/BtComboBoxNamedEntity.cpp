/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/BtComboBoxNamedEntity.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "widgets/BtComboBoxNamedEntity.h"

#include <QListView>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStylePainter>

BtComboBoxNamedEntity::BtComboBoxNamedEntity(QWidget* parent) : QComboBox(parent) {
   return;
}

BtComboBoxNamedEntity::~BtComboBoxNamedEntity() = default;

/**
 * \brief Set the ID of the selected \c NamedEntity
 *
 * \param value -1 means nothing selected
 */
void BtComboBoxNamedEntity::setCurrentId(int value) {
   int const index {this->findData(value)};
   // It's a coding error to set an ID we don't know about
   Q_ASSERT(index >= 0);
   this->setCurrentIndex(index);
   return;
}

int BtComboBoxNamedEntity::getCurrentId() const {
   QVariant id = this->currentData();
   return id.toInt();
}

BT_COMBO_BOX_NAMED_ENTITY_CODE(Boil        )
BT_COMBO_BOX_NAMED_ENTITY_CODE(Equipment   )
BT_COMBO_BOX_NAMED_ENTITY_CODE(Mash        )
BT_COMBO_BOX_NAMED_ENTITY_CODE(Fermentation)
BT_COMBO_BOX_NAMED_ENTITY_CODE(Style       )
BT_COMBO_BOX_NAMED_ENTITY_CODE(Water       )
