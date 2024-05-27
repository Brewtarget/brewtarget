/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Fermentable.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Blair Bonnett <blair.bonnett@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#ifndef MODEL_FERMENTABLE_H
#define MODEL_FERMENTABLE_H
#pragma once

#include <array>
#include <optional>

#include <QStringList>
#include <QString>
#include <QSqlRecord>

#include "measurement/Amount.h"
#include "measurement/ConstrainedAmount.h"
#include "measurement/Unit.h"
#include "model/Ingredient.h"
#include "model/IngredientBase.h"
#include "model/IngredientAmount.h"
#include "utils/EnumStringMapping.h"

class InventoryFermentable;
class RecipeAdditionFermentable;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Fermentable { BtStringConst const property{#property}; }
AddPropertyName(alphaAmylase_dextUnits   )
AddPropertyName(betaGlucan_ppm           )
AddPropertyName(coarseFineDiff_pct       )
AddPropertyName(coarseGrindYield_pct     )
AddPropertyName(color_srm                )
AddPropertyName(diastaticPower_lintner   )
AddPropertyName(di_ph                    )
AddPropertyName(dmsP_ppm                 )
AddPropertyName(fan_ppm                  )
AddPropertyName(fermentability_pct       )
AddPropertyName(fineGrindYield_pct       )
AddPropertyName(friability_pct           )
AddPropertyName(grainGroup               )
AddPropertyName(hardnessPrpGlassy_pct    )
AddPropertyName(hardnessPrpHalf_pct      )
AddPropertyName(hardnessPrpMealy_pct     )
AddPropertyName(ibuGalPerLb              )
AddPropertyName(kernelSizePrpPlump_pct   )
AddPropertyName(kernelSizePrpThin_pct    )
AddPropertyName(kolbachIndex_pct         )
AddPropertyName(maxInBatch_pct           )
AddPropertyName(moisture_pct             )
AddPropertyName(notes                    )
AddPropertyName(origin                   )
AddPropertyName(potentialYield_sg        )
AddPropertyName(producer                 )
AddPropertyName(productId                )
AddPropertyName(protein_pct              )
AddPropertyName(recommendMash            )
AddPropertyName(supplier                 )
AddPropertyName(type                     )
AddPropertyName(viscosity_cP             )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/*!
 * \class Fermentable
 *
 * \brief Model for a fermentable record in the database.
 */
class Fermentable : public Ingredient, public IngredientBase<Fermentable> {
   Q_OBJECT

   INGREDIENT_BASE_DECL(Fermentable)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /**
    * \brief The type of Fermentable.
    */
   enum class Type {Grain,
                    Sugar,
                    Extract,
                    Dry_Extract,
                    Other_Adjunct,     // Was Adjunct.  Corresponds to "other" in BeerJSON
                    // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
                    Fruit,
                    Juice,
                    Honey};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   /*!
    * \brief Mapping between \c Fermentable::Type and string values suitable for serialisation in DB, BeerJSON, etc (but
    *        \b not BeerXML)
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const typeStringMapping;

   /*!
    * \brief Localised names of \c Fermentable::Type values suitable for displaying to the end user
    */
   static EnumStringMapping const typeDisplayNames;

   /**
    * \brief An additional classification of \c Fermentable introduced in BeerJSON
    *
    *        The schema doesn't elaborate, but it only makes sense for this to take a value when \c type == \c Grain
    *        Note that, for optional or not-always-valid values such as this, we don't have an enum value for NULL;
    *        instead we include the "nullability" via std::optional in whatever variables hold the enum values.  The
    *        rationale for this is that it means the compiler knows something is nullable, and can therefore help
    *        enforce that we handle the null case.
    */
   enum class GrainGroup {Base,
                          Caramel,
                          Flaked,
                          Roasted,
                          Specialty,
                          Smoked,
                          Adjunct};
   // This allows us to store the above enum class in a QVariant.  Note, however, that for serialisation we will
   // actually store std::optional<int> in QVariant for the reasons explained in the comment above Q_DECLARE_METATYPE in
   // model/NamedEntity.h
   Q_ENUM(GrainGroup)

