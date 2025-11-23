/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * MashWizard.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Adam Hawes <ach@hawes.net.au>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • David Grundberg <individ@acc.umu.se>
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
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
#include "MashWizard.h"

#include <QButtonGroup>
#include <QMessageBox>

#include "Algorithms.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Measurement.h"
#include "measurement/PhysicalConstants.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "utils/FuzzyCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MashWizard.cpp"
#endif

namespace {
   //
   // For the moment, I don't propose to add Kelvin to our supported units, as it's only something we use internally,
   // and the conversion (per https://en.wikipedia.org/wiki/Kelvin) is pretty simple.
   //
   double CelsiusToKelvin(double const celsius) { return celsius + 273.15; }
   double KelvinToCelsius(double const kelvin ) { return kelvin  - 273.15; }
}

MashWizard::MashWizard(QWidget* parent) :
   QDialog(parent) {
   this->setupUi(this);
   this->m_bGroup = new QButtonGroup();
   this->m_bGroup->addButton(radioButton_noSparge);
   this->m_bGroup->addButton(radioButton_batchSparge);
   this->m_bGroup->addButton(radioButton_flySparge);

//   radioButton_batchSparge->setChecked(true);

   connect(this->m_bGroup , QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &MashWizard::toggleSpinBox);
   connect(this->buttonBox, &QDialogButtonBox::accepted,                                    this, &MashWizard::wizardry);
   connect(this->buttonBox, &QDialogButtonBox::rejected,                                    this, &QWidget::close);
   return;
}

void MashWizard::toggleSpinBox(QAbstractButton* button) {
   if (button == radioButton_noSparge) {
      this->widget_batches->setEnabled(false);
      this->widget_mashThickness->setEnabled(false);
   } else if (button == radioButton_flySparge) {
      this->widget_batches->setEnabled(false);
      this->widget_mashThickness->setEnabled(true);
   } else {
      this->widget_batches->setEnabled(true);
      this->widget_mashThickness->setEnabled(true);
   }
   return;
}

void MashWizard::setRecipe(Recipe* rec) {
   m_recObs = rec;
   return;
}

void MashWizard::show() {
   // Don't show if there is no Recipe or no Mash!
   if (!this->m_recObs || !this->m_recObs->mash()) {
      return;
   }

   // Ensure at least one mash step.
   if (this->m_recObs->mash()->mashSteps().size() == 0) {
      QMessageBox::information(this, tr("No steps"), tr("There must be at least one mash step to run the wizard."));
      return;
   }

   Measurement::getThicknessUnits(&this->m_volumeUnit, &this->m_weightUnit);
   this->label_mashThickness->setText(tr("Mash thickness (%1/%2)").arg(m_volumeUnit->name, m_weightUnit->name));

   auto firstStep = m_recObs->mash()->mashSteps().first();
   auto lastStep  = m_recObs->mash()->mashSteps().last();

   // Recalculate the mash thickness
   double const thickNum = firstStep->amount_l()/m_recObs->grainsInMash_kg();
   double const thickness = thickNum * m_weightUnit->toCanonical(1).quantity / m_volumeUnit->toCanonical(1).quantity ;
   this->doubleSpinBox_thickness->setValue(thickness);

   // Is this a batch, fly or no sparge?
   if (firstStep == lastStep) {
      this->radioButton_noSparge->setChecked(true);
      this->widget_batches->setEnabled(false);
      this->widget_mashThickness->setEnabled(false);
   } else if (lastStep->type() == MashStep::Type::FlySparge) {
      this->radioButton_flySparge->setChecked(true);
      this->widget_batches->setEnabled(false);
      this->widget_mashThickness->setEnabled(true);
   } else {
      int const numSpargeSteps = m_recObs->mash()->numMashStepsMatching(
         [](MashStep const & ms) { return ms.isSparge(); }
      );

      this->radioButton_batchSparge->setChecked(true);
      this->widget_batches->setEnabled(true);
      this->widget_mashThickness->setEnabled(true);
      this->spinBox_batches->setValue(numSpargeSteps);
   }

   this->setVisible(true);
   return;
}

