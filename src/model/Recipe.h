/*
 * model/Recipe.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef MODEL_RECIPE_H
#define MODEL_RECIPE_H

#include <memory> // For PImpl

#include <QColor>
#include <QDate>
#include <QList>
#include <QMutex>
#include <QSqlRecord>
#include <QString>
#include <QVariant>

#include "model/BrewNote.h"
#include "model/NamedEntity.h"
#include "model/Hop.h" // Dammit! Have to include these for Hop::Use (see hopSteps()) and Misc::Use (see miscSteps()).
#include "model/Misc.h"
#include "model/Salt.h"  // Needed for Salt::WhenToAdd (see getReagents())

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::Recipe {static char const * const property = #property; }
AddPropertyName(ABV_pct)
AddPropertyName(age)
AddPropertyName(ageTemp_c)
AddPropertyName(ancestorId)
AddPropertyName(asstBrewer)
AddPropertyName(batchSize_l)
AddPropertyName(boilGrav)
AddPropertyName(boilSize_l)
AddPropertyName(boilTime_min)
AddPropertyName(boilVolume_l)
AddPropertyName(brewer)
AddPropertyName(brewNotes)
AddPropertyName(calories)
AddPropertyName(carbonationTemp_c)
AddPropertyName(carbonation_vols)
AddPropertyName(color_srm)
AddPropertyName(date)
AddPropertyName(efficiency_pct)
AddPropertyName(equipment)
AddPropertyName(equipmentId)
AddPropertyName(fermentableIds)
AddPropertyName(fermentables)
AddPropertyName(fermentationStages)
AddPropertyName(fg)
AddPropertyName(finalVolume_l)
AddPropertyName(forcedCarbonation)
AddPropertyName(grainsInMash_kg)
AddPropertyName(grains_kg)
AddPropertyName(hopIds)
AddPropertyName(hops)
AddPropertyName(IBU)
AddPropertyName(IBUs)
AddPropertyName(instructionIds)
AddPropertyName(instructions)
AddPropertyName(kegPrimingFactor)
AddPropertyName(locked)
AddPropertyName(mash)
AddPropertyName(mashId)
AddPropertyName(miscIds)
AddPropertyName(miscs)
AddPropertyName(notes)
AddPropertyName(og)
AddPropertyName(points)
AddPropertyName(postBoilVolume_l)
AddPropertyName(primaryAge_days)
AddPropertyName(primaryTemp_c)
AddPropertyName(primingSugarEquiv)
AddPropertyName(primingSugarName)
AddPropertyName(recipeType)
AddPropertyName(saltIds)
AddPropertyName(secondaryAge_days)
AddPropertyName(secondaryTemp_c)
AddPropertyName(SRMColor)
AddPropertyName(style)
AddPropertyName(styleId)
AddPropertyName(tasteNotes)
AddPropertyName(tasteRating)
AddPropertyName(tertiaryAge_days)
AddPropertyName(tertiaryTemp_c)
AddPropertyName(type)
AddPropertyName(waterIds)
AddPropertyName(waters)
AddPropertyName(wortFromMash_l)
AddPropertyName(yeastIds)
AddPropertyName(yeasts)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


// Forward declarations
class Equipment;
class Fermentable;
class Instruction;
class Mash;
class MashStep;
class PreInstruction;
class Style;
class Water;
class Yeast;


/*!
 * \class Recipe
 *
 * \brief Model class for recipe records in the database.
 */
class Recipe : public NamedEntity {
   Q_OBJECT
   Q_CLASSINFO("signal", "recipes")


   friend class RecipeFormatter;
   friend class MainWindow;
   friend class WaterDialog;
public:

   Recipe(QString name, bool cache = true);
   Recipe(NamedParameterBundle const & namedParameterBundle);
   Recipe(Recipe const & other);

   virtual ~Recipe();

    //! \brief the user can select what delete means
   enum delOptions {
      ANCESTOR,   // delete the recipe and all its ancestors
      DESCENDANT  // delete only the recipe (orphan and delete)
   };

