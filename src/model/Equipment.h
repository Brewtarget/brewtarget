/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Equipment.h is part of Brewtarget, and is copyright the following authors 2009-2024:
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

#include "model/FolderBase.h"
#include "model/NamedEntity.h"

//╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Equipment { BtStringConst const property{#property}; }
AddPropertyName(agingVesselLoss_l          )
AddPropertyName(agingVesselNotes           )
AddPropertyName(agingVesselType            )
AddPropertyName(agingVesselVolume_l        )
AddPropertyName(fermenterBatchSize_l       )
AddPropertyName(boilingPoint_c             )
AddPropertyName(kettleBoilSize_l           )
AddPropertyName(boilTime_min               )
AddPropertyName(calcBoilVolume             )
AddPropertyName(evapRate_pctHr             )
AddPropertyName(fermenterLoss_l            )
AddPropertyName(fermenterNotes             )
AddPropertyName(fermenterType              )
AddPropertyName(hltLoss_l                  )
AddPropertyName(hltNotes                   )
AddPropertyName(hltSpecificHeat_calGC      )
AddPropertyName(hltType                    )
AddPropertyName(hltVolume_l                )
AddPropertyName(hltWeight_kg               )
AddPropertyName(hopUtilization_pct         )
AddPropertyName(kettleEvaporationPerHour_l )
AddPropertyName(kettleNotes                )
AddPropertyName(kettleOutflowPerMinute_l   )
AddPropertyName(kettleSpecificHeat_calGC   )
AddPropertyName(kettleType                 )
AddPropertyName(kettleWeight_kg            )
AddPropertyName(lauterTunDeadspaceLoss_l   )
AddPropertyName(lauterTunNotes             )
AddPropertyName(lauterTunSpecificHeat_calGC)
AddPropertyName(lauterTunType              )
AddPropertyName(lauterTunVolume_l          )
AddPropertyName(lauterTunWeight_kg         )
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
AddPropertyName(kettleTrubChillerLoss_l    )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌


/*!
 * \class Equipment
 *
 * \brief Model representing a single equipment record.
 *
 *        This is where things get fun.  In BeerXML, Equipment is a single record representing all the hot-side
 *        equipment used in a recipe.  In BeerJSON however, the model is of a named array of EquipmentItemType objects,
 *        each of which can be one of "HLT", "Mash Tun", "Lauter Tun", "Brew Kettle", "Fermenter", "Aging Vessel" or
 *        "Packaging Vessel".
 *
 *        We take the view that it is right to have a single Equipment object, but that subdividing it into the 7
 *        categories of BeerJSON is also useful.  (Although nothing in BeerJSON precludes you from having multiple
 *        EquipmentItemType objects in a single EquipmentType array, we take the view that this would not be meaningful,
 *        and so we do not support it.)
 *
 *        There are a few wrinkles around the edges.  In BeerJSON, you don't have to have a record for a particular
 *        vessel (eg you might not have an "Aging Vessel") but, if a vessel record is present, it has to have values for
 *        "name", "maxiumum_volume" and "loss".  This means some \c Equipment fields should, technically, be "optional
 *        in certain circumstances" -- eg \c agingVesselLoss_l should be optional unless any of the other agingVessel
 *        fields are set.  However, for the moment at least, we simplify and say something is either optional or it's
 *        not.  And, since \c agingVesselLoss_l can be required, then it is never null.  (Our default values for "name",
 *        "maxiumum_volume" and "loss" are "", 0.0 and 0.0 respectively.)  The upshot of this is that, when we write an
 *        \c Equipment record out to BeerJSON, we will write records for all seven vessel types.  This is slightly ugly,
 *        but I don't think it has any significant bad consequences.  At worst, it should be self evident to the user
 *        when there is no substantive data in a record.  However, we can, in fact, make things better in the UI.  When
 *        a vessel volume is 0 then we set its other attributes to 0 and hide it in the equipment editor.  The user can
 *        click "add Lauter Tun" etc if s/he needs such a vessel in the equipment profile.
 *
 *        NOTE: Now that we support hot liquor tanks, lauter tuns and so on, we need to be careful about places in the
 *              code & UI where there was previously an implicit assumption about the (maximum) number of vessels.  Eg,
 *              in a system with no separate lautering tun, the lautering losses are part of the mash tun losses, but,
 *              where there is a separate lautering tun, lautering losses will be associated with that tun.
 * TODO: Make the above NOTE true!
 */
