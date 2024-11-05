/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/FermentationStep.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_FERMENTATIONSTEP_H
#define MODEL_FERMENTATIONSTEP_H
#pragma once

#include <optional>

#include "model/Fermentation.h"
#include "model/StepBase.h"
#include "model/StepExtended.h"
#include "utils/EnumStringMapping.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::FermentationStep { inline BtStringConst const property{#property}; }
AddPropertyName(freeRise)
AddPropertyName(vessel  )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================
/**
 * On \c FermentationStep, \c stepTime_mins and \c startTemp_c are optional, and \c rampTime_mins is not to be used
 */
#define FermentationStepOptions StepBaseOptions{.stepTimeRequired = false, .startTempRequired = false, .rampTimeSupported = false}
/**
 * \class FermentationStep is a step in a a fermentation process.
 *
 * \brief As a \c MashStep is to a \c Mash, and a \c BoilStep is to a \c Boil, so a \c FermentationStep is to a
 *        \c Fermentation.
 */
class FermentationStep : public StepExtended, public StepBase<FermentationStep, Fermentation, FermentationStepOptions> {
   Q_OBJECT

   STEP_COMMON_DECL(Fermentation, FermentationStepOptions)
   // See model/SteppedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId      READ ownerId      WRITE setOwnerId   )
   Q_PROPERTY(int stepNumber   READ stepNumber   WRITE setStepNumber)
   // See model/StepBase.h for info, getters and setters for these properties
   Q_PROPERTY(std::optional<double> stepTime_mins   READ stepTime_mins   WRITE setStepTime_mins)
   Q_PROPERTY(std::optional<double> stepTime_days   READ stepTime_days   WRITE setStepTime_days)
   Q_PROPERTY(std::optional<double> startTemp_c     READ startTemp_c     WRITE setStartTemp_c  )
   Q_PROPERTY(std::optional<double> rampTime_mins   READ rampTime_mins   WRITE setRampTime_mins)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   //
   // This alias makees it easier to template a number of functions that are essentially the same for all subclasses of
   // Step.
   //
   using OwnerClass = Fermentation;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   FermentationStep(QString name = "");
   FermentationStep(NamedParameterBundle const & namedParameterBundle);
   FermentationStep(FermentationStep const & other);

   virtual ~FermentationStep();

   //=================================================== PROPERTIES ====================================================
   // ⮜⮜⮜ All below added for BeerJSON support(!) ⮞⮞⮞
   /** \brief If \c true, indicates a fermentation step where the exothermic fermentation is allowed to raise the
    *         temperature without restriction.
    */
   Q_PROPERTY(std::optional<bool> freeRise READ freeRise WRITE setFreeRise)
   //! \brief This is purely descriptive, and there is currently no direct link with \c Equipment
   Q_PROPERTY(QString vessel  READ vessel  WRITE setVessel)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   std::optional<bool> freeRise() const;
   QString             vessel  () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setFreeRise(std::optional<bool> const   val);
   void setVessel  (QString             const & val);

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const override;

private:
   std::optional<bool> m_freeRise;
   QString             m_vessel  ;
};

BT_DECLARE_METATYPES(FermentationStep)

#endif
