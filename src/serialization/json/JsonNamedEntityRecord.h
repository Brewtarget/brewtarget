/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonNamedEntityRecord.h is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#ifndef SERIALIZATION_JSON_JSONNAMEDENTITYRECORD_H
#define SERIALIZATION_JSON_JSONNAMEDENTITYRECORD_H
#pragma once

#include <QDebug>
#include <QString>
#include <QList>

#include "database/ObjectStoreWrapper.h"
#include "model/BrewNote.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/NamedEntity.h"
#include "model/Recipe.h"
#include "serialization/json/JsonRecord.h"


/**
 * \brief Provides class-specific extensions to \b JsonRecord.  See comment in json/JsonCoding.h for more details.
 */
template<class NE>
class JsonNamedEntityRecord : public JsonRecord {
public:
   /**
    * \brief This constructor doesn't have to do much more than create an appropriate new subclass of \b NamedEntity.
    *        Everything else is done in the base class.
    */
   JsonNamedEntityRecord(JsonCoding const & jsonCoding,
                         boost::json::value & recordData,
                         JsonRecordDefinition const & recordDefinition) :
      JsonRecord{jsonCoding, recordData, recordDefinition /*, NE::staticMetaObject.className()*/} {
      this->m_includeInStats = this->includedInStats();
      return;
   }

protected:
   virtual void constructNamedEntity() {
      // It's a coding error if this function is called when we already have a NamedEntity
      Q_ASSERT(nullptr == this->m_namedEntity.get());

      // Normally leave the next log statement commented out as it can create very long lines
//      qDebug() <<
//         Q_FUNC_INFO << this->m_recordDefinition.m_namedEntityClassName << "this->m_namedParameterBundle:" <<
//         this->m_namedParameterBundle;
      this->m_namedEntity = std::make_shared<NE>(this->m_namedParameterBundle);
   }

   virtual int storeNamedEntityInDb() {
      return ObjectStoreWrapper::insert(std::static_pointer_cast<NE>(this->m_namedEntity));
   }

public:
   virtual void deleteNamedEntityFromDb() {
      ObjectStoreWrapper::hardDelete(*std::static_pointer_cast<NE>(this->m_namedEntity));
      return;
   }

protected:
   //
   // TODO It's a bit clunky to have the knowledge/logic in this class for whether duplicates and name clashes are
   //      allowed.  Ideally this should be part of the NamedEntity subclasses themselves and the traits used here.
   //      The same applies to whether a NamedEntity subclass is "owned" by another NamedEntity (in the sense that a
   //      MashStep is owned by a Mash.
   //

   /**
    * \brief Implementation for general case where instances are supposed to be unique.  NB: What we really mean here
    *        is that, if we find a Hop/Yeast/Fermentable/etc in an JSON file that is "the same" as one that we already
    *        have stored, then we should not read it in.  This says nothing about whether we ourselves have multiple
    *        copies of such objects - eg as is currently the case when you add a Hop to a Recipe and a copy of the Hop
    *        is created.  (In the long-run we might want to change how that bit of the code works, but that's another
    *        story.)
    */
   virtual bool isDuplicate() {
      // It's a coding error if we are searching for a duplicate of a null object
      Q_ASSERT(this->m_namedEntity);

      // This copy of the pointer is just to make it clearer what we're passing to lambda in findFirstMatching() below
      std::shared_ptr<NE const> const currentEntity = std::static_pointer_cast<NE const>(this->m_namedEntity);
      auto matchResult = ObjectStoreTyped<NE>::getInstance().findFirstMatching(
         //
         // Note that, because we run this check both before and after something has been stored in the database (for
         // reasons explained in JsonRecord::normaliseAndStoreInDb) we need to be particularly careful NOT to match the
         // object with itself!
         //
         // Note too that we don't want to match against soft-deleted entities.  (Otherwise, if you delete something and
         // then try to import it again, it will never import!)
         //
         [currentEntity](std::shared_ptr<NE> ne) {
            return (*ne == *currentEntity) &&
                   (ne->key() != currentEntity->key()) &&
                   (!ne->deleted());
         }
      );
      if (matchResult) {
         qDebug() <<
            Q_FUNC_INFO << "Found a match (#" << matchResult->key() << "," << matchResult->name() <<
            ") for #" << this->m_namedEntity->key() << ", " << this->m_namedEntity->name();
         // Set our Hop/Yeast/Fermentable/etc to the one we found already stored in the database, so that any
         // containing Recipe etc can refer to it.  The new object we created will get deleted by the magic of shared
         // pointers.
         this->m_namedEntity = matchResult;
         return true;
      }
      qDebug() << Q_FUNC_INFO << "No match found for "<< this->m_namedEntity->name();
      return false;
   }

