/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAddition.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef MODEL_NAMEDENTITYINRECIPE_H
#define MODEL_NAMEDENTITYINRECIPE_H
#pragma once

#include <QString>

#include "model/IngredientInRecipe.h"
#include "utils/EnumStringMapping.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::RecipeAddition { inline BtStringConst const property{#property}; }
AddPropertyName(stage          )
AddPropertyName(step           )
AddPropertyName(addAtTime_mins )
AddPropertyName(addAtGravity_sg)
AddPropertyName(addAtAcidity_pH)
AddPropertyName(duration_mins  )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Common base class for recipe addition classes: \c RecipeAdditionHop, \c RecipeAdditionFermentable,
 *        \c RecipeAdditionMisc and \c RecipeAdditionYeast.
 *
 *        This follows the corresponding BeerJSON \c HopAdditionType, \c FermentableAdditionType, etc types.  (However,
 *        note that we do \b not have a class corresponding with BeerJSON's \c WaterAdditionType as it's simpler just to
 *        include the two component fields directly in \c Recipe.)
 *
 *        Our naming is slightly more cumbersome (eg RecipeAdditionHop instead of HopAddition), but has the merit of
 *        grouping all the "recipe addition" class files together in alphabetical directory listings etc.
 *
 *        In BeerJSON, the inheritance structure of, eg, hop-related records is:
 *
 *                                         HopVarietyBase
 *                                            /     \
 *                                           /       \
 *                           VarietyInformation     HopAdditionType
 *
 *        And RecipeType->ingredients->hop_additions is an array of HopAdditionType (which adds amount and timing info
 *        to HopVarietyBase).
 *
 *        HOWEVER, this is not the right structure for us.  The objective in BeerJSON is that a RecipeType record should
 *        contain a "minimal collection of the description of ingredients, procedures and other required parameters
 *        necessary to recreate a batch of beer".  Thus its HopAdditionType needs to give enough information about a hop
 *        for the recipe to be usable even if the software reading the record does not already have a record for that
 *        hop.
 *
 *        For our purposes, we want a \c RecipeAdditionHop to have the timing and quantity info of BeerJSON's
 *        HopAdditionType, but not to duplicate information that is already in \c Hop.
 *
 *        In the long run, we'd like to distinguish between generic information about a particular hop variety (eg
 *        Fuggle) and specific information about a particular batch of that hop (eg Xyz supplier's 2022 harvest pellets
 *        with 4.4% alpha acid).  This is, however, outside the scope of the BeerJSON work, and will be something we
 *        come back to as a future enhancement.  In the meantime, we have:
 *
 *                                           NamedEntity
 *                                           /    |     \
 *                                          /     |      \     RecipeAdditionBase<RecipeAdditionHop, Hop>
 *                                         /      |       \              |
 *           IngredientBase<Hop>   Ingredient   Recipe   RecipeAddition  |   IngredientAmount<RecipeAdditionHop, Hop>
 *                           \     /                               \     |       /
 *                            \   /                                 \    |      /
 *                             Hop                                RecipeAdditionHop
 *
 *        NOTE: We handle \c Water differently than other ingredients / additions because it has a lot less in common
 *              with them.  There is no stage/step/time/etc info (because that is implied by the mash schedule).  A
 *              \c Water addition in a \c Recipe essentially just says how much water of a particular profile that
 *              recipe uses.  Hence we have \c RecipeUseOfWater not \c RecipeAdditionWater.
 */
class RecipeAddition : public OwnedByRecipe {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_stage          ();
   static QString localisedName_step           ();
   static QString localisedName_addAtTime_mins ();
   static QString localisedName_addAtGravity_sg();
   static QString localisedName_addAtAcidity_pH();
   static QString localisedName_duration_mins  ();

