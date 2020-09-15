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
#include "salt.h"
#include "PreInstruction.h"
#include "Algorithms.h"
#include "IbuMethods.h"
#include "ColorMethods.h"
#include "HeatCalculations.h"
#include "PhysicalConstants.h"
#include "QueuedMethod.h"

#include "TableSchemaConst.h"
#include "RecipeSchema.h"

static const QString kMashStepSection("mashStepTableModel");
static const QString kMiscTableSection("miscTableModel");
static const QString kFermentableTableSection("fermentableTable");
static const QString kHopTableSection("hopTable");
static const QString kSaltTableSection("saltTable");
static const QString kTabRecipeSection("tab_recipe");

static const QString kTimeAttr("time");
static const QString kInfuseAmountAttr("infuseAmount_l");
static const QString kInfuseTempAttr("infuseTemp_c");
static const QString kStepTempAttr("stepTemp_c");
static const QString kDecoctionAmountAttr("decoctionAmount_l");
static const QString kStepTimeAttr("stepTime_min");
static const QString kBoilSizeAttr("boilSize_l");
static const QString kBatchSizeAttr("batchSize_l");
static const QString kSaltAmountAttr("amount");


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
   : Ingredient(table, key),
   m_type(QString()),
   m_brewer(QString()),
   m_asstBrewer(QString()),
   m_batchSize_l(0.0),
   m_boilSize_l(0.0),
   m_boilTime_min(0.0),
   m_efficiency_pct(0.0),
   m_fermentationStages(1),
   m_primaryAge_days(0.0),
   m_primaryTemp_c(0.0),
   m_secondaryAge_days(0.0),
   m_secondaryTemp_c(0.0),
   m_tertiaryAge_days(0.0),
   m_tertiaryTemp_c(0.0),
   m_age(0.0),
   m_ageTemp_c(0.0),
   m_date(QDate::fromString(QString("01/01/1970"), QString("d/M/yyyy"))),
   m_carbonation_vols(0.0),
   m_forcedCarbonation(false),
   m_primingSugarName(QString()),
   m_carbonationTemp_c(0.0),
   m_primingSugarEquiv(0.0),
   m_kegPrimingFactor(0.0),
   m_notes(QString()),
   m_tasteNotes(QString()),
   m_tasteRating(0.0),
   m_style_id(0),
   m_og(1.0),
   m_fg(1.0),
   m_cacheOnly(false)
{
}

Recipe::Recipe(QString name, bool cache)
   : Ingredient(Brewtarget::RECTABLE, -1, name, true),
   m_type(QString()),
   m_brewer(QString()),
   m_asstBrewer(QString()),
   m_batchSize_l(0.0),
   m_boilSize_l(0.0),
   m_boilTime_min(0.0),
   m_efficiency_pct(0.0),
   m_fermentationStages(1),
   m_primaryAge_days(0.0),
   m_primaryTemp_c(0.0),
   m_secondaryAge_days(0.0),
   m_secondaryTemp_c(0.0),
   m_tertiaryAge_days(0.0),
   m_tertiaryTemp_c(0.0),
   m_age(0.0),
   m_ageTemp_c(0.0),
   m_date(QDate::fromString(QString("01/01/1970"), QString("d/M/yyyy"))),
   m_carbonation_vols(0.0),
   m_forcedCarbonation(false),
   m_primingSugarName(QString()),
   m_carbonationTemp_c(0.0),
   m_primingSugarEquiv(0.0),
   m_kegPrimingFactor(0.0),
   m_notes(QString()),
   m_tasteNotes(QString()),
   m_tasteRating(0.0),
   m_style_id(0),
   m_og(1.0),
   m_fg(1.0),
   m_cacheOnly(cache)
{
}

