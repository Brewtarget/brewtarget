/*
 * recipe.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Kregg K <gigatropolis@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
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

#include "instruction.h"
#include "brewtarget.h"
#include "database.h"

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
#include "PhysicalConstants.h"
#include "QueuedMethod.h"



/************* Columns *************/
const QString kName("name");
const QString kType("type");
const QString kBrewer("brewer");
const QString kBatchSize("batch_size");
const QString kBoilSize("boil_size");
const QString kBoilTime("boil_time");
const QString kEfficiency("efficiency");
const QString kAsstBrewer("assistant_brewer");
const QString kNotes("notes");
const QString kTasteNotes("taste_notes");
const QString kTasteRating("taste_rating");
const QString kOG("og");
const QString kFG("fg");
const QString kFermentationStages("fermentation_stages");
const QString kPrimaryAgeDays("primary_age");
const QString kPrimaryTemp("primary_temp");
const QString kSecondaryAgeDays("secondary_age");
const QString kSecondaryTemp("secondary_temp");
const QString kTertiaryAgeDays("tertiary_age");
const QString kTertiaryTemp("tertiary_temp");
const QString kAge("age");
const QString kAgeTemp("age_temp");
const QString kDate("date");
const QString kCarbonationVols("carb_volume");
const QString kForcedCarbonation("forced_carb");
const QString kPrimingSugarName("priming_sugar_name");
const QString kCarbonationTemp("carbonationTemp_c");
const QString kPrimingSugarEquiv("priming_sugar_equiv");
const QString kKegPrimingFactor("keg_priming_factor");


/************** Props **************/
const QString kNameProp("name");
const QString kTypeProp("type");
const QString kBrewerProp("brewer");
const QString kBatchSizeProp("batchSize_l");
const QString kBoilSizeProp("boilSize_l");
const QString kBoilTimeProp("boilTime_min");
const QString kEfficiencyProp("efficiency_pct");
const QString kAsstBrewerProp("asstBrewer");
const QString kNotesProp("notes");
const QString kTasteNotesProp("tasteNotes");
const QString kTasteRatingProp("tasteRating");
const QString kOGProp("og");
const QString kFGProp("fg");
const QString kFermentationStagesProp("fermentationStages");
const QString kPrimaryAgeDaysProp("primaryAge_days");
const QString kPrimaryTempProp("primaryTemp_c");
const QString kSecondaryAgeDaysProp("secondaryAge_days");
const QString kSecondaryTempProp("secondaryTemp_c");
const QString kTertiaryAgeDaysProp("tertiaryAge_days");
const QString kTertiaryTempProp("tertiaryTemp_c");
const QString kAgeProp("age");
const QString kAgeTempProp("ageTemp_c");
const QString kDateProp("date");
const QString kCarbonationVolsProp("carbonation_vols");
const QString kForcedCarbonationProp("forcedCarbonation");
const QString kPrimingSugarNameProp("primingSugarName");
const QString kCarbonationTempProp("carbonationTemp_c");
const QString kPrimingSugarEquivProp("primingSugarEquiv");
const QString kKegPrimingFactorProp("kegPrimingFactor");


static const QString kMashStepSection("mashStepTableModel");
static const QString kMiscTableSection("miscTableModel");
static const QString kFermentableTableSection("fermentableTable");
static const QString kHopTableSection("hopTable");
static const QString kTabRecipeSection("tab_recipe");

static const QString kTimeAttr("time");
static const QString kAmountKgAttr("amount_kg");
static const QString kAmountAttr("amount");
static const QString kInfuseAmountAttr("infuseAmount_l");
static const QString kInfuseTempAttr("infuseTemp_c");
static const QString kStepTempAttr("stepTemp_c");
static const QString kDecoctionAmountAttr("decoctionAmount_l");
static const QString kStepTimeAttr("stepTime_min");
static const QString kBoilSizeAttr("boilSize_l");
static const QString kBatchSizeAttr("batchSize_l");


QHash<QString,QString> Recipe::tagToProp = Recipe::tagToPropHash();

QHash<QString,QString> Recipe::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = kNameProp;
   propHash["TYPE"] = kTypeProp;
   propHash["BREWER"] = kBrewerProp;
   propHash["BATCH_SIZE"] = kBatchSizeProp;
   propHash["BOIL_SIZE"] = kBoilSizeProp;
   propHash["BOIL_TIME"] = kBoilTimeProp;
   propHash["EFFICIENCY"] = kEfficiencyProp;
   propHash["ASST_BREWER"] = kAsstBrewerProp;
   propHash["NOTES"] = kNotesProp;
   propHash["TASTE_NOTES"] = kTasteNotesProp;
   propHash["TASTE_RATING"] = kTasteRatingProp;
   propHash["OG"] = kOGProp;
   propHash["FG"] = kFGProp;
   propHash["FERMENTATION_STAGES"] = kFermentationStagesProp;
   propHash["PRIMARY_AGE"] = kPrimaryAgeDaysProp;
   propHash["PRIMARY_TEMP"] = kPrimaryTempProp;
   propHash["SECONDARY_AGE"] = kSecondaryAgeDaysProp;
   propHash["SECONDARY_TEMP"] = kSecondaryTempProp;
   propHash["TERTIARY_AGE"] = kTertiaryAgeDaysProp;
   propHash["TERTIARY_TEMP"] = kTertiaryTempProp;
   propHash["AGE"] = kAgeProp;
   propHash["AGE_TEMP"] = kAgeTempProp;
   propHash["DATE"] = kDateProp;
   propHash["CARBONATION"] = kCarbonationVolsProp;
   propHash["FORCED_CARBONATION"] = kForcedCarbonationProp;
   propHash["PRIMING_SUGAR_NAME"] = kPrimingSugarNameProp;
   propHash["CARBONATION_TEMP"] = kCarbonationTempProp;
   propHash["PRIMING_SUGAR_EQUIV"] = kPrimingSugarEquivProp;
   propHash["KEG_PRIMING_FACTOR"] = kKegPrimingFactorProp;
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

