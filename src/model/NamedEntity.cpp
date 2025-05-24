/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/NamedEntity.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#include "model/NamedEntity.h"

#include <compare>
#include <typeinfo>
#include <string>

#include <QDebug>
#include <QMetaProperty>

#include "database/ObjectStore.h"
#include "measurement/ConstrainedAmount.h"
#include "model/NamedParameterBundle.h"

#include "model/Boil.h"
#include "model/BoilStep.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/InventoryFermentable.h"
#include "model/InventoryHop.h"
#include "model/InventoryMisc.h"
#include "model/InventoryYeast.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/Recipe.h"
#include "model/RecipeUseOfWater.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_NamedEntity.cpp"
#endif

namespace {
   /**
    * \brief This is a regexp that will match the " (n)" (for n some positive integer) added on the end of a name to
    *        prevent name clashes.  It will also "capture" n to allow you to extract it.
    *
    *        Note that, in the regexp, to match a bracket, we need to escape it, thus "\(" instead of "(".  However, we
    *        must also escape the backslash so that the C++ compiler doesn't think we want a special character (such as
    *        '\n') and barf a "unknown escape sequence" warning at us.  So "\\(" is needed in the string literal here to
    *        pass "\(" to the regexp to match literal "(" (and similarly for close bracket).
    *
    *        The inner brackets for the capturing group do not require any escaping.
    *
    *        A bug in some versions of the code meant that "Foobar ( (1)" was used instead of "Foobar (1)", so we allow
    *        for the erroneous extra bracket etc here so that we may at least partially correct things.
    */
   QRegularExpression const duplicateNameNumberMatcher{" *\\([( ]*([0-9]+)\\)$"};
}

QString NamedEntity::localisedName() { return tr("Named Entity"); }

NamedEntity::NamedEntity(QString t_name) :
   QObject        {nullptr  },
   m_key          {-1       },
   m_name         {t_name   },
   m_deleted      {false    },
   m_beingModified{false    } {

   CONSTRUCTOR_END
   return;
}

//
// Construct from NamedParameterBundle
//
// The "key", "display" and "deleted" properties are optional because they will be set if we're creating from a DB
// record, but not if we're creating from an XML record.
//
// The "name" property has to be optional because not all subclasses have them.  (BrewNote is the subclass without a
// name, and, yes, I know the existence of a NamedEntity without a name calls into question our class naming! :->)
//
NamedEntity::NamedEntity(NamedParameterBundle const & namedParameterBundle) :
   QObject        {nullptr},
   SET_REGULAR_FROM_NPB (m_key    , namedParameterBundle, PropertyNames::NamedEntity::key,       -1       ),
   SET_REGULAR_FROM_NPB (m_name   , namedParameterBundle, PropertyNames::NamedEntity::name,      QString{}),
   SET_REGULAR_FROM_NPB (m_deleted, namedParameterBundle, PropertyNames::NamedEntity::deleted,   false    ),
   m_beingModified{false} {

   CONSTRUCTOR_END
   return;
}

// Strictly speaking a QObject is not allowed to be copied, which would mean that since we do not use any state in the
// QObject from which we inherit, we allow NamedEntity to be copied and just default-initialise the QObject base class
// in the copy.  Hopefully this will never come back to bite us...
NamedEntity::NamedEntity(NamedEntity const & other) :
   QObject        {nullptr        }, // QObject doesn't have a copy constructor, so just make a new one
   m_key          {-1             }, // We don't want to copy the other object's key/ID
   m_name         {other.m_name   },
   m_deleted      {other.m_deleted},
   m_beingModified{false} {

   CONSTRUCTOR_END
   return;
}

