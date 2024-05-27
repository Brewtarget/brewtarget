/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/Fonts.cpp is part of Brewtarget, and is copyright the following authors 2023:
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
#include "utils/Fonts.h"

#include <QDebug>
#include <QFontDatabase>

namespace {
   /**
    * \brief Loads the specified font family file (typically from our resource bundle) and returns its name
    */
   QString loadFontFamily(char const * const fileName) {
      int id = QFontDatabase::addApplicationFont(fileName);
      if (id < 0) {
         // This is a coding error
         qCritical() << Q_FUNC_INFO << "Got negative id (" << id << ") loading" << fileName;
         Q_ASSERT(false);
      }
      return QFontDatabase::applicationFontFamilies(id).at(0);
   }
}

Fonts const & Fonts::getInstance() {
   // Per https://isocpp.org/wiki/faq/ctors#static-init-order, we use the "Construct On First Use Idiom", aka a Meyers
   // Singleton, to ensure we initialise after main() is invoked.  This is thread-safe since C++11 (see
   // https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton#h3-guarantees-of-the-c-runtime).
   //
   // We are not worried about the Fonts class being used during application shutdown, so we do not use a pointer here.
   // (See the isocpp.org link above for more discussion about this.)
   static Fonts const instance{};
   return instance;
}

Fonts::Fonts() :
   TexGyreSchola_BoldItalic{loadFontFamily(":/fonts/TexGyreSchola-BoldItalic.otf")},
   TexGyreSchola_Bold      {loadFontFamily(":/fonts/TexGyreSchola-Bold.otf"      )},
   TexGyreSchola_Italic    {loadFontFamily(":/fonts/TexGyreSchola-Italic.otf"    )},
   TexGyreSchola_Regular   {loadFontFamily(":/fonts/TexGyreSchola-Regular.otf"   )} {

   return;
}

Fonts::~Fonts() = default;
