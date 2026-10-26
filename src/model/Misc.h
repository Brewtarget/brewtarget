/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Misc.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#ifndef MODEL_MISC_H
#define MODEL_MISC_H
#pragma once

#include <QString>
#include <QSqlRecord>

#include "model/FolderPropertyBase.h"
#include "model/Ingredient.h"
#include "model/IngredientBase.h"
#include "model/IngredientAmount.h"
#include "model/Water.h"
#include "utils/EnumStringMapping.h"

class MiscCatalog;
class MiscEditor;
class StockPurchaseMisc;
class MiscItemDelegate;
class RecipeAdditionMisc;
class MiscSortFilterProxyModel;
class MiscTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Misc { inline BtStringConst const property{#property}; }
AddPropertyName(notes    )
AddPropertyName(producer )
AddPropertyName(productId)
AddPropertyName(type     )
AddPropertyName(useFor   )
AddPropertyName(waterAgentIsAcid     )
AddPropertyName(waterAgentPercentAcid)
AddPropertyName(waterAgentType       )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Misc
 *
 * \brief Model for a misc record in the database.
 */
class Misc : public Ingredient,
             public IngredientBase<Misc>,
             public FolderPropertyBase<Misc> {
   Q_OBJECT

   INGREDIENT_BASE_DECL(Misc)
   FOLDER_BASE_DECL(Misc)
   // See model/FolderPropertyBase.h for info, getters and setters for these properties
   Q_PROPERTY(int     containedInFolderId     READ containedInFolderId     WRITE setContainedInFolderId  )
   Q_PROPERTY(QString containedInFolderPath   READ containedInFolderPath   WRITE setContainedInFolderPath)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_notes    ();
   static QString localisedName_producer ();
   static QString localisedName_productId();
   static QString localisedName_type     ();
   static QString localisedName_useFor   ();
   static QString localisedName_waterAgentIsAcid     ();
   static QString localisedName_waterAgentPercentAcid();
   static QString localisedName_waterAgentType       ();

   /**
    * \brief The type of ingredient.
    */
   enum class Type {Spice     ,
                    Fining    ,
                    WaterAgent,
                    Herb      ,
                    Flavor    ,
                    Other     ,
                    // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
                    Wood      ,};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   /*!
    * \brief Mapping between \c Misc::Type and string values suitable for serialisation in DB, BeerJSON, etc (but
    *        \b not BeerXML)
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection, and we won't need to do things this way.)
    */
   static EnumStringMapping const typeStringMapping;

   /*!
    * \brief Localised names of \c Misc::Type values suitable for displaying to the end user
    */
   static EnumStringMapping const typeDisplayNames;

   /*!
    * \brief This is used only for water agents.
    *
    *        A water agent is a sub-category of miscellaneous ingredient (\c Misc) that modifies the water profile
    *        (\c Water).  In some ways a better name might be "water adjustment" but "water agent" seems to be the more
    *        widely-used term amongst brewers.  Many water agents are salts (hence the old name of this class was
    *        \c Salt) but, of course, some, such as lactic acid and phosphoric acid, are not.
    *
    *        In the past, we had this as a freestanding class, entirely independent of \c Misc.  But it now seems better
    *        to treat this as some extra attributes for \c Misc items of type \c Misc::Type::Water_Agent.  This aligns
    *        better with BeerXML and BeerJSON, and also better reflects the fact that a lot of the substance of a water
    *        agent in the application is hard-coded logic rather than database data.
    *
    *        Equally, although it might be strictly "correct" in object-oriented terms to make \c WaterAgent a subclass
    *        of \c Misc, given that it's only two fields, it's far simpler if we add the fields to \c Misc with a naming
    *        convention and some logic that they are only set and applicable for \c Misc::Type::Water_Agent.
    *
    *        Another simplification is that we list actual compounds here.  In a purist approach, we might classify by
    *        the underlying salt (eg Calcium Chloride = CaCl₂) and then have another field to differentiate between the
    *        anhydrous form (eg pure CaCl₂) and the hydrate form(s) (eg CaCl₂·H₂O, CaCl₂·2H₂O, CaCl₂·4H₂O, CaCl₂·6H₂O
    *        are the four hydrate forms of calcium chloride).  Most probably this would be done by proportion by storing
    *        what proportion of mass is the underlying salt.  However, brewing only uses a small number of salts and
    *        only a small proportion of the forms of those salts (eg for calcium chloride, only CaCl₂ and CaCl₂·2H₂O are
    *        of interest to brewers).  So, we lean the other way and enumerate the actual forms of the salts here.
    *
    *        We slightly break our normal naming convention to introduce an underscore before the anhydrous/hydrate part
    *        of the name, as it (IMHO) improves readability.
    *
    *
    *        calcium hydroxide : Calcium hydroxide Ca(OH)₂, also called slaked lime, is a strong base that can be used
    *                            to increase mash pH.  Generally, sodium bicarbonate (baking soda) is preferred for the
    *                            purpose of pH adjustment because it is less hazardous and equally effective.
    *
    *        magnesium chloride : Magnesium chloride (hexahydrate MgCl₂•6H₂O) is a salt that can be used to increase
    *                             the magnesium level of the brewing water or the wort.
    *
    *        sulfuric acid : Eg sulphuric acid 25% is available as a ready for use food grade acid to reduce alkalinity
    *                        and to increase desirable ions in product waters.  It is especially used in the brewing
    *                        industry as a liquor treatment.  (See
    *                        https://www.murphyandson.co.uk/wp-content/uploads/2020/07/Sulphuric-Acid-Rev-3.pdf.)
    *
    *        hydrochloric acid : Hydrochloric acid is not commonly used in home brewing.  Nonetheless, you can use it
    *                            to replace lactic or phosphoric acid to lower the pH of your mash.
    *                            If your source water is low in chloride, hydrochloric acid is the easiest modifier to
    *                            find and use.
    *                            In brewing, the concentration of the solution being used should be 0.1 M
    */
   enum class WaterAgentType {
      CalciumChloride_Anhydrous    , // <- CaCl₂
      CalciumChloride_Dihydrate    , // <- CaCl₂·2H₂O                 (NB: Other CaCl₂ hydrates not used in brewing)
      CalciumCarbonate             , // <- CaCO₃                      (NB: Hydrate forms not used in brewing)
      CalciumSulfate_Dihydrate     , // <- CaSO₄·2H₂O = gypsum        (NB: Anhydrous form not used in brewing)
      CalciumHydroxide             , // <- Ca(OH)₂ = pickling lime
      MagnesiumSulfate_Heptahydrate, // <- MgSO₄·7H₂O = Epsom Salt    (NB: Anhydrous form not used in brewing)
      MagnesiumChloride_Hexahydrate, // <- MgCl₂·6H₂O                 (NB: 11 other hydrates not used in brewing)
      SodiumChloride               , // <- NaCl  aka "regular" Salt
      SodiumBicarbonate            , // <- NaHCO₃
      LacticAcid                   , // <- CH₃CH(OH)CO₂H (extended formula) = C₃H₆O₃ (regular formula)
      PhosphoricAcid               , // <- H₃PO₄
      SulfuricAcid                 , // <- H₂SO₄
      HydrochloricAcid             , // <- HCl
      Other                        ,
   };

   // This allows us to store the above enum class in a QVariant
   Q_ENUM(WaterAgentType)

   /*!
    * \brief Mapping between \c WaterAgentType and string values suitable for serialisation in DB
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const waterAgentTypeStringMapping;

   /*!
    * \brief Localised names of \c WaterAgentType values suitable for displaying to the end user
    */
   static EnumStringMapping const waterAgentTypeDisplayNames;

   /**
    * \brief This is where we centrally define how \c Misc objects can be measured.
    */
   static constexpr auto validMeasures  = Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count;
   static constexpr auto defaultMeasure = Measurement::PhysicalQuantity::Mass;

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using CatalogClass              = MiscCatalog;
   using EditorClass               = MiscEditor;
   using StockPurchaseClass        = StockPurchaseMisc;
   using ItemDelegateClass         = MiscItemDelegate;
   using RecipeAdditionClass       = RecipeAdditionMisc;
   using SortFilterProxyModelClass = MiscSortFilterProxyModel;
   using TableModelClass           = MiscTableModel;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   explicit Misc(QString const & name = "");
   explicit Misc(NamedParameterBundle const & namedParameterBundle);
   Misc(Misc const & other);

   ~Misc() override;

   //=================================================== PROPERTIES ====================================================
   //! \brief The \c Type.
   Q_PROPERTY(Type type           READ type        WRITE setType     )
   //! \brief Short description of what the ingredient is used for
   Q_PROPERTY(QString useFor      READ useFor      WRITE setUseFor   )
   //! \brief The notes.
   Q_PROPERTY(QString notes       READ notes       WRITE setNotes    )
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   Q_PROPERTY(QString producer    READ producer    WRITE setProducer )
   Q_PROPERTY(QString productId   READ productId   WRITE setProductId)
   // All below migrated from Salt
   /**
    * \brief If this Misc is a water agent, this property holds what kind it is
    *
    *        See comment on Fermentable::GrainGroup for why optional enums need to be read and written as optional ints
    *        when we declare them as properties (essentially because you can't "just cast" between
    *        std::optional<WaterAgentType> and std::optional<int>
    */
   Q_PROPERTY(std::optional<int> waterAgentType   READ waterAgentTypeAsInt   WRITE setWaterAgentTypeAsInt)
   /**
    * \brief If this Misc is a water agent, this will tell you whether it is an acid or not.
    *        Deduced from \c waterAgentType.  (Returns \c false if \c waterAgentType is \c Other.)
    */
   Q_PROPERTY(bool waterAgentIsAcid   READ waterAgentIsAcid   STORED false)
   /**
    * \brief What percent is acid - valid only for acids (ie when \c waterAgentIsAcid() returns \c true) or for
    *        \c WaterAgentType::Other (eg a user-defined acid solution, such as citric acid, that we don't model
    *        explicitly).  This is the same rule as dotBeer uses.  See \c canHavePercentAcid.
    */
   Q_PROPERTY(std::optional<double> waterAgentPercentAcid   READ waterAgentPercentAcid   WRITE setWaterAgentPercentAcid)

   SUPPORT_NUM_RECIPES_USED_IN

   //=== Helper functions for water agents ===
   //! \brief It's useful in other places (eg WaterAgentEditor.cpp) to be able to check whether a WaterAgentType is an acid
   static bool isAcid(WaterAgentType const waterAgentType);
   /**
    * \brief Whether \c waterAgentPercentAcid is meaningful for a given \c WaterAgentType -- ie it's an acid, or it's
    *        \c WaterAgentType::Other (which might be some other acid).
    */
   static bool canHavePercentAcid(WaterAgentType const waterAgentType);
   static Measurement::PhysicalQuantity suggestedMeasureFor(WaterAgentType const waterAgentType);

   /**
    * \return Mass concentration (in parts per million) of \c ion for one gram of this \c Misc in one liter of water.
    *
    *         To get the actual concentration, multiply by the mass in grams and divide by the volume in liters.
    */
   template<Water::MineralIon ion>
   static double concentrationPerGramPerLiter_massConcPpm(WaterAgentType const waterAgentType);

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Type    type     () const;
   QString useFor   () const;
   QString notes    () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString producer () const;
   QString productId() const;
   // All below migrated from Salt
   std::optional<WaterAgentType> waterAgentType() const;
   std::optional<int>            waterAgentTypeAsInt() const;
   bool                          waterAgentIsAcid() const;
   std::optional<double>         waterAgentPercentAcid() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setType     (Type    const   val);
   void setUseFor   (QString const & val);
   void setNotes    (QString const & val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setProducer (QString const & val);
   void setProductId(QString const & val);
   // All below migrated from Salt
   void setWaterAgentType       (std::optional<WaterAgentType> const val);
   void setWaterAgentTypeAsInt  (std::optional<int>            const val);
   void setWaterAgentPercentAcid(std::optional<double>         const val);

signals:

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   Type    m_type     = Type::Other;
   QString m_useFor   = "";
   QString m_notes    = "";
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString m_producer = "";
   QString m_productId= "";
   // All below migrated from Salt
   std::optional<WaterAgentType> m_waterAgentType        = std::nullopt;
   std::optional<double>         m_waterAgentPercentAcid = std::nullopt;
};

BT_DECLARE_METATYPES(Misc)

#endif