void NamedEntity::swap(NamedEntity & other) noexcept {
   // We assert that we only swap two objects of the same class.  We never want to swap a Hop with a Recipe etc.
   Q_ASSERT(typeid(*this) == typeid(other));

   // Assume nothing important to swap in QObject (see comment in model/NamedEntity.h
   //
   // Since we're only using this for assignment operator, which in turn uses copy constructor, we're assuming we are
   // NEVER swapping two objects that both have a valid key.  Assert that here.
   Q_ASSERT(-1 == this->m_key || -1 == other.m_key);
   // Similarly, we assert we are never trying to swap an object that is in the middle of being modified
   Q_ASSERT(!this->m_beingModified);
   Q_ASSERT(!other.m_beingModified);
   // Now do the actual swapping
   std::swap(this->m_name   , other.m_name   );
   std::swap(this->m_deleted, other.m_deleted);
   return;
}

TypeLookup const NamedEntity::typeLookup {
   "NamedEntity",
   {
      // As long as we map each property name to its corresponding member variable, the compiler should be able to work
      // everything else out.  The only exception is that, for enums, we have to pretend they are stored as int, because
      // that's what's going to come out of the Qt property system (and it would significantly complicate other bits of
      // the code to separately register every different enum that we use.)
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::NamedEntity::deleted  , NamedEntity::m_deleted                             ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::NamedEntity::key      , NamedEntity::m_key                                 ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::NamedEntity::name     , NamedEntity::m_name   , NonPhysicalQuantity::String),
   },
   // Parent class lookup - none as we're top of the tree
   {}
};

NamedEntity::~NamedEntity() = default;

// See https://zpz.github.io/blog/overloading-equality-operator-in-cpp-class-hierarchy/ (and cross-references to
// http://www.gotw.ca/publications/mill18.htm) for good discussion on implementation of operator== in a class
// hierarchy.  Our implementation differs slightly for a couple of reasons:
//   - This class is already abstract so it's good to force subclasses to implement isEqualTo() by making it pure
//     virtual here.  (Of course that doesn't help us for sub-sub-classes etc, but it's better than nothing)
//   - We want to do the type comparison first, as this saves us repeating this test in each subclass
//
bool NamedEntity::operator==(NamedEntity const & other) const {
   // The first thing to do is check we are even comparing two objects of the same class.  A Hop is never equal to
   // a Recipe etc.
   if (typeid(*this) != typeid(other)) {
//      qDebug() << Q_FUNC_INFO << "No type id match (" << typeid(*this).name() << "/" << typeid(other).name() << ")";
      return false;
   }

   //
   // For the base class attributes, we deliberately don't compare m_key, table or m_folder.  If we've read in an object
   // from a file and want to  see if it's the same as one in the database, then the DB-related info and folder
   // classification are not a helpful part of that comparison.  Similarly, we do not compare m_display and m_deleted as
   // they are more related to the UI than whether, in essence, two objects are the same.
   //
   if (this->m_name != other.m_name) {
      // Normally leave the next line commented out as it generates a lot of logging
//      qDebug() << Q_FUNC_INFO << "No name match (" << this->m_name << "/" << other.m_name << ")";
      //
      // If the names don't match, let's check it's not for a trivial reason.  Eg, if you have one Hop called
      // "Tettnang" and another called "Tettnang (1)" we wouldn't say they are different just because of the names.
      // So we want to strip off any number in brackets at the ends of the names and then compare again.
      //
      QString names[2] {this->m_name, other.m_name};
      for (auto ii = 0; ii < 2; ++ii) {
         QRegularExpressionMatch match = duplicateNameNumberMatcher.match(names[ii]);
         if (match.hasMatch()) {
            // Regular expression capturing groups are traditionally numbered from 1, with number 0 being reserved for
            // "the implicit capturing group ... capturing the substring matched by the entire pattern".
            auto const positionOfMatch = match.capturedStart(0);
            // Normally leave the next line commented out as it generates a lot of logging
//            qDebug() << Q_FUNC_INFO << names[ii] << "has match" << match << "at" << positionOfMatch;
            // There's some integer in brackets at the end of the name.  Chop it off.
            names[ii].truncate(positionOfMatch);
         }
      }
      // Normally leave the next line commented out as it generates a lot of logging
//      qDebug() << Q_FUNC_INFO << "Adjusted names to " << names[0] << " & " << names[1];
      if (names[0] != names[1]) {
         return false;
      }
   }

   return this->isEqualTo(other);
}

