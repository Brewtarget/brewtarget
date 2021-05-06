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

#include <QColor>
#include <QDate>
#include <QDomDocument>
#include <QDomNode>
#include <QList>
#include <QMutex>
#include <QString>
#include <QVariant>

#include "model/BrewNote.h"
#include "model/Hop.h" // Dammit! Have to include these for Hop::Use and Misc::Use.
#include "model/Misc.h"
#include "model/NamedEntity.h"
#include "model/Salt.h"

namespace PropertyNames::Recipe { static char const * const fg = "fg"; /* previously kpropFG */ }
namespace PropertyNames::Recipe { static char const * const og = "og"; /* previously kpropOG */ }
namespace PropertyNames::Recipe { static char const * const boilTime_min = "boilTime_min"; /* previously kpropBoilTime */ }
namespace PropertyNames::Recipe { static char const * const boilSize_l = "boilSize_l"; /* previously kpropBoilSize */ }
namespace PropertyNames::Recipe { static char const * const batchSize_l = "batchSize_l"; /* previously kpropBatchSize */ }
namespace PropertyNames::Recipe { static char const * const type = "type"; /* previously kpropType */ }
namespace PropertyNames::Recipe { static char const * const notes = "notes"; /* previously kpropNotes */ }
namespace PropertyNames::Recipe { static char const * const kegPrimingFactor = "kegPrimingFactor"; /* previously kpropKegPrimFact */ }
namespace PropertyNames::Recipe { static char const * const primingSugarEquiv = "primingSugarEquiv"; /* previously kpropPrimSugEquiv */ }
namespace PropertyNames::Recipe { static char const * const carbonationTemp_c = "carbonationTemp_c"; /* previously kpropCarbTemp */ }
namespace PropertyNames::Recipe { static char const * const primingSugarName = "primingSugarName"; /* previously kpropPrimSugName */ }
namespace PropertyNames::Recipe { static char const * const forcedCarbonation = "forcedCarbonation"; /* previously kpropForcedCarb */ }
namespace PropertyNames::Recipe { static char const * const carbonation_vols = "carbonation_vols"; /* previously kpropCarbVols */ }
namespace PropertyNames::Recipe { static char const * const points = "points"; /* previously kpropPoints */ }
namespace PropertyNames::Recipe { static char const * const date = "date"; /* previously kpropDate */ }
namespace PropertyNames::Recipe { static char const * const ageTemp_c = "ageTemp_c"; /* previously kpropAgeTemp */ }
namespace PropertyNames::Recipe { static char const * const age = "age"; /* previously kpropAge */ }
namespace PropertyNames::Recipe { static char const * const tertiaryTemp_c = "tertiaryTemp_c"; /* previously kpropTertTemp */ }
namespace PropertyNames::Recipe { static char const * const tertiaryAge_days = "tertiaryAge_days"; /* previously kpropTertAgeDays */ }
namespace PropertyNames::Recipe { static char const * const secondaryTemp_c = "secondaryTemp_c"; /* previously kpropSecTemp */ }
namespace PropertyNames::Recipe { static char const * const secondaryAge_days = "secondaryAge_days"; /* previously kpropSecAgeDays */ }
namespace PropertyNames::Recipe { static char const * const primaryTemp_c = "primaryTemp_c"; /* previously kpropPrimTemp */ }
namespace PropertyNames::Recipe { static char const * const primaryAge_days = "primaryAge_days"; /* previously kpropPrimAgeDays */ }
namespace PropertyNames::Recipe { static char const * const fermentationStages = "fermentationStages"; /* previously kpropFermStages */ }
namespace PropertyNames::Recipe { static char const * const tasteRating = "tasteRating"; /* previously kpropTasteRating */ }
namespace PropertyNames::Recipe { static char const * const tasteNotes = "tasteNotes"; /* previously kpropTasteNotes */ }
namespace PropertyNames::Recipe { static char const * const asstBrewer = "asstBrewer"; /* previously kpropAsstBrewer */ }
namespace PropertyNames::Recipe { static char const * const efficiency_pct = "efficiency_pct"; /* previously kpropEffPct */ }
namespace PropertyNames::Recipe { static char const * const brewer = "brewer"; /* previously kpropBrewer */ }
namespace PropertyNames::Recipe { static char const * const color_srm = "color_srm"; /* previously kpropColor */ }
namespace PropertyNames::Recipe { static char const * const postBoilVolume_l = "postBoilVolume_l"; /* previously kpropPostBoilVol */ }
namespace PropertyNames::Recipe { static char const * const finalVolume_l = "finalVolume_l"; /* previously kpropFinVol */ }

