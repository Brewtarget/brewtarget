/*======================================================================================================================
 * model/StockUse.h is part of Brewtarget, and is copyright the following authors 2025:
 *   • Matt Young <mfsy@yahoo.com>
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
 =====================================================================================================================*/
#ifndef MODEL_STOCKUSE_H
#define MODEL_STOCKUSE_H
#pragma once

#include <QDate>
#include <QString>

#include "model/BrewNote.h"
#include "model/NamedEntity.h"
#include "model/NamedParameterBundle.h"
#include "utils/EnumStringMapping.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StockUse { inline BtStringConst const property{#property}; }
AddPropertyName(brewNote    )
AddPropertyName(brewNoteId  )
AddPropertyName(comment     )
AddPropertyName(date        )
AddPropertyName(ownerId     )
AddPropertyName(quantityUsed)
AddPropertyName(reason      )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \class StockUse
 *
 * \brief This represents either the use of a batch/purchase an ingredient or some other disposal of it.
 *
 *        We could perhaps have called this \c StockPurchaseUse but that would be confusing as \c StockPurchaseHop et al
 *        are subclasses of \c StockPurchase, whereas \c StockUse is not.
 */
class StockUse : public NamedEntity {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_brewNote    ();
   static QString localisedName_brewNoteId  ();
   static QString localisedName_comment     ();
   static QString localisedName_date        ();
   static QString localisedName_ownerId     ();
   static QString localisedName_quantityUsed();
   static QString localisedName_reason      ();

   /**
    * \brief The reason for the inventory change.
    *
    *        Unlike a lot of our other enums, I've put these names in the past tense as I think it reads more naturally
    *        given that inventory updates are mostly "what happened" rather than "what's going to happen".
    *
    *        I thought about having "Correction" as a type of use, but I think that can best be handled in the
    *        \c StockPurchase subclasses.  Eg, if you ordered 1000g of something and it turns out your supplier sent you
    *        1017g of it, then the former is \c StockPurchaseBase::amountOrdered and the latter is
    *        \c StockPurchaseBase::amountReceived.  This keeps things simple as we don't have to extend the code to handle
    *        negative amounts.
    */
   enum class Reason {
      Used    , // Typically use in recipe
      Lost    , // Spillage and spoilage
      Disposed, //
   };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Reason)

   /*!
    * \brief Mapping between \c StockUse::Reason and string values suitable for serialisation in DB
    *
    *        This can also be used to obtain the number of values of \c Reason, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const reasonStringMapping;

   /*!
    * \brief Localised names of \c StockUse::Reason values suitable for displaying to the end user
    */
   static EnumStringMapping const reasonDisplayNames;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   StockUse(QString name = "");
   StockUse(NamedParameterBundle const & namedParameterBundle);
   StockUse(StockUse const & other);

   virtual ~StockUse();

   /**
    * \brief StockUse instances are ordered by date rather than name, so we have to override \c NamedEntity ordering
    */
   std::strong_ordering operator<=>(StockUse const & other) const;

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(QDate   date           READ date           WRITE setDate        )

   Q_PROPERTY(Reason  reason         READ reason         WRITE setReason      )

   //! The owning inventory object will determine whether this is mass, volume, etc
   Q_PROPERTY(double  quantityUsed   READ quantityUsed   WRITE setQuantityUsed)

   /**
    * \brief This is an optional brief descriptive note of the use.
    *
    *        I avoided calling this \c note or \c notes, because we use \c notes in other classes for multi-line
    *        descriptive text, whereas this is intended to be something that will just have a single line edit input.
    *        (Of course both are strings with no length limit, but I think it's still a useful distinction.)
    *
    *        I did also consider calling this field \c description for consistency with \c Step, but I think that's less
    *        accurate.  \c BoilStep, \c MashStep etc are process descriptions ("Do this, when that, etc") whereas
    *        \c StockUse subclasses are "Where this amount of this ingredient ended up".
    *
    *        It was tempting to (ab)use the \c name field inherited from \c NamedEntity, instead of creating this new
    *        one.  However, if we ever decide to store \c StockUse subclass instances in some extension of BeerXML or
    *        BeerJSON, that would cause us problems because there is logic in the deserialisation code that tries to
    *        ensure names are unique.
    */
   Q_PROPERTY(QString comment READ comment  WRITE setComment)

   //! If the use was in a Recipe, this is the relevant brew of that recipe
   Q_PROPERTY(std::shared_ptr<BrewNote> brewNote      READ brewNote      WRITE setBrewNote     STORED false)
   Q_PROPERTY(int                       brewNoteId    READ brewNoteId    WRITE setBrewNoteId)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   QDate                     date        () const;
   Reason                    reason      () const;
   double                    quantityUsed() const;
   QString                   comment     () const;
   std::shared_ptr<BrewNote> brewNote    () const;
   int                       brewNoteId  () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setDate        (QDate                     const   val);
   void setReason      (Reason                    const   val);
   void setQuantityUsed(double                    const   val);
   void setComment     (QString                   const & val);
   void setBrewNote    (std::shared_ptr<BrewNote> const   val);
   void setBrewNoteId  (int                       const   val);

protected:
   virtual bool compareWith(NamedEntity const & other,
                            QList<BtStringConst const *> * propertiesThatDiffer) const override;

   QDate   m_date             ;
   Reason  m_reason           ;
   double  m_quantityUsed     ;
   QString m_comment          ;
   int     m_brewNoteId       ;
};

#endif
