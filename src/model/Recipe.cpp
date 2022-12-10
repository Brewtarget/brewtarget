/*
 * model/Recipe.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "model/Recipe.h"

#include <cmath> // For pow/log

#include <QDate>
#include <QDebug>
#include <QInputDialog>
#include <QList>
#include <QObject>

#include "Algorithms.h"
#include "database/ObjectStoreWrapper.h"
#include "HeatCalculations.h"
#include "Localization.h"
#include "measurement/ColorMethods.h"
#include "measurement/IbuMethods.h"
#include "measurement/Measurement.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/NamedParameterBundle.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"
#include "PhysicalConstants.h"
#include "PreInstruction.h"

namespace {
   /**
    * \brief Check whether the supplied instance of (subclass of) NamedEntity (a) is an "instance of use of" (ie has a
    *        parent) and (b) is not used in any Recipe.
    */
   template<class NE> bool isUnusedInstanceOfUseOf(NE & var) {
      NE * parentOfVar = static_cast<NE *>(var.getParent());
      if (nullptr == parentOfVar) {
         // The var has no parent and so is not "an instance of use of"
         return false;
      }

      qDebug() <<
         Q_FUNC_INFO << var.metaObject()->className() << "#" << var.key() << "has parent #" << parentOfVar->key();
      //
      // Parameter has a parent.  See if it (the parameter, not its parent!) is used in a recipe.
      // (NB: The parent of the NamedEntity is not the same thing as its parent recipe.  We should perhaps find some
      // different terms!)
      //
      auto matchingRecipe = ObjectStoreTyped<Recipe>::getInstance().findFirstMatching(
         // NB: Important to do the lambda capture of var here by reference, otherwise we'll be passing in a copy of
         //     var, which won't have an ID and therefore will never give a match.
         [&var](Recipe * recipe) {
            return recipe->uses(var);
         }
      );
      if (matchingRecipe == nullptr) {
         // The parameter is not already used in a recipe, so we'll be able to add it without making a copy
         // Note that we can't just take the address of var and use it to make a new shared_ptr as that would mean
         // we had two completely unrelated shared_ptr objects (one in the object store and one newly created here)
         // pointing to the same address.  We need to get an instance of shared_ptr that's copied from (and thus
         // shares the internal reference count of) the one held by the object store.
         qDebug() << Q_FUNC_INFO << var.metaObject()->className() << "#" << var.key() << "not used in any recipe";
         return true;
      }

      // The var is used in another Recipe.  (We shouldn't really find ourselves in this position, but the way the rest
      // of the code works means that, even if we do, we should recover OK - or at least not make the situation any
      // worse.)
      qWarning() <<
         Q_FUNC_INFO << var.metaObject()->className() << "#" << var.key() <<
         "is unexpectedly already used in recipe #" << matchingRecipe->key();
      return false;
   }

   /**
    * \brief Decide whether the supplied instance of (subclass of) NamedEntity needs to be copied before being added to
    *        a recipe.
    *
    * \param var The Hop/Fermentable/etc that we want to add to a Recipe.  We'll either add it directly or make a copy
    *            of it and add that.
    *
    * \return A copy of var if it needs to be copied (either because it has no parent or because it is already used in
    *         another recipe), or var itself otherwise
    */
   template<class NE> std::shared_ptr<NE> copyIfNeeded(NE & var) {
      // It's the caller's responsibility to ensure var is already in an ObjectStore
      Q_ASSERT(var.key() > 0);

      //
      // If the supplied Hop/Fermentable/etc has no parent then we need to make a copy of it, because it's the master
      // instance of that Hop/Fermentable/etc.
      //
      // Otherwise, if it has a parent, then whether we need to make a copy depends on whether it is already used in a
      // recipe (_including_ this one, because the same ingredient can be added more than once to a recipe - eg Hops
      // added at different times).
      //
      // All this logic is handled in isUnusedInstanceOfUseOf() because it's the same process for checking it's OK to
      // delete something when it's been removed from a Recipe.
      //
      if (isUnusedInstanceOfUseOf(var)) {
         return ObjectStoreWrapper::getById<NE>(var.key());
      }

      qDebug() << Q_FUNC_INFO << "Making copy of " << var.metaObject()->className() << "#" << var.key();

      // We need to make a copy...
      auto copy = std::make_shared<NE>(var);
      // ...then make sure the copy is a "child" (ie "instance of use of")...
      copy->makeChild(var);
      // ...and finally ensure the copy is stored.
      ObjectStoreWrapper::insert(copy);
      return copy;
   }

   //
   // After we modified a property via a templated member function of Recipe, we need to tell the object store to
   // update the database.  These template specialisations map from property type to property name.
   //
   template<class NE> BtStringConst const & propertyToPropertyName();
   template<> BtStringConst const & propertyToPropertyName<Equipment>()   {
      return PropertyNames::Recipe::equipmentId;
   }
   template<> BtStringConst const & propertyToPropertyName<Fermentable>() {
      return PropertyNames::Recipe::fermentableIds;
   }
   template<> BtStringConst const & propertyToPropertyName<Hop>()         {
      return PropertyNames::Recipe::hopIds;
   }
   template<> BtStringConst const & propertyToPropertyName<Instruction>() {
      return PropertyNames::Recipe::instructionIds;
   }
   template<> BtStringConst const & propertyToPropertyName<Mash>()        {
      return PropertyNames::Recipe::mashId;
   }
   template<> BtStringConst const & propertyToPropertyName<Misc>()        {
      return PropertyNames::Recipe::miscIds;
   }
   template<> BtStringConst const & propertyToPropertyName<Salt>()        {
      return PropertyNames::Recipe::saltIds;
   }
   template<> BtStringConst const & propertyToPropertyName<Style>()       {
      return PropertyNames::Recipe::styleId;
   }
   template<> BtStringConst const & propertyToPropertyName<Water>()       {
      return PropertyNames::Recipe::waterIds;
   }
   template<> BtStringConst const & propertyToPropertyName<Yeast>()       {
      return PropertyNames::Recipe::yeastIds;
   }

   QHash<QString, Recipe::Type> const RECIPE_TYPE_STRING_TO_TYPE {
      {"Extract",      Recipe::Type::Extract},
      {"Partial Mash", Recipe::Type::PartialMash},
      {"All Grain",    Recipe::Type::AllGrain}
   };
}


// This private implementation class holds all private non-virtual members of Recipe
class Recipe::impl {
public:

   /**
    * Constructor
    */
   impl(Recipe & recipe) :
      recipe{recipe},
      fermentableIds{},
      hopIds{},
      instructionIds{},
      miscIds{},
      saltIds{},
      waterIds{},
      yeastIds{} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * \brief Make copies of the ingredients of a particular type (Hop, Fermentable, etc) from one Recipe and add them
    *        to another - typically because we are copying the Recipe.
    */
   template<class NE> void copyList(Recipe & us, Recipe const & other) {
      qDebug() << Q_FUNC_INFO;
      for (int otherIngId : other.pimpl->accessIds<NE>()) {
         // Make and store a copy of the current Hop/Fermentable/etc object we're looking at in the other Recipe
         auto otherIngredient = ObjectStoreWrapper::getById<NE>(otherIngId);
         auto ourIngredient = copyIfNeeded(*otherIngredient);
         // Store the ID of the copy in our recipe
         this->accessIds<NE>().append(ourIngredient->key());

         qDebug() <<
            Q_FUNC_INFO << "After adding" << ourIngredient->metaObject()->className() << "#" << ourIngredient->key() <<
            ", Recipe" << us.name() << "has" << this->accessIds<NE>().size() << "of" <<
            NE::staticMetaObject.className();

         // Connect signals so that we are notified when there are changes to the Hop/Fermentable/etc we just added to
         // our recipe.
         connect(ourIngredient.get(), &NamedEntity::changed, &us, &Recipe::acceptChangeToContainedObject);
      }
      return;
   }

   /**
    * \brief If the Recipe is about to be deleted, we delete all the things that belong to it.  Note that, with the
    *        exception of Instruction, what we are actually deleting here is not the Hops/Fermentables/etc but the "use
    *        of" Hops/Fermentables/etc records (which are distinguished by having a parent ID.
    */
   template<class NE> void hardDeleteAllMy() {
      qDebug() << Q_FUNC_INFO;
      for (auto id : this->accessIds<NE>()) {
         ObjectStoreWrapper::hardDelete<NE>(id);
      }
      return;
   }

   //
   // Inside the class implementation, it's useful to be able to access fermentableIds, hopIds, etc in templated
   // functions.  This allows us to write this->accessIds<NE>() in such a function and have it resolve to
   // this->accessIds<Fermentable>(), this->accessIds<Hop>(), etc, which in turn returns this->fermentableIds,
   // this->hopIds.
   //
   // Note that the specialisations need to be defined outside the class
   //
   template<class NE> QVector<int> & accessIds();

   /**
    * \brief Get shared pointers to all ingredients etc of a particular type (Hop, Fermentable, etc) in this Recipe
    */
   template<class NE> QList< std::shared_ptr<NE> > getAllMy() {
      return ObjectStoreTyped<NE>::getInstance().getByIds(this->accessIds<NE>());
   }

   /**
    * \brief Get raw pointers to all ingredients etc of a particular type (Hop, Fermentable, etc) in this Recipe
    */
   template<class NE> QList<NE *> getAllMyRaw() {
      return ObjectStoreTyped<NE>::getInstance().getByIdsRaw(this->accessIds<NE>());
   }

   /**
    * \brief Connect signals for this Recipe.  See comment for \c Recipe::connectSignalsForAllRecipes for more
    *        explanation.
    */
   void connectSignals() {
      Equipment * equipment = this->recipe.equipment();
      if (equipment) {
         connect(equipment, &NamedEntity::changed,           &this->recipe, &Recipe::acceptChangeToContainedObject);
         connect(equipment, &Equipment::changedBoilSize_l,   &this->recipe, &Recipe::setBoilSize_l);
         connect(equipment, &Equipment::changedBoilTime_min, &this->recipe, &Recipe::setBoilTime_min);
      }

      QList<Fermentable *> fermentables = this->recipe.fermentables();
      for (auto fermentable : fermentables) {
         connect(fermentable, &NamedEntity::changed, &this->recipe, &Recipe::acceptChangeToContainedObject);
      }

      QList<Hop *> hops = this->recipe.hops();
      for (auto hop : hops) {
         connect(hop, &NamedEntity::changed, &this->recipe, &Recipe::acceptChangeToContainedObject);
      }

      QList<Yeast *> yeasts = this->recipe.yeasts();
      for (auto yeast : yeasts) {
         connect(yeast, &NamedEntity::changed, &this->recipe, &Recipe::acceptChangeToContainedObject);
      }

      Mash * mash = this->recipe.mash();
      if (mash) {
         connect(mash, &NamedEntity::changed, &this->recipe, &Recipe::acceptChangeToContainedObject);
      }

      return;
   }

   // Member variables
   Recipe & recipe;
   QVector<int> fermentableIds;
   QVector<int> hopIds;
   QVector<int> instructionIds;
   QVector<int> miscIds;
   QVector<int> saltIds;
   QVector<int> waterIds;
   QVector<int> yeastIds;

};

template<> QVector<int> & Recipe::impl::accessIds<Fermentable>() { return this->fermentableIds; }
template<> QVector<int> & Recipe::impl::accessIds<Hop>()         { return this->hopIds; }
template<> QVector<int> & Recipe::impl::accessIds<Instruction>() { return this->instructionIds; }
template<> QVector<int> & Recipe::impl::accessIds<Misc>()        { return this->miscIds; }
template<> QVector<int> & Recipe::impl::accessIds<Salt>()        { return this->saltIds; }
template<> QVector<int> & Recipe::impl::accessIds<Water>()       { return this->waterIds; }
template<> QVector<int> & Recipe::impl::accessIds<Yeast>()       { return this->yeastIds; }

bool Recipe::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Recipe const & rhs = static_cast<Recipe const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type              == rhs.m_type              &&
      this->m_batchSize_l       == rhs.m_batchSize_l       &&
      this->m_boilSize_l        == rhs.m_boilSize_l        &&
      this->m_boilTime_min      == rhs.m_boilTime_min      &&
      this->m_efficiency_pct    == rhs.m_efficiency_pct    &&
      this->m_primaryAge_days   == rhs.m_primaryAge_days   &&
      this->m_primaryTemp_c     == rhs.m_primaryTemp_c     &&
      this->m_secondaryAge_days == rhs.m_secondaryAge_days &&
      this->m_secondaryTemp_c   == rhs.m_secondaryTemp_c   &&
      this->m_tertiaryAge_days  == rhs.m_tertiaryAge_days  &&
      this->m_tertiaryTemp_c    == rhs.m_tertiaryTemp_c    &&
      this->m_age               == rhs.m_age               &&
      this->m_ageTemp_c         == rhs.m_ageTemp_c         &&
      ObjectStoreWrapper::compareById<Style>(    this->styleId,     rhs.styleId)     &&
      ObjectStoreWrapper::compareById<Mash>(     this->mashId,      rhs.mashId)      &&
      ObjectStoreWrapper::compareById<Equipment>(this->equipmentId, rhs.equipmentId) &&
      this->m_og                == rhs.m_og                &&
      this->m_fg                == rhs.m_fg                &&
      ObjectStoreWrapper::compareListByIds<Fermentable>(this->pimpl->fermentableIds, rhs.pimpl->fermentableIds) &&
      ObjectStoreWrapper::compareListByIds<Hop>(        this->pimpl->hopIds,         rhs.pimpl->hopIds)         &&
      ObjectStoreWrapper::compareListByIds<Instruction>(this->pimpl->instructionIds, rhs.pimpl->instructionIds) &&
      ObjectStoreWrapper::compareListByIds<Misc>(       this->pimpl->miscIds,        rhs.pimpl->miscIds)        &&
      ObjectStoreWrapper::compareListByIds<Salt>(       this->pimpl->saltIds,        rhs.pimpl->saltIds)        &&
      ObjectStoreWrapper::compareListByIds<Water>(      this->pimpl->waterIds,       rhs.pimpl->waterIds)       &&
      ObjectStoreWrapper::compareListByIds<Yeast>(      this->pimpl->yeastIds,       rhs.pimpl->yeastIds)
   );
}

ObjectStore & Recipe::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Recipe>::getInstance();
}

