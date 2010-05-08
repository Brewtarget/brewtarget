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

   enum{INSTRUCTION, MASH};

   friend bool operator<(Recipe &r1, Recipe &r2 );
   friend bool operator==(Recipe &r1, Recipe &r2 );
   friend class RecipeFormatter;

   virtual void fromNode(const QDomNode& node); // From BeerXMLElement
   virtual void toXml(QDomDocument& doc, QDomNode& parent); // From BeerXMLElement
   
   void clear(); // Retains only the name, but sets everything else to defaults.
   virtual void notify(Observable *notifier, QVariant info = QVariant()); // Inherited from MultipleObserver.
   
   void setName( const std::string &var );
   void setType( const std::string &var );
   void setBrewer( const QString &var );
   void setStyle( Style *var );
   void setBatchSize_l( double var );
   void setBoilSize_l( double var );
   void setBoilTime_min( double var );
   void setEfficiency_pct( double var );
   
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

   void addInstruction( Instruction* ins );
   void removeInstruction( Instruction* ins );
   void swapInstructions( unsigned int j, unsigned int k );
   void clearInstructions();
   void insertInstruction( Instruction* ins, int pos );
   int getNumInstructions();
   Instruction* getInstruction(int i);
   void generateInstructions();
   QString nextAddToBoil(double& time);

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
   void setPrimingSugarName( const std::string &var );
   void setCarbonationTemp_c( double var );
   void setPrimingSugarEquiv( double var );
   void setKegPrimingFactor( double var );

   std::string getName() const;
   std::string getType() const;
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
   std::string getPrimingSugarName() const;
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
   double estimateFinalVolume_l() const; // Estimate final volume based on user inputs.
   double getGrainsInMash_kg() const;

private:

   std::string name;
   static const int version = 1;
   std::string type;
   QString brewer;
   Style* style;
   double batchSize_l;
   double boilSize_l;
   double boilTime_min;
   double efficiency_pct;
   std::vector<Hop*> hops;
   std::vector<Fermentable*> fermentables;
   std::vector<Misc*> miscs;
   std::vector<Yeast*> yeasts;
   std::vector<Water*> waters;
   Mash *mash;
   std::vector<Instruction*> instructions;
   
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
   std::string primingSugarName;
   double carbonationTemp_c;
   double primingSugarEquiv;
   double kegPrimingFactor;
   
   void setDefaults();
   bool isValidType( const std::string &str );
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
