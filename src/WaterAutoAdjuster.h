/*======================================================================================================================
 * WaterAutoAdjuster.h is part of Brewtarget, and is copyright the following authors 2026:
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
 =====================================================================================================================*/
#ifndef WATERAUTOADJUSTER_H
#define WATERAUTOADJUSTER_H
#pragma once

#include <array>
#include <optional>

#include <QCoreApplication> // For Q_DECLARE_TR_FUNCTIONS
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#include "model/Misc.h"
#include "model/Water.h"

class QUndoCommand;
class Recipe;

/**
 * \brief Works out how much of each brewing salt to add to the recipe's water to get it as close as possible to the
 *        target water profile and, optionally, how much acid to add to get the target mash pH.  This is what's behind
 *        the "Auto-Adjust" button and the "Keep in sync with recipe" option on the Water tab (see
 *        \c WaterAdjuster::autoAdjust and \c Recipe::waterKeptInSync).  It is the water equivalent of
 *        \c MashAutoAdjuster.
 *
 *        It also holds the mash pH model, so that what's displayed on the Water tab (see \c predict) agrees with what's
 *        calculated here.
 *
 *        The core calculation (\c calculate) has no UI and does not touch the model, so it is easy to unit test.  The
 *        "recipe-level" functions (\c predict, \c prepare, \c autoAdjust) read the inputs from a \c Recipe and, for the
 *        last two, apply the results.
 *
 *        SALTS
 *        -----
 *
 *        The model is:
 *
 *           actual ppm of ion i = (base water fraction × base ppm of i)
 *                                 + (ppm of i from additions we are not changing)
 *                                 + Σ over salts j of (grams of j × ppm of i per gram per liter of j) ÷ total liters
 *
 *        where "ppm of i per gram per liter of j" comes from \c Misc::concentrationPerGramPerLiter_massConcPpm and
 *        "total liters" is all the water (mash and sparge), as salts are added to all of it.
 *
 *        Since salts can only add ions, not remove them, the target can't, in general, be hit exactly.  (Eg gypsum
 *        adds both calcium and sulfate in a fixed ratio.)  So we find the non-negative amounts that minimise the sum
 *        of squared *relative* errors over the six ions brewers care about (calcium, magnesium, sodium, chloride,
 *        sulfate and bicarbonate).  We use relative errors (with a floor -- see \c relativeErrorFloor_ppm) because
 *        otherwise the ions with large concentrations (typically sulfate) would dominate, and 10 ppm out on
 *        magnesium matters a lot more than 10 ppm out on sulfate.
 *
 *        The problem is small (six ions, and rarely more than six or seven salts), so we solve it exactly by trying
 *        every subset of the salts: for each subset we solve the unconstrained weighted least-squares problem, and
 *        keep the best solution where all the amounts are non-negative.  (The optimum of a non-negative least squares
 *        problem is the unconstrained optimum over its own non-zero variables, so it's always one of the candidates.)
 *        This also copes gracefully with salts that are interchangeable -- eg anhydrous and dihydrate calcium
 *        chloride -- where the full problem is singular.
 *
 *        We then drop salts whose amounts are too small to bother weighing out (see \c Inputs::minAddition_g) and
 *        solve again.
 *
 *        MASH pH
 *        -------
 *
 *        This is based on the work of Kai Troester, published at
 *        http://braukaiser.com/wiki/index.php/Beer_color_to_mash_pH_(v2.0), and the spreadsheet associated with that
 *        page.  The mash pH is:
 *
 *           pH = distilled water mash pH + (RA × mash water liters − acid mEq) ÷ (B × grain kg)
 *
 *        where:
 *          - the distilled water mash pH is estimated from the beer color, gravity and the proportion of color that
 *            comes from specialty (crystal / roasted) malts -- see \c distilledWaterMashPh
 *          - RA is the residual alkalinity (in mEq/L) of the mash water, using Kolbach's formula:
 *               RA = alkalinity − Ca²⁺/3.5 − Mg²⁺/7  (all in mEq/L)
 *          - acid mEq is the total acid (in milliequivalents) in the mash, from acid additions and acidulated malt
 *          - B is the buffering capacity of the grist (see \c mashBufferCapacity_mEqPerKgPh)
 *
 *        NB: Previous versions of the code divided by mash thickness (L/kg) rather than grain mass, and got the units
 *        of the acid additions wrong (eg treating milligrams of lactic acid as kilograms), so gave quite different
 *        (and, with any lactic acid, wildly wrong) answers.
 *
 *        Because the pH is linear in the amount of acid, once we know the pH without any added acid, the amount of
 *        acid needed to get to the target pH follows directly.  Acid can only lower the pH, so if it's already below
 *        the target, we don't add any, and warn the user.  We work out the salts first (as they don't depend on the
 *        acid) and then the acid (which does depend on the salts, as calcium and magnesium lower pH and bicarbonate
 *        raises it).
 */
