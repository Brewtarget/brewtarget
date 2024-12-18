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
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/ImportRecordCount.h"

class SerializationRecordDefinition;

/**
 * \brief Base class for \c XmlRecord and \c JsonRecord
 *
 * TODO: There is more common functionality that could be pulled out into this base class, most easily by templating it
 *
 * TODO: I think this could be templated on Coding and RecordDefinition
 */
template<class Derived> class SerializationRecordPhantom;
template<typename Derived, typename CodingType, typename RecordDefinitionType>
class SerializationRecord : public CuriouslyRecurringTemplateBase<SerializationRecordPhantom, Derived> {
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

   SerializationRecord(CodingType const & coding,
                       RecordDefinitionType const & recordDefinition) :
      m_namedParameterBundle{NamedParameterBundle::OperationMode::NotStrict},
      m_namedEntity{nullptr},
      m_coding{coding},
      m_recordDefinition{recordDefinition},
      m_childRecordSets{} {
      return;
   }

   // Need a virtual destructor as we have virtual member functions
   virtual ~SerializationRecord() = default;

   /**
    * \brief Getter for the NamedParameterBundle we read in from this record
    *
    *        This is needed for the same reasons as \c getNamedEntity() below
    *
    * \return Reference to an object that the caller does NOT own
    */
   NamedParameterBundle const & getNamedParameterBundle() const {
      return this->m_namedParameterBundle;
   }

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
   std::shared_ptr<NamedEntity> getNamedEntity() const {
      return this->m_namedEntity;
   }

protected:
   virtual SerializationRecordDefinition const & recordDefinition() const = 0;

   /**
    * \brief Subclasses need to implement this to populate this->namedEntity with a suitably-constructed object using
    *        the contents of \c this->m_namedParameterBundle
    */
   virtual void constructNamedEntity() {
      // Base class does not have a NamedEntity or a container, so nothing to do
      // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
      // NamedEntity, and subclasses that do have one should override this function.
      qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      Q_ASSERT(false && "Trying to construct named entity for base record");
      return;
   }

   /**
    * \brief Subclasses  need to implement this to store this->namedEntity in the appropriate ObjectStore
    * \return the ID of the newly-inserted object
    */
   virtual int storeNamedEntityInDb() {
      qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      Q_ASSERT(false && "Trying to store named entity for base record");
      return -1;
   }

public:
   /**
    * \brief Subclasses need to implement this to delete \c this->m_namedEntity from the appropriate ObjectStore (this
    *        is in the event of problems detected after the call to this->storeNamedEntityInDb()
    */
   virtual void deleteNamedEntityFromDb() {
      qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      Q_ASSERT(false && "Trying to delete named entity for base record");
      return;
   }

