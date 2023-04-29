/*
 * model/NamedEntity.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Jeff Bailey <skydvr38@verizon.net>
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
#ifndef MODEL_NAMEDENTITY_H
#define MODEL_NAMEDENTITY_H
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QMetaProperty>
#include <QObject>
#include <QVariant>

#include "utils/BtStringConst.h"
#include "utils/TypeLookup.h"

class NamedParameterBundle;
class ObjectStore;
class Recipe;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// Make this class's property names available via constants in sub-namespace of PropertyNames
// One advantage of using these constants is you get compile-time checking for typos etc
//
// Note that, because we are both declaring and defining these in the header file, I don't think we can guarantee on
// every platform there is always exactly one instance of each property name.  So, whilst it's always valid to compare
// the values of two property names, we cannot _guarantee_ that two identical property names always have the same
// address in memory.  In other words, _don't_ do `if (&somePropName == &PropertyNames::NamedEntity::Folder) ...`.
//
// I did also think about creating a macro that would combine this with Q_PROPERTY, but I didn't see an elegant way to
// do it given that these need to be outside the class and Q_PROPERTY needs to be inside it.
//
// IMPORTANT: These property names are unique within a class, but they are not globally unique, so we have to be a bit
//            careful about how we use them in look-ups.
//
#define AddPropertyName(property) namespace PropertyNames::NamedEntity { BtStringConst const property{#property}; }
AddPropertyName(deleted)
AddPropertyName(display)
AddPropertyName(folder)
AddPropertyName(key)
AddPropertyName(name)
AddPropertyName(parentKey)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/*!
 * \class NamedEntity
 *
 * \brief The base class for our substantive storable items.  There are really two sorts of storable items: ones that
 *        are freestanding and ones that are owned by other storable items.  Eg, a Hop exists in its own right and may
 *        or may not be used in one or more Recipes, but a MashStep only exists as part of a single Mash.
 *           \b BrewNote is owned by its \b Recipe
 *           \b Equipment
 *           \b Fermentable
 *           \b Hop
 *           \b Instruction is owned by its \b Recipe
 *           \b Mash
 *           \b MashStep is owned by its \b Mash
 *           \b Misc
 *           \b Recipe
 *           \b Salt
 *           \b Style
 *           \b Water
 *           \b Yeast
 *
 *        I know \b NamedEntity isn't the snappiest name, but it's the best we've come up with so far.  If you look at
 *        older versions of the code, you'll see that this class has previously been called \b Ingredient and
 *        \b BeerXMLElement.  The current name tries to best reflect what the class represents.  Although some of this
 *        class's subclasses (eg \b Hop, \b Fermentable, \b Yeast) are ingredients in the normal sense of the word,
 *        others (eg \b Instruction, \b Equipment, \b Style, \b Mash) are not really.  Equally, the fact that derived
 *        classes can be instantiated from BeerXML is not their defining characteristic.
 *
 *        NOTE: One of the things that can be confusing about our class and DB structure is that we are often
 *        doubling-up two different concepts: a global "variety" record/object and a recipe-specific "use of"
 *        record/object.  Eg, there might be one global "variety" record/object for Fuggle hops, with information about
 *        origin, average alpha acid etc.  Then, each time this type of hop is used in a recipe, there will be a
 *        related record with information about quantity used, actual alpha acid, etc PLUS a COPY of all the information
 *        in the variety record.  The "use of" and "variety" records are stored in the same DB table and the
 *        relationship between them is tracked via parent_id/child_id - where "variety" is the parent and "use of" is
 *        the child.
 *           This structure exists for historical reasons because the code was originally modelled on the BeerXML data
 *        structure (which doesn't really distinguish between "variety" and "use of") and, in the beginning, did not use
 *        a relational database (operating directly on BeerXML files instead).
 *           If we were starting today from a blank sheet of paper, you might, quite reasonably, argue that we should
 *        have separate DB tables and classes for "variety" and "use of" -- eg HopVariety and HopUse (or HopAddition).
 *        However, given where we actually are at the moment, it would be a very considerable amount of work to get to
 *        such a structure -- and it feels like we have a lot more important issues to which we should devote our
 *        efforts.
 *           Moreover, such a would actually remove some potentially valuable flexibility from the code.  It is open to
 *        debate exactly which fields belong in the "variety" record, which ones belong in the "use of" record, and
 *        which ones should be should be in both, and we might want to leave it open (within reason) for different users
 *        to do different things.  Eg, for hops, BeerJSON requires name and alpha acid in both types of records, allows
 *        (but does not require) producer, product ID, origin, year, form (ie leaf/pellet/etc) and beta acid in either
 *        type, but only allows oil content and inventory information in the "variety" record.  This isn't wrong per se,
 *        but it means that, if you want separate BeerJSON inventory records for Fuggle 2021 harvest and Fuggle 2022
 *        harvest, then you need two separate Fuggle variety records, which might not be what you want.  (This also then
 *        makes you think there should be three types of Hop record, but I'm not going to go there!)
 *           So, for the moment at least, we (mostly) retain the idea that a single class/table for both "variety" and
 *        "use of" objects/records.  You just have to be mindful of this when looking at the code and the DB -- eg
 *        the amount field of a Hop can be either inventory or how much to add to a recipe, depending on whether it's a
 *        "variety" or "use of" record.
 *        .:TODO:. It would be good to make explicit which member variables (and their getters/setters) are valid ONLY
 *        for "use of".
 *
 *        NOTE: Although we can template individual member functions, we cannot make this a template class (eg to use
 *        https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) because the Qt Meta-Object Compiler (moc)
 *        cannot handle templates, and we want to be able to use the Qt Property system as well as signals and slots.
 *
 *        NOTE: Because NamedEntity inherits from QObject, no extra work is required to store pointers to NamedEntity
 *        objects inside QVariant.  We also get to have Qt properties for free.  Because we do not use any state in the
 *        QObject from which we inherit, we can get away with not trying to move/copy such state in our copy
 *        constructor, assignment operator, etc.  This is good because there isn't a handy way to do such moves or
 *        copies.
 *
 *        NOTE: When modifying or extending subclasses of \c NamedEntity, there are mostly plenty of examples to guide
 *        you, but a couple of things are worth knowing in advance:
 *          - Attributes that we want to be able to store, eg in the DB and/or in an XML or JSON file, need to be Qt
 *            properties.  There are then mappings in xml/BeerXml.cpp, json/BeerJson.cpp and
 *            database/ObjectStoreTyped.cpp that determine how these properties are read/written.
 *          - Our data structures have, for obvious reasons, been quite heavily influenced by BeerXML and BeerJSON.  The
 *            latter is a much larger data model than the former, so there are a few minor contortions in the BeerXML
 *            mappings (as you'll see from the comments in xml/BeerXml.cpp).
 *          - Because Qt properties don't really handle Null (or std::optional), there are a few places in the code
 *            where we say "we'll treat this value as null".  Eg, for any int that's a DB key, -1 means it's null.
 *          - Mostly the order of enum values should not matter, eg for serialisation where we generally to convert to
 *            strings.  \b However, the .ui files and the .conf file still contain a lot of instances where the order
 *            and/or the int value of an enum do matter, so it's best to avoid changing this, for now at least.
 *
 * .:TODO:. It would be nice to have a canonical serialisation of enums
 */
