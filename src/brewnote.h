/*
 * brewnote.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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
bool operator<(BrewNote const& lhs, BrewNote const& rhs);
bool operator==(BrewNote const& lhs, BrewNote const& rhs);

/*!
 * \class BrewNote
 * \author Mik Firestone
 *
 * \brief Model for a brewnote record, which records what you did on brewday.
 */
class BrewNote : public BeerXMLElement
{
   Q_OBJECT
   friend class Database;
   friend bool operator<(BrewNote &lhs, BrewNote &rhs);
   friend bool operator==(BrewNote &lhs, BrewNote &rhs);
   
public:
   enum {DONOTUSE, RECIPE};

   virtual ~BrewNote() {}

   static QString classNameStr();

   Q_PROPERTY( QDateTime brewDate READ brewDate WRITE setBrewDate /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( QDateTime fermentDate READ fermentDate  WRITE setFermentDate /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double sg READ sg WRITE setSg /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double abv READ abv WRITE setABV /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double effIntoBK_pct READ effIntoBK_pct WRITE setEffIntoBK_pct STORED false )
   Q_PROPERTY( double brewhouseEff_pct READ brewhouseEff_pct WRITE setBrewhouseEff_pct STORED false )
   Q_PROPERTY( double volumeIntoBK_l READ volumeIntoBK_l WRITE setVolumeIntoBK_l /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double strikeTemp_c READ strikeTemp_c WRITE setStrikeTemp_c /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double mashFinTemp_c READ mashFinTemp_c WRITE setMashFinTemp_c /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double og READ og WRITE setOg /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double postBoilVolume_l READ postBoilVolume_l WRITE setPostBoilVolume_l /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double volumeIntoFerm_l READ volumeIntoFerm_l WRITE setVolumeIntoFerm_l /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double pitchTemp_c READ pitchTemp_c WRITE setPitchTemp_c /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double fg READ fg WRITE setFg /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double attenuation READ attenuation WRITE setAttenuation /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double finalVolume_l READ finalVolume_l WRITE setFinalVolume_l /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double boilOff_l READ boilOff_l WRITE setBoilOff_l /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projBoilGrav READ projBoilGrav WRITE setProjBoilGrav /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projVolIntoBK_l READ projVolIntoBK_l WRITE setProjVolIntoBK_l /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projStrikeTemp_c READ projStrikeTemp_c WRITE setProjStrikeTemp_c /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projMashFinTemp_c READ projMashFinTemp_c WRITE setProjMashFinTemp_c /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projOg READ projOg WRITE setProjOg /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projVolIntoFerm_l READ projVolIntoFerm_l WRITE setProjVolIntoFerm_l /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projFg READ projFg WRITE setProjFg /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projEff_pct READ projEff_pct WRITE setProjEff_pct /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projABV_pct READ projABV_pct WRITE setProjABV_pct /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projPoints READ projPoints WRITE setProjPoints /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projFermPoints READ projFermPoints WRITE setProjFermPoints /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( double projAtten READ projAtten WRITE setProjAtten /*NOTIFY changed*/ STORED false )

   // Setters
   void setABV(double var, bool cacheOnly = false);
   void setAttenuation(double var, bool cacheOnly = false);
   void setBrewDate(QDateTime const& date = QDateTime::currentDateTime(), bool cacheOnly = false);
   void setFermentDate(QDateTime const& date, bool cacheOnly = false);
   void setNotes(const QString& var, bool cacheOnly = false);
   void setSg(double var, bool cacheOnly = false);
   void setVolumeIntoBK_l(double var, bool cacheOnly = false);
   void setBrewhouseEff_pct(double var, bool cacheOnly = false);
   void setEffIntoBK_pct(double var, bool cacheOnly = false);
   void setStrikeTemp_c(double var, bool cacheOnly = false);
   void setMashFinTemp_c(double var, bool cacheOnly = false);
   void setOg(double var, bool cacheOnly = false);
   void setPostBoilVolume_l(double var, bool cacheOnly = false);
   void setVolumeIntoFerm_l(double var, bool cacheOnly = false);
   void setPitchTemp_c(double var, bool cacheOnly = false);
   void setFg(double var, bool cacheOnly = false);
   void setFinalVolume_l(double var, bool cacheOnly = false);  
   void setBoilOff_l(double var, bool cacheOnly = false);
   // Metasetter
   void populateNote(Recipe* parent);
   void recalculateEff(Recipe* parent);
   void setLoading(bool flag);

   // Getters
   QDateTime brewDate() const;
   //! Convenience method.
   QString   brewDate_str() const;
   //! Convenience method.
   QString   brewDate_short() const;
    QDateTime fermentDate() const;
   //! Convenience method.
   QString   fermentDate_str() const;
   //! Convenience method.
   QString   fermentDate_short() const;
   double sg() const;
   double abv() const;
   double attenuation() const;
   double volumeIntoBK_l() const;
   double effIntoBK_pct() const;
   double brewhouseEff_pct() const;
   double strikeTemp_c() const;
   double mashFinTemp_c() const;
   double og() const;
   double postBoilVolume_l() const;
   double volumeIntoFerm_l() const;
   double pitchTemp_c() const;
   double fg() const;
   double finalVolume_l() const;  
   double boilOff_l() const;  
   QString notes() const;
   // ick, but I don't see another way. I need a unique key that has *nothing*
   // to do with the data entered. The best one I can think of is the
   // database's key
   int key() const;

   // Calculations
   double calculateEffIntoBK_pct();
   double calculateOg();
   double calculateBrewHouseEff_pct();
   //! Projected ABV after fermentation.
   double calculateABV_pct();
   //! Actual ABV after we have measured og/fg.
   double calculateActualABV_pct();
   //! Actual attenuation, based on measured og/fg
   double calculateAttenuation_pct();

   // Projected values
   void setProjBoilGrav(double var, bool cacheOnly = false);
   void setProjVolIntoBK_l(double var, bool cacheOnly = false);
   void setProjStrikeTemp_c(double var, bool cacheOnly = false);
   void setProjMashFinTemp_c(double var, bool cacheOnly = false);
   void setProjOg(double var, bool cacheOnly = false);
   void setProjVolIntoFerm_l(double var, bool cacheOnly = false);
   void setProjFg(double var, bool cacheOnly = false);
   void setProjEff_pct(double var, bool cacheOnly = false);
   void setProjABV_pct(double var, bool cacheOnly = false);
   void setProjPoints(double var, bool cacheOnly = false);
   void setProjFermPoints(double var, bool cacheOnly = false);
   void setProjAtten(double var, bool cacheOnly = false);

   double projBoilGrav() const;
   double projVolIntoBK_l() const;
   double projStrikeTemp_c() const;
   double projMashFinTemp_c() const;
   double projOg() const;
   double projVolIntoFerm_l() const;
   double projFg() const;
   double projEff_pct() const;
   double projABV_pct() const;
   double projPoints() const;
   double projFermPoints() const;
   double projAtten() const;
   
signals:
   void brewDateChanged(const QDateTime&);

private:
   BrewNote(Brewtarget::DBTable table, int key);
   BrewNote(Brewtarget::DBTable table, int key, QSqlRecord rec);
   BrewNote(BrewNote const& other);
   bool loading;

   QDateTime _brewDate;
   QDateTime _fermentDate;
   QString _notes;
   double _sg;
   double _abv;
   double _effIntoBK_pct;
   double _brewhouseEff_pct;
   double _volumeIntoBK_l;
   double _strikeTemp_c;
   double _mashFinTemp_c;
   double _og;
   double _postBoilVolume_l;
   double _volumeIntoFerm_l;
   double _pitchTemp_c;
   double _fg;
   double _attenuation;
   double _finalVolume_l;
   double _boilOff_l;
   double _projBoilGrav;
   double _projVolIntoBK_l;
   double _projStrikeTemp_c;
   double _projMashFinTemp_c;
   double _projOg;
   double _projVolIntoFerm_l;
   double _projFg;
   double _projEff_pct;
   double _projABV_pct;
   double _projPoints;
   double _projFermPoints;
   double _projAtten;

   QHash<QString,double> info;
   QHash<QString,QString> XMLTagToName();
   QHash<QString,QString> NameToXMLTag();
   static QHash<QString,QString> tagToPropHash();
   static QHash<QString,QString> tagToProp;
};

Q_DECLARE_METATYPE( QList<BrewNote*> )

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
