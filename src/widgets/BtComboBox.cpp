/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/BtComboBox.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "widgets/BtComboBox.h"

#include "widgets/SmartLineEdit.h"

// This private implementation class holds all private non-virtual members of BtComboBox
class BtComboBox::impl {
public:
   impl(BtComboBox & self) :
      m_self              {self },
      m_initialised       {false},
      m_editorName        {"Uninitialised m_editorName!"    },
      m_comboBoxName      {"Uninitialised m_comboBoxName!"  },
      m_comboBoxFqName    {"Uninitialised m_comboBoxFqName!"},
      m_nameMapping       {nullptr},
      m_displayNameMapping{nullptr},
      m_typeInfo          {nullptr},
      m_controlledField   {nullptr} {
      return;
   }

   ~impl() = default;

   BtComboBox              & m_self              ;
   bool                      m_initialised       ;
   char const *              m_editorName        ;
   char const *              m_comboBoxName      ;
   char const *              m_comboBoxFqName    ;
   EnumStringMapping const * m_nameMapping       ;
   EnumStringMapping const * m_displayNameMapping;
   TypeInfo          const * m_typeInfo          ;
   SmartLineEdit *           m_controlledField   ;

};

BtComboBox::BtComboBox(QWidget * parent) :
   QComboBox{parent},
   pimpl {std::make_unique<impl>(*this)} {
   // QOverload is needed on next line because the signal currentIndexChanged is overloaded in QComboBox - see
   // https://doc.qt.io/qt-5/qcombobox.html#currentIndexChanged
   connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BtComboBox::onIndexChanged);
   return;
}

BtComboBox::~BtComboBox() = default;

void BtComboBox::init(char const * const        editorName        ,
                      char const * const        comboBoxName      ,
                      char const * const        comboBoxFqName    ,
                      EnumStringMapping const & nameMapping       ,
                      EnumStringMapping const & displayNameMapping,
                      TypeInfo          const & typeInfo          ,
                      std::vector<int>  const * restrictTo        ,
                      SmartLineEdit *           controlledField) {
   qDebug() << Q_FUNC_INFO << comboBoxFqName << ":" << typeInfo;

   // It's a coding error to call init twice
   Q_ASSERT(!this->pimpl->m_initialised);

   // It's a coding error if the type we're displaying is not an enum -- unless it's the special case of a
   // Measurement::ChoiceOfPhysicalQuantity-restricted Measurement::Amount value
   Q_ASSERT(typeInfo.isEnum() ||
            (typeInfo.fieldType && holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType)));

   // If we are dealing with Measurement::ChoiceOfPhysicalQuantity, then there usually needs to be a controlled field
   // UNLESS we are in a table model.
//   Q_ASSERT(typeInfo.isEnum() == (controlledField == nullptr));

   this->pimpl->m_editorName         =  editorName        ;
   this->pimpl->m_comboBoxName       =  comboBoxName      ;
   this->pimpl->m_comboBoxFqName     =  comboBoxFqName    ;
   this->pimpl->m_nameMapping        = &nameMapping       ;
   this->pimpl->m_displayNameMapping = &displayNameMapping;
   this->pimpl->m_typeInfo           = &typeInfo          ;
   this->pimpl->m_controlledField    =  controlledField   ;

   // If this is an optional enum, then we need a blank value
   if (typeInfo.isOptional()) {
      this->addItem("", "");
   }

   // It's a coding error if the two enum mappings do not have the same number of entries
   auto const numEnumVals = this->pimpl->m_nameMapping->size();
   Q_ASSERT(this->pimpl->m_displayNameMapping->size() == numEnumVals);
   for (auto ii = 0; ii < numEnumVals; ++ii) {
      if (!restrictTo ||
          std::find(restrictTo->cbegin(), restrictTo->cend(), ii) != restrictTo->cend()) {
         this->addItem(*this->pimpl->m_displayNameMapping->enumAsIntToString(ii),
                       *this->pimpl->m_nameMapping       ->enumAsIntToString(ii));
      }
   }

   this->pimpl->m_initialised = true;

   // In the special case where we're handling Measurement::ChoiceOfPhysicalQuantity, we need to pick up the right
   // initial value
   if (this->pimpl->m_controlledField) {
      this->autoSetFromControlledField();
   }

   //
   // By default, a QComboBox "will adjust to its contents the first time it is shown", which means that, on some
   // platforms at least, if it somehow gets shown before it is populated, then it will be far too narrow.
   //
   this->QComboBox::setSizeAdjustPolicy(QComboBox::AdjustToContents);

   return;
}

