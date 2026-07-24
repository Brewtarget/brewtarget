/*======================================================================================================================
 * BrewLogWidget.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#include "BrewLogWidget.h"

#include <QDate>
#include <QDebug>

#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/UnitSystem.h"
#include "model/BrewLog.h"
#include "PersistentSettings.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BrewLogWidget.cpp"
#endif

namespace {
   double constexpr lowLimitPct  = 0.95;
   double constexpr highLimitPct = 1.05;

   void updateSgField(SmartLabel const & smartLabel,
                      SmartDigitWidget & smartDigitWidget,
                      double const sgValue) {
      smartDigitWidget.setLowLim ( lowLimitPct * sgValue);
      smartDigitWidget.setHighLim(highLimitPct * sgValue);

      Measurement::UnitSystem const & displayUnitSystem = smartLabel.getDisplayUnitSystem();
      int const precision{
         (displayUnitSystem == Measurement::UnitSystems::density_Plato ||
          displayUnitSystem == Measurement::UnitSystems::density_GravityPoints ) ? 0 : 3
      };
      // Set precision before setting amount as setPrecision does not update the display, whereas setQuantity does
      smartDigitWidget.setPrecision(precision);
      smartDigitWidget.setQuantity(sgValue);
      return;
   }

}

BrewLogWidget::BrewLogWidget(QWidget *parent) : QWidget(parent) {
   setupUi(this);
   setObjectName("BrewLogWidget");

   SMART_FIELD_INIT(BrewLogWidget, label_measuredFinalGravity       , lineEdit_measuredFinalGravity       , BrewLog, PropertyNames::BrewLog::measuredFinalGravity_sg      );
   SMART_FIELD_INIT(BrewLogWidget, label_measuredOriginalGravity    , lineEdit_measuredOriginalGravity    , BrewLog, PropertyNames::BrewLog::measuredOriginalGravity_sg   );
   SMART_FIELD_INIT(BrewLogWidget, label_measuredPreBoilGravity     , lineEdit_measuredPreBoilGravity     , BrewLog, PropertyNames::BrewLog::measuredPreBoilGravity_sg    );
   SMART_FIELD_INIT(BrewLogWidget, label_measuredMashFinalTemp      , lineEdit_measuredMashFinalTemp      , BrewLog, PropertyNames::BrewLog::measuredMashFinalTemp_c      , 1);
   SMART_FIELD_INIT(BrewLogWidget, label_measuredPitchTemp          , lineEdit_measuredPitchTemp          , BrewLog, PropertyNames::BrewLog::measuredPitchTemp_c          , 1);
   SMART_FIELD_INIT(BrewLogWidget, label_measuredStrikeTemp         , lineEdit_measuredStrikeTemp         , BrewLog, PropertyNames::BrewLog::measuredStrikeTemp_c         , 1);
   SMART_FIELD_INIT(BrewLogWidget, label_measuredFinalVolume        , lineEdit_measuredFinalVolume        , BrewLog, PropertyNames::BrewLog::measuredFinalVolume_l        , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_measuredPostBoilVolume     , lineEdit_measuredPostBoilVolume     , BrewLog, PropertyNames::BrewLog::measuredPostBoilVolume_l     , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_measuredPreBoilVolume      , lineEdit_measuredPreBoilVolume      , BrewLog, PropertyNames::BrewLog::measuredPreBoilVolume_l      , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_measuredVolumeIntoFermentor, lineEdit_measuredVolumeIntoFermentor, BrewLog, PropertyNames::BrewLog::measuredVolumeIntoFermentor_l, 2);
//   SMART_FIELD_INIT(BrewLogWidget, label_fermentDate   , lineEdit_fermentDate  , BrewLog, PropertyNames::BrewLog::fermentDate     ); No specialisation for QDateTimeEdit
   SMART_FIELD_INIT(BrewLogWidget, label_expectedOriginalGravity    , lcdnumber_expectedOriginalGravity   , BrewLog, PropertyNames::BrewLog::expectedOriginalGravity_sg   );
   SMART_FIELD_INIT(BrewLogWidget, label_forecastOriginalGravity    , lcdnumber_forecastOriginalGravity   , BrewLog, PropertyNames::BrewLog::forecastOriginalGravity_sg   );
   // TODO: Add forecast here
   SMART_FIELD_INIT(BrewLogWidget, label_computedPreBoilEfficiency  , lcdnumber_computedPreBoilEfficiency , BrewLog, PropertyNames::BrewLog::computedPreBoilEfficiency_pct, 2);
   SMART_FIELD_INIT(BrewLogWidget, label_computedEfficiency         , lcdnumber_computedEfficiency        , BrewLog, PropertyNames::BrewLog::computedEfficiency_pct       , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_expectedAlcoholByVolume    , lcdnumber_expectedAlcoholByVolume   , BrewLog, PropertyNames::BrewLog::expectedAlcoholByVolume_pct  , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_computedAlcoholByVolume    , lcdnumber_computedAlcoholByVolume   , BrewLog, PropertyNames::BrewLog::computedAlcoholByVolume_pct  , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_expectedAttenuation        , lcdnumber_expectedAttenuation       , BrewLog, PropertyNames::BrewLog::expectedAttenuation_pct      , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_computedAttenuation        , lcdnumber_computedAttenuation       , BrewLog, PropertyNames::BrewLog::computedAttenuation_pct      , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_batchNumber                , lineEdit_batchNumber                , BrewLog, PropertyNames::NamedEntity::name                     );

   connect(this->dateEdit_brewDate                   , &QDateTimeEdit::dateChanged , this, &BrewLogWidget::updateBrewDate                 );
   connect(this->lineEdit_batchNumber                , &SmartLineEdit::textModified, this, &BrewLogWidget::updateName                     );
   connect(this->lineEdit_measuredPreBoilGravity     , &SmartLineEdit::textModified, this, &BrewLogWidget::updateMeasuredPreBoilGravity_sg);
   connect(this->lineEdit_measuredPreBoilVolume      , &SmartLineEdit::textModified, this, &BrewLogWidget::updateMeasuredPreBoilVolume_l  );
   connect(this->lineEdit_measuredStrikeTemp         , &SmartLineEdit::textModified, this, &BrewLogWidget::updateMeasuredStrikeTemp_c     );
   connect(this->lineEdit_measuredMashFinalTemp      , &SmartLineEdit::textModified, this, &BrewLogWidget::updateMeasuredMashFinalTemp_c  );
   connect(this->lineEdit_measuredOriginalGravity    , &SmartLineEdit::textModified, this, &BrewLogWidget::updateOG                       );
   connect(this->lineEdit_measuredPostBoilVolume     , &SmartLineEdit::textModified, this, &BrewLogWidget::updateMeasuredPostBoilVolume_l );
   connect(this->lineEdit_measuredVolumeIntoFermentor, &SmartLineEdit::textModified, this, &BrewLogWidget::updateVolumeIntoFermentor_l    );
   connect(this->lineEdit_measuredPitchTemp          , &SmartLineEdit::textModified, this, &BrewLogWidget::updatePitchTemp_c              );
   connect(this->lineEdit_measuredFinalGravity       , &SmartLineEdit::textModified, this, &BrewLogWidget::updateFG                       );
   connect(this->lineEdit_measuredFinalVolume        , &SmartLineEdit::textModified, this, &BrewLogWidget::updateFinalVolume_l            );
   connect(this->dateEdit_fermentDate                , &QDateTimeEdit::dateChanged , this, &BrewLogWidget::updateFermentDate              );
   connect(this->btTextEdit_brewLogs                 , &BtTextEdit::textModified   , this, &BrewLogWidget::updateNotes                    );

   // A few labels on this page need special handling, so I connect them here
   // instead of how we would normally do this.
   connect(this->label_expectedOriginalGravity, &SmartLabel::changedSystemOfMeasurementOrScale, this, &BrewLogWidget::updateExpectedOg);
   connect(this->label_forecastOriginalGravity, &SmartLabel::changedSystemOfMeasurementOrScale, this, &BrewLogWidget::updateForecastOg);

   // I think this might work
   this->updateDateFormat();
   return;
}

BrewLogWidget::~BrewLogWidget() = default;

//.:TBD:. See comment in PitchDialog::updateProductionDate() for how we might re-implement per-field date format
// selection
void BrewLogWidget::updateDateFormat() {
   auto const dateFormat = Localization::getDateFormat();
   QString const format = Localization::numericToStringDateFormat(dateFormat);
   this->dateEdit_brewDate   ->setDisplayFormat(format);
   this->dateEdit_fermentDate->setDisplayFormat(format);
   return;
}

void BrewLogWidget::updateExpectedOg() {
   updateSgField(*this->label_expectedOriginalGravity,
                 *this->lcdnumber_expectedOriginalGravity,
                 this->m_brewLog->expectedOriginalGravity_sg());
   return;
}

void BrewLogWidget::updateForecastOg() {
   updateSgField(*this->label_forecastOriginalGravity,
                 *this->lcdnumber_forecastOriginalGravity,
                 this->m_brewLog->forecastOriginalGravity_sg());
   return;
}

void BrewLogWidget::setBrewLog(BrewLog* bNote) {
   qDebug() << Q_FUNC_INFO << "BrewLog:" << bNote;

   if (this->m_brewLog) {
      disconnect(this->m_brewLog, nullptr, this, nullptr);
   }

   this->m_brewLog = bNote;
   if (bNote) {
      connect(this->m_brewLog, &NamedEntity::changed, this, &BrewLogWidget::changed);

      // Set the highs and the lows for the lcds
      this->lcdnumber_computedPreBoilEfficiency->setLowLim (m_brewLog->expectedEfficiency_pct() * lowLimitPct);
      this->lcdnumber_computedPreBoilEfficiency->setHighLim(m_brewLog->expectedEfficiency_pct() * highLimitPct);

      this->lcdnumber_expectedOriginalGravity->setLowLim (m_brewLog->expectedOriginalGravity_sg() * lowLimitPct);
      this->lcdnumber_expectedOriginalGravity->setHighLim(m_brewLog->expectedOriginalGravity_sg() * highLimitPct);

      this->lcdnumber_forecastOriginalGravity->setLowLim (m_brewLog->forecastOriginalGravity_sg() * lowLimitPct);
      this->lcdnumber_forecastOriginalGravity->setHighLim(m_brewLog->forecastOriginalGravity_sg() * highLimitPct);

      this->lcdnumber_computedEfficiency->setLowLim (m_brewLog->expectedEfficiency_pct() * lowLimitPct);
      this->lcdnumber_computedEfficiency->setHighLim(m_brewLog->expectedEfficiency_pct() * highLimitPct);

      this->lcdnumber_expectedAlcoholByVolume->setLowLim (m_brewLog->expectedAlcoholByVolume_pct() * lowLimitPct);
      this->lcdnumber_expectedAlcoholByVolume->setHighLim(m_brewLog->expectedAlcoholByVolume_pct() * highLimitPct);

      this->lcdnumber_computedAlcoholByVolume->setLowLim (m_brewLog->expectedAlcoholByVolume_pct() * lowLimitPct);
      this->lcdnumber_computedAlcoholByVolume->setHighLim(m_brewLog->expectedAlcoholByVolume_pct() * highLimitPct);

      this->lcdnumber_computedAttenuation->setLowLim (m_brewLog->expectedAttenuation_pct() * lowLimitPct);
      this->lcdnumber_computedAttenuation->setHighLim(m_brewLog->expectedAttenuation_pct() * highLimitPct);

      this->lcdnumber_expectedAttenuation->setLowLim (m_brewLog->expectedAttenuation_pct() * lowLimitPct);
      this->lcdnumber_expectedAttenuation->setHighLim(m_brewLog->expectedAttenuation_pct() * highLimitPct);

      this->showChanges();
   }
   return;
}

BrewLog * BrewLogWidget::brewLog() const {
   return this->m_brewLog;
}

void BrewLogWidget::updateBrewDate(QDate const & datetime)    { if (this->m_brewLog) { this->m_brewLog->setBrewDate        (datetime);                                                                 } return; }
void BrewLogWidget::updateName()                              { if (this->m_brewLog) { this->m_brewLog->setName            (this->lineEdit_batchNumber->text()                 );                      } return; }
void BrewLogWidget::updateMeasuredPreBoilGravity_sg()         { if (this->m_brewLog) { this->m_brewLog->setMeasuredPreBoilGravity_sg(this->lineEdit_measuredPreBoilGravity->getNonOptCanonicalQty());                } return; }
void BrewLogWidget::updateMeasuredPreBoilVolume_l()           { if (this->m_brewLog) { this->m_brewLog->setMeasuredPreBoilVolume_l  (this->lineEdit_measuredPreBoilVolume  ->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateMeasuredStrikeTemp_c()              { if (this->m_brewLog) { this->m_brewLog->setMeasuredStrikeTemp_c    (this->lineEdit_measuredStrikeTemp ->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateMeasuredMashFinalTemp_c()           { if (this->m_brewLog) { this->m_brewLog->setMeasuredMashFinalTemp_c   (this->lineEdit_measuredMashFinalTemp->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateOG()                                { if (this->m_brewLog) { this->m_brewLog->setMeasuredOriginalGravity_sg(this->lineEdit_measuredOriginalGravity         ->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateMeasuredPostBoilVolume_l()          { if (this->m_brewLog) { this->m_brewLog->setMeasuredPostBoilVolume_l(this->lineEdit_measuredPostBoilVolume->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewLogWidget::updateVolumeIntoFermentor_l()             { if (this->m_brewLog) { this->m_brewLog->setMeasuredVolumeIntoFermentor_l(this->lineEdit_measuredVolumeIntoFermentor->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewLogWidget::updatePitchTemp_c()                       { if (this->m_brewLog) { this->m_brewLog->setMeasuredPitchTemp_c     (this->lineEdit_measuredPitchTemp  ->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewLogWidget::updateFG()                                { if (this->m_brewLog) { this->m_brewLog->setMeasuredFinalGravity_sg              (this->lineEdit_measuredFinalGravity         ->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewLogWidget::updateFinalVolume_l()                     { if (this->m_brewLog) { this->m_brewLog->setMeasuredFinalVolume_l   (this->lineEdit_measuredFinalVolume->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateFermentDate(QDate const & datetime) { if (this->m_brewLog) { this->m_brewLog->setFermentDate     (datetime);                                                                 } return; }
void BrewLogWidget::updateNotes()                             { if (this->m_brewLog) { this->m_brewLog->setNotes           (this->btTextEdit_brewLogs->toPlainText() );                                } return; }

void BrewLogWidget::changed([[maybe_unused]] QMetaProperty prop,
                            [[maybe_unused]] QVariant val) {
   if (this->sender() != this->m_brewLog) {
      return;
   }

   this->showChanges();
   return;
}

void BrewLogWidget::showChanges([[maybe_unused]] QString field) {
   if (!this->m_brewLog) {
      return;
   }

   this->dateEdit_brewDate->setDate       (m_brewLog->brewDate        ());
   this->lineEdit_batchNumber->setText    (m_brewLog->name            ());
   this->lineEdit_measuredPreBoilGravity->setQuantity(m_brewLog->measuredPreBoilGravity_sg());
   this->lineEdit_measuredPreBoilVolume ->setQuantity(m_brewLog->measuredPreBoilVolume_l  ());
   this->lineEdit_measuredStrikeTemp ->setQuantity(m_brewLog->measuredStrikeTemp_c    ());
   this->lineEdit_measuredMashFinalTemp->setQuantity(m_brewLog->measuredMashFinalTemp_c   ());
   this->lineEdit_measuredOriginalGravity         ->setQuantity(m_brewLog->measuredOriginalGravity_sg());
   this->lineEdit_measuredPostBoilVolume->setQuantity(m_brewLog->measuredPostBoilVolume_l());
   this->lineEdit_measuredVolumeIntoFermentor->setQuantity(m_brewLog->measuredVolumeIntoFermentor_l());
   this->lineEdit_measuredPitchTemp  ->setQuantity(m_brewLog->measuredPitchTemp_c     ());
   this->lineEdit_measuredFinalGravity         ->setQuantity(m_brewLog->measuredFinalGravity_sg             ());
   this->lineEdit_measuredFinalVolume->setQuantity(m_brewLog->measuredFinalVolume_l   ());
   this->dateEdit_fermentDate->setDate    (m_brewLog->fermentDate     ());
   this->btTextEdit_brewLogs->setPlainText(m_brewLog->notes           ());

   // Now with the calculated stuff
   this->lcdnumber_computedPreBoilEfficiency->setQuantity(m_brewLog->computedPreBoilEfficiency_pct());
   this->lcdnumber_expectedOriginalGravity->setQuantity(m_brewLog->expectedOriginalGravity_sg());

   // Need to think about these? Maybe use the bubbles?
   this->updateExpectedOg();
   this->updateForecastOg();

   this->lcdnumber_computedEfficiency->setQuantity(m_brewLog->computedEfficiency_pct());
   this->lcdnumber_expectedAlcoholByVolume ->setQuantity(m_brewLog->expectedAlcoholByVolume_pct());
   this->lcdnumber_computedAlcoholByVolume ->setQuantity(m_brewLog->computedAlcoholByVolume_pct());
   this->lcdnumber_computedAttenuation->setQuantity(m_brewLog->computedAttenuation_pct());
   this->lcdnumber_expectedAttenuation->setQuantity(m_brewLog->expectedAttenuation_pct());
   return;
}

void BrewLogWidget::focusOutEvent([[maybe_unused]] QFocusEvent * e) {
   return;
}