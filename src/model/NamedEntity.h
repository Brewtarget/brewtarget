/*
 * model/NamedEntity.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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

#include <QDateTime>
#include <QList>
#include <QMetaProperty>
#include <QObject>
#include <QVariant>

#include "brewtarget.h"
#include "utils/BtStringConst.h"

class NamedParameterBundle;
class ObjectStore;
class Recipe;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// Make this class's property names available via constants in sub-namespace of PropertyNames
// One advantage of using these constants is you get compile-time checking for typos etc
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
 * Note that this class has previously been called \b Ingredient and \b BeerXMLElement.  We've changed the name to try
 * to best reflect what the class represents.  Although some of this class's subclasses (eg \b Hop, \b Fermentable,
 * \b Yeast) are ingredients in the normal sense of the word, others (eg \b Instruction, \b Equipment, \b Style,
 * \b Mash) are not really.  Equally, the fact that derived classes can be instantiated from BeerXML is not their
 * defining characteristic.
 *
 * NB: Although we can template individual member functions, we cannot make this a template class (eg to use
 * https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) because the Qt Meta-Object Compiler (moc) cannot
 * handle templates, and we want to be able to use the Qt Property system as well as signals and slots.
 *
 * NB: Because NamedEntity inherits from QObject, no extra work is required to store pointers to NamedEntity objects
 *     inside QVariant
 */
class NamedEntity : public QObject {
   Q_OBJECT
   Q_CLASSINFO("version","1")


public:
   NamedEntity(int key, bool cache = true, QString t_name = QString(), bool t_display = false, QString folder = QString());
   NamedEntity(NamedEntity const & other);

   /**
    * \brief Note that, if you want a \b child of a \c NamedEntity (to add to a \c Recipe), you should call
    *        \c makeChild() on the copy, which will do the right things about parentage and inventory.
    */
   NamedEntity(NamedParameterBundle const & namedParameterBundle);

   // Our destructor needs to be virtual because we sometimes point to an instance of a derived class through a pointer
   // to this class -- ie NamedEntity * namedEntity = new Hop() and suchlike.  We do already get a virtual destructor by
   // virtue of inheriting from QObject, but this declaration does no harm.
   virtual ~NamedEntity();

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
    * \brief We don't have a need to assign one NamedEntity to another, and the compiler implementation of this would
    *        be wrong, so we delete it.
    */
   NamedEntity & operator=(NamedEntity const &) = delete;

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
   Q_PROPERTY( QString name   READ name WRITE setName )
   Q_PROPERTY( bool deleted   READ deleted WRITE setDeleted )
   Q_PROPERTY( bool display   READ display WRITE setDisplay )
   Q_PROPERTY( QString folder READ folder WRITE setFolder )

   Q_PROPERTY( int key READ key WRITE setKey )
   Q_PROPERTY( int parentKey READ getParentKey WRITE setParentKey )
   //! \brief To cache or not to cache
   Q_PROPERTY( bool cacheOnly READ cacheOnly WRITE setCacheOnly /*NOTIFY changed*/ )

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

   bool cacheOnly() const;
   void setCacheOnly(bool cache);

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
    * Passes the meta property that has changed about this object.
    * NOTE: when subclassing, be \em extra careful not to create a method with
    * the same signature. Otherwise, everything will silently break.
    */
   void changed(QMetaProperty, QVariant value = QVariant()) const;
   void changedFolder(QString);
   void changedName(QString);

protected:
   //! The key of this entity in its table.
   int m_key;
   // This is 0 if there is no parent (or parent is not yet known)
   int parentKey;
   bool m_cacheOnly;

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

#endif