   /*!
    * \brief Mapping between \c Fermentable::GrainGroup and string values suitable for serialisation in DB, BeerJSON,
    *        BeerXML, etc.
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const grainGroupStringMapping;

   /*!
    * \brief Localised names of \c Fermentable::GrainGroup values suitable for displaying to the end user
    */
   static EnumStringMapping const grainGroupDisplayNames;

   /**
    * \brief This is where we centrally define how \c Fermentable objects can be measured.  Essentially, mass is used in
    *        almost all cases, but volume is eg useful for \c Fermentable::Type::Juice
    */
   static constexpr auto validMeasures  = Measurement::ChoiceOfPhysicalQuantity::Mass_Volume;
   static constexpr auto defaultMeasure = Measurement::PhysicalQuantity::Mass;

   //
   // These aliases make it easier to template a number of functions that are essentially the same for all subclasses of
   // Ingredient.
   //
   using InventoryClass      = InventoryFermentable;
   using RecipeAdditionClass = RecipeAdditionFermentable;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Fermentable(QString name = "");
   Fermentable(NamedParameterBundle const & namedParameterBundle);
   Fermentable(Fermentable const & other);

   virtual ~Fermentable();

   //=================================================== PROPERTIES ====================================================
   //! \brief The \c Type.
   Q_PROPERTY(Type           type                   READ type                   WRITE setType                               )
   //! \brief The color in SRM.
   Q_PROPERTY(double         color_srm              READ color_srm              WRITE setColor_srm                          )
   //! \brief The origin.
   Q_PROPERTY(QString        origin                 READ origin                 WRITE setOrigin                             )
   //! \brief The supplier.  NB: Not supported by BeerJSON (which does have Producer and Product ID)
   Q_PROPERTY(QString        supplier               READ supplier               WRITE setSupplier                           )
   //! \brief The notes.
   Q_PROPERTY(QString        notes                  READ notes                  WRITE setNotes                              )
   /**
    * \brief Extract Fine Grind/Coarse Grind Difference (FG/CG) - aka the difference in yield between coarsely milled
    *        and finely milled grain.  A FG/CG difference of 0.5–1.0 percentage points is well suited to a single step
    *        infusion, while a value greater than 1.5 percentage points indicates that a protein rest may be advisable.
    *
    *        Optional in both BeerJSON and BeerXML.
    *
    *        Note that \c coarseFineDiff_pct = \c fineGrindYield_pct - \c coarseGrindYield_pct
    *        .:TODO:. We should attempt to enforce this when two or more of the values are set.
    */
   Q_PROPERTY(std::optional<double>   coarseFineDiff_pct     READ coarseFineDiff_pct     WRITE setCoarseFineDiff_pct                 )
   //! \brief The moisture in pct.  Only appropriate for a "Grain" or "Other_Adjunct" type.   NB Optional in both BeerXML and BeerJSON.
   Q_PROPERTY(std::optional<double>   moisture_pct           READ moisture_pct           WRITE setMoisture_pct                       )
   //! \brief The diastatic power in Lintner.  Only appropriate for a "Grain" or "Other_Adjunct" type.  NB Optional in both BeerXML and BeerJSON.
   Q_PROPERTY(std::optional<double> diastaticPower_lintner READ diastaticPower_lintner WRITE setDiastaticPower_lintner             )
   //! \brief The percent protein.  Only appropriate for a "Grain" or "Other_Adjunct" type.  NB Optional in both BeerXML and BeerJSON.
   Q_PROPERTY(std::optional<double> protein_pct            READ protein_pct            WRITE setProtein_pct                        )
   //! \brief The maximum recommended amount in a batch, as a percentage of the total grains.  NB Optional in both BeerXML and BeerJSON.
   Q_PROPERTY(std::optional<double> maxInBatch_pct         READ maxInBatch_pct         WRITE setMaxInBatch_pct                     )
   /**
    * \brief Whether a mash is recommended. \c true means \c Fermentable must be mashed, \c false means if it can be
    *        steeped.  Note that this does NOT indicate whether the \c Fermentable is mashed or not – it is only a
    *        recommendation used in recipe formulation.  NB Optional in both BeerXML and BeerJSON.
    */
   Q_PROPERTY(std::optional<bool>   recommendMash          READ recommendMash          WRITE setRecommendMash                      )
   /**
    * \brief For hopped extracts only - an estimate of the number of IBUs per pound of extract in a gallon of water.
    *        To convert to IBUs we multiply this number by the amount in pounds and divide by the number of gallons in
    *        the batch.  Based on a sixty minute boil.  Only suitable for use with an "Extract" type, otherwise this
    *        value is ignored.
    *
    *        This is an optional field in BeerXML and unsupported in BeerJSON.
    *
    *        .:TBD:. If we care about this, then it would be more consistent to store in internally as a metric measure.
    */
   Q_PROPERTY(std::optional<double> ibuGalPerLb            READ ibuGalPerLb            WRITE setIbuGalPerLb                        )
   //! \brief Whether this fermentable is an extract.
   Q_PROPERTY(bool           isExtract              READ isExtract                                              STORED false)
   //! \brief Whether this fermentable is a sugar. Somewhat redundant, but it makes for nice symmetry elsewhere
   Q_PROPERTY(bool           isSugar                READ isSugar                                                STORED false)

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   /**
    * \brief If \c type is \c Grain then this gives more detail, otherwise it's \c std::nullopt
    *
    *        Note that type has to be std::optional<int> in the interface rather than std::optional<GrainGroup>,
    *        otherwise generic code (for serialisation to/from DB, BeerJSON, BeerXML) will not be able to access the
    *        value.  We don't need to do this trick for non-optional enum fields, as the Qt property system will do the
    *        necessary casting for us.  But, you can't "just cast" between std::optional<GrainGroup> and
    *        std::optional<int>.
    */
   Q_PROPERTY(std::optional<int>    grainGroup              READ grainGroupAsInt         WRITE setGrainGroupAsInt       )
   Q_PROPERTY(QString               producer                READ producer                WRITE setProducer              )
   Q_PROPERTY(QString               productId               READ productId               WRITE setProductId             )
   /**
    * \brief Extract Yield Dry Basis Fine Grind (DBFG) - aka percentage yield, compared to sucrose, of a fine grind.
    *
    *        We treat this as synonymous with the BeerXML field YIELD, defined as "Percent dry yield (fine grain) for
    *        the grain, or the raw yield by weight if this is an extract adjunct or sugar."  HOWEVER, note that the
    *        BeerXML field is required whereas internally, and in BeerJSON, this is an optional field.
    *
    *        NOTE: Actually in BeerJSON, there is a required field of yield which is a record containing optional fields
    *              fine_grind, coarse_grind, fine_coarse_difference and potential.  Technically if none of these fields
    *              is set, then we should still write out an empty yield record, but we would not.  Hopefully this won't
    *              be a problem in practice.
    */
   Q_PROPERTY(std::optional<double> fineGrindYield_pct      READ fineGrindYield_pct      WRITE setFineGrindYield_pct    )
   //! \brief Extract Yield Dry Basis Coarse Grind (DBCG) - aka percentage yield, compared to sucrose, of a coarse grind
   Q_PROPERTY(std::optional<double> coarseGrindYield_pct    READ coarseGrindYield_pct    WRITE setCoarseGrindYield_pct  )
   /**
    * \brief The potential yield is the specific gravity that can be achieved with 1.00 pound (455 g) of malt mashed in
    *        1.00 gallon (3.78 L) of water.  Calculated as (extract) potential (SG) = 1 + (DBFG / 100) * 0.04621.
    */
   Q_PROPERTY(std::optional<double> potentialYield_sg       READ potentialYield_sg       WRITE setPotentialYield_sg     )
   /**
    * \brief Total amount of alpha-amylase in the malted grain, measured in dextrinizing units.  (Since you ask, one
    *        α-amylase dextrinizing unit is defined as the quantity of α-amylase that will dextrinize soluble starch in
    *        the presence of an excess of β-amylase at the rate of 1 g/h at 30°C.  Or, at least, that's what it says at
    *        https://www.deerland.com/wp-content/uploads/2015/04/EnzymeAssayUnits_Deerland.pdf.)  Anyway, a value of
    *        25-50 is desirable for base malt.
    *
    *        It would be great if we could have variable and property names such as αAmylase_dextUnits, but Qt's MOC
    *        cannot handle them, so we don't. Also capital a (A) and capital α (Α) look far too similar in most fonts to
    *        risk setΑAmylase_dextUnits!)
    */
   Q_PROPERTY(std::optional<double> alphaAmylase_dextUnits  READ alphaAmylase_dextUnits  WRITE setAlphaAmylase_dextUnits)
   /**
    * \brief The Kolbach Index, also known as soluble to total ratio of nitrogen or protein, is used to indicate the
    *        degree of malt modification. A value above 35% is desired for simple single infusion mashing, undermodified
    *        malt may require multiple step mashes or decoction.
    */
   Q_PROPERTY(std::optional<double> kolbachIndex_pct   READ kolbachIndex_pct   WRITE setKolbachIndex_pct      )

