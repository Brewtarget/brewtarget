/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionBase.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef MODEL_RECIPEADDITIONBASE_H
#define MODEL_RECIPEADDITIONBASE_H
#pragma once

#include "database/ObjectStoreWrapper.h"
#include "utils/AutoCompare.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeTraits.h"

class RecipeAddition;
class RecipeUseOfWater;

//
// See comment in qtModels/tableModels/TableModelBase.h about benefits (and limitations) of using concepts.
//
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <typename T> concept CONCEPT_FIX_UP IsRegularAddition = std::is_base_of_v<RecipeAddition, T>;

/**
 * \brief Small template base class to provide templated code for recipe addition classes: \c RecipeAdditionHop,
 *        \c RecipeAdditionFermentable, \c RecipeAdditionMisc, \c RecipeAdditionYeast.
 *
 * \param Derived = the derived class, eg \c RecipeAdditionHop
 * \param Ingr    = the ingredient class, eg \c Hop
 *
 * NOTE: Derived classes need to have a static member function \c instanceNameTemplate for the translated name of class
 *       instances (eg `tr("Addition of %1 hops")` or `tr("Use of %1 water")`).  The declaration of this is handled by
 *       the \c RECIPE_ADDITION_DECL macro, but the trivial class-specific definition needs to done by hand.  Note that
 *       \c instanceNameTemplate is a member function rather than a member variable for the same reasons as
 *       \c localisedName.  See comment in \c model/NamedEntity.h.
 */
template<class Derived> class RecipeAdditionPhantom;
template<class Derived, class Ingr>
class RecipeAdditionBase : public CuriouslyRecurringTemplateBase<RecipeAdditionPhantom, Derived> {

protected:
   RecipeAdditionBase() {
      return;
   }

   ~RecipeAdditionBase() = default;

public:

   //
   // This alias makes it easier to template a number of functions that are essentially the same for all subclasses of
   // RecipeAddition.  Eg RecipeAdditionHop::IngredientClass is Hop; RecipeUseOfWater::IngredientClass is Water; etc.
   // Note that the alias and the template parameter cannot have the same name, hence why we use Ingr
   // for the latter.
   //
   using IngredientClass = Ingr;

   /**
    * \brief Create \c RecipeAddition##NeName objects for a given \c Recipe from \c NeName objects
    */
   static QList<std::shared_ptr<Derived>> create(Recipe & recipe, QList<Ingr *> ingredients) {
      QList<std::shared_ptr<Derived>> listOfAdditions;
      for (auto ingredient : ingredients) {
         auto addition = std::make_shared<Derived>(recipe, *ingredient);
         listOfAdditions.append(addition);
      }
      return listOfAdditions;
   }

   /**
    * \brief We define a canonical sort order for any given type of recipe addition.  This is useful when comparing
    *        lists of them (eg all the hop additions in two recipes) for equality.
    *
    *        Note that, because RecipeAdjustmentSalt has different fields, it needs a different implementation.
    *
    *        Of course, strictly, we should call this function doThreeWayComparisonOperator but doSpaceship has the
    *        merit of brevity, and everyone knows <=> as the spaceship operator.
    */
   std::strong_ordering doSpaceship(Derived const & other) const requires IsRegularAddition<Derived> {
      //
      // Note that Amounts will not necessarily be truly comparable -- eg one Misc addition might be measured by weight
      // and another by volume -- but we can assume they are in canonical units and we can make an arbitrary canonical
      // ordering even of units for different physical measurements.
      //
      // Note too the importance of including name as a tie-break for where we have identical amounts of two different
      // additions added at the same time (something that is relatively common).
      //
      Derived const & lhs {this->derived()};
      Derived const & rhs {other};
      return Utils::Auto3WayCompare(lhs.m_stage          , rhs.m_stage          ,
                                    lhs.m_step           , rhs.m_step           ,
                                    lhs.m_addAtTime_mins , rhs.m_addAtTime_mins ,
                                    lhs.m_amount         , rhs.m_amount         ,
                                    lhs.m_addAtGravity_sg, rhs.m_addAtGravity_sg,
                                    lhs.m_addAtAcidity_pH, rhs.m_addAtAcidity_pH,
                                    lhs.m_duration_mins  , rhs.m_duration_mins  ,
                                    lhs.name()           , rhs.name()           );
   }

