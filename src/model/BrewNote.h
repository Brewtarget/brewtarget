/*
 * model/BrewNote.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
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
#ifndef MODEL_BREWNOTE_H
#define MODEL_BREWNOTE_H

#include <QDate>
#include <QDomDocument>
#include <QDomNode>
#include <QString>
#include <QStringList>

#include "model/NamedEntity.h"
namespace PropertyNames::BrewNote { static char const * const fg = "fg"; /* previously kpropFG */ }
namespace PropertyNames::BrewNote { static char const * const og = "og"; /* previously kpropOG */ }
namespace PropertyNames::BrewNote { static char const * const notes = "notes"; /* previously kpropNotes */ }
namespace PropertyNames::BrewNote { static char const * const projVolIntoFerm_l = "projVolIntoFerm_l"; /* previously kpropProjVolIntoFerm */ }
namespace PropertyNames::BrewNote { static char const * const projVolIntoBK_l = "projVolIntoBK_l"; /* previously kpropProjVolIntoBoil */ }
namespace PropertyNames::BrewNote { static char const * const postBoilVolume_l = "postBoilVolume_l"; /* previously kpropPostBoilVol */ }
namespace PropertyNames::BrewNote { static char const * const projBoilGrav = "projBoilGrav"; /* previously kpropProjBoilGrav */ }
namespace PropertyNames::BrewNote { static char const * const projFermPoints = "projFermPoints"; /* previously kpropProjFermPnts */ }
namespace PropertyNames::BrewNote { static char const * const projPoints = "projPoints"; /* previously kpropProjPnts */ }
namespace PropertyNames::BrewNote { static char const * const projOg = "projOg"; /* previously kpropProjOG */ }
namespace PropertyNames::BrewNote { static char const * const projFg = "projFg"; /* previously kpropProjFG */ }
namespace PropertyNames::BrewNote { static char const * const projEff_pct = "projEff_pct"; /* previously kpropProjEff */ }
namespace PropertyNames::BrewNote { static char const * const projABV_pct = "projABV_pct"; /* previously kpropProjABV */ }
namespace PropertyNames::BrewNote { static char const * const projAtten = "projAtten"; /* previously kpropProjAtten */ }
namespace PropertyNames::BrewNote { static char const * const projMashFinTemp_c = "projMashFinTemp_c"; /* previously kpropProjMashFinTemp */ }
namespace PropertyNames::BrewNote { static char const * const projStrikeTemp_c = "projStrikeTemp_c"; /* previously kpropProjStrikeTemp */ }
namespace PropertyNames::BrewNote { static char const * const boilOff_l = "boilOff_l"; /* previously kpropBoilOff */ }
namespace PropertyNames::BrewNote { static char const * const volumeIntoFerm_l = "volumeIntoFerm_l"; /* previously kpropVolIntoFerm */ }
namespace PropertyNames::BrewNote { static char const * const volumeIntoBK_l = "volumeIntoBK_l"; /* previously kpropVolIntoBoil */ }
namespace PropertyNames::BrewNote { static char const * const brewhouseEff_pct = "brewhouseEff_pct"; /* previously kpropBrewhsEff */ }
namespace PropertyNames::BrewNote { static char const * const finalVolume_l = "finalVolume_l"; /* previously kpropFinVol */ }
namespace PropertyNames::BrewNote { static char const * const pitchTemp_c = "pitchTemp_c"; /* previously kpropPitchTemp */ }
namespace PropertyNames::BrewNote { static char const * const mashFinTemp_c = "mashFinTemp_c"; /* previously kpropMashFinTemp */ }
namespace PropertyNames::BrewNote { static char const * const strikeTemp_c = "strikeTemp_c"; /* previously kpropStrikeTemp */ }
namespace PropertyNames::BrewNote { static char const * const effIntoBK_pct = "effIntoBK_pct"; /* previously kpropEffIntoBoil */ }
namespace PropertyNames::BrewNote { static char const * const attenuation = "attenuation"; /* previously kpropAtten */ }
namespace PropertyNames::BrewNote { static char const * const abv = "abv"; /* previously kpropABV */ }
namespace PropertyNames::BrewNote { static char const * const sg = "sg"; /* previously kpropSG */ }
namespace PropertyNames::BrewNote { static char const * const fermentDate = "fermentDate"; /* previously kpropFermDate */ }
namespace PropertyNames::BrewNote { static char const * const brewDate = "brewDate"; /* previously kpropBrewDate */ }