Recipe::Recipe(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool(), rec.value(kcolFolder).toString()),
   m_type(rec.value(kcolRecipeType).toString()),
   m_brewer(rec.value(kcolRecipeBrewer).toString()),
   m_asstBrewer(rec.value(kcolRecipeAsstBrewer).toString()),
   m_batchSize_l(rec.value(kcolRecipeBatchSize).toDouble()),
   m_boilSize_l(rec.value(kcolRecipeBoilSize).toDouble()),
   m_boilTime_min(rec.value(kcolRecipeBoilTime).toDouble()),
   m_efficiency_pct(rec.value(kcolRecipeEff).toDouble()),
   m_fermentationStages(rec.value(kcolRecipeFermStages).toInt()),
   m_primaryAge_days(rec.value(kcolRecipePrimAgeDays).toDouble()),
   m_primaryTemp_c(rec.value(kcolRecipePrimTemp).toDouble()),
   m_secondaryAge_days(rec.value(kcolRecipeSecAgeDays).toDouble()),
   m_secondaryTemp_c(rec.value(kcolRecipeSecTemp).toDouble()),
   m_tertiaryAge_days(rec.value(kcolRecipeTertAgeDays).toDouble()),
   m_tertiaryTemp_c(rec.value(kcolRecipeTertTemp).toDouble()),
   m_age(rec.value(kcolRecipeAge).toDouble()),
   m_ageTemp_c(rec.value(kcolRecipeAgeTemp).toDouble()),
   m_date(QDate::fromString(rec.value(kcolRecipeDate).toString(), QString("d/M/yyyy"))),
   m_carbonation_vols(rec.value(kcolRecipeCarbVols).toDouble()),
   m_forcedCarbonation(rec.value(kcolRecipeForcedCarb).toBool()),
   m_primingSugarName(rec.value(kcolRecipePrimSugName).toString()),
   m_carbonationTemp_c(rec.value(kcolRecipeCarbTemp).toDouble()),
   m_primingSugarEquiv(rec.value(kcolRecipePrimSugEquiv).toDouble()),
   m_kegPrimingFactor(rec.value(kcolRecipeKegPrimFact).toDouble()),
   m_notes(rec.value(kcolNotes).toString()),
   m_tasteNotes(rec.value(kcolRecipeTasteNotes).toString()),
   m_tasteRating(rec.value(kcolRecipeTasteRating).toDouble()),
   m_style_id(rec.value(kcolRecipeStyleId).toInt()),
   m_og(rec.value(kcolRecipeOG).toDouble()),
   m_fg(rec.value(kcolRecipeFG).toDouble()),
   m_cacheOnly(false)
{
}

Recipe::Recipe( Recipe const& other ) : Ingredient(other),
   m_type(other.m_type),
   m_brewer(other.m_brewer),
   m_asstBrewer(other.m_asstBrewer),
   m_batchSize_l(other.m_batchSize_l),
   m_boilSize_l(other.m_boilSize_l),
   m_boilTime_min(other.m_boilTime_min),
   m_efficiency_pct(other.m_efficiency_pct),
   m_fermentationStages(other.m_fermentationStages),
   m_primaryAge_days(other.m_primaryAge_days),
   m_primaryTemp_c(other.m_primaryTemp_c),
   m_secondaryAge_days(other.m_secondaryAge_days),
   m_secondaryTemp_c(other.m_secondaryTemp_c),
   m_tertiaryAge_days(other.m_tertiaryAge_days),
   m_tertiaryTemp_c(other.m_tertiaryTemp_c),
   m_age(other.m_age),
   m_ageTemp_c(other.m_ageTemp_c),
   m_date(other.m_date),
   m_carbonation_vols(other.m_carbonation_vols),
   m_forcedCarbonation(other.m_forcedCarbonation),
   m_primingSugarName(other.m_primingSugarName),
   m_carbonationTemp_c(other.m_carbonationTemp_c),
   m_primingSugarEquiv(other.m_primingSugarEquiv),
   m_kegPrimingFactor(other.m_kegPrimingFactor),
   m_notes(other.m_notes),
   m_tasteNotes(other.m_tasteNotes),
   m_tasteRating(other.m_tasteRating),
   m_style_id(other.m_style_id),
   m_og(other.m_og),
   m_fg(other.m_fg),
   m_cacheOnly(other.m_cacheOnly)
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
   if( ins == nullptr || !(instructions().contains(ins)) )
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

Instruction* Recipe::saltWater(Salt::WhenToAdd when)
{
   Instruction *ins;
   QString str,tmp;
   QStringList reagents;
   int i;

   if ( mash() == nullptr || salts().size() == 0 )
      return nullptr;

   reagents = getReagents(salts(), when);
   if ( reagents.size() == 0 )
      return nullptr;

   ins = Database::instance().newInstruction(this);
   tmp = when == Salt::MASH ? tr("mash") : tr("sparge");
   ins->setName(tr("Modify %1 water").arg( tmp ));
   str = tr("Dissolve ");

   for( i = 0; i < reagents.size(); ++i )
      str += reagents.at(i);

   str += QString(tr(" into the %1 water").arg(tmp));
   ins->setDirections(str);
   return ins;
}

