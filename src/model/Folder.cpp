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

void FolderCommon::commonPathPrefix(QString & baseFolderPath, QString const & folderPath) {
   //
   // Bail out if baseFolderPath is already the minimal possible prefix
   //
   if (baseFolderPath.isEmpty() || baseFolderPath == "/") {
      return;
   }

   //
   // We have to be careful here that we are looking for a common path prefix, not a common string prefix.
   // Eg given "/foo/bar" and "/foo/barbie", the common path prefix is "/foo", not "/foo/bar".  Nonetheless,
   // it keeps our life simple if we search initially for a common string prefix, and then truncate it down
   // to a common path prefix if need be.
   //
   // QString doesn't have native functions to get a common string prefix.  But the C++ standard library
   // saves us from reinventing the wheel: std::mismatch returns a pair of iterators to the first pair of
   // mismatching elements from two target ranges.
   //
   auto const firstMismatches = std::mismatch(baseFolderPath.constBegin(),
                                              baseFolderPath.constEnd(),
                                              folderPath.constBegin(),
                                              folderPath.constEnd());
   if (firstMismatches.first == baseFolderPath.constEnd()) {
      //
      // Here, baseFolderPath is a string prefix for folderPath (and a non-trivial one, otherwise we would have bailed
      // out above).  There is nothing to do if folderPath is the same length (and thus the same) as baseFolderPath.
      //
      if (firstMismatches.second == folderPath.constEnd()) {
         return;
      }

      //
      // Here, folderPath is longer than baseFolderPath, and the latter is non-trivial.  We look at the next character
      // in folderPath after the baseFolderPath prefix.  If it's '/', then baseFolderPath is a common path prefix, not
      // just a common string prefix, and there's nothing to do.  Otherwise, we truncate baseFolderPath at its last '/'
      // to turn it into a common path prefix.
      //
      if (*firstMismatches.second == QChar('/')) {
         return;
      }
      // If no '/' is found, QString::lastIndexOf() returns -1 and QString::truncate() treats this as
      // meaning truncate at 0 (ie erase string), which is what we want.
      auto const lastSlashPosition = baseFolderPath.lastIndexOf("/");
      baseFolderPath.truncate(lastSlashPosition);
      return;
   }

   if (firstMismatches.second == folderPath.constEnd()) {
      //
      // Here, folderPath is a string prefix for baseFolderPath.  Since the reverse is not true (otherwise we'd already
      // be in the branch above), it must be shorter than baseFolderPath.
      //
      // If folderPath is trivial ("" or "/"), then this is the new baseFolderPath.
      //
      if (folderPath.isEmpty() || folderPath == "/") {
         baseFolderPath = folderPath;
         return;
      }
      //
      // Here, folderPath is non-trivial, shorter than baseFolderPath, and a string prefix of baseFolderPath, so it's
      // the mirror case of one we handled above.  If the first character in baseFolderPath after the folderPath prefix
      // is '/', then folderPath is a common path prefix.  Otherwise, we truncate folderPath at its last '/' to make it
      // a common path prefix.  (In fact we truncate baseFolderPath at the position of folderPath's last '/' to save us
      // an assignment.)
      //
      if (*firstMismatches.first == QChar('/')) {
         baseFolderPath = folderPath;
         return;
      }
      // Per comment above, it is intentional here that we search in folderPath but truncate in baseFolderPath.
      auto const lastSlashPosition = folderPath.lastIndexOf("/");
      baseFolderPath.truncate(lastSlashPosition);
      return;
   }

   //
   // If we reached here then neither string is a string prefix of the other.
   //
   // First we handle the case that the two strings share no substantive common prefix
   //
   if (firstMismatches.first == baseFolderPath.constBegin() + 1) {
      baseFolderPath = "";
      return;
   }
   if (firstMismatches.first == baseFolderPath.constBegin() + 2 && *baseFolderPath.constBegin() == QChar('/')) {
      baseFolderPath = "/";
      return;
   }

   //
   // If the common string prefix is also a common path prefix then the character before the first mismatch will be a
   // '/'.  Eg for the paths below, the common string prefix is "/foo/bar/".
   //    /foo/bar/hum
   //    /foo/bar/bug
   //    ---------^
   //
   // In this case, we want to chop off this trailing /.
   //
   // The remaining case is that the common string prefix is not a common path prefix, eg:
   //    /foo/bar/humble
   //    /foo/bar/humbug
   //    -------------^
   // or
   //    /foo/bar/hum/bug
   //    /foo/bar/hem
   //    ----------^
   //
   // The good news is that the logic is the same.  We work back from the mismatch (in either string, because prior to
   // the mismatch they are the same!) until we find a '/', then chop from there onwards.
   //
   // Although we could search with the iterators via std::find, by using std::make_reverse_iterator to turn them into
   // reverse iterators, there's a simpler approach.  We first chop off everything but the common string prefix, and
   // then use QString's built-in search to find the last '/' in that common string prefix.
   //
   baseFolderPath.erase(firstMismatches.first, baseFolderPath.constEnd());
   baseFolderPath.truncate(baseFolderPath.lastIndexOf("/"));
   return;
}