/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tools/OgCorrectionTool.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Eric Tamme <etamme@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
 =====================================================================================================================*/
#include "tools/OgCorrectionTool.h"

#include <QMessageBox>

#include "Algorithms.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_OgCorrectionTool.cpp"
#endif

namespace {
   auto constexpr style_redBorder =
      "border: 1px solid red; "
      "padding-top: 2px; "
      "padding-bottom: 2px; "
      "border-radius: 2px";
   auto constexpr style_green  = "color : #2E7D32;";
   auto constexpr style_red    = "color : #C62828;";
   auto constexpr style_blue   = "color : #1976D2;";
   auto constexpr style_orange = "color : #ff8c00;";
}

OgCorrectionTool::OgCorrectionTool(QWidget * parent) :
   QDialog{parent} {
   setupUi(this);

   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_targetOg        , value_targetOg        , double, Measurement::PhysicalQuantity::Density    , 3);
   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_measuredSg      , lineEdit_measuredSg   , double, Measurement::PhysicalQuantity::Density    , 3);
   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_measuredTemp    , lineEdit_measuredTemp , double, Measurement::PhysicalQuantity::Temperature, 1);
   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_calTemp         , lineEdit_calTemp      , double, Measurement::PhysicalQuantity::Temperature, 1);
   SMART_FIELD_INIT_FIXED(OgCorrectionTool, label_actualSg        , value_actualSg        , double, Measurement::Units::specificGravity       , 3);
   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_preBoilVolume   , lineEdit_preBoilVolume, double, Measurement::PhysicalQuantity::Volume        );
   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_noActionOg      , value_noActionOg      , double, Measurement::PhysicalQuantity::Density    , 3);
   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_changeBoilVolume, value_changeBoilVolume, double, Measurement::PhysicalQuantity::Volume        );
   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_finalBatchSize  , value_finalBatchSize  , double, Measurement::PhysicalQuantity::Volume        );
   SMART_FIELD_INIT_FS   (OgCorrectionTool, label_addSugarOrDme   , value_addSugarOrDme   , double, Measurement::PhysicalQuantity::Mass       , 1);

   connect(this->pushButton_calculate, &QAbstractButton::clicked, this, &OgCorrectionTool::calculate);
   connect(&MainWindow::instance(), &MainWindow::newRecipeSelected, this, &OgCorrectionTool::newlySelectedRecipe);

   this->hideOutputs();
   return;
}

OgCorrectionTool::~OgCorrectionTool() = default;

void OgCorrectionTool::setRecipe(Recipe * rec) {
   if (rec && rec != this->m_recObs) {
      this->m_recObs = rec;
      this->value_targetOg->setQuantity(this->m_recObs->og());
      this->hideOutputs();
   }
   return;
}

void OgCorrectionTool::newlySelectedRecipe() {
   this->setRecipe(MainWindow::instance().currentRecipe());
   return;
}

