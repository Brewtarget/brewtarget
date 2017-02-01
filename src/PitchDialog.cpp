/*
 * PitchDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - A.J. Drobnich <aj.drobnich@gmail.com>
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

#include "PitchDialog.h"
#include <QChar>
#include "brewtarget.h"
#include "Algorithms.h"
#include "unit.h"
#include <math.h>

PitchDialog::PitchDialog(QWidget* parent) : QDialog(parent)
{
   setupUi(this);

   // Set default dates
   dateEdit_ProductionDate->setMaximumDate(QDate::currentDate());
   dateEdit_ProductionDate->setDate(QDate::currentDate());
   updateViabilityFromDate(QDate::currentDate());

   connect( lineEdit_vol, &BtLineEdit::textModified, this, &PitchDialog::calculate);
   connect( lineEdit_OG, &BtLineEdit::textModified, this, &PitchDialog::calculate);
   connect( slider_pitchRate, &QAbstractSlider::valueChanged, this, &PitchDialog::calculate );
   connect( slider_pitchRate, &QAbstractSlider::valueChanged, this, &PitchDialog::updateShownPitchRate );
   connect( spinBox_Viability, SIGNAL(valueChanged(int)), this, SLOT(calculate()));
   connect( spinBox_VialsPitched, SIGNAL(valueChanged(int)), this, SLOT(calculate()));
   connect( comboBox_AerationMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(calculate()));
   connect( dateEdit_ProductionDate, &QDateTimeEdit::dateChanged, this, &PitchDialog::updateViabilityFromDate);
   connect( checkBox_CalculateViability, &QCheckBox::stateChanged, this, &PitchDialog::toggleViabilityFromDate);

   // Dates are a little more cranky
   connect(label_productionDate,&BtLabel::labelChanged,this,&PitchDialog::updateProductionDate);
   updateProductionDate(Unit::noUnit,Unit::noScale);
   updateShownPitchRate(0);
}

PitchDialog::~PitchDialog()
{
}

void PitchDialog::updateProductionDate(Unit::unitDisplay dsp, Unit::unitScale scl)
{
   QString format;
   // I need the new unit, not the old
   Unit::unitDisplay unitDsp = (Unit::unitDisplay)Brewtarget::option("productionDate", Brewtarget::getDateFormat(), "pitchRateCalc", Brewtarget::UNIT).toInt();

   switch(unitDsp)
   {
      case Unit::displayUS:
         format = "MM-dd-yyyy";
         break;
      case Unit::displayImp:
         format = "dd-MM-yyyy";
         break;
      case Unit::displaySI:
      default:
         format = "yyyy-MM-dd";
   }
   dateEdit_ProductionDate->setDisplayFormat(format);
}

void PitchDialog::setWortVolume_l(double volume)
{
   lineEdit_vol->setText(volume);
}

void PitchDialog::setWortDensity(double sg)
{
   lineEdit_OG->setText(sg);
}

void PitchDialog::calculate()
{

   // Allow selection of 0.75 to 2 million cells per mL per degree P.
   double rate_MpermLP = (2-0.75) * ((double)slider_pitchRate->value()) / 100.0 + 0.75;

   // This isn't right.
   double og = lineEdit_OG->toSI();
   double vol_l = lineEdit_vol->toSI();

   // I somewhat aribtrarily defined "SI" for density to be specific gravity.
   // Since these calcs need plato, convert
   double plato = Algorithms::SG_20C20C_toPlato(og);

   double cells = (rate_MpermLP * 1e6) * (vol_l * 1e3) * plato;
   double vials = cells / (spinBox_Viability->value() * 1e9); // ~100 billion cells per vial/pack, taking viability into account.
   double dry_g = cells / 20e9; // 20 billion cells per dry gram.

   // Set the maximum number of vials pitched based on # of vials needed without a starter.
   spinBox_VialsPitched->setMaximum(vials < 1 ? 1 : floor(vials));

   // Set the aeration factor for the starter size
   double aerationFactor;
   switch (comboBox_AerationMethod->currentIndex())
   {
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

   lineEdit_cells->setText(cells/1e9, 1);
   lineEdit_starterVol->setText(starterVol_l);
   lineEdit_yeast->setText(dry_g/1000); //Needs to be converted into default unit (kg)
   lineEdit_vials->setText(vials,0);
}

void PitchDialog::updateShownPitchRate(int percent)
{
   // Allow selection of 0.75 to 2 million cells per mL per degree P.
   double rate_MpermLP = (2-0.75) * ((double)percent) / 100.0 + 0.75;

   // NOTE: We are changing the LABEL here, not the LineEdit. Leave it be
   label_pitchRate->setText( QString("%L1").arg(rate_MpermLP, 1, 'f', 2, QChar('0')) );
}

/*
 * Toggles whether or not the viability box and date edit
 * is enabled or disabled.
 */
void PitchDialog::toggleViabilityFromDate(int state)
{
   if (state == Qt::Unchecked)
   {
      // If the box is not checked, disable the date and allow
      // the user to manually set the viability.
      spinBox_Viability->setEnabled(true);
      dateEdit_ProductionDate->setEnabled(false);
   }
   else if (state == Qt::Checked)
   {
      // If the box is checked, prevent the user from manually setting
      // the viability.  Use the date editor instead.
      spinBox_Viability->setEnabled(false);
      dateEdit_ProductionDate->setEnabled(true);
      updateViabilityFromDate(dateEdit_ProductionDate->date());
   }
}

/*
 * Updates the current viability based on the date.
 */
void PitchDialog::updateViabilityFromDate(QDate date)
{
   // Set the viability based on the number of days since the yeast
   // production date.
   int daysDifference = date.daysTo(QDate::currentDate());
   spinBox_Viability->setValue(97 - 0.7 * daysDifference);
}
