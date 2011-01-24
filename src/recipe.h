/*
 * recipe.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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
#include <string>
#include <exception>
#include "style.h"
#include "misc.h"
#include "mash.h"
#include "hop.h"
#include "fermentable.h"
#include "equipment.h"
#include "yeast.h"
#include "water.h"
#include "observable.h"
#include "instruction.h"

class Recipe : public Observable, public MultipleObserver, public BeerXMLElement
{
public:

   Recipe();
   Recipe(const QDomNode& recipeNode);
   Recipe(Recipe* other); // Deep copy constructor.

   /*!
    * This enum is for passing information via Observer::notify().
    * DONOTUSE is first because it will equal zero, and by default, QVariant.toInt() returns 0. So,
    * put this as a place holder and don't ever use it.
    * \todo Make sure other Observables have this dummy element.
    */
   enum{DONOTUSE, INSTRUCTION, MASH};

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
   
   /*!
    * Set recipe name.
    */
   void setName( const QString &var );
   /*!
    * Set recipe type.
    */
   void setType( const QString &var );
   /*!
    * Set brewer.
    */
   void setBrewer( const QString &var );
   /*!
    * Set style.
    */
   void setStyle( Style *var );
   /*!
    * Set the batch size in liters.
    */
   void setBatchSize_l( double var );
   /*!
    * Set the boil size in liters.
    */
   void setBoilSize_l( double var );
   /*!
    * Set the boil time in minutes.
    */
   void setBoilTime_min( double var );
   /*!
    * Set the overall efficiency in percent (out of 100).
    */
   void setEfficiency_pct( double var );
   
   /*!
    * Add a hop.
    */
   void addHop( Hop *var );
   /*!
    * Remove a hop.
    */
   bool removeHop( Hop *var );
   /*!
    * Add a fermentable.
    */
   void addFermentable( Fermentable* var );
   /*!
    * Remove a fermentable.
    */
   bool removeFermentable( Fermentable* var );
   /*!
    * Add a misc.
    */
   void addMisc( Misc* var );
   /*!
    * Remove a misc.
    */
   bool removeMisc( Misc* var );
   /*!
    * Add a yeast.
    */
   void addYeast( Yeast* var );
   /*!
    * Remove a yeast.
    */
   bool removeYeast( Yeast* var );
   /*!
    * Add a water.
    */
   void addWater( Water* var );
   /*!
    * Remove a water.
    */
   bool removeWater( Water* var );

   /*!
    * Add an instruction.
    */
   void addInstruction( Instruction* ins );
   /*!
    * Remove an instruction.
    */
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

   //! Set mash.
   void setMash( Mash *var );
   //! Set assistant brewer.
   void setAsstBrewer( const QString &var );
   //! Set equipment.
   void setEquipment( Equipment *var );
   //! Set notes.
   void setNotes( const QString &var );
   //! Set taste notes.
   void setTasteNotes( const QString &var );
   //! Set taste rating.
   void setTasteRating( double var );
   //! Set OG.
   void setOg( double var );
   //! Set FG.
   void setFg( double var );
   //! Set the number of fermentation stages.
   void setFermentationStages( int var );
   //! Set the primary age in days.
   void setPrimaryAge_days( double var );
   //! Set the primary temp in celsius.
   void setPrimaryTemp_c( double var );
   //! Set the secondary age in days.
   void setSecondaryAge_days( double var );
   //! Set the secondary temp in celsius.
   void setSecondaryTemp_c( double var );
   //! Set the tertiary time in days.
   void setTertiaryAge_days( double var );
   //! Set the tertiary temp in celsius.
   void setTertiaryTemp_c( double var );
   //! Set the age time in days.
   void setAge_days( double var );
   //! Set the age temp in celsius.
   void setAgeTemp_c( double var );
   //! Set the date in a reasonable date format.
   void setDate( const QString &var );
   //! Set the carbonation in volumes of CO2 (1L CO2 per liter of beer at standard temp and pressure).
   void setCarbonation_vols( double var );
   //! Set the forced carbonation flag.
   void setForcedCarbonation( bool var );
   //! Set the priming sugar name. Change this to QString.
   /*!
    * \todo Change to QString.
    */
   void setPrimingSugarName( const QString &var );
   //! Set carbonation temp in C.
   void setCarbonationTemp_c( double var );
   //! Set the multiplication factor to convert mass of glucose to mass of this priming sugar.
   void setPrimingSugarEquiv( double var );
   //! Set multiplication factor to convert mass of glucose reqd. to bottle prime to that required to keg prime.
   void setKegPrimingFactor( double var );

   QString getName() const;
   QString getType() const;
   QString getBrewer() const;
   Style *getStyle() const;
   double getBatchSize_l() const;
   double getBoilSize_l() const;
   double getBoilTime_min() const;
   double getEfficiency_pct() const;
   
   unsigned int getNumHops() const;
   Hop* getHop(unsigned int i);
   unsigned int getNumFermentables() const;
   Fermentable* getFermentable(unsigned int i);
   unsigned int getNumMiscs() const;
   Misc* getMisc(unsigned int i);
   unsigned int getNumYeasts() const;
   Yeast* getYeast(unsigned int i);
   unsigned int getNumWaters() const;
   Water* getWater(unsigned int i);
   
   Mash* getMash() const;

   QString getAsstBrewer() const;
   Equipment* getEquipment() const;
   QString getNotes() const;
   QString getTasteNotes() const;
   double getTasteRating() const;
   double getOg() const;
   double getFg() const;
   int getFermentationStages() const;
   double getPrimaryAge_days() const;
   double getPrimaryTemp_c() const;
   double getSecondaryAge_days() const;
   double getSecondaryTemp_c() const;
   double getTertiaryAge_days() const;
   double getTertiaryTemp_c() const;
   double getAge_days() const;
   double getAgeTemp_c() const;
   QString getDate() const;
   double getCarbonation_vols() const;
   bool getForcedCarbonation() const;
   QString getPrimingSugarName() const;
   double getCarbonationTemp_c() const;
   double getPrimingSugarEquiv() const;
   double getKegPrimingFactor() const;

   void recalculate(); // Calculates some parameters.
   double getABV_pct();
   double getColor_srm();
   double getBoilGrav();
   double getIBU();
   double getIBUFromHop( unsigned int i );
   QColor getSRMColor();
   double estimateWortFromMash_l() const; // Estimate amount of wort collected immediately after the mash.
   double estimateBoilVolume_l() const; // Estimate boil volume based on user inputs.
   double estimatePostBoilVolume_l() const; // How much wort immediately post boil.
   double estimateFinalVolume_l() const; // Estimate final volume based on user inputs.
   double estimateCalories() const;    // Estimate final calories of the beer
   double getGrainsInMash_kg() const;
   double getGrains_kg() const;

private:

   QString name;
   static const int version = 1;
   QString type;
   QString brewer;
   Style* style;
   double batchSize_l;
   double boilSize_l;
   double boilTime_min;
   double efficiency_pct;
   QVector<Hop*> hops;
   QVector<Fermentable*> fermentables;
   QVector<Misc*> miscs;
   QVector<Yeast*> yeasts;
   QVector<Water*> waters;
   Mash *mash;
   QVector<Instruction*> instructions;
   
   QString asstBrewer;
   Equipment* equipment;
   QString notes;
   QString tasteNotes;
   double tasteRating;
   double og;
   double fg;
   int fermentationStages;
   double primaryAge_days;
   double primaryTemp_c;
   double secondaryAge_days;
   double secondaryTemp_c;
   double tertiaryAge_days;
   double tertiaryTemp_c;
   double age_days;
   double ageTemp_c;
   QString date;
   double carbonation_vols;
   bool forcedCarbonation;
   QString primingSugarName;
   double carbonationTemp_c;
   double primingSugarEquiv;
   double kegPrimingFactor;
   double estimatedCalories;
   
   void setDefaults();
   bool isValidType( const QString &str );
};

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
