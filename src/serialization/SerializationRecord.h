/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/SerializationRecord.h is part of Brewtarget, and is copyright the following authors 2020-2023:
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
#ifndef SERIALIZATION_SERIALIZATIONRECORD_H
#define SERIALIZATION_SERIALIZATIONRECORD_H
#pragma once

#include <memory>

#include "model/NamedEntity.h"
#include "model/NamedParameterBundle.h"

/**
 * \brief Base class for \c XmlRecord and \c JsonRecord
 *
 * TODO: There is more common functionality that could be pulled out into this base class
 *
 * TODO: I think this could be templated on Coding and RecordDefinition
 */
class SerializationRecord {
public:
   /**
    * At various stages of reading in a JSON or XML file, we need to distinguish between three cases:
    *   \c Succeeded - everything went OK and we should continue
    *   \c Failed - there was a problem and we should stop trying to read in the file
    *   \c FoundDuplicate - we realised that the record we are processing is a duplicate of one we already have in the
    *                       DB, in which case we should skip over this record and carry on processing the rest of the
    *                       file
    */
   enum class ProcessingResult {
      Succeeded,
      Failed,
      FoundDuplicate
   };

   SerializationRecord();

   // Need a virtual destructor as we have virtual member functions
   virtual ~SerializationRecord();

   /**
    * \brief Getter for the NamedParameterBundle we read in from this record
    *
    *        This is needed for the same reasons as \c getNamedEntity() below
    *
    * \return Reference to an object that the caller does NOT own
    */
   NamedParameterBundle const & getNamedParameterBundle() const;

   /**
    * \brief Getter for the NamedEntity we are reading in from this record
    *
    *        This is needed to allow one \c SerializationRecord (or subclass) object to read the data from another (eg
    *        for \c JsonRecipeRecord to work with contained \c JsonRecord objects).  (The protected access on
    *        \c SerializationRecord::namedEntity only allows an instance of a derived class to access this field on its
    *        own instance.)
    *
    * \return Shared pointer, which will contain nullptr for the root record
    */
   std::shared_ptr<NamedEntity> getNamedEntity() const;

protected:
   /**
    * \brief Subclasses need to implement this to populate this->namedEntity with a suitably-constructed object using
    *        the contents of \c this->m_namedParameterBundle
    */
   virtual void constructNamedEntity();

   /**
    * \brief Subclasses  need to implement this to store this->namedEntity in the appropriate ObjectStore
    * \return the ID of the newly-inserted object
    */
   virtual int storeNamedEntityInDb();

public:
   /**
    * \brief Subclasses need to implement this to delete \c this->m_namedEntity from the appropriate ObjectStore (this
    *        is in the event of problems detected after the call to this->storeNamedEntityInDb()
    */
   virtual void deleteNamedEntityFromDb();

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

protected:

   /**
    * \brief Checks whether the \b NamedEntity for this record is, in all the ways that count, a duplicate of one we
    *        already have stored in the DB
    *
    *        Note that this is \b not a \c const function as, in the case that we do find a duplicate, we will update
    *        some of our internal data to point to the existing stored \c NamedEntity.
    *
    * \return \b true if this is a duplicate and should be skipped rather than stored
    */
   [[nodiscard]] virtual bool isDuplicate();

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
   virtual void setContainingEntity(std::shared_ptr<NamedEntity> containingEntity);


   // Name-value pairs containing all the field data from the XML or JSON record that will be used to construct/populate
   // this->m_namedEntity
   NamedParameterBundle m_namedParameterBundle;

   //
   // If we created a new NamedEntity (ie Hop/Yeast/Recipe/etc) object to populate with data read in from an XML or JSON
   // file, then we need to ensure it is properly destroyed if we abort that processing.  Putting it in this RAII
   // container handles that automatically for us.
   //
   // Once the object is populated, and we give ownership to the relevant Object Store there will be another instance of
   // this shared pointer (in the object store), which is perfect because, at this point, we don't want the new
   // Hop/Yeast/Recipe/etc object to be destroyed when the XmlNamedEntityRecord or JsonNamedEntityRecord is destroyed
   // (typically at end of document processing).
   //
   std::shared_ptr<NamedEntity> m_namedEntity;

   // This determines whether we include this record in the stats we show the user (about how many records were read in
   // or skipped from a file.  By default it's true.  Subclass constructors set it to false for types of record that
   // are entirely owned and contained by other records (eg MashSteps are just part of a Mash, so we tell the user
   // about reading in a Mash but not about reading in a MashStep).
   bool m_includeInStats;

};

#endif
