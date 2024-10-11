/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Recipe.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Greg Greenaae <ggreenaae@gmail.com>
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Jonathon Harding <github@jrhardin.net>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "model/Recipe.h"

#include <cmath> // For pow/log
#include <compare> //

#include <QDate>
#include <QDebug>
#include <QInputDialog>
#include <QList>
#include <QObject>

#include "Algorithms.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "HeatCalculations.h"
#include "Localization.h"
#include "measurement/Amount.h"
#include "measurement/ColorMethods.h"
#include "measurement/IbuMethods.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Boil.h"
#include "model/BoilStep.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/NamedParameterBundle.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdjustmentSalt.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUseOfWater.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"
#include "PhysicalConstants.h"
#include "utils/AutoCompare.h"

namespace {

   /**
    * \brief This is used to assist the creation of instructions.
    */
   struct PreInstruction {
      PreInstruction(QString text, QString title, double  time) : text{text}, title{title}, time{time} { return; }
      // Since we'll, amongst other things, be storing PreInstruction in a QVector, it needs to be default-constructable
      PreInstruction() : text{""}, title{""}, time{0.0} { return; }
      ~PreInstruction() = default;

      QString text;
      QString title;
      double  time;
   };
   auto operator<=>(PreInstruction const & lhs, PreInstruction const & rhs) {
      return lhs.time <=> rhs.time;
   }

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
   template<> std::shared_ptr<RecipeAdditionFermentable> copyIfNeeded(RecipeAdditionFermentable & var) = delete;
   template<> std::shared_ptr<RecipeAdditionHop        > copyIfNeeded(RecipeAdditionHop         & var) = delete;
   template<> std::shared_ptr<RecipeAdditionMisc       > copyIfNeeded(RecipeAdditionMisc        & var) = delete;
   template<> std::shared_ptr<RecipeAdditionYeast      > copyIfNeeded(RecipeAdditionYeast       & var) = delete;
   template<> std::shared_ptr<RecipeAdjustmentSalt     > copyIfNeeded(RecipeAdjustmentSalt      & var) = delete;
   template<> std::shared_ptr<RecipeUseOfWater         > copyIfNeeded(RecipeUseOfWater          & var) = delete;
   template<> std::shared_ptr<Fermentable> copyIfNeeded(Fermentable & var) = delete;
   template<> std::shared_ptr<Hop        > copyIfNeeded(Hop         & var) = delete;
   template<> std::shared_ptr<Misc       > copyIfNeeded(Misc        & var) = delete;
   template<> std::shared_ptr<Yeast      > copyIfNeeded(Yeast       & var) = delete;
   template<> std::shared_ptr<Salt       > copyIfNeeded(Salt        & var) = delete;
   template<> std::shared_ptr<Water      > copyIfNeeded(Water       & var) = delete;

   bool isFermentableSugar(Fermentable * fermy) {
      // TODO: This probably doesn't work in languages other than English!
      if (fermy->type() == Fermentable::Type::Sugar && fermy->name() == "Milk Sugar (Lactose)") {
         return false;
      }

      return true;
   }
}

//
// These specialisations are above the impl class because need to be defined before they are used in this file,
// otherwise we'll get a "specialization after instantiation" error on GCC.
//
// After we modified a property via a templated member function of Recipe, we need to tell the object store to
// update the database.  These template specialisations map from property type to property name.
//
template<> BtStringConst const & Recipe::propertyNameFor<Boil                     >() { return PropertyNames::Recipe::boilId                ; }
template<> BtStringConst const & Recipe::propertyNameFor<Equipment                >() { return PropertyNames::Recipe::equipmentId           ; }
template<> BtStringConst const & Recipe::propertyNameFor<Fermentation             >() { return PropertyNames::Recipe::fermentationId        ; }
template<> BtStringConst const & Recipe::propertyNameFor<Instruction              >() { return PropertyNames::Recipe::instructionIds        ; }
template<> BtStringConst const & Recipe::propertyNameFor<Mash                     >() { return PropertyNames::Recipe::mashId                ; }
template<> BtStringConst const & Recipe::propertyNameFor<Style                    >() { return PropertyNames::Recipe::styleId               ; }
// NB it is fermentableAdditions not fermentableAdditionIds that we want to use here, etc
template<> BtStringConst const & Recipe::propertyNameFor<RecipeAdditionFermentable>() { return PropertyNames::Recipe::fermentableAdditions; }
template<> BtStringConst const & Recipe::propertyNameFor<RecipeAdditionHop        >() { return PropertyNames::Recipe::hopAdditions        ; }
template<> BtStringConst const & Recipe::propertyNameFor<RecipeAdditionMisc       >() { return PropertyNames::Recipe::miscAdditions       ; }
template<> BtStringConst const & Recipe::propertyNameFor<RecipeAdditionYeast      >() { return PropertyNames::Recipe::yeastAdditions      ; }
template<> BtStringConst const & Recipe::propertyNameFor<RecipeAdjustmentSalt     >() { return PropertyNames::Recipe::saltAdjustments     ; }
template<> BtStringConst const & Recipe::propertyNameFor<RecipeUseOfWater         >() { return PropertyNames::Recipe::waterUses           ; }

// TBD: This is needed for WaterButton, but we should have a proper look at that some day
template<> BtStringConst const & Recipe::propertyNameFor<Water                    >() { return PropertyNames::Recipe::waterUses             ; }


// This private implementation class holds all private non-virtual members of Recipe
class Recipe::impl {
public:

   /**
    * Constructor
    */
   impl(Recipe & self) :
      m_self                 {self},
      instructionIds         {}   ,
      m_ABV_pct              {0.0},
      m_color_srm            {0.0},
      m_boilGrav             {0.0},
      m_IBU                  {0.0},
      m_ibus                 {}   ,
      m_wortFromMash_l       {0.0},
      m_boilVolume_l         {0.0},
      m_postBoilVolume_l     {0.0},
      m_finalVolume_l        {0.0},
      m_finalVolumeNoLosses_l{0.0},
      m_caloriesPerLiter     {0.0},
      m_grainsInMash_kg      {0.0},
      m_grains_kg            {0.0},
      m_SRMColor             {},
      m_og_fermentable       {0.0},
      m_fg_fermentable       {0.0} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * \brief Make copies of the additions of a particular type (\c RecipeAdditionHop, \c RecipeAdditionFermentable,
    *        etc) from one \c Recipe and add them to another - typically because we are copying the \c Recipe.
    *
    *        This also works for \c RecipeAdjustmentSalt and \c RecipeUseOfWater.
    */
   template<class RA> void copyAdditions(Recipe const & other) {
      for (RA * otherAddition : other.pimpl->allMyRaw<RA>()) {
         std::shared_ptr<RA> ourAddition = std::make_shared<RA>(*otherAddition);
         this->m_self.addAddition(ourAddition);
      }
      return;
   }

   /**
    * \brief Make copies of the Instructions from one Recipe and add them to another - typically
    *        because we are copying the Recipe.
    */
   void copyInstructions(Recipe & us, Recipe const & other) {
      qDebug() << Q_FUNC_INFO;
      for (int otherInstructionId : other.pimpl->instructionIds) {
         // Make and store a copy of the current Hop/Fermentable/etc object we're looking at in the other Recipe
         auto otherInstruction = ObjectStoreWrapper::getById<Instruction>(otherInstructionId);
         auto ourInstruction = copyIfNeeded(*otherInstruction);
         // Store the ID of the copy in our recipe
         this->instructionIds.append(ourInstruction->key());

         qDebug() <<
            Q_FUNC_INFO << "After adding Instruction #" << ourInstruction->key() <<
            ", Recipe" << us.name() << "has" << this->instructionIds.size() << "Instructions";

         // Connect signals so that we are notified when there are changes to the Hop/Fermentable/etc we just added to
         // our recipe.
         connect(ourInstruction.get(), &NamedEntity::changed, &us, &Recipe::acceptChangeToContainedObject);
      }
      return;
   }

