/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * MashWizard.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "HeatCalculations.h"
#include "measurement/Measurement.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "PhysicalConstants.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_MashWizard.cpp"

MashWizard::MashWizard(QWidget* parent) :
   QDialog(parent),
   m_recObs{nullptr},
   m_weightUnit{nullptr},
   m_volumeUnit{nullptr} {
   setupUi(this);
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
   if ( button == radioButton_noSparge ) {
      widget_batches->setEnabled(false);
      widget_mashThickness->setEnabled(false);
   } else if ( button == radioButton_flySparge ) {
      widget_batches->setEnabled(false);
      widget_mashThickness->setEnabled(true);
   } else {
      widget_batches->setEnabled(true);
      widget_mashThickness->setEnabled(true);
   }
   return;
}

void MashWizard::setRecipe(Recipe* rec) {
   m_recObs = rec;
   return;
}

void MashWizard::show() {
   if ( m_recObs == nullptr || m_recObs->mash() == nullptr ) {
      return;
   }

   // Ensure at least one mash step.
   if ( m_recObs->mash()->mashSteps().size() == 0 ) {
      QMessageBox::information(this, tr("No steps"), tr("There must be at least one mash step to run the wizard."));
      return;
   }

   Measurement::getThicknessUnits(&m_volumeUnit,&m_weightUnit);
   label_mashThickness->setText(tr("Mash thickness (%1/%2)").arg(m_volumeUnit->name, m_weightUnit->name));

   auto firstStep = m_recObs->mash()->mashSteps().first();
   auto lastStep  = m_recObs->mash()->mashSteps().last();

   // Recalculate the mash thickness
   double thickNum = firstStep->amount_l()/m_recObs->grainsInMash_kg();
   double thickness = thickNum * m_weightUnit->toCanonical(1).quantity / m_volumeUnit->toCanonical(1).quantity ;
   doubleSpinBox_thickness->setValue(thickness);

   // Is this a batch, fly or no sparge?
   if ( firstStep == lastStep ) {
      radioButton_noSparge->setChecked(true);
      widget_batches->setEnabled(false);
      widget_mashThickness->setEnabled(false);
   } else if ( lastStep->type() == MashStep::Type::FlySparge ) {
      radioButton_flySparge->setChecked(true);
      widget_batches->setEnabled(false);
      widget_mashThickness->setEnabled(true);
   } else {
      int countSteps = 0;
      auto steps = m_recObs->mash()->mashSteps();
      for(auto step : steps) {
         if (step->isSparge()) {
            countSteps++;
         }
      }
      widget_batches->setEnabled(true);
      widget_mashThickness->setEnabled(true);
      radioButton_batchSparge->setChecked(true);
      spinBox_batches->setValue(countSteps);
   }

   setVisible(true);
   return;
}

double MashWizard::calcDecoctionAmount(MashStep * step,
                                       Mash * mash,
                                       double waterMass,
                                       double grainMass,
                                       double lastTemp,
                                       double boiling) {
   double const grainDensity = PhysicalConstants::grainDensity_kgL;

   double const stepTemp  = step->startTemp_c().value_or(0.0);
   double const equipMass = (mash->equipAdjust()) ? mash->mashTunWeight_kg().value_or(0.0) : 0.0;
   double const c_e       = (mash->equipAdjust()) ? mash->mashTunSpecificHeat_calGC().value_or(0.0) : 0.0;

   double const grHeat = grainMass * HeatCalculations::Cw_calGC;
   double const waHeat = waterMass * HeatCalculations::Cgrain_calGC;
   double const eqHeat = equipMass * c_e;

   double const totalHeat = grHeat + waHeat;
   double const deltaTemp = stepTemp - lastTemp;

   // r is the ratio of water and grain to take out for decoction.
   double const r = ((totalHeat + eqHeat)*deltaTemp) / (totalHeat*(boiling - stepTemp) + totalHeat*deltaTemp);

   if( r < 0 || r > 1 )
   {
      QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation.") );
      qCritical() << Q_FUNC_INFO << "r=" << r;
      return -1;
   }
   return r * (waterMass + grainMass/grainDensity);

}

