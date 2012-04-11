/*
 * recipe.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#include "instruction.h"
#include "brewtarget.h"
#include "database.h"

//#include <algorithm>
//#include <ctime>
#include <cmath> // For pow/log

#include <QList>
#include <QDate>
#include <QInputDialog>
#include <QObject>
#include <QDebug>
#include <QSharedPointer>

#include "recipe.h"
#include "style.h"
#include "misc.h"
#include "mash.h"
#include "mashstep.h"
#include "hop.h"
#include "fermentable.h"
#include "equipment.h"
#include "yeast.h"
#include "water.h"
#include "PreInstruction.h"
#include "Algorithms.h"
#include "IbuMethods.h"
#include "ColorMethods.h"

#include "HeatCalculations.h"
#include "QueuedMethod.h"

QHash<QString,QString> Recipe::tagToProp = Recipe::tagToPropHash();

QHash<QString,QString> Recipe::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = "name";
   propHash["TYPE"] = "type";
   propHash["BREWER"] = "brewer";
   propHash["BATCH_SIZE"] = "batchSize_l";
   propHash["BOIL_SIZE"] = "boilSize_l";
   propHash["BOIL_TIME"] = "boilTime_min";
   propHash["EFFICIENCY"] = "efficiency_pct";
   propHash["ASST_BREWER"] = "asstBrewer";
   propHash["NOTES"] = "notes";
   propHash["TASTE_NOTES"] = "tasteNotes";
   propHash["TASTE_RATING"] = "tasteRating";
   propHash["OG"] = "og";
   propHash["FG"] = "fg";
   propHash["FERMENTATION_STAGES"] = "fermentationStages";
   propHash["PRIMARY_AGE"] = "primaryAge_days";
   propHash["PRIMARY_TEMP"] = "primaryTemp_c";
   propHash["SECONDARY_AGE"] = "secondaryAge_days";
   propHash["SECONDARY_TEMP"] = "secondaryTemp_c";
   propHash["TERTIARY_AGE"] = "tertiaryAge_days";
   propHash["TERTIARY_TEMP"] = "tertiaryTemp_c";
   propHash["AGE"] = "age";
   propHash["AGE_TEMP"] = "ageTemp_c";
   propHash["DATE"] = "date";
   propHash["CARBONATION"] = "carbonation_vols";
   propHash["FORCED_CARBONATION"] = "forcedCarbonation";
   propHash["PRIMING_SUGAR_NAME"] = "primingSugarName";
   propHash["CARBONATION_TEMP"] = "carbonationTemp_c";
   propHash["PRIMING_SUGAR_EQUIV"] = "primingSugarEquiv";
   propHash["KEG_PRIMING_FACTOR"] = "kegPrimingFactor";
   return propHash;
}

bool operator<(Recipe &r1, Recipe &r2 )
{
   return r1.name() < r2.name();
}

bool operator==(Recipe &r1, Recipe &r2 )
{
   return r1.name() == r2.name();
}

void Recipe::clear()
{
   // TODO: implement.
   /*
   QString name = getName();
   setDefaults();
   setName(name);
   hasChanged();
   */
}

Recipe::Recipe()
   : BeerXMLElement(),
     _points(0),
     _ABV_pct(0),
     _color_srm(0),
     _boilGrav(1.000),
     _IBU(0),
     _wortFromMash_l(1.0),
     _boilVolume_l(1.0),
     _postBoilVolume_l(1.0),
     _finalVolume_l(1.0),
     _calories(0),
     _grainsInMash_kg(0),
     _grains_kg(0),
     _SRMColor(255,255,0),
     _og(1.000),
     _fg(1.000),
     _uninitializedCalcs(true)
{
}

Recipe::Recipe( Recipe const& other ) : BeerXMLElement(other)
{
}

void Recipe::removeInstruction(Instruction* ins)
{
   Database::instance().removeFromRecipe( this, ins );
}

void Recipe::swapInstructions( Instruction* ins1, Instruction* ins2 )
{
   QList<Instruction*> ins = instructions();
   if( !(ins.contains(ins1) && ins.contains(ins2)) )
      return;
   
   Database::instance().swapInstructionOrder(ins1, ins2);
}

void Recipe::clearInstructions()
{
   QList<Instruction*> ins = instructions();
   int i, size;
   size = ins.size();
   for( i = 0; i < size; ++i )
      removeInstruction(ins[i]);
}

void Recipe::insertInstruction(Instruction* ins, int pos)
{
   if( ins == 0 || !(instructions().contains(ins)) )
      return;

   Database::instance().insertInstruction(ins,pos);
}

Instruction* Recipe::mashFermentableIns()
{
   Instruction* ins;
   QString str,tmp;
   unsigned int i;

   /*** Add grains ***/
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Add grains"));
   str = tr("Add ");
   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      if( ferms[i]->isMashed() )
      {
         tmp = QString("%1 %2, ")
               .arg(Brewtarget::displayAmount(ferms[i]->amount_kg(), Units::kilograms))
               .arg(ferms[i]->name());
         str += tmp;
         ins->addReagent(tmp);
      }
   }
   str += tr("to the mash tun.");
   ins->setDirections(str);

   return ins;

}

Instruction* Recipe::mashWaterIns(unsigned int size)
{
   Instruction* ins;
   MashStep* mstep;
   QString str, tmp;
   unsigned int i;

   if( mash() == 0 )
      return 0;
   
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Heat water"));
   str = tr("Bring ");
   QList<MashStep*> msteps = mash()->mashSteps();
   for( i = 0; i < size; ++i )
   {
      mstep = msteps[i];
      if( mstep->type() != MashStep::Infusion )
         continue;

      tmp = tr("%1 water to %2, ")
             .arg(Brewtarget::displayAmount(mstep->infuseAmount_l(), Units::liters))
             .arg(Brewtarget::displayAmount(mstep->infuseTemp_c(), Units::celsius));
      str += tmp;
      ins->addReagent(tmp);
   }
   str += tr("for upcoming infusions.");
   ins->setDirections(str);

   return ins;
}

