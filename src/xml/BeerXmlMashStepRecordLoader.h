/*
 * xml/BeerXmlMashStepRecordLoader.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_BEERXMLMASHSTEPRECORDLOADER_H
#define _XML_BEERXMLMASHSTEPRECORDLOADER_H
#pragma once

#include "xml/XPathRecordLoader.h"
#include "mashstep.h"

class Mash;

/**
 * \brief Loads a <MASH_STEP>...</MASH_STEP> record in from a BeerXML file
 */
class BeerXmlMashStepRecordLoader : public XPathRecordLoader {
public:
   BeerXmlMashStepRecordLoader();
   virtual MashStep * findByName(QString nameToFind) { return XPathRecordLoader::findByNameOld<MashStep>(nameToFind); }
   virtual bool normalise(QTextStream & userMessage);

   /**
    * \brief A MashStep does not get stored in the DB other than in association with a Mash, so it's simpler to have
    *        the Mash first.
    * \param mash The Mash with which the MashStep needs to be associated
    */
   bool storeInDb(Mash * mash, QTextStream & userMessage);
};
#endif
