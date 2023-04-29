/*
 * xml/XmlRecord.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2023
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
#include <QXmlStreamWriter>

#include <xalanc/XalanDOM/XalanNodeList.hpp>
#include <xalanc/XPath/NodeRefList.hpp>
#include <xalanc/XPath/XPathEvaluator.hpp>
#include <xalanc/XalanDOM/XalanNamedNodeMap.hpp>

#include "xml/XmlCoding.h"
#include "utils/OptionalHelpers.h"

//
// Variables and constant definitions that we need only in this file
//
namespace {
   // See https://apache.github.io/xalan-c/api/XalanNode_8hpp_source.html for possible indexes into this array
   char const * const XALAN_NODE_TYPES[] {
      "UNKNOWN_NODE",                 // = 0,
      "ELEMENT_NODE",                 // = 1,
      "ATTRIBUTE_NODE",               // = 2,
      "TEXT_NODE",                    // = 3,
      "CDATA_SECTION_NODE",           // = 4,
      "ENTITY_REFERENCE_NODE",        // = 5,
      "ENTITY_NODE",                  // = 6,
      "PROCESSING_INSTRUCTION_NODE",  // = 7,
      "COMMENT_NODE",                 // = 8,
      "DOCUMENT_NODE",                // = 9,
      "DOCUMENT_TYPE_NODE",           // = 10,
      "DOCUMENT_FRAGMENT_NODE",       // = 11,
      "NOTATION_NODE",                // = 12
      "UNRECOGNISED!"
   };

   /**
    * \brief Helper function for writing multiple indents
    */
   void writeIndents(QTextStream & out,
                     int indentLevel,
                     char const * const indentString) {
      for (int ii = 0; ii < indentLevel; ++ii) {
         out << indentString;
      };
      return;
   }
}

XmlRecord::FieldDefinition::FieldDefinition(FieldType           fieldType,
                                            XQString            xPath,
                                            BtStringConst const & propertyName,
                                            EnumStringMapping const * enumMapping) :
   fieldType{fieldType},
   xPath{xPath},
   propertyName{propertyName},
   enumMapping{enumMapping} {
   return;
}

XmlRecord::XmlRecord(QString const & recordName,
                     XmlCoding const & xmlCoding,
                     FieldDefinitions const & fieldDefinitions,
                     TypeLookup       const * const typeLookup,
                     QString          const & namedEntityClassName) :
   recordName{recordName},
   xmlCoding{xmlCoding},
   fieldDefinitions{fieldDefinitions},
   typeLookup{typeLookup},
   namedEntityClassName{namedEntityClassName},
   namedParameterBundle{NamedParameterBundle::NotStrict},
   namedEntity{nullptr},
   includeInStats{true},
   childRecords{} {
   return;
}

XmlRecord::~XmlRecord() = default;

QString XmlRecord::getRecordName() const {
   return this->recordName;
}

NamedParameterBundle const & XmlRecord::getNamedParameterBundle() const {
   return this->namedParameterBundle;
}

