/*======================================================================================================================
 * utils/CWrappers.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef UTILS_CWRAPPERS_H
#define UTILS_CWRAPPERS_H
#pragma once

#include <memory>

/**
 * Convenience tools for using C APIs (eg libxml2)
 */
namespace CWrappers {

   /**
    * Because std::unique_ptr can take a custom deleter, it can handle resource ownership even for libxml2 entities, even
    * though each of these is a C struct with its own custom "free" function to release the resource.
    *
    * By default, if you have a custom deleter type for std::unique_ptr, you need to specify the instance of it when you
    * initialise or assign to the unique_ptr.  In our case, each type always has a single global function to release
    * resources.  So, rather than keep repeating this, we create a default-constructable deleter functor that calls the
    * relevant global function.
    */
   template<auto freeFunctionForT>
   struct freeFunctionCaller {
      //
      // This struct has no member variables, so its default constructor and destructor are trivial.  (The constructor
      // also won't throw any exception, which is another requirement for using with std::unique_ptr.)  We just need to
      // provide the "functor" bit.  Although there is only one type to which it can apply, having this bit itself
      // templated simplifies the calling code -- and the compiler will tell us if we got it wrong because we'll be
      // trying to pass the wrong pointer type to freeFunctionForT.
      //
      template<typename T>
      void operator()(T * pointer) const noexcept {
         if (pointer) {
            freeFunctionForT(pointer);
         }
         return;
      }
   };

   /**
    * A version of std::unique_ptr that that stores a pointer to some C-library resource and, on going out of scope,
    * will call the function to call to release that resource.
    *
    * Note that we do not store the function to acquire the resource, because we don't need to "remember" it, and it
    * would just add a lot of complexity.  (Resource acquisition requires more and more varied arguments then resource
    * release.)
    *
    * \tparam T The type of pointer to store for the resource
    * \tparam freeFunctionForT The function to call to release the resource
    */
   template<typename T, auto freeFunctionForT>
   using unique_ptr = std::unique_ptr<T, freeFunctionCaller<freeFunctionForT>>;
}

#endif