void MashWizard::wizardry() {
   if (!this->m_recObs || !this->m_recObs->mash()) {
      return;
   }

   auto mash = m_recObs->mash();
   double const grainDensity = PhysicalConstants::grainDensity_kgL;
   double absorption_LKg = PhysicalConstants::grainAbsorption_Lkg;
   double boilingPoint_c = 100.0;
   double lauterDeadspace = 0.0;

   // If we have an equipment, utilize the custom absorption and boiling temp.
   if (m_recObs->equipment()) {
      absorption_LKg = m_recObs->equipment()->mashTunGrainAbsorption_LKg().value_or(Equipment::default_mashTunGrainAbsorption_LKg);
      boilingPoint_c = m_recObs->equipment()->boilingPoint_c();
      lauterDeadspace = m_recObs->equipment()->getLauteringDeadspaceLoss_l();
   }

   auto steps = mash->mashSteps();
   decltype(steps) tmp;

   // We ensured that there was at least one mash step when we displayed the thickness dialog in show().
   auto mashStep = steps.at(0);
   if (!mashStep) {
      qCritical() << Q_FUNC_INFO << "First mash step was null.";
      return;
   }

   // Ensure first mash step is an infusion.
   if (!mashStep->isInfusion() && !mashStep->isSparge()) {
      QMessageBox::information(this, tr("First step"), tr("Your first mash step must be an infusion."));
      return;
   }

   // Find any batch sparges and remove them
   for (auto step : steps) {
      if (step->isSparge()) {
         mash->remove(step);
      } else {
         tmp.append(step);
      }
   }

   double thickness_LKg;
   double thickNum;
   steps = tmp;
   double grainMass = m_recObs->grainsInMash_kg();
   if ( m_bGroup->checkedButton() != radioButton_noSparge ) {
      thickNum = doubleSpinBox_thickness->value();
      thickness_LKg = thickNum * m_volumeUnit->toCanonical(1).quantity / m_weightUnit->toCanonical(1).quantity;
   } else {
      // not sure I like this. Why is this here and not somewhere later?
      if (steps.size() == 1 ) {
         mashStep->setAmount_l(m_recObs->targetTotalMashVol_l());
      }
      // For no sparge, get the thickness of the first mash step
      thickNum = mashStep->amount_l()/grainMass;
      thickness_LKg = thickNum;
   }

   if( thickness_LKg <= 0.0 ) {
      QMessageBox::information(this, tr("Bad thickness"), tr("You must have a positive mash thickness."));
      return;
   }

   // Do first step
   double tempFinal = mashStep->startTemp_c().value_or(0.0);
   double tempInitial = mash->grainTemp_c();
   double massWater = thickness_LKg * grainMass;
   // Thermal mass of mash and water.
   double const MCw = HeatCalculations::Cw_calGC * massWater;
   double MC = HeatCalculations::Cgrain_calGC * grainMass;

   // I am specifically ignoring BeerXML's request to only do this if mash->getEquipAdjust() is set.
   double tempWater = MC/MCw * (tempFinal-tempInitial) + (mash->mashTunSpecificHeat_calGC().value_or(0.0)*mash->mashTunWeight_kg().value_or(0.0))/MCw * (tempFinal-mash->tunTemp_c().value_or(0.0)) + tempFinal;

   // Can't have water above boiling.
   if( tempWater > boilingPoint_c ) {
      QMessageBox::information(this,
                               tr("Mash too thick"),
                               tr("Your mash is too thick for desired temp. at first step."));
      return;
   }

   mashStep->setAmount_l(massWater);
   mashStep->setInfuseTemp_c(tempWater);
   //================End of first step=====================

   // Do rest of steps.
   // Add thermal mass of equipment to MC.
   // I am specifically ignoring BeerXML's request to only do this if mash->getEquipAdjust() is set.
   MC += mash->mashTunSpecificHeat_calGC().value_or(0.0) * mash->mashTunWeight_kg().value_or(0.0);

   for (int i = 1; i < steps.size(); ++i) {
      mashStep = steps[i];

      if (mashStep->isTemperature()) {
         continue;
      } else if (mashStep->isDecoction()) {
         double m_w, m_g, m_e, r;
         double c_w, c_g, c_e;

         tempFinal = mashStep->startTemp_c().value_or(0.0);
         tempInitial = steps[i-1]->startTemp_c().value_or(0.0);

         m_w = 0; // Total mass of water.
         for (int j = 0; j < i; ++j) {
            m_w += steps[j]->amount_l();
         }
         m_g = grainMass;
         m_e = mash->equipAdjust() ? mash->mashTunWeight_kg().value_or(0.0) : 0.0;

         c_w = HeatCalculations::Cw_calGC;
         c_g = HeatCalculations::Cgrain_calGC;
         c_e = mash->equipAdjust() ? mash->mashTunSpecificHeat_calGC().value_or(0.0) : 0.0;

         // r is the ratio of water and grain to take out for decoction.
         r = ((m_w*c_w + m_g*c_g + m_e*c_e)*(tempFinal-tempInitial)) / ((m_w*c_w + m_g*c_g)*(boilingPoint_c-tempFinal) + (m_w*c_w + m_g*c_g)*(tempFinal-tempInitial));
         if( r < 0 || r > 1 ) {
            QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation.") );
            qCritical().nospace() << Q_FUNC_INFO << "Decoction: r=" << r;
            return;
         }

         mashStep->setAmount_l( r*(m_w + m_g/grainDensity) );
      }
      else {
         tempFinal = mashStep->startTemp_c().value_or(0.0);
         tempInitial = steps[i-1]->startTemp_c().value_or(0.0);
         tempWater = boilingPoint_c; // Assume adding boiling water to minimize final volume.
         MC += massWater * HeatCalculations::Cw_calGC; // Add thermal mass of last addition.

         massWater = (MC*(tempFinal-tempInitial))/(HeatCalculations::Cw_calGC * (tempWater-tempFinal));

         mashStep->setAmount_l(massWater);
         mashStep->setInfuseTemp_c(tempWater);
      }
   }

   // if no sparge, adjust volume of last step to meet target runoff volume
   if ( m_bGroup->checkedButton() == radioButton_noSparge  && steps.size() > 1) {
      double otherMashStepTotal = 0.0;
      for (int i = 0; i < steps.size()-1; ++i) {
         otherMashStepTotal += steps[i]->amount_l();
      }

      mashStep = steps.back();

      if (steps.size() > 1 ) {
         tempInitial = steps[steps.size()-2]->startTemp_c().value_or(0.0);
      } else {
         tempInitial = mash->grainTemp_c();
      }

      double targetWortFromMash= m_recObs->targetTotalMashVol_l() + lauterDeadspace;

      massWater = (targetWortFromMash - otherMashStepTotal)*Algorithms::getWaterDensity_kgL(0);

      tempFinal = mashStep->startTemp_c().value_or(0.0);
      MC += massWater * HeatCalculations::Cw_calGC; // Add thermal mass of last addition.


      tempWater = (MC*(tempFinal-tempInitial))/massWater/HeatCalculations::Cw_calGC + tempFinal;

      if(tempWater > boilingPoint_c)
         QMessageBox::information(this,
                                  tr("Infusion temp."),
                                  tr("In order to hit your target temp on the final step, the infusion water must be above boiling. Lower your initial infusion volume."));

      mashStep->setAmount_l(massWater);
      mashStep->setInfuseTemp_c(tempWater);
   }

   // Now, do a sparge step, using just enough water that the total
   // volume sums up to the target pre-boil size. We need to account for the potential
   // lauter dead space, I think?
   double spargeWater_l = m_recObs->targetTotalMashVol_l() - m_recObs->mash()->totalMashWater_l();

   // If I've done my math right, we should never get here on nosparge
   // not sure why I am inferring this when I could just check the button group?
   if( spargeWater_l >= 0.001 )
   {
      spargeWater_l += lauterDeadspace;
      int lastMashStep = steps.size()-1;
      tempFinal = mash->spargeTemp_c().value_or(0.0); // TODO We should have some better logic if spargeTemp_c is unset or 0.
      if( lastMashStep >= 0 )
         tempInitial = steps[lastMashStep]->startTemp_c().value_or(0.0) - 10.0; // You will lose about 10C from last step.
      else
      {
         qCritical() << Q_FUNC_INFO << "Should have had at least one mash step before getting to sparging.";
         return;
      }
      MC = m_recObs->grainsInMash_kg() * HeatCalculations::Cgrain_calGC
           + absorption_LKg * m_recObs->grainsInMash_kg() * HeatCalculations::Cw_calGC
           + mash->mashTunWeight_kg().value_or(0.0) * mash->mashTunSpecificHeat_calGC().value_or(0.0);

      massWater = spargeWater_l;

      tempWater = (MC/(massWater*HeatCalculations::Cw_calGC))*(tempFinal-tempInitial) + tempFinal;

      if(tempWater > boilingPoint_c)
         QMessageBox::information(this,
                                  tr("Sparge temp."),
                                  tr("In order to hit your sparge temp, the sparge water must be above boiling. Lower your sparge temp, or allow for more sparge water."));

      if ( m_bGroup->checkedButton() == radioButton_batchSparge ) {
         int numSteps = spinBox_batches->value();
         double volPerBatch = spargeWater_l/numSteps; // its evil, but deal with it
         for(int i=0; i < numSteps; ++i ) {
            auto newMashStep = std::make_shared<MashStep>(tr("Batch Sparge %1").arg(i+1));
            newMashStep->setType(MashStep::Type::BatchSparge);
            newMashStep->setAmount_l(volPerBatch);
            newMashStep->setInfuseTemp_c(tempWater);
            newMashStep->setEndTemp_c(tempWater);
            newMashStep->setStartTemp_c(tempFinal);
            newMashStep->setStepTime_mins(15);
            newMashStep->setOwnerId(mash->key());
            ObjectStoreWrapper::insert(newMashStep);
            steps.append(newMashStep);
            newMashStep->setStepNumber(steps.size());
            emit newMashStep->changed(
               newMashStep->metaObject()->property(
                     newMashStep->metaObject()->indexOfProperty(*PropertyNames::MashStep::type)
               )
            );
         }
         emit mash->stepsChanged();
      }
      // fly sparge, I think
      else {
         auto newMashStep = std::make_shared<MashStep>(tr("Fly Sparge"));
         newMashStep->setType(MashStep::Type::FlySparge);
         newMashStep->setAmount_l(spargeWater_l);
         newMashStep->setInfuseTemp_c(tempWater);
         newMashStep->setEndTemp_c(tempWater);
         newMashStep->setStartTemp_c(tempFinal);
         newMashStep->setStepTime_mins(15);
         newMashStep->setOwnerId(mash->key());
         ObjectStoreWrapper::insert(newMashStep);
         steps.append(newMashStep);
         newMashStep->setStepNumber(steps.size());
         emit newMashStep->changed(
            newMashStep->metaObject()->property(
                  newMashStep->metaObject()->indexOfProperty(*PropertyNames::MashStep::type)
            )
         );
      }

   }
   else if ( m_bGroup->checkedButton() != radioButton_noSparge )
   {
      QMessageBox::information(this,
                               tr("Too much wort"),
                               tr("You have too much wort from the mash for your boil size. I suggest increasing the boil size by increasing the boil time, or reducing your mash thickness."));
   }
   return;
}
