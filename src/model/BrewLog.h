/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/BrewLog.h is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#ifndef MODEL_BREWLOG_H
#define MODEL_BREWLOG_H
#pragma once

#include <QDate>
#include <QDomDocument>
#include <QDomNode>
#include <QSqlRecord>
#include <QString>
#include <QStringList>

#include "model/OwnedByRecipe.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::BrewLog { inline BtStringConst const property{#property}; }
AddPropertyName(brewDate                     )
AddPropertyName(computedAlcoholByVolume_pct  )
AddPropertyName(computedAttenuation_pct      )
AddPropertyName(computedEfficiency_pct       )
AddPropertyName(computedPreBoilEfficiency_pct)
AddPropertyName(expectedAlcoholByVolume_pct  )
AddPropertyName(expectedAttenuation_pct      )
AddPropertyName(expectedBoilOff_l            )
AddPropertyName(expectedEfficiency_pct       )
AddPropertyName(expectedFinalGravity_sg      )
AddPropertyName(expectedMashFinalTemp_c      )
AddPropertyName(expectedOriginalGravity_sg   )
AddPropertyName(expectedPreBoilGravity_sg    )
AddPropertyName(expectedPreBoilVolume_l      )
AddPropertyName(expectedStrikeTemp_c         )
AddPropertyName(expectedVolumeIntoFermentor_l)
AddPropertyName(fermentDate                  )
AddPropertyName(forecastOriginalGravity_sg   )
AddPropertyName(measuredFinalGravity_sg      )
AddPropertyName(measuredFinalVolume_l        )
AddPropertyName(measuredMashFinalTemp_c      )
AddPropertyName(measuredOriginalGravity_sg   )
AddPropertyName(measuredPitchTemp_c          )
AddPropertyName(measuredPostBoilVolume_l     )
AddPropertyName(measuredPreBoilGravity_sg    )
AddPropertyName(measuredPreBoilVolume_l      )
AddPropertyName(measuredStrikeTemp_c         )
AddPropertyName(measuredVolumeIntoFermentor_l)
AddPropertyName(notes                        )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

// Forward declarations;
class Recipe;

/*!
 * \class BrewLog
 *
 * \brief Model for a brew log, which records what you did on brewday (and afterwards through fermentation up to having
 *        the finished beer).
 *
 *        The "name" of a BrewLog is used as "batch number" in the UI -- ie the brewer's own unique identifier for this
 *        brew.  It can contain numbers and/or letters and/or symbols, so, "number" might seem a bit of a misnomer;
 *        nonetheless, it is the standard term for such an identifying code.
 *
 *        NOTE that \c BrewLog is not part of the official BeerXML or BeerJSON standards.  We add it in to our BeerXML
 *             files, because we can, but TBD whether this is possible with BeerJSON.
 */
class BrewLog : public OwnedByRecipe {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_expectedBoilOff_l                    ();
   static QString localisedName_brewDate                     ();
   static QString localisedName_computedAlcoholByVolume_pct              ();
   static QString localisedName_computedAttenuation_pct      ();
   static QString localisedName_computedEfficiency_pct       ();
   static QString localisedName_computedPreBoilEfficiency_pct();
   static QString localisedName_expectedAlcoholByVolume_pct              ();
   static QString localisedName_expectedAttenuation_pct      ();
   static QString localisedName_expectedEfficiency_pct       ();
   static QString localisedName_expectedFinalGravity_sg      ();
   static QString localisedName_expectedMashFinalTemp_c      ();
   static QString localisedName_expectedOriginalGravity_sg   ();
   static QString localisedName_expectedPreBoilGravity_sg    ();
   static QString localisedName_expectedPreBoilVolume_l      ();
   static QString localisedName_expectedStrikeTemp_c         ();
   static QString localisedName_expectedVolumeIntoFermentor_l();
   static QString localisedName_fermentDate                  ();
   static QString localisedName_forecastOriginalGravity_sg   ();
   static QString localisedName_measuredFinalGravity_sg      ();
   static QString localisedName_measuredFinalVolume_l        ();
   static QString localisedName_measuredMashFinalTemp_c      ();
   static QString localisedName_measuredOriginalGravity_sg   ();
   static QString localisedName_measuredPitchTemp_c          ();
   static QString localisedName_measuredPostBoilVolume_l     ();
   static QString localisedName_measuredPreBoilGravity_sg    ();
   static QString localisedName_measuredPreBoilVolume_l      ();
   static QString localisedName_measuredStrikeTemp_c         ();
   static QString localisedName_measuredVolumeIntoFermentor_l();
   static QString localisedName_notes                        ();
   static QString localisedName_projFermPoints               ();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   explicit BrewLog(QString name = "");
   explicit BrewLog(Recipe const & recipe);
   explicit BrewLog(QDate dateNow, QString const & name = "");
   explicit BrewLog(NamedParameterBundle const & namedParameterBundle);
   BrewLog(BrewLog const & other);

