/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/NamedEntity.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Jeff Bailey <skydvr38@verizon.net>
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
#include <QRegularExpression>
#include <QVariant>

#include "model/FolderBase.h"
#include "model/NamedEntityCasters.h"
#include "utils/BtStringConst.h"
#include "utils/MetaTypes.h"
#include "utils/TypeLookup.h"

class NamedEntity;
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
// address in memory.  In other words, _don't_ do `if (&somePropName == &PropertyNames::NamedEntity::name) ...`.
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
AddPropertyName(key)
AddPropertyName(name)
AddPropertyName(parentKey)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief See \c NamedEntity::typeLookup.  This macro -- to include just after \c typeLookup in the class declaration of
 *        \c NamedEntity and every non-abstract sublass thereof -- adds a virtual member function to return the static
 *        \c typeLookup member.
 *
 *        I'm not sure whether there is a clever way to do this with the curiously recurring template pattern, but, if
 *        there is, I haven't figured it out yet.  So, for now at least, macros are better than copy-and-paste code
 *        everywhere.
 *
 *        NOTE that, strictly, this is not needed on abstract classes, though it is relatively harmless to include on
 *        them.
 */
#define TYPE_LOOKUP_GETTER inline virtual TypeLookup const & getTypeLookup() const { return typeLookup; }

/*!
 * \class NamedEntity
 *
 * \brief The base class for our substantive storable items.  There are really two sorts of storable items: ones that
 *        are freestanding and ones that are owned by other storable items.  Eg, a Hop exists in its own right and may
 *        or may not be used in one or more Recipes, but a MashStep only exists as part of a single Mash.  See comment
 *        on \c owningRecipe below for more on this.
 *
 *        I know \b NamedEntity isn't the snappiest name, but it's the best we've come up with so far.  If you look at
 *        older versions of the code, you'll see that this class has previously been called \b Ingredient and
 *        \b BeerXMLElement.  The current name tries to best reflect what the class represents.  Although some of this
 *        class's subclasses (eg \b Hop, \b Fermentable, \b Yeast) are ingredients in the normal sense of the word (and
 *        this is now reflected in the \c Ingredient abstract class), others (eg \b Instruction, \b Equipment, \b Style,
 *        \b Mash) are not really.  Equally, the fact that derived classes can be instantiated from BeerXML is not their
 *        defining characteristic.
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
 */
class NamedEntity : public QObject {
   Q_OBJECT
   Q_CLASSINFO("version","1")

public:

   /**
    * \brief Subclasses should provide their localised (ie translated) name via this static member function, so that
    *        templated functions can use it.  Note that translations are obtained at run-time (not least because it is
    *        possible to change language at run-time), which is one reason this needs to be a function rather than a
    *        member variable.  (The other reason is static initialization order being undefined - per
    *        https://en.cppreference.com/w/cpp/language/siof.)
    *
    *        We shouldn't ever need to use the name for \c NamedEntity itself, but it's here for completeness.
    */
   static QString localisedName();

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
    *
    *        In other circumstances, if we need the \c TypeLookup for an instance of an unknown subclass of
    *        \c NamedEntity (eg in the \c PropertyPath), we should call the virtual member function \c getTypeLookup
    *        which just returns the relevant object.  That's a turn-the-handle function
    */
   static TypeLookup const typeLookup;
   // See comment above for what this does
   TYPE_LOOKUP_GETTER

   NamedEntity(QString t_name, bool t_display = false);
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

protected:
   /**
    * \brief Swap the contents of two NamedEntity objects - which provides an exception-safe way of implementing
    *        operator=.  This is used in \c Water but not many other places AFAICT.
    */
   virtual void swap(NamedEntity & other) noexcept;

public:

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
    *        TODO: We are trying to retire this!
    *
    * \param copiedFrom The object from which this one was copied
    */
   [[deprecated]] virtual void makeChild(NamedEntity const & copiedFrom);

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

   /**
    * \brief As you might expect, this ensures we order \b NamedEntity objects by name
    *
    *        Most subclasses do not need any more ordering than this.  However \c RecipeAddition subclasses do need to
    *        override this, so we can have a canonical ordering of a list of, eg, pointers to \c RecipeAdditionHop, so
    *        that we can then easily compare two such lists for equality.
    */
   std::strong_ordering operator<=>(NamedEntity const & other) const;

   // Everything that inherits from NamedEntity has these properties
   Q_PROPERTY(QString name      READ name         WRITE setName     )
   Q_PROPERTY(bool    deleted   READ deleted      WRITE setDeleted  )
   Q_PROPERTY(bool    display   READ display      WRITE setDisplay  )
   //! Key (ID) in the table we are stored in
   Q_PROPERTY(int     key       READ key          WRITE setKey      )
   //! TODO Once \c makeChild is retired, this can be retired too
   Q_PROPERTY(int     parentKey READ getParentKey WRITE setParentKey)