   /**
    * \brief This determines whether we include this record in the stats we show the user (about how many records were
    *        read in or skipped from a file.  By default it returns \c false.  Subclasses should return \c false for
    *        types of record that are entirely owned and contained by other records (eg \c MashStep records are just
    *        part of a \c Mash, so we tell the user about reading in a \c Mash but not about reading in a \c MashStep)
    *        and \c true otherwise.
    */
   virtual bool includedInStats() const {
      return false;
   }

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
   [[nodiscard]] virtual ProcessingResult normaliseAndStoreInDb(std::shared_ptr<NamedEntity> containingEntity,
                                                                QTextStream & userMessage,
                                                                ImportRecordCount & stats) {
      if (this->m_namedEntity) {
         qDebug() <<
            Q_FUNC_INFO << "Normalise and store " << this->recordDefinition().m_namedEntityClassName << "(" <<
            this->m_namedEntity->metaObject()->className() << "):" << this->m_namedEntity->name();

         //
         // If the object we are reading in is a duplicate of something we already have (and duplicates are not allowed)
         // then skip over this record (and any records it contains).  (This is _not_ an error, so we return true rather
         // than false in this event.)
         //
         // Note, however, that some objects -- in particular those such as Recipe that contain other objects -- need
         // to be further along in their construction (ie have had all their contained objects added) before we can
         // determine whether they are duplicates.  This is why we check again, after storing in the DB, below.
         //
         if (this->resolveDuplicates()) {
            qDebug() <<
               Q_FUNC_INFO << "(Early found) duplicate" << this->recordDefinition().m_namedEntityClassName <<
               (this->includedInStats() ? " will" : " won't") << " be included in stats";
            if (this->includedInStats()) {
               stats.skipped(*this->recordDefinition().m_namedEntityClassName);
            }
            return SerializationRecord::ProcessingResult::FoundDuplicate;
         }

         this->normaliseName();

         // Some classes of object are owned by their containing entity and can't sensibly be saved without knowing what it
         // is.  Subclasses of XmlRecord will override setContainingEntity() to pass the info in if it is needed (or ignore
         // it if not).
         this->setContainingEntity(containingEntity);

         // Now we're ready to store in the DB
         int id = this->storeNamedEntityInDb();
         if (id <= 0) {
            userMessage << "Error storing " << this->m_namedEntity->metaObject()->className() <<
            " in database.  See logs for more details";
            return SerializationRecord::ProcessingResult::Failed;
         }
      }

      SerializationRecord::ProcessingResult processingResult;

      //
      // Finally (well, nearly) orchestrate storing any contained records
      //
      // Note, of course, that this still needs to be done, even if nullptr == this->m_namedEntity, because that just means
      // we're processing the root node.
      //
      if (this->normaliseAndStoreChildRecordsInDb(userMessage, stats)) {
         //
         // Now all the processing succeeded, we do that final duplicate check for any complex object such as Recipe that
         // had to be fully constructed before we could meaningfully check whether it's the same as something we already
         // have in the object store.
         //
         if (nullptr == this->m_namedEntity.get()) {
            // Child records OK and no duplicate check needed (root record), which also means no further processing
            // required.
            return SerializationRecord::ProcessingResult::Succeeded;
         }
         processingResult = this->resolveDuplicates() ? SerializationRecord::ProcessingResult::FoundDuplicate :
                                                        SerializationRecord::ProcessingResult::Succeeded;
      } else {
         // There was a problem with one of our child records
         processingResult = SerializationRecord::ProcessingResult::Failed;
      }

      if (nullptr != this->m_namedEntity.get()) {
         //
         // We potentially do stats for everything except failure
         //
         if (SerializationRecord::ProcessingResult::FoundDuplicate == processingResult) {
            qDebug() <<
               Q_FUNC_INFO << "(Late found) duplicate" << this->recordDefinition().m_namedEntityClassName << "(" <<
               this->recordDefinition().m_localisedEntityName << ") #" << this->m_namedEntity->key() <<
               (this->includedInStats() ? " will" : " won't") << " be included in stats";
            if (this->includedInStats()) {
               stats.skipped(this->recordDefinition().m_localisedEntityName);
            }
         } else {
            if (SerializationRecord::ProcessingResult::Succeeded == processingResult && this->includedInStats()) {
               qDebug() <<
                  Q_FUNC_INFO << "Completed reading" << this->recordDefinition().m_namedEntityClassName << "#" <<
                  this->m_namedEntity->key() << " (which" <<
                  (this->includedInStats() ? "will" : "won't") << "be included in stats)";
               stats.processedOk(this->recordDefinition().m_localisedEntityName);
            }
         }

         //
         // Clean-up if things went wrong.  (Note that resolveDuplicates will already have deleted any newly-read-in
         // duplicate, so we don't have any clean-up to do for SerializationRecord::ProcessingResult::FoundDuplicate.)
         //
         if (SerializationRecord::ProcessingResult::Failed == processingResult) {
            //
            // If we reach here, it means there was a problem with one of our child records.  We've already stored our
            // NamedEntity record in the DB, so we need to try to undo that by deleting it.  It is the responsibility of
            // each NamedEntity subclass to take care of deleting any owned stored objects, via the virtual member function
            // NamedEntity::hardDeleteOwnedEntities().  So we don't have to worry about child records that have already
            // been stored.  (Eg if this is a Mash, and we stored it and 2 MashSteps before hitting an error on the 3rd
            // MashStep, then deleting the Mash from the DB will also result in those 2 stored MashSteps getting deleted
            // from the DB.)
            //
            qDebug() <<
               Q_FUNC_INFO << "Deleting stored" << this->recordDefinition().m_namedEntityClassName <<
               "as failed to read all child records";
            this->deleteNamedEntityFromDb();
         }
      }

      return processingResult;
   }

