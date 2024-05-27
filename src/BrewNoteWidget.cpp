/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BrewNoteWidget.cpp is part of Brewtarget, and is copyright the following authors 2009-2023:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "BrewNoteWidget.h"

#include <QDate>
#include <QDebug>

#include "Localization.h"
#include "measurement/Measurement.h"
#include "model/BrewNote.h"
#include "PersistentSettings.h"

namespace {
   double const lowLimitPct  = 0.95;
   double const highLimitPct = 1.05;
}

BrewNoteWidget::BrewNoteWidget(QWidget *parent) : QWidget(parent) {
   setupUi(this);
   bNoteObs = 0;
   setObjectName("BrewNoteWidget");

   SMART_FIELD_INIT(BrewNoteWidget, label_Fg         , lineEdit_Fg          , BrewNote, PropertyNames::BrewNote::fg              );
   SMART_FIELD_INIT(BrewNoteWidget, label_Og         , lineEdit_Og          , BrewNote, PropertyNames::BrewNote::og              );
   SMART_FIELD_INIT(BrewNoteWidget, label_Sg         , lineEdit_Sg          , BrewNote, PropertyNames::BrewNote::sg              );
   SMART_FIELD_INIT(BrewNoteWidget, label_mashFinTemp, lineEdit_mashFinTemp , BrewNote, PropertyNames::BrewNote::mashFinTemp_c   );
   SMART_FIELD_INIT(BrewNoteWidget, label_pitchTemp  , lineEdit_pitchTemp   , BrewNote, PropertyNames::BrewNote::pitchTemp_c     );
   SMART_FIELD_INIT(BrewNoteWidget, label_strikeTemp , lineEdit_strikeTemp  , BrewNote, PropertyNames::BrewNote::strikeTemp_c    );
   SMART_FIELD_INIT(BrewNoteWidget, label_finalVolume, lineEdit_finalVolume , BrewNote, PropertyNames::BrewNote::finalVolume_l   );
   SMART_FIELD_INIT(BrewNoteWidget, label_postBoilVol, lineEdit_postBoilVol , BrewNote, PropertyNames::BrewNote::postBoilVolume_l);
   SMART_FIELD_INIT(BrewNoteWidget, label_volIntoBk  , lineEdit_volIntoBk   , BrewNote, PropertyNames::BrewNote::volumeIntoBK_l  );
   SMART_FIELD_INIT(BrewNoteWidget, label_volIntoFerm, lineEdit_volIntoFerm , BrewNote, PropertyNames::BrewNote::volumeIntoFerm_l);
//   SMART_FIELD_INIT(BrewNoteWidget, label_fermentDate   , lineEdit_fermentDate  , BrewNote, PropertyNames::BrewNote::fermentDate     ); No specialisation for QDateTimeEdit
   SMART_FIELD_INIT(BrewNoteWidget, label_projectedOg   , lcdnumber_projectedOG , BrewNote, PropertyNames::BrewNote::projOg          );
   SMART_FIELD_INIT(BrewNoteWidget, label_effInfoBk     , lcdnumber_effBK       , BrewNote, PropertyNames::BrewNote::effIntoBK_pct   , 2);
   SMART_FIELD_INIT(BrewNoteWidget, label_brewHouseEff  , lcdnumber_brewhouseEff, BrewNote, PropertyNames::BrewNote::brewhouseEff_pct, 2);
   SMART_FIELD_INIT(BrewNoteWidget, label_projectedAbv  , lcdnumber_projABV     , BrewNote, PropertyNames::BrewNote::projABV_pct     , 2);
   SMART_FIELD_INIT(BrewNoteWidget, label_Abv           , lcdnumber_abv         , BrewNote, PropertyNames::BrewNote::abv             , 2);
   SMART_FIELD_INIT(BrewNoteWidget, label_yeastProjAtten, lcdnumber_projAtten   , BrewNote, PropertyNames::BrewNote::projAtten       , 2);
   SMART_FIELD_INIT(BrewNoteWidget, label_yeastAtten    , lcdnumber_atten       , BrewNote, PropertyNames::BrewNote::attenuation     , 2);


   connect(this->lineEdit_Sg,          &SmartLineEdit::textModified,   this, &BrewNoteWidget::updateSG              );
   connect(this->lineEdit_volIntoBk,   &SmartLineEdit::textModified,   this, &BrewNoteWidget::updateVolumeIntoBK_l  );
   connect(this->lineEdit_strikeTemp,  &SmartLineEdit::textModified,   this, &BrewNoteWidget::updateStrikeTemp_c    );
   connect(this->lineEdit_mashFinTemp, &SmartLineEdit::textModified,   this, &BrewNoteWidget::updateMashFinTemp_c   );
   connect(this->lineEdit_Og,          &SmartLineEdit::textModified,   this, &BrewNoteWidget::updateOG              );
   connect(this->lineEdit_postBoilVol, &SmartLineEdit::textModified,   this, &BrewNoteWidget::updatePostBoilVolume_l);
   connect(this->lineEdit_volIntoFerm, &SmartLineEdit::textModified,   this, &BrewNoteWidget::updateVolumeIntoFerm_l);
   connect(this->lineEdit_pitchTemp,   &SmartLineEdit::textModified,   this, &BrewNoteWidget::updatePitchTemp_c     );
   connect(this->lineEdit_Fg,          &SmartLineEdit::textModified,   this, &BrewNoteWidget::updateFG              );
   connect(this->lineEdit_finalVolume, &SmartLineEdit::textModified,   this, &BrewNoteWidget::updateFinalVolume_l   );
   connect(this->lineEdit_fermentDate, &QDateTimeEdit::dateChanged,    this, &BrewNoteWidget::updateFermentDate     );
   connect(this->btTextEdit_brewNotes, &BtTextEdit::textModified,      this, &BrewNoteWidget::updateNotes           );

   // A few labels on this page need special handling, so I connect them here
   // instead of how we would normally do this.
   connect(this->label_projectedOg, &SmartLabel::changedSystemOfMeasurementOrScale, this, &BrewNoteWidget::updateProjOg);

   // I think this might work
   updateDateFormat();
   return;
}

