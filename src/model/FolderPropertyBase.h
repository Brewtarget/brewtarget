/*======================================================================================================================
 * model/FolderPropertyBase.h is part of Brewtarget, and is copyright the following authors 2024-2026:
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
 =====================================================================================================================*/
#ifndef MODEL_FOLDERPROPERTYBASE_H
#define MODEL_FOLDERPROPERTYBASE_H
#pragma once

#include <QString>

// NB: Derived class .cpp file needs to #include "database/ObjectStoreWrapper.h".  (We can't do that include in this
//     header, otherwise we end up with circular dependencies.)
#include "model/NamedParameterBundle.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeLookup.h"

template<class NE> class Folder;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::FolderPropertyBase { inline BtStringConst const property{#property}; }
AddPropertyName(containedInFolder   )
AddPropertyName(containedInFolderId )
AddPropertyName(containedInFolderRaw)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief This is a minimal enum to make the second template parameter of \c FolderPropertyBase more self-describing.
 */
enum class IsFolder { Yes, No };

/**
 * \brief Adds "containedInFolder" and "containedInFolderId" properties to a class.  Typically, this is used only on
 *        classes that are not dependent on others (see comments in model/NamedEntity.h).
 *
 *        NOTE that the long-winded name is intentional to try to avoid ambiguity.  Folders themselves have a
 *        "containedInFolder" and "containedInFolderId" properties, because one folder can be inside another.  If we
 *        used more concise names such as "folder" and "folderId", it could be confusing that myFolder.folderId() is
 *        not the ID (ie key) of myFolder itself.  The hope is that using the longer names makes it more obvious that,
 *        if folder \c bar is inside folder \c foo then bar.containedInFolderId() == foo.key().
 */
template<class Derived> class FolderPropertyBasePhantom;
template<class Derived, IsFolder DerivedIsFolder = IsFolder::No>
class FolderPropertyBase : public CuriouslyRecurringTemplateBase<FolderPropertyBasePhantom, Derived> {
   /**
    * For something that is not a \c Folder, eg a \c Hop, its folder type is based it being the parameter to the
    * \c Folder template class, eg Folder<Hop>.  But, for something that is already a \c Folder, eg \c Folder<Hop>, this
    * would be wrong; in such cases, the class is already its own folder type.  Rather than keep working this out
    * everywhere, we make an alias here.
    *
    * Our first instinct might be to have the compiler figure everything out by writing, say:
    *
    *    using FolderType = std::conditional_t<std::is_base_of_v<FolderCommon, Derived>, Derived, Folder<Derived>>
    *
    * However, this isn't going to fly because, at this point, Derived is not fully defined (because we are a CRTP base
    * of Defined).  So, instead, we use a template parameter to determine things manually.
    */
   using FolderType = std::conditional_t<DerivedIsFolder == IsFolder::Yes, Derived, Folder<Derived>>;

protected:
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   //! Non-virtual equivalent of compareWith
   bool doCompareWith([[maybe_unused]] FolderPropertyBase const & other,
                      [[maybe_unused]] QList<BtStringConst const *> * propertiesThatDiffer) const {
      // With one exception, for the moment at least, we do not consider the fact that things are in different folders
      // prevents them from being equal.  The exception is that, for Folders themselves, we do care about where in the
      // folder tree something is and, in general, two folders with the same name etc are _NOT_ equal unless they are
      // the same folder (ie same key).
      if constexpr (DerivedIsFolder == IsFolder::No) {
         return true;
      } else {
         // Base class (NamedEntity) will have ensured this cast is valid
         auto const & rhs = static_cast<FolderPropertyBase const &>(other);
         return AUTO_PROPERTY_COMPARE(this, rhs, m_containedInFolderId  , PropertyNames::FolderPropertyBase::containedInFolderId, propertiesThatDiffer);
      }
   }

private:
   friend Derived;
   FolderPropertyBase() = default;

   explicit FolderPropertyBase(NamedParameterBundle const & namedParameterBundle) :
      SET_REGULAR_FROM_NPB(m_containedInFolderId, namedParameterBundle, PropertyNames::FolderPropertyBase::containedInFolderId, -1) {
      return;
   }

   FolderPropertyBase(FolderPropertyBase const & other) :
      m_containedInFolderId{other.m_containedInFolderId} {
      return;
   }

   ~FolderPropertyBase() = default;

public:
   FolderType * containedInFolderRaw() const {
      return FolderType::getByIdRaw(this->m_containedInFolderId);
   }

   std::shared_ptr<FolderType> containedInFolder() const {
      return FolderType::getById(this->m_containedInFolderId);
   }

   void setContainedInFolder(FolderType                  const * val) { this->doSetContainedInFolderId(val ? val->key() : -1); }
   void setContainedInFolder(std::shared_ptr<FolderType> const & val) { this->doSetContainedInFolderId(val ? val->key() : -1); }
   void setContainedInFolder(FolderType                  const & val) { this->doSetContainedInFolderId(val->key()); }

   static QString localisedName_containedInFolder  () { return Derived::tr("Contained in Folder"   ); }
   static QString localisedName_containedInFolderId() { return Derived::tr("Contained in Folder ID"); }

protected:
   [[nodiscard]] int doContainedInFolderId() const {
      return this->m_containedInFolderId;
   }

   void doSetContainedInFolderId(int const val) {
      this->derived().setAndNotify(PropertyNames::FolderPropertyBase::containedInFolderId, this->m_containedInFolderId, val);
      Q_ASSERT(this->m_containedInFolderId == val);
      return;
   }

protected:
   //
   // Note that, even for a Folder, this is the ID of the containing folder (what we call "parent_id" in the database
   // tables).
   //
   int m_containedInFolderId = -1;
};

template<class Derived, IsFolder DerivedIsFolder>
TypeLookup const FolderPropertyBase<Derived, DerivedIsFolder>::typeLookup {
   "FolderPropertyBase",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::FolderPropertyBase::containedInFolderId,
       TypeInfo::construct<decltype(FolderPropertyBase<Derived>::m_containedInFolderId)>(
          PropertyNames::FolderPropertyBase::containedInFolderId,
          FolderPropertyBase::localisedName_containedInFolderId,
          TypeLookupOf<decltype(FolderPropertyBase<Derived>::m_containedInFolderId)>::value
       )}
   },
   // Parent class lookup: none as we are at the top of this branch of the inheritance tree
   {}
};