   [[nodiscard]] virtual bool normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
                                                                ImportRecordCount & stats) {
      //
      // We are assuming it does not matter which order different children are processed in.
      //
      // Where there are several children of the same type, we need to process them in the same order as they were read in
      // from the XML document because, in some cases, this order matters.  In particular, in both BeerXML and BeerJSON,
      // the Mash Steps inside a Mash (eg for BeerXML the MASH_STEP tags inside a MASH_STEPS tag inside a MASH tag) are
      // stored in order without any other means of identifying order.
      //
      // So it's simplest just to process all the child records in the order they were read out of the XML document.  This
      // is the advantage of storing things in a list such as QVector.  (Alternatives such as QMultiHash iterate through
      // items that share the same key in the opposite order to which they were inserted and don't offer STL reverse
      // iterators, so going backwards would be a bit clunky.)
      //
      qDebug() <<
         Q_FUNC_INFO << "this->m_childRecordSets for" << this->m_recordDefinition << "has" <<
         this->m_childRecordSets.size() << "entries";
      for (auto & childRecordSet : this->m_childRecordSets) {
         if (childRecordSet.parentFieldDefinition) {
            qDebug() <<
               Q_FUNC_INFO << this->m_recordDefinition << ": childRecordSet" << *childRecordSet.parentFieldDefinition <<
               "now holds" << childRecordSet.records.size() << "record(s)";
         } else {
            qDebug() << Q_FUNC_INFO << "Top-level record has" << childRecordSet.records.size() << "entries";
         }

         // If the list of children is empty, there is no work to do.  Explicitly move on to the next loop item.  (This
         // means that, in the code below, we know the list is non-empty, so it's valid to look at the first item etc.)
         if (0 == childRecordSet.records.size()) {
            continue;
         }

         QList< std::shared_ptr<NamedEntity> > processedChildren;
         for (auto & childRecord : childRecordSet.records) {
            // The childRecord variable is a reference to a std::unique_ptr (because the vector we're looping over owns the
            // records it contains), which is why we have all the "member of pointer" (->) operators below.
            qDebug() <<
               Q_FUNC_INFO << "Storing" << childRecord->m_recordDefinition.m_namedEntityClassName << "child of" <<
               this->m_recordDefinition.m_namedEntityClassName << ":" << this->m_namedEntity;
            if (SerializationRecord::ProcessingResult::Failed ==
               childRecord->normaliseAndStoreInDb(this->m_namedEntity, userMessage, stats)) {
               return false;
            }
            processedChildren.append(childRecord->m_namedEntity);
         }

         //
         // Now we've stored the child record (or recognised it as a duplicate of one we already hold), we want to link it
         // (or as the case may be the record it's a duplicate of) to the parent.  If this is possible via a property (eg
         // the style on a recipe), then we can just do that here.  Otherwise the work needs to be done in the appropriate
         // subclass of XmlNamedEntityRecord.
         //
         // We can't use the presence or absence of a property name to determine whether the child record can be set via
         // a property because some properties are read-only (and need to be present in the FieldDefinition for export to
         // XML to work).  Instead we distinguish between two types of records: Record, which can be set via a
         // property, and ListOfRecords, which can't.
         //
         if (childRecordSet.parentFieldDefinition) {
            auto const & fieldDefinition{*childRecordSet.parentFieldDefinition};
            Q_ASSERT(std::holds_alternative<RecordDefinitionType const *>(fieldDefinition.valueDecoder));
            Q_ASSERT(std::get              <RecordDefinitionType const *>(fieldDefinition.valueDecoder));
            RecordDefinitionType const & childRecordDefinition{
               *std::get<RecordDefinitionType const *>(fieldDefinition.valueDecoder)
            };

            auto const & propertyPath = fieldDefinition.propertyPath;
            if (!propertyPath.isNull()) {
               // It's a coding error if we had a property defined for a record that's not trying to populate a NamedEntity
               // (ie for the root record).
               Q_ASSERT(this->m_namedEntity);

               QVariant valueToSet;
               //
               // How we set the property depends on whether this is a single child record or an array of them
               //
               if (fieldDefinition.type != RecordDefinitionType::FieldType::ListOfRecords) {
                  // It's a coding error if we ended up with more than on child when there's only supposed to be one!
                  if (processedChildren.size() > 1) {
                     qCritical() <<
                        Q_FUNC_INFO << "Only expecting one record for" << propertyPath << "property on" <<
                        this->m_recordDefinition.m_namedEntityClassName << "object, but found" << processedChildren.size();
                     Q_ASSERT(false);
                  }

                  //
                  // We need to pass a pointer to the relevant subclass of NamedEntity (call it of class ChildEntity for
                  // the sake of argument) in to the property setter (inside a QVariant).  As in JsonRecord::toJson, we
                  // need to handle any of the following forms:
                  //    ChildEntity *                               -- eg Equipment *
                  //    std::shared_ptr<ChildEntity>                -- eg std::shared_ptr<Mash>
                  //    std::optional<std::shared_ptr<ChildEntity>> -- eg std::optional<std::shared_ptr<Boil>>
                  //
                  // First we assert that they type is _some_ sort of pointer, otherwise it's a coding error.
                  //
                  auto const & typeInfo = fieldDefinition.propertyPath.getTypeInfo(*this->m_recordDefinition.m_typeLookup);
                  Q_ASSERT(typeInfo.pointerType != TypeInfo::PointerType::NotPointer);

                  if (typeInfo.pointerType == TypeInfo::PointerType::RawPointer) {
                     // For a raw pointer, we don't have to upcast as the pointer will get upcast in the setter during the
                     // extraction from QVariant
                     valueToSet = QVariant::fromValue(processedChildren.first().get());
                  } else {
                     // Should be the only possibility left.
                     Q_ASSERT(typeInfo.pointerType == TypeInfo::PointerType::SharedPointer);
                     Q_ASSERT(childRecordDefinition.m_upAndDownCasters.m_pointerUpcaster);
                     valueToSet = QVariant::fromValue(
                        childRecordDefinition.m_upAndDownCasters.m_pointerUpcaster(processedChildren.first())
                     );
                  }

               } else {
                  //
                  // Multi-item setters for class T all take a list of shared pointers to T, so we need to upcast from
                  // our list of shared pointers to NamedEntity.
                  //
                  // Note that we need the child's upcaster, not the parent's.  Eg, if we are setting the hopAdditions
                  // property on a Recipe, we need the RecipeAdditionHop upcaster to cast
                  // QList<std::shared_ptr<NamedEntity> > to QList<std::shared_ptr<RecipeAdditionHop> >
                  //
                  valueToSet =
                     childRecordSet.records.at(0)->m_recordDefinition.m_upAndDownCasters.m_listUpcaster(processedChildren);
               }

               qDebug() <<
                  Q_FUNC_INFO << "Setting" << propertyPath << "property on" <<
                  this->m_recordDefinition.m_namedEntityClassName << "with" << processedChildren.size() << "value(s):" <<
                  valueToSet;
               if (!propertyPath.setValue(*this->m_namedEntity, valueToSet)) {
                  // It's a coding error if we could not set the property we use to pass in the child records
                  qCritical() <<
                     Q_FUNC_INFO << "Could not write" << propertyPath << "property on" <<
                     this->m_recordDefinition.m_namedEntityClassName;
                  Q_ASSERT(false);
                  return false;
               }
            }
         }
      }

      return true;

   }

