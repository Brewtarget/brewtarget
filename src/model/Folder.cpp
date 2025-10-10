/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Folder.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "model/Folder.h"

#include <QDebug>
#include <QString>

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "utils/AutoCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Folder.cpp"
#endif

QString Folder::localisedName() { return tr("Folder"); }
QString Folder::localisedName_path()     { return tr("Path"     ); }
QString Folder::localisedName_fullPath() { return tr("Full Path"); }


bool Folder::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Folder const & rhs = static_cast<Folder const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_path  , PropertyNames::Folder::path, propertiesThatDiffer)
   );
}

ObjectStore & Folder::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Folder>::getInstance();
}

TypeLookup const Folder::typeLookup {
   "Folder",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(Folder, path    , m_path  , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_NO_MV(Folder, fullPath, fullPath, NonPhysicalQuantity::String),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup}
};

Folder::Folder(QString const & fullPath) :
   NamedEntity{""} {
   this->setFullPath(fullPath);
   setObjectName("Folder");

   CONSTRUCTOR_END
   return;
}

Folder::Folder(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_path, namedParameterBundle, PropertyNames::Folder::path) {
   setObjectName("Folder");

   CONSTRUCTOR_END
   return;
}

Folder::Folder(Folder const & other) :
   NamedEntity{other},
   m_path    {other.m_path    } {
   setObjectName("Folder");

   CONSTRUCTOR_END
   return;
}

Folder::~Folder() = default;

QString Folder::path() const { return m_path; }
QString Folder::fullPath() const { return QString{"%1/%2"}.arg(this->m_path).arg(this->name()); }

void Folder::setPath(QString var) {
   m_path = var;
   return;
}

// changing the full path necessarily changes the name and the path
void Folder::setFullPath(QString var) {
   QStringList pieces = var.split("/", Qt::SkipEmptyParts);

   if (!pieces.isEmpty()) {
      this->setName(pieces.last());
      pieces.removeLast();
      this->setPath(pieces.join("/"));
   } else {
      this->setName(var);
      this->setPath(var);
   }
   return;
}