QString Recipe::classNameStr()
{
   static const QString name("Recipe");
   return name;
}

Recipe::Recipe(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key),
     _ABV_pct(0),
     _color_srm(0),
     _boilGrav(1.000),
     _IBU(0),
     _wortFromMash_l(1.0),
     _boilVolume_l(1.0),
     _postBoilVolume_l(1.0),
     _finalVolume_l(1.0),
     _finalVolumeNoLosses_l(1.0),
     _calories(0),
     _grainsInMash_kg(0),
     _grains_kg(0),
     _SRMColor(255,255,0),
     _og(1.000),
     _fg(1.000),
     _uninitializedCalcs(true)
{
   setObjectName("Recipe"); 
}

Recipe::Recipe( Recipe const& other ) : BeerXMLElement(other)
{
   setObjectName("Recipe"); 
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
   int i;

   /*** Add grains ***/
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Add grains"));
   str = tr("Add ");
   QList<QString> reagents = getReagents(fermentables());

   for( i = 0; i < reagents.size(); ++i )
      str += reagents.at(i);

   str += tr("to the mash tun.");
   ins->setDirections(str);

   return ins;

}

Instruction* Recipe::mashWaterIns(unsigned int size)
{
   Instruction* ins;
   QString str, tmp;
   int i;

   if( mash() == 0 )
      return 0;
   
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Heat water"));
   str = tr("Bring ");
   QList<QString> reagents = getReagents(mash()->mashSteps());
   for( i = 0; i < reagents.size(); ++i )
      str += reagents.at(i);

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

      if( mstep->isInfusion() )
      {
         str = tr("Add %1 water at %2 to mash to bring it to %3.")
               .arg(Brewtarget::displayAmount(mstep->infuseAmount_l(), kMashStepSection, kInfuseAmountAttr, Units::liters))
               .arg(Brewtarget::displayAmount(mstep->infuseTemp_c(), kMashStepSection, kInfuseTempAttr, Units::celsius))
               .arg(Brewtarget::displayAmount(mstep->stepTemp_c(), kMashStepSection, kStepTempAttr, Units::celsius));
         totalWaterAdded_l += mstep->infuseAmount_l();
      }
      else if( mstep->isTemperature() )
      {
         str = tr("Heat mash to %1.").arg(Brewtarget::displayAmount(mstep->stepTemp_c(), kMashStepSection, kStepTempAttr, Units::celsius));
      }
      else if( mstep->isDecoction() )
      {
         str = tr("Bring %1 of the mash to a boil and return to the mash tun to bring it to %2.")
               .arg(Brewtarget::displayAmount(mstep->decoctionAmount_l(), kMashStepSection, kDecoctionAmountAttr, Units::liters))
               .arg(Brewtarget::displayAmount(mstep->stepTemp_c(), kMashStepSection, kStepTempAttr, Units::celsius));
      }

      str += tr(" Hold for %1.").arg(Brewtarget::displayAmount(mstep->stepTime_min(), kMashStepSection, kStepTimeAttr, Units::minutes));

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

         str = str.arg(Brewtarget::displayAmount(hop->amount_kg(), kHopTableSection, kAmountKgAttr, Units::kilograms))
                  .arg(hop->name())
                  .arg(Brewtarget::displayAmount(hop->time_min(),kHopTableSection, kTimeAttr,  Units::minutes));

         preins.push_back(PreInstruction(str, tr("Hop addition"), hop->time_min()));
      }
   }
   return preins;
}

QVector<PreInstruction> Recipe::miscSteps(Misc::Use type)
{
   QVector<PreInstruction> preins;
   QString str;
   Unit* kindOf;
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

         kindOf = misc->amountIsWeight() ? (Unit*)Units::kilograms : (Unit*)Units::liters;
         str = str .arg(Brewtarget::displayAmount(misc->amount(), kMiscTableSection, kAmountKgAttr, kindOf))
                   .arg(misc->name())
                   .arg(Brewtarget::displayAmount(misc->time(), kMiscTableSection, kTimeAttr, Units::minutes));

         preins.push_back(PreInstruction(str, tr("Misc addition"), misc->time()));
      }
   }
   return preins;
}

