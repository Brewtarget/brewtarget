/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Equipment.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#ifndef MODEL_EQUIPMENT_H
#define MODEL_EQUIPMENT_H
#pragma once

#include <optional>

#include <QDomNode>
#include <QSqlRecord>

#include "model/FolderPropertyBase.h"
#include "model/NamedEntity.h"

class EquipmentCatalog;
class EquipmentEditor;
class EquipmentItemDelegate;
class EquipmentSortFilterProxyModel;
class EquipmentTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Equipment { inline BtStringConst const property{#property}; }
AddPropertyName(agingVesselLoss_l          )
AddPropertyName(agingVesselNotes           )
AddPropertyName(agingVesselType            )
AddPropertyName(agingVesselVolume_l        )
AddPropertyName(boilingPoint_c             )
AddPropertyName(evapRate_pctHr             )
AddPropertyName(fvBatchSize_l              )
AddPropertyName(fvLoss_l                   )
AddPropertyName(fvNotes                    )
AddPropertyName(fvType                     )
AddPropertyName(hltLoss_l                  )
AddPropertyName(hltNotes                   )
AddPropertyName(hltSpecificHeat_calGC      )
AddPropertyName(hltType                    )
AddPropertyName(hltVolume_l                )
AddPropertyName(hltWeight_kg               )
AddPropertyName(hopUtilization_pct         )
AddPropertyName(kettleBoilSize_l           )
AddPropertyName(kettleEvaporationPerHour_l )
AddPropertyName(kettleInternalDiameter_cm  )
AddPropertyName(kettleNotes                )
AddPropertyName(kettleOpeningDiameter_cm   )
AddPropertyName(kettleOutflowPerMinute_l   )
AddPropertyName(kettleSpecificHeat_calGC   )
AddPropertyName(kettleTrubChillerLoss_l    )
AddPropertyName(kettleType                 )
AddPropertyName(kettleWeight_kg            )
AddPropertyName(lauterTunDeadspaceLoss_l   )
AddPropertyName(lauterTunNotes             )
AddPropertyName(lauterTunSpecificHeat_calGC)
AddPropertyName(lauterTunType              )
AddPropertyName(lauterTunVolume_l          )
AddPropertyName(lauterTunWeight_kg         )
AddPropertyName(mashTunFoundationVolume_l   )
AddPropertyName(mashTunGrainAbsorption_LKg )
AddPropertyName(mashTunLoss_l              )
AddPropertyName(mashTunNotes               )
AddPropertyName(mashTunSpecificHeat_calGC  )
AddPropertyName(mashTunType                )
AddPropertyName(mashTunVolume_l            )
AddPropertyName(mashTunWeight_kg           )
AddPropertyName(packagingVesselLoss_l      )
AddPropertyName(packagingVesselNotes       )
AddPropertyName(packagingVesselType        )
AddPropertyName(packagingVesselVolume_l    )
AddPropertyName(topUpKettle_l              )
AddPropertyName(topUpWater_l               )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Equipment
 *
 * \brief Model representing a single equipment record.
 *
 *        This is where things get fun.  In BeerXML, Equipment is a single record representing all the hot-side
 *        equipment used in a recipe.  In BeerJSON however, the model is of a named array of EquipmentItemType objects,
 *        each of which can be one of "HLT", "Mash Tun", "Lauter Tun", "Brew Kettle", "fv", "Aging Vessel" or
 *        "Packaging Vessel".
 *
 *        We take the view that it is right to have a single Equipment object, but that subdividing it into the 7
 *        categories of BeerJSON is also useful.  (Although nothing in BeerJSON precludes you from having multiple
 *        EquipmentItemType objects in a single EquipmentType array, we take the view that this would not be meaningful,
 *        and so we do not support it.)
 *
 *        There are a few wrinkles around the edges.  In BeerJSON, you don't have to have a record for a particular
 *        vessel (eg you might not have an "Aging Vessel") but, if a vessel record is present, it has to have values for
 *        "name", "maximum_volume" and "loss".  This means some \c Equipment fields should, technically, be "optional
 *        in certain circumstances" -- eg \c agingVesselLoss_l should be optional unless any of the other agingVessel
 *        fields are set.  However, for the moment at least, we simplify and say something is either optional or it's
 *        not.  And, since \c agingVesselLoss_l can be required, then it is never null.  (Our default values for "name",
 *        "maximum_volume" and "loss" are "", 0.0 and 0.0 respectively.)  The upshot of this is that, when we write an
 *        \c Equipment record out to BeerJSON, we will write records for all seven vessel types.  This is slightly ugly,
 *        but I don't think it has any significant bad consequences.  At worst, it should be self evident to the user
 *        when there is no substantive data in a record.  However, we can, in fact, make things better in the UI.  When
 *        a vessel volume is 0 then we set its other attributes to 0 and hide it in the equipment editor.  The user can
 *        click "add Lauter Tun" etc if s/he needs such a vessel in the equipment profile.
 *
 *        NOTE: Now that we support hot liquor tanks, lauter tuns and so on, we need to be careful about places in the
 *              code & UI where there was previously an implicit assumption about the (maximum) number of vessels.  Eg,
 *              in a system with no separate lautering tun, the lautering losses are part of the mash tun losses, but,
 *              where there is a separate lautering tun, lautering losses will be associated with that tun.  Code that
 *              needs to know about mash/lauter volumes should therefore go through \c getLauteringDeadspaceLoss_l and
 *              \c strikeWaterAllowance_l rather than reading the individual vessel fields, so that it does not need to
 *              know how many vessels the brewer is using.
 *
 *        Volumes in the mash tun (and, where there is one, the lauter tun) relate to each other as follows:
 *
 *              ┌──────────────────────────────┐ ─┬─
 *              │   mash: grain + the water    │  │
 *              │   that is in contact with it │  │ mashTunVolume_l = total usable capacity, including
 *              │                              │  │                   mashTunFoundationVolume_l and mashTunLoss_l
 *              ├┄┄┄┄┄┄ false bottom ┄┄┄┄┄┄┄┄┄┄┤  │
 *              │   mashTunFoundationVolume_l  │  │ ← drains out with the wort (recovered)
 *              ├────── outlet ────────────────┤  │
 *              │   mashTunLoss_l              │  │ ← stays behind (not recovered)
 *              └──────────────────────────────┘ ─┴─
 *
 *        Grain absorption (\c mashTunGrainAbsorption_LKg) is separate from all of these: it is water retained by the
 *        grain rather than by the vessel.
 */
class Equipment : public NamedEntity,
                  public FolderPropertyBase<Equipment> {
   Q_OBJECT
   FOLDER_BASE_DECL(Equipment)
   // See model/FolderPropertyBase.h for info, getters and setters for these properties
   Q_PROPERTY(int     containedInFolderId     READ containedInFolderId     WRITE setContainedInFolderId  )
   Q_PROPERTY(QString containedInFolderPath   READ containedInFolderPath   WRITE setContainedInFolderPath)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_agingVesselLoss_l          ();
   static QString localisedName_agingVesselNotes           ();
   static QString localisedName_agingVesselType            ();
   static QString localisedName_agingVesselVolume_l        ();
   static QString localisedName_boilingPoint_c             ();
   static QString localisedName_evapRate_pctHr             ();
   static QString localisedName_fvBatchSize_l       ();
   static QString localisedName_fvLoss_l            ();
   static QString localisedName_fvNotes             ();
   static QString localisedName_fvType              ();
   static QString localisedName_hltLoss_l                  ();
   static QString localisedName_hltNotes                   ();
   static QString localisedName_hltSpecificHeat_calGC      ();
   static QString localisedName_hltType                    ();
   static QString localisedName_hltVolume_l                ();
   static QString localisedName_hltWeight_kg               ();
   static QString localisedName_hopUtilization_pct         ();
   static QString localisedName_kettleBoilSize_l           ();
   static QString localisedName_kettleEvaporationPerHour_l ();
   static QString localisedName_kettleInternalDiameter_cm  ();
   static QString localisedName_kettleNotes                ();
   static QString localisedName_kettleOpeningDiameter_cm   ();
   static QString localisedName_kettleOutflowPerMinute_l   ();
   static QString localisedName_kettleSpecificHeat_calGC   ();
   static QString localisedName_kettleTrubChillerLoss_l    ();
   static QString localisedName_kettleType                 ();
   static QString localisedName_kettleWeight_kg            ();
   static QString localisedName_lauterTunDeadspaceLoss_l   ();
   static QString localisedName_lauterTunNotes             ();
   static QString localisedName_lauterTunSpecificHeat_calGC();
   static QString localisedName_lauterTunType              ();
   static QString localisedName_lauterTunVolume_l          ();
   static QString localisedName_lauterTunWeight_kg         ();
   static QString localisedName_mashTunFoundationVolume_l  ();
   static QString localisedName_mashTunGrainAbsorption_LKg ();
   static QString localisedName_mashTunLoss_l              ();
   static QString localisedName_mashTunNotes               ();
   static QString localisedName_mashTunSpecificHeat_calGC  ();
   static QString localisedName_mashTunType                ();
   static QString localisedName_mashTunVolume_l            ();
   static QString localisedName_mashTunWeight_kg           ();
   static QString localisedName_packagingVesselLoss_l      ();
   static QString localisedName_packagingVesselNotes       ();
   static QString localisedName_packagingVesselType        ();
   static QString localisedName_packagingVesselVolume_l    ();
   static QString localisedName_topUpKettle_l              ();
   static QString localisedName_topUpWater_l               ();
   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using CatalogClass              = EquipmentCatalog;
   using EditorClass               = EquipmentEditor;
   using ItemDelegateClass         = EquipmentItemDelegate;
   using SortFilterProxyModelClass = EquipmentSortFilterProxyModel;
   using TableModelClass           = EquipmentTableModel;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   explicit Equipment(QString name = "");
   explicit Equipment(NamedParameterBundle const & namedParameterBundle);
   Equipment(Equipment const & other);

   ~Equipment() override;

   /**
    * \brief Some default values we use in calculations when no value is set in this record (or it this record is itself
    *        not present)
    */
   //! @{
   static constexpr double default_hopUtilization_pct         = 100.0;
   static constexpr double default_kettleEvaporationPerHour_l = 4.0;
   static constexpr double default_mashTunGrainAbsorption_LKg = 1.086; // See also PhysicalConstants::grainAbsorption_Lkg
   static constexpr double default_mashTunSpecificHeat_calGC  = 0.0;
   static constexpr double default_mashTunWeight_kg           = 0.0;
   static constexpr double default_topUpKettle_l              = 0.0;
   static constexpr double default_topUpWater_l               = 0.0;
   //! @}

   //=================================================== PROPERTIES ====================================================
   /**
    * \brief The boil size in liters: the pre-boil volume used in this particular instance for this equipment setup.
    *
    *        In BeerJSON this just the maxiumum_volume of the "Brew Kettle".
    */
   Q_PROPERTY(double kettleBoilSize_l       READ kettleBoilSize_l       WRITE setKettleBoilSize_l    ) // Required in BeerJSON (when Brew Kettle record present)
   /**
    * \brief The batch size in liters, aka the target volume of the batch at the start of fermentation.
    *
    *        In BeerJSON, this corresponds to the maxiumum_volume of the "fv".
    */
   Q_PROPERTY(double fvBatchSize_l   READ fvBatchSize_l   WRITE setFvBatchSize_l) // Required in BeerJSON (when fv record present)
   /**
    * \brief The mash tun volume in liters.                ⮜⮜⮜ Optional in BeerXML but required in BeerJSON ⮞⮞⮞
    *        This parameter can be used to calculate if a particular mash and grain profile will fit in the mash tun.
    *        It may also be used for thermal calculations in the case of a partially full mash tun.
    */
   Q_PROPERTY(double mashTunVolume_l        READ mashTunVolume_l        WRITE setMashTunVolume_l     )  // Required in BeerJSON (when Mash Tun record present)
   /**
    * \brief The tun mass in kg.                  ⮜⮜⮜ Optional in BeerXML & BeerJSON ⮞⮞⮞
    *        Used primarily to calculate the thermal parameters of the mash tun – in conjunction with the volume and
    *        specific heat.
    */
   Q_PROPERTY(std::optional<double> mashTunWeight_kg      READ mashTunWeight_kg      WRITE setMashTunWeight_kg          )
   /**
    * \brief The mash tun specific heat in cal/(g*C)   ⮜⮜⮜ Optional in BeerXML & BeerJSON ⮞⮞⮞
    *        This is usually a function of the material the mash tun is made of.  Typical ranges are 0.1-0.25 for metal
    *        and 0.2-0.5 for plastic materials.
    */
   Q_PROPERTY(std::optional<double> mashTunSpecificHeat_calGC READ mashTunSpecificHeat_calGC WRITE setMashTunSpecificHeat_calGC )
   /**
    * \brief The top-up water in liters.               ⮜⮜⮜ Optional in BeerXML, not part of BeerJSON ⮞⮞⮞
    *        The amount of top up water normally added just prior to starting fermentation.  Usually used for extract
    *        brewing.
    *
    *        Note this is not stored in BeerJSON.  Instead, all water top-ups are part of the Recipe to which they
    *        relate.
    *
    *        TODO: This should get moved to Recipe
    */
   Q_PROPERTY(std::optional<double> topUpWater_l          READ topUpWater_l          WRITE setTopUpWater_l          )
   /**
    * \brief The loss to trub and chiller in liters.  ⮜⮜⮜ Optional in BeerXML but required in BeerJSON ⮞⮞⮞
    *
    *        The amount of wort normally lost during transition from the boiler to the fermentation vessel.  Includes
    *        both unusable wort due to trub and wort lost to the chiller and transfer systems.
    *
    *
    *        Note that BeerJSON has a per-vessel "loss" field, so this is the brew kettle loss.
    *        Since this is required in BeerJSON, we'll keep it required here, but default it (and the other loss fields)
    *        to 0.
    */
   Q_PROPERTY(double kettleTrubChillerLoss_l     READ kettleTrubChillerLoss_l     WRITE setKettleTrubChillerLoss_l     )
   /**
    * \brief The evaporation rate in percent of the boil size per hour. *** DO NOT USE. *** Only for BeerXML compatibility.  ⮜⮜⮜ Optional in BeerXML.  Not supported in BeerJSON. ⮞⮞⮞
    *
    *        We keep this in step with \c kettleEvaporationPerHour_l (which is what we use in calculations), as a
    *        percentage of \c kettleBoilSize_l.  When reading a BeerXML file from another program, this may be the only
    *        evaporation rate we have, in which case we use it to set \c kettleEvaporationPerHour_l.
    *
    *        .:TBD:. Can we drop this from the DB?
    */
   Q_PROPERTY(std::optional<double> evapRate_pctHr        READ evapRate_pctHr        WRITE setEvapRate_pctHr        )
   /**
    * \brief The evaporation rate in liters/hr.  NB: Not part of BeerXML
    *
    *        This is boil_rate_per_hour for Brew Kettle in BeerJSON: "The volume boiled off during 1 hour, measured
    *        before and after at room temperature."
    *
    *        Note that, although, strictly, this is a "volume per time" measurement, we follow BeerJSON's lead in
    *        treating the "per hour" bit as set in stone and thus simplify this down to a "volume" measurement in the
    *        UI.
    */
   Q_PROPERTY(std::optional<double> kettleEvaporationPerHour_l          READ kettleEvaporationPerHour_l          WRITE setKettleEvaporationPerHour_l          )
   /**
    * \brief The lauter tun's deadspace in liters.                              ⮜⮜⮜ Optional in BeerXML but required in BeerJSON ⮞⮞⮞
    *        Amount lost to the lauter tun and equipment associated with the lautering process.
    *
    *        In BeerJSON, this is the "loss" of Lauter Tun.
    */
   Q_PROPERTY(double lauterTunDeadspaceLoss_l     READ lauterTunDeadspaceLoss_l     WRITE setLauterTunDeadspaceLoss_l     )
   /**
    * \brief The kettle top up in liters.                                       ⮜⮜⮜ Optional in BeerXML.  Not supported in BeerJSON. ⮞⮞⮞
    *        Amount normally added to the boil kettle before the boil.
    *
    *        Note this is not stored in BeerJSON.  Instead, all water top-ups are part of the Recipe to which they
    *        relate.
    *
    *        TODO: Move to Recipe
    */
   Q_PROPERTY(std::optional<double>  topUpKettle_l         READ topUpKettle_l         WRITE setTopUpKettle_l         )
   /**
    * \brief The hop utilization factor. I do not believe this is used.         ⮜⮜⮜ Optional in BeerXML.  Not supported in BeerJSON. ⮞⮞⮞
    *
    *        Large batch hop utilization.  This value should be 100% for batches less than 20 gallons, but may be higher
    *        (200% or more) for very large batch equipment.
    */
   Q_PROPERTY(std::optional<double> hopUtilization_pct    READ hopUtilization_pct    WRITE setHopUtilization_pct    )
   /**
    * \brief The Brew Kettle Notes.
    *
    *        In BeerXML, there is one "Notes" field for the whole equipment record.  In BeerJSON, there is no overall
    *        Notes field, but, instead, each vessel ("HLT", "Mash Tun", etc) has its own Notes field.  To bridge the
    *        gap, we treat the "Brew Kettle" Notes field of BeerJSON as the overall Notes field of BeerXML.
    */
   Q_PROPERTY(QString kettleNotes                READ kettleNotes                 WRITE setKettleNotes                 )
   /**
    * \brief How much water the grains absorb in liters/kg.  NB: Not part of BeerXML (but is part of BeerJSON)
    *
    *        The apparent volume absorbed by grain, typical values are 0.125 gal/lb (1.04 L/kg) for a mash tun,
    *        0.08 gal/lb (0.66 L/kg) for BIAB.
    */
   Q_PROPERTY(std::optional<double> mashTunGrainAbsorption_LKg   READ mashTunGrainAbsorption_LKg   WRITE setMashTunGrainAbsorption_LKg   )
   /**
    * \brief Volume of liquid in the mash tun that is outside the grain bed -- typically below a false bottom, screen or
    *        malt pipe.  It is needed as well as the water that wets the grain, but, unlike \c mashTunLoss_l, it drains
    *        out with the wort and so is recovered (eg into the boil).  Typical of all-in-one systems -- eg 3.5 l for a
    *        Grainfather G30.  0 if there is no such space (eg brew-in-a-bag with no false bottom).
    *
    *        In dotBeer, this is \c foundation_volume on \c mash_tun.  (In brewing, "foundation water" is the water put
    *        under the false bottom before doughing in.)  BeerJSON and BeerXML have no equivalent field: in BeerXML we
    *        write it as an extension tag; in BeerJSON it is not written, and is therefore lost.  (Brewfather calls it
    *        "Mash-Tun Deadspace" and BeerSmith "Mash Tun Addition".  We avoid the term "dead space" because, to many
    *        brewers, as in BeerXML's LAUTER_DEADSPACE, dead space means volume that is lost.)
    *
    *        It does not change how much water is needed in total, only how that water is split between strike water
    *        and sparge water.  See \c strikeWaterAllowance_l.
    *
    *        NB: Not part of BeerXML or BeerJSON
    */
   Q_PROPERTY(double                mashTunFoundationVolume_l    READ mashTunFoundationVolume_l    WRITE setMashTunFoundationVolume_l    )
   /**
    * \brief The boiling point of water in Celsius.  NB: Not part of BeerXML or BeerJSON
    *
    *        TODO: Again, this doesn't really belong on Equipment.  It's a function of the brewer's location rather than
    *              the equipment itself.  We should move it to Options.
    */
   Q_PROPERTY(double boilingPoint_c        READ boilingPoint_c        WRITE setBoilingPoint_c        )

   /**
    * \brief The interior diameter of the kettle at the surface of the wort.  Used to calculate the surface area of wort
    *        exposed to air, eg for mIBU calculation.  NB: Not part of BeerXML or BeerJSON
    */
   Q_PROPERTY(std::optional<double> kettleInternalDiameter_cm   READ kettleInternalDiameter_cm   WRITE setKettleInternalDiameter_cm   )

   /**
    * \brief The interior diameter of the opening in the kettle through which steam can escape.  Used in mIBU
    *        calculation.  NB: Not part of BeerXML or BeerJSON
    */
   Q_PROPERTY(std::optional<double> kettleOpeningDiameter_cm    READ kettleOpeningDiameter_cm    WRITE setKettleOpeningDiameter_cm   )

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞

   Q_PROPERTY(QString               hltType                           READ hltType                           WRITE setHltType                          )
   Q_PROPERTY(QString               mashTunType                       READ mashTunType                       WRITE setMashTunType                      )
   Q_PROPERTY(QString               lauterTunType                     READ lauterTunType                     WRITE setLauterTunType                    )
   Q_PROPERTY(QString               kettleType                        READ kettleType                        WRITE setKettleType                       )
   Q_PROPERTY(QString               fvType                     READ fvType                     WRITE setFvType                    )
   Q_PROPERTY(QString               agingVesselType                   READ agingVesselType                   WRITE setAgingVesselType                  )
   Q_PROPERTY(QString               packagingVesselType               READ packagingVesselType               WRITE setPackagingVesselType              )

   Q_PROPERTY(double                hltVolume_l                       READ hltVolume_l                       WRITE setHltVolume_l                      ) // Required in BeerJSON (when HLT record present)
   // mashTunVolume_l -- see above
   Q_PROPERTY(double                lauterTunVolume_l                 READ lauterTunVolume_l                 WRITE setLauterTunVolume_l                ) // Required in BeerJSON (when Lauter Tun record present)
   // kettleVolume_l -- see kettleBoilSize_l above
   // fvVolume_l -- see fvBatchSize_l above
   Q_PROPERTY(double                agingVesselVolume_l               READ agingVesselVolume_l               WRITE setAgingVesselVolume_l              ) // Required in BeerJSON (when Aging Vessel record present)
   Q_PROPERTY(double                packagingVesselVolume_l           READ packagingVesselVolume_l           WRITE setPackagingVesselVolume_l          ) // Required in BeerJSON (when Packaging Vessel record present)

   Q_PROPERTY(double                hltLoss_l                         READ hltLoss_l                         WRITE setHltLoss_l                        ) // Required in BeerJSON (when HLT record present)
   Q_PROPERTY(double                mashTunLoss_l                     READ mashTunLoss_l                     WRITE setMashTunLoss_l                    ) // Required in BeerJSON (when Mash Tun record present)
   // lauterTunLoss_l -- see lauterTunDeadspaceLoss_l above
   // kettleLoss_l -- see kettleTrubChillerLoss_l above
   Q_PROPERTY(double                fvLoss_l                   READ fvLoss_l                   WRITE setFvLoss_l                  ) // Required in BeerJSON (when fv record present)
   Q_PROPERTY(double                agingVesselLoss_l                 READ agingVesselLoss_l                 WRITE setAgingVesselLoss_l                ) // Required in BeerJSON (when Aging Vessel record present)
   Q_PROPERTY(double                packagingVesselLoss_l             READ packagingVesselLoss_l             WRITE setPackagingVesselLoss_l            ) // Required in BeerJSON (when Packaging Vessel record present)

   /**
    * \brief In BeerJSON, this is "drain_rate_per_minute: the volume that leaves the kettle, especially important for
    *        non-immersion chillers that cool the wort as it leaves the kettle."
    */
   Q_PROPERTY(std::optional<double> kettleOutflowPerMinute_l          READ kettleOutflowPerMinute_l          WRITE setKettleOutflowPerMinute_l         )

   Q_PROPERTY(std::optional<double> hltWeight_kg                      READ hltWeight_kg                      WRITE setHltWeight_kg                     )
   // mashTunWeight_kg -- see above
   Q_PROPERTY(std::optional<double> lauterTunWeight_kg                READ lauterTunWeight_kg                WRITE setLauterTunWeight_kg               )
   Q_PROPERTY(std::optional<double> kettleWeight_kg                   READ kettleWeight_kg                   WRITE setKettleWeight_kg                  )

   Q_PROPERTY(std::optional<double> hltSpecificHeat_calGC             READ hltSpecificHeat_calGC             WRITE setHltSpecificHeat_calGC            )
   // mashTunSpecificHeat_calGC -- see above
   Q_PROPERTY(std::optional<double> lauterTunSpecificHeat_calGC       READ lauterTunSpecificHeat_calGC       WRITE setLauterTunSpecificHeat_calGC      )
   Q_PROPERTY(std::optional<double> kettleSpecificHeat_calGC          READ kettleSpecificHeat_calGC          WRITE setKettleSpecificHeat_calGC         )

   Q_PROPERTY(QString               hltNotes                          READ hltNotes                          WRITE setHltNotes                         )
   Q_PROPERTY(QString               mashTunNotes                      READ mashTunNotes                      WRITE setMashTunNotes                     )
   Q_PROPERTY(QString               lauterTunNotes                    READ lauterTunNotes                    WRITE setLauterTunNotes                   )
   // kettleNotes -- see above
   Q_PROPERTY(QString               fvNotes                    READ fvNotes                    WRITE setFvNotes                   )
   Q_PROPERTY(QString               agingVesselNotes                  READ agingVesselNotes                  WRITE setAgingVesselNotes                 )
   Q_PROPERTY(QString               packagingVesselNotes              READ packagingVesselNotes              WRITE setPackagingVesselNotes             )

   SUPPORT_NUM_RECIPES_USED_IN

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   double                kettleBoilSize_l           () const;
   double                fvBatchSize_l       () const;
   double                mashTunVolume_l            () const;
   std::optional<double> mashTunWeight_kg           () const;
   std::optional<double> mashTunSpecificHeat_calGC  () const;
   std::optional<double> topUpWater_l               () const;
   double                kettleTrubChillerLoss_l    () const;
   std::optional<double> evapRate_pctHr             () const;
   std::optional<double> kettleEvaporationPerHour_l () const;
   double                lauterTunDeadspaceLoss_l   () const;
   std::optional<double> topUpKettle_l              () const;
   std::optional<double> hopUtilization_pct         () const;
   QString               kettleNotes                () const;
   std::optional<double> mashTunGrainAbsorption_LKg () const;
   double                mashTunFoundationVolume_l  () const;
   double                boilingPoint_c             () const;
   std::optional<double> kettleInternalDiameter_cm  () const;
   std::optional<double> kettleOpeningDiameter_cm   () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               hltType                    () const;
   QString               mashTunType                () const;
   QString               lauterTunType              () const;
   QString               kettleType                 () const;
   QString               fvType              () const;
   QString               agingVesselType            () const;
   QString               packagingVesselType        () const;
   double                hltVolume_l                () const;
   double                lauterTunVolume_l          () const;
   double                agingVesselVolume_l        () const;
   double                packagingVesselVolume_l    () const;
   double                hltLoss_l                  () const;
   double                mashTunLoss_l              () const;
   double                fvLoss_l            () const;
   double                agingVesselLoss_l          () const;
   double                packagingVesselLoss_l      () const;
   std::optional<double> kettleOutflowPerMinute_l   () const;
   std::optional<double> hltWeight_kg               () const;
   std::optional<double> lauterTunWeight_kg         () const;
   std::optional<double> kettleWeight_kg            () const;
   std::optional<double> hltSpecificHeat_calGC      () const;
   std::optional<double> lauterTunSpecificHeat_calGC() const;
   std::optional<double> kettleSpecificHeat_calGC   () const;
   QString               hltNotes                   () const;
   QString               mashTunNotes               () const;
   QString               lauterTunNotes             () const;
   QString               fvNotes             () const;
   QString               agingVesselNotes           () const;
   QString               packagingVesselNotes       () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setKettleBoilSize_l           (double                const   val);
   void setFvBatchSize_l       (double                const   val);
   void setMashTunVolume_l            (double                const   val);
   void setMashTunWeight_kg           (std::optional<double> const   val);
   void setMashTunSpecificHeat_calGC  (std::optional<double> const   val);
   void setTopUpWater_l               (std::optional<double> const   val);
   void setKettleTrubChillerLoss_l    (double                const   val);
   void setEvapRate_pctHr             (std::optional<double> const   val);
   void setKettleEvaporationPerHour_l (std::optional<double> const   val);
   void setLauterTunDeadspaceLoss_l   (double                const   val);
   void setTopUpKettle_l              (std::optional<double> const   val);
   void setHopUtilization_pct         (std::optional<double> const   val);
   void setKettleNotes                (QString               const & val);
   void setMashTunGrainAbsorption_LKg (std::optional<double> const   val);
   void setMashTunFoundationVolume_l  (double                const   val);
   void setBoilingPoint_c             (double                const   val);
   void setKettleInternalDiameter_cm  (std::optional<double> const   val);
   void setKettleOpeningDiameter_cm   (std::optional<double> const   val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setHltType                    (QString               const & val);
   void setMashTunType                (QString               const & val);
   void setLauterTunType              (QString               const & val);
   void setKettleType                 (QString               const & val);
   void setFvType              (QString               const & val);
   void setAgingVesselType            (QString               const & val);
   void setPackagingVesselType        (QString               const & val);
   void setHltVolume_l                (double                const   val);
   void setLauterTunVolume_l          (double                const   val);
   void setAgingVesselVolume_l        (double                const   val);
   void setPackagingVesselVolume_l    (double                const   val);
   void setHltLoss_l                  (double                const   val);
   void setMashTunLoss_l              (double                const   val);
   void setFvLoss_l            (double                const   val);
   void setAgingVesselLoss_l          (double                const   val);
   void setPackagingVesselLoss_l      (double                const   val);
   void setKettleOutflowPerMinute_l   (std::optional<double> const   val);
   void setHltWeight_kg               (std::optional<double> const   val);
   void setLauterTunWeight_kg         (std::optional<double> const   val);
   void setKettleWeight_kg            (std::optional<double> const   val);
   void setHltSpecificHeat_calGC      (std::optional<double> const   val);
   void setLauterTunSpecificHeat_calGC(std::optional<double> const   val);
   void setKettleSpecificHeat_calGC   (std::optional<double> const   val);
   void setHltNotes                   (QString               const & val);
   void setMashTunNotes               (QString               const & val);
   void setLauterTunNotes             (QString               const & val);
   void setFvNotes             (QString               const & val);
   void setAgingVesselNotes           (QString               const & val);
   void setPackagingVesselNotes       (QString               const & val);

   /**
    * \brief Gives the Lautering Deadspace Loss + any mash losses not related to grain absorption, without the caller
    *        having to know whether a separate lauter tun is used.
    *
    *        This is wort that is lost between the mash and the boil, so it has to be allowed for in the total water
    *        for the mash, and is subtracted from the wort that reaches the boil.  Note that it does \b not include
    *        \c mashTunFoundationVolume_l, which is recovered.
    */
   double getLauteringDeadspaceLoss_l() const;

   /**
    * \brief How much water needs to be in the mash tun, on top of what the grain bed itself needs (ie grain mass ×
    *        mash thickness), when the grain goes in.  This is \c mashTunFoundationVolume_l plus \c mashTunLoss_l.
    *
    *        So, strike water = (grain mass × mash thickness) + strikeWaterAllowance_l().  This keeps "mash thickness"
    *        meaning the ratio of water to grain in the grain bed, whatever the equipment.
    *
    *        Note that this is only about where the water goes, not how much is needed in total: the foundation volume
    *        is recovered with the wort and the mash tun loss is already allowed for in the total water (via
    *        \c getLauteringDeadspaceLoss_l).  Thus, the more of the total water that goes in as strike water, the less
    *        is needed for sparging.
    *
    *        Lauter tun losses are not included, because they are not in the mash tun.
    */
   double strikeWaterAllowance_l() const;

   /**
    * \brief The hot liquor tank, lauter tun, aging vessel and packaging vessel are "optional" vessels, ie ones the user
    *        might not have.  An optional vessel with a volume of 0 is treated as not being present (eg see
    *        \c getLauteringDeadspaceLoss_l), so its other attributes (loss, weight, specific heat, type and notes) are
    *        meaningless.  This function resets them, so that we don't store them.  (The equipment editor tells the user
    *        that "Attributes of 'optional' vessels will not be saved if their volumes are left blank or set to 0".)
    *
    * \return \c true if anything was changed, \c false otherwise
    */
   bool clearUnusedOptionalVessels();

   /**
    * \brief Calculate how much wort is left immediately at knockout.
    *
    * \param kettleWort_l
    * \param boilTime_mins
    */
   double wortEndOfBoil_l(double kettleWort_l, double boilTime_mins) const;

signals:

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   double                m_kettleBoilSize_l          ;
   double                m_fvBatchSize_l      ;
   double                m_mashTunVolume_l           ;
   std::optional<double> m_mashTunWeight_kg          ;
   std::optional<double> m_mashTunSpecificHeat_calGC ;
   std::optional<double> m_topUpWater_l              ;
   double                m_kettleTrubChillerLoss_l   ;
   std::optional<double> m_evapRate_pctHr            ;
   std::optional<double> m_kettleEvaporationPerHour_l;
   double                m_lauterTunDeadspaceLoss_l  ;
   std::optional<double> m_topUpKettle_l             ;
   std::optional<double> m_hopUtilization_pct        ;
   QString               m_kettleNotes               ;
   std::optional<double> m_mashTunGrainAbsorption_LKg;
   double                m_mashTunFoundationVolume_l ;
   double                m_boilingPoint_c            ;
   std::optional<double> m_kettleInternalDiameter_cm ;
   std::optional<double> m_kettleOpeningDiameter_cm  ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               m_hltType                    ;
   QString               m_mashTunType                ;
   QString               m_lauterTunType              ;
   QString               m_kettleType                 ;
   QString               m_fvType              ;
   QString               m_agingVesselType            ;
   QString               m_packagingVesselType        ;
   double                m_hltVolume_l                ;
   double                m_lauterTunVolume_l          ;
   double                m_agingVesselVolume_l        ;
   double                m_packagingVesselVolume_l    ;
   double                m_hltLoss_l                  ;
   double                m_mashTunLoss_l              ;
   double                m_fvLoss_l            ;
   double                m_agingVesselLoss_l          ;
   double                m_packagingVesselLoss_l      ;
   std::optional<double> m_kettleOutflowPerMinute_l   ;
   std::optional<double> m_hltWeight_kg               ;
   std::optional<double> m_lauterTunWeight_kg         ;
   std::optional<double> m_kettleWeight_kg            ;
   std::optional<double> m_hltSpecificHeat_calGC      ;
   std::optional<double> m_lauterTunSpecificHeat_calGC;
   std::optional<double> m_kettleSpecificHeat_calGC   ;
   QString               m_hltNotes                   ;
   QString               m_mashTunNotes               ;
   QString               m_lauterTunNotes             ;
   QString               m_fvNotes             ;
   QString               m_agingVesselNotes           ;
   QString               m_packagingVesselNotes       ;

};

BT_DECLARE_METATYPES(Equipment)

#endif