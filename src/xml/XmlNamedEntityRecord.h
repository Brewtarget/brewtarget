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

/////////////////
//
// TODO Still need to sort out MashStep getting pointer to the Mash
//
// TODO Still need to do Recipe
//
/////////////////


/**
 * TODO Rewrite this! <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 * \brief \b XmlNamedEntityRecord is a base class that provides the generic functionality for parsing individual "object"
 * records (eg corresponding to Hop, Yeast, Equipment, Recipe etc) from an XML document.  An object of this base class
 * (initialised with suitable mapping data) can read "simple" fields (strings, decimals, integers, booleans, enums)
 * into a hash map and simultaneously populate scalar attributes of an object that inherits from NamedEntity.  (And the
 * implementation for doing this is mostly hidden from users of the class.)  However, what this base class can't do is:
 *   (a) create a new Hop/Yeast/Equipment/etc object or get a list of all existing stored Hop/Yeast/Equipment/etc objects
 *   (b) handle nested records (eg the fact that a Recipe records contains Hop records)
 * For that, child classes are needed.
 *
 * For BeerXML specifically, we have several child classes.
 *
 * \b BeerXmlSimpleRecordLoader is a templated class that inherits from \b XmlNamedEntityRecord and solves (a) by being
 * able to do class-specific things with the class of its template parameter.  (It also, via template specialisations,
 * initialises mapping data for its base class.)  In a number of cases, this is all that is needed:
 *    \b BeerXmlSimpleRecordLoader<Equipment> suffices to read EQUIPMENT records
 *    \b BeerXmlSimpleRecordLoader<Fermentable> suffices to read FERMENTABLE records
 *    \b BeerXmlSimpleRecordLoader<Hop> suffices to read HOP records
 *    \b BeerXmlSimpleRecordLoader<Misc> suffices to read MISC records
 *    \b BeerXmlSimpleRecordLoader<Style> suffices to read STYLE records
 *    \b BeerXmlSimpleRecordLoader<Water> suffices to read WATER records
 *    \b BeerXmlSimpleRecordLoader<Yeast> suffices to read YEAST records
 * For other cases, this needs to be extended further:
 *    \b BeerXmlMashRecordLoader : public BeerXmlSimpleRecordLoader<Mash> - handles fact that Mash - contains MashStep
 *    \b BeerXmlMashStepRecordLoader : public BeerXmlSimpleRecordLoader<MashStep> - handles fact that MashStep needs to
 *                                                                                  know its Mash
 *    \b BeerXmlRecipeRecordLoader : public BeerXmlSimpleRecordLoader<Recipe> - handles fact that Recipe contains lots
 *                                                                              of other things
 *
 * Eg say you have the following section in an XML document (amended and simplified from real BeerXML):
 *    <HOPS>
 *       <HOP>
 *          <NAME>Fuggle</NAME>
 *          <ALPHA>4.2</ALPHA>
 *          <WEIGHT>
 *             <UNIT_OF_MEASUREMENT>g<UNIT_OF_MEASUREMENT>
 *             <VALUE>100</VALUE>
 *          </WEIGHT>
 *          <TYPE>Aroma</TYPE>
 *       </HOP>
 *       <HOP>
 *          <NAME>East Kent Golding</NAME>
 *          <ALPHA>3.7</ALPHA>
 *          <WEIGHT>
 *             <UNIT_OF_MEASUREMENT>g<UNIT_OF_MEASUREMENT>
 *             <VALUE>50</VALUE>
 *          </WEIGHT>
 *          <TYPE>Both</TYPE>
 *       </HOP>
 *    </HOPS>
 * As we process the XML document, at each of the <HOP> tags, we can use a BeerXmlHopRecordLoader object (which
 * inherits from XmlNamedEntityRecord) to parse the contents (ie everything inside the current <HOP>...</HOP> pair) into a
 * new Hop object.  If that parsing succeeds then the object can be saved (or, as the case may be, passed back to code
 * in BeerXmlRecipeRecordLoader that is processing a containing <RECIPE>...</RECIPE> tag pair).
 *
 * For every field in the XML that we want to be able to process, we need an \b XmlNamedEntityRecord::Field entry which
 * has:
 *
 *    \b fieldType    The base data type we want to read the XML field into (QString, double, int, etc).  Though NB for
 *                    something that we'll end up storing as an enum, we first read it into a QString
 *
 *    \b xPath        The relative XPath of the tag in the XML inside the record we're looking at.  Usually this is
 *                    just the tag name (eg "NAME", "ALPHA", "TYPE" in our example above) but nested tags are supported
 *                    too(eg "WEIGHT/UNIT_OF_MEASUREMENT" and "WEIGHT/VALUE" in our example above).
 *
 *                    Note, however, that where there is a complex contained item, we'll usually want to process that
 *                    separately with its own XmlNamedEntityRecord.  Eg, inside a RECIPE, we'll want separate
 *                    XmlNamedEntityRecords to process HOPS, MISCS, STYLE, etc.
 *
 *    \b propertyName If set, this is the name of the Q_PROPERTY on the object we're constructing from this XML record.
 *                    Via the magic of the Qt Property System, we'll then be able to pass the field value in to the
 *                    object via its setter without having to deal with a lot of pointers to functions etc.
 *
 *                    If not set, then we'll read the property from the file and make it available to the caller via
 *                    getField(), but won't try to store it in the object.
 *
 *    \b stringToEnum This is an optional parameter that maps a string to an enum value.  Eg for the USE field of a HOP
 *                    record, we want to map "TYPE" string field to a Hop::Type enum value.
 *
 */
