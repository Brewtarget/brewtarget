/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * PrimingDialog.cpp is part of Brewtarget, and is copyright the following authors 2009-2023:
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
#include "PrimingDialog.h"

#include <cmath>

#include <QButtonGroup>
#include <QDebug>
#include <QWidget>

#include "measurement/Unit.h"

PrimingDialog::PrimingDialog(QWidget* parent) : QDialog(parent) {
   this->setupUi(this);

   this->sugarGroup = new QButtonGroup(this);
   this->sugarGroup->setExclusive(true); // Can select only one.

   this->sugarGroup->addButton(radioButton_glucMono);
   this->sugarGroup->addButton(radioButton_gluc);
   this->sugarGroup->addButton(radioButton_sucrose);
   this->sugarGroup->addButton(radioButton_dme);

   SMART_FIELD_INIT_FS(PrimingDialog, label_beerVol, lineEdit_beerVol, double, Measurement::PhysicalQuantity::Volume        );
   SMART_FIELD_INIT_FS(PrimingDialog, label_temp   , lineEdit_temp   , double, Measurement::PhysicalQuantity::Temperature, 1);
   SMART_FIELD_INIT_FS(PrimingDialog, label_vols   , lineEdit_vols   , double, Measurement::PhysicalQuantity::Carbonation, 1);
   SMART_FIELD_INIT_FS(PrimingDialog, label_output , lineEdit_output , double, Measurement::PhysicalQuantity::Mass          );

   connect(pushButton_calculate, &QAbstractButton::clicked, this, &PrimingDialog::calculate);
   return;
}

PrimingDialog::~PrimingDialog() = default;

void PrimingDialog::calculate() {

   double const beer_l      = lineEdit_beerVol->getNonOptCanonicalQty();
   double const temp_c      = lineEdit_temp   ->getNonOptCanonicalQty();
   double const desiredVols = lineEdit_vols   ->getNonOptCanonicalQty();
   qDebug() <<
      Q_FUNC_INFO << "Beer volume (liters):" << beer_l << ", Temp (°C):" << temp_c << ", Desired Volumes:" <<
      desiredVols;

   double const residualVols = 1.57 * pow(0.97, temp_c); // Amount of CO2 still in suspension.
   double const addedVols = desiredVols - residualVols;
   double const co2_l = addedVols * beer_l; // Liters of CO2 we need to generate (at 273 K and 1 atm).
   double const co2_mol = co2_l / 22.4; // Mols of CO2 we need.

   double sugar_mol;
   double sugar_g;

   //
   // The calculation depends on which type of sugar is selected, via a set of radio buttons.
   //
   QAbstractButton* button = sugarGroup->checkedButton();
   if (button == radioButton_glucMono) {
      sugar_mol = co2_mol / 2;
      sugar_g = sugar_mol * 198; // Glucose monohydrate is 198 g/mol.
   } else if (button == radioButton_gluc) {
      sugar_mol = co2_mol / 2;
      sugar_g = sugar_mol * 180; // Glucose is 180g/mol.
   } else if (button == radioButton_sucrose) {
      sugar_mol = co2_mol / 4;
      sugar_g = sugar_mol * 342; // Sucrose is 342 g/mol.
   } else if (button == radioButton_dme) {
      sugar_mol = co2_mol / 2;
      sugar_g = sugar_mol * 180 / 0.60; // DME is equivalently about 60% glucose.
   } else {
      // If no radio button is set, then we can't do the calculation
      qDebug() << Q_FUNC_INFO << "No sugar type selected";
      return;
   }

   // The amount have to be set in default unit to SmartLineEdit.
   // We should find a better solution, but until it is not, we must do it this way.
   lineEdit_output->setQuantity(sugar_g/1000);

   return;
}