class NamedEntity : public QObject {
   Q_OBJECT
   Q_CLASSINFO("version","1")

public:

   /**
    * \brief Type lookup info for this class.  Note this is intentionally static, public and const.  Subclasses need to
    *        override this member with one that chains to it.  (See \c TypeLookup constructor for more info.)
    *
    *        Note that this is a static member variable and is \b not intended to do run-time validation (eg to say
    *        whether the object is in a state where the property is allowed to be null).  It just allows us to tell
    *        (amongst other things) whether, in principle, a given field can ever be null.
    *
    *        Why is this a static member variable and not a virtual function?  It's because we need to be able to access
    *        it \b before we have created the object.  Eg, if we are reading a \c Fermentable from the DB, we first read
    *        all the fields and construct a \c NamedParameterBundle, and then use that \c NamedParameterBundle to
    *        construct the \c Fermentable.
    */
   static TypeLookup const typeLookup;

   NamedEntity(QString t_name, bool t_display = false, QString folder = QString());
   NamedEntity(NamedEntity const & other);

   /**
    * \brief Note that, if you want a \b child of a \c NamedEntity (to add to a \c Recipe), you should call
    *        \c makeChild() on the copy, which will do the right things about parentage and inventory.
    */
   NamedEntity(NamedParameterBundle const & namedParameterBundle);

protected:
   /**
    * \brief Swap the contents of two NamedEntity objects - which provides an exception-safe way of implementing
    *        operator=
    */
   void swap(NamedEntity & other) noexcept;

public:
   // Our destructor needs to be virtual because we sometimes point to an instance of a derived class through a pointer
   // to this class -- ie NamedEntity * namedEntity = new Hop() and suchlike.  We do already get a virtual destructor by
   // virtue of inheriting from QObject, but this declaration does no harm.
   virtual ~NamedEntity();