Instruction* Recipe::firstWortHopsIns()
{
   Instruction* ins;
   QString str;
   QList<QString> reagents;

   str = tr("Do first wort hopping with ");

   reagents = getReagents(hops(), true);

   if ( reagents.size() > 0 )
   {
      for( int i = 0; i < reagents.size(); ++i )
         str += reagents.at(i);

      str += ".";
      ins = Database::instance().newInstruction(this);
      ins->setName(tr("First wort hopping"));
      ins->setDirections(str);
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
         .arg(Brewtarget::displayAmount( wortInBoil_l, kTabRecipeSection, kBoilSizeAttr, Units::liters));
      if ( e->topUpKettle_l() != 0 )
      {
         wortInBoil_l += e->topUpKettle_l();
         tmp = tr(" Add %1 water to the kettle, bringing pre-boil volume to %2.")
            .arg(Brewtarget::displayAmount(e->topUpKettle_l(), kTabRecipeSection, kBoilSizeAttr,  Units::liters))
            .arg(Brewtarget::displayAmount(wortInBoil_l, kTabRecipeSection, kBoilSizeAttr,  Units::liters));

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

bool Recipe::hasBoilExtract()
{
   unsigned int i;
   for ( i = 0; static_cast<int>(i) < fermentables().size(); ++i )
   {
      Fermentable* ferm = fermentables()[i];
      if( ferm->isExtract() )
         return true;
      else
         continue;
   }
   return false;
}

PreInstruction Recipe::boilFermentablesPre(double timeRemaining)
{
   QString str;
   unsigned int i;
   int size;

   str = tr("Boil or steep ");
   QList<Fermentable*> flist = fermentables();
   size = flist.size();
   for( i = 0; static_cast<int>(i) < size; ++i )
   {
     Fermentable* ferm = flist[i];
     if( ferm->isMashed() || ferm->addAfterBoil() || ferm->isExtract() )
       continue;

     str += QString("%1 %2, ")
          .arg(Brewtarget::displayAmount(ferm->amount_kg(), kFermentableTableSection, kAmountKgAttr, Units::kilograms))
          .arg(ferm->name());
   }
   str += ".";

   return PreInstruction(str, tr("Boil/steep fermentables"), timeRemaining);
}

bool Recipe::isFermentableSugar(Fermentable *fermy)
{
  if (fermy->type() == Fermentable::Sugar && fermy->name() == "Milk Sugar (Lactose)" )
    return false;
  else
    return true;
}

PreInstruction Recipe::addExtracts(double timeRemaining) const
{
   QString str;
   unsigned int i;
   int size;

   str = tr("Raise water to boil and then remove from heat. Stir in  ");
   const QList<Fermentable*> flist = fermentables();
   size = flist.size();
   for( i = 0; static_cast<int>(i) < size; ++i )
   {
      const Fermentable* ferm = flist[i];
      if ( ferm->isExtract() )
      {
         str += QString("%1 %2, ")
            .arg(Brewtarget::displayAmount(ferm->amount_kg(), kFermentableTableSection, kAmountKgAttr, Units::kilograms))
            .arg(ferm->name());
      }
   }
   str += ".";

   return PreInstruction(str, tr("Add Extracts to water"), timeRemaining);
}

Instruction* Recipe::postboilFermentablesIns()
{
   Instruction* ins;
   QString str,tmp;
   unsigned int i;
   int size;
   bool hasFerms = false;

   str = tr("Add ");
   QList<Fermentable*> flist = fermentables();
   size = flist.size();
   for( i = 0; static_cast<int>(i) < size; ++i )
   {
      Fermentable* ferm = flist[i];
      if( ! ferm->addAfterBoil() )
         continue;

      hasFerms = true;
      tmp = QString("%1 %2, ")
             .arg(Brewtarget::displayAmount(ferm->amount_kg(), kFermentableTableSection, kAmountKgAttr, Units::kilograms))
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
            .arg(Brewtarget::displayAmount( wort_l, kTabRecipeSection, kBatchSizeAttr, Units::liters));
      str += tr("\nYou anticipate losing %1 to trub and chiller loss.")
            .arg(Brewtarget::displayAmount( e->trubChillerLoss_l(), kTabRecipeSection, kBatchSizeAttr,  Units::liters));
      wort_l -= e->trubChillerLoss_l();
      if( e->topUpWater_l() > 0.0 )
          str += tr("\nAdd %1 top up water into primary.")
               .arg(Brewtarget::displayAmount( e->topUpWater_l(), kTabRecipeSection, kBatchSizeAttr,  Units::liters));
      wort_l += e->topUpWater_l();
      str += tr("\nThe final volume in the primary is %1.")
             .arg(Brewtarget::displayAmount(wort_l, kTabRecipeSection, kBatchSizeAttr,  Units::liters));

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

   if ( ! instructions().empty() )
      clearInstructions();

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
      timeRemaining = Brewtarget::qStringToSI(QInputDialog::getText(0,
                                        tr("Boil time"),
                                        tr("You did not configure an equipment (which you really should), so tell me the boil time.")),
                                        Units::minutes);
   }
   
   str = tr("Bring the wort to a boil and hold for %1.").arg(Brewtarget::displayAmount( timeRemaining, "tab_recipe", "boilTime_min", Units::minutes));
   ins = Database::instance().newInstruction(this);
   ins->setName(tr("Start boil"));
   ins->setInterval(timeRemaining);
   ins->setDirections(str);
   
   /*** Get fermentables unless we haven't added yet ***/
   if ( hasBoilFermentable() )
      preinstructions.push_back(boilFermentablesPre(timeRemaining));
   
   // add the intructions for including Extracts to wort
   if ( hasBoilExtract() )
      preinstructions.push_back(addExtracts(timeRemaining-1));

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
         .arg(Brewtarget::displayAmount(fg(), "tab_recipe", "fg", Units::sp_grav, 3));
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

   // END fermentation instructions. Let everybody know that now is the time
   // to update instructions
   emit changed( metaProperty("instructions"), instructions().size() );
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
               .arg(Brewtarget::displayAmount(h->amount_kg(), kHopTableSection, kAmountKgAttr, Units::kilograms))
               .arg(h->name())
               .arg(Brewtarget::displayAmount(h->time_min(), kHopTableSection, kTimeAttr,  Units::minutes));

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
            ret = ret.arg(Brewtarget::displayAmount(m->amount(), kMiscTableSection, kAmountAttr, Units::kilograms));
         else
            ret = ret.arg(Brewtarget::displayAmount(m->amount(), kMiscTableSection, kAmountAttr,  Units::liters));

         ret = ret.arg(m->name());
         ret = ret.arg(Brewtarget::displayAmount(m->time(), kMiscTableSection, kTimeAttr, Units::minutes));
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
   connect(var, &Fermentable::saved, this, &Recipe::onFermentableChanged);
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

   set( kTypeProp, kType, tmp );
}

void Recipe::setBrewer( const QString &var )
{
   set( kBrewerProp, kBrewer, var );
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

   set( kBatchSizeProp, kBatchSize, tmp );
   
   // NOTE: this is bad, but we have to call recalcAll(), because the estimated
   // boil/batch volumes depend on the target volumes when there are no mash
   // steps to actually provide an estimate for the volumes.
   recalcAll();
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

   set( kBoilSizeProp, kBoilSize, tmp );
   
   // NOTE: this is bad, but we have to call recalcAll(), because the estimated
   // boil/batch volumes depend on the target volumes when there are no mash
   // steps to actually provide an estimate for the volumes.
   recalcAll();
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

   set( kBoilTimeProp, kBoilTime, tmp);
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


   set( kEfficiencyProp, kEfficiency, tmp );

   // If you change the efficency, you really should recalc. And I'm afraid it
   // means recalc all, since og and fg will change, which means your ratios
   // change
   recalcAll();
}

void Recipe::setAsstBrewer( const QString &var )
{
   set( kAsstBrewerProp, kAsstBrewer, var );
}

void Recipe::setNotes( const QString &var )
{
   set( kNotesProp, kNotes, var );
}

void Recipe::setTasteNotes( const QString &var )
{
   set( kTasteNotesProp, kTasteNotes, var );
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

   set( kTasteRatingProp, kTasteRating, tmp );
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

   set( kOGProp, kOG, tmp );
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

   set( kFGProp, kFG, tmp );
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

   set( kFermentationStagesProp, kFermentationStages, tmp );
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

   set( kPrimaryAgeDaysProp, kPrimaryAgeDays, tmp );
}

void Recipe::setPrimaryTemp_c( double var )
{
   set( kPrimaryTempProp, kPrimaryTemp, var );
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

   set( kSecondaryAgeDaysProp, kSecondaryAgeDays, tmp );
}

void Recipe::setSecondaryTemp_c( double var )
{
   set( kSecondaryTempProp, kSecondaryTemp, var );
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

   set( kTertiaryAgeDaysProp, kTertiaryAgeDays, tmp );
}

void Recipe::setTertiaryTemp_c( double var )
{
   set( kTertiaryTempProp, kTertiaryTemp, var );
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

   set( kAgeProp, kAge, tmp );
}

void Recipe::setAgeTemp_c( double var )
{
   set( kAgeTempProp, kAgeTemp, var );
}

void Recipe::setDate( const QDate &var )
{
   static const QString dateFormat("d/M/yyyy");
   set( kDateProp, kDate, var.toString(dateFormat) );
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

   set( kCarbonationVolsProp, kCarbonationVols, tmp );
}

void Recipe::setForcedCarbonation( bool var )
{
   set( kForcedCarbonationProp, kForcedCarbonation, var );
}

void Recipe::setPrimingSugarName( const QString &var )
{
   set( kPrimingSugarNameProp, kPrimingSugarName, var );
}

void Recipe::setCarbonationTemp_c( double var )
{
   set( kCarbonationTempProp, kCarbonationTemp, var );
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

   set( kPrimingSugarEquivProp, kPrimingSugarEquiv, tmp );
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

   set( kKegPrimingFactorProp, kKegPrimingFactor, tmp );
}

//==========================Calculated Getters============================

double Recipe::og()
{
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

double Recipe::calories12oz()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _calories;
}

double Recipe::calories33cl()
{
   if( _uninitializedCalcs )
      recalcAll();
   return _calories*3.3/3.55;
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
   return (_og-1.0)*1e3;
}

//=========================Relational Getters=============================
Style* Recipe::style() const
{
   return Database::instance().style(this);
}

Mash* Recipe::mash() const
{
   return Database::instance().mash( this );
}

Equipment* Recipe::equipment() const
{
   return Database::instance().equipment(this);
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
QString Recipe::type() const
{
   return get(kType).toString();
}

QString Recipe::brewer() const
{
   return get(kBrewer).toString();
}

QString Recipe::asstBrewer() const
{
   return get(kAsstBrewer).toString();
}

QString Recipe::notes() const
{
   return get(kNotes).toString();
}

QString Recipe::tasteNotes() const
{
   return get(kTasteNotes).toString();
}

QString Recipe::primingSugarName() const
{
   return get(kPrimingSugarName).toString();
}

bool Recipe::forcedCarbonation() const
{
   return get(kForcedCarbonation).toBool();
}

double Recipe::batchSize_l() const
{
   return get(kBatchSize).toDouble();
}

double Recipe::boilSize_l() const
{
   return get(kBoilSize).toDouble();
}

double Recipe::boilTime_min() const
{
   return get(kBoilTime).toDouble();
}

double Recipe::efficiency_pct() const
{
   return get(kEfficiency).toDouble();
}

double Recipe::tasteRating() const
{
   return get(kTasteRating).toDouble();
}

double Recipe::primaryAge_days() const
{
   return get(kPrimaryAgeDays).toDouble();
}

double Recipe::primaryTemp_c() const
{
   return get(kPrimaryTemp).toDouble();
}

double Recipe::secondaryAge_days() const
{
   return get(kSecondaryAgeDays).toDouble();
}

double Recipe::secondaryTemp_c() const
{
   return get(kSecondaryTemp).toDouble();
}

double Recipe::tertiaryAge_days() const
{
   return get(kTertiaryAgeDays).toDouble();
}

double Recipe::tertiaryTemp_c() const
{
   return get(kTertiaryTemp).toDouble();
}

double Recipe::age_days() const
{
   return get(kAge).toDouble();
}

double Recipe::ageTemp_c() const
{
   return get(kAgeTemp).toDouble();
}

double Recipe::carbonation_vols() const
{
   return get(kCarbonationVols).toDouble();
}

double Recipe::carbonationTemp_c() const
{
   return get(kCarbonationTemp).toDouble();
}

double Recipe::primingSugarEquiv() const
{
   return get(kPrimingSugarEquiv).toDouble();
}

double Recipe::kegPrimingFactor() const
{
   return get(kKegPrimingFactor).toDouble();
}

int Recipe::fermentationStages() const
{
   return get(kFermentationStages).toInt();
}

QDate Recipe::date() const
{
   static const QString dateFormat("d/M/yyyy");
   return QDate::fromString( get(kDate).toString(), dateFormat);
}
//=============================Removers========================================

// Returns true if var is found and removed.
void Recipe::remove( BeerXMLElement *var )
{
   // brewnotes a bit odd
   if ( dynamic_cast<BrewNote*>(var) )
      // the cast is required to force the template to gets it thing right
      Database::instance().remove(qobject_cast<BrewNote*>(var));
   else
      Database::instance().removeIngredientFromRecipe( this, var );
}

double Recipe::batchSizeNoLosses_l()
{
   double ret = batchSize_l();
   Equipment* e = equipment();
   if( e )
      ret += e->trubChillerLoss_l();
   
   return ret;
}

//==============================Recalculators==================================

void Recipe::recalcAll()
{
   // WARNING
   // Infinite recursion possible, since these methods will emit changed(),
   // causing other objects to call finalVolume_l() for example, which may
   // cause another call to recalcAll() and so on.
   //
   // GSG: Now only emit when _uninitializedCalcs is true, which helps some.

   // Someone has already called this function back in the call stack, so return to avoid recursion.
   if( !_recalcMutex.tryLock() )
      return;
   
   // Times are in seconds, and are cumulative.
   recalcGrainsInMash_kg(); // 0.01
   recalcGrains_kg(); // 0.03
   recalcVolumeEstimates(); // 0.06
   recalcColor_srm(); // 0.08
   recalcSRMColor(); // 0.08
   recalcOgFg(); // 0.11
   recalcABV_pct(); // 0.12
   recalcBoilGrav(); // 0.14
   recalcIBU(); // 0.15
   recalcCalories();
   
   _uninitializedCalcs = false;
   
   _recalcMutex.unlock();
}

void Recipe::recalcABV_pct()
{
   double ret;

   // The complex formula, and variations comes from Ritchie Products Ltd, (Zymurgy, Summer 1995, vol. 18, no. 2)
   // Michael L. Hall’s article Brew by the Numbers: Add Up What’s in Your Beer, and Designing Great Beers by Daniels.
   ret = (76.08 * (_og_fermentable - _fg_fermentable) / (1.775 - _og_fermentable)) * (_fg_fermentable / 0.794);
  
   if ( ret != _ABV_pct ) 
   {
      _ABV_pct = ret;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("ABV_pct"), _ABV_pct );
      }
   }
}

void Recipe::recalcColor_srm()
{
   Fermentable *ferm;
   double mcu = 0.0;
   double ret;
   unsigned int i;

   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];
      // Conversion factor for lb/gal to kg/l = 8.34538.
      mcu += ferm->color_srm()*8.34538 * ferm->amount_kg()/_finalVolumeNoLosses_l;
   }

   ret = ColorMethods::mcuToSrm(mcu);
 
   if ( _color_srm != ret ) 
   {
      _color_srm = ret;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("color_srm"), _color_srm );
      }
   }

}

