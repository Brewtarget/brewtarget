/*
 * xml/BeerXmlSimpleRecordLoader.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_BEERXMLSIMPLERECORDLOADER_H
#define _XML_BEERXMLSIMPLERECORDLOADER_H
#pragma once

#include "xml/XPathRecordLoader.h"
#include "model/NamedEntity.h"


/**
 * \brief Extends \b XPathRecordLoader to read in "simple" BeerXML records (ie ones that don't contain other BeerXML
 * records).  See comment in xml/XPathRecordLoader.h for more complete explanation.
 */
template<class NE>
class BeerXmlSimpleRecordLoader : public XPathRecordLoader {
public:
   /**
    * We only use template specialisations of this constructor, so just declare here and put specialisation definitions
    * in xml/BeerXmlSimpleRecordLoader.cpp
    */
   BeerXmlSimpleRecordLoader();

   /**
    * \brief Finds the first instance of \b NE with \b name() matching \b nameToFind
    * \param nameToFind
    * \return A pointer to a \b NE with a matching \b name(), if there is one, or \b nullptr if not.  Note that this
    *         function does not tell you whether more than one \b NE has the name \b nameToFind
    */
   virtual NamedEntity * findByName(QString nameToFind) {
      QList<NE *> listOfAllStored = Database::instance().getAll<NE>();
      auto found = std::find_if(listOfAllStored.begin(),
                                listOfAllStored.end(),
                                [nameToFind](NE * ne) {return ne->name() == nameToFind;});
      if (found == listOfAllStored.end()) {
         return nullptr;
      }
      return *found;
   }
};

#endif