Instruction* Recipe::mashWaterIns()
{
   Instruction* ins;
   QString str, tmp;
   int i;

   if( mash() == nullptr )
      return nullptr;

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

   if( mash() == nullptr )
      return preins;

   QList<MashStep*> msteps = mash()->mashSteps();
   for( i = 0; i < size; ++i )
   {
      mstep = msteps[static_cast<int>(i)];

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
      Hop* hop = hlist[static_cast<int>(i)];
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

         str = str.arg(Brewtarget::displayAmount(hop->amount_kg(), kHopTableSection, kpropAmountKg, Units::kilograms))
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
      Misc* misc = mlist[static_cast<int>(i)];
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

         kindOf = misc->amountIsWeight() ? static_cast<Unit*>(Units::kilograms) : static_cast<Unit*>(Units::liters);
         str = str .arg(Brewtarget::displayAmount(misc->amount(), kMiscTableSection, kpropAmount, kindOf))
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
   return nullptr;
}

Instruction* Recipe::topOffIns()
{
   double wortInBoil_l = 0.0;
   QString str,tmp;
   Instruction* ins;

   Equipment* e = equipment();
   if( e != nullptr )
   {
      wortInBoil_l = wortFromMash_l() - e->lauterDeadspace_l();
      str = tr("You should now have %1 wort.")
         .arg(Brewtarget::displayAmount( wortInBoil_l, kTabRecipeSection, kBoilSizeAttr, Units::liters));
      if ( e->topUpKettle_l() != 0.0 )
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
   return nullptr;
}

bool Recipe::hasBoilFermentable()
{
   int i;
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
   int i;
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
   int i;
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
          .arg(Brewtarget::displayAmount(ferm->amount_kg(), kFermentableTableSection, kpropAmountKg, Units::kilograms))
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
   int i;
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
            .arg(Brewtarget::displayAmount(ferm->amount_kg(), kFermentableTableSection, kpropAmountKg, Units::kilograms))
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
   int i;
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
             .arg(Brewtarget::displayAmount(ferm->amount_kg(), kFermentableTableSection, kpropAmountKg, Units::kilograms))
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
      return nullptr;
   }
}

Instruction* Recipe::postboilIns()
{
   QString str;
   Instruction* ins;
   double wort_l = 0.0;
   double wortInBoil_l = 0.0;

   Equipment* e = equipment();
   if( e != nullptr )
   {
      wortInBoil_l = wortFromMash_l() - e->lauterDeadspace_l();
      if ( e->topUpKettle_l() != 0.0 )
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
      return nullptr;
   }
}

void Recipe::addPreinstructions( QVector<PreInstruction> preins )
{
   unsigned int i;
   Instruction* ins;

    // Add instructions in descending mash time order.
    std::sort( preins.begin(), preins.end(), std::greater<PreInstruction>() );
    for( i=0; static_cast<int>(i) < preins.size(); ++i )
    {
       PreInstruction pi = preins[static_cast<int>(i)];
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

   size = (mash() == nullptr) ? 0 : static_cast<unsigned int>(mash()->mashSteps().size());
   if( size > 0 )
   {
     /*** prepare mashed fermentables ***/
     mashFermentableIns();

     /*** salt the water ***/
     saltWater(Salt::MASH);
     saltWater(Salt::SPARGE);

     /*** Prepare water additions ***/
     mashWaterIns();

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
   if( equipment() != nullptr ) {
      timeRemaining = equipment()->boilTime_min();
   }
   else {
      timeRemaining = Brewtarget::qStringToSI(QInputDialog::getText(nullptr,
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
      Yeast* yeast = ylist[static_cast<int>(i)];
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
               .arg(Brewtarget::displayAmount(h->amount_kg(), kHopTableSection, kpropAmountKg, Units::kilograms))
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
            ret = ret.arg(Brewtarget::displayAmount(m->amount(), kMiscTableSection, kpropAmount, Units::kilograms));
         else
            ret = ret.arg(Brewtarget::displayAmount(m->amount(), kMiscTableSection, kpropAmount,  Units::liters));

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

void Recipe::addSalt( Salt* var )
{
   Database::instance().addToRecipe( this, var );
}

//==============================="SET" METHODS=================================
void Recipe::setType( const QString &var )
{
   QString tmp;
   if ( ! isValidType(var) ) {
      Brewtarget::logW( QString("Recipe: invalid type: %1").arg(var) );
      tmp = "All Grain";
   }
   else {
      tmp = QString(var);
   }
   m_type = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropType, tmp );
   }
}

void Recipe::setBrewer( const QString &var )
{
   m_brewer = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropBrewer, var );
   }
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

   m_batchSize_l = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropBatchSize, tmp );
   }

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

   m_boilSize_l = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropBoilSize, tmp );
   }

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

   m_boilTime_min = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropBoilTime, tmp);
   }
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

   m_efficiency_pct = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropEffPct, tmp );
   }

   // If you change the efficency, you really should recalc. And I'm afraid it
   // means recalc all, since og and fg will change, which means your ratios
   // change
   recalcAll();
}

void Recipe::setAsstBrewer( const QString &var )
{
   m_asstBrewer = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAsstBrewer, var );
   }
}

void Recipe::setNotes( const QString &var )
{
   m_notes = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropNotes, var );
   }
}

void Recipe::setTasteNotes( const QString &var )
{
   m_tasteNotes = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropTasteNotes, var );
   }
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

   m_tasteRating = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropTasteRating, tmp );
   }
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

   m_og = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropOG, tmp );
   }
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

   m_fg = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropFG, tmp );
   }
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

   m_fermentationStages = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropFermStages, tmp );
   }
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

   m_primaryAge_days = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropPrimAgeDays, tmp );
   }
}

void Recipe::setPrimaryTemp_c( double var )
{
   m_primaryTemp_c = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropPrimTemp, var );
   }
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

   m_secondaryAge_days = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropSecAgeDays, tmp );
   }
}

