/*======================================================================================================================
 * MashAutoAdjuster.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "MashAutoAdjuster.h"

#include <algorithm>
#include <cmath>

#include <QDebug>
#include <QSet>
#include <QUndoCommand>

#include "measurement/Measurement.h"
#include "measurement/PhysicalConstants.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "model/RecipeUtils.h"

namespace {
   double constexpr cW = PhysicalConstants::waterSpecificHeat_calGC;
   double constexpr cG = PhysicalConstants::grainSpecificHeat_calGC;

   /**
    * \brief Display temperature or volume for messages to the user.
    *
    *        We set 1 decimal place as the default precision, but, as always, this can be overridden by the user's
    *        global preferences for temperature or volume.
    */
   [[nodiscard]] QString displayTemp(double const temp_c) {
      return Measurement::displayAmount(Measurement::Amount{temp_c, Measurement::Units::celsius}, 1);
   }
   [[nodiscard]] QString displayVolume(double const volume_l) {
      return Measurement::displayAmount(Measurement::Amount{volume_l, Measurement::Units::liters}, 1);
   }

   /**
    * \brief What the water added at a step is mixed with: one or more things, each with a heat capacity and a
    *        temperature.  For the strike water, this is the grain and the tun (which can be at different temperatures).
    *        For later steps, it is the whole mash (grain, water and tun).  For batch sparges, it is the drained grain
    *        bed.
    *
    *        If we add amount A of water at temperature T_w, then, by conservation of energy, the final temperature T
    *        satisfies:
    *
    *           A × c_w × (T_w - T) = Σ HC_k × (T - T_k)
    *
    *        where HC_k and T_k are the heat capacity and initial temperature of each thing in the mix.  Given any two of
    *        A, T_w and T, we can work out the third.
    */
   struct Contents {
      struct Part {
         double heatCapacity_kcalC;
         double temp_c;
      };
      QList<Part> parts;

      double heatCapacity_kcalC() const {
         double total = 0.0;
         for (auto const & part : this->parts) {
            total += part.heatCapacity_kcalC;
         }
         return total;
      }

      //! Heat (in kcal) needed to bring the contents to the supplied temperature (negative if they need cooling)
      double heatNeeded_kcal(double const temp_c) const {
         double total = 0.0;
         for (auto const & part : this->parts) {
            total += part.heatCapacity_kcalC * (temp_c - part.temp_c);
         }
         return total;
      }

      //! Temperature we end up at if we add \c amount_l of water at \c waterTemp_c
      double finalTemp_c(double const amount_l, double const waterTemp_c) const {
         double heatCapacity = amount_l * cW;
         double energy       = amount_l * cW * waterTemp_c;
         for (auto const & part : this->parts) {
            heatCapacity += part.heatCapacity_kcalC;
            energy       += part.heatCapacity_kcalC * part.temp_c;
         }
         return heatCapacity > 0.0 ? energy / heatCapacity : waterTemp_c;
      }

      //! Water temperature needed to reach \c temp_c if we add \c amount_l of water.  Caller must ensure amount_l > 0.
      double waterTempFor(double const temp_c, double const amount_l) const {
         return temp_c + this->heatNeeded_kcal(temp_c) / (amount_l * cW);
      }

      /**
       * \brief Amount of water needed to reach \c temp_c if we add water at \c waterTemp_c, or \c std::nullopt if it
       *        can't be done (eg water is not hotter than the target temperature but we need to heat the mash).
       */
      std::optional<double> amountFor(double const temp_c, double const waterTemp_c) const {
         double const heatNeeded_kcal = this->heatNeeded_kcal(temp_c);
         double const waterTempDiff_c = waterTemp_c - temp_c;
         if (qFuzzyIsNull(heatNeeded_kcal)) {
            return 0.0;
         }
         if (qFuzzyIsNull(waterTempDiff_c)) {
            return std::nullopt;
         }
         double const amount_l = heatNeeded_kcal / (cW * waterTempDiff_c);
         if (amount_l < 0.0) {
            return std::nullopt;
         }
         return amount_l;
      }
   };

}

MashAutoAdjuster::Inputs MashAutoAdjuster::Inputs::fromRecipe(Recipe const & recipe, Mash const & mash) {
   Inputs inputs = fromRecipe(recipe);
   //
   // This mirrors Recipe::mashTunHeatCapacity_kcalC and Recipe::mashTunTemp_c, but for the supplied mash
   //
   inputs.grainTemp_c = mash.grainTemp_c();
   inputs.tunTemp_c   = mash.initialTunTemp_c().value_or(mash.grainTemp_c());
   auto const equipment = recipe.equipment();
   std::optional<double> const equipmentHeatCapacity_kcalC =
      equipment ? equipment->mashVesselHeatCapacity_kcalC() : std::nullopt;
   inputs.tunHeatCapacity_kcalC = equipmentHeatCapacity_kcalC.value_or(
      // kg × cal/(g·°C) = kcal/°C
      mash.tunWeight_kg().value_or(0.0) * mash.tunSpecificHeat_calGC().value_or(0.0)
   );
   return inputs;
}

MashAutoAdjuster::Inputs MashAutoAdjuster::Inputs::fromRecipe(Recipe const & recipe) {
   Inputs inputs;
   auto const equipment = recipe.equipment();
   auto const mash      = recipe.mash();
   inputs.grain_kg              = recipe.grainsInMash_kg();
   inputs.grainTemp_c           = mash ? mash->grainTemp_c() : 20.0;
   inputs.tunHeatCapacity_kcalC = recipe.mashTunHeatCapacity_kcalC();
   inputs.tunTemp_c             = recipe.mashTunTemp_c();
   inputs.boilingPoint_c        = equipment ? equipment->boilingPoint_c() : 100.0;
   inputs.grainAbsorption_LKg   =
      equipment ? equipment->grainAbsorption_LKg().value_or(Equipment::default_grainAbsorption_LKg) :
                  PhysicalConstants::grainAbsorption_Lkg;
   inputs.strikeWaterAllowance_l = equipment ? equipment->strikeWaterAllowance_l() : 0.0;
   inputs.targetTotalWater_l     = recipe.targetTotalMashVol_l();
   if (equipment && equipment->mashVesselVolume_l() > 0.0) {
      inputs.mashVesselVolume_l = equipment->mashVesselVolume_l();
   }
   return inputs;
}

