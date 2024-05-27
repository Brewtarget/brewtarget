/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/FileSystemHelpers.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "utils/FileSystemHelpers.h"

QString FileSystemHelpers::toQString(std::filesystem::path const & path) {
   //
   // On Linux and Mac, native format for file paths is UTF-8, but on Windows it is double-byte characters (wchar_t)
   // which are not even UTF-16.
   //
   // Most of the std::filesystem::path functions for converting to strings leave things in native format (ie UTF-8 on
   // Linux/Mac and wchar_t on Windows).
   //
   // std::filesystem::path::generic_u8string() guarantees to give us UTF-8 on any platform, but using the new
   // std::u8string / char8_t types which are a bit of a pain to work with.
   //
   // In newer versions of Qt, a QString can be constructed directly from a null-terminated string of const char8_t*
   //
#if QT_VERSION < QT_VERSION_CHECK(6,1,0)
   // See comment below in makePath() for why this reinterpret_cast is always valid
   return QString::fromUtf8(QByteArray{reinterpret_cast<char const *>(path.generic_u8string().c_str())});
#else
   return QString::fromUtf8(path.generic_u8string().c_str());
#endif
}

std::filesystem::path FileSystemHelpers::makePath(QString const & val) {
   //
   // Similar comments as above apply here.  We're just going in the other direction.
   //
   // Per https://en.cppreference.com/w/cpp/language/types#Character_types, char8_t "has the same size, signedness, and
   // alignment as unsigned char (and therefore, the same size and alignment as char and signed char)" so this
   // reinterpret_cast is a valid thing to do.
   //
   return std::filesystem::path{reinterpret_cast<char8_t const *>(val.toUtf8().constData())};
}

QDebug & operator<<(QDebug & stream, std::filesystem::path const & path) {
   stream << FileSystemHelpers::toQString(path);
   return stream;
}
