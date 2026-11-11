/*======================================================================================================================
 * WaterAutoAdjuster.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "WaterAutoAdjuster.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include <QDebug>
#include <QUndoCommand>

#include "database/ObjectStoreWrapper.h"
#include "measurement/PhysicalConstants.h"
#include "measurement/Unit.h"
#include "model/Fermentable.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionMisc.h"
#include "undoRedo/SimpleUndoableUpdate.h"
#include "undoRedo/UndoableAddOrRemoveList.h"

namespace {
   constexpr std::size_t numIons = WaterAutoAdjuster::ions.size();

   //! Ion concentration from a map, defaulting to 0
   double ppmOf(QMap<Water::MineralIon, double> const & map, Water::MineralIon const ion) {
      return map.value(ion, 0.0);
   }

   /**
    * \brief The ions the water will have before we add any of the salts we are calculating
    */
   double startingPpm(WaterAutoAdjuster::Inputs const & inputs, Water::MineralIon const ion) {
      double const baseWaterFraction = std::clamp(inputs.baseWaterFraction, 0.0, 1.0);
      return baseWaterFraction * ppmOf(inputs.base_ppm, ion) + ppmOf(inputs.fixed_ppm, ion);
   }

   /**
    * \brief Concentration of \c ion from additions (both fixed ones and \c amounts_g), over all the water
    */
   double addedPpm(WaterAutoAdjuster::Inputs const & inputs,
                   QMap<Misc::WaterAgentType, double> const & amounts_g,
                   Water::MineralIon const ion) {
      double ppm = ppmOf(inputs.fixed_ppm, ion);
      if (inputs.totalWater_l > 0.0) {
         // NB: QMap::asKeyValueRange needs Qt 6.4, and we still support 6.2
         for (auto it = amounts_g.cbegin(); it != amounts_g.cend(); ++it) {
            ppm += it.value() * WaterAutoAdjuster::concentrationPerGramPerLiter_ppm(ion, it.key()) / inputs.totalWater_l;
         }
      }
      return ppm;
   }

   double totalGrain_kg(WaterAutoAdjuster::Inputs const & inputs) {
      double total_kg = 0.0;
      for (auto const & grain : inputs.grains) {
         total_kg += std::max(grain.amount_kg, 0.0);
      }
      return total_kg;
   }

   /**
    * \brief Solves the square linear system \c matrix × x = \c rhs by Gaussian elimination with partial pivoting.
    *
    * \return The solution, or \c std::nullopt if the matrix is (close to) singular.
    */
   std::optional<std::vector<double>> solveLinearSystem(std::vector<std::vector<double>> matrix,
                                                        std::vector<double> rhs) {
      std::size_t const size = rhs.size();
      double maxDiagonal = 0.0;
      for (std::size_t ii = 0; ii < size; ++ii) {
         maxDiagonal = std::max(maxDiagonal, std::abs(matrix[ii][ii]));
      }
      if (maxDiagonal <= 0.0) {
         return std::nullopt;
      }
      // Relative threshold for "this pivot is effectively zero", ie the matrix is singular
      double const pivotThreshold = maxDiagonal * 1.0e-10;

      for (std::size_t col = 0; col < size; ++col) {
         std::size_t pivotRow = col;
         for (std::size_t row = col + 1; row < size; ++row) {
            if (std::abs(matrix[row][col]) > std::abs(matrix[pivotRow][col])) {
               pivotRow = row;
            }
         }
         if (std::abs(matrix[pivotRow][col]) < pivotThreshold) {
            return std::nullopt;
         }
         std::swap(matrix[col], matrix[pivotRow]);
         std::swap(rhs   [col], rhs   [pivotRow]);
         for (std::size_t row = col + 1; row < size; ++row) {
            double const factor = matrix[row][col] / matrix[col][col];
            for (std::size_t kk = col; kk < size; ++kk) {
               matrix[row][kk] -= factor * matrix[col][kk];
            }
            rhs[row] -= factor * rhs[col];
         }
      }

      std::vector<double> solution(size, 0.0);
      for (std::size_t ii = size; ii-- > 0; ) {
         double sum = rhs[ii];
         for (std::size_t kk = ii + 1; kk < size; ++kk) {
            sum -= matrix[ii][kk] * solution[kk];
         }
         solution[ii] = sum / matrix[ii][ii];
      }
      return solution;
   }

   /**
    * \brief The weighted least-squares problem: minimise Σᵢ (wᵢ × (Σⱼ aᵢⱼ gⱼ − bᵢ))² over gⱼ ≥ 0, where gⱼ is the
    *        grams of salt j.  We store the weights already multiplied in -- ie \c columns[j][i] is wᵢaᵢⱼ and \c rhs[i]
    *        is wᵢbᵢ.
    */
   struct Problem {
      std::vector<std::array<double, numIons>> columns; // One per salt
      std::array<double, numIons> rhs;

      double residual(std::vector<double> const & amounts_g) const {
         double sumOfSquares = 0.0;
         for (std::size_t ii = 0; ii < numIons; ++ii) {
            double value = -this->rhs[ii];
            for (std::size_t jj = 0; jj < this->columns.size(); ++jj) {
               value += this->columns[jj][ii] * amounts_g[jj];
            }
            sumOfSquares += value * value;
         }
         return sumOfSquares;
      }

      /**
       * \brief Exact non-negative least squares by trying every subset of the salts.  See class comment in header.
       *
       *        Where two subsets give the same residual (eg because two salts are interchangeable), we prefer the one
       *        with fewer salts and then the one with less salt in total.
       */
      std::vector<double> solve() const {
         std::size_t const numSalts = this->columns.size();
         std::vector<double> best(numSalts, 0.0);
         double bestResidual = this->residual(best);
         std::size_t bestNumUsed = 0;
         double bestTotal_g = 0.0;

         for (unsigned int mask = 1; mask < (1u << numSalts); ++mask) {
            std::vector<std::size_t> used;
            for (std::size_t jj = 0; jj < numSalts; ++jj) {
               if (mask & (1u << jj)) {
                  used.push_back(jj);
               }
            }
            // Normal equations (AᵀA)g = Aᵀb restricted to the salts in this subset
            std::size_t const size = used.size();
            std::vector<std::vector<double>> normalMatrix(size, std::vector<double>(size, 0.0));
            std::vector<double> normalRhs(size, 0.0);
            for (std::size_t rr = 0; rr < size; ++rr) {
               auto const & colR = this->columns[used[rr]];
               for (std::size_t cc = 0; cc < size; ++cc) {
                  auto const & colC = this->columns[used[cc]];
                  for (std::size_t ii = 0; ii < numIons; ++ii) {
                     normalMatrix[rr][cc] += colR[ii] * colC[ii];
                  }
               }
               for (std::size_t ii = 0; ii < numIons; ++ii) {
                  normalRhs[rr] += colR[ii] * this->rhs[ii];
               }
            }
            auto const subSolution = solveLinearSystem(normalMatrix, normalRhs);
            if (!subSolution) {
               // Singular -- eg two salts in the subset are interchangeable.  A smaller subset will cover this case.
               continue;
            }
            // For an unconstrained optimum over this subset to be a candidate, every amount in it must be positive
            if (std::any_of(subSolution->begin(), subSolution->end(), [](double const val) { return val <= 0.0; })) {
               continue;
            }

            std::vector<double> candidate(numSalts, 0.0);
            double total_g = 0.0;
            for (std::size_t rr = 0; rr < size; ++rr) {
               candidate[used[rr]] = (*subSolution)[rr];
               total_g += (*subSolution)[rr];
            }
            double const candidateResidual = this->residual(candidate);
            double const epsilon = 1.0e-9 * (1.0 + bestResidual);
            bool const isBetter =
               (candidateResidual < bestResidual - epsilon) ||
               (std::abs(candidateResidual - bestResidual) <= epsilon &&
                (size < bestNumUsed || (size == bestNumUsed && total_g < bestTotal_g)));
            if (isBetter) {
               best         = candidate;
               bestResidual = candidateResidual;
               bestNumUsed  = size;
               bestTotal_g  = total_g;
            }
         }
         return best;
      }
   };

   QString ionName(Water::MineralIon const ion) {
      return Water::ionDisplayNames[ion];
   }

   QString ppmText(double const ppm) {
      return QString::number(ppm, 'f', 0);
   }

   QString phText(double const pH) {
      return QString::number(pH, 'f', 2);
   }

   //=========================================== Acid chemistry helpers ============================================

   struct DensityPoint {
      double strength_pct;
      double density_gPerMl;
   };

   /**
    * \brief Linear interpolation in a table of densities at 20 °C, sorted by strength.  Outside the range of the table
    *        we use the nearest end.
    */
   double interpolateDensity(std::vector<DensityPoint> const & table, double const strength_pct) {
      Q_ASSERT(!table.empty());
      if (strength_pct <= table.front().strength_pct) {
         return table.front().density_gPerMl;
      }
      for (std::size_t ii = 1; ii < table.size(); ++ii) {
         if (strength_pct <= table[ii].strength_pct) {
            DensityPoint const & lower = table[ii - 1];
            DensityPoint const & upper = table[ii];
            double const fraction = (strength_pct - lower.strength_pct) / (upper.strength_pct - lower.strength_pct);
            return lower.density_gPerMl + fraction * (upper.density_gPerMl - lower.density_gPerMl);
         }
      }
      return table.back().density_gPerMl;
   }

   double acidMolarMass(Misc::WaterAgentType const acid) {
      switch (acid) {
         case Misc::WaterAgentType::LacticAcid      : return MolarMass::LacticAcid      ;
         case Misc::WaterAgentType::PhosphoricAcid  : return MolarMass::PhosphoricAcid  ;
         case Misc::WaterAgentType::SulfuricAcid    : return MolarMass::SulfuricAcid    ;
         case Misc::WaterAgentType::HydrochloricAcid: return MolarMass::HydrochloricAcid;
         default: break;
      }
      return 0.0;
   }

   //! Protons per molecule that count at mash pH -- see comment in header for acidMEqPerMl
   double acidProtonsAtMashPh(Misc::WaterAgentType const acid) {
      return acid == Misc::WaterAgentType::SulfuricAcid ? 2.0 : 1.0;
   }

   //=========================================== Recipe-reading helpers ============================================

   /**
    * \brief Amount of a salt addition, in grams.  As in \c RecipeAdditionMiscTableModel::concentrationPerLiter_massConcPpm,
    *        if it is measured by volume, we treat 1 L as weighing 1 kg.
    */
   double saltAmount_g(RecipeAdditionMisc const & addition) {
      return 1000.0 * addition.amount().quantity;
   }

   double acidStrength_pct(Misc const & misc, Misc::WaterAgentType const acid) {
      auto const strength_pct = misc.waterAgentPercentAcid();
      return strength_pct ? *strength_pct : WaterAutoAdjuster::defaultAcidStrength_pct(acid);
   }

   //! Amount of an acid addition, in mL of solution
   double acidAmount_ml(RecipeAdditionMisc const & addition, Misc::WaterAgentType const acid, double const strength_pct) {
      if (addition.getMeasure() == Measurement::PhysicalQuantity::Mass) {
         // kg → g → mL
         return 1000.0 * addition.amount().quantity / WaterAutoAdjuster::acidDensity_gPerMl(acid, strength_pct);
      }
      // L → mL
      return 1000.0 * addition.amount().quantity;
   }

   Measurement::Amount saltAmountFor(double const amount_g) {
      double const rounded_g = std::round(amount_g / WaterAutoAdjuster::saltRounding_g) * WaterAutoAdjuster::saltRounding_g;
      return Measurement::Amount{rounded_g / 1000.0, Measurement::Units::kilograms};
   }

   /**
    * \brief Amount to set on an acid addition.  If \c existing is measured by mass, we keep it that way, otherwise we
    *        use volume (which is how acids are normally measured).
    */
   Measurement::Amount acidAmountFor(RecipeAdditionMisc const * existing,
                                     double const amount_ml,
                                     Misc::WaterAgentType const acid,
                                     double const strength_pct) {
      if (existing && existing->getMeasure() == Measurement::PhysicalQuantity::Mass) {
         double const amount_g = amount_ml * WaterAutoAdjuster::acidDensity_gPerMl(acid, strength_pct);
         return Measurement::Amount{amount_g / 1000.0, Measurement::Units::kilograms};
      }
      return Measurement::Amount{amount_ml / 1000.0, Measurement::Units::liters};
   }

   /**
    * \brief Find a water agent (ie a \c Misc) of the given type that we can add to a recipe
    */
   Misc * findWaterAgent(Misc::WaterAgentType const waterAgentType) {
      return ObjectStoreWrapper::findFirstMatching<Misc>(
         [waterAgentType](Misc * misc) {
            // NB: Need to check the type before calling waterAgentType(), otherwise it logs a warning
            return !misc->deleted() &&
                   misc->type() == Misc::Type::WaterAgent &&
                   misc->waterAgentType() == waterAgentType;
         }
      );
   }

   /**
    * \brief Whether a fermentable counts as a specialty (crystal or roasted) malt for the distilled water mash pH.
    *
    *        Previously, anything without diastatic power counted as specialty malt.  But diastatic power is often not
    *        set (eg it is optional in BeerJSON), and treating base malts as specialty malts gives a mash pH that is
    *        much too high.  So, if diastatic power is not set, we go by color instead.
    */
   bool isSpecialty(Fermentable const & fermentable) {
      if (auto const diastaticPower_lintner = fermentable.diastaticPower_lintner(); diastaticPower_lintner) {
         return *diastaticPower_lintner < 1.0;
      }
      // Base malts are typically 1.5 to 6 SRM, Munich around 9; crystal and roasted malts are darker
      return fermentable.color_srm() > 10.0;
   }

   /**
    * \brief The recipe's water agent additions, sorted by what we do with them
    */
   struct Classified {
      //! First addition of each salt we can calculate
      QMap<Misc::WaterAgentType, std::shared_ptr<RecipeAdditionMisc>> salts;
      //! Subsequent additions of salts in \c salts
      QList<std::shared_ptr<RecipeAdditionMisc>> duplicateSalts;
      //! All the acid additions, by type, in recipe order
      QMap<Misc::WaterAgentType, QList<std::shared_ptr<RecipeAdditionMisc>>> acids;
      //! Everything else (chalk, pickling lime, "other")
      QList<std::shared_ptr<RecipeAdditionMisc>> others;
   };

   Classified classify(Recipe const & recipe) {
      Classified classified;
      for (auto const & addition : recipe.miscAdditions()) {
         Misc const * const misc = addition->misc();
         if (!misc || misc->type() != Misc::Type::WaterAgent) {
            continue;
         }
         auto const waterAgentType = misc->waterAgentType();
         if (!waterAgentType) {
            continue;
         }
         if (WaterAutoAdjuster::canCalculate(*waterAgentType)) {
            if (classified.salts.contains(*waterAgentType)) {
               classified.duplicateSalts.append(addition);
            } else {
               classified.salts.insert(*waterAgentType, addition);
            }
         } else if (WaterAutoAdjuster::isAcid(*waterAgentType)) {
            classified.acids[*waterAgentType].append(addition);
         } else {
            classified.others.append(addition);
         }
      }
      return classified;
   }

   /**
    * \brief Count \c addition as something we are not changing
    */
   void addFixed(WaterAutoAdjuster::Inputs & inputs, RecipeAdditionMisc const & addition) {
      Misc const * const misc = addition.misc();
      Q_ASSERT(misc && misc->waterAgentType());
      Misc::WaterAgentType const waterAgentType = *misc->waterAgentType();
      if (WaterAutoAdjuster::isAcid(waterAgentType)) {
         double const strength_pct = acidStrength_pct(*misc, waterAgentType);
         inputs.fixedAcid_mEq +=
            acidAmount_ml(addition, waterAgentType, strength_pct) *
            WaterAutoAdjuster::acidMEqPerMl(waterAgentType, strength_pct);
         return;
      }
      if (inputs.totalWater_l <= 0.0) {
         return;
      }
      double const amount_g = saltAmount_g(addition);
      for (auto const ion : WaterAutoAdjuster::ions) {
         inputs.fixed_ppm[ion] +=
            amount_g * WaterAutoAdjuster::concentrationPerGramPerLiter_ppm(ion, waterAgentType) / inputs.totalWater_l;
      }
      // Carbonate (eg from chalk) counts towards alkalinity
      inputs.fixed_ppm[Water::MineralIon::Carbonate] +=
         amount_g *
         WaterAutoAdjuster::concentrationPerGramPerLiter_ppm(Water::MineralIon::Carbonate, waterAgentType) /
         inputs.totalWater_l;
      return;
   }

   /**
    * \brief Fill in everything in \c inputs that comes directly from the recipe -- ie everything apart from the salts
    *        and acid we are going to calculate, and the effects of the water agents we're not changing.
    */
   void readRecipe(Recipe const & recipe, WaterAutoAdjuster::Inputs & inputs) {
      auto const mash = recipe.mash();
      double const totalWater_l  = mash ? mash->totalMashWater_l     () : 0.0;
      double const infusion_l    = mash ? mash->totalInfusionAmount_l() : 0.0;
      double const sparge_l      = mash ? mash->totalSpargeAmount_l  () : 0.0;
      double const roMash_frac   = std::clamp(recipe.roWaterMash_pct  () / 100.0, 0.0, 1.0);
      double const roSparge_frac = std::clamp(recipe.roWaterSparge_pct() / 100.0, 0.0, 1.0);

      auto const base   = recipe.waterBase();
      auto const target = recipe.waterTarget();

      inputs.totalWater_l = totalWater_l;
      inputs.mashWater_l  = infusion_l;
      //
      // If no base water profile is specified then we assume it is RO water -- ie with no ions.  (This is what the
      // Water tab has always done.)
      //
      if (base && totalWater_l > 0.0) {
         double const roTotal_l = roMash_frac * infusion_l + roSparge_frac * sparge_l;
         inputs.baseWaterFraction = std::clamp(1.0 - roTotal_l / totalWater_l, 0.0, 1.0);
      } else {
         inputs.baseWaterFraction = 0.0;
      }
      inputs.mashBaseWaterFraction  = base ? 1.0 - roMash_frac : 0.0;
      inputs.baseAlkalinity_mEqPerL = base ? base->alkalinity_mEqPerL() : 0.0;
      for (auto const ion : WaterAutoAdjuster::ions) {
         if (base) {
            inputs.base_ppm[ion] = base->ionConcentration_ppm(ion).value_or(0.0);
         }
         if (target) {
            inputs.target_ppm[ion] = target->ionConcentration_ppm(ion).value_or(0.0);
         }
      }

      for (auto const & fermentableAddition : recipe.fermentableAdditions()) {
         Fermentable const * const fermentable = fermentableAddition->fermentable();
         if (!fermentable || fermentableAddition->getMeasure() != Measurement::PhysicalQuantity::Mass) {
            continue;
         }
         double const amount_kg = fermentableAddition->amount().quantity;
         switch (fermentable->type()) {
            case Fermentable::Type::Grain:
            case Fermentable::Type::Extract:
            case Fermentable::Type::Dry_Extract:
               inputs.grains.append(WaterAutoAdjuster::Grain{
                  .amount_kg   = amount_kg,
                  .color_srm   = fermentable->color_srm(),
                  .isSpecialty = isSpecialty(*fermentable),
               });
               break;
            case Fermentable::Type::Sugar:
            case Fermentable::Type::Other_Adjunct:
            case Fermentable::Type::Fruit:
            case Fermentable::Type::Juice:
            case Fermentable::Type::Honey:
               // For the moment, at least, assume these types of fermentables do not affect mash pH
               break;
         }
         // Acidulated malt.  NB: lacticAcidByWeight_pct is a percentage, eg 2.0 for 2%.
         if (auto const lacticAcid_pct = fermentable->lacticAcidByWeight_pct(); lacticAcid_pct && *lacticAcid_pct > 0.0) {
            double const lacticAcid_g = 1000.0 * amount_kg * *lacticAcid_pct / 100.0;
            inputs.fixedAcid_mEq += 1000.0 * lacticAcid_g / MolarMass::LacticAcid;
         }
      }

      inputs.og_plato      = Measurement::Units::plato.fromCanonical(recipe.og());
      inputs.beerColor_srm = recipe.color_srm();
      inputs.targetMash_pH = recipe.targetMashAcidity_pH();
      return;
   }
}

