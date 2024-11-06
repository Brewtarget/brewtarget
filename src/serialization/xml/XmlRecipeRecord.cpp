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
      boilBundle.insertIfNotPresent(PropertyNames::NamedEntity::name, QObject::tr("Boil for %1").arg(recipe->name()));
      boilBundle.insertIfNotPresent(PropertyNames::Boil::description, QObject::tr("Automatically created by BeerXML import"));
      auto boil = std::make_shared<Boil>(boilBundle);
      // This call will also ensure the boil gets saved in the DB
      recipe->setBoil(boil);

      qDebug() << Q_FUNC_INFO << "Created Boil #" << boil->key() << "on Recipe #" << recipe->key();
   }
   if (this->m_namedParameterBundle.containsBundle(PropertyNames::Recipe::fermentation)) {
      // It's a coding error if the recipe already has a fermentation
      Q_ASSERT(!recipe->fermentation());

      auto fermentationBundle = this->m_namedParameterBundle.getBundle(PropertyNames::Recipe::fermentation);
      fermentationBundle.insertIfNotPresent(PropertyNames::NamedEntity::name,
                                            QObject::tr("Fermentation for %1").arg(recipe->name()));
      fermentationBundle.insertIfNotPresent(PropertyNames::Fermentation::description,
                                            QObject::tr("Automatically created by BeerXML import"));
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
         primaryBundle.insertIfNotPresent(PropertyNames::NamedEntity::name,
                                          QObject::tr("Primary Fermentation Step for %1").arg(recipe->name()));
         primaryBundle.insertIfNotPresent(PropertyNames::Step::description,
                                          QObject::tr("Automatically created by BeerXML import"));
         fermentation->addStep(std::make_shared<FermentationStep>(primaryBundle));
         if (fermentationBundle.containsBundle(PropertyNames::Fermentation::secondary)) {
            auto secondaryBundle {fermentationBundle.getBundle(PropertyNames::Fermentation::secondary)};
            secondaryBundle.insertIfNotPresent(PropertyNames::NamedEntity::name,
                                               QObject::tr("Secondary Fermentation Step for %1").arg(recipe->name()));
            secondaryBundle.insertIfNotPresent(PropertyNames::Step::description,
                                               QObject::tr("Automatically created by BeerXML import"));
            fermentation->addStep(std::make_shared<FermentationStep>(secondaryBundle));
            if (fermentationBundle.containsBundle(PropertyNames::Fermentation::tertiary)) {
               auto tertiaryBundle {fermentationBundle.getBundle(PropertyNames::Fermentation::tertiary)};
               tertiaryBundle.insertIfNotPresent(PropertyNames::NamedEntity::name,
                                                 QObject::tr("Tertiary Fermentation Step for %1").arg(recipe->name()));
               tertiaryBundle.insertIfNotPresent(PropertyNames::Step::description,
                                                 QObject::tr("Automatically created by BeerXML import"));
               fermentation->addStep(std::make_shared<FermentationStep>(tertiaryBundle));
            }
         }
      }
   }

   static_cast<Recipe *>(this->m_namedEntity.get())->setCalcsEnabled(true);
   return XmlRecord::ProcessingResult::Succeeded;
}

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