// Forward declarations;
class Recipe;

/*!
 * \class BrewNote
 * \author Mik Firestone
 *
 * \brief Model for a brewnote record, which records what you did on brewday.
 */
class BrewNote : public NamedEntity
{
   Q_OBJECT
   friend class Database;
   friend class BeerXML;

public:
   BrewNote(QString name, bool cache = true);
   virtual ~BrewNote() = default;

   bool operator<(BrewNote const & other) const;
   bool operator>(BrewNote const & other) const;

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
   void setABV(double var);
   void setAttenuation(double var);
   void setBrewDate(QDateTime const& date = QDateTime::currentDateTime());
   void setFermentDate(QDateTime const& date);
   void setNotes(const QString& var);
   void setSg(double var);
   void setVolumeIntoBK_l(double var);
   void setBrewhouseEff_pct(double var);
   void setEffIntoBK_pct(double var);
   void setStrikeTemp_c(double var);
   void setMashFinTemp_c(double var);
   void setOg(double var);
   void setPostBoilVolume_l(double var);
   void setVolumeIntoFerm_l(double var);
   void setPitchTemp_c(double var);
   void setFg(double var);
   void setFinalVolume_l(double var);
   void setBoilOff_l(double var);
   // Metasetter
   void populateNote(Recipe* parent);
   void recalculateEff(Recipe* parent);
   void setLoading(bool flag);
   void setCacheOnly(bool cache);
   void setRecipe(Recipe * recipe);

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
   void setProjBoilGrav(double var);
   void setProjVolIntoBK_l(double var);
   void setProjStrikeTemp_c(double var);
   void setProjMashFinTemp_c(double var);
   void setProjOg(double var);
   void setProjVolIntoFerm_l(double var);
   void setProjFg(double var);
   void setProjEff_pct(double var);
   void setProjABV_pct(double var);
   void setProjPoints(double var);
   void setProjFermPoints(double var);
   void setProjAtten(double var);

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
   bool cacheOnly() const;

   // BrewNote objects do not have parents
   NamedEntity * getParent() { return nullptr; }
   virtual int insertInDatabase();
   virtual void removeFromDatabase();

signals:
   void brewDateChanged(const QDateTime&);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

private:
   BrewNote(TableSchema* table, QSqlRecord rec, int t_key = -1);
   /*
   BrewNote(Brewtarget::DBTable table, int key);
   BrewNote(QDateTime dateNow, bool cache = true, QString const & name = "");
   */
   BrewNote(BrewNote const& other);
   bool loading;

   QDateTime m_brewDate;
   QDateTime m_fermentDate;
   QString m_notes;
   double m_sg;
   double m_abv;
   double m_effIntoBK_pct;
   double m_brewhouseEff_pct;
   double m_volumeIntoBK_l;
   double m_strikeTemp_c;
   double m_mashFinTemp_c;
   double m_og;
   double m_postBoilVolume_l;
   double m_volumeIntoFerm_l;
   double m_pitchTemp_c;
   double m_fg;
   double m_attenuation;
   double m_finalVolume_l;
   double m_boilOff_l;
   double m_projBoilGrav;
   double m_projVolIntoBK_l;
   double m_projStrikeTemp_c;
   double m_projMashFinTemp_c;
   double m_projOg;
   double m_projVolIntoFerm_l;
   double m_projFg;
   double m_projEff_pct;
   double m_projABV_pct;
   double m_projPoints;
   double m_projFermPoints;
   double m_projAtten;
   bool m_cacheOnly;
   Recipe * m_recipe;

///   QHash<QString,double> info;
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
