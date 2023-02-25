/*
 * BrewNoteWidget.cpp is part of Brewtarget, and is Copyright the following
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

   connect(lineEdit_SG, &BtLineEdit::textModified, this, &BrewNoteWidget::updateSG);
   connect(lineEdit_volIntoBK, &BtLineEdit::textModified, this, &BrewNoteWidget::updateVolumeIntoBK_l);
   connect(lineEdit_strikeTemp, &BtLineEdit::textModified, this, &BrewNoteWidget::updateStrikeTemp_c);
   connect(lineEdit_mashFinTemp, &BtLineEdit::textModified, this, &BrewNoteWidget::updateMashFinTemp_c);

   connect(lineEdit_OG, &BtLineEdit::textModified, this, &BrewNoteWidget::updateOG);
   connect(lineEdit_postBoilVol, &BtLineEdit::textModified, this, &BrewNoteWidget::updatePostBoilVolume_l);
   connect(lineEdit_volIntoFerm, &BtLineEdit::textModified, this, &BrewNoteWidget::updateVolumeIntoFerm_l);
   connect(lineEdit_pitchTemp, &BtLineEdit::textModified, this, &BrewNoteWidget::updatePitchTemp_c);

   connect(lineEdit_FG, &BtLineEdit::textModified, this, &BrewNoteWidget::updateFG);
   connect(lineEdit_finalVol, &BtLineEdit::textModified, this, &BrewNoteWidget::updateFinalVolume_l);
   connect(lineEdit_fermentDate, &QDateTimeEdit::dateChanged, this, &BrewNoteWidget::updateFermentDate);

   connect(btTextEdit_brewNotes, &BtTextEdit::textModified, this, &BrewNoteWidget::updateNotes);

   // A few labels on this page need special handling, so I connect them here
   // instead of how we would normally do this.
   connect(btLabel_projectedOg, &BtLabel::changedSystemOfMeasurementOrScale, this, &BrewNoteWidget::updateProjOg);
///   connect(btLabel_fermentDate, &BtLabel::changedSystemOfMeasurementOrScale, this, &BrewNoteWidget::updateDateFormat);

   // I think this might work
   updateDateFormat();
}

BrewNoteWidget::~BrewNoteWidget() = default;

//.:TBD:. See comment in PitchDialog::updateProductionDate() for how we might re-implement per-field date format
// selection
// I should really do this better, but I really cannot bring myself to do
// another UnitSystem for one input field.
void BrewNoteWidget::updateDateFormat() {
//   auto dateFormat = Localization::getDateFormatForField(PersistentSettings::BrewNote::fermentDate,
//                                                         PersistentSettings::Sections::page_postferment);
   auto dateFormat = Localization::getDateFormat();
   QString format = Localization::numericToStringDateFormat(dateFormat);
   this->lineEdit_fermentDate->setDisplayFormat(format);
   return;
}


void BrewNoteWidget::updateProjOg() {
   // Density UnitSystems only have one scale, so we don't bother looking up UnitSystem::RelativeScale
   auto forcedSystemOfMeasurement =
      Measurement::getForcedSystemOfMeasurementForField(*PropertyNames::BrewNote::projOg,
                                                        *PersistentSettings::Sections::page_preboil);
   double quant = Measurement::amountDisplay(Measurement::Amount{this->bNoteObs->projOg(), Measurement::Units::sp_grav},
                                             forcedSystemOfMeasurement);
   this->lcdnumber_projectedOG->setLowLim( lowLimitPct  * quant);
   this->lcdnumber_projectedOG->setHighLim(highLimitPct * quant);

   Measurement::UnitSystem const & displayUnitSystem =
      Measurement::getUnitSystemForField(*PropertyNames::BrewNote::projOg,
                                         *PersistentSettings::Sections::page_preboil,
                                         Measurement::PhysicalQuantity::Density);
   int precision = (displayUnitSystem == Measurement::UnitSystems::density_Plato) ? 0 : 3;

   this->lcdnumber_projectedOG->display(quant, precision);
   return;
}

void BrewNoteWidget::setBrewNote(BrewNote* bNote) {

   if (this->bNoteObs) {
      disconnect(this->bNoteObs, nullptr, this, nullptr);
   }

   if (bNote) {
      this->bNoteObs = bNote;
      connect(this->bNoteObs, &NamedEntity::changed, this, &BrewNoteWidget::changed);

      // Set the highs and the lows for the lcds
      lcdnumber_effBK->setLowLim(bNoteObs->projEff_pct() * lowLimitPct);
      lcdnumber_effBK->setHighLim(bNoteObs->projEff_pct() * highLimitPct);

      lcdnumber_projectedOG->setLowLim( bNoteObs->projOg() * lowLimitPct);
      lcdnumber_projectedOG->setHighLim( bNoteObs->projOg() * highLimitPct);

      lcdnumber_brewhouseEff->setLowLim(bNoteObs->projEff_pct() * lowLimitPct);
      lcdnumber_brewhouseEff->setHighLim(bNoteObs->projEff_pct() * highLimitPct);

      lcdnumber_projABV->setLowLim( bNoteObs->projABV_pct() * lowLimitPct);
      lcdnumber_projABV->setHighLim( bNoteObs->projABV_pct() * highLimitPct);

      lcdnumber_abv->setLowLim( bNoteObs->projABV_pct() * lowLimitPct);
      lcdnumber_abv->setHighLim( bNoteObs->projABV_pct() * highLimitPct);

      lcdnumber_atten->setLowLim( bNoteObs->projAtten() * lowLimitPct );
      lcdnumber_atten->setHighLim( bNoteObs->projAtten() * highLimitPct );

      lcdnumber_projAtten->setLowLim( bNoteObs->projAtten() * lowLimitPct );
      lcdnumber_projAtten->setHighLim( bNoteObs->projAtten() * highLimitPct );

      showChanges();
   }
   return;
}

bool BrewNoteWidget::isBrewNote(BrewNote* note) {
   return this->bNoteObs == note;
}

void BrewNoteWidget::updateSG() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setSg(lineEdit_SG->toCanonical().quantity());
   return;
}

void BrewNoteWidget::updateVolumeIntoBK_l() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setVolumeIntoBK_l(lineEdit_volIntoBK->toCanonical().quantity());
   return;
}

void BrewNoteWidget::updateStrikeTemp_c() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setStrikeTemp_c(lineEdit_strikeTemp->toCanonical().quantity());
   return;
}

void BrewNoteWidget::updateMashFinTemp_c() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setMashFinTemp_c(lineEdit_mashFinTemp->toCanonical().quantity());
   return;
}

void BrewNoteWidget::updateOG() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setOg(lineEdit_OG->toCanonical().quantity());
   return;
}

void BrewNoteWidget::updatePostBoilVolume_l() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setPostBoilVolume_l(lineEdit_postBoilVol->toCanonical().quantity());
   this->showChanges();
   return;
}

void BrewNoteWidget::updateVolumeIntoFerm_l() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setVolumeIntoFerm_l(lineEdit_volIntoFerm->toCanonical().quantity());
   this->showChanges();
   return;
}

void BrewNoteWidget::updatePitchTemp_c() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setPitchTemp_c(lineEdit_pitchTemp->toCanonical().quantity());
   this->showChanges();
   return;
}

void BrewNoteWidget::updateFG() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setFg(lineEdit_FG->toCanonical().quantity());
   this->showChanges();
   return;
}

void BrewNoteWidget::updateFinalVolume_l() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setFinalVolume_l(lineEdit_finalVol->toCanonical().quantity());
//   this->showChanges();
   return;
}

void BrewNoteWidget::updateFermentDate(QDate const & datetime) {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setFermentDate(datetime);
   return;
}

void BrewNoteWidget::updateNotes() {
   if (this->bNoteObs == nullptr) {
      return;
   }

   this->bNoteObs->setNotes(btTextEdit_brewNotes->toPlainText() );
   return;
}

void BrewNoteWidget::changed([[maybe_unused]] QMetaProperty prop,
                             [[maybe_unused]] QVariant val) {
   if (this->sender() != this->bNoteObs) {
      return;
   }

   this->showChanges();
   return;
}

void BrewNoteWidget::showChanges([[maybe_unused]] QString field) {
   if (this->bNoteObs == nullptr) {
      return;
   }

   lineEdit_SG->setText(bNoteObs);
   lineEdit_volIntoBK->setText(bNoteObs);
   lineEdit_strikeTemp->setText(bNoteObs);
   lineEdit_mashFinTemp->setText(bNoteObs);
   lineEdit_OG->setText(bNoteObs);
   lineEdit_postBoilVol->setText(bNoteObs);
   lineEdit_volIntoFerm->setText(bNoteObs);
   lineEdit_pitchTemp->setText(bNoteObs);
   lineEdit_FG->setText(bNoteObs);
   lineEdit_finalVol->setText(bNoteObs);

   lineEdit_fermentDate->setDate(bNoteObs->fermentDate());
   btTextEdit_brewNotes->setPlainText(bNoteObs->notes());

   // Now with the calculated stuff
   lcdnumber_effBK->display(bNoteObs->effIntoBK_pct(),2);

   // Need to think about these? Maybe use the bubbles?
   this->updateProjOg(); // this requires more work, but updateProj does it

   lcdnumber_brewhouseEff->display(bNoteObs->brewhouseEff_pct(),2);
   lcdnumber_projABV->display(bNoteObs->projABV_pct(),2);
   lcdnumber_abv->display(bNoteObs->abv(),2);
   lcdnumber_atten->display(bNoteObs->attenuation(),2);
   lcdnumber_projAtten->display(bNoteObs->projAtten(),2);
   return;
}

void BrewNoteWidget::focusOutEvent([[maybe_unused]] QFocusEvent * e) {
   return;
}