QList<Misc::WaterAgentType> const WaterAutoAdjuster::defaultSalts {
   Misc::WaterAgentType::CalciumSulfate_Dihydrate     , // Gypsum
   Misc::WaterAgentType::CalciumChloride_Dihydrate    , // Calcium chloride flake (the form most home brewers have)
   Misc::WaterAgentType::MagnesiumSulfate_Heptahydrate, // Epsom salt
   Misc::WaterAgentType::MagnesiumChloride_Hexahydrate,
   Misc::WaterAgentType::SodiumChloride               , // Table salt
   Misc::WaterAgentType::SodiumBicarbonate            , // Baking soda
};

QList<Misc::WaterAgentType> const WaterAutoAdjuster::acids {
   Misc::WaterAgentType::LacticAcid      ,
   Misc::WaterAgentType::PhosphoricAcid  ,
   Misc::WaterAgentType::SulfuricAcid    ,
   Misc::WaterAgentType::HydrochloricAcid,
};

bool WaterAutoAdjuster::canCalculate(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::CalciumChloride_Anhydrous    :
      case Misc::WaterAgentType::CalciumChloride_Dihydrate    :
      case Misc::WaterAgentType::CalciumSulfate_Dihydrate     :
      case Misc::WaterAgentType::MagnesiumSulfate_Heptahydrate:
      case Misc::WaterAgentType::MagnesiumChloride_Hexahydrate:
      case Misc::WaterAgentType::SodiumChloride               :
      case Misc::WaterAgentType::SodiumBicarbonate            :
         return true;
      case Misc::WaterAgentType::CalciumCarbonate             :
      case Misc::WaterAgentType::CalciumHydroxide             :
      case Misc::WaterAgentType::LacticAcid                   :
      case Misc::WaterAgentType::PhosphoricAcid               :
      case Misc::WaterAgentType::SulfuricAcid                 :
      case Misc::WaterAgentType::HydrochloricAcid             :
      case Misc::WaterAgentType::Other                        :
         return false;
      // No default case as we want the compiler to warn us if we missed one
   }
   Q_UNREACHABLE();
}