bool NamedEntity::operator!=(NamedEntity const & other) const {
   // Don't reinvent the wheel '!=' should just be the opposite of '=='
   return !(*this == other);
}

std::strong_ordering NamedEntity::operator<=>(NamedEntity const & other) const {
   // The spaceship operator is not defined for two QString objects, but it is defined for a pair of std::u16string,
   // which is close to the same thing (in that QString stores "a string of 16-bit QChars, where each QChar corresponds
   // to one UTF-16 code unit".
#ifdef __clang__
   // As of 2023-09-01 Apple Clang is version 14 and does not support operator <=> on std::u16string
   return this->m_name.data() <=> other.m_name.data();
#else
   return this->m_name.toStdU16String() <=> other.m_name.toStdU16String();
#endif
}

bool NamedEntity::deleted() const {
   return this->m_deleted;
}

void NamedEntity::setDeleted(bool const var) {
   if (this->newValueMatchesExisting(PropertyNames::NamedEntity::deleted, this->m_deleted, var)) {
      return;
   }

   this->m_deleted = var;
   this->propagatePropertyChange(PropertyNames::NamedEntity::deleted);
   return;
}

QString NamedEntity::name() const {
   return this->m_name;
}

QString NamedEntity::strippedName() const {
   QString name {this->m_name};
   QRegularExpressionMatch const match = duplicateNameNumberMatcher.match(name);
   QString matchedValue = match.captured(1);
   if (matchedValue.size() > 0) {
      // There's some integer in brackets at the end of the name, so we chop it off.  Comments in modifyClashingName
      // apply here too.
      name.truncate(match.capturedStart(0));
   }

   return name;
}

void NamedEntity::setName(QString const & var) {
   SET_AND_NOTIFY(PropertyNames::NamedEntity::name, this->m_name, var);
   return;
}

int NamedEntity::key() const {
   return this->m_key;
}

void NamedEntity::setKey(int key) {
   // This will get called by the ObjectStore after inserting something in the DB, so we _don't_ want to call
   // this->propagatePropertyChange, as this would result in some hilarious and pointless circularity where we call
   // down again to the ObjectStore to get it to update the property in the DB.
   this->m_key = key;
   return;
}

bool NamedEntity::subsidiary() const {
   // Per comments in the header file, default implementation is that nothing is a subsidiary.  This behaviour is
   // overridden in Recipe.
   return false;
}

void NamedEntity::setBeingModified(bool set) {
   // The m_beingModified member variable is not stored in the DB, so we don't call this->propagatePropertyChange etc here
   this->m_beingModified = set;
   return;
}

bool NamedEntity::isBeingModified() const {
   return this->m_beingModified;
}

QMetaProperty NamedEntity::metaProperty(char const * const name) const {
   return this->metaObject()->property(this->metaObject()->indexOfProperty(name));
}

void NamedEntity::setEitherOrReqParams(NamedParameterBundle const & namedParameterBundle,
                                       BtStringConst const & quantityParameterName,
                                       BtStringConst const & isFirstUnitParameterName,
                                       BtStringConst const & combinedWithUnitsParameterName,
                                       Measurement::PhysicalQuantity const firstUnitPhysicalQuantity,
                                       double & quantityReturn,
                                       bool & isFirstUnitReturn,
                                       std::optional<bool> const defaultIsFirstUnit) {
   if (namedParameterBundle.contains(quantityParameterName)) {
      quantityReturn    = namedParameterBundle.val<double>(quantityParameterName   );
      if (defaultIsFirstUnit) {
         isFirstUnitReturn = namedParameterBundle.val<bool>(isFirstUnitParameterName, *defaultIsFirstUnit);
      } else {
         isFirstUnitReturn = namedParameterBundle.val<bool>(isFirstUnitParameterName);
      }
   } else {
      auto const combinedWithUnits = namedParameterBundle.val<Measurement::Amount>(combinedWithUnitsParameterName);
      // It is the caller's responsibility to have converted to canonical units -- ie a coding error if this did not
      // happen.  Asserting without the diagnostic info is not much use, so we do the check first, then the assert.
      auto const * suppliedUnit = combinedWithUnits.unit;
      if (!suppliedUnit->isCanonical()) {
         qCritical() <<
            Q_FUNC_INFO << this->name() << "CODING ERROR:" << combinedWithUnitsParameterName << "supplied in" <<
            suppliedUnit << "instead of" << suppliedUnit->getCanonical();
         Q_ASSERT(false);
      } else {
         quantityReturn    = combinedWithUnits.quantity;
         isFirstUnitReturn = combinedWithUnits.unit->getPhysicalQuantity() == firstUnitPhysicalQuantity;
      }
   }
   return;
}