class WaterAutoAdjuster {
   Q_DECLARE_TR_FUNCTIONS(WaterAutoAdjuster)

public:
   //! The ions we try to match, in the order we display them
   static constexpr std::array<Water::MineralIon, 6> ions{
      Water::MineralIon::Calcium    ,
      Water::MineralIon::Magnesium  ,
      Water::MineralIon::Sodium     ,
      Water::MineralIon::Chloride   ,
      Water::MineralIon::Sulfate    ,
      Water::MineralIon::Bicarbonate,
   };

   /**
    * \brief The salts we use if the user hasn't said which ones they want (by adding them to the recipe).  These are
    *        the ones most commonly used by home brewers.  Chalk (calcium carbonate) and pickling lime (calcium
    *        hydroxide) are deliberately not included: chalk barely dissolves without extra effort, and lime is for
    *        raising pH rather than matching a profile.
    */
   static QList<Misc::WaterAgentType> const defaultSalts;

   /**
    * \brief The acids we can calculate, in order of preference for which one we add if the recipe doesn't already have
    *        one.  (Lactic is the one home brewers are most likely to have.)
    */
   static QList<Misc::WaterAgentType> const acids;

   /**
    * \return \c true if we can work out an amount for \c waterAgentType as a salt -- ie it's a salt that adds at least
    *         one of \c ions.  Chalk and pickling lime return \c false for the reasons given in \c defaultSalts.  (If the
    *         user has added either to the recipe, we leave the amount alone, but still count the ions it adds.)
    */
   static bool canCalculate(Misc::WaterAgentType const waterAgentType);

   //! \return \c true if \c waterAgentType is one of \c acids
   static bool isAcid(Misc::WaterAgentType const waterAgentType);

   /**
    * \brief Run-time wrapper around \c Misc::concentrationPerGramPerLiter_massConcPpm
    */
   static double concentrationPerGramPerLiter_ppm(Water::MineralIon const ion,
                                                  Misc::WaterAgentType const waterAgentType);

   /**
    * \brief Density, in g/mL, of an aqueous solution of an acid of the given strength (percent by weight), interpolated
    *        from published tables.  Returns 1.0 (ie water) for anything that isn't one of \c acids.
    */
   static double acidDensity_gPerMl(Misc::WaterAgentType const acid, double const strength_pct);

   /**
    * \brief How many milliequivalents of acid one mL of the solution provides at mash pH.  We count one proton per
    *        molecule for lactic acid (pKa 3.86, so almost entirely dissociated at mash pH), phosphoric acid (the
    *        second pKa is 7.2, so only the first proton counts) and hydrochloric acid, and two for sulfuric acid.
    */
   static double acidMEqPerMl(Misc::WaterAgentType const acid, double const strength_pct);

   /**
    * \brief The strength we assume if a water agent that is an acid doesn't have \c Misc::waterAgentPercentAcid set.
    *        These match the acids in the default database.
    */
   static double defaultAcidStrength_pct(Misc::WaterAgentType const acid);

   //! Below this concentration, we treat relative errors as if the target were this concentration
   static constexpr double relativeErrorFloor_ppm = 10.0;

   /**
    * \brief If the final concentration of an ion is further than this from the target (and also further than
    *        \c warningTolerance_ppm), we warn the user.
    */
   static constexpr double warningTolerance_fraction = 0.10;
   static constexpr double warningTolerance_ppm      = 5.0;

   //! If the mash pH is further than this from the target, we warn the user
   static constexpr double warningTolerance_pH = 0.05;

   /**
    * \brief Buffering capacity of the grist, in mEq per kg per pH unit -- ie how many milliequivalents of acid it takes
    *        to lower the pH of a mash with 1 kg of grain by 1.  This is the figure Kai Troester uses.  (In reality it
    *        varies by malt, roughly between 30 and 60.)
    */
   static constexpr double mashBufferCapacity_mEqPerKgPh = 50.0;

