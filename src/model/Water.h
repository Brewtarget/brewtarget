/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Water.h is part of Brewtarget, and is copyright the following authors 2009-2025:
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

#include "model/FolderBase.h"
#include "model/OutlineableNamedEntity.h"
#include "utils/EnumStringMapping.h"

class WaterCatalog;
class WaterEditor;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Water { inline BtStringConst const property{#property}; }
AddPropertyName(alkalinity_ppm  )
AddPropertyName(alkalinityAsHCO3)
///AddPropertyName(amount          )
AddPropertyName(bicarbonate_ppm )
AddPropertyName(calcium_ppm     )
AddPropertyName(carbonate_ppm   )
AddPropertyName(chloride_ppm    )
AddPropertyName(fluoride_ppm    )
AddPropertyName(iron_ppm        )
AddPropertyName(magnesium_ppm   )
AddPropertyName(mashRo_pct      )
AddPropertyName(nitrate_ppm     )
AddPropertyName(nitrite_ppm     )
AddPropertyName(notes           )
AddPropertyName(ph              )
AddPropertyName(potassium_ppm   )
AddPropertyName(sodium_ppm      )
AddPropertyName(spargeRo_pct    )
AddPropertyName(sulfate_ppm     )
AddPropertyName(type            )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

class RecipeUseOfWater;

/*!
 * \class Water
 *
 * \brief Model for water records in the database.
 *
 *        Note that we do not support the BeerJSON "producer" field on water as it is not clear what it means!
 */
class Water : public OutlineableNamedEntity,
              public FolderBase<Water> {
   Q_OBJECT
   FOLDER_BASE_DECL(Water)
   // See model/FolderBase.h for info, getters and setters for these properties
   Q_PROPERTY(QString folderPath        READ folderPath        WRITE setFolderPath)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /**
    * \brief
    *        NOTE: This does not appear to be part of BeerXML or BeerJSON.  TBD I am not 100% certain that we need it.
    */
   enum class Type {
      Base  ,
      Target,
   };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   /*!
    * \brief Mapping between \c Water::Type and string values suitable for serialisation in DB
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const typeStringMapping;

   /*!
    * \brief Localised names of \c Water::Type values suitable for displaying to the end user
    */
   static EnumStringMapping const typeDisplayNames;

   /**
    * \brief
    *
    * .:TBD:. If we could add CO3 to this list and move the enum to \c Salt, it would help us template a bunch of very
    *         similar functions in Salt.cpp and RecipeAdjustmentSaltTableModel.cpp.
    */
   enum class Ion {
      Ca  ,
      Cl  ,
      HCO3,
      Mg  ,
      Na  ,
      SO4 ,
   };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Ion)

   static EnumStringMapping const ionStringMapping;
   static EnumStringMapping const ionDisplayNames;

   //
   // These aliases make it easier to template a number of functions that are essentially the same for a number of
   // different NamedEntity subclasses.
   //
   // Although Water is a bit different from other "ingredients" (eg no inventory), RecipeAdditionClass is still helpful
   // for templating functions where it is valid to create RecipeUseOfWater as a RecipeAddition class.
   //
   using CatalogClass        = WaterCatalog;
   using EditorClass         = WaterEditor;
   using RecipeAdditionClass = RecipeUseOfWater;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   Water(QString name = "");
   Water(NamedParameterBundle const & namedParameterBundle);
   Water(Water const & other);

   virtual ~Water();

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
   /**
    * \brief What kind of water is this.  NB: Not part of BeerXML or BeerJSON.
    *
    *        See comment in \c model/Fermentable.h for \c grainGroup property for why this has to be
    *        \c std::optional<int>, not \c std::optional<Type>
    */
   Q_PROPERTY(std::optional<int> type READ typeAsInt        WRITE setTypeAsInt)
   //! \brief percent of the mash water that is RO (reverse osmosis)   NB: Not part of BeerXML or BeerJSON
   Q_PROPERTY(std::optional<double> mashRo_pct       READ mashRo_pct       WRITE setMashRo_pct)
   //! \brief percent of the sparge water that is RO (reverse osmosis)   NB: Not part of BeerXML or BeerJSON
   Q_PROPERTY(std::optional<double> spargeRo_pct     READ spargeRo_pct     WRITE setSpargeRo_pct)
   //! \brief is the alkalinity measured as HCO3 (bicarbonate) or CO3 (carbonate)?  NB: Not part of BeerXML or BeerJSON
   Q_PROPERTY(bool   alkalinityAsHCO3 READ alkalinityAsHCO3 WRITE setAlkalinityAsHCO3)

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   Q_PROPERTY(std::optional<double> carbonate_ppm  READ carbonate_ppm    WRITE setCarbonate_ppm)
   Q_PROPERTY(std::optional<double> potassium_ppm  READ potassium_ppm    WRITE setPotassium_ppm)
   Q_PROPERTY(std::optional<double> iron_ppm       READ iron_ppm         WRITE setIron_ppm     )
   Q_PROPERTY(std::optional<double> nitrate_ppm    READ nitrate_ppm      WRITE setNitrate_ppm  )
   Q_PROPERTY(std::optional<double> nitrite_ppm    READ nitrite_ppm      WRITE setNitrite_ppm  )
   Q_PROPERTY(std::optional<double> fluoride_ppm   READ fluoride_ppm     WRITE setFluoride_ppm )

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
   std::optional<Type>   type            () const;
   std::optional<int>    typeAsInt       () const;
   std::optional<double> mashRo_pct      () const;
   std::optional<double> spargeRo_pct    () const;
   bool                  alkalinityAsHCO3() const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> carbonate_ppm () const;
   std::optional<double> potassium_ppm () const;
   std::optional<double> iron_ppm      () const;
   std::optional<double> nitrate_ppm   () const;
   std::optional<double> nitrite_ppm   () const;
   std::optional<double> fluoride_ppm  () const;

   double       ppm(Water::Ion const ion) const;

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
   void setType            (std::optional<Type>   const   val);
   void setTypeAsInt       (std::optional<int>    const   val);
   void setMashRo_pct      (std::optional<double> const   val);
   void setSpargeRo_pct    (std::optional<double> const   val);
   void setAlkalinityAsHCO3(bool                  const   val);

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setCarbonate_ppm   (std::optional<double> const   val);
   void setPotassium_ppm   (std::optional<double> const   val);
   void setIron_ppm        (std::optional<double> const   val);
   void setNitrate_ppm     (std::optional<double> const   val);
   void setNitrite_ppm     (std::optional<double> const   val);
   void setFluoride_ppm    (std::optional<double> const   val);

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const override;
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
   std::optional<Type>   m_type              ;
   std::optional<double> m_mashRo_pct        ;
   std::optional<double> m_spargeRo_pct      ;
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
