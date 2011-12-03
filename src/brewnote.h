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

#include <QDomNode>
#include <QDomDocument>
#include <QString>
#include <QStringList>
#include <QDate>
#include "BeerXMLElement.h"

// Forward declarations;
class Recipe;
class BrewNote;


class BrewNote : public BeerXMLElement
{
public:
   enum {DONOTUSE, RECIPE};

   BrewNote(Recipe* parent);
   BrewNote(BrewNote& other);
   BrewNote(Recipe* parent, const QDomNode& bnoteNode);

   virtual ~BrewNote() {}

   friend bool operator<(BrewNote &lhs, BrewNote &rhs);
   friend bool operator==(BrewNote &lhs, BrewNote &rhs);

   virtual void fromNode(const QDomNode& node);
   virtual void toXml(QDomDocument& doc, QDomNode& parent);

   // Sets
   void setParent(Recipe* parent);
   void setBrewDate(QString date = "");
   void setFermentDate(QString date);
   void setNotes(const QString& var);

   void setSG(QString var);
   void setVolumeIntoBK_l(double var);
   void setStrikeTemp_c(double var);
   void setMashFinTemp_c(double var);
   void setOG(QString var);
   void setPostBoilVolume_l(double var);
   void setVolumeIntoFerm_l(double var);
   void setPitchTemp_c(double var);
   void setFG(QString var);
   void setFinalVolume_l(double var);  
   void setBoilOff_l(double var);  

   // Gets
   Recipe*   getParent() const;
   QDateTime getBrewDate() const;
   QString   getBrewDate_str() const;
   QString   getBrewDate_short() const;
   QDateTime getFermentDate() const;
   QString   getFermentDate_str() const;
   QString   getFermentDate_short() const;

   double getSG() const;
   double getVolumeIntoBK_l() const;
   double getStrikeTemp_c() const;
   double getMashFinTemp_c() const;
   double getOG() const;
   double getPostBoilVolume_l() const;
   double getVolumeIntoFerm_l() const;
   double getPitchTemp_c() const;
   double getFG() const;
   double getFinalVolume_l() const;  
   double getBoilOff_l() const;  

   QString getNotes() const;

   // Calculations
   double calculateEffIntoBK_pct();
   double calculateOG();
   double calculateBrewHouseEff_pct();
   double calculateABV_pct();
   double actualABV_pct();

   // Projected values
   void setProjBoilGrav(double var);
   void setProjVolIntoBK_l(double var);
   void setProjStrikeTemp_c(double var);
   void setProjMashFinTemp_c(double var);
   void setProjOG(double var);
   void setProjVolIntoFerm_l(double var);
   void setProjFG(double var);
   void setProjEff_pct(double var);
   void setProjABV_pct(double var);
   void setProjPoints(double var);
   void setProjAtten(double var);

   double getProjBoilGrav() const;
   double getProjVolIntoBK_l() const;
   double getProjStrikeTemp_c() const;
   double getProjMashFinTemp_c() const;
   double getProjOG() const;
   double getProjVolIntoFerm_l() const;
   double getProjFG() const;
   double getProjEff_pct() const;
   double getProjABV_pct() const;
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

   double translateSG(QString qstr);
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
