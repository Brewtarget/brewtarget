/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Hop.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Mattias Måhl <mattias@kejsarsten.com>
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
#ifndef MODEL_HOP_H
#define MODEL_HOP_H
#pragma once

#include <array>

#include <QString>
#include <QStringList>
#include <QSqlRecord>

#include "model/Ingredient.h"
#include "model/IngredientBase.h"
#include "model/IngredientAmount.h"
#include "utils/EnumStringMapping.h"

class HopCatalog;
class HopEditor;
class InventoryHop;
class HopItemDelegate;
class RecipeAdditionHop;
class HopSortFilterProxyModel;
class HopTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Hop { inline BtStringConst const property{#property}; }
AddPropertyName(alpha_pct         )
AddPropertyName(beta_pct          )
AddPropertyName(bPinene_pct       )
AddPropertyName(caryophyllene_pct )
AddPropertyName(cohumulone_pct    )
AddPropertyName(farnesene_pct     )
AddPropertyName(form              )
AddPropertyName(geraniol_pct      )
AddPropertyName(hsi_pct           )
AddPropertyName(humulene_pct      )
AddPropertyName(limonene_pct      )
AddPropertyName(linalool_pct      )
AddPropertyName(myrcene_pct       )
AddPropertyName(nerol_pct         )
AddPropertyName(notes             )
AddPropertyName(origin            )
AddPropertyName(pinene_pct        )
AddPropertyName(polyphenols_pct   )
AddPropertyName(producer          )
AddPropertyName(productId         )
AddPropertyName(substitutes       )
AddPropertyName(totalOil_mlPer100g)
AddPropertyName(type              )
AddPropertyName(xanthohumol_pct   )
AddPropertyName(year              )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Hop
 *
 * \brief Model class for a hop record in the database.
 *
 *        TBD: Ideally we would find a way to model:
 *           > A general type of Hop - eg "East Kent Goldings", which will have an alpha acid range
 *           > A specific form and harvest - eg "Pellet 2020 harvest", which will have a single alpha acid value eg 6.0%
 *        Probably we should do this via Inventory.
 *
 * TODO TODO TODO TODO TODO TODO TODO TODO TODO Sort out inventory!
 */
class Hop : public Ingredient, public IngredientBase<Hop> {
   Q_OBJECT

   INGREDIENT_BASE_DECL(Hop)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /*!
    * \brief The form of the hop.
    */
   enum class Form {Leaf,
                    Pellet,
                    Plug,
                    // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
                    Extract,
                    WetLeaf,
                    Powder};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Form)

   /*!
    * \brief Mapping between \c Hop::Form and string values suitable for serialisation in DB, BeerJSON, etc (but \b not
    *        BeerXML)
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const formStringMapping;

   /*!
    * \brief Localised names of \c Hop::Form values suitable for displaying to the end user
    */
   static EnumStringMapping const formDisplayNames;

   /*!
    * \brief The type of hop, meaning for what properties it is used.  Arguably we should have three binary flags
    *        (aroma, bittering and flavor), but keeping a single enum makes the mappings to/from BeerXML and BeerJSON
    *        easier.
    */
   enum class Type {Bittering,
                    Aroma,
                    AromaAndBittering, // was Both
                    // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
                    Flavor,
                    BitteringAndFlavor,
                    AromaAndFlavor,
                    AromaBitteringAndFlavor};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   /*!
    * \brief Mapping between \c Hop::Type and string values suitable for serialisation in DB, BeerJSON, etc (but \b not
    *        BeerXML)
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const typeStringMapping;

   /*!
    * \brief Localised names of \c Hop::Type values suitable for displaying to the end user
    */
   static EnumStringMapping const typeDisplayNames;

   /**
    * \brief This is where we centrally define how \c Hop objects can be measured.  Essentially, mass is used in almost
    *        all cases, but volume is useful for \c Hop::Form::Extract
    */
   static constexpr auto validMeasures  = Measurement::ChoiceOfPhysicalQuantity::Mass_Volume;
   static constexpr auto defaultMeasure = Measurement::PhysicalQuantity::Mass;

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using CatalogClass              = HopCatalog;
   using EditorClass               = HopEditor;
   using InventoryClass            = InventoryHop;
   using ItemDelegateClass         = HopItemDelegate;
   using RecipeAdditionClass       = RecipeAdditionHop;
   using SortFilterProxyModelClass = HopSortFilterProxyModel;
   using TableModelClass           = HopTableModel;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Hop(QString name = "");
   Hop(NamedParameterBundle const & namedParameterBundle);
   Hop(Hop const & other);