template<class NE>
class XmlNamedEntityRecord : public XmlRecord {
public:
   /**
    * \brief This constructor does all the boilerplate work and then calls init() to do the template parameter specific
    *        stuff.  (We could do template specialisations of the constructor and do away with init(), but it's a lot
    *        of copy-and-paste code.)  TODO EXPLAIN THIS BETTER
    */
   XmlNamedEntityRecord(XmlCoding const & xmlCoding,
                        QString const recordName,
                        XmlRecord::FieldDefinitions const & fieldDefinitions) :
   XmlRecord{xmlCoding,
             recordName,
             fieldDefinitions},
   instanceNamesAreUnique{true}, // This is the default value, but might be changed in init()
   entityToPopulate{new NE{"Empty Object"} } {
      this->init();
      return;
   }

   /**
    * \brief Called by the constructor to the template parameter specific initialisation.
    *
    *        We only use template specialisations of this member function, so just declare here and put specialisation
    *        definitions in xml/XmlNamedEntityRecord.cpp
    */
   void init();

protected:

   /**
    * \brief Constructor for derived classes to call
    * \param recordName Name of the type of XML record we are parsing (eg "HOP").
    * \param uniquenessOfInstanceNames If \b EachInstanceNameShouldBeUnique, then we try to ensure that what we load in
    *        does not create duplicate names.  Eg, if we already have a Recipe called "Oatmeal Stout" and then read in
    *        a (different) recipe with the same name, then we will change the name of the newly read-in one to "Oatmeal
    *        Stout (1)" (or "Oatmeal Stout (2)" if "Oatmeal Stout (1)" is taken, and so on).  Where we don't care about
    *        duplicate names (eg loading in MashStep records), this parameter should be set to
    *        \b InstancesWithDuplicateNamesOk.
    * \param fieldDefinitions A list of fields we expect to find in this record (other fields will be ignored) and how
    *                         they relate to Brewtarget objects.  See class description for more details.
    * \param entityToPopulate A new/empty instance of a suitable subclass of NamedEntity for us to populate (eg a Hop if
    *        we are reading a HOP record)
    */
//   XmlNamedEntityRecord(XmlCoding const & xmlCoding,
//                        QString const recordName,
//                        NameUniqueness uniquenessOfInstanceNames,
//                        QVector<Field> const & fieldDefinitions,
//                        NamedEntity * entityToPopulate);
//
   /**
    * \brief Finds the first instance of \b T with \b name() matching \b nameToFind.  (Child classes should implement
    *        this virtual member function by making a suitable call to the templated static function of the same name.)
    * \param nameToFind
    * \return A pointer to a T with a matching \b name(), if there is one, or \b nullptr if not.  Note that this function
    *         does not tell you whether more than one T has the name \b nameToFind
    */
//   virtual NamedEntity * findByName(QString nameToFind) = 0;
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
   NamedEntity * findByName(QString nameToFind) {
      QList<NE *> listOfAllStored = Database::instance().getAll<NE>();
      auto found = std::find_if(listOfAllStored.begin(),
                                listOfAllStored.end(),
                                [nameToFind](NE * ne) {return ne->name() == nameToFind;});
      if (found == listOfAllStored.end()) {
         return nullptr;
      }
      return *found;
   }

public:

   /**
    * \brief Implementation of \b XmlRecord::storeField()
    */
   virtual void storeField(Field const & fieldDefinition,
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


   /**
    * \brief Implementation of \b XmlRecord::normaliseAndStoreInDb(). Child classes should extend this member
    *        function for any class-specific logic.
    */
   virtual bool normaliseAndStoreInDb(QTextStream & userMessage,
                                      XmlRecordCount & stats) {
      if (this->instanceNamesAreUnique) {
         QString currentName = this->entityToPopulate->name();

         for (NamedEntity * matchingEntity = this->findByName(currentName);
            nullptr != matchingEntity;
            matchingEntity = this->findByName(currentName)) {

            qDebug() <<
               Q_FUNC_INFO << "Existing " << this->recordName << "named" << currentName << "was" <<
               ((nullptr == matchingEntity) ? "not" : "") << "found";

            XmlRecord::modifyClashingName(currentName);

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

protected:

   bool instanceNamesAreUnique;

   // It's the job of each subclass to create a suitable object (Hop, Yeast, etc)
   // Once the object is populated, if we give ownership to something else (eg Database class), we should call
   // release(), otherwise, entityToPopulate will get destroyed when the XmlNamedEntityRecord containing it goes out of
   // scope.
   std::unique_ptr<NamedEntity> entityToPopulate;

};

#endif