bool MashAutoAdjuster::Step::isInfusion() const {
   return this->type == MashStep::Type::Infusion;
}

bool MashAutoAdjuster::Step::isSparge() const {
   return this->type == MashStep::Type::BatchSparge || this->type == MashStep::Type::FlySparge;
}

double MashAutoAdjuster::Step::finalTemp_c() const {
   return this->endTemp_c.value_or(this->startTemp_c);
}

bool MashAutoAdjuster::Result::ok() const {
   return std::none_of(
      this->problems.cbegin(),
      this->problems.cend(),
      [](Problem const & problem) { return problem.severity == Problem::Severity::Error; }
   );
}

QStringList MashAutoAdjuster::Result::messages(Problem::Severity const severity) const {
   QStringList messages;
   for (auto const & problem : this->problems) {
      if (problem.severity != severity) {
         continue;
      }

      if (!std::holds_alternative<Problem::StepScope>(problem.scope)) {
         messages.append(problem.message);
      } else {
         int const stepIndex = std::get<Problem::StepScope>(problem.scope).stepIndex;
         QString const & stepName = this->steps.at(stepIndex).name;
         messages.append(
            tr("%1: %2").arg(stepName.isEmpty() ? tr("Step %1").arg(stepIndex + 1) : stepName, problem.message)
         );
      }
   }
   return messages;
}

QList<MashAutoAdjuster::Step> MashAutoAdjuster::stepsFrom(Mash const & mash) {
   QList<Step> steps;
   int index = 0;
   for (auto const & mashStep : mash.mashSteps()) {
      steps.append(Step{
         .sourceIndex     = index++,
         .name            = mashStep->name(),
         .type            = mashStep->type(),
         .startTemp_c     = mashStep->startTemp_c(),
         .endTemp_c       = mashStep->endTemp_c(),
         .stepTime_mins   = mashStep->stepTime_mins(),
         .amount_l        = mashStep->amount_l(),
         .infuseTemp_c    = mashStep->infuseTemp_c(),
         .amountFixed     = mashStep->amountFixed(),
         .infuseTempFixed = mashStep->infuseTempFixed(),
         .liquorToGristRatio_lKg = mashStep->liquorToGristRatio_lKg(),
      });
   }
   return steps;
}

MashAutoAdjuster::SpargeMethod MashAutoAdjuster::spargeMethod(QList<Step> const & steps) {
   bool hasFlySparge = false;
   for (auto const & step : steps) {
      if (step.type == MashStep::Type::BatchSparge) {
         return SpargeMethod::BatchSparge;
      }
      if (step.type == MashStep::Type::FlySparge) {
         hasFlySparge = true;
      }
   }
   return hasFlySparge ? SpargeMethod::FlySparge : SpargeMethod::NoSparge;
}

void MashAutoAdjuster::setSparge(QList<Step> & steps,
                               SpargeMethod const method,
                               int const numBatches,
                               double const spargeTemp_c) {
   int const numBatchSparges = std::count_if(
      steps.cbegin(), steps.cend(), [](Step const & step) { return step.type == MashStep::Type::BatchSparge; }
   );
   int const numFlySparges = std::count_if(
      steps.cbegin(), steps.cend(), [](Step const & step) { return step.type == MashStep::Type::FlySparge; }
   );

   //
   // If the existing sparge steps already match what's asked for, leave them alone, so we don't lose any fixed values
   //
   switch (method) {
      case SpargeMethod::NoSparge:
         if (numBatchSparges == 0 && numFlySparges == 0) {
            return;
         }
         break;
      case SpargeMethod::BatchSparge:
         if (numBatchSparges == std::max(numBatches, 1) && numFlySparges == 0) {
            return;
         }
         break;
      case SpargeMethod::FlySparge:
         if (numBatchSparges == 0 && numFlySparges == 1) {
            return;
         }
         break;
   }

   steps.removeIf([](Step const & step) { return step.isSparge(); });

   auto makeSpargeStep = [spargeTemp_c](QString const & name, MashStep::Type const type) {
      return Step{
         .name          = name,
         .type          = type,
         .startTemp_c   = spargeTemp_c,
         .stepTime_mins = 15.0,
      };
   };
   if (method == SpargeMethod::BatchSparge) {
      for (int ii = 1; ii <= std::max(numBatches, 1); ++ii) {
         steps.append(makeSpargeStep(tr("Batch Sparge %1").arg(ii), MashStep::Type::BatchSparge));
      }
   } else if (method == SpargeMethod::FlySparge) {
      steps.append(makeSpargeStep(tr("Fly Sparge"), MashStep::Type::FlySparge));
   }
   return;
}

