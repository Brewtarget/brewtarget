/*
 * utils/OptionalToStream.h is part of Brewtarget, and is copyright the following
 * authors 2022:
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
#ifndef UTILS_OPTIONALTOSTREAM_H
#define UTILS_OPTIONALTOSTREAM_H
#pragma once

#include <optional>

/**
 * \brief Convenience function to allow output of \c std::optional to \c QDebug or \c QTextStream stream
 *
 *        (For some reason, \c QDebug does not inherit from \c QTextStream so we template the stream class as well as
 *        what we're outputting.)
 */
template<class S, class T>
S & operator<<(S & stream, std::optional<T> optionalItem) {
   if (optionalItem) {
      stream << *optionalItem;
   } else {
      stream << "NULL";
   }
   return stream;
}

#endif
