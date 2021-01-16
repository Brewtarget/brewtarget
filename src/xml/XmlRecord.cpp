/*
 * xml/XmlRecord.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/XmlRecord.h"

#include <QDebug>

#include <xalanc/XalanDOM/XalanNodeList.hpp>
#include <xalanc/XPath/NodeRefList.hpp>
#include <xalanc/XPath/XPathEvaluator.hpp>

#include "xml/XmlCoding.h"

constexpr char const * const XmlRecord::XALAN_NODE_TYPES[] {
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

XmlRecord::XmlRecord(XmlCoding const & xmlCoding,
                     QString const recordName,
                     QVector<Field> const & fieldDefinitions) :
   xmlCoding{xmlCoding},
   recordName{recordName},
   fieldDefinitions{fieldDefinitions},
   instanceNamesAreUnique{true}, // Default value. Child class constructors may modify
   namedEntity{nullptr},
   namedEntityRaiiContainer{nullptr},
   childRecords{} {
   return;
}

QString const & XmlRecord::getRecordName() const {
   return this->recordName;
}

bool XmlRecord::load(xalanc::DOMSupport & domSupport,
                     xalanc::XalanNode * rootNodeOfRecord,
                     QTextStream & userMessage,
                     std::shared_ptr< QHash<QString, QVariant> > fieldsRead) {
   qDebug() << Q_FUNC_INFO;

   xalanc::XPathEvaluator xPathEvaluator;
   //
   // Loop through all the fields that we know/care about.  Anything else is intentionally ignored.  (We won't know
   // what to do with it, and, if it weren't allowed to be there, it would have generated an error at XSD parsing.)
   //
   for (auto fieldDefinition = this->fieldDefinitions.cbegin(); fieldDefinition < this->fieldDefinitions.cend(); ++fieldDefinition) {
      //
      // NB: If we don't find a node, there's nothing for us to do.  The XSD parsing should already flagged up an error
      // if there are missing _required_ fields or if string fields that are present are not allowed to be blank.  (See
      // comments in BeerXml.xsd for why it is, in practice, plausible and acceptable for some "required" text fields
      // to be empty/blank.)
      //
      // We're not expecting multiple instances of simple fields (strings, numbers, etc) and XSD parsing should mostly
      // have flagged up errors if there were any present.  But it is often valid to have multiple child records (eg
      // Hops inside a Recipe).
      //
      xalanc::NodeRefList nodesForCurrentXPath;
      xPathEvaluator.selectNodeList(nodesForCurrentXPath,
                                    domSupport,
                                    rootNodeOfRecord,
                                    fieldDefinition->xPath.getXalanString());
      auto numChildNodes = nodesForCurrentXPath.getLength();
      qDebug() << Q_FUNC_INFO << "Found" << numChildNodes << "node(s) for " << fieldDefinition->xPath;
      if (XmlRecord::Record == fieldDefinition->fieldType) {
         //
         // Depending on the context, it may or may not be valid to have multiple children of this type of record (eg
         // a Recipe might have multiple Hops but it only has one Equipment).  We don't really have to worry about that
         // here though as any rules should have been enforced in the XSD.
         //
         if (!this->loadChildRecords(domSupport, nodesForCurrentXPath, userMessage)) {
            return false;
         }
      } else if (numChildNodes > 0) {
         //
         // If the field we're looking at is not a record, so the XSD should mostly have enforced no duplicates.  If
         // there are any though, we'll ignore them.
         //
         if (numChildNodes > 1) {
            qWarning() <<
               Q_FUNC_INFO << numChildNodes << " nodes found with path " << fieldDefinition->xPath << ".  Taking value only of the "
               "first one.";
         }
         xalanc::XalanNode * fieldContainerNode = nodesForCurrentXPath.item(0);

         // Normally the node for the tag will be type ELEMENT_NODE and will not have a value in and of itself.
         // To get the "contents", we need to look at the value of the child node, which, for strings and numbers etc,
         // should be type TEXT_NODE (and name "#text").
         XQString fieldName{fieldContainerNode->getNodeName()};
         xalanc::XalanNodeList const * fieldContents = fieldContainerNode->getChildNodes();
         int numChildrenOfContainerNode = fieldContents->getLength();
         qDebug() <<
            Q_FUNC_INFO << "Node " << fieldDefinition->xPath << "(" << fieldName << ":" <<
            XmlRecord::XALAN_NODE_TYPES[fieldContainerNode->getNodeType()] << ") has " <<
            numChildrenOfContainerNode << " children";
         if (0 == numChildrenOfContainerNode) {
            qDebug() << Q_FUNC_INFO << "Empty!";
         } else {
            {
               //
               // The field is not a sub-record, so it must be something simple (a string, number, boolean or enum)
               //
               if (numChildrenOfContainerNode > 1) {
                  // This is probably a coding error, as it would mean the XML node had child nodes, rather than just
                  // text content, which should have already generated an error during XSD validation.
                  qWarning() <<
                     Q_FUNC_INFO << "Node " << fieldDefinition->xPath << " has " <<
                     numChildrenOfContainerNode << " children.  Taking value only of the first one.";
               }
               xalanc::XalanNode * valueNode = fieldContents->item(0);
               XQString value(valueNode->getNodeValue());
               qDebug() << Q_FUNC_INFO << "Value " << value;

               bool parsedValueOk = false;
               QVariant parsedValue;

               // A field should have a stringToEnum mapping if and only if it's of type Enum
               // Anything else is a coding error at the caller
               Q_ASSERT((XmlRecord::Enum == fieldDefinition->fieldType) != (nullptr == fieldDefinition->stringToEnum));

               switch(fieldDefinition->fieldType) {

                  case XmlRecord::Bool:
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
                           Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as could not be parsed as BOOLEAN";
                     }
                     break;

                  case XmlRecord::Int:
                     // QString's toInt method will report success/failure of parsing straight back into our flag
                     parsedValue.setValue(value.toInt(&parsedValueOk));
                     if (!parsedValueOk) {
                        // This is almost certainly a coding error, as we should have already validated the field via XSD
                        // parsing.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as could not be parsed as integer";
                     }
                     break;

                  case XmlRecord::UInt:
                     // QString's toUInt method will report success/failure of parsing straight back into our flag
                     parsedValue.setValue(value.toUInt(&parsedValueOk));
                     if (!parsedValueOk) {
                        // This is almost certainly a coding error, as we should have already validated the field via XSD
                        // parsing.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as could not be parsed as unsigned integer";
                     }
                     break;

                  case XmlRecord::Double:
                     // QString's toDouble method will report success/failure of parsing straight back into our flag
                     parsedValue.setValue(value.toDouble(&parsedValueOk));
                     if (!parsedValueOk) {
                        // This is almost certainly a coding error, as we should have already validated the field via XSD
                        // parsing.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as could not be parsed as decimal number (double)";
                     }
                     break;

                  case XmlRecord::Enum:
                     // It's definitely a coding error if there is no stringToEnum mapping for a field declared as Enum!
                     Q_ASSERT(nullptr != fieldDefinition->stringToEnum);
                     if (!fieldDefinition->stringToEnum->contains(value)) {
                        // This is probably a coding error as the XSD parsing should already have verified that the
                        // contents of the node are one of the expected values.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->recordName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as value not recognised";
                     } else {
                        parsedValue.setValue(fieldDefinition->stringToEnum->value(value));
                        parsedValueOk = true;
                     }
                     break;

                  // By default we assume it's a string
                  case XmlRecord::String:
                  default:
                     if (fieldDefinition->fieldType != XmlRecord::String) {
                        // This is almost certainly a coding error in this class as we should be able to parse all the
                        // types callers need us to.
                        qWarning() <<
                           Q_FUNC_INFO << "Treating " << this->recordName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as string because did not recognise requested parse type " << fieldDefinition->fieldType;
                     }
                     parsedValue.setValue(static_cast<QString>(value));
                     parsedValueOk = true;
                     break;
               }

               //
               // If we parsed the value OK, and a look-up map was supplied, store the value.  Likely our caller needs
               // access to at least some of the read in values for further validation or processing.
               //
               if (parsedValueOk && nullptr != fieldsRead.get()) {
                  // (It's a programming error if we already have a node of this name in the fieldsRead map.  Most probably
                  // either the caller didn't provide an empty map, or there was a duplicate entry in
                  // this->fieldDefinitions.)
                  //
                  Q_ASSERT(!fieldsRead->contains(fieldDefinition->xPath));

                  fieldsRead->insert(fieldDefinition->xPath, parsedValue);
               }

               //
               // What we do if we couldn't parse the value depends.  If it was a value that we didn't need to set on the
               // supplied Hop/Yeast/Recipe/Etc object, then we can just ignore the problem and carry on processing.  But,
               // if this was a field we were expecting to use, then it's a problem that we couldn't parse it and we should
               // bail.
               //
               if (!parsedValueOk && nullptr != fieldDefinition->propertyName) {
                  userMessage <<
                     "Could not parse " << this->recordName << " node " << fieldDefinition->xPath << "=" << value << " into " <<
                     fieldDefinition->propertyName;
                  return false;
               }

               //
               // So we've either parsed the value OK or we don't need it (or both)
               //
               // If we do need it, we now store the value
               //
               if (nullptr != fieldDefinition->propertyName) {
                  //
                  // It's a coding error if we're trying to store a simple field without somewhere to store it.  It
                  // should only be the root record that doesn't have a NamedEntity to populate, and, equally, the root
                  // record should not be configured to parse anything other than contained records.
                  //
                  Q_ASSERT(nullptr != this->namedEntity && "Trying to parse simple field on root record");
                  if (!this->namedEntity->setProperty(fieldDefinition->propertyName, parsedValue)) {
                     //
                     // It's also a coding error if we are trying to read and store a field that does not exist on the
                     // object we are loading (because we only try to store fields we (a) recognise and (b) are
                     // interested in).  Nonetheless, if asserts are disabled, we may be able to continue past this
                     // coding error by ignoring the current field.
                     //
                     Q_ASSERT(false && "Trying to update undeclared property");
                     qCritical() <<
                        Q_FUNC_INFO << "Trying to update undeclared property " << fieldDefinition->propertyName << " of " <<
                        this->namedEntity->metaObject()->className();
                  }
               }
            }
         }
      }
   }
   return true;
}

bool XmlRecord::normaliseAndStoreInDb(NamedEntity * containingEntity,
                                      QTextStream & userMessage,
                                      XmlRecordCount & stats) {
   if (nullptr != this->namedEntity) {
      if (this->instanceNamesAreUnique) {
         QString currentName = this->namedEntity->name();

         for (NamedEntity * matchingEntity = this->findByName(currentName);
            nullptr != matchingEntity;
            matchingEntity = this->findByName(currentName)) {

            qDebug() <<
               Q_FUNC_INFO << "Existing " << this->recordName << "named" << currentName << "was" <<
               ((nullptr == matchingEntity) ? "not" : "") << "found";

            XmlRecord::modifyClashingName(currentName);

            //
            // Now the for loop will search again with the new name
            //
            qDebug() << Q_FUNC_INFO << "Trying " << currentName;
         }

         this->namedEntity->setName(currentName);
      }

      // Now we're ready to store in the DB, something the NamedEntity knows how to make happen
      this->namedEntity->insertInDatabase();

      // Once we've stored the object, we no longer have to take responsibility for destroying it because its registry
      // (currently the Database singleton) will now own it.
      this->namedEntityRaiiContainer.release();

      stats.processedOk(this->recordName.toLower());
   }

   // Finally orchestrate storing any contained records
   return this->normaliseAndStoreChildRecordsInDb(userMessage, stats);
}


bool XmlRecord::normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
                                                  XmlRecordCount & stats) {
   for (auto ii = this->childRecords.begin(); ii != this->childRecords.end(); ++ii) {
      qDebug() << Q_FUNC_INFO << "Storing" << ii.key();
      if (!ii.value()->normaliseAndStoreInDb(this->namedEntity, userMessage, stats)) {
         return false;
      }
   }
   return true;
}


bool XmlRecord::loadChildRecords(xalanc::DOMSupport & domSupport,
                                 xalanc::NodeRefList & nodesForCurrentXPath,
                                 QTextStream & userMessage) {
   //
   // This is where we have one or more substantive records of a particular type inside the one we are
   // reading - eg some Hops inside a Recipe.  So we need to loop though these "child" records and read
   // each one in with an XmlRecord object of the relevant type.
   //
   // Note an advantage of using XPaths means we can just "see through" any grouping or containing nodes.
   // For instance, in BeerXML, inside a <RECIPE>...</RECIPE> record there will be a <HOPS>...</HOPS>
   // "record set" node containing the <HOP>...</HOP> record(s) for this recipe, but we can just say in our
   // this->fieldDefinitions that we want the "HOPS/HOP" nodes inside a "RECIPE" and thus skip straight to
   // having a list of all the <HOP>...</HOP> nodes without having to explicitly parse the <HOPS>...</HOPS>
   // node.
   //
   for (xalanc::NodeRefList::size_type ii = 0; ii < nodesForCurrentXPath.getLength(); ++ii) {
      //
      // It's a coding error if we don't recognise the type of node that we've been configured (via
      // this->fieldDefinitions) to read in.  Again, an advantage of using XPaths is that we just
      // automatically ignore nodes we're not looking for.  Eg, imagine, in a BeerXML file, there's the
      // following:
      //    <RECIPE>
      //    ...
      //       <HOPS>
      //          <FOO>...</FOO>
      //          <BAR>...</BAR>
      //          <HOP>...</HOP>
      //       </HOPS>...
      //    ...
      //    </RECIPE>
      // Requesting the HOPS/HOP subpath of RECIPE will not return FOO or BAR
      //
      xalanc::XalanNode * childRecordNode = nodesForCurrentXPath.item(ii);
      XQString childRecordName{childRecordNode->getNodeName()};
      Q_ASSERT(this->xmlCoding.isKnownXmlRecordType(childRecordName));

      std::shared_ptr<XmlRecord> xmlRecord = this->xmlCoding.getNewXmlRecord(childRecordName);
      this->childRecords.insert(childRecordName, xmlRecord);
      if (!xmlRecord->load(domSupport, childRecordNode, userMessage)) {
         return false;
      }
   }

   return true;
}


NamedEntity * XmlRecord::findByName(QString nameToFind) {
   // It's actually a coding error for this base class implementation to be called
   Q_ASSERT(false && "Base class should not be trying to store a NamedEntity itself!");
   return nullptr;
}


/**
 * \brief Given a name that is a duplicate of an existing one, modify it to a potential alternative.
 *        Callers should call this function as many times as necessary to find a non-clashing name.
 *
 *        Eg if the supplied clashing name is "Oatmeal Stout", we'll try adding a "duplicate number" in brackets to
 *        the end of the name, ie amending it to "Oatmeal Stout (1)".  If the caller determines that that clashes too
 *        then the next call (supplying "Oatmeal Stout (1)") will make us modify the name to "Oatmeal Stout (2)" (and
 *        NOT "Oatmeal Stout (1) (1)"!).
 *
 * \param candidateName The name that we should attempt to modify.  (Modification is done in place.)
 */
void XmlRecord::modifyClashingName(QString & candidateName) {
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
   int positionOfMatch = nameNumberMatcher.indexIn(candidateName);
   if (positionOfMatch > -1) {
      // There's already some integer in brackets at the end of the name, extract it, add one, and truncate the
      // name.
      duplicateNumber = nameNumberMatcher.cap(1).toInt() + 1;
      candidateName.truncate(positionOfMatch);
   }
   candidateName += QString(" (%1)").arg(duplicateNumber);
   return;
}
