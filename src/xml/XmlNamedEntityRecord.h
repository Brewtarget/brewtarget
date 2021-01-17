/*
 * xml/XmlNamedEntityRecord.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_XMLNAMEDENTITYRECORD_H
#define _XML_XMLNAMEDENTITYRECORD_H
#pragma once

#include <memory> // For smart pointers

#include <QHash>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVector>

#include "database.h"

#include "model/NamedEntity.h"
#include "xml/XQString.h"
#include "xml/XmlRecord.h"

/**
 * \brief Provides some class-specific extensions to \b XmlRecord.  See comment in xml/XmlCoding.h for more details.
 */
template<class NE>
class XmlNamedEntityRecord : public XmlRecord {
public:
   /**
    * \brief This constructor really just has to create an appropriate new subclass of NamedEntity.  Everything else is
    *        done in the base class.
    */
   XmlNamedEntityRecord(XmlCoding const & xmlCoding,
                        QString const recordName,
                        XmlRecord::FieldDefinitions const & fieldDefinitions) :
   XmlRecord{xmlCoding,
             recordName,
             fieldDefinitions} {
      this->namedEntityRaiiContainer.reset(new NE{"Empty Object"});
      this->namedEntity = this->namedEntityRaiiContainer.get();
      return;
   }

protected:

   /**
    * \brief Finds the first instance of \b NE with \b name() matching \b nameToFind.  This is used to avoid name
    *        clashes when loading some subclass of NamedEntity (eg Hop, Yeast, Equipment, Recipe) from an XML file (eg
    *        if are reading in a Recipe called "Oatmeal Stout" then this function can check whether we already have a
    *        Recipe with that name so that, assuming the new one is not a duplicate, we can amend its name to "Oatmeal
    *        Stout (1)" or some such.
    *
    *        Note that caller does not need to know which subclass of NamedEntity we are reading in, whereas, the
    *        implementation is specific to a subclass (albeit that we can write it once for all subclasses with the
    *        magic of templates).
    * \param nameToFind
    * \return A pointer to a \b NE with a matching \b name(), if there is one, or \b nullptr if not.  Note that this
    *         function does not tell you whether more than one \b NE has the name \b nameToFind
    */
   virtual NamedEntity * findByName(QString nameToFind) {
      QList<NE *> listOfAllStored = Database::instance().getAll<NE>();
      auto found = std::find_if(listOfAllStored.begin(),
                                listOfAllStored.end(),
                                [nameToFind](NE * ne) {return ne->name() == nameToFind;});
      if (found == listOfAllStored.end()) {
         return nullptr;
      }
      return *found;
   }

};

#endif
