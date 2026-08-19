/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Folder.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/Hop.h"
#include "model/NamedParameterBundle.h"
#include "model/Mash.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "utils/AutoCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Folder.cpp"
#endif

QString FolderCommon::localisedName()          { return tr("Folder"); }
QString FolderCommon::localisedName_path()     { return tr("Path"     ); }
QString FolderCommon::localisedName_fullPath() { return tr("Full Path"); }


bool FolderCommon::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   FolderCommon const & rhs = static_cast<FolderCommon const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_PROPERTY_COMPARE_FN(this, rhs, fullPath, PropertyNames::FolderCommon::fullPath, propertiesThatDiffer)
   );
}

TypeLookup const FolderCommon::typeLookup {
   "FolderCommon",
   {
      PROPERTY_TYPE_LOOKUP_NO_MV(FolderCommon, path    , path      , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_NO_MV(FolderCommon, fullPath, fullPath  , NonPhysicalQuantity::String),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup}
};

FolderCommon::FolderCommon(QString const & name) :
   NamedEntity{name} {

   CONSTRUCTOR_END
   return;
}

FolderCommon::FolderCommon(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle} {

   CONSTRUCTOR_END
   return;
}

FolderCommon::FolderCommon(FolderCommon const & other) :
   NamedEntity{other} {

   CONSTRUCTOR_END
   return;
}

FolderCommon::~FolderCommon() = default;

[[nodiscard]] QString FolderCommon::joinPaths(QString const & leftPath, QString const & rightPath) {
   //
   // We don't worry about leading slash on leftPath or trailing slash on rightPath, as those are harmless.  But we
   // want to ensure there is exactly one slash where they join.
   //
   return QString("%1/%2").arg(
      leftPath.chopped(leftPath.endsWith('/') ? 1 : 0),    // Chop any trailing slash off leftPath
      rightPath.sliced(rightPath.startsWith('/') ? 1 : 0)  // Chop any leading slash off rightPath
   );
}

[[nodiscard]] QString FolderCommon::subPath(QString const & basePath, QString const & fullPath) {
   if (basePath.isEmpty() || basePath == "/" || !fullPath.startsWith(basePath)) {
      return fullPath;
   }

   return fullPath.sliced(basePath.length());
}