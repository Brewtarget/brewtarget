/*======================================================================================================================
 * model/LockablePropertyBase.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef MODEL_LOCKABLEPROPERTYBASE_H
#define MODEL_LOCKABLEPROPERTYBASE_H
#pragma once

#include <concepts>

#include <QString>

#include "model/NamedParameterBundle.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeLookup.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::LockablePropertyBase { inline BtStringConst const property{#property}; }
AddPropertyName(locked)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Adds a "locked" property to a class.  This is used for things that can be shared between \c Recipe objects,
 *        currently \c Mash, \c Boil and \c Fermentation.  (\c Recipe has its own \c locked property, which predates
 *        this class.)
 *
 *        A locked item cannot be changed or deleted.  If the user wants to change it, they either make a copy (which
 *        is not locked) or explicitly unlock it (which the UI makes deliberately hard to do by accident).  See
 *        \c RecipeComponentUtils for the related idea that an item used by a locked \c Recipe is also, in effect,
 *        locked.
 *
 *        Typical uses are:
 *          - sample mash profiles etc that we ship as default data, so that users can use them as-is in as many
 *            recipes as they like, but get prompted to make a copy as soon as they want to change something;
 *          - the user's own "standard" mash profiles etc that they want to protect from accidental changes.
 *
 *        Note that "locked" is about what the user can change via the UI.  The model does not stop code changing a
 *        locked item (eg when reading data in from the database or a file).
 */
template<class Derived> class LockablePropertyBasePhantom;
template<class Derived>
class LockablePropertyBase : public CuriouslyRecurringTemplateBase<LockablePropertyBasePhantom, Derived> {
protected:
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   /**
    * \brief Non-virtual equivalent of compareWith.  We deliberately do not compare the locked flag: two mashes with the
    *        same steps etc are the same mash whether or not one of them is locked.  (Otherwise, eg, importing a recipe
    *        that uses one of our locked sample mash profiles would create a duplicate of it.)
    */
   bool doCompareWith([[maybe_unused]] LockablePropertyBase const & other,
                      [[maybe_unused]] QList<BtStringConst const *> * propertiesThatDiffer) const {
      return true;
   }

private:
   friend Derived;
   LockablePropertyBase() = default;

   explicit LockablePropertyBase(NamedParameterBundle const & namedParameterBundle) :
      SET_REGULAR_FROM_NPB(m_locked, namedParameterBundle, PropertyNames::LockablePropertyBase::locked, false) {
      return;
   }

   /**
    * \brief Copying an item gives an unlocked copy.  The whole point of making a copy of a locked item is usually so
    *        that the copy can be changed.
    */
   LockablePropertyBase([[maybe_unused]] LockablePropertyBase const & other) :
      m_locked{false} {
      return;
   }

   ~LockablePropertyBase() = default;

public:
   static QString localisedName_locked() { return Derived::tr("Locked"); }

protected:
   [[nodiscard]] bool doLocked() const {
      return this->m_locked;
   }

   void doSetLocked(bool const val) {
      this->derived().setAndNotify(PropertyNames::LockablePropertyBase::locked, this->m_locked, val);
      return;
   }

   bool m_locked = false;
};

template<class Derived>
TypeLookup const LockablePropertyBase<Derived>::typeLookup {
   "LockablePropertyBase",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::LockablePropertyBase::locked,
       TypeInfo::construct<decltype(LockablePropertyBase<Derived>::m_locked)>(
          PropertyNames::LockablePropertyBase::locked,
          LockablePropertyBase::localisedName_locked,
          TypeLookupOf<decltype(LockablePropertyBase<Derived>::m_locked)>::value
       )},
   },
   // Parent class lookup: none as we are at the top of this branch of the inheritance tree
   {}
};

/**
 * \brief Anything with a \c locked() member function -- ie classes inheriting from \c LockablePropertyBase, and
 *        \c Recipe.  Useful for things like disabling "Delete" for locked items in trees and catalogs.
 */
template <typename T> concept CONCEPT_FIX_UP IsLockable = requires (T const & t) {
   { t.locked() } -> std::convertible_to<bool>;
};

/**
 * \brief Classes inheriting from \c LockablePropertyBase -- ie things that can be shared between Recipes and locked.
 *        Used, eg, in \c EditorBase to decide whether to show a \c LockAndShareBar.
 */
template <typename T> concept CONCEPT_FIX_UP HasLockablePropertyBase = std::is_base_of_v<LockablePropertyBase<T>, T>;

/**
 * \brief Concrete derived classes should include this in their header file, right after Q_OBJECT.  They also need to
 *        include the following (see comment in model/StepBase.h for why):
 *           // See model/LockablePropertyBase.h for info, getters and setters for this property
 *           Q_PROPERTY(bool locked READ locked WRITE setLocked)
 */
#define LOCKABLE_BASE_DECL(Derived) \
   /* This allows LockablePropertyBase to call protected and private members of Derived. */ \
   friend class LockablePropertyBase<Derived>;                                               \
                                                                                             \
   public:                                                                                   \
      [[nodiscard]] bool locked() const;                                                     \
      void setLocked(bool const val);                                                        \

/**
 * \brief Concrete derived classes should include this in their .cpp file
 */
#define LOCKABLE_BASE_COMMON_CODE(Derived) \
   bool Derived::locked() const { return this->doLocked(); }                             \
   void Derived::setLocked(bool const val) { this->doSetLocked(val); return; }           \

#endif
