/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/FuzzyCompare.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "utils/FuzzyCompare.h"

#include <QtGlobal>

bool Utils::FuzzyCompare(double lhs, double rhs) {
   // Of course we should never compare doubles, but if they really are exactly the same (eg both are 0.0, which is not
   // that uncommon), we might as well short circuit everything below.
   if (lhs == rhs) {
      return true;
   }

   //
   // Note that the qFuzzyCompare documentation says it "will not work" if either of the parameters is 0.0 or NaN or
   // infinity.  (Specifically qFuzzyCompare(0.0, 1e-16) will return false.)   If one of the values is always 0.0, we
   // are directed to use qFuzzyIsNull instead.    If one of the values is likely to be 0.0, it suggests adding 1.0 to
   // both values.
   //
   // Since we're paranoid, we cover the possibility that one or both sides could be -1.0.  This loop below should run
   // at most twice (eg in the case that lhs == 0.0 and rhs == -1.0).
   //
   while (0.0 == lhs || 0.0 == rhs) {
      lhs += 1.0;
      rhs += 1.0;
   }
   return qFuzzyCompare(lhs, rhs);
}

bool Utils::FuzzyCompare(std::optional<double> lhs, std::optional<double> rhs) {
   // I'm sure this can be done in a single line, but it would make my head hurt
   if (!lhs && !rhs) {
      return true;
   }
   if (!lhs || !rhs) {
      return false;
   }
   return Utils::FuzzyCompare(*lhs, *rhs);
}
