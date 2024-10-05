/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/BoilStep.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_BOILSTEP_H
#define MODEL_BOILSTEP_H
#pragma once

#include <optional>

#include "model/Boil.h"
#include "model/StepBase.h"
#include "model/StepExtended.h"
#include "utils/EnumStringMapping.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::BoilStep { BtStringConst const property{#property}; }
AddPropertyName(chillingType)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================
/**
 * On \c BoilStep, \c stepTime_mins and \c startTemp_c are optional
 */
#define BoilStepOptions StepBaseOptions{.stepTimeRequired = false, .startTempRequired = false, .rampTimeSupported = true}
/**
 * \class BoilStep is a step in a a boil process.  It can be used to support pre-boil steps, non-boiling pasteurisation
 *                 steps, boiling, whirlpool steps, and chilling.
 *
 * \brief As a \c MashStep is to a \c Mash, so a \c BoilStep is to a \c Boil
 */
class BoilStep : public StepExtended, public StepBase<BoilStep, Boil, BoilStepOptions> {
   Q_OBJECT

   STEP_COMMON_DECL(Boil, BoilStepOptions)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /*!
    * \brief Chilling type separates batch chilling, eg immersion chillers, where the entire volume of wort is brought
    *        down in temperature as a whole, vs inline chilling where the wort is chilled while it is being drained,
    *        which can leave a significant amount of hop isomerization occurring in the boil kettle.
    */
   enum class ChillingType {Batch ,
                            Inline};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(ChillingType)

   /*!
    * \brief Mapping between \c BoilStep::ChillingType and string values suitable for serialisation in DB, BeerJSON, etc
    *
    *        This can also be used to obtain the number of values of \c ChillingType, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const chillingTypeStringMapping;

   /*!
    * \brief Localised names of \c BoilStep::ChillingType values suitable for displaying to the end user
    */
   static EnumStringMapping const chillingTypeDisplayNames;

   //
   // This alias makees it easier to template a number of functions that are essentially the same for all subclasses of
   // Step.
   //
   using OwnerClass = Boil;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   BoilStep(QString name = "");
   BoilStep(NamedParameterBundle const & namedParameterBundle);
   BoilStep(BoilStep const & other);

   virtual ~BoilStep();

   //=================================================== PROPERTIES ====================================================
   // ⮜⮜⮜ All below added for BeerJSON support(!) ⮞⮞⮞
   /**
    * \brief See comment on \c grainGroup property in \c model/Fermentable.h for why an \b optional enum property has to
    *        be accessed as an optional \c int.
    */
   Q_PROPERTY(std::optional<int> chillingType  READ chillingTypeAsInt  WRITE setChillingTypeAsInt)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   std::optional<ChillingType> chillingType     () const;
   std::optional<int>          chillingTypeAsInt() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setChillingType     (std::optional<ChillingType> const val);
   void setChillingTypeAsInt(std::optional<int>          const val);

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

private:
   std::optional<ChillingType> m_chillingType;

};

BT_DECLARE_METATYPES(BoilStep)

#endif
