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

#include <string>
#include <exception>
#include "xmlnode.h"
#include "style.h"
#include "misc.h"
#include "mash.h"
#include "hop.h"
#include "fermentable.h"
#include "equipment.h"
#include "yeast.h"
#include "water.h"
#include "observable.h"


class Recipe;
class RecipeException;

class Recipe : public Observable
{
public:

   Recipe();
   Recipe(const XmlNode *node);

   std::string toXml();
   
   void setName( const std::string &var );
   void setType( const std::string &var );
   void setBrewer( const std::string &var );
   void setStyle( Style *var );
   void setBatchSize_l( double var );
   void setBoilSize_l( double var );
   void setBoilTime_min( double var );
   void setEfficiency_pct( double var );
   
   void addHop( Hop *var );
   void addFermentable( Fermentable* var );
   void addMisc( Misc* var );
   void addYeast( Yeast* var );
   void addWater( Water* var );
   
   void setMash( Mash *var );

   void setAsstBrewer( const std::string &var );
   void setEquipment( Equipment *var );
   void setNotes( const std::string &var );
   void setTasteNotes( const std::string &var );
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
   void setDate( const std::string &var );
   void setCarbonation_vols( double var );
   void setForcedCarbonation( bool var );
   void setPrimingSugarName( const std::string &var );
   void setCarbonationTemp_c( double var );
   void setPrimingSugarEquiv( double var );
   void setKegPrimingFactor( double var );

   std::string getName() const;
   std::string getType() const;
   std::string getBrewer() const;
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

   std::string getAsstBrewer() const;
   Equipment* getEquipment() const;
   std::string getNotes() const;
   std::string getTasteNotes() const;
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
   std::string getDate() const;
   double getCarbonation_vols() const;
   bool getForcedCarbonation() const;
   std::string getPrimingSugarName() const;
   double getCarbonationTemp_c() const;
   double getPrimingSugarEquiv() const;
   double getKegPrimingFactor() const;

private:

   std::string name;
   static const int version = 1;
   std::string type;
   std::string brewer;
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
   
   std::string asstBrewer;
   Equipment* equipment;
   std::string notes;
   std::string tasteNotes;
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
   std::string date;
   double carbonation_vols;
   bool forcedCarbonation;
   std::string primingSugarName;
   double carbonationTemp_c;
   double primingSugarEquiv;
   double kegPrimingFactor;
   
   void setDefaults();
   bool isValidType( const std::string &str );
};

class RecipeException : public std::exception
{
public:

   virtual const char* what() const throw()
   {
      return std::string("BeerXML RECIPE error: " + _err + "\n").c_str();
   }

   RecipeException( std::string message )
   {
      _err = message;
   }

   ~RecipeException() throw() {}

private:

   std::string _err;
};
