/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlRecipeRecord.cpp is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#include "serialization/xml/XmlRecipeRecord.h"

#include <cstring>
#include <functional>

#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Style.h"
#include "model/Yeast.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUseOfWater.h"

///namespace {
///   //
///   // To keep us on our toes, the various ingredients you might add to a recipe have different ways of specifying how
///   // much to add and when to add them.  We'll use template specialisation to ensure we call the right member
///   // functions.
///   //
///   template<typename CNE>
///   void setAmountsEtc([[maybe_unused]] CNE & ingredient, [[maybe_unused]] NamedParameterBundle const & npb) {
///      return;
///   }
///   template<> void setAmountsEtc(RecipeAdditionHop & hopAddition, NamedParameterBundle const & npb) {
//////      // For Hop, assume amount is weight unless otherwise specified because base BeerXML does not include the
//////      // possibility of hops being measured by volume.  (It is an extension we have added as a result of
//////      // implementing support for BeerJSON.)
//////      hop.setAmount        (npb.val<double>(PropertyNames::Hop::amount        ));
//////      hop.setAmountIsWeight(npb.val<bool  >(PropertyNames::Hop::amountIsWeight, true));
//////      hop.setTime_min      (npb.val<double>(PropertyNames::Hop::time_min ));
///      return;
///   }
///   template<> void setAmountsEtc(RecipeAdditionFermentable & fermentableAddition, NamedParameterBundle const & npb) {
///      // For Fermentable, assume amount is weight unless otherwise specified because base BeerXML does not include the
///      // possibility of fermentables being measured by volume.  (It is an extension we have added as a result of
///      // implementing support for BeerJSON.)
//////      fermentable.setAmount        (npb.val<double>(PropertyNames::Fermentable::amount        ));
//////      fermentable.setAmountIsWeight(npb.val<bool  >(PropertyNames::Fermentable::amountIsWeight, true));
//////      fermentable.setAddAfterBoil  (npb.val<bool  >(PropertyNames::Fermentable::addAfterBoil  ));
//////      fermentable.setIsMashed      (npb.val<bool  >(PropertyNames::Fermentable::isMashed      ));
///      return;
///   }
///   template<> void setAmountsEtc(RecipeAdditionMisc & miscAddition, NamedParameterBundle const & npb) {
//////      misc.setAmount        (npb.val<double>(PropertyNames::Misc::amount        ));
//////      misc.setAmountIsWeight(npb.val<bool  >(PropertyNames::Misc::amountIsWeight));
//////      misc.setTime_min      (npb.val<double>(PropertyNames::Misc::time_min      ));
///      return;
///   }
///   template<> void setAmountsEtc(RecipeAdditionYeast & yeastAddition, NamedParameterBundle const & npb) {
//////      yeast.setAmount        (npb.val<double>(PropertyNames::Yeast::amount        ));
//////      yeast.setAmountIsWeight(npb.val<bool  >(PropertyNames::Yeast::amountIsWeight));
///      return;
///   }
///   template<> void setAmountsEtc(RecipeUseOfWater & water, NamedParameterBundle const & npb) {
//////      water.setAmount(npb.val<double>(PropertyNames::Water::amount));
///      return;
///   }
///
///}


