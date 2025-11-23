/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/BoilStep.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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

class BoilStepEditor;
class BoilStepItemDelegate;
class BoilStepTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::BoilStep { inline BtStringConst const property{#property}; }
AddPropertyName(chillingType)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================
/**
 * On \c BoilStep, \c stepTime_mins and \c startTemp_c are optional
 * (See comment in model/Step.h for summary of fields on different step types.)
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
   // See model/EnumeratedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId          READ ownerId          WRITE setOwnerId       )
   Q_PROPERTY(int sequenceNumber   READ sequenceNumber   WRITE setSequenceNumber)
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
   static QString localisedName_chillingType();

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
   // This alias makes it easier to template a number of functions that are essentially the same for all subclasses of
   // Step.
   //
   using OwnerClass = Boil;

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using EditorClass       = BoilStepEditor;
   using ItemDelegateClass = BoilStepItemDelegate;
   using TableModelClass   = BoilStepTableModel;

   /**
    * \brief Similarly it is useful to be able to get the editor for a \c Step subclass from template code (and without
    *        needing to directly call a \c MainWindow function, otherwise we get circular dependencies).
    */
   static EditorClass & getEditor();

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
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;

private:
   std::optional<ChillingType> m_chillingType;
};

/**
 * \brief Because \c BoilStep inherits from multiple bases, more than one of which has a match for \c operator<<, we
 *        need to provide an overload of \c operator<< that combines the output of those for all the base classes.
 */
template<class S>
S & operator<<(S & stream, BoilStep const & boilStep) {
   stream <<
      static_cast<StepExtended const &>(boilStep) << " " <<
      static_cast<StepBase<BoilStep, Boil, BoilStepOptions> const &>(boilStep);
   return stream;
}

BT_DECLARE_METATYPES(BoilStep)

#endif