QVector<PreInstruction> Recipe::mashInstructions(double timeRemaining, double totalWaterAdded_l, unsigned int size)
{
   QVector<PreInstruction> preins;
   MashStep* mstep;
   QString str;
   unsigned int i;

   if( mash() == 0 )
      return preins;
   
   QList<MashStep*> msteps = mash()->mashSteps();
   for( i = 0; i < size; ++i )
   {
      mstep = msteps[i];

      if( mstep->type() == MashStep::Infusion )
      {
         str = tr("Add %1 water at %2 to mash to bring it to %3.")
               .arg(Brewtarget::displayAmount(mstep->infuseAmount_l(), Units::liters))
               .arg(Brewtarget::displayAmount(mstep->infuseTemp_c(), Units::celsius))
               .arg(Brewtarget::displayAmount(mstep->stepTemp_c(), Units::celsius));
         totalWaterAdded_l += mstep->infuseAmount_l();
      }
      else if( mstep->type() == MashStep::Temperature )
      {
         str = tr("Heat mash to %1.").arg(Brewtarget::displayAmount(mstep->stepTemp_c(), Units::celsius));
      }
      else if( mstep->type() == MashStep::Decoction )
      {
         str = tr("Bring %1 of the mash to a boil and return to the mash tun to bring it to %2.")
               .arg(Brewtarget::displayAmount(mstep->decoctionAmount_l(), Units::liters))
               .arg(Brewtarget::displayAmount(mstep->stepTemp_c(), Units::celsius));
      }

      str += tr(" Hold for %1.").arg(Brewtarget::displayAmount(mstep->stepTime_min(), Units::minutes));

      preins.push_back(PreInstruction(str, QString("%1 - %2").arg(mstep->typeStringTr()).arg(mstep->name()),
                  timeRemaining));
      timeRemaining -= mstep->stepTime_min();
   }
   return preins;
}

QVector<PreInstruction> Recipe::hopSteps(Hop::Use type)
{
   QVector<PreInstruction> preins;
   QString str;
   unsigned int i;
   int size;

   preins.clear();
   QList<Hop*> hlist = hops();
   size = hlist.size();
   for( i = 0; static_cast<int>(i) < size; ++i )
   {
      Hop* hop = hlist[i];
      if( hop->use() == type )
      {
         if( type == Hop::Boil )
            str = tr("Put %1 %2 into boil for %3.");
         else if( type == Hop::Dry_Hop )
            str = tr("Put %1 %2 into fermenter for %3.");
         else if( type == Hop::First_Wort )
            str = tr("Put %1 %2 into first wort for %3.");
         else if( type == Hop::Mash )
            str = tr("Put %1 %2 into mash for %3.");
         else if( type == Hop::UseAroma )
            str = tr("Steep %1 %2 in wort for %3.");
         else
         {
            Brewtarget::logW("Recipe::hopSteps(): Unrecognized hop use.");
            str = tr("Use %1 %2 for %3");
         }

         str = str.arg(Brewtarget::displayAmount(hop->amount_kg(), Units::kilograms))
                  .arg(hop->name())
                  .arg(Brewtarget::displayAmount(hop->time_min(), Units::minutes));

         preins.push_back(PreInstruction(str, tr("Hop addition"), hop->time_min()));
      }
   }
   return preins;
}

QVector<PreInstruction> Recipe::miscSteps(Misc::Use type)
{
   QVector<PreInstruction> preins;
   QString str;
   unsigned int i;
   int size;

   QList<Misc*> mlist = miscs();
   size = mlist.size();
   for( i = 0; static_cast<int>(i) < size; ++i )
   {
      Misc* misc = mlist[i];
      if( misc->use() == type )
      {
         if( type == Misc::Boil )
            str = tr("Put %1 %2 into boil for %3.");
         else if( type == Misc::Bottling )
            str = tr("Use %1 %2 at bottling for %3.");
         else if( type == Misc::Mash )
            str = tr("Put %1 %2 into mash for %3.");
         else if( type == Misc::Primary )
            str = tr("Put %1 %2 into primary for %3.");
         else if( type == Misc::Secondary )
            str = tr("Put %1 %2 into secondary for %3.");
         else
         {
            Brewtarget::logW("Recipe::getMiscSteps(): Unrecognized misc use.");
            str = tr("Use %1 %2 for %3.");
         }

         str = str .arg(Brewtarget::displayAmount(misc->amount(), ((misc->amountIsWeight()) ? (Unit*)(Units::kilograms) : (Unit*)(Units::liters) )))
                   .arg(misc->name())
                   .arg(Brewtarget::displayAmount(misc->time(), Units::minutes));

         preins.push_back(PreInstruction(str, tr("Misc addition"), misc->time()));
      }
   }
   return preins;
}

Instruction* Recipe::firstWortHopsIns()
{
   Instruction* ins;
   QString str,tmp;
   unsigned int i;
   int size;
   bool hasHop = false;

   str = tr("Do first wort hopping with ");
   QList<Hop*> hlist = hops();
   size = hlist.size();
   for( i = 0; static_cast<int>(i) < size; ++i )
   {
     Hop* hop = hlist[i];
     if( hop->use() == Hop::First_Wort )
     {
       tmp = QString("%1 %2,")
            .arg(Brewtarget::displayAmount(hop->amount_kg(), Units::kilograms))
            .arg(hop->name());
       str += tmp;
       hasHop = true;
     }
   }
   str += ".";
   if( hasHop )
   {
     ins = Database::instance().newInstruction(this);
     ins->setName(tr("First wort hopping"));
     ins->setDirections(str);
     ins->addReagent(tmp);
     return ins;
   }
   return 0;
}

Instruction* Recipe::topOffIns()
{
   double wortInBoil_l = 0.0;
   QString str,tmp;
   Instruction* ins;

   Equipment* e = equipment();
   if( e != 0 )
   {
      wortInBoil_l = wortFromMash_l() - e->lauterDeadspace_l();
      str = tr("You should now have %1 wort.")
         .arg(Brewtarget::displayAmount( wortInBoil_l, Units::liters));
      if ( e->topUpKettle_l() != 0 )
      {
         wortInBoil_l += e->topUpKettle_l();
         tmp = tr(" Add %1 water to the kettle, bringing pre-boil volume to %2.")
            .arg(Brewtarget::displayAmount(e->topUpKettle_l(), Units::liters))
            .arg(Brewtarget::displayAmount(wortInBoil_l, Units::liters));

         str += tmp;

         ins = Database::instance().newInstruction(this);
         ins->setName(tr("Pre-boil"));
         ins->setDirections(str);
         ins->addReagent(tmp);
         return ins;
      }
   }
   return 0;
}