MashAutoAdjuster::Result MashAutoAdjuster::calculate(Inputs const & inputs,
                                                 QList<Step> const & stepsIn,
                                                 Options const & options) {
   Result result{.steps = stepsIn, .problems = {}};
   QList<Step> & steps = result.steps;

   auto appendWarningToResult = [&result](Problem::Scope const & scope,
                                          QString const & message) -> void {
      qDebug() << Q_FUNC_INFO << "Warning for" << scope << ":" << message;
      result.problems.append(
         Problem{.severity = Problem::Severity::Warning,
                 .scope = scope,
                 .message = message}
      );
      return;
   };
   /// Unlike appending a warning, appending an error returns the result because we're always going to bail from the
   /// function when we hit an error.  So we can write `return appendErrorToResult(...);` instead of
   /// `appendErrorToResult(...); return result`.
   auto appendErrorToResult = [&result] [[nodiscard]] (Problem::Scope const & scope,
                                                       QString const & message) -> Result const & {
      qInfo() << Q_FUNC_INFO << "Error for" << scope << ":" << message;
      result.problems.append(
         Problem{.severity = Problem::Severity::Error,
                 .scope = scope,
                 .message = message}
      );
      return result;
   };

   //
   // Basic checks on the structure
   //
   if (steps.isEmpty()) {
      return appendErrorToResult(Problem::MashScope{}, tr("There are no mash steps."));
   }
   if (inputs.grain_kg <= 0.0) {
      return appendErrorToResult(
         Problem::MashScope{},
         tr("There are no grains in the mash.  You need some mash-stage fermentable additions before the "
            "mash can be calculated.")
      );
   }

   //
   // Helper function to check water/mash/etc temperature is sane -- ie above freezing and below boiling
   //
   // We treat the initial grain temperature as the temperature of the room, and therefore also of the coldest water
   // available (eg for an infusion to cool the mash).  New mashes used to have it as 0 °C, which is almost certainly
   // not what the user meant.
   //
   auto temperatureMakesSense = [&inputs] [[nodiscard]] (double const temp_c) -> bool {
      return 0.0 < temp_c && temp_c < inputs.boilingPoint_c;
   };

   if (!temperatureMakesSense(inputs.grainTemp_c)) {
      return appendErrorToResult(
         Problem::MashPropertyScope{*PropertyNames::Mash::grainTemp_c},
         tr("The initial grain temperature is %1.  Please set it to the temperature of the grain before "
            "mashing, which is normally room temperature.").arg(displayTemp(inputs.grainTemp_c))
      );
   }
   if (!temperatureMakesSense(inputs.tunTemp_c)) {
      return appendErrorToResult(
         Problem::MashPropertyScope{*PropertyNames::Mash::initialTunTemp_c},
         tr("The initial mash tun temperature is %1, which needs to be between freezing and boiling point "
            "(%2).  (If it is not set, we assume the tun is at the same temperature as the grain.)").arg(
            displayTemp(inputs.tunTemp_c),
            displayTemp(inputs.boilingPoint_c)
         )
      );
   }
   for (qsizetype ii = 0; ii < steps.size(); ++ii) {
      Step & step = steps[ii];
      if (temperatureMakesSense(step.startTemp_c)) {
         continue;
      }
      //
      // For a sparge step, the step temperature is just the target temperature for the grain bed, so, if it doesn't
      // make sense (eg 0 °C, which older versions of the software could give a new sparge step), we can use the
      // mash's sparge temperature (or a default) instead.  For other steps, the user needs to decide what temperature
      // they want.
      //
      if (step.isSparge() && !options.checkOnly) {
         double const newTemp_c =
            temperatureMakesSense(options.spargeTemp_c) ? options.spargeTemp_c : defaultSpargeTemp_c;
         appendWarningToResult(
            Problem::StepScope{static_cast<int>(ii), *PropertyNames::StepBase::startTemp_c},
            tr("The step temperature was %1, which doesn't make sense, so it has been set to %2.").arg(
               displayTemp(step.startTemp_c),
               displayTemp(newTemp_c)
            )
         );
         step.startTemp_c = newTemp_c;
         continue;
      }
      return appendErrorToResult(
         Problem::StepScope{static_cast<int>(ii), *PropertyNames::StepBase::startTemp_c},
         tr("The step temperature is %1, which needs to be between freezing and boiling point (%2).").arg(
            displayTemp(step.startTemp_c),
            displayTemp(inputs.boilingPoint_c)
         )
      );
   }
   if (!steps.first().isInfusion()) {
      return appendErrorToResult(
         Problem::StepScope{0, *PropertyNames::MashStep::type},
         tr("The first step must be an infusion (ie adding the strike water).")
      );
   }
   //
   // Sparge steps, if any, have to come after all the other steps.  We call the other steps "mash steps".
   //
   qsizetype const numMashSteps = std::find_if(
      steps.cbegin(), steps.cend(), [](Step const & step) { return step.isSparge(); }
   ) - steps.cbegin();
   for (qsizetype ii = numMashSteps; ii < steps.size(); ++ii) {
      if (!steps[ii].isSparge()) {
         return appendErrorToResult(
            Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::type},
            tr("Sparge steps must come after all the other steps.")
         );
      }
   }
   bool const noSparge = (numMashSteps == steps.size());

   //
   // With no sparge, the last infusion whose amount isn't fixed has to make the water up to the total required.  See
   // class comment.
   //
   qsizetype balancingIndex = -1;
   if (noSparge) {
      for (qsizetype ii = 0; ii < numMashSteps; ++ii) {
         if (steps[ii].isInfusion() && !steps[ii].amountFixed) {
            balancingIndex = ii;
         }
      }
   }

   double thickness_LKg = defaultThickness_LKg;
   if (options.thickness_LKg) {
      thickness_LKg = *options.thickness_LKg;
      if (thickness_LKg <= 0.0) {
         return appendErrorToResult(Problem::MashScope{}, tr("The mash thickness must be positive."));
      }
   } else if (steps.first().liquorToGristRatio_lKg && *steps.first().liquorToGristRatio_lKg > 0.0) {
      thickness_LKg = *steps.first().liquorToGristRatio_lKg;
   } else if (steps.first().amount_l > inputs.strikeWaterAllowance_l) {
      thickness_LKg = (steps.first().amount_l - inputs.strikeWaterAllowance_l) / inputs.grain_kg;
   }

   double const grainHeatCapacity_kcalC = inputs.grain_kg * cG;
   double const grainVolume_l           = inputs.grain_kg / PhysicalConstants::grainDensity_kgL;

   double water_l           = 0.0; // Water in the mash so far (not counting sparge water)
   double priorTemp_c       = 0.0; // Final temperature of the previous step
   double maxMashVolume_l   = 0.0; // Largest the mash gets

   for (qsizetype ii = 0; ii < numMashSteps; ++ii) {
      Step & step = steps[ii];
      double const targetTemp_c = step.startTemp_c;

      //
      // What the water (or, for a decoction, the boiled part of the mash) is being mixed with
      //
      Contents contents;
      if (ii == 0) {
         contents.parts = {{grainHeatCapacity_kcalC, inputs.grainTemp_c}, {inputs.tunHeatCapacity_kcalC, inputs.tunTemp_c}};
      } else {
         contents.parts = {{grainHeatCapacity_kcalC + water_l * cW + inputs.tunHeatCapacity_kcalC, priorTemp_c}};
      }

      switch (step.type) {
         case MashStep::Type::Infusion:
            {
               std::optional<double> amount_l;
               std::optional<double> waterTemp_c;
               if (step.infuseTempFixed) {
                  if (!step.infuseTemp_c) {
                     return appendErrorToResult(
                        Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::infuseTemp_c},
                        tr("The infusion temperature is pinned, but has not been set.")
                     );
                  }
                  waterTemp_c = *step.infuseTemp_c;
               }

               //
               // Work out the amount, if it isn't fixed
               //
               if (step.amountFixed) {
                  amount_l = step.amount_l;
               } else if (ii == balancingIndex) {
                  double waterInOtherSteps_l = water_l;
                  for (qsizetype jj = ii + 1; jj < numMashSteps; ++jj) {
                     // By definition of balancingIndex, all later infusions have fixed amounts
                     if (steps[jj].isInfusion()) {
                        waterInOtherSteps_l += steps[jj].amount_l;
                     }
                  }
                  amount_l = inputs.targetTotalWater_l - waterInOtherSteps_l;
                  if (*amount_l <= 0.0) {
                     return appendErrorToResult(
                        Problem::StepScope{static_cast<int>(ii)},
                        tr("With no sparge, this step has to make up the total water needed (%1), but the other steps "
                           "already add %2.").arg(displayVolume(inputs.targetTotalWater_l), displayVolume(waterInOtherSteps_l))
                     );
                  }
               } else if (waterTemp_c) {
                  amount_l = contents.amountFor(targetTemp_c, *waterTemp_c);
                  if (!amount_l) {
                     return appendErrorToResult(
                        Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::infuseTemp_c},
                        tr("Cannot reach %1 with water at %2.").arg(displayTemp(targetTemp_c),
                                                                    displayTemp(*waterTemp_c))
                     );
                  }
               } else if (ii == 0) {
                  amount_l = thickness_LKg * inputs.grain_kg + inputs.strikeWaterAllowance_l;
               } else {
                  //
                  // We use the hottest (or, if cooling, coldest) water we reasonably can, as that minimises the amount
                  // of water needed.
                  //
                  bool const heating = contents.heatNeeded_kcal(targetTemp_c) >= 0.0;
                  double const usualWaterTemp_c = heating ? inputs.boilingPoint_c : inputs.grainTemp_c;
                  amount_l = contents.amountFor(targetTemp_c, usualWaterTemp_c);
                  if (!amount_l) {
                     return appendErrorToResult(
                        Problem::StepScope{static_cast<int>(ii), *PropertyNames::StepBase::startTemp_c},
                        tr("Cannot reach %1 with water at %2.").arg(displayTemp(targetTemp_c),
                                                                    displayTemp(usualWaterTemp_c))
                     );
                  }
                  waterTemp_c = usualWaterTemp_c;
               }

               if (*amount_l <= 0.0) {
                  if (step.amountFixed) {
                     return appendErrorToResult(
                        Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::amount_l},
                        tr("The infusion amount must be more than zero.")
                     );
                  }
                  //
                  // Otherwise, this can only happen if the step doesn't need any heat, ie it is at the same temperature
                  // as the previous one, in which case the amount of water is up to the brewer.
                  //
                  return appendErrorToResult(
                     Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::amount_l},
                     tr("This infusion does not change the temperature of the mash, so its amount cannot "
                        "be calculated.  It needs to be set and pinned.")
                  );
               }

               //
               // Work out the water temperature, if it isn't fixed (or already set above)
               //
               bool const bothGiven = waterTemp_c && (step.amountFixed || ii == balancingIndex);
               if (!waterTemp_c) {
                  waterTemp_c = contents.waterTempFor(targetTemp_c, *amount_l);
               }

               if (*waterTemp_c > inputs.boilingPoint_c + temperatureRangeTolerance_c) {
                  QString const advice = (ii == 0 && !step.amountFixed && ii != balancingIndex) ?
                     tr("Try a thinner mash (ie more water).") : tr("Try more water or a lower step temperature.");
                  return appendErrorToResult(
                     Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::infuseTemp_c},
                     tr("The water would need to be at %1, which is above boiling point (%2).  %3").arg(
                        displayTemp(*waterTemp_c), displayTemp(inputs.boilingPoint_c), advice
                     )
                  );
               }
               if (!step.infuseTempFixed && *waterTemp_c < inputs.grainTemp_c - temperatureRangeTolerance_c) {
                  //
                  // We can't supply water colder than the cold water we have, which we take to be at the initial grain
                  // temperature (see above).  This can happen, eg, if the amount is fixed (or, with no sparge, has to
                  // make up the rest of the water) and is more than is needed to reach the step temperature.
                  //
                  QString const reason = contents.heatNeeded_kcal(targetTemp_c) < 0.0 ?
                     tr("The mash would already be hotter than %1 at this point.").arg(displayTemp(targetTemp_c)) :
                     tr("This is more water than is needed to reach %1.").arg(displayTemp(targetTemp_c));
                  QString const advice =
                     step.amountFixed     ? tr("Try a smaller pinned amount, or unpinning it.") :
                     ii == balancingIndex ? tr("With no sparge, this step has to add the rest of the water needed.  Try "
                                               "having a sparge, or more water in the earlier steps.") :
                                            tr("Try a thicker mash (ie less water).");
                  return appendErrorToResult(
                     Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::infuseTemp_c},
                     tr("To reach %1 with %2 of water, the water would need to be at %3, which is colder than the "
                        "coldest water available (taken to be at the initial grain temperature, %4).  %5  %6").arg(
                        displayTemp(targetTemp_c), displayVolume(*amount_l), displayTemp(*waterTemp_c),
                        displayTemp(inputs.grainTemp_c), reason, advice
                     )
                  );
               }

               if (!step.infuseTempFixed) {
                  // If we're only just outside the range (see temperatureRangeTolerance_c), bring the value into range
                  waterTemp_c = std::clamp(*waterTemp_c, inputs.grainTemp_c, inputs.boilingPoint_c);
               }

               if (bothGiven) {
                  if (double const actualTemp_c = contents.finalTemp_c(*amount_l, *waterTemp_c);
                      std::abs(actualTemp_c - targetTemp_c) > temperatureTolerance_c) {
                     appendWarningToResult(
                        Problem::StepScope{static_cast<int>(ii)},
                        tr("%1 of water at %2 will give a mash temperature of %3 rather than %4.").arg(
                           displayVolume(*amount_l),
                           displayTemp(*waterTemp_c),
                           displayTemp(actualTemp_c),
                           displayTemp(targetTemp_c)
                        )
                     );
                  }
               }

               step.amount_l     = *amount_l;
               step.infuseTemp_c = *waterTemp_c;
               water_l += *amount_l;
               step.liquorToGristRatio_lKg = std::max(water_l - inputs.strikeWaterAllowance_l, 0.0) / inputs.grain_kg;
            }
            break;

         case MashStep::Type::Decoction:
            {
               //
               // We take out part of the mash (proportion r), heat it to boiling, and put it back.  Assuming that, while
               // this happens, the rest of the mash (and the tun) stay at priorTemp_c, we have:
               //
               //    r × HC_wg × (T_boil - T) = (HC_total - r × HC_wg) × (T - T_prior)
               //
               // where HC_wg is the heat capacity of the water and grain (ie not the tun) and HC_total includes the tun.
               // So:
               //
               //    r = HC_total × (T - T_prior) / (HC_wg × (T_boil - T_prior))
               //
               double const waterAndGrainHeatCapacity_kcalC = grainHeatCapacity_kcalC + water_l * cW;
               double const totalHeatCapacity_kcalC = waterAndGrainHeatCapacity_kcalC + inputs.tunHeatCapacity_kcalC;
               double const mashVolume_l = water_l + grainVolume_l;
               double const boilingDiff_c = inputs.boilingPoint_c - priorTemp_c;
               if (boilingDiff_c <= 0.0) {
                  return appendErrorToResult(Problem::StepScope{static_cast<int>(ii)},
                                             tr("The mash is already at boiling point."));
               }
               if (step.amountFixed) {
                  double const ratio = step.amount_l / mashVolume_l;
                  if (ratio < 0.0 || ratio > 1.0) {
                     return appendErrorToResult(
                        Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::amount_l},
                        tr("The decoction amount (%1) must be between zero and the volume of the mash "
                           "(%2).").arg(displayVolume(step.amount_l), displayVolume(mashVolume_l))
                     );
                  }
                  double const actualTemp_c =
                     priorTemp_c + ratio * waterAndGrainHeatCapacity_kcalC * boilingDiff_c / totalHeatCapacity_kcalC;
                  if (std::abs(actualTemp_c - targetTemp_c) > temperatureTolerance_c) {
                     appendWarningToResult(
                        Problem::StepScope{static_cast<int>(ii)},
                        tr("A decoction of %1 will give a mash temperature of %2 rather than %3.").arg(
                           displayVolume(step.amount_l), displayTemp(actualTemp_c), displayTemp(targetTemp_c)
                        )
                     );
                  }
               } else {
                  if (targetTemp_c < priorTemp_c) {
                     return appendErrorToResult(
                        Problem::StepScope{static_cast<int>(ii), *PropertyNames::StepBase::startTemp_c},
                        tr("A decoction cannot lower the temperature of the mash.")
                     );
                  }
                  double const ratio = totalHeatCapacity_kcalC * (targetTemp_c - priorTemp_c) /
                                       (waterAndGrainHeatCapacity_kcalC * boilingDiff_c);
                  if (ratio > 1.0) {
                     return appendErrorToResult(
                        Problem::StepScope{static_cast<int>(ii), *PropertyNames::StepBase::startTemp_c},
                        tr("Cannot reach %1 by decoction, even by boiling the whole mash.").arg(
                           displayTemp(targetTemp_c)
                        )
                     );
                  }
                  step.amount_l = ratio * mashVolume_l;
               }
               // A decoction doesn't change the amount of water in the mash
            }
            break;

         case MashStep::Type::Temperature:
         case MashStep::Type::SouringMash:
         case MashStep::Type::SouringWort:
            // Nothing to calculate
            break;

         case MashStep::Type::FlySparge:
         case MashStep::Type::BatchSparge:
            // Should be impossible, because of how we calculated numMashSteps
            qCritical() << Q_FUNC_INFO << "Unexpected sparge step at" << ii;
            break;
      }

      priorTemp_c = step.finalTemp_c();
      maxMashVolume_l = std::max(maxMashVolume_l, water_l + grainVolume_l);
   }

   if (inputs.mashVesselVolume_l && maxMashVolume_l > *inputs.mashVesselVolume_l + volumeTolerance_l) {
      appendWarningToResult(
         Problem::MashScope{},
         tr("At its fullest, the mash will be about %1, which is more than the capacity of the mash tun (%2).").arg(
            displayVolume(maxMashVolume_l), displayVolume(*inputs.mashVesselVolume_l)
         )
      );
   }

   if (noSparge) {
      if (balancingIndex < 0 && std::abs(water_l - inputs.targetTotalWater_l) > volumeTolerance_l) {
         appendWarningToResult(
            Problem::MashScope{},
            tr("The infusions add up to %1 of water, but %2 is needed in total.").arg(
               displayVolume(water_l), displayVolume(inputs.targetTotalWater_l)
            )
         );
      }
      return result;
   }

   //
   // Now the sparge steps.  First work out how much water goes in each one.
   //
   double fixedSpargeWater_l = 0.0;
   int numUnfixedSparges = 0;
   for (qsizetype ii = numMashSteps; ii < steps.size(); ++ii) {
      if (steps[ii].amountFixed) {
         fixedSpargeWater_l += steps[ii].amount_l;
      } else {
         ++numUnfixedSparges;
      }
   }
   double const remainingWater_l = inputs.targetTotalWater_l - water_l - fixedSpargeWater_l;
   double spargeAmount_l = 0.0;
   if (numUnfixedSparges > 0) {
      if (remainingWater_l < volumeTolerance_l) {
         appendWarningToResult(
            Problem::MashScope{},
            tr("There is no water left for sparging: the total water needed is %1, but the mash steps already "
               "add %2%3.  You will collect more wort than needed.  Try a thicker mash (ie less water).").arg(
               displayVolume(inputs.targetTotalWater_l),
               displayVolume(water_l),
               fixedSpargeWater_l > 0.0 ?
                  tr(" and the pinned sparge amounts add %1").arg(displayVolume(fixedSpargeWater_l)) : QString{}
            )
         );
      } else {
         spargeAmount_l = remainingWater_l / numUnfixedSparges;
      }
   } else if (std::abs(remainingWater_l) > volumeTolerance_l) {
      appendWarningToResult(
         Problem::MashScope{},
         tr("The mash and sparge water add up to %1, but %2 is needed in total.").arg(
            displayVolume(water_l + fixedSpargeWater_l),
            displayVolume(inputs.targetTotalWater_l)
         )
      );
   }

   //
   // Now the sparge water temperatures.  For a batch sparge, the water is mixed with the drained grain bed, ie the
   // grain, the water it has absorbed, and the tun.
   //
   double const bedHeatCapacity_kcalC =
      grainHeatCapacity_kcalC + inputs.grainAbsorption_LKg * inputs.grain_kg * cW + inputs.tunHeatCapacity_kcalC;
   for (qsizetype ii = numMashSteps; ii < steps.size(); ++ii) {
      Step & step = steps[ii];
      if (!step.amountFixed) {
         step.amount_l = spargeAmount_l;
      }
      // Set if we know the grain bed won't reach the step temperature
      std::optional<double> bedTemp_c = std::nullopt;
      double const targetTemp_c = step.startTemp_c;

      if (step.infuseTempFixed && !step.infuseTemp_c) {
         return appendErrorToResult(
            Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::infuseTemp_c},
            tr("The sparge water temperature is pinned, but has not been set.")
         );
      }

      if (step.type == MashStep::Type::FlySparge) {
         //
         // A fly sparge is a continuous process, so a simple heat balance doesn't really apply.  The sparge water is
         // normally just at the temperature we want the grain bed to be.
         //
         if (!step.infuseTempFixed) {
            step.infuseTemp_c = targetTemp_c;
         }
      } else if (step.amount_l > 0.0) {
         Contents const bed{{{bedHeatCapacity_kcalC, priorTemp_c - options.spargeHeatLoss_c}}};
         if (step.infuseTempFixed) {
            double const actualTemp_c = bed.finalTemp_c(step.amount_l, *step.infuseTemp_c);
            if (std::abs(actualTemp_c - targetTemp_c) > temperatureTolerance_c) {
               appendWarningToResult(
                  Problem::StepScope{static_cast<int>(ii)},
                  tr("%1 of water at %2 will give a grain bed temperature of %3 rather than %4.").arg(
                     displayVolume(step.amount_l),
                     displayTemp(*step.infuseTemp_c),
                     displayTemp(actualTemp_c),
                     displayTemp(targetTemp_c)
                  )
               );
            }
            bedTemp_c = actualTemp_c;
         } else {
            step.infuseTemp_c = bed.waterTempFor(targetTemp_c, step.amount_l);
            if (*step.infuseTemp_c < targetTemp_c) {
               //
               // The grain bed is already hotter than the step temperature (eg after a hot mash out).  It doesn't make
               // sense to use cold water to cool it, so we use water at the step temperature, and the bed will end up a
               // bit warmer than the step temperature.
               //
               step.infuseTemp_c = targetTemp_c;
               bedTemp_c = bed.finalTemp_c(step.amount_l, targetTemp_c);
               if (*bedTemp_c > targetTemp_c + temperatureTolerance_c) {
                  appendWarningToResult(
                     Problem::StepScope{static_cast<int>(ii)},
                     tr("The grain bed is already hotter than %1, so we use water at that temperature, and the bed "
                        "will be at about %2.").arg(displayTemp(targetTemp_c), displayTemp(*bedTemp_c))
                  );
               }
            }
            if (*step.infuseTemp_c > inputs.boilingPoint_c) {
               //
               // Not enough sparge water in this batch to heat the grain bed all the way to the target temperature.
               // This isn't critical (the grain bed temperature is less important for a batch sparge than for a fly
               // sparge), so we use boiling water and tell the user what temperature the bed will reach.
               //
               step.infuseTemp_c = inputs.boilingPoint_c;
               bedTemp_c = bed.finalTemp_c(step.amount_l, inputs.boilingPoint_c);
               if (*bedTemp_c < targetTemp_c - temperatureTolerance_c) {
                  appendWarningToResult(
                     Problem::StepScope{static_cast<int>(ii)},
                     tr("Even with boiling water, the grain bed will only reach %1 rather than %2.  This is "
                        "usually fine for a batch sparge, but you could use fewer batches (ie more water per "
                        "batch) or a lower sparge temperature.").arg(displayTemp(*bedTemp_c),
                                                                     displayTemp(targetTemp_c))
                  );
               }
            }
         }
      } else if (!step.infuseTempFixed) {
         // No water, so no water temperature
         step.infuseTemp_c = std::nullopt;
      }

      if (step.infuseTemp_c && *step.infuseTemp_c > inputs.boilingPoint_c + temperatureRangeTolerance_c) {
         return appendErrorToResult(
            Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::infuseTemp_c},
            tr("The sparge water would need to be at %1, which is above boiling point (%2).  Try a lower "
               "sparge temperature or more sparge water.").arg(
               displayTemp(*step.infuseTemp_c), displayTemp(inputs.boilingPoint_c)
            )
         );
      }

      priorTemp_c = bedTemp_c.value_or(step.finalTemp_c());
   }

   return result;
}