void BtComboBox::autoSetFromControlledField() {
   // Normally keep this log statement commented out otherwise it generates too many lines in the log file
//   qDebug().noquote() <<
//      Q_FUNC_INFO << this->pimpl->m_comboBoxFqName << ":" << this->pimpl->m_typeInfo->fieldType <<
//      Logging::getStackTrace();

   // It's a coding error to call this when there is no controlled field
   Q_ASSERT(this->pimpl->m_controlledField);

   // Equally it's a coding error if we have a controlled field for anything other than choice of physical quantity
   Q_ASSERT(holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*this->pimpl->m_typeInfo->fieldType));

   Measurement::PhysicalQuantity const physicalQuantity = this->pimpl->m_controlledField->getPhysicalQuantity();

   // Uncomment the next statement for diagnosing asserts!
//   qDebug() <<
//      Q_FUNC_INFO << "TypeInfo:" << this->pimpl->m_typeInfo->fieldType << ", physicalQuantity: " << physicalQuantity;

   this->setValue(static_cast<int>(physicalQuantity));
   return;
}


[[nodiscard]] bool BtComboBox::isOptional() const {
   Q_ASSERT(this->pimpl->m_initialised);
   return this->pimpl->m_typeInfo->isOptional();
}

void BtComboBox::setNull() {
   this->setCurrentIndex(0);
   // For an optional field, it's a coding error if the first value is not empty string
   Q_ASSERT(this->currentData().toString().isEmpty());
   return;
}

void BtComboBox::setValue(int value) {
   Q_ASSERT(this->pimpl->m_initialised);
   this->setCurrentIndex(this->findData(this->pimpl->m_nameMapping->enumToString(value)));
   // It's a coding error if we have an empty string here
   Q_ASSERT(!this->currentData().toString().isEmpty());
   return;
}


void BtComboBox::setDefault() {
   this->setCurrentIndex(0);
   return;
}

void BtComboBox::setFromVariant(QVariant const & value) {
   Q_ASSERT(this->pimpl->m_initialised);
   // We assume the QVariant holds an int or an optional int, as we do for serialisation etc, as otherwise it gets hard
   // to handle strongly-typed enums generically at runtime.
   if (this->pimpl->m_typeInfo->isOptional()) {
      auto vv {value.value<std::optional<int>>()};
      if (vv) {
         this->setValue(*vv);
      } else {
         this->setNull();
      }
   } else {
      this->setValue(value.value<int>());
   }
   return;
}

QVariant BtComboBox::getAsVariant() const {
   Q_ASSERT(this->pimpl->m_initialised);
   if (this->pimpl->m_typeInfo->isOptional()) {
      return QVariant::fromValue(this->getOptIntValue());
   }
   return QVariant::fromValue(this->getNonOptIntValue());
}

[[nodiscard]] std::optional<int> BtComboBox::getOptIntValue() const {
   Q_ASSERT(this->pimpl->m_initialised);
   QString const rawValue = this->currentData().toString();
   if (rawValue.isEmpty()) {
      Q_ASSERT(this->isOptional());
      return std::nullopt;
   }

   // It's a coding error if we don't recognise the values in our own combo boxes
   auto value = this->pimpl->m_nameMapping->stringToEnumAsInt(rawValue);
   Q_ASSERT(value);
   return value;
}

[[nodiscard]] int BtComboBox::getNonOptIntValue() const {
   Q_ASSERT(this->pimpl->m_initialised);
   QString const rawValue = this->currentData().toString();
   Q_ASSERT(!rawValue.isEmpty());

   // It's a coding error if we don't recognise the values in our own combo boxes
   auto value = this->pimpl->m_nameMapping->stringToEnumAsInt(rawValue);
   Q_ASSERT(value);
   return *value;
}

void BtComboBox::onIndexChanged([[maybe_unused]] int const index) {
   if (this->pimpl->m_initialised && this->pimpl->m_controlledField) {
      // Uncomment the next statement for diagnosing asserts!
//      qDebug() <<
//         Q_FUNC_INFO << "index:" << index << ", raw value:" << this->currentData() << ", decodes to: " <<
//         this->getOptIntValue();
      std::optional<int> const rawPhysicalQuantity = this->getOptIntValue();
      if (rawPhysicalQuantity) {
         Measurement::PhysicalQuantity const physicalQuantity =
            static_cast<Measurement::PhysicalQuantity>(*rawPhysicalQuantity);
         this->pimpl->m_controlledField->selectPhysicalQuantity(physicalQuantity);
      }
   }
   return;
}
