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
}

BrewLogWidget::BrewLogWidget(QWidget *parent) : QWidget(parent) {
   setupUi(this);
   setObjectName("BrewLogWidget");

   SMART_FIELD_INIT(BrewLogWidget, label_Fg            , lineEdit_Fg          , BrewLog, PropertyNames::BrewLog::fg              );
   SMART_FIELD_INIT(BrewLogWidget, label_Og            , lineEdit_Og          , BrewLog, PropertyNames::BrewLog::og              );
   SMART_FIELD_INIT(BrewLogWidget, label_Sg            , lineEdit_Sg          , BrewLog, PropertyNames::BrewLog::sg              );
   SMART_FIELD_INIT(BrewLogWidget, label_mashFinTemp   , lineEdit_mashFinTemp , BrewLog, PropertyNames::BrewLog::mashFinTemp_c   , 1);
   SMART_FIELD_INIT(BrewLogWidget, label_pitchTemp     , lineEdit_pitchTemp   , BrewLog, PropertyNames::BrewLog::pitchTemp_c     , 1);
   SMART_FIELD_INIT(BrewLogWidget, label_strikeTemp    , lineEdit_strikeTemp  , BrewLog, PropertyNames::BrewLog::strikeTemp_c    , 1);
   SMART_FIELD_INIT(BrewLogWidget, label_finalVolume   , lineEdit_finalVolume , BrewLog, PropertyNames::BrewLog::finalVolume_l   , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_postBoilVol   , lineEdit_postBoilVol , BrewLog, PropertyNames::BrewLog::postBoilVolume_l, 2);
   SMART_FIELD_INIT(BrewLogWidget, label_volIntoBk     , lineEdit_volIntoBk   , BrewLog, PropertyNames::BrewLog::volumeIntoBK_l  , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_volIntoFerm   , lineEdit_volIntoFerm , BrewLog, PropertyNames::BrewLog::volumeIntoFerm_l, 2);
//   SMART_FIELD_INIT(BrewLogWidget, label_fermentDate   , lineEdit_fermentDate  , BrewLog, PropertyNames::BrewLog::fermentDate     ); No specialisation for QDateTimeEdit
   SMART_FIELD_INIT(BrewLogWidget, label_projectedOg   , lcdnumber_projectedOG , BrewLog, PropertyNames::BrewLog::projOg          );
   SMART_FIELD_INIT(BrewLogWidget, label_effInfoBk     , lcdnumber_effBK       , BrewLog, PropertyNames::BrewLog::effIntoBK_pct   , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_brewHouseEff  , lcdnumber_brewhouseEff, BrewLog, PropertyNames::BrewLog::brewhouseEff_pct, 2);
   SMART_FIELD_INIT(BrewLogWidget, label_projectedAbv  , lcdnumber_projABV     , BrewLog, PropertyNames::BrewLog::projABV_pct     , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_Abv           , lcdnumber_abv         , BrewLog, PropertyNames::BrewLog::abv             , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_yeastProjAtten, lcdnumber_projAtten   , BrewLog, PropertyNames::BrewLog::projAtten       , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_yeastAtten    , lcdnumber_atten       , BrewLog, PropertyNames::BrewLog::attenuation     , 2);
   SMART_FIELD_INIT(BrewLogWidget, label_batchNumber   , lineEdit_batchNumber  , BrewLog, PropertyNames::NamedEntity::name        );

   connect(this->dateEdit_brewDate,    &QDateTimeEdit::dateChanged,    this, &BrewLogWidget::updateBrewDate        );
   connect(this->lineEdit_batchNumber, &SmartLineEdit::textModified,   this, &BrewLogWidget::updateName            );
   connect(this->lineEdit_Sg,          &SmartLineEdit::textModified,   this, &BrewLogWidget::updateSG              );
   connect(this->lineEdit_volIntoBk,   &SmartLineEdit::textModified,   this, &BrewLogWidget::updateVolumeIntoBK_l  );
   connect(this->lineEdit_strikeTemp,  &SmartLineEdit::textModified,   this, &BrewLogWidget::updateStrikeTemp_c    );
   connect(this->lineEdit_mashFinTemp, &SmartLineEdit::textModified,   this, &BrewLogWidget::updateMashFinTemp_c   );
   connect(this->lineEdit_Og,          &SmartLineEdit::textModified,   this, &BrewLogWidget::updateOG              );
   connect(this->lineEdit_postBoilVol, &SmartLineEdit::textModified,   this, &BrewLogWidget::updatePostBoilVolume_l);
   connect(this->lineEdit_volIntoFerm, &SmartLineEdit::textModified,   this, &BrewLogWidget::updateVolumeIntoFerm_l);
   connect(this->lineEdit_pitchTemp,   &SmartLineEdit::textModified,   this, &BrewLogWidget::updatePitchTemp_c     );
   connect(this->lineEdit_Fg,          &SmartLineEdit::textModified,   this, &BrewLogWidget::updateFG              );
   connect(this->lineEdit_finalVolume, &SmartLineEdit::textModified,   this, &BrewLogWidget::updateFinalVolume_l   );
   connect(this->dateEdit_fermentDate, &QDateTimeEdit::dateChanged,    this, &BrewLogWidget::updateFermentDate     );
   connect(this->btTextEdit_brewLogs,  &BtTextEdit::textModified,      this, &BrewLogWidget::updateNotes           );

   // A few labels on this page need special handling, so I connect them here
   // instead of how we would normally do this.
   connect(this->label_projectedOg, &SmartLabel::changedSystemOfMeasurementOrScale, this, &BrewLogWidget::updateProjOg);

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


void BrewLogWidget::updateProjOg() {
   // SmartDigitWidget::setLowLim and SmartDigitWidget::setHighLim take their parameter in canonical units -- in this
   // case, SG.
   double const quant = this->m_brewLog->projOg();
   this->lcdnumber_projectedOG->setLowLim( lowLimitPct  * quant);
   this->lcdnumber_projectedOG->setHighLim(highLimitPct * quant);

   Measurement::UnitSystem const & displayUnitSystem = this->label_projectedOg->getDisplayUnitSystem();
   int const precision = (displayUnitSystem == Measurement::UnitSystems::density_Plato) ? 0 : 3;

   // Set precision before setting amount as setPrecision does not update the display, whereas setQuantity does
   this->lcdnumber_projectedOG->setPrecision(precision);
   this->lcdnumber_projectedOG->setQuantity(quant);
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
      this->lcdnumber_effBK->setLowLim (m_brewLog->projEff_pct() * lowLimitPct);
      this->lcdnumber_effBK->setHighLim(m_brewLog->projEff_pct() * highLimitPct);

      this->lcdnumber_projectedOG->setLowLim (m_brewLog->projOg() * lowLimitPct);
      this->lcdnumber_projectedOG->setHighLim(m_brewLog->projOg() * highLimitPct);

      this->lcdnumber_brewhouseEff->setLowLim (m_brewLog->projEff_pct() * lowLimitPct);
      this->lcdnumber_brewhouseEff->setHighLim(m_brewLog->projEff_pct() * highLimitPct);

      this->lcdnumber_projABV->setLowLim (m_brewLog->projABV_pct() * lowLimitPct);
      this->lcdnumber_projABV->setHighLim(m_brewLog->projABV_pct() * highLimitPct);

      this->lcdnumber_abv->setLowLim (m_brewLog->projABV_pct() * lowLimitPct);
      this->lcdnumber_abv->setHighLim(m_brewLog->projABV_pct() * highLimitPct);

      this->lcdnumber_atten->setLowLim (m_brewLog->projAtten() * lowLimitPct);
      this->lcdnumber_atten->setHighLim(m_brewLog->projAtten() * highLimitPct);

      this->lcdnumber_projAtten->setLowLim (m_brewLog->projAtten() * lowLimitPct);
      this->lcdnumber_projAtten->setHighLim(m_brewLog->projAtten() * highLimitPct);

      this->showChanges();
   }
   return;
}