void Recipe::recalcIBU()
{
   unsigned int i;
   double ibus = 0.0;
   double tmp = 0.0;
   
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

   if ( ibus != _IBU ) 
   {
      _IBU = ibus;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("IBU"), _IBU );
      }
   }
}

void Recipe::recalcVolumeEstimates()
{
   double waterAdded_l;
   double absorption_lKg;
   double tmp = 0.0;
   double tmp_wfm = 0.0;
   double tmp_bv = 0.0;
   double tmp_fv = 0.0;
   double tmp_pbv = 0.0;

   // wortFromMash_l ==========================
   if( mash() == 0 )
      _wortFromMash_l = 0.0;
   else
   {
      waterAdded_l = mash()->totalMashWater_l();
      if( equipment() != 0 )
         absorption_lKg = equipment()->grainAbsorption_LKg();
      else
         absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;

      tmp_wfm = (waterAdded_l - absorption_lKg * _grainsInMash_kg);
   }
   
   // boilVolume_l ==============================
   
   if( equipment() != 0 )
      tmp = tmp_wfm - equipment()->lauterDeadspace_l() + equipment()->topUpKettle_l();
   else
      tmp = tmp_wfm;
   
   // Need to account for extract/sugar volume also.
   QList<Fermentable*> ferms = fermentables();
   foreach( Fermentable* f, ferms )
   {
      Fermentable::Type type = f->type();
      if( type == Fermentable::Extract )
         tmp += f->amount_kg() / PhysicalConstants::liquidExtractDensity_kgL;
      else if( type == Fermentable::Sugar )
         tmp += f->amount_kg() / PhysicalConstants::sucroseDensity_kgL;
      else if( type == Fermentable::Dry_Extract )
         tmp += f->amount_kg() / PhysicalConstants::dryExtractDensity_kgL;
   }
   
   if( tmp <= 0.0 )
      tmp = boilSize_l(); // Give up.
 
   tmp_bv = tmp;
   
   // finalVolume_l ==============================
   
   // NOTE: the following figure is not based on the other volume estimates
   // since we want to show og,fg,ibus,etc. as if the collected wort is correct.
   _finalVolumeNoLosses_l = batchSizeNoLosses_l();
   if( equipment() != 0 )
   {
      //_finalVolumeNoLosses_l = equipment()->wortEndOfBoil_l(tmp_bv) + equipment()->topUpWater_l();
      tmp_fv = equipment()->wortEndOfBoil_l(tmp_bv) + equipment()->topUpWater_l() - equipment()->trubChillerLoss_l();
   }
   else
   {
      _finalVolume_l = tmp_bv - 4.0; // This is just shooting in the dark. Can't do much without an equipment.
      //_finalVolumeNoLosses_l = _finalVolume_l;
   }
   
   // postBoilVolume_l ===========================

   if( equipment() != 0 )
      tmp_pbv = equipment()->wortEndOfBoil_l( tmp_bv );
   else
      tmp_pbv = batchSize_l(); // Give up.

   if ( tmp_wfm != _wortFromMash_l )
   {
      _wortFromMash_l = tmp_wfm;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("wortFromMash_l"), _wortFromMash_l );
      }
   }

   if ( tmp_bv != _boilVolume_l )
   {
      _boilVolume_l = tmp_bv;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("boilVolume_l"), _boilVolume_l );
      }
   }
   
   if ( tmp_fv != _finalVolume_l )
   {
      _finalVolume_l = tmp_fv;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("finalVolume_l"), _finalVolume_l );
      }
   }

   if ( tmp_pbv != _postBoilVolume_l )
   {
      _postBoilVolume_l = tmp_pbv;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("postBoilVolume_l"), _postBoilVolume_l );
      }
   }
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

   if ( ret != _grainsInMash_kg ) 
   {
      _grainsInMash_kg = ret;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("grainsInMash_kg"), _grainsInMash_kg );
      }
   }
}