Recipe::Recipe(QString name) :
   NamedEntity         {name, true                   },
   pimpl               {std::make_unique<impl>(*this)},
   m_type              {"All Grain"                  },
   m_brewer            {""                           },
   m_asstBrewer        {"Brewtarget: free beer software"},
   m_batchSize_l       {0.0                          },
   m_boilSize_l        {0.0                          },
   m_boilTime_min      {0.0                          },
   m_efficiency_pct    {0.0                          },
   m_fermentationStages{1                            },
   m_primaryAge_days   {0.0                          },
   m_primaryTemp_c     {0.0                          },
   m_secondaryAge_days {0.0                          },
   m_secondaryTemp_c   {0.0                          },
   m_tertiaryAge_days  {0.0                          },
   m_tertiaryTemp_c    {0.0                          },
   m_age               {0.0                          },
   m_ageTemp_c         {0.0                          },
   m_date              {QDate::currentDate()         },
   m_carbonation_vols  {0.0                          },
   m_forcedCarbonation {false                        },
   m_primingSugarName  {""                           },
   m_carbonationTemp_c {0.0                          },
   m_primingSugarEquiv {0.0                          },
   m_kegPrimingFactor  {0.0                          },
   m_notes             {""                           },
   m_tasteNotes        {""                           },
   m_tasteRating       {0.0                          },
   styleId             {-1                           },
   mashId              {-1                           },
   equipmentId         {-1                           },
   m_og                {1.0                          },
   m_fg                {1.0                          },
   m_locked            {false                        },
   m_ancestor_id       {-1                           },
   m_ancestors         {},
   m_hasDescendants    {false                        } {
   return;
}

