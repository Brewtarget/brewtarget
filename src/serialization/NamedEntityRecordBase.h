/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/NamedEntityRecordBase.h is part of Brewtarget, and is copyright the following authors 2020-2026:
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
#ifndef SERIALIZATION_NAMEDENTITYRECORDBASE_H
#define SERIALIZATION_NAMEDENTITYRECORDBASE_H
#pragma once

#include "database/ObjectStoreUtils.h"
#include "serialization/SerializationRecord.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeTraits.h"

namespace Serialization {
   /**
    * \class NamedEntityRecordBase CRTP base for \c XmlNamedEntityRecord and \c JsonNamedEntityRecord
    */
   template<class Derived> class NamedEntityRecordPhantom;
   template<class Derived, class NE>
   class NamedEntityRecordBase : public CuriouslyRecurringTemplateBase<NamedEntityRecordPhantom, Derived> {

   friend Derived;

   private:
      NamedEntityRecordBase() {
         return;
      }

      void doConstructNamedEntity() {
         // It's a coding error if this function is called when we already have a NamedEntity
         Q_ASSERT(nullptr == this->derived().m_namedEntity.get());
         // Normally keep this log statement commented out otherwise it generates too many lines in the log file
//         qDebug() <<
//            Q_FUNC_INFO << "Constructing" << NE::staticMetaObject.className() << "from" << this->derived().m_namedParameterBundle;

         this->derived().m_namedEntity = std::make_shared<NE>(this->derived().m_namedParameterBundle);
         return;
      }

      int doStoreNamedEntityInDb() {
         auto namedEntity = std::static_pointer_cast<NE>(this->derived().m_namedEntity);

         //
         // Where this object is owned by another, it may already have been inserted in the database by
         // doSetContainingEntity().
         //
         if constexpr (std::is_base_of<OwnedByRecipe, NE>::value ||
                       IsBaseClassTemplateOf<EnumeratedBase, NE>) {
            if (namedEntity->key() > 0) {
               return namedEntity->key();
            }
         }

         return ObjectStoreWrapper::insert(namedEntity);
      }

      void doDeleteNamedEntityFromDb() {
         auto namedEntity = std::static_pointer_cast<NE>(this->derived().m_namedEntity);

         //
         // Where this object is owned by another, then, instead of trying to delete it here, we should leave it to the
         // owning object to do deletion when it itself is deleted.
         //
         if constexpr (std::is_base_of<OwnedByRecipe, NE>::value ||
                       IsBaseClassTemplateOf<EnumeratedBase, NE>) {
            if (namedEntity->ownerId() > 0) {
               return;
            }
         }
         ObjectStoreWrapper::hardDelete(*namedEntity);
         return;
      }

      bool doIncludedInStats() const {
         if constexpr (std::is_base_of<OwnedByRecipe, NE>::value ||
                       IsBaseClassTemplateOf<EnumeratedBase, NE>) {
            // "Owned" items do not have an existence independent of their owner, so we do not include them in the stats
            return false;
         }
         // Otherwise, by default, things are included in stats
         return true;
      }

