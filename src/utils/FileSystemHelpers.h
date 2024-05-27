/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/FileSystemHelpers.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef UTILS_FILESYSTEMHELPERS_H
#define UTILS_FILESYSTEMHELPERS_H
#pragma once

#include <array>
#include <filesystem>

#include <QDebug>
#include <QString>
#include <QTextStream>

namespace {
   struct permissionAndFlag {
      std::filesystem::perms permission;
      char                   flag;
   };
   constexpr std::array allPermsAndFlags{
      permissionAndFlag{std::filesystem::perms::owner_read  , 'r'},
      permissionAndFlag{std::filesystem::perms::owner_write , 'w'},
      permissionAndFlag{std::filesystem::perms::owner_exec  , 'x'},
      permissionAndFlag{std::filesystem::perms::group_read  , 'r'},
      permissionAndFlag{std::filesystem::perms::group_write , 'w'},
      permissionAndFlag{std::filesystem::perms::group_exec  , 'x'},
      permissionAndFlag{std::filesystem::perms::others_read , 'r'},
      permissionAndFlag{std::filesystem::perms::others_write, 'w'},
      permissionAndFlag{std::filesystem::perms::others_exec , 'x'},
      //
      // We just add extra output for these "special" bits, rather than trying to mimic the output of, say, ls -l
      //
      permissionAndFlag{std::filesystem::perms::set_uid     , 'u'},
      permissionAndFlag{std::filesystem::perms::set_gid     , 'g'},
      permissionAndFlag{std::filesystem::perms::sticky_bit  , 's'},
   };
}


/**
 * \brief Convenience function to allow output of \c std::filesystem::perms to \c QDebug or \c QTextStream stream
 *
 *        (For some reason, \c QDebug does not inherit from \c QTextStream so we template the stream class.)
 *
 *        NB: Need this to work for std::ostringstream too
 */
template<class S>
S & operator<<(S & stream, std::filesystem::perms const & permissions) {

   // Since we want this to work for std::ostringstream, we have to be careful that we end up writing char * to stream,
   // rather than QString or std::string
   std::ostringstream output;

   for (auto ii : allPermsAndFlags) {
      char const outputChar{
         std::filesystem::perms::none == (ii.permission & permissions) ? '-' : ii.flag
      };
      output << outputChar;
   }

   stream << output.str().c_str();

   return stream;
}

/**
 * \brief Convenience function to allow output of \c std::filesystem::file_type to \c QDebug or \c QTextStream stream
 */
template<class S>
S & operator<<(S & stream, std::filesystem::file_type const & fileType) {
   switch (fileType) {
      case std::filesystem::file_type::none     : stream << "none"     ; break;
      case std::filesystem::file_type::not_found: stream << "not_found"; break;
      case std::filesystem::file_type::regular  : stream << "regular"  ; break;
      case std::filesystem::file_type::directory: stream << "directory"; break;
      case std::filesystem::file_type::symlink  : stream << "symlink"  ; break;
      case std::filesystem::file_type::block    : stream << "block"    ; break;
      case std::filesystem::file_type::character: stream << "character"; break;
      case std::filesystem::file_type::fifo     : stream << "fifo"     ; break;
      case std::filesystem::file_type::socket   : stream << "socket"   ; break;
      case std::filesystem::file_type::unknown  : stream << "unknown"  ; break;
   }
   return stream;
}

/**
 * \brief Convenience function to allow output of \c std::filesystem::file_status to \c QDebug or \c QTextStream stream
 */
template<class S>
S & operator<<(S & stream, std::filesystem::file_status const & status) {
   stream << "File type:" << status.type() << ", permissions:" << status.permissions();
   return stream;
}

namespace FileSystemHelpers {
   /**
    * \brief Helper function for converting \c std::filesystem::path to \c QString
    */
   QString toQString(std::filesystem::path const & path);

   /**
    * \brief Helper function for converting \c QString to \c std::filesystem::path
    */
   std::filesystem::path makePath(QString const & val);
}

/**
 * \brief Convenience function to allow output of \c std::filesystem::path to \c QDebug stream
 *
 *        Not templated as don't want to interfere with output to std::ostringstream etc.
 */
QDebug & operator<<(QDebug & stream, std::filesystem::path const & path);


#endif