   ~BrewLog() override;

   /**
    * \brief BrewLog instances are ordered by date rather than name, so we have to override \c NamedEntity ordering
    */
   std::strong_ordering operator<=>(BrewLog const & other) const;

   //=================================================== PROPERTIES ====================================================
   //! The date of the brewday
   Q_PROPERTY(QDate   brewDate            READ brewDate            WRITE setBrewDate         )
   //! The date fermentation was deemed finished and final gravity readings were taken
   Q_PROPERTY(QDate   fermentDate         READ fermentDate         WRITE setFermentDate      )

   Q_PROPERTY(QString notes               READ notes               WRITE setNotes            )

   //
   // For a lot of properties, we have "planned" vs "actual" values.  In the past, we have used the term "projected" to
   // mean both "what was originally expected" and "what is expected based on current information" -- eg "projected OG"
   // was initially set to the value obtained from the Recipe, and then updated based on measured pre-boil gravity and
   // pre-boil volume, etc.  This is potentially confusing, so we now try to be more rigorous about distinguishing
   // between different types of value.  We use the following terms:
   //
   //    "expected" = Mostly from the Recipe (either directly or by calculation)
   //    "measured" = Supplied by the brewer for this batch -- eg measured OG and FG
   //    "computed" = Derived from values supplied by the brewer -- eg ABV calculated from measured OG and FG
   //    "forecast" = An interim value derived from values supplied by the brewer, but to be superseded by a measured
   //                 value -- eg forecast of OG at the start of the boil
   //
   // These are not necessarily the "best" adjectives (so the wording we use in the UI might be different), but, for the
   // code, they at have the merit of being the same length, which makes related variable names etc line up neatly. :-)
   //

