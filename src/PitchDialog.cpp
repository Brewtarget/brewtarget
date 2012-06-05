/*
 * PitchDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2010-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

   connect( lineEdit_vol, SIGNAL(editingFinished()), this, SLOT(calculate()));
   connect( lineEdit_OG, SIGNAL(editingFinished()), this, SLOT(calculate()));
   connect( lineEdit_starterOG, SIGNAL(editingFinished()), this, SLOT(calculate()));
   connect( slider_pitchRate, SIGNAL(sliderMoved(int)), this, SLOT(calculate()) );
   connect( slider_pitchRate, SIGNAL(sliderMoved(int)), this, SLOT(updateShownPitchRate(int)) );

   updateShownPitchRate(0);
}

PitchDialog::~PitchDialog()
{
}

void PitchDialog::calculate()
{
   bool ok;

   // Allow selection of 0.75 to 2 million cells per mL per degree P.
   double rate_MpermLP = (2-0.75) * ((double)slider_pitchRate->value()) / 100.0 + 0.75;
   double og = lineEdit_OG->text().toDouble(&ok);
   double vol_l = Brewtarget::volQStringToSI(lineEdit_vol->text());
   //ok &= tmp;
   double plato = Algorithms::Instance().SG_20C20C_toPlato(og);

   if( !ok )
      return;

   double cells = (rate_MpermLP * 1e6) * (vol_l * 1e3) * plato;
   double vials = cells/100e9; // 100 billion cells per vial/pack.
   double dry_g = cells / 20e9; // 20 billion cells per dry gram.
   double inoculationRate = pow(1251.94 / (cells / 1e9), 2.1793); // cell count = 1251.94 * inoculation^-.45887
   double starterVol_l = 100.0 / inoculationRate; // Starter Volume = 100 / inoculation rate

   lineEdit_cells->setText(QString("%1").arg(cells/1e9, 1, 'f', 0, QChar('0')));
   lineEdit_starterVol->setText(Brewtarget::displayAmount(starterVol_l, Units::liters));
   lineEdit_yeast->setText(Brewtarget::displayAmount(dry_g, Units::grams));
   lineEdit_vials->setText(QString("%1").arg(vials, 1, 'f', 1, QChar('0')));
}

void PitchDialog::updateShownPitchRate(int percent)
{
   // Allow selection of 0.75 to 2 million cells per mL per degree P.
   double rate_MpermLP = (2-0.75) * ((double)percent) / 100.0 + 0.75;

   label_pitchRate->setText( QString("%1").arg(rate_MpermLP, 1, 'f', 2, QChar('0')) );
}