MashAutoAdjuster::Result MashAutoAdjuster::check(Inputs const & inputs, QList<Step> const & stepsIn) {
   QList<Step> steps = stepsIn;
   QList<Problem> missingAmounts;
   for (qsizetype ii = 0; ii < steps.size(); ++ii) {
      Step & step = steps[ii];
      bool const needsAmount = step.isInfusion() || step.isSparge() || step.type == MashStep::Type::Decoction;
      step.amountFixed     = !needsAmount || step.amount_l > 0.0;
      step.infuseTempFixed = step.infuseTemp_c.has_value();
      if (!step.amountFixed) {
         missingAmounts.append(
            Problem{.severity = Problem::Severity::Warning,
                    .scope = Problem::StepScope{static_cast<int>(ii), *PropertyNames::MashStep::amount_l},
                    .message = tr("No amount is set.")}
         );
      }
   }
   Result result = calculate(inputs, steps, Options{.checkOnly = true});
   result.problems.append(missingAmounts);
   return result;
}

MashAutoAdjuster::Result MashAutoAdjuster::calculate(Inputs const & inputs, QList<Step> const & steps) {
   return calculate(inputs, steps, Options{});
}

void MashAutoAdjuster::apply(QList<Step> const & steps, Mash & mash) {
   QList<std::shared_ptr<MashStep>> const existingSteps = mash.mashSteps();

   QSet<int> kept;
   int lastSourceIndex = -1;
   bool seenNewStep = false;
   for (auto const & step : steps) {
      if (step.sourceIndex < 0) {
         seenNewStep = true;
         continue;
      }
      if (step.sourceIndex >= existingSteps.size() || step.sourceIndex <= lastSourceIndex || seenNewStep) {
         //
         // This is a coding error.  We carry on (as there is no harm in updating the steps we can), but the user
         // might not get all the changes.
         //
         qCritical() <<
            Q_FUNC_INFO << "Step" << step.name << "has unexpected source index" << step.sourceIndex << "(mash has" <<
            existingSteps.size() << "steps; last source index" << lastSourceIndex << "; seenNewStep" << seenNewStep <<
            ")";
         continue;
      }
      lastSourceIndex = step.sourceIndex;
      kept.insert(step.sourceIndex);
      MashStep & mashStep = *existingSteps.at(step.sourceIndex);
      if (mashStep.amount_l() != step.amount_l) {
         mashStep.setAmount_l(step.amount_l);
      }
      if (mashStep.infuseTemp_c() != step.infuseTemp_c) {
         mashStep.setInfuseTemp_c(step.infuseTemp_c);
      }
      // The calculation can change a sparge step's temperature if it didn't make sense
      if (mashStep.startTemp_c() != step.startTemp_c) {
         mashStep.setStartTemp_c(step.startTemp_c);
      }
      if (mashStep.liquorToGristRatio_lKg() != step.liquorToGristRatio_lKg) {
         mashStep.setLiquorToGristRatio_lKg(step.liquorToGristRatio_lKg);
      }
   }

   bool stepsAddedOrRemoved = false;
   for (int ii = 0; ii < existingSteps.size(); ++ii) {
      if (!kept.contains(ii)) {
         mash.remove(existingSteps.at(ii));
         stepsAddedOrRemoved = true;
      }
   }

   for (auto const & step : steps) {
      if (step.sourceIndex >= 0) {
         continue;
      }
      auto mashStep = std::make_shared<MashStep>(step.name);
      mashStep->setType           (step.type           );
      mashStep->setStartTemp_c    (step.startTemp_c    );
      mashStep->setEndTemp_c      (step.endTemp_c      );
      mashStep->setStepTime_mins  (step.stepTime_mins  );
      mashStep->setAmount_l       (step.amount_l       );
      mashStep->setInfuseTemp_c   (step.infuseTemp_c   );
      mashStep->setAmountFixed    (step.amountFixed    );
      mashStep->setInfuseTempFixed(step.infuseTempFixed);
      mashStep->setLiquorToGristRatio_lKg(step.liquorToGristRatio_lKg);
      // Setting of step owner and storing in DB ultimately happens in OwnedSet::extend()
      mash.add(mashStep);
      stepsAddedOrRemoved = true;
   }

   if (stepsAddedOrRemoved) {
      emit mash.ownedItemsChanged();
   }
   return;
}