//template<typename CNE>
//void XmlRecipeRecord::addChildren() {
//   //
//   // This cast is safe because we know this->m_namedEntity was populated with a Recipe * in the constructor of our
//   // parent class (XmlNamedEntityRecord<Recipe>).
//   //
//   auto recipe = std::static_pointer_cast<Recipe>(this->m_namedEntity);
//
//   char const * const childClassName = CNE::staticMetaObject.className();
//   //
//   // Previously we stored child records in a QMultiHash, which makes accessing children of a particular type easy but
//   // gives an iteration order the opposite of insertion order, which is annoying when order matters (eg for Mash Steps
//   // in BeerXML).  Using a list gives us a slightly less elegant loop here, but ensures that
//   // normaliseAndStoreChildRecordsInDb() deals with children in the right order.
//   //
//   for (auto ii : this->m_childRecordSets) {
//      if (ii.parentFieldDefinition->namedEntityClassName == childClassName) {
//         qDebug() <<
//            Q_FUNC_INFO << "Adding " << childClassName << "#" << ii.xmlRecord->getNamedEntity()->key() << "to Recipe";
//
//         // It would be a (pretty unexpected) coding error if the NamedEntity subclass object isn't of the class it's
//         // supposed to be.
//         Q_ASSERT(ii.xmlRecord->getNamedEntity()->metaObject()->className() == QString(childClassName));
//
//         // Actually add the Hop/Yeast/etc to the Recipe
//         std::shared_ptr<CNE> child{std::static_pointer_cast<CNE>(ii.xmlRecord->getNamedEntity())};
//         auto added = recipe->add<CNE>(child);
//
//         //
//         // For historical reasons (specifically that early versions of Brewtarget stored data in BeerXML files, not a
//         // database), the amount of each Hop/Fermentable/etc in a Recipe is stored, not in the Recipe object but in the
//         // Hop/Fermentable/etc in question.  The same is true for addition times for Hops.
//         //
//         // When we add something to a Recipe, typically a copy is made so that we have a Hop/Fermentable/etc that is not
//         // shared with any other Recipes and thus there is no ambiguity about storing the amount in it.
//         //
//         // However, when we read in from BeerXML, we try to avoid creating unnecessary duplicates of things.  If there's
//         // a Fuggle hop in the file and we already have a Fuggle hop in the database, then we don't create another one
//         // for the sake of it.  This is the right thing to do if we're reading in Hops outside the context of a Recipe.
//         // But if the hop in the BeerXML file was inside a Recipe record, then we we need to make sure we captured the
//         // "how much and when to add" info inside that hop record.
//         //
//         // So, now that we added the Hop/Fermentable/etc to the Recipe, and we have the actual object associated with the
//         // Recipe, we need to set the "how much and when to add" info based on the fields we retained from XML record.
//         //
//         Q_ASSERT(added);
//         NamedParameterBundle const & npb = ii.xmlRecord->getNamedParameterBundle();
//         qDebug() <<
//            Q_FUNC_INFO << "Setting amounts for" << childClassName << "#" << added->key() <<
//            "to Recipe, using bundle" << npb;
//         setAmountsEtc(*added, npb);
//      }
//   }
//   return;
//}
//
//
//template<typename CNE>
//void XmlRecipeRecord::addIngredientChildren() {
//   //
//   // This cast is safe because we know this->m_namedEntity was populated with a Recipe * in the constructor of our
//   // parent class (XmlNamedEntityRecord<Recipe>).
//   //
//   auto recipe = std::static_pointer_cast<Recipe>(this->m_namedEntity);
//
//   char const * const childClassName = CNE::staticMetaObject.className();
//   //
//   // Previously we stored child records in a QMultiHash, which makes accessing children of a particular type easy but
//   // gives an iteration order the opposite of insertion order, which is annoying when order matters (eg for Mash Steps
//   // in BeerXML).  Using a list gives us a slightly less elegant loop here, but ensures that
//   // normaliseAndStoreChildRecordsInDb() deals with children in the right order.
//   //
//   for (auto ii : this->childRecords) {
//      if (ii.xmlRecord->namedEntityClassName == childClassName) {
//         qDebug() <<
//            Q_FUNC_INFO << "Adding " << childClassName << "#" << ii.xmlRecord->getNamedEntity()->key() << "to Recipe";
//
//         // It would be a (pretty unexpected) coding error if the NamedEntity subclass object isn't of the class it's
//         // supposed to be.
//         Q_ASSERT(ii.xmlRecord->getNamedEntity()->metaObject()->className() == QString(childClassName));
//
//         //
//         // Actually add the Hop/Yeast/etc to the Recipe
//         //
//         // Here's where we have another impedance mismatch...
//         //
//         // In contrast to our internal data model (and BeerJSON), in BeerXML, there is no difference between a
//         // "freestanding" <HOP>...</HOP> record and one used inside a recipe.  Although this has the merit of
//         // simplicity, it also has drawbacks.  In particular, the <HOP>...</HOP> record contains <AMOUNT>...</AMOUNT>
//         // and <TIME>...</TIME> fields which are only meaningful in the context of a recipe addition but which are both
//         // mandatory even for "freestanding" hops.
//         //
//         // In our internal data model, we have two classes: Hop and RecipeAdditionHop, the latter of which contains a
//         // link to the former.  Amount and time information are part of RecipeAdditionHop, not Hop.  This aligns with
//         // the BeerJSON standard (which aims ultimately to replace BeerXML).
//         //
//         // TODO Finish this!
//
//         std::shared_ptr<CNE> child{std::static_pointer_cast<CNE>(ii.xmlRecord->getNamedEntity())};
//         auto added = recipe->add<CNE>(child);
//
//         //
//         // For historical reasons (specifically that early versions of Brewtarget stored data in BeerXML files, not a
//         // database), the amount of each Hop/Fermentable/etc in a Recipe is stored, not in the Recipe object but in the
//         // Hop/Fermentable/etc in question.  The same is true for addition times for Hops.
//         //
//         // When we add something to a Recipe, typically a copy is made so that we have a Hop/Fermentable/etc that is not
//         // shared with any other Recipes and thus there is no ambiguity about storing the amount in it.
//         //
//         // However, when we read in from BeerXML, we try to avoid creating unnecessary duplicates of things.  If there's
//         // a Fuggle hop in the file and we already have a Fuggle hop in the database, then we don't create another one
//         // for the sake of it.  This is the right thing to do if we're reading in Hops outside the context of a Recipe.
//         // But if the hop in the BeerXML file was inside a Recipe record, then we we need to make sure we captured the
//         // "how much and when to add" info inside that hop record.
//         //
//         // So, now that we added the Hop/Fermentable/etc to the Recipe, and we have the actual object associated with the
//         // Recipe, we need to set the "how much and when to add" info based on the fields we retained from XML record.
//         //
//         Q_ASSERT(added);
//         NamedParameterBundle const & npb = ii.xmlRecord->getNamedParameterBundle();
//         qDebug() <<
//            Q_FUNC_INFO << "Setting amounts for" << childClassName << "#" << added->key() <<
//            "to Recipe, using bundle" << npb;
//         setAmountsEtc(*added, npb);
//      }
//   }
//   return;
//}


