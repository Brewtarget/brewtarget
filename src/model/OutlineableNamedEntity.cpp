/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/OutlineableNamedEntity.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#include "model/OutlineableNamedEntity.h"

#include "measurement/NonPhysicalQuantity.h"
#include "model/NamedParameterBundle.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_OutlineableNamedEntity.cpp"
#endif

QString OutlineableNamedEntity::localisedName() { return tr("Outlineable Named Entity"); }

TypeLookup const OutlineableNamedEntity::typeLookup {
   "OutlineableNamedEntity",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::OutlineableNamedEntity::outline, OutlineableNamedEntity::m_outline, NonPhysicalQuantity::Bool),
   },
   // Parent class lookup.  NB: RecipeAddition not NamedEntity!
   {&NamedEntity::typeLookup}
};

OutlineableNamedEntity::OutlineableNamedEntity(QString name) :
   NamedEntity{name},
   m_outline{false} {

   CONSTRUCTOR_END
   return;
}

OutlineableNamedEntity::OutlineableNamedEntity(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   SET_REGULAR_FROM_NPB(m_outline, namedParameterBundle, PropertyNames::OutlineableNamedEntity::outline, false) {

   CONSTRUCTOR_END
   return;
}

OutlineableNamedEntity::OutlineableNamedEntity(OutlineableNamedEntity const & other) :
   NamedEntity{other},
   m_outline{other.m_outline} {

   CONSTRUCTOR_END
   return;
}

OutlineableNamedEntity::~OutlineableNamedEntity() = default;

bool OutlineableNamedEntity::outline() const {
   return this->m_outline;
}

void OutlineableNamedEntity::setOutline(bool const val) {
   // Normally in a setter for an object derived from NamedEntity, we would call SET_AND_NOTIFY.  But we know that
   // m_outline is not stored in the DB and not used in any of the display logic, so we can skip that.
   this->m_outline = val;
   return;
}
