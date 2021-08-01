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
#pragma once

#include <QDate>
#include <QDomDocument>
#include <QDomNode>
#include <QSqlRecord>
#include <QString>
#include <QStringList>

#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::BrewNote {static char const * const property = #property; }
AddPropertyName(abv)
AddPropertyName(attenuation)
AddPropertyName(boilOff_l)
AddPropertyName(brewDate)
AddPropertyName(brewhouseEff_pct)
AddPropertyName(effIntoBK_pct)
AddPropertyName(fermentDate)
AddPropertyName(fg)
AddPropertyName(finalVolume_l)
AddPropertyName(mashFinTemp_c)
AddPropertyName(notes)
AddPropertyName(og)
AddPropertyName(pitchTemp_c)
AddPropertyName(postBoilVolume_l)
AddPropertyName(projABV_pct)
AddPropertyName(projAtten)
AddPropertyName(projBoilGrav)
AddPropertyName(projEff_pct)
AddPropertyName(projFermPoints)
AddPropertyName(projFg)
AddPropertyName(projMashFinTemp_c)
AddPropertyName(projOg)
AddPropertyName(projPoints)
AddPropertyName(projStrikeTemp_c)
AddPropertyName(projVolIntoBK_l)
AddPropertyName(projVolIntoFerm_l)
AddPropertyName(recipeId)
AddPropertyName(sg)
AddPropertyName(strikeTemp_c)
AddPropertyName(volumeIntoBK_l)
AddPropertyName(volumeIntoFerm_l)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

// Forward declarations;
class Recipe;

/*!
 * \class BrewNote
 *
 * \brief Model for a brewnote record, which records what you did on brewday.
 */
class BrewNote : public NamedEntity {
   Q_OBJECT

public:
   BrewNote(QString name = "", bool cache = true);
   BrewNote(Recipe const & recipe);
   BrewNote(QDate dateNow, bool cache = true, QString const & name = "");
   BrewNote(NamedParameterBundle const & namedParameterBundle);
   BrewNote(BrewNote const & other);

   virtual ~BrewNote() = default;

   bool operator<(BrewNote const & other) const;
   bool operator>(BrewNote const & other) const;

   Q_PROPERTY( QDate brewDate READ brewDate WRITE setBrewDate /*NOTIFY changed*/ STORED false )
   Q_PROPERTY( QDate fermentDate READ fermentDate  WRITE setFermentDate /*NOTIFY changed*/ STORED false )
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
   Q_PROPERTY( int    recipeId  READ getRecipeId WRITE setRecipeId STORED false )

   // Setters
   void setABV(double var);
   void setAttenuation(double var);
   void setBrewDate(QDate const& date = QDate::currentDate());
   void setFermentDate(QDate const& date);
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
   void setRecipeId(int recipeId);
   void setRecipe(Recipe * recipe);

   // Getters
   QDate brewDate() const;
   //! Convenience method.
   QString   brewDate_str() const;
   //! Convenience method.
   QString   brewDate_short() const;
   QDate fermentDate() const;
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
   int getRecipeId() const;

/*
 * .:TBD:. Think we can comment this out and rely on same function in base class!
 *
   // ick, but I don't see another way. I need a unique key that has *nothing*
   // to do with the data entered. The best one I can think of is the
   // database's key
   int key() const;
*/
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

   virtual Recipe * getOwningRecipe();

signals:
   void brewDateChanged(const QDate &);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:

   bool loading;

   QDate m_brewDate;
   QDate m_fermentDate;
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
   int  m_recipeId;

};

Q_DECLARE_METATYPE( QList<BrewNote*> )
#endif