bool WaterAutoAdjuster::isAcid(Misc::WaterAgentType const waterAgentType) {
   return acids.contains(waterAgentType);
}

double WaterAutoAdjuster::concentrationPerGramPerLiter_ppm(Water::MineralIon const ion,
                                                         Misc::WaterAgentType const waterAgentType) {
   switch (ion) {
      case Water::MineralIon::Bicarbonate: return Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Bicarbonate>(waterAgentType);
      case Water::MineralIon::Calcium    : return Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Calcium    >(waterAgentType);
      case Water::MineralIon::Chloride   : return Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Chloride   >(waterAgentType);
      case Water::MineralIon::Magnesium  : return Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Magnesium  >(waterAgentType);
      case Water::MineralIon::Sodium     : return Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Sodium     >(waterAgentType);
      case Water::MineralIon::Sulfate    : return Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Sulfate    >(waterAgentType);
      case Water::MineralIon::Carbonate  : return Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Carbonate  >(waterAgentType);
      // None of the water agents we model adds any of the following
      case Water::MineralIon::Copper     :
      case Water::MineralIon::Iron       :
      case Water::MineralIon::Manganese  :
      case Water::MineralIon::Nitrate    :
      case Water::MineralIon::Nitrite    :
      case Water::MineralIon::Phosphate  :
      case Water::MineralIon::Potassium  :
      case Water::MineralIon::Zinc       :
         return 0.0;
      // No default case as we want the compiler to warn us if we missed one
   }
   Q_UNREACHABLE();
}