namespace PropertyNames::Recipe { static char const * const recipeType = "recipeType"; }
namespace PropertyNames::Recipe { static char const * const style = "style"; }
namespace PropertyNames::Recipe { static char const * const equipment  = "equipment"; }
namespace PropertyNames::Recipe { static char const * const mash = "mash"; }

namespace PropertyNames::Recipe { static char const * const ABV_pct = "ABV_pct"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const boilGrav = "boilGrav"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const IBU = "IBU"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const IBUs = "IBUs"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const wortFromMash_l = "wortFromMash_l"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const boilVolume_l = "boilVolume_l"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const calories = "calories"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const grainsInMash_kg = "grainsInMash_kg"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const grains_kg = "grains_kg"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const SRMColor = "SRMColor"; /* not stored */ }
namespace PropertyNames::Recipe { static char const * const style_id = "styleId"; /* not stored */ }


// Forward declarations.
class Style;
class Mash;
class Fermentable;
class Equipment;
class Yeast;
class Water;
class Instruction;
class PreInstruction;
class BrewNote;
class MashStep;


/*!
 * \class Recipe
 * \author Philip G. Lee
 *
 * \brief Model class for recipe records in the database.
 */
class Recipe : public NamedEntity
{
   Q_OBJECT
   Q_CLASSINFO("signal", "recipes")

   friend class Database;
   friend class BeerXML;
   friend class RecipeFormatter;
   friend class MainWindow;
   friend class WaterDialog;

public:

   Recipe(QString name, bool cache = true);
   virtual ~Recipe() {}

   // NOTE: move to database?
   //! \brief Retains only the name, but sets everything else to defaults.
   void clear();

   //! \brief The type of recipe
   enum Type { Extract, PartialMash, AllGrain };
   Q_ENUMS( Type )

   //! \brief The \b Type
   Q_PROPERTY( Type recipeType READ recipeType WRITE setRecipeType /*NOTIFY changed*/ /*changedType*/ )