   /**
    * \brief Percentage of malt that is "glassy".  For a malt, % "glassy" + % "half glassy" + % "mealy" = 100%.
    *
    *        From https://byo.com/article/understanding-malt-spec-sheets-advanced-brewing/:
    *
    *           Malt is also classified in terms of hardness. By convention, it is described as “mealy,” “half-glassy”
    *           and “glassy.” Mealy kernels have an endosperm (the partially geminated portion at the heart of the
    *           kernel that contains the starches) that is 25% or less glassy (hard). Glassy kernels have an endosperm
    *           that is more than 75% hard. The remaining kernels (26–75% hard) are said to be half-glassy.
    *
    *        See also https://www.probrewer.com/library/malt/understanding-malt-analysis-sheets/:
    *
    *           By convention, malt is classified by what percentage of the lot is “mealy,” “half-glassy/glassy-ends”
    *           and “glassy.”
    *           ...
    *           Any base malt destined for brewing should be at least 90% mealy; if it is to be infusion-mashed it must
    *           be at least 95% mealy.  For base malts whose mealiness is expressed as a ratio,
    *           mealy/half-glassy/glassy, the ratio should be 92%/7%/1% for decoction and step mashing, and 95%/4%/1% or
    *           better for infusion mashing.
    */
   Q_PROPERTY(std::optional<double> hardnessPrpGlassy_pct   READ hardnessPrpGlassy_pct   WRITE setHardnessPrpGlassy_pct )