   //! \brief The type of recipe
   enum Type { Extract, PartialMash, AllGrain };
   Q_ENUMS(Type)

   //! \brief The \b Type
   Q_PROPERTY(Type recipeType READ recipeType WRITE setRecipeType /*NOTIFY changed*/ /*changedType*/)

   //! \brief The type (extract, partial mash, all grain) stored as a string
   //         TBD (MY 2021-01-18) Not sure why this is stored as a string rather than an enum.  Have created an enum wrapper above
   Q_PROPERTY(QString type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/)
   //! \brief The brewer.
   Q_PROPERTY(QString brewer READ brewer WRITE setBrewer /*NOTIFY changed*/ /*changedBrewer*/)
   //! \brief The batch size in liters.
   Q_PROPERTY(double batchSize_l READ batchSize_l WRITE setBatchSize_l /*NOTIFY changed*/ /*changedBatchSize_l*/)
   //! \brief The boil size in liters.
   Q_PROPERTY(double boilSize_l READ boilSize_l WRITE setBoilSize_l /*NOTIFY changed*/ /*changedBoilSize_l*/)
   //! \brief The boil time in minutes.
   Q_PROPERTY(double boilTime_min READ boilTime_min WRITE setBoilTime_min /*NOTIFY changed*/ /*changedBoilTime_min*/)
   //! \brief The overall efficiency in percent.
   Q_PROPERTY(double efficiency_pct READ efficiency_pct WRITE setEfficiency_pct /*NOTIFY changed*/ /*changedEfficiency_pct*/)
   //! \brief The assistant brewer.
   Q_PROPERTY(QString asstBrewer READ asstBrewer WRITE setAsstBrewer /*NOTIFY changed*/ /*changedAsstBrewer*/)
   //! \brief The notes.
   Q_PROPERTY(QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/)
   //! \brief The tasting notes.
   Q_PROPERTY(QString tasteNotes READ tasteNotes WRITE setTasteNotes /*NOTIFY changed*/ /*changedTasteNotes*/)
   //! \brief The taste rating.
   Q_PROPERTY(double tasteRating READ tasteRating WRITE setTasteRating /*NOTIFY changed*/ /*changedTasteRating*/)
   //! \brief The number of fermentation stages.
   Q_PROPERTY(int fermentationStages READ fermentationStages WRITE setFermentationStages /*NOTIFY changed*/ /*changedFermentationStages*/)
   //! \brief How many days in primary.
   Q_PROPERTY(double primaryAge_days READ primaryAge_days WRITE setPrimaryAge_days /*NOTIFY changed*/ /*changedPrimaryAge_days*/)
   //! \brief The temp in C in the primary.
   Q_PROPERTY(double primaryTemp_c READ primaryTemp_c WRITE setPrimaryTemp_c /*NOTIFY changed*/ /*changedPrimaryTemp_c*/)
   //! \brief How many days in secondary.
   Q_PROPERTY(double secondaryAge_days READ secondaryAge_days WRITE setSecondaryAge_days /*NOTIFY changed*/ /*changedSecondaryAge_days*/)
   //! \brief The temp in C in secondary.
   Q_PROPERTY(double secondaryTemp_c READ secondaryTemp_c WRITE setSecondaryTemp_c /*NOTIFY changed*/ /*changedSecondaryTemp_c*/)
   //! \brief How many days in tertiary.
   Q_PROPERTY(double tertiaryAge_days READ tertiaryAge_days WRITE setTertiaryAge_days /*NOTIFY changed*/ /*changedTertiaryAge_days*/)
   //! \brief The temp in C in tertiary.
   Q_PROPERTY(double tertiaryTemp_c READ tertiaryTemp_c WRITE setTertiaryTemp_c /*NOTIFY changed*/ /*changedTertiaryTemp_c*/)
   //! \brief The number of days to age the beer after bottling.
   Q_PROPERTY(double age READ age_days WRITE setAge_days /*NOTIFY changed*/ /*changedAge_days*/)
   //! \brief The temp in C as beer is aging after bottling.
   Q_PROPERTY(double ageTemp_c READ ageTemp_c WRITE setAgeTemp_c /*NOTIFY changed*/ /*changedAgeTemp_c*/)
   //! \brief The date the recipe was created or brewed. I'm not sure yet.
   Q_PROPERTY(QDate date READ date WRITE setDate /*NOTIFY changed*/ /*changedDate*/)
   //! \brief The carbonation in volumes of CO2 at standard temperature and pressure (STP).
   Q_PROPERTY(double carbonation_vols READ carbonation_vols WRITE setCarbonation_vols /*NOTIFY changed*/ /*changedCarbonation_vols*/)
   //! \brief Whether the beer is force carbonated.
   Q_PROPERTY(bool forcedCarbonation READ forcedCarbonation WRITE setForcedCarbonation /*NOTIFY changed*/ /*changedForcedCarbonation*/)
   //! \brief The name of the priming sugar.
   Q_PROPERTY(QString primingSugarName READ primingSugarName WRITE setPrimingSugarName /*NOTIFY changed*/ /*changedPrimingSugarName*/)
   //! \brief The temperature in C while carbonating.
   Q_PROPERTY(double carbonationTemp_c READ carbonationTemp_c WRITE setCarbonationTemp_c /*NOTIFY changed*/ /*changedCarbonationTemp_c*/)
   //! \brief The factor required to convert this priming agent to an equivalent amount of glucose monohyrate.
   Q_PROPERTY(double primingSugarEquiv READ primingSugarEquiv WRITE setPrimingSugarEquiv /*NOTIFY changed*/ /*changedPrimingSugarEquiv*/)
   //! \brief The factor required to convert the amount of sugar required for bottles to keg (usually about 0.5).
   Q_PROPERTY(double kegPrimingFactor READ kegPrimingFactor WRITE setKegPrimingFactor /*NOTIFY changed*/ /*changedKegPrimingFactor*/)
   //! \brief Whether the recipe is locked against changes
   Q_PROPERTY(bool locked READ locked WRITE setLocked /*NOTIFY changed*/ /*changed*/)

