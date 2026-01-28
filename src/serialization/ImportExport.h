/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/ImportExport.h is part of Brewtarget, and is copyright the following authors 2013-2026:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#ifndef SERIALIZATION_IMPORTEXPORT_H
#define SERIALIZATION_IMPORTEXPORT_H
#pragma once

#include <optional>

#include <QList>

class Equipment;
class Fermentable;
class Hop;
class Misc;
class Recipe;
class Style;
class Water;
class Yeast;
class Mash;
class Boil;
class Fermentation;

namespace ImportExport {
   /**
    * Everything we want to export should be in one of these lists
    */
   struct Lists {
      QList<Recipe       const *> const * recipes       = nullptr;
      QList<Equipment    const *> const * equipments    = nullptr;
      QList<Fermentable  const *> const * fermentables  = nullptr;
      QList<Hop          const *> const * hops          = nullptr;
      QList<Misc         const *> const * miscs         = nullptr;
      QList<Style        const *> const * styles        = nullptr;
      QList<Water        const *> const * waters        = nullptr;
      QList<Yeast        const *> const * yeasts        = nullptr;
      QList<Mash         const *> const * mashes        = nullptr;
      QList<Boil         const *> const * boils         = nullptr;
      QList<Fermentation const *> const * fermentations = nullptr;
   };

   /**
    * \brief Import recipes, hops, equipment, etc from BeerXML or BeerJSON files either specified by the user or in the
    *        parameter.
    *
    *        For export, we let the user choose between BeerXML and BeerJSON by the file extension they choose.  This is
    *        similar to how other programs work (eg LibreOffice, Gimp), so I think it's OK, but we'll see what feedback
    *        is on usability.
    *
    * \param inputFiles If \c std::nullopt (ie not supplied) then user will be prompted for file(s) through the UI
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool importFromFiles(std::optional<QStringList> inputFiles = std::nullopt);

   /**
    * \brief Export recipes, hops, equipment, etc to a BeerXML or BeerJSON file specified by the user
    *        (We'll work out whether it's BeerXML or BeerJSON based on the filename extension, so doesn't need to be
    *        specified in advance.)
    *
    *        Each of the parameters is allowed to be \c nullptr or an empty list, but it is the caller's responsibility
    *        to ensure that not \b all of them are!
    *
    * \param recipes
    * \param equipments
    * \param fermentables
    * \param hops
    * \param miscs
    * \param styles
    * \param waters
    * \param yeasts
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool exportToFile(Lists const & exportLists);

   /**
    * \brief Version for when we know we're only exporting one type of thing
    *
    */
   template<typename NE> bool exportToFile(QList<NE const *> const & items);
}

#endif
