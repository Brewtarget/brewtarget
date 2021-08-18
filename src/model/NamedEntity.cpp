/*
 * model/NamedEntity.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
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
#include "model/NamedEntity.h"

#include <typeinfo>

#include <QMetaProperty>

#include "brewtarget.h"
#include "database/ObjectStore.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

NamedEntity::NamedEntity(int key,  bool cache, QString t_name, bool t_display, QString folder) :
   QObject    {nullptr  },
   m_key      {key      },
   parentKey  {-1       },
   m_cacheOnly{cache    },
   m_folder   {folder   },
   m_name     {t_name   },
   m_display  {t_display},
   m_deleted  {false    },
   m_beingModified{false} {
   return;
}

NamedEntity::NamedEntity(NamedEntity const & other) :
   QObject     {nullptr          }, // QObject doesn't have a copy constructor, so just make a new one
   m_key       {-1               }, // We don't want to copy the other object's key/ID
   parentKey   {other.parentKey  },
   m_cacheOnly {other.m_cacheOnly},
   m_folder    {other.m_folder   },
   m_name      {other.m_name     },
   m_display   {other.m_display  },
   m_deleted   {other.m_deleted  },
   m_beingModified{false} {
   return;
}

//
// Construct from NamedParameterBundle
//
// The "key", "display" and "deleted" properties are optional because they will be set if we're creating from a DB
// record, but not if we're creating from an XML record.
//
// The "folder", "name" and "parent" properties have to be optional because not all subclasses have them.  (BrewNote is
// the subclass without a name, and, yes, I know the existence of a NamedEntity without a name calls into question our
// class naming! :->)
//
// .:TBD:. For the moment, parent IDs are actually stored outside the main object table (eg in equipment_children
//         rather than equipment), so this will always set parentKey to -1, but we could envisage changing that in
//         future.
//
NamedEntity::NamedEntity(NamedParameterBundle const & namedParameterBundle) :
   QObject{nullptr},
   m_key      {namedParameterBundle(PropertyNames::NamedEntity::key, -1)          },
   parentKey  {namedParameterBundle(PropertyNames::NamedEntity::parentKey, -1)    },
   m_cacheOnly{false                                                              },
   m_folder   {namedParameterBundle(PropertyNames::NamedEntity::folder, QString{})},
   m_name     {namedParameterBundle(PropertyNames::NamedEntity::name, QString{})  },
   m_display  {namedParameterBundle(PropertyNames::NamedEntity::display, true)    },
   m_deleted  {namedParameterBundle(PropertyNames::NamedEntity::deleted, false)   },
   m_beingModified{false} {
   return;
}

void NamedEntity::makeChild(NamedEntity const & copiedFrom) {
   // It's a coding error if we're not starting out with objects that are copies of each other
   Q_ASSERT(*this == copiedFrom);
   Q_ASSERT(this->parentKey == copiedFrom.parentKey);

   // We also assume that this newly-created object has not yet been put in the database (so we don't need to call
   // down to the ObjectStore to update fields in the DB).
   Q_ASSERT(this->m_key <= 0);

   // By default, we have the same parent as the object from which we were copied.  But, if that means we have no
   // parent, then we take object from which we were copied as our parent, on the assumption that it is the master
   // version of this Hop/Fermentable/etc.
   if (this->parentKey <= 0) {
      this->parentKey = copiedFrom.m_key;
   }

   //
   // A _child_ of a Hop (or Style/Fermentable/etc) is "an instance of use of" the parent Hop (etc).  Thus we don't want
   // it to show up in the list of all Hops (etc).
   //
   // .:TBD:. It would be nicer to do away with m_display and have NamedEntity::display() do some logic (eg don't
   // display if deleted or has a parent) that might be overridden by Recipe to add the extra logic around ancestors.
   //
   this->m_display = false;

   // So, now, we should have some plausible parent ID, and in particular we should not be our own parent!
   Q_ASSERT(this->parentKey != this->m_key);
   return;
}

QRegExp const & NamedEntity::getDuplicateNameNumberMatcher() {
   //
   // Note that, in the regexp, to match a bracket, we need to escape it, thus "\(" instead of "(".  However, we
   // must also escape the backslash so that the C++ compiler doesn't think we want a special character (such as
   // '\n') and barf a "unknown escape sequence" warning at us.  So "\\(" is needed in the string literal here to
   // pass "\(" to the regexp to match literal "(" (and similarly for close bracket).
   //
   static QRegExp const duplicateNameNumberMatcher{" *\\(([0-9]+)\\)$"};
   return duplicateNameNumberMatcher;
}


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
   // For the base class attributes, we deliberately don't compare m_key, parentKey, table or m_folder.  If we've read
   // in an object from a file and want to  see if it's the same as one in the database, then the DB-related info and
   // folder classification are not a helpful part of that comparison.  Similarly, we do not compare _display and
   // m_deleted as they are more related to the UI than whether, in essence, two objects are the same.
   //
   if (this->m_name != other.m_name) {
//      qDebug() << Q_FUNC_INFO << "No name match (" << this->m_name << "/" << other.m_name << ")";
      //
      // If the names don't match, let's check it's not for a trivial reason.  Eg, if you have one Hop called
      // "Tettnang" and another called "Tettnang (1)" we wouldn't say they are different just because of the names.
      // So we want to strip off any number in brackets at the ends of the names and then compare again.
      //
      QRegExp const & duplicateNameNumberMatcher = NamedEntity::getDuplicateNameNumberMatcher();
      QString names[2] {this->m_name, other.m_name};
      for (auto ii = 0; ii < 2; ++ii) {
         int positionOfMatch = duplicateNameNumberMatcher.indexIn(names[ii]);
         if (positionOfMatch > -1) {
            // There's some integer in brackets at the end of the name.  Chop it off.
            names[ii].truncate(positionOfMatch);
         }
      }
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

bool NamedEntity::operator<(const NamedEntity & other) const { return (this->m_name < other.m_name); }
bool NamedEntity::operator>(const NamedEntity & other) const { return (this->m_name > other.m_name); }

bool NamedEntity::deleted() const {
   return this->m_deleted;
}

bool NamedEntity::display() const {
   return this->m_display;
}

// Sigh. New databases, more complexity
void NamedEntity::setDeleted(bool const var) {
   if (this->newValueMatchesExisting(PropertyNames::NamedEntity::deleted, this->m_deleted, var)) {
      return;
   }

   this->m_deleted = var;
   this->propagatePropertyChange(PropertyNames::NamedEntity::deleted);
   return;
}

void NamedEntity::setDisplay(bool var) {
   if (this->newValueMatchesExisting(PropertyNames::NamedEntity::display, this->m_display, var)) {
      return;
   }
   this->m_display = var;
   this->propagatePropertyChange(PropertyNames::NamedEntity::display);
   return;
}

QString NamedEntity::folder() const {
   return this->m_folder;
}

void NamedEntity::setFolder(QString const & var) {
   if (this->newValueMatchesExisting(PropertyNames::NamedEntity::folder, this->m_folder, var)) {
      return;
   }
   this->m_folder = var;
   this->propagatePropertyChange(PropertyNames::NamedEntity::folder);
   return;
}

QString NamedEntity::name() const {
   return this->m_name;
}

void NamedEntity::setName(QString const & var) {
   this->setAndNotify(PropertyNames::NamedEntity::name, this->m_name, var);
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

int NamedEntity::getParentKey() const {
   return this->parentKey;
}

bool NamedEntity::cacheOnly() const {
   return this->m_cacheOnly;
}

void NamedEntity::setParentKey(int parentKey) {
   this->parentKey = parentKey;

   //
   // If the data is obviously messed up then let's at least log it.  (It doesn't necessarily mean there is a bug in
   // the current version of the code.  It could be the result of a bug in an earlier version.  If so, a manual data
   // fix is needed in the database.)
   //
   // Something should not be its own parent for instance
   //
   if (this->parentKey == this->m_key) {
      qCritical() << Q_FUNC_INFO << this->metaObject()->className() << "#" << this->m_key << "is its own parent!";
   }

   return;
}

void NamedEntity::setCacheOnly(bool cache) {
   // The m_cacheOnly member variable is not stored in the DB, so we don't call this->propagatePropertyChange etc here
   this->m_cacheOnly = cache;
   return;
}

void NamedEntity::setBeingModified(bool set) {
   this->m_beingModified = set;
   return;
}

bool NamedEntity::isBeingModified() const {
   return this->m_beingModified;
}

QVector<int> NamedEntity::getParentAndChildrenIds() const {
   QVector<int> results;
   NamedEntity const * parent = this->getParent();
   if (nullptr == parent) {
      parent = this;
   }

   // We are assuming that grandparents do not exist - ie that it's a coding error if they do
   // We want more than just an assert in that case as debugging would be a lot harder without knowing which
   // NamedEntity has the unexpected data.
   if (parent->parentKey > 0) {
      qCritical() <<
         Q_FUNC_INFO << this->metaObject()->className() << "#" << this->m_key << "has parent #" << this->parentKey <<
         "with parent #" << parent->parentKey;
      Q_ASSERT(false);
   }

   // We've got the parent ingredient...
   results.append(parent->m_key);

   // ...now find all the children, ie all the other ingredients of this type whose parent is the ingredient we just
   // found
   QList<std::shared_ptr<QObject> > children = this->getObjectStoreTypedInstance().findAllMatching(
      [parent](std::shared_ptr<QObject> obj) { return std::static_pointer_cast<NamedEntity>(obj)->getParentKey() == parent->key(); }
   );
   for (auto child : children) {
      results.append(std::static_pointer_cast<NamedEntity>(child)->key());
   }
   return results;
}

QMetaProperty NamedEntity::metaProperty(char const * const name) const {
   return this->metaObject()->property(this->metaObject()->indexOfProperty(name));
}

void NamedEntity::prepareForPropertyChange(BtStringConst const & propertyName) {
   //
   // At the moment, the only thing we want to do in this pre-change check is to see whether we need to version a
   // Recipe.  Obviously we leave all the details of that to the Recipe-related namespace.
   //
   RecipeHelper::prepareForPropertyChange(*this, propertyName);
   return;
}

void NamedEntity::propagatePropertyChange(BtStringConst const & propertyName, bool notify) const {
   // If we're in "cache only" mode, there's nothing to do
   if (this->m_cacheOnly) {
      return;
   }

   // If we're already stored in the object store, tell it about the property change so that it can write it to the
   // database.  (We don't pass the new value as it will get read out of the object via propertyName.)
   if (this->m_key > 0) {
      this->getObjectStoreTypedInstance().updateProperty(*this, propertyName);
   }

   // Send a signal if needed
   if (notify) {
      // It's obviously a coding error to supply a property name that is not registered with Qt as a property of this
      // object
      int idx = this->metaObject()->indexOfProperty(*propertyName);
      Q_ASSERT(idx >= 0);
      QMetaProperty metaProperty = this->metaObject()->property(idx);
      QVariant value = metaProperty.read(this);
      emit this->changed(metaProperty, value);
   }

   return;
}

NamedEntity * NamedEntity::getParent() const {
   if (this->parentKey <= 0) {
      return nullptr;
   }

   return static_cast<NamedEntity *>(this->getObjectStoreTypedInstance().getById(this->parentKey).get());
}

void NamedEntity::setParent(NamedEntity const & parentNamedEntity) {
   this->setAndNotify(PropertyNames::NamedEntity::parentKey, this->parentKey, parentNamedEntity.m_key);
   return;
}

void NamedEntity::hardDeleteOwnedEntities() {
   // If we are not overridden in the subclass then there is no work to do
   qDebug() << Q_FUNC_INFO << this->metaObject()->className() << "owns no other entities";
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