   //! \brief The type (extract, partial mash, all grain) stored as a string
   //         TBD (MY 2021-01-18) Not sure why this is stored as a string rather than an enum.  Have created an enum wrapper above
   Q_PROPERTY( QString type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The brewer.
   Q_PROPERTY( QString brewer READ brewer WRITE setBrewer /*NOTIFY changed*/ /*changedBrewer*/ )
   //! \brief The batch size in liters.
   Q_PROPERTY( double batchSize_l READ batchSize_l WRITE setBatchSize_l /*NOTIFY changed*/ /*changedBatchSize_l*/ )
   //! \brief The boil size in liters.
   Q_PROPERTY( double boilSize_l READ boilSize_l WRITE setBoilSize_l /*NOTIFY changed*/ /*changedBoilSize_l*/ )
   //! \brief The boil time in minutes.
   Q_PROPERTY( double boilTime_min READ boilTime_min WRITE setBoilTime_min /*NOTIFY changed*/ /*changedBoilTime_min*/ )
   //! \brief The overall efficiency in percent.
   Q_PROPERTY( double efficiency_pct READ efficiency_pct WRITE setEfficiency_pct /*NOTIFY changed*/ /*changedEfficiency_pct*/ )
   //! \brief The assistant brewer.
   Q_PROPERTY( QString asstBrewer READ asstBrewer WRITE setAsstBrewer /*NOTIFY changed*/ /*changedAsstBrewer*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief The tasting notes.
   Q_PROPERTY( QString tasteNotes READ tasteNotes WRITE setTasteNotes /*NOTIFY changed*/ /*changedTasteNotes*/ )
   //! \brief The taste rating.
   Q_PROPERTY( double tasteRating READ tasteRating WRITE setTasteRating /*NOTIFY changed*/ /*changedTasteRating*/ )
   //! \brief The number of fermentation stages.
   Q_PROPERTY( int fermentationStages READ fermentationStages WRITE setFermentationStages /*NOTIFY changed*/ /*changedFermentationStages*/ )
   //! \brief How many days in primary.
   Q_PROPERTY( double primaryAge_days READ primaryAge_days WRITE setPrimaryAge_days /*NOTIFY changed*/ /*changedPrimaryAge_days*/ )
   //! \brief The temp in C in the primary.
   Q_PROPERTY( double primaryTemp_c READ primaryTemp_c WRITE setPrimaryTemp_c /*NOTIFY changed*/ /*changedPrimaryTemp_c*/ )
   //! \brief How many days in secondary.
   Q_PROPERTY( double secondaryAge_days READ secondaryAge_days WRITE setSecondaryAge_days /*NOTIFY changed*/ /*changedSecondaryAge_days*/ )
   //! \brief The temp in C in secondary.
   Q_PROPERTY( double secondaryTemp_c READ secondaryTemp_c WRITE setSecondaryTemp_c /*NOTIFY changed*/ /*changedSecondaryTemp_c*/ )
   //! \brief How many days in tertiary.
   Q_PROPERTY( double tertiaryAge_days READ tertiaryAge_days WRITE setTertiaryAge_days /*NOTIFY changed*/ /*changedTertiaryAge_days*/ )
   //! \brief The temp in C in tertiary.
   Q_PROPERTY( double tertiaryTemp_c READ tertiaryTemp_c WRITE setTertiaryTemp_c /*NOTIFY changed*/ /*changedTertiaryTemp_c*/ )
   //! \brief The number of days to age the beer after bottling.
   Q_PROPERTY( double age READ age_days WRITE setAge_days /*NOTIFY changed*/ /*changedAge_days*/ )
   //! \brief The temp in C as beer is aging after bottling.
   Q_PROPERTY( double ageTemp_c READ ageTemp_c WRITE setAgeTemp_c /*NOTIFY changed*/ /*changedAgeTemp_c*/ )
   //! \brief The date the recipe was created or brewed. I'm not sure yet.
   Q_PROPERTY( QDate date READ date WRITE setDate /*NOTIFY changed*/ /*changedDate*/ )
   //! \brief The carbonation in volumes of CO2 at standard temperature and pressure (STP).
   Q_PROPERTY( double carbonation_vols READ carbonation_vols WRITE setCarbonation_vols /*NOTIFY changed*/ /*changedCarbonation_vols*/ )
   //! \brief Whether the beer is force carbonated.
   Q_PROPERTY( bool forcedCarbonation READ forcedCarbonation WRITE setForcedCarbonation /*NOTIFY changed*/ /*changedForcedCarbonation*/ )
   //! \brief The name of the priming sugar.
   Q_PROPERTY( QString primingSugarName READ primingSugarName WRITE setPrimingSugarName /*NOTIFY changed*/ /*changedPrimingSugarName*/ )
   //! \brief The temperature in C while carbonating.
   Q_PROPERTY( double carbonationTemp_c READ carbonationTemp_c WRITE setCarbonationTemp_c /*NOTIFY changed*/ /*changedCarbonationTemp_c*/ )
   //! \brief The factor required to convert this priming agent to an equivalent amount of glucose monohyrate.
   Q_PROPERTY( double primingSugarEquiv READ primingSugarEquiv WRITE setPrimingSugarEquiv /*NOTIFY changed*/ /*changedPrimingSugarEquiv*/ )
   //! \brief The factor required to convert the amount of sugar required for bottles to keg (usually about 0.5).
   Q_PROPERTY( double kegPrimingFactor READ kegPrimingFactor WRITE setKegPrimingFactor /*NOTIFY changed*/ /*changedKegPrimingFactor*/ )

   // Calculated stored properties.
   //! \brief The calculated OG.
   Q_PROPERTY( double og READ og WRITE setOg /*NOTIFY changed*/ /*changedOg*/ )
   //! \brief The calculated FG.
   Q_PROPERTY( double fg READ fg WRITE setFg /*NOTIFY changed*/ /*changedFg*/ )

   // Calculated unstored properties. These need to listen for changes to
   // the uncalculated properties they depend on, and re-emit changed()
   // when appropriate.
   //! \brief The calculated points (1000*(\c og()-1.0)).
   Q_PROPERTY( double points READ points /*WRITE*/ /*NOTIFY changed*/ /*changedPoints*/ STORED false)
   //! \brief The calculated ABV in percent.
   Q_PROPERTY( double ABV_pct READ ABV_pct /*WRITE*/ /*NOTIFY changed*/ /*changedABV*/ STORED false)
   //! \brief The calculated color in SRM.
   Q_PROPERTY( double color_srm READ color_srm /*WRITE*/ /*NOTIFY changed*/ /*changedColor_srm*/ STORED false)
   //! \brief The calculated boil gravity.
   Q_PROPERTY( double boilGrav READ boilGrav /*WRITE*/ /*NOTIFY changed*/ /*changedBoilGrav*/ STORED false)
   //! \brief The calculated IBUs.
   Q_PROPERTY( double IBU READ IBU /*WRITE*/ /*NOTIFY changed*/ /*changedIBU*/ )
   //! \brief IBU contributions from each hop.
   Q_PROPERTY( QList<double> IBUs READ IBUs )
   //! \brief The calculated wort coming from the mash in liters.
   Q_PROPERTY( double wortFromMash_l READ wortFromMash_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateWortFromMash_l*/ STORED false)
   //! \brief The calculated preboil volume in liters.
   Q_PROPERTY( double boilVolume_l READ boilVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateBoilVolume_l*/ STORED false)
   //! \brief The calculated postboil volume in liters.
   Q_PROPERTY( double postBoilVolume_l READ postBoilVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimatePostBoilVolume_l*/ STORED false)
   //! \brief The calculated final volume into the primary in liters.
   Q_PROPERTY( double finalVolume_l READ finalVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateFinalVolume_l*/ STORED false)
   //! \brief The calculated Calories per 12 oz. (kcal).
   Q_PROPERTY( double calories READ calories12oz /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateCalories*/ STORED false)
   //! \brief The amount of grains in the mash in kg.
   Q_PROPERTY( double grainsInMash_kg READ grainsInMash_kg /*WRITE*/ /*NOTIFY changed*/ /*changedGrainsInMash_kg*/ STORED false)
   //! \brief The total amount of grains in the recipe in kg.
   Q_PROPERTY( double grains_kg READ grains_kg /*WRITE*/ /*NOTIFY changed*/ /*changedGrains_kg*/ STORED false)
   //! \brief The beer color as a displayable QColor.
   Q_PROPERTY( QColor SRMColor READ SRMColor /*WRITE*/ /*NOTIFY changed*/ STORED false )

   // Relational properties.
   //! \brief The mash.
   Q_PROPERTY( Mash* mash READ mash WRITE setMash /*NOTIFY changed*/ STORED false)
   //! \brief The equipment.
   Q_PROPERTY( Equipment* equipment READ equipment WRITE setEquipment /*NOTIFY changed*/ STORED false)
   //! \brief The style.
   Q_PROPERTY( Style* style READ style WRITE setStyle /*NOTIFY changed*/ STORED false)
   // These QList properties should only emit changed() when their size changes, or when
   // one of their elements is replaced by another with a different key.
   //! \brief The brew notes.
   Q_PROPERTY( QList<BrewNote*> brewNotes READ brewNotes /*WRITE*/ /*NOTIFY changed*/ STORED false )
   //! \brief The hops.
   Q_PROPERTY( QList<Hop*> hops READ hops /*WRITE*/ /*NOTIFY changed*/ STORED false )
   //! \brief The instructions.
   Q_PROPERTY( QList<Instruction*> instructions READ instructions /*WRITE*/ /*NOTIFY changed*/ STORED false )
   //! \brief The fermentables.
   Q_PROPERTY( QList<Fermentable*> fermentables READ fermentables /*WRITE*/ /*NOTIFY changed*/ STORED false )
   //! \brief The miscs.
   Q_PROPERTY( QList<Misc*> miscs READ miscs /*WRITE*/ /*NOTIFY changed*/ STORED false )
   //! \brief The yeasts.
   Q_PROPERTY( QList<Yeast*> yeasts READ yeasts /*WRITE*/ /*NOTIFY changed*/ STORED false )
   //! \brief The waters.
   Q_PROPERTY( QList<Water*> waters READ waters /*WRITE*/ /*NOTIFY changed*/ STORED false )
   //! \brief The salts.
   Q_PROPERTY( QList<Salt*> salts READ salts /*WRITE*/ /*NOTIFY changed*/ STORED false )

   // Relational setters.
   // NOTE: do these add/remove methods belong here? Should they only exist in Database?
   // One method to bring them all and in darkness bind them
   // .:TBD:. (MY 2020-11-23) At the moment, it feels like there are a lot of places in the code that keep the object
   //         model and the database in sync, which can get complicated.  In the long run, it would be simpler to have
   //         the GUI interact with the object model (Recipes, NamedEntitys, etc) and make it the responsibility of the
   //         object model to store/retrieve/modify what's in the database via some abstraction layer.  Might be worth
   //         looking at https://www.qxorm.com or similar for this.
   //            In the meantime, we cannot define a templated member function _in this header_ that calls
   //         Database::instance() (or indeed any other member function of Database) because that would require us to
   //         #include "database.h" and database.h already needs to #include "model/Recipe.h", so we'd be trapped in circular
   //         dependencies.  Fortunately there is a trick that allows us to declare the function in the header and
   //         define it in the cpp file, even though it's templated.

   /*!
    * \brief Remove \c var from the recipe and return what was removed - ie \c var
    *
    * We want callers to use this strongly-typed version because it makes the implementation of Undo/Redo easier (by
    * making add and remove more symmetric).
    */
   template<class T> T * remove(T * var) {
      return static_cast<T *>(this->removeNamedEntity(var));
   }

   template<class T> QList<T *> remove(QList<T *> many) {

      QList<NamedEntity*> into;
      QList<T*> outof;

      foreach(auto i, many) {
         into.append( static_cast<NamedEntity*>(i));
      }

      foreach(auto i, this->removeNamedEntity(into)) {
         outof.append( static_cast<T*>(i) );
      }

      return outof;
   }

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
    *    Fermentable * newCopyOfSomeFermentable = myRecipe->addFermentable(&someFermentable);   // DO
    *    myRecipe->removeFermentable(newCopyOfSomeFermentable);                                 // UNDO
    *
    * The remover function returns a pointer to the NamedEntity that it removed.  This is useful because it makes add and
    * remove symmetric and simplifies the implementation of UndoableAddOrRemove.
    *
    * TBD: (MY 2020-11-23) It would be good one day to pull out all the non-changeable aspects of ingredients and keep
    *      just one copy of them in the DB and in memory.
    */
   template<class T> T * add(T * var);
   template<class T> QList<T *> add(QList<T *> many);

   void removeBrewNote(BrewNote* var);
   void removeInstruction( Instruction* ins );
   /*!
    * \brief Swap instructions \c ins1 and \c ins2
    */
   void swapInstructions( Instruction* ins1, Instruction* ins2 );
   //! \brief Remove all instructions.
   void clearInstructions();
   //! \brief Insert instruction ins into slot pos.
   void insertInstruction( Instruction* ins, int pos );
   //! \brief Automagically generate a list of instructions.
   void generateInstructions();
   /*!
    * Finds the next ingredient to add that has a time
    * less than time. Changes time to be the time of the found
    * ingredient, or if none are found, -1. Returns a string
    * in the form "Add %1 to %2 at %3".
    */
   QString nextAddToBoil(double& time);

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
   bool cacheOnly() const;

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
   QList<Hop*> hops() const;
   QList<Instruction*> instructions() const;
   QList<Fermentable*> fermentables() const;
   QList<Misc*>  miscs() const;
   QList<Yeast*> yeasts() const;
   QList<Water*> waters() const;
   QList<Salt*>  salts() const;
   QList<BrewNote*> brewNotes() const;

   Mash* mash() const;
   Equipment* equipment() const;
   Style* style();

   // Relational setters
   void setStyle(Style * style);
   void setEquipment(Equipment * equipment);
   void setMash(Mash * var);

   // Other junk.
   QVector<PreInstruction> mashInstructions(double timeRemaining, double totalWaterAdded_l, unsigned int size);
   QVector<PreInstruction> mashSteps();
   QVector<PreInstruction> hopSteps(Hop::Use type = Hop::Boil);
   QVector<PreInstruction> miscSteps(Misc::Use type = Misc::Boil);
   PreInstruction boilFermentablesPre(double timeRemaining);
   bool hasBoilFermentable();
   bool hasBoilExtract();
   static bool isFermentableSugar(Fermentable*);
   PreInstruction addExtracts(double timeRemaining) const;

   // Helpers
   //! \brief Get the ibus from a given \c hop.
   double ibuFromHop(Hop const* hop);
   //! \brief Formats the fermentables for instructions
   QList<QString> getReagents( QList<Fermentable*> ferms );
   //! \brief Formats the mashsteps for instructions
   QList<QString> getReagents( QList<MashStep*> msteps );
   //! \brief Formats the hops for instructions
   QList<QString> getReagents( QList<Hop*> hops, bool firstWort = false );
   //! \brief Formats the salts for instructions
   QStringList getReagents( QList<Salt*> salts, Salt::WhenToAdd wanted);
   QHash<QString,double> calcTotalPoints();

   static QString classNameStr();

   // Setters that are not slots
   void setRecipeType(Type var);
   void setType( const QString &var );
   void setBrewer( const QString &var );
   void setBatchSize_l( double var );
   void setBoilSize_l( double var );
   void setBoilTime_min( double var );
   void setEfficiency_pct( double var );
   void setAsstBrewer( const QString &var );
   void setNotes( const QString &var );
   void setTasteNotes( const QString &var );
   void setTasteRating( double var );
   void setOg( double var );
   void setFg( double var );
   void setFermentationStages( int var );
   void setPrimaryAge_days( double var );
   void setPrimaryTemp_c( double var );
   void setSecondaryAge_days( double var );
   void setSecondaryTemp_c( double var );
   void setTertiaryAge_days( double var );
   void setTertiaryTemp_c( double var );
   void setAge_days( double var );
   void setAgeTemp_c( double var );
   void setDate( const QDate &var );
   void setCarbonation_vols( double var );
   void setForcedCarbonation( bool var );
   void setPrimingSugarName( const QString &var );
   void setCarbonationTemp_c( double var );
   void setPrimingSugarEquiv( double var );
   void setKegPrimingFactor( double var );
   void setCacheOnly( bool cache );

   NamedEntity * getParent();
   virtual int insertInDatabase();
   virtual void removeFromDatabase();

signals:

public slots:
   void acceptEquipChange(QMetaProperty prop, QVariant val);
   void acceptFermChange(QMetaProperty prop, QVariant val);
   void acceptHopChange(QMetaProperty prop, QVariant val);
   void acceptYeastChange(QMetaProperty prop, QVariant val);
   void acceptMashChange(QMetaProperty prop, QVariant val);

   void onFermentableChanged();
   void acceptHopChange(Hop* hop);
   void acceptYeastChange(Yeast* yeast);
   void acceptMashChange(Mash* mash);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

private:
//   Recipe(Brewtarget::DBTable table, int key);
   Recipe(TableSchema* table, QSqlRecord rec, int t_key = -1);
   Recipe(Recipe const& other);

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
   int m_style_id;

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

   bool m_cacheOnly;
   // True when constructed, indicates whether recalcAll has been called.
   bool m_uninitializedCalcs;
   QMutex m_uninitializedCalcsMutex;
   QMutex m_recalcMutex;

   // Batch size without losses.
   double batchSizeNoLosses_l();

   // Some recalculators for calculated properties.

   /*!
    * \brief Remove \c var from the recipe and return what was removed - ie \c var
    */
   NamedEntity * removeNamedEntity( NamedEntity *var);
   QList<NamedEntity *> removeNamedEntity( QList<NamedEntity *> many);

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
   Instruction* postboilFermentablesIns();
   Instruction* postboilIns();
   Instruction* mashFermentableIns();
   Instruction* mashWaterIns();
   Instruction* firstWortHopsIns();
   Instruction* topOffIns();
   Instruction* saltWater(Salt::WhenToAdd when);

   //void setDefaults();
   void addPreinstructions( QVector<PreInstruction> preins );
   bool isValidType( const QString &str );
};
/*
inline bool RecipePtrLt( Recipe* lhs, Recipe* rhs)
{
   return *lhs < *rhs;
}

inline bool RecipePtrEq( Recipe* lhs, Recipe* rhs)
{
   return *lhs == *rhs;
}

struct Recipe_ptr_cmp
{
   bool operator()( Recipe* lhs, Recipe* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Recipe_ptr_equals
{
   bool operator()( Recipe* lhs, Recipe* rhs )
   {
      return *lhs == *rhs;
   }
};
*/
#endif /* _RECIPE_H */
