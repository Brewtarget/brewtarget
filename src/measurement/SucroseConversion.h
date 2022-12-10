/*
 * measurement/SucroseConversion.h is part of Brewtarget, and is copyright the following
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
#ifndef MEASUREMENT_SUCROSECONVERSION_H
#define MEASUREMENT_SUCROSECONVERSION_H
#pragma once

#include <cstddef> // For size_t

namespace Measurement {

   struct SucroseConversion {
      double refractiveIndexAt20C;
      double degreesBrix;
      double apparentSgAt2020C;
   };

   extern SucroseConversion const sucroseConversions[];

   extern size_t const sucroseConversions_size;
}

#endif