class Equipment : public NamedEntity,
                  public FolderBase<Equipment> {
   Q_OBJECT
   FOLDER_BASE_DECL(Equipment)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Equipment(QString name = "");
   Equipment(NamedParameterBundle const & namedParameterBundle);
   Equipment(Equipment const & other);

   virtual ~Equipment();

   /**
    * \brief Some default values we use in calculations when no value is set in this record
    */
   //! @{
   static constexpr double default_boilTime_mins               = 60.0;
   static constexpr double default_hopUtilization_pct         = 100.0;
   static constexpr double default_kettleEvaporationPerHour_l = 4.0;
   static constexpr double default_mashTunGrainAbsorption_LKg = 1.086; // See also PhysicalConstants::grainAbsorption_Lkg
   static constexpr double default_mashTunSpecificHeat_calGC  = 0.0;
   static constexpr double default_mashTunWeight_kg           = 0.0;
   static constexpr double default_topUpKettle_l              = 0.0;
   static constexpr double default_topUpWater_l               = 0.0;
   //! @}

   /**
    * \brief The boil size in liters: the pre-boil volume used in this particular instance for this equipment setup.
    *        Note that this may be a calculated value depending on the calcBoilVolume property.
    *
    *        In BeerJSON, there is no record of whether this is a calculated value, it is just the maxiumum_volume of
    *        the "Brew Kettle".
    */
   Q_PROPERTY(double kettleBoilSize_l            READ kettleBoilSize_l            WRITE setKettleBoilSize_l            ) // Required in BeerJSON (when Brew Kettle record present)
   /**
    * \brief The batch size in liters, aka the target volume of the batch at the start of fermentation.
    *
    *        In BeerJSON, this corresponds to the maxiumum_volume of the "Fermenter".
    */
   Q_PROPERTY(double fermenterBatchSize_l           READ fermenterBatchSize_l           WRITE setFermenterBatchSize_l           ) // Required in BeerJSON (when Fermenter record present)
   /**
    * \brief The mash tun volume in liters.                ⮜⮜⮜ Optional in BeerXML but required in BeerJSON ⮞⮞⮞
    *        This parameter can be used to calculate if a particular mash and grain profile will fit in the mash tun.
    *        It may also be used for thermal calculations in the case of a partially full mash tun.
    */
   Q_PROPERTY(double mashTunVolume_l       READ mashTunVolume_l       WRITE setMashTunVolume_l           )  // Required in BeerJSON (when Mash Tun record present)
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
    * \brief The top-up water in liters.               ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
    *        The amount of top up water normally added just prior to starting fermentation.  Usually used for extract
    *        brewing.
    *
    *        Note this is not stored in BeerJSON.  .:TBD.JSON:. Does this become part of the Recipe?
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
    * \brief The boil time in minutes: the normal amount of time one boils for this equipment setup.  This can be used
    *        with the evaporation rate to calculate the evaporation loss.         ⮜⮜⮜ Optional in BeerXML.  Not supported in BeerJSON. ⮞⮞⮞
    *
    *        This is not stored in BeerJSON.
    *
    *        .:TBD:. MY 2023-06-17 I don't see that boil time is really an attribute of equipment.  It seems more like a
    *                per-recipe field.
    */
   Q_PROPERTY(std::optional<double> boilTime_min          READ boilTime_min          WRITE setBoilTime_min          )
   /**
    * \brief Whether you want the boil volume to be automatically calculated.    ⮜⮜⮜ Optional in BeerXML.  Not supported in BeerJSON. ⮞⮞⮞
    */
   Q_PROPERTY(bool calcBoilVolume        READ calcBoilVolume        WRITE setCalcBoilVolume          )
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
    */
   Q_PROPERTY(std::optional<double>  topUpKettle_l         READ topUpKettle_l         WRITE setTopUpKettle_l         )
   /**
    * \brief The hop utilization factor. I do not believe this is used.         ⮜⮜⮜ Optional in BeerXML.  Not supported in BeerJSON. ⮞⮞⮞
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
    *        The apparent volume absorbed by grain, typical values are 0.125 qt/lb (1.04 L/kg) for a mashtun,
    *        0.08 gal/lb (0.66 L/kg) for BIAB.
    */
   Q_PROPERTY(std::optional<double> mashTunGrainAbsorption_LKg   READ mashTunGrainAbsorption_LKg   WRITE setMashTunGrainAbsorption_LKg   )
   /**
    * \brief The boiling point of water in Celsius.  NB: Not part of BeerXML or BeerJSON
    */
   Q_PROPERTY(double boilingPoint_c        READ boilingPoint_c        WRITE setBoilingPoint_c        )

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞

   Q_PROPERTY(QString               hltType                           READ hltType                           WRITE setHltType                          )
   Q_PROPERTY(QString               mashTunType                       READ mashTunType                       WRITE setMashTunType                      )
   Q_PROPERTY(QString               lauterTunType                     READ lauterTunType                     WRITE setLauterTunType                    )
   Q_PROPERTY(QString               kettleType                        READ kettleType                        WRITE setKettleType                       )
   Q_PROPERTY(QString               fermenterType                     READ fermenterType                     WRITE setFermenterType                    )
   Q_PROPERTY(QString               agingVesselType                   READ agingVesselType                   WRITE setAgingVesselType                  )
   Q_PROPERTY(QString               packagingVesselType               READ packagingVesselType               WRITE setPackagingVesselType              )

   Q_PROPERTY(double                hltVolume_l                       READ hltVolume_l                       WRITE setHltVolume_l                      ) // Required in BeerJSON (when HLT record present)
   // mashTunVolume_l -- see above
   Q_PROPERTY(double                lauterTunVolume_l                 READ lauterTunVolume_l                 WRITE setLauterTunVolume_l                ) // Required in BeerJSON (when Lauter Tun record present)
   // kettleVolume_l -- see kettleBoilSize_l above
   // fermenterVolume_l -- see fermenterBatchSize_l above
   Q_PROPERTY(double                agingVesselVolume_l               READ agingVesselVolume_l               WRITE setAgingVesselVolume_l              ) // Required in BeerJSON (when Aging Vessel record present)
   Q_PROPERTY(double                packagingVesselVolume_l           READ packagingVesselVolume_l           WRITE setPackagingVesselVolume_l          ) // Required in BeerJSON (when Packaging Vessel record present)

   Q_PROPERTY(double                hltLoss_l                         READ hltLoss_l                         WRITE setHltLoss_l                        ) // Required in BeerJSON (when HLT record present)
   Q_PROPERTY(double                mashTunLoss_l                     READ mashTunLoss_l                     WRITE setMashTunLoss_l                    ) // Required in BeerJSON (when Mash Tun record present)
   // lauterTunLoss_l -- see lauterTunDeadspaceLoss_l above
   // kettleLoss_l -- see kettleTrubChillerLoss_l above
   Q_PROPERTY(double                fermenterLoss_l                   READ fermenterLoss_l                   WRITE setFermenterLoss_l                  ) // Required in BeerJSON (when Fermenter record present)
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
   Q_PROPERTY(QString               fermenterNotes                    READ fermenterNotes                    WRITE setFermenterNotes                   )
   Q_PROPERTY(QString               agingVesselNotes                  READ agingVesselNotes                  WRITE setAgingVesselNotes                 )
   Q_PROPERTY(QString               packagingVesselNotes              READ packagingVesselNotes              WRITE setPackagingVesselNotes             )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   double                kettleBoilSize_l           () const;
   double                fermenterBatchSize_l       () const;
   double                mashTunVolume_l            () const;
   std::optional<double> mashTunWeight_kg           () const;
   std::optional<double> mashTunSpecificHeat_calGC  () const;
   std::optional<double> topUpWater_l               () const;
   double                kettleTrubChillerLoss_l          () const;
   std::optional<double> evapRate_pctHr             () const;
   std::optional<double> kettleEvaporationPerHour_l () const;
   std::optional<double> boilTime_min               () const;
   bool                  calcBoilVolume             () const;
   double                lauterTunDeadspaceLoss_l      () const;
   std::optional<double> topUpKettle_l              () const;
   std::optional<double> hopUtilization_pct         () const;
   QString               kettleNotes                () const;
   std::optional<double> mashTunGrainAbsorption_LKg () const;
   double                boilingPoint_c             () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               hltType                    () const;
   QString               mashTunType                () const;
   QString               lauterTunType              () const;
   QString               kettleType                 () const;
   QString               fermenterType              () const;
   QString               agingVesselType            () const;
   QString               packagingVesselType        () const;
   double                hltVolume_l                () const;
   double                lauterTunVolume_l          () const;
   double                agingVesselVolume_l        () const;
   double                packagingVesselVolume_l    () const;
   double                hltLoss_l                  () const;
   double                mashTunLoss_l              () const;
   double                fermenterLoss_l            () const;
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
   QString               fermenterNotes             () const;
   QString               agingVesselNotes           () const;
   QString               packagingVesselNotes       () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setKettleBoilSize_l           (double                const   val);
   void setFermenterBatchSize_l       (double                const   val);
   void setMashTunVolume_l            (double                const   val);
   void setMashTunWeight_kg           (std::optional<double> const   val);
   void setMashTunSpecificHeat_calGC  (std::optional<double> const   val);
   void setTopUpWater_l               (std::optional<double> const   val);
   void setKettleTrubChillerLoss_l    (double                const   val);
   void setEvapRate_pctHr             (std::optional<double> const   val);
   void setKettleEvaporationPerHour_l (std::optional<double> const   val);
   void setBoilTime_min               (std::optional<double> const   val);
   void setCalcBoilVolume             (bool                  const   val);
   void setLauterTunDeadspaceLoss_l      (double                const   val);
   void setTopUpKettle_l              (std::optional<double> const   val);
   void setHopUtilization_pct         (std::optional<double> const   val);
   void setKettleNotes                (QString               const & val);
   void setMashTunGrainAbsorption_LKg (std::optional<double> const   val);
   void setBoilingPoint_c             (double                const   val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setHltType                    (QString               const & val);
   void setMashTunType                (QString               const & val);
   void setLauterTunType              (QString               const & val);
   void setKettleType                 (QString               const & val);
   void setFermenterType              (QString               const & val);
   void setAgingVesselType            (QString               const & val);
   void setPackagingVesselType        (QString               const & val);
   void setHltVolume_l                (double                const   val);
   void setLauterTunVolume_l          (double                const   val);
   void setAgingVesselVolume_l        (double                const   val);
   void setPackagingVesselVolume_l    (double                const   val);
   void setHltLoss_l                  (double                const   val);
   void setMashTunLoss_l              (double                const   val);
   void setFermenterLoss_l            (double                const   val);
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
   void setFermenterNotes             (QString               const & val);
   void setAgingVesselNotes           (QString               const & val);
   void setPackagingVesselNotes       (QString               const & val);

   /**
    * \brief Gives the Lautering Deadspace Loss + any mash losses not related to grain absorption, without the caller
    *        having to know whether a separate lauter tun is used.
    */
   double getLauteringDeadspaceLoss_l() const;

   //! \brief Calculate how much wort is left immediately at knockout.
   double wortEndOfBoil_l( double kettleWort_l ) const;

///   virtual Recipe * getOwningRecipe() const;

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   double                m_kettleBoilSize_l          ;
   double                m_fermenterBatchSize_l      ;
   double                m_mashTunVolume_l           ;
   std::optional<double> m_mashTunWeight_kg          ;
   std::optional<double> m_mashTunSpecificHeat_calGC ;
   std::optional<double> m_topUpWater_l              ;
   double                m_kettleTrubChillerLoss_l   ;
   std::optional<double> m_evapRate_pctHr            ;
   std::optional<double> m_kettleEvaporationPerHour_l;
   std::optional<double> m_boilTime_min              ;
   bool                  m_calcBoilVolume            ;
   double                m_lauterTunDeadspaceLoss_l         ;
   std::optional<double> m_topUpKettle_l             ;
   std::optional<double> m_hopUtilization_pct        ;
   QString               m_kettleNotes               ;
   std::optional<double> m_mashTunGrainAbsorption_LKg;
   double                m_boilingPoint_c            ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               m_hltType                    ;
   QString               m_mashTunType                ;
   QString               m_lauterTunType              ;
   QString               m_kettleType                 ;
   QString               m_fermenterType              ;
   QString               m_agingVesselType            ;
   QString               m_packagingVesselType        ;
   double                m_hltVolume_l                ;
   double                m_lauterTunVolume_l          ;
   double                m_agingVesselVolume_l        ;
   double                m_packagingVesselVolume_l    ;
   double                m_hltLoss_l                  ;
   double                m_mashTunLoss_l              ;
   double                m_fermenterLoss_l            ;
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
   QString               m_fermenterNotes             ;
   QString               m_agingVesselNotes           ;
   QString               m_packagingVesselNotes       ;




   // Calculate the boil size.
   void doCalculations();
};

BT_DECLARE_METATYPES(Equipment)

#endif
