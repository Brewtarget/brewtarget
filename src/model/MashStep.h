/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/MashStep.h is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#ifndef MODEL_MASHSTEP_H
#define MODEL_MASHSTEP_H
#pragma once

#include <QString>
#include <QStringList>

#include "model/NamedEntity.h"
#include "model/Step.h"
#include "model/StepBase.h"
#include "utils/EnumStringMapping.h"
#include "utils/OptionalHelpers.h"

class MashStepEditor;
class MashStepItemDelegate;
class MashStepTableModel;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::MashStep { inline BtStringConst const property{#property}; }
AddPropertyName(amount_l              )
AddPropertyName(decoctionAmount_l     ) // Should only be used for BeerXML
AddPropertyName(infuseAmount_l        ) // Should only be used for BeerXML
AddPropertyName(infuseTemp_c          )
AddPropertyName(liquorToGristRatio_lKg)
AddPropertyName(type                  )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

class Mash;

/**
 * On \c MashStep, \c stepTime_mins is required, \c startTemp_c is required
 * (See comment in model/Step.h for summary of fields on different step types.)
 */
#define MashStepOptions StepBaseOptions{.stepTimeRequired = true, .startTempRequired = true, .rampTimeSupported = true}
/*!
 * \class MashStep
 *
 * \brief Model for a mash step record in the database.
 */
class MashStep : public Step, public StepBase<MashStep, Mash, MashStepOptions> {
   Q_OBJECT

   STEP_COMMON_DECL(Mash, MashStepOptions)
   // See model/EnumeratedBase.h for info, getters and setters for these properties
   Q_PROPERTY(int ownerId      READ ownerId      WRITE setOwnerId   )
   Q_PROPERTY(int stepNumber   READ stepNumber   WRITE setStepNumber)
   // See model/StepBase.h for info, getters and setters for these properties
   Q_PROPERTY(              double  stepTime_mins   READ stepTime_mins   WRITE setStepTime_mins)
   Q_PROPERTY(              double  stepTime_days   READ stepTime_days   WRITE setStepTime_days)
   Q_PROPERTY(              double  startTemp_c     READ startTemp_c     WRITE setStartTemp_c  )
   Q_PROPERTY(std::optional<double> rampTime_mins   READ rampTime_mins   WRITE setRampTime_mins)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_amount_l              ();
   static QString localisedName_decoctionAmount_l     ();
   static QString localisedName_infuseAmount_l        ();
   static QString localisedName_infuseTemp_c          ();
   static QString localisedName_liquorToGristRatio_lKg();
   static QString localisedName_type                  ();

   /**
    * \brief The type of step:
    *           \c Infusion denotes adding hot water
    *           \c Temperature denotes heating with an outside heat source
    *           \c Decoction denotes drawing off some mash for boiling
    *           \c FlySparge
    *           \c BatchSparge
    *           \c SouringMash
    *           \c SouringWort
    */
   enum class Type {Infusion   ,
                    Temperature,
                    Decoction  ,
                    FlySparge  , // In BeerJSON this is simply "sparge" (because it's the "normal" method)
                    BatchSparge, // In BeerJSON this is "drain mash tun"
                    // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
                    SouringMash,
                    SouringWort};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   /*!
    * \brief Mapping between \c MashStep::Type and string values suitable for serialisation in DB, BeerJSON, etc (but
    *        \b not BeerXML)
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const typeStringMapping;

   /*!
    * \brief Localised names of \c MashStep::Type values suitable for displaying to the end user
    */
   static EnumStringMapping const typeDisplayNames;

   //
   // This alias makes it easier to template a number of functions that are essentially the same for all subclasses of
   // Step.
   //
   using OwnerClass = Mash;

   //
   // Aliases to make it easier to template various functions that are essentially the same across different NamedEntity
   // subclasses.
   //
   using EditorClass       = MashStepEditor;
   using ItemDelegateClass = MashStepItemDelegate;
   using TableModelClass   = MashStepTableModel;

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

   MashStep(QString name = "");
   MashStep(NamedParameterBundle const & namedParameterBundle);
   MashStep(MashStep const & other);

   virtual ~MashStep();

   //=================================================== PROPERTIES ====================================================
   //! \brief The \c Type.
   Q_PROPERTY(Type   type READ type WRITE setType )