bool Recipe::hasBoilFermentable()
{
   unsigned int i;
   for ( i = 0; static_cast<int>(i) < fermentables().size(); ++i )
   {
      Fermentable* ferm = fermentables()[i];
      if( ferm->isMashed() || ferm->addAfterBoil() )
         continue;
      else
         return true;
   }
   return false;
}

PreInstruction Recipe::boilFermentablesPre(double timeRemaining)
{
   bool hasFerms = false;
   QString str;
   unsigned int i;
   int size;

   str = tr("Boil or steep ");
   QList<Fermentable*> flist = fermentables();
   size = flist.size();
   for( i = 0; static_cast<int>(i) < size; ++i )
   {
     Fermentable* ferm = flist[i];
     if( ferm->isMashed() || ferm->addAfterBoil() )
       continue;

     hasFerms = true;
     str += QString("%1 %2, ")
          .arg(Brewtarget::displayAmount(ferm->amount_kg(), Units::kilograms))
          .arg(ferm->name());
   }
   str += ".";

   return PreInstruction(str, tr("Boil/steep fermentables"), timeRemaining);
}

Instruction* Recipe::postboilFermentablesIns()
{
   Instruction* ins;
   QString str,tmp;
   unsigned int i;
   int size;
   bool hasFerms = false;

   str = tr("Add ");
   QList<Fermentable*> flist;
   size = flist.size();
   for( i = 0; static_cast<int>(i) < size; ++i )
   {
      Fermentable* ferm = flist[i];
      if( ! ferm->addAfterBoil() )
         continue;

      hasFerms = true;
      tmp = QString("%1 %2, ")
             .arg(Brewtarget::displayAmount(ferm->amount_kg(), Units::kilograms))
             .arg(ferm->name());
      str += tmp;
   }
   str += tr("to the boil at knockout.");

   if( hasFerms )
   {
      ins = Database::instance().newInstruction(this);
      ins->setName(tr("Knockout additions"));
      ins->setDirections(str);
      ins->addReagent(tmp);
      return ins;
   }
   else
   {
      return 0;
   }
}

Instruction* Recipe::postboilIns()
{
   QString str;
   Instruction* ins;
   double wort_l = 0.0;
   double wortInBoil_l = 0.0;

   Equipment* e = equipment();
   if( e != 0 )
   {
      wortInBoil_l = wortFromMash_l() - e->lauterDeadspace_l();
      if ( e->topUpKettle_l() != 0 )
         wortInBoil_l += e->topUpKettle_l();

      wort_l = e->wortEndOfBoil_l(wortInBoil_l);
      str = tr("You should have %1 wort post-boil.")
            .arg(Brewtarget::displayAmount( wort_l, Units::liters));
      str += tr("\nYou anticipate losing %1 to trub and chiller loss.")
            .arg(Brewtarget::displayAmount( e->trubChillerLoss_l(), Units::liters));
      wort_l -= e->trubChillerLoss_l();
      if( e->topUpWater_l() > 0.0 )
          str += tr("\nAdd %1 top up water into primary.")
               .arg(Brewtarget::displayAmount( e->topUpWater_l(), Units::liters));
      wort_l += e->topUpWater_l();
      str += tr("\nThe final volume in the primary is %1.")
             .arg(Brewtarget::displayAmount(wort_l, Units::liters));

      ins = Database::instance().newInstruction(this);
      ins->setName(tr("Post boil"));
      ins->setDirections(str);
      return ins;
   }
   else
   {
      return 0;
   }
}

void Recipe::addPreinstructions( QVector<PreInstruction> preins )
{
   unsigned int i;
   Instruction* ins;

    // Add instructions in descending mash time order.
    qSort( preins.begin(), preins.end(), qGreater<PreInstruction>() );
    for( i=0; static_cast<int>(i) < preins.size(); ++i )
    {
       PreInstruction pi = preins[i];
       ins = Database::instance().newInstruction(this);
       ins->setName(pi.getTitle());
       ins->setDirections(pi.getText());
       ins->setInterval(pi.getTime());
    }
}

void Recipe::generateInstructions()
{
   Instruction* ins;
   QString str, tmp;
   unsigned int i, size;
   double timeRemaining;
   double totalWaterAdded_l = 0.0;

   QVector<PreInstruction> preinstructions;

   // Mash instructions

   size = (mash() == 0) ? 0 : mash()->mashSteps().size();
   if( size > 0 )
   {
     /*** prepare mashed fermentables ***/
     mashFermentableIns();

     /*** Prepare water additions ***/
     mashWaterIns(size);

     timeRemaining = mash()->totalTime();

     /*** Generate the mash instructions ***/
     preinstructions = mashInstructions(timeRemaining, totalWaterAdded_l, size);

      /*** Hops mash additions ***/
     preinstructions += hopSteps(Hop::Mash);

      /*** Misc mash additions ***/
     preinstructions += miscSteps(Misc::Mash);

     /*** Add the preinstructions into the instructions ***/
     addPreinstructions(preinstructions);

   } // END mash instructions.

   // First wort hopping
   firstWortHopsIns();

   // Need to top up the kettle before boil?
   topOffIns();

   // Boil instructions
   preinstructions.clear();   
   
   // Find boil time.
   if( equipment() != 0 )
      timeRemaining = equipment()->boilTime_min();
   else
   {
      timeRemaining = Brewtarget::timeQStringToSI(QInputDialog::getText(0,
                                        tr("Boil time"),
                                        tr("You did not configure an equipment (which you really should), so tell me the boil time.")));
   }
   
   str = tr("Bring the wort to a boil and hold for %1.").arg(Brewtarget::displayAmount( timeRemaining, Units::minutes));
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Start boil"));
   ins->setInterval(timeRemaining);
   ins->setDirections(str);
   
   /*** Get fermentables we haven't added yet ***/
   if ( hasBoilFermentable() )
      preinstructions.push_back(boilFermentablesPre(timeRemaining));
   
   /*** Boiled hops ***/
   preinstructions += hopSteps(Hop::Boil);

   /*** Boiled miscs ***/
   preinstructions += miscSteps(Misc::Boil);

   // END boil instructions.

   // Add instructions in descending mash time order.
   addPreinstructions(preinstructions);

   // FLAMEOUT
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Flameout"));
   ins->setDirections(tr("Stop boiling the wort."));

   // Steeped aroma hops
   preinstructions.clear();
   preinstructions += hopSteps(Hop::UseAroma);
   addPreinstructions(preinstructions);
   
   // Fermentation instructions
   preinstructions.clear();

   /*** Fermentables added after boil ***/
   postboilFermentablesIns();

   /*** post boil ***/
   postboilIns();
   
   /*** Primary yeast ***/
   str = tr("Cool wort and pitch ");
   QList<Yeast*> ylist = yeasts();
   for( i = 0; static_cast<int>(i) < ylist.size(); ++i )
   {
      Yeast* yeast = ylist[i];
      if( ! yeast->addToSecondary() )
         str += tr("%1 %2 yeast, ").arg(yeast->name()).arg(yeast->typeStringTr());
   }
   str += tr("to the primary.");
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Pitch yeast"));
   ins->setDirections(str);
   /*** End primary yeast ***/

   /*** Primary misc ***/
   addPreinstructions(miscSteps(Misc::Primary));

   str = tr("Let ferment until FG is %1.")
         .arg(Brewtarget::displayAmount(fg()));
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Ferment"));
   ins->setDirections(str);

   str = tr("Transfer beer to secondary.");
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Transfer to secondary"));
   ins->setDirections(str);

   /*** Secondary misc ***/
   addPreinstructions(miscSteps(Misc::Secondary));

   /*** Dry hopping ***/
   addPreinstructions(hopSteps(Hop::Dry_Hop));

   // END fermentation instructions
   //emit changed(metaObject()->property(metaObject->indexOfProperty("instructions")), instructions());
}