Recipe::Recipe(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   pimpl{std::make_unique<impl>(*this)},
   m_type              {
      // .:TODO:. Change so we store enum not string!
      RECIPE_TYPE_STRING_TO_TYPE.key(static_cast<Recipe::Type>(namedParameterBundle(PropertyNames::Recipe::recipeType).toInt()))
   },
   m_brewer            {namedParameterBundle(PropertyNames::Recipe::brewer).toString()           },
   m_asstBrewer        {namedParameterBundle(PropertyNames::Recipe::asstBrewer).toString()       },
   m_batchSize_l       {namedParameterBundle(PropertyNames::Recipe::batchSize_l).toDouble()      },
   m_boilSize_l        {namedParameterBundle(PropertyNames::Recipe::boilSize_l).toDouble()       },
   m_boilTime_min      {namedParameterBundle(PropertyNames::Recipe::boilTime_min).toDouble()     },
   m_efficiency_pct    {namedParameterBundle(PropertyNames::Recipe::efficiency_pct).toDouble()   },
   m_fermentationStages{namedParameterBundle(PropertyNames::Recipe::fermentationStages).toInt()  },
   m_primaryAge_days   {namedParameterBundle(PropertyNames::Recipe::primaryAge_days).toDouble()  },
   m_primaryTemp_c     {namedParameterBundle(PropertyNames::Recipe::primaryTemp_c).toDouble()    },
   m_secondaryAge_days {namedParameterBundle(PropertyNames::Recipe::secondaryAge_days).toDouble()},
   m_secondaryTemp_c   {namedParameterBundle(PropertyNames::Recipe::secondaryTemp_c).toDouble()  },
   m_tertiaryAge_days  {namedParameterBundle(PropertyNames::Recipe::tertiaryAge_days).toDouble() },
   m_tertiaryTemp_c    {namedParameterBundle(PropertyNames::Recipe::tertiaryTemp_c).toDouble()   },
   m_age               {namedParameterBundle(PropertyNames::Recipe::age).toDouble()              },
   m_ageTemp_c         {namedParameterBundle(PropertyNames::Recipe::ageTemp_c).toDouble()        },
   m_date              {namedParameterBundle(PropertyNames::Recipe::date).toDate()               },
   m_carbonation_vols  {namedParameterBundle(PropertyNames::Recipe::carbonation_vols).toDouble() },
   m_forcedCarbonation {namedParameterBundle(PropertyNames::Recipe::forcedCarbonation).toBool()  },
   m_primingSugarName  {namedParameterBundle(PropertyNames::Recipe::primingSugarName).toString() },
   m_carbonationTemp_c {namedParameterBundle(PropertyNames::Recipe::carbonationTemp_c).toDouble()},
   m_primingSugarEquiv {namedParameterBundle(PropertyNames::Recipe::primingSugarEquiv).toDouble()},
   m_kegPrimingFactor  {namedParameterBundle(PropertyNames::Recipe::kegPrimingFactor).toDouble() },
   m_notes             {namedParameterBundle(PropertyNames::Recipe::notes).toString()            },
   m_tasteNotes        {namedParameterBundle(PropertyNames::Recipe::tasteNotes).toString()       },
   m_tasteRating       {namedParameterBundle(PropertyNames::Recipe::tasteRating).toDouble()      },
   styleId             {namedParameterBundle(PropertyNames::Recipe::styleId).toInt()             },
   mashId              {namedParameterBundle(PropertyNames::Recipe::mashId).toInt()              },
   equipmentId         {namedParameterBundle(PropertyNames::Recipe::equipmentId).toInt()         },
   m_og                {namedParameterBundle(PropertyNames::Recipe::og).toDouble()               },
   m_fg                {namedParameterBundle(PropertyNames::Recipe::fg).toDouble()               },
   m_locked            {namedParameterBundle(PropertyNames::Recipe::locked).toBool()             },
   m_ancestor_id       {namedParameterBundle(PropertyNames::Recipe::ancestorId).toInt()          },
   m_ancestors         {},
   m_hasDescendants    {false                                                                    } {
   // At this stage, we haven't set any Hops, Fermentables, etc.  This is deliberate because the caller typically needs
   // to access subsidiary records to obtain this info.   Callers will usually use setters (setHopIds, etc but via
   // setProperty) to finish constructing the object.
   return;
}


Recipe::Recipe(Recipe const & other) :
   NamedEntity{other},
   pimpl{std::make_unique<impl>(*this)},
   m_type              {other.m_type              },
   m_brewer            {other.m_brewer            },
   m_asstBrewer        {other.m_asstBrewer        },
   m_batchSize_l       {other.m_batchSize_l       },
   m_boilSize_l        {other.m_boilSize_l        },
   m_boilTime_min      {other.m_boilTime_min      },
   m_efficiency_pct    {other.m_efficiency_pct    },
   m_fermentationStages{other.m_fermentationStages},
   m_primaryAge_days   {other.m_primaryAge_days   },
   m_primaryTemp_c     {other.m_primaryTemp_c     },
   m_secondaryAge_days {other.m_secondaryAge_days },
   m_secondaryTemp_c   {other.m_secondaryTemp_c   },
   m_tertiaryAge_days  {other.m_tertiaryAge_days  },
   m_tertiaryTemp_c    {other.m_tertiaryTemp_c    },
   m_age               {other.m_age               },
   m_ageTemp_c         {other.m_ageTemp_c         },
   m_date              {other.m_date              },
   m_carbonation_vols  {other.m_carbonation_vols  },
   m_forcedCarbonation {other.m_forcedCarbonation },
   m_primingSugarName  {other.m_primingSugarName  },
   m_carbonationTemp_c {other.m_carbonationTemp_c },
   m_primingSugarEquiv {other.m_primingSugarEquiv },
   m_kegPrimingFactor  {other.m_kegPrimingFactor  },
   m_notes             {other.m_notes             },
   m_tasteNotes        {other.m_tasteNotes        },
   m_tasteRating       {other.m_tasteRating       },
   styleId             {other.styleId             },  // But see additional logic in body
   mashId              {other.mashId              },  // But see additional logic in body
   equipmentId         {other.equipmentId         },  // But see additional logic in body
   m_og                {other.m_og                },
   m_fg                {other.m_fg                },
   m_locked            {other.m_locked            },
   // Copying a Recipe doesn't copy its descendants
   m_ancestor_id       {-1                        },
   m_ancestors         {},
   m_hasDescendants    {false                     } {
   setObjectName("Recipe"); // .:TBD:. Would be good to understand why we need this

   //
   // We don't want to be versioning something while we're still constructing it
   //
   NamedEntityModifyingMarker modifyingMarker(*this);

   //
   // When we make a copy of a Recipe, it needs to be a deep(ish) copy.  In particular, we need to make copies of the
   // Hops, Fermentables etc as some attributes of the recipe (eg how much and when to add) are stored inside these
   // ingredients.
   //
   // We _don't_ want to copy BrewNotes (an instance of brewing the Recipe).  (This is easy not to do as we don't
   // currently store BrewNote IDs in Recipe.)
   //
   this->pimpl->copyList<Fermentable>(*this, other);
   this->pimpl->copyList<Hop> (*this, other);
   this->pimpl->copyList<Instruction>(*this, other);
   this->pimpl->copyList<Misc> (*this, other);
   this->pimpl->copyList<Salt> (*this, other);
   this->pimpl->copyList<Water> (*this, other);
   this->pimpl->copyList<Yeast> (*this, other);

   //
   // You might think that Style, Mash and Equipment could safely be shared between Recipes.   However, AFAICT, none of
   // them is.  Presumably this is because users expect to be able to edit them in one Recipe without changing the
   // settings for any other Recipe.
   //
   // We also need to be careful here as one or more of these may not be set to a valid value.
   //
   if (other.equipmentId > 0) {
      auto equipment = copyIfNeeded(*ObjectStoreWrapper::getById<Equipment>(other.equipmentId));
      this->equipmentId = equipment->key();
   }

   if (other.mashId > 0) {
      auto mash = copyIfNeeded(*ObjectStoreWrapper::getById<Mash>(other.mashId));
      this->mashId = mash->key();
   }

   if (other.styleId > 0) {
      auto style = copyIfNeeded(*ObjectStoreWrapper::getById<Style>(other.styleId));
      this->styleId = style->key();
   }

   this->pimpl->connectSignals();

   this->recalcAll();

   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
Recipe::~Recipe() = default;

void Recipe::setKey(int key) {
   //
   // This function is called because we've just inserted a new Recipe in the DB and we now know its primary key.  By
   // convention, a new Recipe with no ancestor should have itself as its own ancestor.  So we need to check whether to
   // set that default here (which will then result in a DB update).  Otherwise, ancestor ID would remain as null.
   //
   // .:TBD:. Would it really be so bad for Ancestor ID to be NULL in the DB when there is no direct ancestor?
   //
   this->NamedEntity::setKey(key);
   if (this->m_ancestor_id <= 0) {
      qDebug() << Q_FUNC_INFO << "Setting default ancestor ID on Recipe #" << key;

      // We want to store the new ancestor ID in the DB, but we don't want to signal the UI about this change, so
      // suppress signal sending.
      this->setAncestorId(key, false);
   }
   return;
}


void Recipe::connectSignalsForAllRecipes() {
   qDebug() << Q_FUNC_INFO << "Connecting signals for all Recipes";
   // Connect fermentable, hop changed signals to their parent recipe
   for (auto recipe : ObjectStoreTyped<Recipe>::getInstance().getAllRaw()) {
//      qDebug() << Q_FUNC_INFO << "Connecting signals for Recipe #" << recipe->key();
      recipe->pimpl->connectSignals();
   }

   return;
}


void Recipe::mashFermentableIns() {
   /*** Add grains ***/
   auto ins = std::make_shared<Instruction>();
   ins->setName(tr("Add grains"));
   QString str = tr("Add ");
   QList<QString> reagents = this->getReagents(this->fermentables());

   for (int ii = 0; ii < reagents.size(); ++ii) {
      str += reagents.at(ii);
   }

   str += tr("to the mash tun.");
   ins->setDirections(str);

   this->add(ins);

   return;
}

void Recipe::saltWater(Salt::WhenToAdd when) {

   if (this->mash() == nullptr || this->salts().size() == 0) {
      return;
   }

   QStringList reagents = this->getReagents(salts(), when);
   if (reagents.size() == 0) {
      return;
   }

   auto ins = std::make_shared<Instruction>();
   QString tmp = when == Salt::WhenToAdd::MASH ? tr("mash") : tr("sparge");
   ins->setName(tr("Modify %1 water").arg(tmp));
   QString str = tr("Dissolve ");

   for (int ii = 0; ii < reagents.size(); ++ii) {
      str += reagents.at(ii);
   }

   str += QString(tr(" into the %1 water").arg(tmp));
   ins->setDirections(str);

   this->add(ins);

   return;
}

void Recipe::mashWaterIns() {

   if (this->mash() == nullptr) {
      return;
   }

   auto ins = std::make_shared<Instruction>();
   ins->setName(tr("Heat water"));
   QString str = tr("Bring ");
   QList<QString> reagents = getReagents(mash()->mashSteps());

   for (int ii = 0; ii < reagents.size(); ++ii) {
      str += reagents.at(ii);
   }

   str += tr("for upcoming infusions.");
   ins->setDirections(str);

   this->add(ins);

   return;
}

QVector<PreInstruction> Recipe::mashInstructions(double timeRemaining,
                                                 double totalWaterAdded_l,
                                                 [[maybe_unused]] unsigned int size) {
   QVector<PreInstruction> preins;

   if (mash() == nullptr) {
      return preins;
   }

   for (auto step : this->mash()->mashSteps()) {
      QString str;
      if (step->isInfusion()) {
         str = tr("Add %1 water at %2 to mash to bring it to %3.")
               .arg(Measurement::displayAmount(Measurement::Amount{step->infuseAmount_l(), Measurement::Units::liters},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::infuseAmount_l))
               .arg(Measurement::displayAmount(Measurement::Amount{step->infuseTemp_c(), Measurement::Units::celsius},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::infuseTemp_c))
               .arg(Measurement::displayAmount(Measurement::Amount{step->stepTemp_c(), Measurement::Units::celsius},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::stepTemp_c));
         totalWaterAdded_l += step->infuseAmount_l();
      } else if (step->isTemperature()) {
         str = tr("Heat mash to %1.").arg(Measurement::displayAmount(Measurement::Amount{step->stepTemp_c(),
                                                                                         Measurement::Units::celsius},
                                                                     PersistentSettings::Sections::mashStepTableModel,
                                                                     PropertyNames::MashStep::stepTemp_c));
      } else if (step->isDecoction()) {
         str = tr("Bring %1 of the mash to a boil and return to the mash tun to bring it to %2.")
               .arg(Measurement::displayAmount(Measurement::Amount{step->decoctionAmount_l(),
                                                                   Measurement::Units::liters},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::decoctionAmount_l))
               .arg(Measurement::displayAmount(Measurement::Amount{step->stepTemp_c(), Measurement::Units::celsius},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::stepTemp_c));
      }

      str += tr(" Hold for %1.").arg(Measurement::displayAmount(Measurement::Amount{step->stepTime_min(),
                                                                                    Measurement::Units::minutes},
                                                                PersistentSettings::Sections::mashStepTableModel,
                                                                PropertyNames::MashStep::stepTime_min));

      preins.push_back(PreInstruction(str, QString("%1 - %2").arg(step->typeStringTr()).arg(step->name()),
                                      timeRemaining));
      timeRemaining -= step->stepTime_min();
   }
   return preins;
}

QVector<PreInstruction> Recipe::hopSteps(Hop::Use type) {
   QVector<PreInstruction> preins;

   preins.clear();
   QList<Hop *> hlist = hops();
   int size = hlist.size();
   for (int i = 0; static_cast<int>(i) < size; ++i) {
      Hop * hop = hlist[static_cast<int>(i)];
      if (hop->use() == type) {
         QString str;
         if (type == Hop::Use::Boil) {
            str = tr("Put %1 %2 into boil for %3.");
         } else if (type == Hop::Use::Dry_Hop) {
            str = tr("Put %1 %2 into fermenter for %3.");
         } else if (type == Hop::Use::First_Wort) {
            str = tr("Put %1 %2 into first wort for %3.");
         } else if (type == Hop::Use::Mash) {
            str = tr("Put %1 %2 into mash for %3.");
         } else if (type == Hop::Use::Aroma) {
            str = tr("Steep %1 %2 in wort for %3.");
         } else {
            qWarning() << "Recipe::hopSteps(): Unrecognized hop use.";
            str = tr("Use %1 %2 for %3");
         }

         str = str.arg(Measurement::displayAmount(Measurement::Amount{hop->amount_kg(), Measurement::Units::kilograms},
                                                  PersistentSettings::Sections::hopTable,
                                                  PropertyNames::Hop::amount_kg))
               .arg(hop->name())
               .arg(Measurement::displayAmount(Measurement::Amount{hop->time_min(), Measurement::Units::minutes},
                                               PersistentSettings::Sections::hopTable,
                                               PropertyNames::Misc::time));

         preins.push_back(PreInstruction(str, tr("Hop addition"), hop->time_min()));
      }
   }
   return preins;
}

QVector<PreInstruction> Recipe::miscSteps(Misc::Use type) {
   QVector<PreInstruction> preins;

   QList<Misc *> mlist = miscs();
   int size = mlist.size();
   for (unsigned int i = 0; static_cast<int>(i) < size; ++i) {
      QString str;
      Misc * misc = mlist[static_cast<int>(i)];
      if (misc->use() == type) {
         if (type == Misc::Use::Boil) {
            str = tr("Put %1 %2 into boil for %3.");
         } else if (type == Misc::Use::Bottling) {
            str = tr("Use %1 %2 at bottling for %3.");
         } else if (type == Misc::Use::Mash) {
            str = tr("Put %1 %2 into mash for %3.");
         } else if (type == Misc::Use::Primary) {
            str = tr("Put %1 %2 into primary for %3.");
         } else if (type == Misc::Use::Secondary) {
            str = tr("Put %1 %2 into secondary for %3.");
         } else {
            qWarning() << "Recipe::getMiscSteps(): Unrecognized misc use.";
            str = tr("Use %1 %2 for %3.");
         }

         str = str .arg(Measurement::displayAmount(Measurement::Amount{
                                                      misc->amount(),
                                                      misc->amountIsWeight() ? Measurement::Units::kilograms : Measurement::Units::liters
                                                   },
                                                   PersistentSettings::Sections::miscTableModel,
                                                   PropertyNames::Misc::amount))
               .arg(misc->name())
               .arg(Measurement::displayAmount(Measurement::Amount{misc->time(), Measurement::Units::minutes},
                                               PersistentSettings::Sections::miscTableModel,
                                               PropertyNames::Misc::time));

         preins.push_back(PreInstruction(str, tr("Misc addition"), misc->time()));
      }
   }
   return preins;
}

void Recipe::firstWortHopsIns() {
   QList<QString> reagents = getReagents(hops(), true);
   if (reagents.size() == 0) {
      return;
   }

   QString str = tr("Do first wort hopping with ");

   for (int ii = 0; ii < reagents.size(); ++ii) {
      str += reagents.at(ii);
   }
   str += ".";

   auto ins = std::make_shared<Instruction>();
   ins->setName(tr("First wort hopping"));
   ins->setDirections(str);

   this->add(ins);

   return;
}

void Recipe::topOffIns() {
   Equipment * e = this->equipment();
   if (e == nullptr) {
      return;
   }

   double wortInBoil_l = wortFromMash_l() - e->lauterDeadspace_l();
   QString str = tr("You should now have %1 wort.")
                 .arg(Measurement::displayAmount(Measurement::Amount{wortInBoil_l, Measurement::Units::liters},
                                                 PersistentSettings::Sections::tab_recipe,
                                                 PropertyNames::Recipe::boilSize_l));
   if (e->topUpKettle_l() != 0.0) {
      return;
   }

   wortInBoil_l += e->topUpKettle_l();
   QString tmp = tr(" Add %1 water to the kettle, bringing pre-boil volume to %2.")
                 .arg(Measurement::displayAmount(Measurement::Amount{e->topUpKettle_l(), Measurement::Units::liters},
                                                 PersistentSettings::Sections::tab_recipe,
                                                 PropertyNames::Recipe::boilSize_l))
                 .arg(Measurement::displayAmount(Measurement::Amount{wortInBoil_l, Measurement::Units::liters},
                                                 PersistentSettings::Sections::tab_recipe,
                                                 PropertyNames::Recipe::boilSize_l));

   str += tmp;

   auto ins = std::make_shared<Instruction>();
   ins->setName(tr("Pre-boil"));
   ins->setDirections(str);
   ins->addReagent(tmp);

   this->add(ins);

   return;
}

bool Recipe::hasBoilFermentable() {
   int i;
   for (i = 0; static_cast<int>(i) < fermentables().size(); ++i) {
      Fermentable * ferm = fermentables()[i];
      if (ferm->isMashed() || ferm->addAfterBoil()) {
         continue;
      } else {
         return true;
      }
   }
   return false;
}

bool Recipe::hasBoilExtract() {
   int i;
   for (i = 0; static_cast<int>(i) < fermentables().size(); ++i) {
      Fermentable * ferm = fermentables()[i];
      if (ferm->isExtract()) {
         return true;
      } else {
         continue;
      }
   }
   return false;
}

PreInstruction Recipe::boilFermentablesPre(double timeRemaining) {
   QString str;
   int i;
   int size;

   str = tr("Boil or steep ");
   QList<Fermentable *> flist = fermentables();
   size = flist.size();
   for (i = 0; static_cast<int>(i) < size; ++i) {
      Fermentable * ferm = flist[i];
      if (ferm->isMashed() || ferm->addAfterBoil() || ferm->isExtract()) {
         continue;
      }

      str += QString("%1 %2, ")
             .arg(Measurement::displayAmount(Measurement::Amount{ferm->amount_kg(), Measurement::Units::kilograms},
                                             PersistentSettings::Sections::fermentableTable,
                                             PropertyNames::Fermentable::amount_kg))
             .arg(ferm->name());
   }
   str += ".";

   return PreInstruction(str, tr("Boil/steep fermentables"), timeRemaining);
}

bool Recipe::isFermentableSugar(Fermentable * fermy) {
   if (fermy->type() == Fermentable::Type::Sugar && fermy->name() == "Milk Sugar (Lactose)") {
      return false;
   }

   return true;
}

PreInstruction Recipe::addExtracts(double timeRemaining) const {
   QString str;
   int i;
   int size;

   str = tr("Raise water to boil and then remove from heat. Stir in  ");
   const QList<Fermentable *> flist = fermentables();
   size = flist.size();
   for (i = 0; static_cast<int>(i) < size; ++i) {
      const Fermentable * ferm = flist[i];
      if (ferm->isExtract()) {
         str += QString("%1 %2, ")
                .arg(Measurement::displayAmount(Measurement::Amount{ferm->amount_kg(), Measurement::Units::kilograms},
                                                PersistentSettings::Sections::fermentableTable,
                                                PropertyNames::Fermentable::amount_kg))
                .arg(ferm->name());
      }
   }
   str += ".";

   return PreInstruction(str, tr("Add Extracts to water"), timeRemaining);
}

void Recipe::postboilFermentablesIns() {
   QString tmp;
   bool hasFerms = false;

   QString str = tr("Add ");
   QList<Fermentable *> flist = this->fermentables();
   int size = flist.size();
   for (int ii = 0; ii < size; ++ii) {
      Fermentable * ferm = flist[ii];
      if (!ferm->addAfterBoil()) {
         continue;
      }

      hasFerms = true;
      tmp = QString("%1 %2, ")
            .arg(Measurement::displayAmount(Measurement::Amount{ferm->amount_kg(), Measurement::Units::kilograms},
                                            PersistentSettings::Sections::fermentableTable,
                                            PropertyNames::Fermentable::amount_kg))
            .arg(ferm->name());
      str += tmp;
   }
   str += tr("to the boil at knockout.");

   if (!hasFerms) {
      return;
   }

   auto ins = std::make_shared<Instruction>();
   ins->setName(tr("Knockout additions"));
   ins->setDirections(str);
   ins->addReagent(tmp);

   this->add(ins);

   return;
}

void Recipe::postboilIns() {
   Equipment * e = equipment();
   if (e == nullptr) {
      return;
   }

   double wortInBoil_l = wortFromMash_l() - e->lauterDeadspace_l();
   if (e->topUpKettle_l() != 0.0) {
      wortInBoil_l += e->topUpKettle_l();
   }

   double wort_l = e->wortEndOfBoil_l(wortInBoil_l);
   QString str = tr("You should have %1 wort post-boil.")
                 .arg(Measurement::displayAmount(Measurement::Amount{wort_l, Measurement::Units::liters},
                                                 PersistentSettings::Sections::tab_recipe,
                                                 PropertyNames::Recipe::batchSize_l));
   str += tr("\nYou anticipate losing %1 to trub and chiller loss.")
          .arg(Measurement::displayAmount(Measurement::Amount{e->trubChillerLoss_l(), Measurement::Units::liters},
                                          PersistentSettings::Sections::tab_recipe,
                                          PropertyNames::Recipe::batchSize_l));
   wort_l -= e->trubChillerLoss_l();
   if (e->topUpWater_l() > 0.0)
      str += tr("\nAdd %1 top up water into primary.")
             .arg(Measurement::displayAmount(Measurement::Amount{e->topUpWater_l(), Measurement::Units::liters},
                                             PersistentSettings::Sections::tab_recipe,
                                             PropertyNames::Recipe::batchSize_l));
   wort_l += e->topUpWater_l();
   str += tr("\nThe final volume in the primary is %1.")
          .arg(Measurement::displayAmount(Measurement::Amount{wort_l, Measurement::Units::liters},
                                          PersistentSettings::Sections::tab_recipe,
                                          PropertyNames::Recipe::batchSize_l));

   auto ins = std::make_shared<Instruction>();
   ins->setName(tr("Post boil"));
   ins->setDirections(str);
   this->add(ins);

   return;
}

void Recipe::addPreinstructions(QVector<PreInstruction> preins) {
   // Add instructions in descending mash time order.
   std::sort(preins.begin(), preins.end(), std::greater<PreInstruction>());
   for (int ii = 0; ii < preins.size(); ++ii) {
      PreInstruction pi = preins[ii];

      auto ins = std::make_shared<Instruction>();
      ins->setName(pi.getTitle());
      ins->setDirections(pi.getText());
      ins->setInterval(pi.getTime());

      this->add(ins);
   }
   return;
}

void Recipe::generateInstructions() {
   double timeRemaining;
   double totalWaterAdded_l = 0.0;

   if (!this->instructions().empty()) {
      this->clearInstructions();
   }

   QVector<PreInstruction> preinstructions;

   // Mash instructions

   int size = (mash() == nullptr) ? 0 : mash()->mashSteps().size();
   if (size > 0) {
      /*** prepare mashed fermentables ***/
      this->mashFermentableIns();

      /*** salt the water ***/
      saltWater(Salt::WhenToAdd::MASH);
      saltWater(Salt::WhenToAdd::SPARGE);

      /*** Prepare water additions ***/
      this->mashWaterIns();

      timeRemaining = mash()->totalTime();

      /*** Generate the mash instructions ***/
      preinstructions = mashInstructions(timeRemaining, totalWaterAdded_l, size);

      /*** Hops mash additions ***/
      preinstructions += hopSteps(Hop::Use::Mash);

      /*** Misc mash additions ***/
      preinstructions += miscSteps(Misc::Use::Mash);

      /*** Add the preinstructions into the instructions ***/
      addPreinstructions(preinstructions);

   } // END mash instructions.

   // First wort hopping
   this->firstWortHopsIns();

   // Need to top up the kettle before boil?
   topOffIns();

   // Boil instructions
   preinstructions.clear();

   // Find boil time.
   if (equipment() != nullptr) {
      timeRemaining = equipment()->boilTime_min();
   } else {
      timeRemaining =
         Measurement::qStringToSI(QInputDialog::getText(nullptr,
                                                        tr("Boil time"),
                                                        tr("You did not configure an equipment (which you really should), so tell me the boil time.")),
                                  Measurement::PhysicalQuantity::Time).quantity;
   }

   QString str = tr("Bring the wort to a boil and hold for %1.").arg(
      Measurement::displayAmount(Measurement::Amount{timeRemaining, Measurement::Units::minutes},
                                 PersistentSettings::Sections::tab_recipe,
                                 PropertyNames::Recipe::boilTime_min)
   );

   auto startBoilIns = std::make_shared<Instruction>();
   startBoilIns->setName(tr("Start boil"));
   startBoilIns->setInterval(timeRemaining);
   startBoilIns->setDirections(str);
   this->add(startBoilIns);

   /*** Get fermentables unless we haven't added yet ***/
   if (hasBoilFermentable()) {
      preinstructions.push_back(boilFermentablesPre(timeRemaining));
   }

   // add the intructions for including Extracts to wort
   if (hasBoilExtract()) {
      preinstructions.push_back(addExtracts(timeRemaining - 1));
   }

   /*** Boiled hops ***/
   preinstructions += hopSteps(Hop::Use::Boil);

   /*** Boiled miscs ***/
   preinstructions += miscSteps(Misc::Use::Boil);

   // END boil instructions.

   // Add instructions in descending mash time order.
   addPreinstructions(preinstructions);

   // FLAMEOUT
   auto flameoutIns = std::make_shared<Instruction>();
   flameoutIns->setName(tr("Flameout"));
   flameoutIns->setDirections(tr("Stop boiling the wort."));
   this->add(flameoutIns);

   // Steeped aroma hops
   preinstructions.clear();
   preinstructions += hopSteps(Hop::Use::Aroma);
   addPreinstructions(preinstructions);

   // Fermentation instructions
   preinstructions.clear();

   /*** Fermentables added after boil ***/
   postboilFermentablesIns();

   /*** post boil ***/
   postboilIns();

   /*** Primary yeast ***/
   str = tr("Cool wort and pitch ");
   QList<Yeast *> ylist = yeasts();
   for (int ii = 0; ii < ylist.size(); ++ii) {
      Yeast * yeast = ylist[ii];
      if (! yeast->addToSecondary()) {
         str += tr("%1 %2 yeast, ").arg(yeast->name()).arg(yeast->typeStringTr());
      }
   }
   str += tr("to the primary.");

   auto pitchIns = std::make_shared<Instruction>();
   pitchIns->setName(tr("Pitch yeast"));
   pitchIns->setDirections(str);
   this->add(pitchIns);
   /*** End primary yeast ***/

   /*** Primary misc ***/
   addPreinstructions(miscSteps(Misc::Use::Primary));

   str = tr("Let ferment until FG is %1.").arg(
      Measurement::displayAmount(Measurement::Amount{fg(), Measurement::Units::sp_grav},
                                 PersistentSettings::Sections::tab_recipe,
                                 PropertyNames::Recipe::fg,
                                 3)
   );

   auto fermentIns = std::make_shared<Instruction>();
   fermentIns->setName(tr("Ferment"));
   fermentIns->setDirections(str);
   this->add(fermentIns);

   str = tr("Transfer beer to secondary.");
   auto transferIns = std::make_shared<Instruction>();
   transferIns->setName(tr("Transfer to secondary"));
   transferIns->setDirections(str);
   this->add(transferIns);

   /*** Secondary misc ***/
   addPreinstructions(miscSteps(Misc::Use::Secondary));

   /*** Dry hopping ***/
   addPreinstructions(hopSteps(Hop::Use::Dry_Hop));

   // END fermentation instructions. Let everybody know that now is the time
   // to update instructions
   emit changed(metaProperty(*PropertyNames::Recipe::instructions), this->instructions().size());

   return;
}

QString Recipe::nextAddToBoil(double & time) {
   int i, size;
   double max = 0;
   bool foundSomething = false;
   Hop * h;
   QList<Hop *> hhops = hops();
   Misc * m;
   QList<Misc *> mmiscs = miscs();
   QString ret;

   // Search hops
   size = hhops.size();
   for (i = 0; i < size; ++i) {
      h = hhops[i];
      if (h->use() != Hop::Use::Boil) {
         continue;
      }
      if (h->time_min() < time && h->time_min() > max) {
         ret = tr("Add %1 %2 to boil at %3.")
               .arg(Measurement::displayAmount(Measurement::Amount{h->amount_kg(), Measurement::Units::kilograms},
                                               PersistentSettings::Sections::hopTable,
                                               PropertyNames::Hop::amount_kg))
               .arg(h->name())
               .arg(Measurement::displayAmount(Measurement::Amount{h->time_min(), Measurement::Units::minutes},
                                               PersistentSettings::Sections::hopTable,
                                               PropertyNames::Misc::time));

         max = h->time_min();
         foundSomething = true;
      }
   }

   // Search miscs
   size = mmiscs.size();
   for (i = 0; i < size; ++i) {
      m = mmiscs[i];
      if (m->use() != Misc::Use::Boil) {
         continue;
      }
      if (m->time() < time && m->time() > max) {
         ret = tr("Add %1 %2 to boil at %3.");
         if (m->amountIsWeight()) {
            ret = ret.arg(Measurement::displayAmount(Measurement::Amount{m->amount(), Measurement::Units::kilograms},
                                                     PersistentSettings::Sections::miscTableModel,
                                                     PropertyNames::Misc::amount));
         } else {
            ret = ret.arg(Measurement::displayAmount(Measurement::Amount{m->amount(), Measurement::Units::liters},
                                                     PersistentSettings::Sections::miscTableModel,
                                                     PropertyNames::Misc::amount));
         }

         ret = ret.arg(m->name());
         ret = ret.arg(Measurement::displayAmount(Measurement::Amount{m->time(), Measurement::Units::minutes},
                                                  PersistentSettings::Sections::miscTableModel,
                                                  PropertyNames::Misc::time));
         max = m->time();
         foundSomething = true;
      }
   }

   time = foundSomething ? max : -1.0;
   return ret;
}

//============================Relational Setters===============================
template<class NE> std::shared_ptr<NE> Recipe::add(std::shared_ptr<NE> ne) {
   // It's a coding error if we've ended up with a null shared_ptr
   Q_ASSERT(ne);

   // If the object being added is not already in the ObjectStore, we need to add it.  This is typically when we're
   // adding a new Instruction (which is an object owned by Recipe) or undoing a remove of a Hop/Fermentable/etc (thus
   // the "instance of use of" object was previously in the ObjectStore, but was removed and now we want to add it
   // back).
   if (ne->key() <= 0) {
      // With shared pointer parameter, ObjectStoreWrapper::insert returns what we passed it (ie our shared pointer
      // remains valid after the call).
      qDebug() << Q_FUNC_INFO << "Inserting" << ne->metaObject()->className() << "in object store";
      ObjectStoreWrapper::insert(ne);
   } else {
      //
      // The object was already in the ObjectStore, so let's check whether we need to copy it.
      //
      // Note that std::shared_ptr does all the right things if this assignment ends up boiling down to ne = ne!
      //
      ne = copyIfNeeded(*ne);
   }

   this->pimpl->accessIds<NE>().append(ne->key());
   connect(ne.get(), &NamedEntity::changed, this, &Recipe::acceptChangeToContainedObject);
   this->propagatePropertyChange(propertyToPropertyName<NE>());

   this->recalcIfNeeded(ne->metaObject()->className());
   return ne;
}

//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// means, amongst other things, that we can reference the pimpl.)
//
template std::shared_ptr<Hop        > Recipe::add(std::shared_ptr<Hop        > var);
template std::shared_ptr<Fermentable> Recipe::add(std::shared_ptr<Fermentable> var);
template std::shared_ptr<Misc       > Recipe::add(std::shared_ptr<Misc       > var);
template std::shared_ptr<Yeast      > Recipe::add(std::shared_ptr<Yeast      > var);
template std::shared_ptr<Water      > Recipe::add(std::shared_ptr<Water      > var);
template std::shared_ptr<Salt       > Recipe::add(std::shared_ptr<Salt       > var);
template std::shared_ptr<Instruction> Recipe::add(std::shared_ptr<Instruction> var);

template<class NE> bool Recipe::uses(NE const & var) const {
   int idToLookFor = var.key();
   if (idToLookFor <= 0) {
      //
      // We shouldn't be trying to look for a Fermentable/Hop/etc that hasn't even been stored (and therefore does not
      // yet have an ID).  The most likely reason for this happening would be a coding error that results in a copy of
      // a Fermentable/Hop/etc being taken and passed in as the parameter to this function (because copies do not take
      // the ID of the thing from which they were copied).
      //
      qCritical() <<
         Q_FUNC_INFO << "Trying to search for use of" << var.metaObject()->className() << "that is not stored!";
      return false;
   }

   auto match = std::find_if(this->pimpl->accessIds<NE>().cbegin(),
                             this->pimpl->accessIds<NE>().cend(),
   [idToLookFor](int id) {
      return idToLookFor == id;
   });

   return match != this->pimpl->accessIds<NE>().cend();
}
template bool Recipe::uses(Fermentable  const & var) const;
template bool Recipe::uses(Hop          const & var) const;
template bool Recipe::uses(Instruction  const & var) const;
template bool Recipe::uses(Misc         const & var) const;
template bool Recipe::uses(Salt         const & var) const;
template bool Recipe::uses(Water        const & var) const;
template bool Recipe::uses(Yeast        const & var) const;
template<> bool Recipe::uses<Equipment> (Equipment  const & var) const {
   return var.key() == this->equipmentId;
}
template<> bool Recipe::uses<Mash> (Mash const & var) const {
   return var.key() == this->mashId;
}
template<> bool Recipe::uses<Style> (Style const & var) const {
   return var.key() == this->styleId;
}

template<class NE> std::shared_ptr<NE> Recipe::remove(std::shared_ptr<NE> var) {
   // It's a coding error to supply a null shared pointer
   Q_ASSERT(var);

   int idToRemove = var->key();
   if (!this->pimpl->accessIds<NE>().removeOne(idToRemove)) {
      // It's a coding error if we try to remove something from the Recipe that wasn't in it in the first place!
      qCritical() <<
         Q_FUNC_INFO << "Tried to remove" << var->metaObject()->className() << "with ID" << idToRemove <<
         "but couldn't find it in Recipe #" << this->key();
      Q_ASSERT(false);
   } else {
      this->propagatePropertyChange(propertyToPropertyName<NE>());
      this->recalcIBU(); // .:TODO:. Don't need to do this recalculation when it's Instruction
   }

   //
   // Because Hop/Fermentable/etc objects in a Recipe are actually "Instance of use of Hop/Fermentable/etc" we usually
   // want to delete the object from the ObjectStore at this point.  But, because we're a bit paranoid, we'll check
   // first that the object we're removing has a parent (ie really is an "instance of use of") and is not used in any
   // other Recipes.
   //
   if (isUnusedInstanceOfUseOf(*var)) {
      qDebug() <<
         Q_FUNC_INFO << "Deleting" << var->metaObject()->className() << "#" << var->key() <<
         "as it is \"instance of use of\" that is no longer needed";
      ObjectStoreWrapper::hardDelete<NE>(var->key());
   }
   // The caller now owns the removed object unless and until they pass it in to Recipe::add() (typically to undo the
   // remove).
   return var;
}
template std::shared_ptr<Hop        > Recipe::remove(std::shared_ptr<Hop        > var);
template std::shared_ptr<Fermentable> Recipe::remove(std::shared_ptr<Fermentable> var);
template std::shared_ptr<Misc       > Recipe::remove(std::shared_ptr<Misc       > var);
template std::shared_ptr<Yeast      > Recipe::remove(std::shared_ptr<Yeast      > var);
template std::shared_ptr<Water      > Recipe::remove(std::shared_ptr<Water      > var);
template std::shared_ptr<Salt       > Recipe::remove(std::shared_ptr<Salt       > var);
template std::shared_ptr<Instruction> Recipe::remove(std::shared_ptr<Instruction> var);

int Recipe::instructionNumber(Instruction const & ins) const {
   // C++ arrays etc are indexed from 0, but for end users we want instruction numbers to start from 1
   return this->pimpl->instructionIds.indexOf(ins.key()) + 1;
}

void Recipe::swapInstructions(Instruction * ins1, Instruction * ins2) {

   int indexOf1 = this->pimpl->instructionIds.indexOf(ins1->key());
   int indexOf2 = this->pimpl->instructionIds.indexOf(ins2->key());

   // We can't swap them if we can't find both of them
   // There's no point swapping them if they're the same
   if (-1 == indexOf1 || -1 == indexOf2 || indexOf1 == indexOf2) {
      return;
   }

   // As of Qt 5.14 we could write:
   //    this->pimpl->instructionIds.swapItemsAt(indexOf1, indexOf2);
   // However, we still need to support slightly older versions of Qt (5.12 in particular), hence the more cumbersome
   // way here.
   std::swap(this->pimpl->instructionIds[indexOf1], this->pimpl->instructionIds[indexOf2]);

   ObjectStoreWrapper::updateProperty(*this, PropertyNames::Recipe::instructionIds);
   return;
}

void Recipe::clearInstructions() {
   for (int ii : this->pimpl->instructionIds) {
      ObjectStoreTyped<Instruction>::getInstance().softDelete(ii);
   }
   this->pimpl->instructionIds.clear();
   this->propagatePropertyChange(propertyToPropertyName<Instruction>());
   return;
}

void Recipe::insertInstruction(Instruction const & ins, int pos) {
   if (this->pimpl->instructionIds.contains(ins.key())) {
      qDebug() <<
         Q_FUNC_INFO << "Request to insert instruction ID" << ins.key() << "at position" << pos << "for recipe #" <<
         this->key() << "ignored as this instruction is already in the list at position" <<
         this->instructionNumber(ins);
      return;
   }

   // The position should be indexed from 1, so it's a coding error if it's less than this
   Q_ASSERT(pos >= 1);

   qDebug() <<
      Q_FUNC_INFO << "Inserting instruction #" << ins.key() << "(" << ins.name() << ") at position" << pos <<
      "in list of" << this->pimpl->instructionIds.size();
   this->pimpl->instructionIds.insert(pos - 1, ins.key());
   this->propagatePropertyChange(propertyToPropertyName<Instruction>());
   return;
}

void Recipe::setStyle(Style * var) {
   if (var->key() == this->styleId) {
      return;
   }

   std::shared_ptr<Style> styleToAdd = copyIfNeeded(*var);
   this->styleId = styleToAdd->key();
   this->propagatePropertyChange(propertyToPropertyName<Style>());
   return;
}

void Recipe::setEquipment(Equipment * var) {
   if (var->key() == this->equipmentId) {
      return;
   }

   std::shared_ptr<Equipment> equipmentToAdd = copyIfNeeded(*var);
   this->equipmentId = equipmentToAdd->key();
   this->propagatePropertyChange(propertyToPropertyName<Equipment>());
   return;
}

void Recipe::setMash(std::shared_ptr<Mash> mash) {
   // In the long run we'll keep this setter and retire the raw pointer version.  For now though, they pretty much
   // share the same implementation.
   this->setMash(mash.get());
   return;
}

void Recipe::setMash(Mash * var) {
   if (var->key() == this->mashId) {
      return;
   }

   // .:TBD:. Do we need to disconnect the old Mash?

   std::shared_ptr<Mash> mashToAdd = copyIfNeeded(*var);
   this->mashId = mashToAdd->key();
   this->propagatePropertyChange(propertyToPropertyName<Mash>());

   connect(mashToAdd.get(), &NamedEntity::changed, this, &Recipe::acceptChangeToContainedObject);
   emit this->changed(this->metaProperty(*PropertyNames::Recipe::mash), QVariant::fromValue<Mash *>(mashToAdd.get()));

   this->recalcAll();

   return;
}

void Recipe::setStyleId(int id) {
   this->styleId = id;
}

void Recipe::setEquipmentId(int id) {
   this->equipmentId = id;
}

void Recipe::setMashId(int id) {
   this->mashId = id;
   return;
}

void Recipe::setFermentableIds(QVector<int> fermentableIds) {
   this->pimpl->fermentableIds = fermentableIds;
   return;
}

void Recipe::setHopIds(QVector<int> hopIds) {
   this->pimpl->hopIds = hopIds;
   return;
}

void Recipe::setInstructionIds(QVector<int> instructionIds) {
   this->pimpl->instructionIds = instructionIds;
   return;
}

void Recipe::setMiscIds(QVector<int> miscIds) {
   this->pimpl->miscIds = miscIds;
   return;
}

void Recipe::setSaltIds(QVector<int> saltIds) {
   this->pimpl->saltIds = saltIds;
   return;
}

void Recipe::setWaterIds(QVector<int> waterIds) {
   this->pimpl->waterIds = waterIds;
   return;
}

void Recipe::setYeastIds(QVector<int> yeastIds) {
   this->pimpl->yeastIds = yeastIds;
   return;
}


//==============================="SET" METHODS=================================
void Recipe::setRecipeType(Recipe::Type var) {
   this->setType(RECIPE_TYPE_STRING_TO_TYPE.key(var));
   return;
}

void Recipe::setType(const QString & var) {
   QString tmp;
   if (! isValidType(var)) {
      qWarning() << QString("Recipe: invalid type: %1").arg(var);
      tmp = "All Grain";
   } else {
      tmp = QString(var);
   }
   this->setAndNotify(PropertyNames::Recipe::type, this->m_type, tmp);
   return;
}

void Recipe::setBrewer(QString const & var) {
   this->setAndNotify(PropertyNames::Recipe::brewer, this->m_brewer, var);
   return;
}

void Recipe::setBatchSize_l(double var) {
   this->setAndNotify(
                                   PropertyNames::Recipe::batchSize_l,
                                   this->m_batchSize_l,
                                   this->enforceMin(var, "batch size"));

   // NOTE: this is bad, but we have to call recalcAll(), because the estimated
   // boil/batch volumes depend on the target volumes when there are no mash
   // steps to actually provide an estimate for the volumes.
   recalcAll();
}

void Recipe::setBoilSize_l(double var) {
   this->setAndNotify(
                                   PropertyNames::Recipe::boilSize_l,
                                   this->m_boilSize_l,
                                   this->enforceMin(var, "boil size"));

   // NOTE: this is bad, but we have to call recalcAll(), because the estimated
   // boil/batch volumes depend on the target volumes when there are no mash
   // steps to actually provide an estimate for the volumes.
   recalcAll();
   return;
}

void Recipe::setBoilTime_min(double var) {
   this->setAndNotify(
                                   PropertyNames::Recipe::boilTime_min,
                                   this->m_boilTime_min,
                                   this->enforceMin(var, "boil time"));
   return;
}

void Recipe::setEfficiency_pct(double var) {
   this->setAndNotify(
                                   PropertyNames::Recipe::efficiency_pct,
                                   this->m_efficiency_pct,
                                   this->enforceMinAndMax(var, "efficiency", 0.0, 100.0, 70.0));

   // If you change the efficency, you really should recalc. And I'm afraid it
   // means recalc all, since og and fg will change, which means your ratios
   // change
   recalcAll();
}

void Recipe::setAsstBrewer(const QString & var) {
   this->setAndNotify(
                                   PropertyNames::Recipe::asstBrewer,
                                   this->m_asstBrewer,
                                   var);
   return;
}

void Recipe::setNotes(const QString & var) {
   this->setAndNotify(PropertyNames::Recipe::notes, this->m_notes, var);
   return;
}

void Recipe::setTasteNotes(const QString & var) {
   this->setAndNotify(PropertyNames::Recipe::tasteNotes, this->m_tasteNotes, var);
   return;
}

void Recipe::setTasteRating(double var) {
   this->setAndNotify(PropertyNames::Recipe::tasteRating, this->m_tasteRating, this->enforceMinAndMax(var, "taste rating", 0.0, 50.0, 0.0));
   return;
}

void Recipe::setOg(double var) {
   this->setAndNotify(PropertyNames::Recipe::og, this->m_og, this->enforceMin(var, "og", 0.0, 1.0));
   return;
}

void Recipe::setFg(double var) {
   this->setAndNotify(PropertyNames::Recipe::fg, this->m_fg, this->enforceMin(var, "fg", 0.0, 1.0));
   return;
}

void Recipe::setFermentationStages(int var) {
   this->setAndNotify(PropertyNames::Recipe::fermentationStages, this->m_fermentationStages,
                                   this->enforceMin(var, "stages"));
   return;
}

void Recipe::setPrimaryAge_days(double var) {
   this->setAndNotify(PropertyNames::Recipe::primaryAge_days, this->m_primaryAge_days, this->enforceMin(var, "primary age"));
   return;
}

void Recipe::setPrimaryTemp_c(double var) {
   this->setAndNotify(PropertyNames::Recipe::primaryTemp_c, this->m_primaryTemp_c, var);
   return;
}

void Recipe::setSecondaryAge_days(double var) {
   this->setAndNotify(PropertyNames::Recipe::secondaryAge_days, this->m_secondaryAge_days, this->enforceMin(var, "secondary age"));
   return;
}

void Recipe::setSecondaryTemp_c(double var) {
   this->setAndNotify(PropertyNames::Recipe::secondaryTemp_c, this->m_secondaryTemp_c, var);
   return;
}

void Recipe::setTertiaryAge_days(double var) {
   this->setAndNotify(PropertyNames::Recipe::tertiaryAge_days, this->m_tertiaryAge_days, this->enforceMin(var, "tertiary age"));
   return;
}

void Recipe::setTertiaryTemp_c(double var) {
   this->setAndNotify(PropertyNames::Recipe::tertiaryTemp_c, this->m_tertiaryTemp_c, var);
   return;
}

void Recipe::setAge_days(double var) {
   this->setAndNotify(PropertyNames::Recipe::age, this->m_age, this->enforceMin(var, "age"));
   return;
}

void Recipe::setAgeTemp_c(double var) {
   this->setAndNotify(PropertyNames::Recipe::ageTemp_c, this->m_ageTemp_c, var);
   return;
}

void Recipe::setDate(const QDate & var) {
   this->setAndNotify(PropertyNames::Recipe::date, this->m_date, var);
   return;
}

void Recipe::setCarbonation_vols(double var) {
   this->setAndNotify(PropertyNames::Recipe::carbonation_vols, this->m_carbonation_vols, this->enforceMin(var, "carb"));
   return;
}

void Recipe::setForcedCarbonation(bool var) {
   this->setAndNotify(PropertyNames::Recipe::forcedCarbonation, this->m_forcedCarbonation, var);
   return;
}

void Recipe::setPrimingSugarName(const QString & var) {
   this->setAndNotify(PropertyNames::Recipe::primingSugarName, this->m_primingSugarName, var);
   return;
}

void Recipe::setCarbonationTemp_c(double var) {
   this->setAndNotify(PropertyNames::Recipe::carbonationTemp_c, this->m_carbonationTemp_c, var);
   return;
}

void Recipe::setPrimingSugarEquiv(double var) {
   this->setAndNotify(PropertyNames::Recipe::primingSugarEquiv, this->m_primingSugarEquiv, this->enforceMin(var, "priming sugar equiv", 0.0, 1.0));
   return;
}

void Recipe::setKegPrimingFactor(double var) {
   this->setAndNotify(PropertyNames::Recipe::kegPrimingFactor, this->m_kegPrimingFactor, this->enforceMin(var, "keg priming factor", 0.0, 1.0));
   return;
}

void Recipe::setLocked(bool isLocked) {
   // Locking a Recipe doesn't count as changing it for the purposes of versioning or the UI, so no call to setAndNotify
   // here.
   if (this->newValueMatchesExisting(PropertyNames::Recipe::locked, this->m_locked, isLocked)) {
      return;
   }
   this->m_locked = isLocked;
   this->propagatePropertyChange(PropertyNames::Recipe::locked);
   return;
}

QList<Recipe *> Recipe::ancestors() const {
   // If we know we have some ancestors, and we didn't yet load them, do so now
   if (this->m_ancestor_id > 0 && this->m_ancestor_id != this->key() && this->m_ancestors.size() == 0) {
      // NB: In previous versions of the code, we included the Recipe in the list along with its ancestors, but it's
      //     now just the ancestors in the list.
      Recipe * ancestor = const_cast<Recipe *>(this);
      while (ancestor->m_ancestor_id > 0 && ancestor->m_ancestor_id != ancestor->key()) {
         ancestor = ObjectStoreWrapper::getByIdRaw<Recipe>(ancestor->m_ancestor_id);
         ancestor->m_hasDescendants = true;
         this->m_ancestors.append(ancestor);
      }
   }

   return this->m_ancestors;
}

bool Recipe::hasAncestors() const {
   return this->ancestors().size() > 0;
}

bool Recipe::isMyAncestor(Recipe const & maybe) const {
   return this->ancestors().contains(const_cast<Recipe *>(&maybe));
}

bool Recipe::hasDescendants() const {
   return this->m_hasDescendants;
}
void Recipe::setHasDescendants(bool spawned) {
   // This is not explicitly stored in the database, so no setAndNotify call etc here
   this->m_hasDescendants = spawned;
   return;
}

void Recipe::setAncestorId(int ancestorId, bool notify) {
   // Setting Recipe's ancestor ID doesn't count as changing it for the purposes of versioning or the UI, so no call to
   // setAndNotify here.  However, we do want the DB to get updated, so we do call propagatePropertyChange.
   if (this->newValueMatchesExisting(PropertyNames::Recipe::ancestorId, this->m_ancestor_id, ancestorId)) {
      return;
   }
   this->m_ancestor_id = ancestorId;
   this->propagatePropertyChange(PropertyNames::Recipe::ancestorId, notify);
   return;
}

void Recipe::setAncestor(Recipe & ancestor) {
   //
   // Typical usage is:
   //    - Recipe A is about to be modified
   //    - We create Recipe B as a deep copy of Recipe A
   //    - Recipe B becomes Recipe A's immediate ancestor, via call to this function
   //    - Recipe A is modified
   // This means that, if Recipe A already has a direct ancestor, then Recipe B needs to take it
   //
   qDebug() <<
      Q_FUNC_INFO << "Setting Recipe #" << ancestor.key() << "to be immediate prior version (ancestor) of Recipe #" <<
      this->key();

   if (this->m_ancestor_id > 0 && this->m_ancestor_id != this->key()) {
      // We already have ancestors (aka previous versions)

      if (&ancestor == this) {
         // Setting a Recipe to be its own ancestor is a kooky way of saying we want the Recipe not to have any
         // ancestors
         if (this->ancestors().size() > 0) {
            // We have some ancestors so we just have to tell the immediate one that it no longer has descendants
            this->ancestors().at(0)->setHasDescendants(false);
            this->ancestors().clear();
         }
      } else {
         // Give our existing ancestors them to the new direct ancestor (aka immediate prior version).  Note that it's
         // a coding error if this new direct ancestor already has its own ancestors.
         Q_ASSERT(ancestor.m_ancestor_id == ancestor.key() || ancestor.m_ancestor_id <= 0);
         ancestor.m_ancestor_id = this->m_ancestor_id;
         ancestor.m_ancestors = this->ancestors();
      }
   }

   // Skip most of the remaining work if we're really setting "no ancestors"
   if (&ancestor != this) {
      // Either we verified the lazy-load of this->m_ancestors in the call to this->ancestors() in the if statement above
      // or it should have been empty to begin with.  In both cases, we should be good to append the new ancestor here.
      this->m_ancestors.append(&ancestor);

      ancestor.setDisplay(false);
      ancestor.setLocked(true);
      ancestor.setHasDescendants(true);
   }

   this->setAncestorId(ancestor.key());

   return;
}

Recipe * Recipe::revertToPreviousVersion() {
   // If there are no ancestors then there is nothing to do
   if (!this->hasAncestors()) {
      return nullptr;
   }

   // Reactivate our immediate ancestor (aka previous version)
   Recipe * ancestor = ObjectStoreWrapper::getByIdRaw<Recipe>(this->m_ancestor_id);
   ancestor->setDisplay(true);
   ancestor->setLocked(false);
   ancestor->setHasDescendants(false);

   // Then forget we ever had any ancestors
   this->setAncestorId(this->key());

   return ancestor;
}


//==========================Calculated Getters============================

double Recipe::og() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_og;
}

double Recipe::fg() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_fg;
}

