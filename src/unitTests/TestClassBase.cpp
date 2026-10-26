/*======================================================================================================================
 * unitTests/TestClassBase.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "unitTests/TestClassBase.h"

#include "unitTests/TestEnvironment.h"

UnitTests::TestClassBase::TestClassBase() : QObject() {
   return;
}

UnitTests::TestClassBase::~TestClassBase() = default;

void UnitTests::TestClassBase::initTestClass() {
   return;
}

void UnitTests::TestClassBase::cleanupTestClass() {
   return;
}

void UnitTests::TestClassBase::initTestCase() {
   // See comment in header file for why we need to do this here
   TestEnvironment::useApplicationLogging();
   this->initTestClass();
   return;
}

void UnitTests::TestClassBase::cleanupTestCase() {
   this->cleanupTestClass();
   return;
}
