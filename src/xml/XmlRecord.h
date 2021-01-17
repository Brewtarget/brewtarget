/*
 * xml/XmlRecord.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_XMLRECORD_H
#define _XML_XMLRECORD_H
#pragma once

#include <memory>

#include <QTextStream>

#include <xalanc/DOMSupport/DOMSupport.hpp>
#include <xalanc/XalanDOM/XalanNode.hpp>
#include <xalanc/XPath/NodeRefList.hpp>

#include "model/NamedEntity.h"
#include "xml/XmlRecordCount.h"
#include "xml/XQString.h"

class XmlCoding;

/////////////////
//
// TODO Still need to test MashStep and Mash
//
// TODO Still need to do Recipe
//
// TODO What about BrewNotes
//
/////////////////


/**
 * \brief This class and its derived classes represent a record in an XML document.  See comment in xml/XmlCoding.h for
 *        more detail.
 */
class XmlRecord {
public:

   /**
    * \brief The types of fields that we know how to process.  Used in \b Field records
    */
   enum FieldType {
      Bool,
      Int,
      UInt,
      Double,
      String,
      Enum,
      Record
   };

   /**
    * \brief Map from a string in an XML file to the value of an enum in a Brewtarget class
    *
    * .:TODO:. Need to make this two-way
    *
    * Could use QMap or QHash here.  Doubt it makes much difference either way for the quantity of data /
    * number of look-ups we're doing.  (Documentation says QHash is "significantly faster" if you don't need ordering,
    * but some people say that's only true beyond a certain number of elements stored.  We could benchmark it if we
    * were anxious about performance here.)
    */
   typedef QHash<QString, int> EnumLookupMap;

   /**
    * \brief How to parse every field that we want to be able to read out of the XML file.  See class description for
    *        more details.
    */
   struct Field {
      FieldType           fieldType;
      XQString            xPath;
      char const * const  propertyName;
      EnumLookupMap const * stringToEnum;
   };

   typedef QVector<Field> FieldDefinitions;

   /**
    * \brief Constructor
    * \param xmlCoding An \b XmlCoding object representing the XML Coding we are using (eg BeerXML 1.0).  This is what
    *                  we'll need to look up how to handle nested records inside this one.
    * \param recordName The name of the XML tag containing this type of record, eg "HOP", "YEAST", etc
    * \param fieldDefinitions A list of fields we expect to find in this record (other fields will be ignored) and how
    *                         to parse them.
    */
   XmlRecord(XmlCoding const & xmlCoding,
             QString const recordName,
             FieldDefinitions const & fieldDefinitions);

   /**
    * \brief
    * \return
    */
   QString const & getRecordName() const;

   /**
    * TODO Get rid of fieldsRead
    *
    * \brief From the supplied record (ie node) in an XML document, load into memory the data it contains, including
    *        any other records nested inside it.
    *
    * \param domSupport
    * \param rootNodeOfRecord
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    * \param fieldsRead Optional pointer to an empty hashmap.  If set, this method should populate it with what fields
    *                   it read in.  (This is useful for child classes that override this method and want to do further
    *                   validation after calling the parent class method.)
    *
    * \return \b true if load succeeded, \b false if there was an error
    */
   bool load(xalanc::DOMSupport & domSupport,
             xalanc::XalanNode * rootNodeOfRecord,
             QTextStream & userMessage,
             std::shared_ptr< QHash<QString, QVariant> > fieldsRead = nullptr);