void Recipe::setSecondaryTemp_c( double var )
{
   m_secondaryTemp_c = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropSecTemp, var );
   }
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

   m_tertiaryAge_days = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropTertAgeDays, tmp );
   }
}

void Recipe::setTertiaryTemp_c( double var )
{
   m_tertiaryTemp_c = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropTertTemp, var );
   }
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

   m_age = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAge, tmp );
   }
}

void Recipe::setAgeTemp_c( double var )
{
   m_ageTemp_c = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAgeTemp, var );
   }
}

void Recipe::setDate( const QDate &var )
{
   m_date = var;
   // do not like this. I thought we had everything in ISO format
   if ( ! m_cacheOnly ) {
      static const QString dateFormat("d/M/yyyy");
      setEasy(kpropDate, var.toString(dateFormat) );
   }
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

   m_carbonation_vols = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropCarbVols, tmp );
   }
}

void Recipe::setForcedCarbonation( bool var )
{
   m_forcedCarbonation = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropForcedCarb, var );
   }
}

void Recipe::setPrimingSugarName( const QString &var )
{
   m_primingSugarName = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropPrimSugName, var );
   }
}

void Recipe::setCarbonationTemp_c( double var )
{
   m_carbonationTemp_c = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropCarbTemp, var );
   }
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

   m_primingSugarEquiv = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropPrimSugEquiv, tmp );
   }
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

   m_kegPrimingFactor = tmp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropKegPrimFact, tmp );
   }
}

void Recipe::setCacheOnly( bool cache ) { m_cacheOnly = cache; }

//==========================Calculated Getters============================

double Recipe::og()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_og;
}

double Recipe::fg()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_fg;
}

double Recipe::color_srm()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_color_srm;
}

double Recipe::ABV_pct()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_ABV_pct;
}

double Recipe::IBU()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_IBU;
}

QList<double> Recipe::IBUs()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_ibus;
}

double Recipe::boilGrav()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_boilGrav;
}

double Recipe::calories12oz()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_calories;
}

double Recipe::calories33cl()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_calories *3.3/3.55;
}

double Recipe::wortFromMash_l()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_wortFromMash_l;
}

double Recipe::boilVolume_l()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_boilVolume_l;
}

double Recipe::postBoilVolume_l()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_postBoilVolume_l;
}

double Recipe::finalVolume_l()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_finalVolume_l;
}

QColor Recipe::SRMColor()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_SRMColor;
}

double Recipe::grainsInMash_kg()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_grainsInMash_kg;
}

double Recipe::grains_kg()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return m_grains_kg;
}

double Recipe::points()
{
   if( m_uninitializedCalcs )
      recalcAll();
   return (m_og -1.0)*1e3;
}

//=========================Relational Getters=============================
Style* Recipe::style()
{
   Style *tmp;
   if ( m_style_id != 0 ) {
      tmp =  Database::instance().styleById(m_style_id);
   }
   else {
      tmp = Database::instance().style(this);
      if ( tmp ) {
         m_style_id = tmp->key();
      }
      else {
         m_style_id = 0;
      }
   }
   return tmp;
}

// I wonder if we could cache any of this. It is an awful lot of back and forth to the db
Mash* Recipe::mash() const { return Database::instance().mash( this ); }
Equipment* Recipe::equipment() const { return Database::instance().equipment(this); }

QList<Instruction*> Recipe::instructions() const { return Database::instance().instructions(this); }
QList<BrewNote*> Recipe::brewNotes() const { return Database::instance().brewNotes(this); }
QList<Hop*> Recipe::hops() const { return Database::instance().hops(this); }
QList<Fermentable*> Recipe::fermentables() const { return Database::instance().fermentables(this); }
QList<Misc*> Recipe::miscs() const { return Database::instance().miscs(this); }
QList<Yeast*> Recipe::yeasts() const { return Database::instance().yeasts(this); }
QList<Water*> Recipe::waters() const { return Database::instance().waters(this); }
QList<Salt*> Recipe::salts() const { return Database::instance().salts(this); }

//==============================Getters===================================
QString Recipe::type() const { return m_type; }
QString Recipe::brewer() const { return m_brewer; }
QString Recipe::asstBrewer() const { return m_asstBrewer; }
QString Recipe::notes() const { return m_notes; }
QString Recipe::tasteNotes() const { return m_tasteNotes; }
QString Recipe::primingSugarName() const { return m_primingSugarName; }
bool Recipe::forcedCarbonation() const { return m_forcedCarbonation; }
double Recipe::batchSize_l() const { return m_batchSize_l; }
double Recipe::boilSize_l() const { return m_boilSize_l; }
double Recipe::boilTime_min() const { return m_boilTime_min; }
double Recipe::efficiency_pct() const { return m_efficiency_pct; }
double Recipe::tasteRating() const { return m_tasteRating; }
double Recipe::primaryAge_days() const { return m_primaryAge_days; }
double Recipe::primaryTemp_c() const { return m_primaryTemp_c; }
double Recipe::secondaryAge_days() const { return m_secondaryAge_days; }
double Recipe::secondaryTemp_c() const { return m_secondaryTemp_c; }
double Recipe::tertiaryAge_days() const { return m_tertiaryAge_days; }
double Recipe::tertiaryTemp_c() const { return m_tertiaryTemp_c; }
double Recipe::age_days() const { return m_age; }
double Recipe::ageTemp_c() const { return m_ageTemp_c; }
double Recipe::carbonation_vols() const { return m_carbonation_vols; }
double Recipe::carbonationTemp_c() const { return m_carbonationTemp_c; }
double Recipe::primingSugarEquiv() const { return m_primingSugarEquiv; }
double Recipe::kegPrimingFactor() const { return m_kegPrimingFactor; }
int Recipe::fermentationStages() const { return m_fermentationStages; }
QDate Recipe::date() const { return m_date; }
bool Recipe::cacheOnly() const { return m_cacheOnly; }