   QString name() const;
   bool deleted() const;
   bool display() const;
   int key() const;
   [[deprecated]] int getParentKey() const;

   /**
    * \brief Returns a regexp that will match the " (n)" (for n some positive integer) added on the end of a name to
    *        prevent name clashes.  It will also "capture" n to allow you to extract it.
    */
   static QRegularExpression const & getDuplicateNameNumberMatcher();

   void setName(QString const & var);
   void setDeleted(bool const var);
   void setDisplay(bool const var);
   /**
    * \brief Set the ID (aka key) by which this object is uniquely identified in its DB table
    *
    *        This is virtual because, in some cases, subclasses are going to want to do additional work here
    */
   virtual void setKey(int key);
   [[deprecated]] void setParentKey(int parentKey);

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
    * \brief Get the IDs of this object's parent, children and siblings (plus the ID of the object itself).
    *        A child object is just a copy of the parent that's being used in a Recipe.  Not all NamedEntity subclasses
    *        have children, just Equipment, Fermentable, Hop, Misc and Yeast.
    */
   [[deprecated]] QVector<int> getParentAndChildrenIds() const;

   //! Convenience method to get a meta property by name.
   QMetaProperty metaProperty(char const * const name) const;

   /**
    * \brief Subclasses need to override this to return the \c Recipe, if any, to which this object belongs.  (Note that
    *        a \c Recipe belongs to itself for the purposes of this function.)
    *
    *        Broadly speaking, there are three categories of \c NamedEntity:
    *
    *         - Dependent items such as \c BrewNote, \c Instruction and \c RecipeAdditionHop which \b always belong to
    *           exactly one \c Recipe and which get deleted if that \c Recipe is deleted.  A change to one of these
    *           items is treated as a change to the \c Recipe.
    *
    *         - Independent items such as \c Equipment, \c Hop, \c InventoryHop which exist independently of any
    *           \c Recipe.  Even if all recipes were deleted, these things would continue to exist.  (However the
    *           reverse is not necessarily true, in that we should not delete, eg, a \c Hop if it is being used, via
    *           \c RecipeAdditionHop, in one or more \c Recipes.)  A change to one of these items \b may affect one or
    *           more \c Recipes, requiring recalculations therein, but is not treated as a \c change in a \c Recipe.
    *
    *         - Semi-Independent items such as \c Mash, \c MashStep, \c Boil, \c BoilStep, \c Fermentation,
    *           \c FermentationStep which, strictly, exist independently of any \c Recipe but which are often used only
    *           by one \c Recipe.  Although deletion of a \c Recipe never causes deletion of a semi-independent item, we
    *           may treat a change to a semi-independent item used in only one \c Recipe as a change to that \c Recipe
    *           (because this is what most users would expect, I think).
    *
    *        NOTE that, although semi-independent of \c Recipe, \c MashStep is entirely dependent on its \c Mash and has
    *        no independent existence from it.  Same for \c BoilStep and \c Boil, \c FermentationStep and
    *        \c Fermentation, etc.
    *
    *        NOTE too that Independent (and Semi-Independent) items have folders unless they are owned by another item
    *        (eg \c Mash has a folder but \c MashStep does not).  Dependent items do not have folders (because they are
    *        owned by \c Recipe).  See model/FolderBase.h for more.
    *
    *        The following pseudo-inheritance diagram shows which \c NamedEntity classes are Dependent, Independent and
    *        Semi-Independent.  (The class \c OwnedByRecipe exists, but \c IndependentOfRecipe does not.)
    *
    *           OwnedByRecipe                          IndependentOfRecipe († = semi-independent)
    *             ├── BrewNote                           ├── Boil †
    *             ├── Instruction                        ├── Equipment
    *             ├── RecipeAddition                     ├── Fermentation †
    *             │    ├── RecipeAdditionFermentable     ├── Ingredient
    *             │    ├── RecipeAdditionHop             │    ├── Fermentable
    *             │    ├── RecipeAdditionMisc            │    ├── Hop
    *             │    ├── RecipeAdjustmentSalt          │    ├── Misc
    *             │    └── RecipeAdditionYeast           │    ├── Salt
    *             └── RecipeUseOfWater                   │    └── Yeast
    *                                                    ├── Inventory
    *                                                    │    ├── InventoryFermentable (owned by its Fermentable)
    *                                                    │    ├── InventoryHop         (owned by its Hop        )
    *                                                    │    ├── InventoryMisc        (owned by its Misc       )
    *                                                    │    └── InventoryYeast       (owned by its Yeast      )
    *                                                    ├── Mash †
    *                                                    ├── Recipe (but owns itself for the purpose of changes)
    *                                                    ├── Step
    *                                                    │    ├── MashStep † (owned by its Mash)
    *                                                    │    └── StepExtended
    *                                                    │         ├── BoilStep † (owned by its Boil)
    *                                                    │         └── FermentationStep † (owned by its Fermentation)
    *                                                    ├── Style
    *                                                    └── Water
    *
    *        HOWEVER, even aside from the case of semi-independent items, we want run-time determination of whether an
    *        object has an owning \c Recipe to make it easy to determine whether a change to a base class property
    *        constitutes a change to a \c Recipe (and if so which one).  Hence this function.
    *
    * \return \c nullptr if this object does not belong to a \c Recipe (ie it is an Independent Item or a
    *         Semi-Independent Item that is either used in more than one \c Recipe or not used in any \c Recipe)
    */
   virtual std::shared_ptr<Recipe> owningRecipe() const;

