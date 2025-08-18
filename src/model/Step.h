/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Step.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef MODEL_STEP_H
#define MODEL_STEP_H
#pragma once

#include <optional>

#include <QString>

#include "database/ObjectStoreWrapper.h"
#include "model/FolderBase.h"
#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Step { inline BtStringConst const property{#property}; }
AddPropertyName(description    )
AddPropertyName(endAcidity_pH  )
AddPropertyName(endTemp_c      )
AddPropertyName(startAcidity_pH)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/**
 * \brief Common abstract base class for \c MashStep, \c BoilStep, \c FermentationStep
 *
 *        In BeerJSON, the step types have overlapping sets of fields, which correspond to our properties as follows
 *        (where ‡ means a field is required):
 *
 *           MashStepType         BoilStepType        FermentationStepType  |  Property
 *           ------------         ------------        --------------------  |  --------
 *         ‡ name               ‡ name              ‡ name                  |       NamedEntity::name
 *           description          description         description           |              Step::description
 *           ramp_time            ramp_time                                 |              Step::rampTime_mins
 *         ‡ step_time            step_time           step_time             |              Step::stepTime_mins
 *         ‡ step_temperature     start_temperature   start_temperature     |              Step::startTemp_c
 *           end_temperature      end_temperature     end_temperature       |              Step::  endTemp_c
 *           start_ph             start_ph            start_ph              |              Step::startAcidity_pH
 *           end_ph               end_ph              end_ph                |              Step::  endAcidity_pH
 *                                start_gravity       start_gravity         |      StepExtended::startGravity_sg
 *                                end_gravity         end_gravity           |      StepExtended::  endGravity_sg
 *                                                                          |
 *         ‡ type                                                           |          MashStep::type
 *           amount                                                         |          MashStep::amount_l
 *           water_grain_ratio                                              |          MashStep::liquorToGristRatio_lKg
 *           infuse_temperature                                             |          MashStep::infuseTemp_c
 *                                chilling_type                             |          BoilStep::chillingType
 *                                                    free_rise             |  FermentationStep::freeRise
 *                                                    vessel                |  FermentationStep::vessel
 *
 *
 *        NOTE that MashStepType breaks the naming convention by having step_temperature instead of start_temperature
 *        used by BoilStepType and FermentationStepType, but it is reasonably clear from the descriptions that they are
 *        the same thing.
 *
 *        NOTE too that two of the shared fields are required on MashStepType but only optional on BoilStepType and
 *        FermentationStepType.
 *
 *        We presume that FermentationStepType does not have a ramp_time field because step_time is so much longer than
 *        for BoilStepType or MashStepType (days rather than minutes), so the ramp time is negligible in the scheme of
 *        things.  See below for why it is simplest to leave it as an unused inherited field on \c FermentationStep.
 *
 * \sa \c StepBase
 */
class Step : public NamedEntity {
Q_OBJECT
public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_description    ();
   static QString localisedName_endAcidity_pH  ();
   static QString localisedName_endTemp_c      ();
   static QString localisedName_startAcidity_pH();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Step(QString name = "");
   Step(NamedParameterBundle const & namedParameterBundle);
   Step(Step const & other);

   virtual ~Step();

   //=================================================== PROPERTIES ====================================================
   /**
    * \brief The target ending temp of the step in °C.                       ⮜⮜⮜ Optional in BeerXML & BeerJSON ⮞⮞⮞
    *
    *        On a \c MashStep, this field is used in BeerXML and BeerJSON to signify "The expected temperature the mash
    *        falls to after a long mash step."
    */
   Q_PROPERTY(std::optional<double> endTemp_c         READ endTemp_c         WRITE setEndTemp_c      )
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   Q_PROPERTY(QString               description       READ description       WRITE setDescription    )
   Q_PROPERTY(std::optional<double> startAcidity_pH   READ startAcidity_pH   WRITE setStartAcidity_pH)
   Q_PROPERTY(std::optional<double>   endAcidity_pH   READ   endAcidity_pH   WRITE   setEndAcidity_pH)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================

   std::optional<double> endTemp_c      () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               description    () const;
   std::optional<double> startAcidity_pH() const;
   std::optional<double>   endAcidity_pH() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setEndTemp_c      (std::optional<double> const   val       );
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setDescription    (QString               const & val);
   void setStartAcidity_pH(std::optional<double> const   val);
   void setEndAcidity_pH  (std::optional<double> const   val);

signals:

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;

protected:
   std::optional<double> m_endTemp_c      ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               m_description    ;
   std::optional<double> m_startAcidity_pH;
   std::optional<double> m_endAcidity_pH  ;

};

#endif