      /**
       * \brief
       */
      bool doResolveDuplicates() {
         // It's a coding error if we are searching for a duplicate of a null object
         Q_ASSERT(this->derived().m_namedEntity);

         // Deal first with the cases where duplicates are allowed (because each entity is owned by another)
         if constexpr (std::is_base_of<OwnedByRecipe, NE>::value ||
                       IsBaseClassTemplateOf<EnumeratedBase, NE>) {
            return false;
         }

         //
         // The substance of the function handles the general case, where instances are supposed to be unique.  NB: What
         // we really mean here is that, if we find a Hop/Yeast/Fermentable/etc in an JSON or BeerXML file that is "the
         // same" as one that we already have stored, then we should not read it in.  This says nothing about whether we
         // ourselves have multiple copies of such objects.
         //

         // This copy of the pointer is just to make it clearer what we're passing to lambda in findFirstMatching() below
         std::shared_ptr<NE const> const namedEntity =
            std::static_pointer_cast<NE const>(this->derived().m_namedEntity);
         auto matchResult = ObjectStoreTyped<NE>::getInstance().findFirstMatching(
            //
            // Note that, because we run this check both before and after something has been stored in the database (for
            // reasons explained in XmlRecord::normaliseAndStoreInDb and JsonRecord::normaliseAndStoreInDb) we need to
            // be particularly careful NOT to match the object with itself!
            //
            // Note too that we don't want to match against soft-deleted entities.  (Otherwise, if you delete something
            // and then try to import it again, it will never import!)
            //
            [namedEntity](std::shared_ptr<NE> ne) {
               return
                  // Don't compare object against itself!
                  (ne->key() != namedEntity->key()) &&
                  // Don't compare with deleted objects
                  (!ne->deleted()) &&
                  // Substantive comparison
                  (*ne == *namedEntity);
            }
         );
         if (matchResult) {
            qDebug() <<
               Q_FUNC_INFO << "Found a match (#" << matchResult->key() << "," << matchResult->name() <<
               ") for #" << namedEntity->key() << ", " << namedEntity->name();

            //
            // If we haven't yet written the newly read-in Hop/Yeast/Fermentable/etc to the database, then we can just
            // set it to the existing one we found already stored in the database, so that any containing Recipe etc can
            // refer to it.  The new object we created will get deleted by the magic of shared pointers.
            //
            // However, if our newly read-in object is already in the database (typically because it wasn't possible to
            // detect it as a duplicate before reading in sub-objects), then we need to hard delete it first.
            //
            if (namedEntity->key() > 0) {
               ObjectStoreWrapper::hardDelete(*namedEntity);
            }
            // NB: Need to assign to derived class member variable, not our local variable!
            this->derived().m_namedEntity = matchResult;
            return true;
         }
         qDebug() << Q_FUNC_INFO << "No match found for "<< namedEntity->name();
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
     void doNormaliseName() {
         QString const currentName = this->derived().m_namedEntity->name();
         this->derived().m_namedEntity->setName(ObjectStoreUtils::normaliseName<NE>(currentName));

         return;
      }


      /**
       * \brief Implementation of the general case where the object is independent of its containing entity
       */
      void doSetContainingEntity([[maybe_unused]] std::shared_ptr<NamedEntity> containingEntity) {
         if constexpr (std::is_base_of<OwnedByRecipe, NE>::value) {
            qDebug() <<
               Q_FUNC_INFO << NE::staticMetaObject.className() << "*" <<
               static_cast<void *>(this->derived().m_namedEntity.get()) << ", Recipe * " <<
               static_cast<void *>(containingEntity.get());
            auto ownedByRecipe = std::static_pointer_cast<NE>(this->derived().m_namedEntity);
            ownedByRecipe->setRecipe(static_cast<Recipe *>(containingEntity.get()));
         } else if constexpr (IsBaseClassTemplateOf<EnumeratedBase, NE>) {
            qDebug() <<
               Q_FUNC_INFO << "Setting" << containingEntity->metaObject()->className() << "ID" <<
               containingEntity->key() << "on" << this->derived().m_namedEntity->metaObject()->className() << "#" <<
               this->derived().m_namedEntity->key();

            auto enumerated = std::static_pointer_cast<NE>(this->derived().m_namedEntity);
            auto owner = std::static_pointer_cast<typename NE::OwnerClass>(containingEntity);

            //
            // We need to set the owner here, but also set the correct step/sequence number (which is not stored in
            // BeerXML / BeerJSON and thus needs to be deduced from how many items have already been added).
            // StepOwnerBase::add() and Recipe::add(std::shared_ptr<Instruction>) both call OwnedSet::add(), which
            // handles doing both these things correctly.
            //
            owner->add(enumerated);
         }
         return;
      }

   };
}

/**
 * \brief Derived classes should include this in their header file, right after their constructor.  (Note that, since
 *        the derived class is itself templated, everything is in header files and there is no
 *        \c NAMED_ENTITY_RECORD_COMMON_CODE macro.
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define NAMED_ENTITY_RECORD_COMMON_DECL(Derived, NeName)                                             \
   /* This allows NamedEntityRecordBase to call protected and private members of Derived */          \
   friend class Serialization::NamedEntityRecordBase<Derived, NeName>;                               \
                                                                                                     \
   /* Need a virtual destructor as we have virtual member functions */                               \
   virtual ~Derived() = default;                                                                     \
                                                                                                     \
   protected:                                                                                        \
      virtual void constructNamedEntity() override { this->doConstructNamedEntity(); return; }       \
      virtual int storeNamedEntityInDb() override { return this->doStoreNamedEntityInDb(); }         \
   public:                                                                                           \
      virtual void deleteNamedEntityFromDb() override { this->doDeleteNamedEntityFromDb(); return; } \
      virtual bool includedInStats() const override { return this->doIncludedInStats(); }            \
   protected:                                                                                        \
      virtual bool resolveDuplicates() override { return this->doResolveDuplicates(); }              \
      virtual void normaliseName() override { this->doNormaliseName(); return; }                     \
      virtual void setContainingEntity(std::shared_ptr<NamedEntity> containingEntity) override {     \
         this->doSetContainingEntity(containingEntity); return;                                      \
      }                                                                                              \

#endif