QString Recipe::nextAddToBoil(double& time)
{
   int i, size;
   double max = 0;
   bool foundSomething = false;
   Hop* h;
   QList<Hop*> hhops = hops();
   Misc* m;
   QList<Misc*> mmiscs = miscs();
   QString ret;

   // Search hops
   size = hhops.size();
   for( i = 0; i < size; ++i )
   {
      h = hhops[i];
      if( h->use() != Hop::Boil )
         continue;
      if( h->time_min() < time && h->time_min() > max )
      {
         ret = tr("Add %1 %2 to boil at %3.")
               .arg(Brewtarget::displayAmount(h->amount_kg(), Units::kilograms))
               .arg(h->name())
               .arg(Brewtarget::displayAmount(h->time_min(), Units::minutes));

         max = h->time_min();
         foundSomething = true;
      }
   }

   // Search miscs
   size = mmiscs.size();
   for( i = 0; i < size; ++i )
   {
      m = mmiscs[i];
      if( m->use() != Misc::Boil )
         continue;
      if( m->time() < time && m->time() > max )
      {
         ret = tr("Add %1 %2 to boil at %3.");
         if( m->amountIsWeight() )
            ret = ret.arg(Brewtarget::displayAmount(m->amount(), Units::kilograms));
         else
            ret = ret.arg(Brewtarget::displayAmount(m->amount(), Units::liters));

         ret = ret.arg(m->name());
         ret = ret.arg(Brewtarget::displayAmount(m->time(), Units::minutes));
         max = m->time();
         foundSomething = true;
      }
   }
   
   time = foundSomething ? max : -1.0;
   return ret;
}

//============================Relational Setters===============================

void Recipe::addHop( Hop *var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addFermentable( Fermentable* var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addMisc( Misc* var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addYeast( Yeast* var )
{
   Database::instance().addToRecipe( this, var );
}

void Recipe::addWater( Water* var )
{
   Database::instance().addToRecipe( this, var );
}

//==============================="SET" METHODS=================================
void Recipe::setName( const QString &var )
{
   set( "name", "name", var );
}

void Recipe::setType( const QString &var )
{
   QString tmp;
   if( ! isValidType(var) )
   {
      Brewtarget::logW( QString("Recipe: invalid type: %1").arg(var) );
      tmp = "All Grain";
   }
   else
   {
      tmp = QString(var);
   }

   set( "type", "type", tmp );
}

void Recipe::setBrewer( const QString &var )
{
   set( "brewer", "brewer", var );
}

void Recipe::setBatchSize_l( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: batch size < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "batchSize_l", "batch_size", tmp );
}

void Recipe::setBoilSize_l( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: boil size < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "boilSize_l", "boil_size", tmp );
}

void Recipe::setBoilTime_min( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: boil time < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "boilTime_min", "boil_time", tmp );
}

void Recipe::setEfficiency_pct( double var )
{
   double tmp;
   if( var < 0.0  || var > 100.0 )
   {
      Brewtarget::logW( QString("Recipe: 0 < efficiency < 100: %1").arg(var) );
      tmp = 70;
   }
   else
   {
      tmp = var;
   }

   set( "efficiency_pct", "efficiency", tmp );
}

void Recipe::setAsstBrewer( const QString &var )
{
   set( "asstBrewer", "assistant_brewer", var );
}

void Recipe::setNotes( const QString &var )
{
   set( "notes", "notes", var );
}

void Recipe::setTasteNotes( const QString &var )
{
   set( "tasteNotes", "taste_notes", var );
}

void Recipe::setTasteRating( double var )
{
   double tmp;
   if( var < 0.0 || var > 50.0 )
   {
      Brewtarget::logW( QString("Recipe: 0 < taste rating < 50: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "tasteRating", "taste_rating", var );
}

void Recipe::setOg( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: og < 0: %1").arg(var) );
      tmp = 1.0;
   }
   else
   {
      tmp = var;
   }

   set( "og", "og", tmp );
}

void Recipe::setFg( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: fg < 0: %1").arg(var) );
      tmp = 1.0;
   }
   else
   {
      tmp = var;
   }

   set( "fg", "fg", tmp );
}

void Recipe::setFermentationStages( int var )
{
   int tmp;
   if( var < 0 )
   {
      Brewtarget::logW( QString("Recipe: stages < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "fermentationStages", "fermentation_stages", tmp );
}

void Recipe::setPrimaryAge_days( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: primary age < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "primaryAge_days", "primary_age", tmp );
}

void Recipe::setPrimaryTemp_c( double var )
{
   set( "primaryTemp_c", "primary_temp", var );
}

void Recipe::setSecondaryAge_days( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: secondary age < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "secondaryAge_days", "secondary_age", tmp );
}

void Recipe::setSecondaryTemp_c( double var )
{
   set( "secondaryTemp_c", "secondary_temp", var );
}

void Recipe::setTertiaryAge_days( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: tertiary age < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "tertiaryAge_days", "tertiary_age", tmp );
}

void Recipe::setTertiaryTemp_c( double var )
{
   set( "tertiaryTemp_c", "tertiary_temp", var );
}

void Recipe::setAge_days( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: age < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "age", "age", tmp );
}

