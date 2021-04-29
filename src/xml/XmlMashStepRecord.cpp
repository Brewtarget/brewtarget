/*
 * xml/XmlMashStepRecord.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/XmlMashStepRecord.h"
#include "model/Mash.h"

XmlRecord::ProcessingResult XmlMashStepRecord::normaliseAndStoreInDb(NamedEntity * containingEntity,
                                                                     QTextStream & userMessage,
                                                                     XmlRecordCount & stats) {
   // It's a coding error if either there's no containing entity or it's not a Mash.  Both conditions should have been
   // enforced by XSD parsing.  Thus static_cast should be safe.
   Mash * mash = static_cast<Mash *>(containingEntity);
   Q_ASSERT(nullptr != mash);

   //
   // There are a couple of extra things to check/fix on MashStep objects we are reading in.
   //
   // Per the notes in beerxml/v1/BeerXml.xsd, one thing we can't currently check via XML XSD is the restriction that
   // it doesn't make sense to have infuseAmount_l set to anything other than 0 (the default) if thy type of step is a
   // Decoction (which doesn't involve infusing, ie adding, water).
   //
   // We _could_ just barf here and refuse to continue the import, but there is a sane and simple "fix" for the bad
   // data, so let's just correct it and log a warning about what we did.
   // INFUSE_AMOUNT is not supposed to be specified if TYPE is "Decoction".  We can check it here in code though.
   //
   MashStep * mashStep = static_cast<MashStep *>(this->namedEntity);
   if (mashStep->type() == MashStep::Decoction && mashStep->infuseAmount_l() != 0) {
      qWarning() <<
        Q_FUNC_INFO << "Read in a decoction Mash Step with a non-zero infusion volume (" <<
        mashStep->infuseAmount_l() << ").  Ignoring supplied infusion volume and using 0.0";
      mashStep->setInfuseAmount_l(0.0);
   }

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
   if (0 == mashStep->name().length()) {
      qWarning() << Q_FUNC_INFO << "Setting default name on unnamed MASH_STEP record";
      // Note that tr() is a static member function of QObject.  We do not inherit from QObject, but MashStep does
      // (via NamedEntity).
      mashStep->setName(QString(MashStep::tr("Unnamed Mash Step")));
   }

   //
   // Now we've fixed up any bad data, we can store the MashStep.
   //
   // Note that we do not update the XmlRecordCount object (stats) because MashStep objects do not have a freestanding
   // existence outside of their owning Mash object.
   //
   try {
      //
      // This call actually stores the MashStep in the DB as well as associating it with the Mash.  Hence we _don't_
      // call the base class method (XmlRecord::normaliseAndStoreInDb) because we are replacing rather than extending
      // its functionality.
      //
      mash->addMashStep(mashStep);
   } catch (QString ex) {
      // Error will already have been logged by caller
      userMessage << "Database error: " << ex;
      return XmlRecord::Failed;
   }

   //
   // DB storing succeeded, so we no longer own the MashStep object
   //
   this->namedEntityRaiiContainer.release();

   //
   // We deliberately don't update the stats
   //
   // This is the point where we would call XmlRecord::normaliseAndStoreChildRecordsInDb(), but that would be a no-op
   // because MashStep records are not expected to have any children, so we don't bother.
   //
   return XmlRecord::Succeeded;
}