std::shared_ptr<NamedEntity> XmlRecord::getNamedEntity() const {
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
   for (auto fieldDefinition = this->fieldDefinitions.cbegin();
        fieldDefinition < this->fieldDefinitions.cend();
        ++fieldDefinition) {
      //
      // NB: If we don't find a node, there's nothing for us to do.  The XSD parsing should already flagged up an error
      // if there are missing _required_ fields or if string fields that are present are not allowed to be blank.  (See
      // comments in BeerXml.xsd for why it is, in practice, plausible and acceptable for some "required" text fields
      // to be empty/blank.)
      //
      // Equally, although we only look for nodes we know about, some of these we won't use.  If there is no property
      // name in our field definition then it's a field we neither read nor write.  We'll parse it but we won't try to
      // pass it to the object we're creating.  But there are some fields that are "write only", such as IBU on Recipe.
      // These have a property name in the field definition, so they will be written out in XmlRecord::toXml, but the
      // relevant object constructor ignores them when they appear in a NamedParameterBundle.  (In the case of IBU on
      // Recipe, this is because it is a calculated value.  It is helpful to some users to export it in the XML, but
      // there is no point trying to read it in from XML as the value would get overwritten by our own calculated one.)
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
      if (XmlRecord::FieldType::RecordSimple == fieldDefinition->fieldType ||
          XmlRecord::FieldType::RecordComplex == fieldDefinition->fieldType) {
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
               Q_FUNC_INFO << numChildNodes << " nodes found with path " << fieldDefinition->xPath << ".  Taking value "
               "only of the first one.";
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

               // A field should have an enumMapping if and only if it's of type Enum
               // Anything else is a coding error at the caller
               Q_ASSERT((XmlRecord::FieldType::Enum == fieldDefinition->fieldType) !=
                        (nullptr == fieldDefinition->enumMapping));

               //
               // We're going to need to know whether this field is "optional" in our internal data model.  If it is,
               // then, for whatever underlying type T it is, we need the parsedValue QVariant to hold std::optional<T>
               // instead of just T.
               //
               // (Note we can't do this mapping inside NamedParameterBundle, as we don't have the type information
               // there.  We could conceivably do it in the constructors that take a NamedParameterBundle parameter, but
               // I think it gets messy to have different types there than on the QProperty setters.  It's not much
               // overhead to do things here IMHO.)
               //
               // Note that:
               //    - propertyName is not actually a property name when fieldType is RequiredConstant
               //    - when propertyName is not set, there is nothing to look up (because this is a field we don't
               //      support, usually an "Extension tag")
               //
               bool const propertyIsOptional {
                  (fieldDefinition->fieldType == XmlRecord::FieldType::RequiredConstant ||
                   fieldDefinition->propertyName.isNull()) ?
                     false : this->typeLookup->isOptional(fieldDefinition->propertyName)
               };

               switch (fieldDefinition->fieldType) {

                  case XmlRecord::FieldType::Bool:
                     // Unlike other XML documents, boolean fields in BeerXML are caps, so we have to accommodate that
                     if (value.toLower() == "true") {
                        parsedValue = Optional::variantFromRaw(true, propertyIsOptional);
                        parsedValueOk = true;
                     } else if (value.toLower() == "false") {
                        parsedValue = Optional::variantFromRaw(false, propertyIsOptional);
                        parsedValueOk = true;
                     } else {
                        // This is almost certainly a coding error, as we should have already validated that the field
                        // via XSD parsing.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " <<
                           fieldDefinition->xPath << "=" << value << " as could not be parsed as BOOLEAN";
                     }
                     break;

                  case XmlRecord::FieldType::Int:
                     {
                        // QString's toInt method will report success/failure of parsing straight back into our flag
                        auto const rawValue = value.toInt(&parsedValueOk);
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                        if (!parsedValueOk) {
                           // This is almost certainly a coding error, as we should have already validated the field via
                           // XSD parsing.
                           qWarning() <<
                              Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " <<
                              fieldDefinition->xPath << "=" << value << " as could not be parsed as integer";
                        }
                     }
                     break;

                  case XmlRecord::FieldType::UInt:
                     {
                        // QString's toUInt method will report success/failure of parsing straight back into our flag
                        auto const rawValue = value.toUInt(&parsedValueOk);
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                        if (!parsedValueOk) {
                           // This is almost certainly a coding error, as we should have already validated the field via
                           // XSD parsing.
                           qWarning() <<
                              Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " <<
                              fieldDefinition->xPath << "=" << value << " as could not be parsed as unsigned integer";
                        }
                     }
                     break;

                  case XmlRecord::FieldType::Double:
                     {
                        // QString's toDouble method will report success/failure of parsing straight back into our flag
                        auto rawValue = value.toDouble(&parsedValueOk);
                        if (!parsedValueOk) {
                           //
                           // Although it is not explicitly stated in the BeerXML 1.0 standard, it is clear from the
                           // sample files downloadable from www.beerxml.com that some "ignorable" percentage and decimal
                           // values can be specified as "-".  I haven't found a straightforward way to filter or
                           // transform these during XSD validation.  Nor, as yet, do I know whether it's possible from a
                           // xalanc::XalanNode to get back to the Post-Schema-Validation Infoset (PSVI) information in
                           // Xerces that might allow us to examine the XSD rules applied to the current node.
                           //
                           // For the moment, we assume that, if a "-" didn't get filtered out by XSD then it's allowed
                           // and should be interpreted as NULL, which therefore means we store 0.0.
                           //
                           qInfo() <<
                              Q_FUNC_INFO << "Treating " << this->namedEntityClassName << " node " <<
                              fieldDefinition->xPath << "=" << value << " as 0.0";
                           parsedValueOk = true;
                           rawValue = 0.0;
                        }
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                     }
                     break;

                  case XmlRecord::FieldType::Date:
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
                           // non-USA-format ones, so we're retaining existing behaviour by trying things in this
                           // order.)
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
                        parsedValue = Optional::variantFromRaw(date, propertyIsOptional);
                     }
                     if (!parsedValueOk) {
                        // This is almost certainly a coding error, as we should have already validated the field via
                        // XSD parsing.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " <<
                           fieldDefinition->xPath << "=" << value << " as could not be parsed as ISO 8601 date";
                     }
                     break;

                  case XmlRecord::FieldType::Enum:
                     // It's definitely a coding error if there is no stringToEnum mapping for a field declared as Enum!
                     Q_ASSERT(nullptr != fieldDefinition->enumMapping);
                     {
                        auto match = fieldDefinition->enumMapping->stringToEnumAsInt(value);
                        if (!match) {
                           // This is probably a coding error as the XSD parsing should already have verified that the
                           // contents of the node are one of the expected values.
                           qWarning() <<
                              Q_FUNC_INFO << "Ignoring " << this->namedEntityClassName << " node " <<
                              fieldDefinition->xPath << "=" << value << " as value not recognised";
                        } else {
                           auto const rawValue = match.value();
                           parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                           parsedValueOk = true;
                        }
                     }
                     break;

                  case XmlRecord::FieldType::RequiredConstant:
                     //
                     // This is a field that is required to be in the XML, but whose value we don't need (and for which
                     // we always write a constant value on output).  At the moment it's only needed for the VERSION tag
                     // in BeerXML.
                     //
                     // Note that, because we abuse the propertyName field to hold the default value (ie what we write
                     // out), we can't carry on to normal processing below.  So jump straight to processing the next
                     // node in the loop (via continue).
                     //
                     qDebug() <<
                        Q_FUNC_INFO << "Skipping " << this->namedEntityClassName << " node " <<
                        fieldDefinition->xPath << "=" << value << "(" << fieldDefinition->propertyName <<
                        ") as not useful";
                     continue; // NB: _NOT_break here.  We want to jump straight to the next run through the for loop.

                  // By default we assume it's a string
                  case XmlRecord::FieldType::String:
                  default:
                     {
                        if (fieldDefinition->fieldType != XmlRecord::FieldType::String) {
                           // This is almost certainly a coding error in this class as we should be able to parse all the
                           // types callers need us to.
                           qWarning() <<
                              Q_FUNC_INFO << "Treating " << this->namedEntityClassName << " node " <<
                              fieldDefinition->xPath << "=" << value << " as string because did not recognise requested "
                              "parse type " << static_cast<int>(fieldDefinition->fieldType);
                        }
                        auto const rawValue = static_cast<QString>(value);
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                        parsedValueOk = true;
                     }
                     break;
               }

               //
               // What we do if we couldn't parse the value depends.  If it was a value that we didn't need to set on
               // the supplied Hop/Yeast/Recipe/Etc object, then we can just ignore the problem and carry on processing.
               // But, if this was a field we were expecting to use, then it's a problem that we couldn't parse it and
               // we should bail.
               //
               if (!parsedValueOk && !fieldDefinition->propertyName.isNull()) {
                  userMessage <<
                     "Could not parse " << this->namedEntityClassName << " node " << fieldDefinition->xPath << "=" <<
                     value << " into " << fieldDefinition->propertyName;
                  return false;
               }

               //
               // So we've either parsed the value OK or we don't need it (or both)
               //
               // If we do need it, we now store the value
               //
               if (!fieldDefinition->propertyName.isNull()) {
                  this->namedParameterBundle.insert(fieldDefinition->propertyName, parsedValue);
               }
            }
         }
      }
   }

   //
   // For everything but the root record, we now construct a suitable object (Hop, Recipe, etc) from the
   // NamedParameterBundle (which will be empty for the root record).
   //
   if (!this->namedParameterBundle.isEmpty()) {
      this->constructNamedEntity();
   }

   return true;
}