   /**
    * \brief Although we might need to implement assignment operator for some of its derived classes (eg Water),
    *        NamedEntity itself is an abstract class, so there isn't a meaningful implementation, and we don't ever
    *        want the compiler to try to create one (per the "Rule of Three").
    */
   NamedEntity & operator=(NamedEntity const &) = delete;

   /**
    * \brief Don't think we want move assignment either.
    */
   NamedEntity & operator=(NamedEntity &&) = delete;

   /**
    * \brief For the moment, we don't need a move constructor, so make sure the compiler doesn't generate one for us (as
    *        it would likely be wrong).
    */
   NamedEntity(NamedEntity &&) = delete;

   /**
    * \brief Turns a straight copy of an object into a "child" copy that can be used in a Recipe.  (A child copy is
    *        essentially an "instance of use of".)
    *
    *        NB: This function must be called \b before the object is added to its \c ObjectStore
    *
    * \param copiedFrom The object from which this one was copied
    */
   virtual void makeChild(NamedEntity const & copiedFrom);

   /**
    * \brief This generic version of operator== should work for subclasses provided they correctly _override_ (NB not
    *        overload) the protected virtual isEqualTo() function.
    */
   bool operator==(NamedEntity const & other) const;

   /**
    * \brief This generic version of operator!= should work for subclasses provided they correctly _override_ (NB not
    *        overload) the protected virtual isEqualTo() function.
    */
   bool operator!=(NamedEntity const & other) const;

   //
   // TODO We should replace the following with the spaceship operator once compiler support for C++20 is more widespread
   //
   /**
    * \brief As you might expect, this ensures we order \b NamedEntity objects by name
    */
   bool operator<(NamedEntity const & other) const;
   bool operator>(NamedEntity const & other) const;

   // Everything that inherits from BeerXML has a name, delete, display and a folder
   Q_PROPERTY(QString name   READ name WRITE setName )
   Q_PROPERTY(bool deleted   READ deleted WRITE setDeleted )
   Q_PROPERTY(bool display   READ display WRITE setDisplay )
   Q_PROPERTY(QString folder READ folder WRITE setFolder )

   Q_PROPERTY(int key READ key WRITE setKey )
   Q_PROPERTY(int parentKey READ getParentKey WRITE setParentKey )

   //! \returns our key in the table we are stored in.
   int key() const;
   //! Access to the name attribute.
   QString name() const;
   //! Convenience method to determine if we are deleted or displayed
   bool deleted() const;
   bool display() const;
   //! Access to the folder attribute.
   QString folder() const;

