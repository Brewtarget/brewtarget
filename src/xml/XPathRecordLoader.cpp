/*
 * xml/XPathRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/XPathRecordLoader.h"

#include <QDebug>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QMetaProperty>

#include <xalanc/XPath/XPathEvaluator.hpp>
#include <xalanc/XalanDOM/XalanNodeList.hpp>

#include "database.h"

#include "xml/BeerXmlSimpleRecordLoader.h"
#include "xml/BeerXmlMashRecordLoader.h"

constexpr char const * const XPathRecordLoader::XALAN_NODE_TYPES[] {
   "UNKNOWN_NODE",                 //= 0,
   "ELEMENT_NODE",                 //= 1,
   "ATTRIBUTE_NODE",               //= 2,
   "TEXT_NODE",                    //= 3,
   "CDATA_SECTION_NODE",           //= 4,
   "ENTITY_REFERENCE_NODE",        //= 5,
   "ENTITY_NODE",                  //= 6,
   "PROCESSING_INSTRUCTION_NODE",  //= 7,
   "COMMENT_NODE",                 //= 8,
   "DOCUMENT_NODE",                //= 9,
   "DOCUMENT_TYPE_NODE",           //= 10,
   "DOCUMENT_FRAGMENT_NODE",       //= 11,
   "NOTATION_NODE",                //= 12
   "UNRECOGNISED!"
};

// In theory, once we know the record set name, we can deduce the name of the records and vice versa (HOPS <-> HOP etc)
// but this is only because BeerXML uses "MASHS" as a mangled plural of "MASH" (instead of "MASHES").  Doing a proper
// mapping keeps things open for, say, possible future improved versions of BeerXML.
//
// And we can't just look at the child node of the record set, as the BeerXML 1.0 Standard allows extra undefined tags
// to be added to a document.
QHash<QString, XPathRecordLoader::Factory> XPathRecordLoader::RECORD_SET_TO_LOADER_LOOKUP {
   {"HOPS",          &XPathRecordLoader::construct< BeerXmlSimpleRecordLoader<Hop> >},
   {"FERMENTABLES",  &XPathRecordLoader::construct< BeerXmlSimpleRecordLoader<Fermentable> >},
   {"YEASTS",        &XPathRecordLoader::construct< BeerXmlSimpleRecordLoader<Yeast> >},
   {"MISCS",         &XPathRecordLoader::construct< BeerXmlSimpleRecordLoader<Misc> >},
   {"WATERS",        &XPathRecordLoader::construct< BeerXmlSimpleRecordLoader<Water> >},
   {"STYLES",        &XPathRecordLoader::construct< BeerXmlSimpleRecordLoader<Style> >},
   {"MASHS",         &XPathRecordLoader::construct<BeerXmlMashRecordLoader>},
   {"RECIPES",       nullptr}, //TODO
   {"EQUIPMENTS",    &XPathRecordLoader::construct< BeerXmlSimpleRecordLoader<Equipment> > }
};

bool XPathRecordLoader::factoryExists(QString recordSetName) {
   return XPathRecordLoader::RECORD_SET_TO_LOADER_LOOKUP.contains(recordSetName);
}

XPathRecordLoader::Factory XPathRecordLoader::getFactory(QString recordSetName) {
   return XPathRecordLoader::RECORD_SET_TO_LOADER_LOOKUP.value(recordSetName);
}

XPathRecordLoader::XPathRecordLoader(QString const recordName,
                                     XPathRecordLoader::NameUniqueness uniquenessOfInstanceNames,
                                     QVector<Field> const & fieldDefinitions,
                                     NamedEntity * entityToPopulate) :
   recordName{recordName},
   uniquenessOfInstanceNames{uniquenessOfInstanceNames},
   fieldDefinitions{fieldDefinitions},
   entityToPopulate{entityToPopulate},
   fieldsRead{} {
   Q_ASSERT(nullptr != entityToPopulate);
   return;
}

QString const & XPathRecordLoader::getRecordName() const {
   return this->recordName;
}


bool XPathRecordLoader::load(xalanc::DOMSupport & domSupport,
                             xalanc::XalanNode * rootNodeOfRecord,
                             QTextStream & userMessage) {
   qDebug() << Q_FUNC_INFO;

   xalanc::XPathEvaluator xPathEvaluator;
   for (auto ii = this->fieldDefinitions.cbegin(); ii < this->fieldDefinitions.cend(); ++ii) {

      // NB: If we don't find a node, there's nothing for us to do.  The XSD parsing should already flagged up an error
      // if there are missing required fields or if string fields that are present are not allowed to be blank.  (See
      // comments in BeerXml.xsd for why it is, in practice, plausible and acceptable for some "required" text fields
      // to be empty/blank.)
      xalanc::XalanNode * fieldContainerNode = xPathEvaluator.selectSingleNode(domSupport,
                                                                               rootNodeOfRecord,
                                                                               ii->xPath.getXalanString());
      if (fieldContainerNode) {
         // Normally the node for the tag will be type ELEMENT_NODE and will not have a value in and of itself.
         // To get the "contents", we need to look at the value of the child node, which should be type TEXT_NODE (and
         // name "#text").
         XQString fieldName{fieldContainerNode->getNodeName()};
         xalanc::XalanNodeList const * fieldContents = fieldContainerNode->getChildNodes();
         int numChildrenOfContainerNode = fieldContents->getLength();
         qDebug() <<
            Q_FUNC_INFO << "Node " << ii->xPath << "(" << fieldName << ":" <<
            XPathRecordLoader::XALAN_NODE_TYPES[fieldContainerNode->getNodeType()] << ") has " <<
            numChildrenOfContainerNode << " children";
         if (0 == numChildrenOfContainerNode) {
            qDebug() << Q_FUNC_INFO << "Empty!";
         } else {
            if (numChildrenOfContainerNode > 1) {
               qWarning() <<
                  Q_FUNC_INFO << "Node " << ii->xPath << " has " <<
                  numChildrenOfContainerNode << " children.  Taking value only of the first one.";
            }
            xalanc::XalanNode * valueNode = fieldContents->item(0);
            XQString value(valueNode->getNodeValue());
            qDebug() << Q_FUNC_INFO << "Value " << value;
            bool parsedValueOk = false;
            QVariant parsedValue;

            // A field should have a stringToEnum mapping if and only if it's of type Enum
            // Anything else is a coding error at the caller
            Q_ASSERT((XPathRecordLoader::Enum == ii->fieldType) != (nullptr == ii->stringToEnum));

            switch(ii->fieldType) {

               case XPathRecordLoader::Bool:
                  // Unlike other XML documents, boolean fields in BeerXML are caps
                  if (value == "TRUE") {
                     parsedValue.setValue(true);
                     parsedValueOk = true;
                  } else if (value == "FALSE") {
                     parsedValue.setValue(true);
                     parsedValueOk = true;
                  } else {
                     // This is almost certainly a coding error, as we should have already validated that the field is
                     // TRUE or FALSE via XSD parsing.
                     qWarning() <<
                        Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << ii->xPath << "=" <<
                        value << " as could not be parsed as BOOLEAN";
                  }
                  break;

               case XPathRecordLoader::Int:
                  // QString's toInt method will report success/failure of parsing straight back into our flag
                  parsedValue.setValue(value.toInt(&parsedValueOk));
                  if (!parsedValueOk) {
                     // This is almost certainly a coding error, as we should have already validated the field via XSD
                     // parsing.
                     qWarning() <<
                        Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << ii->xPath << "=" <<
                        value << " as could not be parsed as integer";
                  }
                  break;

               case XPathRecordLoader::UInt:
                  // QString's toUInt method will report success/failure of parsing straight back into our flag
                  parsedValue.setValue(value.toUInt(&parsedValueOk));
                  if (!parsedValueOk) {
                     // This is almost certainly a coding error, as we should have already validated the field via XSD
                     // parsing.
                     qWarning() <<
                        Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << ii->xPath << "=" <<
                        value << " as could not be parsed as unsigned integer";
                  }
                  break;

               case XPathRecordLoader::Double:
                  // QString's toDouble method will report success/failure of parsing straight back into our flag
                  parsedValue.setValue(value.toDouble(&parsedValueOk));
                  if (!parsedValueOk) {
                     // This is almost certainly a coding error, as we should have already validated the field via XSD
                     // parsing.
                     qWarning() <<
                        Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << ii->xPath << "=" <<
                        value << " as could not be parsed as decimal number (double)";
                  }
                  break;

               case XPathRecordLoader::Enum:
                  // It's definitely a coding error if there is no stringToEnum mapping for a field declared as Enum!
                  Q_ASSERT(nullptr != ii->stringToEnum);
                  if (!ii->stringToEnum->contains(value)) {
                     // This is probably a coding error as the XSD parsing should already have verified that the
                     // contents of the node are one of the expected values.
                     qWarning() <<
                        Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << ii->xPath << "=" <<
                        value << " as value not recognised";
                  } else {
                     parsedValue.setValue(ii->stringToEnum->value(value));
                     parsedValueOk = true;
                  }
                  break;

               // By default we assume it's a string
               case XPathRecordLoader::String:
               default:
                  if (ii->fieldType != XPathRecordLoader::String) {
                     // This is almost certainly a coding error in this class as we should be able to parse all the
                     // types callers need us to.
                     qWarning() <<
                        Q_FUNC_INFO << "Treating " << this->recordName << " node " << ii->xPath << "=" <<
                        value << " as string because did not recognise requested parse type " << ii->fieldType;
                  }
                  parsedValue.setValue(static_cast<QString>(value));
                  parsedValueOk = true;
                  break;
            }

            //
            // What we do if we couldn't parse the value depends.  If it was a value that we didn't need to set on the
            // supplied Hop/Yeast/Recipe/Etc object, then we can just ignore the problem and carry on processing.  But,
            // if this was a field we were expecting to use, then, with one exception, it's a problem that we couldn't
            // parse it and we should bail.
            //
            // The exception is if we already successfully parsed a field of this name in this record and this is an
            // unexpected (and unwanted) subsequent occurrence of the same field that we would ignore anyway.
            //
            if (!parsedValueOk && nullptr != ii->propertyName) {
               if (this->fieldsRead.contains(ii->xPath)) {
                  qWarning() <<
                     Q_FUNC_INFO << "Ignoring unparseable " << this->recordName << " node " << ii->xPath <<
                     "=" << value << " as we already read and used a value (" <<
                     this->fieldsRead.value(ii->xPath).toString() << ") for " << ii->propertyName;
               } else {
                  userMessage <<
                     "Could not parse " << this->recordName << " node " << ii->xPath << "=" << value << " into " <<
                     ii->propertyName;
                  return false;
               }
            }

            //
            // So we've either parsed the value OK or we don't need it (or both)
            //
            if (parsedValueOk) {
               //
               // If we parsed it OK, store it in the look-up map (unless we already parsed one of these fields) so we
               // can report duplicate fields.
               //
               if (this->fieldsRead.contains(ii->xPath)) {
                  qWarning() <<
                     Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << ii->xPath << "=" <<
                     value << " as we already read a value (" << this->fieldsRead.value(ii->xPath).toString() << ")";
               } else {
                  this->fieldsRead.insert(ii->xPath, parsedValue);

                  // Now we've stored the value in our field map, we can, if one was supplied, also call the relevant
                  // setter on the record object via the magic of the Qt Property System
                  if (nullptr != ii->propertyName) {
                     int propertyIndex = this->entityToPopulate->metaObject()->indexOfProperty(ii->propertyName);

                     if ( propertyIndex < 0 ) {
                        // Getting here means a coding error at the caller.  But try to fail gracefully nonetheless.
                        qCritical() <<
                           Q_FUNC_INFO << "Trying to update undeclared property " << ii->propertyName << " of " <<
                           this->entityToPopulate->metaObject()->className();
                        userMessage <<
                           "Unable to set property " << ii->propertyName << " of " <<
                           this->entityToPopulate->metaObject()->className();
                        return false;
                     }

                     QMetaProperty metaProperty = this->entityToPopulate->metaObject()->property(propertyIndex);
                     metaProperty.write(this->entityToPopulate.get(), parsedValue);
                  }
               }
            }
         }
      }
   }
   return true;
}


bool XPathRecordLoader::normalise(QTextStream & userMessage) {
   if (XPathRecordLoader::EachInstanceNameShouldBeUnique == this->uniquenessOfInstanceNames) {
      QString currentName = this->entityToPopulate->name();

      for (NamedEntity * matchingEntity = this->findByName(currentName);
           nullptr != matchingEntity;
           matchingEntity = this->findByName(currentName)) {

         qDebug() <<
            Q_FUNC_INFO << "Existing " << this->recordName << "named" << currentName << "was" <<
            ((nullptr == matchingEntity) ? "not" : "") << "found";

         //
         // Amend currentName and see if it's still a clash.  Eg if the clashing name is "Oatmeal Stout", we'll try
         // adding a "duplicate number" in brackets to the end of the name, ie amending it to "Oatmeal Stout (1)".  If
         // that clashes too then we want to try "Oatmeal Stout (2)" (and NOT "Oatmeal Stout (1) (1)"!).
         //
         // First, see whether there's already a (n) (ie "(1)", "(2)" etc) at the end of the name (with or without
         // space(s) preceding the left bracket.  If so, we want to replace this with " (n+1)".  If not, we try " (1)".
         //
         // Note that, in the regexp, to match a bracket, we need to escape it, thus "\(" instead of "(".  However, we
         // must also escape the backslash so that the C++ compiler doesn't think we want a special character (such as
         // '\n') and barf a "unknown escape sequence" warning at us.  So "\\(" is needed in the string literal here to
         // pass "\(" to the regexp to match literal "(" (and similarly for close bracket).
         //
         int duplicateNumber = 1;
         QRegExp nameNumberMatcher{" *\\(([0-9]+)\\)$"};
         int positionOfMatch = nameNumberMatcher.indexIn(currentName);
         if (positionOfMatch > -1) {
            // There's already some integer in brackets at the end of the name, extract it, add one, and truncate the
            // name.
            duplicateNumber = nameNumberMatcher.cap(1).toInt() + 1;
            currentName.truncate(positionOfMatch);
         }
         currentName += QString(" (%1)").arg(duplicateNumber);

         //
         // Now the for loop will search again with the new name
         //
         qDebug() << Q_FUNC_INFO << "Trying " << currentName;
      }

      this->entityToPopulate->setName(currentName);
   }
   return true;
}


bool XPathRecordLoader::storeInDb(QTextStream & userMessage) {
   // Most entities know how to save themselves to the DB
   this->entityToPopulate->insertInDatabase();

   // Once we've stored the object, we no longer have to take responsibility for destroying it because its registry
   // (currently the Database singleton) will now own it.
   this->entityToPopulate.release();

   return true;
}