   /**
    * \brief If the Recipe is about to be deleted, we delete all the things that belong to it.
    */
   template<class NE> void hardDeleteAdditions() {
      qDebug() << Q_FUNC_INFO;
      for (int id : this->allMyIds<NE>()) {
         qDebug() << Q_FUNC_INFO << "Hard deleting" << NE::staticMetaObject.className() << "#" << id;
         ObjectStoreWrapper::hardDelete<NE>(id);
      }
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

   template<typename T>
   T getCalculated(T & memberVariable) {
      if (this->m_self.m_uninitializedCalcs) {
         this->m_self.recalcAll();
      }
      return memberVariable;
   }

   /**
    * \brief Get shared pointers to all this Recipe's BrewNotes or RecipeAdditions of a particular type
    *        (RecipeAdditionHop, RecipeAdditionFermentable, etc).
    */
   template<class NE>
   QList<std::shared_ptr<NE>> allMy() const {
      int const recipeId = this->m_self.key();
      return ObjectStoreWrapper::findAllMatching<NE>([recipeId](std::shared_ptr<NE> ne) {
                                                        return ne->recipeId() == recipeId;
                                                     });
   }

   /**
    * \brief Get raw pointers to all this Recipe's BrewNotes or RecipeAdditions of a particular type (RecipeAdditionHop,
    *        RecipeAdditionFermentable, etc).
    */
   template<class NE>
   QList<NE *> allMyRaw() const {
      int const recipeId = this->m_self.key();
      return ObjectStoreWrapper::findAllMatching<NE>([recipeId](NE const * ne) {
                                                        return ne->recipeId() == recipeId;
                                                     });
   }

   /**
    * \brief Get IDs of all this Recipe's BrewNotes or RecipeAdditions of a particular type (RecipeAdditionHop,
    *        RecipeAdditionFermentable, etc).
    */
   template<class NE>
   QVector<int> allMyIds() const {
      int const recipeId = this->m_self.key();
      return ObjectStoreWrapper::idsOfAllMatching<NE>([recipeId](NE const * ne) {
                                                         return ne->recipeId() == recipeId;
                                                      });
   }

   //
   // .:TODO:. This will go away once we lose junction tables etc for Fermentables, Miscs and replace them with proper
   //          RecipeAddition objects.
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
    * \brief Called from Recipe::hardDeleteOrphanedEntities
    */
   template<class NE> void hardDeleteOrphanedStepOwner() {
      auto stepOwner = this->m_self.get<NE>();
      if (stepOwner && stepOwner->name() == "") {
         qDebug() <<
            Q_FUNC_INFO << "Checking whether our unnamed" << NE::staticMetaObject.className() << "is used elsewhere";
         auto recipesUsingThisStepOwner = ObjectStoreWrapper::findAllMatching<Recipe>(
            [stepOwner](Recipe const * rec) {
               return rec->uses(*stepOwner);
            }
         );
         if (1 == recipesUsingThisStepOwner.size()) {
            qDebug() <<
               Q_FUNC_INFO << "Deleting unnamed" << NE::staticMetaObject.className() << "# " << stepOwner->key() <<
               " used only by Recipe #" << this->m_self.key();
            Q_ASSERT(recipesUsingThisStepOwner.at(0)->key() == this->m_self.key());
            ObjectStoreWrapper::hardDelete<NE>(*stepOwner);
         }
      }

      return;
   }

   /**
    * \brief Connect signals for this Recipe.  See comment for \c Recipe::connectSignalsForAllRecipes for more
    *        explanation.
    */
   void connectSignals() {
      auto equipment = this->m_self.equipment();
      if (equipment) {
         // We used to have special signals for changes to Equipment's boilSize_l and boilTime_min properties, but these
         // are now picked up in Recipe::acceptChangeToContainedObject from the generic `changed` signal
         connect(equipment.get(), &NamedEntity::changed, &this->m_self, &Recipe::acceptChangeToContainedObject);
      }

      auto fermentableAdditions = this->m_self.fermentableAdditions();
      for (auto fermentableAddition : fermentableAdditions) {
         connect(fermentableAddition->fermentable(),
                 &NamedEntity::changed,
                 &this->m_self,
                 &Recipe::acceptChangeToContainedObject);
      }

      auto hopAdditions = this->m_self.hopAdditions();
      for (auto hopAddition : hopAdditions) {
         connect(hopAddition->hop(),
                 &NamedEntity::changed,
                 &this->m_self,
                 &Recipe::acceptChangeToContainedObject);
      }

      auto yeastAdditions = this->m_self.yeastAdditions();
      for (auto yeastAddition : yeastAdditions) {
         connect(yeastAddition->yeast(),
                 &NamedEntity::changed,
                 &this->m_self,
                 &Recipe::acceptChangeToContainedObject);
      }

      auto mash = this->m_self.mash();
      if (mash) {
         connect(mash.get(), &NamedEntity::changed, &this->m_self, &Recipe::acceptChangeToContainedObject);
      }

      return;
   }

   template<class NE>
   void set(std::shared_ptr<NE> val, int & ourId) {
      if (!val && ourId < 0) {
         // No change (from "not set" to "not set")
         return;
      }

      if (ourId > 0) {
         // This comparison is only valid if we have a valid ID, because val might not yet be stored in the DB.
         if (val && val->key() == ourId) {
            // No change (same object as we already have)
            return;
         }

         std::shared_ptr<NE> oldVal = ObjectStoreWrapper::getById<NE>(ourId);
         disconnect(oldVal.get(), nullptr, &this->m_self, nullptr);
      }

      if (!val) {
         ourId = -1;
         return;
      }

      if (val->key() < 0) {
         ourId = ObjectStoreWrapper::insert<NE>(val);
      } else {
         // TBD: Would be nice to get rid of this call to copyIfNeeded
         val = copyIfNeeded(*val);
         ourId = val->key();
      }

      BtStringConst const & property = Recipe::propertyNameFor<NE>();
      qDebug() << Q_FUNC_INFO << "Setting" << property << "to" << ourId;
      this->m_self.propagatePropertyChange(property);

      connect(val.get(), &NamedEntity::changed, &this->m_self, &Recipe::acceptChangeToContainedObject);
      emit this->m_self.changed(this->m_self.metaProperty(*property), QVariant::fromValue<NE *>(val.get()));

      this->m_self.recalcAll();
      return;
   }

   /**
    * \brief Getting a recipe's \c Boil, \c Fermentation, etc is pretty much the same logic, so we template it
    *
    *        In BeerJSON, each of mash, boil and fermentation is optional.  I guess no fermentation is for recipes for
    *        making hop water etc.  Equally, there are people making beer without boiling -- eg see
    *        https://byo.com/article/raw-ale/.  In both cases, our current support for "no boil" and/or "no ferment" is
    *        somewhat limited and untested for now.
    */
   template<class NE>
   std::shared_ptr<NE> get(int const & ourId) const {
      // Normally leave the next line commented out otherwise it generates too much logging
//      qDebug() << Q_FUNC_INFO << "Recipe #" << this->m_self.key() << NE::staticMetaObject.className() << "ID" << ourId;
      if (ourId < 0) {
         // Negative ID just means there isn't one -- because this is how we store "NULL" for a foreign key
         qDebug() << Q_FUNC_INFO << "No" << NE::staticMetaObject.className() << "on Recipe #" << this->m_self.key();
         return nullptr;
      }
      auto retVal = ObjectStoreWrapper::getById<NE>(ourId);
      if (!retVal) {
         // I would think it's a coding error to have a seemingly valid boil/etc ID that's not in the database, but we
         // try to recover as best we can.
         qCritical() <<
            Q_FUNC_INFO << "Invalid" << NE::staticMetaObject.className() << "ID (" << ourId << ") on Recipe #" <<
            this->m_self.key();
         return nullptr;
      }
      return retVal;
   }

   QVector<PreInstruction> mashInstructions(double timeRemaining,
                                            double totalWaterAdded_l,
                                            [[maybe_unused]] unsigned int size) {
      QVector<PreInstruction> preins;
      if (!m_self.mash()) {
         return preins;
      }

      for (auto step : m_self.mash()->mashSteps()) {
         QString str;
         if (step->isInfusion()) {
            str = tr("Add %1 water at %2 to mash to bring it to %3.")
                  .arg(Measurement::displayAmount(Measurement::Amount{step->amount_l(), Measurement::Units::liters}))
                  .arg(Measurement::displayAmount(Measurement::Amount{step->infuseTemp_c().value_or(step->startTemp_c().value_or(0.0)), Measurement::Units::celsius}))
                  .arg(Measurement::displayAmount(Measurement::Amount{step->startTemp_c().value_or(0.0), Measurement::Units::celsius}));
            totalWaterAdded_l += step->amount_l();
         } else if (step->isTemperature()) {
            str = tr("Heat mash to %1.").arg(Measurement::displayAmount(Measurement::Amount{step->startTemp_c().value_or(0.0),
                                                                                          Measurement::Units::celsius}));
         } else if (step->isDecoction()) {
            str = tr("Bring %1 of the mash to a boil and return to the mash tun to bring it to %2.")
                  .arg(Measurement::displayAmount(Measurement::Amount{step->amount_l(),
                                                                     Measurement::Units::liters}))
                  .arg(Measurement::displayAmount(Measurement::Amount{step->startTemp_c().value_or(0.0), Measurement::Units::celsius}));
         }

         str += tr(" Hold for %1.").arg(Measurement::displayAmount(Measurement::Amount{step->stepTime_mins().value_or(0.0),
                                                                                       Measurement::Units::minutes}));

         preins.push_back(PreInstruction(str, QString("%1 - %2").arg(MashStep::typeDisplayNames[step->type()]).arg(step->name()),
                                       timeRemaining));
         timeRemaining -= step->stepTime_mins().value_or(0.0);
      }
      return preins;
   }

   QVector<PreInstruction> hopSteps(RecipeAddition::Stage const stage) {
      // TBD: What about hopAddition->addAtTime_mins()?
      QVector<PreInstruction> preins;
      for (auto hopAddition : m_self.hopAdditions()) {
         auto hop = hopAddition->hop();
         if (hopAddition->stage() == stage) {
            QString str;
            switch (stage) {
               case RecipeAddition::Stage::Mash:
                  str = tr("Put %1 %2 into mash for %3.");
                  break;
               case RecipeAddition::Stage::Boil:
                  if (hopAddition->isFirstWort()) {
                     str = tr("Put %1 %2 into first wort for %3.");
                  } else if (hopAddition->isAroma()) {
                     str = tr("Steep %1 %2 in wort for %3.");
                  } else {
                     str = tr("Put %1 %2 into boil for %3.");
                  }
                  break;
               case RecipeAddition::Stage::Fermentation:
                  str = tr("Put %1 %2 into fermenter for %3.");
                  break;
               case RecipeAddition::Stage::Packaging:
                  // We don't really support this yet, but best to say something if we read in a recipe that has this
                  str = tr("Put %1 %2 into packaging for %3.");
                  break;
               // NB: No default case as we want compiler to warn us if we missed a value above
            }

            str = str.arg(Measurement::displayAmount(hopAddition->amount()))
                     .arg(hop->name())
                     .arg(Measurement::displayAmount(Measurement::Amount{hopAddition->duration_mins().value_or(0.0), Measurement::Units::minutes}));

            preins.push_back(PreInstruction(str, tr("Hop addition"), hopAddition->duration_mins().value_or(0.0)));
         }
      }
      return preins;
   }

   QVector<PreInstruction> miscSteps(RecipeAdditionMisc::Use type) {
      QVector<PreInstruction> preins;
      for (auto miscAddition : m_self.miscAdditions()) {
         QString str;
         auto misc = miscAddition->misc();
         if (miscAddition->use() == type) {
            if (type == RecipeAdditionMisc::Use::Boil) {
               str = tr("Put %1 %2 into boil for %3.");
            } else if (type == RecipeAdditionMisc::Use::Bottling) {
               str = tr("Use %1 %2 at bottling for %3.");
            } else if (type == RecipeAdditionMisc::Use::Mash) {
               str = tr("Put %1 %2 into mash for %3.");
            } else if (type == RecipeAdditionMisc::Use::Primary) {
               str = tr("Put %1 %2 into primary for %3.");
            } else if (type == RecipeAdditionMisc::Use::Secondary) {
               str = tr("Put %1 %2 into secondary for %3.");
            } else {
               qWarning() << Q_FUNC_INFO << "Unrecognized misc use.";
               str = tr("Use %1 %2 for %3.");
            }

            str = str.arg(Measurement::displayAmount(miscAddition->amount()))
                     .arg(misc->name())
                     .arg(Measurement::displayAmount(Measurement::Amount{miscAddition->duration_mins().value_or(0.0), Measurement::Units::minutes}));

            preins.push_back(PreInstruction(str, tr("Misc addition"), miscAddition->duration_mins().value_or(0.0)));
         }
      }
      return preins;
   }

   PreInstruction boilFermentablesPre(double timeRemaining) {
      QString str = tr("Boil or steep ");
      for (auto const & fermentableAddition : m_self.fermentableAdditions()) {
         if (fermentableAddition->stage() != RecipeAddition::Stage::Boil ||
            fermentableAddition->addAfterBoil() || fermentableAddition->fermentable()->isExtract()) {
            continue;
         }

         str += QString("%1 %2, ")
               .arg(Measurement::displayAmount(fermentableAddition->amount()))
               .arg(fermentableAddition->name());
      }
      str += ".";

      return PreInstruction(str, tr("Boil/steep fermentables"), timeRemaining);
   }

   bool hasBoilFermentable() {
      for (auto const & fermentableAddition : m_self.fermentableAdditions()) {
         if (fermentableAddition->stage() == RecipeAddition::Stage::Mash || fermentableAddition->addAfterBoil()) {
            continue;
         } else {
            return true;
         }
      }
      return false;
   }

   bool hasBoilExtract() {
      for (auto const & fermentableAddition : m_self.fermentableAdditions()) {
         if (fermentableAddition->fermentable()->isExtract()) {
            return true;
         } else {
            continue;
         }
      }
      return false;
   }

   PreInstruction addExtracts(double timeRemaining) const {
      QString str = tr("Raise water to boil and then remove from heat. Stir in  ");
      for (auto const & fermentableAddition : m_self.fermentableAdditions()) {
         if (fermentableAddition->fermentable()->isExtract()) {
            str += QString("%1 %2, ")
                  .arg(Measurement::displayAmount(fermentableAddition->amount()))
                  .arg(fermentableAddition->fermentable()->name());
         }
      }
      str += ".";

      return PreInstruction(str, tr("Add Extracts to water"), timeRemaining);
   }

   void addPreinstructions(QVector<PreInstruction> preins) {
      // Add instructions in descending mash time order.
      std::sort(preins.begin(), preins.end(), std::greater<PreInstruction>());
      for (int ii = 0; ii < preins.size(); ++ii) {
         PreInstruction pi = preins[ii];

         auto ins = std::make_shared<Instruction>();
         ins->setName(pi.title);
         ins->setDirections(pi.text);
         ins->setInterval(pi.time);

         m_self.add(ins);
      }
      return;
   }


   /**
    * \brief This does the logic for \c nonOptBoil, \c nonOptFermentation, etc
    *
    * \param itemId IN/OUT
    */
   template<class T>
   std::shared_ptr<T> nonOptionalItem(int & itemId) {
      if (itemId < 0) {
         std::shared_ptr<T> item{std::make_shared<T>()};
         BtStringConst const & propertyName = Recipe::propertyNameFor<T>();
         this->m_self.setAndNotify(propertyName, itemId, ObjectStoreWrapper::insert(item));
      }
      return ObjectStoreWrapper::getById<T>(itemId);
   }

   /**
    * \brief Returns the boil size in liters, or the supplied value if there is either no boil or no boil size set on
    *        the boil.
    */
   double boilSizeInLitersOr(double const defaultValue) const {
      auto boil = this->m_self.boil();
      if (!boil) {
         return defaultValue;
      }
      return boil->preBoilSize_l().value_or(defaultValue);
   }

   /**
    * \brief Returns the boil time in minutes, or the supplied value if there is no boil.
    */
   double boilTimeInMinutesOr(double const defaultValue) const {
      auto boil = this->m_self.boil();
      if (!boil) {
         return defaultValue;
      }
      return boil->boilTime_mins();
   }

   //! \brief send me a list of salts and if we are wanting to add to the
   //! mash or the sparge, and I will return a list of instructions
   QStringList getReagents(QList<std::shared_ptr<RecipeAdjustmentSalt>> saltAdditions,
                           RecipeAdjustmentSalt::WhenToAdd wanted) {
      QStringList reagents = QStringList();

      for (auto saltAddition : saltAdditions ) {
         auto const whenToAdd = saltAddition->whenToAdd();
         auto const salt      = saltAddition->salt();
         QString tmp;

         if (whenToAdd == wanted || whenToAdd == RecipeAdjustmentSalt::WhenToAdd::Equal) {
            tmp = tr("%1 %2").arg(Measurement::displayAmount(saltAddition->amount())).arg(salt->name());
         } else if (whenToAdd == RecipeAdjustmentSalt::WhenToAdd::Ratio) {
            double ratio = 1.0;
            if (wanted == RecipeAdjustmentSalt::WhenToAdd::Sparge) {
               ratio = this->m_self.mash()->totalSpargeAmount_l() / this->m_self.mash()->totalInfusionAmount_l();
            }

            auto adjustedAmount = saltAddition->amount();
            adjustedAmount.quantity *= ratio;
            tmp = tr("%1 %2").arg(Measurement::displayAmount(adjustedAmount)).arg(salt->name());
         } else {
            continue;
         }

         if (reagents.size() > 0) {
            reagents.append(tr(", "));
         }
         reagents.append(tmp);
      }
      return reagents;
   }

   // Adds instructions to the recipe.
   void postboilFermentablesIns() {
      QString tmp;
      bool hasFerms = false;

      QString str = tr("Add ");
      for (auto const & fermentableAddition : this->m_self.fermentableAdditions()) {
         if (!fermentableAddition->addAfterBoil()) {
            continue;
         }

         hasFerms = true;
         tmp = QString("%1 %2, ")
               .arg(Measurement::displayAmount(fermentableAddition->amount()))
               .arg(fermentableAddition->fermentable()->name());
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

      this->m_self.add(ins);

      return;
   }

   void postboilIns() {
      auto equipment = this->m_self.equipment();
      if (!equipment) {
         return;
      }

      double wortInBoil_l = this->m_self.wortFromMash_l() - equipment->getLauteringDeadspaceLoss_l();
      wortInBoil_l += equipment->topUpKettle_l().value_or(0.0);

      double wort_l = equipment->wortEndOfBoil_l(wortInBoil_l);
      QString str = tr("You should have %1 wort post-boil.")
                  .arg(Measurement::displayAmount(Measurement::Amount{wort_l, Measurement::Units::liters}));
      str += tr("\nYou anticipate losing %1 to trub and chiller loss.")
            .arg(Measurement::displayAmount(Measurement::Amount{equipment->kettleTrubChillerLoss_l(), Measurement::Units::liters}));
      wort_l -= equipment->kettleTrubChillerLoss_l();
      if (equipment->topUpWater_l() > 0.0)
         str += tr("\nAdd %1 top up water into primary.")
               .arg(Measurement::displayAmount(Measurement::Amount{equipment->topUpWater_l().value_or(Equipment::default_topUpWater_l), Measurement::Units::liters}));
      wort_l += equipment->topUpWater_l().value_or(Equipment::default_topUpWater_l);
      str += tr("\nThe final volume in the primary is %1.")
            .arg(Measurement::displayAmount(Measurement::Amount{wort_l, Measurement::Units::liters}));

      auto ins = std::make_shared<Instruction>();
      ins->setName(tr("Post boil"));
      ins->setDirections(str);
      this->m_self.add(ins);

      return;
   }

   void mashFermentableIns() {
      /*** Add grains ***/
      auto ins = std::make_shared<Instruction>();
      ins->setName(tr("Add grains"));
      QString str = tr("Add ");
      QList<QString> reagents = this->m_self.getReagents(this->m_self.fermentableAdditions());

      for (int ii = 0; ii < reagents.size(); ++ii) {
         str += reagents.at(ii);
      }

      str += tr("to the mash tun.");
      ins->setDirections(str);

      this->m_self.add(ins);

      return;
   }

   void mashWaterIns() {

      if (this->m_self.mash() == nullptr) {
         return;
      }

      auto ins = std::make_shared<Instruction>();
      ins->setName(tr("Heat water"));
      QString str = tr("Bring ");
      QList<QString> reagents = this->m_self.getReagents(this->m_self.mash()->mashSteps());

      for (int ii = 0; ii < reagents.size(); ++ii) {
         str += reagents.at(ii);
      }

      str += tr("for upcoming infusions.");
      ins->setDirections(str);

      this->m_self.add(ins);

      return;
   }

   void firstWortHopsIns() {
      QList<QString> reagents = this->m_self.getReagents(this->m_self.hopAdditions(), true);
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

      this->m_self.add(ins);

      return;
   }

   void topOffIns() {
      auto equipment = this->m_self.equipment();
      if (!equipment) {
         return;
      }

      double wortInBoil_l = this->m_self.wortFromMash_l() - equipment->getLauteringDeadspaceLoss_l();
      QString str = tr("You should now have %1 wort.")
                  .arg(Measurement::displayAmount(Measurement::Amount{wortInBoil_l, Measurement::Units::liters}));
      if (!equipment->topUpKettle_l() || *equipment->topUpKettle_l() == 0.0) {
         return;
      }

      wortInBoil_l += *equipment->topUpKettle_l();
      QString tmp = tr(" Add %1 water to the kettle, bringing pre-boil volume to %2.")
                  .arg(Measurement::displayAmount(Measurement::Amount{*equipment->topUpKettle_l(), Measurement::Units::liters}))
                  .arg(Measurement::displayAmount(Measurement::Amount{wortInBoil_l, Measurement::Units::liters}));

      str += tmp;

      auto ins = std::make_shared<Instruction>();
      ins->setName(tr("Pre-boil"));
      ins->setDirections(str);
      ins->addReagent(tmp);

      this->m_self.add(ins);

      return;
   }

   void saltWater(RecipeAdjustmentSalt::WhenToAdd when) {

      if (!this->m_self.mash() || this->m_self.saltAdjustmentIds().size() == 0) {
         return;
      }

      QStringList reagents = this->getReagents(this->m_self.saltAdjustments(), when);
      if (reagents.size() == 0) {
         return;
      }

      auto ins = std::make_shared<Instruction>();
      QString tmp = when == RecipeAdjustmentSalt::WhenToAdd::Mash ? tr("mash") : tr("sparge");
      ins->setName(tr("Modify %1 water").arg(tmp));
      QString str = tr("Dissolve ");

      for (int ii = 0; ii < reagents.size(); ++ii) {
         str += reagents.at(ii);
      }

      str += QString(tr(" into the %1 water").arg(tmp));
      ins->setDirections(str);

      this->m_self.add(ins);

      return;
   }

   // Batch size without losses.
   double batchSizeNoLosses_l() {
      double ret = this->m_self.batchSize_l();
      auto equipment = this->m_self.equipment();
      if (equipment) {
         ret += equipment->kettleTrubChillerLoss_l();
      }

      return ret;
   }

   //============================================== Calculation Functions ==============================================
   /**
    * Emits changed(grains_kg), changed(grainsInMash_kg). Depends on: --.
    */
   void recalcGrains() {
      double calculatedGrains_kg = 0.0;
      double calculatedGrainsInMash_kg = 0.0;

      for (auto const & fermentableAddition : this->m_self.fermentableAdditions()) {
         if (fermentableAddition->fermentable() &&
             fermentableAddition->fermentable()->type() == Fermentable::Type::Grain) {
            // I wouldn't have thought you would want to measure grain by volume, but best to check
            if (fermentableAddition->amountIsWeight()) {
               calculatedGrains_kg += fermentableAddition->amount().quantity;
               if (fermentableAddition->stage() == RecipeAddition::Stage::Mash) {
                  calculatedGrainsInMash_kg += fermentableAddition->amount().quantity;
               }
            } else {
               qWarning() <<
                  Q_FUNC_INFO << "Ignoring grain fermentable addition #" << fermentableAddition->key() << "(" <<
                  fermentableAddition->name() << ") as measured by volume";
            }
         }
      }

      if (!qFuzzyCompare(calculatedGrains_kg, this->m_grains_kg)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated weight of grains: " << calculatedGrains_kg << ", stored weight: " << this->m_grains_kg;
         this->m_grains_kg = calculatedGrains_kg;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::grains_kg),
                                      this->m_grains_kg);
         }
      }