void Recipe::recalcGrains_kg()
{
   unsigned int i, size;
   double ret = 0.0;

   QList<Fermentable*> ferms = fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
      ret += ferms[i]->amount_kg();

   if ( ret != _grains_kg ) 
   {
      _grains_kg = ret;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("grains_kg"), _grains_kg );
      }
   }
}

void Recipe::recalcSRMColor()
{
   QColor tmp = Algorithms::srmToColor(_color_srm);

   if ( tmp != _SRMColor )
   {
      _SRMColor = tmp;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("SRMColor"), _SRMColor );
      }
   }
}

// the formula in here are taken from http://hbd.org/ensmingr/
void Recipe::recalcCalories()
{
   double startPlato, finishPlato, RE, abw, oog, ffg, tmp;

   oog = _og;
   ffg = _fg;

   // Need to translate OG and FG into plato
   startPlato  = -463.37 + ( 668.72 * oog ) - (205.35 * oog * oog);
   finishPlato = -463.37 + ( 668.72 * ffg ) - (205.35 * ffg * ffg);

   // RE (real extract)
   RE = (0.1808 * startPlato) + (0.8192 * finishPlato);

   // Alcohol by weight?
   abw = (startPlato-RE)/(2.0665 - (0.010665 * startPlato));

   // The final results of this formular are calories per 100 ml. 
   // The 3.55 puts it in terms of 12 oz. I really should have stored it
   // without that adjust.
   tmp = ((6.9*abw) + 4.0 * (RE-0.1)) * ffg * 3.55;

   //! If there are no fermentables in the recipe, if there is no mash, etc.,
   //  then the calories/12 oz ends up negative. Since negative doesn't make
   //  sense, set it to 0
   if ( tmp < 0 )
      tmp = 0;

   if ( tmp != _calories ) 
   {
      _calories = tmp;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("calories"), _calories );
      }
   }
}