   virtual ~Hop();

   //=================================================== PROPERTIES ====================================================
   //! \brief The percent alpha acid
   Q_PROPERTY(double                alpha_pct    READ alpha_pct    WRITE setAlpha_pct            )
   /**
    * \brief The \c Form.                ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    *
    *        See comment in \c model/Fermentable.h for \c grainGroup property for why this has to be
    *        \c std::optional<int>, not \c std::optional<Use>
    */
   Q_PROPERTY(std::optional<int>    form         READ formAsInt    WRITE setFormAsInt            )
   //! \brief The percent of beta acids.  ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> beta_pct     READ beta_pct     WRITE setBeta_pct             )
   //! \brief The origin.
   Q_PROPERTY(QString               origin       READ origin       WRITE setOrigin               )
   //! \brief The notes.
   Q_PROPERTY(QString               notes                 READ notes                 WRITE setNotes                )
   /**
    * \brief The \c Type.           ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    *
    *        See comment in \c model/Fermentable.h for \c grainGroup property for why this has to be
    *        \c std::optional<int>, not \c std::optional<Use>
    */
   Q_PROPERTY(std::optional<int>    type                  READ typeAsInt             WRITE setTypeAsInt            )
   /**
    * \brief The hop stability index in percent.  The Hop Stability Index (HSI) is defined as the percentage of hop
    *        alpha lost in 6 months of storage.  It may be related to the Hop Storage Index...
    *
    *        In BeerJSON, this is called `percent_lost`
    *
    *        ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    */
   Q_PROPERTY(std::optional<double> hsi_pct               READ hsi_pct               WRITE setHsi_pct              )
   //! \brief The list of substitutes.
   Q_PROPERTY(QString               substitutes           READ substitutes           WRITE setSubstitutes          )
   //! \brief Humulene as a percentage of total hop oil.      ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> humulene_pct          READ humulene_pct          WRITE setHumulene_pct         )
   //! \brief Caryophyllene as a percentage of total hop oil. ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> caryophyllene_pct     READ caryophyllene_pct     WRITE setCaryophyllene_pct    )
   //! \brief Cohumulone as a percentage of total hop oil.    ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> cohumulone_pct        READ cohumulone_pct        WRITE setCohumulone_pct       )
   //! \brief Myrcene as a percentage of total hop oil.       ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double> myrcene_pct           READ myrcene_pct           WRITE setMyrcene_pct          )

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   Q_PROPERTY(std::optional<double> totalOil_mlPer100g    READ totalOil_mlPer100g   WRITE setTotalOil_mlPer100g)
   Q_PROPERTY(std::optional<double> farnesene_pct         READ farnesene_pct        WRITE setFarnesene_pct     )
   Q_PROPERTY(std::optional<double> geraniol_pct          READ geraniol_pct         WRITE setGeraniol_pct      )
   Q_PROPERTY(std::optional<double> bPinene_pct           READ bPinene_pct          WRITE setBPinene_pct       )
   Q_PROPERTY(std::optional<double> linalool_pct          READ linalool_pct         WRITE setLinalool_pct      )
   Q_PROPERTY(std::optional<double> limonene_pct          READ limonene_pct         WRITE setLimonene_pct      )
   Q_PROPERTY(std::optional<double> nerol_pct             READ nerol_pct            WRITE setNerol_pct         )
   Q_PROPERTY(std::optional<double> pinene_pct            READ pinene_pct           WRITE setPinene_pct        )
   Q_PROPERTY(std::optional<double> polyphenols_pct       READ polyphenols_pct      WRITE setPolyphenols_pct   )
   Q_PROPERTY(std::optional<double> xanthohumol_pct       READ xanthohumol_pct      WRITE setXanthohumol_pct   )
   Q_PROPERTY(QString               producer              READ producer             WRITE setProducer          )
   Q_PROPERTY(QString               productId             READ productId            WRITE setProductId         )
   /**
    * \brief It might seem odd to store year as a string rather than, say, std::optional<unsigned int>, but this is
    *        deliberate and for two reasons.  Firstly BeerJSON treats it as a string.  Secondly, we don't want it
    *        formatted as a number when we display it.  Nobody writes "2,023" or "2 023" for the year 2023.
    */
   Q_PROPERTY(QString               year         READ year         WRITE setYear                 )


   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   double                alpha_pct         () const;
   std::optional<Form  > form              () const;
   std::optional<int   > formAsInt         () const;
   std::optional<double> beta_pct          () const;
   QString               origin            () const;
   QString               notes             () const;
   std::optional<Type>   type              () const;
   std::optional<int>    typeAsInt         () const;
   std::optional<double> hsi_pct           () const;
   QString               substitutes       () const;
   std::optional<double> humulene_pct      () const;
   std::optional<double> caryophyllene_pct () const;
   std::optional<double> cohumulone_pct    () const;
   std::optional<double> myrcene_pct       () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> totalOil_mlPer100g() const;
   std::optional<double> farnesene_pct     () const;
   std::optional<double> geraniol_pct      () const;
   std::optional<double> bPinene_pct       () const;
   std::optional<double> linalool_pct      () const;
   std::optional<double> limonene_pct      () const;
   std::optional<double> nerol_pct         () const;
   std::optional<double> pinene_pct        () const;
   std::optional<double> polyphenols_pct   () const;
   std::optional<double> xanthohumol_pct   () const;
   QString               producer          () const;
   QString               productId         () const;
   QString               year              () const;