   /**
    * \brief Returns a regexp that will match the " (n)" (for n some positive integer) added on the end of a name to
    *        prevent name clashes.  It will also "capture" n to allow you to extract it.
    */
   static QRegExp const & getDuplicateNameNumberMatcher();

   //! And ways to set those flags
   void setDeleted(bool const var);
   void setDisplay(bool const var);
   //! and a way to set the folder
   virtual void setFolder(QString const & var);

   //!
   void setName(QString const & var);

   /**
    * \brief This sets or unsets the "being modified" flag on the object.  Callers should preferably access this via
    *        the \c NamedEntityModifyingMarker RAII wrapper.
    *
    *        This is a transient attribute (not stored in the DB) that it supposed to be used to mark when we are in the
    *        process of modifying or copying the object and therefore do not want to heed signals that would trigger
    *        further modifications or copies.  It's otherwise too easy to generate endless loops via a sequence of well-
    *        intentioned signals.  (Finding and eliminating all these potential loops is non-trivial.)
    */
   void setBeingModified(bool set);
   bool isBeingModified() const;

   /**
    * \brief Set the ID (aka key) by which this object is uniquely identified in its DB table
    *
    *        This is virtual because, in some cases, subclasses are going to want to do additional work here
    */
   virtual void setKey(int key);

   int getParentKey() const;
   void setParentKey(int parentKey);

   /**
    * \brief Get the IDs of this object's parent, children and siblings (plus the ID of the object itself).
    *        A child object is just a copy of the parent that's being used in a Recipe.  Not all NamedEntity subclasses
    *        have children, just Equipment, Fermentable, Hop, Misc and Yeast.
    */
   QVector<int> getParentAndChildrenIds() const;

   //! Convenience method to get a meta property by name.
   QMetaProperty metaProperty(char const * const name) const;

   /**
    * \brief Subclasses need to override this to return the Recipe, if any, to which this object belongs.
    *
    * \return \c nullptr if this object is not, and does not belong to, any Recipe
    */
   virtual Recipe * getOwningRecipe() = 0;

   /*!
    * \brief Some entities (eg Fermentable, Hop) get copied when added to a recipe, but others (eg Instruction) don't.
    *        For those that do, we think of the copy as being a child of the original NamedEntity.  This function allows
    *        us to access that parent.
    * \return Pointer to the parent NamedEntity from which this one was originally copied, or null if no such parent exists.
    */
   NamedEntity * getParent() const;

   void setParent(NamedEntity const & parentNamedEntity);

   /**
    * \brief If we are _really_ deleting (rather than just marking deleted) an entity that owns other entities (eg a
    *        Mash owns its MashSteps) then we need to delete those owned entities immediately beforehand.
    *
    *        By default this function does nothing.  Subclasses override it if needed.
    */
   virtual void hardDeleteOwnedEntities();

   /**
    * \brief Similar to \c hardDeleteOwnedEntities but for cases where the related entities need to be deleted
    *        immediately \b after rather than immediately \b before the entity to which they are related.  (Which is
    *        required typically depends on the order of the underlying foreign key relationships in the database.)
    *
    *        By default this function does nothing.  Subclasses override it if needed.
    */
   virtual void hardDeleteOrphanedEntities();

signals:
   /*!
    * \brief Passes the meta property that has changed about this object.
    *
    * NOTE: When subclassing, be \em extra careful not to create a member function with the same signature.
    *       Otherwise, everything will silently break.
    */
   void changed(QMetaProperty, QVariant value = QVariant()) const;
   void changedFolder(QString);
   void changedName(QString);

protected:
   //! The key of this entity in its table.
   int m_key;
   // This is <=0 if there is no parent (or parent is not yet known)
   int parentKey;

   /**
    * \brief Subclasses need to overload (NB not override) this function to do the substantive work for operator==.
    *        By the time this function is called on a subclass, we will already have established that the two objects
    *        being compared are of the same class (eg we are not comparing a Hop with a Yeast) and that the names match,
    *        so subclasses do not need to repeat these tests.
    *
    *        We do not currently anticipate sub-sub-classes of \b NamedEntity but if one ever were created, it should
    *        call its parent's implementation of this function before doing its own class-specific tests.
    * \return \b true if this object is, in all the ways that matter, equal to \b other
    */
   virtual bool isEqualTo(NamedEntity const & other) const = 0;

