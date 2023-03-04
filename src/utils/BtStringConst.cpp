/*
 * utils/BtStringConst.cpp is part of Brewtarget, and is Copyright the following
 * authors 2021-2023
 * - Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "utils/BtStringConst.h"

#include <cstring>

#include <QDebug>
#include <QString>
#include <QTextStream>

BtStringConst const BtString::NULL_STR{static_cast<char const *>(nullptr)};
BtStringConst const BtString::EMPTY_STR{""};

BtStringConst::BtStringConst(char const * const cString) : cString(cString) {
   return;
}

BtStringConst::BtStringConst(BtStringConst const &) = default;
BtStringConst::BtStringConst(BtStringConst &&) = default;

BtStringConst::~BtStringConst() = default;

bool BtStringConst::operator==(BtStringConst const & rhs) const {
   // A very common case of equality should be that two strings are in fact the same constant, so checking whether the
   // addresses match is the first thing we do.
   //
   // Note, however, that we cannot guarantee the reverse of this.  Two strings at different memory addresses might be
   // the same eg if they are defined in headers that are included in multiple translation units and the compiler does
   // not optimise that away.
   if (this == &rhs) {
      return true;
   }
   if (this->cString == nullptr && rhs.cString == nullptr) { return true;  }
   if (this->cString == nullptr || rhs.cString == nullptr) { return false; }
   return 0 == std::strcmp(this->cString, rhs.cString);
}

bool BtStringConst::isNull() const {
   return (nullptr == this->cString);
}

char const * BtStringConst::operator*() const {
   return this->cString;
}

bool operator==(char const * const lhs, BtStringConst const & rhs) {
   return BtStringConst(lhs) == rhs;
}

bool operator==(BtStringConst const & lhs, char const * const rhs) {
   return lhs == BtStringConst(rhs);
}

bool operator==(QString const & lhs, BtStringConst const & rhs) {
   // Qt already provides operator ==  to compare QString with char const * const
   return lhs == *rhs;
}

bool operator==(BtStringConst const & lhs, QString const & rhs) {
   // Qt already provides operator ==  to compare QString with char const * const
   return *lhs == rhs;
}

bool operator!=(QString const & lhs, BtStringConst const & rhs) {
   return !(lhs == rhs);
}

QTextStream & operator<<(QTextStream & outputStream, BtStringConst const & btStringConst) {
   return btStringConst.writeTo(outputStream);
}

QDebug & operator<<(QDebug & outputStream, BtStringConst const & btStringConst) {
   return btStringConst.writeTo(outputStream);
}