// Forward declaration for concepts below
class FolderCommon;

/**
 *\brief It is useful to be able to constrain some template parameters as to whether or not they are folders.
 */
template <typename T> concept CONCEPT_FIX_UP FolderClass    = std::is_base_of_v<FolderCommon, T>;
template <typename T> concept CONCEPT_FIX_UP NonFolderClass = std::negation_v<std::is_base_of<FolderCommon, T>>;

/**
 * \brief For some templated functions, it's useful at compile time to have one version for NE classes with folders and
 *        one for those without.  We need to put the concepts here in the base class for them to be accessible.
 *
 *        See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it).
 */
template <typename T> concept CONCEPT_FIX_UP HasFolder   = (
   std::is_base_of_v<FolderCommon, T> || std::is_base_of_v<FolderPropertyBase<T>, T>
);
template <typename T> concept CONCEPT_FIX_UP HasNoFolder = (
   std::negation_v<std::is_base_of<FolderCommon, T>> && std::negation_v<std::is_base_of<FolderPropertyBase<T, IsFolder::No>, T>>
);


/**
 * \brief We sometimes need to combine these concepts
 */
template <typename T> concept CONCEPT_FIX_UP ValidFolderType = (HasFolder<T> && NonFolderClass<T>);


/**
 * \brief Except for Folder classes themselves, concrete derived classes should (either directly or via inclusion in an
 *        intermediate class's equivalent macro) include this in their header file, right after Q_OBJECT.  Concrete
 *        derived classes also need to include the following block (see comment in model/StepBase.h for why):
 *
 *           // See model/FolderPropertyBase.h for info, getters and setters for these properties
 *           Q_PROPERTY(int containedInFolderId        READ containedInFolderId        WRITE setFolderId)
 *
 *        We can't use this macro for Folder<NE> classes because templated classes need their member function
 *        declarations and definitions combined.
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define FOLDER_BASE_DECL(Derived) \
   /* This allows FolderPropertyBase to call protected and private members of Derived. */ \
   /* Note that a friend statement can either apply to all instances of                */ \
   /* IngredientAmount or to one specialisation.  It cannot apply to a partial         */ \
   /* specialisation.  Hence why we need to specify IsFolder here.                     */ \
   friend class FolderPropertyBase<Derived, IsFolder::No>;                                \
                                                                                          \
   public:                                                                                     \
      /*=========================== FB "GETTER" MEMBER FUNCTIONS ===========================*/ \
      [[nodiscard]] int containedInFolderId() const;                                           \
      /*=========================== FB "SETTER" MEMBER FUNCTIONS ===========================*/ \
      void setContainedInFolderId(int const val);                                              \
      /* We need these to avoid circular dependencies with ObjectStoreWrapper */               \
      static std::shared_ptr<Derived> getById(int const id);                                   \
      static Derived * getByIdRaw(int const id);                                               \

/**
 * \brief Except for Folder classes themselves, concrete derived classes should include this in their .cpp file
 *
 *        NOTE that we call ObjectStoreWrapper functions directly here, as trying to do it from a FolderPropertyBase
 *        function leads to circular dependencies.
 *
 *        We don't use this macro for Folder<NE> classes as we need some minor tweaks there that aren't worth the
 *        complexity of handling in the macro.
 *
 *        NOTE that derived class .cpp file needs to #include "database/ObjectStoreWrapper.h".  (We can't do that
 *        include in this header, otherwise we end up with circular dependencies.)
 *
 *        Note we have to be careful about comment formats in macro definitions.
 */
#define FOLDER_BASE_COMMON_CODE(Derived) \
   /*====================================== FB "GETTER" MEMBER FUNCTIONS ======================================*/ \
   int Derived::containedInFolderId() const { return this->doContainedInFolderId(); }                             \
   /*====================================== FB "SETTER" MEMBER FUNCTIONS ======================================*/ \
   void Derived::setContainedInFolderId(int const val) { this->doSetContainedInFolderId(val); return; }           \
                                                                                                                  \
   std::shared_ptr<Derived> Derived::getById(int const id) { return ObjectStoreWrapper::getById<Derived>(id); }   \
   Derived * Derived::getByIdRaw(int const id) { return ObjectStoreWrapper::getByIdRaw<Derived>(id); }            \

#endif