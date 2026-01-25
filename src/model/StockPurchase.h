/*======================================================================================================================
 * model/StockPurchase.h is part of Brewtarget, and is copyright the following authors 2021-2026:
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
#ifndef MODEL_STOCKPURCHASE_H
#define MODEL_STOCKPURCHASE_H
#pragma once

#include <memory>

#include <QObject>

#include "database/ObjectStoreWrapper.h"
#include "measurement/CurrencyAmount.h"
#include "model/NamedParameterBundle.h"
#include "model/Ingredient.h"
#include "model/NamedEntity.h"
#include "utils/MetaTypes.h"
#include "utils/AutoCompare.h"

class ObjectStore;
class TypeLookup;


//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StockPurchase { inline BtStringConst const property{#property}; }
AddPropertyName(dateBestBefore )
AddPropertyName(dateOrdered    )
AddPropertyName(dateReceived   )
AddPropertyName(note           )
AddPropertyName(purchasePrice  )
AddPropertyName(purchaseTax    )
AddPropertyName(quantityOrdered)
AddPropertyName(shippingCost   )
AddPropertyName(supplier       )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

//
// Only classes that derive from Ingredient have inventory.
//
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <typename NE> concept CONCEPT_FIX_UP    CanHaveStockPurchase = std::is_base_of_v<Ingredient, NE>;
template <typename NE> concept CONCEPT_FIX_UP CannotHaveStockPurchase = std::negation_v<std::is_base_of<Ingredient, NE>>;

/**
 * \brief Class representing the acquisition (eg purchase) of an ingredient (Hop/Fermentable/Yeast/Misc/Salt).
 *
 *        In older versions of the software, we used "Inventory" to talk about how much stock we have of a given
 *        ingredient.  However, we now want to track acquisition and use of ingredients, but we don't want to end up
 *        with super-long class names such as \c InventoryAcquisitionFermentable.  So, we use "Stock" instead of
 *        "Inventory".
 *
 *        We would still have \c StockAcquisitionFermentable being a bit on the long side though.  So, since most
 *        ingredient acquisitions are purchases, we use \c StockPurchase rather than \c StockAcquisition, with the
 *        proviso that a purchase can be at zero or unspecified cost.
 *
 *        Each use of an ingredient from a given \c StockPurchase is tracked in an \c StockUse record (eg
 *        \c StockUseHop, \c StockUseFermentable).
 *
 *        The intent is to allow the user not just to say "How much of ingredient X do I have" but also to track
 *        separate purchases of the same ingredient so s/he can see price changes over time, do stock rotation (ie
 *        ensure older purchases used first) and track which ingredients are approaching their best-before date.
 *
 *        Subclasses need to supply a `using IngredientClass` alias analogous to the `using StockPurchaseClass` one in
 *        the Ingredient classes.  This is handled automatically by macros in \c model/StockPurchaseBase.h.
 *
 *        NB: We want each type of inventory to be a different class so that it works with \c ObjectStoreTyped and is
 *            stored in a separate database table.
 *
 *            It would be tempting to make StockPurchase a templated class (for \c StockPurchase<Hop>,
 *            \c StockPurchase<Fermentable>, etc), however we need StockPurchase to inherit from QObject so we can use Qt
 *            Properties in the \c ObjectStore layer, and this precludes the use of templates.  (The Qt meta-object
 *            compiler, aka moc, does not understand C++ templates.)
 *
 *            Instead we use inheritance to get \c StockPurchaseHop, \c StockPurchaseFermentable, etc, and have those
 *            subclasses also inherit from the templates classes \c IngredientAmount and \c StockPurchaseBase.
 *
 *        TBD: We do not group StockPurchase items into orders, but it is something we could do in future if there were
 *             demand for it.
 */
class StockPurchase : public NamedEntity {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_dateOrdered    ();
   static QString localisedName_dateReceived   ();
   static QString localisedName_dateBestBefore ();
   static QString localisedName_supplier       ();
   static QString localisedName_note           ();
   static QString localisedName_quantityOrdered();
   static QString localisedName_purchasePrice  ();
   static QString localisedName_purchaseTax    ();
   static QString localisedName_shippingCost   ();
   static QString localisedName_ingredient     ();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   StockPurchase(QString const & name = "");
   StockPurchase(NamedParameterBundle const & namedParameterBundle);
   StockPurchase(StockPurchase const & other);

