/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlRecord.h is part of Brewtarget, and is copyright the following authors 2020-2023:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef SERIALIZATION_XML_XMLRECORD_H
#define SERIALIZATION_XML_XMLRECORD_H
#pragma once

#include <vector>

#include <QTextStream>
#include <QVector>

#include <xalanc/DOMSupport/DOMSupport.hpp>
#include <xalanc/XalanDOM/XalanNode.hpp>
#include <xalanc/XPath/NodeRefList.hpp>

#include "serialization/xml/XmlRecordDefinition.h"
#include "serialization/SerializationRecord.h"
#include "utils/ImportRecordCount.h"
#include "utils/TypeLookup.h"

class XmlCoding;


/**
 * \brief This class and its derived classes represent a record in an XML document.  See comment in xml/XmlCoding.h for
 *        more detail.
 *
 *        Note that one structural difference between \c XmlRecord and \c JsonRecord is that, in the former we only pass
 *        underlying (ie Xalan) record data in when we are reading from XML, not when we are writing, so the parameter
 *        is on the \c load function, not the constructor.  Maybe one day we could rejig the XML so it works more like
 *        the Json code, or, more likely, we'll just leave it alone once everything is working.
 */
class XmlRecord : public SerializationRecord<XmlRecord, XmlCoding, XmlRecordDefinition> {
public:

   /**
    * \brief Constructor
    * \param recordName The name of the outer tag around this type of record, eg "RECIPE" for a "<RECIPE>...</RECIPE>"
    *                   record in BeerXML.
    * \param xmlCoding An \b XmlCoding object representing the XML Coding we are using (eg BeerXML 1.0).  This is what
    *                  we'll need to look up how to handle nested records inside this one.
    * \param fieldDefinitions A list of fields we expect to find in this record (other fields will be ignored) and how
    *                         to parse them.
    * \param typeLookup The \c TypeLookup object that, amongst other things allows us to tell whether Qt properties on
    *                   this object type are "optional" (ie wrapped in \c std::optional)
    * \param namedEntityClassName The class name of the \c NamedEntity to which this record relates, or empty string if
    *                             there is none
    */
   XmlRecord(XmlCoding           const & xmlCoding,
             XmlRecordDefinition const & recordDefinition);

   // Need a virtual destructor as we have virtual member functions
   virtual ~XmlRecord();

   virtual SerializationRecordDefinition const & recordDefinition() const override;

   /**
    * \brief From the supplied record (ie node) in an XML document, load into memory the data it contains, including
    *        any other records nested inside it.
    *
    * \param domSupport
    * \param rootNodeOfRecord
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    *
    * \return \b true if load succeeded, \b false if there was an error
    */
   bool load(xalanc::DOMSupport & domSupport,
             xalanc::XalanNode * rootNodeOfRecord,
             QTextStream & userMessage);

///   /**
///    * \brief Once the record (including all its sub-records) is loaded into memory, we this function does any final
///    *        validation and data correction before then storing the object(s) in the database.  Most validation should
///    *        already have been done via the XSD, but there are some validation rules have to be done in code, including
///    *        checking for duplicates and name clashes.
///    *
///    *        Child classes may override this function to extend functionality but should make sure to call this base
///    *        class version to ensure child nodes are saved.
///    *
///    * \param containingEntity If not null, this is the entity that contains this one.  Eg, for a MashStep it should
///    *                         always be the containing Mash.  For a Style inside a Recipe, this will be a pointer to
///    *                         the Recipe, but for a freestanding Style, this will be null.
///    * \param userMessage Where to append any error messages that we want the user to see on the screen
///    * \param stats This object keeps tally of how many records (of each type) we skipped or stored
///    *
///    * \return \b Succeeded, if processing succeeded, \b Failed, if there was an unresolvable problem, \b FoundDuplicate
///    *         if the current record is a duplicate of one already in the DB and should be skipped.
///    */
///   virtual ProcessingResult normaliseAndStoreInDb(std::shared_ptr<NamedEntity> containingEntity,
///                                                  QTextStream & userMessage,
///                                                  ImportRecordCount & stats);

