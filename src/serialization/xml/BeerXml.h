/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/BeerXml.h is part of Brewtarget, and is copyright the following authors 2020-2022:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#ifndef SERIALIZATION_XML_BEERXML_H
#define SERIALIZATION_XML_BEERXML_H
#pragma once

#include <QFile>
#include <QString>
#include <QTextStream>

/*!
 * \class BeerXML
 *
 * \brief Singleton that handles all translations to and from BeerXML
 *
 * TODO: This should probably become a namespace...
 */
class BeerXML {
public:

   /**
    * \brief Get the singleton instance
    */
   static BeerXML & getInstance();

   virtual ~BeerXML();

   // Export to BeerXML =======================================================

   /**
    * \brief Creates a blank BeerXML document in the supplied file (which the caller should have opened for writing
    *        already).  This can then be supplied to subsequent calls to add BeerXML for Recipes, Hops, etc.
    */
   void createXmlFile(QFile & outFile) const;

   /**
    * \brief Write a list of objects to the supplied file
    */
   template<class NE> void toXml(QList<NE const *> const & nes, QFile & outFile) const;

   /*! Import ingredients, recipes, etc from BeerXML documents.
    * \param filename
    * \param userMessage Where to write any (brief!) message we want to be shown to the user after the import.
    *                    Typically this is either the reason the import failed or a summary of what was imported.
    * \return true if succeeded, false otherwise
    */
   bool importFromXML(QString const & filename, QTextStream & userMessage);

private:

   /**
    * Private constructor as singleton
    */
   BeerXML();

   //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
   BeerXML(BeerXML const&) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
   BeerXML& operator=(BeerXML const&) = delete;
   //! No move constructor
   BeerXML(BeerXML &&) = delete;
   //! No move assignment
   BeerXML & operator=(BeerXML &&) = delete;
};

#endif