   /**
    * \brief Percentage of malt that is "half glassy".  For a malt, % "glassy" + % "half glassy" + % "mealy" = 100%.
    */
   Q_PROPERTY(std::optional<double> hardnessPrpHalf_pct     READ hardnessPrpHalf_pct     WRITE setHardnessPrpHalf_pct   )

   /**
    * \brief Percentage of malt that is "mealy".  For a malt, % "glassy" + % "half glassy" + % "mealy" = 100%.
    *
    *        (The opposite of mealiness is “vitreosity,” which is sometimes used as an alternative measurement.  A value
    *        of 1 is assigned to glassy (vitreous) kernels, 0.5 to half-glassy and 0 to mealy kernels. The percentages
    *        of each are summed and averaged; a vitreosity value of 0.25 or less is considered desirable.)
    */
   Q_PROPERTY(std::optional<double> hardnessPrpMealy_pct    READ hardnessPrpMealy_pct    WRITE setHardnessPrpMealy_pct  )

   /**
    * \brief Percentage of grain that is "plump". The percentage of grain that masses through sieves with gaps of 7/64"
    *        and 6/64", desired values of 80% or higher which indicate plump kernels.
    *
    *        From https://byo.com/article/understanding-malt-spec-sheets-advanced-brewing/:
    *
    *            The kernel size [of the malt] is typically expressed in terms of screen separation, that is, the
    *            fraction of kernels that do not pass through screens of various sizes.  In general, larger kernels will
    *            exhibit higher extract yields.  Kernels smaller than 2 mm (0.079 in.) can be indications of poor or
    *            nonexistent modification.  Sometimes the size value is given only in terms of the percentage of kernels
    *            that are “plump” or “thin.”  Malt that is more than 2% thin can cause problems when it is milled; a
    *            relatively uniform kernel size is desirable from this standpoint.
    *
    *        From https://www.probrewer.com/library/malt/understanding-malt-analysis-sheets/:
    *
    *            European malts often list only the percentage of malt that can be sieved through 2.2 mm openings.
    *            Brewers will reject a malt if it’s more than 1% thin or 2% less than 2.2 mm, because these values
    *            indicate unmodified kernels.  Other analyses are given in terms of screen separation and brewers will
    *            typically see percentages of kernels that will remain on a screen with 5/64 inch, 6/64 inch, and 7/64
    *            inch openings.  Kernels considered thin will fall through the 5/64-in. opening. Generally speaking, the
    *            plumper the malt kernels, the better the yield.  The uniformity of malt sizes measures how uniformly
    *            the malt will crush. Any lot of malt that will crush reasonably well must have kernels that are at
    *            least 90% adjacent sizes, regardless of the plumpness.
    */
   Q_PROPERTY(std::optional<double> kernelSizePrpPlump_pct  READ kernelSizePrpPlump_pct  WRITE setKernelSizePrpPlump_pct)