   /**
    * \brief Export to XML
    * \param namedEntityToExport The object that we want to export to XML
    * \param out Where to write the XML
    * \param includeRecordNameTags Normally this should be \c true but, when we're writing a sub-record as though it
    *                              were part of the main records (eg RecipeAdditionHop::hop) then we don't want its
    *                              fields to be enclosed in another tag pair
    * \param indentLevel Current number of indents to put before each opening tag (default 1)
    * \param indentString String to use for each indent (default two spaces)
    */
   void toXml(NamedEntity const & namedEntityToExport,
              QTextStream & out,
              bool const includeRecordNameTags,
              int indentLevel = 1,
              char const * const indentString = "  ") const;

private:
   /**
    * \brief Load in child records.  It is for derived classes to determine whether and when they have child records to
    *        process (eg Hop records inside a Recipe).  But the algorithm for processing is generic, so we implement it
    *        in this base class.
    */
   bool loadChildRecords(xalanc::DOMSupport & domSupport,
                         XmlRecordDefinition::FieldDefinition const & parentFieldDefinition,
                         XmlRecordDefinition const & childRecordDefinition,
                         std::vector<xalanc::XalanNode *> & nodesForCurrentXPath,
                         QTextStream & userMessage);

protected:
///   bool normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
///                                          ImportRecordCount & stats);

   /**
    * \brief Called by \c toXml to write out any fields that are themselves records.
    *        Subclasses should provide the obvious recursive implementation.
    * \param fieldDefinition Which of the fields we're trying to export.  It will be of type \c XmlRecord::Record
    * \param subRecord A suitably constructed subclass of \c XmlRecord that can do the export.  (Note that because
    *                  exporting to XML is const on \c XmlRecord, we only need one of these even if there are multiple
    *                  records to export.)
    * \param namedEntityToExport The object containing (or referencing) the data we want to export to XML
    * \param out Where to write the XML
    */
   virtual void subRecordToXml(XmlRecordDefinition::FieldDefinition const & fieldDefinition,
                               XmlRecord const & subRecord,
                               NamedEntity const & namedEntityToExport,
                               QTextStream & out,
                               int indentLevel,
                               char const * const indentString) const;

   /**
    * \brief Writes a comment to the XML output when there is no contained record to output (to make it explicit that
    *        the omission was not by accident.
    */
   void writeNone(XmlRecord const & subRecord,
                  NamedEntity const & namedEntityToExport,
                  QTextStream & out,
                  int indentLevel,
                  char const * const indentString) const;

public:

protected:
///   XmlCoding           const & m_coding;

///   XmlRecordDefinition const & m_recordDefinition;

///   //
///   // Keep track of any child (ie contained) records as we're reading in FROM an XML file.  (NB: We don't need to do
///   // this when writing out TO an XML file as we don't have to worry about duplicate detection or construction order
///   // etc.)
///   //
///   // Note that we don't use QVector here or below as it always wants to be able to copy things, which doesn't play
///   // nicely with there being a std::unique_ptr inside the ChildRecordSet struct.
///   //
///   struct ChildRecordSet {
///      /**
///       * \brief Notes the attribute/field to which this set of child records relates.  Eg, if a recipe record has hop
///       *        and fermentable child records, then it needs to know which is which and how to store them.
///       *        If it's \c nullptr then that means this is a top-level record (eg just a hop variety rather than a use
///       *        of a hop in a recipe).
///       */
///      XmlRecordDefinition::FieldDefinition const * parentFieldDefinition;
///
///      /**
///       * \brief The actual child record
///       */
///      std::vector< std::unique_ptr<XmlRecord> > records;
///   };
///
///   std::vector<ChildRecordSet> m_childRecordSets;
};

#endif