double Recipe::color_srm() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_color_srm;
}

double Recipe::ABV_pct() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_ABV_pct;
}

double Recipe::IBU() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_IBU;
}

QList<double> Recipe::IBUs() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_ibus;
}

double Recipe::boilGrav() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_boilGrav;
}

double Recipe::calories12oz() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_calories;
}

double Recipe::calories33cl() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_calories * 3.3 / 3.55;
}

double Recipe::wortFromMash_l() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_wortFromMash_l;
}

double Recipe::boilVolume_l() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_boilVolume_l;
}

double Recipe::postBoilVolume_l() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_postBoilVolume_l;
}

double Recipe::finalVolume_l() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_finalVolume_l;
}

QColor Recipe::SRMColor() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_SRMColor;
}

double Recipe::grainsInMash_kg() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_grainsInMash_kg;
}

double Recipe::grains_kg() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return m_grains_kg;
}

double Recipe::points() {
   if (m_uninitializedCalcs) {
      recalcAll();
   }
   return (m_og - 1.0) * 1e3;
}

//=========================Relational Getters=============================
Style * Recipe::style() const {
   return ObjectStoreWrapper::getByIdRaw<Style>(this->styleId);
}
int Recipe::getStyleId() const {
   return this->styleId;
}
std::shared_ptr<Mash> Recipe::getMash() const {
   return ObjectStoreWrapper::getById<Mash>(this->mashId);
}
Mash * Recipe::mash() const {
   return ObjectStoreWrapper::getByIdRaw<Mash>(this->mashId);
}
int Recipe::getMashId() const {
   return this->mashId;
}
Equipment * Recipe::equipment() const {
   return ObjectStoreWrapper::getByIdRaw<Equipment>(this->equipmentId);
}
int Recipe::getEquipmentId() const {
   return this->equipmentId;
}