double WaterAutoAdjuster::acidDensity_gPerMl(Misc::WaterAgentType const acid, double const strength_pct) {
   //
   // Densities at 20 °C from the CRC Handbook of Chemistry and Physics ("Concentrative properties of aqueous
   // solutions") and suppliers' data sheets
   //
   static std::vector<DensityPoint> const lacticTable{
      {0.0, 1.000}, {10.0, 1.022}, {25.0, 1.057}, {50.0, 1.118}, {75.0, 1.177}, {88.0, 1.206}, {100.0, 1.230}
   };
   static std::vector<DensityPoint> const phosphoricTable{
      {0.0, 1.000}, {10.0, 1.053}, {20.0, 1.113}, {30.0, 1.181}, {50.0, 1.335}, {75.0, 1.579}, {85.0, 1.685}
   };
   static std::vector<DensityPoint> const sulfuricTable{
      {0.0, 1.000}, {10.0, 1.066}, {20.0, 1.139}, {30.0, 1.219}, {50.0, 1.395}, {75.0, 1.669}, {98.0, 1.836}
   };
   static std::vector<DensityPoint> const hydrochloricTable{
      {0.0, 1.000}, {10.0, 1.048}, {20.0, 1.098}, {30.0, 1.149}, {37.0, 1.184}
   };
   switch (acid) {
      case Misc::WaterAgentType::LacticAcid      : return interpolateDensity(lacticTable      , strength_pct);
      case Misc::WaterAgentType::PhosphoricAcid  : return interpolateDensity(phosphoricTable  , strength_pct);
      case Misc::WaterAgentType::SulfuricAcid    : return interpolateDensity(sulfuricTable    , strength_pct);
      case Misc::WaterAgentType::HydrochloricAcid: return interpolateDensity(hydrochloricTable, strength_pct);
      default: break;
   }
   return 1.0;
}