   //! The pH of a mash of grain with no color in distilled water.  Again, this is Kai Troester's figure.
   static constexpr double noColorMash_pH = 5.6;

   //! Amounts of salts are rounded to this many grams, as nobody can weigh out more precisely at home
   static constexpr double saltRounding_g = 0.01;

   //! Amounts of acid are rounded to this many mL, which is about as precise as a syringe or pipette
   static constexpr double acidRounding_ml = 0.1;

   /**
    * \brief One of the fermentables in the grist, for working out the distilled water mash pH
    */
   struct Grain {
      double amount_kg = 0.0;
      double color_srm = 0.0;
      //! Whether this is a crystal or roasted malt (which lowers mash pH less, per unit of color, than base malt)
      bool   isSpecialty = false;
   };

   struct Inputs {
      //================================================== Salts ==================================================
      //! All the water (mash and sparge) that the additions will be dissolved in -- see \c Mash::totalMashWater_l
      double totalWater_l = 0.0;
      //! The fraction (0 to 1) of \c totalWater_l that is base water (rather than RO / distilled water)
      double baseWaterFraction = 1.0;
      //! Ion concentrations in the base water.  Anything missing is taken as 0.
      QMap<Water::MineralIon, double> base_ppm;
      //! Target ion concentrations.  Anything missing is taken as 0.
      QMap<Water::MineralIon, double> target_ppm;
      /**
       * \brief Ions from additions that we are not going to change (eg chalk, or a water agent of type "Other"),
       *        already divided by \c totalWater_l.  Anything missing is taken as 0.  As well as \c ions, this can include
       *        \c Water::MineralIon::Carbonate (eg from chalk), which counts towards alkalinity.
       */
      QMap<Water::MineralIon, double> fixed_ppm;
      //! The salts we are allowed to use.  Any for which \c canCalculate returns \c false are ignored.  Can be empty.
      QList<Misc::WaterAgentType> salts;
      //! Amounts below this are dropped (and the rest recalculated without them)
      double minAddition_g = 0.1;

      //================================================= Mash pH =================================================
      //! The grist.  If this is empty, we can't work out the mash pH.
      QList<Grain> grains;
      //! Original gravity of the beer, in °P
      double og_plato = 0.0;
      //! Color of the beer, in SRM
      double beerColor_srm = 0.0;
      //! The water in the mash itself (ie strike water and any other infusions, but not sparge water)
      double mashWater_l = 0.0;
      //! The fraction (0 to 1) of \c mashWater_l that is base water (rather than RO / distilled water)
      double mashBaseWaterFraction = 1.0;
      //! Alkalinity of the base water (undiluted) in mEq/L -- see \c Water::alkalinity_mEqPerL
      double baseAlkalinity_mEqPerL = 0.0;
      //! Acid from things we are not changing (eg acidulated malt or another acid addition), in milliequivalents
      double fixedAcid_mEq = 0.0;
      //! The mash pH to aim for.  If not set, we don't work out any acid.
      std::optional<double> targetMash_pH = std::nullopt;
      //! The acid to use to get to \c targetMash_pH.  If not set, we don't work out any acid.
      std::optional<Misc::WaterAgentType> acid = std::nullopt;
      //! Strength (percent by weight) of \c acid
      double acidStrength_pct = 0.0;
   };

   struct Result {
      //! Empty if OK, otherwise why we couldn't do the calculation
      QString error;
      //! Amount, in grams, for every salt in \c Inputs::salts that we can calculate.  (Zero if not used.)
      QMap<Misc::WaterAgentType, double> amounts_g;
      //! Amount of \c Inputs::acid to add, in mL.  (Zero if we aren't adjusting pH, or no acid is needed.)
      double acid_ml = 0.0;
      //! What the ion concentrations will be with \c amounts_g added
      QMap<Water::MineralIon, double> predicted_ppm;
      //! What the mash pH will be with \c amounts_g and \c acid_ml added, if we can work it out
      std::optional<double> predictedMash_pH = std::nullopt;
      //! Things the user should know about -- eg ions we couldn't get close to the target, and why
      QStringList warnings;

      bool ok() const { return this->error.isEmpty(); }
   };

   /**
    * \brief Do the calculation.  See class comment for details.
    */
   static Result calculate(Inputs const & inputs);