void XmlRecord::constructNamedEntity() {
   // Base class does not have a NamedEntity or a container, so nothing to do
   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
   // NamedEntity, and subclasses that do have one should override this function.
   Q_ASSERT(false && "Trying to construct named entity for base record");
   return;
}

int XmlRecord::storeNamedEntityInDb() {
   Q_ASSERT(false && "Trying to store named entity for base record");
   return -1;
}

void XmlRecord::deleteNamedEntityFromDb() {
   Q_ASSERT(false && "Trying to delete named entity for base record");
   return;
}

XmlRecord::ProcessingResult XmlRecord::normaliseAndStoreInDb(std::shared_ptr<NamedEntity> containingEntity,
                                                             QTextStream & userMessage,
                                                             ImportRecordCount & stats) {
   if (this->namedEntity) {
      qDebug() <<
         Q_FUNC_INFO << "Normalise and store " << this->namedEntityClassName << "(" <<
         this->namedEntity->metaObject()->className() << "):" << this->namedEntity->name();

      //
      // If the object we are reading in is a duplicate of something we already have (and duplicates are not allowed)
      // then skip over this record (and any records it contains).  (This is _not_ an error, so we return true not
      // false in this event.)
      //
      // Note, however, that some objects -- in particular those such as Recipe that contain other objects -- need
      // to be further along in their construction (ie have had all their contained objects added) before we can
      // determine whether they are duplicates.  This is why we check again, after storing in the DB, below.
      //
      if (this->isDuplicate()) {
         qDebug() <<
            Q_FUNC_INFO << "(Early found) duplicate" << this->namedEntityClassName <<
            (this->includeInStats ? " will" : " won't") << " be included in stats";
         if (this->includeInStats) {
            stats.skipped(this->namedEntityClassName.toLower());
         }
         return XmlRecord::ProcessingResult::FoundDuplicate;
      }

      this->normaliseName();

      // Some classes of object are owned by their containing entity and can't sensibly be saved without knowing what it
      // is.  Subclasses of XmlRecord will override setContainingEntity() to pass the info in if it is needed (or ignore
      // it if not).
      this->setContainingEntity(containingEntity);

      // Now we're ready to store in the DB
      int id = this->storeNamedEntityInDb();
      if (id <= 0) {
         userMessage << "Error storing" << this->namedEntity->metaObject()->className() <<
         "in database.  See logs for more details";
         return XmlRecord::ProcessingResult::Failed;
      }
   }

   XmlRecord::ProcessingResult processingResult;

   //
   // Finally (well, nearly) orchestrate storing any contained records
   //
   // Note, of course, that this still needs to be done, even if nullptr == this->namedEntity, because that just means
   // we're processing the root node.
   //
   if (this->normaliseAndStoreChildRecordsInDb(userMessage, stats)) {
      //
      // Now all the processing succeeded, we do that final duplicate check for any complex object such as Recipe that
      // had to be fully constructed before we could meaningfully check whether it's the same as something we already
      // have in the object store.
      //
      if (nullptr == this->namedEntity.get()) {
         // Child records OK and no duplicate check needed (root record), which also means no further processing
         // required
         return XmlRecord::ProcessingResult::Succeeded;
      }
      processingResult = this->isDuplicate() ? XmlRecord::ProcessingResult::FoundDuplicate :
                                               XmlRecord::ProcessingResult::Succeeded;
   } else {
      // There was a problem with one of our child records
      processingResult = XmlRecord::ProcessingResult::Failed;
   }

   if (nullptr != this->namedEntity.get()) {
      //
      // We potentially do stats for everything except failure
      //
      if (XmlRecord::ProcessingResult::FoundDuplicate == processingResult) {
         qDebug() <<
            Q_FUNC_INFO << "(Late found) duplicate" << this->namedEntityClassName <<
            (this->includeInStats ? " will" : " won't") << " be included in stats";
         if (this->includeInStats) {
            stats.skipped(this->namedEntityClassName.toLower());
         }
      } else if (XmlRecord::ProcessingResult::Succeeded == processingResult && this->includeInStats) {
         stats.processedOk(this->namedEntityClassName.toLower());
      }

      //
      // Clean-up
      //
      if (XmlRecord::ProcessingResult::FoundDuplicate == processingResult ||
          XmlRecord::ProcessingResult::Failed == processingResult) {
         //
         // If we reach here, it means either there was a problem with one of our child records or we ourselves are a
         // late-detected duplicate.  We've already stored our NamedEntity record in the DB, so we need to try to undo
         // that by deleting it.  It is the responsibility of each NamedEntity subclass to take care of deleting any
         // owned stored objects, via the virtual member function NamedEntity::hardDeleteOwnedEntities().  So we don't
         // have to worry about child records that have already been stored.  (Eg if this is a Mash, and we stored it
         // and 2 MashSteps before hitting an error on the 3rd MashStep, then deleting the Mash from the DB will also
         // result in those 2 stored MashSteps getting deleted from the DB.)
         //
         qDebug() <<
            Q_FUNC_INFO << "Deleting stored" << this->namedEntityClassName << "as" <<
            (XmlRecord::ProcessingResult::FoundDuplicate == processingResult ? "duplicate" : "failed to read all child records");
         this->deleteNamedEntityFromDb();
      }
   }

   return processingResult;
}


