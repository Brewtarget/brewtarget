/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/SerializationRecord.cpp is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#include "serialization/SerializationRecord.h"

#include <QRegularExpression>

#include "serialization/SerializationRecordDefinition.h"

///SerializationRecord::SerializationRecord() :
///   m_namedParameterBundle{NamedParameterBundle::OperationMode::NotStrict},
///   m_namedEntity{nullptr} {
///   return;
///}
///
///SerializationRecord::~SerializationRecord() = default;

///NamedParameterBundle const & SerializationRecord::getNamedParameterBundle() const {
///   return this->m_namedParameterBundle;
///}
///
///std::shared_ptr<NamedEntity> SerializationRecord::getNamedEntity() const {
///   return this->m_namedEntity;
///}

///void SerializationRecord::constructNamedEntity() {
///   // Base class does not have a NamedEntity or a container, so nothing to do
///   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
///   // NamedEntity, and subclasses that do have one should override this function.
//////   qCritical() <<
//////      Q_FUNC_INFO << this->m_recordDefinition.m_namedEntityClassName << "this->m_namedParameterBundle:" <<
//////      this->m_namedParameterBundle;
///   qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
///   Q_ASSERT(false && "Trying to construct named entity for base record");
///   return;
///}
///
///int SerializationRecord::storeNamedEntityInDb() {
///   Q_ASSERT(false && "Trying to store named entity for base record");
///   return -1;
///}
///
///void SerializationRecord::deleteNamedEntityFromDb() {
///   Q_ASSERT(false && "Trying to delete named entity for base record");
///   return;
///}
///
///bool SerializationRecord::includedInStats() const {
///   return false;
///}

