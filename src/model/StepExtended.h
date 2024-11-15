/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/StepExtended.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_STEPEXTENDED_H
#define MODEL_STEPEXTENDED_H
#pragma once

#include "model/Step.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StepExtended { inline BtStringConst const property{#property}; }
AddPropertyName(startGravity_sg)
AddPropertyName(  endGravity_sg)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Adds some properties to \c Step that are needed by \c BoilStep and \c FermentationStep (but not \c MashStep).
 *
 *        Maybe ExtendedStep would have been a better name, but I wanted the source files to be near each other in the
 *        directory listing.  Anyway, it could have been worse.  By a great effort of willpower, I avoided calling this
 *        class StepPlusPlus, StepFurther or Leap. :)
 */
class StepExtended : public Step {
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

   StepExtended(QString name = "");
   StepExtended(NamedParameterBundle const & namedParameterBundle);
   StepExtended(StepExtended const & other);

   virtual ~StepExtended();

   //=================================================== PROPERTIES ====================================================
   // ⮜⮜⮜ All below added for BeerJSON support(!) ⮞⮞⮞
   Q_PROPERTY(std::optional<double> startGravity_sg  READ startGravity_sg  WRITE setStartGravity_sg)
   Q_PROPERTY(std::optional<double>   endGravity_sg  READ   endGravity_sg  WRITE   setEndGravity_sg)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   // ⮜⮜⮜ All below added for BeerJSON support(!) ⮞⮞⮞
   std::optional<double> startGravity_sg() const;
   std::optional<double>   endGravity_sg() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   // ⮜⮜⮜ All below added for BeerJSON support(!) ⮞⮞⮞
   void setStartGravity_sg(std::optional<double> const val);
   void   setEndGravity_sg(std::optional<double> const val);

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const override;

protected:
   // ⮜⮜⮜ All below added for BeerJSON support(!) ⮞⮞⮞
   std::optional<double> m_startGravity_sg;
   std::optional<double> m_endGravity_sg  ;
};

#endif
