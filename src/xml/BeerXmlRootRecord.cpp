/*
 * xml/BeerXmlRootRecord.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlRootRecord.h"

#include <QDebug>
#include <QVector>

#include <xalanc/XPath/XPathEvaluator.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Top-level field mappings for BeerXML files
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static QVector<XmlRecord::Field> const ROOT_RECORD_FIELDS {
   // Type              XML Path                    Q_PROPERTY   Enum Mapper
   {XmlRecord::Record, "HOPS/HOP",                 nullptr,     nullptr},
   {XmlRecord::Record, "FERMENTABLES/FERMENTABLE", nullptr,     nullptr},
   {XmlRecord::Record, "YEASTS/YEAST",             nullptr,     nullptr},
   {XmlRecord::Record, "MISCS/MISC",               nullptr,     nullptr},
   {XmlRecord::Record, "WATERS/WATER",             nullptr,     nullptr},
   {XmlRecord::Record, "STYLES/STYLE",             nullptr,     nullptr},
   {XmlRecord::Record, "MASHS/MASH",               nullptr,     nullptr},
   {XmlRecord::Record, "RECIPES/RECIPE",           nullptr,     nullptr},
   {XmlRecord::Record, "EQUIPMENTS/EQUIPMENT",     nullptr,     nullptr},
};

BeerXmlRootRecord::BeerXmlRootRecord(XmlCoding const & xmlCoding) :
   XmlRecord{xmlCoding, "BEER_XML", ROOT_RECORD_FIELDS} {
   return;
}