// other efficiency calculations need access to the maximum theoretical sugars
// available. The only way I can see of doing that which doesn't suck is to
// split that calcuation out of recalcOgFg();
QHash<QString,double> Recipe::calcTotalPoints()
{
   int i;
   double sugar_kg_ignoreEfficiency = 0.0;
   double sugar_kg                  = 0.0;
   double nonFermetableSugars_kg    = 0.0;
   double lateAddition_kg           = 0.0;
   double lateAddition_kg_ignoreEff = 0.0;

   Fermentable* ferm;

   QList<Fermentable*> ferms = fermentables();
   QHash<QString,double> ret;
   
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];

      // If we have some sort of non-grain, we have to ignore efficiency.
      if( ferm->isSugar() || ferm->isExtract() )
      {
         sugar_kg_ignoreEfficiency += ferm->equivSucrose_kg();

         if (ferm->addAfterBoil())
            lateAddition_kg_ignoreEff += ferm->equivSucrose_kg();

         if ( !isFermentableSugar(ferm) )
           nonFermetableSugars_kg += ferm->equivSucrose_kg();
      }
      else
      {
         sugar_kg += ferm->equivSucrose_kg();

         if (ferm->addAfterBoil())
            lateAddition_kg += ferm->equivSucrose_kg();
      }
   }   
   
   ret.insert("sugar_kg", sugar_kg);
   ret.insert("nonFermetableSugars_kg", nonFermetableSugars_kg);
   ret.insert("sugar_kg_ignoreEfficiency", sugar_kg_ignoreEfficiency);
   ret.insert("lateAddition_kg", lateAddition_kg);
   ret.insert("lateAddition_kg_ignoreEff", lateAddition_kg_ignoreEff);

   return ret;

}

void Recipe::recalcBoilGrav()
{
   double sugar_kg = 0.0;
   double sugar_kg_ignoreEfficiency = 0.0;
   double lateAddition_kg           = 0.0;
   double lateAddition_kg_ignoreEff = 0.0;
   double ret;
   QHash<QString,double> sugars;

   sugars = calcTotalPoints();
   sugar_kg = sugars.value("sugar_kg");
   sugar_kg_ignoreEfficiency = sugars.value("sugar_kg_ignoreEfficiency");
   lateAddition_kg = sugars.value("lateAddition_kg");
   lateAddition_kg_ignoreEff = sugars.value("lateAddition_kg_ignoreEff");
   
   // Since the efficiency refers to how much sugar we get into the fermenter,
   // we need to adjust for that here.
   sugar_kg = (efficiency_pct()/100.0 * (sugar_kg - lateAddition_kg) + sugar_kg_ignoreEfficiency - lateAddition_kg_ignoreEff);

   ret = Algorithms::PlatoToSG_20C20C( Algorithms::getPlato(sugar_kg, boilSize_l()) );
 
   if ( ret != _boilGrav )
   {
      _boilGrav = ret;
      if (!_uninitializedCalcs)
      {
        emit changed( metaProperty("boilGrav"), _boilGrav );
      }
   }
}