   /**
    * \brief Subclasses need to override this function to return the appropriate instance of \c ObjectStoreTyped.
    *        This allows us in this base class to access \c ObjectStoreTyped<Hop> for \c Hop,
    *        \c ObjectStoreTyped<Fermentable> for \c Fermentable, etc.
    */
   virtual ObjectStore & getObjectStoreTypedInstance() const = 0;

   /**
    * \brief Used by setters to force a value not to be below a certain amount
    *
    * \param value the value to check
    * \param name the name of the value being set, so we can log a warning about it being out of range
    * \param minValue what value must not be below -- 0 if not specified
    * \param defaultValue what to use instead of value if it is below minValue -- 0 if not specified
    *
    * \return What to use for the value (ie \c value or \c defaultValue, depending on whether \c value is in range
    */
   template<typename T> T enforceMin(T const value,
                                     char const * const name,
                                     T const minValue = 0,
                                     T const defaultValue = 0) {
      if (value < minValue) {
         qWarning() <<
            Q_FUNC_INFO << this->metaObject()->className() << ":" << name << "value" << value <<
            "below min of" << minValue << "so using" << defaultValue << "instead";
         return defaultValue;
      }
      return value;
   }

   /**
    * \brief Partial specialisation for optional value
    */
   template<typename T> std::optional<T> enforceMin(std::optional<T> const value,
                                                    char const * const name,
                                                    T const minValue = 0,
                                                    T const defaultValue = 0) {
      if (value) {
         return this->enforceMin(*value, name, minValue, defaultValue);
      }
      return value;
   }

   /**
    * \brief Like \c enforceMin, but for a range
    *
    *        (We often want \c minValue = 0 and \c maxValue = 100, but I don't default them here as I want it to be
    *         hard to get \c enforceMin and \c enforceMinAndMax mixed up.)
    */
   template<typename T> T enforceMinAndMax(T const value,
                                           char const * const name,
                                           T const minValue,
                                           T const maxValue,
                                           T const defaultValue = 0) {
      if (value < minValue || value > maxValue) {
         qWarning() <<
            Q_FUNC_INFO << this->metaObject()->className() << ":" << name << "value" << value <<
            "outside range min of" << minValue << "-" << maxValue << "so using" << defaultValue << "instead";
         return defaultValue;
      }
      return value;
   }

   /**
    * \brief Partial specialisation for optional value
    */
   template<typename T> std::optional<T> enforceMinAndMax(std::optional<T> const value,
                                                          char const * const name,
                                                          T const minValue,
                                                          T const maxValue,
                                                          T const defaultValue = 0) {
      if (value) {
         return this->enforceMinAndMax(*value, name, minValue, maxValue, defaultValue);
      }
      return value;
   }

   /**
    * \brief This is intended to be called from setter member functions (including those of derived classes),
    *        \b before changing a property.  It triggers a check for whether this property change would require us to
    *        create a new version of a Recipe - eg because we are modifying some ingredient or other attribute of the
    *        Recipe and automatic versioning is enabled.
    */
   void prepareForPropertyChange(BtStringConst const & propertyName);

   /**
    * \brief This is intended to be called from setter member functions (including those of derived classes), \b after
    *        changing a property.  It checks whether the object is is in "cache only" mode and, if not, propagates the
    *        change down to the database layer and, optionally, also emits a "changed" signal.
    *
    * \param propertyName The name of the property that has been changed
    * \param emitChangedSignal Whether to emit a "changed" signal. Default is \c true
    */
   void propagatePropertyChange(BtStringConst const & propertyName, bool notify = true) const;


