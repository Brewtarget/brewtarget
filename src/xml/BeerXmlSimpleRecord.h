/*
 * xml/BeerXmlSimpleRecord.h is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
 * - Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _XML_BEERXMLSIMPLERECORD_H
#define _XML_BEERXMLSIMPLERECORD_H
#pragma once

#include "xml/XmlNamedEntityRecord.h"
#include "model/NamedEntity.h"

#include "database.h"

////**
/// * \brief Extends \b XmlNamedEntityRecord to read in "simple" BeerXML records (ie ones that don't contain other BeerXML
/// * records).  See comment in xml/XmlNamedEntityRecord.h for more complete explanation.
/// */
///template<class NE>
///class BeerXmlSimpleRecord : public XmlNamedEntityRecord {
///public:
   /**
    * We only use template specialisations of this constructor, so just declare here and put specialisation definitions
    * in xml/BeerXmlSimpleRecord.cpp (or the equivalent file for a derived class)
    */
///   BeerXmlSimpleRecord(XmlCoding const & xmlCoding);

   /**
    * \brief Finds the first instance of \b NE with \b name() matching \b nameToFind.  This is used to avoid name
    *        clashes when loading some subclass of NamedEntity (eg Hop, Yeast, Equipment, Recipe) from an XML file (eg
    *        if are reading in a Recipe called "Oatmeal Stout" then this function can check whether we already have a
    *        Recipe with that name so that, assuming the new one is not a duplicate, we can amend its name to "Oatmeal
    *        Stout (1)" or some such.
    *
    *        Note that caller does not need to know which subclass of NamedEntity we are reading in, whereas, the
    *        implementation is specific to a subclass (albeit that we can write it once for all subclasses with the
    *        magic of templates).
    * \param nameToFind
    * \return A pointer to a \b NE with a matching \b name(), if there is one, or \b nullptr if not.  Note that this
    *         function does not tell you whether more than one \b NE has the name \b nameToFind
    */
///   virtual NamedEntity * findByName(QString nameToFind) {
///      QList<NE *> listOfAllStored = Database::instance().getAll<NE>();
///      auto found = std::find_if(listOfAllStored.begin(),
///                                listOfAllStored.end(),
///                                [nameToFind](NE * ne) {return ne->name() == nameToFind;});
///      if (found == listOfAllStored.end()) {
///         return nullptr;
///      }
///      return *found;
///   }
///};

#endif
