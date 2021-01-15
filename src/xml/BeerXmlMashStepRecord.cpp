/*
 * xml/BeerXmlMashStepRecord.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlMashStepRecord.h"
/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <MASH_STEP>...</MASH_STEP> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::EnumLookupMap const MASH_STEP_TYPE_MAPPER {
   {"Infusion",     MashStep::Infusion},
   {"Temperature",  MashStep::Temperature},
   {"Decoction",    MashStep::Decoction}
   // Inside Brewtarget we also have MashStep::flySparge and MashStep::batchSparge which are not mentioned in the
   // BeerXML 1.0 Standard.  They get treated as "Infusion" when we write to BeerXML
};

static QVector<XmlRecord::Field> const MASH_STEP_RECORD_FIELDS {
   // Type              XPath                 Q_PROPERTY           Enum Mapper
   {XmlRecord::String,  "NAME",               "name",              nullptr},
   {XmlRecord::UInt,    "VERSION",            nullptr,             nullptr},
   {XmlRecord::Enum,    "TYPE",               "type",              &MASH_STEP_TYPE_MAPPER},
   {XmlRecord::Double,  "INFUSE_AMOUNT",      "infuseAmount_l",    nullptr}, // Should not be supplied if TYPE is "Decoction"
   {XmlRecord::Double,  "STEP_TEMP",          "stepTemp_c",        nullptr},
   {XmlRecord::Double,  "STEP_TIME",          "stepTime_min",      nullptr},
   {XmlRecord::Double,  "RAMP_TIME",          "rampTime_min",      nullptr},
   {XmlRecord::Double,  "END_TEMP",           "endTemp_c",         nullptr},
   {XmlRecord::String,  "DESCRIPTION",        nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "WATER_GRAIN_RATIO",  nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "DECOCTION_AMT",      nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "INFUSE_TEMP",        nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_STEP_TEMP",  nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_INFUSE_AMT", nullptr,             nullptr}, // Extension tag
   {XmlRecord::Double,  "DECOCTION_AMOUNT",   "decoctionAmount_l", nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
};
*/
/**
 * \brief BeerXmlSimpleRecord<MashStep> specialisation for reading <MASH_STEP>...</MASH_STEP> BeerXML records
 * into \b MashStep objects.  Note that this class is further specialised by \b BeerXmlMashStepRecord
 *
template<>
BeerXmlSimpleRecord<MashStep>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "MASH_STEP",
                        XmlNamedEntityRecord::InstancesWithDuplicateNamesOk,
                        MASH_STEP_RECORD_FIELDS,
                        new MashStep{}} {
   return;
}

BeerXmlMashStepRecord::BeerXmlMashStepRecord(XmlCoding const & xmlCoding) :
   BeerXmlSimpleRecord<MashStep>{xmlCoding} {
   return;
}
*/

bool BeerXmlMashStepRecord::load(xalanc::DOMSupport & domSupport,
                                 xalanc::XalanNode * rootNodeOfRecord,
                                 QTextStream & userMessage,
                                 std::shared_ptr< QHash<QString, QVariant> > fieldsRead) {
   // We'll want to use the fieldsRead hashmap (to see what the base class method read in) so, if the caller didn't
   // supply one, make a new one for the life of this function.
   if (nullptr == fieldsRead.get()) {
      fieldsRead.reset(new QHash<QString, QVariant>);
   }
   //
   // Get the base class to do the heavy lifting of reading in all the data...
   //
   if (!XmlNamedEntityRecord::load(domSupport, rootNodeOfRecord, userMessage, fieldsRead)) {
      return false;
   }
   //
   // ...then, if that worked, we have an extra thing to check.
   //
   // Per the notes in beerxml/v1/BeerXml.xsd, one thing we can't currently check via XML XSD is the restriction that
   // INFUSE_AMOUNT is not supposed to be specified if TYPE is "Decoction".  We can check it here in code though.
   if (fieldsRead->contains("INFUSE_AMOUNT") &&
       fieldsRead->contains("TYPE") &&
       MashStep::Decoction == fieldsRead->value("TYPE").toInt()) {
      //
      // We can work around the bad input data and it's not a big enough deal to flag up to the user on the screen, so
      // just log the correction we're making.
      //
      qWarning() <<
        Q_FUNC_INFO << "A MASH_STEP record had TYPE = Decoction but also specified INFUSE_AMOUNT =" <<
        fieldsRead->value("INFUSE_AMOUNT").toDouble() << ".  Ignoring supplied INFUSE_AMOUNT and using 0.0";
      MashStep * mashStep = dynamic_cast<MashStep *>(this->entityToPopulate.get());
      mashStep->setInfuseAmount_l(0.0);
   }

   return true;
}

//TODO>>>NEED TO CHANGE THIS TO NOT TAKE MASH AS PARAM
bool BeerXmlMashStepRecord::normaliseAndStoreInDb(Mash * mash, QTextStream & userMessage) {
   Q_ASSERT(nullptr != mash);

   //
   // Despite what one might infer from the constructor signatures, every MashStep is, in theory, supposed to have a
   // name.  (It does after all inherit from NamedEntity.)  However, at least some versions of Brewtarget have allowed
   // creation and export of recipes with one or more unnamed MashSteps.  Moreover, the BeerXML 1.0 standard only says
   // the NAME tag has to be present, not that it can't be empty.  (We might wish that the standard had been more
   // explicit about such things, but it is what it is.)
   //
   // MashStep names of are certainly not expected to be globally unique, and are usually simply descriptive of what
   // the step is (eg "Mash In", "Mash Out", "Conversion", "Final Batch Sparge").
   //
   // Therefore, if only a blank name was supplied in the XML, we will amend this to "Unnamed Mash Step" (or whatever
   // that translates to in your language).
   //
   MashStep * mashStep = dynamic_cast<MashStep *>(this->entityToPopulate.get());
   if (mashStep->name() == "") {
      qWarning() << Q_FUNC_INFO << "Setting default name on unnamed MASH_STEP record";
      // Note that tr() is a static member function of QObject.  We do not inherit from QObject, but MashStep does
      // (via NamedEntity).
      mashStep->setName(QString(MashStep::tr("Unnamed Mash Step")));
   }

   try {
      // This call actually stores the MashStep in the DB as well as associating it with the Mash.  Hence we _don't_
      // call the base class method (XmlNamedEntityRecord::normaliseAndStoreInDb).
      mash->addMashStep(mashStep);
   } catch (QString ex) {
      // Error will already have been logged by caller
      userMessage << "Database error: " << ex;
      return false;
   }

   this->entityToPopulate.release();

   return true;
}
