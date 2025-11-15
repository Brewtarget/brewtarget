/*======================================================================================================================
 * widgets/BtComboBoxObject.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#include "widgets/BtComboBoxObject.h"

#include "Logging.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BtComboBoxObject.cpp"
#endif

BtComboBoxObject::BtComboBoxObject(char const * const name, QWidget* parent) :
   QComboBox{parent},
   m_name{name} {
   return;
}

BtComboBoxObject::~BtComboBoxObject() = default;

/**
 * \brief Set the ID of the selected \c NamedEntity
 *
 * \param value -1 means nothing selected
 */
void BtComboBoxObject::setCurrentId(int value) {
   //
   // See comment in widgets/BtComboBoxObject.h for why the "nothing selected" option cannot appear explicitly
   //
   int const index {value < 0 ? -1 : this->findData(value)};
   qDebug() << Q_FUNC_INFO << this->m_name << "value:" << value << ", index:" << index;

   // It's probably a coding error to set an ID we don't know about, but it could also be bad data.  In either case, we
   // can recover.
   if (value > 0 && index < 0) {
      qWarning() << Q_FUNC_INFO << "Unable to find value" << value << "for BtComboBoxObject" << this->m_name;
      qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      return;
   }

   this->setCurrentIndex(index);
   return;
}

void BtComboBoxObject::setDefault() {
   this->setCurrentId(-1);
   return;
}

int BtComboBoxObject::getCurrentId() const {
   QVariant id = this->currentData();
   return id.toInt();
}