BrewLog * BrewLogWidget::brewLog() const {
   return this->m_brewLog;
}

void BrewLogWidget::updateBrewDate(QDate const & datetime)    { if (this->m_brewLog) { this->m_brewLog->setBrewDate        (datetime);                                                                 } return; }
void BrewLogWidget::updateName()                              { if (this->m_brewLog) { this->m_brewLog->setName            (this->lineEdit_batchNumber->text()                 );                      } return; }
void BrewLogWidget::updateSG()                                { if (this->m_brewLog) { this->m_brewLog->setSg              (this->lineEdit_Sg         ->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateVolumeIntoBK_l()                    { if (this->m_brewLog) { this->m_brewLog->setVolumeIntoBK_l  (this->lineEdit_volIntoBk  ->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateStrikeTemp_c()                      { if (this->m_brewLog) { this->m_brewLog->setStrikeTemp_c    (this->lineEdit_strikeTemp ->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateMashFinTemp_c()                     { if (this->m_brewLog) { this->m_brewLog->setMashFinTemp_c   (this->lineEdit_mashFinTemp->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updateOG()                                { if (this->m_brewLog) { this->m_brewLog->setOg              (this->lineEdit_Og         ->getNonOptCanonicalQty());                      } return; }
void BrewLogWidget::updatePostBoilVolume_l()                  { if (this->m_brewLog) { this->m_brewLog->setPostBoilVolume_l(this->lineEdit_postBoilVol->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewLogWidget::updateVolumeIntoFerm_l()                  { if (this->m_brewLog) { this->m_brewLog->setVolumeIntoFerm_l(this->lineEdit_volIntoFerm->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewLogWidget::updatePitchTemp_c()                       { if (this->m_brewLog) { this->m_brewLog->setPitchTemp_c     (this->lineEdit_pitchTemp  ->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewLogWidget::updateFG()                                { if (this->m_brewLog) { this->m_brewLog->setFg              (this->lineEdit_Fg         ->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewLogWidget::updateFinalVolume_l()                     { if (this->m_brewLog) { this->m_brewLog->setFinalVolume_l   (this->lineEdit_finalVolume->getNonOptCanonicalQty());                      } return; }
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
   this->lineEdit_Sg         ->setQuantity(m_brewLog->sg              ());
   this->lineEdit_volIntoBk  ->setQuantity(m_brewLog->volumeIntoBK_l  ());
   this->lineEdit_strikeTemp ->setQuantity(m_brewLog->strikeTemp_c    ());
   this->lineEdit_mashFinTemp->setQuantity(m_brewLog->mashFinTemp_c   ());
   this->lineEdit_Og         ->setQuantity(m_brewLog->og              ());
   this->lineEdit_postBoilVol->setQuantity(m_brewLog->postBoilVolume_l());
   this->lineEdit_volIntoFerm->setQuantity(m_brewLog->volumeIntoFerm_l());
   this->lineEdit_pitchTemp  ->setQuantity(m_brewLog->pitchTemp_c     ());
   this->lineEdit_Fg         ->setQuantity(m_brewLog->fg              ());
   this->lineEdit_finalVolume->setQuantity(m_brewLog->finalVolume_l   ());
   this->dateEdit_fermentDate->setDate    (m_brewLog->fermentDate     ());
   this->btTextEdit_brewLogs->setPlainText(m_brewLog->notes           ());

   // Now with the calculated stuff
   this->lcdnumber_effBK->setQuantity(m_brewLog->effIntoBK_pct());

   // Need to think about these? Maybe use the bubbles?
   this->updateProjOg(); // this requires more work, but updateProj does it

   this->lcdnumber_brewhouseEff->setQuantity(m_brewLog->brewhouseEff_pct());
   this->lcdnumber_projABV     ->setQuantity(m_brewLog->projABV_pct     ());
   this->lcdnumber_abv         ->setQuantity(m_brewLog->abv             ());
   this->lcdnumber_atten       ->setQuantity(m_brewLog->attenuation     ());
   this->lcdnumber_projAtten   ->setQuantity(m_brewLog->projAtten       ());
   return;
}

void BrewLogWidget::focusOutEvent([[maybe_unused]] QFocusEvent * e) {
   return;
}
