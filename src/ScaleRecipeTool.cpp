/*
 * ScaleRecipeTool.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include "ScaleRecipeTool.h"
#include <QMessageBox>
#include <QButtonGroup>
#include "brewtarget.h"
#include "recipe.h"
#include "fermentable.h"
#include "mash.h"
#include "mashstep.h"
#include "hop.h"
#include "misc.h"
#include "yeast.h"
#include "water.h"

ScaleRecipeTool::ScaleRecipeTool(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
   recObs = 0;

   scaleGroup.addButton(checkBox_batchSize);
   scaleGroup.addButton(checkBox_efficiency);

   checkBox_batchSize->setCheckState( Qt::Checked );
   lineEdit_newEfficiency->setDisabled(true);
   
   connect(&scaleGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(scaleGroupButtonPressed(QAbstractButton*)));
   connect(buttonBox, SIGNAL(accepted()), this, SLOT(scale()) );
   connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()) );
}

void ScaleRecipeTool::scaleGroupButtonPressed(QAbstractButton *button)
{
   if( button == qobject_cast<QAbstractButton*>(checkBox_batchSize) )
   {
      lineEdit_newBatchSize->setDisabled(false);
      lineEdit_newEfficiency->setDisabled(true);
      return;
   }

   if( button == qobject_cast<QAbstractButton*>(checkBox_efficiency) )
   {
      lineEdit_newBatchSize->setDisabled(true);
      lineEdit_newEfficiency->setDisabled(false);
      return;
   }
}

void ScaleRecipeTool::setRecipe(Recipe* rec)
{
   recObs = rec;
}

void ScaleRecipeTool::show()
{
   // Set the batch size display to the current batch size.
   if( recObs != 0 )
      lineEdit_newBatchSize->setText(recObs);
   
   setVisible(true);
}

void ScaleRecipeTool::scale()
{
   QCheckBox* button = qobject_cast<QCheckBox*>(scaleGroup.checkedButton());

   if( button == checkBox_batchSize )
      scaleByVolume();
   else if( button == checkBox_efficiency )
      scaleByEfficiency();
}

void ScaleRecipeTool::scaleByEfficiency()
{
   if( recObs == 0 )
      return;

   int i, size;

   double oldEfficiency = recObs->efficiency_pct();
   double newEfficiency = lineEdit_newEfficiency->toSI();

   double ratio = oldEfficiency / newEfficiency;

   recObs->setEfficiency_pct(newEfficiency);

   QList<Fermentable*> ferms = recObs->fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
   {
      Fermentable* ferm = ferms[i];
      // NOTE: why the hell do we need this?
      if( ferm == 0 )
         continue;

      if( !ferm->isSugar() && !ferm->isExtract() )
      {
         ferm->setAmount_kg(ferm->amount_kg() * ratio);
      }
   }

   Mash* mash = recObs->mash();
   if( mash == 0 )
      return;

   QList<MashStep*> mashSteps = mash->mashSteps();
   size = mashSteps.size();
   for( i = 0; i < size; ++i )
   {
      MashStep* step = mashSteps[i];
      // NOTE: why the hell do we need this?
      if( step == 0 )
         continue;

      // Reset all these to zero so that the user
      // will know to re-run the mash wizard.
      step->setDecoctionAmount_l(0);
      step->setInfuseAmount_l(0);
   }

   // Let the user know what happened.
   QMessageBox::information(this, tr("Recipe Scaled"),
             tr("The mash has been reset due to the fact that mash temperatures do not scale easily. Please re-run the mash wizard.") );
}

void ScaleRecipeTool::scaleByVolume()
{
   if( recObs == 0 )
      return;
   
   int i, size;
   
   double currentBatchSize_l = recObs->batchSize_l();
   double newBatchSize_l = lineEdit_newBatchSize->toSI();
   
   double ratio = newBatchSize_l / currentBatchSize_l;
   
   // I think you want the equipment to be clean.
   //recObs->setEquipment(new Equipment());
   recObs->setBatchSize_l(newBatchSize_l);
   recObs->setBoilSize_l(newBatchSize_l);
   
   QList<Fermentable*> ferms = recObs->fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
   {
      Fermentable* ferm = ferms[i];
      // NOTE: why the hell do we need this?
      if( ferm == 0 )
         continue;
      
      ferm->setAmount_kg(ferm->amount_kg() * ratio);
   }
   
   QList<Hop*> hops = recObs->hops();
   size = hops.size();
   for( i = 0; i < size; ++i )
   {
      Hop* hop = hops[i];
      // NOTE: why the hell do we need this?
      if( hop == 0 )
         continue;
      
      hop->setAmount_kg(hop->amount_kg() * ratio);
   }
   
   QList<Misc*> miscs = recObs->miscs();
   size = miscs.size();
   for( i = 0; i < size; ++i )
   {
      Misc* misc = miscs[i];
      // NOTE: why the hell do we need this?
      if( misc == 0 )
         continue;
      
      misc->setAmount( misc->amount() * ratio );
   }
   
   QList<Water*> waters = recObs->waters();
   size = waters.size();
   for( i = 0; i < size; ++i )
   {
      Water* water = waters[i];
      // NOTE: why the hell do we need this?
      if( water == 0 )
         continue;
      
      water->setAmount_l(water->amount_l() * ratio);
   }
   
   Mash* mash = recObs->mash();
   if( mash == 0 )
      return;
   
   QList<MashStep*> mashSteps = mash->mashSteps();
   size = mashSteps.size();
   for( i = 0; i < size; ++i )
   {
      MashStep* step = mashSteps[i];
      // NOTE: why the hell do we need this?
      if( step == 0 )
         continue;
      
      // Reset all these to zero so that the user
      // will know to re-run the mash wizard.
      step->setDecoctionAmount_l(0);
      step->setInfuseAmount_l(0);
   }
   
   // I don't think I should scale the yeasts.
   
   // Let the user know what happened.
   QMessageBox::information(this, tr("Recipe Scaled"),
             tr("The equipment and mash have been reset due to the fact that mash temperatures do not scale easily. Please re-run the mash wizard.") );
}