   /**
    * \brief Convenience function to check for the set being a no-op. (Sometimes the UI will call all setters, even on
    *        fields that haven't changed.)
    *
    * \return \c true if there's nothing to change, \c false otherwise
    */
   template<typename T>
   bool newValueMatchesExisting(BtStringConst const & propertyName,
                                T & memberVariable,
                                T const newValue) {
      if (newValue == memberVariable) {
         qDebug() <<
            Q_FUNC_INFO << this->metaObject()->className() << "#" << this->key() << ": ignoring call to setter for" <<
            propertyName << "as value not changing";
         return true;
      }
      return false;
   }

   /**
    * \brief Convenience function that wraps preparing for a property change, making it and propagating it.
    */
   template<typename T>
   void setAndNotify(BtStringConst const & propertyName,
                     T & memberVariable,
                     T const newValue) {
      if (this->newValueMatchesExisting(propertyName, memberVariable, newValue)) {
         return;
      }
      this->prepareForPropertyChange(propertyName);
      memberVariable = newValue;
      this->propagatePropertyChange(propertyName);
      return;
   }

private:
  QString m_folder;
  QString m_name;
  bool m_display;
  bool m_deleted;
  bool m_beingModified;
};

/**
 * \brief Convenience typedef for pointer to \c isOptional();
 */
using IsOptionalFnPtr = bool (*)(BtStringConst const &);

/**
 * \class NamedEntityModifyingMarker
 *
 * \brief RAII helper for temporarily marking a class as being modified
 */
class NamedEntityModifyingMarker {
public:
   NamedEntityModifyingMarker(NamedEntity & namedEntity);
   ~NamedEntityModifyingMarker();
private:
   NamedEntity & namedEntity;
   bool savedModificationState;

   // RAII class shouldn't be getting copied or moved
   NamedEntityModifyingMarker(NamedEntityModifyingMarker const &) = delete;
   NamedEntityModifyingMarker & operator=(NamedEntityModifyingMarker const &) = delete;
   NamedEntityModifyingMarker(NamedEntityModifyingMarker &&) = delete;
   NamedEntityModifyingMarker & operator=(NamedEntityModifyingMarker &&) = delete;
};

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, NamedEntity const & namedEntity) {
   stream << namedEntity.metaObject()->className() << " #" << namedEntity.key() << "(" << namedEntity.name() << ")";
   return stream;
}

template<class S>
S & operator<<(S & stream, NamedEntity const * namedEntity) {
   if (namedEntity) {
      stream << *namedEntity;
   } else {
      stream << "Null";
   }
   return stream;
}

/**
 * \brief Convenience function for logging, including coping with null pointers
 *
 *        std::is_base_of<NamedEntity, NE>::value is \c true if NE is \c NamedEntity or a subclass thereof
 *        std::enable_if_t<condition> is only defined if condition is true
 *        Thus std::enable_if_t<std::is_base_of<NamedEntity, NE>::value> is only defined if NE is \c NamedEntity or a
 *        subclass thereof.  This means this template should not be instantiated for any other classes.
 *
 *        .:TODO:. This isn't quite working yet!
 */
template<class S, class NE,
         std::enable_if_t<std::is_base_of<NamedEntity, NE>::value> >
S & operator<<(S & stream, NE const * namedEntity) {
   if (namedEntity) {
      stream << *namedEntity;
   } else {
      stream << "Null " << NE::staticMetaObject.metaObject()->className();
   }
   return stream;
}

/**
 * \brief Convenience function for, in effect, casting std::optional<int> to std::optional<T> where T is an enum class
 */
template <class T>
std::optional<T> castFromOptInt(std::optional<int> const & val) {
   if (val.has_value()) {
      return static_cast<T>(val.value());
   }
   return std::nullopt;
}

/**
 * \brief Convenience function for, in effect, casting std::optional<T> to std::optional<int> where T is an enum class
 */
template <class T>
std::optional<int> castToOptInt(std::optional<T> const & val) {
   if (val.has_value()) {
      return static_cast<int>(val.value());
   }
   return std::nullopt;
}

#endif