   /**
    * \brief The infusion or decoction amount for this step.
    *
    *        Historically, we had two separate amount properties -- \c infuseAmount_l and \c decoctionAmount_l.  This
    *        is because BeerXML only allows an infusion amount to be specified and prohibits the use of this field for
    *        a decoction amount, whilst not actually providing any mechanism to specify the decoction amount.  We then
    *        added a decoction amount field to our BeerXML records as an extension tag.
    *
    *        With BeerJSON, this nonsense is corrected and there is one amount field whose meaning (infusion amount or
    *        decoction amount) can be determined from the type field.
    *
    *        We retain the \c infuseAmount_l and \c decoctionAmount_l fields for ease of reading from / writing to
    *        BeerXML, but their use is @deprecated in other contexts as the underlying storage is now one amount field.
    *        (Because of the way we do serialisation, we only need the getter functions for these legacy attributes.
    *        When we are reading from BeerXML, they get put in a \c NamedParameterBundle.)
    *
    *        Strictly speaking this is an optional field in BeerJSON, because it's not required for every type of mash
    *        step, but I don't think it's too ambiguous for us to retain 0.0 as the "unspecified / not relevant" value.
    *        This saves us a bit of hassle in adding logic to check whether the field should be set and what default
    *        value to use if it's not etc.
    */
   Q_PROPERTY(double                amount_l               READ amount_l              WRITE setAmount_l                         )
   //! \brief The infusion amount in liters - NB: Retained only for BeerXML; DO NOT USE.
   Q_PROPERTY(double                infuseAmount_l         READ infuseAmount_l       STORED false)
   //! \brief The decoction amount in liters - NB: Retained only for BeerXML; DO NOT USE.
   Q_PROPERTY(double                decoctionAmount_l      READ decoctionAmount_l    STORED false)
   /**
    * \brief The infusion temp in C.                                        ⮜⮜⮜ Not part of BeerXML; optional in BeerJSON ⮞⮞⮞
    *
    *        An infusion step is where you're adding hot water to the mash, so this is the temperature of the water
    *        being added.  This is not part of BeerJSON, so I guess the thinking there is that the temperature of the
    *        water being added is either not important or can be calculated from other data (including the difference
    *        between the start and end temperatures of the step).
    */
   Q_PROPERTY(std::optional<double> infuseTemp_c           READ infuseTemp_c           WRITE setInfuseTemp_c                    )
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   /**
    * \brief Mash thickness, aka liquor-to-grist ratio, is the volume of strike water (liters) divided by the mass of
    *        grist (kilograms).  Its practical range is 2 to 4 and most often is around 2.5 to 3.2.
    */
   Q_PROPERTY(std::optional<double> liquorToGristRatio_lKg READ liquorToGristRatio_lKg WRITE setLiquorToGristRatio_lKg          )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Type type() const;
   double                amount_l              () const; // ⮜⮜⮜ Added, to replace infuseAmount_l & decoctionAmount_l, for BeerJSON support ⮞⮞⮞
   [[deprecated]] double infuseAmount_l        () const;
   [[deprecated]] double decoctionAmount_l     () const;
   std::optional<double> infuseTemp_c          () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> liquorToGristRatio_lKg() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setType                  (Type                  const   val);
   void setAmount_l              (double                const   val); // ⮜⮜⮜ Added, to replace setInfuseAmount_l & setDecoctionAmount_l, for BeerJSON support ⮞⮞⮞
   void setInfuseTemp_c          (std::optional<double> const   val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setLiquorToGristRatio_lKg(std::optional<double> const   val);

   //! some convenience methods
   bool isInfusion() const;
   bool isSparge() const;
   bool isTemperature() const;
   bool isDecoction() const;

signals:

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;

   /**
    * \brief Because \c MashStep inherits from multiple bases, more than one of which has a match for \c operator<<, we
    *        need to provide an overload of \c operator<< that combines the output of those for all the base classes.
    */
   template<class S>
   friend S & operator<<(S & stream, MashStep const & mashStep) {
      stream <<
         static_cast<Step const &>(mashStep) <<
         ", type: " << MashStep::typeStringMapping[mashStep.m_type] <<
         ", m_amount_l " << mashStep.m_amount_l <<
         ", m_infuseTemp_c " << Optional::toString(mashStep.m_infuseTemp_c) <<
         ", m_liquorToGristRatio_lKg " << Optional::toString(mashStep.m_liquorToGristRatio_lKg) << " " <<
         static_cast<StepBase<MashStep, Mash, MashStepOptions> const &>(mashStep);
      return stream;
   }

private:
   Type                  m_type                  ;
   double                m_amount_l              ;
   std::optional<double> m_infuseTemp_c          ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<double> m_liquorToGristRatio_lKg;
};


BT_DECLARE_METATYPES(MashStep)

static_assert(!HasFolder<MashStep>);
static_assert(HasNoFolder<MashStep>);

#endif
