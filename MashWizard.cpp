/*
 * MashWizard.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "unit.h"
#include "MashWizard.h"
#include "mash.h"
#include "mashstep.h"
#include "fermentable.h"
#include <QMessageBox>
#include "HeatCalculations.h"
#include "stringparsing.h"
#include "brewtarget.h"

MashWizard::MashWizard(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
   recObs = 0;
   
   connect(buttonBox, SIGNAL(accepted()), this, SLOT(wizardry()) );
   connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()) );
}

void MashWizard::setRecipe(Recipe* rec)
{
   recObs = rec;
}

void MashWizard::wizardry()
{
   if( recObs == 0 || recObs->getMash() == 0 )
      return;

   Mash* mash = recObs->getMash();
   MashStep* mashStep;
   int i, j, size;
   double thickness_LKg;
   double MC, MCw;
   double tw, tf, t1;
   double grainMass = 0.0, massWater = 0.0;
   double grainDensity = HeatCalculations::rhoGrain_KgL;
   
   size = mash->getNumMashSteps();
   thickness_LKg = Unit::qstringToSI(lineEdit_mashThickness->text());

   if( thickness_LKg <= 0.0 )
   {
      QMessageBox::information(this, tr("Bad thickness"), tr("You must have a positive mash thickness."));
      return;
   }
   // Ensure at least one mash step.
   if( size == 0 )
   {
      QMessageBox::information(this, tr("No steps"), tr("You must have at least one mash step to run the wizard."));
      return;
   }

   mashStep = mash->getMashStep(0);
   // Ensure first mash step is an infusion.
   if( mashStep->getType() != "Infusion" )
   {
      QMessageBox::information(this, tr("First step"), tr("Your first mash step must be an infusion."));
      return;
   }

   /*
   for( i = 0; i < recObs->getNumFermentables(); i++ )
   {
      Fermentable* ferm = recObs->getFermentable(i);
      // NOTE: we are assuming it's in the mash because it's recommended.
      // We are also assuming it's a grain.
      if( ferm->getRecommendMash() )
         grainMass += ferm->getAmount_kg();
   }
   */
   
   grainMass = recObs->getGrainsInMash_kg();

   // Do first step
   tf = mashStep->getStepTemp_c();
   t1 = mash->getGrainTemp_c();
   massWater = thickness_LKg * grainMass;
   MCw = HeatCalculations::Cw_calGC * massWater;
   MC = HeatCalculations::Cgrain_calGC * grainMass;
   // I am specifically ignoring BeerXML's request to only do this if mash->getEquipAdjust() is set.
   //if( mash->getEquipAdjust() )
      tw = MC/MCw * (tf-t1) + (mash->getTunSpecificHeat_calGC()*mash->getTunWeight_kg())/MCw * (tf-mash->getTunTemp_c()) + tf;
   //else
   //   tw = MC/MCw * (tf-t1) + tf;

   // Can't have water above boiling.
   if( tw > 100 )
   {
      QMessageBox::information(this, tr("Mash too thick"), tr("Your mash is too thick for desired temp. at first step."));
      return;
   }

   mashStep->setInfuseAmount_l(massWater);
   mashStep->setInfuseTemp_c(tw);
   // End of first step.

   // Do rest of steps.
   // Add mass*specific heat constant of equipment to MC.
   // I am specifically ignoring BeerXML's request to only do this if mash->getEquipAdjust() is set.
   //if( mash->getEquipAdjust() )
      MC += mash->getTunSpecificHeat_calGC()*mash->getTunWeight_kg();
   for( i = 1; i < size; ++i )
   {
      mashStep = mash->getMashStep(i);

      if( mashStep->getType() == "Temperature")
         continue;
      else if( mashStep->getType() == "Decoction" )
      {
         QMessageBox::warning(this, tr("Decoction"), tr("Haven't tested decoction calculations yet.\nUse at own risk."));
         double m_w, m_g, m_e, r;
         double c_w, c_g, c_e;

         tf = mashStep->getStepTemp_c();
         t1 = mash->getMashStep(i-1)->getStepTemp_c();

         m_w = 0; // Total mass of water.
         for(j = 0; j < i; ++j )
            m_w += mash->getMashStep(j)->getInfuseAmount_l();
         m_g = grainMass;
         m_e = (mash->getEquipAdjust()) ? mash->getTunWeight_kg() : 0;

         c_w = HeatCalculations::Cw_calGC;
         c_g = HeatCalculations::Cgrain_calGC;
         c_e = (mash->getEquipAdjust()) ? mash->getTunSpecificHeat_calGC() : 0;

         // r is the ratio of water and grain to take out for decoction.
         r = ((m_w*c_w + m_g*c_g + m_e*c_e)*(tf-t1)) / ((m_w*c_w + m_g*c_g)*(100-tf) + (m_w*c_w + m_g*c_g)*(tf-t1));
         if( r < 0 || r > 1 )
         {
            QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation.") );
            Brewtarget::log(Brewtarget::ERROR, "Decoction: r=" + doubleToString(r));
            return;
         }

         mashStep->setDecoctionAmount_l( r*(m_w + m_g/grainDensity) );
      }
      else
      {
         tf = mashStep->getStepTemp_c();
         t1 = mash->getMashStep(i-1)->getStepTemp_c();
         tw = 100; // Assume adding boiling water to minimize final volume.
         MC += massWater * HeatCalculations::Cw_calGC; // Add MC product of last addition.

         massWater = (MC*(tf-t1))/(HeatCalculations::Cw_calGC * (tw-tf));

         mashStep->setInfuseAmount_l(massWater);
         mashStep->setInfuseTemp_c(tw);
      }
   }
   
   // Now, do a sparge step to get the total volume of the mash up to the boil size.
   double wortInBoil_l = recObs->estimateWortFromMash_l();
   if( recObs->getEquipment() != 0 )
      wortInBoil_l -= recObs->getEquipment()->getLauterDeadspace_l();
   
   double spargeWater_l = recObs->getBoilSize_l() - wortInBoil_l;
   if( spargeWater_l >= 0.0 )
   {
      mashStep = new MashStep();
      tf = 74; // 74C is recommended in Palmer's How to Brew
      t1 = mash->getMashStep(size-1)->getStepTemp_c() - 10.0; // You will lose about 10C from last step.
      //MC += massWater * HeatCalculations::Cw_calGC; // Add MC product of last addition.
      MC = recObs->getGrainsInMash_kg() * HeatCalculations::Cgrain_calGC
           + HeatCalculations::absorption_LKg * recObs->getGrainsInMash_kg() * HeatCalculations::Cw_calGC
	   + mash->getTunWeight_kg() * mash->getTunSpecificHeat_calGC();
      massWater = spargeWater_l;
      
      tw = (MC/(massWater*HeatCalculations::Cw_calGC))*(tf-t1) + tf;
      
      mashStep->setName("Batch Sparge");
      mashStep->setType("Infusion");
      mashStep->setInfuseAmount_l(spargeWater_l);
      mashStep->setInfuseTemp_c(tw);
      mashStep->setEndTemp_c(tw);
      mashStep->setStepTemp_c(tf);
      mashStep->setStepTime_min(15);
      
      mash->addMashStep(mashStep);
   }
   else
   {
      QMessageBox::information(this, tr("Too much wort"),
      tr("You have too much wort from the mash for your boil size. I suggest increasing the boil size by increasing the boil time."));
   }
}