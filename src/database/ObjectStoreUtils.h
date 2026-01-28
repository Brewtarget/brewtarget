/*======================================================================================================================
 * database/ObjectStoreUtils.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef DATABASE_OBJECTSTOREUTILS_H
#define DATABASE_OBJECTSTOREUTILS_H
#pragma once

#include <QDebug>

#include "database/ObjectStoreWrapper.h"
#include "model/EnumeratedBase.h"
#include "model/NamedEntity.h"
#include "model/OwnedByRecipe.h"

namespace ObjectStoreUtils {

   /**
    * \brief Given a proposed (or existing) name for this type of \c NamedEntity, this function returns either the same
    *        name (if no object of this type uses it) or a modified name (if there is already an object of this type
    *        with the proposed name).
    */
   template<class NE>
   QString normaliseName(QString const & candidateName) {
      // QString::simplified not only trims trailing and leading spaces but also converts tabs etc to spaces and
      // condenses double spaces to single ones.
      QString normalisedName = candidateName.simplified();

      //
      // If something doesn't have a name, it's useful to give it a default one.  Eg the BeerXML 1.0 standard says
      // the NAME tag has to be present, not that it can't be empty
      //
      if (normalisedName.trimmed().isEmpty()) {
         qInfo() <<
            Q_FUNC_INFO << "Setting default name on unnamed" << NE::staticMetaObject.className() << "record";
         // Note that tr() is a static member function of QObject.  We do not inherit from QObject, but NE does
         // (via NamedEntity).
         normalisedName = QString{NE::tr("Unnamed %1")}.arg(NE::localisedName());
      }

      //
      // As above, we don't substantively normalise the names of "owned" objects.  Eg it's OK, and indeed typical, for
      // mash steps in two different mashes to have the same name.  MashStep names are usually simply descriptive of
      // what the step is (eg "Mash In", "Mash Out", "Conversion", "Final Batch Sparge").
      //
      if constexpr (std::is_base_of<OwnedByRecipe, NE>::value ||
                    IsBaseClassTemplateOf<EnumeratedBase, NE>) {
         return normalisedName;
      }

      while (
         //
         // At the moment, we're pretty strict here and count a name clash even for things that are soft deleted.  If
         // we wanted to allow clashes with such soft-deleted things then we could add a check against ne->deleted()
         // as in the isDuplicate() function.
         //
         auto matchResult = ObjectStoreWrapper::findFirstMatching<NE>(
            [normalisedName](std::shared_ptr<NE> ne) {return ne->name().simplified() == normalisedName;}
         )
      ) {
         qDebug() << Q_FUNC_INFO << "Found existing " << NE::staticMetaObject.className() << "named" << normalisedName;

         NamedEntity::modifyClashingName(normalisedName);

         //
         // Now the for loop will search again with the new name
         //
         qDebug() << Q_FUNC_INFO << "Trying " << normalisedName;
      }

      return normalisedName;

   }


}

#endif
