/*======================================================================================================================
 * widgets/BtLineEditCurrency.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "widgets/BtLineEditCurrency.h"

// This private implementation class holds all private non-virtual members of BtLineEditCurrency
class BtLineEditCurrency::impl {
public:
   impl(BtLineEditCurrency & self) :
      m_self          {self } {
      return;
   }

   ~impl() = default;

   BtLineEditCurrency & m_self;
   bool                 m_initialised    = false;
   char const *         m_editorName     = nullptr;
   char const *         m_lineEditName   = nullptr;
   char const *         m_lineEditFqName = nullptr;
   TypeInfo const *     m_typeInfo       = nullptr;
};


BtLineEditCurrency::BtLineEditCurrency(QWidget* parent) :
   QLineEdit{parent},
   pimpl{std::make_unique<impl>(*this)} {
   connect(this, &QLineEdit::editingFinished, this, &BtLineEditCurrency::onLineChanged);
   return;
}


BtLineEditCurrency::~BtLineEditCurrency() = default;

void BtLineEditCurrency::init(char const * const   editorName,
                              char const * const   lineEditName,
                              char const * const   lineEditFqName,
                              TypeInfo     const & typeInfo) {
   // Normally keep this log statement commented out otherwise it generates too many lines in the log file
   qDebug() << Q_FUNC_INFO << lineEditFqName << ":" << typeInfo;

   // It's a coding error to call init twice
   Q_ASSERT(!this->pimpl->m_initialised);

   // It's a coding error if the type we're displaying is not a CurrencyAmount
   Q_ASSERT(typeInfo.fieldType);
   Q_ASSERT(std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType));
   Q_ASSERT(NonPhysicalQuantity::Currency == std::get<NonPhysicalQuantity>(*typeInfo.fieldType));
   Q_ASSERT(typeInfo.typeIndex == typeid(CurrencyAmount));

   this->pimpl->m_editorName     =  editorName    ;
   this->pimpl->m_lineEditName   =  lineEditName  ;
   this->pimpl->m_lineEditFqName =  lineEditFqName;
   this->pimpl->m_typeInfo       = &typeInfo      ;

   this->pimpl->m_initialised = true;
   return;
}

[[nodiscard]] bool BtLineEditCurrency::isOptional() const {
   Q_ASSERT(this->pimpl->m_initialised);
   return this->pimpl->m_typeInfo->isOptional();
}

void BtLineEditCurrency::setValue(CurrencyAmount const & value) {
   Q_ASSERT(!this->isOptional());

   this->QLineEdit::setText(value.asDisplayable());

   return;
}

void BtLineEditCurrency::setValue(std::optional<CurrencyAmount> const & value) {
   Q_ASSERT(this->isOptional());
   this->QLineEdit::setText(value ? value->asDisplayable() : QString{});
   return;
}

void BtLineEditCurrency::setNull() {
   Q_ASSERT(this->isOptional());
   this->QLineEdit::setText("");
   return;
}

void BtLineEditCurrency::setDefault() {
   this->QLineEdit::setText(this->isOptional() ? CurrencyAmount{"", 0}.asDisplayable() : "");
   return;
}

void BtLineEditCurrency::setFromVariant(QVariant const & value) {
   Q_ASSERT(this->pimpl->m_initialised);

   if (this->isOptional()) {
      this->setValue(value.value<std::optional<CurrencyAmount>>());
   } else {
      this->setValue(value.value<CurrencyAmount>());
   }
   return;
}

[[nodiscard]] CurrencyAmount BtLineEditCurrency::getNonOptValue() const {
   Q_ASSERT(!this->isOptional());
   return CurrencyAmount{this->QLineEdit::text().trimmed()};
}

[[nodiscard]] std::optional<CurrencyAmount> BtLineEditCurrency::getOptValue() const {
   Q_ASSERT(this->isOptional());
   QString const rawValue = this->QLineEdit::text().trimmed();
   if (rawValue.isEmpty()) {
      return std::nullopt;
   }
   return CurrencyAmount{rawValue};
}

[[nodiscard]] QVariant BtLineEditCurrency::getAsVariant() const {
   if (this->isOptional()) {
      return QVariant::fromValue(this->getOptValue());
   }
   return QVariant::fromValue(this->getNonOptValue());
}

void BtLineEditCurrency::onLineChanged() {
//   qDebug() << Q_FUNC_INFO << this->QLineEdit::text();
   Q_ASSERT(this->pimpl->m_initialised);

   //
   // I don't think this should happen, but it doesn't hurt to bail out if it does
   //
   if (sender() != this) {
      return;
   }

   // Per comment in SmartLineEdit::lineChanged, there's nothing to do if nothing changed
   if (!this->isModified()) {
      return;
   }

   //
   // This should correct the display -- eg from "4" to "$4.00" or "4,00 €" etc.
   //
   this->setFromVariant(this->getAsVariant());

   emit textModified();

   return;
}