      if (!qFuzzyCompare(calculatedGrainsInMash_kg, this->m_grainsInMash_kg)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated weight of grains in mash: " << calculatedGrainsInMash_kg << ", stored weight: " <<
            this->m_grainsInMash_kg;
         this->m_grainsInMash_kg = calculatedGrainsInMash_kg;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::grainsInMash_kg),
                                      this->m_grainsInMash_kg);
         }
      }
      return;
   }


   /**
    * Emits changed(wortFromMash_l), changed(boilVolume_l), changed(finalVolume_l), changed(postBoilVolume_l).
    * Depends on: m_grainsInMash_kg
    */
   void recalcVolumeEstimates() {
      double tmp = 0.0;
      double calculatedWortFromMash_l = 0.0;
      double calculatedBoilVolume_l = 0.0;
      double calculatedFinalVolume_l = 0.0;
      double calculatedPostBoilVolume_l = 0.0;

      auto equipment = this->m_self.equipment();

      // wortFromMash_l ==========================
      if (!this->m_self.mash()) {
         this->m_wortFromMash_l = 0.0;
      } else {
         double waterAdded_l = this->m_self.mash()->totalMashWater_l();
         double absorption_lKg;
         if (equipment) {
            absorption_lKg = equipment->mashTunGrainAbsorption_LKg().value_or(Equipment::default_mashTunGrainAbsorption_LKg);
         } else {
            absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;
         }

         calculatedWortFromMash_l = (waterAdded_l - absorption_lKg * this->m_grainsInMash_kg);
      }

      // boilVolume_l ==============================

      if (equipment) {
         tmp = calculatedWortFromMash_l - equipment->getLauteringDeadspaceLoss_l() + equipment->topUpKettle_l().value_or(Equipment::default_topUpKettle_l);
      } else {
         tmp = calculatedWortFromMash_l;
      }

      // .:TODO:. Assumptions below about liquids are almost certainly wrong, also TBD what other cases we have to cover
      // Need to account for extract/sugar volume also.
      for (auto const & fermentableAddition : this->m_self.fermentableAdditions()) {
         auto const & fermentable = fermentableAddition->fermentable();
         switch (fermentable->type()) {
            case Fermentable::Type::Extract:
               if (fermentableAddition->amountIsWeight()) {
                  tmp += fermentableAddition->amount().quantity / PhysicalConstants::liquidExtractDensity_kgL;
               } else {
                  tmp += fermentableAddition->amount().quantity;
               }
               break;
            case Fermentable::Type::Sugar:
               if (fermentableAddition->amountIsWeight()) {
                  tmp += fermentableAddition->amount().quantity / PhysicalConstants::sucroseDensity_kgL;
               } else {
                  tmp += fermentableAddition->amount().quantity;
               }
               break;
            case Fermentable::Type::Dry_Extract:
               if (fermentableAddition->amountIsWeight()) {
                  tmp += fermentableAddition->amount().quantity / PhysicalConstants::dryExtractDensity_kgL;
               } else {
                  tmp += fermentableAddition->amount().quantity;
               }
               break;
         }
      }

      if (tmp <= 0.0) {
         // Give up.
         tmp = this->boilSizeInLitersOr(0.0);
      }

      calculatedBoilVolume_l = tmp;

      // finalVolume_l ==============================

      // NOTE: the following figure is not based on the other volume estimates
      // since we want to show og,fg,ibus,etc. as if the collected wort is correct.
      this->m_finalVolumeNoLosses_l = this->batchSizeNoLosses_l();
      if (equipment) {
         //_finalVolumeNoLosses_l = equipment->wortEndOfBoil_l(calculatedBoilVolume_l) + equipment->topUpWater_l();
         calculatedFinalVolume_l =
            equipment->wortEndOfBoil_l(calculatedBoilVolume_l) +
            equipment->topUpWater_l().value_or(Equipment::default_topUpWater_l) -
            equipment->kettleTrubChillerLoss_l();
      } else {
         this->m_finalVolume_l = calculatedBoilVolume_l - 4.0; // This is just shooting in the dark. Can't do much without an equipment.
         //_finalVolumeNoLosses_l = _finalVolume_l;
      }

      // postBoilVolume_l ===========================

      if (equipment) {
         calculatedPostBoilVolume_l = equipment->wortEndOfBoil_l(calculatedBoilVolume_l);
      } else {
         calculatedPostBoilVolume_l = this->m_self.batchSize_l(); // Give up.
      }

      if (!qFuzzyCompare(calculatedWortFromMash_l, this->m_wortFromMash_l)) {
//         qDebug() <<
//            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
//            "Calculated wort from mash: " << calculatedWortFromMash_l << ", stored: " << this->m_wortFromMash_l;
         this->m_wortFromMash_l = calculatedWortFromMash_l;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::wortFromMash_l),
                                      this->m_wortFromMash_l);
         }
      }

      // TODO: Still need to get rid of m_boilVolume_l
      if (!qFuzzyCompare(calculatedBoilVolume_l, this->m_boilVolume_l)) {
//         qDebug() <<
//            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
//            "Calculated boil volume: " << calculatedBoilVolume_l << ", stored: " << this->m_boilVolume_l;
         this->m_boilVolume_l = calculatedBoilVolume_l;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::boilVolume_l),
                                      this->m_boilVolume_l);
         }
      }

      if (! qFuzzyCompare(calculatedFinalVolume_l, this->m_finalVolume_l)) {
//         qDebug() <<
//            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
//            "Calculated final volume: " << calculatedFinalVolume_l << ", stored: " << this->m_finalVolume_l;
         this->m_finalVolume_l = calculatedFinalVolume_l;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::finalVolume_l),
                                      this->m_finalVolume_l);
         }
      }

      if (! qFuzzyCompare(calculatedPostBoilVolume_l, this->m_postBoilVolume_l)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated post boil volume: " << calculatedPostBoilVolume_l << ", stored: " << this->m_postBoilVolume_l;
         this->m_postBoilVolume_l = calculatedPostBoilVolume_l;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::postBoilVolume_l),
                                      this->m_postBoilVolume_l);
         }
      }
      return;
   }

   /**
    * Emits changed(color_srm). Depends on: m_finalVolume_l
    */
   void recalcColor_srm() {
      double mcu = 0.0;

      for (auto const & fermentableAddition : this->m_self.fermentableAdditions()) {
         if (fermentableAddition->amountIsWeight()) {
            // Conversion factor for lb/gal to kg/l = 8.34538.
            mcu += fermentableAddition->fermentable()->color_srm() * 8.34538 * fermentableAddition->amount().quantity / m_finalVolumeNoLosses_l;
         } else {
            // .:TBD:. What do do about liquids
            qWarning() <<
               Q_FUNC_INFO << "Unimplemented branch for handling color of liquid fermentables - #" <<
               fermentableAddition->fermentable()->key() << ":" << fermentableAddition->name();
         }
      }

      double calculatedColor_srm = ColorMethods::mcuToSrm(mcu);
      if (!qFuzzyCompare(this->m_color_srm, calculatedColor_srm)) {
//         qDebug() <<
//            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
//            "Calculated color: " << calculatedColor_srm << ", stored: " << this->m_color_srm;
         this->m_color_srm = calculatedColor_srm;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::color_srm), this->m_color_srm);
         }
      }

      return;
   }

   /**
    * Emits changed(SRMColor). Depends on: m_color_srm.
    */
   void recalcSRMColor() {
      QColor calculatedSRMColor = Algorithms::srmToColor(this->m_color_srm);
      if (calculatedSRMColor != this->m_SRMColor) {
         this->m_SRMColor = calculatedSRMColor;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::SRMColor), this->m_SRMColor);
         }
      }
      return;
   }

   /**
    * Emits changed(og), changed(fg).
    * Depends on: m_wortFromMash_l, m_finalVolume_l
    */
   void recalcOgFg() {

      this->m_og_fermentable = this->m_fg_fermentable = 0.0;

      // The first time through really has to get the m_og and m_fg from the
      // database, not use the initialized values of 1. I (maf) tried putting
      // this in the initialize, but it just hung. So I moved it here, but only
      // if we aren't initialized yet.
      //
      // GSG: This doesn't work, this og and fg are already set to 1.0 so
      // until we load these values from the database on startup, we have
      // to calculate.
      if (this->m_self.m_uninitializedCalcs) {
         this->m_self.m_og = Localization::toDouble(this->m_self, PropertyNames::Recipe::og, Q_FUNC_INFO);
         this->m_self.m_fg = Localization::toDouble(this->m_self, PropertyNames::Recipe::fg, Q_FUNC_INFO);
      }

      // Find out how much sugar we have.
      auto const sugars = this->m_self.calcTotalPoints();
      double sugar_kg                  = sugars.sugar_kg;  // Mass of sugar that *is* affected by mash efficiency
      double sugar_kg_ignoreEfficiency = sugars.sugar_kg_ignoreEfficiency;  // Mass of sugar that *is not* affected by mash efficiency
      double nonFermentableSugars_kg   = sugars.nonFermentableSugars_kg;  // Mass of sugar that is not fermentable (also counted in sugar_kg_ignoreEfficiency)

      // Uncomment for diagnosing problems with calculations
//      qDebug() <<
//         Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
//         "sugar_kg: " << sugar_kg << ", sugar_kg_ignoreEfficiency: " << sugar_kg_ignoreEfficiency <<
//         ", nonFermentableSugars_kg:" << nonFermentableSugars_kg;

      // We might lose some sugar in the form of Trub/Chiller loss and lauter deadspace.
      auto equipment = this->m_self.equipment();
      if (equipment) {
         double const kettleWort_l = (this->m_wortFromMash_l - equipment->getLauteringDeadspaceLoss_l()) +
                                     equipment->topUpKettle_l().value_or(Equipment::default_topUpKettle_l);
         double const postBoilWort_l = equipment->wortEndOfBoil_l(kettleWort_l);
         double ratio = (postBoilWort_l - equipment->kettleTrubChillerLoss_l()) / postBoilWort_l;
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
      sugar_kg = sugar_kg * this->m_self.efficiency_pct() / 100.0 + sugar_kg_ignoreEfficiency;
      double plato = Algorithms::getPlato(sugar_kg, this->m_finalVolumeNoLosses_l);

      double calculatedOg = Algorithms::PlatoToSG_20C20C(plato);    // og from all sugars
      double tmp_pnts = (calculatedOg - 1) * 1000.0; // points from all sugars
      double tmp_nonferm_pnts;
      if (nonFermentableSugars_kg != 0.0) {
         double ferm_kg = sugar_kg - nonFermentableSugars_kg;  // Mass of only fermentable sugars
         plato = Algorithms::getPlato(ferm_kg, this->m_finalVolumeNoLosses_l);   // Plato from fermentable sugars
         this->m_og_fermentable = Algorithms::PlatoToSG_20C20C(plato);    // og from only fermentable sugars
         plato = Algorithms::getPlato(nonFermentableSugars_kg, this->m_finalVolumeNoLosses_l);   // Plate from non-fermentable sugars
         tmp_nonferm_pnts = ((Algorithms::PlatoToSG_20C20C(plato)) - 1) * 1000.0; // og points from non-fermentable sugars
      } else {
         this->m_og_fermentable = calculatedOg;
         tmp_nonferm_pnts = 0.0;
      }

      // Calculage FG
      double attenuation_pct = 0.0;
      for (auto yeastAddition : this->m_self.yeastAdditions()) {
         // Get the yeast with the greatest attenuation.
         if (yeastAddition->attenuation_pct() > attenuation_pct) {
            attenuation_pct = yeastAddition->yeast()->attenuationTypical_pct();
         }
      }
      // This means we have yeast, but they neglected to provide attenuation percentages.
      if (this->m_self.yeastAdditions().size() > 0 && attenuation_pct <= 0.0)  {
         attenuation_pct = Yeast::DefaultAttenuation_pct; // Use an average attenuation.
      }

      double calculatedFg;
      if (nonFermentableSugars_kg != 0.0) {
         double tmp_ferm_pnts = (tmp_pnts - tmp_nonferm_pnts) * (1.0 - attenuation_pct / 100.0); // fg points from fermentable sugars
         tmp_pnts = tmp_ferm_pnts + tmp_nonferm_pnts;  // FG points from both fermentable and non-fermentable sugars
         //tmp_pnts *= (1.0 - attenuation_pct/100.0);  // WTF, this completely ignores all the calculations about non-fermentable sugars and just converts everything!
         calculatedFg =  1 + tmp_pnts / 1000.0; // new FG value
         this->m_fg_fermentable =  1 + tmp_ferm_pnts / 1000.0; // FG from fermentables only
      } else {
         tmp_pnts *= (1.0 - attenuation_pct / 100.0);
         calculatedFg =  1 + tmp_pnts / 1000.0;
         this->m_fg_fermentable = calculatedFg;
      }

      // Uncomment for diagnosing problems with calculations
//      qDebug() <<
//         Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
//         "m_og_fermentable: " << m_og_fermentable << ", m_fg_fermentable: " << m_fg_fermentable;

      if (!qFuzzyCompare(this->m_self.m_og, calculatedOg)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated OG: " << calculatedOg << ", stored: " << this->m_self.m_og;
         this->m_self.m_og = calculatedOg;
         // NOTE: We don't want to do this on the first load of the recipe.
         // NOTE: We are we recalculating all of these on load? Shouldn't we be
         // reading these values from the database somehow?
         //
         // GSG: Yes we can, but until the code is added to intialize these calculated
         // values from the database, we can calculate them on load. They should be
         // the same as the database values since the database values were set with
         // these functions in the first place.
         if (!this->m_self.m_uninitializedCalcs) {
            this->m_self.propagatePropertyChange(PropertyNames::Recipe::og, false);
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::og    ), this->m_self.m_og);
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::points), (this->m_self.m_og - 1.0) * 1e3);
         }
      }

      if (!qFuzzyCompare(this->m_self.m_fg, calculatedFg)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated FG: " << calculatedFg << ", stored: " << this->m_self.m_fg;
         this->m_self.m_fg = calculatedFg;
         if (!this->m_self.m_uninitializedCalcs) {
            this->m_self.propagatePropertyChange(PropertyNames::Recipe::fg, false);
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::fg), this->m_self.m_fg);
         }
      }
      return;
   }


   /**
    * Emits changed(ABV_pct). Depends on: m_og, m_fg
    */
   void recalcABV_pct() {
      // The complex formula, and variations comes from Ritchie Products Ltd, (Zymurgy, Summer 1995, vol. 18, no. 2)
      // Michael L. Hall’s article Brew by the Numbers: Add Up What’s in Your Beer, and Designing Great Beers by Daniels.
      double calculatedABV_pct =
         (76.08 * (this->m_og_fermentable - this->m_fg_fermentable) / (1.775 - this->m_og_fermentable)) *
         (this->m_fg_fermentable / 0.794);

      if (!qFuzzyCompare(calculatedABV_pct, m_ABV_pct)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated ABV: " << calculatedABV_pct << ", stored: " << this->m_ABV_pct;
         this->m_ABV_pct = calculatedABV_pct;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::ABV_pct), this->m_ABV_pct);
         }
      }
      return;
   }

   /**
    * Emits changed(boilGrav). Depends on: _postBoilVolume_l, _boilVolume_l
    */
   void recalcBoilGrav() {
      auto const sugars = this->m_self.calcTotalPoints();
      double sugar_kg                  = sugars.sugar_kg;
      double sugar_kg_ignoreEfficiency = sugars.sugar_kg_ignoreEfficiency;
      double lateAddition_kg           = sugars.lateAddition_kg;
      double lateAddition_kg_ignoreEff = sugars.lateAddition_kg_ignoreEff;

      // Since the efficiency refers to how much sugar we get into the fermenter,
      // we need to adjust for that here.
      sugar_kg = (this->m_self.efficiency_pct() / 100.0 * (sugar_kg - lateAddition_kg) + sugar_kg_ignoreEfficiency -
                  lateAddition_kg_ignoreEff);

      double calculatedBoilGrav = Algorithms::PlatoToSG_20C20C(Algorithms::getPlato(sugar_kg,
                                                                                    this->boilSizeInLitersOr(0.0)));
      if (! qFuzzyCompare(calculatedBoilGrav, this->m_boilGrav)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated Boil Grav: " << calculatedBoilGrav << ", stored: " << this->m_boilGrav;
         this->m_boilGrav = calculatedBoilGrav;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::boilGrav), this->m_boilGrav);
         }
      }
      return;
   }

   /**
    * Emits changed(IBU). Depends on: _batchSize_l, _boilGrav, _boilVolume_l, _finalVolume_l
    */
   void recalcIBU() {
      double calculatedIbu = 0.0;

      // Bitterness due to hops...
      //
      // Note that, normally, we don't want to take a reference to a smart pointer.  However, in this context, it's safe
      // (because the hop additions aren't going to change while we look at them) and it gets rid of a compiler warning.
      this->m_ibus.clear();
      for (auto const & hopAddition : this->m_self.hopAdditions()) {
         double tmp = this->m_self.ibuFromHopAddition(*hopAddition);
         this->m_ibus.append(tmp);
         calculatedIbu += tmp;
      }

      // Bitterness due to hopped extracts...
      for (auto const & fermentableAddition : this->m_self.fermentableAdditions()) {
         if (fermentableAddition->amountIsWeight()) {
            // Conversion factor for lb/gal to kg/l = 8.34538.
            calculatedIbu += fermentableAddition->fermentable()->ibuGalPerLb().value_or(0.0) * (fermentableAddition->amount().quantity / this->m_self.batchSize_l()) / 8.34538;
         } else {
            // .:TBD:. What do do about liquids
            qWarning() <<
               Q_FUNC_INFO << "Unimplemented branch for handling IBU of liquid fermentables - #" <<
               fermentableAddition->fermentable()->key() << ":" << fermentableAddition->name();
         }
      }

      if (! qFuzzyCompare(calculatedIbu, this->m_IBU)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated IBU: " << calculatedIbu << ", stored: " << this->m_IBU;
         this->m_IBU = calculatedIbu;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::IBU), this->m_IBU);
         }
      }

      return;
   }

   /**
    * Emits changed(calories). Depends on: m_og, m_fg.
    */
   void recalcCalories() {
      //
      // The Journal of the Institute of Brewing (JIB) is published by the Institute of Brewing and Distilling.
      // On pages 320-321 of Volume 88 of the JIB, dated "September - October 1982", there is an article on "Calculation of
      // Calorific Value of Beer" submitted by P A Martin on behalf of the IOB (Institute of Brewing) Analysis Committee.
      //
      // The article discusses four methods for calculating the calories in beer, and, in summary, recommends calculating
      // Calories/100ml as follows:
      //    1.1 Estimate the alcohol content of the beer ... [and] convert ... to alcohol g/100ml
      //    1.2 Estimate total carbohydrate of the beer (g/100ml as glucose) ...
      //    1.3 Estimate protein content of the beer (g/100ml)
      //    2.1 Calories/100ml = [alcohol (g/100ml) × 7] +
      //                         [total carbohydrate (as glucose g/100ml) × 3.75] +
      //                         [protein (g/100ml) × 4]
      //    2.2 In a collaborative trial the precision of the method was ±2.02 Calories for highly attenuated beers and
      //        ±3.06 Calories for normally fermented products.
      //
      // We should come back to this at some point...
      //
      // the formula in here are taken from http://hbd.org/ensmingr/
      //

      // Need to translate OG and FG into plato

      double const startPlato  = Measurement::Units::plato.fromCanonical(this->m_self.m_og);
      double const finishPlato = Measurement::Units::plato.fromCanonical(this->m_self.m_fg);

      double const realExtract = (0.1808 * startPlato) + (0.8192 * finishPlato);

      // Alcohol by weight?
      double const abw = (startPlato - realExtract) / (2.0665 - (0.010665 * startPlato));

      // The final results of this formula are calories per 100 ml.
      // The 10.0 puts it in terms of liters.
      double calculatedCaloriesPerLiter = ((6.9 * abw) + 4.0 * (realExtract - 0.1)) * this->m_self.m_fg * 10.0;

      //! If there are no fermentables in the recipe, if there is no mash, etc.,
      //  then the calories/12 oz ends up negative. Since negative doesn't make
      //  sense, set it to 0
      if (calculatedCaloriesPerLiter < 0) {
         calculatedCaloriesPerLiter = 0;
      }

      if (!qFuzzyCompare(calculatedCaloriesPerLiter, this->m_caloriesPerLiter)) {
         qDebug() <<
            Q_FUNC_INFO << "Recipe #" << this->m_self.key() << "(" << this->m_self.name() << ") "
            "Calculated calories/liter: " << calculatedCaloriesPerLiter << ", stored: " << this->m_caloriesPerLiter;
         this->m_caloriesPerLiter = calculatedCaloriesPerLiter;
         if (!this->m_self.m_uninitializedCalcs) {
            emit this->m_self.changed(this->m_self.metaProperty(*PropertyNames::Recipe::caloriesPerLiter),
                                      this->m_caloriesPerLiter);
         }
      }
      return;
   }


   //================================================ Member variables =================================================
   Recipe & m_self;
   QVector<int> instructionIds;

   // Calculated properties.
   double        m_ABV_pct              ;
   double        m_color_srm            ;
   double        m_boilGrav             ;
   double        m_IBU                  ;
   QList<double> m_ibus                 ;
   double        m_wortFromMash_l       ;
   double        m_boilVolume_l         ;
   double        m_postBoilVolume_l     ;
   double        m_finalVolume_l        ;
   // Final volume before any losses out of the kettle, used in calculations for sg/ibu/etc.
   double        m_finalVolumeNoLosses_l;
   double        m_caloriesPerLiter     ;
   double        m_grainsInMash_kg      ;
   double        m_grains_kg            ;
   QColor        m_SRMColor             ;
   double        m_og_fermentable       ;
   double        m_fg_fermentable       ;

};