   // Combined getters (all added for BeerJSON support)
   [[deprecated]] MassOrVolumeAmt       amountWithUnits      () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setAlpha_pct         (double                const   val);
   void setForm              (std::optional<Form  > const   val);
   void setFormAsInt         (std::optional<int   > const   val);
   void setBeta_pct          (std::optional<double> const   val);
   void setOrigin            (QString               const & val);
   void setNotes             (QString               const & val);
   void setType              (std::optional<Type>   const   val);
   void setTypeAsInt         (std::optional<int>    const   val);
   void setHsi_pct           (std::optional<double> const   val);
   void setSubstitutes       (QString               const & val);
   void setHumulene_pct      (std::optional<double> const   val);
   void setCaryophyllene_pct (std::optional<double> const   val);
   void setCohumulone_pct    (std::optional<double> const   val);
   void setMyrcene_pct       (std::optional<double> const   val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setTotalOil_mlPer100g(std::optional<double> const   val);
   void setFarnesene_pct     (std::optional<double> const   val);
   void setGeraniol_pct      (std::optional<double> const   val);
   void setBPinene_pct       (std::optional<double> const   val);
   void setLinalool_pct      (std::optional<double> const   val);
   void setLimonene_pct      (std::optional<double> const   val);
   void setNerol_pct         (std::optional<double> const   val);
   void setPinene_pct        (std::optional<double> const   val);
   void setPolyphenols_pct   (std::optional<double> const   val);
   void setXanthohumol_pct   (std::optional<double> const   val);
   void setProducer          (QString               const & val);
   void setProductId         (QString               const & val);
   void setYear              (QString               const   val);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   double                m_alpha_pct         ;
   std::optional<Form  > m_form              ;
   std::optional<double> m_beta_pct          ;
   QString               m_origin            ;
   std::optional<Type>   m_type              ;
   QString               m_notes             ;
   std::optional<double> m_hsi_pct           ;
   QString               m_substitutes       ;
   std::optional<double> m_humulene_pct      ;
   std::optional<double> m_caryophyllene_pct ;
   std::optional<double> m_cohumulone_pct    ;
   std::optional<double> m_myrcene_pct       ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> m_totalOil_mlPer100g;
   std::optional<double> m_farnesene_pct     ;
   std::optional<double> m_geraniol_pct      ;
   std::optional<double> m_bPinene_pct       ;
   std::optional<double> m_linalool_pct      ;
   std::optional<double> m_limonene_pct      ;
   std::optional<double> m_nerol_pct         ;
   std::optional<double> m_pinene_pct        ;
   std::optional<double> m_polyphenols_pct   ;
   std::optional<double> m_xanthohumol_pct   ;
   QString               m_producer          ;
   QString               m_productId         ;
   QString               m_year              ;
};

BT_DECLARE_METATYPES(Hop)

#endif