bool XmlRecord::normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
                                                  ImportRecordCount & stats) {
   //
   // We are assuming it does not matter which order different children are processed in.
   //
   // Where there are several children of the same type, we need to process them in the same order as they were read in
   // from the XML document because, in some cases, this order matters.  In particular, in BeerXML, the Mash Steps
   // inside a Mash (or rather MASH_STEP tags inside a MASH_STEPS tag inside a MASH tag) are stored in order without any
   // other means of identifying order.
   //
   // So it's simplest just to process all the child records in the order they were read out of the XML document.  This
   // is the advantage of storing things in a list such as QVector.  (Alternatives such as QMultiHash iterate through
   // items that share the same key in the opposite order to which they were inserted and don't offer STL reverse
   // iterators, so going backwards would be a bit clunky.)
   //
   for (auto ii = this->childRecords.begin(); ii != this->childRecords.end(); ++ii) {
      qDebug() <<
         Q_FUNC_INFO << "Storing" << ii->xmlRecord->namedEntityClassName << "child of" << this->namedEntityClassName;
      if (XmlRecord::ProcessingResult::Failed ==
         ii->xmlRecord->normaliseAndStoreInDb(this->namedEntity, userMessage, stats)) {
         return false;
      }
      //
      // Now we've stored the child record (or recognised it as a duplicate of one we already hold), we want to link it
      // (or as the case may be the record it's a duplicate of) to the parent.  If this is possible via a property (eg
      // the style on a recipe), then we can just do that here.  Otherwise the work needs to be done in the appropriate
      // subclass of XmlNamedEntityRecord.
      //
      // We can't use the presence or absence of a property name to determine whether the child record can be set via
      // a property because some properties are read-only (and need to be present in the FieldDefinition for export to
      // XML to work).  Instead we distinguish between two types of records: RecordSimple, which can be set via a
      // property, and RecordComplex, which can't.
      //
      if (XmlRecord::FieldType::RecordSimple == ii->fieldDefinition->fieldType) {
         char const * const propertyName = *ii->fieldDefinition->propertyName;
         Q_ASSERT(nullptr != propertyName);
         // It's a coding error if we had a property defined for a record that's not trying to populate a NamedEntity
         // (ie for the root record).
         Q_ASSERT(nullptr != this->namedEntity.get());
         // It's a coding error if we're trying to set a non-existent property on the NamedEntity subclass for this
         // record.
         QMetaObject const * metaObject = this->namedEntity->metaObject();
         int propertyIndex = metaObject->indexOfProperty(propertyName);
         Q_ASSERT(propertyIndex >= 0);
         QMetaProperty metaProperty = metaObject->property(propertyIndex);
         Q_ASSERT(metaProperty.isWritable());
         // It's a coding error if we can't create a valid QVariant from a pointer to class we are trying to "set"
         Q_ASSERT(QVariant::fromValue(ii->xmlRecord->namedEntity.get()).isValid());

         qDebug() <<
            Q_FUNC_INFO << "Setting" << propertyName << "property (type = " <<
            this->namedEntity->metaObject()->property(
               this->namedEntity->metaObject()->indexOfProperty(propertyName)
            ).typeName() << ") on" << this->namedEntityClassName << "object";
         this->namedEntity->setProperty(propertyName,
                                        QVariant::fromValue(ii->xmlRecord->namedEntity.get()));
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
      this->childRecords.append(XmlRecord::ChildRecord{fieldDefinition, xmlRecord});
      //
      // The return value of xalanc::XalanNode::getIndex() doesn't have an instantly obvious direct meaning, but AFAICT
      // higher values are for nodes that were later in the input file, so useful to log.
      //
      qDebug() <<
         Q_FUNC_INFO << "Loading child record" << childRecordName << "with index" << childRecordNode->getIndex();
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

void XmlRecord::setContainingEntity([[maybe_unused]] std::shared_ptr<NamedEntity> containingEntity) {
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

void XmlRecord::toXml(NamedEntity const & namedEntityToExport,
                      QTextStream & out,
                      int indentLevel,
                      char const * const indentString) const {
   // Callers are not allowed to supply null indent string
   Q_ASSERT(nullptr != indentString);
   qDebug() <<
      Q_FUNC_INFO << "Exporting XML for" << namedEntityToExport.metaObject()->className() << "#" <<
      namedEntityToExport.key();
   writeIndents(out, indentLevel, indentString);
   out << "<" << this->recordName << ">\n";

   // For the moment, we are constructing XML output without using Xerces (or similar), on the grounds that, in this
   // direction (ie to XML rather than from XML), it's a pretty simple algorithm and we don't need to validate anything
   // (because we assume that our own data is valid).

   // BeerXML doesn't care about field order, so we don't either (though it would be relatively small additional work
   // to control field order precisely).
   for (auto & fieldDefinition : this->fieldDefinitions) {
      // If there isn't a property name that means this is not a field we support so there's nothing to write out.
      if (fieldDefinition.propertyName.isNull()) {
         // At the moment at least, we support all XmlRecord::RecordSimple and XmlRecord::RecordComplex fields, so it's
         // a coding error if one of them does not have a property name.
         Q_ASSERT(XmlRecord::FieldType::RecordSimple  != fieldDefinition.fieldType);
         Q_ASSERT(XmlRecord::FieldType::RecordComplex != fieldDefinition.fieldType);
         continue;
      }

      // Nested record fields are of two types.  XmlRecord::RecordSimple can be handled generically.
      // XmlRecord::RecordComplex need to be handled in part by subclasses.
      if (XmlRecord::FieldType::RecordSimple  == fieldDefinition.fieldType ||
          XmlRecord::FieldType::RecordComplex == fieldDefinition.fieldType) {
         //
         // Some of the work is generic, so we do it here.  In particular, we can work out what tags are needed to
         // contain the record (from the XPath, if any, prior to the last slash), but also what type of XmlRecord(s) we
         // will need by looking at the end of the XPath for this field.
         //
         // (In BeerXML, these contained XPaths are only 1-2 elements, so numContainingTags is always 0 or 1.  If and
         // when we support a different XML coding, we might need to look at this code more closely.)
         //
         QStringList xPathElements = fieldDefinition.xPath.split("/");
         Q_ASSERT(xPathElements.size() >= 1);
         int numContainingTags = xPathElements.size() - 1;
         for (int ii = 0; ii < numContainingTags; ++ii) {
            writeIndents(out, indentLevel + 1 + ii, indentString);
            out << "<" << xPathElements.at(ii) << ">\n";
         }
         qDebug() << Q_FUNC_INFO << xPathElements;
         qDebug() << Q_FUNC_INFO << xPathElements.last();
         std::shared_ptr<XmlRecord> subRecord = this->xmlCoding.getNewXmlRecord(xPathElements.last());

         if (XmlRecord::FieldType::RecordSimple == fieldDefinition.fieldType) {
            NamedEntity * childNamedEntity =
               namedEntityToExport.property(*fieldDefinition.propertyName).value<NamedEntity *>();
            if (childNamedEntity) {
               subRecord->toXml(*childNamedEntity, out, indentLevel + numContainingTags + 1, indentString);
            } else {
               this->writeNone(*subRecord, namedEntityToExport, out, indentLevel + numContainingTags + 1, indentString);
            }
         } else {
            //
            // In theory we could get a list of the contained records via the Qt Property system.  However, the
            // different things we would get back inside the QVariant (QList<BrewNote *>, QList<Hop *> etc) have no
            // common base class, so we can't safely treat them as, or upcast them to, QList<NamedEntity *>.
            //
            // Instead, we get the subclass of this class (eg XmlRecipeRecord) to do the work
            //
            this->subRecordToXml(fieldDefinition,
                                 *subRecord,
                                 namedEntityToExport,
                                 out,
                                 indentLevel + numContainingTags + 1,
                                 indentString);
         }

         // Obviously closing tags need to be written out in reverse order
         for (int ii = numContainingTags - 1; ii >= 0 ; --ii) {
            writeIndents(out, indentLevel + 1 + ii, indentString);
            out << "</" << xPathElements.at(ii) << ">\n";
         }
         continue;
      }

      QString valueAsText;
      if (fieldDefinition.fieldType == XmlRecord::FieldType::RequiredConstant) {
         //
         // This is a field that is required to be in the XML, but whose value we don't need, and for which we always
         // write a constant value on output.  At the moment it's only needed for the VERSION tag in BeerXML.
         //
         // Because it's such an edge case, we abuse the propertyName field to hold the default value (ie what we
         // write out).  This saves having an extra almost-never-used field on XmlRecord::FieldDefinition.
         //
         valueAsText = *fieldDefinition.propertyName;
      } else {
         QVariant value = namedEntityToExport.property(*fieldDefinition.propertyName);
         Q_ASSERT(value.isValid());

         // It's a coding error if we are trying here to write out some field with a complex XPath
         if (fieldDefinition.xPath.contains("/")) {
            qCritical() << Q_FUNC_INFO <<
               "Invalid use of non-trivial XPath (" << fieldDefinition.xPath << ") for output of property" <<
               fieldDefinition.propertyName << "of" << namedEntityToExport.metaObject()->className();
            Q_ASSERT(false); // Stop here on a debug build
            continue;        // Soldier on in a prod build
         }

         //
         // If the Qt property is an optional value, we need to unwrap it from std::optional and then, if it's null,
         // skip writing it out.  Strong typing of std::optional makes this a bit more work here (but it helps us in
         // other ways elsewhere).
         //
         // Note that:
         //    - propertyName is not actually a property name when fieldType is RequiredConstant
         //    - when propertyName is not set, there is nothing to look up (because this is a field we don't support,
         //      usually an "Extension tag")
         //
         bool const propertyIsOptional {
            (fieldDefinition.fieldType == XmlRecord::FieldType::RequiredConstant ||
               fieldDefinition.propertyName.isNull()) ?
               false : this->typeLookup->isOptional(fieldDefinition.propertyName)
         };
         switch (fieldDefinition.fieldType) {

            case XmlRecord::FieldType::Bool:
               if (Optional::removeOptionalWrapperIfPresent<bool>(value, propertyIsOptional)) {
                  // Unlike other XML documents, boolean fields in BeerXML are caps, so we have to accommodate that
                  valueAsText = value.toBool() ? "TRUE" : "FALSE";
               }
               break;

            case XmlRecord::FieldType::Int:
               if (Optional::removeOptionalWrapperIfPresent<int>(value, propertyIsOptional)) {
                  // QVariant knows how to convert a number to a string
                  valueAsText = value.toString();
               }
               break;

            case XmlRecord::FieldType::UInt:
               if (Optional::removeOptionalWrapperIfPresent<unsigned int>(value, propertyIsOptional)) {
                  // QVariant knows how to convert a number to a string
                  valueAsText = value.toString();
               }
               break;

            case XmlRecord::FieldType::Double:
               if (Optional::removeOptionalWrapperIfPresent<double>(value, propertyIsOptional)) {
                  // QVariant knows how to convert a number to a string
                  valueAsText = value.toString();
               }
               break;

            case XmlRecord::FieldType::Date:
               if (Optional::removeOptionalWrapperIfPresent<QDate>(value, propertyIsOptional)) {
                  // There is only one true date format :-)
                  valueAsText = value.toDate().toString(Qt::ISODate);
               }
               break;

            case XmlRecord::FieldType::Enum:
               // It's definitely a coding error if there is no enumMapping for a field declared as Enum!
               Q_ASSERT(nullptr != fieldDefinition.enumMapping);
               if (Optional::removeOptionalWrapperIfPresent<int>(value, propertyIsOptional)) {
                  // It's a coding error if we don't find a result (in which case EnumStringMapping::enumToString will
                  // log an error and throw an exception).
                  valueAsText = fieldDefinition.enumMapping->enumToString(value.toInt());
               }
               break;

            // By default we assume it's a string
            case XmlRecord::FieldType::String:
            default:
               if (Optional::removeOptionalWrapperIfPresent<QString>(value, propertyIsOptional)) {
                  // We use this to escape "&" to "&amp;" and so on in string content.  (Other data types should not
                  // have anything in their string representation that needs escaping in XML.)
                  QXmlStreamWriter qXmlStreamWriter(&valueAsText);
                  qXmlStreamWriter.writeCharacters(value.toString());
               }
               break;
         }

         if (propertyIsOptional && value.isNull()) {
            qDebug() <<
               Q_FUNC_INFO << "Not writing XPath" << fieldDefinition.xPath << "as property" <<
               fieldDefinition.propertyName << "is unset, ie set to std::nullopt";
            continue;
         }
      }

      writeIndents(out, indentLevel + 1, indentString);
      out << "<" << fieldDefinition.xPath << ">" << valueAsText << "</" << fieldDefinition.xPath << ">\n";
   }

   writeIndents(out, indentLevel, indentString);
   out << "</" << this->recordName << ">\n";
   return;
}

void XmlRecord::subRecordToXml(XmlRecord::FieldDefinition const & fieldDefinition,
                               [[maybe_unused]] XmlRecord const & subRecord,
                               NamedEntity const & namedEntityToExport,
                               [[maybe_unused]] QTextStream & out,
                               [[maybe_unused]] int indentLevel,
                               [[maybe_unused]] char const * const indentString) const {
   // Base class does not know how to handle nested records
   // It's a coding error if we get here as this virtual member function should be overridden classes that have nested records
   qCritical() << Q_FUNC_INFO <<
      "Coding error: cannot export" << namedEntityToExport.metaObject()->className() << "(" <<
      this->namedEntityClassName << ") property" << fieldDefinition.propertyName << "to <" << fieldDefinition.xPath <<
      "> from base class XmlRecord";
   Q_ASSERT(false);
   return;
}

void XmlRecord::writeNone(XmlRecord const & subRecord,
                          NamedEntity const & namedEntityToExport,
                          QTextStream & out,
                          int indentLevel,
                          char const * const indentString) const {
   //
   // The fact that we don't have anything to write for a particular subrecord may or may not be a problem in a given
   // XML coding.  Eg, we allow a recipe to exist without a style, equipment or mash, but, in BeerXML, only the latter
   // two of these three are optional.  For the moment we just log what's going on.
   //
   qInfo() <<
      Q_FUNC_INFO << "Skipping" << subRecord.getRecordName() << "tag while exporting" <<
      this->getRecordName() << "XML record for" << namedEntityToExport.metaObject()->className() <<
      "as no data to write";
   writeIndents(out, indentLevel, indentString);
   out << "<!-- No " << subRecord.getRecordName() << " in this " << this->getRecordName() << " -->\n";
   return;
}
