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
#include <QButtonGroup>

MashWizard::MashWizard(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
   bGroup = new QButtonGroup();
   recObs = 0;

   bGroup->addButton(radioButton_noSparge);
   bGroup->addButton(radioButton_batchSparge);
   bGroup->addButton(radioButton_flySparge);

//   radioButton_batchSparge->setChecked(true);

   connect(bGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(toggleSpinBox(QAbstractButton*)));
   connect(buttonBox, SIGNAL(accepted()), this, SLOT(wizardry()) );
   connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()) );
}

void MashWizard::toggleSpinBox(QAbstractButton* button)
{
   if ( button == radioButton_noSparge ) {
      widget_batches->setEnabled(false);
      widget_mashThickness->setEnabled(false);
   }
   else if ( button == radioButton_flySparge ) {
      widget_batches->setEnabled(false);
      widget_mashThickness->setEnabled(true);
   }
   else {
      widget_batches->setEnabled(true);
      widget_mashThickness->setEnabled(true);
   }
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

   MashStep *firstStep = recObs->mash()->mashSteps().first();
   MashStep *lastStep = recObs->mash()->mashSteps().last();

   // Recalculate the mash thickness
   double thickNum = firstStep->infuseAmount_l()/recObs->grainsInMash_kg();
   double thickness = thickNum * weightUnit->toSI(1) / volumeUnit->toSI(1) ;
   doubleSpinBox_thickness->setValue(thickness);

   // Is this a batch, fly or no sparge?
   if ( firstStep == lastStep ) {
      radioButton_noSparge->setChecked(true);
      widget_batches->setEnabled(false);
      widget_mashThickness->setEnabled(false);
   }
   else if ( lastStep->type() == MashStep::flySparge ) {
      radioButton_flySparge->setChecked(true);
      widget_batches->setEnabled(false);
      widget_mashThickness->setEnabled(true);
   }
   else {
      int countSteps = 0;
      QList<MashStep*> steps = recObs->mash()->mashSteps();
      foreach( MashStep* step, steps) {
         if ( step->isSparge() ) {
            countSteps++;
         }
      }
      widget_batches->setEnabled(true);
      widget_mashThickness->setEnabled(true);
      radioButton_batchSparge->setChecked(true);
      spinBox_batches->setValue(countSteps);
   }

   setVisible(true);
}

double MashWizard::calcDecoctionAmount( MashStep* step, Mash* mash, double waterMass, double grainMass, double lastTemp, double boiling)
{
   double grainDensity = PhysicalConstants::grainDensity_kgL;

   double stepTemp  = step->stepTemp_c();
   double equipMass = (mash->equipAdjust()) ? mash->tunWeight_kg() : 0;
   double c_e       = (mash->equipAdjust()) ? mash->tunSpecificHeat_calGC() : 0;

   double grHeat = grainMass * HeatCalculations::Cw_calGC;
   double waHeat = waterMass * HeatCalculations::Cgrain_calGC;
   double eqHeat = equipMass * c_e;

   double totalHeat = grHeat + waHeat;
   double deltaTemp = stepTemp - lastTemp;

   // r is the ratio of water and grain to take out for decoction.
   double r = ((totalHeat + eqHeat)*deltaTemp) / (totalHeat*(boiling - stepTemp) + totalHeat*deltaTemp);

   if( r < 0 || r > 1 )
   {
      QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation.") );
      Brewtarget::logE(QString("%1: r=%2").arg(Q_FUNC_INFO).arg(r));
      return -1;
   }
   return r * (waterMass + grainMass/grainDensity);

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

   grainMass = recObs->grainsInMash_kg();
   if ( bGroup->checkedButton() != radioButton_noSparge ) {
      thickNum = doubleSpinBox_thickness->value();
      thickness_LKg = thickNum * volumeUnit->toSI(1) / weightUnit->toSI(1);
   }
   // No sparge is mostly heat calculations. I think this could work
   else {
      thickNum = mash->totalMashWater_l()/grainMass;
      // We don't need to adjust for units, since we are already using kg of
      // grains and L of water
      thickness_LKg = thickNum;
   }

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
   if( ! mashStep->isInfusion() && ! mashStep->isSparge() )
   {
      QMessageBox::information(this, tr("First step"), tr("Your first mash step must be an infusion."));
      return;
   }

   // Find any batch sparges and remove them for now.
   for( i = 0; i < steps.size(); ++i)
   {
      MashStep* step = steps[i];
      // NOTE: For backwards compatibility, the Final Batch Sparge comparison
      // must be allowed. No matter how much we desire otherwise.
      if( step->isSparge() || step->name() == "Final Batch Sparge" )
         Database::instance().removeFrom(mash,step);
   }

   steps = mash->mashSteps();

   // Do first step
   tf = mashStep->stepTemp_c();
   t1 = mash->grainTemp_c();
   massWater = thickness_LKg * grainMass;
   MCw = HeatCalculations::Cw_calGC * massWater;
   MC = HeatCalculations::Cgrain_calGC * grainMass;

   // I am specifically ignoring BeerXML's request to only do this if mash->getEquipAdjust() is set.
   tw = MC/MCw * (tf-t1) + (mash->tunSpecificHeat_calGC()*mash->tunWeight_kg())/MCw * (tf-mash->tunTemp_c()) + tf;

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

      if( mashStep->isTemperature() )
         continue;
      else if( mashStep->isDecoction() )
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

   // If I've done my math right, we should never get here on nosparge
   if( spargeWater_l >= 0.0 )
   {
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

      if ( bGroup->checkedButton() == radioButton_batchSparge ) {
         int numSteps = spinBox_batches->value();
         double volPerBatch = spargeWater_l/numSteps; // its evil, but deal with it

         for(int i=0; i < numSteps; ++i ) {
            mashStep = Database::instance().newMashStep(mash);
            steps.append(mashStep);

            mashStep->setName(tr("Batch Sparge %1").arg(i+1));
            mashStep->setType(MashStep::batchSparge);
            mashStep->setInfuseAmount_l(volPerBatch);
            mashStep->setInfuseTemp_c(tw);
            mashStep->setEndTemp_c(tw);
            mashStep->setStepTemp_c(tf);
            mashStep->setStepTime_min(15);
         }
      }
      // fly sparge, I think
      else {
         mashStep = Database::instance().newMashStep(mash);
         steps.append(mashStep);

         mashStep->setName(tr("Fly Sparge"));
         mashStep->setType(MashStep::flySparge);
         mashStep->setInfuseAmount_l(spargeWater_l);
         mashStep->setInfuseTemp_c(tw);
         mashStep->setEndTemp_c(tw);
         mashStep->setStepTemp_c(tf);
         mashStep->setStepTime_min(15);
      }

   }
   else if ( bGroup->checkedButton() != radioButton_noSparge )
   {
      QMessageBox::information(this,
                               tr("Too much wort"),
                               tr("You have too much wort from the mash for your boil size. I suggest increasing the boil size by increasing the boil time, or reducing your mash thickness."));
   }
}

