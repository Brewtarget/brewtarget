/*
 * xml/BeerXmlEquipmentRecordLoader.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_BEERXMLEQUIPMENTRECORDLOADER_H
#define _XML_BEERXMLEQUIPMENTRECORDLOADER_H
#pragma once

#include "xml/XPathRecordLoader.h"
#include "equipment.h"

/**
 * \brief Loads a <EQUIPMENT>...</EQUIPMENT> record in from a BeerXML file
 */
class BeerXmlEquipmentRecordLoader : public XPathRecordLoader {
public:
   BeerXmlEquipmentRecordLoader();
   virtual Equipment * findByName(QString nameToFind) { return XPathRecordLoader::findByName<Equipment>(nameToFind); }
};
#endif
