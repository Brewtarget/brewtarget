/*======================================================================================================================
 * utils/CastAndConvert.h is part of Brewtarget, and is copyright the following authors 2021-2026:
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
#ifndef UTILS_CASTANDCONVERT_H
#define UTILS_CASTANDCONVERT_H
#pragma once
#include <QList>


namespace CastAndConvert {
   /**
    * \brief Convert a list of shared pointers to a list of raw pointers
    * @tparam NE
    * @param input
    * @return
    */
   template<typename NE>
   QList<NE *> toRaw(QList<std::shared_ptr<NE>> const & input) {
      QList<NE *> output{input.size()};
      std::transform(input.cbegin(),
                     input.cend(),
                     output.begin(),
                     [](std::shared_ptr<NE> const & val) { return val.get(); });
      return output;
   }

   template<typename NE>
   QList<NE const *> toConstRaw(QList<std::shared_ptr<NE>> const & input) {
      QList<NE const *> output{input.size()};
      std::transform(input.cbegin(),
                     input.cend(),
                     output.begin(),
                     [](std::shared_ptr<NE> const & val) { return val.get(); });
      return output;
   }


   /**
    * \brief Convert QList<std::shared_ptr<QObject> > to QList<std::shared_ptr<NE> >
    */
   template<typename NE>
   QList<std::shared_ptr<NE>> toShared(QList<std::shared_ptr<QObject> > const & input) {
      // We can't just cast the resulting QList<std::shared_ptr<QObject> > to QList<std::shared_ptr<NE> >, so we need
      // to create a new QList of the type we want and copy the elements across.
      QList<std::shared_ptr<NE>> output;
      output.reserve(input.size());
      std::transform(input.cbegin(),
                     input.cend(),
                     std::back_inserter(output),
                     [](auto & sharedPointer) { return std::static_pointer_cast<NE>(sharedPointer); });
      return output;
   }

   /**
    * \brief Convert QList<std::shared_ptr<QObject> > to QList<NE *>
    */
   template<typename NE>
   QList<NE *> toRaw(QList<std::shared_ptr<QObject> > const & input) {
      // We can't just cast the resulting QList<std::shared_ptr<QObject> > to QList<std::shared_ptr<NE> >, so we need
      // to create a new QList of the type we want and copy the elements across.
      QList<NE *> output;
      output.reserve(input.size());
      std::transform(input.cbegin(),
                     input.cend(),
                     std::back_inserter(output),
                     [](auto & sharedPointer) { return static_cast<NE *>(sharedPointer.get()); });
      return output;
   }

}


#endif
