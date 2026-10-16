/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Hop.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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

#include "model/FolderPropertyBase.h"
#include "model/Ingredient.h"
#include "model/IngredientBase.h"
#include "model/IngredientAmount.h"
#include "utils/EnumStringMapping.h"

class HopCatalog;
class HopEditor;
class StockPurchaseHop;
class HopItemDelegate;
class RecipeAdditionHop;
class HopSortFilterProxyModel;
class HopTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Hop { inline BtStringConst const property{#property}; }
AddPropertyName(alphaMax_pct         )
AddPropertyName(alphaMin_pct         )
AddPropertyName(alpha_pct            )
AddPropertyName(bPinene_pct          )
AddPropertyName(betaMax_pct          )
AddPropertyName(betaMin_pct          )
AddPropertyName(beta_pct             )
AddPropertyName(caryophyllene_pct    )
AddPropertyName(cohumulone_pct       )
AddPropertyName(farnesene_pct        )
AddPropertyName(form                 )
AddPropertyName(geraniol_pct         )
AddPropertyName(humulene_pct         )
AddPropertyName(limonene_pct         )
AddPropertyName(linalool_pct         )
AddPropertyName(myrcene_pct          )
AddPropertyName(nerol_pct            )
AddPropertyName(notes                )
AddPropertyName(origin               )
AddPropertyName(pinene_pct           )
AddPropertyName(polyphenols_pct      )
AddPropertyName(producer             )
AddPropertyName(productId            )
AddPropertyName(sixMonthAlphaLoss_pct)
AddPropertyName(substitutes          )
AddPropertyName(totalOil_mlPer100g   )
AddPropertyName(type                 )
AddPropertyName(xanthohumol_pct      )
AddPropertyName(year                 )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Hop
 *
 * \brief Model class for a hop record in the database.
 *
 *        A hop record can represent either:
 *           - a general type of Hop - eg "East Kent Goldings", which will have an alpha acid range; or
 *           - a specific form and harvest - eg "Pellet 2020 harvest", which will have a single alpha acid value eg 6.0%
 *
 *        Strictly, we ought to have two separate classes -- eg HopVariety and DriedHop -- but this does not reflect the
 *        structure of BeerXML and BeerJSON.
 *
 *        Probably we could do some of this via StockPurchase.
 */
class Hop : public Ingredient,
            public IngredientBase<Hop>,
            public FolderPropertyBase<Hop> {
   Q_OBJECT

   INGREDIENT_BASE_DECL(Hop)
   FOLDER_BASE_DECL(Hop)
   // See model/FolderPropertyBase.h for info, getters and setters for these properties
   Q_PROPERTY(int     containedInFolderId     READ containedInFolderId     WRITE setContainedInFolderId  )
   Q_PROPERTY(QString containedInFolderPath   READ containedInFolderPath   WRITE setContainedInFolderPath)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_alphaMax_pct         ();
   static QString localisedName_alphaMin_pct         ();
   static QString localisedName_alpha_pct            ();
   static QString localisedName_bPinene_pct          ();
   static QString localisedName_betaMax_pct          ();
   static QString localisedName_betaMin_pct          ();
   static QString localisedName_beta_pct             ();
   static QString localisedName_caryophyllene_pct    ();
   static QString localisedName_cohumulone_pct       ();
   static QString localisedName_farnesene_pct        ();
   static QString localisedName_form                 ();
   static QString localisedName_geraniol_pct         ();
   static QString localisedName_humulene_pct         ();
   static QString localisedName_limonene_pct         ();
   static QString localisedName_linalool_pct         ();
   static QString localisedName_myrcene_pct          ();
   static QString localisedName_nerol_pct            ();
   static QString localisedName_notes                ();
   static QString localisedName_origin               ();
   static QString localisedName_pinene_pct           ();
   static QString localisedName_polyphenols_pct      ();
   static QString localisedName_producer             ();
   static QString localisedName_productId            ();
   static QString localisedName_sixMonthAlphaLoss_pct();
   static QString localisedName_substitutes          ();
   static QString localisedName_totalOil_mlPer100g   ();
   static QString localisedName_type                 ();
   static QString localisedName_xanthohumol_pct      ();
   static QString localisedName_year                 ();

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
   using StockPurchaseClass        = StockPurchaseHop;
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

   explicit Hop(QString const & name = "");
   explicit Hop(NamedParameterBundle const & namedParameterBundle);
   Hop(Hop const & other);

   ~Hop() override;

   //=================================================== PROPERTIES ====================================================
   /**
    * \brief \c alpha_pct is the actual alpha acid content of the specific year's harvest (and batch) of this type of
    *        hop.  ⮜⮜⮜ Required in BeerJSON and BeerXML ⮞⮞⮞
    *
    *        \c alphaMax_pct and alphaMin_pct give the typical range of alpha acid for this type of hop -- and are not
    *        part of BeerXML or BeerJSON.
    */
   Q_PROPERTY(double                alpha_pct       READ alpha_pct       WRITE setAlpha_pct   )
   Q_PROPERTY(std::optional<double> alphaMin_pct    READ alphaMin_pct    WRITE setAlphaMin_pct)
   Q_PROPERTY(std::optional<double> alphaMax_pct    READ alphaMax_pct    WRITE setAlphaMax_pct)

   /**
    * \brief The \c Form.                ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    *
    *        See comment in \c model/Fermentable.h for \c grainGroup property for why this has to be
    *        \c std::optional<int>, not \c std::optional<Use>
    */
   Q_PROPERTY(std::optional<int>    form         READ formAsInt    WRITE setFormAsInt            )

   /**
    * \brief \c beta_pct is the actual beta acid content of the specific year's harvest (and batch) of this type of
    *        hop.  ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    *
    *        \c betaMax_pct and betaMin_pct give the typical range of beta acid for this type of hop -- and are not part
    *        of BeerXML or BeerJSON.
    */
   Q_PROPERTY(std::optional<double> beta_pct      READ beta_pct      WRITE setBeta_pct   )
   Q_PROPERTY(std::optional<double> betaMin_pct   READ betaMin_pct   WRITE setBetaMin_pct)
   Q_PROPERTY(std::optional<double> betaMax_pct   READ betaMax_pct   WRITE setBetaMax_pct)

   //! \brief Country of origin for the hop variety.
   Q_PROPERTY(QString               origin       READ origin       WRITE setOrigin               )
   //! \brief The notes.
   Q_PROPERTY(QString               notes        READ notes        WRITE setNotes                )
   /**
    * \brief The \c Type.           ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    *
    *        See comment in \c model/Fermentable.h for \c grainGroup property for why this has to be
    *        \c std::optional<int>, not \c std::optional<Use>
    */
   Q_PROPERTY(std::optional<int>    type         READ typeAsInt    WRITE setTypeAsInt            )
   /**
    * \brief The percentage of hop alpha acid lost in 6 months of storage.
    *
    *        Note that this is percentage and not percentage points.  So, eg, if a hop starts out with 4.0% alpha_pct
    *        and has 25% sixMonthAlphaLoss_pct, then we would expect its alpha acid to be 3.0% after six months in
    *        storage.
    *
    *        BeerXML calls this HSI for "Hop Stability Index" -- see eg
    *        https://hopdoctor.co.uk/2024/12/02/understanding-the-hop-stability-index-key-for-smaller-scale-brewers/.
    *
    *        However, we changed the name (a) because "Hop Stability Index" does not appear to be a massively widely
    *        used term and (b) to avoid confusion with Hop Storage Index, which is derived from ultraviolet absorbance
    *        measured with a spectrophotometer (HSI = absorption at 275 nm divided by absorption at 325 nm, or
    *        A275/A325).
    *
    *        In BeerJSON, this is called `percent_lost`.
    *
    *        TODO: At the moment we do not use this in any calculations, but it would be neat to be able to do so.
    *
    *        ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    */
   Q_PROPERTY(std::optional<double> sixMonthAlphaLoss_pct   READ sixMonthAlphaLoss_pct   WRITE setSixMonthAlphaLoss_pct)
   //! \brief Alternate hop varieties that can be used in place of this hop variety
   Q_PROPERTY(QString               substitutes           READ substitutes           WRITE setSubstitutes          )
   /**
    * \brief Humulene as a percentage of total hop oil.
    *
    *        Brad Smith writes "The second largest hop oil by percentage, humulene makes up between 12% and 50% of total
    *        hop oils.  Humulene is responsible for earthy and spicy flavors found in traditional noble hops.  While
    *        humulene has a higher boiling point (around 106°C), it is very volatile and hydrophobic, so it is still
    *        best used in the whirlpool or dry hopping.  Humulene is also easily oxidized, though the oxidized
    *        humulene-epoxide III play a major role in the flavor of varieties like Hallertauer Mittelfrüh."  (See
    *        https://beersmith.com/blog/2024/12/19/major-aromatic-hop-oils-in-beer-brewing/)
    *
    *        ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    */
   Q_PROPERTY(std::optional<double> humulene_pct          READ humulene_pct          WRITE setHumulene_pct         )
   /**
    * \brief Caryophyllene (more formally β-caryophyllene or BCP) as a percentage of total hop oil.
    *
    *        Brad Smith writes "Caryophyllene makes up between 6% and 15% of total hop oils in most varieties.  It is a
    *        major compound in many aromatic plants including cloves, cannabis, rosemary, and hops.  It is also a
    *        significant aromatic compound in black pepper.  The aroma is described as woody, earthy, and peppery though
    *        it also has a strong herbal component.  The spicy, woody aroma is often evident when you crush dried hops
    *        in your hands.  Many English hop varieties such as East Kent Goldings have the largest percentage of
    *        caryphyllene giving them a woody, earthy finish.  While not quite as volatile as myrcene, caryophyllene it
    *        will boil quickly and is best used in the whirlpool or as a dry hop."  (See
    *        https://beersmith.com/blog/2024/12/19/major-aromatic-hop-oils-in-beer-brewing/)
    *
    *        ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    */
   Q_PROPERTY(std::optional<double> caryophyllene_pct     READ caryophyllene_pct     WRITE setCaryophyllene_pct    )
   /**
    * \brief Cohumulone as a percentage of total hop alpha acids.
    *
    *        Per (English translation of) https://fr.wikipedia.org/wiki/Cohumulone:
    *
    *           Cohumulone is one of the three hop-derived alpha acids that provide beer with most of its bitterness.
    *           Some brewers believe that hop varieties with a high cohumulone content produce a harsher bitterness.
    *           Cohumulone generally accounts for between 20% and 50% of a hop’s alpha acid content.
    *
    *           Cohumulone, like other known alpha acids (the main ones being humulone and adhumulone), is produced by
    *           the breakdown of lupulin (derived from hops).   However, it has greater polarity and better solubility
    *           in water than other alpha acids.  In water, its isomerization produces two compounds: cis-isocohumulone
    *           and trans-isocohumulone, which appear to impart a harsher bitterness than that of isohumulones.
    *
    *        ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    */
   Q_PROPERTY(std::optional<double> cohumulone_pct        READ cohumulone_pct        WRITE setCohumulone_pct       )
   /**
    * \brief Myrcene as a percentage of total hop oil.
    *
    *        Brad Smith writes "This is the most significant hop oil, making up from 30-60% of total hop oils in most
    *        varieties.  Myrcene is often described as the “fresh hop oil” and has flavors and aroma varying from
    *        herbaceous to resinous, green, balsamic, and slightly metallic.  It is a major component in many hop
    *        varieties from the Pacific Northwest, and is therefore a critical oil in IPAs.  Cascade, for example has a
    *        myrcene content of nearly 60%.  Myrcene has a low boiling point and is highly volatile.  In fact it will
    *        virtually disappear in most boil additions, and even tends to be volatile in higher temperature
    *        whirlpool/steeping additions.  It also oxidizes very rapidly, and is probably best used in dry hop
    *        applications."  (See https://beersmith.com/blog/2024/12/19/major-aromatic-hop-oils-in-beer-brewing/)
    *
    *        ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
    */
   Q_PROPERTY(std::optional<double> myrcene_pct           READ myrcene_pct           WRITE setMyrcene_pct          )

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞

   Q_PROPERTY(std::optional<double> totalOil_mlPer100g    READ totalOil_mlPer100g   WRITE setTotalOil_mlPer100g)

   /**
    *\brief Farnesene as a percentage of total hop oil.
    *
    *       Brad Smith writes "Farnesene is a volatile hop oil that is loosely associated with certain noble hops like
    *       Saaz, but can make up 10-20% of total hop oils in other noble hops like Tettnanger, Styrian hops and many
    *       American varieties.  It has a woody, herbal, citrus aroma and sometimes also takes on a floral character.
    *       Because it is very volatile, it is rarely found in the finished beer unless it has been dry hopped.  [It] is
    *       often associated with varieties that take on a noble hop character, so if you see a hop high in Farnesene it
    *       likely has the aroma profile similar to many noble hops."  (See
    *       https://beersmith.com/blog/2024/12/19/major-aromatic-hop-oils-in-beer-brewing/)
    */
   Q_PROPERTY(std::optional<double> farnesene_pct         READ farnesene_pct        WRITE setFarnesene_pct     )
   /**
    * \brief Geraniol as a percentage of total hop oil.
    *
    *        Brad Smith writes "Geraniol is an aromatic oil with a floral, sweet or rose aroma found in many American,
    *        New Zealand and Australian hops.  It is used in rose oil, palmarosa oil, and citronella oil, and is a
    *        component of common spices like lavender, nutmeg, coriander, rose and blackberry.  Many brewers also will
    *        add hops with geraniol during the tail end of active fermentation to promote tropical and citrus fruit
    *        flavors.  Hops with high geraniol have become popular for creating tropical/fruity IPA styles."  (See
    *        https://beersmith.com/blog/2024/12/19/major-aromatic-hop-oils-in-beer-brewing/)
    */
   Q_PROPERTY(std::optional<double> geraniol_pct          READ geraniol_pct         WRITE setGeraniol_pct      )
   /**
    * \brief β-Pinene as a percentage of total hop oil.
    *
    *        Brad Smith writes "Though not a major hop oil, β-Pinene has a green-woody and piney aroma.  It is a
    *        monoterpene, commonly found in many plants and it is soluble in alcohol but not water."  (See
    *        https://beersmith.com/blog/2024/12/19/major-aromatic-hop-oils-in-beer-brewing/)
    */
   Q_PROPERTY(std::optional<double> bPinene_pct           READ bPinene_pct          WRITE setBPinene_pct       )
   /**
    * \brief Linalool as a percentage of total hop oil.
    *
    *        Brad Smith writes "Related closely to Myrcene chemically, linalool has a floral aroma that smells like
    *        rose, orange, citrus or lavender.  It is more soluble in wort than myrcene, and also has a lower flavor
    *        threshold so it can be potent despite the small fraction it makes up of total hop oils.  Like geraniol,
    *        linalool is associated with many of the tropical, floral and citrus flavors that are popular in fruity and
    *        tropical IPAs."  (See https://beersmith.com/blog/2024/12/19/major-aromatic-hop-oils-in-beer-brewing/)
    */
   Q_PROPERTY(std::optional<double> linalool_pct          READ linalool_pct         WRITE setLinalool_pct      )
   /**
    * \brief Limonene as a percentage of total hop oil.
    *
    *        Limonene is a relatively stable monoterpene occurring naturally in citrus fruit peels, hops and numerous
    *        coniferous and broadleaved trees.  Limonene delivers a powerful citrus character, and is associated with
    *        punchy citrus based hops, such as Citra, Simcoe and Centennial.
    */
   Q_PROPERTY(std::optional<double> limonene_pct          READ limonene_pct         WRITE setLimonene_pct      )

   /**
    * \brief Nerol as a percentage of total hop oil.
    *
    *        Nerol is an isomer of geraniol.  Like geraniol, it has a sweet rose odor but it is considered to be
    *        fresher.  Its aroma is described as sweet, natural, neroli, citrus, magnolia, fresh, floral, green, lemon,
    *        and lime.
    */
   Q_PROPERTY(std::optional<double> nerol_pct             READ nerol_pct            WRITE setNerol_pct         )

   /**
    * \brief  α-Pinene as a percentage of total hop oil.
    *
    *         Alpha-Pinene has a distinctly woody, herbal scent and can be found in cone-bearing trees and herbs like
    *         rosemary.
    *
    *         TODO: We should raname this to aPinene_pct
    */
   Q_PROPERTY(std::optional<double> pinene_pct            READ pinene_pct           WRITE setPinene_pct        )

   /**
    * \brief Polyphenols as a percentage of hop mass.
    *
    *        Polyphenols are a large family of naturally occurring phenols with "several hydroxyl groups on aromatic
    *        rings".  They include four principal classes: phenolic acids, flavonoids, stilbenes, and lignans.
    *        Polyphenols in hops contribute to the astringency and fullness of beer's taste, enhancing its overall
    *        flavor profile.  They do not have a direct aroma but can affect beer taste by contributing to its
    *        mouthfeel, astringency, and haze formation.  Additionally, they play a role in beer stability, as they have
    *        antioxidant properties that can help prevent the oxidation of flavor compounds.
    */
   Q_PROPERTY(std::optional<double> polyphenols_pct       READ polyphenols_pct      WRITE setPolyphenols_pct   )

   /**
    * \brief Xanthohumol as a percentage of hop mass.
    *
    *        Xanthohumol is a natural flavonoid found in the female inflorescences of hops.  Its impact on beer taste is not as significant that of alpha acids or essential oils, but it may contribute to the overall complexity and subtle nuances in beer flavors.  (Xanthohumol has also been the subject of considerable research for its potential health benefits, but typically in quantities far exceeding any amount that could be obtained by ordinary dietary intake.)
    */
   Q_PROPERTY(std::optional<double> xanthohumol_pct       READ xanthohumol_pct      WRITE setXanthohumol_pct   )
   Q_PROPERTY(QString               producer              READ producer             WRITE setProducer          )
   Q_PROPERTY(QString               productId             READ productId            WRITE setProductId         )
   /**
    * \brief It might seem odd to store year as a string rather than, say, std::optional<unsigned int>, but this is
    *        deliberate and for two reasons.  Firstly BeerJSON treats it as a string.  Secondly, we don't want it
    *        formatted as a number when we display it.  Nobody writes "2,023" or "2 023" for the year 2023.
    */
   Q_PROPERTY(QString               year         READ year         WRITE setYear                 )

   SUPPORT_NUM_RECIPES_USED_IN

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   double                alpha_pct            () const;
   std::optional<double> alphaMin_pct         () const;
   std::optional<double> alphaMax_pct         () const;
   std::optional<Form  > form                 () const;
   std::optional<int   > formAsInt            () const;
   std::optional<double> beta_pct             () const;
   std::optional<double> betaMin_pct          () const;
   std::optional<double> betaMax_pct          () const;
   QString               origin               () const;
   QString               notes                () const;
   std::optional<Type>   type                 () const;
   std::optional<int>    typeAsInt            () const;
   std::optional<double> sixMonthAlphaLoss_pct() const;
   QString               substitutes          () const;
   std::optional<double> humulene_pct         () const;
   std::optional<double> caryophyllene_pct    () const;
   std::optional<double> cohumulone_pct       () const;
   std::optional<double> myrcene_pct          () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> totalOil_mlPer100g   () const;
   std::optional<double> farnesene_pct        () const;
   std::optional<double> geraniol_pct         () const;
   std::optional<double> bPinene_pct          () const;
   std::optional<double> linalool_pct         () const;
   std::optional<double> limonene_pct         () const;
   std::optional<double> nerol_pct            () const;
   std::optional<double> pinene_pct           () const;
   std::optional<double> polyphenols_pct      () const;
   std::optional<double> xanthohumol_pct      () const;
   QString               producer             () const;
   QString               productId            () const;
   QString               year                 () const;

   // Combined getters (all added for BeerJSON support)
   [[deprecated]] MassOrVolumeAmt       amountWithUnits      () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setAlpha_pct            (double                const   val);
   void setAlphaMin_pct         (std::optional<double> const   val);
   void setAlphaMax_pct         (std::optional<double> const   val);
   void setForm                 (std::optional<Form  > const   val);
   void setFormAsInt            (std::optional<int   > const   val);
   void setBeta_pct             (std::optional<double> const   val);
   void setBetaMin_pct          (std::optional<double> const   val);
   void setBetaMax_pct          (std::optional<double> const   val);
   void setOrigin               (QString               const & val);
   void setNotes                (QString               const & val);
   void setType                 (std::optional<Type>   const   val);
   void setTypeAsInt            (std::optional<int>    const   val);
   void setSixMonthAlphaLoss_pct(std::optional<double> const   val);
   void setSubstitutes          (QString               const & val);
   void setHumulene_pct         (std::optional<double> const   val);
   void setCaryophyllene_pct    (std::optional<double> const   val);
   void setCohumulone_pct       (std::optional<double> const   val);
   void setMyrcene_pct          (std::optional<double> const   val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setTotalOil_mlPer100g   (std::optional<double> const   val);
   void setFarnesene_pct        (std::optional<double> const   val);
   void setGeraniol_pct         (std::optional<double> const   val);
   void setBPinene_pct          (std::optional<double> const   val);
   void setLinalool_pct         (std::optional<double> const   val);
   void setLimonene_pct         (std::optional<double> const   val);
   void setNerol_pct            (std::optional<double> const   val);
   void setPinene_pct           (std::optional<double> const   val);
   void setPolyphenols_pct      (std::optional<double> const   val);
   void setXanthohumol_pct      (std::optional<double> const   val);
   void setProducer             (QString               const & val);
   void setProductId            (QString               const & val);
   void setYear                 (QString               const & val);

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   double                m_alpha_pct             = 0.0;
   std::optional<double> m_alphaMin_pct          = std::nullopt;
   std::optional<double> m_alphaMax_pct          = std::nullopt;
   std::optional<Form  > m_form                  = std::nullopt;
   std::optional<double> m_beta_pct              = std::nullopt;
   std::optional<double> m_betaMin_pct           = std::nullopt;
   std::optional<double> m_betaMax_pct           = std::nullopt;
   QString               m_origin                = "";
   std::optional<Type>   m_type                  = std::nullopt;
   QString               m_notes                 = "";
   std::optional<double> m_sixMonthAlphaLoss_pct = std::nullopt;
   QString               m_substitutes           = "";
   std::optional<double> m_humulene_pct          = std::nullopt;
   std::optional<double> m_caryophyllene_pct     = std::nullopt;
   std::optional<double> m_cohumulone_pct        = std::nullopt;
   std::optional<double> m_myrcene_pct           = std::nullopt;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> m_totalOil_mlPer100g    = std::nullopt;
   std::optional<double> m_farnesene_pct         = std::nullopt;
   std::optional<double> m_geraniol_pct          = std::nullopt;
   std::optional<double> m_bPinene_pct           = std::nullopt;
   std::optional<double> m_linalool_pct          = std::nullopt;
   std::optional<double> m_limonene_pct          = std::nullopt;
   std::optional<double> m_nerol_pct             = std::nullopt;
   std::optional<double> m_pinene_pct            = std::nullopt;
   std::optional<double> m_polyphenols_pct       = std::nullopt;
   std::optional<double> m_xanthohumol_pct       = std::nullopt;
   QString               m_producer              = "";
   QString               m_productId             = "";
   QString               m_year                  = "";
};

BT_DECLARE_METATYPES(Hop)

#endif