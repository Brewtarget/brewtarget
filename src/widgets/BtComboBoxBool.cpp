/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/BtComboBoxBool.cpp is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#include "widgets/BtComboBoxBool.h"

#include <QString>
#include <QVariant>

#include "utils/MetaTypes.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BtComboBoxBool.cpp"
#endif

namespace {
   // We store string values inside the combo box as it's less confusing when the value is optional
   QString const falseValue = QStringLiteral("false");
   QString const  trueValue = QStringLiteral("true" );
}

// This private implementation class holds all private non-virtual members of BtComboBoxBool
class BtComboBoxBool::impl {
public:
   impl(BtComboBoxBool & self) :
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

   BtComboBoxBool & m_self          ;
   bool             m_initialised   ;
   char     const * m_editorName    ;
   char     const * m_comboBoxName  ;
   char     const * m_comboBoxFqName;
   QString  const * m_unsetDisplay  ;
   QString  const * m_setDisplay    ;
   TypeInfo const * m_typeInfo      ;
};

BtComboBoxBool::BtComboBoxBool(QWidget * parent) :
   QComboBox{parent},
   pimpl{std::make_unique<impl>(*this)} {
   return;
}

BtComboBoxBool::~BtComboBoxBool() = default;

void BtComboBoxBool::init(char const * const   editorName    ,
                          char const * const   comboBoxName  ,
                          char const * const   comboBoxFqName,
                          QString      const & unsetDisplay  ,
                          QString      const & setDisplay    ,
                          TypeInfo     const & typeInfo      ) {
   // Normally keep this log statement commented out otherwise it generates too many lines in the log file
//   qDebug() << Q_FUNC_INFO << comboBoxFqName << ":" << typeInfo;

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

   // It's a coding error if we already have any items in the combo box.  (This could eg happen if any were defined in
   // the .ui file.)
   Q_ASSERT(0 == this->count());

   // If this is an optional enum, then we need a blank value
   if (typeInfo.isOptional()) {
      this->addItem("", "");
   }

   this->addItem(*this->pimpl->m_unsetDisplay, QVariant::fromValue<QString>(falseValue));
   this->addItem(*this->pimpl->m_setDisplay  , QVariant::fromValue<QString>( trueValue));

   this->pimpl->m_initialised = true;

   //
   // By default, a QComboBox "will adjust to its contents the first time it is shown", which means that, on some
   // platforms at least, if it somehow gets shown before it is populated, then it will be far too narrow.
   //
   this->QComboBox::setSizeAdjustPolicy(QComboBox::AdjustToContents);
   return;
}

[[nodiscard]] bool BtComboBoxBool::isOptional() const {
   Q_ASSERT(this->pimpl->m_initialised);
   return this->pimpl->m_typeInfo->isOptional();
}

void BtComboBoxBool::setValue(bool const value) {
   Q_ASSERT(!this->isOptional());

   // We could just short-cut things here and cast bool to int to get false -> 0, true -> 1, because this is the same
   // order we added falseValue and trueValue in init().   However, it's safer to ask Qt "What is the index of...".
   this->setCurrentIndex(this->findData(value ? trueValue : falseValue));
   return;
}

void BtComboBoxBool::setValue(std::optional<bool> const value) {
   Q_ASSERT(this->isOptional());

   // See comment above for why we use findData() here
   if (!value) {
      this->setCurrentIndex(this->findData(""));
   } else {
      this->setCurrentIndex(this->findData(*value ? trueValue : falseValue));
   }

   return;
}

void BtComboBoxBool::setNull() {
   Q_ASSERT(this->isOptional());
   this->setCurrentIndex(this->findData(""));
   return;
}

void BtComboBoxBool::setDefault() {
   this->setCurrentIndex(0);
   return;
}

void BtComboBoxBool::setFromVariant(QVariant const & value) {
   Q_ASSERT(this->pimpl->m_initialised);

   if (this->isOptional()) {
      this->setValue(value.value<std::optional<bool>>());
   } else {
      this->setValue(value.value<bool>());
   }
   return;
}

[[nodiscard]] bool BtComboBoxBool::getNonOptBoolValue() const {
   Q_ASSERT(!this->isOptional());
   QString const rawValue = this->currentData().toString();
   Q_ASSERT(rawValue == falseValue || rawValue == trueValue);
   return rawValue == trueValue;
}

[[nodiscard]] std::optional<bool> BtComboBoxBool::getOptBoolValue() const {
   Q_ASSERT(this->isOptional());
   QString const rawValue = this->currentData().toString();
   if (rawValue.isEmpty()) {
      return std::nullopt;
   }
   Q_ASSERT(rawValue == falseValue || rawValue == trueValue);
   return rawValue == trueValue;
}

[[nodiscard]] QVariant BtComboBoxBool::getAsVariant() const {
   if (this->isOptional()) {
      return QVariant::fromValue(this->getOptBoolValue());
   }
   return QVariant::fromValue(this->getNonOptBoolValue());
}
