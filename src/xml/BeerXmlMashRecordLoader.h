/*
 * xml/BeerXmlMashRecordLoader.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_BEERXMLMASHRECORDLOADER_H
#define _XML_BEERXMLMASHRECORDLOADER_H
#pragma once

#include "xml/BeerXmlSimpleRecordLoader.h"
#include "xml/BeerXmlMashStepRecordLoader.h"

#include "mash.h"


/**
 * \brief Loads a <MASH>...</MASH> record in from a BeerXML file, including the <MASH_STEP>...</MASH_STEP> records it
 * contains (via \b BeerXmlMashStepRecordLoader).  See comment in xml/XPathRecordLoader.h for more complete
 * explanation.
 */
class BeerXmlMashRecordLoader : public BeerXmlSimpleRecordLoader<Mash> {
public:
   BeerXmlMashRecordLoader();

   virtual bool load(xalanc::DOMSupport & domSupport,
                     xalanc::XalanNode * rootNodeOfRecord,
                     QTextStream & userMessage);
   virtual bool normalise(QTextStream & userMessage);
   virtual bool storeInDb(QTextStream & userMessage);
private:
   // We only need a list here, but Qt docs steer you towards using QVector rather than QList -- see
   // https://doc.qt.io/qt-5/qlist.html#details.   Either way, it's unlikely to be a long enough list to make any
   // noticeable performance difference.
   QVector<std::shared_ptr<BeerXmlMashStepRecordLoader> > mashStepRecordLoaders;
};

#endif
