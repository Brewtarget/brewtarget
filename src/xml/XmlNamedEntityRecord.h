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
#include "brewnote.h" ///
#include "instruction.h" ///

#include "model/NamedEntity.h"
#include "xml/XQString.h"
#include "xml/XmlRecord.h"


/**
 * \brief Provides class-specific extensions to \b XmlRecord.  See comment in xml/XmlCoding.h for more details.
 */
template<class NE>
class XmlNamedEntityRecord : public XmlRecord {
public:
   /**
    * \brief This constructor doesn't have to do much more than create an appropriate new subclass of \b NamedEntity.
    *        Everything else is done in the base class.
    */
   XmlNamedEntityRecord(XmlCoding const & xmlCoding,
                        XmlRecord::FieldDefinitions const & fieldDefinitions) :
   XmlRecord{xmlCoding,
             fieldDefinitions} {
      this->namedEntityRaiiContainer.reset(new NE{"Empty Object"});
      this->namedEntity = this->namedEntityRaiiContainer.get();
      this->namedEntityClassName = this->namedEntity->metaObject()->className();
      this->includeInStats = this->includedInStats();
      return;
   }


protected:

   //
   // TODO It's a bit clunky to have the knowledge/logic in this class for whether duplicates and name clashes are
   //      allowed.  Ideally this should be part of the NamedEntity subclasses themselves and the traits used here.
   //      The same applies to whether a NamedEntity subclass is "owned" by another NamedEntity (in the sense that a
   //      MashStep is owned by a Mash.
   //

   /**
    * \brief Implementation for general case where instances are supposed to be unique.  NB: What we really mean here
    *        is that, if we find a Hop/Yeast/Fermentable/etc in an XML file that is "the same" as one that we already
    *        have stored, then we should not read it in.  This says nothing about whether we ourselves multiple copies
    *        of such objects - eg as is currently the case when you add a Hop to a Recipe and a copy of the Hop is
    *        created.  (In the long-run we might want to change how that bit of the code works, but that's another
    *        story.)
    */
   virtual bool isDuplicate() {
      auto currentEntity = this->namedEntity;
      QList<NE *> listOfAllStored = Database::instance().getAll<NE>();
      qDebug() <<
         Q_FUNC_INFO << "Searching list of " << listOfAllStored.size() << " existing " << this->namedEntityClassName <<
         " objects for duplicate with the one we are reading in";
      auto matchingEntity = std::find_if(listOfAllStored.begin(),
                                                      listOfAllStored.end(),
                                                      [currentEntity](NE * ne) {return *ne == *currentEntity;});
      if (matchingEntity != listOfAllStored.end()) {
         qDebug() << Q_FUNC_INFO << "Found a match for " << this->namedEntity->name();
         // Set our pointer to the Hop/Yeast/Fermentable/etc that we already have stored in the database, so that any
         // containing Recipe etc can refer to it.  The new object we created is still held in
         // this->namedEntityRaiiContainer and will automatically be deleted when we go out of scope.
         this->namedEntity = *matchingEntity;
         return true;
      }
      qDebug() << Q_FUNC_INFO << "No match found for "<< this->namedEntity->name();
      return false;
   }

   /**
    * \brief Implementation for general case where name is supposed to be unique.  Before storing, we try to ensure
    *        that what we load in does not create duplicate names.  Eg, if we already have a Recipe called "Oatmeal
    *        Stout" and then read in a (different) recipe with the same name, then we will change the name of the
    *        newly read-in one to "Oatmeal Stout (1)" (or "Oatmeal Stout (2)" if "Oatmeal Stout (1)" is taken, and so
    *        on).  For those NamedEntity subclasses where we don't care about duplicate names (eg MashStep records),
    *        there is a no-op specialisation of this function.
    *
    *        See below for trivial specialisations of this function for classes where names are not unique.
    */
   virtual void normaliseName() {
      QString currentName = this->namedEntity->name();
      QList<NE *> listOfAllStored = Database::instance().getAll<NE>();

      for (auto matchingEntity = std::find_if(listOfAllStored.begin(),
                                                      listOfAllStored.end(),
                                                      [currentName](NE * ne) {return ne->name() == currentName;});
         matchingEntity != listOfAllStored.end();
         matchingEntity = std::find_if(listOfAllStored.begin(),
                                       listOfAllStored.end(),
                                       [currentName](NE * ne) {return ne->name() == currentName;})) {

         qDebug() << Q_FUNC_INFO << "Found existing " << this->namedEntityClassName << "named" << currentName;

         XmlRecord::modifyClashingName(currentName);

         //
         // Now the for loop will search again with the new name
         //
         qDebug() << Q_FUNC_INFO << "Trying " << currentName;
      }

      this->namedEntity->setName(currentName);

      return;
   }

   /**
    * \brief Implementation of the general case where the object is independent of its containing entity
    */
   virtual void setContainingEntity(NamedEntity * containingEntity) {
      return;
   }

private:
   /**
    *
    */
   bool includedInStats() const { return true; }

};

// Specialisations for cases where duplicates are allowed
template<> inline bool XmlNamedEntityRecord<Instruction>::isDuplicate() { return false; }
template<> inline bool XmlNamedEntityRecord<Mash>::isDuplicate() { return false; }
template<> inline bool XmlNamedEntityRecord<MashStep>::isDuplicate() { return false; }
template<> inline bool XmlNamedEntityRecord<BrewNote>::isDuplicate() { return false; }

// Specialisations for cases where name is not required to be unique
template<> inline void XmlNamedEntityRecord<Instruction>::normaliseName() { return; }
template<> inline void XmlNamedEntityRecord<Mash>::normaliseName() { return; }
template<> inline void XmlNamedEntityRecord<MashStep>::normaliseName() { return; }
template<> inline void XmlNamedEntityRecord<BrewNote>::normaliseName() { return; }

// Specialisations for cases where object is owned by its containing entity
template<> inline void XmlNamedEntityRecord<BrewNote>::setContainingEntity(NamedEntity * containingEntity) {
   qDebug() << Q_FUNC_INFO << "BrewNote * " << static_cast<void*>(this->namedEntity) << ", Recipe * " << static_cast<void*>(containingEntity);
   BrewNote * brewNote = static_cast<BrewNote *>(this->namedEntity);
   brewNote->setRecipe(static_cast<Recipe *>(containingEntity));
   return;
}
template<> inline void XmlNamedEntityRecord<Instruction>::setContainingEntity(NamedEntity * containingEntity) {
   Instruction * instruction = static_cast<Instruction *>(this->namedEntity);
   instruction->setRecipe(static_cast<Recipe *>(containingEntity));
   return;
}

// Specialisations for cases where we don't want the objects included in the stats
template<> inline bool XmlNamedEntityRecord<Instruction>::includedInStats() const { return false; }
template<> inline bool XmlNamedEntityRecord<BrewNote>::includedInStats() const { return false; }
template<> inline bool XmlNamedEntityRecord<MashStep>::includedInStats() const { return false; }


#endif