protected:

   /**
    * \brief Checks whether the \b NamedEntity for this record is, in all the ways that count, a duplicate of one we
    *        already have stored in the DB
    *
    *        Note that this is \b not a \c const function as, in the case that we do find a duplicate, we will update
    *        some of our internal data to point to the existing stored \c NamedEntity and delete the newly read-in
    *        duplicate.
    *
    * \return \b true if this is a duplicate and should be marked in the stats as skipped rather than stored
    */
   [[nodiscard]] virtual bool resolveDuplicates() {
      // Base class does not have a NamedEntity so nothing to check
      // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
      // NamedEntity, and subclasses that do have one should override this function.
      Q_ASSERT(false && "Trying to check for duplicate NamedEntity when there is none");
      return false;
   }

   /**
    * \brief If the \b NamedEntity for this record is supposed to have globally unique names, then this method will
    *        check the current name and modify it if necessary.  NB: This function should be called _after_
    *        \b isDuplicate().
    */
   virtual void normaliseName() {
      // Base class does not have a NamedEntity so nothing to normalise
      // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
      // NamedEntity, and subclasses that do have one should override this function.
      Q_ASSERT(false && "Trying to normalise name of NamedEntity when there is none");
      return;
   }

   /**
    * \brief If the \b NamedEntity for this record needs to know about its containing entity (because it is owned by
    *        that containing entity), this function should set it - eg this is where a \b BrewNote gets its \b Recipe
    *        set.  For other classes, this function is a no-op.
    */
   virtual void setContainingEntity([[maybe_unused]] std::shared_ptr<NamedEntity> containingEntity) {
      // Base class does not have a NamedEntity or a container, so nothing to do
      // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
      // NamedEntity, and subclasses that do have one should override this function.
      Q_ASSERT(false && "Trying to set containing entity when there is none");
      return;
   }

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

   CodingType const & m_coding;

   RecordDefinitionType const & m_recordDefinition;

   //
   // Keep track of any child (ie contained) records as we're reading in FROM a JSON file.  (NB: We don't need to do
   // this when writing out TO a JSON file as we don't have to worry about duplicate detection or construction order
   // etc.)
   //
   // This is used both for lists of children (eg hop additions in a recipe, or steps in a mash) and for single children
   // (eg boil of a recipe, mash of a recipe).
   //
   // Note that we don't use QVector here or below as it always wants to be able to copy things, which doesn't play
   // nicely with there being a std::unique_ptr inside the ChildRecordSet struct.  OTOH, std::vector is guaranteed to
   // be able to hold std_unique_ptr (provided, of course, that we use move semantics to put the elements in the
   // vector).
   //
   struct ChildRecordSet {
      /**
       * \brief This holds info about the attribute/field to which this set of child records relates.  Eg, if a recipe
       *        record has hop addition and fermentable addition child records, then it needs to know which is which and
       *        how to connect them with the recipe.
       *
       *        If it's \c nullptr then that means this is a top-level record (eg just a hop variety rather than a use
       *        of a hop in a recipe), in which, once the records are read in, there's no further work to do: the
       *        records do not need to be connected with anything else.
       */
      RecordDefinitionType::FieldDefinition const * parentFieldDefinition;

      /**
       * \brief The actual child record(s)
       */
      std::vector< std::unique_ptr<Derived> > records;
   };

   std::vector<ChildRecordSet> m_childRecordSets;

};

#endif