double WaterAutoAdjuster::acidMEqPerMl(Misc::WaterAgentType const acid, double const strength_pct) {
   double const molarMass = acidMolarMass(acid);
   if (molarMass <= 0.0 || strength_pct <= 0.0) {
      return 0.0;
   }
   // Grams of acid per mL of solution, then millimoles, then milliequivalents
   double const acid_gPerMl = acidDensity_gPerMl(acid, strength_pct) * strength_pct / 100.0;
   return 1000.0 * acid_gPerMl / molarMass * acidProtonsAtMashPh(acid);
}

double WaterAutoAdjuster::defaultAcidStrength_pct(Misc::WaterAgentType const acid) {
   return acid == Misc::WaterAgentType::LacticAcid ? 88.0 : 10.0;
}

QMap<Water::MineralIon, double> WaterAutoAdjuster::predict(Inputs const & inputs,
                                                         QMap<Misc::WaterAgentType, double> const & amounts_g) {
   QMap<Water::MineralIon, double> result;
   double const baseWaterFraction = std::clamp(inputs.baseWaterFraction, 0.0, 1.0);
   for (auto const ion : ions) {
      result[ion] = baseWaterFraction * ppmOf(inputs.base_ppm, ion) + addedPpm(inputs, amounts_g, ion);
   }
   return result;
}

std::optional<double> WaterAutoAdjuster::distilledWaterMashPh(Inputs const & inputs) {
   double const total_kg = totalGrain_kg(inputs);
   if (total_kg <= 0.0) {
      return std::nullopt;
   }

   //
   // Work out what proportion of the color comes from specialty malts, as their color lowers the pH less than that of
   // base malts.  We use degrees Lovibond, as in Kai Troester's spreadsheet.
   //
   // NB: Previous versions of the code capped the color of specialty malts above 120 SRM at 19 °L when working out
   //     their share (but not when working out the total), which meant that the color of roasted malts was mostly
   //     counted as if it came from base malt, giving a much too low pH for dark beers.
   //
   double weightedColor_lovibond = 0.0;
   double specialtyColor_lovibond = 0.0;
   for (auto const & grain : inputs.grains) {
      double const share = std::max(grain.amount_kg, 0.0) / total_kg;
      double const color_lovibond = (grain.color_srm + 0.6) / 1.35;
      weightedColor_lovibond += share * color_lovibond;
      if (grain.isSpecialty) {
         specialtyColor_lovibond += share * color_lovibond;
      }
   }
   double const specialtyColorRatio = weightedColor_lovibond > 0.0 ? specialtyColor_lovibond / weightedColor_lovibond : 0.0;

   if (inputs.og_plato <= 0.0) {
      return noColorMash_pH;
   }
   // Magic constants Kai Troester derives in the document mentioned in the header
   double constexpr pHSlopeLight = 0.21;
   double constexpr pHSlopeDark  = 0.06;
   double const pHAdjustment =
      (pHSlopeLight * (1.0 - specialtyColorRatio) + pHSlopeDark * specialtyColorRatio) *
      inputs.beerColor_srm / inputs.og_plato;
   return noColorMash_pH - pHAdjustment;
}

std::optional<double> WaterAutoAdjuster::mashPh(Inputs const & inputs,
                                               QMap<Misc::WaterAgentType, double> const & amounts_g,
                                               double const acid_mEq) {
   std::optional<double> const distilledWater_pH = distilledWaterMashPh(inputs);
   double const grain_kg = totalGrain_kg(inputs);
   if (!distilledWater_pH || grain_kg <= 0.0 || inputs.mashWater_l <= 0.0) {
      return std::nullopt;
   }

   //
   // Composition of the mash water.  The salts are dissolved in all the water, so their concentration in the mash water
   // is the same as over all the water.  The base water is diluted by the RO water in the mash.  The 2 is the valence of
   // Ca²⁺, Mg²⁺ and CO₃²⁻, to convert from mmol/L to mEq/L.
   //
   double const baseFraction = std::clamp(inputs.mashBaseWaterFraction, 0.0, 1.0);
   double const calcium_mEqPerL =
      (baseFraction * ppmOf(inputs.base_ppm, Water::MineralIon::Calcium) +
       addedPpm(inputs, amounts_g, Water::MineralIon::Calcium)) / MolarMass::Calcium * 2.0;
   double const magnesium_mEqPerL =
      (baseFraction * ppmOf(inputs.base_ppm, Water::MineralIon::Magnesium) +
       addedPpm(inputs, amounts_g, Water::MineralIon::Magnesium)) / MolarMass::Magnesium * 2.0;
   double const alkalinity_mEqPerL =
      baseFraction * inputs.baseAlkalinity_mEqPerL +
      addedPpm(inputs, amounts_g, Water::MineralIon::Bicarbonate) / MolarMass::Bicarbonate +
      addedPpm(inputs, amounts_g, Water::MineralIon::Carbonate  ) / MolarMass::Carbonate * 2.0;

   // Kolbach's residual alkalinity
   double const residualAlkalinity_mEqPerL = alkalinity_mEqPerL - calcium_mEqPerL / 3.5 - magnesium_mEqPerL / 7.0;

   double const net_mEq = residualAlkalinity_mEqPerL * inputs.mashWater_l - inputs.fixedAcid_mEq - acid_mEq;
   return *distilledWater_pH + net_mEq / (mashBufferCapacity_mEqPerKgPh * grain_kg);
}