   // Calculated stored properties.
   //! \brief The calculated OG.
   Q_PROPERTY(double og READ og WRITE setOg /*NOTIFY changed*/ /*changedOg*/)
   //! \brief The calculated FG.
   Q_PROPERTY(double fg READ fg WRITE setFg /*NOTIFY changed*/ /*changedFg*/)

   // Calculated unstored properties. These need to listen for changes to
   // the uncalculated properties they depend on, and re-emit changed()
   // when appropriate.
   //! \brief The calculated points (1000*(\c og()-1.0)).
   Q_PROPERTY(double points READ points /*WRITE*/ /*NOTIFY changed*/ /*changedPoints*/ STORED false)
   //! \brief The calculated ABV in percent.
   Q_PROPERTY(double ABV_pct READ ABV_pct /*WRITE*/ /*NOTIFY changed*/ /*changedABV*/ STORED false)
   //! \brief The calculated color in SRM.
   Q_PROPERTY(double color_srm READ color_srm /*WRITE*/ /*NOTIFY changed*/ /*changedColor_srm*/ STORED false)
   //! \brief The calculated boil gravity.
   Q_PROPERTY(double boilGrav READ boilGrav /*WRITE*/ /*NOTIFY changed*/ /*changedBoilGrav*/ STORED false)
   //! \brief The calculated IBUs.
   Q_PROPERTY(double IBU READ IBU /*WRITE*/ /*NOTIFY changed*/ /*changedIBU*/)
   //! \brief IBU contributions from each hop.
   Q_PROPERTY(QList<double> IBUs READ IBUs)
   //! \brief The calculated wort coming from the mash in liters.
   Q_PROPERTY(double wortFromMash_l READ wortFromMash_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateWortFromMash_l*/ STORED false)
   //! \brief The calculated preboil volume in liters.
   Q_PROPERTY(double boilVolume_l READ boilVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateBoilVolume_l*/ STORED false)
   //! \brief The calculated postboil volume in liters.
   Q_PROPERTY(double postBoilVolume_l READ postBoilVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimatePostBoilVolume_l*/ STORED false)
   //! \brief The calculated final volume into the primary in liters.
   Q_PROPERTY(double finalVolume_l READ finalVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateFinalVolume_l*/ STORED false)
   //! \brief The calculated Calories per 12 oz. (kcal).
   Q_PROPERTY(double calories READ calories12oz /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateCalories*/ STORED false)
   //! \brief The amount of grains in the mash in kg.
   Q_PROPERTY(double grainsInMash_kg READ grainsInMash_kg /*WRITE*/ /*NOTIFY changed*/ /*changedGrainsInMash_kg*/ STORED false)
   //! \brief The total amount of grains in the recipe in kg.
   Q_PROPERTY(double grains_kg READ grains_kg /*WRITE*/ /*NOTIFY changed*/ /*changedGrains_kg*/ STORED false)
   //! \brief The beer color as a displayable QColor.
   Q_PROPERTY(QColor SRMColor READ SRMColor /*WRITE*/ /*NOTIFY changed*/ STORED false)