template<> QVector<int> & Recipe::impl::accessIds<Instruction>() { return this->instructionIds; }

QString Recipe::localisedName() { return tr("Recipe"); }

// Note that Recipe::typeStringMapping and Recipe::FormMapping are as defined by BeerJSON, but we also use them for the DB and
// for the UI.  We can't use them for BeerXML as it only supports subsets of these types.
EnumStringMapping const Recipe::typeStringMapping {
   {Recipe::Type::Extract    , "extract"     },
   {Recipe::Type::PartialMash, "partial mash"},
   {Recipe::Type::AllGrain   , "all grain"   },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   {Recipe::Type::Cider      , "cider"       },
   {Recipe::Type::Kombucha   , "kombucha"    },
   {Recipe::Type::Soda       , "soda"        },
   {Recipe::Type::Other      , "other"       },
   {Recipe::Type::Mead       , "mead"        },
   {Recipe::Type::Wine       , "wine"        },
};

EnumStringMapping const Recipe::typeDisplayNames {
   {Recipe::Type::Extract    , tr("Extract"     )},
   {Recipe::Type::PartialMash, tr("Partial Mash")},
   {Recipe::Type::AllGrain   , tr("All Grain"   )},
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   {Recipe::Type::Cider      , tr("Cider"       )},
   {Recipe::Type::Kombucha   , tr("Kombucha"    )},
   {Recipe::Type::Soda       , tr("Soda"        )},
   {Recipe::Type::Other      , tr("Other"       )},
   {Recipe::Type::Mead       , tr("Mead"        )},
   {Recipe::Type::Wine       , tr("Wine"        )},
};


bool Recipe::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Recipe const & rhs = static_cast<Recipe const &>(other);

   // Base class will already have ensured names are equal
   return (
      Utils::AutoCompare(this->m_type          , rhs.m_type          ) &&
      Utils::AutoCompare(this->m_batchSize_l   , rhs.m_batchSize_l   ) &&
      Utils::AutoCompare(this->m_efficiency_pct, rhs.m_efficiency_pct) &&
      Utils::AutoCompare(this->m_age_days      , rhs.m_age_days      ) &&
      Utils::AutoCompare(this->m_ageTemp_c     , rhs.m_ageTemp_c     ) &&
      Utils::AutoCompare(this->m_og            , rhs.m_og            ) &&
      Utils::AutoCompare(this->m_fg            , rhs.m_fg            ) &&
      ObjectStoreWrapper::compareById<Style    >(this->m_styleId,     rhs.m_styleId    ) &&
      ObjectStoreWrapper::compareById<Mash     >(this->m_mashId,      rhs.m_mashId     ) &&
      ObjectStoreWrapper::compareById<Boil     >(this->m_boilId,      rhs.m_boilId     ) &&
      //
      // We don't include any of the following in the equality test:
      //    - BrewNotes as those are records of actually brewing a Recipe and shouldn't form part of determining whether
      //      two Recipes are identical.
      //    - Instructions, since these are generated from the Recipe
      //    - Equipment, as you could brew the same recipe on different sets of equipment
      //    - Salt additions, since salts are typically added to correct water profiles
      //
//      ObjectStoreWrapper::compareById<Equipment>(this->m_equipmentId, rhs.m_equipmentId) &&
//      ObjectStoreWrapper::compareListByIds<Instruction      >(this->pimpl->instructionIds  , rhs.pimpl->instructionIds) &&
      // The comparisons for each type of addition depend on them being in some canonical ordering that does not depend
      // on their database IDs.  However, we don't have to worry about this here.  The AutoCompare does the sorting for
      // us (on copies of the lists) using the operator<=> defined in RecipeAdditionBase.
      Utils::AutoCompare(this->fermentableAdditions(), rhs.fermentableAdditions()) &&
      Utils::AutoCompare(this->        hopAdditions(), rhs.        hopAdditions()) &&
      Utils::AutoCompare(this->       miscAdditions(), rhs.       miscAdditions()) &&
      Utils::AutoCompare(this->      yeastAdditions(), rhs.      yeastAdditions()) &&
      Utils::AutoCompare(this->waterUses           (), rhs.waterUses           ())

   );
}

ObjectStore & Recipe::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Recipe>::getInstance();
}

TypeLookup const Recipe::typeLookup {
   "Recipe",
   {
      // Note that the age_days properties is dimensionless because:
      //    - It's not meaningful to measure it with greater precision
      //    - The canonical unit for Measurement::PhysicalQuantity::Time is Measurement::Units::minutes, so we'd have to
      //      either store as minutes or do some special-case handling to say we're not storing in canonical units.
      //      Both would be ugly -- but doable, as we have done elsewhere
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::type              , Recipe::m_type              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::brewer            , Recipe::m_brewer            ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::asstBrewer        , Recipe::m_asstBrewer        ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::batchSize_l       , Recipe::m_batchSize_l       , Measurement::PhysicalQuantity::Volume        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::efficiency_pct    , Recipe::m_efficiency_pct    ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::age_days          , Recipe::m_age_days          ,           NonPhysicalQuantity::Dimensionless ), // See comment above for why Dimensionless, not Time
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::ageTemp_c         , Recipe::m_ageTemp_c         , Measurement::PhysicalQuantity::Temperature   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::date              , Recipe::m_date              ,           NonPhysicalQuantity::Date          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::carbonation_vols  , Recipe::m_carbonation_vols  , Measurement::PhysicalQuantity::Carbonation   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::forcedCarbonation , Recipe::m_forcedCarbonation ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::primingSugarName  , Recipe::m_primingSugarName  ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::carbonationTemp_c , Recipe::m_carbonationTemp_c , Measurement::PhysicalQuantity::Temperature   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::primingSugarEquiv , Recipe::m_primingSugarEquiv ,           NonPhysicalQuantity::Dimensionless ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::kegPrimingFactor  , Recipe::m_kegPrimingFactor  ,           NonPhysicalQuantity::Dimensionless ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::notes             , Recipe::m_notes             ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::tasteNotes        , Recipe::m_tasteNotes        ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::tasteRating       , Recipe::m_tasteRating       ,           NonPhysicalQuantity::Dimensionless ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::styleId           , Recipe::m_styleId           ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::mashId            , Recipe::m_mashId            ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::boilId            , Recipe::m_boilId            ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::fermentationId    , Recipe::m_fermentationId    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::equipmentId       , Recipe::m_equipmentId       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::og                , Recipe::m_og                , Measurement::PhysicalQuantity::Density       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::fg                , Recipe::m_fg                , Measurement::PhysicalQuantity::Density       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::locked            , Recipe::m_locked            ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::calcsEnabled      , Recipe::m_calcsEnabled      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::ancestorId        , Recipe::m_ancestor_id       ),

      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::ABV_pct          , Recipe::ABV_pct         ,           NonPhysicalQuantity::Percentage   ), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::boilGrav         , Recipe::boilGrav        , Measurement::PhysicalQuantity::Density      ), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::boilVolume_l     , Recipe::boilVolume_l    , Measurement::PhysicalQuantity::Volume       ), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::caloriesPerLiter , Recipe::caloriesPerLiter,           NonPhysicalQuantity::Dimensionless), // Calculated, not in DB .:TBD:. One day this should perhaps become Measurement::PhysicalQuantity::Energy
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::caloriesPerUs12oz, Recipe::caloriesPerUs12oz,         NonPhysicalQuantity::Dimensionless),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::caloriesPerUsPint, Recipe::caloriesPerUsPint,         NonPhysicalQuantity::Dimensionless),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::color_srm        , Recipe::color_srm         , Measurement::PhysicalQuantity::Color     ), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::finalVolume_l    , Recipe::finalVolume_l     , Measurement::PhysicalQuantity::Volume    ), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::grainsInMash_kg  , Recipe::grainsInMash_kg   , Measurement::PhysicalQuantity::Mass      ), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::grains_kg        , Recipe::grains_kg         , Measurement::PhysicalQuantity::Mass      ), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::IBU              , Recipe::IBU               , Measurement::PhysicalQuantity::Bitterness), // Calculated, not in DB
//      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::IBUs              , Recipe::m_IBUs              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::instructionIds    , Recipe::impl::instructionIds),
//      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::instructions      , Recipe::m_instructions      ),
//      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::points            , Recipe::m_points            ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::postBoilVolume_l, Recipe::postBoilVolume_l  , Measurement::PhysicalQuantity::Volume     ), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::SRMColor        , Recipe::SRMColor          ),  // Calculated, not in DB.  NB: This is an RGB display color
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::wortFromMash_l  , Recipe::wortFromMash_l    , Measurement::PhysicalQuantity::Volume     ), // Calculated, not in DB

      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::style               , Recipe::style               ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::mash                , Recipe::mash                ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::boil                , Recipe::boil                ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::fermentation        , Recipe::fermentation        ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::equipment           , Recipe::equipment           ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::brewNotes           , Recipe::brewNotes           ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::fermentableAdditions, Recipe::fermentableAdditions),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::hopAdditions        , Recipe::hopAdditions        ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::miscAdditions       , Recipe::miscAdditions       ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::waterUses           , Recipe::waterUses           ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::yeastAdditions      , Recipe::yeastAdditions      ),

      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::beerAcidity_pH         , Recipe::m_beerAcidity_pH         , Measurement::PhysicalQuantity::Acidity   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Recipe::apparentAttenuation_pct, Recipe::m_apparentAttenuation_pct,           NonPhysicalQuantity::Percentage),

      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::fermentableAdditionIds, Recipe::fermentableAdditionIds),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::hopAdditionIds        , Recipe::hopAdditionIds        ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::miscAdditionIds       , Recipe::miscAdditionIds       ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::yeastAdditionIds      , Recipe::yeastAdditionIds      ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::saltAdjustmentIds     , Recipe::saltAdjustmentIds     ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Recipe::waterUseIds           , Recipe::waterUseIds           ),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderBase<Recipe>::typeLookup)}
};
static_assert(std::is_base_of<FolderBase<Recipe>, Recipe>::value);

