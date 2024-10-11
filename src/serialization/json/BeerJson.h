/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/BeerJson.h is part of Brewtarget, and is copyright the following authors 2021-2022:
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
#ifndef SERIALIZATION_JSON_BEERJSON_H
#define SERIALIZATION_JSON_BEERJSON_H
#pragma once

#include <memory> // For PImpl

#include <QFile>
#include <QList>
#include <QString>
#include <QTextStream>

namespace BeerJson {
   /*!
    * \brief Import ingredients, recipes, etc from a BeerJSON file
    *
    * \param filename
    * \param userMessage Where to write any (brief!) message we want to be shown to the user after the import.
    *                    Typically this is either the reason the import failed or a summary of what was imported.
    *
    * \return true if succeeded, false otherwise
    */
   bool import(QString const & filename, QTextStream & userMessage);

   /**
    * \brief Objects of this class are intended to be relatively short-lived, existing only for the time it takes to
    *        construct the serialized representation and write it to a file.
    */
   class Exporter {
   public:
      /**
      * \param outFile Should be open already.  Caller is responsible for closing it after \c close() or our destructor
      *                is called
      * \param userMessage
      */
      Exporter(QFile & outFile, QTextStream & userMessage);
      ~Exporter();

      /**
      * \brief Add a list of \c NamedEntity objects to the serializer
      */
      template<class NE> void add(QList<NE const *> const & nes);

      /**
      * \brief Write the serialized data to the file.  Will be called in destructor if not already invoked directly.
      */
      void close();

   private:
      // Private implementation details - see https://herbsutter.com/gotw/_100/
      class impl;
      std::unique_ptr<impl> pimpl;

      //! No copy constructor, as we don't want two objects trying to write to the same file
      Exporter(Exporter const&) = delete;
      //! No assignment operator, as never want anyone, not even our friends, to make copies of a singleton.
      Exporter& operator=(Exporter const&) = delete;
      //! No move constructor
      Exporter(Exporter &&) = delete;
      //! No move assignment
      Exporter& operator=(Exporter &&) = delete;
   };

}

#endif