XmlRecord::ProcessingResult XmlRecipeRecord::normaliseAndStoreInDb(std::shared_ptr<NamedEntity> containingEntity,
                                                                   QTextStream & userMessage,
                                                                   ImportRecordCount & stats) {
   auto recipe = static_cast<Recipe *>(this->m_namedEntity.get());

   //
   // We need to turn the Recipe's calculations off temporarily.  They are not meaningful until we have stored all the
   // child objects such as ingredient additions, mash, boil etc, and trying to run them before all these things are
   // set causes crashes.
   //
   // TBD: Maybe one day we should do this with RAII.
   //
   recipe->setCalcsEnabled(false);

   // This call to the base class function will store the Recipe and all the objects it contains, as well as link the
   // Recipe to its Style and Equipment.
   XmlRecord::ProcessingResult result = XmlRecord::normaliseAndStoreInDb(containingEntity, userMessage, stats);
   if (result != XmlRecord::ProcessingResult::Succeeded) {
      // The result was either Failed (= abort) or FoundDuplicate (= stop trying to process the current record), so we
      // bail here.
      return result;
   }

   qDebug() << Q_FUNC_INFO << "Final tidy up for Recipe #" << recipe->key() << "(" << recipe->name() << ")";

   //
   // We now need to tie some other things together
   //
//   this->addChildren<Hop>();
//   this->addChildren<Fermentable>();
//   this->addChildren<Misc>();
//   this->addChildren<Yeast>();
//   this->addChildren<Water>();
//
//   this->addChildren<Instruction>();


   // BrewNotes are a bit different than some of the other fields.  Each BrewNote relates to only one Recipe, but the
   // Recipe class does not (currently) have an interface for adding BrewNotes.  It suffices to tell each BrewNote what
   // its Recipe is, something we achieve via template specialisation of XmlNamedEntityRecord::setContainingEntity


   //
   // We have to go through and handle a few things that it is hard to do generically.  Eg, in BeerXML, there is no
   // separate Boil object, but various parameters such as BOIL_SIZE and BOIL_TIME exist directly on Recipe and we use
   // property paths (eg {PropertyNames::Recipe::boil, PropertyNames::Boil::boilTime_mins}) to map them to our internal
   // structure.
   //
   // At this point, the Recipe is stored in the database (so it has a valid ID), and we still have all the parameters
   // read in from the RECIPE record (which NamedParameterBundle will have grouped into sub-bundles for us), so it's
   // straightforward finish things off.
   //
   if (this->m_namedParameterBundle.containsBundle(PropertyNames::Recipe::boil)) {
      // It's a coding error if the recipe already has a boil
      Q_ASSERT(!recipe->boil());

      auto boilBundle = this->m_namedParameterBundle.getBundle(PropertyNames::Recipe::boil);
      if (!boilBundle.contains(PropertyNames::NamedEntity::name)) {
         boilBundle.insert(PropertyNames::NamedEntity::name, QObject::tr("Boil for %1").arg(recipe->name()));
      }
      if (!boilBundle.contains(PropertyNames::Boil::description)) {
         boilBundle.insert(PropertyNames::Boil::description, QObject::tr("Automatically created by BeerXML import"));
      }
      auto boil = std::make_shared<Boil>(boilBundle);
      // This call will also ensure the boil gets saved in the DB
      recipe->setBoil(boil);

      qDebug() << Q_FUNC_INFO << "Created Boil #" << boil->key() << "on Recipe #" << recipe->key();
   }
   if (this->m_namedParameterBundle.containsBundle(PropertyNames::Recipe::fermentation)) {
      // It's a coding error if the recipe already has a fermentation
      Q_ASSERT(!recipe->fermentation());

      auto fermentationBundle = this->m_namedParameterBundle.getBundle(PropertyNames::Recipe::fermentation);
      if (!fermentationBundle.contains(PropertyNames::NamedEntity::name)) {
         fermentationBundle.insert(PropertyNames::NamedEntity::name,
                                   QObject::tr("Fermentation for %1").arg(recipe->name()));
      }
      if (!fermentationBundle.contains(PropertyNames::Fermentation::description)) {
         fermentationBundle.insert(PropertyNames::Fermentation::description,
                                   QObject::tr("Automatically created by BeerXML import"));
      }
      auto fermentation = std::make_shared<Fermentation>(fermentationBundle);
      // This call will also ensure the fermentation gets saved in the DB
      recipe->setFermentation(fermentation);

      qDebug() << Q_FUNC_INFO << "Created Fermentation #" << fermentation->key() << "on Recipe #" << recipe->key();

      //
      // Now we handle RECIPE > PRIMARY_AGE / PRIMARY_TEMP / SECONDARY_AGE / SECONDARY_TEMP / TERTIARY_AGE /
      // TERTIARY_TEMP.
      //
      // To keep things simple we make the (hopefully quite reasonable) assumptions that we should ignore secondary if
      // primary is not present, and ignore tertiary if secondary is not present.
      //
      // The call to fermentation->addStep automatically handles saving the step in the DB
      //
      if (fermentationBundle.containsBundle(PropertyNames::Fermentation::primary)) {
         auto primaryBundle {fermentationBundle.getBundle(PropertyNames::Fermentation::primary)};
         qDebug() << Q_FUNC_INFO << primaryBundle;
         if (!primaryBundle.contains(PropertyNames::NamedEntity::name)) {
            primaryBundle.insert(PropertyNames::NamedEntity::name,
                                 QObject::tr("Primary Fermentation Step for %1").arg(recipe->name()));
         }
         fermentation->addStep(std::make_shared<FermentationStep>(primaryBundle));
         if (fermentationBundle.containsBundle(PropertyNames::Fermentation::secondary)) {
            auto secondaryBundle {fermentationBundle.getBundle(PropertyNames::Fermentation::secondary)};
            if (!secondaryBundle.contains(PropertyNames::NamedEntity::name)) {
               secondaryBundle.insert(PropertyNames::NamedEntity::name,
                                    QObject::tr("Secondary Fermentation Step for %1").arg(recipe->name()));
            }
            fermentation->addStep(std::make_shared<FermentationStep>(secondaryBundle));
            if (fermentationBundle.containsBundle(PropertyNames::Fermentation::tertiary)) {
               auto tertiaryBundle {fermentationBundle.getBundle(PropertyNames::Fermentation::tertiary)};
               if (!tertiaryBundle.contains(PropertyNames::NamedEntity::name)) {
                  tertiaryBundle.insert(PropertyNames::NamedEntity::name,
                                       QObject::tr("Tertiary Fermentation Step for %1").arg(recipe->name()));
               }
               fermentation->addStep(std::make_shared<FermentationStep>(tertiaryBundle));
            }
         }
      }


   }

   static_cast<Recipe *>(this->m_namedEntity.get())->setCalcsEnabled(true);
   return XmlRecord::ProcessingResult::Succeeded;
}