   /*!
    * \brief Some entities (eg Fermentable, Hop) get copied when added to a recipe, but others (eg Instruction) don't.
    *        For those that do, we think of the copy as being a child of the original NamedEntity.  This function allows
    *        us to access that parent.
    *
    * \return Pointer to the parent NamedEntity from which this one was originally copied, or null if no such parent
    *         exists.
    */
   [[deprecated]] NamedEntity * getParent() const;

   [[deprecated]] void setParent(NamedEntity const & parentNamedEntity);

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

   /**
    * \brief Where a \c NamedEntity contains and owns another \c NamedEntity (eg as \c Recipe can contain a \c Boil),
    *        this allows generic code to ensure that such a contained object exists -- typically because we want to set
    *        one of its properties.
    *
    *        Child classes need to override this for any properties where it is relevant.
    *
    * \return Pointer to the object whose existence we want to ensure (which will have been newly created if necessary).
    */
   virtual NamedEntity * ensureExists(BtStringConst const & property);

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
   /**
    * \brief If a derived class calls a setter from its constructor (eg as we do in \c Step to handle setting
    *        \c m_stepTime_mins from either \c PropertyNames::Step::stepTime_mins or
    *        \c PropertyNames::Step::stepTime_days) we don't want to be sending signals or trying to write to the
    *        database.  Besides being somewhat circular to write to the DB whilst we are perhaps reading the object from
    *        the DB, it's not always possible.  Eg, \c Step is a pure virtual class, and, at the point its constructor
    *        is running, it is not valid to call \c this->getObjectStoreTypedInstance() (because the vtable for the
    *        derived class such as \c BoilStep or \c MashStep has not yet been created).
    *
    *        So, this flag is set to \c false in the \c NamedEntity constructor (courtesy of the default here) to
    *        disable signalling and propagation down to the DB when properties change.  Concrete subclasses should turn
    *        this flag on as the last action of their constructor.  HOWEVER to make life simple, we just include the
    *        CONSTRUCTOR_END macro (see below) at the end of every constructor, which turns the flag on if the class is
    *        not abstract.
    */
   bool m_propagationAndSignalsEnabled = false;

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
    *        A sub-sub-class of \c NamedEntity (eg \c RecipeAdditionHop) should call its parent's implementation of this
    *        function before doing its own class-specific tests.
    * \return \b true if this object is, in all the ways that matter, equal to \b other
    */
   virtual bool isEqualTo(NamedEntity const & other) const = 0;

   /**
    * \brief Subclasses need to override this function to return the appropriate instance of \c ObjectStoreTyped.
    *        This allows us in this base class to access \c ObjectStoreTyped<Hop> for \c Hop,
    *        \c ObjectStoreTyped<Fermentable> for \c Fermentable, etc.
    */
   virtual ObjectStore & getObjectStoreTypedInstance() const = 0;

   // Depending on who created the bundle, the "either-or" amounts can be set either via their individual properties (eg
   // if we're reading from the database) or via their composite attributes (eg if we're reading from a BeerJSON file).

   /**
    * \brief Some "either-or" attributes can be measured and stored in two ways, eg the amount of a \c Fermentable can
    *        be measured either by \c Mass or by \c Volume (typically depending on what type of fermentable it is).  In
    *        these cases, our internal storage is two fields, a \c double measuring the quantity and a \c bool
    *        indicating whether it is the "first" or the "second" type.  Eg for something that's either \c Mass or
    *        \c Volume, we always have \c Mass as the "first" type and \c Volume as the "second". This is formalised
    *        somewhat in instances of \c Measurement::ConstrainedAmount (specifically \c MassOrVolumeAmt
    *        and \c MassOrVolumeConcentrationAmt).
    *
    *        When constructing an object from a \c NamedParameterBundle, there are two ways that an "either-or"
    *        attribute can be specified.  If we're reading from the database, or from BeerXML, then each of the
    *        underlying fields will be specified individually.  Eg, \c PropertyNames::Fermentable::amount and
    *        \c PropertyNames::Fermentable::amountIsWeight will each be specified in the bundle.  However, if we're
    *        reading from BeerJSON, we'll get a combined quantity-and-units parameter,
    *        eg \c PropertyNames::Fermentable::amountWithUnits.
    *
    *        This function does the generic work for initialising such either-or parameters from a
    *        \c NamedParameterBundle.
    */
   void setEitherOrReqParams(NamedParameterBundle const & namedParameterBundle,
                             BtStringConst const & quantityParameterName,
                             BtStringConst const & isFirstUnitParameterName,
                             BtStringConst const & combinedWithUnitsParameterName,
                             Measurement::PhysicalQuantity const firstUnitPhysicalQuantity,
                             double & quantityReturn,
                             bool & isFirstUnitReturn,
                             std::optional<bool> const defaultIsFirstUnit = std::nullopt);

