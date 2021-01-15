/*
 * xml/XmlNamedEntityRecord.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/XmlNamedEntityRecord.h"

#include <QDebug>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QMetaProperty>

#include "xml/XmlCoding.h"

template<> void XmlNamedEntityRecord<Hop>::init()         { return; }
template<> void XmlNamedEntityRecord<Fermentable>::init() { return; }
template<> void XmlNamedEntityRecord<Yeast>::init()       { return; }
template<> void XmlNamedEntityRecord<Misc>::init()        { return; }
template<> void XmlNamedEntityRecord<Water>::init()       { return; }
template<> void XmlNamedEntityRecord<Style>::init()       { return; }
template<> void XmlNamedEntityRecord<MashStep>::init()    { this->instanceNamesAreUnique = false; return; }
template<> void XmlNamedEntityRecord<Mash>::init()        { return; }
template<> void XmlNamedEntityRecord<Equipment>::init()   { return; }


/*
XmlNamedEntityRecord::XmlNamedEntityRecord(XmlCoding const & xmlCoding,
                                           QString const recordName,
                                           XmlNamedEntityRecord::NameUniqueness uniquenessOfInstanceNames,
                                           QVector<Field> const & fieldDefinitions,
                                           NamedEntity * entityToPopulate) :
   XmlRecord{xmlCoding, recordName, fieldDefinitions},
   uniquenessOfInstanceNames{uniquenessOfInstanceNames},
   entityToPopulate{entityToPopulate} {
   Q_ASSERT(nullptr != entityToPopulate);
   return;
}
*/
/*
void XmlNamedEntityRecord::storeField(XmlRecord::Field const & fieldDefinition,
                                      QVariant parsedValue) {
   int propertyIndex = this->entityToPopulate->metaObject()->indexOfProperty(fieldDefinition.propertyName);

   //
   // It's a coding error if we are trying to read and store a field that does not exist on the object we are loading
   //
   Q_ASSERT(propertyIndex >= 0 && "Trying to update undeclared property");
   if ( propertyIndex < 0 ) {
      //
      // If asserts are disabled, we may be able to continue past this coding error by ignoring the current field
      //
      qCritical() <<
         Q_FUNC_INFO << "Trying to update undeclared property " << fieldDefinition.propertyName << " of " <<
         this->entityToPopulate->metaObject()->className();
      return;
   }

   QMetaProperty metaProperty = this->entityToPopulate->metaObject()->property(propertyIndex);
   metaProperty.write(this->entityToPopulate.get(), parsedValue);
   return;
}
*/

/*
bool XmlNamedEntityRecord::normaliseAndStoreInDb(QTextStream & userMessage,
                                                 XmlRecordCount & stats) {
   if (XmlNamedEntityRecord::EachInstanceNameShouldBeUnique == this->uniquenessOfInstanceNames) {
      QString currentName = this->entityToPopulate->name();

      for (NamedEntity * matchingEntity = this->findByName(currentName);
           nullptr != matchingEntity;
           matchingEntity = this->findByName(currentName)) {

         qDebug() <<
            Q_FUNC_INFO << "Existing " << this->recordName << "named" << currentName << "was" <<
            ((nullptr == matchingEntity) ? "not" : "") << "found";

         XmlNamedEntityRecord::modifyClashingName(currentName);

         //
         // Now the for loop will search again with the new name
         //
         qDebug() << Q_FUNC_INFO << "Trying " << currentName;
      }

      this->entityToPopulate->setName(currentName);
   }

   // Now we're ready to store in the DB, something the NamedEntity knows how to make happen
   this->entityToPopulate->insertInDatabase();

   // Once we've stored the object, we no longer have to take responsibility for destroying it because its registry
   // (currently the Database singleton) will now own it.
   this->entityToPopulate.release();

   stats.processedOk(this->recordName.toLower());

   return true;
}
*/