void Recipe::setAgeTemp_c( double var )
{
   set( "ageTemp_c", "age_temp", var );
}

void Recipe::setDate( const QDate &var )
{
   set( "date", "date", var.toString("d/M/yyyy") );
}

void Recipe::setCarbonation_vols( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: carb < 0: %1").arg(var) );
      tmp = 0;
   }
   else
   {
      tmp = var;
   }

   set( "carbonation_vols", "carb_volume", var );
}

void Recipe::setForcedCarbonation( bool var )
{
   set( "forcedCarbonation", "forced_carb", var );
}

void Recipe::setPrimingSugarName( const QString &var )
{
   set( "primingSugarName", "priming_sugar_name", var );
}

void Recipe::setCarbonationTemp_c( double var )
{
   set( "carbonationTemp_c", "carbonationTemp_c", var );
}

void Recipe::setPrimingSugarEquiv( double var )
{
   double tmp;
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: primingsugarequiv < 0: %1").arg(var) );
      tmp = 1;
   }
   else
   {
      tmp = var;
   }

   set( "primingSugarEquiv", "priming_sugar_equiv", tmp );
}

void Recipe::setKegPrimingFactor( double var )
{
   double tmp;
   
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Recipe: keg priming factor < 0: %1").arg(var) );
      tmp = 1;
   }
   else
   {
      tmp = var;
   }

   set( "kegPrimingFactor", "keg_priming_factor", tmp );
}

//==========================Calculated Getters============================

double Recipe::og()
{
   /*
   if( _uninitializedCalcsMutex.tryLock() && _uninitializedCalcs )
   {
      _uninitializedCalcsMutex.unlock();
      recalcAll();
   }
   */
   if( _uninitializedCalcs )
      recalcAll();
   return _og;
}

double Recipe::fg()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _fg;
}

double Recipe::color_srm()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _color_srm;
}

double Recipe::ABV_pct()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _ABV_pct;
}

double Recipe::IBU()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _IBU;
}

QList<double> Recipe::IBUs()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _ibus;
}

double Recipe::boilGrav()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _boilGrav;
}

double Recipe::calories()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _calories;
}

double Recipe::wortFromMash_l()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _wortFromMash_l;
}

double Recipe::boilVolume_l()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _boilVolume_l;
}

double Recipe::postBoilVolume_l()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _postBoilVolume_l;
}

double Recipe::finalVolume_l()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _finalVolume_l;
}

QColor Recipe::SRMColor()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _SRMColor;
}

double Recipe::grainsInMash_kg()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _grainsInMash_kg;
}

double Recipe::grains_kg()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _grains_kg;
}

double Recipe::points()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _points;
}

//=========================Relational Getters=============================

Style* Recipe::style() const
{
   QVariant key = get("style_id");
   if( key.isValid() )
      return Database::instance().style( key.toInt() );
   else
      return 0;
}

Mash* Recipe::mash() const
{
   QVariant key = get("mash_id");
   if( key.isValid() )
      return Database::instance().mash( key.toInt() );
   else
      return 0;
}

Equipment* Recipe::equipment() const
{
   QVariant key = get("equipment_id");
   if( key.isValid() )
      return Database::instance().equipment( key.toInt() );
   else
      return 0;
}

QList<Instruction*> Recipe::instructions() const
{
   return Database::instance().instructions(this);
}

QList<BrewNote*> Recipe::brewNotes() const
{
   return Database::instance().brewNotes(this);
}

QList<Hop*> Recipe::hops() const
{
   return Database::instance().hops(this);
}

QList<Fermentable*> Recipe::fermentables() const
{
   return Database::instance().fermentables(this);
}

QList<Misc*> Recipe::miscs() const
{
   return Database::instance().miscs(this);
}

QList<Yeast*> Recipe::yeasts() const
{
   return Database::instance().yeasts(this);
}

QList<Water*> Recipe::waters() const
{
   return Database::instance().waters(this);
}

//==============================Getters===================================
QString Recipe::name() const
{
   return get("name").toString();
}

QString Recipe::type() const
{
   return get("type").toString();
}

QString Recipe::brewer() const
{
   return get("brewer").toString();
}

double Recipe::batchSize_l() const
{
   return get("batch_size").toDouble();
}

double Recipe::boilSize_l() const
{
   return get("boil_size").toDouble();
}

double Recipe::boilTime_min() const
{
   return get("boil_time").toDouble();
}

double Recipe::efficiency_pct() const
{
   return get("efficiency").toDouble();
}

QString Recipe::asstBrewer() const
{
   return get("assistant_brewer").toString();
}

QString Recipe::notes() const
{
   return get("notes").toString();
}

QString Recipe::tasteNotes() const
{
   return get("taste_notes").toString();
}

double Recipe::tasteRating() const
{
   return get("taste_rating").toDouble();
}

int Recipe::fermentationStages() const
{
   return get("fermentation_stages").toInt();
}

double Recipe::primaryAge_days() const
{
   return get("primary_age").toDouble();
}

double Recipe::primaryTemp_c() const
{
   return get("primary_temp").toDouble();
}

double Recipe::secondaryAge_days() const
{
   return get("secondary_age").toDouble();
}

double Recipe::secondaryTemp_c() const
{
   return get("secondary_temp").toDouble();
}

double Recipe::tertiaryAge_days() const
{
   return get("tertiary_age").toDouble();
}

double Recipe::tertiaryTemp_c() const
{
   return get("tertiary_temp").toDouble();
}

