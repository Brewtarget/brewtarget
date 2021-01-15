/*
 * xml/XmlCoding.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/XmlCoding.h"

#include <QDebug>

#include <xalanc/XalanDOM/XalanNodeList.hpp>

#include "xml/XmlRecordCount.h"

XmlCoding::XmlCoding(QString const name,
                     QHash<QString, XmlRecordDefinition> const & entityNameToXmlRecordDefinition) :
   name{name},
   entityNameToXmlRecordDefinition{entityNameToXmlRecordDefinition} {
   qDebug() << Q_FUNC_INFO;
   return;
}


bool XmlCoding::isKnownXmlRecordType(QString recordName) const {
   return this->entityNameToXmlRecordDefinition.contains(recordName);
}


std::shared_ptr<XmlRecord> XmlCoding::getNewXmlRecord(QString recordName) const {
   XmlCoding::XmlRecordConstructorWrapper constructorWrapper =
      this->entityNameToXmlRecordDefinition.value(recordName).constructorWrapper;
   XmlRecord::FieldDefinitions const * fieldDefinitions =
      this->entityNameToXmlRecordDefinition.value(recordName).fieldDefinitions;
   return std::shared_ptr<XmlRecord>(constructorWrapper(*this, recordName, *fieldDefinitions));
}

bool XmlCoding::loadNormaliseAndStoreInDb(xalanc::DOMSupport & domSupport,
                                          xalanc::XalanNode * rootNode,
                                          QTextStream & userMessage) const {

   XQString rootNodeName{rootNode->getNodeName()};
   qDebug() << Q_FUNC_INFO << "Processing root node: " << rootNodeName;

   // It's a coding error if we don't understand how to process the root node, because it should have been validated
   // by the XSD.  (In the case of BeerXML, the root node is a manufactured one that we inserted, which is all the more
   // reason we should know how to process it!)
   Q_ASSERT(this->isKnownXmlRecordType(rootNodeName));

   std::shared_ptr<XmlRecord> rootRecord = this->getNewXmlRecord(rootNodeName);

   XmlRecordCount stats;

   if (!rootRecord->load(domSupport, rootNode, userMessage)) {
      return false;
   }

   if (!rootRecord->normaliseAndStoreInDb(userMessage, stats)) {
      return false;
   }

   // Everything went OK, so summarise what we read in into the message displayed on-screen to the user
   stats.writeToUserMessage(userMessage);

   return true;

}