   // Relational properties.
   // NB: the setBlahId() calls are needed by ObjectStore and are not intended for more general use.
   //! \brief The mash.
   Q_PROPERTY(Mash * mash   READ mash      WRITE setMash /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(int    mashId READ getMashId WRITE setMashId)

   //! \brief The equipment.
   Q_PROPERTY(Equipment * equipment   READ equipment      WRITE setEquipment /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(int         equipmentId READ getEquipmentId WRITE setEquipmentId)
   //! \brief The style.
   Q_PROPERTY(Style * style   READ style      WRITE setStyle /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(int     styleId READ getStyleId WRITE setStyleId)

   // These QList properties should only emit changed() when their size changes, or when
   // one of their elements is replaced by another with a different key.
   //! \brief The brew notes.
   Q_PROPERTY(QList<BrewNote *> brewNotes READ brewNotes /*WRITE*/ /*NOTIFY changed*/ STORED false)
   //! \brief The hops.
   Q_PROPERTY(QList<Hop *> hops   READ hops /*WRITE*/ /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(QVector<int> hopIds READ getHopIds WRITE setHopIds)
   //! \brief The instructions.
   Q_PROPERTY(QList<Instruction *> instructions   READ instructions /*WRITE*/ /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(QVector<int>         instructionIds READ getInstructionIds WRITE setInstructionIds)
   //! \brief The fermentables.
   Q_PROPERTY(QList<Fermentable *> fermentables   READ fermentables /*WRITE*/ /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(QVector<int>         fermentableIds READ getFermentableIds WRITE setFermentableIds)

   //! \brief The miscs.
   Q_PROPERTY(QList<Misc *> miscs   READ miscs /*WRITE*/ /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(QVector<int>  miscIds READ getMiscIds WRITE setMiscIds)
   //! \brief The yeasts.
   Q_PROPERTY(QList<Yeast *> yeasts   READ yeasts /*WRITE*/ /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(QVector<int>   yeastIds READ getYeastIds WRITE setYeastIds)
   //! \brief The waters.
   Q_PROPERTY(QList<Water *> waters   READ waters /*WRITE*/ /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(QVector<int>   waterIds READ getWaterIds WRITE setWaterIds)
   //! \brief The salts.
   Q_PROPERTY(QList<Salt *> salts   READ salts /*WRITE*/ /*NOTIFY changed*/ STORED false)
   Q_PROPERTY(QVector<int>  saltIds READ getSaltIds WRITE setSaltIds)

   Q_PROPERTY(int    ancestorId READ getAncestorId WRITE setAncestorId)
   //! \brief The ancestors.
   Q_PROPERTY(QList<Recipe *> ancestors READ ancestors /*WRITE*/ /*NOTIFY changed*/ STORED false)

   /**
    * \brief We need to override \c NamedEntity::setKey to do some extra ancestor stuff
    */
   virtual void setKey(int key);

   /**
    * \brief Connect Fermentable, Hop changed signals etc to their parent Recipes.
    *
    *        This is needed because each Recipe needs to know when one of its constituent parts has been modified, eg
    *        if the alpha acid on a hop is modified then that will affect the recipe's IBU.
    *
    *        Needs to be called \b after all the calls to ObjectStoreTyped<FooBar>::getInstance().loadAll()
    */
   static void connectSignals();

   /*!
    * \brief Add a copy of \c var from the recipe and return the copy
    *
    * For many types of ingredient, when we add an ingredient to a recipe, we make a copy of it, and it is the copy that
    * it associated with the recipe.  Amongst other things, this allows the same ingredient to be added multiple times to
    * a recipe - eg the same type of hops might well be added at multiple points in the recipe.  It also allows an
    * ingredient in a recipe to be modified without those modifications affecting the use of the ingredient in other
    * recipes.
    *
    * So, calling "myRecipe->addFermentable(&someFermentable)" will result in a COPY of someFermentable
    * being created and added to the recipe, which means the inverse operation is NOT just
    * myRecipe->removeFermentable(&someFermentable).  Instead, the add function returns a pointer to the
    * newly-created ingredient:
    *
    *    Fermentable * newCopyOfSomeFermentable = myRecipe->add<Fermentable>(&someFermentable);   // DO
    *    myRecipe->remove<Fermentable>(newCopyOfSomeFermentable);                                 // UNDO
    *
    * The remover function returns a pointer to the NamedEntity that it removed.  This is useful because it makes add and
    * remove symmetric and simplifies the implementation of UndoableAddOrRemove.
    *
    * TBD: (MY 2020-11-23) It would be good one day to pull out all the non-changeable aspects of ingredients and keep
    *      just one copy of them in the DB and in memory.
    */
   template<class T> T * add(T * var);

   /*!
    * \brief Remove \c var from the recipe and return what was removed - ie \c var
    *
    * We want callers to use this strongly-typed version because it makes the implementation of Undo/Redo easier (by
    * making add and remove more symmetric).
    */
   template<class T> T * remove(T * var);

   /*!
    * \brief Returns whether \c var is used in this recipe
    */
   template<class T> bool uses(T const & var) const;

   int instructionNumber(Instruction const & ins) const;
   /*!
    * \brief Swap instructions \c ins1 and \c ins2
    */
   void swapInstructions(Instruction * ins1, Instruction * ins2);
   //! \brief Remove all instructions.
   void clearInstructions();
   //! \brief Insert instruction ins into slot pos.
   void insertInstruction(Instruction * ins, int pos);
   //! \brief Automagically generate a list of instructions.
   void generateInstructions();
   /*!
    * Finds the next ingredient to add that has a time
    * less than time. Changes time to be the time of the found
    * ingredient, or if none are found, -1. Returns a string
    * in the form "Add %1 to %2 at %3".
    */
   QString nextAddToBoil(double & time);

   //! \brief convenience method to set ancestors
   void setAncestor(Recipe & ancestor);

   /**
    * \brief Usually called before deleting a Recipe.  Unlinks this Recipe from its its ancestors (aka previous
    *        versions) and set the most recent of these to be editable again.
    * \return The immediately previous version, or \c nullptr if there is none
    */
   Recipe * revertToPreviousVersion();

   // Getters
   Type recipeType() const;
   QString type() const;
   QString brewer() const;
   double batchSize_l() const;
   double boilSize_l() const;
   double boilTime_min() const;
   double efficiency_pct() const;
   QString asstBrewer() const;
   QString notes() const;
   QString tasteNotes() const;
   double tasteRating() const;
   double og();
   double fg();
   int fermentationStages() const;
   double primaryAge_days() const;
   double primaryTemp_c() const;
   double secondaryAge_days() const;
   double secondaryTemp_c() const;
   double tertiaryAge_days() const;
   double tertiaryTemp_c() const;
   double age_days() const;
   double ageTemp_c() const;
   QDate date() const;
   double carbonation_vols() const;
   bool forcedCarbonation() const;
   QString primingSugarName() const;
   double carbonationTemp_c() const;
   double primingSugarEquiv() const;
   double kegPrimingFactor() const;
   bool locked() const;

   // Calculated getters.
   double points();
   double ABV_pct();
   double color_srm();
   double boilGrav();
   double IBU();
   QColor SRMColor();
   double targetCollectedWortVol_l();
   double targetTotalMashVol_l();
   double wortFromMash_l();
   double boilVolume_l();
   double postBoilVolume_l();
   double finalVolume_l();
   double calories12oz();
   double calories33cl();
   double grainsInMash_kg();
   double grains_kg();
   QList<double> IBUs();

   // Relational getters
   QList<Hop *> hops() const;
   QVector<int> getHopIds() const;
   QList<Instruction *> instructions() const;
   QVector<int> getInstructionIds() const;
   QList<Fermentable *> fermentables() const;
   QVector<int> getFermentableIds() const;
   QList<Misc *>  miscs() const;
   QVector<int> getMiscIds() const;
   QList<Yeast *> yeasts() const;
   QVector<int> getYeastIds() const;
   QList<Water *> waters() const;
   QVector<int> getWaterIds() const;
   QList<Salt *>  salts() const;
   QVector<int> getSaltIds() const;
   QList<BrewNote *> brewNotes() const;
   QList<Recipe *> ancestors() const;
   Mash * mash() const;
   int getMashId() const;
   Equipment * equipment() const;
   int getEquipmentId() const;
   Style * style() const;
   int getStyleId() const;

   int getAncestorId() const;

   // Relational setters
   void setEquipment(Equipment * equipment);
   void setMash(Mash * var);
   void setStyle(Style * style);

   //
   // The following calls are intended for use by the ObjectStore when pulling data from the database.  As such they do
   // not do additional work (eg to ensure that an ingredient being added is a child).
   //
   void setEquipmentId(int equipmentId);
   void setMashId(int mashId);
   void setStyleId(int styleId);
   void setFermentableIds(QVector<int> fermentableIds);
   void setHopIds(QVector<int> hopIds);
   void setInstructionIds(QVector<int> instructionIds);
   void setMiscIds(QVector<int> miscIds);
   void setSaltIds(QVector<int> saltIds);
   void setWaterIds(QVector<int> waterIds);
   void setYeastIds(QVector<int> yeastIds);
   void setAncestorId(int ancestorId, bool notify = true);

   // Other junk.
   QVector<PreInstruction> mashInstructions(double timeRemaining, double totalWaterAdded_l, unsigned int size);
   QVector<PreInstruction> mashSteps();
   QVector<PreInstruction> hopSteps(Hop::Use type = Hop::Boil);
   QVector<PreInstruction> miscSteps(Misc::Use type = Misc::Boil);
   PreInstruction boilFermentablesPre(double timeRemaining);
   bool hasBoilFermentable();
   bool hasBoilExtract();
   static bool isFermentableSugar(Fermentable *);
   bool hasAncestors() const;
   bool isMyAncestor(Recipe const & maybe) const;
   bool hasDescendants() const;
   PreInstruction addExtracts(double timeRemaining) const;

   // Helpers
   //! \brief Get the ibus from a given \c hop.
   double ibuFromHop(Hop const * hop);
   //! \brief Formats the fermentables for instructions
   QList<QString> getReagents(QList<Fermentable *> ferms);
   //! \brief Formats the mashsteps for instructions
   QList<QString> getReagents(QList<MashStep *> msteps);
   //! \brief Formats the hops for instructions
   QList<QString> getReagents(QList<Hop *> hops, bool firstWort = false);
   //! \brief Formats the salts for instructions
   QStringList getReagents(QList<Salt *> salts, Salt::WhenToAdd wanted);
   QHash<QString, double> calcTotalPoints();

   // Setters that are not slots
   void setRecipeType(Type var);
   void setType(const QString & var);
   void setBrewer(const QString & var);
   void setBatchSize_l(double var);
   void setBoilSize_l(double var);
   void setBoilTime_min(double var);
   void setEfficiency_pct(double var);
   void setAsstBrewer(const QString & var);
   void setNotes(const QString & var);
   void setTasteNotes(const QString & var);
   void setTasteRating(double var);
   void setOg(double var);
   void setFg(double var);
   void setFermentationStages(int var);
   void setPrimaryAge_days(double var);
   void setPrimaryTemp_c(double var);
   void setSecondaryAge_days(double var);
   void setSecondaryTemp_c(double var);
   void setTertiaryAge_days(double var);
   void setTertiaryTemp_c(double var);
   void setAge_days(double var);
   void setAgeTemp_c(double var);
   void setDate(const QDate & var);
   void setCarbonation_vols(double var);
   void setForcedCarbonation(bool var);
   void setPrimingSugarName(const QString & var);
   void setCarbonationTemp_c(double var);
   void setPrimingSugarEquiv(double var);
   void setKegPrimingFactor(double var);
   void setLocked(bool isLocked);
   void setHasDescendants(bool spawned);

   virtual Recipe * getOwningRecipe();

   /**
    * \brief A Recipe owns some of its contained objects, so needs to delete those if it itself is being deleted
    */
   virtual void hardDeleteOwnedEntities();

signals:

public slots:
   void acceptChangeToContainedObject(QMetaProperty prop, QVariant val);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   // Cached properties that are written directly to db
   QString m_type;
   QString m_brewer;
   QString m_asstBrewer;
   double m_batchSize_l;
   double m_boilSize_l;
   double m_boilTime_min;
   double m_efficiency_pct;
   int m_fermentationStages;
   double m_primaryAge_days;
   double m_primaryTemp_c;
   double m_secondaryAge_days;
   double m_secondaryTemp_c;
   double m_tertiaryAge_days;
   double m_tertiaryTemp_c;
   double m_age;
   double m_ageTemp_c;
   QDate m_date;
   double m_carbonation_vols;
   bool m_forcedCarbonation;
   QString m_primingSugarName;
   double m_carbonationTemp_c;
   double m_primingSugarEquiv;
   double m_kegPrimingFactor;
   QString m_notes;
   QString m_tasteNotes;
   double m_tasteRating;
   int styleId;

   int mashId;
   int equipmentId;

   // Calculated properties.
   double m_ABV_pct;
   double m_color_srm;
   double m_boilGrav;
   double m_IBU;
   QList<double> m_ibus;
   double m_wortFromMash_l;
   double m_boilVolume_l;
   double m_postBoilVolume_l;
   double m_finalVolume_l;
   // Final volume before any losses out of the kettle, used in calculations for sg/ibu/etc.
   double m_finalVolumeNoLosses_l;
   double m_calories;
   double m_grainsInMash_kg;
   double m_grains_kg;
   QColor m_SRMColor;

   // Calculated, but stored...BeerXML is weird sometimes.
   double m_og;
   double m_fg;
   double m_og_fermentable;
   double m_fg_fermentable;

   bool m_locked;

   // True when constructed, indicates whether recalcAll has been called.
   bool m_uninitializedCalcs;
   QMutex m_uninitializedCalcsMutex;
   QMutex m_recalcMutex;

   // version things
   int m_ancestor_id;
   mutable QList<Recipe *> m_ancestors;
   mutable bool m_hasDescendants;

   // Batch size without losses.
   double batchSizeNoLosses_l();

   // Some recalculators for calculated properties.

   void recalcIfNeeded(QString classNameOfWhatWasAddedOrChanged);

   /* Recalculates all the calculated properties.
    *
    * WARNING: this call took 0.15s in rev 916!
    */
   void recalcAll();
   // Emits changed(ABV_pct). Depends on: _og, _fg
   Q_INVOKABLE void recalcABV_pct();
   // Emits changed(color_srm). Depends on: _finalVolume_l
   Q_INVOKABLE void recalcColor_srm();
   // Emits changed(boilGrav). Depends on: _postBoilVolume_l, _boilVolume_l
   Q_INVOKABLE void recalcBoilGrav();
   // Emits changed(IBU). Depends on: _batchSize_l, _boilGrav, _boilVolume_l, _finalVolume_l
   Q_INVOKABLE void recalcIBU();
   // Emits changed(wortFromMash_l), changed(boilVolume_l), changed(finalVolume_l), changed(postBoilVolume_l). Depends on: _grainsInMash_kg
   Q_INVOKABLE void recalcVolumeEstimates();
   // Emits changed(grainsInMash_kg). Depends on: --.
   Q_INVOKABLE void recalcGrainsInMash_kg();
   // Emits changed(grains_kg). Depends on: --.
   Q_INVOKABLE void recalcGrains_kg();
   // Emits changed(SRMColor). Depends on: _color_srm.
   Q_INVOKABLE void recalcSRMColor();
   // Emits changed(calories). Depends on: _og, _fg.
   Q_INVOKABLE void recalcCalories();
   // Emits changed(og), changed(fg). Depends on: _wortFromMash_l, _finalVolume_l
   Q_INVOKABLE void recalcOgFg();

   // Adds instructions to the recipe.
   void postboilFermentablesIns();
   void postboilIns();
   void mashFermentableIns();
   void mashWaterIns();
   void firstWortHopsIns();
   void topOffIns();
   void saltWater(Salt::WhenToAdd when);

   //void setDefaults();
   void addPreinstructions(QVector<PreInstruction> preins);
   bool isValidType(const QString & str);

   /**
    * \brief Add a Hop/Fermentable/Instruction etc that is already in an Object Store and is known not to be used by
    *        any other Recipe.
    */
   template<class NE> NE * add(std::shared_ptr<NE> ne);

   /**
    * \brief Create and add a new Hop/Fermentable/Instruction etc, first to the relevant Object Store and then to this
    *        Recipe
    */
   template<class NE> void addNew(std::shared_ptr<NE> ne);

};

/**
 * \brief Non-member functions for \c Recipe
 */
namespace RecipeHelper {
   /**
    * \brief Gets the BrewNotes for a Recipe and all its ancestors
    */
   QList<BrewNote *> brewNotesForRecipeAndAncestors(Recipe const & recipe);

   /**
    * \brief Checks whether an about-to-be-made property change require us to create a new version of a Recipe - eg
    *        because we are modifying some ingredient or other attribute of the Recipe and automatic versioning is
    *        enabled.
    */
   void prepareForPropertyChange(NamedEntity & ne, char const * const propertyName);

   /**
    * \brief Turn automatic versioning on or off
    */
   void setAutomaticVersioningEnabled(bool enabled);

   /**
    * \brief Returns \c true if automatic versioning is enabled, \c false otherwise
    */
   bool getAutomaticVersioningEnabled();

   /**
    * \brief Mini RAII class that allows automatic Recipe versioning to be suspended for the time that it's in scope
    */
   class SuspendRecipeVersioning {
   public:
      SuspendRecipeVersioning();
      ~SuspendRecipeVersioning();
   private:
      bool savedVersioningValue;
      // RAII class shouldn't be getting copied or moved
      SuspendRecipeVersioning(SuspendRecipeVersioning const &) = delete;
      SuspendRecipeVersioning & operator=(SuspendRecipeVersioning const &) = delete;
      SuspendRecipeVersioning(SuspendRecipeVersioning &&) = delete;
      SuspendRecipeVersioning & operator=(SuspendRecipeVersioning &&) = delete;
   };

}
#endif