Recipe::Recipe(QString name) :
   NamedEntity              {name, true                   },
   FolderBase<Recipe>       {},
   pimpl                    {std::make_unique<impl>(*this)},
   m_type                   {Recipe::Type::AllGrain       },
   m_brewer                 {""                           },
   m_asstBrewer             {QString{"%1: free beer software"}.arg(CONFIG_APPLICATION_NAME_UC)},
   m_batchSize_l            {0.0                 },
   m_efficiency_pct         {0.0                 },
   m_age_days               {std::nullopt        },
   m_ageTemp_c              {std::nullopt        },
   m_date                   {QDate::currentDate()}, // Date is allowed to be blank, but we default it to today
   m_carbonation_vols       {std::nullopt        },
   m_forcedCarbonation      {false               },
   m_primingSugarName       {""                  },
   m_carbonationTemp_c      {0.0                 },
   m_primingSugarEquiv      {0.0                 },
   m_kegPrimingFactor       {0.0                 },
   m_notes                  {""                  },
   m_tasteNotes             {""                  },
   m_tasteRating            {0.0                 },
   m_styleId                {-1                  },
   m_equipmentId            {-1                  },
   m_mashId                 {-1                  },
   m_boilId                 {-1                  },
   m_fermentationId         {-1                  },
   m_beerAcidity_pH         {std::nullopt        },
   m_apparentAttenuation_pct{std::nullopt        },
   m_og                     {1.0                 },
   m_fg                     {1.0                 },
   m_locked                 {false               },
   m_calcsEnabled           {true                },
   m_uninitializedCalcs     {true                },
   m_uninitializedCalcsMutex{},
   m_recalcMutex            {},
   m_ancestor_id            {-1                  },
   m_ancestors              {},
   m_hasDescendants         {false               } {

   CONSTRUCTOR_END
   return;
}