QList<Instruction *> Recipe::instructions() const {
   return this->pimpl->getAllMyRaw<Instruction>();
}
QVector<int> Recipe::getInstructionIds() const {
   return this->pimpl->instructionIds;
}
QList<BrewNote *> Recipe::brewNotes() const {
   // The Recipe owns its BrewNotes, but, for the moment at least, it's the BrewNote that knows which Recipe it's in
   // rather than the Recipe which knows which BrewNotes it has, so we have to ask.
   int const recipeId = this->key();
   return ObjectStoreTyped<BrewNote>::getInstance().findAllMatching(
      [recipeId](BrewNote const * bn) {
         return bn->getRecipeId() == recipeId;
      }
   );
}

template<typename NE> QList< std::shared_ptr<NE> > Recipe::getAll() const {
   return this->pimpl->getAllMy<NE>();
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// means, amongst other things, that we can reference the pimpl.)
//
template QList< std::shared_ptr<Hop> > Recipe::getAll<Hop>() const;
template QList< std::shared_ptr<Fermentable> > Recipe::getAll<Fermentable>() const;
template QList< std::shared_ptr<Misc> > Recipe::getAll<Misc>() const;
template QList< std::shared_ptr<Salt> > Recipe::getAll<Salt>() const;
template QList< std::shared_ptr<Yeast> > Recipe::getAll<Yeast>() const;
template QList< std::shared_ptr<Water> > Recipe::getAll<Water>() const;

QList<Hop *> Recipe::hops() const {   return this->pimpl->getAllMyRaw<Hop>();                       }
QVector<int> Recipe::getHopIds() const {   return this->pimpl->hopIds;                              }
QList<Fermentable *> Recipe::fermentables() const { return this->pimpl->getAllMyRaw<Fermentable>(); }
QVector<int> Recipe::getFermentableIds() const    { return this->pimpl->fermentableIds;             }
QList<Misc *> Recipe::miscs() const               { return this->pimpl->getAllMyRaw<Misc>();        }
QVector<int> Recipe::getMiscIds() const           { return this->pimpl->miscIds;                    }
QList<Yeast *> Recipe::yeasts() const             { return this->pimpl->getAllMyRaw<Yeast>();       }
QVector<int> Recipe::getYeastIds() const          { return this->pimpl->yeastIds;                   }
QList<Water *> Recipe::waters() const             { return this->pimpl->getAllMyRaw<Water>();       }
QVector<int> Recipe::getWaterIds() const          { return this->pimpl->waterIds;                   }
QList<Salt *> Recipe::salts() const               { return this->pimpl->getAllMyRaw<Salt>();        }
QVector<int> Recipe::getSaltIds() const           { return this->pimpl->saltIds;                    }
int Recipe::getAncestorId() const                 { return this->m_ancestor_id;                     }

//==============================Getters===================================
Recipe::Type Recipe::recipeType() const {
   return RECIPE_TYPE_STRING_TO_TYPE.value(this->type());
}
QString Recipe::type() const               { return m_type;               }
QString Recipe::brewer() const             { return m_brewer;             }
QString Recipe::asstBrewer() const         { return m_asstBrewer;         }
QString Recipe::notes() const              { return m_notes;              }
QString Recipe::tasteNotes() const         { return m_tasteNotes;         }
QString Recipe::primingSugarName() const   { return m_primingSugarName;   }
bool    Recipe::forcedCarbonation() const  { return m_forcedCarbonation;  }
double  Recipe::batchSize_l() const        { return m_batchSize_l;        }
double  Recipe::boilSize_l() const         { return m_boilSize_l;         }
double  Recipe::boilTime_min() const       { return m_boilTime_min;       }
double  Recipe::efficiency_pct() const     { return m_efficiency_pct;     }
double  Recipe::tasteRating() const        { return m_tasteRating;        }
double  Recipe::primaryAge_days() const    { return m_primaryAge_days;    }
double  Recipe::primaryTemp_c() const      { return m_primaryTemp_c;      }
double  Recipe::secondaryAge_days() const  { return m_secondaryAge_days;  }
double  Recipe::secondaryTemp_c() const    { return m_secondaryTemp_c;    }
double  Recipe::tertiaryAge_days() const   { return m_tertiaryAge_days;   }
double  Recipe::tertiaryTemp_c() const     { return m_tertiaryTemp_c;     }
double  Recipe::age_days() const           { return m_age;                }
double  Recipe::ageTemp_c() const          { return m_ageTemp_c;          }
double  Recipe::carbonation_vols() const   { return m_carbonation_vols;   }
double  Recipe::carbonationTemp_c() const  { return m_carbonationTemp_c;  }
double  Recipe::primingSugarEquiv() const  { return m_primingSugarEquiv;  }
double  Recipe::kegPrimingFactor() const   { return m_kegPrimingFactor;   }
int     Recipe::fermentationStages() const { return m_fermentationStages; }
QDate   Recipe::date() const               { return m_date;               }
bool    Recipe::locked() const             { return m_locked;             }

//=============================Adders and Removers========================================


double Recipe::batchSizeNoLosses_l() {
   double ret = batchSize_l();
   Equipment * e = equipment();
   if (e) {
      ret += e->trubChillerLoss_l();
   }

   return ret;
}

//==============================Recalculators==================================

void Recipe::recalcIfNeeded(QString classNameOfWhatWasAddedOrChanged) {
   qDebug() << Q_FUNC_INFO << classNameOfWhatWasAddedOrChanged;
   // We could just compare with "Hop", "Equipment", etc but there's then no compile-time checking of typos.  Using
   // ::staticMetaObject.className() is a bit more clunky but it's safer.

   if (classNameOfWhatWasAddedOrChanged == Hop::staticMetaObject.className()) {
      this->recalcIBU();
      return;
   }

   if (classNameOfWhatWasAddedOrChanged == Equipment::staticMetaObject.className() ||
       classNameOfWhatWasAddedOrChanged == Fermentable::staticMetaObject.className() ||
       classNameOfWhatWasAddedOrChanged == Mash::staticMetaObject.className()) {
      this->recalcAll();
      return;
   }

   if (classNameOfWhatWasAddedOrChanged == Yeast::staticMetaObject.className()) {
      this->recalcOgFg();
      this->recalcABV_pct();
      return;
   }

   return;
}

void Recipe::recalcAll() {
   // WARNING
   // Infinite recursion possible, since these methods will emit changed(),
   // causing other objects to call finalVolume_l() for example, which may
   // cause another call to recalcAll() and so on.
   //
   // GSG: Now only emit when _uninitializedCalcs is true, which helps some.

   // Someone has already called this function back in the call stack, so return to avoid recursion.
   if (! m_recalcMutex.tryLock()) {
      return;
   }

   // Times are in seconds, and are cumulative.
   recalcGrainsInMash_kg(); // 0.01
   recalcGrains_kg(); // 0.03
   recalcVolumeEstimates(); // 0.06
   recalcColor_srm(); // 0.08
   recalcSRMColor(); // 0.08
   recalcOgFg(); // 0.11
   recalcABV_pct(); // 0.12
   recalcBoilGrav(); // 0.14
   recalcIBU(); // 0.15
   recalcCalories();

   m_uninitializedCalcs = false;

   m_recalcMutex.unlock();
}

void Recipe::recalcABV_pct() {
   double ret;

   // The complex formula, and variations comes from Ritchie Products Ltd, (Zymurgy, Summer 1995, vol. 18, no. 2)
   // Michael L. Hall’s article Brew by the Numbers: Add Up What’s in Your Beer, and Designing Great Beers by Daniels.
   ret = (76.08 * (m_og_fermentable - m_fg_fermentable) / (1.775 - m_og_fermentable)) * (m_fg_fermentable / 0.794);

   if (! qFuzzyCompare(ret, m_ABV_pct)) {
      m_ABV_pct = ret;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::ABV_pct), m_ABV_pct);
      }
   }
}