BrewNoteWidget::~BrewNoteWidget() = default;

//.:TBD:. See comment in PitchDialog::updateProductionDate() for how we might re-implement per-field date format
// selection
void BrewNoteWidget::updateDateFormat() {
   auto dateFormat = Localization::getDateFormat();
   QString format = Localization::numericToStringDateFormat(dateFormat);
   this->lineEdit_fermentDate->setDisplayFormat(format);
   return;
}


void BrewNoteWidget::updateProjOg() {
   // SmartDigitWidget::setLowLim and SmartDigitWidget::setHighLim take their parameter in canonical units -- in this
   // case, SG.
   double const quant = this->bNoteObs->projOg();
   this->lcdnumber_projectedOG->setLowLim( lowLimitPct  * quant);
   this->lcdnumber_projectedOG->setHighLim(highLimitPct * quant);

   Measurement::UnitSystem const & displayUnitSystem = this->label_projectedOg->getDisplayUnitSystem();
   int precision = (displayUnitSystem == Measurement::UnitSystems::density_Plato) ? 0 : 3;

   // Set precision before setting amount as setPrecision does not update the display, whereas setQuantity does
   this->lcdnumber_projectedOG->setPrecision(precision);
   this->lcdnumber_projectedOG->setQuantity(quant);
   return;
}

void BrewNoteWidget::setBrewNote(BrewNote* bNote) {
   qDebug() << Q_FUNC_INFO << "BrewNote:" << bNote;

   if (this->bNoteObs) {
      disconnect(this->bNoteObs, nullptr, this, nullptr);
   }

   if (bNote) {
      this->bNoteObs = bNote;
      connect(this->bNoteObs, &NamedEntity::changed, this, &BrewNoteWidget::changed);

      // Set the highs and the lows for the lcds
      this->lcdnumber_effBK->setLowLim (bNoteObs->projEff_pct() * lowLimitPct);
      this->lcdnumber_effBK->setHighLim(bNoteObs->projEff_pct() * highLimitPct);

      this->lcdnumber_projectedOG->setLowLim (bNoteObs->projOg() * lowLimitPct);
      this->lcdnumber_projectedOG->setHighLim(bNoteObs->projOg() * highLimitPct);

      this->lcdnumber_brewhouseEff->setLowLim (bNoteObs->projEff_pct() * lowLimitPct);
      this->lcdnumber_brewhouseEff->setHighLim(bNoteObs->projEff_pct() * highLimitPct);

      this->lcdnumber_projABV->setLowLim (bNoteObs->projABV_pct() * lowLimitPct);
      this->lcdnumber_projABV->setHighLim(bNoteObs->projABV_pct() * highLimitPct);

      this->lcdnumber_abv->setLowLim (bNoteObs->projABV_pct() * lowLimitPct);
      this->lcdnumber_abv->setHighLim(bNoteObs->projABV_pct() * highLimitPct);

      this->lcdnumber_atten->setLowLim (bNoteObs->projAtten() * lowLimitPct);
      this->lcdnumber_atten->setHighLim(bNoteObs->projAtten() * highLimitPct);

      this->lcdnumber_projAtten->setLowLim (bNoteObs->projAtten() * lowLimitPct);
      this->lcdnumber_projAtten->setHighLim(bNoteObs->projAtten() * highLimitPct);

      this->showChanges();
   }
   return;
}

bool BrewNoteWidget::isBrewNote(BrewNote* note) {
   return this->bNoteObs == note;
}

