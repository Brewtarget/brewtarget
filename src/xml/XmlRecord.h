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


/**
 * \brief This class and its derived classes represent a record in an XML document.  See comment in xml/XmlCoding.h for
 *        more detail.
 */
class XmlRecord {
public:
   /**
    * At various stages of reading in an XML file, we need to distinguish between three cases:
    *   \c Succeeded - everything went OK and we should continue
    *   \c Failed - there was a problem and we should stop trying to read in the file
    *   \c FoundDuplicate - we realised that the record we are processing is a duplicate of one we already have in the
    *                       DB, in which case we should skip over this record and carry on processing the rest of the
    *                       file
    */
   enum ProcessingResult {
      Succeeded,
      Failed,
      FoundDuplicate
   };

   /**
    * \brief The types of fields that we know how to process.  Used in \b Field records
    */
   enum FieldType {
      Bool,
      Int,
      UInt,
      Double,
      String,
      Date,
      Enum,
      Record
   };

   /**
    * \brief Map from a string in an XML file to the value of an enum in a Brewtarget class
    *
    * .:TODO:. In theory we'll need to make this two-way when we extend to support saving XML, but a straight search
    *          through the whole map is not actually that burdensome
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
   struct FieldDefinition {
      FieldType           fieldType;
      XQString            xPath;
      char const * const  propertyName;
      EnumLookupMap const * stringToEnum;
   };

   typedef QVector<FieldDefinition> FieldDefinitions;

   /**
    * \brief Constructor
    * \param xmlCoding An \b XmlCoding object representing the XML Coding we are using (eg BeerXML 1.0).  This is what
    *                  we'll need to look up how to handle nested records inside this one.
    * \param fieldDefinitions A list of fields we expect to find in this record (other fields will be ignored) and how
    *                         to parse them.
    */
   XmlRecord(XmlCoding const & xmlCoding,
             FieldDefinitions const & fieldDefinitions);

   /**
    * \brief Getter for the NamedEntity we are reading in from this record
    * \return Pointer to an object that the caller does NOT own (or nullptr for the root record)
    */
   NamedEntity * getNamedEntity() const;

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
    * \return \b Succeeded, if processing succeeded, \b Failed, if there was an unresolvable problem, \b FoundDuplicate
    *         if the current record is a duplicate of one already in the DB and should be skipped.
    */
   virtual ProcessingResult normaliseAndStoreInDb(NamedEntity * containingEntity,
                                                  QTextStream & userMessage,
                                                  XmlRecordCount & stats);

private:
   /**
    * \brief Load in child records.  It is for derived classes to determine whether and when they have child records to
    *        process (eg Hop records inside a Recipe).  But the algorithm for processing is generic, so we implement it
    *        in this base class.
    */
   bool loadChildRecords(xalanc::DOMSupport & domSupport,
                         FieldDefinition const * fieldDefinition,
                         xalanc::NodeRefList & nodesForCurrentXPath,
                         QTextStream & userMessage);

protected:
   bool normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
                                          XmlRecordCount & stats);

   /**
    * \brief Checks whether the \b NamedEntity for this record is, in all the ways that count, a duplicate of one we
    *        already have stored in the DB
    * \return \b true if this is a duplicate and should be skipped rather than stored
    */
   virtual bool isDuplicate();

   /**
    * \brief If the \b NamedEntity for this record is supposed to have globally unique names, then this method will
    *        check the current name and modify it if necessary.  NB: This function should be called _after_
    *        \b isDuplicate().
    */
   virtual void normaliseName();

   /**
    * \brief If the \b NamedEntity for this record needs to know about its containing entity (because it is owned by
    *        that containing entity), this function should set it - eg this is where a \b BrewNote gets its \b Recipe
    *        set.  For other classes, this function is a no-op.
    */
   virtual void setContainingEntity(NamedEntity * containingEntity);

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
   static void modifyClashingName(QString & candidateName);

   XmlCoding const &        xmlCoding;

   FieldDefinitions const & fieldDefinitions;

   // The name of the class of object contained in this type of record, eg "Hop", "Yeast", etc.
   // Blank for the root record (which is just a container and doesn't have a NamedEntity).
   QString namedEntityClassName;

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
   // MOREOVER, there are circumstances where we want this->namedEntity and this->namedEntityRaiiContainer to point to
   // different things.  Specifically, if we are reading in, say a Hop and we discover that we already have the same
   // Hop (where "the same" means "as determined by NamedEntity.operator==") in the Database, then we will want to set
   // this->namedEntity to the Hop we already have stored (in case other objects we are reading in need to cross-refer
   // to it) and leave this->namedEntityRaiiContainer holding the newly-created Hop object that needs to be discarded.
   //
   // (An alternative approach would be to do replace this->namedEntityRaiiContainer with some boolean flag saying
   // whether we own the object and then write a custom destructor to check the flag and delete this->namedEntity if
   // necessary.  But this creates complication for dealing with duplicates.)
   //
   NamedEntity * namedEntity; // This is null for the root record of a document
   std::unique_ptr<NamedEntity> namedEntityRaiiContainer;

   // This determines whether we include this record in the stats we show the user (about how many records were read in
   // or skipped from a file.  By default it's true.  Subclass constructors set it to false for types of record that
   // are entirely owned and contained by other records (eg MashSteps are just part of a Mash, so we tell the user
   // about reading in a Mash but not about reading in a MashStep).
   bool includeInStats;

   //
   // Keep track of any child (ie contained) records
   // Key is the name of the class of the NamedEntity (eg "Hop", "Yeast", "MashStep", etc)
   // Value is a pair of:
   //   • The name, if any, of the property of this class that stores this child
   //   • A smart pointer to the child XmlRecord.  (Smart pointer ensures each child record is destroyed properly when
   //     our own destructor is called.
   //
   typedef std::pair<char const * const, std::shared_ptr<XmlRecord> > ChildRecord;
   QMultiHash<QString const &, ChildRecord> childRecords;
};

#endif