   //! Expected pre-boil specific gravity.
   Q_PROPERTY(double  expectedPreBoilGravity_sg       READ expectedPreBoilGravity_sg       WRITE setExpectedPreBoilGravity_sg    )
   //! Actual (measured) pre-boil specific gravity.
   Q_PROPERTY(double  measuredPreBoilGravity_sg       READ measuredPreBoilGravity_sg       WRITE setMeasuredPreBoilGravity_sg    )
   //! Expected volume of wort collected from mash into boil kettle
   Q_PROPERTY(double  expectedPreBoilVolume_l         READ expectedPreBoilVolume_l         WRITE setExpectedPreBoilVolume_l      )
   //! Actual (measured) volume of wort collected from mash into boil kettle
   Q_PROPERTY(double  measuredPreBoilVolume_l         READ measuredPreBoilVolume_l         WRITE setMeasuredPreBoilVolume_l      )
   //! Expected (planned) strike water temperature (ie water temperature immediately prior to adding grains at mash start)
   Q_PROPERTY(double  expectedStrikeTemp_c            READ expectedStrikeTemp_c            WRITE setExpectedStrikeTemp_c         )
   //! Actual (measured) strike water temperature
   Q_PROPERTY(double  measuredStrikeTemp_c            READ measuredStrikeTemp_c            WRITE setMeasuredStrikeTemp_c         )
   //! Expected (planned) final mash temperature (before any mash out)
   Q_PROPERTY(double  expectedMashFinalTemp_c         READ expectedMashFinalTemp_c         WRITE setExpectedMashFinalTemp_c      )
   //! Actual (measured) final mash temperature (before any mash out)
   Q_PROPERTY(double  measuredMashFinalTemp_c         READ measuredMashFinalTemp_c         WRITE setMeasuredMashFinalTemp_c      )
   //! Expected (planned) original (post-boil, pre-fermentation) specific gravity  TODO: At least in some places this should be computedOG!
   Q_PROPERTY(double  expectedOriginalGravity_sg      READ expectedOriginalGravity_sg      WRITE setExpectedOriginalGravity_sg   )
   //! Forecast original (post-boil, pre-fermentation) specific gravity based on measured pre-boil volume & gravity
   Q_PROPERTY(double  forecastOriginalGravity_sg      READ forecastOriginalGravity_sg      WRITE setForecastOriginalGravity_sg   )
   //! Actual (measured) original (post-boil, pre-fermentation) specific gravity
   Q_PROPERTY(double  measuredOriginalGravity_sg      READ measuredOriginalGravity_sg      WRITE setMeasuredOriginalGravity_sg   )
   // TBD: Do we need expectedPostBoilVolume_l ?
   //! Actual (measured) volume of wort in kettle after boil
   Q_PROPERTY(double  measuredPostBoilVolume_l        READ measuredPostBoilVolume_l        WRITE setMeasuredPostBoilVolume_l     )
   //! Expected (planned) volume of wort into fermentor
   Q_PROPERTY(double  expectedVolumeIntoFermentor_l   READ expectedVolumeIntoFermentor_l   WRITE setExpectedVolumeIntoFermentor_l)
   //! Actual (measured) volume of wort into fermentor
   Q_PROPERTY(double  measuredVolumeIntoFermentor_l   READ measuredVolumeIntoFermentor_l   WRITE setMeasuredVolumeIntoFermentor_l)
   // TBD: Do we need expectedPitchTemp_c ?
   //! Actual (measured) temperature of wort when yeast is pitched
   Q_PROPERTY(double  measuredPitchTemp_c             READ measuredPitchTemp_c             WRITE setMeasuredPitchTemp_c          )
   //! Expected (planned) final (post-fermentation) specific gravity
   Q_PROPERTY(double  expectedFinalGravity_sg         READ expectedFinalGravity_sg         WRITE setExpectedFinalGravity_sg      )
   //! Actual (measured) final (post-fermentation) specific gravity
   Q_PROPERTY(double  measuredFinalGravity_sg         READ measuredFinalGravity_sg         WRITE setMeasuredFinalGravity_sg      )
   // TBD: Do we need expectedFinalVolume_l ?
   //! Actual (measured) final (post-fermentation) volume
   Q_PROPERTY(double  measuredFinalVolume_l           READ measuredFinalVolume_l           WRITE setMeasuredFinalVolume_l        )
   //! Expected alcohol by volume based on the Recipe OG
   Q_PROPERTY(double  expectedAlcoholByVolume_pct     READ expectedAlcoholByVolume_pct     WRITE setExpectedAlcoholByVolume_pct  )
   //! Actual alcohol by volume based on "original" and "final" gravity readings
   Q_PROPERTY(double  computedAlcoholByVolume_pct     READ computedAlcoholByVolume_pct     WRITE setComputedAlcoholByVolume_pct  )
   //! Expected attenuation from the Recipe
   Q_PROPERTY(double  expectedAttenuation_pct         READ expectedAttenuation_pct         WRITE setExpectedAttenuation_pct      )
   //! Actual attenuation based on gravity readings
   Q_PROPERTY(double  computedAttenuation_pct         READ computedAttenuation_pct         WRITE setComputedAttenuation_pct      )
   /**
    * Expected brewhouse (ie overall) efficiency from the Recipe, capturing the combined impact of mash conversion,
    * lautering, kettle losses, and transfer
    */
   Q_PROPERTY(double  expectedEfficiency_pct          READ expectedEfficiency_pct          WRITE setExpectedEfficiency_pct       )
   //! Actual brewhouse (ie overall) efficiency based on gravity readings
   Q_PROPERTY(double  computedEfficiency_pct          READ computedEfficiency_pct          WRITE setComputedEfficiency_pct       )
   /**
    * Actual pre-boil (aka "into boil kettle") efficiency, measuring the percentage of total available sugars that made
    * it into the kettle.
    */
   Q_PROPERTY(double  computedPreBoilEfficiency_pct   READ computedPreBoilEfficiency_pct   WRITE setComputedPreBoilEfficiency_pct)
   /**
    * Expected boil-off based on the equipment profile and the length of the boil
    */
   Q_PROPERTY(double  expectedBoilOff_l           READ expectedBoilOff_l           WRITE setExpectedBoilOff_l        )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   QDate   brewDate                     () const;
   QString brewDate_str                 () const;
   QString brewDate_short               () const;
   QDate   fermentDate                  () const;
   QString fermentDate_str              () const;
   QString fermentDate_short            () const;
   double  measuredPreBoilGravity_sg    () const;
   double  computedAlcoholByVolume_pct  () const;
   double  computedAttenuation_pct      () const;
   double  measuredPreBoilVolume_l      () const;
   double  computedPreBoilEfficiency_pct() const;
   double  computedEfficiency_pct       () const;
   double  measuredStrikeTemp_c         () const;
   double  measuredMashFinalTemp_c      () const;
   double  measuredOriginalGravity_sg   () const;
   double  measuredPostBoilVolume_l     () const;
   double  measuredVolumeIntoFermentor_l() const;
   double  measuredPitchTemp_c          () const;
   double  measuredFinalGravity_sg      () const;
   double  measuredFinalVolume_l        () const;
   double  expectedBoilOff_l            () const;
   QString notes                        () const;
   double  expectedPreBoilGravity_sg    () const;
   double  expectedPreBoilVolume_l      () const;
   double  expectedStrikeTemp_c         () const;
   double  expectedMashFinalTemp_c      () const;
   double  expectedOriginalGravity_sg   () const;
   double  forecastOriginalGravity_sg   () const;
   double  expectedVolumeIntoFermentor_l() const;
   double  expectedFinalGravity_sg      () const;
   double  expectedEfficiency_pct       () const;
   double  expectedAlcoholByVolume_pct  () const;
   double  expectedAttenuation_pct      () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setComputedAlcoholByVolume_pct  (double var);
   void setComputedAttenuation_pct      (double var);
   void setBrewDate                     (QDate   const & var = QDate::currentDate());
   void setFermentDate                  (QDate   const & var);
   void setNotes                        (QString const & var);
   void setMeasuredPreBoilGravity_sg    (double var);
   void setMeasuredPreBoilVolume_l      (double var);
   void setComputedEfficiency_pct       (double var);
   void setComputedPreBoilEfficiency_pct(double var);
   void setMeasuredStrikeTemp_c         (double var);
   void setMeasuredMashFinalTemp_c      (double var);
   void setMeasuredOriginalGravity_sg   (double var);
   void setMeasuredPostBoilVolume_l     (double var);
   void setMeasuredVolumeIntoFermentor_l(double var);
   void setMeasuredPitchTemp_c          (double var);
   void setMeasuredFinalGravity_sg      (double var);
   void setMeasuredFinalVolume_l        (double var);
   void setExpectedBoilOff_l            (double var);
   void setExpectedPreBoilGravity_sg    (double var);
   void setExpectedPreBoilVolume_l      (double var);
   void setExpectedStrikeTemp_c         (double var);
   void setExpectedMashFinalTemp_c      (double var);
   void setExpectedOriginalGravity_sg   (double var);
   void setForecastOriginalGravity_sg   (double var);
   void setExpectedVolumeIntoFermentor_l(double var);
   void setExpectedFinalGravity_sg      (double var);
   void setExpectedEfficiency_pct       (double var);
   void setExpectedAlcoholByVolume_pct  (double var);
   void setExpectedAttenuation_pct      (double var);

