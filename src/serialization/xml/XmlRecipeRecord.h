/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlRecipeRecord.h is part of Brewtarget, and is copyright the following authors 2020-2023:
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
#ifndef SERIALIZATION_XML_XMLRECIPERECORD_H
#define SERIALIZATION_XML_XMLRECIPERECORD_H
#pragma once

#include "serialization/xml/XmlNamedEntityRecord.h"
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
   [[nodiscard]] virtual XmlRecord::ProcessingResult normaliseAndStoreInDb(std::shared_ptr<NamedEntity> containingEntity,
                                                                           QTextStream & userMessage,
                                                                           ImportRecordCount & stats) override;

   /**
    * \brief We override \c XmlRecord::normaliseAndStoreChildRecordsInDb because we want to create a child record for
    *        \c Boil (which isn't modelled as a child record in BeerXML).
    */
   [[nodiscard]] virtual bool normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
                                                                ImportRecordCount & stats) override;

   /**
    * \brief We need to override \c XmlRecord::propertiesToXml for similar reasons that we override
    *        \c normaliseAndStoreInDb()
    */
   virtual void subRecordToXml(XmlRecordDefinition::FieldDefinition const & fieldDefinition,
                               XmlRecord const & subRecord,
                               NamedEntity const & namedEntityToExport,
                               QTextStream & out,
                               int indentLevel,
                               char const * const indentString) const override;

private:
   /**
    * \brief Add to the recipe child (ie contained) objects of type CNE that have already been read in and stored
    */
   template<typename CNE> void addChildren();

   // As of C++ we have the moral equivalent of templated typdefs, which, here, helps make pointers to member functions
   // on Recipe less ugly
   template <typename CNE> using RecipeChildGetterRaw    = QList<                CNE *> (Recipe::*)() const;
   template <typename CNE> using RecipeChildGetterShared = QList<std::shared_ptr<CNE> > (Recipe::*)() const;

   /**
    * \brief If the supplied property names match, write all the corresponding type to XML
    */
   template<typename RecipeChildGetter>
   bool childrenToXml(XmlRecordDefinition::FieldDefinition const & fieldDefinition,
                      XmlRecord const & subRecord,
                      Recipe const & recipe,
                      QTextStream & out,
                      int indentLevel,
                      char const * const indentString,
                      BtStringConst const & propertyNameForGetter,
                      RecipeChildGetter getter) const;
};
#endif
