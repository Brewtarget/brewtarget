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
#define AddPropertyName(property) namespace PropertyNames::Step { BtStringConst const property{#property}; }
AddPropertyName(description    )
AddPropertyName(endAcidity_pH  )
AddPropertyName(endTemp_c      )
AddPropertyName(ownerId        )
AddPropertyName(rampTime_mins  )
AddPropertyName(startAcidity_pH)
AddPropertyName(startTemp_c    )
AddPropertyName(stepNumber     )
AddPropertyName(stepTime_days  ) // Mostly needed for BeerXML
AddPropertyName(stepTime_mins  )
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
    * \brief The time of the step in min.
    *        NOTE: This is required for MashStep but optional for BoilStep and FermentationStep.  We make it optional
    *              here but classes that need it required should set \c StepBaseOptions.stepTimeIsRequired parameter on
    *              \c StepBase template.  See \c StepBase for getters and setters.
    */
   Q_PROPERTY(std::optional<double> stepTime_mins          READ stepTime_mins          WRITE setStepTime_mins           )
   /**
    * \brief The time of the step in days - primarily for convenience on FermentationStep where measuring in minutes is
    *        overly precise.  The underlying measure in the database remains minutes however, for consistency.
    */
   Q_PROPERTY(std::optional<double> stepTime_days          READ stepTime_days          WRITE setStepTime_days           )
   /**
    * \brief Per comment above, this is also referred to as step temperature when talking about Mash Steps.
    *        For a \c MashStep, this is the target temperature of this step in C.  This is the main field to use when
    *        dealing with the mash step temperature.
    *
    *        NOTE: This is required for MashStep but optional for BoilStep and FermentationStep.  We make it optional
    *              here but classes that need it required should set \c StepBaseOptions.startTempIsRequired parameter on
    *              \c StepBase template.  See \c StepBase for getters and setters.
    */
   Q_PROPERTY(std::optional<double> startTemp_c      READ startTemp_c      WRITE setStartTemp_c    )
   /**
    * \brief The target ending temp of the step in C.                       ⮜⮜⮜ Optional in BeerXML & BeerJSON ⮞⮞⮞
    *
    *        On a \c MashStep, this field is used in BeerXML and BeerJSON to signify "The expected temperature the mash
    *        falls to after a long mash step."
    */
   Q_PROPERTY(std::optional<double> endTemp_c              READ endTemp_c              WRITE setEndTemp_c                       )
   /**
    * \brief The time it takes to ramp the temp to the target temp in min - ie the amount of time that passes before
    *        this step begins.                                                           ⮜⮜⮜ Optional in BeerXML & BeerJSON ⮞⮞⮞
    *
    *        Eg for \c MashStep, moving from a mash step (step 1) of 148F, to a new temperature step of 156F (step 2)
    *        may take 8 minutes to heat the mash. Step 2 would have a ramp time of 8 minutes.
    *
    *        Similarly, for a \c BoilStep, moving from a boiling step (step 1) to a whirlpool step (step 2) may take 5
    *        minutes.  Step 2 would have a ramp time of 5 minutes, hop isomerization and bitterness calculations will
    *        need to account for this accordingly.
    *
    *        NOTE: This property is \b not used by \c FermentationStep.  (It is the only property shared by \c MashStep
    *              and \c BoilStep that is not also needed in \c FermentationStep.  We can't really do mix-ins in Qt, so
    *              it's simplest just to not use it in \c FermentationStep.  We require the classes that use this
    *              property to set set \c StepBaseOptions.rampTimeIsSupported parameter on \c StepBase template, so we
    *              can at least get a run-time error if we accidentally try to use this property on a
    *              \c FermentationStep.)  See \c StepBase for getters and setters.
    */
   Q_PROPERTY(std::optional<double> rampTime_mins          READ rampTime_mins          WRITE setRampTime_mins                   )
   //! \brief The step number in a sequence of other steps.  Step numbers start from 1.
   Q_PROPERTY(int                   stepNumber             READ stepNumber             WRITE setStepNumber          STORED false)
   //! \brief The Mash, Boil or Fermentation to which this Step belongs
   Q_PROPERTY(int                   ownerId                READ ownerId                WRITE setOwnerId                         )
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   Q_PROPERTY(QString               description            READ description            WRITE setDescription                     )
   Q_PROPERTY(std::optional<double> startAcidity_pH        READ startAcidity_pH        WRITE setStartAcidity_pH                 )
   Q_PROPERTY(std::optional<double>   endAcidity_pH        READ   endAcidity_pH        WRITE   setEndAcidity_pH                 )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================

   std::optional<double>   endTemp_c    () const;
   int                   stepNumber     () const;
   int                   ownerId        () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               description    () const;
   std::optional<double> startAcidity_pH() const;
   std::optional<double>   endAcidity_pH() const;

   // See model/StepBase.h for overrides of these
   virtual std::optional<double> stepTime_mins() const = 0;
   virtual std::optional<double> stepTime_days() const = 0;
   virtual std::optional<double> startTemp_c  () const = 0;
   virtual std::optional<double> rampTime_mins() const = 0;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================

   // See model/StepBase.h for overrides of these
   virtual void setStepTime_mins(std::optional<double> const val) = 0;
   virtual void setStepTime_days(std::optional<double> const val) = 0;
   virtual void setStartTemp_c  (std::optional<double> const val) = 0;
   virtual void setRampTime_mins(std::optional<double> const val) = 0;

   void setEndTemp_c      (std::optional<double> const   val       );
   void setStepNumber     (int                   const   stepNumber);
   void setOwnerId        (int                   const   ownerId   );
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setDescription    (QString               const & val);
   void setStartAcidity_pH(std::optional<double> const   val);
   void setEndAcidity_pH  (std::optional<double> const   val);

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

protected:
   std::optional<double> m_stepTime_mins  ;
   std::optional<double> m_startTemp_c    ;
   std::optional<double> m_endTemp_c      ;
   int                   m_stepNumber     ;
   int                   m_ownerId        ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   QString               m_description    ;
   std::optional<double> m_rampTime_mins  ;
   std::optional<double> m_startAcidity_pH;
   std::optional<double> m_endAcidity_pH  ;

};

#endif