   /**
    * Note that we rely on these values being in "chronological" order for \c lessThanByTime
    */
   enum class Stage {Mash        ,
                     Boil        ,
                     Fermentation,
                     Packaging   };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Stage)

   /*!
    * \brief Mapping between \c RecipeAddition::Stage and string values suitable for serialisation in DB, BeerJSON,
    *        etc (but \b not BeerXML)
    *
    *        This can also be used to obtain the number of values of \c Stage, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const stageStringMapping;

   /*!
    * \brief Localised names of \c RecipeAddition::Stage values suitable for displaying to the end user
    */
   static EnumStringMapping const stageDisplayNames;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   RecipeAddition(QString name = "", int const recipeId = -1);
   RecipeAddition(RecipeAddition const & other);
   RecipeAddition(NamedParameterBundle const & namedParameterBundle);

   virtual ~RecipeAddition();

   /**
    * \brief This function is used (as a parameter to std::sort) for sorting in the recipe formatter.
    *
    *        See also \c RecipeAdditionBase::doSpaceship
    */
   [[nodiscard]] static bool lessThanByTime(RecipeAddition const & lhs, RecipeAddition const & rhs);

   template<typename RA>
   [[nodiscard]] static bool lessThanByTime(std::shared_ptr<RA> const lhs, std::shared_ptr<RA> const rhs) {
      return lessThanByTime(*lhs, *rhs);
   }

   //=================================================== PROPERTIES ====================================================

   /**
    * \brief Strictly speaking in BeerJSON, this is an optional parameter, but we have to assume something if it's not
    *        present, so we make it required and subclasses should default it in their constructor (eg to
    *        \c RecipeAddition::Stage::Boil for a \c RecipeAdditionHop).
    *
    *        Also, BeerJSON calls this "use", with values of "add_to_mash", "add_to_boil", etc.  This may well because
    *        it replaces \c Hop::use etc.
    */
   Q_PROPERTY(Stage stage    READ stage    WRITE setStage)

   /**
    * \brief If this is set, it tells at what step of \c stage the addition is to be made.  Steps are numbered from 1.
    */
   Q_PROPERTY(std::optional<int> step READ step WRITE setStep)

   /**
    * \brief If set, tells us how long after the start of the step (or stage if step is not set) to make the addition.
    *
    *        For boil additions, we typically show the reverse of this number in the UI -- ie
    *        "boilLength_min - addAfter_min"
    */
   Q_PROPERTY(std::optional<double> addAtTime_mins   READ addAtTime_mins   WRITE setAddAtTime_mins)

   /**
    * \brief If set, tells us to wait until a particular specific gravity is reached before making the addition.  Eg we
    *        might want to do a particular dry hop addition when SG is 1.018.
    */
   Q_PROPERTY(std::optional<double> addAtGravity_sg  READ addAtGravity_sg  WRITE setAddAtGravity_sg)

   /**
    * \brief If set, tells us to wait until a particular acidity level is reached before making the addition.  Eg we
    *        might want to add brett when pH is 3.4.
    */
   Q_PROPERTY(std::optional<double> addAtAcidity_pH  READ addAtAcidity_pH  WRITE setAddAtAcidity_pH)

   //
   // NOTE that there is another optional boolean BeerJSON property called "continuous" that, if set to true, means the
   // addition "is spread out evenly and added during the entire process step".  It gives the example of "60 minute IPA
   // by dogfish head" which apparently "takes all of the hop additions and adds them throughout the entire boil".
   //
   // I've not implemented this for now because (a) I haven't found any published recipes (not even clones of 60 minute
   // IPA that use this method and (b) we haven't, as yet, implemented any support for it in the calculations.  If it
   // turns out to be something that people really want to have then we can add it later.
   //

   /**
    * \brief If set, tells us how long an ingredient addition remains.  This corresponds to the TIME attribute in
    *        BeerXML.  Obviously this does not make sense for every addition.
    */
   Q_PROPERTY(std::optional<double> duration_mins   READ duration_mins    WRITE setDuration_mins)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Stage                 stage          () const;
   std::optional<int>    step           () const;
   std::optional<double> addAtTime_mins () const;
   std::optional<double> addAtGravity_sg() const;
   std::optional<double> addAtAcidity_pH() const;
   std::optional<double> duration_mins  () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setStage          (Stage                 const val);
   void setStep           (std::optional<int>    const val);
   void setAddAtTime_mins (std::optional<double> const val);
   void setAddAtGravity_sg(std::optional<double> const val);
   void setAddAtAcidity_pH(std::optional<double> const val);
   void setDuration_mins  (std::optional<double> const val);

   virtual QString extraLogInfo() const override;

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;

protected:
   Stage                 m_stage          ;
   std::optional<int>    m_step           ;
   std::optional<double> m_addAtTime_mins ;
   std::optional<double> m_addAtGravity_sg;
   std::optional<double> m_addAtAcidity_pH;
   std::optional<double> m_duration_mins  ;
};

#endif