   /**
    * \brief Implementation for general case where name is supposed to be unique.  Before storing, we try to ensure
    *        that what we load in does not create duplicate names.  Eg, if we already have a Recipe called "Oatmeal
    *        Stout" and then read in a (different) recipe with the same name, then we will change the name of the
    *        newly read-in one to "Oatmeal Stout (1)" (or "Oatmeal Stout (2)" if "Oatmeal Stout (1)" is taken, and so
    *        on).  For those NamedEntity subclasses where we don't care about duplicate names (eg MashStep records),
    *        there is a no-op specialisation of this function.
    *
    *        See below for trivial specialisations of this function for classes where names are not unique.
    */
   virtual void normaliseName() {
      QString currentName = this->m_namedEntity->name();

      while (
         //
         // At the moment, we're pretty strict here and count a name clash even for things that are soft deleted.  If
         // we wanted to allow clashes with such soft-deleted things then we could add a check against ne->deleted()
         // as in the isDuplicate() function.
         //
         auto matchResult = ObjectStoreTyped<NE>::getInstance().findFirstMatching(
            [currentName](std::shared_ptr<NE> ne) {return ne->name() == currentName;}
         )
      ) {
         qDebug() << Q_FUNC_INFO << "Found existing " << this->m_recordDefinition.m_namedEntityClassName << "named" << currentName;

         JsonRecord::modifyClashingName(currentName);

         //
         // Now the for loop will search again with the new name
         //
         qDebug() << Q_FUNC_INFO << "Trying " << currentName;
      }

      this->m_namedEntity->setName(currentName);

      return;
   }

   /**
    * \brief Implementation of the general case where the object is independent of its containing entity
    */
   virtual void setContainingEntity([[maybe_unused]] std::shared_ptr<NamedEntity> containingEntity) {
      return;
   }

   /**
    * By default things are included in stats
    */
   virtual bool includedInStats() const { return true; }

};

// Specialisations for cases where duplicates are allowed
template<> inline bool JsonNamedEntityRecord<Instruction>::isDuplicate() { return false; }
template<> inline bool JsonNamedEntityRecord<Mash>::isDuplicate() { return false; }
template<> inline bool JsonNamedEntityRecord<MashStep>::isDuplicate() { return false; }
template<> inline bool JsonNamedEntityRecord<BrewNote>::isDuplicate() { return false; }

// Specialisations for cases where name is not required to be unique
template<> inline void JsonNamedEntityRecord<Instruction>::normaliseName() { return; }
template<> inline void JsonNamedEntityRecord<Mash>::normaliseName() { return; }
template<> inline void JsonNamedEntityRecord<MashStep>::normaliseName() { return; }
template<> inline void JsonNamedEntityRecord<BrewNote>::normaliseName() { return; }

// Specialisations for cases where object is owned by its containing entity
template<> inline void JsonNamedEntityRecord<BrewNote>::setContainingEntity(std::shared_ptr<NamedEntity> containingEntity) {
   qDebug() <<
      Q_FUNC_INFO << "BrewNote * " << static_cast<void*>(this->m_namedEntity.get()) << ", Recipe * " <<
      static_cast<void*>(containingEntity.get());
   auto brewNote = std::static_pointer_cast<BrewNote>(this->m_namedEntity);
   brewNote->setRecipe(static_cast<Recipe *>(containingEntity.get()));
   return;
}

// Specialisations for cases where we don't want the objects included in the stats
template<> inline bool JsonNamedEntityRecord<Instruction>::includedInStats() const { return false; }
template<> inline bool JsonNamedEntityRecord<BrewNote>::includedInStats() const { return false; }
template<> inline bool JsonNamedEntityRecord<MashStep>::includedInStats() const { return false; }


#endif
