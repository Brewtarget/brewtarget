/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/NoCopy.h is part of Brewtarget, and is copyright the following authors 2023:
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
#ifndef UTILS_NOCOPY_H
#define UTILS_NOCOPY_H
#pragma once

/**
 * \brief This is a useful macro to include in header file to prevent a class being copyable.  Instead of writing, eg:
 *
 *           //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
 *           ObjectStore(ObjectStore const &) = delete;
 *           //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
 *           ObjectStore & operator=(ObjectStore const &) = delete;
 *           //! No move constructor
 *           ObjectStore(ObjectStore &&) = delete;
 *           //! No move assignment
 *           ObjectStore & operator=(ObjectStore &&) = delete;
 *
 *        we write:
 *
 *           NO_COPY_DECLARATIONS(ObjectStore)
 */
#define NO_COPY_DECLARATIONS(ClassName) \
   ClassName(ClassName const &) = delete; \
   ClassName & operator=(ClassName const &) = delete; \
   ClassName(ClassName &&) = delete; \
   ClassName & operator=(ClassName &&) = delete;

#endif
