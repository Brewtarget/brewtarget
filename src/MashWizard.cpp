/*
 * MashWizard.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - David Grundberg <individ@acc.umu.se>
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "database.h"
#include "brewtarget.h"
#include "unit.h"
#include "MashWizard.h"
#include "mash.h"
#include "mashstep.h"
#include "fermentable.h"
#include <QMessageBox>
#include "HeatCalculations.h"
#include "brewtarget.h"
#include "equipment.h"
#include "PhysicalConstants.h"

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

void MashWizard::show()
{
   if( recObs == 0 || recObs->mash() == 0 )
      return;

   // Ensure at least one mash step.
   if( recObs->mash()->mashSteps().size() == 0 )
   {
      QMessageBox::information(this, tr("No steps"), tr("There must be at least one mash step to run the wizard."));
      return;
   }

   Brewtarget::getThicknessUnits(&volumeUnit,&weightUnit);
   label_mashThickness->setText(tr("Mash thickness (%1/%2)").arg(volumeUnit->getUnitName(),weightUnit->getUnitName()));
   
   setVisible(true);
}

void MashWizard::wizardry()
{
   if( recObs == 0 || recObs->mash() == 0 )
      return;

   Mash* mash = recObs->mash();
   MashStep* mashStep;
   int i, j;
   double thickness_LKg;
   double thickNum;
   double MC, MCw; // Thermal mass of mash and water.
   double tw, tf, t1; // Water, final, and initial temps.
   double grainMass = 0.0, massWater = 0.0;
   double grainDensity = PhysicalConstants::grainDensity_kgL;
   double absorption_LKg;
   double boilingPoint_c; 

   bool ok = false;

   // If we have an equipment, utilize the custom absorption and boiling temp.
   if( recObs->equipment() != 0 )
   {
      absorption_LKg = recObs->equipment()->grainAbsorption_LKg();
      boilingPoint_c = recObs->equipment()->boilingPoint_c();
   }
   else
   {
      absorption_LKg = PhysicalConstants::grainAbsorption_Lkg;
      boilingPoint_c = 100.0;
   }

   thickNum = Brewtarget::toDouble(lineEdit_mashThickness->text(), &ok);
   if ( ! ok ) 
      Brewtarget::logW( QString("MashWizard::wizardry() could not convert %1 to double").arg(lineEdit_mashThickness->text()));

   thickness_LKg = thickNum * volumeUnit->toSI(1) / weightUnit->toSI(1);

   if( thickness_LKg <= 0.0 )
   {
      QMessageBox::information(this, tr("Bad thickness"), tr("You must have a positive mash thickness."));
      return;
   }
   
   QList<MashStep*> steps = mash->mashSteps();
   // We ensured that there was at least one mash step when we displayed the thickness dialog in show().
   mashStep = steps.at(0);
   if( mashStep == 0 )
   {
      Brewtarget::logE( "MashWizard::wizardry(): first mash step was null." );
      return;
   }
   
   // Ensure first mash step is an infusion.
   if( mashStep->type() != MashStep::Infusion )
   {
      QMessageBox::information(this, tr("First step"), tr("Your first mash step must be an infusion."));
      return;
   }

   // Find any batch sparges and remove them for now.
   for( i = 0; i < steps.size(); ++i)
   {
      MashStep* step = steps[i];
      if( step && step->name() == "Final Batch Sparge" )
         Database::instance().removeFrom(mash,step);
   }

   steps = mash->mashSteps();
   grainMass = recObs->grainsInMash_kg();

   // Do first step
   tf = mashStep->stepTemp_c();
   t1 = mash->grainTemp_c();
   massWater = thickness_LKg * grainMass;
   MCw = HeatCalculations::Cw_calGC * massWater;
   MC = HeatCalculations::Cgrain_calGC * grainMass;
   // I am specifically ignoring BeerXML's request to only do this if mash->getEquipAdjust() is set.
   // Good or bad?
   //if( mash->getEquipAdjust() )
      tw = MC/MCw * (tf-t1) + (mash->tunSpecificHeat_calGC()*mash->tunWeight_kg())/MCw * (tf-mash->tunTemp_c()) + tf;
   //else
   //   tw = MC/MCw * (tf-t1) + tf;

   // Can't have water above boiling.
   if( tw > boilingPoint_c )
   {
      QMessageBox::information(this,
                               tr("Mash too thick"),
                               tr("Your mash is too thick for desired temp. at first step."));
      return;
   }

   mashStep->setInfuseAmount_l(massWater);
   mashStep->setInfuseTemp_c(tw);
   //================End of first step=====================

   // Do rest of steps.
   // Add thermal mass of equipment to MC.
   // I am specifically ignoring BeerXML's request to only do this if mash->getEquipAdjust() is set.
   MC += mash->tunSpecificHeat_calGC()*mash->tunWeight_kg();
   
   for( i = 1; i < steps.size(); ++i )
   {
      mashStep = steps[i];

      if( mashStep->type() == MashStep::Temperature )
         continue;
      else if( mashStep->type() == MashStep::Decoction )
      {
         double m_w, m_g, m_e, r;
         double c_w, c_g, c_e;

         tf = mashStep->stepTemp_c();
         t1 = steps[i-1]->stepTemp_c();

         m_w = 0; // Total mass of water.
         for(j = 0; j < i; ++j )
            m_w += steps[j]->infuseAmount_l();
         m_g = grainMass;
         m_e = (mash->equipAdjust()) ? mash->tunWeight_kg() : 0;

         c_w = HeatCalculations::Cw_calGC;
         c_g = HeatCalculations::Cgrain_calGC;
         c_e = (mash->equipAdjust()) ? mash->tunSpecificHeat_calGC() : 0;

         // r is the ratio of water and grain to take out for decoction.
         r = ((m_w*c_w + m_g*c_g + m_e*c_e)*(tf-t1)) / ((m_w*c_w + m_g*c_g)*(boilingPoint_c-tf) + (m_w*c_w + m_g*c_g)*(tf-t1));
         if( r < 0 || r > 1 )
         {
            QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation.") );
            Brewtarget::logE(QString("Decoction: r=%1").arg(r));
            return;
         }

         mashStep->setDecoctionAmount_l( r*(m_w + m_g/grainDensity) );
      }
      else
      {
         tf = mashStep->stepTemp_c();
         t1 = steps[i-1]->stepTemp_c();
         tw = boilingPoint_c; // Assume adding boiling water to minimize final volume.
         MC += massWater * HeatCalculations::Cw_calGC; // Add thermal mass of last addition.

         massWater = (MC*(tf-t1))/(HeatCalculations::Cw_calGC * (tw-tf));

         mashStep->setInfuseAmount_l(massWater);
         mashStep->setInfuseTemp_c(tw);
      }
   }
   
   // Now, do a sparge step, using just enough water that the total
   // volume sums up to the target pre-boil size.
   double spargeWater_l = recObs->boilSize_l() - recObs->wortFromMash_l();

   if( recObs->equipment() != 0 )
   {
      // These variables are part of the boil size but not the wort
      // size and the wizard should account for that.
      spargeWater_l += recObs->equipment()->lauterDeadspace_l();
      spargeWater_l -= recObs->equipment()->topUpKettle_l();
   }

   // Need to account for extract/sugar volume also.
   QList<Fermentable*> ferms = recObs->fermentables();
   foreach( Fermentable* f, ferms )
   {
      Fermentable::Type type = f->type();
      if( type == Fermentable::Extract )
         spargeWater_l -= f->amount_kg() / PhysicalConstants::liquidExtractDensity_kgL;
      else if( type == Fermentable::Sugar )
         spargeWater_l -= f->amount_kg() / PhysicalConstants::sucroseDensity_kgL;
      else if( type == Fermentable::Dry_Extract )
         spargeWater_l -= f->amount_kg() / PhysicalConstants::dryExtractDensity_kgL;
   }

   if( spargeWater_l >= 0.0 )
   {
      // If the recipe already has a mash step named "Final Batch Sparge",
      // find it and use it instead of making a new one.
      bool foundSparge = false;
      for( j = 0; j < steps.size(); ++j )
      {
         if( steps[j]->name() == "Final Batch Sparge" )
         {
            mashStep = steps[j];
            foundSparge = true;
            break;
         }
      }
      if( ! foundSparge )
      {
         mashStep = Database::instance().newMashStep(mash); // Or just make a new one.
         steps.append(mashStep);
      }

      int lastMashStep = steps.size()-1;
      tf = mash->spargeTemp_c();
      if( lastMashStep >= 0 )
         t1 = steps[lastMashStep]->stepTemp_c() - 10.0; // You will lose about 10C from last step.
      else
      {
         Brewtarget::logE( "MashWizard::wizardry(): Should have had at least one mash step before getting to sparging." );
         return;
      }
      MC = recObs->grainsInMash_kg() * HeatCalculations::Cgrain_calGC
           + absorption_LKg * recObs->grainsInMash_kg() * HeatCalculations::Cw_calGC
      + mash->tunWeight_kg() * mash->tunSpecificHeat_calGC();
      massWater = spargeWater_l;
      
      tw = (MC/(massWater*HeatCalculations::Cw_calGC))*(tf-t1) + tf;
      
      if(tw > boilingPoint_c)
         QMessageBox::information(this,
                                  tr("Sparge temp."),
                                  tr("In order to hit your sparge temp, the sparge water must be above boiling. Lower your sparge temp, or allow for more sparge water."));

      mashStep->setName("Final Batch Sparge");
      mashStep->setType(MashStep::Infusion);
      mashStep->setInfuseAmount_l(spargeWater_l);
      mashStep->setInfuseTemp_c(tw);
      mashStep->setEndTemp_c(tw);
      mashStep->setStepTemp_c(tf);
      mashStep->setStepTime_min(15);
   }
   else
   {
      QMessageBox::information(this,
                               tr("Too much wort"),
                               tr("You have too much wort from the mash for your boil size. I suggest increasing the boil size by increasing the boil time, or reducing your mash thickness."));
   }
}

