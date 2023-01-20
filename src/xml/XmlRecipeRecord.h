/*
 * xml/XmlRecipeRecord.h is part of Brewtarget, and is Copyright the following
 * authors 2020-2022
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
#ifndef XML_XMLRECIPERECORD_H
#define XML_XMLRECIPERECORD_H
#pragma once

#include "xml/XmlNamedEntityRecord.h"
#include "model/Recipe.h"

/**
 * \brief Read and write a \c Recipe record (including any records it contains) from or to an XML file
 */
class XmlRecipeRecord : public XmlNamedEntityRecord<Recipe> {
public:
   // We only want to override a couple of member functions, so the parent class's constructors are fine for us
   using XmlNamedEntityRecord<Recipe>::XmlNamedEntityRecord;

protected:
   /**
    * \brief We override \c XmlRecord::normaliseAndStoreInDb because we need to be able to store multiple instances of
    *        some child records (Hops, Fermentables, Instructions, etc) and accessing these generically via Qt
    *        properties is hard unless you make the getters and setters all use the same list type, eg QList<QVariant>
    *        instead of QList<Hop *>, QList<Fermentable *>, QList<Instruction *>, etc.
    */
   virtual XmlRecord::ProcessingResult normaliseAndStoreInDb(std::shared_ptr<NamedEntity> containingEntity,
                                                             QTextStream & userMessage,
                                                             ImportRecordCount & stats);

   /**
    * \brief We need to override \c XmlRecord::propertiesToXml for similar reasons that we override
    *        \c normaliseAndStoreInDb()
    */
   virtual void subRecordToXml(XmlRecord::FieldDefinition const & fieldDefinition,
                               XmlRecord const & subRecord,
                               NamedEntity const & namedEntityToExport,
                               QTextStream & out,
                               int indentLevel,
                               char const * const indentString) const;

private:
   /**
    * \brief Add to the recipe child (ie contained) objects of type CNE that have already been read in and stored
    */
   template<typename CNE> void addChildren();

   // As of C++ we have the moral equivalent of templated typdefs, which, here, helps make pointers to member functions
   // on Recipe less ugly
   template <typename CNE>
   using RecipeChildGetter = QList<CNE *> (Recipe::*)() const;

   /**
    * \brief If the supplied property names match, write all the corresponding type to XML
    */
   template<typename CNE>
   bool childrenToXml(XmlRecord::FieldDefinition const & fieldDefinition,
                      XmlRecord const & subRecord,
                      Recipe const & recipe,
                      QTextStream & out,
                      int indentLevel,
                      char const * const indentString,
                      BtStringConst const & propertyNameForGetter,
                      RecipeChildGetter<CNE> getter) const;

};
#endif