namespace {
   /**
    * \brief One step of a mash, as it was at a particular point, for \c ApplyStepsCommand
    */
   struct StepState {
      std::shared_ptr<MashStep> mashStep;
      //! NB: \c sourceIndex is not used here
      MashAutoAdjuster::Step    values;
   };

   QList<StepState> snapshot(Mash const & mash) {
      QList<StepState> result;
      QList<std::shared_ptr<MashStep>> const mashSteps = mash.mashSteps();
      QList<MashAutoAdjuster::Step> const steps = MashAutoAdjuster::stepsFrom(mash);
      Q_ASSERT(mashSteps.size() == steps.size());
      for (qsizetype ii = 0; ii < mashSteps.size(); ++ii) {
         result.append(StepState{mashSteps.at(ii), steps.at(ii)});
      }
      return result;
   }

   //! Set the fields of \c mashStep that have a different value in \c step.  (We don't want lots of spurious signals.)
   void setFields(MashStep & mashStep, MashAutoAdjuster::Step const & step) {
      if (mashStep.name           () != step.name           ) { mashStep.setName           (step.name           ); }
      if (mashStep.type           () != step.type           ) { mashStep.setType           (step.type           ); }
      if (mashStep.startTemp_c    () != step.startTemp_c    ) { mashStep.setStartTemp_c    (step.startTemp_c    ); }
      if (mashStep.endTemp_c      () != step.endTemp_c      ) { mashStep.setEndTemp_c      (step.endTemp_c      ); }
      if (mashStep.stepTime_mins  () != step.stepTime_mins  ) { mashStep.setStepTime_mins  (step.stepTime_mins  ); }
      if (mashStep.amount_l       () != step.amount_l       ) { mashStep.setAmount_l       (step.amount_l       ); }
      if (mashStep.infuseTemp_c   () != step.infuseTemp_c   ) { mashStep.setInfuseTemp_c   (step.infuseTemp_c   ); }
      if (mashStep.amountFixed    () != step.amountFixed    ) { mashStep.setAmountFixed    (step.amountFixed    ); }
      if (mashStep.infuseTempFixed() != step.infuseTempFixed) { mashStep.setInfuseTempFixed(step.infuseTempFixed); }
      if (mashStep.liquorToGristRatio_lKg() != step.liquorToGristRatio_lKg) {
         mashStep.setLiquorToGristRatio_lKg(step.liquorToGristRatio_lKg);
      }
      return;
   }