   /**
    * \brief Percentage of grain that is "tine", ie makes it through a thin mesh screen, typically 5/64 inch.
    *        Values less than 3% are desired.  (In BeerJSON this is called "thru".)
    */
   Q_PROPERTY(std::optional<double> kernelSizePrpThin_pct   READ kernelSizePrpThin_pct   WRITE setKernelSizePrpThin_pct)

   /**
    * \brief Friability is the relative ease of crumbling when a malt is milled.  It is related to mealiness, and may be
    *        reported in its place.  It is used as an indicator for easy gelatinization of the grain and starches, as
    *        well as modification of the malt.  All malt should be at least 80% friable.  A value of 85% of higher
    *        indicates a well modified malt and is suitable for single step / infusion mashes. Lower values may require
    *        a step mash.
    */
   Q_PROPERTY(std::optional<double> friability_pct       READ friability_pct       WRITE setFriability_pct     )

   /**
    * \brief DI pH is the pH of the resultant wort for 40 grams of grain mashed in 100 mL of distilled water (or 1 lb of
    *        grain mashed in 1 gallon of distilled water).  Can be used in water chemistry and/or mash pH prediction
    *        calculations.
    *
    *        As explained at https://www.thescrewybrewer.com/2016/12/the-influence-of-grain-di-ph-on-mash-ph.html, "The
    *        term DI pH is derived from the word distilled as in distilled water and pH as in pH value.  You can
    *        determine the DI pH value of any grain by finely crushing 40 grams of grain, mixing it in with 100
    *        milliliters of distilled water and then heating the resulting mash to 125°F [52°C] for 20 minutes.  A pH
    *        reading taken of the resulting wort when cooled to 77°F [25°C] is the DI pH value of the grain.  Various
    *        grain types, and sometimes the same grain type sourced from several maltsters, will have different DI pH
    *        values.  Accurately calculating brewing water adjustments to optimize enzyme activity in the mash is
    *        largely dependent on knowing the correct DI pH value for each grain."
    */
   Q_PROPERTY(std::optional<double> di_ph               READ di_ph                WRITE setDi_ph     )

   /**
    * \brief Viscosity of this malt in a "congress mash".  A congress mash is a standardized small-scale mashing
    *        procedure, instituted by the European Brewing Congress (EBC) in 1975, to produce a "congress wort" that is
    *        used to assess malt quality.
    *
    *        Wort viscosity is typically associated with the breakdown of beta-glucans.  The higher the viscosity, the
    *        greater the need for a glucan rest and the less suitable for a fly sparge.
    *
    *        See measurement/Units for more on viscosity measurement; cP = centipoise.
    */
   Q_PROPERTY(std::optional<double> viscosity_cP               READ viscosity_cP                WRITE setViscosity_cP     )

   /**
    * \brief The amount of DMS precursors, namely S-methyl methionine (SMM) and dimethyl sulfoxide (DMSO) in the malt
    *        which convert to dimethyl sulfide (DMS) when the wort is heated.  The DMS-P (also sometimes written DMSP or
    *        DMSp) should be 5-15 ppm for lager malts, less for ales.  The more fully modified the malt, the lower the
    *        DMS-P levels should be.
    *
    *        Measurement of DMS-P is often performed by heating "congress wort" samples (see above) in closed vials,
    *        sampling the headspace after incubation, and quantifying DMS using gas chromatography.
    *
    *        BeerJSON allows this attribute to be specified as either mass fraction (ppm or ppb) or mass concentration
    *        (mg / L).  Per the comments in measurement/PhysicalQuantity.h, we treat the latter as easily convertible to
    *        the former in the context of brewing.
    */
   Q_PROPERTY(std::optional<double> dmsP_ppm                  READ dmsP_ppm                  WRITE setDmsP_ppm               )

