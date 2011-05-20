/*  
 * brewnote.h is part of Brewtarget, written by Mik Firestone
 * (mikfire@gmail.com).   Copyright is given to Philip G. Lee
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

#ifndef _BREWNOTE_H
#define _BREWNOTE_H

class BrewNote;

#include <string>
#include <exception>
#include "BeerXMLElement.h"
#include <QDomNode>
#include <QDomDocument>
#include <QString>
#include <QStringList>
#include <QDate>
#include "recipe.h"

class BrewNote : public Observable, public BeerXMLElement
{
public:
   enum {DONOTUSE, RECIPE};

   BrewNote(Recipe* parent);
   BrewNote(BrewNote& other);
   BrewNote(Recipe* parent, const QDomNode& bnoteNode);

   friend bool operator<(BrewNote &b1, BrewNote &b2);
   friend bool operator==(BrewNote &b1, BrewNote &b2);

   virtual void fromNode(const QDomNode& node);
   virtual void toXml(QDomDocument& doc, QDomNode& parent);

   // Sets
   void setParent(Recipe* parent);
   void setBrewDate(QString date = "");
   void setFermentDate(QString date);
   void setNotes(const QString& var);

   void setSG(double var);
   void setVolumeIntoBK(double var);
   void setStrikeTemp(double var);
   void setMashFinTemp(double var);
   void setOG(double var);
   void setPostBoilVolume(double var);
   void setVolumeIntoFerm(double var);
   void setPitchTemp(double var);
   void setFG(double var);
   void setFinalVolume(double var);  
   void setBoilOff(double var);  

   // Gets
   Recipe*   getParent() const;
   QDateTime getBrewDate() const;
   QString   getBrewDate_str() const;
   QString   getBrewDate_short() const;
   QDateTime getFermentDate() const;
   QString   getFermentDate_str() const;

   double getSG() const;
   double getVolumeIntoBK() const;
   double getStrikeTemp() const;
   double getMashFinTemp() const;
   double getOG() const;
   double getPostBoilVolume() const;
   double getVolumeIntoFerm() const;
   double getPitchTemp() const;
   double getFG() const;
   double getFinalVolume() const;  
   double getBoilOff() const;  

   QString getNotes() const;

   // Calculations
   double calculateEffIntoBK();
   double calculateOG();
   double calculateBrewHouseEff();
   double calculateABV();
   double actualABV();

   // Projected values
   void setProjBoilGrav(double var);
   void setProjVolIntoBK(double var);
   void setProjStrikeTemp(double var);
   void setProjMashFinTemp(double var);
   void setProjOG(double var);
   void setProjVolIntoFerm(double var);
   void setProjFG(double var);
   void setProjEff(double var);
   void setProjABV(double var);
   void setProjPoints(double var);
   void setProjAtten(double var);

   double getProjBoilGrav() const;
   double getProjVolIntoBK() const;
   double getProjStrikeTemp() const;
   double getProjMashFinTemp() const;
   double getProjOG() const;
   double getProjVolIntoFerm() const;
   double getProjFG() const;
   double getProjEff() const;
   double getProjABV() const;
   double getProjPoints() const;
   double getProjAtten() const;

private:
   static const int version = 1;
   Recipe* rec;

   QDateTime brewDate;
   QDateTime fermentDate;
   QString notes;

   QHash<QString,double> info;

   QHash<QString,QString> XMLTagToName();
   QHash<QString,QString> NameToXMLTag();


   void setInfo(QString label, double var);
   void setDefaults(Recipe* parent);

   int baseCompare(double lhs, double rhs);
};

inline bool BrewNotePtrLt( BrewNote* lhs, BrewNote* rhs)
{
   return *lhs < *rhs;
}

inline bool BrewNotePtrEq( BrewNote* lhs, BrewNote* rhs)
{
   return *lhs == *rhs;
}

struct BrewNote_ptr_cmp
{
   bool operator()(BrewNote* lhs, BrewNote* rhs)
   {
      return *lhs < *rhs;
   }
};

struct BrewNote_ptr_equals
{
   bool operator()(BrewNote* lhs, BrewNote* rhs)
   {
      return *lhs == *rhs;
   }
};

#endif /* _BREWNOTE_H */