double Recipe::age_days() const
{
   return get("age").toDouble();
}

double Recipe::ageTemp_c() const
{
   return get("age_temp").toDouble();
}

QDate Recipe::date() const
{
   return QDate::fromString( get("date").toString(), "d/M/yyyy");
}

double Recipe::carbonation_vols() const
{
   return get("carb_volume").toDouble();
}

bool Recipe::forcedCarbonation() const
{
   return get("forced_carb").toBool();
}

QString Recipe::primingSugarName() const
{
   return get("priming_sugar_name").toString();
}

double Recipe::carbonationTemp_c() const
{
   return get("carbonationTemp_c").toDouble();
}

double Recipe::primingSugarEquiv() const
{
   return get("priming_sugar_equiv").toDouble();
}

double Recipe::kegPrimingFactor() const
{
   return get("keg_priming_factor").toDouble();
}

//=============================Removers========================================

// Returns true if var is found and removed.
void Recipe::removeHop( Hop *var )
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeFermentable(Fermentable* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeMisc(Misc* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeWater(Water* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeYeast(Yeast* var)
{
   Database::instance().removeFromRecipe( this, var );
}

void Recipe::removeBrewNote(BrewNote* var)
{
   Database::instance().removeFromRecipe( this, var );
}

//==============================Recalculators==================================

void Recipe::recalcAll()
{
   // WARNING
   // Infinite recursion possible, since these methods will emit changed(),
   // causing other objects to call finalVolume_l() for example, which may
   // cause another call to recalcAll() and so on.

   // Someone has already called this function back in the call stack, so return to avoid recursion.
   if( !_recalcMutex.tryLock() )
      return;
   /*
   if( !_uninitializedCalcsMutex.tryLock() )
   {
      _recalcMutex.unlock();
      return;
   }
   */
   
   recalcGrainsInMash_kg();
   recalcGrains_kg();
   recalcVolumeEstimates();
   recalcColor_srm();
   recalcSRMColor();
   recalcOgFg();
   recalcABV_pct();
   recalcBoilGrav();
   recalcIBU();
   
   _uninitializedCalcs = false;
   
   //_uninitializedCalcsMutex.unlock();
   _recalcMutex.unlock();
}

void Recipe::recalcPoints(double volume)
{
   unsigned int i;
   Fermentable* ferm;
   double sugar_kg = 0.0;
   double sugar_kg_ignoreEfficiency = 0.0;
   Fermentable::Type type;

   // Calculate OG
   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];
      if( ferm->addAfterBoil() )
         continue;

      // If we have some sort of non-grain, we have to ignore efficiency.
      type = ferm->type();
      if( type==Fermentable::Sugar|| type==Fermentable::Extract || type==Fermentable::Dry_Extract )
         sugar_kg_ignoreEfficiency += (ferm->yield_pct()/100.0)*ferm->amount_kg();
      else
         sugar_kg += (ferm->yield_pct()/100.0)*ferm->amount_kg();
   }

   _points = 1000 * (  Algorithms::Instance().PlatoToSG_20C20C( Algorithms::Instance().getPlato(sugar_kg,volume)) - 1);
   
   emit changed( metaProperty("points"), _points );
}

void Recipe::recalcABV_pct()
{
    // Alcohol by weight.  This is a different formula than used
    // when calculating the calories.
    //abw = 76.08 * (og-fg)/(1.775-og);
    //return abw * (fg/0.794);

    // George Fix: Brewing Science and Practice, page 686.
    // The multiplicative factor actually varies from
    // 125 for weak beers to 135 for strong beers.
    _ABV_pct = 130*(_og-_fg);

    // From http://en.wikipedia.org/w/index.php?title=Alcohol_by_volume&oldid=414661414
    // Has no citations, so I don't know how trustworthy it is.
    // It seems to be in conflict with Fix's method above, because
    // if the beer is weak, it should have a low fg, meaning the
    // multiplicative factor is higher.
    // return 132.9*(og - fg)/fg;
    
    emit changed( metaProperty("ABV_pct"), _ABV_pct );
}

void Recipe::recalcColor_srm()
{
   Fermentable *ferm;
   double mcu = 0.0;
   unsigned int i;

   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];
      // Conversion factor for lb/gal to kg/l = 8.34538.
      mcu += ferm->color_srm()*8.34538 * ferm->amount_kg()/_finalVolume_l;
   }

   _color_srm = ColorMethods::mcuToSrm(mcu);
   
   emit changed( metaProperty("color_srm"), _color_srm );
}

void Recipe::recalcBoilGrav()
{
   unsigned int i;
   Fermentable* ferm;
   double sugar_kg = 0.0;
   double sugar_kg_ignoreEfficiency = 0.0;
   Fermentable::Type type;

   // Calculate OG
   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];
      if( ferm->addAfterBoil() )
         continue;

      // If we have some sort of non-grain, we have to ignore efficiency.
      type = ferm->type();
      if( type==Fermentable::Sugar || type==Fermentable::Extract || type==Fermentable::Dry_Extract )
         sugar_kg_ignoreEfficiency += ferm->equivSucrose_kg();
      else
         sugar_kg += ferm->equivSucrose_kg();
   }

   // We might lose some sugar in the form of lauter deadspace.
   /*** Forget lauter deadspace...this loss is included in efficiency ***/

   // Since the efficiency refers to how much sugar we get into the fermenter,
   // we need to adjust for that here.
   sugar_kg = (efficiency_pct()/100.0 * sugar_kg + sugar_kg_ignoreEfficiency);
   if( equipment() )
      sugar_kg = sugar_kg / (1 - equipment()->trubChillerLoss_l()/_postBoilVolume_l);

   _boilGrav = Algorithms::Instance().PlatoToSG_20C20C( Algorithms::Instance().getPlato(sugar_kg, _boilVolume_l) );
   
   emit changed( metaProperty("boilGrav"), _boilGrav );
}