   /**
    * \brief Put \c mash back to how it was when we took \c state (see \c snapshot)
    */
   void restore(Mash & mash, QList<StepState> const & state) {
      bool stepsAddedOrRemoved = false;

      QList<std::shared_ptr<MashStep>> wanted;
      for (auto const & stepState : state) {
         wanted.append(stepState.mashStep);
      }
      for (auto const & mashStep : mash.mashSteps()) {
         if (!wanted.contains(mashStep)) {
            mash.remove(mashStep);
            stepsAddedOrRemoved = true;
         }
      }

      //
      // Now put back any steps that are missing, in the right places.  (NB: Mash::insert counts from 1.)  Steps that are
      // in the mash but in the wrong place shouldn't happen, as we only ever remove and add steps, but we handle it for
      // completeness.
      //
      for (qsizetype ii = 0; ii < state.size(); ++ii) {
         QList<std::shared_ptr<MashStep>> const current = mash.mashSteps();
         std::shared_ptr<MashStep> const & mashStep = state.at(ii).mashStep;
         if (ii < current.size() && current.at(ii) == mashStep) {
            continue;
         }
         if (current.contains(mashStep)) {
            qWarning() << Q_FUNC_INFO << "Moving" << *mashStep << "back to position" << ii + 1;
            mash.remove(mashStep);
         }
         mash.insert(mashStep, static_cast<int>(ii + 1));
         stepsAddedOrRemoved = true;
      }

      for (auto const & stepState : state) {
         setFields(*stepState.mashStep, stepState.values);
      }

      if (stepsAddedOrRemoved) {
         emit mash.ownedItemsChanged();
      }
      return;
   }