   /**
    * \brief As \c setEitherOrReqParams but for when the attribute is optional
    */
   void setEitherOrOptParams(NamedParameterBundle const & namedParameterBundle,
                             BtStringConst const & quantityParameterName,
                             BtStringConst const & isFirstUnitParameterName,
                             BtStringConst const & combinedWithUnitsParameterName,
                             Measurement::PhysicalQuantity const firstUnitPhysicalQuantity,
                             std::optional<double> & quantityReturn,
                             bool & isFirstUnitReturn);

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
      // We could use std::clamp here, but, since we want to print the warning message, it wouldn't buy us anything
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
    * \brief Emit a "changed" signal for the supplied \c propertyName.  Usually called from \c propagatePropertyChange,
    *        but can be called directly when the property being updated is not stored in the DB (or not stored in the
    *        default way -- see eg RecipeAddition subclasses).
    */
   void notifyPropertyChange(BtStringConst const & propertyName) const;

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
            propertyName << "as value (" << newValue << ") not changing";
         return true;
      }
      return false;
   }

   /**
    * \brief Convenience function that wraps preparing for a property change, making it and propagating it.
    *
    * \return \c true if the property actually changed, \c false if it did not (ie the "set" was in fact setting the
    *         value to what it already was.  This is useful for the caller if, eg, there might be recalculations
    *         required when a value changes
    */
   template<typename T>
   bool setAndNotify(BtStringConst const & propertyName,
                     T & memberVariable,
                     T const newValue) {
      // Normally leave this log statement commented out as it generates too many lines in the log file
//      qDebug() << Q_FUNC_INFO << propertyName << ": change from" << memberVariable << "to" << newValue;
      if (this->newValueMatchesExisting(propertyName, memberVariable, newValue)) {
         return false;
      }
      this->prepareForPropertyChange(propertyName);
      memberVariable = newValue;
      this->propagatePropertyChange(propertyName);
      return true;
   }

private:
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

template<class S>
S & operator<<(S & stream, std::shared_ptr<NamedEntity> namedEntity) {
   stream << namedEntity.get();
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

// Note that we cannot write `Q_DECLARE_METATYPE(NamedEntity)` here, because NamedEntity is an abstract class
Q_DECLARE_METATYPE(NamedEntity *)
Q_DECLARE_METATYPE(NamedEntity const *)
Q_DECLARE_METATYPE(std::shared_ptr<NamedEntity>)

/**
 * \brief Convenience macro
 */
#define SET_AND_NOTIFY(...) this->setAndNotify(__VA_ARGS__)

template<typename T> constexpr bool IsAbstract(T const *) { return std::is_abstract<T>::value; }

/**
 * \brief Subclasses should include this macro at the end of \b all of their constructors.  See comment on
 *        \c m_propagationAndSignalsEnabled for more info.
 *
 *        There is probably a way to make this if statement constexpr, but I haven't figured it out yet!
 *
 *        TODO: We could probably remove a \b lot of boilerplate from the three main constructors (create empty,
 *              copy, create from NamedParameterBundle) by having generic code that uses the static typeLookup member
 *              variable to loop through and initialise all of the instance member variables.  We'd need to move default
 *              values to the header (which is best practice now anyway) and add something to \c TypeInfo to say when
 *              default values are OK for constructing from NamedParameterBundle, but that should be doable).
 */
#define CONSTRUCTOR_END \
  if (!IsAbstract(this)) { \
     this->NamedEntity::m_propagationAndSignalsEnabled = true; \
  }

/**
 * \brief For some templated functions, it's useful at compile time to have one version for NE classes with folders and
 *        one for those without.  We need to put the concepts here in the base class for them to be accessible.
 *
 *        See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it).
 */
template <typename T> concept CONCEPT_FIX_UP HasFolder   = std::is_base_of_v<FolderBase<T>, T>;
template <typename T> concept CONCEPT_FIX_UP HasNoFolder = std::negation_v<std::is_base_of<FolderBase<T>, T>>;

#endif