WaterAutoAdjuster::Result WaterAutoAdjuster::calculate(Inputs const & inputs) {
   Result result;

   if (inputs.totalWater_l <= 0.0) {
      result.error = tr("The recipe's mash has no water in it, so there is nothing to add salts to.  Set up the mash "
                        "first (eg with Auto-Adjust in the mash editor).");
      return result;
   }

   //
   // Work out which salts we can use.  We ignore duplicates, but, since we pick the simplest solution, it doesn't
   // matter if there are two salts that do the same job (eg anhydrous and dihydrate calcium chloride).
   //
   QList<Misc::WaterAgentType> salts;
   for (auto const salt : inputs.salts) {
      if (canCalculate(salt) && !salts.contains(salt)) {
         salts.append(salt);
         result.amounts_g[salt] = 0.0;
      }
   }
   bool const adjustAcid = inputs.targetMash_pH.has_value() && inputs.acid.has_value() && isAcid(*inputs.acid);
   if (salts.isEmpty() && !adjustAcid) {
      result.error = tr("There is nothing to adjust: none of the water agents can be used to match the target profile "
                        "(chalk and pickling lime are left alone), and no acid is being adjusted for mash pH.");
      return result;
   }
   // Any more than this and trying every subset starts to take a noticeable time.  In practice there are only seven
   // salts we can calculate, so this should never happen.
   Q_ASSERT(salts.size() <= 16);

   //
   // Salts
   //
   if (!salts.isEmpty()) {
      std::array<double, numIons> weights;
      std::array<double, numIons> rhs;
      for (std::size_t ii = 0; ii < numIons; ++ii) {
         Water::MineralIon const ion = ions[ii];
         double const target_ppm = ppmOf(inputs.target_ppm, ion);
         weights[ii] = 1.0 / std::max(target_ppm, relativeErrorFloor_ppm);
         rhs    [ii] = weights[ii] * (target_ppm - startingPpm(inputs, ion));
      }

      //
      // Solve, then drop any salt whose amount is too small to bother with, and solve again with the rest.
      //
      QList<Misc::WaterAgentType> remaining = salts;
      QMap<Misc::WaterAgentType, double> amounts_g;
      while (true) {
         Problem problem;
         problem.rhs = rhs;
         for (auto const salt : remaining) {
            std::array<double, numIons> column;
            for (std::size_t ii = 0; ii < numIons; ++ii) {
               column[ii] = weights[ii] * concentrationPerGramPerLiter_ppm(ions[ii], salt) / inputs.totalWater_l;
            }
            problem.columns.push_back(column);
         }
         std::vector<double> const solution = problem.solve();

         amounts_g.clear();
         std::optional<qsizetype> smallestIndex = std::nullopt;
         for (qsizetype jj = 0; jj < remaining.size(); ++jj) {
            double const amount_g = solution[static_cast<std::size_t>(jj)];
            amounts_g[remaining[jj]] = amount_g;
            if (amount_g > 0.0 && amount_g < inputs.minAddition_g &&
                (!smallestIndex || amount_g < solution[static_cast<std::size_t>(*smallestIndex)])) {
               smallestIndex = jj;
            }
         }
         if (!smallestIndex) {
            break;
         }
         qDebug() <<
            Q_FUNC_INFO << "Dropping" << remaining[*smallestIndex] << "as amount" <<
            solution[static_cast<std::size_t>(*smallestIndex)] << "g is below minimum" << inputs.minAddition_g << "g";
         remaining.removeAt(*smallestIndex);
      }

      for (auto it = amounts_g.cbegin(); it != amounts_g.cend(); ++it) {
         result.amounts_g[it.key()] = it.value();
      }
   }
   result.predicted_ppm = predict(inputs, result.amounts_g);

   //
   // Tell the user about any ions that are a long way from the target, and, where we can, why.  (No point if we're not
   // adjusting salts.)
   //
   for (auto const ion : ions) {
      if (salts.isEmpty()) {
         break;
      }
      double const target_ppm    = ppmOf(inputs.target_ppm, ion);
      double const predicted_ppm = result.predicted_ppm.value(ion);
      double const starting_ppm  = startingPpm(inputs, ion);
      double const tolerance_ppm = std::max(warningTolerance_fraction * target_ppm, warningTolerance_ppm);
      if (std::abs(predicted_ppm - target_ppm) <= tolerance_ppm) {
         continue;
      }

      QString const comparison = tr("%1 will be %2 ppm, compared with a target of %3 ppm.").arg(
         ionName(ion), ppmText(predicted_ppm), ppmText(target_ppm)
      );
      QString reason;
      if (predicted_ppm > target_ppm) {
         if (starting_ppm > target_ppm + tolerance_ppm) {
            reason = tr("The water already has more than this before any salts are added, so you might want to use "
                        "more RO or distilled water.");
         } else {
            reason = tr("The salts that add other ions also add this one.");
         }
      } else {
         bool const anySaltAddsIt = std::any_of(
            salts.cbegin(), salts.cend(),
            [ion](Misc::WaterAgentType const salt) { return concentrationPerGramPerLiter_ppm(ion, salt) > 0.0; }
         );
         if (!anySaltAddsIt) {
            reason = tr("None of the salts being used adds this ion.");
         } else {
            reason = tr("Adding more would take other ions further from their targets.");
         }
      }
      result.warnings.append(comparison + " " + reason);
   }

   //
   // Profiles high in both calcium and bicarbonate (eg Dublin, Munich, London) historically came from water that had
   // flowed through chalk.  None of the salts we calculate add both, so, if we're short of both, point the user in the
   // right direction.
   //
   if (!salts.isEmpty()) {
      auto isShortOf = [&](Water::MineralIon const ion) {
         double const target_ppm = ppmOf(inputs.target_ppm, ion);
         double const tolerance_ppm = std::max(warningTolerance_fraction * target_ppm, warningTolerance_ppm);
         return result.predicted_ppm.value(ion) < target_ppm - tolerance_ppm;
      };
      if (isShortOf(Water::MineralIon::Calcium) && isShortOf(Water::MineralIon::Bicarbonate)) {
         result.warnings.append(
            tr("To add both calcium and bicarbonate, you could add chalk (dissolved with CO₂) or pickling lime to the "
               "recipe yourself.  Auto-Adjust takes these into account, but doesn't work out how much to add.")
         );
      }
   }

   //
   // Acid
   //
   if (!adjustAcid) {
      result.predictedMash_pH = mashPh(inputs, result.amounts_g, 0.0);
      return result;
   }

   double const target_pH = *inputs.targetMash_pH;
   double const acid_mEqPerMl = acidMEqPerMl(*inputs.acid, inputs.acidStrength_pct);
   std::optional<double> const withoutAcid_pH = mashPh(inputs, result.amounts_g, 0.0);
   if (!withoutAcid_pH) {
      result.warnings.append(
         tr("The mash pH can't be worked out (there needs to be some grain and some water in the mash), so no acid has "
            "been added.")
      );
   } else if (acid_mEqPerMl <= 0.0) {
      result.warnings.append(
         tr("The strength of the %1 is not set, so no acid has been added.").arg(
            Misc::waterAgentTypeDisplayNames[*inputs.acid]
         )
      );
   } else {
      double const needed_mEq = (*withoutAcid_pH - target_pH) * mashBufferCapacity_mEqPerKgPh * totalGrain_kg(inputs);
      if (needed_mEq > 0.0) {
         result.acid_ml = std::round(needed_mEq / acid_mEqPerMl / acidRounding_ml) * acidRounding_ml;
      } else if (*withoutAcid_pH < target_pH - warningTolerance_pH) {
         result.warnings.append(
            tr("The mash pH will be %1 without any added acid, which is already below the target of %2.  Acid can only "
               "lower the pH.  To raise it, you could use less acidulated malt or add some baking soda or pickling "
               "lime.").arg(phText(*withoutAcid_pH), phText(target_pH))
         );
      }
   }
   result.predictedMash_pH = mashPh(inputs, result.amounts_g, result.acid_ml * acid_mEqPerMl);
   return result;
}