Recipe::Recipe(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity          {namedParameterBundle},
   FolderBase<Recipe>   {namedParameterBundle},
   pimpl                {std::make_unique<impl>(*this)},
   SET_REGULAR_FROM_NPB (m_type                   , namedParameterBundle, PropertyNames::Recipe::type                   ),
   SET_REGULAR_FROM_NPB (m_brewer                 , namedParameterBundle, PropertyNames::Recipe::brewer                 , ""),
   SET_REGULAR_FROM_NPB (m_asstBrewer             , namedParameterBundle, PropertyNames::Recipe::asstBrewer             , ""),
   SET_REGULAR_FROM_NPB (m_batchSize_l            , namedParameterBundle, PropertyNames::Recipe::batchSize_l            ),
   SET_REGULAR_FROM_NPB (m_efficiency_pct         , namedParameterBundle, PropertyNames::Recipe::efficiency_pct         ),
   SET_REGULAR_FROM_NPB (m_age_days               , namedParameterBundle, PropertyNames::Recipe::age_days               , std::nullopt),
   SET_REGULAR_FROM_NPB (m_ageTemp_c              , namedParameterBundle, PropertyNames::Recipe::ageTemp_c              , std::nullopt),
   SET_REGULAR_FROM_NPB (m_date                   , namedParameterBundle, PropertyNames::Recipe::date                   ),
   SET_REGULAR_FROM_NPB (m_carbonation_vols       , namedParameterBundle, PropertyNames::Recipe::carbonation_vols       ),
   SET_REGULAR_FROM_NPB (m_forcedCarbonation      , namedParameterBundle, PropertyNames::Recipe::forcedCarbonation      ),
   SET_REGULAR_FROM_NPB (m_primingSugarName       , namedParameterBundle, PropertyNames::Recipe::primingSugarName       , ""),
   SET_REGULAR_FROM_NPB (m_carbonationTemp_c      , namedParameterBundle, PropertyNames::Recipe::carbonationTemp_c      ),
   SET_REGULAR_FROM_NPB (m_primingSugarEquiv      , namedParameterBundle, PropertyNames::Recipe::primingSugarEquiv      ),
   SET_REGULAR_FROM_NPB (m_kegPrimingFactor       , namedParameterBundle, PropertyNames::Recipe::kegPrimingFactor       ),
   SET_REGULAR_FROM_NPB (m_notes                  , namedParameterBundle, PropertyNames::Recipe::notes                  , ""),
   SET_REGULAR_FROM_NPB (m_tasteNotes             , namedParameterBundle, PropertyNames::Recipe::tasteNotes             , ""),
   SET_REGULAR_FROM_NPB (m_tasteRating            , namedParameterBundle, PropertyNames::Recipe::tasteRating            ),
   // Although some of these IDs are not really optional, we need default values for them for when reading from BeerXML or BeerJSON
   SET_REGULAR_FROM_NPB (m_styleId                , namedParameterBundle, PropertyNames::Recipe::styleId                , -1),
   SET_REGULAR_FROM_NPB (m_equipmentId            , namedParameterBundle, PropertyNames::Recipe::equipmentId            , -1),
   SET_REGULAR_FROM_NPB (m_mashId                 , namedParameterBundle, PropertyNames::Recipe::mashId                 , -1),
   SET_REGULAR_FROM_NPB (m_boilId                 , namedParameterBundle, PropertyNames::Recipe::boilId                 , -1),
   SET_REGULAR_FROM_NPB (m_fermentationId         , namedParameterBundle, PropertyNames::Recipe::fermentationId         , -1),
   SET_REGULAR_FROM_NPB (m_beerAcidity_pH         , namedParameterBundle, PropertyNames::Recipe::beerAcidity_pH         , std::nullopt),
   SET_REGULAR_FROM_NPB (m_apparentAttenuation_pct, namedParameterBundle, PropertyNames::Recipe::apparentAttenuation_pct, std::nullopt),
   // Note that, although we read them in here, the OG and FG are going to get recalculated when someone first tries to
   // access them.
   SET_REGULAR_FROM_NPB (m_og                     , namedParameterBundle, PropertyNames::Recipe::og                     ),
   SET_REGULAR_FROM_NPB (m_fg                     , namedParameterBundle, PropertyNames::Recipe::fg                     ),
   SET_REGULAR_FROM_NPB (m_locked                 , namedParameterBundle, PropertyNames::Recipe::locked                 , false),
                         m_calcsEnabled           {true},
                         m_uninitializedCalcs     {true},
                         m_uninitializedCalcsMutex{},
                         m_recalcMutex            {},
   SET_REGULAR_FROM_NPB (m_ancestor_id            , namedParameterBundle, PropertyNames::Recipe::ancestorId             , -1),
                         m_ancestors              {},
                         m_hasDescendants         {false} {
   // At this stage, we haven't set any Hops, Fermentables, etc.  This is deliberate because the caller typically needs
   // to access subsidiary records to obtain this info.   Callers will usually use setters (setHopIds, etc but via
   // setProperty) to finish constructing the object.

   CONSTRUCTOR_END
   return;
}

Recipe::Recipe(Recipe const & other) :
   NamedEntity{other},
   FolderBase<Recipe>{other},
   pimpl{std::make_unique<impl>(*this)},
   m_type                   {other.m_type              },
   m_brewer                 {other.m_brewer            },
   m_asstBrewer             {other.m_asstBrewer        },
   m_batchSize_l            {other.m_batchSize_l       },
   m_efficiency_pct         {other.m_efficiency_pct    },
   m_age_days               {other.m_age_days          },
   m_ageTemp_c              {other.m_ageTemp_c         },
   m_date                   {other.m_date              },
   m_carbonation_vols       {other.m_carbonation_vols  },
   m_forcedCarbonation      {other.m_forcedCarbonation },
   m_primingSugarName       {other.m_primingSugarName  },
   m_carbonationTemp_c      {other.m_carbonationTemp_c },
   m_primingSugarEquiv      {other.m_primingSugarEquiv },
   m_kegPrimingFactor       {other.m_kegPrimingFactor  },
   m_notes                  {other.m_notes             },
   m_tasteNotes             {other.m_tasteNotes        },
   m_tasteRating            {other.m_tasteRating       },
   m_styleId                {other.m_styleId           },  // But see additional logic in body
   m_equipmentId            {other.m_equipmentId       },  // But see additional logic in body
   m_mashId                 {other.m_mashId            },  // But see additional logic in body
   m_boilId                 {other.m_boilId            },  // But see additional logic in body
   m_fermentationId         {other.m_fermentationId    },  // But see additional logic in body
   m_beerAcidity_pH         {other.m_beerAcidity_pH    },
   m_apparentAttenuation_pct{other.m_apparentAttenuation_pct},
   m_og                     {other.m_og                },
   m_fg                     {other.m_fg                },
   m_locked                 {other.m_locked            },
   m_calcsEnabled           {other.m_calcsEnabled      },
   m_uninitializedCalcs     {true                      },
   m_uninitializedCalcsMutex{},
   m_recalcMutex            {},
   // Copying a Recipe doesn't copy its descendants
   m_ancestor_id            {-1                        },
   m_ancestors              {},
   m_hasDescendants         {false                     } {
   setObjectName("Recipe"); // .:TBD:. Would be good to understand whether/why we need this

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
   this->pimpl->copyAdditions<RecipeAdditionFermentable>(other);
   this->pimpl->copyAdditions<RecipeAdditionHop        >(other);
   this->pimpl->copyAdditions<RecipeAdditionMisc       >(other);
   this->pimpl->copyAdditions<RecipeAdditionYeast      >(other);
   this->pimpl->copyAdditions<RecipeAdjustmentSalt     >(other);
   this->pimpl->copyAdditions<RecipeUseOfWater         >(other);
   this->pimpl->copyInstructions(*this, other);

   //
   // You might think that Style, Mash and Equipment could safely be shared between Recipes.   However, AFAICT, none of
   // them is.  Presumably this is because users expect to be able to edit them in one Recipe without changing the
   // settings for any other Recipe.  TODO: We should change this so we can retire copyIfNeeded.
   //
   // We also need to be careful here as one or more of these may not be set to a valid value.
   //
   if (other.m_styleId        > 0) { auto item = copyIfNeeded(*ObjectStoreWrapper::getById<Style       >(other.m_styleId       )); this->m_styleId        = item->key(); }
   if (other.m_equipmentId    > 0) { auto item = copyIfNeeded(*ObjectStoreWrapper::getById<Equipment   >(other.m_equipmentId   )); this->m_equipmentId    = item->key(); }
   if (other.m_mashId         > 0) { auto item = copyIfNeeded(*ObjectStoreWrapper::getById<Mash        >(other.m_mashId        )); this->m_mashId         = item->key(); }
   if (other.m_boilId         > 0) { auto item = copyIfNeeded(*ObjectStoreWrapper::getById<Boil        >(other.m_boilId        )); this->m_boilId         = item->key(); }
   if (other.m_fermentationId > 0) { auto item = copyIfNeeded(*ObjectStoreWrapper::getById<Fermentation>(other.m_fermentationId)); this->m_fermentationId = item->key(); }

   this->pimpl->connectSignals();

   this->recalcAll();

   // This turns on writing to the DB and sending signals when things change.  However, we do this _after_ calling
   // recalcAll() as the newly constructed object will not yet be stored in the DB and won't yet have anyone listening
   // to its signals.
   CONSTRUCTOR_END
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
      this->pimpl->mashFermentableIns();

      /*** salt the water ***/
      this->pimpl->saltWater(RecipeAdjustmentSalt::WhenToAdd::Mash);
      this->pimpl->saltWater(RecipeAdjustmentSalt::WhenToAdd::Sparge);

      /*** Prepare water additions ***/
      this->pimpl->mashWaterIns();

      timeRemaining = mash()->totalTime();

      /*** Generate the mash instructions ***/
      preinstructions = this->pimpl->mashInstructions(timeRemaining, totalWaterAdded_l, size);

      /*** Hops mash additions ***/
      preinstructions += this->pimpl->hopSteps(RecipeAddition::Stage::Mash);

      /*** Misc mash additions ***/
      preinstructions += this->pimpl->miscSteps(RecipeAdditionMisc::Use::Mash);

      /*** Add the preinstructions into the instructions ***/
      this->pimpl->addPreinstructions(preinstructions);

   } // END mash instructions.

   // First wort hopping
   this->pimpl->firstWortHopsIns();

   // Need to top up the kettle before boil?
   this->pimpl->topOffIns();

   // Boil instructions
   preinstructions.clear();

   // Find boil time.
   if (equipment() != nullptr) {
      timeRemaining = equipment()->boilTime_min().value_or(Equipment::default_boilTime_mins);
   } else {
      timeRemaining =
         Measurement::qStringToSI(QInputDialog::getText(nullptr,
                                                        tr("Boil time"),
                                                        tr("You did not configure an equipment (which you really should), so tell me the boil time.")),
                                  Measurement::PhysicalQuantity::Time).quantity;
   }

   QString str = tr("Bring the wort to a boil and hold for %1.").arg(
      Measurement::displayAmount(Measurement::Amount{timeRemaining, Measurement::Units::minutes})
   );

   auto startBoilIns = std::make_shared<Instruction>();
   startBoilIns->setName(tr("Start boil"));
   startBoilIns->setInterval(timeRemaining);
   startBoilIns->setDirections(str);
   this->add(startBoilIns);

   /*** Get fermentables unless we haven't added yet ***/
   if (this->pimpl->hasBoilFermentable()) {
      preinstructions.push_back(this->pimpl->boilFermentablesPre(timeRemaining));
   }

   // add the intructions for including Extracts to wort
   if (this->pimpl->hasBoilExtract()) {
      preinstructions.push_back(this->pimpl->addExtracts(timeRemaining - 1));
   }

   /*** Boiled hops ***/
   preinstructions += this->pimpl->hopSteps(RecipeAddition::Stage::Boil);

   /*** Boiled miscs ***/
   preinstructions += this->pimpl->miscSteps(RecipeAdditionMisc::Use::Boil);

   // END boil instructions.

   // Add instructions in descending mash time order.
   this->pimpl->addPreinstructions(preinstructions);

   // FLAMEOUT
   auto flameoutIns = std::make_shared<Instruction>();
   flameoutIns->setName(tr("Flameout"));
   flameoutIns->setDirections(tr("Stop boiling the wort."));
   this->add(flameoutIns);

   // TODO: These get included in RecipeAddition::Stage::Boil above.  But we're going to want to rework this anyway to
   //       order by stage, step, time.
   // Steeped aroma hops
   // preinstructions = this->pimpl->hopSteps(Hop::Use::Aroma);
   this->pimpl->addPreinstructions(preinstructions);

   // Fermentation instructions
   preinstructions.clear();

   /*** Fermentables added after boil ***/
   this->pimpl->postboilFermentablesIns();

   /*** post boil ***/
   this->pimpl->postboilIns();

   /*** Primary yeast ***/
   str = tr("Cool wort and pitch ");
   for (auto yeastAddition : this->yeastAdditions()) {
      if (1 == yeastAddition->step()) {
         auto yeast = yeastAddition->yeast();
         str += tr("%1 %2 yeast, ").arg(yeast->name()).arg(Yeast::typeDisplayNames[yeast->type()]);
      }
   }
   str += tr("to the primary.");

   auto pitchIns = std::make_shared<Instruction>();
   pitchIns->setName(tr("Pitch yeast"));
   pitchIns->setDirections(str);
   this->add(pitchIns);
   /*** End primary yeast ***/

   /*** Primary misc ***/
   this->pimpl->addPreinstructions(this->pimpl->miscSteps(RecipeAdditionMisc::Use::Primary));

   str = tr("Let ferment until FG is %1.").arg(
      Measurement::displayAmount(Measurement::Amount{fg(), Measurement::Units::specificGravity}, 3)
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
   this->pimpl->addPreinstructions(this->pimpl->miscSteps(RecipeAdditionMisc::Use::Secondary));

   /*** Dry hopping ***/
   this->pimpl->addPreinstructions(this->pimpl->hopSteps(RecipeAddition::Stage::Fermentation));

   // END fermentation instructions. Let everybody know that now is the time
   // to update instructions
   emit changed(metaProperty(*PropertyNames::Recipe::instructions), this->instructions().size());

   return;
}

QString Recipe::nextAddToBoil(double & time) {
   double max = 0;
   bool foundSomething = false;
   QString ret;

   // Search hop additions
   for (auto hopAddition : this->hopAdditions()) {
      if (hopAddition->stage() != RecipeAddition::Stage::Boil) {
         continue;
      }
      if (!hopAddition->addAtTime_mins()) {
         continue;
      }
      double const addAtTime_mins = *hopAddition->addAtTime_mins();
      if (addAtTime_mins < time && addAtTime_mins > max) {
         ret = tr("Add %1 %2 to boil at %3.")
               .arg(Measurement::displayAmount(hopAddition->amount()))
               .arg(hopAddition->hop()->name())
               .arg(Measurement::displayAmount(Measurement::Amount{addAtTime_mins, Measurement::Units::minutes}));

         max = addAtTime_mins;
         foundSomething = true;
      }
   }

   // Search misc additions
   for (auto miscAddition : this->miscAdditions()) {
      if (miscAddition->stage() != RecipeAddition::Stage::Boil) {
         continue;
      }
      if (!miscAddition->addAtTime_mins()) {
         continue;
      }
      double const addAtTime_mins = *miscAddition->addAtTime_mins();
      if (addAtTime_mins < time && addAtTime_mins > max) {
         ret = tr("Add %1 %2 to boil at %3.");
         ret = ret.arg(Measurement::displayAmount(miscAddition->amount()));
         ret = ret.arg(miscAddition->misc()->name());
         ret = ret.arg(Measurement::displayAmount(Measurement::Amount{addAtTime_mins, Measurement::Units::minutes}));
         max = addAtTime_mins;
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
   this->propagatePropertyChange(Recipe::propertyNameFor<NE>());

   this->recalcIfNeeded(ne->metaObject()->className());
   return ne;
}

//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// means, amongst other things, that we can reference the pimpl.)
//
template std::shared_ptr<Instruction> Recipe::add(std::shared_ptr<Instruction> var);
template<> std::shared_ptr<Fermentable> Recipe::add(std::shared_ptr<Fermentable> var) = delete;
template<> std::shared_ptr<Hop        > Recipe::add(std::shared_ptr<Hop        > var) = delete;
template<> std::shared_ptr<Misc       > Recipe::add(std::shared_ptr<Misc       > var) = delete;
template<> std::shared_ptr<Yeast      > Recipe::add(std::shared_ptr<Yeast      > var) = delete;
template<> std::shared_ptr<Salt       > Recipe::add(std::shared_ptr<Salt       > var) = delete;
template<> std::shared_ptr<Water      > Recipe::add(std::shared_ptr<Water      > var) = delete;

template<class RA> std::shared_ptr<RA> Recipe::addAddition(std::shared_ptr<RA> addition) {
   // It's a coding error if we've ended up with a null shared_ptr
   Q_ASSERT(addition);

   // Tell the addition that it belongs to this recipe
   addition->setRecipeId(this->key());

   // Recipe additions are owned by the Recipe, so, if the object being added is not already in the ObjectStore, we need
   // to add it.
   if (addition->key() <= 0) {
      // With shared pointer parameter, ObjectStoreWrapper::insert returns what we passed it (ie our shared pointer
      // remains valid after the call).
      qDebug() <<
         Q_FUNC_INFO << "Inserting" << addition->metaObject()->className() << "for" <<
         addition->ingredient()->metaObject()->className() << "#" << addition->ingredient()->key() << "in object store";
      ObjectStoreWrapper::insert(addition);
   }

   //
   // Doing this connect here means that a signal will be sent to acceptChangeToContainedObject() by the call to
   // notifyPropertyChange() below.
   //
   connect(addition.get(), &NamedEntity::changed, this, &Recipe::acceptChangeToContainedObject);

   //
   // We don't want to call this->propagatePropertyChange here because the RecipeAddition is not stored either in the
   // Recipe database table or or one of the Recipe junction tables (see comments in database/ObjectStoreTyped.cpp).
   //
   // Instead, we merely want to emit the signals to tell anyone listening that the property was updated
   //
   this->notifyPropertyChange(Recipe::propertyNameFor<RA>());

   this->recalcIfNeeded(addition->ingredient()->metaObject()->className());
   return addition;
}
template std::shared_ptr<RecipeAdditionFermentable> Recipe::addAddition(std::shared_ptr<RecipeAdditionFermentable> addition);
template std::shared_ptr<RecipeAdditionHop        > Recipe::addAddition(std::shared_ptr<RecipeAdditionHop        > addition);
template std::shared_ptr<RecipeAdditionMisc       > Recipe::addAddition(std::shared_ptr<RecipeAdditionMisc       > addition);
template std::shared_ptr<RecipeAdditionYeast      > Recipe::addAddition(std::shared_ptr<RecipeAdditionYeast      > addition);
template std::shared_ptr<RecipeAdjustmentSalt     > Recipe::addAddition(std::shared_ptr<RecipeAdjustmentSalt     > addition);
template std::shared_ptr<RecipeUseOfWater         > Recipe::addAddition(std::shared_ptr<RecipeUseOfWater         > addition);

template<class NE> bool Recipe::uses(NE const & val) const {
   int idToLookFor = val.key();
   if (idToLookFor <= 0) {
      //
      // We shouldn't be trying to look for something that hasn't even been stored (and therefore does not yet have an
      // ID).
      //
      qCritical() <<
         Q_FUNC_INFO << "Trying to search for use of" << val.metaObject()->className() << "that is not stored!";
      return false;
   }

   auto match = std::find_if(this->pimpl->accessIds<NE>().cbegin(),
                             this->pimpl->accessIds<NE>().cend(),
   [idToLookFor](int id) {
      return idToLookFor == id;
   });

   return match != this->pimpl->accessIds<NE>().cend();
}
template bool Recipe::uses(Instruction  const & val) const;
template<> bool Recipe::uses(Fermentable  const & val) const = delete;
template<> bool Recipe::uses(Misc         const & val) const = delete;
template<> bool Recipe::uses(Salt         const & val) const = delete;
template<> bool Recipe::uses(Water        const & val) const = delete;
template<> bool Recipe::uses(Yeast        const & val) const = delete;
template<> bool Recipe::uses<Equipment   > (Equipment    const & val) const { return val.key() == this->m_equipmentId   ; }
template<> bool Recipe::uses<Style       > (Style        const & val) const { return val.key() == this->m_styleId       ; }
template<> bool Recipe::uses<Mash        > (Mash         const & val) const { return val.key() == this->m_mashId        ; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
template<> bool Recipe::uses<Boil        > (Boil         const & val) const { return val.key() == this->m_boilId        ; }
template<> bool Recipe::uses<Fermentation> (Fermentation const & val) const { return val.key() == this->m_fermentationId; }
template<> bool Recipe::uses<RecipeAdditionFermentable>(RecipeAdditionFermentable const & val) const { return val.recipeId() == this->key(); }
template<> bool Recipe::uses<RecipeAdditionHop        >(RecipeAdditionHop         const & val) const { return val.recipeId() == this->key(); }
template<> bool Recipe::uses<RecipeAdditionMisc       >(RecipeAdditionMisc        const & val) const { return val.recipeId() == this->key(); }
template<> bool Recipe::uses<RecipeAdditionYeast      >(RecipeAdditionYeast       const & val) const { return val.recipeId() == this->key(); }
template<> bool Recipe::uses<RecipeAdjustmentSalt     >(RecipeAdjustmentSalt      const & val) const { return val.recipeId() == this->key(); }
template<> bool Recipe::uses<RecipeUseOfWater         >(RecipeUseOfWater          const & val) const { return val.recipeId() == this->key(); }

std::shared_ptr<Instruction> Recipe::remove(std::shared_ptr<Instruction> var) {
   // It's a coding error to supply a null shared pointer
   Q_ASSERT(var);

   int idToRemove = var->key();
   if (!this->pimpl->accessIds<Instruction>().removeOne(idToRemove)) {
      // It's a coding error if we try to remove something from the Recipe that wasn't in it in the first place!
      qCritical() <<
         Q_FUNC_INFO << "Tried to remove" << var->metaObject()->className() << "with ID" << idToRemove <<
         "but couldn't find it in Recipe #" << this->key();
      Q_ASSERT(false);
   } else {
      this->propagatePropertyChange(Recipe::propertyNameFor<Instruction>());
      // NB: Don't need to call this->pimpl->recalcIBU() for removing an Instruction!
   }

   //
   // Because Instruction objects are owned by their Recipe we usually want to delete them object from the ObjectStore
   // at this point.  But, because we're a bit paranoid, we'll check first that the object we're removing has a parent
   // (ie really is an "instance of use of") and is not used in any other Recipes.  THIS EXTRA CHECK IS ALMOST CERTAINLY
   // UNNECESSARY.
   //
   if (isUnusedInstanceOfUseOf(*var)) {
      qDebug() <<
         Q_FUNC_INFO << "Deleting" << var->metaObject()->className() << "#" << var->key() <<
         "as it is \"instance of use of\" that is no longer needed";
      ObjectStoreWrapper::hardDelete<Instruction>(var->key());
   }
   // The caller now owns the removed object unless and until they pass it in to Recipe::add() (typically to undo the
   // remove).
   return var;
}
//template std::shared_ptr<Instruction> Recipe::remove(std::shared_ptr<Instruction> var);

template<class RA> std::shared_ptr<RA> Recipe::removeAddition(std::shared_ptr<RA> addition) {
   // It's a coding error to supply a null shared pointer
   Q_ASSERT(addition);

   // It's a coding error if we try to remove something from the Recipe that wasn't in it in the first place!
   Q_ASSERT(addition->recipeId() == this->key());

   addition->setRecipeId(-1);

   disconnect(addition.get(), &NamedEntity::changed, this, &Recipe::acceptChangeToContainedObject);
   //
   // For the same reason as in addAddition(), we don't want to call this->propagatePropertyChange here
   //
   this->notifyPropertyChange(Recipe::propertyNameFor<RA>());

   this->recalcIfNeeded(addition->ingredient()->metaObject()->className());

   //
   // Because RecipeAdditionHop etc objects are owned by their Recipe, we need to delete the object from the ObjectStore
   // at this point.
   //
   qDebug() << Q_FUNC_INFO << "Deleting" << addition->metaObject()->className() << "#" << addition->key();
   ObjectStoreWrapper::hardDelete<RA>(addition->key());

   // The caller now owns the removed object unless and until they pass it in to Recipe::add() (typically to undo the
   // remove).
   return addition;
}
template std::shared_ptr<RecipeAdditionFermentable> Recipe::removeAddition(std::shared_ptr<RecipeAdditionFermentable> addition);
template std::shared_ptr<RecipeAdditionHop        > Recipe::removeAddition(std::shared_ptr<RecipeAdditionHop        > addition);
template std::shared_ptr<RecipeAdditionMisc       > Recipe::removeAddition(std::shared_ptr<RecipeAdditionMisc       > addition);
template std::shared_ptr<RecipeAdditionYeast      > Recipe::removeAddition(std::shared_ptr<RecipeAdditionYeast      > addition);
template std::shared_ptr<RecipeAdjustmentSalt     > Recipe::removeAddition(std::shared_ptr<RecipeAdjustmentSalt     > addition);
template std::shared_ptr<RecipeUseOfWater         > Recipe::removeAddition(std::shared_ptr<RecipeUseOfWater         > addition);

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
   this->propagatePropertyChange(Recipe::propertyNameFor<Instruction>());
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
   this->propagatePropertyChange(Recipe::propertyNameFor<Instruction>());
   return;
}

// .:TBD:. We need to think about when/how we're going to detect changes to the Boil object referred to by this->m_boilId...

void Recipe::setMash        (std::shared_ptr<Mash        > val) { this->pimpl->set<Mash        >(val, this->m_mashId        ); return; }
void Recipe::setBoil        (std::shared_ptr<Boil        > val) { this->pimpl->set<Boil        >(val, this->m_boilId        ); return; }
void Recipe::setFermentation(std::shared_ptr<Fermentation> val) { this->pimpl->set<Fermentation>(val, this->m_fermentationId); return; }
void Recipe::setStyle       (std::shared_ptr<Style       > val) { this->pimpl->set<Style       >(val, this->m_styleId       ); return; }
void Recipe::setEquipment   (std::shared_ptr<Equipment   > val) { this->pimpl->set<Equipment   >(val, this->m_equipmentId   ); return; }

template<typename RA> void Recipe::setAdditions(QList<std::shared_ptr<RA>> val) {
//   qDebug() << Q_FUNC_INFO << "Adding" << val.size() << RA::staticMetaObject.className() << "entries";
   for (auto ii : val) {
//      qDebug() << Q_FUNC_INFO << "Setting Recipe ID #" << this->key() << "on" << RA::staticMetaObject.className() << "#" << ii->key();
      ii->setRecipeId(this->key());
   }
   return;
}

void Recipe::setFermentableAdditions(QList<std::shared_ptr<RecipeAdditionFermentable>> val) { this->setAdditions(val); return; }
void Recipe::setHopAdditions        (QList<std::shared_ptr<RecipeAdditionHop        >> val) { this->setAdditions(val); return; }
void Recipe::setMiscAdditions       (QList<std::shared_ptr<RecipeAdditionMisc       >> val) { this->setAdditions(val); return; }
void Recipe::setYeastAdditions      (QList<std::shared_ptr<RecipeAdditionYeast      >> val) { this->setAdditions(val); return; }
void Recipe::setSaltAdjustments     (QList<std::shared_ptr<RecipeAdjustmentSalt     >> val) { this->setAdditions(val); return; }
void Recipe::setWaterUses           (QList<std::shared_ptr<RecipeUseOfWater         >> val) { this->setAdditions(val); return; }

// Note that, because these setBlahId member functions are supposed only to be used by by ObjectStore, and are not
// intended for more general use, they do not call setAndNofify
void Recipe::setStyleId       (int const id) { this->m_styleId        = id; return; }
void Recipe::setEquipmentId   (int const id) { this->m_equipmentId    = id; return; }
void Recipe::setMashId        (int const id) { this->m_mashId         = id; return; }
void Recipe::setBoilId        (int const id) { this->m_boilId         = id; return; }
void Recipe::setFermentationId(int const id) { this->m_fermentationId = id; return; }

void Recipe::setInstructionIds(QVector<int> ids) {    this->pimpl->instructionIds = ids; return; }

//==============================="SET" METHODS=================================
void Recipe::setType(Recipe::Type const val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::type, this->m_type, val);
   return;
}

void Recipe::setBrewer(QString const & var) {
   SET_AND_NOTIFY(PropertyNames::Recipe::brewer, this->m_brewer, var);
   return;
}

void Recipe::setBatchSize_l(double var) {
   SET_AND_NOTIFY(PropertyNames::Recipe::batchSize_l,
                      this->m_batchSize_l,
                      this->enforceMin(var, "batch size"));

   // NOTE: this is bad, but we have to call recalcAll(), because the estimated
   // boil/batch volumes depend on the target volumes when there are no mash
   // steps to actually provide an estimate for the volumes.
   recalcAll();
}

void Recipe::setEfficiency_pct(double val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::efficiency_pct,
                      this->m_efficiency_pct,
                      this->enforceMinAndMax(val, "efficiency", 0.0, 100.0, 70.0));

   // If you change the efficency, you really should recalc. And I'm afraid it
   // means recalc all, since og and fg will change, which means your ratios
   // change
   recalcAll();
}

void Recipe::setAsstBrewer(const QString & val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::asstBrewer, this->m_asstBrewer, val);
   return;
}

void Recipe::setNotes(const QString & val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::notes, this->m_notes, val);
   return;
}