void Recipe::recalcIBU()
{
   unsigned int i;
   double ibus = 0.0;
   double tmp;
   
   // Bitterness due to hops...
   _ibus.clear();
   QList<Hop*> hhops = hops();
   for( i = 0; static_cast<int>(i) < hhops.size(); ++i )
   {
      tmp = ibuFromHop(hhops[i]);
      _ibus.append(tmp);
      ibus += tmp;
   }

   // Bitterness due to hopped extracts...
   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      // Conversion factor for lb/gal to kg/l = 8.34538.
      ibus +=
              ferms[i]->ibuGalPerLb() *
              (ferms[i]->amount_kg() / batchSize_l()) / 8.34538;
   }

   _IBU = ibus;
   
   emit changed( metaProperty("IBU"), _IBU );
}

void Recipe::recalcVolumeEstimates()
{
   // wortFromMash_l ==========================
   double waterAdded_l;
   double absorption_lKg;
   
   if( mash() == 0 )
      _wortFromMash_l = 0.0;
   else
   {
   
       waterAdded_l = mash()->totalMashWater_l();
       if( equipment() != 0 )
          absorption_lKg = equipment()->grainAbsorption_LKg();
       else
          absorption_lKg = HeatCalculations::absorption_LKg;

       _wortFromMash_l = (waterAdded_l - absorption_lKg * _grainsInMash_kg);
   }
   
   // boilVolume_l ==============================
   //double mashVol_l;
   double tmp = 0.0;
   
   //if( mashVol_l <= 0.0 ) // Give up.
   //   return boilSize_l;
   
   if( equipment() != 0 )
      tmp = _wortFromMash_l - equipment()->lauterDeadspace_l() + equipment()->topUpKettle_l();
   else
      tmp = _wortFromMash_l;
   
   if( tmp <= 0.0 )
      tmp = boilSize_l(); // Give up.
   
   _boilVolume_l = tmp;
   
   // finalVolume_l ==============================
   
   if( equipment() != 0 )
      _finalVolume_l = equipment()->wortEndOfBoil_l(_boilVolume_l) - equipment()->trubChillerLoss_l() + equipment()->topUpWater_l();
   else
      _finalVolume_l = _boilVolume_l - 4.0; // This is just shooting in the dark. Can't do much without an equipment.
   
   // postBoilVolume_l ===========================

   if( equipment() != 0 )
      _postBoilVolume_l = equipment()->wortEndOfBoil_l( _boilVolume_l );
   else
      _postBoilVolume_l = batchSize_l(); // Give up.
      
   // Emit changes.
   emit changed( metaProperty("wortFromMash_l"), _wortFromMash_l );
   emit changed( metaProperty("boilVolume_l"), _boilVolume_l );
   emit changed( metaProperty("finalVolume_l"), _finalVolume_l );
   emit changed( metaProperty("postBoilVolume_l"), _postBoilVolume_l );
}

void Recipe::recalcGrainsInMash_kg()
{
   unsigned int i, size;
   double ret = 0.0;
   Fermentable* ferm;
   
   QList<Fermentable*> ferms = fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
   {
      ferm = ferms[i];
      
      if( ferm->type() == Fermentable::Grain && ferm->isMashed() )
         ret += ferm->amount_kg();
   }
   
   _grainsInMash_kg = ret;
   
   emit changed( metaProperty("grainsInMash_kg"), _grainsInMash_kg );
}

void Recipe::recalcGrains_kg()
{
   unsigned int i, size;
   double ret = 0.0;

   QList<Fermentable*> ferms = fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
      ret += ferms[i]->amount_kg();

   _grains_kg = ret;
   
   emit changed( metaProperty("grains_kg"), _grains_kg );
}

void Recipe::recalcSRMColor()
{
   /**** Original method from a website: Came out dark. ***
   
   // Luminance Y
   double Y = 94.6914*exp(-0.131272*color_srm);
   // Chroma x
   double x = 0.73978 - 0.25442*exp(-0.037865*color_srm) - 0.017511*exp(-0.24307*color_srm);
   // Chroma y
   double y = 0.197785 + 0.260472*exp( -pow( (x-0.491021)/.214194, 2)   );
   // Chroma z
   double z = 1 - x - y;

   double X = (Y/y)*x;
   double Z = (Y/y)*z;

   // Get [0,255] RGB values.
   int R = (int)ceil(1.910*X  - 0.533*Y - 0.288*Z);
   R = (R<0)?0:((R>255)?255:R);

   int G = (int)ceil(-0.985*X + 2.000*Y - 0.0280*Z);
   G = (G<0)?0:((G>255)?255:G);

   int B = (int)ceil(0.058*X -0.118*Y + 0.896*Z);
   B = (B<0)?0:((B>255)?255:B);

   QColor ret;

   ret.setRgb( R, G, B, 255 );

   return ret;
   ***********/

   //==========My approximation from a photo and spreadsheet===========

   double red = 232.9 * pow( (double)0.93, _color_srm );
   double green = (double)-106.25 * log(_color_srm) + 280.9;

   int r = (red < 0)? 0 : ((red > 255)? 255 : (int)Algorithms::Instance().round(red));
   int g = (green < 0)? 0 : ((green > 255)? 255 : (int)Algorithms::Instance().round(green));
   int b = 0;

   _SRMColor.setRgb( r, g, b );

   emit changed( metaProperty("SRMColor"), _SRMColor );
}

// the formula in here are taken from http://hbd.org/ensmingr/
void Recipe::recalcCalories()
{
    double startPlato, finishPlato, RE, abw, oog, ffg;

    oog = _og;
    ffg = _fg;

    // Need to translate OG and FG into plato
    startPlato  = -463.37 + ( 668.72 * oog ) - (205.35 * oog * oog);
    finishPlato = -463.37 + ( 668.72 * ffg ) - (205.35 * ffg * ffg);

    // RE (real extract)
    RE = (0.1808 * startPlato) + (0.8192 * finishPlato);

    // Alcohol by weight?
    abw = (startPlato-RE)/(2.0665 - (0.010665 * startPlato));

    _calories = ((6.9*abw) + 4.0 * (RE-0.1)) * ffg * 3.55;

    emit changed( metaProperty("calories"), _calories );
}

