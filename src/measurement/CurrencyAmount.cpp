/*======================================================================================================================
 * measurement/CurrencyAmount.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "measurement/CurrencyAmount.h"

#include <QDebug>
#include <QLocale>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "Localization.h"

CurrencyAmount::CurrencyAmount(QString const & inputString) {
   //
   // Sadly Qt does not offer us the inverse of QLocale::toCurrencyString, so we have to roll it ourselves
   //
   // Approach here is similar to that in Measurement::Unit::splitAmountString, and most of the comments there also
   // apply here.  The extra complication is that the currency symbol could be at the start (eg "€ 1,23") or the end (eg
   // "1,23 €").
   //
   static QRegularExpression const moneyRegexp {
      "([^\\s]+)?\\s*((?:\\d+" + QRegularExpression::escape(Localization::getLocale().groupSeparator()) + ")?\\d+(?:" +
      QRegularExpression::escape(Localization::getLocale().decimalPoint()) + "\\d+)?|" +
      QRegularExpression::escape(Localization::getLocale().decimalPoint()) + "\\d+)\\s*([^\\s]+)?",
      QRegularExpression::CaseInsensitiveOption
   };

   QRegularExpressionMatch match = moneyRegexp.match(inputString);
   if (!match.hasMatch()) {
      qDebug() << Q_FUNC_INFO << "Unable to parse" << inputString << "so treating as 0.00";
      this->currencySymbol = Localization::getLocale().currencySymbol(QLocale::CurrencySymbolFormat::CurrencySymbol);
      this->asCents = 0;
      return;
   }

   //
   // Currency symbol can be at beginning or end
   //
   this->currencySymbol = match.captured(1);
   if (this->currencySymbol.isEmpty()) {
      this->currencySymbol = match.captured(3);
   }

   QString numericPartOfInput{match.captured(2)};
   double const amount = Localization::toDouble(numericPartOfInput, Q_FUNC_INFO, nullptr);
   this->asCents = static_cast<int>(amount * 100.0);
   return;
}

CurrencyAmount::CurrencyAmount(double const amount, QString const symbol) :
   currencySymbol{symbol},
   asCents{static_cast<int>(amount * 100.0)} {
   return;
}

CurrencyAmount::CurrencyAmount() :
   currencySymbol{Localization::getLocale().currencySymbol(QLocale::CurrencySymbolFormat::CurrencySymbol)},
   asCents{0} {
   return;
}

CurrencyAmount::~CurrencyAmount() = default;

// Copy constructor
CurrencyAmount::CurrencyAmount(CurrencyAmount const & other) :
   currencySymbol{other.currencySymbol},
   asCents       {other.asCents       } {
   return;
}

// Copy assignment operator
CurrencyAmount & CurrencyAmount::operator=(CurrencyAmount const & other) {
   *this = CurrencyAmount{other};
   return *this;
}

// Move constructor
CurrencyAmount::CurrencyAmount(CurrencyAmount && other) noexcept :
   currencySymbol{other.currencySymbol},
   asCents       {other.asCents       } {
   return;
}

// Move assignment operator
CurrencyAmount & CurrencyAmount::operator=(CurrencyAmount && other) noexcept {
   std::swap(this->currencySymbol, other.currencySymbol);
   std::swap(this->asCents       , other.asCents       );
   return *this;
}

bool CurrencyAmount::operator==(CurrencyAmount const & other) const {
   return (this->currencySymbol == other.currencySymbol &&
           this->asCents        == other.asCents       );

}

bool CurrencyAmount::operator!=(CurrencyAmount const & other) const {
   // Don't reinvent the wheel '!=' should just be the opposite of '=='
   return !(*this == other);
}

std::partial_ordering CurrencyAmount::operator<=>(CurrencyAmount const & other) const {
   //
   // We're not realistically going to know how to compare amounts in different currencies.  For the moment,
   // we'll group different currencies together.
   //
   if (this->currencySymbol != other.currencySymbol) {
      return std::partial_ordering::unordered;
   }
   if (this->asCents == other.asCents) {
      return std::partial_ordering::equivalent;
   }
   return (this->asCents < other.asCents) ? std::partial_ordering::less :
                                            std::partial_ordering::greater;
}

double CurrencyAmount::asUnits() const {
   return this->asCents / 100.0;
}

QString CurrencyAmount::asDisplayable() const {
   //
   // QLocale::toCurrencyString does the heavy lifting for us, including working out how many decimal places to display
   //
   return Localization::getLocale().toCurrencyString(this->asUnits(), this->currencySymbol);
}

int CurrencyAmount::centsPart() const { return this->asCents % 100; }
int CurrencyAmount::unitsPart() const { return this->asCents / 100; }