void Recipe::setTasteNotes(const QString & val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::tasteNotes, this->m_tasteNotes, val);
   return;
}

void Recipe::setTasteRating(double val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::tasteRating, this->m_tasteRating, this->enforceMinAndMax(val, "taste rating", 0.0, 50.0, 0.0));
   return;
}

void Recipe::setOg(double val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::og, this->m_og, this->enforceMin(val, "og", 0.0, 1.0));
   return;
}

void Recipe::setFg(double val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::fg, this->m_fg, this->enforceMin(val, "fg", 0.0, 1.0));
   return;
}

void Recipe::setAge_days(std::optional<double> val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::age_days, this->m_age_days, this->enforceMin(val, "age_days"));
   return;
}

void Recipe::setAgeTemp_c(std::optional<double> val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::ageTemp_c, this->m_ageTemp_c, val);
   return;
}

void Recipe::setDate(std::optional<QDate> const val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::date, this->m_date, val);
   return;
}

void Recipe::setCarbonation_vols(std::optional<double> const val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::carbonation_vols, this->m_carbonation_vols, this->enforceMin(val, "carb"));
   return;
}

void Recipe::setForcedCarbonation(bool val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::forcedCarbonation, this->m_forcedCarbonation, val);
   return;
}

void Recipe::setPrimingSugarName(const QString & val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::primingSugarName, this->m_primingSugarName, val);
   return;
}

void Recipe::setCarbonationTemp_c(double val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::carbonationTemp_c, this->m_carbonationTemp_c, val);
   return;
}

void Recipe::setPrimingSugarEquiv(double val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::primingSugarEquiv, this->m_primingSugarEquiv, this->enforceMin(val, "priming sugar equiv", 0.0, 1.0));
   return;
}

void Recipe::setKegPrimingFactor(double val) {
   SET_AND_NOTIFY(PropertyNames::Recipe::kegPrimingFactor, this->m_kegPrimingFactor, this->enforceMin(val, "keg priming factor", 0.0, 1.0));
   return;
}

void Recipe::setBeerAcidity_pH         (std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::Recipe::beerAcidity_pH         , this->m_beerAcidity_pH         , val); return; }
void Recipe::setApparentAttenuation_pct(std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::Recipe::apparentAttenuation_pct, this->m_apparentAttenuation_pct, val); return; }

void Recipe::setLocked(bool const isLocked) {
   // Locking a Recipe doesn't count as changing it for the purposes of versioning or the UI, so no call to setAndNotify
   // here.
   if (this->newValueMatchesExisting(PropertyNames::Recipe::locked, this->m_locked, isLocked)) {
      return;
   }
   this->m_locked = isLocked;
   this->propagatePropertyChange(PropertyNames::Recipe::locked);
   return;
}

void Recipe::setCalcsEnabled(bool const val) {
   this->m_calcsEnabled = val;
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

double        Recipe::og              () { return this->pimpl->getCalculated(this->m_og); }
double        Recipe::fg              () { return this->pimpl->getCalculated(this->m_fg); }
double        Recipe::color_srm       () { return this->pimpl->getCalculated(this->pimpl->m_color_srm       ); }
double        Recipe::ABV_pct         () { return this->pimpl->getCalculated(this->pimpl->m_ABV_pct         ); }
double        Recipe::IBU             () { return this->pimpl->getCalculated(this->pimpl->m_IBU             ); }
QList<double> Recipe::IBUs            () { return this->pimpl->getCalculated(this->pimpl->m_ibus            ); }
double        Recipe::boilGrav        () { return this->pimpl->getCalculated(this->pimpl->m_boilGrav        ); }
double        Recipe::caloriesPerLiter() { return this->pimpl->getCalculated(this->pimpl->m_caloriesPerLiter); }
double        Recipe::caloriesPer33cl  () { return this->caloriesPerLiter() * 0.33          ; }
double        Recipe::caloriesPerUs12oz() {
   static double const us12ozInLiters = Measurement::Units::us_fluidOunces.toCanonical(12.0).quantity;
   return this->caloriesPerLiter() * us12ozInLiters;
}
double        Recipe::caloriesPerUsPint() {
   static double const usPintInLiters = Measurement::Units::us_fluidOunces.toCanonical(16.0).quantity;
   return this->caloriesPerLiter() * usPintInLiters;
}
double        Recipe::wortFromMash_l  () { return this->pimpl->getCalculated(this->pimpl->m_wortFromMash_l  );}
double        Recipe::boilVolume_l    () { return this->pimpl->getCalculated(this->pimpl->m_boilVolume_l    );}
double        Recipe::postBoilVolume_l() { return this->pimpl->getCalculated(this->pimpl->m_postBoilVolume_l);}
double        Recipe::finalVolume_l   () { return this->pimpl->getCalculated(this->pimpl->m_finalVolume_l   );}
QColor        Recipe::SRMColor        () { return this->pimpl->getCalculated(this->pimpl->m_SRMColor        );}
double        Recipe::grainsInMash_kg () { return this->pimpl->getCalculated(this->pimpl->m_grainsInMash_kg );}
double        Recipe::grains_kg       () { return this->pimpl->getCalculated(this->pimpl->m_grains_kg       );}

double Recipe::points() {
   return (this->og() - 1.0) * 1e3;
}

//=========================Relational Getters=============================

template<> std::shared_ptr<Mash        > Recipe::get<Mash        >() const { return this->pimpl->get<Mash        >(this->m_mashId        ); }
template<> std::shared_ptr<Boil        > Recipe::get<Boil        >() const { return this->pimpl->get<Boil        >(this->m_boilId        ); }
template<> std::shared_ptr<Fermentation> Recipe::get<Fermentation>() const { return this->pimpl->get<Fermentation>(this->m_fermentationId); }
template<> std::shared_ptr<Style       > Recipe::get<Style       >() const { return this->pimpl->get<Style       >(this->m_styleId       ); }
template<> std::shared_ptr<Equipment   > Recipe::get<Equipment   >() const { return this->pimpl->get<Equipment   >(this->m_equipmentId   ); }
template<> std::shared_ptr<Water       > Recipe::get<Water       >() const {
   // Water is a bit different as there can be more than one
   auto waterUses = this->waterUses();
   if (waterUses.size() > 0) {
      return ObjectStoreWrapper::getSharedFromRaw(waterUses.at(0)->water());
   }
   return nullptr;
}

std::shared_ptr<Mash        > Recipe::mash        () const { return this->get<Mash        >(); }
std::shared_ptr<Boil        > Recipe::boil        () const { return this->get<Boil        >(); }
std::shared_ptr<Fermentation> Recipe::fermentation() const { return this->get<Fermentation>(); }
std::shared_ptr<Style       > Recipe::style       () const { return this->get<Style       >(); }
std::shared_ptr<Equipment   > Recipe::equipment   () const { return this->get<Equipment   >(); }

std::shared_ptr<Boil        > Recipe::nonOptBoil        () { return this->pimpl->nonOptionalItem<Boil        >(this->m_boilId        ); }
std::shared_ptr<Fermentation> Recipe::nonOptFermentation() { return this->pimpl->nonOptionalItem<Fermentation>(this->m_fermentationId); }

int Recipe::getStyleId       () const { return this->m_styleId       ; }
int Recipe::getEquipmentId   () const { return this->m_equipmentId   ; }
int Recipe::getMashId        () const { return this->m_mashId        ; }
int Recipe::getBoilId        () const { return this->m_boilId        ; }
int Recipe::getFermentationId() const { return this->m_fermentationId; }

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
         return bn->recipeId() == recipeId;
      }
   );
}