   virtual ~StockPurchase();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(std::optional<QDate> dateOrdered      READ dateOrdered      WRITE setDateOrdered   )
   Q_PROPERTY(std::optional<QDate> dateReceived     READ dateReceived     WRITE setDateReceived  )
   Q_PROPERTY(std::optional<QDate> dateBestBefore   READ dateBestBefore   WRITE setDateBestBefore)

   Q_PROPERTY(QString supplier   READ supplier   WRITE setSupplier)

   /**
    * \brief This is intended to be a brief note (hence singular \c note rather than \c notes that we use on other
    *        classes for more extensive text.
    */
   Q_PROPERTY(QString note   READ note   WRITE setNote)

   /**
    * \brief This is how much of the ingredient was ordered, whereas \c IngredientAmount::amount is the actual amount
    *        that arrived.  Often there is a small difference -- eg 1000g ordered, 1039g arrived.
    *
    *        Note that units for \c quantityOrdered are the same as \c IngredientAmount::amount, hence why we only store
    *        quantity for this field.  (Yes, in theory, it could be that you ordered something by volume and then, when
    *        it arrived, decided to measure it by mass instead but I think that's a somewhat hypothetical edge case.
    *        It's more realistic to assume that it is the nature of the ingredient that determines whether it is
    *        measured by mass or volume etc -- eg Hops are measured by mass except where form is \c Hop::Form::Extract.)
    */
   Q_PROPERTY(std::optional<double> quantityOrdered      READ quantityOrdered      WRITE setQuantityOrdered)

   /**
    * \brief Normally this will be in the same currency as \c QLocale::currencySymbol, but we allow the user to override
    *        currency, eg for cross-border purchases.
    */
   Q_PROPERTY(std::optional<CurrencyAmount> purchasePrice   READ purchasePrice   WRITE setPurchasePrice)
   Q_PROPERTY(std::optional<CurrencyAmount> purchaseTax     READ purchaseTax     WRITE setPurchaseTax  )
   Q_PROPERTY(std::optional<CurrencyAmount> shippingCost    READ shippingCost    WRITE setShippingCost )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   std::optional<QDate>          dateOrdered    () const;
   std::optional<QDate>          dateReceived   () const;
   std::optional<QDate>          dateBestBefore () const;
   QString                       supplier       () const;
   QString                       note           () const;
   std::optional<double>         quantityOrdered() const;
   std::optional<CurrencyAmount> purchasePrice  () const;
   std::optional<CurrencyAmount> purchaseTax    () const;
   std::optional<CurrencyAmount> shippingCost   () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setDateOrdered    (std::optional<QDate>          const & val);
   void setDateReceived   (std::optional<QDate>          const & val);
   void setDateBestBefore (std::optional<QDate>          const & val);
   void setSupplier       (QString                       const & val);
   void setNote           (QString                       const & val);
   void setQuantityOrdered(std::optional<double>         const   val);
   void setPurchasePrice  (std::optional<CurrencyAmount> const & val);
   void setPurchaseTax    (std::optional<CurrencyAmount> const & val);
   void setShippingCost   (std::optional<CurrencyAmount> const & val);

signals:
   //! Emitted when the number of \c StockUse items changes, or when you should call \c changes() again.
   void ownedItemsChanged();

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;

   std::optional<QDate>          m_dateOrdered    ;
   std::optional<QDate>          m_dateReceived   ;
   std::optional<QDate>          m_dateBestBefore ;
   QString                       m_supplier       ;
   QString                       m_note           ;
   std::optional<double>         m_quantityOrdered;
   std::optional<CurrencyAmount> m_purchasePrice  ;
   std::optional<CurrencyAmount> m_purchaseTax    ;
   std::optional<CurrencyAmount> m_shippingCost   ;
};

#endif