//=============================Removers========================================

// Returns true if var is found and removed.
void Recipe::remove( Ingredient *var )
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
   if( ! m_recalcMutex.tryLock() )
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

   m_uninitializedCalcs = false;

   m_recalcMutex.unlock();
}

void Recipe::recalcABV_pct()
{
   double ret;

   // The complex formula, and variations comes from Ritchie Products Ltd, (Zymurgy, Summer 1995, vol. 18, no. 2)
   // Michael L. Hall’s article Brew by the Numbers: Add Up What’s in Your Beer, and Designing Great Beers by Daniels.
   ret = (76.08 * (m_og_fermentable - m_fg_fermentable) / (1.775 - m_og_fermentable)) * (m_fg_fermentable / 0.794);

   if ( ! qFuzzyCompare(ret,m_ABV_pct ) ) {
      m_ABV_pct = ret;
      if (!m_uninitializedCalcs)
      {
        emit changed( metaProperty("ABV_pct"), m_ABV_pct );
      }
   }
}

void Recipe::recalcColor_srm()
{
   Fermentable *ferm;
   double mcu = 0.0;
   double ret;
   int i;

   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i )
   {
      ferm = ferms[i];
      // Conversion factor for lb/gal to kg/l = 8.34538.
      mcu += ferm->color_srm()*8.34538 * ferm->amount_kg()/ m_finalVolumeNoLosses_l;
   }

   ret = ColorMethods::mcuToSrm(mcu);

   if ( ! qFuzzyCompare(m_color_srm, ret ) ) {
      m_color_srm = ret;
      if (!m_uninitializedCalcs)
      {
        emit changed( metaProperty("color_srm"), m_color_srm );
      }
   }

}