   /**
    * \brief Arguably this belongs in \c RecipeAdjustmentSalt, but it's simpler to put it here since that's where we
    *        have the version for all the Recipe Additions.
    */
   std::strong_ordering doSpaceship(Derived const & other) const requires (!IsRegularAddition<Derived>) {
      // Comments in the other version of doSpaceship, above, apply equally here.
      auto const & ll {this->derived()};
      auto const & rr {other};
      // RecipeUseOfWater doesn't have m_whenToAdd or m_amount, but instead it has m_volume_l
      if constexpr (!std::same_as<Derived, RecipeUseOfWater>) {
         if (ll.m_whenToAdd != rr.m_whenToAdd) {
            return ll.m_whenToAdd < rr.m_whenToAdd ? std::strong_ordering::less : std::strong_ordering::greater;
         }
         if (ll.m_amount != rr.m_amount) {
            return ll.m_amount < rr.m_amount ? std::strong_ordering::less : std::strong_ordering::greater;
         }
      } else {
         if (ll.m_volume_l != rr.m_volume_l) {
            return ll.m_volume_l < rr.m_volume_l ? std::strong_ordering::less : std::strong_ordering::greater;
         }
      }
      int nameComparison {ll.name().compare(rr.name(), Qt::CaseInsensitive)};
      if (0 != nameComparison) {
         return nameComparison < 0 ? std::strong_ordering::less : std::strong_ordering::greater;
      }
      return std::strong_ordering::equal;
   }

   std::shared_ptr<Ingr> ingredient() const {
      return ObjectStoreWrapper::getById<Ingr>(this->derived().m_ingredientId);
   }

   /**
    * \brief Derived classes also return same info via functions with more friendly names (eg
    *        \c RecipeAdditionHop::hop(), \c RecipeUseOfWater::water()).  It would be neat to be able to just alias
    *        those names, but I'm not sure it's possible (because the CRTP base class is an incomplete type inside the
    *        derived class declaration).  So we use macros instead (see below).
    */
   Ingr * ingredientRaw() const {
      // Normally there should always be a valid Hop/Salt/etc in a RecipeAdjustmentHop/RecipeAdjustmentSalt/etc.  (The
      // Recipe ID may be -1 if the addition is only just about to be added to the Recipe or has just been removed from
      // it, but there's no great reason for the Hop/Salt/etc ID not to be valid).
      if (this->derived().m_ingredientId <= 0) {
         qWarning() <<
            Q_FUNC_INFO << "No" << Ingr::staticMetaObject.className() << "set on " <<
            Derived::staticMetaObject.className() << " #" << this->derived().key();
         return nullptr;
      }

      return ObjectStoreWrapper::getByIdRaw<Ingr>(this->derived().m_ingredientId);
   }

   /**
    * \brief As with \c ingredientRaw, it's the same deal for setters
    */
   void setIngredientRaw(Ingr * const val) {
      if (val) {
         this->derived().setIngredientId(val->key());
         this->derived().setName(Derived::tr("Addition of %1").arg(val->name()));
      } else {
         // Normally we don't want to invalidate the Ingredient on a RecipeAddition, because it doesn't buy us anything.
         qWarning() <<
            Q_FUNC_INFO << "Null" << Ingr::staticMetaObject.className() << "set on " <<
            Derived::staticMetaObject.className() << " #" << this->derived().key();
         this->derived().setIngredientId(-1);
         this->derived().setName(Derived::tr("Invalid!"));
      }
      return;
   }


};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        We can't derive RecipeAddition class name from ingredient name (eg RecipeAdditionHop from Hop) because we
 *        intentionally break the naming pattern with Salt (RecipeAdjustmentSalt).  Hence why these macros take two
 *        parameters.
 */
#define RECIPE_ADDITION_DECL(RaIngrd, Ingrd, ingrd) \
   /* This allows RecipeAdditionBase to call protected and private members of Derived */   \
   friend class RecipeAdditionBase<RaIngrd, Ingrd>;                                        \
                                                                                           \
   public:                                                                                 \
   RaIngrd(Recipe & recipe, Ingrd & ne);                                                   \
   std::strong_ordering operator<=>(RaIngrd const & other) const;                          \
   static QString instanceNameTemplate();                                                  \
   Ingrd * ingrd() const;                                                                  \
   void set##Ingrd(Ingrd * const val);                                                     \

/**
 * \brief Derived classes should include this in their implementation file.
 *
 *        They also need to #include "model/Recipe.h"
 */
#define RECIPE_ADDITION_CODE(RaIngrd, Ingrd, ingrd) \
   RaIngrd::RaIngrd(Recipe & recipe, Ingrd & ne) :                           \
     RaIngrd::RaIngrd{RaIngrd::instanceNameTemplate().arg(ne.name()),        \
                      recipe.key(),                                          \
                      ne.key()} {                                            \
     return;                                                                 \
   }                                                                         \
   std::strong_ordering RaIngrd::operator<=>(RaIngrd const & other) const {  \
      return this->doSpaceship(other);                                       \
   }                                                                         \
   Ingrd * RaIngrd::ingrd() const { return this->ingredientRaw(); }          \
   void RaIngrd::set##Ingrd(Ingrd * const val) {                             \
      this->setIngredientRaw(val); return;                                   \
   }                                                                         \

#endif
