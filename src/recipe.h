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
#include <QList>
#include <QDomNode>
#include <QDomDocument>
#include <QString>
#include <QDate>
#include "BeerXMLElement.h"
#include "hop.h"
#include "misc.h"

// Forward declarations.
//class Hop;
//class Misc;
class Style;
class Mash;
class Fermentable;
class Equipment;
class Yeast;
class Water;
class Instruction;
class PreInstruction;
class BrewNote;
/*!
 * Compares recipes based on name.
 */
bool operator<(Recipe &r1, Recipe &r2 );
/*!
 * Compares recipes based on name.
 */
bool operator==(Recipe &r1, Recipe &r2 );

class Recipe : public BeerXMLElement
{
   Q_OBJECT
   
   friend class Database;
public:

   virtual ~Recipe() {}

   friend bool operator<(Recipe &r1, Recipe &r2 );
   friend bool operator==(Recipe &r1, Recipe &r2 );
   friend class RecipeFormatter;
   
   // NOTE: move to database?
   /*!
    * Retains only the name, but sets everything else to defaults.
    */
   void clear();
   
   Q_PROPERTY( QString name READ name WRITE setName /*NOTIFY changed*/ /*changedName*/ )
   Q_PROPERTY( QString type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   Q_PROPERTY( QString brewer READ brewer WRITE setBrewer /*NOTIFY changed*/ /*changedBrewer*/ )
   Q_PROPERTY( double batchSize_l READ batchSize_l WRITE setBatchSize_l /*NOTIFY changed*/ /*changedBatchSize_l*/ )
   Q_PROPERTY( double boilSize_l READ boilSize_l WRITE setBoilSize_l /*NOTIFY changed*/ /*changedBoilSize_l*/ )
   Q_PROPERTY( double boilTime_min READ boilTime_min WRITE setBoilTime_min /*NOTIFY changed*/ /*changedBoilTime_min*/ )
   Q_PROPERTY( double efficiency_pct READ efficiency_pct WRITE setEfficiency_pct /*NOTIFY changed*/ /*changedEfficiency_pct*/ )
   Q_PROPERTY( QString asstBrewer READ asstBrewer WRITE setAsstBrewer /*NOTIFY changed*/ /*changedAsstBrewer*/ )
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   Q_PROPERTY( QString tasteNotes READ tasteNotes WRITE setTasteNotes /*NOTIFY changed*/ /*changedTasteNotes*/ )
   Q_PROPERTY( double tasteRating READ tasteRating WRITE setTasteRating /*NOTIFY changed*/ /*changedTasteRating*/ )
   Q_PROPERTY( int fermentationStages READ fermentationStages WRITE setFermentationStages /*NOTIFY changed*/ /*changedFermentationStages*/ )
   Q_PROPERTY( double primaryAge_days READ primaryAge_days WRITE setPrimaryAge_days /*NOTIFY changed*/ /*changedPrimaryAge_days*/ )
   Q_PROPERTY( double primaryTemp_c READ primaryTemp_c WRITE setPrimaryTemp_c /*NOTIFY changed*/ /*changedPrimaryTemp_c*/ )
   Q_PROPERTY( double secondaryAge_days READ secondaryAge_days WRITE setSecondaryAge_days /*NOTIFY changed*/ /*changedSecondaryAge_days*/ )
   Q_PROPERTY( double secondaryTemp_c READ secondaryTemp_c WRITE setSecondaryTemp_c /*NOTIFY changed*/ /*changedSecondaryTemp_c*/ )
   Q_PROPERTY( double tertiaryAge_days READ tertiaryAge_days WRITE setTertiaryAge_days /*NOTIFY changed*/ /*changedTertiaryAge_days*/ )
   Q_PROPERTY( double tertiaryTemp_c READ tertiaryTemp_c WRITE setTertiaryTemp_c /*NOTIFY changed*/ /*changedTertiaryTemp_c*/ )
   Q_PROPERTY( double age_days READ age_days WRITE setAge_days /*NOTIFY changed*/ /*changedAge_days*/ )
   Q_PROPERTY( double ageTemp_c READ ageTemp_c WRITE setAgeTemp_c /*NOTIFY changed*/ /*changedAgeTemp_c*/ )
   Q_PROPERTY( QDate date READ date WRITE setDate /*NOTIFY changed*/ /*changedDate*/ )
   Q_PROPERTY( double carbonation_vols READ carbonation_vols WRITE setCarbonation_vols /*NOTIFY changed*/ /*changedCarbonation_vols*/ )
   Q_PROPERTY( bool forcedCarbonation READ forcedCarbonation WRITE setForcedCarbonation /*NOTIFY changed*/ /*changedForcedCarbonation*/ )
   Q_PROPERTY( QString primingSugarName READ primingSugarName WRITE setPrimingSugarName /*NOTIFY changed*/ /*changedPrimingSugarName*/ )
   Q_PROPERTY( double carbonationTemp_c READ carbonationTemp_c WRITE setCarbonationTemp_c /*NOTIFY changed*/ /*changedCarbonationTemp_c*/ )
   Q_PROPERTY( double primingSugarEquiv READ primingSugarEquiv WRITE setPrimingSugarEquiv /*NOTIFY changed*/ /*changedPrimingSugarEquiv*/ )
   Q_PROPERTY( double kegPrimingFactor READ kegPrimingFactor WRITE setKegPrimingFactor /*NOTIFY changed*/ /*changedKegPrimingFactor*/ )
   
   // Calculated stored properties.
   // Do we need to be able to set og and fg rather than let them be calculated?
   Q_PROPERTY( double og READ og WRITE setOg /*NOTIFY changed*/ /*changedOg*/ )
   Q_PROPERTY( double fg READ fg WRITE setFg /*NOTIFY changed*/ /*changedFg*/ )
   
   // Calculated unstored properties. These need to listen for changes to
   // the uncalculated properties they depend on, and re-emit changed()
   // when appropriate.
   Q_PROPERTY( double points READ points /*WRITE*/ /*NOTIFY changed*/ /*changedPoints*/ STORED false)
   Q_PROPERTY( double ABV_pct READ ABV_pct /*WRITE*/ /*NOTIFY changed*/ /*changedABV*/ STORED false)
   Q_PROPERTY( double color_srm READ color_srm /*WRITE*/ /*NOTIFY changed*/ /*changedColor_srm*/ STORED false)
   Q_PROPERTY( double boilGrav READ boilGrav /*WRITE*/ /*NOTIFY changed*/ /*changedBoilGrav*/ STORED false)
   Q_PROPERTY( double IBU READ IBU /*WRITE*/ /*NOTIFY changed*/ /*changedIBU*/ )
   Q_PROPERTY( double wortFromMash_l READ wortFromMash_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateWortFromMash_l*/ STORED false)
   Q_PROPERTY( double boilVolume_l READ boilVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateBoilVolume_l*/ STORED false)
   Q_PROPERTY( double postBoilVolume_l READ postBoilVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimatePostBoilVolume_l*/ STORED false)
   Q_PROPERTY( double finalVolume_l READ finalVolume_l /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateFinalVolume_l*/ STORED false)
   Q_PROPERTY( double calories READ calories /*WRITE*/ /*NOTIFY changed*/ /*changedEstimateCalories*/ STORED false)
   Q_PROPERTY( double grainsInMash_kg READ grainsInMash_kg /*WRITE*/ /*NOTIFY changed*/ /*changedGrainsInMash_kg*/ STORED false)
   Q_PROPERTY( double grains_kg READ grains_kg /*WRITE*/ /*NOTIFY changed*/ /*changedGrains_kg*/ STORED false)
   Q_PROPERTY( QColor SRMColor READ SRMColor /*WRITE*/ /*NOTIFY changed*/ STORED false )
   
   // Relational properties.
   Q_PROPERTY( Mash* mash READ mash /*WRITE*/ /*NOTIFY changed*/ STORED false);
   Q_PROPERTY( Equipment* equipment READ equipment /*WRITE*/ /*NOTIFY changed*/ STORED false);
   Q_PROPERTY( Style* style READ style /*WRITE*/ /*NOTIFY changed*/ STORED false);
   // These QList properties should only emit changed() when their size changes, or when
   // one of their elements is replaced by another with a different key.
   Q_PROPERTY( QList<BrewNote*> brewNotes READ brewNotes /*WRITE*/ /*NOTIFY changed*/ STORED false );
   Q_PROPERTY( QList<Hop*> hops READ hops /*WRITE*/ /*NOTIFY changed*/ STORED false );
   Q_PROPERTY( QList<Instruction*> instructions READ instructions /*WRITE*/ /*NOTIFY changed*/ STORED false );
   Q_PROPERTY( QList<Fermentable*> fermentables READ fermentables /*WRITE*/ /*NOTIFY changed*/ STORED false );
   Q_PROPERTY( QList<Misc*> miscs READ miscs /*WRITE*/ /*NOTIFY changed*/ STORED false );
   Q_PROPERTY( QList<Yeast*> yeasts READ yeasts /*WRITE*/ /*NOTIFY changed*/ STORED false );
   Q_PROPERTY( QList<Water*> waters READ waters /*WRITE*/ /*NOTIFY changed*/ STORED false );
   
   // Setters
   void setName( const QString &var );
   void setType( const QString &var );
   void setBrewer( const QString &var );
   //void setStyle( Style *var );
   void setBatchSize_l( double var );
   void setBoilSize_l( double var );
   void setBoilTime_min( double var );
   void setEfficiency_pct( double var );
   //void setMash( Mash *var );
   void setAsstBrewer( const QString &var );
   //void setEquipment( Equipment *var );
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
   //! Set the multiplication factor to convert mass of glucose to mass of this priming sugar.
   void setPrimingSugarEquiv( double var );
   //! Set multiplication factor to convert mass of glucose reqd. to bottle prime to that required to keg prime.
   void setKegPrimingFactor( double var );

   // Relational setters.
   // NOTE: do these add/remove methods belong here? Should they only exist in Database?
   void addHop( Hop *var );
   void removeHop( Hop *var );
   void addFermentable( Fermentable* var );
   void removeFermentable( Fermentable* var );
   void addMisc( Misc* var );
   void removeMisc( Misc* var );
   void addYeast( Yeast* var );
   void removeYeast( Yeast* var );
   void addWater( Water* var );
   void removeWater( Water* var );
   //void addBrewNote(BrewNote* var);
   void removeBrewNote(BrewNote* var);
   //void addInstruction( Instruction* ins );
   void removeInstruction( Instruction* ins );
   /*!
    * Swap instructions j and k.
    * \param j some integer less than getNumInstructions()
    * \param k some integer less than getNumInstructions()
    */
   void swapInstructions( Instruction* ins1, Instruction* ins2 );
   //! Remove all instructions.
   void clearInstructions();
   //! Insert instruction ins into slot pos.
   void insertInstruction( Instruction* ins, int pos );
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
   QDate date() const;
   double carbonation_vols() const;
   bool forcedCarbonation() const;
   QString primingSugarName() const;
   double carbonationTemp_c() const;
   double primingSugarEquiv() const;
   double kegPrimingFactor() const;
   
   // Calculated getters.
   //! Gets points from og().
   double points() const;
   //! Gets points if the final \b volume is given.
   double points(double volume);
   double ABV_pct() const;
   double color_srm() const;
   double boilGrav() const;
   double IBU() const;
   QColor SRMColor() const;
   //! Estimate amount of wort collected immediately after the mash.
   double wortFromMash_l() const;
   //! Estimate boil volume based on user inputs.
   double boilVolume_l() const;
   //! Estimate how much wort immediately post boil.
   double postBoilVolume_l() const;
   //! Estimate final volume based on user inputs.
   double finalVolume_l() const;
   //! Estimate final calories of the beer
   double calories() const;
   double grainsInMash_kg() const;
   double grains_kg() const;
   //! Get a list of IBU contributions from each hop.
   QList<double> IBUs() const;
   
   // Relational getters
   QList<Hop*> hops() const;
   QList<Instruction*> instructions() const;
   QList<Fermentable*> fermentables() const;
   QList<Misc*> miscs() const;
   QList<Yeast*> yeasts() const;
   QList<Water*> waters() const;
   QList<BrewNote*> brewNotes() const;
   
   Mash* mash() const;
   Equipment* equipment() const;
   Style* style() const;
   
   // Other junk.
   QVector<PreInstruction> mashInstructions(double timeRemaining, double totalWaterAdded_l, unsigned int size);
   QVector<PreInstruction> mashSteps();
   QVector<PreInstruction> hopSteps(Hop::Use type = Hop::Boil);
   QVector<PreInstruction> miscSteps(Misc::Use type = Misc::Boil);
   PreInstruction boilFermentablesPre(double timeRemaining);
   bool hasBoilFermentable();

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

public slots:
   void changed(QMetaProperty prop, QVariant val);
   
private:
   
   Recipe();
   Recipe(Recipe const& other);
   
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
   
   // Calculated properties.
   double _points;
   double _ABV_pct;
   double _color_srm;
   double _boilGrav;
   double _IBU;
   QList<double> _ibus;
   double _wortFromMash_l;
   double _boilVolume_l;
   double _postBoilVolume_l;
   double _finalVolume_l;
   double _calories;
   double _grainsInMash_kg;
   double _grains_kg;
   QColor _SRMColor;
   
   // Calculated, but stored...BeerXML is weird sometimes.
   double _og;
   double _fg;
   
   // Some recalculators for calculated properties.
   /*! The theoretical maximum yield without any non-mashed anything. This
    * will need to be communicated somewhere. Emits changed(points).
    */
   void recalcPoints(double volume);
   //! Emits changed(ABV_pct)
   void recalcABV_pct();
   //! Emits changed(color_srm)
   void recalcColor_srm();
   //! Emits changed(boilGrav)
   void recalcBoilGrav();
   //! Emits changed(IBU)
   void recalcIBU();
   //! Emits changed(wortFromMash_l), changed(boilVolume_l), changed(finalVolume_l), changed(postBoilVolume_l).
   void recalcVolumeEstimates();
   //! Emits changed(grainsInMash_kg)
   void recalcGrainsInMash_kg();
   //! Emits changed(grains_kg)
   void recalcGrains_kg();
   //! Emits changed(SRMColor)
   void recalcSRMColor();
   //! Emits changed(calories)
   void recalcCalories();
   //! Emits changed(og), changed(fg)
   void recalcOgFg();
   
   // Helper
   double ibuFromHop(Hop const* hop);
   
   // Adds instructions to the recipe.
   Instruction* postboilFermentablesIns();
   Instruction* postboilIns();
   Instruction* mashFermentableIns();
   Instruction* mashWaterIns(unsigned int size);
   Instruction* firstWortHopsIns();
   Instruction* topOffIns();
   
   //void setDefaults();
   void addPreinstructions( QVector<PreInstruction> preins );
   bool isValidType( const QString &str );
   
   static QHash<QString,QString> tagToProp;
   static QHash<QString,QString> tagToPropHash();
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
