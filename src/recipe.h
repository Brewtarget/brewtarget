/*
 * recipe.h is part of Brewtarget, and is Copyright Philip G. Lee
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
#ifndef _RECIPE_H
#define _RECIPE_H

class Recipe;

#include <QColor>
#include <QVariant>
#include <QDomNode>
#include <QDomDocument>
#include <QString>
#include "BeerXMLElement.h"
#include "style.h"
#include "misc.h"
#include "mash.h"
#include "hop.h"
#include "fermentable.h"
#include "equipment.h"
#include "yeast.h"
#include "water.h"
#include "instruction.h"
#include "PreInstruction.h"
#include "brewnote.h"

class Recipe : public BeerXMLElement
{
   Q_OBJECT
public:

   Recipe();
   Recipe(Recipe const& other);

   virtual ~Recipe() {}

   /*!
    * Compares recipes based on name.
    */
   friend bool operator<(Recipe &r1, Recipe &r2 );
   /*!
    * Compares recipes based on name.
    */
   friend bool operator==(Recipe &r1, Recipe &r2 );
   friend class RecipeFormatter;

   /*!
    * From BeerXMLElement.
    */
   virtual void fromNode(const QDomNode& node);
   /*!
    * From BeerXMLElement.
    */
   virtual void toXml(QDomDocument& doc, QDomNode& parent);
   
   /*!
    * Retains only the name, but sets everything else to defaults.
    */
   void clear();
   /*!
    * Inherited from MultipleObserver.
    */
   virtual void notify(Observable *notifier, QVariant info = QVariant());
   
   Q_PROPERTY( QString name READ name WRITE setName NOTIFY changed /*changedName*/ )
   Q_PROPERTY( QString type READ type WRITE setType NOTIFY changed /*changedType*/ )
   Q_PROPERTY( QString brewer READ brewer WRITE setBrewer NOTIFY changed /*changedBrewer*/ )
   Q_PROPERTY( double batchSize_l READ batchSize_l WRITE setBatchSize_l NOTIFY changed /*changedBatchSize_l*/ )
   Q_PROPERTY( double boilSize_l READ boilSize_l WRITE setBoilSize_l NOTIFY changed /*changedBoilSize_l*/ )
   Q_PROPERTY( double boilTime_min READ boilTime_min WRITE setBoilTime_min NOTIFY changed /*changedBoilTime_min*/ )
   Q_PROPERTY( double efficiency_pct READ efficiency_pct WRITE setEfficiency_pct NOTIFY changed /*changedEfficiency_pct*/ )
   Q_PROPERTY( QString asstBrewer READ asstBrewer WRITE setAsstBrewer NOTIFY changed /*changedAsstBrewer*/ )
   Q_PROPERTY( QString notes READ notes WRITE setNotes NOTIFY changed /*changedNotes*/ )
   Q_PROPERTY( QString tasteNotes READ tasteNotes WRITE setTasteNotes NOTIFY changed /*changedTasteNotes*/ )
   Q_PROPERTY( double tasteRating READ tasteRating WRITE setTasteRating NOTIFY changed /*changedTasteRating*/ )
   Q_PROPERTY( double og READ og WRITE setOg NOTIFY changed /*changedOg*/ )
   Q_PROPERTY( double fg READ fg WRITE setFg NOTIFY changed /*changedFg*/ )
   Q_PROPERTY( int fermentationStages READ fermentationStages WRITE setFermentationStages NOTIFY changed /*changedFermentationStages*/ )
   Q_PROPERTY( double primaryAge_days READ primaryAge_days WRITE setPrimaryAge_days NOTIFY changed /*changedPrimaryAge_days*/ )
   Q_PROPERTY( double primaryTemp_c READ primaryTemp_c WRITE setPrimaryTemp_c NOTIFY changed /*changedPrimaryTemp_c*/ )
   Q_PROPERTY( double secondaryAge_days READ secondaryAge_days WRITE setSecondaryAge_days NOTIFY changed /*changedSecondaryAge_days*/ )
   Q_PROPERTY( double secondaryTemp_c READ secondaryTemp_c WRITE setSecondaryTemp_c NOTIFY changed /*changedSecondaryTemp_c*/ )
   Q_PROPERTY( double tertiaryAge_days READ tertiaryAge_days WRITE setTertiaryAge_days NOTIFY changed /*changedTertiaryAge_days*/ )
   Q_PROPERTY( double tertiaryTemp_c READ tertiaryTemp_c WRITE setTertiaryTemp_c NOTIFY changed /*changedTertiaryTemp_c*/ )
   Q_PROPERTY( double age_days READ age_days WRITE setAge_days NOTIFY changed /*changedAge_days*/ )
   Q_PROPERTY( double ageTemp_c READ ageTemp_c WRITE setAgeTemp_c NOTIFY changed /*changedAgeTemp_c*/ )
   Q_PROPERTY( QString date READ date WRITE setDate NOTIFY changed /*changedDate*/ )
   Q_PROPERTY( double carbonation_vols READ carbonation_vols WRITE setCarbonation_vols NOTIFY changed /*changedCarbonation_vols*/ )
   Q_PROPERTY( bool forcedCarbonation READ forcedCarbonation WRITE setForcedCarbonation NOTIFY changed /*changedForcedCarbonation*/ )
   Q_PROPERTY( QString primingSugarName READ primingSugarName WRITE setPrimingSugarName NOTIFY changed /*changedPrimingSugarName*/ )
   Q_PROPERTY( double carbonationTemp_c READ carbonationTemp_c WRITE setCarbonationTemp_c NOTIFY changed /*changedCarbonationTemp_c*/ )
   Q_PROPERTY( double primingSugarEquiv READ primingSugarEquiv WRITE setPrimingSugarEquiv NOTIFY changed /*changedPrimingSugarEquiv*/ )
   Q_PROPERTY( double kegPrimingFactor READ kegPrimingFactor WRITE setKegPrimingFactor NOTIFY changed /*changedKegPrimingFactor*/ )
   Q_PROPERTY( double estimatedCalories READ estimatedCalories WRITE setEstimatedCalories NOTIFY changed /*changedEstimatedCalories*/ )
   
   // Calculated properties.
   Q_PROPERTY( double points READ points /*WRITE*/ NOTIFY changed /*changedPoints*/ STORED false)
   Q_PROPERTY( double ABV READ ABV /*WRITE*/ NOTIFY changed /*changedABV*/ STORED false)
   Q_PROPERTY( double color_srm READ color_srm /*WRITE*/ NOTIFY changed /*changedColor_srm*/ STORED false)
   Q_PROPERTY( double boilGrav READ boilGrav /*WRITE*/ NOTIFY changed /*changedBoilGrav*/ STORED false)
   Q_PROPERTY( double IBU READ IBU /*WRITE*/ NOTIFY changed /*changedIBU*/ )
   Q_PROPERTY( double estimateWortFromMash_l READ estimateWortFromMash_l /*WRITE*/ NOTIFY changed /*changedEstimateWortFromMash_l*/ STORED false)
   Q_PROPERTY( double estimateBoilVolume_l READ estimateBoilVolume_l /*WRITE*/ NOTIFY changed /*changedEstimateBoilVolume_l*/ STORED false)
   Q_PROPERTY( double estimatePostBoilVolume_l READ estimatePostBoilVolume_l /*WRITE*/ NOTIFY changed /*changedEstimatePostBoilVolume_l*/ STORED false)
   Q_PROPERTY( double estimateFinalVolume_l READ estimateFinalVolume_l /*WRITE*/ NOTIFY changed /*changedEstimateFinalVolume_l*/ STORED false)
   Q_PROPERTY( double estimateCalories READ estimateCalories /*WRITE*/ NOTIFY changed /*changedEstimateCalories*/ STORED false)
   Q_PROPERTY( double grainsInMash_kg READ grainsInMash_kg /*WRITE*/ NOTIFY changed /*changedGrainsInMash_kg*/ STORED false)
   Q_PROPERTY( double grains_kg READ grains_kg /*WRITE*/ NOTIFY changed /*changedGrains_kg*/ STORED false)
   
   // Setters
   void setName( const QString &var );
   void setType( const QString &var );
   void setBrewer( const QString &var );
   void setStyle( Style *var );
   void setBatchSize_l( double var );
   void setBoilSize_l( double var );
   void setBoilTime_min( double var );
   void setEfficiency_pct( double var );
   void setMash( Mash *var );
   void setAsstBrewer( const QString &var );
   void setEquipment( Equipment *var );
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
   void setDate( const QString &var );
   void setCarbonation_vols( double var );
   void setForcedCarbonation( bool var );
   void setPrimingSugarName( const QString &var );
   void setCarbonationTemp_c( double var );
   //! Set the multiplication factor to convert mass of glucose to mass of this priming sugar.
   void setPrimingSugarEquiv( double var );
   //! Set multiplication factor to convert mass of glucose reqd. to bottle prime to that required to keg prime.
   void setKegPrimingFactor( double var );

   // Relational setters
   void addHop( Hop *var );
   bool removeHop( Hop *var );
   void addFermentable( Fermentable* var );
   bool removeFermentable( Fermentable* var );
   void addMisc( Misc* var );
   bool removeMisc( Misc* var );
   void addYeast( Yeast* var );
   bool removeYeast( Yeast* var );
   void addWater( Water* var );
   bool removeWater( Water* var );
   void addBrewNote(BrewNote* var);
   bool removeBrewNote(BrewNote* var);
   bool removeBrewNote(QList<BrewNote*> var);
   void addInstruction( Instruction* ins );
   void removeInstruction( Instruction* ins );
   /*!
    * Swap instructions j and k.
    * \param j some integer less than getNumInstructions()
    * \param k some integer less than getNumInstructions()
    */
   void swapInstructions( unsigned int j, unsigned int k );
   //! Remove all instructions.
   void clearInstructions();
   //! Insert instruction ins into slot pos.
   void insertInstruction( Instruction* ins, int pos );
   //! Get the total number of instructions.
   int getNumInstructions();
   //! Get instruction i.
   Instruction* getInstruction(unsigned int i);
   //! Automagically generate a list of instructions.
   void generateInstructions();
   /*!
    * Finds the next ingredient to add that has a time
    * less than time. Changes time to be the time of the found
    * ingredient, or if none are found, -1. Returns a string
    * in the form "Add %1 to %2 at %3".
    */
   QString nextAddToBoil(double& time);

   // Getters
   QString name() const;
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
   double og() const;
   double fg() const;
   int fermentationStages() const;
   double primaryAge_days() const;
   double primaryTemp_c() const;
   double secondaryAge_days() const;
   double secondaryTemp_c() const;
   double tertiaryAge_days() const;
   double tertiaryTemp_c() const;
   double age_days() const;
   double ageTemp_c() const;
   QString date() const;
   double carbonation_vols() const;
   bool forcedCarbonation() const;
   QString primingSugarName() const;
   double carbonationTemp_c() const;
   double primingSugarEquiv() const;
   double kegPrimingFactor() const;
   
   // Calculated getters.
   double points(double volume);
   double ABV_pct();
   double color_srm();
   double boilGrav();
   double IBU();
   QColor SRMColor();
   double estimateWortFromMash_l() const; // Estimate amount of wort collected immediately after the mash.
   double estimateBoilVolume_l() const; // Estimate boil volume based on user inputs.
   double estimatePostBoilVolume_l() const; // How much wort immediately post boil.
   double estimateFinalVolume_l() const; // Estimate final volume based on user inputs.
   double estimateCalories() const;    // Estimate final calories of the beer
   double grainsInMash_kg() const;
   double grains_kg() const;
   bool hasBoilFermentable();
   
   // Relational getters
   Style* style() const;
   unsigned int numHops() const;
   Hop* getHop(unsigned int i);
   unsigned int numFermentables() const;
   Fermentable* fermentable(unsigned int i);
   unsigned int numMiscs() const;
   Misc* misc(unsigned int i);
   unsigned int numYeasts() const;
   Yeast* yeast(unsigned int i);
   unsigned int numWaters() const;
   Water* water(unsigned int i);
   unsigned int numBrewNotes() const;
   BrewNote* brewNote(unsigned int i);
   Mash* mash() const;
   Equipment* equipment() const;
   double IBUFromHop( unsigned int i );
   
   // Other junk.
   Instruction* getPostboilFermentables();
   Instruction* getPostboilSteps();
   Instruction* getMashFermentable() const;
   Instruction* getMashWater(unsigned int size) const;
   Instruction* getFirstWortHops() const;
   Instruction* getTopOff() const;
   QVector<PreInstruction> mashInstructions(double timeRemaining, double totalWaterAdded_l, unsigned int size) const;
   QVector<PreInstruction> mashSteps() const;
   QVector<PreInstruction> hopSteps(Hop::Use type = Hop::USEBOIL) const;
   QVector<PreInstruction> miscSteps(Misc::Use type = Misc::USEBOIL) const;
   PreInstruction boilFermentables(double timeRemaining) const;