///template<typename CNE>
///bool XmlRecipeRecord::childrenToXml(XmlRecordDefinition::FieldDefinition const & fieldDefinition,
///                                    XmlRecord const & subRecord,
///                                    Recipe const & recipe,
///                                    QTextStream & out,
///                                    int indentLevel,
///                                    char const * const indentString,
///                                    BtStringConst const & propertyNameForGetter,
///                                    RecipeChildGetter<CNE> getter) const {
///   if (fieldDefinition.propertyPath.asXPath() != propertyNameForGetter) {
///      return false;
///   }
///   QList<CNE *> children = std::invoke(getter, recipe);
///   if (children.size() == 0) {
///      this->writeNone(subRecord, recipe, out, indentLevel, indentString);
///   } else {
///      for (CNE * child : children) {
///         subRecord.toXml(*child, out, true, indentLevel, indentString);
///      }
///   }
///   return true;
///}

template<typename RecipeChildGetter>
bool XmlRecipeRecord::childrenToXml(XmlRecordDefinition::FieldDefinition const & fieldDefinition,
                                    XmlRecord const & subRecord,
                                    Recipe const & recipe,
                                    QTextStream & out,
                                    int indentLevel,
                                    char const * const indentString,
                                    BtStringConst const & propertyNameForGetter,
                                    RecipeChildGetter getter) const {
   if (fieldDefinition.propertyPath.asXPath() != propertyNameForGetter) {
      return false;
   }
   auto children = std::invoke(getter, recipe);
   if (children.size() == 0) {
      this->writeNone(subRecord, recipe, out, indentLevel, indentString);
   } else {
      for (auto child : children) {
         subRecord.toXml(*child, out, true, indentLevel, indentString);
      }
   }
   return true;
}

void XmlRecipeRecord::subRecordToXml(XmlRecordDefinition::FieldDefinition const & fieldDefinition,
                                     XmlRecord const & subRecord,
                                     NamedEntity const & namedEntityToExport,
                                     QTextStream & out,
                                     int indentLevel,
                                     char const * const indentString) const {
   //
   // This cast should be safe because Recipe & should be what's passed to XmlRecipeRecord::toXml() (which invokes the
   // base class member function which ultimately calls this one with the same parameter).
   //
   Recipe const & recipe = static_cast<Recipe const &>(namedEntityToExport);

   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::hopAdditions        , &Recipe::hopAdditions        )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::fermentableAdditions, &Recipe::fermentableAdditions)) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::miscAdditions       , &Recipe::miscAdditions       )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::yeastAdditions      , &Recipe::yeastAdditions      )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::waterUses           , &Recipe::waterUses           )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::instructions        , &Recipe::instructions        )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::brewNotes           , &Recipe::brewNotes           )) { return; }

   // It's a coding error if we get here
   qCritical() << Q_FUNC_INFO << "Don't know how to export Recipe property " << fieldDefinition.propertyPath.asXPath();
   Q_ASSERT(false); // Stop in a debug build
   return;          // Soldier on in a production build
}