void Recipe::recalcOgFg()
{
   unsigned int i;
   double plato;
   double sugar_kg = 0;
   double sugar_kg_ignoreEfficiency = 0.0;
   double nonFermetableSugars_kg = 0.0;
   double kettleWort_l = 0.0;
   double postBoilWort_l = 0.0;
   double ratio = 0.0;
   double ferm_kg = 0.0;
   double attenuation_pct = 0.0;
   double tmp_og, tmp_fg, tmp_pnts, tmp_ferm_pnts;
   Yeast* yeast;
   QHash<QString,double> sugars;
  
   _og_fermentable = _fg_fermentable = 0.0;

   // The first time through really has to get the _og and _fg from the
   // database, not use the initialized values of 1. I (maf) tried putting
   // this in the initialize, but it just hung. So I moved it here, but only
   // if if we aren't initialized yet.
   //
   // GSG: This doesn't work, this og and fg are already set to 1.0 so
   // until we load these values from the database on startup, we have
   // to calculate.
   if ( _uninitializedCalcs )
   {
      _og = Brewtarget::toDouble(this, kOG, "Recipe::recalcOgFg()");
      _fg = Brewtarget::toDouble(this, kFG, "Recipe::recalcOgFg()");
   }

   // Find out how much sugar we have.
   sugars = calcTotalPoints();
   sugar_kg                  = sugars.value("sugar_kg");
   sugar_kg_ignoreEfficiency = sugars.value("sugar_kg_ignoreEfficiency");
   nonFermetableSugars_kg    = sugars.value("nonFermetableSugars_kg");

   // We might lose some sugar in the form of Trub/Chiller loss and lauter deadspace.
   if( equipment() != 0 )
   {
      
      kettleWort_l = (_wortFromMash_l - equipment()->lauterDeadspace_l()) + equipment()->topUpKettle_l();
      postBoilWort_l = equipment()->wortEndOfBoil_l(kettleWort_l);
      ratio = (postBoilWort_l - equipment()->trubChillerLoss_l()) / postBoilWort_l;
      if( ratio > 1.0 ) // Usually happens when we don't have a mash yet.
         ratio = 1.0;
      else if( ratio < 0.0 )
         ratio = 0.0;
      else if( Algorithms::isNan(ratio) )
         ratio = 1.0;
      // Ignore this again since it should be included in efficiency.
      //sugar_kg *= ratio;
      sugar_kg_ignoreEfficiency *= ratio;
      if ( nonFermetableSugars_kg != 0.0 )
         nonFermetableSugars_kg *= ratio;
   }

   sugar_kg = sugar_kg * efficiency_pct()/100.0 + sugar_kg_ignoreEfficiency;
   plato = Algorithms::getPlato( sugar_kg, _finalVolumeNoLosses_l);

   tmp_og = Algorithms::PlatoToSG_20C20C( plato );
   tmp_pnts = (tmp_og-1)*1000.0;
   if ( nonFermetableSugars_kg != 0.0 )
   {
      ferm_kg = sugar_kg - nonFermetableSugars_kg;
      plato = Algorithms::getPlato( ferm_kg, _finalVolumeNoLosses_l);
      _og_fermentable = Algorithms::PlatoToSG_20C20C( plato );
      plato = Algorithms::getPlato( nonFermetableSugars_kg, _finalVolumeNoLosses_l); 
      tmp_ferm_pnts = ((Algorithms::PlatoToSG_20C20C( plato ))-1)*1000.0;
   }
   else
   {
      _og_fermentable = tmp_og;
      tmp_ferm_pnts = 0;
   }

   // Calculage FG
   QList<Yeast*> yeasties = yeasts();
   for( i = 0; static_cast<int>(i) < yeasties.size(); ++i )
   {
      yeast = yeasties[i];
      // Get the yeast with the greatest attenuation.
      if( yeast->attenuation_pct() > attenuation_pct )
         attenuation_pct = yeast->attenuation_pct();
   }
   if( yeasties.size() > 0 && attenuation_pct <= 0.0 ) // This means we have yeast, but they neglected to provide attenuation percentages.
      attenuation_pct = 75.0; // 75% is an average attenuation.
   
   if ( nonFermetableSugars_kg != 0.0 )
   {
      tmp_ferm_pnts = (tmp_pnts-tmp_ferm_pnts) * (1.0 - attenuation_pct/100.0);
      tmp_pnts *= (1.0 - attenuation_pct/100.0);
      tmp_fg =  1 + tmp_pnts/1000.0;
      _fg_fermentable =  1 + tmp_ferm_pnts/1000.0;
   }
   else
   {
      tmp_pnts *= (1.0 - attenuation_pct/100.0);
      tmp_fg =  1 + tmp_pnts/1000.0;
      _fg_fermentable = tmp_fg;
   }
   
   if ( _og != tmp_og ) 
   {
      _og     = tmp_og;
      // NOTE: We don't want to do this on the first load of the recipe.
      // NOTE: We are we recalculating all of these on load? Shouldn't we be
      // reading these values from the database somehow?
      //
      // GSG: Yes we can, but until the code is added to intialize these calculated
      // values from the database, we can calculate them on load. They should be
      // the same as the database values since the database values were set with
      // these functions in the first place.
      if (!_uninitializedCalcs)
      {
        set( kOGProp, kOG, _og, false );
        emit changed( metaProperty(kOGProp), _og );
        emit changed( metaProperty("points"), (_og-1.0)*1e3 );
      }
   }

   if ( tmp_fg != _fg ) 
   {
      _fg     = tmp_fg;
      if (!_uninitializedCalcs)
      {
        set( kFGProp, kFG, _fg, false );
        emit changed( metaProperty(kFGProp), _fg );
      }
   }
}

//====================================Helpers===========================================

