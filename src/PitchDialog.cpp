/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * PitchDialog.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "PitchDialog.h"

#include <math.h>

#include <QChar>

#include "Algorithms.h"
#include "Localization.h"
#include "measurement/Unit.h"
#include "PersistentSettings.h"

PitchDialog::PitchDialog(QWidget* parent) : QDialog(parent) {
   setupUi(this);

   // Set default dates
   this->dateEdit_ProductionDate->setMaximumDate(QDate::currentDate());
   this->dateEdit_ProductionDate->setDate(QDate::currentDate());
   this->updateViabilityFromDate(QDate::currentDate());

   SMART_FIELD_INIT_FS(PitchDialog, label_vol       , lineEdit_vol       , double, Measurement::PhysicalQuantity::Volume          ); // Input: Wort Volume
   SMART_FIELD_INIT_FS(PitchDialog, label_og        , lineEdit_OG        , double, Measurement::PhysicalQuantity::Density         ); // Input: OG
   SMART_FIELD_INIT_FS(PitchDialog, label_cells     , lineEdit_cells     , double,           NonPhysicalQuantity::Dimensionless, 1); // Output: Billions of Yeast Cells Required
   SMART_FIELD_INIT_FS(PitchDialog, label_vials     , lineEdit_vials     , double,           NonPhysicalQuantity::Dimensionless, 0); // Output: # Vials/Smack Packs w/o Starter
   SMART_FIELD_INIT_FS(PitchDialog, label_yeast     , lineEdit_yeast     , double, Measurement::PhysicalQuantity::Mass            ); // Output: Dry Yeast
   SMART_FIELD_INIT_FS(PitchDialog, label_starterVol, lineEdit_starterVol, double, Measurement::PhysicalQuantity::Volume          ); // Output: Starter Volume

   connect(this->lineEdit_vol,                &SmartLineEdit::textModified,                        this, &PitchDialog::calculate              );
   connect(this->lineEdit_OG,                 &SmartLineEdit::textModified,                        this, &PitchDialog::calculate              );
   connect(this->slider_pitchRate,            &QAbstractSlider::valueChanged,                      this, &PitchDialog::calculate              );
   connect(this->slider_pitchRate,            &QAbstractSlider::valueChanged,                      this, &PitchDialog::updateShownPitchRate   );
   connect(this->spinBox_Viability,           QOverload<int>::of(&QSpinBox::valueChanged),         this, &PitchDialog::calculate              );
   connect(this->spinBox_VialsPitched,        QOverload<int>::of(&QSpinBox::valueChanged),         this, &PitchDialog::calculate              );
   connect(this->comboBox_AerationMethod,     QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PitchDialog::calculate              );
   connect(this->dateEdit_ProductionDate,     &QDateTimeEdit::dateChanged,                         this, &PitchDialog::updateViabilityFromDate);
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
   connect(this->checkBox_CalculateViability, &QCheckBox::checkStateChanged,                       this, &PitchDialog::toggleViabilityFromDate);
#else
   connect(this->checkBox_CalculateViability, &QCheckBox::stateChanged     ,                       this, &PitchDialog::toggleViabilityFromDate);
#endif

   // Dates are a little more cranky
   this->updateProductionDate();
   this->updateShownPitchRate(0);
   return;
}

PitchDialog::~PitchDialog() = default;

void PitchDialog::updateProductionDate() {
   // I need the new unit, not the old
   // .:TBD:. For the moment, we stick with whatever date format the user has set for the whole program.  Would need to
   // uncomment the following line and implement the corresponding function in Localization, plus do an appropriate
   // pop-up menu etc if we want to select date format per-field.
//   auto dateFormat = Localization::getDateFormatForField(PersistentSettings::Names::productionDate,
//                                                         PersistentSettings::Sections::pitchRateCalc);
   auto dateFormat = Localization::getDateFormat();
   QString format = Localization::numericToStringDateFormat(dateFormat);
   this->dateEdit_ProductionDate->setDisplayFormat(format);
}