void MashWizard::wizardry() {
   if (!this->m_recObs || !this->m_recObs->mash()) {
      return;
   }

   std::shared_ptr<Mash> mash = m_recObs->mash();

   // If we have an equipment, utilize the custom absorption and boiling temp.
   bool const hasEquipment = m_recObs->equipment() ? true : false;
   double const absorption_LKg =
      hasEquipment ?
         m_recObs->equipment()->mashTunGrainAbsorption_LKg().value_or(Equipment::default_mashTunGrainAbsorption_LKg) :
         PhysicalConstants::grainAbsorption_Lkg;
   double const boilingPoint_c = hasEquipment ? m_recObs->equipment()->boilingPoint_c() : 100.0;
   double const lauterDeadspace_l = hasEquipment ? m_recObs->equipment()->getLauteringDeadspaceLoss_l() : 0.0;

   double const grainsInMash_kg = this->m_recObs->grainsInMash_kg();
   if (Utils::FuzzyCompare(grainsInMash_kg, 0.0)) {
      // Without any grains, we'll be dividing by zero below, which will, of course, lead to crazy results
      QMessageBox::information(this,
                               tr("No grains in the mash"),
                               tr("Check the 'Fermentables' tab.  You need some mash-stage grain additions before you "
                                  "can run the wizard."));
      return;
   }

   double waterFromRemovedSteps_l = 0.0;
   std::shared_ptr<MashStep> firstMashStep{nullptr};
   bool doneFirstStep = false;
   for (std::shared_ptr<MashStep> step : mash->mashSteps()) {
      if (!doneFirstStep) {
         firstMashStep = step;
         doneFirstStep = true;

         // We ensured that there was at least one mash step when we displayed the thickness dialog in show().
         if (!firstMashStep) {
            qCritical() << Q_FUNC_INFO << "First mash step was null.";
            return;
         }

         // Ensure first mash step is an infusion.
         if (!firstMashStep->isInfusion() /*&& !firstMashStep->isSparge()*/) {
            QMessageBox::information(this, tr("First step"), tr("Your first mash step must be an infusion."));
            return;
         }
      } else if (step->isSparge()) {
         //
         // Remove any batch sparges, but keep track of how much water was in them
         //
         waterFromRemovedSteps_l += step->amount_l();
         mash->remove(step);
      }
   }

   // If we didn't process the first step, it means there weren't any steps!
   if (!doneFirstStep) {
      QMessageBox::information(this,
                               tr("No mash steps"),
                               tr("You need to have mash steps before you can run the wizard."));
      return;
   }

   bool const noSparge = (this->m_bGroup->checkedButton() == radioButton_noSparge);

   if (noSparge) {
      //
      // Since there aren't any sparge steps, all the water must be in infusion(s).  If we "lost" some water above when
      // we removed sparge steps, we add it back to the first step.
      //
      if (waterFromRemovedSteps_l > 0.0) {
         qDebug() <<
            Q_FUNC_INFO << "Adding to first step (" << *firstMashStep << ") the" << waterFromRemovedSteps_l <<
            "litres of water from removed sparge step(s)";
         firstMashStep->setAmount_l(firstMashStep->amount_l() + waterFromRemovedSteps_l);
      }
   }

   QList<std::shared_ptr<MashStep>> const steps = mash->mashSteps();

   // For no sparge, get the thickness of the first mash step
   double const thickNum      =
      noSparge ? firstMashStep->amount_l()/grainsInMash_kg : this->doubleSpinBox_thickness->value();
   double const thickness_LKg =
      noSparge ? thickNum :
                 thickNum * this->m_volumeUnit->toCanonical(1.0).quantity /
                            this->m_weightUnit->toCanonical(1.0).quantity;
   qDebug() <<
      Q_FUNC_INFO << "grainsInMash_kg:" << grainsInMash_kg << ", thickNum:" << thickNum << ", thickness_LKg:" <<
      thickness_LKg << ", lauterDeadspace_l:" << lauterDeadspace_l;

   if (thickness_LKg <= 0.0) {
      QMessageBox::information(this,
                               tr("Bad thickness"),
                               tr("You must have a positive mash thickness."));
      return;
   }

   double grainHeatCapacity_kcalC = PhysicalConstants::grainSpecificHeat_calGC * grainsInMash_kg;

   // Do first step
   double waterMass_kg = thickness_LKg * grainsInMash_kg;
   {
      //
      // We use a lot of the same variable names in different places for each step.  Rather than try to come up with
      // unique names for them, or re-use them in confusing ways, we limit the scope and make them const where possible.
      //
      double const stepTemp_c = firstMashStep->startTemp_c();
      double const grainTemp_c = mash->grainTemp_c();
      if (stepTemp_c < grainTemp_c) {
         qWarning() <<
            Q_FUNC_INFO << "Mash Step temperature (" << stepTemp_c << "°C) is below grain temperature (" <<
            grainTemp_c << "°C)";
      }

      //
      // Heat capacity (aka thermal mass) of mash and water = mass × specific heat capacity.  Since we're measuring mass
      // in kg, this means the heat capacity is measured in kcal per °C.
      //
      double const waterHeatCapacity_kcalC = PhysicalConstants::waterSpecificHeat_calGC * waterMass_kg;
      qDebug() <<
         Q_FUNC_INFO << "waterMass_kg:" << waterMass_kg << ", stepTemp_c:" << stepTemp_c << ", grainTemp_c:" <<
         grainTemp_c << ", waterHeatCapacity_kcalC:" << waterHeatCapacity_kcalC;

      //
      // The following is adapted from https://jansson.us/MashPhysics.html
      //
      // If we write "E_foo" for "(heat) energy of foo", then by conservation of energy, we have the following equality
      // when we combine the strike (ie initial) water with the grain in the mash tun:
      //
      //    E_water + E_grain + E_tun = E_waterAndGrainAndTun = E_mash
      //
      // Remember that the heat energy of something is just its heat capacity times its absolute temperature.  So,
      // writing "HC_foo" for "heat capacity of foo" and "T_foo" for "Temperature of foo", we can expand the above
      // equality to:
      //
      //    HC_water × T_water + HC_grain × T_grain + HC_tun × T_tun  =  Temp_mash × (HC_water + HC_grain + HC_tun)
      //
      // We want to find "T_water", so we rearrange to:
      //
      //    T_water = (Temp_mash × (HC_water + HC_grain + HC_tun) - HC_grain × T_grain - HC_tun × T_tun) / HC_water
      //
      //            = (E_mash - E_grain - E_tun) / HC_water
      //
      // Of course we have to convert the temperatures from Celsius to Kelvin, but that's a small price to pay.
      //
      // NOTE that we are specifically ignoring BeerXML's request to only adjust strike temperatures to account for the
      // tun if mash->equipAdjust() is set.  (This setting was dropped in BeerJSON.)  If the mash tun weight is zero,
      // this will effectively give it zero contribution to the calculation.
      //
      double const tunHeatCapacity_kcalC =
         (mash->mashTunSpecificHeat_calGC().value_or(0.0)*mash->mashTunWeight_kg().value_or(0.0));
      double const totalHeatCapacity_kcalC = waterHeatCapacity_kcalC +
                                             grainHeatCapacity_kcalC +
                                               tunHeatCapacity_kcalC;
      // If we can't get a temperature for the tun, assume it's the same as the grain
      double const tunTemp_c = mash->tunTemp_c().value_or(grainTemp_c);
      if (stepTemp_c < tunTemp_c) {
         qWarning() <<
            Q_FUNC_INFO << "Mash Step temperature (" << stepTemp_c << "°C) is below tun temperature (" <<
            tunTemp_c << "°C)";
      }

      double const  mashEnergy_kcal = CelsiusToKelvin(stepTemp_c) * totalHeatCapacity_kcalC;
      double const grainEnergy_kcal = CelsiusToKelvin(grainTemp_c) * grainHeatCapacity_kcalC;
      double const   tunEnergy_kcal = CelsiusToKelvin(  tunTemp_c) *   tunHeatCapacity_kcalC;

      double const infuseWaterTemp_c = KelvinToCelsius(
         (mashEnergy_kcal - grainEnergy_kcal - tunEnergy_kcal) / waterHeatCapacity_kcalC
      );

      qDebug() <<
         Q_FUNC_INFO << "tunHeatCapacity_kcalC:" << tunHeatCapacity_kcalC <<
                      ", totalHeatCapacity_kcalC:" << totalHeatCapacity_kcalC <<
                      ", tunTemp_c:" << tunTemp_c <<
                      ", mashEnergy_kcal:" << mashEnergy_kcal <<
                      ", grainEnergy_kcal:" << grainEnergy_kcal <<
                      ", tunEnergy_kcal:" << tunEnergy_kcal <<
                      ", infuseWaterTemp_c:" << infuseWaterTemp_c;

      // Can't have water above boiling.
      if (infuseWaterTemp_c > boilingPoint_c) {
         QMessageBox::information(this,
                                  tr("Mash too thick"),
                                  tr("Your mash is too thick for desired temperature at first step."));
         return;
      }

      qDebug() <<
         Q_FUNC_INFO << "Changing first mash step volume from" << firstMashStep->amount_l() << "to" << waterMass_kg <<
         "liters.  Changing infuse temp from " << firstMashStep->infuseTemp_c() << "to" << infuseWaterTemp_c << "°C";

      firstMashStep->setAmount_l(waterMass_kg);
      firstMashStep->setInfuseTemp_c(infuseWaterTemp_c);
      //================End of first step=====================
      qDebug() << Q_FUNC_INFO << "First step:" << *firstMashStep;

   }

   // Do rest of steps.

   //
   // Add thermal mass of equipment to grainHeatCapacity_kcalC.
   // I am specifically ignoring BeerXML's request to only do this if mash->getEquipAdjust() is set.
   //
   grainHeatCapacity_kcalC += mash->mashTunSpecificHeat_calGC().value_or(0.0) * mash->mashTunWeight_kg().value_or(0.0);

   //
   // As elsewhere, we take 1 liter of water = 1 kg of water.  Per https://en.wikipedia.org/wiki/Litre, this is a very
   // good approximation at 3.984°C (at standard pressure), which is the maximal density of water.  TBD Strictly
   // speaking, we ought to make an adjustment for the temperature at which the water volume was measured.
   //
   double waterFromPriorSteps_kg = 0.0;
   for (int ii = 1; ii < steps.size(); ++ii) {
      std::shared_ptr<MashStep> priorMashStep = steps[ii - 1];
      // A decoction step doesn't change the total amount of water in the mash
      if (!priorMashStep->isDecoction()) {
         waterFromPriorSteps_kg += priorMashStep->amount_l();
      }

      std::shared_ptr<MashStep> mashStep = steps[ii];

      qDebug() << Q_FUNC_INFO << "step #" << ii - 1 << ":" << *priorMashStep;
      qDebug() << Q_FUNC_INFO << "step #" << ii << ":" << *mashStep;

      if (mashStep->isTemperature()) {
         continue;
      }

      if (mashStep->isDecoction()) {
         //
         // Decoction step
         //
         double const tempFinal_c = mashStep->startTemp_c();
         double const tempInitial_c = steps[ii - 1]->startTemp_c();

         //
         // Per the comment in measurement/Unit.h, "calories per degree Celsius per gram" is identical to "kilocalories
         // (Calories) per degree Celsius per kilogram".
         //
         // Heat capacity is mass × specific heat capacity, so units are "kilocalories (Calories) per degree Celsius".
         //
         double const priorStepsWaterHeatCapacity_CalC =
            waterFromPriorSteps_kg * PhysicalConstants::waterSpecificHeat_calGC;

         double const grainHeatCapacity_CalC = grainsInMash_kg * PhysicalConstants::grainSpecificHeat_calGC;

         double const waterAndGrainHeatCapacity_CalC = priorStepsWaterHeatCapacity_CalC + grainHeatCapacity_CalC;

         // Only include the mash tun in the calculations if asked to
         double const mashTunHeatCapacity_CalC =
            mash->equipAdjust() ?
               (mash->mashTunWeight_kg         ().value_or(0.0) *
                mash->mashTunSpecificHeat_calGC().value_or(0.0)) : 0.0;

         //
         // ratio is the ratio of water and grain to take out for decoction.
         //
         // Note that
         //    A×(boilingPoint_c - tempFinal_c) + A×(tempFinal_c - tempInitial_c) = A×(boilingPoint_c - tempInitial_c)
         //
         double const ratio =
            ((waterAndGrainHeatCapacity_CalC + mashTunHeatCapacity_CalC) * (tempFinal_c - tempInitial_c)) /
               (waterAndGrainHeatCapacity_CalC * (boilingPoint_c - tempInitial_c));
         if (ratio < 0.0 || ratio > 1.0) {
            QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation.") );
            qCritical().nospace() << Q_FUNC_INFO << "Decoction: ratio=" << ratio;
            return;
         }

         waterMass_kg =
            ratio * (waterFromPriorSteps_kg + grainsInMash_kg/PhysicalConstants::grainDensity_kgL);

         qDebug() <<
            Q_FUNC_INFO << "step #" << ii << ", waterMass_kg:" << waterMass_kg << ", ratio:" << ratio <<
            ", waterFromPriorSteps_kg:" << waterFromPriorSteps_kg << ", grainsInMash_kg:" << grainsInMash_kg;

         mashStep->setAmount_l(waterMass_kg);
      } else {
         //
         // Non-decoction step
         //
         double const tempFinal_c = mashStep->startTemp_c();
         double const tempInitial_c = steps[ii - 1]->startTemp_c();
         double const waterTemp_c = boilingPoint_c; // Assume adding boiling water to minimize final volume.
         grainHeatCapacity_kcalC += waterMass_kg * PhysicalConstants::waterSpecificHeat_calGC; // Add thermal mass of last addition.

         waterMass_kg =
            (grainHeatCapacity_kcalC * (tempFinal_c - tempInitial_c)) /
            (PhysicalConstants::waterSpecificHeat_calGC * (waterTemp_c - tempFinal_c));

         mashStep->setAmount_l(waterMass_kg);
         mashStep->setInfuseTemp_c(waterTemp_c);
      }
   }

   //
   // If no sparge, adjust volume of last step to meet target runoff volume
   //
   // TBD: Not sure this is doing the right things...
   //
   if (noSparge && steps.size() > 1) {
      double otherMashStepsTotal_l = 0.0;
      for (int ii = 0; ii < steps.size() - 1; ++ii) {
         //
         // Note that the volume of decoction steps does not change the volume of the mash!
         //
         if (steps[ii]->type() != MashStep::Type::Decoction) {
            otherMashStepsTotal_l += steps[ii]->amount_l();
         }
      }

      double const targetTotalMashVol_l = m_recObs->targetTotalMashVol_l();
      double const targetWortFromMash_l = targetTotalMashVol_l + lauterDeadspace_l;
      qDebug() <<
         Q_FUNC_INFO << "targetWortFromMash_l:" << targetWortFromMash_l << ", targetTotalMashVol_l:" <<
         targetTotalMashVol_l << ", lauterDeadspace_l:" << lauterDeadspace_l << ", otherMashStepsTotal_l:" <<
         otherMashStepsTotal_l;

      waterMass_kg = (targetWortFromMash_l - otherMashStepsTotal_l) * Algorithms::getWaterDensity_kgL(0.0);

      if (waterMass_kg < 0.0) {
         QMessageBox::information(this,
                                  tr("Too much water"),
                                  tr("Cannot adjust volume of last step to meet target runoff volume"));
         return;
      }

      auto lastMashStep = steps.back();
      double const tempFinal_c = lastMashStep->startTemp_c();
      grainHeatCapacity_kcalC += waterMass_kg * PhysicalConstants::waterSpecificHeat_calGC; // Add thermal mass of last addition.

      double const tempInitial_c {
         (steps.size() > 1) ? steps[steps.size() - 2]->startTemp_c() : mash->grainTemp_c()
      };

      double const waterTemp_c =
         (grainHeatCapacity_kcalC * (tempFinal_c - tempInitial_c)) /
         waterMass_kg /
         PhysicalConstants::waterSpecificHeat_calGC +
         tempFinal_c;
      qDebug() << Q_FUNC_INFO << "waterTemp_c:" << waterTemp_c;

      if (waterTemp_c > boilingPoint_c) {
         QMessageBox::information(this,
                                  tr("Infusion temp."),
                                  tr("In order to hit your target temp on the final step, the infusion water would "
                                     "need to be above boiling.  Lower your initial infusion volume."));
         return;
      }

      qDebug() <<
         Q_FUNC_INFO << "Changing last mash step volume from" << lastMashStep->amount_l() << "to" << waterMass_kg <<
         "liters.  Changing infuse temp from " << lastMashStep->infuseTemp_c() << "to" << waterTemp_c << "°C";

      lastMashStep->setAmount_l(waterMass_kg);
      lastMashStep->setInfuseTemp_c(waterTemp_c);
   }

   // Now, do a sparge step, using just enough water that the total
   // volume sums up to the target pre-boil size. We need to account for the potential
   // lauter dead space, I think?
   double spargeWater_l = m_recObs->targetTotalMashVol_l() - m_recObs->mash()->totalMashWater_l();

   // If I've done my math right, we should never get here on nosparge
   // not sure why I am inferring this when I could just check the button group?
   if (spargeWater_l >= 0.001) {
      spargeWater_l += lauterDeadspace_l;
      int const lastMashStepIndex = steps.size() - 1;
      double const tempFinal_c = mash->spargeTemp_c().value_or(0.0); // TODO We should have some better logic if spargeTemp_c is unset or 0.
      if (lastMashStepIndex  < 0) {
         qCritical() << Q_FUNC_INFO << "Should have had at least one mash step before getting to sparging.";
         return;
      }

      double const tempInitial_c = steps[lastMashStepIndex]->startTemp_c() - 10.0; // You will lose about 10C from last step.

      grainHeatCapacity_kcalC =
         m_recObs->grainsInMash_kg() * PhysicalConstants::grainSpecificHeat_calGC +
         absorption_LKg * m_recObs->grainsInMash_kg() * PhysicalConstants::waterSpecificHeat_calGC +
         mash->mashTunWeight_kg().value_or(0.0) * mash->mashTunSpecificHeat_calGC().value_or(0.0);

      waterMass_kg = spargeWater_l;

      double const waterTemp_c =
         (grainHeatCapacity_kcalC / (waterMass_kg * PhysicalConstants::waterSpecificHeat_calGC)) *
         (tempFinal_c - tempInitial_c) +
         tempFinal_c;

      if (waterTemp_c > boilingPoint_c) {
         QMessageBox::information(this,
                                  tr("Sparge temp."),
                                  tr("In order to hit your sparge temp, the sparge water must be above boiling.  Lower "
                                     "your sparge temp, or allow for more sparge water."));
      }

      if ( m_bGroup->checkedButton() == radioButton_batchSparge ) {
         int const numSteps = spinBox_batches->value();
         double const volPerBatch = spargeWater_l/numSteps; // its evil, but deal with it
         for (int ii = 0; ii < numSteps; ++ii) {
            auto newMashStep = std::make_shared<MashStep>(tr("Batch Sparge %1").arg(ii + 1));
            newMashStep->setType(MashStep::Type::BatchSparge);
            newMashStep->setAmount_l(volPerBatch);
            newMashStep->setInfuseTemp_c(waterTemp_c);
            newMashStep->setEndTemp_c(waterTemp_c);
            newMashStep->setStartTemp_c(tempFinal_c);
            newMashStep->setStepTime_mins(15);
            // Setting of step owner and storing of DB ultimately happens in OwnedSet::extend()
            mash->add(newMashStep);
            emit newMashStep->changed(
               newMashStep->metaObject()->property(
                  newMashStep->metaObject()->indexOfProperty(*PropertyNames::MashStep::type)
               )
            );
         }
         emit mash->ownedItemsChanged();
      } else {
         auto newMashStep = std::make_shared<MashStep>(tr("Fly Sparge"));
         newMashStep->setType(MashStep::Type::FlySparge);
         newMashStep->setAmount_l(spargeWater_l);
         newMashStep->setInfuseTemp_c(waterTemp_c);
         newMashStep->setEndTemp_c(waterTemp_c);
         newMashStep->setStartTemp_c(tempFinal_c);
         newMashStep->setStepTime_mins(15);
         // Setting of step owner and storing of DB ultimately happens in OwnedSet::extend()
         mash->add(newMashStep);
         emit newMashStep->changed(
            newMashStep->metaObject()->property(
               newMashStep->metaObject()->indexOfProperty(*PropertyNames::MashStep::type)
            )
         );
      }

   } else if (this->m_bGroup->checkedButton() != radioButton_noSparge) {
      QMessageBox::information(this,
                               tr("Too much wort"),
                               tr("You have too much wort from the mash for your boil size.  Try increasing the "
                                  "boil size by increasing the boil time, or reducing your mash thickness."));
   }
   return;
}