double Recipe::ibuFromHop(Hop const* hop)
{
   Equipment* equip = equipment();
   double ibus = 0.0;
   double fwhAdjust = Brewtarget::toDouble(Brewtarget::option("firstWortHopAdjustment", 1.1).toString(), "Recipe::ibmFromHop()");
   double mashHopAdjust = Brewtarget::toDouble(Brewtarget::option("mashHopAdjustment", 0).toString(), "Recipe::ibmFromHop()");
   
   if( hop == 0 )
      return 0.0;
   
   double AArating = hop->alpha_pct()/100.0;
   double grams = hop->amount_kg()*1000.0;
   double minutes = hop->time_min();
   // Assume 100% utilization until further notice
   double hopUtilization = 1.0;
   // Assume 60 min boil until further notice
   int boilTime = 60;

   // NOTE: we used to carefully calculate the average boil gravity and use it in the
   // IBU calculations. However, due to John Palmer
   // (http://homebrew.stackexchange.com/questions/7343/does-wort-gravity-affect-hop-utilization),
   // it seems more appropriate to just use the OG directly, since it is the total
   // amount of break material that truly affects the IBUs.
   
   if( equip )
   {
      hopUtilization = equip->hopUtilization_pct() / 100.0;
      boilTime = equip->boilTime_min();
   }
   
   if( hop->use() == Hop::Boil)
      ibus = IbuMethods::getIbus( AArating, grams, _finalVolumeNoLosses_l, _og, minutes );
   else if( hop->use() == Hop::First_Wort )
      ibus = fwhAdjust * IbuMethods::getIbus( AArating, grams, _finalVolumeNoLosses_l, _og, boilTime );
   else if( hop->use() == Hop::Mash && mashHopAdjust > 0.0 )
      ibus = mashHopAdjust * IbuMethods::getIbus( AArating, grams, _finalVolumeNoLosses_l, _og, boilTime );

   // Adjust for hop form. Tinseth's table was created from whole cone data,
   // and it seems other formulae are optimized that way as well. So, the
   // utilization is considered unadjusted for whole cones, and adjusted
   // up for plugs and pellets.
   //
   // - http://www.realbeer.com/hops/FAQ.html
   // - https://groups.google.com/forum/#!topic/brewtarget-help/mv2qvWBC4sU
   switch( hop->form() ) {
   case Hop::Plug:
      hopUtilization *= 1.02;
      break;
   case Hop::Pellet:
      hopUtilization *= 1.10;
      break;
   default:
      break;
   }

   // Adjust for hop utilization. 
   ibus *= hopUtilization;

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

QList<QString> Recipe::getReagents( QList<Fermentable*> ferms )
{
   QList<QString> reagents;
   QString format,tmp;

   for ( int i = 0; i < ferms.size(); ++i )
   {
      if ( ferms[i]->isMashed() )
      {
         if ( i+1 < ferms.size() )
         {
            tmp = QString("%1 %2, ")
                  .arg(Brewtarget::displayAmount(ferms[i]->amount_kg(), kFermentableTableSection, kAmountKgAttr, Units::kilograms))
                  .arg(ferms[i]->name());
         }
         else
         {
            tmp = QString("%1 %2 ")
                  .arg(Brewtarget::displayAmount(ferms[i]->amount_kg(), kFermentableTableSection, kAmountKgAttr,  Units::kilograms))
                  .arg(ferms[i]->name());
         }
         reagents.append(tmp);
      }
   }
   return reagents;
}

QList<QString> Recipe::getReagents(QList<Hop*> hops, bool firstWort)
{
   QString tmp;
   QList<QString> reagents;

   for( int i = 0; i < hops.size(); ++i )
   {
      if( firstWort && 
         (hops[i]->use() == Hop::First_Wort) )
      {
         tmp = QString("%1 %2,")
               .arg(Brewtarget::displayAmount(hops[i]->amount_kg(), kHopTableSection, kAmountKgAttr,  Units::kilograms))
               .arg(hops[i]->name());
         reagents.append(tmp);
      }
   }
   return reagents;
}

QList<QString> Recipe::getReagents( QList<MashStep*> msteps )
{
   QString tmp;
   QList<QString> reagents;

   for ( int i = 0; i < msteps.size(); ++i )
   {
      if( ! msteps[i]->isInfusion() )
         continue;

      if ( i+1 < msteps.size() ) 
      {
         tmp = tr("%1 water to %2, ")
                .arg(Brewtarget::displayAmount(msteps[i]->infuseAmount_l(), kMashStepSection, kInfuseAmountAttr, Units::liters))
                .arg(Brewtarget::displayAmount(msteps[i]->infuseTemp_c(), kMashStepSection, kInfuseTempAttr,  Units::celsius));
      }
      else 
      {
         tmp = tr("%1 water to %2 ")
                .arg(Brewtarget::displayAmount(msteps[i]->infuseAmount_l(), kMashStepSection, kInfuseAmountAttr, Units::liters))
                .arg(Brewtarget::displayAmount(msteps[i]->infuseTemp_c(), kMashStepSection, kInfuseTempAttr, Units::celsius));
      }
      reagents.append(tmp);
   }
   return reagents;
}

//==========================Accept changes from ingredients====================

void Recipe::acceptEquipChange(QMetaProperty prop, QVariant val)
{
   recalcAll();
}

void Recipe::acceptFermChange(QMetaProperty prop, QVariant val)
{
   recalcAll();
}

void Recipe::onFermentableChanged()
{
   recalcAll();
}

void Recipe::acceptHopChange(QMetaProperty prop, QVariant val)
{
   recalcIBU();
}

void Recipe::acceptHopChange(Hop* hop) 
{
   recalcIBU();
}

void Recipe::acceptYeastChange(QMetaProperty prop, QVariant val)
{
   recalcOgFg();
   recalcABV_pct();
}

void Recipe::acceptYeastChange(Yeast* yeast)
{
   recalcOgFg();
   recalcABV_pct();
}

void Recipe::acceptMashChange(QMetaProperty prop, QVariant val)
{
   Mash* mashSend = qobject_cast<Mash*>(sender());

   if ( mashSend == 0 )
      return;
   
   recalcAll();
}

void Recipe::acceptMashChange(Mash* newMash)
{
   if ( newMash == mash() )
      recalcAll();
}

double Recipe::targetCollectedWortVol_l() {

   // Need to account for extract/sugar volume also.
   float postMashAdditionVolume_l = 0;
   QList<Fermentable*> ferms = fermentables();
         foreach( Fermentable* f, ferms )
      {
         Fermentable::Type type = f->type();
         if( type == Fermentable::Extract )
            postMashAdditionVolume_l  += f->amount_kg() / PhysicalConstants::liquidExtractDensity_kgL;
         else if( type == Fermentable::Sugar )
            postMashAdditionVolume_l  += f->amount_kg() / PhysicalConstants::sucroseDensity_kgL;
         else if( type == Fermentable::Dry_Extract )
            postMashAdditionVolume_l  += f->amount_kg() / PhysicalConstants::dryExtractDensity_kgL;
      }

   return boilSize_l() - equipment()->topUpKettle_l() - postMashAdditionVolume_l;
}

double Recipe::targetTotalMashVol_l() {

   double absorption_lKg;
   if( equipment() )
      absorption_lKg = equipment()->grainAbsorption_LKg();
   else
      absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;


   return targetCollectedWortVol_l() + absorption_lKg * grainsInMash_kg();
}