void Recipe::recalcColor_srm() {
   Fermentable * ferm;
   double mcu = 0.0;
   double ret;
   int i;

   QList<Fermentable *> ferms = fermentables();
   for (i = 0; static_cast<int>(i) < ferms.size(); ++i) {
      ferm = ferms[i];
      // Conversion factor for lb/gal to kg/l = 8.34538.
      mcu += ferm->color_srm() * 8.34538 * ferm->amount_kg() / m_finalVolumeNoLosses_l;
   }

   ret = ColorMethods::mcuToSrm(mcu);

   if (! qFuzzyCompare(m_color_srm, ret)) {
      m_color_srm = ret;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::color_srm), m_color_srm);
      }
   }

}

void Recipe::recalcIBU() {
   int i;
   double ibus = 0.0;
   double tmp = 0.0;

   // Bitterness due to hops...
   m_ibus.clear();
   QList<Hop *> hhops = hops();
   for (i = 0; i < hhops.size(); ++i) {
      tmp = ibuFromHop(hhops[i]);
      m_ibus.append(tmp);
      ibus += tmp;
   }

   // Bitterness due to hopped extracts...
   QList<Fermentable *> ferms = fermentables();
   for (i = 0; static_cast<int>(i) < ferms.size(); ++i) {
      // Conversion factor for lb/gal to kg/l = 8.34538.
      ibus +=
         ferms[i]->ibuGalPerLb() *
         (ferms[i]->amount_kg() / batchSize_l()) / 8.34538;
   }

   if (! qFuzzyCompare(ibus, m_IBU)) {
      m_IBU = ibus;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::IBU), m_IBU);
      }
   }
}

void Recipe::recalcVolumeEstimates() {
   double waterAdded_l;
   double absorption_lKg;
   double tmp = 0.0;
   double tmp_wfm = 0.0;
   double tmp_bv = 0.0;
   double tmp_fv = 0.0;
   double tmp_pbv = 0.0;

   // wortFromMash_l ==========================
   if (mash() == nullptr) {
      m_wortFromMash_l = 0.0;
   } else {
      waterAdded_l = mash()->totalMashWater_l();
      if (equipment() != nullptr) {
         absorption_lKg = equipment()->grainAbsorption_LKg();
      } else {
         absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;
      }

      tmp_wfm = (waterAdded_l - absorption_lKg * m_grainsInMash_kg);
   }

   // boilVolume_l ==============================

   if (equipment() != nullptr) {
      tmp = tmp_wfm - equipment()->lauterDeadspace_l() + equipment()->topUpKettle_l();
   } else {
      tmp = tmp_wfm;
   }

   // Need to account for extract/sugar volume also.
   QList<Fermentable *> ferms = fermentables();
   foreach (Fermentable * f, ferms) {
      Fermentable::Type type = f->type();
      if (type == Fermentable::Type::Extract) {
         tmp += f->amount_kg() / PhysicalConstants::liquidExtractDensity_kgL;
      } else if (type == Fermentable::Type::Sugar) {
         tmp += f->amount_kg() / PhysicalConstants::sucroseDensity_kgL;
      } else if (type == Fermentable::Type::Dry_Extract) {
         tmp += f->amount_kg() / PhysicalConstants::dryExtractDensity_kgL;
      }
   }

   if (tmp <= 0.0) {
      tmp = boilSize_l();   // Give up.
   }

   tmp_bv = tmp;

   // finalVolume_l ==============================

   // NOTE: the following figure is not based on the other volume estimates
   // since we want to show og,fg,ibus,etc. as if the collected wort is correct.
   m_finalVolumeNoLosses_l = batchSizeNoLosses_l();
   if (equipment() != nullptr) {
      //_finalVolumeNoLosses_l = equipment()->wortEndOfBoil_l(tmp_bv) + equipment()->topUpWater_l();
      tmp_fv = equipment()->wortEndOfBoil_l(tmp_bv) + equipment()->topUpWater_l() - equipment()->trubChillerLoss_l();
   } else {
      m_finalVolume_l = tmp_bv - 4.0; // This is just shooting in the dark. Can't do much without an equipment.
      //_finalVolumeNoLosses_l = _finalVolume_l;
   }

   // postBoilVolume_l ===========================

   if (equipment() != nullptr) {
      tmp_pbv = equipment()->wortEndOfBoil_l(tmp_bv);
   } else {
      tmp_pbv = batchSize_l(); // Give up.
   }

   if (! qFuzzyCompare(tmp_wfm, m_wortFromMash_l)) {
      m_wortFromMash_l = tmp_wfm;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::wortFromMash_l), m_wortFromMash_l);
      }
   }

   if (! qFuzzyCompare(tmp_bv, m_boilVolume_l)) {
      m_boilVolume_l = tmp_bv;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::boilVolume_l), m_boilVolume_l);
      }
   }

   if (! qFuzzyCompare(tmp_fv, m_finalVolume_l)) {
      m_finalVolume_l = tmp_fv;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::finalVolume_l), m_finalVolume_l);
      }
   }

   if (! qFuzzyCompare(tmp_pbv, m_postBoilVolume_l)) {
      m_postBoilVolume_l = tmp_pbv;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::postBoilVolume_l), m_postBoilVolume_l);
      }
   }
}

void Recipe::recalcGrainsInMash_kg() {
   int i, size;
   double ret = 0.0;
   Fermentable * ferm;

   QList<Fermentable *> ferms = fermentables();
   size = ferms.size();
   for (i = 0; i < size; ++i) {
      ferm = ferms[i];

      if (ferm->type() == Fermentable::Type::Grain && ferm->isMashed()) {
         ret += ferm->amount_kg();
      }
   }

   if (! qFuzzyCompare(ret, m_grainsInMash_kg)) {
      m_grainsInMash_kg = ret;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::grainsInMash_kg), m_grainsInMash_kg);
      }
   }
}

