/*
 * xml/XmlRecipeRecord.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
 * - Matt Young <mfsy@yahoo.com>
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
#include "xml/XmlRecipeRecord.h"

#include <cstring>
#include <functional>

#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"

namespace {
   //
   // To keep us on our toes, the various ingredients you might add to a recipe have different ways of specifying how
   // much to add and when to add them.  We'll use template specialisation to ensure we call the right member
   // functions.
   //
   template<typename CNE>
   void setAmountsEtc(CNE & ingredient, NamedParameterBundle const & npb) {
      return;
   }
   template<> void setAmountsEtc(Hop & hop, NamedParameterBundle const & npb) {
      hop.setAmount_kg(npb(PropertyNames::Hop::amount_kg).toDouble());
      hop.setTime_min( npb(PropertyNames::Hop::time_min).toDouble());
      return;
   }
   template<> void setAmountsEtc(Fermentable & fermentable, NamedParameterBundle const & npb) {
      fermentable.setAmount_kg(npb(PropertyNames::Fermentable::amount_kg).toDouble());
      return;
   }
   template<> void setAmountsEtc(Misc & misc, NamedParameterBundle const & npb) {
      misc.setAmount(        npb(PropertyNames::Misc::amount).toDouble());
      misc.setAmountIsWeight(npb(PropertyNames::Misc::amountIsWeight).toBool());
      misc.setTime(          npb(PropertyNames::Misc::time).toDouble());
      return;
   }
   template<> void setAmountsEtc(Yeast & yeast, NamedParameterBundle const & npb) {
      yeast.setAmount(        npb(PropertyNames::Yeast::amount).toDouble());
      yeast.setAmountIsWeight(npb(PropertyNames::Yeast::amountIsWeight).toBool());
      return;
   }
   template<> void setAmountsEtc(Water & water, NamedParameterBundle const & npb) {
      water.setAmount(        npb(PropertyNames::Water::amount).toDouble());
      return;
   }

}


template<typename CNE>
void XmlRecipeRecord::addChildren() {
   //
   // This cast is safe because we know this->namedEntity was populated with a Recipe * in the constructor of our
   // parent class (XmlNamedEntityRecord<Recipe>).
   //
   Recipe * recipe = static_cast<Recipe *>(this->namedEntity);

   char const * const childClassName = CNE::staticMetaObject.className();

   //
   // QMultiHash guarantees that items that share the same key will appear consecutively, from the most recently to
   // the least recently inserted value.  So the most efficient way to obtain all values with the same key is to call
   // find() and iterate from there.  (The alternative, is to call values() which returns a QList of matching values,
   // but requires a copy which is (a) less efficient and (b) not accepted by all compilers for the types we are
   // using.)
   //
   for (auto ii = this->childRecords.find(childClassName);
        ii != this->childRecords.end() && ii.key() == childClassName;
        ++ii) {
      qDebug() << Q_FUNC_INFO << "Adding " << childClassName << " to Recipe";

      // It would be a (pretty unexpected) coding error if the NamedEntity subclass object stored against a class name
      // isn't of the same class against which it was stored.
      Q_ASSERT(ii->second->getNamedEntity()->metaObject()->className() == QString(childClassName));

      // Actually add the Hop/Yeast/etc to the Recipe
      CNE * added = recipe->add<CNE>(static_cast<CNE *>(ii->second->getNamedEntity()));

      //
      // For historical reasons (specifically that early versions of Brewtarget stored data in BeerXML files, not a
      // database), the amount of each Hop/Fermentable/etc in a Recipe is stored, not in the Recipe object but in the
      // Hop/Fermentable/etc in question.  The same is true for addition times for Hops.
      //
      // When we add something to a Recipe, typically a copy is made so that we have a Hop/Fermentable/etc that is not
      // shared with any other Recipes and thus there is no ambiguity about storing the amount in it.
      //
      // However, when we read in from BeerXML, we try to avoid creating unnecessary duplicates of things.  If there's
      // a Fuggle hop in the file and we already have a Fuggle hop in the database, then we don't create another one
      // for the sake of it.  This is the right thing to do if we're reading in Hops outside the context of a Recipe.
      // But if the hop in the BeerXML file was inside a Recipe record, then we we need to make sure we captured the
      // "how much and when to add" info inside that hop record.
      //
      // So, now that we added the Hop/Fermentable/etc to the Recipe, and we have the actual object associated with the
      // Recipe, we need to set the "how much and when to add" info based on the fields we retained from XML record.
      //
      Q_ASSERT(added != nullptr);
      setAmountsEtc(*added, ii->second->getNamedParameterBundle());
   }
   return;
}


XmlRecord::ProcessingResult XmlRecipeRecord::normaliseAndStoreInDb(NamedEntity * containingEntity,
                                                                   QTextStream & userMessage,
                                                                   XmlRecordCount & stats) {
   // This call to the base class function will store the Recipe and all the objects it contains, as well as link the
   // Recipe to its Style and Equipment.
   XmlRecord::ProcessingResult result = XmlRecord::normaliseAndStoreInDb(containingEntity, userMessage, stats);
   if (result != XmlRecord::Succeeded) {
      // The result was either Failed (= abort) or FoundDuplicate (= stop trying to process the current record), so we
      // bail here.
      return result;
   }

   //
   // We now need to tie some other things together
   //
   this->addChildren<Hop>();
   this->addChildren<Fermentable>();
   this->addChildren<Misc>();
   this->addChildren<Yeast>();
   this->addChildren<Water>();

   this->addChildren<Instruction>();


   // BrewNotes are a bit different than some of the other fields.  Each BrewNote relates to only one Recipe, but the
   // Recipe class does not (currently) have an interface for adding BrewNotes.  It suffices to tell each BrewNote what
   // its Recipe is, something we achieve via template specialisation of XmlNamedEntityRecord::setContainingEntity

   return XmlRecord::Succeeded;
}

template<typename CNE>
bool XmlRecipeRecord::childrenToXml(XmlRecord::FieldDefinition const & fieldDefinition,
                                    XmlRecord const & subRecord,
                                    Recipe const & recipe,
                                    QTextStream & out,
                                    int indentLevel,
                                    char const * const indentString,
                                    BtStringConst const & propertyNameForGetter,
                                    RecipeChildGetter<CNE> getter) const {
   if (fieldDefinition.propertyName != propertyNameForGetter) {
      return false;
   }
   QList<CNE *> children = std::invoke(getter, recipe);
   if (children.size() == 0) {
      this->writeNone(subRecord, recipe, out, indentLevel, indentString);
   } else {
      for (CNE * child : children) {
         subRecord.toXml(*child, out, indentLevel, indentString);
      }
   }
   return true;
}

void XmlRecipeRecord::subRecordToXml(XmlRecord::FieldDefinition const & fieldDefinition,
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

   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::hops        , &Recipe::hops        )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::fermentables, &Recipe::fermentables)) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::miscs       , &Recipe::miscs       )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::yeasts      , &Recipe::yeasts      )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::waters      , &Recipe::waters      )) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::instructions, &Recipe::instructions)) { return; }
   if (this->childrenToXml(fieldDefinition, subRecord, recipe, out, indentLevel, indentString, PropertyNames::Recipe::brewNotes   , &Recipe::brewNotes   )) { return; }

   // It's a coding error if we get here
   qCritical() << Q_FUNC_INFO << "Don't know how to export Recipe property " << fieldDefinition.propertyName;
   Q_ASSERT(false); // Stop in a debug build
   return;          // Soldier on in a production build
}