void NamedEntity::setEitherOrOptParams(NamedParameterBundle const & namedParameterBundle,
                                       BtStringConst const & quantityParameterName,
                                       BtStringConst const & isFirstUnitParameterName,
                                       BtStringConst const & combinedWithUnitsParameterName,
                                       Measurement::PhysicalQuantity const firstUnitPhysicalQuantity,
                                       std::optional<double> & quantityReturn,
                                       bool & isFirstUnitReturn) {
   if (namedParameterBundle.contains(quantityParameterName)) {
      quantityReturn    = namedParameterBundle.val<std::optional<double>>(quantityParameterName   );
      isFirstUnitReturn = namedParameterBundle.val<bool                 >(isFirstUnitParameterName);
      return;
   }

   auto const combinedWithUnits = namedParameterBundle.val<std::optional<Measurement::Amount>>(combinedWithUnitsParameterName);
   if (!combinedWithUnits) {
      // Strictly the isFirstUnitReturn is meaningless / ignored in this case, but we use true as the "default" value
      // by convention.  (Other than increased complexity, having it std::optional<bool> wouldn't buy us much.)
      quantityReturn    = std::nullopt;
      isFirstUnitReturn = true;
      return;
   }

   // It is the caller's responsibility to have converted to canonical units -- ie a coding error if this did not
   // happen.  Asserting without the diagnostic info is not much use, so we do the check first, then the assert.
   if (!combinedWithUnits->unit->isCanonical()) {
      qCritical() <<
         Q_FUNC_INFO << this->name() << "CODING ERROR:" << combinedWithUnitsParameterName << "supplied in" <<
         combinedWithUnits->unit << "instead of" << combinedWithUnits->unit->getCanonical();
      Q_ASSERT(false);
   }
   quantityReturn    = combinedWithUnits->quantity;
   isFirstUnitReturn = combinedWithUnits->unit->getPhysicalQuantity() == firstUnitPhysicalQuantity;

   return;
}

void NamedEntity::prepareForPropertyChange(BtStringConst const & propertyName) {
   //
   // At the moment, the only thing we want to do in this pre-change check is to see whether we need to version a
   // Recipe.  Obviously we leave all the details of that to the Recipe-related namespace.
   //
   // Obviously nothing gets versioned if it's not yet in the DB
   //
   auto owningRecipe = this->owningRecipe();
   if (owningRecipe) {
      RecipeHelper::prepareForPropertyChange(*this, propertyName);
   }
   return;
}

void NamedEntity::propagatePropertyChange(BtStringConst const & propertyName, bool notify) const {
   //
   // Normally leave this log statement commented out as otherwise it can generate a lot of lines in the log files
   //
//   qDebug() <<
//      Q_FUNC_INFO << "Property name" << *propertyName << "change on" << this->metaObject()->className() <<
//      ": m_propagationAndSignalsEnabled " << (this->m_propagationAndSignalsEnabled ? "set" : "unset") << ", notify" <<
//      (notify ? "on" : "off");
   if (!this->m_propagationAndSignalsEnabled) {
      return;
   }

   // If we're already stored in the object store, tell it about the property change so that it can write it to the
   // database.  (We don't pass the new value as it will get read out of the object via propertyName.)
   if (this->m_key > 0) {
      this->getObjectStoreTypedInstance().updateProperty(*this, propertyName);
   }

   // Send a signal if needed
   if (notify) {
      this->notifyPropertyChange(propertyName);
   }

   return;
}

