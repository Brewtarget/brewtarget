/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Water.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef MODEL_WATER_H
#define MODEL_WATER_H
#pragma once

#include <QString>
#include <QSqlRecord>

#include "model/FolderPropertyBase.h"
#include "model/OutlineableNamedEntity.h"
#include "utils/EnumStringMapping.h"

class WaterCatalog;
class WaterEditor;
class WaterItemDelegate;
class WaterSortFilterProxyModel;
class WaterTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Water { inline BtStringConst const property{#property}; }
AddPropertyName(alkalinity_ppm  )
AddPropertyName(alkalinityAsHCO3)
AddPropertyName(bicarbonate_ppm )
AddPropertyName(calcium_ppm     )
AddPropertyName(carbonate_ppm   )
AddPropertyName(chloride_ppm    )
AddPropertyName(fluoride_ppm    )
AddPropertyName(iron_ppm        )
AddPropertyName(magnesium_ppm   )
AddPropertyName(nitrate_ppm     )
AddPropertyName(nitrite_ppm     )
AddPropertyName(notes           )
AddPropertyName(ph              )
AddPropertyName(potassium_ppm   )
AddPropertyName(sodium_ppm      )
AddPropertyName(sulfate_ppm     )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/*!
 * \class Water
 *
 * \brief Model for water records in the database.
 *
 *        Note that we do not support the BeerJSON "producer" field on water as it is not clear what it means!
 *
 *        In general, although it may initially seem tempting to treat water as an ingredient akin to Hops,
 *        Fermentables, etc, there are some significant differences:
 *           - For most brewers, water does not have stock or inventory
 *           - Although you might want to modify the water profile, including by using a mixture of reverse-osmosis
 *             water and tap-water, most brewers do not need to use multiple water profiles in a single recipe
 */
class Water : public OutlineableNamedEntity,
              public FolderPropertyBase<Water> {
   Q_OBJECT
   FOLDER_BASE_DECL(Water)
   // See model/FolderPropertyBase.h for info, getters and setters for these properties
   Q_PROPERTY(int containedInFolderId   READ containedInFolderId   WRITE setContainedInFolderId)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_alkalinity_ppm  ();
   static QString localisedName_alkalinityAsHCO3();
   static QString localisedName_bicarbonate_ppm ();
   static QString localisedName_calcium_ppm     ();
   static QString localisedName_carbonate_ppm   ();
   static QString localisedName_chloride_ppm    ();
   static QString localisedName_fluoride_ppm    ();
   static QString localisedName_iron_ppm        ();
   static QString localisedName_magnesium_ppm   ();
   static QString localisedName_nitrate_ppm     ();
   static QString localisedName_nitrite_ppm     ();
   static QString localisedName_notes           ();
   static QString localisedName_ph              ();
   static QString localisedName_potassium_ppm   ();
   static QString localisedName_sodium_ppm      ();
   static QString localisedName_sulfate_ppm     ();

   /**
    * \brief In the past, we have given these sorts of things names corresponding to their chemical symbols/formulae --
    *        eg Ca for calcium and HCO3 for bicarbonate, but this seems unsatisfactory.  It would be more accurate to
    *        write these things with subscripts, superscripts etc -- eg CO₃²⁻ for carbonate -- but that creates
    *        different problems.  (Last time I looked, there is still not universal compiler support for Unicode
    *        characters in identifiers.  And, even if there were, they would be hard to type.  Moreover, the superscript
    *        and subscript elements make variable names a bit hard read, unless you jack up your font size.)  So, we've
    *        gone with the English names instead.
    *
    * .:TBD:. If we could add CO3 to this list and move the enum to \c WaterAdjustment, it would help us template a bunch of very
    *         similar functions in WaterAdjustment.cpp and RecipeAdditionWaterAdjustmentTableModel.cpp.
    */
   enum class MineralIon {
      Bicarbonate, // HCO₃⁻
      Calcium    , // Ca²⁺
      Carbonate  , // CO₃²⁻
      Chloride   , // Cl⁻
      Copper     , // Cu²⁺
      Iron       , // Fe²⁺
      Magnesium  , // Mg²⁺
      Manganese  , // Mn²⁺
      Nitrate    , // NO₃⁻
      Nitrite    , // NO₂⁻
      Phosphate  , // PO₄³⁻
      Potassium  , // K⁺
      Sodium     , // Na⁺
      Sulfate    , // SO₄²⁻
      Zinc       , // Zn²⁺
   };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(MineralIon)

   static EnumStringMapping const ionStringMapping;
   static EnumStringMapping const ionDisplayNames;

   /**
    * \brief This is where we centrally define how \c Water objects can be measured.
    */
   static constexpr auto validMeasures  = Measurement::PhysicalQuantity::Volume;
   static constexpr auto defaultMeasure = Measurement::PhysicalQuantity::Volume;

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using CatalogClass              = WaterCatalog;
   using EditorClass               = WaterEditor;
   using ItemDelegateClass         = WaterItemDelegate;
   using SortFilterProxyModelClass = WaterSortFilterProxyModel;
   using TableModelClass           = WaterTableModel;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   explicit Water(QString name = "");
   explicit Water(NamedParameterBundle const & namedParameterBundle);
   Water(Water const & other);

   ~Water() override;

   // It is useful to be able to assign one Water to another - see eg editors/WaterEditor.cpp
   Water & operator=(Water other);

protected:
   /**
    * \brief Swap the contents of two Water objects - which provides an exception-safe way of implementing operator=
    *
    *        Note that we are overriding NamedEntity::swap, so we want to keep the same signature.
    */
   virtual void swap(NamedEntity & other) noexcept override;

public:
   // .:TODO:. On a base or target profile, bicarbonate and alkalinity cannot both be used. I'm gonna have fun figuring that out

   //=================================================== PROPERTIES ====================================================
   //! \brief The ppm of calcium.  Required in BeerXML and BeerJSON.
   Q_PROPERTY(double calcium_ppm     READ calcium_ppm     WRITE setCalcium_ppm)
   //! \brief The ppm of bicarbonate.  Required in BeerXML and BeerJSON.
   Q_PROPERTY(double bicarbonate_ppm READ bicarbonate_ppm WRITE setBicarbonate_ppm)
   //! \brief The ppm of sulfate.  Required in BeerXML and BeerJSON.
   Q_PROPERTY(double sulfate_ppm     READ sulfate_ppm     WRITE setSulfate_ppm)
   //! \brief The ppm of chloride.  Required in BeerXML and BeerJSON.
   Q_PROPERTY(double chloride_ppm    READ chloride_ppm    WRITE setChloride_ppm)
   //! \brief The ppm of sodium.  Required in BeerXML and BeerJSON.
   Q_PROPERTY(double sodium_ppm      READ sodium_ppm      WRITE setSodium_ppm)
   //! \brief The ppm of magnesium.  Required in BeerXML and BeerJSON.
   Q_PROPERTY(double magnesium_ppm   READ magnesium_ppm   WRITE setMagnesium_ppm)
   //! \brief The pH.  NB: Optional in both BeerXML and BeerJSON.
   Q_PROPERTY(std::optional<double> ph              READ ph              WRITE setPh)
   //! \brief The residual alkalinity.  Units are ppm.  NB: Not part of BeerXML or BeerJSON.
   Q_PROPERTY(std::optional<double> alkalinity_ppm  READ alkalinity_ppm  WRITE setAlkalinity_ppm)
   //! \brief The notes.
   Q_PROPERTY(QString notes          READ notes           WRITE setNotes)
   //! \brief is the alkalinity measured as HCO3 (bicarbonate) or CO3 (carbonate)?  NB: Not part of BeerXML or BeerJSON
   Q_PROPERTY(bool   alkalinityAsHCO3 READ alkalinityAsHCO3 WRITE setAlkalinityAsHCO3)

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   Q_PROPERTY(std::optional<double> carbonate_ppm  READ carbonate_ppm    WRITE setCarbonate_ppm)
   Q_PROPERTY(std::optional<double> potassium_ppm  READ potassium_ppm    WRITE setPotassium_ppm)
   Q_PROPERTY(std::optional<double> iron_ppm       READ iron_ppm         WRITE setIron_ppm     )
   Q_PROPERTY(std::optional<double> nitrate_ppm    READ nitrate_ppm      WRITE setNitrate_ppm  )
   Q_PROPERTY(std::optional<double> nitrite_ppm    READ nitrite_ppm      WRITE setNitrite_ppm  )
   Q_PROPERTY(std::optional<double> fluoride_ppm   READ fluoride_ppm     WRITE setFluoride_ppm )

   SUPPORT_NUM_RECIPES_USED_IN

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   double                calcium_ppm     () const;
   double                bicarbonate_ppm () const;
   double                sulfate_ppm     () const;
   double                chloride_ppm    () const;
   double                sodium_ppm      () const;
   double                magnesium_ppm   () const;
   std::optional<double> ph              () const;
   std::optional<double> alkalinity_ppm  () const;
   QString               notes           () const;
   bool                  alkalinityAsHCO3() const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> carbonate_ppm () const;
   std::optional<double> potassium_ppm () const;
   std::optional<double> iron_ppm      () const;
   std::optional<double> nitrate_ppm   () const;
   std::optional<double> nitrite_ppm   () const;
   std::optional<double> fluoride_ppm  () const;

   //! Get the concentration (or, strictly speaking, the mass fraction) of the specified ion
   std::optional<double> ionConcentration_ppm(MineralIon const ion) const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setCalcium_ppm     (double                const   val);
   void setSulfate_ppm     (double                const   val);
   void setBicarbonate_ppm (double                const   val);
   void setChloride_ppm    (double                const   val);
   void setSodium_ppm      (double                const   val);
   void setMagnesium_ppm   (double                const   val);
   void setPh              (std::optional<double> const   val);
   void setAlkalinity_ppm  (std::optional<double> const   val);
   void setNotes           (QString               const & val);
   void setAlkalinityAsHCO3(bool                  const   val);

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setCarbonate_ppm   (std::optional<double> const   val);
   void setPotassium_ppm   (std::optional<double> const   val);
   void setIron_ppm        (std::optional<double> const   val);
   void setNitrate_ppm     (std::optional<double> const   val);
   void setNitrite_ppm     (std::optional<double> const   val);
   void setFluoride_ppm    (std::optional<double> const   val);

   //! Set the concentration (or, strictly speaking, the mass fraction) of the specified ion
   void setIonConcentration_ppm(MineralIon const ion, std::optional<double> const val);

signals:

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   double                m_calcium_ppm       ;
   double                m_bicarbonate_ppm   ;
   double                m_sulfate_ppm       ;
   double                m_chloride_ppm      ;
   double                m_sodium_ppm        ;
   double                m_magnesium_ppm     ;
   std::optional<double> m_ph                ;
   std::optional<double> m_alkalinity_ppm    ;
   QString               m_notes             ;
   bool                  m_alkalinity_as_hco3;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> m_carbonate_ppm   ;
   std::optional<double> m_potassium_ppm   ;
   std::optional<double> m_iron_ppm        ;
   std::optional<double> m_nitrate_ppm     ;
   std::optional<double> m_nitrite_ppm     ;
   std::optional<double> m_fluoride_ppm    ;
};

BT_DECLARE_METATYPES(Water)

static_assert(HasFolder<Water>);
static_assert(!HasNoFolder<Water>);

#endif