   // Metasetter
   void populateNote(Recipe * recipe);
   void setLoading(bool flag);

   // Calculations
   double calculateEffIntoBK_pct();
   double calculateOg();
   double calculateBrewHouseEff_pct();
   //! Projected ABV after fermentation.
   double calculateABV_pct();
   /**
    * Actual ABV after we have measured measuredOriginalGravity_sg/measuredFinalGravity_sg.
    *
    * NB: If the user enters an FG that is larger than OG, this will return NaN
    */
   double calculateActualABV_pct();

   //! Actual computedAttenuation_pct, based on measured measuredOriginalGravity_sg/measuredFinalGravity_sg
   double calculateAttenuation_pct();

   //! Needed by \c TreeModelBase
   static QList<std::shared_ptr<BrewLog>> ownedBy(Recipe const & recipe);

signals:
   void brewDateChanged(QDate const &);

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:

   bool loading;

   QDate   m_brewDate                     ;
   QDate   m_fermentDate                  ;
   QString m_notes                         = "";
   double  m_measuredPreBoilGravity_sg     = 0.0;
   double  m_computedAlcoholByVolume_pct   = 0.0;
   double  m_computedPreBoilEfficiency_pct = 0.0;
   double  m_computedEfficiency_pct        = 0.0;
   double  m_measuredPreBoilVolume_l       = 0.0;
   double  m_measuredStrikeTemp_c          = 0.0;
   double  m_measuredMashFinalTemp_c       = 0.0;
   double  m_measuredOriginalGravity_sg    = 0.0;
   double  m_measuredPostBoilVolume_l      = 0.0;
   double  m_measuredVolumeIntoFermentor_l = 0.0;
   double  m_measuredPitchTemp_c           = 0.0;
   double  m_measuredFinalGravity_sg       = 0.0;
   double  m_computedAttenuation_pct       = 0.0;
   double  m_measuredFinalVolume_l         = 0.0;
   double  m_expectedBoilOff_l             = 0.0;
   double  m_expectedPreBoilGravity_sg     = 0.0;
   double  m_expectedPreBoilVolume_l       = 0.0;
   double  m_expectedStrikeTemp_c          = 0.0;
   double  m_expectedMashFinalTemp_c       = 0.0;
   double  m_expectedOriginalGravity_sg    = 0.0;
   double  m_forecastOriginalGravity_sg    = 0.0;
   double  m_expectedVolumeIntoFermentor_l = 0.0;
   double  m_expectedFinalGravity_sg       = 0.0;
   double  m_expectedEfficiency_pct        = 0.0;
   double  m_expectedAlcoholByVolume_pct   = 0.0;
   double  m_expectedAttenuation_pct       = 0.0;
};

BT_DECLARE_METATYPES(BrewLog)

#endif