void NamedEntity::notifyPropertyChange(BtStringConst const & propertyName) const {
   // It's obviously a coding error to supply a property name that is not registered with Qt as a property of this
   // object
   int idx = this->metaObject()->indexOfProperty(*propertyName);
   Q_ASSERT(idx >= 0);
   QMetaProperty metaProperty = this->metaObject()->property(idx);
   QVariant value = metaProperty.read(this);
   // Normally leave this log statement commented out as otherwise it can generate a lot of lines in the log files
//   qDebug() << Q_FUNC_INFO << this->metaObject()->className() << ":" << propertyName << "=" << value;
   emit this->changed(metaProperty, value);

   return;
}

std::shared_ptr<Recipe> NamedEntity::owningRecipe() const {
   // Default is for NamedEntity not to be owned.
   return nullptr;
}

void NamedEntity::hardDeleteOwnedEntities() {
   // If we are not overridden in the subclass then there is no work to do
   qDebug() << Q_FUNC_INFO << this->metaObject()->className() << "owns no other entities";
   return;
}

void NamedEntity::hardDeleteOrphanedEntities() {
   // If we are not overridden in the subclass then there is no work to do
   qDebug() << Q_FUNC_INFO << this->metaObject()->className() << "leaves no other entities as orphans";
   return;
}

NamedEntity * NamedEntity::ensureExists(BtStringConst const & property) {
   // It's a coding error if this gets called and is not overridden.  (We can't make the function pure virtual because
   // not all child classes need to override it.
   qCritical() <<
      Q_FUNC_INFO << this->metaObject()->className() << "does not know how to ensure property" << property << "exists";
   // Stop here on debug builds
   Q_ASSERT(false);
   return nullptr;
}

void NamedEntity::modifyClashingName(QString & candidateName) {
   //
   // First, see whether there's already a (n) (ie "(1)", "(2)" etc) at the end of the name (with or without
   // space(s) preceding the left bracket.  If so, we want to replace this with " (n+1)".  If not, we try " (1)".
   //
   int duplicateNumber = 1;
   QRegularExpressionMatch const match = duplicateNameNumberMatcher.match(candidateName);
   QString matchedValue = match.captured(1);
   if (matchedValue.size() > 0) {
      // There's already some integer in brackets at the end of the name, extract it, add one, and truncate the name
      // NB: Regular expression capturing groups are traditionally numbered from 1, with number 0 being reserved for
      // "the implicit capturing group ... capturing the substring matched by the entire pattern".
      duplicateNumber = matchedValue.toInt() + 1;
      candidateName.truncate(match.capturedStart(0));
   }
   candidateName += QString(" (%1)").arg(duplicateNumber);
   return;
}

//======================================================================================================================
// NamedEntityModifyingMarker
//======================================================================================================================
NamedEntityModifyingMarker::NamedEntityModifyingMarker(NamedEntity & namedEntity) :
   namedEntity{namedEntity},
   savedModificationState{namedEntity.isBeingModified()} {
   qDebug() <<
      Q_FUNC_INFO << "Marking" << this->namedEntity.metaObject()->className() << "#" << this->namedEntity.key() <<
      "as being modified (" << (this->savedModificationState ? "no change" : "previously was not") << ")";
   this->namedEntity.setBeingModified(true);
   return;
}

NamedEntityModifyingMarker::~NamedEntityModifyingMarker() {
   qDebug() <<
      Q_FUNC_INFO << "Restoring" << this->namedEntity.metaObject()->className() << "#" << this->namedEntity.key() <<
      "\"being modified\" state to" << (this->savedModificationState ? "on" : "off");
   this->namedEntity.setBeingModified(this->savedModificationState);
   return;
}