   /**
    * \brief Free Amino Nitrogen (FAN) is a measure of the concentration of nitrogen-containing compounds -- including
    *        amino acids, ammonia and small peptides (one to three units) -- which can be metabolized by yeast for cell
    *        growth and proliferation.  As with some of the other measures here, this is a measure of the wort, so, in
    *        the context of a fermentable, it is measure of a congress wort produced from this malt.
    *
    *        As a diagnostic test, low FAN measurements indicate slow or incomplete fermentation, while high FAN
    *        measurements may indicate haze issues and/or diacetyl formation.
    *
    *        BeerJSON allows this attribute to be specified as either mass fraction (ppm or ppb) or mass concentration
    *        (mg / L).  Per the comments in measurement/PhysicalQuantity.h, we treat the latter as easily convertible to
    *        the former in the context of brewing.
    */
   Q_PROPERTY(std::optional<double> fan_ppm                  READ fan_ppm                  WRITE setFan_ppm               )

   /**
    * \brief Fermentability is used in Extracts to indicate a baseline typical apparent attenuation for a typical medium
    *        attenuation yeast.
    */
   Q_PROPERTY(std::optional<double> fermentability_pct   READ fermentability_pct   WRITE setFermentability_pct)

   /**
    * \brief Beta-glucans (β-glucans) are polysaccharides of D-glucose monomers linked by beta-glycosidic bonds.
    *        β-glucans are present in the cell walls of various cereals and are capable of clogging process filters; so,
    *        too high a concentration of β-glucans in the wort may cause result in haze in the end product beer.
    *
    *        As with some of the other measures here, the concentration of β-glucans is measured in the wort, so, in the
    *        context of a fermentable, it is measure of a congress wort produced from this malt.
    *
    *        BeerJSON allows this attribute to be specified as either mass fraction (ppm or ppb) or mass concentration
    *        (mg / L).  Per the comments in measurement/PhysicalQuantity.h, we treat the latter as easily convertible to
    *        the former in the context of brewing.
    *
    *        Yes, it would be neat to include β in the property, variable and function names etc, but the Qt MOC doesn't
    *        appear to like it.
    */
   Q_PROPERTY(std::optional<double> betaGlucan_ppm                  READ betaGlucan_ppm                  WRITE setBetaGlucan_ppm               )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Type    type                                       () const;
   double  color_srm                                  () const;
   QString origin                                     () const;
   QString supplier                                   () const;
   QString notes                                      () const;
   std::optional<double>     coarseFineDiff_pct       () const;
   std::optional<double>     moisture_pct             () const;
   std::optional<double>     diastaticPower_lintner   () const;
   std::optional<double>     protein_pct              () const;
   std::optional<double>     maxInBatch_pct           () const;
   std::optional<bool>       recommendMash            () const;
   std::optional<double>     ibuGalPerLb              () const;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<GrainGroup> grainGroup               () const;
   std::optional<int>        grainGroupAsInt          () const;
   QString                   producer                 () const;
   QString                   productId                () const;
   std::optional<double>     fineGrindYield_pct       () const;
   std::optional<double>     coarseGrindYield_pct     () const;
   std::optional<double>     potentialYield_sg        () const;
   std::optional<double>     alphaAmylase_dextUnits   () const;
   std::optional<double>     kolbachIndex_pct         () const;
   std::optional<double>     hardnessPrpGlassy_pct    () const;
   std::optional<double>     hardnessPrpHalf_pct      () const;
   std::optional<double>     hardnessPrpMealy_pct     () const;
   std::optional<double>     kernelSizePrpPlump_pct   () const;
   std::optional<double>     kernelSizePrpThin_pct    () const;
   std::optional<double>     friability_pct           () const;
   std::optional<double>     di_ph                    () const;
   std::optional<double>     viscosity_cP             () const;
   std::optional<double>     dmsP_ppm                 () const;
   std::optional<double>     fan_ppm                  () const;
   std::optional<double>     fermentability_pct       () const;
   std::optional<double>     betaGlucan_ppm           () const;

