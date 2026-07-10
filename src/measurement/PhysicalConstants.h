/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/PhysicalConstants.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#ifndef MEASUREMENT_PHYSICALCONSTANTS_H
#define MEASUREMENT_PHYSICALCONSTANTS_H
#pragma once

//
// TBD: Strictly speaking, we should perhaps put everything here in the PhysicalConstants namespace (below).  For the
//      moment, at least, I have not done this because it would make usage even more unwieldy (eg
//      PhysicalConstants::MolarMasses::Bicarbonate_gramsPerMole instead of MolarMasses::Bicarbonate_gramsPerMole).
//      This does make it mildly less obvious where to look for, eg, MolarMasses definitions, but I don't think it's
//      going to baffle anyone.
//

/**
 * Per https://en.wikipedia.org/wiki/Mole_(unit), in chemistry, one mole is an aggregate of exactly 6.02214076 × 10²³
 * elementary entities which can be atoms, molecules, ions, ion pairs, or other particles.
 *
 * Of course, no-one wants to keep repeating 6.02214076 × 10²³, so this number has a name: the Avogadro constant (aka
 * Avogadro's number).  And it is generally written as 'N' followed by subscript 'A'.  That's a bit hard for us to
 * represent in source code, so we write NAv instead.  (N_A is another common representation in source code.)
 *
 * The value of NAv was chosen to make other sums easier -- eg the mass of 1 mole of carbon-12 is 12 grams.  Thus,
 * the molar mass of carbon-12 is 12 g/mol, which corresponds to carbon-12's atomic weight of 12.  (Strictly, the units
 * for atomic mass and molecular mass are Daltons, but we don't need to worry about that!)
 */
namespace MolarMass {
   //
   // These values come from https://iupac.qmul.ac.uk/AtWt/, which lists atomic weights for all elements.  For molecules
   // etc, we "do the math" (which is fortunately trivial).
   //
   // For elements, the comment at the end of each line shows the original figure(s) from the source.  Note that "a
   // number in parentheses indicates the uncertainty in the last digit of the atomic weight".
   //
   // For a dozen elements, including magnesium, there is a range of atomic weights because such "elements [have] two or
   // more stable isotopes [and thus] have variability of atomic-weight values in natural terrestrial materials".
   // Fortunately, the source also provides a single suggested value to use "for material where the origin of the sample
   // is unknown", so this is what we use below in such cases.
   //
   // NOTE that we treat the molar mass of an ion (eg Cl⁻) as the same as that of its corresponding element (eg Cl).
   // Strictly speaking, an ion's molar mass differs from that of the neutral atom by the mass of the electrons gained
   // or lost, but the difference is far below the precision of most chemical calculations.
   //
   // At first, in line with our general convention about including units in variable names, I had "_gramsPerMole" on
   // the end of all these names -- eg Calcium_gramsPerMole, Magnesium_gramsPerMole.  However, this gets cumbersome in a
   // few places.  So, given that the units of molar mass are always grams per mole, even in the USA(!), we make an
   // exception and do not specify them in the constant names here.  Where relevant, we still use the "_gramsPerMole"
   // suffix on variables elsewhere in the code.
   //

   // ======== Elements ========
   double constexpr Calcium    = 40.0784      ; // 40.078(4)         <- Ca
   double constexpr Carbon     = 12.0112      ; // 12.011(2)         <- C
   double constexpr Chlorine   = 35.45        ; // 35.45             <- Cl
   double constexpr Hydrogen   =  1.00802     ; //  1.0080(2)        <- H
   double constexpr Magnesium  = 24.3052      ; // 24.305(2)         <- Mg
   double constexpr Oxygen     = 15.9991      ; // 15.999(1)         <- O
   double constexpr Phosphorus = 30.9737619985; // 30.973 761 998(5) <- P
   double constexpr Sodium     = 22.989769282 ; // 22.989 769 28(2)  <- Na
   double constexpr Sulfur          = 32.062       ; // 32.06(2)          <- S

   // ======== Monatomic Ions with different names from their corresponding elements ========
   double constexpr Chloride  = Chlorine;

   // ======== Polyatomic ions ========
   double constexpr Bicarbonate = Hydrogen + Carbon + 3 * Oxygen; // <- HCO₃⁻
   double constexpr Carbonate   = Carbon + 3 * Oxygen;            // <- CO₃²⁻
   double constexpr Sulfate     = Sulfur + 4 * Oxygen;            // <- SO₄²⁻
   double constexpr Phosphate   = Phosphorus + 4 * Oxygen;        // <- PO₄³⁻

   // ======== Molecules ========
   double constexpr CalciumChloride   = Calcium + 2 * Chloride  ; // <- CaCl₂
   double constexpr CalciumCarbonate  = Calcium + Carbonate     ; // <- CaCO₃
   double constexpr CalciumSulfate    = Calcium + Sulfate       ; // <- CaSO₄
   double constexpr LacticAcid        = 3 * Carbon + 6 * Hydrogen + 3 * Oxygen; // <- CH₃CH(OH)CO₂H (extended formula) / C₃H₆O₃ (regular formula)
   double constexpr MagnesiumSulfate  = Magnesium + Sulfate     ; // <- MgSO₄
   double constexpr SodiumChloride    = Sodium + Chloride       ; // <- NaCl (AKA regular salt)
   double constexpr SodiumBicarbonate = Sodium + Bicarbonate    ; // <- NaHCO₃
   double constexpr PhosphoricAcid    = 3 * Hydrogen + Phosphate; // <- H₃PO₄

}


/*!
 * \brief Collection of other physical constants like density of materials.
 */
namespace PhysicalConstants {
   //! \brief Sucrose density in kg per L.
   double constexpr sucroseDensity_kgL = 1.587;
   //! \brief This estimate for grain density is from my own (Philip G. Lee) experiments.
   double constexpr grainDensity_kgL = 0.963;
   //! \brief Liquid extract density in kg per L.
   double constexpr liquidExtractDensity_kgL = 1.412;
   //! \brief Dry extract density in kg per L.
   double constexpr dryExtractDensity_kgL = sucroseDensity_kgL;

   //! \brief How many liters of water get absorbed by 1 kg of grain.
   double constexpr grainAbsorption_Lkg = 1.085;

   double constexpr absoluteZero = -273.15;

   /***Specific heats***/

   /**
    * \brief Specific heat capacity of water = 4184 J⋅kg⁻¹⋅K⁻¹ per https://en.wikipedia.org/wiki/Specific_heat_capacity
    *                                        = 1 c/g·C
    */
   double constexpr waterSpecificHeat_calGC = 1.0;

   /**
    * \brief Specific heat to use for grain.
    *
    *        Of course there is not one exact figure to use, but this is a reasonable approximation.
    *
    *        See International Agrophysics 1994, 8, 271-275: "Thermal characteristics of barley and oat" by
    *        Bogusława Łapczyńska-Kordon, A Zaremba, K. Kempkiewicz (available at
    *        http://www.international-agrophysics.org/Thermal-characteristics-of-barley-and-oat,139711,0,2.html) for
    *        more detailed analysis on how the specific heats of oats and barley vary by moisture content.
    */
   double constexpr grainSpecificHeat_calGC = 0.4;

}

#endif