void Recipe::recalcGrains_kg() {
   int i, size;
   double ret = 0.0;

   QList<Fermentable *> ferms = fermentables();
   size = ferms.size();
   for (i = 0; i < size; ++i) {
      ret += ferms[i]->amount_kg();
   }

   if (! qFuzzyCompare(ret, m_grains_kg)) {
      m_grains_kg = ret;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::grains_kg), m_grains_kg);
      }
   }
}

void Recipe::recalcSRMColor() {
   QColor tmp = Algorithms::srmToColor(m_color_srm);

   if (tmp != m_SRMColor) {
      m_SRMColor = tmp;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::SRMColor), m_SRMColor);
      }
   }
}

// the formula in here are taken from http://hbd.org/ensmingr/
void Recipe::recalcCalories() {
   double startPlato, finishPlato, RE, abw, oog, ffg, tmp;

   oog = m_og;
   ffg = m_fg;

   // Need to translate OG and FG into plato
   startPlato  = -463.37 + (668.72 * oog) - (205.35 * oog * oog);
   finishPlato = -463.37 + (668.72 * ffg) - (205.35 * ffg * ffg);

   // RE (real extract)
   RE = (0.1808 * startPlato) + (0.8192 * finishPlato);

   // Alcohol by weight?
   abw = (startPlato - RE) / (2.0665 - (0.010665 * startPlato));

   // The final results of this formular are calories per 100 ml.
   // The 3.55 puts it in terms of 12 oz. I really should have stored it
   // without that adjust.
   tmp = ((6.9 * abw) + 4.0 * (RE - 0.1)) * ffg * 3.55;

   //! If there are no fermentables in the recipe, if there is no mash, etc.,
   //  then the calories/12 oz ends up negative. Since negative doesn't make
   //  sense, set it to 0
   if (tmp < 0) {
      tmp = 0;
   }

   if (! qFuzzyCompare(tmp, m_calories)) {
      m_calories = tmp;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::calories), m_calories);
      }
   }
}

// other efficiency calculations need access to the maximum theoretical sugars
// available. The only way I can see of doing that which doesn't suck is to
// split that calcuation out of recalcOgFg();
QHash<QString, double> Recipe::calcTotalPoints() {
   int i;
   double sugar_kg_ignoreEfficiency = 0.0;
   double sugar_kg                  = 0.0;
   double nonFermentableSugars_kg    = 0.0;
   double lateAddition_kg           = 0.0;
   double lateAddition_kg_ignoreEff = 0.0;

   Fermentable * ferm;

   QList<Fermentable *> ferms = fermentables();
   QHash<QString, double> ret;

   for (i = 0; static_cast<int>(i) < ferms.size(); ++i) {
      ferm = ferms[i];

      // If we have some sort of non-grain, we have to ignore efficiency.
      if (ferm->isSugar() || ferm->isExtract()) {
         sugar_kg_ignoreEfficiency += ferm->equivSucrose_kg();

         if (ferm->addAfterBoil()) {
            lateAddition_kg_ignoreEff += ferm->equivSucrose_kg();
         }

         if (!isFermentableSugar(ferm)) {
            nonFermentableSugars_kg += ferm->equivSucrose_kg();
         }
      } else {
         sugar_kg += ferm->equivSucrose_kg();

         if (ferm->addAfterBoil()) {
            lateAddition_kg += ferm->equivSucrose_kg();
         }
      }
   }

   ret.insert("sugar_kg", sugar_kg);
   ret.insert("nonFermentableSugars_kg", nonFermentableSugars_kg);
   ret.insert("sugar_kg_ignoreEfficiency", sugar_kg_ignoreEfficiency);
   ret.insert("lateAddition_kg", lateAddition_kg);
   ret.insert("lateAddition_kg_ignoreEff", lateAddition_kg_ignoreEff);

   return ret;

}

void Recipe::recalcBoilGrav() {
   double sugar_kg = 0.0;
   double sugar_kg_ignoreEfficiency = 0.0;
   double lateAddition_kg           = 0.0;
   double lateAddition_kg_ignoreEff = 0.0;
   double ret;
   QHash<QString, double> sugars;

   sugars = calcTotalPoints();
   sugar_kg = sugars.value("sugar_kg");
   sugar_kg_ignoreEfficiency = sugars.value("sugar_kg_ignoreEfficiency");
   lateAddition_kg = sugars.value("lateAddition_kg");
   lateAddition_kg_ignoreEff = sugars.value("lateAddition_kg_ignoreEff");

   // Since the efficiency refers to how much sugar we get into the fermenter,
   // we need to adjust for that here.
   sugar_kg = (efficiency_pct() / 100.0 * (sugar_kg - lateAddition_kg) + sugar_kg_ignoreEfficiency -
               lateAddition_kg_ignoreEff);

   ret = Algorithms::PlatoToSG_20C20C(Algorithms::getPlato(sugar_kg, boilSize_l()));

   if (! qFuzzyCompare(ret, m_boilGrav)) {
      m_boilGrav = ret;
      if (!m_uninitializedCalcs) {
         emit changed(metaProperty(*PropertyNames::Recipe::boilGrav), m_boilGrav);
      }
   }
}

void Recipe::recalcOgFg() {
   int i;
   double plato;
   double sugar_kg = 0;
   double sugar_kg_ignoreEfficiency = 0.0;
   double nonFermentableSugars_kg = 0.0;
   double kettleWort_l = 0.0;
   double postBoilWort_l = 0.0;
   double ratio = 0.0;
   double ferm_kg = 0.0;
   double attenuation_pct = 0.0;
   double tmp_og, tmp_fg, tmp_pnts, tmp_ferm_pnts, tmp_nonferm_pnts;
   Yeast * yeast;

   m_og_fermentable = m_fg_fermentable = 0.0;

   // The first time through really has to get the _og and _fg from the
   // database, not use the initialized values of 1. I (maf) tried putting
   // this in the initialize, but it just hung. So I moved it here, but only
   // if if we aren't initialized yet.
   //
   // GSG: This doesn't work, this og and fg are already set to 1.0 so
   // until we load these values from the database on startup, we have
   // to calculate.
   if (m_uninitializedCalcs) {
      m_og = Localization::toDouble(*this, PropertyNames::Recipe::og, Q_FUNC_INFO);
      m_fg = Localization::toDouble(*this, PropertyNames::Recipe::fg, Q_FUNC_INFO);
   }

   // Find out how much sugar we have.
   QHash<QString, double> sugars = calcTotalPoints();
   sugar_kg                  = sugars.value("sugar_kg");  // Mass of sugar that *is* affected by mash efficiency
   sugar_kg_ignoreEfficiency =
      sugars.value("sugar_kg_ignoreEfficiency");  // Mass of sugar that *is not* affected by mash efficiency
   nonFermentableSugars_kg    =
      sugars.value("nonFermentableSugars_kg");  // Mass of sugar that is not fermentable (also counted in sugar_kg_ignoreEfficiency)

   // We might lose some sugar in the form of Trub/Chiller loss and lauter deadspace.
   if (equipment() != nullptr) {

      kettleWort_l = (m_wortFromMash_l - equipment()->lauterDeadspace_l()) + equipment()->topUpKettle_l();
      postBoilWort_l = equipment()->wortEndOfBoil_l(kettleWort_l);
      ratio = (postBoilWort_l - equipment()->trubChillerLoss_l()) / postBoilWort_l;
      if (ratio > 1.0) { // Usually happens when we don't have a mash yet.
         ratio = 1.0;
      } else if (ratio < 0.0) {
         ratio = 0.0;
      } else if (Algorithms::isNan(ratio)) {
         ratio = 1.0;
      }
      // Ignore this again since it should be included in efficiency.
      //sugar_kg *= ratio;
      sugar_kg_ignoreEfficiency *= ratio;
      if (nonFermentableSugars_kg != 0.0) {
         nonFermentableSugars_kg *= ratio;
      }
   }

   // Total sugars after accounting for efficiency and mash losses. Implicitly includes non-fermentable sugars
   sugar_kg = sugar_kg * efficiency_pct() / 100.0 + sugar_kg_ignoreEfficiency;
   plato = Algorithms::getPlato(sugar_kg, m_finalVolumeNoLosses_l);

   tmp_og = Algorithms::PlatoToSG_20C20C(plato);    // og from all sugars
   tmp_pnts = (tmp_og - 1) * 1000.0; // points from all sugars
   if (nonFermentableSugars_kg != 0.0) {
      ferm_kg = sugar_kg - nonFermentableSugars_kg;  // Mass of only fermentable sugars
      plato = Algorithms::getPlato(ferm_kg, m_finalVolumeNoLosses_l);   // Plato from fermentable sugars
      m_og_fermentable = Algorithms::PlatoToSG_20C20C(plato);    // og from only fermentable sugars
      plato = Algorithms::getPlato(nonFermentableSugars_kg, m_finalVolumeNoLosses_l);   // Plate from non-fermentable sugars
      tmp_nonferm_pnts = ((Algorithms::PlatoToSG_20C20C(plato)) - 1) * 1000.0; // og points from non-fermentable sugars
   } else {
      m_og_fermentable = tmp_og;
      tmp_nonferm_pnts = 0;
   }

   // Calculage FG
   QList<Yeast *> yeasties = yeasts();
   for (i = 0; static_cast<int>(i) < yeasties.size(); ++i) {
      yeast = yeasties[i];
      // Get the yeast with the greatest attenuation.
      if (yeast->attenuation_pct() > attenuation_pct) {
         attenuation_pct = yeast->attenuation_pct();
      }
   }
   // This means we have yeast, but they neglected to provide attenuation percentages.
   if (yeasties.size() > 0 && attenuation_pct <= 0.0)  {
      attenuation_pct = 75.0; // 75% is an average attenuation.
   }

   if (nonFermentableSugars_kg != 0.0) {
      tmp_ferm_pnts = (tmp_pnts - tmp_nonferm_pnts) * (1.0 - attenuation_pct / 100.0); // fg points from fermentable sugars
      tmp_pnts = tmp_ferm_pnts + tmp_nonferm_pnts;  // FG points from both fermentable and non-fermentable sugars
      //tmp_pnts *= (1.0 - attenuation_pct/100.0);  // WTF, this completely ignores all the calculations about non-fermentable sugars and just converts everything!
      tmp_fg =  1 + tmp_pnts / 1000.0; // new FG value
      m_fg_fermentable =  1 + tmp_ferm_pnts / 1000.0; // FG from fermentables only
   } else {
      tmp_pnts *= (1.0 - attenuation_pct / 100.0);
      tmp_fg =  1 + tmp_pnts / 1000.0;
      m_fg_fermentable = tmp_fg;
   }

   if (! qFuzzyCompare(m_og, tmp_og)) {
      m_og     = tmp_og;
      // NOTE: We don't want to do this on the first load of the recipe.
      // NOTE: We are we recalculating all of these on load? Shouldn't we be
      // reading these values from the database somehow?
      //
      // GSG: Yes we can, but until the code is added to intialize these calculated
      // values from the database, we can calculate them on load. They should be
      // the same as the database values since the database values were set with
      // these functions in the first place.
      if (!m_uninitializedCalcs) {
         this->propagatePropertyChange(PropertyNames::Recipe::og, false);
         emit changed(metaProperty(*PropertyNames::Recipe::og), m_og);
         emit changed(metaProperty(*PropertyNames::Recipe::points), (m_og - 1.0) * 1e3);
      }
   }

   if (! qFuzzyCompare(tmp_fg, m_fg)) {
      m_fg     = tmp_fg;
      if (!m_uninitializedCalcs) {
         this->propagatePropertyChange(PropertyNames::Recipe::fg, false);
         emit changed(metaProperty(*PropertyNames::Recipe::fg), m_fg);
      }
   }
}

//====================================Helpers===========================================

double Recipe::ibuFromHop(Hop const * hop) {
   Equipment * equip = equipment();
   double ibus = 0.0;
   double fwhAdjust = Localization::toDouble(
      PersistentSettings::value(PersistentSettings::Names::firstWortHopAdjustment, 1.1).toString(),
      Q_FUNC_INFO
   );
   double mashHopAdjust = Localization::toDouble(
      PersistentSettings::value(PersistentSettings::Names::mashHopAdjustment, 0).toString(),
      Q_FUNC_INFO
   );

   if (hop == nullptr) {
      return 0.0;
   }

   double AArating = hop->alpha_pct() / 100.0;
   double grams = hop->amount_kg() * 1000.0;
   double minutes = hop->time_min();
   // Assume 100% utilization until further notice
   double hopUtilization = 1.0;
   // Assume 60 min boil until further notice
   int boilTime = 60;

   // NOTE: we used to carefully calculate the average boil gravity and use it in the
   // IBU calculations. However, due to John Palmer
   // (http://homebrew.stackexchange.com/questions/7343/does-wort-gravity-affect-hop-utilization),
   // it seems more appropriate to just use the OG directly, since it is the total
   // amount of break material that truly affects the IBUs.

   if (equip) {
      hopUtilization = equip->hopUtilization_pct() / 100.0;
      boilTime = static_cast<int>(equip->boilTime_min());
   }

   if (hop->use() == Hop::Use::Boil) {
      ibus = IbuMethods::getIbus(AArating, grams, m_finalVolumeNoLosses_l, m_og, minutes);
   } else if (hop->use() == Hop::Use::First_Wort) {
      ibus = fwhAdjust * IbuMethods::getIbus(AArating, grams, m_finalVolumeNoLosses_l, m_og, boilTime);
   } else if (hop->use() == Hop::Use::Mash && mashHopAdjust > 0.0) {
      ibus = mashHopAdjust * IbuMethods::getIbus(AArating, grams, m_finalVolumeNoLosses_l, m_og, boilTime);
   }

   // Adjust for hop form. Tinseth's table was created from whole cone data,
   // and it seems other formulae are optimized that way as well. So, the
   // utilization is considered unadjusted for whole cones, and adjusted
   // up for plugs and pellets.
   //
   // - http://www.realbeer.com/hops/FAQ.html
   // - https://groups.google.com/forum/#!topic"Application.h"lp/mv2qvWBC4sU
   switch (hop->form()) {
      case Hop::Form::Plug:
         hopUtilization *= 1.02;
         break;
      case Hop::Form::Pellet:
         hopUtilization *= 1.10;
         break;
      default:
         break;
   }

   // Adjust for hop utilization.
   ibus *= hopUtilization;

   return ibus;
}

// this was fixed, but not with an at
bool Recipe::isValidType(const QString & str) {
   return RECIPE_TYPE_STRING_TO_TYPE.contains(str);
}