template<typename NE> QList< std::shared_ptr<NE> > Recipe::getAll() const {
   return this->pimpl->allMy<NE>();
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// means, amongst other things, that we can reference the pimpl.)
//
template QList< std::shared_ptr<RecipeAdditionFermentable> > Recipe::getAll<RecipeAdditionFermentable>() const;
template QList< std::shared_ptr<RecipeAdditionHop        > > Recipe::getAll<RecipeAdditionHop        >() const;
template QList< std::shared_ptr<RecipeAdditionMisc       > > Recipe::getAll<RecipeAdditionMisc       >() const;
template QList< std::shared_ptr<RecipeAdditionYeast      > > Recipe::getAll<RecipeAdditionYeast      >() const;
template QList< std::shared_ptr<RecipeAdjustmentSalt     > > Recipe::getAll<RecipeAdjustmentSalt     >() const;
template QList< std::shared_ptr<RecipeUseOfWater         > > Recipe::getAll<RecipeUseOfWater         >() const;

QList<std::shared_ptr<RecipeAdditionFermentable>> Recipe::fermentableAdditions() const { return this->pimpl->allMy<RecipeAdditionFermentable>(); }
QList<std::shared_ptr<RecipeAdditionHop        >> Recipe::        hopAdditions() const { return this->pimpl->allMy<RecipeAdditionHop        >(); }
QList<std::shared_ptr<RecipeAdditionMisc       >> Recipe::       miscAdditions() const { return this->pimpl->allMy<RecipeAdditionMisc       >(); }
QList<std::shared_ptr<RecipeAdditionYeast      >> Recipe::      yeastAdditions() const { return this->pimpl->allMy<RecipeAdditionYeast      >(); }
QList<std::shared_ptr<RecipeAdjustmentSalt     >> Recipe::     saltAdjustments() const { return this->pimpl->allMy<RecipeAdjustmentSalt     >(); }
QList<std::shared_ptr<RecipeUseOfWater         >> Recipe::           waterUses() const { return this->pimpl->allMy<RecipeUseOfWater         >(); }
QVector<int>         Recipe::fermentableAdditionIds() const { return this->pimpl->allMyIds<RecipeAdditionFermentable>(); }
QVector<int>         Recipe::        hopAdditionIds() const { return this->pimpl->allMyIds<RecipeAdditionHop        >(); }
QVector<int>         Recipe::       miscAdditionIds() const { return this->pimpl->allMyIds<RecipeAdditionMisc       >(); }
QVector<int>         Recipe::      yeastAdditionIds() const { return this->pimpl->allMyIds<RecipeAdditionYeast      >(); }
QVector<int>         Recipe::     saltAdjustmentIds() const { return this->pimpl->allMyIds<RecipeAdjustmentSalt     >(); }
QVector<int>         Recipe::           waterUseIds() const { return this->pimpl->allMyIds<RecipeUseOfWater         >(); }

int Recipe::getAncestorId() const { return this->m_ancestor_id; }

//==============================Getters===================================
Recipe::Type Recipe::type()          const { return m_type;               }
QString Recipe::brewer()             const { return m_brewer;             }
QString Recipe::asstBrewer()         const { return m_asstBrewer;         }
QString Recipe::notes()              const { return m_notes;              }
QString Recipe::tasteNotes()         const { return m_tasteNotes;         }
QString Recipe::primingSugarName()   const { return m_primingSugarName;   }
bool    Recipe::forcedCarbonation()  const { return m_forcedCarbonation;  }
double  Recipe::batchSize_l()        const { return m_batchSize_l;        }
double  Recipe::efficiency_pct()     const { return m_efficiency_pct;     }
double  Recipe::tasteRating()        const { return m_tasteRating;        }
std::optional<double>  Recipe::age_days()           const { return m_age_days;                }
std::optional<double>  Recipe::ageTemp_c()          const { return m_ageTemp_c;          }
double  Recipe::carbonationTemp_c()  const { return m_carbonationTemp_c;  }
double  Recipe::primingSugarEquiv()  const { return m_primingSugarEquiv;  }
double  Recipe::kegPrimingFactor()   const { return m_kegPrimingFactor;   }
std::optional<QDate>   Recipe::date()               const { return m_date;               }
std::optional<double>  Recipe::carbonation_vols()   const { return m_carbonation_vols;   }
bool    Recipe::locked()             const { return m_locked      ;        }
bool    Recipe::calcsEnabled()       const { return m_calcsEnabled;        }

// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
std::optional<double> Recipe::beerAcidity_pH         () const { return m_beerAcidity_pH         ; }
std::optional<double> Recipe::apparentAttenuation_pct() const { return m_apparentAttenuation_pct; }

//==============================Recalculators==================================

void Recipe::recalcIfNeeded(QString classNameOfWhatWasAddedOrChanged) {
   qDebug() << Q_FUNC_INFO << classNameOfWhatWasAddedOrChanged;
   // We could just compare with "Hop", "Equipment", etc but there's then no compile-time checking of typos.  Using
   // ::staticMetaObject.className() is a bit more clunky but it's safer.

   if (classNameOfWhatWasAddedOrChanged ==               Hop::staticMetaObject.className() ||
       classNameOfWhatWasAddedOrChanged == RecipeAdditionHop::staticMetaObject.className()) {
      this->pimpl->recalcIBU();
      return;
   }

   if (classNameOfWhatWasAddedOrChanged ==                 Equipment::staticMetaObject.className() ||
       classNameOfWhatWasAddedOrChanged ==               Fermentable::staticMetaObject.className() ||
       classNameOfWhatWasAddedOrChanged == RecipeAdditionFermentable::staticMetaObject.className() ||
       classNameOfWhatWasAddedOrChanged ==                      Mash::staticMetaObject.className()) {
      this->recalcAll();
      return;
   }

   if (classNameOfWhatWasAddedOrChanged ==               Yeast::staticMetaObject.className() ||
       classNameOfWhatWasAddedOrChanged == RecipeAdditionYeast::staticMetaObject.className()) {
      this->pimpl->recalcOgFg();
      this->pimpl->recalcABV_pct();
      return;
   }

   return;
}

void Recipe::recalcAll() {
   if (!this->m_calcsEnabled) {
      qDebug() << Q_FUNC_INFO << "Calculations disabled";
      return;
   }

   // WARNING
   // Infinite recursion possible, since these methods will emit changed(),
   // causing other objects to call finalVolume_l() for example, which may
   // cause another call to recalcAll() and so on.
   //
   // GSG: Now only emit when m_uninitializedCalcs is true, which helps some.

   // Someone has already called this function back in the call stack, so return to avoid recursion.
   if (!this->m_recalcMutex.tryLock()) {
      return;
   }

   // Times are in seconds, and are cumulative.
   this->pimpl->recalcGrains(); // 0.03
   this->pimpl->recalcVolumeEstimates(); // 0.06
   this->pimpl->recalcColor_srm(); // 0.08
   this->pimpl->recalcSRMColor(); // 0.08
   this->pimpl->recalcOgFg(); // 0.11
   this->pimpl->recalcABV_pct(); // 0.12
   this->pimpl->recalcBoilGrav(); // 0.14
   this->pimpl->recalcIBU(); // 0.15
   this->pimpl->recalcCalories();

   this->m_uninitializedCalcs = false;

   this->m_recalcMutex.unlock();
   return;
}

// Other efficiency calculations need access to the maximum theoretical sugars
// available. The only way I can see of doing that which doesn't suck is to
// split that calculation out of recalcOgFg();
Recipe::Sugars Recipe::calcTotalPoints() {
   Recipe::Sugars ret;

   for (auto const & fermentableAddition : this->fermentableAdditions()) {
      auto const & fermentable = fermentableAddition->fermentable();
      qDebug() <<
         "calcTotalPoints Rec" << this->key() << "(" << this->name() << ") "
         "Ferm Add" << fermentable->key() << "(" << fermentable->name() << ") equivSucrose_kg" <<
         fermentableAddition->equivSucrose_kg() << ", isSugar?" << fermentable->isSugar() << ", isExtract?" <<
         fermentable->isExtract() << ", addAfterBoil?" <<
         fermentableAddition->addAfterBoil() << ", isFermentableSugar?" << isFermentableSugar(fermentable);

      // If we have some sort of non-grain, we have to ignore efficiency.
      if (fermentable->isSugar() || fermentable->isExtract()) {
         ret.sugar_kg_ignoreEfficiency += fermentableAddition->equivSucrose_kg();

         if (fermentableAddition->addAfterBoil()) {
            ret.lateAddition_kg_ignoreEff += fermentableAddition->equivSucrose_kg();
         }

         if (!isFermentableSugar(fermentable)) {
            ret.nonFermentableSugars_kg += fermentableAddition->equivSucrose_kg();
         }
      } else {
         ret.sugar_kg += fermentableAddition->equivSucrose_kg();

         if (fermentableAddition->addAfterBoil()) {
            ret.lateAddition_kg += fermentableAddition->equivSucrose_kg();
         }
      }
   }

   return ret;
}



//====================================Helpers===========================================

double Recipe::ibuFromHopAddition(RecipeAdditionHop const & hopAddition) {
   auto equipment = this->equipment();
   double ibus = 0.0;
   double fwhAdjust = Localization::toDouble(
      PersistentSettings::value(PersistentSettings::Names::firstWortHopAdjustment, 1.1).toString(),
      Q_FUNC_INFO
   );
   double mashHopAdjust = Localization::toDouble(
      PersistentSettings::value(PersistentSettings::Names::mashHopAdjustment, 0).toString(),
      Q_FUNC_INFO
   );

   // It's a coding error to ask one recipe about another's hop additions!
   Q_ASSERT(hopAddition.recipeId() == this->key());

   double AArating = hopAddition.hop()->alpha_pct() / 100.0;
   // .:TBD:.  What to do if hopAddition is measured by volume?
   //
   // Per https://beersmith.com/blog/2016/08/31/using-hop-extracts-for-beer-brewing/, for CO2 Hop Extract, a first
   // approximation would be 1 gram hop = 1 ml of hop extract.
   //
   // The same page suggests that, for Isomerized Hop Extract,
   //    IBU = (extract_vol_ml * alpha_content_pct * 1000) / (volume_beer_liters)
   //
   if (!hopAddition.amountIsWeight()) {
      qCritical() << Q_FUNC_INFO << "Using Hop volume as weight - THIS IS PROBABLY WRONG!";
   }
   double grams = hopAddition.quantity() * 1000.0;
   double minutes = hopAddition.addAtTime_mins().value_or(0.0);
   // Assume 100% utilization until further notice
   double hopUtilization = 1.0;
   // Assume 60 min boil until further notice
   double boilTime_mins = 60.0;

   // NOTE: we used to carefully calculate the average boil gravity and use it in the
   // IBU calculations. However, due to John Palmer
   // (http://homebrew.stackexchange.com/questions/7343/does-wort-gravity-affect-hopAddition-utilization),
   // it seems more appropriate to just use the OG directly, since it is the total
   // amount of break material that truly affects the IBUs.

   if (equipment) {
      hopUtilization = equipment->hopUtilization_pct().value_or(Equipment::default_hopUtilization_pct) / 100.0;
      boilTime_mins = static_cast<int>(equipment->boilTime_min().value_or(Equipment::default_boilTime_mins));
   }

   auto boil = this->boil();
   if (boil) {
      boilTime_mins = boil->boilTime_mins();
   }

   IbuMethods::IbuCalculationParms parms = {
      .AArating              = AArating,
      .hops_grams            = grams,
      .postBoilVolume_liters = this->pimpl->m_finalVolumeNoLosses_l,
      .wortGravity_sg        = m_og,
      .boilTime_minutes      = boilTime_mins,  // Seems unlikely in reality that there would be fractions of a minute
      .coolTime_minutes          = boil->coolTime_mins(),
      .kettleInternalDiameter_cm = equipment->kettleInternalDiameter_cm(),
      .kettleOpeningDiameter_cm  = equipment->kettleOpeningDiameter_cm (),
   };
   if (hopAddition.isFirstWort()) {
      ibus = fwhAdjust * IbuMethods::getIbus(parms);
   } else if (hopAddition.stage() == RecipeAddition::Stage::Boil) {
      parms.boilTime_minutes = minutes;
      ibus = IbuMethods::getIbus(parms);
   } else if (hopAddition.stage() == RecipeAddition::Stage::Mash && mashHopAdjust > 0.0) {
      ibus = mashHopAdjust * IbuMethods::getIbus(parms);
   }

   // Adjust for hopAddition form. Tinseth's table was created from whole cone data,
   // and it seems other formulae are optimized that way as well. So, the
   // utilization is considered unadjusted for whole cones, and adjusted
   // up for plugs and pellets.
   //
   // - http://www.realbeer.com/hops/FAQ.html
   // - https://groups.google.com/forum/#!topic"brewtarget.h"lp/mv2qvWBC4sU
   auto const hopForm = hopAddition.hop()->form();
   if (hopForm) {
      switch (*hopForm) {
         case Hop::Form::Plug:
            hopUtilization *= 1.02;
            break;
         case Hop::Form::Pellet:
            hopUtilization *= 1.10;
            break;
         default:
            break;
      }
   }

   // Adjust for hopAddition utilization.
   ibus *= hopUtilization;

   return ibus;
}

QList<QString> Recipe::getReagents(QList<std::shared_ptr<RecipeAdditionFermentable>> fermentableAdditions) {
   QList<QString> reagents;
   bool firstTime = true;
   for (auto const & fermentableAddition : fermentableAdditions) {
      if (fermentableAddition->stage() == RecipeAddition::Stage::Mash) {
         // .:TBD:.  This isn't the most elegant or accurate way of handling commas.  If we're returning a list, we
         // should probably leave it to the caller to put commas in for display.
         QString format;
         if (firstTime) {
            format = "%1 %2";
            firstTime = false;
         } else {
            format = ", %1 %2";
         }
         reagents.append(
            format.arg(Measurement::displayAmount(fermentableAddition->amount()))
                  .arg(fermentableAddition->fermentable()->name())
         );
      }
   }
   return reagents;
}


QList<QString> Recipe::getReagents(QList<std::shared_ptr<RecipeAdditionHop>> hopAdditions, bool firstWort) {
   QList<QString> reagents;

   for (auto hopAddition : hopAdditions) {
      if (firstWort && (hopAddition->isFirstWort())) {
         QString tmp = QString("%1 %2,")
               .arg(Measurement::displayAmount(hopAddition->amount()))
               .arg(hopAddition->hop()->name());
         reagents.append(tmp);
      }
   }
   return reagents;
}

QList<QString> Recipe::getReagents(QList< std::shared_ptr<MashStep> > msteps) {
   QList<QString> reagents;

   for (int ii = 0; ii < msteps.size(); ++ii) {
      if (!msteps[ii]->isInfusion()) {
         continue;
      }

      QString tmp;
      if (ii + 1 < msteps.size()) {
         tmp = tr("%1 water to %2, ")
               .arg(Measurement::displayAmount(Measurement::Amount{msteps[ii]->amount_l(), Measurement::Units::liters}))
               .arg(Measurement::displayAmount(Measurement::Amount{msteps[ii]->infuseTemp_c().value_or(msteps[ii]->startTemp_c().value_or(0.0)), Measurement::Units::celsius}));
      } else {
         tmp = tr("%1 water to %2 ")
               .arg(Measurement::displayAmount(Measurement::Amount{msteps[ii]->amount_l(), Measurement::Units::liters}))
               .arg(Measurement::displayAmount(Measurement::Amount{msteps[ii]->infuseTemp_c().value_or(msteps[ii]->startTemp_c().value_or(0.0)), Measurement::Units::celsius}));
      }
      reagents.append(tmp);
   }
   return reagents;
}


//==========================Accept changes from ingredients====================

void Recipe::acceptChangeToContainedObject(QMetaProperty prop, QVariant val) {
   // This tells us which object sent us the signal
   QObject * signalSender = this->sender();
   if (signalSender != nullptr) {
      QString signalSenderClassName = signalSender->metaObject()->className();
      QString propName = prop.name();
      qDebug() <<
         Q_FUNC_INFO << "Signal received from " << signalSenderClassName << ": changed" << propName << "to" << val;;
      Equipment * equipment = qobject_cast<Equipment *>(signalSender);
      if (equipment) {
         qDebug() << Q_FUNC_INFO << "Equipment #" << equipment->key() << "(ours=" << this->m_equipmentId << ")";
         Q_ASSERT(equipment->key() == this->m_equipmentId);
         if (propName == *PropertyNames::Equipment::kettleBoilSize_l) {
            Q_ASSERT(val.canConvert<double>());
            qDebug() << Q_FUNC_INFO << "We" << (this->boil() ? "have" : "don't have") << "a boil";
            if (this->boil()) {
               this->boil()->setPreBoilSize_l(val.value<double>());
            }
         } else if (propName == PropertyNames::Equipment::boilTime_min) {
            Q_ASSERT(val.canConvert<double>());
            if (this->boil()) {
               this->boil()->setBoilTime_mins(val.value<double>());
            }
         }
      }
      this->recalcIfNeeded(signalSenderClassName);
   } else {
      qDebug() << Q_FUNC_INFO << "No sender";
   }
   return;
}

double Recipe::targetCollectedWortVol_l() {

   // Need to account for extract/sugar volume also.
   double postMashAdditionVolume_l = 0;

   for (auto const & fermentableAddition : this->fermentableAdditions()) {
      auto const & fermentable = fermentableAddition->fermentable();
      switch (fermentable->type()) {
         case Fermentable::Type::Extract:
            if (fermentableAddition->amountIsWeight()) {
               postMashAdditionVolume_l += fermentableAddition->amount().quantity / PhysicalConstants::liquidExtractDensity_kgL;
            } else {
               // .:TBD:. This is probably incorrect!
               postMashAdditionVolume_l += fermentableAddition->amount().quantity;
            }
            break;
         case Fermentable::Type::Sugar:
            if (fermentableAddition->amountIsWeight()) {
               postMashAdditionVolume_l += fermentableAddition->amount().quantity / PhysicalConstants::sucroseDensity_kgL;
            } else {
               // .:TBD:. This is probably incorrect!
               postMashAdditionVolume_l += fermentableAddition->amount().quantity;
            }
            break;
         case Fermentable::Type::Dry_Extract:
            if (fermentableAddition->amountIsWeight()) {
               postMashAdditionVolume_l += fermentableAddition->amount().quantity / PhysicalConstants::dryExtractDensity_kgL;
            } else {
               // .:TBD:. This is probably incorrect!
               postMashAdditionVolume_l += fermentableAddition->amount().quantity;
            }
            break;
         // .:TODO:. Need to handle other types of Fermentable here, even if it's just to add a NO-OP to show the
         // compiler we didn't forget about them.  For now the compiler warning will help us remember this to-do!
      }
   }

   double boilSize_liters = this->pimpl->boilSizeInLitersOr(0.0);
   qDebug() << Q_FUNC_INFO << "Boil size:" << boilSize_liters;

   if (this->equipment()) {
      return boilSize_liters - this->equipment()->getLauteringDeadspaceLoss_l()
                             - this->equipment()->topUpKettle_l().value_or(Equipment::default_topUpKettle_l)
                             - postMashAdditionVolume_l;
   } else {
      return boilSize_liters - postMashAdditionVolume_l;
   }
}

double Recipe::targetTotalMashVol_l() {

   double absorption_lKg;

   if (equipment()) {
      absorption_lKg = equipment()->mashTunGrainAbsorption_LKg().value_or(Equipment::default_mashTunGrainAbsorption_LKg);
   } else {
      absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;
   }

   return this->targetCollectedWortVol_l() + absorption_lKg * this->grainsInMash_kg();
}

void Recipe::hardDeleteOwnedEntities() {
   // It's the BrewNote that stores its Recipe ID, so all we need to do is delete our BrewNotes then the subsequent
   // database delete of this Recipe won't hit any foreign key problems.
   auto brewNotes = this->brewNotes();
   for (auto brewNote : brewNotes) {
      ObjectStoreWrapper::hardDelete<BrewNote>(*brewNote);
   }

   this->pimpl->hardDeleteAdditions<RecipeAdditionFermentable>();
   this->pimpl->hardDeleteAdditions<RecipeAdditionHop        >();
   this->pimpl->hardDeleteAdditions<RecipeAdditionMisc       >();
   this->pimpl->hardDeleteAdditions<RecipeAdditionYeast      >();
   this->pimpl->hardDeleteAdditions<RecipeAdjustmentSalt     >();
   this->pimpl->hardDeleteAdditions<RecipeUseOfWater         >();
   this->pimpl->hardDeleteAllMy<Instruction>();

   return;
}

void Recipe::hardDeleteOrphanedEntities() {
   //
   // Strictly a Recipe does not own its Mash, Boil or Fermentation.  However, if our Mash/Boil/Fermentation does not
   // have a name and is not used by any other Recipe, then we want to delete it, on the grounds that it's not one the
   // user intended to reuse across multiple Recipes.
   //
   // However, if we try to just delete the Mash/etc in Recipe::hardDeleteOwnedEntities(), we'd get a foreign key
   // constraint violation error from the DB as, at that point, the Mash ID is still referenced by this Recipe.
   // (Unsetting the Mash ID in the Recipe record would be a bit tricky as we'd have to set it to NULL rather than just,
   // say, -1 as, otherwise we'll get a different foreign key constraint violation error (because the DB can't find a
   // Mash/etc row with ID -1!).)
   //
   // At this point, however, the Recipe record has been removed from the database, so we can safely delete any
   // orphaned Mash/etc record.
   //
   this->pimpl->hardDeleteOrphanedStepOwner<Mash        >();
   this->pimpl->hardDeleteOrphanedStepOwner<Boil        >();
   this->pimpl->hardDeleteOrphanedStepOwner<Fermentation>();
   return;
}

// Boilerplate code for FolderBase
FOLDER_BASE_COMMON_CODE(Recipe)

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
   auto owningRecipe = ne.owningRecipe();
   if (!owningRecipe) {
      return;
   }
   if (owningRecipe->isBeingModified()) {
      // Change is not related to a recipe or the recipe is already being modified
      return;
   }

   //
   // Automatic versioning means that, once a recipe is brewed, it is "soft locked" and the first change should spawn a
   // new version.  Any subsequent change should not spawn a new version until it is brewed again.
   //
   if (owningRecipe->brewNotes().empty()) {
      // Recipe hasn't been brewed
      return;
   }

   // If the object we're about to change already has descendants, then we don't want to create new ones.
   if (owningRecipe->hasDescendants()) {
      qDebug() << Q_FUNC_INFO << "Recipe #" << owningRecipe->key() << "already has descendants, so not creating any more";
      return;
   }

   //
   // Once we've started doing versioning, we don't want to trigger it again on the same Recipe until we've finished
   //
   NamedEntityModifyingMarker ownerModifyingMarker(*owningRecipe);

   //
   // Versioning when modifying something in a recipe is *hard*.  If we copy the recipe, there is no easy way to say
   // "this ingredient in the old recipe is that ingredient in the new".  One approach would be to use the delete idea,
   // ie copy everything but what's being modified, clone what's being modified and add the clone to the copy.  Another
   // is to take a deep copy of the Recipe and make that the "prior version".
   //

   // Create a deep copy of the Recipe, and put it in the DB, so it has an ID.
   // (This will also emit signalObjectInserted for the new Recipe from ObjectStoreTyped<Recipe>.)
   qDebug() << Q_FUNC_INFO << "Copying Recipe" << owningRecipe->key();

   // We also don't want to trigger versioning on the newly spawned Recipe until we're completely done here!
   std::shared_ptr<Recipe> spawn = std::make_shared<Recipe>(*owningRecipe);
   NamedEntityModifyingMarker spawnModifyingMarker(*spawn);
   ObjectStoreWrapper::insert(spawn);

   qDebug() << Q_FUNC_INFO << "Copied Recipe #" << owningRecipe->key() << "to new Recipe #" << spawn->key();

   // We assert that the newly created version of the recipe has not yet been brewed (and therefore will not get
   // automatically versioned on subsequent changes before it is brewed).
   Q_ASSERT(spawn->brewNotes().empty());

   //
   // By default, copying a Recipe does not copy all its ancestry.  Here, we want the copy to become our ancestor (ie
   // previous version).  This will also emit a signalPropertyChanged from ObjectStoreTyped<Recipe>, which the UI can
   // pick up to update tree display of Recipes etc.
   //
   owningRecipe->setAncestor(*spawn);

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