void Recipe::recalcOgFg()
{
   unsigned int i;
   double kettleWort_l;
   double postBoilWort_l;
   double plato;
   double ratio = 0;
   double sugar_kg = 0;
   double sugar_kg_ignoreEfficiency = 0.0;
   Fermentable::Type fermtype;
   double attenuation_pct = 0.0;
   Fermentable* ferm;
   Yeast* yeast;
   
   QList<Fermentable*> ferms = fermentables();
   
   _points = 0;
   // Calculate OG
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];

      // If we have some sort of non-grain, we have to ignore efficiency.
      fermtype = ferm->type();
      if( fermtype==Fermentable::Sugar|| fermtype==Fermentable::Extract || fermtype==Fermentable::Dry_Extract )
         sugar_kg_ignoreEfficiency += ferm->equivSucrose_kg();
      else
         sugar_kg += ferm->equivSucrose_kg();
   }

   // We might lose some sugar in the form of Trub/Chiller loss and lauter deadspace.
   if( equipment() != 0 )
   {
      /* Ignore lauter deadspace since it should be included in efficiency ***
      // First, lauter deadspace.
      ratio = (estimateWortFromMash_l() - equipment->getLauterDeadspace_l()) / (estimateWortFromMash_l());
      if( ratio > 1.0 ) // Usually happens when we don't have a mash yet.
         ratio = 1.0;
      else if( ratio < 0.0 ) // Only happens if the user is stupid with lauter deadspace.
         ratio = 0.0;
      else if( isnan(ratio) )
         ratio = 1.0; // Need this in case we have no mash, and therefore end up with NaN.
      
      sugar_kg *= ratio;
      // Don't consider this one since nobody adds sugar or extract to the mash.
      //sugar_kg_ignoreEfficiency *= ratio;
      */
      
      // Next, trub/chiller loss.
      kettleWort_l = (_wortFromMash_l - equipment()->lauterDeadspace_l()) + equipment()->topUpKettle_l();
      postBoilWort_l = equipment()->wortEndOfBoil_l(kettleWort_l);
      ratio = (postBoilWort_l - equipment()->trubChillerLoss_l()) / postBoilWort_l;
      if( ratio > 1.0 ) // Usually happens when we don't have a mash yet.
         ratio = 1.0;
      else if( ratio < 0.0 )
         ratio = 0.0;
      else if( Algorithms::Instance().isnan(ratio) )
         ratio = 1.0;
      // Ignore this again since it should be included in efficiency.
      //sugar_kg *= ratio;
      sugar_kg_ignoreEfficiency *= ratio;
   }

   // Combine the two sugars.
   sugar_kg = sugar_kg * efficiency_pct()/100.0 + sugar_kg_ignoreEfficiency;
   plato = Algorithms::Instance().getPlato( sugar_kg, _finalVolume_l);

   _og = Algorithms::Instance().PlatoToSG_20C20C( plato );
   _points = (_og-1)*1000.0;

   // Calculage FG
   for( i = 0; static_cast<int>(i) < yeasts().size(); ++i )
   {
      yeast = yeasts()[i];
      // Get the yeast with the greatest attenuation.
      if( yeast->attenuation_pct() > attenuation_pct )
         attenuation_pct = yeast->attenuation_pct();
   }
   if( yeasts().size() > 0 && attenuation_pct <= 0.0 ) // This means we have yeast, but they neglected to provide attenuation percentages.
      attenuation_pct = 75.0; // 75% is an average attenuation.

   _points = _points*(1.0 - attenuation_pct/100.0);
   _fg =  1 + _points/1000.0;
   
   emit changed( metaProperty("og"), _og );
   emit changed( metaProperty("fg"), _fg );
}

//====================================Helpers===========================================

double Recipe::ibuFromHop(Hop const* hop)
{
   double ibus = 0.0;
   
   if( hop == 0 )
      return 0.0;
   
   double AArating = hop->alpha_pct()/100.0;
   double grams = hop->amount_kg()*1000.0;
   double minutes = hop->time_min();
   //double water_l = estimateFinalVolume_l();
   //double boilVol_l = estimateBoilVolume_l();
   //double boilGrav = boilGrav();
   double boilGrav_final = _boilGrav; 
   double avgBoilGrav;
   
   if( equipment() )
      boilGrav_final = _boilVolume_l / equipment()->wortEndOfBoil_l( _boilVolume_l ) * (_boilGrav-1) + 1;
   
   avgBoilGrav = (_boilGrav + boilGrav_final) / 2;
   
   if( hop->use() == Hop::Boil)
      ibus = IbuMethods::getIbus( AArating, grams, _finalVolume_l, avgBoilGrav, minutes );
   else if( hop->use() == Hop::First_Wort )
      ibus = 1.10 * IbuMethods::getIbus( AArating, grams, _finalVolume_l, avgBoilGrav, 20 ); // I am estimating First wort hops give 10% more ibus than a 20 minute addition.

   // Adjust for hop form.
   if( hop->form() == Hop::Leaf )
      ibus *= 0.90;
   else if( hop->form() == Hop::Plug )
      ibus *= 0.92;
   
   return ibus;
}

bool Recipe::isValidType( const QString &str )
{
   static const QString types[] = {"Extract", "Partial Mash", "All Grain"};
   static const unsigned int size = 3;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( str == types[i] )
         return true;
   
   return false;
}

//==========================Accept changes from ingredients====================

/*
void Recipe::acceptChanges(QMetaProperty prop, QVariant val)
{
   QObject* senderObj = sender();
   // Sender can be null if the sender is from another thread.
   if( senderObj == 0 )
      return;
   
   QString senderClass(senderObj->metaObject()->className());
   
   // Pass along the signal if it's one of our ingredients?
   // I don't know really what to emit here...
   //if( senderClass == "Hop" )
   //   emit changed(...);
   //else if( senderClass == "Fermentable" )
   //   emit changed(...);
   //else if( senderClass == "Misc" )
   //   emit changed(...);
   //else if( senderClass == "Yeast" )
   //   emit changed(...);
   //else if( senderClass == "Water" )
   //   emit changed(...);
   //else if( senderClass == "BrewNote" )
   //   emit changed(...);
   //else if( senderClass == "Instruction" )
   //   emit changed(...);
   
}
*/

void Recipe::acceptFermChange(QMetaProperty /*prop*/, QVariant /*val*/)
{
   recalcAll();
}

void Recipe::acceptHopChange(QMetaProperty prop, QVariant val)
{
   recalcIBU();
}

void Recipe::acceptMashChange(QMetaProperty /*prop*/, QVariant /*val*/)
{
   recalcAll();
}
