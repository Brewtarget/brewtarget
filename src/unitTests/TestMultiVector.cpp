/*======================================================================================================================
 * unitTests/TestMultiVector.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "unitTests/TestMultiVector.h"

#include <QtTest/QtTest>

#include "utils/BtStringConst.h"
#include "utils/MultiVector.h"

namespace {
   std::vector<BtStringConst> fruit{"apples", "oranges", "bananas"};
   std::vector<BtStringConst> animals{"llamas", "alpacas", "wombats", "hares", "elephants"};
   std::vector<BtStringConst> shorty{"single entry"};
}

void UnitTests::doTestsForMultiVector() {
   // Part of the motivation for creating MultiVector is to deal with structs containing BtStringConst, so it's a good
   // thing to test it with
   MultiVector<BtStringConst> const multiVector{
      {"one", "two", "three", "four"},
      // These are deliberately not in the same order as above!
      {&animals, &fruit, &shorty}
   };
   Q_ASSERT(multiVector[0] == BtStringConst{"one"});
   Q_ASSERT(multiVector[3] == BtStringConst{"four"});
   Q_ASSERT(multiVector[5] == BtStringConst{"alpacas"});
   Q_ASSERT(multiVector[9] == BtStringConst{"apples"});
   Q_ASSERT(multiVector[12] == BtStringConst{"single entry"});
   Q_ASSERT(multiVector.size() == 13);
   auto iterator = multiVector.begin();
   Q_ASSERT(*iterator == BtStringConst{"one"});
   ++iterator;
   Q_ASSERT(*iterator == BtStringConst{"two"});
   ++iterator;
   ++iterator;
   ++iterator;
   Q_ASSERT(*iterator == BtStringConst{"llamas"});
   iterator = multiVector.end();
   --iterator;
   Q_ASSERT(*iterator == BtStringConst{"single entry"});
   --iterator;
   Q_ASSERT(*iterator == BtStringConst{"bananas"});
   --iterator;
   Q_ASSERT(*iterator == BtStringConst{"oranges"});

   return;
}