void BrewNoteWidget::updateSG()                                { if (this->bNoteObs) { this->bNoteObs->setSg              (this->lineEdit_Sg         ->getNonOptCanonicalQty());                      } return; }
void BrewNoteWidget::updateVolumeIntoBK_l()                    { if (this->bNoteObs) { this->bNoteObs->setVolumeIntoBK_l  (this->lineEdit_volIntoBk  ->getNonOptCanonicalQty());                      } return; }
void BrewNoteWidget::updateStrikeTemp_c()                      { if (this->bNoteObs) { this->bNoteObs->setStrikeTemp_c    (this->lineEdit_strikeTemp ->getNonOptCanonicalQty());                      } return; }
void BrewNoteWidget::updateMashFinTemp_c()                     { if (this->bNoteObs) { this->bNoteObs->setMashFinTemp_c   (this->lineEdit_mashFinTemp->getNonOptCanonicalQty());                      } return; }
void BrewNoteWidget::updateOG()                                { if (this->bNoteObs) { this->bNoteObs->setOg              (this->lineEdit_Og         ->getNonOptCanonicalQty());                      } return; }
void BrewNoteWidget::updatePostBoilVolume_l()                  { if (this->bNoteObs) { this->bNoteObs->setPostBoilVolume_l(this->lineEdit_postBoilVol->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewNoteWidget::updateVolumeIntoFerm_l()                  { if (this->bNoteObs) { this->bNoteObs->setVolumeIntoFerm_l(this->lineEdit_volIntoFerm->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewNoteWidget::updatePitchTemp_c()                       { if (this->bNoteObs) { this->bNoteObs->setPitchTemp_c     (this->lineEdit_pitchTemp  ->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewNoteWidget::updateFG()                                { if (this->bNoteObs) { this->bNoteObs->setFg              (this->lineEdit_Fg         ->getNonOptCanonicalQty()); this->showChanges(); } return; }
void BrewNoteWidget::updateFinalVolume_l()                     { if (this->bNoteObs) { this->bNoteObs->setFinalVolume_l   (this->lineEdit_finalVolume->getNonOptCanonicalQty());                      } return; }
void BrewNoteWidget::updateFermentDate(QDate const & datetime) { if (this->bNoteObs) { this->bNoteObs->setFermentDate     (datetime);                                                                } return; }
void BrewNoteWidget::updateNotes()                             { if (this->bNoteObs) { this->bNoteObs->setNotes           (this->btTextEdit_brewNotes->toPlainText() );                              } return; }

void BrewNoteWidget::changed([[maybe_unused]] QMetaProperty prop,
                             [[maybe_unused]] QVariant val) {
   if (this->sender() != this->bNoteObs) {
      return;
   }

   this->showChanges();
   return;
}

void BrewNoteWidget::showChanges([[maybe_unused]] QString field) {
   if (!this->bNoteObs) {
      return;
   }

   this->lineEdit_Sg         ->setQuantity   (bNoteObs->sg              ());
   this->lineEdit_volIntoBk  ->setQuantity   (bNoteObs->volumeIntoBK_l  ());
   this->lineEdit_strikeTemp ->setQuantity   (bNoteObs->strikeTemp_c    ());
   this->lineEdit_mashFinTemp->setQuantity   (bNoteObs->mashFinTemp_c   ());
   this->lineEdit_Og         ->setQuantity   (bNoteObs->og              ());
   this->lineEdit_postBoilVol->setQuantity   (bNoteObs->postBoilVolume_l());
   this->lineEdit_volIntoFerm->setQuantity   (bNoteObs->volumeIntoFerm_l());
   this->lineEdit_pitchTemp  ->setQuantity   (bNoteObs->pitchTemp_c     ());
   this->lineEdit_Fg         ->setQuantity   (bNoteObs->fg              ());
   this->lineEdit_finalVolume->setQuantity   (bNoteObs->finalVolume_l   ());
   this->lineEdit_fermentDate->setDate     (bNoteObs->fermentDate     ());
   this->btTextEdit_brewNotes->setPlainText(bNoteObs->notes           ());

   // Now with the calculated stuff
   this->lcdnumber_effBK->setQuantity(bNoteObs->effIntoBK_pct());

   // Need to think about these? Maybe use the bubbles?
   this->updateProjOg(); // this requires more work, but updateProj does it

   this->lcdnumber_brewhouseEff->setQuantity(bNoteObs->brewhouseEff_pct());
   this->lcdnumber_projABV     ->setQuantity(bNoteObs->projABV_pct     ());
   this->lcdnumber_abv         ->setQuantity(bNoteObs->abv             ());
   this->lcdnumber_atten       ->setQuantity(bNoteObs->attenuation     ());
   this->lcdnumber_projAtten   ->setQuantity(bNoteObs->projAtten       ());
   return;
}

void BrewNoteWidget::focusOutEvent([[maybe_unused]] QFocusEvent * e) {
   return;
}