signals:
   /*
   void changedName(QString);
   void changedType(QString);
   void changedBrewer(QString);
   void changedBatchSize_l(double);
   void changedBoilSize_l(double);
   void changedBoilTime_min(double);
   void changedEfficiency_pct(double);
   void changedAsstBrewer(QString);
   void changedNotes(QString);
   void changedTasteNotes(QString);
   void changedTasteRating(double);
   void changedOg(double);
   void changedFg(double);
   void changedFermentationStages(int);
   void changedPrimaryAge_days(double);
   void changedPrimaryTemp_c(double);
   void changedSecondaryAge_days(double);
   void changedSecondaryTemp_c(double);
   void changedTertiaryAge_days(double);
   void changedTertiaryTemp_c(double);
   void changedAge_days(double);
   void changedAgeTemp_c(double);
   void changedDate(QString);
   void changedCarbonation_vols(double);
   void changedForcedCarbonation(bool);
   void changedPrimingSugarName(QString);
   void changedCarbonationTemp_c(double);
   void changedPrimingSugarEquiv(double);
   void changedKegPrimingFactor(double);
   void changedEstimatedCalories(double);
   void changedPoints(double);
   void changedABV(double);
   void changedColor_srm(double);
   void changedBoilGrav(double);
   void changedIBU(double);
   void changedEstimateWortFromMash_l(double);
   void changedEstimateBoilVolume_l(double);
   void changedEstimatePostBoilVolume_l(double);
   void changedEstimateFinalVolume_l(double);
   void changedEstimateCalories(double);
   void changedGrainsInMash_kg(double);
   void changedGrains_kg(double);
   */
   
private:
   // Relational members.
   /*
   Style* style;
   QVector<Hop*> hops;
   QVector<Fermentable*> fermentables;
   QVector<Misc*> miscs;
   QVector<Yeast*> yeasts;
   QVector<Water*> waters;
   Mash *mash;
   QVector<Instruction*> instructions;
   QVector<BrewNote*> brewNotes;
   Equipment* equipment;
   */
   
   void setDefaults();
   void addPreinstructions( QVector<PreInstruction> preins );
   bool isValidType( const QString &str );
   void recalculate(); // Calculates some parameters.
};

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

#endif /* _RECIPE_H */