QList<QString> Recipe::getReagents(QList<Fermentable *> ferms) {
   QList<QString> reagents;
   QString format, tmp;

   for (int i = 0; i < ferms.size(); ++i) {
      if (ferms[i]->isMashed()) {
         if (i + 1 < ferms.size()) {
            tmp = QString("%1 %2, ")
                  .arg(Measurement::displayAmount(Measurement::Amount{ferms[i]->amount_kg(), Measurement::Units::kilograms},
                                                  PersistentSettings::Sections::fermentableTable,
                                                  PropertyNames::Fermentable::amount_kg))
                  .arg(ferms[i]->name());
         } else {
            tmp = QString("%1 %2 ")
                  .arg(Measurement::displayAmount(Measurement::Amount{ferms[i]->amount_kg(), Measurement::Units::kilograms},
                                                  PersistentSettings::Sections::fermentableTable,
                                                  PropertyNames::Fermentable::amount_kg))
                  .arg(ferms[i]->name());
         }
         reagents.append(tmp);
      }
   }
   return reagents;
}

QList<QString> Recipe::getReagents(QList<Hop *> hops, bool firstWort) {
   QString tmp;
   QList<QString> reagents;

   for (int i = 0; i < hops.size(); ++i) {
      if (firstWort && (hops[i]->use() == Hop::Use::First_Wort)) {
         tmp = QString("%1 %2,")
               .arg(Measurement::displayAmount(Measurement::Amount{hops[i]->amount_kg(), Measurement::Units::kilograms},
                                               PersistentSettings::Sections::hopTable,
                                               PropertyNames::Hop::amount_kg))
               .arg(hops[i]->name());
         reagents.append(tmp);
      }
   }
   return reagents;
}

QList<QString> Recipe::getReagents(QList< std::shared_ptr<MashStep> > msteps) {
   QList<QString> reagents;

   for (int i = 0; i < msteps.size(); ++i) {
      if (!msteps[i]->isInfusion()) {
         continue;
      }

      QString tmp;
      if (i + 1 < msteps.size()) {
         tmp = tr("%1 water to %2, ")
               .arg(Measurement::displayAmount(Measurement::Amount{msteps[i]->infuseAmount_l(), Measurement::Units::liters},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::infuseAmount_l))
               .arg(Measurement::displayAmount(Measurement::Amount{msteps[i]->infuseTemp_c(), Measurement::Units::celsius},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::infuseTemp_c));
      } else {
         tmp = tr("%1 water to %2 ")
               .arg(Measurement::displayAmount(Measurement::Amount{msteps[i]->infuseAmount_l(), Measurement::Units::liters},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::infuseAmount_l))
               .arg(Measurement::displayAmount(Measurement::Amount{msteps[i]->infuseTemp_c(), Measurement::Units::celsius},
                                               PersistentSettings::Sections::mashStepTableModel,
                                               PropertyNames::MashStep::infuseTemp_c));
      }
      reagents.append(tmp);
   }
   return reagents;
}


//! \brief send me a list of salts and if we are wanting to add to the
//! mash or the sparge, and I will return a list of instructions
QStringList Recipe::getReagents(QList<Salt *> salts, Salt::WhenToAdd wanted) {
   QString tmp;
   QStringList reagents = QStringList();

   for (int i = 0; i < salts.size(); ++i) {
      Salt::WhenToAdd what = salts[i]->addTo();
      Measurement::Unit const & rightUnit = salts[i]->amountIsWeight() ? Measurement::Units::kilograms : Measurement::Units::liters;
      if (what == wanted) {
         tmp = tr("%1 %2, ")
               .arg(Measurement::displayAmount(Measurement::Amount{salts[i]->amount(), rightUnit},
                                               PersistentSettings::Sections::saltTable,
                                               PropertyNames::Salt::amount))
               .arg(salts[i]->name());
      } else if (what == Salt::WhenToAdd::EQUAL) {
         tmp = tr("%1 %2, ")
               .arg(Measurement::displayAmount(Measurement::Amount{salts[i]->amount(), rightUnit},
                                               PersistentSettings::Sections::saltTable,
                                               PropertyNames::Salt::amount))
               .arg(salts[i]->name());
      } else if (what == Salt::WhenToAdd::RATIO) {
         double ratio = 1.0;
         if (wanted == Salt::WhenToAdd::SPARGE) {
            ratio = mash()->totalSpargeAmount_l() / mash()->totalInfusionAmount_l();
         }
         double amt = salts[i]->amount() * ratio;
         tmp = tr("%1 %2, ")
               .arg(Measurement::displayAmount(Measurement::Amount{amt, rightUnit},
                                               PersistentSettings::Sections::saltTable,
                                               PropertyNames::Salt::amount))
               .arg(salts[i]->name());
      } else {
         continue;
      }
      reagents.append(tmp);
   }
   // How many ways can we remove the trailing ", " because it really, really
   // annoys me?
   if (reagents.size() > 0) {
      QString fixin = reagents.takeLast();
      fixin.remove(fixin.lastIndexOf(","), 2);
      reagents.append(fixin);
   }
   return reagents;
}

//==========================Accept changes from ingredients====================

void Recipe::acceptChangeToContainedObject([[maybe_unused]] QMetaProperty prop,
                                           [[maybe_unused]] QVariant val) {
   // This tells us which object sent us the signal
   QObject * signalSender = this->sender();
   if (signalSender != nullptr) {
      QString signalSenderClassName = signalSender->metaObject()->className();
      qDebug() << Q_FUNC_INFO << "Signal received from " << signalSenderClassName;
      this->recalcIfNeeded(signalSenderClassName);
   } else {
      qDebug() << Q_FUNC_INFO << "No sender";
   }
   return;
}

double Recipe::targetCollectedWortVol_l() {

   // Need to account for extract/sugar volume also.
   float postMashAdditionVolume_l = 0;

   QList<Fermentable *> ferms = fermentables();
   foreach (Fermentable * f, ferms) {
      Fermentable::Type type = f->type();
      if (type == Fermentable::Type::Extract) {
         postMashAdditionVolume_l  += static_cast<float>(f->amount_kg() / PhysicalConstants::liquidExtractDensity_kgL);
      } else if (type == Fermentable::Type::Sugar) {
         postMashAdditionVolume_l  += static_cast<float>(f->amount_kg() / PhysicalConstants::sucroseDensity_kgL);
      } else if (type == Fermentable::Type::Dry_Extract) {
         postMashAdditionVolume_l  += static_cast<float>(f->amount_kg() / PhysicalConstants::dryExtractDensity_kgL);
      }
   }

   if (equipment()) {
      return boilSize_l() - equipment()->topUpKettle_l() - static_cast<double>(postMashAdditionVolume_l);
   } else {
      return boilSize_l() - static_cast<double>(postMashAdditionVolume_l);
   }
}

double Recipe::targetTotalMashVol_l() {

   double absorption_lKg;

   if (equipment()) {
      absorption_lKg = equipment()->grainAbsorption_LKg();
   } else {
      absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;
   }


   return targetCollectedWortVol_l() + absorption_lKg * grainsInMash_kg();
}

Recipe * Recipe::getOwningRecipe() {
   return this;
}

void Recipe::hardDeleteOwnedEntities() {
   // It's the BrewNote that stores its Recipe ID, so all we need to do is delete our BrewNotes then the subsequent
   // database delete of this Recipe won't hit any foreign key problems.
   auto brewNotes = this->brewNotes();
   for (auto brewNote : brewNotes) {
      ObjectStoreWrapper::hardDelete<BrewNote>(*brewNote);
   }

   this->pimpl->hardDeleteAllMy<Fermentable>();
   this->pimpl->hardDeleteAllMy<Hop>        ();
   this->pimpl->hardDeleteAllMy<Instruction>();
   this->pimpl->hardDeleteAllMy<Misc>       ();
   this->pimpl->hardDeleteAllMy<Salt>       ();
   this->pimpl->hardDeleteAllMy<Water>      ();
   this->pimpl->hardDeleteAllMy<Yeast>      ();

   return;
}

void Recipe::hardDeleteOrphanedEntities() {
   //
   // Strictly a Recipe does not own its Mash.  However, if our Mash does not have a name and is not used by any other
   // Recipe, then we want to delete it, on the grounds that it's not one the user intended to reuse across multiple
   // Recipes.
   //
   // However, if we try to just delete the Mash Recipe::hardDeleteOwnedEntities(), we'd get a foreign key constraint
   // violation error from the DB as, at that point, the Mash ID is still referenced by this Recipe.  (Unsetting the
   // Mash ID in the Recipe record would be a bit tricky as we'd have to set it to NULL rather than just, say, -1 as,
   // otherwise we'll get a different foreign key constraint violation error (because the DB can't find a Mash row with
   // ID -1!).)
   //
   // At this point, however, the Recipe record has been removed from the database, so we can safely delete any orphaned
   // Mash record.
   //
   Mash * mash = this->mash();
   if (mash && mash->name() == "") {
      qDebug() << Q_FUNC_INFO << "Checking whether our unnamed Mash is used elsewhere";
      auto recipesUsingThisMash = ObjectStoreWrapper::findAllMatching<Recipe>(
         [mash](Recipe const * rec) {
            return rec->uses(*mash);
         }
      );
      if (1 == recipesUsingThisMash.size()) {
         qDebug() <<
            Q_FUNC_INFO << "Deleting unnamed Mash # " << mash->key() << " used only by Recipe #" << this->key();
         Q_ASSERT(recipesUsingThisMash.at(0)->key() == this->key());
         ObjectStoreWrapper::hardDelete<Mash>(*mash);
      }
   }
   return;
}


//======================================================================================================================
//====================================== Start of Functions in Helper Namespace ========================================
//======================================================================================================================
QList<BrewNote *> RecipeHelper::brewNotesForRecipeAndAncestors(Recipe const & recipe) {
   QList<BrewNote *> brewNotes = recipe.brewNotes();
   QList<Recipe *> ancestors = recipe.ancestors();
   for (auto ancestor : ancestors) {
      brewNotes.append(ancestor->brewNotes());
   }
   return brewNotes;
}

void RecipeHelper::prepareForPropertyChange(NamedEntity & ne, BtStringConst const & propertyName) {

   //
   // If the user has said they don't want versioning, just return
   //
   if (!RecipeHelper::getAutomaticVersioningEnabled()) {
      return;
   }

   qDebug() <<
      Q_FUNC_INFO << "Modifying: " << ne.metaObject()->className() << "#" << ne.key() << "property" << propertyName;

   //
   // If the object we're about to change a property on is a Recipe or is used in a Recipe, then it might need a new
   // version -- unless it's already being versioned.
   //
   Recipe * owner = ne.getOwningRecipe();
   if (!owner || owner->isBeingModified()) {
      // Change is not related to a recipe or the recipe is already being modified
      return;
   }

   //
   // Automatic versioning means that, once a recipe is brewed, it is "soft locked" and the first change should spawn a
   // new version.  Any subsequent change should not spawn a new version until it is brewed again.
   //
   if (owner->brewNotes().empty()) {
      // Recipe hasn't been brewed
      return;
   }

   // If the object we're about to change already has descendants, then we don't want to create new ones.
   if (owner->hasDescendants()) {
      qDebug() << Q_FUNC_INFO << "Recipe #" << owner->key() << "already has descendants, so not creating any more";
      return;
   }

   //
   // Once we've started doing versioning, we don't want to trigger it again on the same Recipe until we've finished
   //
   NamedEntityModifyingMarker ownerModifyingMarker(*owner);

   //
   // Versioning when modifying something in a recipe is *hard*.  If we copy the recipe, there is no easy way to say
   // "this ingredient in the old recipe is that ingredient in the new".  One approach would be to use the delete idea,
   // ie copy everything but what's being modified, clone what's being modified and add the clone to the copy.  Another
   // is to take a deep copy of the Recipe and make that the "prior version".
   //

   // Create a deep copy of the Recipe, and put it in the DB, so it has an ID.
   // (This will also emit signalObjectInserted for the new Recipe from ObjectStoreTyped<Recipe>.)
   qDebug() << Q_FUNC_INFO << "Copying Recipe" << owner->key();

   // We also don't want to trigger versioning on the newly spawned Recipe until we're completely done here!
   std::shared_ptr<Recipe> spawn = std::make_shared<Recipe>(*owner);
   NamedEntityModifyingMarker spawnModifyingMarker(*spawn);
   ObjectStoreWrapper::insert(spawn);

   qDebug() << Q_FUNC_INFO << "Copied Recipe #" << owner->key() << "to new Recipe #" << spawn->key();

   // We assert that the newly created version of the recipe has not yet been brewed (and therefore will not get
   // automatically versioned on subsequent changes before it is brewed).
   Q_ASSERT(spawn->brewNotes().empty());

   //
   // By default, copying a Recipe does not copy all its ancestry.  Here, we want the copy to become our ancestor (ie
   // previous version).  This will also emit a signalPropertyChanged from ObjectStoreTyped<Recipe>, which the UI can
   // pick up to update tree display of Recipes etc.
   //
   owner->setAncestor(*spawn);

   return;
}

/**
 * \brief Turn automatic versioning on or off
 */
void RecipeHelper::setAutomaticVersioningEnabled(bool enabled) {
   PersistentSettings::insert(PersistentSettings::Names::versioning, enabled);
   return;
}

/**
 * \brief Returns \c true if automatic versioning is enabled, \c false otherwise
 */
bool RecipeHelper::getAutomaticVersioningEnabled() {
   return PersistentSettings::value(PersistentSettings::Names::versioning, false).toBool();
}

RecipeHelper::SuspendRecipeVersioning::SuspendRecipeVersioning() {
   this->savedVersioningValue = RecipeHelper::getAutomaticVersioningEnabled();
   if (this->savedVersioningValue) {
      qDebug() << Q_FUNC_INFO << "Temporarily suspending automatic Recipe versioning";
      RecipeHelper::setAutomaticVersioningEnabled(false);
   }
   return;
}
RecipeHelper::SuspendRecipeVersioning::~SuspendRecipeVersioning() {
   if (this->savedVersioningValue) {
      qDebug() << Q_FUNC_INFO << "Re-enabling automatic Recipe versioning";
      RecipeHelper::setAutomaticVersioningEnabled(true);
   }
   return;
}

//======================================================================================================================
//======================================= End of Functions in Helper Namespace =========================================
//======================================================================================================================