//============================================== Recipe-level functions ==============================================

std::optional<WaterAutoAdjuster::Prediction> WaterAutoAdjuster::predict(Recipe const & recipe) {
   Inputs inputs;
   readRecipe(recipe, inputs);
   if (inputs.totalWater_l <= 0.0) {
      return std::nullopt;
   }
   for (auto const & addition : recipe.miscAdditions()) {
      Misc const * const misc = addition->misc();
      if (misc && misc->type() == Misc::Type::WaterAgent && misc->waterAgentType()) {
         addFixed(inputs, *addition);
      }
   }
   return Prediction{
      .ions_ppm = predict(inputs, {}),
      .mash_pH  = mashPh(inputs, {}, 0.0),
   };
}

std::optional<double> WaterAutoAdjuster::suggestedTargetMashPh(Recipe const & recipe) {
   auto const mash = recipe.mash();
   if (!mash) {
      return std::nullopt;
   }
   for (auto const & mashStep : mash->mashSteps()) {
      if (mashStep->isSparge()) {
         continue;
      }
      if (auto const pH = mashStep->startAcidity_pH(); pH) {
         return pH;
      }
      if (auto const pH = mashStep->endAcidity_pH(); pH) {
         return pH;
      }
   }
   return std::nullopt;
}

WaterAutoAdjuster::Preparation WaterAutoAdjuster::prepare(Recipe & recipe, QUndoCommand & parent) {
   Preparation preparation;

   if (recipe.locked()) {
      preparation.result.error = tr("The recipe \"%1\" is locked, so its water agents cannot be changed.").arg(
         recipe.name()
      );
      return preparation;
   }

   Inputs inputs;
   readRecipe(recipe, inputs);

   bool const adjustSalts = (recipe.waterTarget() != nullptr);
   bool const adjustAcid  = inputs.targetMash_pH.has_value();
   if (!adjustSalts && !adjustAcid) {
      preparation.result.error = tr("Please choose a target water profile and/or a target mash pH first.");
      return preparation;
   }

   Classified const classified = classify(recipe);

   //
   // Salts.  If the user hasn't chosen any, use the default ones (or as many of them as we can find).  If we're not
   // adjusting salts, we still need to count what's there.
   //
   QMap<Misc::WaterAgentType, Misc *> saltsToAdd;
   if (adjustSalts) {
      if (!classified.salts.isEmpty()) {
         inputs.salts = classified.salts.keys();
      } else {
         for (auto waterAgentType : defaultSalts) {
            Misc * misc = findWaterAgent(waterAgentType);
            if (!misc && waterAgentType == Misc::WaterAgentType::CalciumChloride_Dihydrate) {
               // Anhydrous calcium chloride does the same job (in different quantities)
               waterAgentType = Misc::WaterAgentType::CalciumChloride_Anhydrous;
               misc = findWaterAgent(waterAgentType);
            }
            if (misc) {
               saltsToAdd.insert(waterAgentType, misc);
               inputs.salts.append(waterAgentType);
            } else {
               preparation.notes.append(
                  tr("There is no water agent of type \"%1\" in the database, so it wasn't considered.").arg(
                     Misc::waterAgentTypeDisplayNames[waterAgentType]
                  )
               );
            }
         }
      }
   } else {
      for (auto const & addition : classified.salts.values() + classified.duplicateSalts) {
         addFixed(inputs, *addition);
      }
      preparation.notes.append(tr("There is no target water profile, so only the acid has been adjusted."));
   }
   for (auto const & addition : classified.others) {
      addFixed(inputs, *addition);
   }

   //
   // Acid.  Use the recipe's acid if it has one (going by the order of preference in `acids` if it has more than one
   // type), otherwise the first acid we can find in the database.  Any other acids are counted as they are.
   //
   std::shared_ptr<RecipeAdditionMisc> acidAddition = nullptr;
   QList<std::shared_ptr<RecipeAdditionMisc>> duplicateAcids;
   Misc * acidToAdd = nullptr;
   if (adjustAcid) {
      for (auto const acid : acids) {
         if (classified.acids.contains(acid)) {
            QList<std::shared_ptr<RecipeAdditionMisc>> const & additions = classified.acids.value(acid);
            acidAddition = additions.first();
            duplicateAcids = additions.mid(1);
            inputs.acid = acid;
            break;
         }
      }
      if (!inputs.acid) {
         for (auto const acid : acids) {
            if (Misc * misc = findWaterAgent(acid); misc) {
               acidToAdd = misc;
               inputs.acid = acid;
               break;
            }
         }
      }
      if (inputs.acid) {
         Misc const & acidMisc = acidAddition ? *acidAddition->misc() : *acidToAdd;
         inputs.acidStrength_pct = acidStrength_pct(acidMisc, *inputs.acid);
         if (!acidMisc.waterAgentPercentAcid()) {
            preparation.notes.append(
               tr("The strength of \"%1\" is not set, so it has been assumed to be %2%.").arg(
                  acidMisc.name(), QString::number(inputs.acidStrength_pct)
               )
            );
         }
      } else {
         preparation.notes.append(
            tr("There is no acid (lactic, phosphoric, sulfuric or hydrochloric) in the database, so the mash pH could "
               "not be adjusted.")
         );
      }
   }
   for (auto it = classified.acids.cbegin(); it != classified.acids.cend(); ++it) {
      if (inputs.acid && it.key() == *inputs.acid) {
         continue;
      }
      for (auto const & addition : it.value()) {
         addFixed(inputs, *addition);
      }
      if (inputs.acid) {
         preparation.notes.append(
            tr("Only the %1 has been adjusted.  The other acid in the recipe has been taken into account as it is.").arg(
               Misc::waterAgentTypeDisplayNames[*inputs.acid]
            )
         );
      }
   }

   preparation.result = calculate(inputs);
   if (!preparation.ok()) {
      return preparation;
   }
   Result const & result = preparation.result;

   //
   // Now create the commands to make the changes
   //
   auto setAmount = [&parent](RecipeAdditionMisc & addition, Measurement::Amount const & newAmount) {
      Measurement::Amount const currentAmount = addition.amount();
      if (currentAmount.unit == newAmount.unit &&
          qFuzzyCompare(1.0 + currentAmount.quantity, 1.0 + newAmount.quantity)) {
         return;
      }
      new SimpleUndoableUpdate(addition,
                               TYPE_INFO(RecipeAdditionMisc, IngredientAmount, amount),
                               QVariant::fromValue(newAmount),
                               tr("Change amount of %1").arg(addition.name()),
                               &parent);
      return;
   };
   Measurement::Amount const zero{0.0, Measurement::Units::kilograms};

   if (adjustSalts) {
      for (auto it = classified.salts.cbegin(); it != classified.salts.cend(); ++it) {
         setAmount(*it.value(), saltAmountFor(result.amounts_g.value(it.key(), 0.0)));
      }
      for (auto const & duplicate : classified.duplicateSalts) {
         setAmount(*duplicate, zero);
      }
      if (!classified.duplicateSalts.isEmpty()) {
         preparation.notes.append(
            tr("Where the same salt was added more than once, the whole amount is now in the first addition.")
         );
      }
   }

   QList<std::shared_ptr<RecipeAdditionMisc>> newAdditions;
   for (auto it = saltsToAdd.cbegin(); it != saltsToAdd.cend(); ++it) {
      double const amount_g = result.amounts_g.value(it.key(), 0.0);
      if (amount_g <= 0.0) {
         continue;
      }
      auto newAddition = std::make_shared<RecipeAdditionMisc>(recipe, *it.value());
      // Salts go in the mash (or, strictly, in all the water, including the sparge water, before it's used)
      newAddition->setStage(RecipeAddition::Stage::Mash);
      newAddition->setAmount(saltAmountFor(amount_g));
      newAdditions.append(newAddition);
   }

   if (inputs.acid) {
      if (acidAddition) {
         setAmount(*acidAddition, acidAmountFor(acidAddition.get(), result.acid_ml, *inputs.acid, inputs.acidStrength_pct));
         for (auto const & duplicate : duplicateAcids) {
            setAmount(*duplicate, acidAmountFor(duplicate.get(), 0.0, *inputs.acid, inputs.acidStrength_pct));
         }
         if (!duplicateAcids.isEmpty()) {
            preparation.notes.append(
               tr("Where the same acid was added more than once, the whole amount is now in the first addition.")
            );
         }
      } else if (acidToAdd && result.acid_ml > 0.0) {
         auto newAddition = std::make_shared<RecipeAdditionMisc>(recipe, *acidToAdd);
         newAddition->setStage(RecipeAddition::Stage::Mash);
         newAddition->setAmount(acidAmountFor(nullptr, result.acid_ml, *inputs.acid, inputs.acidStrength_pct));
         newAdditions.append(newAddition);
      }
   }

   if (!newAdditions.isEmpty()) {
      newUndoableAddOrRemoveList(recipe,
                                 &Recipe::addAddition<RecipeAdditionMisc>,
                                 newAdditions,
                                 &Recipe::removeAddition<RecipeAdditionMisc>,
                                 tr("Add water agents"),
                                 &parent);
   }

   return preparation;
}

std::optional<WaterAutoAdjuster::Preparation> WaterAutoAdjuster::autoAdjust(Recipe & recipe) {
   if (!recipe.waterKeptInSync() || recipe.locked()) {
      return std::nullopt;
   }

   // This is just a container for the changes, which we make directly, rather than putting them on the undo stack
   QUndoCommand changes;
   Preparation preparation = prepare(recipe, changes);
   if (preparation.ok()) {
      qDebug() << Q_FUNC_INFO << "Adjusting water agents for" << recipe << "(" << changes.childCount() << "changes)";
      changes.redo();
   } else {
      qInfo() << Q_FUNC_INFO << "Could not adjust water agents for" << recipe << ":" << preparation.result.error;
   }
   return preparation;
}