void PitchDialog::setWortVolume_l(double volume) {
   this->lineEdit_vol->setQuantity(volume);
}

void PitchDialog::setWortDensity(double sg) {
   this->lineEdit_OG->setQuantity(sg);
}

void PitchDialog::calculate() {

   // Allow selection of 0.75 to 2 million cells per mL per degree P.
   double rate_MpermLP = (2-0.75) * ((double)slider_pitchRate->value()) / 100.0 + 0.75;

   // This isn't right.
   double og = lineEdit_OG->getNonOptCanonicalQty();
   double vol_l = lineEdit_vol->getNonOptCanonicalQty();

   // I somewhat aribtrarily defined "SI" for density to be specific gravity.
   // Since these calcs need plato, convert
   double plato = Algorithms::SG_20C20C_toPlato(og);

   double cells = (rate_MpermLP * 1e6) * (vol_l * 1e3) * plato;
   double vials = cells / (spinBox_Viability->value() * 1e9); // ~100 billion cells per vial/pack, taking viability into account.
   double dry_g = cells / 20e9; // 20 billion cells per dry gram.

   // Set the maximum number of vials pitched based on # of vials needed without a starter.
   this->spinBox_VialsPitched->setMaximum(vials < 1 ? 1 : floor(vials));

   // Set the aeration factor for the starter size
   double aerationFactor;
   switch (comboBox_AerationMethod->currentIndex()) {
      case 1:   // O2 at the start
         aerationFactor = 1.33;
         break;
      case 2:   // Stir plate.
         aerationFactor = 2.66;
         break;
      default:
         aerationFactor = 1;
   }

   // Get the total # of cells pitched based on viability.
   double totalCellsPitched = spinBox_VialsPitched->value() * spinBox_Viability->value();

   // Starter in liters = Growth Rate / Inoculation Rate
   double growthRate = (cells / 1e9) / totalCellsPitched;
   double inoculationRate = pow((12.522 / growthRate), 2.18);
   double starterVol_l = totalCellsPitched / (inoculationRate * aerationFactor);

   this->lineEdit_cells     ->setQuantity(cells/1e9);
   this->lineEdit_starterVol->setQuantity(starterVol_l);
   this->lineEdit_yeast     ->setQuantity(dry_g/1000); //Needs to be converted into default unit (kg)
   this->lineEdit_vials     ->setQuantity(vials);
   return;
}

void PitchDialog::updateShownPitchRate(int percent) {
   // Allow selection of 0.75 to 2 million cells per mL per degree P.
   double rate_MpermLP = (2-0.75) * ((double)percent) / 100.0 + 0.75;

   // NOTE: We are changing the LABEL here, not the LineEdit. Leave it be
   this->label_pitchRate->setText( QString("%L1").arg(rate_MpermLP, 1, 'f', 2, QChar('0')) );
   return;
}

/*
 * Toggles whether or not the viability box and date edit
 * is enabled or disabled.
 */
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void PitchDialog::toggleViabilityFromDate(Qt::CheckState state) {
#else
void PitchDialog::toggleViabilityFromDate(int state) {
#endif
   if (state == Qt::Unchecked) {
      // If the box is not checked, disable the date and allow
      // the user to manually set the viability.
      this->spinBox_Viability->setEnabled(true);
      this->dateEdit_ProductionDate->setEnabled(false);
   } else if (state == Qt::Checked) {
      // If the box is checked, prevent the user from manually setting
      // the viability.  Use the date editor instead.
      this->spinBox_Viability->setEnabled(false);
      this->dateEdit_ProductionDate->setEnabled(true);
      this->updateViabilityFromDate(dateEdit_ProductionDate->date());
   }
   return;
}

/*
 * Updates the current viability based on the date.
 */
void PitchDialog::updateViabilityFromDate(QDate date) {
   // Set the viability based on the number of days since the yeast
   // production date.
   int daysDifference = date.daysTo(QDate::currentDate());
   this->spinBox_Viability->setValue(97 - 0.7 * daysDifference);
   return;
}