void Recipe::recalcIBU()
{
   int i;
   double ibus = 0.0;
   double tmp = 0.0;

   // Bitterness due to hops...
   m_ibus.clear();
   QList<Hop*> hhops = hops();
   for( i = 0; i < hhops.size(); ++i ) {
      tmp = ibuFromHop(hhops[i]);
      m_ibus.append(tmp);
      ibus += tmp;
   }

   // Bitterness due to hopped extracts...
   QList<Fermentable*> ferms = fermentables();
   for( i = 0; static_cast<int>(i) < ferms.size(); ++i ) {
      // Conversion factor for lb/gal to kg/l = 8.34538.
      ibus +=
              ferms[i]->ibuGalPerLb() *
              (ferms[i]->amount_kg() / batchSize_l()) / 8.34538;
   }

   if ( ! qFuzzyCompare(ibus, m_IBU ) ) {
      m_IBU = ibus;
      if (!m_uninitializedCalcs)
      {
        emit changed( metaProperty("IBU"), m_IBU );
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
   if( mash() == nullptr ) {
      m_wortFromMash_l = 0.0;
   }
   else
   {
      waterAdded_l = mash()->totalMashWater_l();
      if( equipment() != nullptr )
         absorption_lKg = equipment()->grainAbsorption_LKg();
      else
         absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;

      tmp_wfm = (waterAdded_l - absorption_lKg * m_grainsInMash_kg);
   }

   // boilVolume_l ==============================

   if( equipment() != nullptr )
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
   m_finalVolumeNoLosses_l = batchSizeNoLosses_l();
   if( equipment() != nullptr )
   {
      //_finalVolumeNoLosses_l = equipment()->wortEndOfBoil_l(tmp_bv) + equipment()->topUpWater_l();
      tmp_fv = equipment()->wortEndOfBoil_l(tmp_bv) + equipment()->topUpWater_l() - equipment()->trubChillerLoss_l();
   }
   else
   {
        m_finalVolume_l = tmp_bv - 4.0; // This is just shooting in the dark. Can't do much without an equipment.
      //_finalVolumeNoLosses_l = _finalVolume_l;
   }

   // postBoilVolume_l ===========================

   if( equipment() != nullptr ) {
      tmp_pbv = equipment()->wortEndOfBoil_l( tmp_bv );
   }
   else {
      tmp_pbv = batchSize_l(); // Give up.
   }

   if ( ! qFuzzyCompare(tmp_wfm, m_wortFromMash_l ) ) {
      m_wortFromMash_l = tmp_wfm;
      if (!m_uninitializedCalcs) {
        emit changed( metaProperty("wortFromMash_l"), m_wortFromMash_l );
      }
   }

   if ( ! qFuzzyCompare(tmp_bv, m_boilVolume_l ) ) {
        m_boilVolume_l = tmp_bv;
      if (!m_uninitializedCalcs) {
        emit changed( metaProperty("boilVolume_l"), m_boilVolume_l );
      }
   }

   if ( ! qFuzzyCompare(tmp_fv, m_finalVolume_l ) ) {
       m_finalVolume_l = tmp_fv;
      if (!m_uninitializedCalcs) {
        emit changed( metaProperty("finalVolume_l"), m_finalVolume_l );
      }
   }

   if ( ! qFuzzyCompare(tmp_pbv, m_postBoilVolume_l ) ) {
      m_postBoilVolume_l = tmp_pbv;
      if (!m_uninitializedCalcs) {
        emit changed( metaProperty("postBoilVolume_l"), m_postBoilVolume_l );
      }
   }
}

void Recipe::recalcGrainsInMash_kg()
{
   int i, size;
   double ret = 0.0;
   Fermentable* ferm;

   QList<Fermentable*> ferms = fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
   {
      ferm = ferms[i];

      if( ferm->type() == Fermentable::Grain && ferm->isMashed() )
      {
         ret += ferm->amount_kg();
      }
   }

   if ( ! qFuzzyCompare(ret, m_grainsInMash_kg )  ) {
      m_grainsInMash_kg = ret;
      if (!m_uninitializedCalcs) {
        emit changed( metaProperty("grainsInMash_kg"), m_grainsInMash_kg );
      }
   }
}

void Recipe::recalcGrains_kg()
{
   int i, size;
   double ret = 0.0;

   QList<Fermentable*> ferms = fermentables();
   size = ferms.size();
   for( i = 0; i < size; ++i )
      ret += ferms[i]->amount_kg();

   if ( ! qFuzzyCompare(ret, m_grains_kg ) ) {
      m_grains_kg = ret;
      if (!m_uninitializedCalcs) {
        emit changed( metaProperty("grains_kg"), m_grains_kg );
      }
   }
}

void Recipe::recalcSRMColor()
{
   QColor tmp = Algorithms::srmToColor(m_color_srm);

   if ( tmp != m_SRMColor )
   {
      m_SRMColor = tmp;
      if (!m_uninitializedCalcs)
      {
        emit changed( metaProperty("SRMColor"), m_SRMColor );
      }
   }
}

// the formula in here are taken from http://hbd.org/ensmingr/
void Recipe::recalcCalories()
{
   double startPlato, finishPlato, RE, abw, oog, ffg, tmp;

   oog = m_og;
   ffg = m_fg;

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
   if ( tmp < 0 ) {
      tmp = 0;
   }

   if ( ! qFuzzyCompare(tmp, m_calories ) ) {
      m_calories = tmp;
      if (!m_uninitializedCalcs) {
        emit changed( metaProperty("calories"), m_calories );
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
   double nonFermentableSugars_kg    = 0.0;
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
           nonFermentableSugars_kg += ferm->equivSucrose_kg();
      }
      else
      {
         sugar_kg += ferm->equivSucrose_kg();

         if (ferm->addAfterBoil())
            lateAddition_kg += ferm->equivSucrose_kg();
      }
   }

   ret.insert("sugar_kg", sugar_kg);
   ret.insert("nonFermentableSugars_kg", nonFermentableSugars_kg);
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

   if ( ! qFuzzyCompare(ret, m_boilGrav ) ) {
      m_boilGrav = ret;
      if (!m_uninitializedCalcs)
      {
        emit changed( metaProperty("boilGrav"), m_boilGrav );
      }
   }
}

void Recipe::recalcOgFg()
{
   int i;
   double plato;
   double sugar_kg = 0;
   double sugar_kg_ignoreEfficiency = 0.0;
   double nonFermentableSugars_kg = 0.0;
   double kettleWort_l = 0.0;
   double postBoilWort_l = 0.0;
   double ratio = 0.0;
   double ferm_kg = 0.0;
   double attenuation_pct = 0.0;
   double tmp_og, tmp_fg, tmp_pnts, tmp_ferm_pnts, tmp_nonferm_pnts;
   Yeast* yeast;
   QHash<QString,double> sugars;

   m_og_fermentable = m_fg_fermentable = 0.0;

   // The first time through really has to get the _og and _fg from the
   // database, not use the initialized values of 1. I (maf) tried putting
   // this in the initialize, but it just hung. So I moved it here, but only
   // if if we aren't initialized yet.
   //
   // GSG: This doesn't work, this og and fg are already set to 1.0 so
   // until we load these values from the database on startup, we have
   // to calculate.
   if ( m_uninitializedCalcs )
   {
      m_og = Brewtarget::toDouble(this, kpropOG, "Recipe::recalcOgFg()");
      m_fg = Brewtarget::toDouble(this, kpropFG, "Recipe::recalcOgFg()");
   }

   // Find out how much sugar we have.
   sugars = calcTotalPoints();
   sugar_kg                  = sugars.value("sugar_kg");  // Mass of sugar that *is* affected by mash efficiency
   sugar_kg_ignoreEfficiency = sugars.value("sugar_kg_ignoreEfficiency");  // Mass of sugar that *is not* affected by mash efficiency
   nonFermentableSugars_kg    = sugars.value("nonFermentableSugars_kg");  // Mass of sugar that is not fermentable (also counted in sugar_kg_ignoreEfficiency)

   // We might lose some sugar in the form of Trub/Chiller loss and lauter deadspace.
   if( equipment() != nullptr ) {

      kettleWort_l = (m_wortFromMash_l - equipment()->lauterDeadspace_l()) + equipment()->topUpKettle_l();
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
      if ( nonFermentableSugars_kg != 0.0 )
         nonFermentableSugars_kg *= ratio;
   }

   // Total sugars after accounting for efficiency and mash losses. Implicitly includes non-fermentable sugars
   sugar_kg = sugar_kg * efficiency_pct()/100.0 + sugar_kg_ignoreEfficiency;
   plato = Algorithms::getPlato( sugar_kg, m_finalVolumeNoLosses_l);

   tmp_og = Algorithms::PlatoToSG_20C20C( plato );  // og from all sugars
   tmp_pnts = (tmp_og-1)*1000.0;  // points from all sugars
   if ( nonFermentableSugars_kg != 0.0 )
   {
      ferm_kg = sugar_kg - nonFermentableSugars_kg;  // Mass of only fermentable sugars
      plato = Algorithms::getPlato( ferm_kg, m_finalVolumeNoLosses_l);  // Plato from fermentable sugars
      m_og_fermentable = Algorithms::PlatoToSG_20C20C( plato );  // og from only fermentable sugars
      plato = Algorithms::getPlato( nonFermentableSugars_kg, m_finalVolumeNoLosses_l);  // Plate from non-fermentable sugars
      tmp_nonferm_pnts = ((Algorithms::PlatoToSG_20C20C( plato ))-1)*1000.0;  // og points from non-fermentable sugars
   }
   else
   {
      m_og_fermentable = tmp_og;
      tmp_nonferm_pnts = 0;
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
   // This means we have yeast, but they neglected to provide attenuation percentages.
   if( yeasties.size() > 0 && attenuation_pct <= 0.0 )  {
      attenuation_pct = 75.0; // 75% is an average attenuation.
   }

   if ( nonFermentableSugars_kg != 0.0 )
   {
      tmp_ferm_pnts = (tmp_pnts-tmp_nonferm_pnts) * (1.0 - attenuation_pct/100.0);  // fg points from fermentable sugars
      tmp_pnts = tmp_ferm_pnts + tmp_nonferm_pnts;  // FG points from both fermentable and non-fermentable sugars
      //tmp_pnts *= (1.0 - attenuation_pct/100.0);  // WTF, this completely ignores all the calculations about non-fermentable sugars and just converts everything!
      tmp_fg =  1 + tmp_pnts/1000.0;  // new FG value
      m_fg_fermentable =  1 + tmp_ferm_pnts/1000.0;  // FG from fermentables only
   }
   else
   {
      tmp_pnts *= (1.0 - attenuation_pct/100.0);
      tmp_fg =  1 + tmp_pnts/1000.0;
      m_fg_fermentable = tmp_fg;
   }

   if ( ! qFuzzyCompare(m_og, tmp_og ) ) {
      m_og     = tmp_og;
      // NOTE: We don't want to do this on the first load of the recipe.
      // NOTE: We are we recalculating all of these on load? Shouldn't we be
      // reading these values from the database somehow?
      //
      // GSG: Yes we can, but until the code is added to intialize these calculated
      // values from the database, we can calculate them on load. They should be
      // the same as the database values since the database values were set with
      // these functions in the first place.
      if (!m_uninitializedCalcs)
      {
        setEasy(kpropOG, m_og, false );
        emit changed( metaProperty(kpropOG), m_og );
        emit changed( metaProperty(kpropPoints), (m_og-1.0)*1e3 );
      }
   }

   if ( ! qFuzzyCompare(tmp_fg, m_fg ) ) {
      m_fg     = tmp_fg;
      if (!m_uninitializedCalcs)
      {
        setEasy(kpropFG, m_fg, false );
        emit changed( metaProperty(kpropFG), m_fg );
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

   if( hop == nullptr )
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

   if( equip ) {
      hopUtilization = equip->hopUtilization_pct() / 100.0;
      boilTime = static_cast<int>(equip->boilTime_min());
   }

   if( hop->use() == Hop::Boil)
      ibus = IbuMethods::getIbus( AArating, grams, m_finalVolumeNoLosses_l, m_og, minutes );
   else if( hop->use() == Hop::First_Wort )
      ibus = fwhAdjust * IbuMethods::getIbus( AArating, grams, m_finalVolumeNoLosses_l, m_og, boilTime );
   else if( hop->use() == Hop::Mash && mashHopAdjust > 0.0 )
      ibus = mashHopAdjust * IbuMethods::getIbus( AArating, grams, m_finalVolumeNoLosses_l, m_og, boilTime );

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

// this was fixed, but not with an at
bool Recipe::isValidType( const QString &str )
{
   QStringList types = QStringList() << "Extract" << "Partial Mash" << "All Grain";

   return types.contains(str);
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
                  .arg(Brewtarget::displayAmount(ferms[i]->amount_kg(), kFermentableTableSection, kpropAmountKg, Units::kilograms))
                  .arg(ferms[i]->name());
         }
         else
         {
            tmp = QString("%1 %2 ")
                  .arg(Brewtarget::displayAmount(ferms[i]->amount_kg(), kFermentableTableSection, kpropAmountKg,  Units::kilograms))
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
      if( firstWort && (hops[i]->use() == Hop::First_Wort) ) {
         tmp = QString("%1 %2,")
               .arg(Brewtarget::displayAmount(hops[i]->amount_kg(), kHopTableSection, kpropAmountKg,  Units::kilograms))
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

//! \brief send me a list of salts and if we are wanting to add to the
//! mash or the sparge, and I will return a list of instructions
QStringList Recipe::getReagents( QList<Salt*> salts, Salt::WhenToAdd wanted)
{
   QString tmp;
   QStringList reagents = QStringList();

   for ( int i = 0; i < salts.size(); ++i )
   {
      Salt::WhenToAdd what = salts[i]->addTo();
      Unit* rightUnit = salts[i]->amountIsWeight() ? static_cast<Unit*>(Units::kilograms): static_cast<Unit*>(Units::liters);
      if ( what == wanted ) {
         tmp = tr("%1 %2, ")
               .arg(Brewtarget::displayAmount(salts[i]->amount(), kSaltTableSection, kSaltAmountAttr, rightUnit))
               .arg(salts[i]->name());
      }
      else if ( what == Salt::EQUAL ) {
         tmp = tr("%1 %2, ")
               .arg(Brewtarget::displayAmount(salts[i]->amount(), kSaltTableSection, kSaltAmountAttr, rightUnit))
               .arg(salts[i]->name());
      }
      else if ( what == Salt::RATIO ) {
         double ratio = 1.0;
         if ( wanted == Salt::SPARGE )
            ratio = mash()->totalSpargeAmount_l()/mash()->totalInfusionAmount_l();
         double amt = salts[i]->amount() * ratio;
         tmp = tr("%1 %2, ")
               .arg(Brewtarget::displayAmount(amt, kSaltTableSection, kSaltAmountAttr, rightUnit))
               .arg(salts[i]->name());
      }
      else {
         continue;
      }
      reagents.append(tmp);
   }
   // How many ways can we remove the trailing ", " because it really, really
   // annoys me?
   if ( reagents.size() > 0 ) {
      QString fixin = reagents.takeLast();
      fixin.remove( fixin.lastIndexOf(","), 2);
      reagents.append(fixin);
   }
   return reagents;
}

//==========================Accept changes from ingredients====================

void Recipe::acceptEquipChange(QMetaProperty prop, QVariant val) {
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

   if ( mashSend == nullptr )
      return;

   recalcAll();
}

void Recipe::acceptMashChange(Mash* newMash)
{
   if ( newMash == mash() )
      recalcAll();
}

double Recipe::targetCollectedWortVol_l()
{

   // Need to account for extract/sugar volume also.
   float postMashAdditionVolume_l = 0;

   QList<Fermentable*> ferms = fermentables();
   foreach( Fermentable* f, ferms ) {
      Fermentable::Type type = f->type();
      if ( type == Fermentable::Extract ) {
         postMashAdditionVolume_l  += static_cast<float>(f->amount_kg() / PhysicalConstants::liquidExtractDensity_kgL);
      }
      else if ( type == Fermentable::Sugar ) {
         postMashAdditionVolume_l  += static_cast<float>(f->amount_kg() / PhysicalConstants::sucroseDensity_kgL);
      }
      else if ( type == Fermentable::Dry_Extract ) {
         postMashAdditionVolume_l  += static_cast<float>(f->amount_kg() / PhysicalConstants::dryExtractDensity_kgL);
      }
   }

   if ( equipment() ) {
      return boilSize_l() - equipment()->topUpKettle_l() - static_cast<double>(postMashAdditionVolume_l);
   }
   else {
      return boilSize_l() - static_cast<double>(postMashAdditionVolume_l);
   }
}

double Recipe::targetTotalMashVol_l()
{

   double absorption_lKg;

   if( equipment() ) {
      absorption_lKg = equipment()->grainAbsorption_LKg();
   }
   else {
      absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;
   }


   return targetCollectedWortVol_l() + absorption_lKg * grainsInMash_kg();
}
