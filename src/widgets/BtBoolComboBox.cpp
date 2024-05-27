/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/BtBoolComboBox.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "widgets/BtBoolComboBox.h"

#include <QString>
#include <QVariant>

#include "utils/MetaTypes.h"

namespace {
   // We store string values inside the combo box as it's less confusing when the value is optional
   QString const falseValue = QStringLiteral("false");
   QString const  trueValue = QStringLiteral("true" );
}

// This private implementation class holds all private non-virtual members of BtBoolComboBox
class BtBoolComboBox::impl {
public:
   impl(BtBoolComboBox & self) :
      m_self          {self },
      m_initialised   {false},
      m_editorName    {"Uninitialised m_editorName!"    },
      m_comboBoxName  {"Uninitialised m_comboBoxName!"  },
      m_comboBoxFqName{"Uninitialised m_comboBoxFqName!"},
      m_unsetDisplay  {nullptr},
      m_setDisplay    {nullptr},
      m_typeInfo      {nullptr} {
      return;
   }

   ~impl() = default;

   BtBoolComboBox & m_self          ;
   bool             m_initialised   ;
   char     const * m_editorName    ;
   char     const * m_comboBoxName  ;
   char     const * m_comboBoxFqName;
   QString  const * m_unsetDisplay  ;
   QString  const * m_setDisplay    ;
   TypeInfo const * m_typeInfo      ;
};

BtBoolComboBox::BtBoolComboBox(QWidget * parent) :
   QComboBox{parent},
   pimpl {std::make_unique<impl>(*this)} {
   return;
}

BtBoolComboBox::~BtBoolComboBox() = default;

void BtBoolComboBox::init(char const * const   editorName    ,
                          char const * const   comboBoxName  ,
                          char const * const   comboBoxFqName,
                          QString      const & unsetDisplay  ,
                          QString      const & setDisplay    ,
                          TypeInfo     const & typeInfo      ) {
   qDebug() << Q_FUNC_INFO << comboBoxFqName << ":" << typeInfo;

   // It's a coding error to call init twice
   Q_ASSERT(!this->pimpl->m_initialised);

   // It's a coding error if the type we're displaying is not a bool
   Q_ASSERT(typeInfo.fieldType);
   Q_ASSERT(std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType));
   Q_ASSERT(NonPhysicalQuantity::Bool == std::get<NonPhysicalQuantity>(*typeInfo.fieldType));
   Q_ASSERT(typeInfo.typeIndex == typeid(bool));

   this->pimpl->m_editorName     =  editorName    ;
   this->pimpl->m_comboBoxName   =  comboBoxName  ;
   this->pimpl->m_comboBoxFqName =  comboBoxFqName;
   this->pimpl->m_unsetDisplay   = &unsetDisplay  ;
   this->pimpl->m_setDisplay     = &setDisplay    ;
   this->pimpl->m_typeInfo       = &typeInfo      ;

   // If this is an optional enum, then we need a blank value
   if (typeInfo.isOptional()) {
      this->addItem("", "");
   }

   this->addItem(*this->pimpl->m_unsetDisplay, falseValue);
   this->addItem(*this->pimpl->m_setDisplay  ,  trueValue);

   this->pimpl->m_initialised = true;
   return;
}

[[nodiscard]] bool BtBoolComboBox::isOptional() const {
   Q_ASSERT(this->pimpl->m_initialised);
   return this->pimpl->m_typeInfo->isOptional();
}

void BtBoolComboBox::setValue(bool const value) {
   Q_ASSERT(!this->isOptional());

   // Standard conversion of bool is false -> 0, true -> 1
   this->setCurrentIndex(static_cast<int>(value));
   return;
}

void BtBoolComboBox::setValue(std::optional<bool> const value) {
   Q_ASSERT(this->isOptional());

   if (!value) {
      this->setCurrentIndex(0);
   } else {
      this->setCurrentIndex(static_cast<int>(*value) + 1);
   }

   return;
}

void BtBoolComboBox::setNull() {
   Q_ASSERT(this->isOptional());
   this->setCurrentIndex(0);
   return;
}

[[nodiscard]] bool BtBoolComboBox::getNonOptBoolValue() const {
   Q_ASSERT(!this->isOptional());
   QString const rawValue = this->currentData().toString();
   Q_ASSERT(rawValue == falseValue || rawValue == trueValue);
   return rawValue == trueValue;
}

[[nodiscard]] std::optional<bool> BtBoolComboBox::getOptBoolValue() const {
   Q_ASSERT(this->isOptional());
   QString const rawValue = this->currentData().toString();
   if (rawValue.isEmpty()) {
      Q_ASSERT(this->isOptional());
      return std::nullopt;
   }
   Q_ASSERT(rawValue == falseValue || rawValue == trueValue);
   return rawValue == trueValue;
}

[[nodiscard]] QVariant BtBoolComboBox::getValue(TypeInfo const & typeInfo) const {
   if (typeInfo.isOptional()) {
      return QVariant::fromValue(this->getOptBoolValue());
   }
   return QVariant::fromValue(this->getNonOptBoolValue());
}