   // Calculated getters.
   bool    isExtract             () const;
   bool    isSugar               () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setType                     (Type                      const   val);
   void setColor_srm                (double                    const   val);
   void setOrigin                   (QString                   const & val);
   void setSupplier                 (QString                   const & val);
   void setNotes                    (QString                   const & val);
   void setCoarseFineDiff_pct       (std::optional<double>     const   val);
   void setMoisture_pct             (std::optional<double>     const   val);
   void setDiastaticPower_lintner   (std::optional<double>     const   val);
   void setProtein_pct              (std::optional<double>     const   val);
   void setMaxInBatch_pct           (std::optional<double>     const   val);
   void setRecommendMash            (std::optional<bool>       const   val);
   void setIbuGalPerLb              (std::optional<double>     const   val);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   void setGrainGroup               (std::optional<GrainGroup> const   val);
   void setGrainGroupAsInt          (std::optional<int>        const   val);
   void setProducer                 (QString                   const & val);
   void setProductId                (QString                   const & val);
   void setFineGrindYield_pct       (std::optional<double>     const   val);
   void setCoarseGrindYield_pct     (std::optional<double>     const   val);
   void setPotentialYield_sg        (std::optional<double>     const   val);
   void setAlphaAmylase_dextUnits   (std::optional<double>     const   val);
   void setKolbachIndex_pct         (std::optional<double>     const   val);
   void setHardnessPrpGlassy_pct    (std::optional<double>     const   val);
   void setHardnessPrpHalf_pct      (std::optional<double>     const   val);
   void setHardnessPrpMealy_pct     (std::optional<double>     const   val);
   void setKernelSizePrpPlump_pct   (std::optional<double>     const   val);
   void setKernelSizePrpThin_pct    (std::optional<double>     const   val);
   void setFriability_pct           (std::optional<double>     const   val);
   void setDi_ph                    (std::optional<double>     const   val);
   void setViscosity_cP             (std::optional<double>     const   val);
   void setDmsP_ppm                 (std::optional<double>     const   val);
   void setFan_ppm                  (std::optional<double>     const   val);
   void setFermentability_pct       (std::optional<double>     const   val);
   void setBetaGlucan_ppm           (std::optional<double>     const   val);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   Type                      m_type                     ;
   double                    m_color_srm                ;
   QString                   m_origin                   ;
   QString                   m_supplier                 ;
   QString                   m_notes                    ;
   std::optional<double>     m_coarseFineDiff_pct       ;
   std::optional<double>     m_moisture_pct             ;
   std::optional<double>     m_diastaticPower_lintner   ;
   std::optional<double>     m_protein_pct              ;
   std::optional<double>     m_maxInBatch_pct           ;
   std::optional<bool>       m_recommendMash            ;
   std::optional<double>     m_ibuGalPerLb              ;
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   std::optional<GrainGroup> m_grainGroup               ;
   QString                   m_producer                 ;
   QString                   m_productId                ;
   std::optional<double>     m_fineGrindYield_pct       ;
   std::optional<double>     m_coarseGrindYield_pct     ;
   std::optional<double>     m_potentialYield_sg        ;
   std::optional<double>     m_alphaAmylase_dextUnits   ;
   std::optional<double>     m_kolbachIndex_pct         ;
   std::optional<double>     m_hardnessPrpGlassy_pct    ;
   std::optional<double>     m_hardnessPrpHalf_pct      ;
   std::optional<double>     m_hardnessPrpMealy_pct     ;
   std::optional<double>     m_kernelSizePrpPlump_pct   ;
   std::optional<double>     m_kernelSizePrpThin_pct    ;
   std::optional<double>     m_friability_pct           ;
   std::optional<double>     m_di_ph                    ;
   std::optional<double>     m_viscosity_cP             ;
   std::optional<double>     m_dmsP_ppm                 ;
   std::optional<double>     m_fan_ppm                  ;
   std::optional<double>     m_fermentability_pct       ;
   std::optional<double>     m_betaGlucan_ppm           ;
};

BT_DECLARE_METATYPES(Fermentable)

#endif
