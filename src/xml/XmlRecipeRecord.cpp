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

#include "model/Hop.h"
#include "model/Fermentable.h"
#include "model/Misc.h"
#include "model/Yeast.h"
#include "model/Water.h"

template<typename CNE>
void XmlRecipeRecord::addChildren() {
   //
   // This cast is safe because we know this->namedEntity was populated with a Recipe * in the constructor of our
   // parent class (XmlNamedEntityRecord<Recipe>).
   //
   Recipe * recipe = static_cast<Recipe *>(this->namedEntity);

   //
   // Subclasses of NamedEntity have a static method that gives us the class name.  Without this we would either have
   // to instantiate a new instance of the class (to use QMetaObject::className()) or pass the class name in as a
   // parameter to this function.
   //
   QByteArray childClassName = CNE::classNameStr().toLatin1();

   //
   // QMultiHash guarantees that items that share the same key will appear consecutively, from the most recently to
   // the least recently inserted value.  So the most efficient way to obtain all values with the same key is to call
   // find() and iterate from there.  (The alternative, is to call values() which returns a QList of matching values,
   // but requires a copy which is (a) less efficient and (b) not accepted by all compilers for the types we are
   // using.)
   //
   for (auto ii = this->childRecords.find(childClassName.constData());
        ii != this->childRecords.end() && ii.key() == childClassName.constData();
        ++ii) {
      qDebug() << Q_FUNC_INFO << "Adding " << childClassName.constData() << " to Recipe";

      // It would be a (pretty unexpected) coding error if the NamedEntity subclass object stored against a class name
      // isn't of the same class against which it was stored.
      Q_ASSERT(ii->second->getNamedEntity()->metaObject()->className() == QString(childClassName.constData()));

      // Actually add the Hop/Yeast/etc to the Recipe
      recipe->add<CNE>(static_cast<CNE *>(ii->second->getNamedEntity()));
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

   // BrewNotes and Instructions are a bit different than some of the other fields.  Each BrewNote and each Instruction
   // relate to only one Recipe, but the Recipe class does not (currently) have an interface for adding BrewNotes or
   // Instructions.  It suffices to tell each BrewNote and each Instruction what its Recipe is, something we achieve
   // via template specialisation of XmlNamedEntityRecord::setContainingEntity

   return XmlRecord::Succeeded;
}
