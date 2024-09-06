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
#define AddPropertyName(property) namespace PropertyNames::FermentationStep { BtStringConst const property{#property}; }
AddPropertyName(freeRise)
AddPropertyName(vessel  )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \class FermentationStep is a step in a a fermentation process.
 *
 * \brief As a \c MashStep is to a \c Mash, and a \c BoilStep is to a \c Boil, so a \c FermentationStep is to a
 *        \c Fermentation.
 */
class FermentationStep : public StepExtended, public StepBase<FermentationStep, Fermentation> {
   Q_OBJECT

   STEP_COMMON_DECL(Fermentation)

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

   //========================================== "DON'T USE" MEMBER FUNCTIONS ===========================================
   // See related Q_PROPERTY in model/Step.h comment above for why these are virtual.  Essentially we inherit them from
   // Step but want to assert at runtime that it is a coding error if they are every called.
   [[deprecated]] virtual std::optional<double> rampTime_mins() const;
   [[deprecated]] virtual void setRampTime_mins(std::optional<double> const val);

   //============================================= OTHER MEMBER FUNCTIONS ==============================================
   [[nodiscard]] virtual bool rampTimeIsSupported() const;

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

private:
   std::optional<bool> m_freeRise;
   QString             m_vessel  ;
};

BT_DECLARE_METATYPES(FermentationStep)

#endif