   /**
    * \brief Once the record (including all its sub-records) is loaded into memory, we this function does any final
    *        validation and data correction before then storing the object(s) in the database.  Most validation should
    *        already have been done via the XSD, but there are some validation rules have to be done in code, including
    *        checking for duplicates and name clashes.
    *
    *        Child classes may override this function to extend functionality but should make sure to call this base
    *        class version to ensure child nodes are saved.
    *
    * \param containingEntity If not null, this is the entity that contains this one.  Eg, for a MashStep it should
    *                         always be the containing Mash.  For a Style inside a Recipe, this will be a pointer to
    *                         the Recipe, but for a freestanding Style, this will be null.
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    * \param stats This object keeps tally of how many records (of each type) we skipped or stored
    *
    * \return \b true if processing succeeded, \b false if there was an unresolvable problem.  Note that neither
    *         skipping over duplicates nor amending names counts as an error.  The skipping or amending will be logged,
    *         but it will not prevent the funciton from returning \b true.
    */
   virtual bool normaliseAndStoreInDb(NamedEntity * containingEntity,
                                      QTextStream & userMessage,
                                      XmlRecordCount & stats);

private:
   /**
    * \brief Load in child records.  It is for derived classes to determine whether and when they have child records to
    *        process (eg Hop records inside a Recipe).  But the algorithm for processing is generic, so we implement it
    *        in this base class.
    */
   bool loadChildRecords(xalanc::DOMSupport & domSupport,
                         xalanc::NodeRefList & nodesForCurrentXPath,
                         QTextStream & userMessage);

protected:
   bool normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
                                          XmlRecordCount & stats);

   /**
    * \brief Finds the first instance of \b NE with \b name() matching \b nameToFind.  This is used to avoid name
    *        clashes when loading some subclass of NamedEntity (eg Hop, Yeast, Equipment, Recipe) from an XML file (eg
    *        if are reading in a Recipe called "Oatmeal Stout" then this function can check whether we already have a
    *        Recipe with that name so that, assuming the new one is not a duplicate, we can amend its name to "Oatmeal
    *        Stout (1)" or some such.
    *
    *        Note child classes need to override this for the subclass of NamedEntity they handle.
    *        The default implementation does nothing and always returns \b nullptr (because it should never actually
    *        get called).
    *
    * \param nameToFind
    * \return A pointer to a \b NE with a matching \b name(), if there is one, or \b nullptr if not.  Note that this
    *         function does not tell you whether more than one \b NE has the name \b nameToFind
    */
   virtual NamedEntity * findByName(QString nameToFind);

   static void modifyClashingName(QString & candidateName);

   XmlCoding const &        xmlCoding;
   QString const            recordName;
   FieldDefinitions const & fieldDefinitions;

   // If this is true (the default), then, in normaliseAndStoreInDb(), before storing, we try to ensure that what we
   // load in does not create duplicate names.  Eg, if we already have a Recipe called "Oatmeal Stout" and then read in
   // a (different) recipe with the same name, then we will change the name of the newly read-in one to "Oatmeal Stout
   // (1)" (or "Oatmeal Stout (2)" if "Oatmeal Stout (1)" is taken, and so on).  For those NamedEntity subclasses where
   // we don't care about duplicate names (eg MashStep records), this attribute should be set to false.
   bool instanceNamesAreUnique;

   //
   // If we created a new NamedEntity (ie Hop/Yeast/Recipe/etc) object to populate with data read in from an XML file,
   // then we need to ensure it is properly destroyed if we abort that processing.  Putting it in this RAII container
   // handles that automatically for us.
   //
   // Once the object is populated, and we give ownership to something else (eg Database class), we should call
   // release(), because we no longer want the new Hop/Yeast/Recipe/etc object to be destroyed when the
   // XmlNamedEntityRecord is destroyed (typically at end of document processing).
   //
   // HOWEVER, we might still need access to the object even after we are no longer its owner.  This is why we also
   // have this->namedEntity.
   //
   // An alternative approach would be to do replace this->namedEntityRaiiContainer with some boolean flag saying
   // whether we own the object and then write a custom destructor to check the flag and delete this->namedEntity if
   // necessary.
   //
   NamedEntity * namedEntity; // This is null for the root record of a document
   std::unique_ptr<NamedEntity> namedEntityRaiiContainer;

   //
   // Keep track of any child records
   //
   QMultiHash< QString, std::shared_ptr<XmlRecord> > childRecords;

   // See https://apache.github.io/xalan-c/api/XalanNode_8hpp_source.html for possible indexes into this array
   static char const * const XALAN_NODE_TYPES[];
};

#endif
