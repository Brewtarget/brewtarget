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
#include "model/Salt.h"
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
