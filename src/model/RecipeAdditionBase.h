/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionBase.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeTraits.h"

class RecipeAddition;

//
// See comment in tableModels/TableModelBase.h about benefits (and limitations) of using concepts.
//
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <typename T> concept CONCEPT_FIX_UP IsRegularAddition = std::is_base_of_v<RecipeAddition, T>;

namespace {
   template<typename T>
   std::strong_ordering compare3way(T const & lhs, T const & rhs) {
      return lhs == rhs ? std::strong_ordering::equal : (lhs < rhs ? std::strong_ordering::less :
                                                                     std::strong_ordering::greater);
   }
}

/**
 * \brief Small template base class to provide templated code for recipe addition classes: \c RecipeAdditionHop,
 *        \c RecipeAdditionFermentable, \c RecipeAdditionMisc, \c RecipeAdditionYeast.
 *
 * \param Derived = the derived class, eg \c RecipeAdditionHop
 * \param Ingr    = the ingredient class, eg \c Hop
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
   // RecipeAddition.  Note that the alias and the template parameter cannot have the same name, hence why we use Ingr
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
      // We want a multi-level comparison for the spaceship operator.  Often the recommendation for this sort of thing
      // is to use a tuple (via std::tie), since operator<=> is already well-defined for tuples.  However, there are
      // limitations, eg you can't put an rvalue in a tuple, that make it a bit fiddly here, so we do old-school
      // if/else statements.
      //
      // Note that Amounts will not necessarily be truly comparable -- eg one Misc addition might be measured by weight
      // and another by volume -- but we can assume they are in canonical units and we can make an arbitrary canonical
      // ordering even of units for different physical measurements.
      //
      auto const & ll {this->derived()};
      auto const & rr {other};
      std::strong_ordering result {std::strong_ordering::equivalent};
      result = compare3way(ll.m_stage          , rr.m_stage          ); if (result != std::strong_ordering::equal) { return result; }
      result = compare3way(ll.m_step           , rr.m_step           ); if (result != std::strong_ordering::equal) { return result; }
      result = compare3way(ll.m_addAtTime_mins , rr.m_addAtTime_mins ); if (result != std::strong_ordering::equal) { return result; }
      result = compare3way(ll.m_amount         , rr.m_amount         ); if (result != std::strong_ordering::equal) { return result; }
      result = compare3way(ll.m_addAtGravity_sg, rr.m_addAtGravity_sg); if (result != std::strong_ordering::equal) { return result; }
      result = compare3way(ll.m_addAtAcidity_pH, rr.m_addAtAcidity_pH); if (result != std::strong_ordering::equal) { return result; }
      result = compare3way(ll.m_duration_mins  , rr.m_duration_mins  ); if (result != std::strong_ordering::equal) { return result; }
      return result;
   }

   /**
    * \brief Arguably this belongs in \c RecipeAdjustmentSalt, but it's simpler to put it here since that's where we
    *        have the version for all the Recipe Additions.
    */
   std::strong_ordering doSpaceship(Derived const & other) const requires (!IsRegularAddition<Derived>) {
      // Comments in the other version of doSpaceship, above, apply equally here.
      auto const & ll {this->derived()};
      auto const & rr {other};
      if (ll.m_whenToAdd != rr.m_whenToAdd) {
         return ll.m_whenToAdd < rr.m_whenToAdd ? std::strong_ordering::less : std::strong_ordering::greater;
      }
      if (ll.m_amount != rr.m_amount) {
         return ll.m_amount < rr.m_amount ? std::strong_ordering::less : std::strong_ordering::greater;
      }
      int nameComparison {ll.name().compare(rr.name(), Qt::CaseInsensitive)};
      if (0 != nameComparison) {
         return nameComparison < 0 ? std::strong_ordering::less : std::strong_ordering::greater;
      }
      return std::strong_ordering::equal;
   }

   std::shared_ptr<Ingr> ingredient() {
      return ObjectStoreWrapper::getById<Ingr>(this->derived().m_ingredientId);
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
#define RECIPE_ADDITION_DECL(RaIngrd, Ingrd) \
   /* This allows RecipeAdditionBase to call protected and private members of Derived */   \
   friend class RecipeAdditionBase<RaIngrd, Ingrd>;                                        \
                                                                                           \
   public:                                                                                 \
   RaIngrd(Recipe & recipe, Ingrd & ne);                                                   \
   std::strong_ordering operator<=>(RaIngrd const & other) const;                          \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define RECIPE_ADDITION_CODE(RaIngrd, Ingrd) \
   RaIngrd::RaIngrd(Recipe & recipe, Ingrd & ne) :                           \
     RaIngrd::RaIngrd{QString(tr("Add %1").arg(ne.name())),                  \
                      recipe.key(),                                          \
                      ne.key()} {                                            \
     return;                                                                 \
   }                                                                         \
   std::strong_ordering RaIngrd::operator<=>(RaIngrd const & other) const {  \
      return this->doSpaceship(other);                                       \
   }                                                                         \

#endif