   /**
    * \return The ion concentrations that would result from adding \c amounts_g to the water described by \c inputs.
    *         (The \c salts member of \c inputs is ignored.)
    */
   static QMap<Water::MineralIon, double> predict(Inputs const & inputs,
                                                  QMap<Misc::WaterAgentType, double> const & amounts_g);

   /**
    * \return The mash pH with grain in distilled water, or \c std::nullopt if there is no grain.  See class comment.
    *
    *         NB: Previous versions of the code only counted the last specialty malt in the grist, rather than all of
    *         them.
    */
   static std::optional<double> distilledWaterMashPh(Inputs const & inputs);

   /**
    * \return The mash pH that would result from adding \c amounts_g of salts and \c acid_mEq of acid (on top of
    *         \c Inputs::fixedAcid_mEq) to the water described by \c inputs, or \c std::nullopt if we can't work it out
    *         (because there is no grain or no mash water).  See class comment.
    */
   static std::optional<double> mashPh(Inputs const & inputs,
                                       QMap<Misc::WaterAgentType, double> const & amounts_g,
                                       double const acid_mEq);

   //============================================ Recipe-level functions ============================================

   /**
    * \brief What the water looks like with the recipe's current water agents.  This is what the Water tab shows.
    */
   struct Prediction {
      QMap<Water::MineralIon, double> ions_ppm;
      std::optional<double> mash_pH;
   };

   /**
    * \return The ion concentrations and mash pH for \c recipe as it currently is, or \c std::nullopt if there is no
    *         water in the mash.
    */
   static std::optional<Prediction> predict(Recipe const & recipe);

   /**
    * \brief If the recipe doesn't have a target mash pH, this is what we suggest as a starting point: the pH of the
    *        first non-sparge step of the recipe's mash that has one set (\c Step::startAcidity_pH, or failing that
    *        \c Step::endAcidity_pH).  We only ever read this -- we don't write the recipe's target back to the mash,
    *        as the mash can be shared between recipes, and mash pH depends on the recipe's grain bill and water.
    *
    * \return \c std::nullopt if there's no mash or none of its (non-sparge) steps has a pH set
    */
   static std::optional<double> suggestedTargetMashPh(Recipe const & recipe);

   /**
    * \brief The outcome of \c prepare
    */
   struct Preparation {
      //! The calculation result.  If this is not OK, no changes were prepared.
      Result result;
      //! Other things the user should know about, eg that a salt they might want isn't in the database
      QStringList notes;

      bool ok() const { return this->result.ok(); }
   };

   /**
    * \brief Work out the changes needed to \c recipe's water agents, and create (but do not execute) child commands of
    *        \c parent that will make them.
    *
    *        If the recipe already has any salts we can calculate (see \c canCalculate), we use only those and change
    *        their amounts.  This is how the user says which salts they want to use.  Otherwise, we add whichever of
    *        \c defaultSalts are needed.  Similarly, if \c Recipe::targetMashAcidity_pH is set, we adjust the recipe's
    *        acid if it has one, or else add the first of \c acids that is in the database.
    *
    *        Other water agents (chalk, pickling lime, "other", acids other than the one we are adjusting) are not
    *        changed, but their effects are taken into account.  So is any acidulated malt.  If there is more than one
    *        addition of the same salt (or of the acid we are adjusting), we put the whole amount in the first one and
    *        zero the others.
    *
    *        Salts are only adjusted if the recipe has a target water profile, and acid only if it has a target mash pH.
    *        If it has neither, the result is an error.
    *
    * \param parent  If the result is OK, the caller must then either push \c parent on to the undo stack or call its
    *                \c redo member function directly, to make the changes.  (If the result is not OK, there will be no
    *                child commands, and \c parent can be discarded.)
    */
   static Preparation prepare(Recipe & recipe, QUndoCommand & parent);

   /**
    * \brief If \c recipe has \c Recipe::waterKeptInSync set, and is not locked, recalculate its water agents and, if
    *        there are no errors, apply the results.  These changes are NOT put on the undo stack -- see comment in
    *        \c UndoableAutoAdjust for why.
    *
    *        Called (via \c Recipe::scheduleWaterAutoAdjust) when something the calculation depends on changes.
    *
    * \return The result of the calculation, or \c std::nullopt if the water was not recalculated
    */
   static std::optional<Preparation> autoAdjust(Recipe & recipe);
};

#endif
