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

#include <QDate>
#include <QDebug>

#include <xalanc/XalanDOM/XalanNodeList.hpp>
#include <xalanc/XPath/NodeRefList.hpp>
#include <xalanc/XPath/XPathEvaluator.hpp>

#include "xml/XmlCoding.h"

//
// Variables and constant definitions that we need only in this file
//
namespace {
   // See https://apache.github.io/xalan-c/api/XalanNode_8hpp_source.html for possible indexes into this array
   char const * const XALAN_NODE_TYPES[] {
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
}

XmlRecord::XmlRecord(XmlCoding const & xmlCoding,
                     FieldDefinitions const & fieldDefinitions) :
   xmlCoding{xmlCoding},
   fieldDefinitions{fieldDefinitions},
   namedEntityClassName{},
   namedParameterBundle{},
   namedEntity{nullptr},
   namedEntityRaiiContainer{nullptr},
   includeInStats{true},
   childRecords{} {
   return;
}

NamedParameterBundle const & XmlRecord::getNamedParameterBundle() const {
   return this->namedParameterBundle;
}

NamedEntity * XmlRecord::getNamedEntity() const {
   return this->namedEntity;
}


bool XmlRecord::load(xalanc::DOMSupport & domSupport,
                     xalanc::XalanNode * rootNodeOfRecord,
                     QTextStream & userMessage) {
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
         if (!this->loadChildRecords(domSupport, fieldDefinition, nodesForCurrentXPath, userMessage)) {
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
            XALAN_NODE_TYPES[fieldContainerNode->getNodeType()] << ") has " <<
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
                     // Unlike other XML documents, boolean fields in BeerXML are caps, so we have to accommodate that
                     if (value.toLower() == "true") {
                        parsedValue.setValue(true);
                        parsedValueOk = true;
                     } else if (value.toLower() == "false") {
                        parsedValue.setValue(true);
                        parsedValueOk = true;
                     } else {
                        // This is almost certainly a coding error, as we should have already validated that the field
                        // via XSD parsing.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" <<
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
                           Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" <<
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
                           Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" <<
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
                           Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as could not be parsed as decimal number (double)";
                     }
                     break;

                  case XmlRecord::Date:
                     {
                        //
                        // Extra braces here as we have a variable (date) that is only used in this case of the switch,
                        // so we need to restrict its scope, otherwise the compiler will complain about the variable
                        // initialisation being "jumped over" in the other case labels.
                        //
                        // Dates are a bit annoying because, in some cases, fields are not restricted to using the One
                        // True Date Format™ (aka ISO 8601).  Eg, in the BeerXML 1.0 standard, for the DATE field of a
                        // Recipe, it merely says 'Date brewed in a easily recognizable format such as “3 Dec 04”', yet
                        // internally we want to store this as a date rather than just a text field.
                        //
                        // So, we make several attempts to parse a date, using various different "standard" encodings.
                        // There is a risk that certain formats are ambiguous - eg 01/04/2021 is 4 January 2021 in
                        // the USA, but 1 April 2021 in most of the rest of the world (except the enlightened countries
                        // that use the One True Date Format) - but there is little we can do about this.
                        //
                        // Start by trying ISO 8601, which is the most logical format :-)
                        //
                        QDate date = QDate::fromString(value, Qt::ISODate);
                        parsedValueOk = date.isValid();
                        if (!parsedValueOk) {
                           // If not ISO 8601, try RFC 2822 Internet Message Format, which is horrible because it
                           // assumes everyone speaks English, but (a) widely used and (b) unambiguous
                           date = QDate::fromString(value, Qt::RFC2822Date);
                           parsedValueOk = date.isValid();
                        }
                        if (!parsedValueOk) {
                           // Next we'll try Qt's "default" date format, which is good for display but not for file
                           // interchange, as it's locale-specific
                           date = QDate::fromString(value, Qt::TextDate);
                           parsedValueOk = date.isValid();
                        }
                        if (!parsedValueOk) {
                           // Now we're rolling our own formats.  See https://doc.qt.io/qt-5/qdate.html for details of
                           // the codes in the format strings.
                           //
                           // Try USA / Philippines numeric format next, though NB this could mis-parse some
                           // non-USA-format dates per example above.  (Historically we assumed USA format dates before
                           // non-USA-format ones, so we're retaining existing behaviour by trying things in this order.)
                           date = QDate::fromString(value, "M/d/yyyy");
                           parsedValueOk = date.isValid();
                        }
                        if (!parsedValueOk) {
                           // Now try the numeric version that is widely used outside the USA & the Philippines
                           date = QDate::fromString(value, "d/M/yyyy");
                           parsedValueOk = date.isValid();
                        }
                        if (!parsedValueOk) {
                           // Now try the numeric version that is widely used outside the USA & the Philippines
                           date = QDate::fromString(value, "d/M/yyyy");
                           parsedValueOk = date.isValid();
                        }
                        if (!parsedValueOk) {
                           // Now try the example "easily recognizable" format from the BeerXML 1.0 standard.
                           //
                           // Of course, this is a horrible format because it is not Y2K compliant.  So the actual date
                           // we store may be out by 100 years.  Hopefully the user will notice and correct this, and
                           // then if we export we can use a non-ambiguous format.
                           date = QDate::fromString(value, "d MMM yy");
                           parsedValueOk = date.isValid();
                        }
                        // .:TBD:. Maybe we could try some more formats here

                        parsedValue.setValue(date);
                     }
                     if (!parsedValueOk) {
                        // This is almost certainly a coding error, as we should have already validated the field via XSD
                        // parsing.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as could not be parsed as ISO 8601 date";
                     }
                     break;

                  case XmlRecord::Enum:
                     // It's definitely a coding error if there is no stringToEnum mapping for a field declared as Enum!
                     Q_ASSERT(nullptr != fieldDefinition->stringToEnum);
                     if (!fieldDefinition->stringToEnum->contains(value)) {
                        // This is probably a coding error as the XSD parsing should already have verified that the
                        // contents of the node are one of the expected values.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" <<
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
                           Q_FUNC_INFO << "Treating " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" <<
                           value << " as string because did not recognise requested parse type " << fieldDefinition->fieldType;
                     }
                     parsedValue.setValue(static_cast<QString>(value));
                     parsedValueOk = true;
                     break;
               }

               //
               // What we do if we couldn't parse the value depends.  If it was a value that we didn't need to set on the
               // supplied Hop/Yeast/Recipe/Etc object, then we can just ignore the problem and carry on processing.  But,
               // if this was a field we were expecting to use, then it's a problem that we couldn't parse it and we should
               // bail.
               //
               if (!parsedValueOk && nullptr != fieldDefinition->propertyName) {
                  userMessage <<
                     "Could not parse " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" << value << " into " <<
                     fieldDefinition->propertyName;
                  return false;
               }

               //
               // So we've either parsed the value OK or we don't need it (or both)
               //
               // If we do need it, we now store the value
               //
               if (nullptr != fieldDefinition->propertyName) {
                  this->namedParameterBundle.insert(fieldDefinition->propertyName, parsedValue);

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

XmlRecord::ProcessingResult XmlRecord::normaliseAndStoreInDb(NamedEntity * containingEntity,
                                                             QTextStream & userMessage,
                                                             XmlRecordCount & stats) {
   if (nullptr != this->namedEntity) {
      qDebug() <<
         Q_FUNC_INFO << "Normalise and store " << this->namedEntityClassName << ": " << this->namedEntity->name();

      // If the object we are reading in is a duplicate of something we already have (and duplicates are not allowed)
      // then skip over this record (and any records it contains).  (This is _not_ an error, so we return true not
      // false in this event.)
      if (this->isDuplicate()) {
         qDebug() <<
            Q_FUNC_INFO << "Duplicate " << this->namedEntityClassName << (this->includeInStats ? " will" : " won't") <<
            " be included in stats";
         if (this->includeInStats) {
            stats.skipped(this->namedEntityClassName.toLower());
         }
         return XmlRecord::FoundDuplicate;
      }

      this->normaliseName();

      // Some classes of object are owned by their containing entity and can't sensibly be saved without knowing what it
      // is.  Subclasses of XmlRecord will override setContainingEntity() to pass the info in if it is needed (or ignore
      // it if not).
      this->setContainingEntity(containingEntity);

      // Now we're ready to store in the DB, something the NamedEntity knows how to make happen
      this->namedEntity->insertInDatabase();

      // Once we've stored the object, we no longer have to take responsibility for destroying it because its registry
      // (currently the Database singleton) will now own it.
      this->namedEntityRaiiContainer.release();

      if (this->includeInStats) {
         stats.processedOk(this->namedEntityClassName.toLower());
      }
   }

   // Finally orchestrate storing any contained records
   if (this->normaliseAndStoreChildRecordsInDb(userMessage, stats)) {
      return XmlRecord::Succeeded;
   }

   // If we reach here, it means there was a problem with one of our child records.  We've already stored our
   // NamedEntity record in the DB, so we need to try to undo that by deleting it.  It should be the case that this
   // deletion will also take care of deleting any owned child records that have already been stored.  (Eg if this is
   // a Mash, and we stored it and 2 MashSteps before hitting an error on the 3rd MashStep, then deleting the Mash
   // from the DB should also result in those 2 stored MashSteps getting deleted from the DB.)
   this->namedEntity->removeFromDatabase();

   // Now we removed the object from the database, we're responsible for calling its destructor
   this->namedEntityRaiiContainer.reset(this->namedEntity);

   return XmlRecord::Failed;
}


bool XmlRecord::normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
                                                  XmlRecordCount & stats) {
   for (auto ii = this->childRecords.begin(); ii != this->childRecords.end(); ++ii) {
      qDebug() << Q_FUNC_INFO << "Storing" << ii.key();
      if (XmlRecord::Failed == ii.value().second->normaliseAndStoreInDb(this->namedEntity, userMessage, stats)) {
         return false;
      }
      // Now we've stored the child record (or recognised it as a duplicate of one we already hold), we want to link it
      // (or as the case may be the record it's a duplicate of) to the parent.  If this is possible via a property (eg
      // the style on a recipe), then we can just do that here.  Otherwise the work needs to be done in the appropriate
      // subclass of XmlNamedEntityRecord
      char const * const propertyName = ii.value().first;
      if (nullptr != propertyName) {
         // It's a coding error if we had a property defined for a record that's not trying to populate a NamedEntity
         // (ie for the root record).
         Q_ASSERT(nullptr != this->namedEntity);
         // It's a coding error if we're trying to set a non-existent property on the NamedEntity subclass for this
         // record.
         Q_ASSERT(this->namedEntity->metaObject()->indexOfProperty(propertyName) > 0);
         // It's a coding error if we can't create a valid QVariant from a pointer to class we are trying to "set"
         Q_ASSERT(QVariant::fromValue(ii.value().second->namedEntity).isValid());

         qDebug() <<
            Q_FUNC_INFO << "Setting" << propertyName << "property (type = " <<
            this->namedEntity->metaObject()->property(
               this->namedEntity->metaObject()->indexOfProperty(propertyName)
            ).typeName() << ") on" << this->namedEntityClassName << "object";
         this->namedEntity->setProperty(propertyName,
                                        QVariant::fromValue(ii.value().second->namedEntity));
      }
   }
   return true;
}


bool XmlRecord::loadChildRecords(xalanc::DOMSupport & domSupport,
                                 XmlRecord::FieldDefinition const * fieldDefinition,
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
      this->childRecords.insert(xmlRecord->namedEntityClassName,
                                XmlRecord::ChildRecord(fieldDefinition->propertyName, xmlRecord));
      if (!xmlRecord->load(domSupport, childRecordNode, userMessage)) {
         return false;
      }
   }

   return true;
}


bool XmlRecord::isDuplicate() {
   // Base class does not have a NamedEntity so nothing to check
   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
   // NamedEntity, and subclasses that do have one should override this function.
   Q_ASSERT(false && "Trying to check for duplicate NamedEntity when there is none");
   return false;
}

void XmlRecord::normaliseName() {
   // Base class does not have a NamedEntity so nothing to normalise
   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
   // NamedEntity, and subclasses that do have one should override this function.
   Q_ASSERT(false && "Trying to normalise name of NamedEntity when there is none");
   return;
}

void XmlRecord::setContainingEntity(NamedEntity * containingEntity) {
   // Base class does not have a NamedEntity or a container, so nothing to do
   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
   // NamedEntity, and subclasses that do have one should override this function.
   Q_ASSERT(false && "Trying to set containing entity when there is none");
   return;
}


void XmlRecord::modifyClashingName(QString & candidateName) {
   //
   // First, see whether there's already a (n) (ie "(1)", "(2)" etc) at the end of the name (with or without
   // space(s) preceding the left bracket.  If so, we want to replace this with " (n+1)".  If not, we try " (1)".
   //
   int duplicateNumber = 1;
   QRegExp const & nameNumberMatcher = NamedEntity::getDuplicateNameNumberMatcher();
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