   /**
    * \brief See \c MashAutoAdjuster::makeApplyCommand
    */
   class ApplyStepsCommand : public QUndoCommand {
   public:
      ApplyStepsCommand(QList<MashAutoAdjuster::Step> const & steps,
                        std::shared_ptr<Mash> mash,
                        QUndoCommand * parent) :
         QUndoCommand{MashAutoAdjuster::tr("Change mash steps"), parent},
         m_steps {steps},
         m_mash  {std::move(mash)},
         m_before{snapshot(*m_mash)},
         m_after {std::nullopt} {
         return;
      }

      void redo() override {
         this->QUndoCommand::redo();
         if (!this->m_after) {
            // First time round, we do the real work
            MashAutoAdjuster::apply(this->m_steps, *this->m_mash);
            this->m_after = snapshot(*this->m_mash);
         } else {
            restore(*this->m_mash, *this->m_after);
         }
         return;
      }

      void undo() override {
         this->QUndoCommand::undo();
         restore(*this->m_mash, this->m_before);
         return;
      }

   private:
      QList<MashAutoAdjuster::Step>   const m_steps;
      std::shared_ptr<Mash>           const m_mash;
      QList<StepState>                const m_before;
      std::optional<QList<StepState>>       m_after;
   };
}

QUndoCommand * MashAutoAdjuster::makeApplyCommand(QList<Step> const & steps,
                                                  std::shared_ptr<Mash> mash,
                                                  QUndoCommand * parent) {
   Q_ASSERT(mash);
   return new ApplyStepsCommand(steps, std::move(mash), parent);
}