///[[nodiscard]] SerializationRecord::ProcessingResult SerializationRecord::normaliseAndStoreInDb(
///   std::shared_ptr<NamedEntity> containingEntity,
///   QTextStream & userMessage,
///   ImportRecordCount & stats
///) {
///   if (this->m_namedEntity) {
///      qDebug() <<
///         Q_FUNC_INFO << "Normalise and store " << this->recordDefinition().m_namedEntityClassName << "(" <<
///         this->m_namedEntity->metaObject()->className() << "):" << this->m_namedEntity->name();
///
///      //
///      // If the object we are reading in is a duplicate of something we already have (and duplicates are not allowed)
///      // then skip over this record (and any records it contains).  (This is _not_ an error, so we return true rather
///      // than false in this event.)
///      //
///      // Note, however, that some objects -- in particular those such as Recipe that contain other objects -- need
///      // to be further along in their construction (ie have had all their contained objects added) before we can
///      // determine whether they are duplicates.  This is why we check again, after storing in the DB, below.
///      //
///      if (this->resolveDuplicates()) {
///         qDebug() <<
///            Q_FUNC_INFO << "(Early found) duplicate" << this->recordDefinition().m_namedEntityClassName <<
///            (this->includedInStats() ? " will" : " won't") << " be included in stats";
///         if (this->includedInStats()) {
///            stats.skipped(*this->recordDefinition().m_namedEntityClassName);
///         }
///         return SerializationRecord::ProcessingResult::FoundDuplicate;
///      }
///
///      this->normaliseName();
///
///      // Some classes of object are owned by their containing entity and can't sensibly be saved without knowing what it
///      // is.  Subclasses of XmlRecord will override setContainingEntity() to pass the info in if it is needed (or ignore
///      // it if not).
///      this->setContainingEntity(containingEntity);
///
///      // Now we're ready to store in the DB
///      int id = this->storeNamedEntityInDb();
///      if (id <= 0) {
///         userMessage << "Error storing " << this->m_namedEntity->metaObject()->className() <<
///         " in database.  See logs for more details";
///         return SerializationRecord::ProcessingResult::Failed;
///      }
///   }
///
///   SerializationRecord::ProcessingResult processingResult;
///
///   //
///   // Finally (well, nearly) orchestrate storing any contained records
///   //
///   // Note, of course, that this still needs to be done, even if nullptr == this->m_namedEntity, because that just means
///   // we're processing the root node.
///   //
///   if (this->normaliseAndStoreChildRecordsInDb(userMessage, stats)) {
///      //
///      // Now all the processing succeeded, we do that final duplicate check for any complex object such as Recipe that
///      // had to be fully constructed before we could meaningfully check whether it's the same as something we already
///      // have in the object store.
///      //
///      if (nullptr == this->m_namedEntity.get()) {
///         // Child records OK and no duplicate check needed (root record), which also means no further processing
///         // required.
///         return SerializationRecord::ProcessingResult::Succeeded;
///      }
///      processingResult = this->resolveDuplicates() ? SerializationRecord::ProcessingResult::FoundDuplicate :
///                                                     SerializationRecord::ProcessingResult::Succeeded;
///   } else {
///      // There was a problem with one of our child records
///      processingResult = SerializationRecord::ProcessingResult::Failed;
///   }
///
///   if (nullptr != this->m_namedEntity.get()) {
///      //
///      // We potentially do stats for everything except failure
///      //
///      if (SerializationRecord::ProcessingResult::FoundDuplicate == processingResult) {
///         qDebug() <<
///            Q_FUNC_INFO << "(Late found) duplicate" << this->recordDefinition().m_namedEntityClassName << "(" <<
///            this->recordDefinition().m_localisedEntityName << ") #" << this->m_namedEntity->key() <<
///            (this->includedInStats() ? " will" : " won't") << " be included in stats";
///         if (this->includedInStats()) {
///            stats.skipped(this->recordDefinition().m_localisedEntityName);
///         }
///      } else {
///         if (SerializationRecord::ProcessingResult::Succeeded == processingResult && this->includedInStats()) {
///            qDebug() <<
///               Q_FUNC_INFO << "Completed reading" << this->recordDefinition().m_namedEntityClassName << "#" <<
///               this->m_namedEntity->key() << " (which" <<
///               (this->includedInStats() ? "will" : "won't") << "be included in stats)";
///            stats.processedOk(this->recordDefinition().m_localisedEntityName);
///         }
///      }
///
///      //
///      // Clean-up if things went wrong.  (Note that resolveDuplicates will already have deleted any newly-read-in
///      // duplicate, so we don't have any clean-up to do for SerializationRecord::ProcessingResult::FoundDuplicate.)
///      //
///      if (SerializationRecord::ProcessingResult::Failed == processingResult) {
///         //
///         // If we reach here, it means there was a problem with one of our child records.  We've already stored our
///         // NamedEntity record in the DB, so we need to try to undo that by deleting it.  It is the responsibility of
///         // each NamedEntity subclass to take care of deleting any owned stored objects, via the virtual member function
///         // NamedEntity::hardDeleteOwnedEntities().  So we don't have to worry about child records that have already
///         // been stored.  (Eg if this is a Mash, and we stored it and 2 MashSteps before hitting an error on the 3rd
///         // MashStep, then deleting the Mash from the DB will also result in those 2 stored MashSteps getting deleted
///         // from the DB.)
///         //
///         qDebug() <<
///            Q_FUNC_INFO << "Deleting stored" << this->recordDefinition().m_namedEntityClassName <<
///            "as failed to read all child records";
///         this->deleteNamedEntityFromDb();
///      }
///   }
///
///   return processingResult;
///}

///[[nodiscard]] bool SerializationRecord::resolveDuplicates() {
///   // Base class does not have a NamedEntity so nothing to check
///   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
///   // NamedEntity, and subclasses that do have one should override this function.
///   Q_ASSERT(false && "Trying to check for duplicate NamedEntity when there is none");
///   return false;
///}
///
///void SerializationRecord::normaliseName() {
///   // Base class does not have a NamedEntity so nothing to normalise
///   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
///   // NamedEntity, and subclasses that do have one should override this function.
///   Q_ASSERT(false && "Trying to normalise name of NamedEntity when there is none");
///   return;
///}
///
///void SerializationRecord::setContainingEntity([[maybe_unused]] std::shared_ptr<NamedEntity> containingEntity) {
///   // Base class does not have a NamedEntity or a container, so nothing to do
///   // Stictly, it's a coding error if this function is called, as caller should first check whether there is a
///   // NamedEntity, and subclasses that do have one should override this function.
///   Q_ASSERT(false && "Trying to set containing entity when there is none");
///   return;
///}