// TODO: There are a LOT of assumptions and simplifications here. Need to change that.
void OgCorrectionTool::calculate() {

   if (!this->m_recObs) {
      QMessageBox::critical(this,
                            tr("No Recipe Selected"),
                            tr("You need to have a recipe open in the main window before you can use this tool."));
      return;
   }

   auto const equipment = this->m_recObs->equipment();
   if (!equipment) {
      QMessageBox::critical(this,
                            tr("No Equipment on Recipe"),
                            tr("Recipe \"%1\" has no equipment.").arg(this->m_recObs->name()));
      return;
   }

   auto const boil = this->m_recObs->boil();
   if (!boil) {
      QMessageBox::critical(this,
                            tr("No Boil on Recipe"),
                            tr("Recipe \"%1\" has no boil.").arg(this->m_recObs->name()));
      return;
   }

   //
   // Start with all the output boxes hidden.  We'll show different ones depending on whether the wort is
   // above-/below-/on-target.
   //
   this->hideOutputs();

   //
   // If the user entered Plato or Brix, we use that as-is.  If they entered Specific Gravity, then, provided they gave
   // the relevant additional info, we correct that for temperature.
   //
   // In all cases, we want to end up with a starting point in specific gravity
   //
   bool parsedOk = false;
   double startGravityAt20C_sg = std::numeric_limits<double>::quiet_NaN();
   if (auto const & inputUnitSystem = this->lineEdit_measuredSg->getDisplayUnitSystem();
       inputUnitSystem == Measurement::UnitSystems::density_Plato ||
       inputUnitSystem == Measurement::UnitSystems::density_Brix) {
      double const rawInput = lineEdit_measuredSg->getNonOptValue<double>(&parsedOk);
      if (parsedOk) {
         if (inputUnitSystem == Measurement::UnitSystems::density_Plato) {
            startGravityAt20C_sg = Measurement::Units::plato.toCanonical(rawInput).quantity;
         } else {
            startGravityAt20C_sg = Measurement::Units::brix.toCanonical(rawInput).quantity;
         }
      }
   } else {
      Q_ASSERT(inputUnitSystem == Measurement::UnitSystems::density_SpecificGravity);
      double const measuredSg = lineEdit_measuredSg->getNonOptCanonicalQty(&parsedOk);
      if (!lineEdit_measuredSg->isBlank() && parsedOk) {
         double const measuredTemp_c = lineEdit_measuredTemp->getNonOptCanonicalQty(&parsedOk);
         if (!lineEdit_measuredTemp->isBlank() && parsedOk) {
            double const hydrometerCalibrationTemp_c = lineEdit_calTemp->getNonOptCanonicalQty(&parsedOk);
            if (!lineEdit_calTemp->isBlank() && parsedOk) {
               startGravityAt20C_sg = Algorithms::correctSgForTemperature(measuredSg,
                                                                          measuredTemp_c,
                                                                          hydrometerCalibrationTemp_c);
            }
         }

         if (std::isnan(startGravityAt20C_sg)) {
            //
            // The user has given us a starting gravity, but not enough additional info to correct it, so we use it
            // as-is.
            //
            startGravityAt20C_sg = measuredSg;
         }
      }
   }

   if (std::isnan(startGravityAt20C_sg)) {
      this->lineEdit_measuredSg->setStyleSheet(style_redBorder);
      // TODO Probably would be better to have this error message in the form rather than a pop-up
      QMessageBox::critical(this,
                            tr("Couldn't read input"),
                            tr("Need a valid value for Measured Gravity"));
      return;
   }
   this->lineEdit_measuredSg->setStyleSheet("");

   this->value_actualSg->setQuantity(startGravityAt20C_sg);

   double const wort_l      = lineEdit_preBoilVolume->getNonOptCanonicalQty(&parsedOk);

   if (lineEdit_preBoilVolume->isBlank() || !parsedOk) {
      this->lineEdit_preBoilVolume->setStyleSheet(style_redBorder);
      // TODO Probably would be better to have this error message in the form rather than a pop-up
      QMessageBox::critical(this,
                            tr("Couldn't read input"),
                            tr("Need a valid value for Wort Volume"));
      return;
   }
   this->lineEdit_preBoilVolume->setStyleSheet("");

   double const evapRate_lHr = equipment->kettleEvaporationPerHour_l().value_or(Equipment::default_kettleEvaporationPerHour_l);

   // Calculate intermediate parameters.
   double const plato = Measurement::Units::plato.fromCanonical(startGravityAt20C_sg);
   double const sugar_kg = startGravityAt20C_sg * Algorithms::getWaterDensity_kgL(20.0) * wort_l * plato/100.0;
   double const water_kg = startGravityAt20C_sg * Algorithms::getWaterDensity_kgL(20.0) * wort_l * (1.0 - plato/100.0);

   // Calculate OG w/o correction.
   double const boilTime_mins = boil->boilTime_mins();
   double finalVolume_l = equipment->wortEndOfBoil_l(wort_l, boilTime_mins);
   double const finalWater_kg =
      water_kg - boilTime_mins/60.0 * evapRate_lHr * Algorithms::getWaterDensity_kgL(20.0);
   double const finalPlato = 100.0 * sugar_kg / (sugar_kg + finalWater_kg);

   double const finalUncorrectedSg_20C = Algorithms::PlatoToSG_20C20C(finalPlato);
   this->value_noActionOg       ->setQuantity(finalUncorrectedSg_20C);

   double const targetOg = this->value_targetOg->getNonOptCanonicalQty();

   //
   // Although qFuzzyCompare helps a bit with comparisons, we also need to do our own rounding.  Eg if target = 1.0561
   // and predicted = 1.0562, we're going to display both as 1.056, so we don't want to treat them as different for the
   // indicators etc.
   //
   double const roundedTargetOg = Algorithms::round(targetOg, 3);
   double const roundedPredictedOg = Algorithms::round(finalUncorrectedSg_20C, 3);

   if (qFuzzyCompare(roundedPredictedOg, roundedTargetOg)) {
      this->label_noActionOgComment->setText(tr("- On Target -"));
      this->label_noActionOgComment->setStyleSheet(style_green);
      this->value_noActionOg       ->setStyleSheet(style_green);
      return;
   }

   if (roundedPredictedOg > roundedTargetOg) {
      this->label_noActionOgComment->setText(tr("↑ Above Target ↑"));
      this->label_noActionOgComment->setStyleSheet(style_red); // #C62828 = red
      this->value_noActionOg       ->setStyleSheet(style_red);
   } else {
      this->label_noActionOgComment->setText(tr("↓ Below Target ↓"));
      this->label_noActionOgComment->setStyleSheet(style_blue); // #1976D2 = blue
      this->value_noActionOg       ->setStyleSheet(style_blue);
   }

   //
   // A multipurpose solution is to change the boil size -- either add water if gravity is too high or boil the wort
   // down if gravity is too low.  We use the same widgets to display either case, but with a few tweaks.
   //
   this->groupBox_changeBoilVolume->setStyleSheet(style_orange);
   this->groupBox_changeBoilVolume->setVisible(true);
   this->label_changeBoilVolume->setStyleSheet(style_green);
   this->value_changeBoilVolume->setStyleSheet(style_green);
   this->label_finalBatchSize->setStyleSheet(style_blue);
   this->value_finalBatchSize->setStyleSheet(style_blue);
   if (roundedPredictedOg > roundedTargetOg) {
      //
      // If the predicted OG is higher than target, then we propose the user add water to the boil
      //
      this->groupBox_changeBoilVolume->setTitle(tr("Remedy: Add Water To Boil"));
      this->label_changeBoilVolume->setText(tr("Amount to Add"));
      this->value_changeBoilVolume->setToolTip(tr("Amount of water you need to add to hit planned OG"));
   } else {
      this->groupBox_changeBoilVolume->setTitle(tr("Remedy: Boil Down To Reduce Wort Volume"));
      this->label_changeBoilVolume->setText(tr("Amount to Boil Off"));
      this->value_changeBoilVolume->setToolTip(tr("Extra wort you need to boil off to hit planned OG"));
   }

   // Calculate volume to add to boil
   double const targetFinalPlato = Algorithms::SG_20C20C_toPlato(targetOg); // This is bad. This assumes the post-boil gravity = og. Need account for post-boil water additions.
   // postBoilWater_kg = batchSize - topUpWater;
   // postBoilSugar_kg = Algorithms::SG_20C20C_toPlato( recObs->getOG() ) / 100.0 * batchSize * recObs->getOG() * Algorithms::getWaterDensity_kgL(20);
   // targetFinalPlato = 100 * postBoilSugar_kg / ( postBoilSugar_kg + postBoilWater_kg );
   double const waterToAdd_kg = 100.0 * sugar_kg / targetFinalPlato - sugar_kg - finalWater_kg;
   double const waterToAdd_l = waterToAdd_kg / Algorithms::getWaterDensity_kgL(20.0);

   // Calculate final batch size.
   finalVolume_l += waterToAdd_l;

   // Display output for changing the boil size
   if (roundedPredictedOg > roundedTargetOg) {
      this->value_changeBoilVolume->setQuantity(waterToAdd_l);
   } else {
      this->value_changeBoilVolume->setQuantity(-waterToAdd_l);
   }
   this->value_finalBatchSize->setQuantity(finalVolume_l);

   //
   // If the predicted OG is too low, another remedy is to add DME or sugar
   //
   if (roundedPredictedOg < roundedTargetOg) {
      this->label_remediesSeparator->setVisible(true);
      this->groupBox_addSugarOrDme->setStyleSheet(style_orange);
      this->label_addSugarOrDme->setStyleSheet(style_green);
      this->value_addSugarOrDme->setStyleSheet(style_green);
      this->groupBox_addSugarOrDme->setVisible(true);

      //
      // If we measure specific gravity, the amount of sugar to add would be:
      //
      //    Sugar (grams)     = ΔGP × Volume (Liters) × 100 / 46
      //    Sugar (kilograms) = ΔGP × Volume (Liters) × 0.1 / 46
      //
      // Where:
      //
      //    ΔGP = Difference in Gravity Points = (Target SG−Current SG)×1000
      //          Example: Target 1.067 - Current 1.062 = 0.005 → 5 Gravity Points.
      //
      //    Volume = Current volume of the wort in Liters.
      //
      //    46 = The approximate gravity points contributed by 1 kg of sucrose (table sugar) per liter of water.
      //
      // The calculations are potentially slightly easier in °Plato, because 1°P represents 1 g of sugars in 100 g of
      // wort, but in either case it's not a huge complexity.
      //
      double const gravityPointsDifference = (targetOg - finalUncorrectedSg_20C) * 1000;
      double const sugarToAdd = gravityPointsDifference * wort_l * 0.1 / 46.0;
      this->value_addSugarOrDme->setQuantity(sugarToAdd);
   }

   return;
}

void OgCorrectionTool::hideOutputs() const {
   this->groupBox_changeBoilVolume->setVisible(false);
   this->label_remediesSeparator  ->setVisible(false);
   this->groupBox_addSugarOrDme   ->setVisible(false);

   this->value_noActionOg->setRawText("");
   this->label_noActionOgComment->setText("");

   return;
}