std::optional<MashAutoAdjuster::Result> MashAutoAdjuster::autoAdjust(Recipe & recipe) {
   std::shared_ptr<Mash> const mash = recipe.mash();
   if (!mash || !mash->keptInSync() || mash->mashSteps().isEmpty()) {
      return std::nullopt;
   }
   if (recipe.locked() || RecipeUtils::lockReason(*mash) != RecipeUtils::LockReason::NotLocked) {
      qDebug() << Q_FUNC_INFO << "Not adjusting locked" << *mash;
      return std::nullopt;
   }
   if (RecipeUtils::recipesUsing(*mash).size() != 1) {
      //
      // Shouldn't happen, as an auto-adjusting mash is copied rather than shared (see Recipe::setKey and
      // MainWindow::setForRecipe), but, if it does, we don't know which recipe to adjust it for.
      //
      qWarning() << Q_FUNC_INFO << "Not adjusting" << *mash << "as it is used by more than one recipe";
      return std::nullopt;
   }

   Options options;
   if (mash->spargeTemp_c() && *mash->spargeTemp_c() > 0.0) {
      options.spargeTemp_c = *mash->spargeTemp_c();
   }
   Result result = calculate(Inputs::fromRecipe(recipe, *mash), stepsFrom(*mash), options);
   if (result.ok()) {
      qDebug() << Q_FUNC_INFO << "Adjusting" << *mash << "for" << recipe;
      apply(result.steps, *mash);
   } else {
      qInfo() <<
         Q_FUNC_INFO << "Could not adjust" << *mash << "for" << recipe << ":" <<
         result.messages(Problem::Severity::Error);
   }
   return result;
}