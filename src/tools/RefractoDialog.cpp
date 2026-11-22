/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tools/RefractoDialog.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Eric Tamme <etamme@gmail.com>
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
 =====================================================================================================================*/
#include "tools/RefractoDialog.h"

#include <cmath>

#include <QEvent>
#include <QDebug>
#include <QString>

#include "calculations/Algorithms.h"
#include "measurement/Measurement.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RefractoDialog.cpp"
#endif

RefractoDialog::RefractoDialog(QWidget* parent) : QDialog(parent) {
   setupUi(this);

   // Note that the labels here are QLabel, not SmartLabel, because we want the units fixed, not user-selectable
   SMART_FIELD_INIT_FIXED(RefractoDialog, label_op     , lineEdit_op     , double, Measurement::Units::plato          , 1); // Original Plato
   SMART_FIELD_INIT_FIXED(RefractoDialog, label_inputOG, lineEdit_inputOG, double, Measurement::Units::specificGravity, 3); // Original gravity in
   SMART_FIELD_INIT_FIXED(RefractoDialog, label_cp     , lineEdit_cp     , double, Measurement::Units::plato          , 1); // Current Plato
   SMART_FIELD_INIT_FIXED(RefractoDialog, label_og     , lineEdit_og     , double, Measurement::Units::specificGravity, 3); // Original gravity out
   SMART_FIELD_INIT_FIXED(RefractoDialog, label_sg     , lineEdit_sg     , double, Measurement::Units::specificGravity, 3); // Specific gravity out
   SMART_FIELD_INIT_FIXED(RefractoDialog, label_re     , lineEdit_re     , double, Measurement::Units::plato          , 1); // Real extract Plato
   SMART_FIELD_INIT_FS   (RefractoDialog, label_ri     , lineEdit_ri     , double, NonPhysicalQuantity::Dimensionless    ); // Refractive index
   SMART_FIELD_INIT_FS   (RefractoDialog, label_abv    , lineEdit_abv    , double, NonPhysicalQuantity::Percentage       ); // Alcohol by volume
   SMART_FIELD_INIT_FS   (RefractoDialog, label_abw    , lineEdit_abw    , double, NonPhysicalQuantity::Percentage       ); // Alcohol by weight

   // We recalculate whenever an input changes, so there is no need for a "Calculate" button
   connect(this->lineEdit_op     , &SmartLineEdit::textModified, this, &RefractoDialog::calculate);
   connect(this->lineEdit_inputOG, &SmartLineEdit::textModified, this, &RefractoDialog::calculate);
   connect(this->lineEdit_cp     , &SmartLineEdit::textModified, this, &RefractoDialog::calculate);

   this->label_warning->setStyleSheet("color : #C62828;");
   this->label_warning->setVisible(false);
   return;
}

RefractoDialog::~RefractoDialog() = default;

void RefractoDialog::calculate() {
   bool haveCP = true;
   bool haveOP = true;
   bool haveOG = true;

   // User can enter in specific gravity or Plato, but the lineEdit is going to convert it to Plato, so we can just
   // grab the number
   double originalPlato = Measurement::extractRawFromString<double>(lineEdit_op     ->text(), &haveOP);
   double inputOG       = Measurement::extractRawFromString<double>(lineEdit_inputOG->text(), &haveOG);
   double currentPlato  = Measurement::extractRawFromString<double>(lineEdit_cp     ->text(), &haveCP);

   this->clearOutputFields();

   // Can't do anything without the current Plato.  (Since we recalculate whenever an input changes, the user may just
   // not have entered it yet.)
   if (!haveCP) {
      return;
   }

   double const ri = Algorithms::refractiveIndex(currentPlato);
   this->lineEdit_ri->setText(Measurement::displayQuantity(ri, 3));

   //
   // The user can give us the original gravity either in Plato or as SG.  We used to copy whichever one we calculated
   // into the other input field, but, now that we recalculate as soon as an input changes, that would get in the way
   // of the user changing their inputs.
   //
   if (!haveOP && !haveOG) {
      qDebug() << Q_FUNC_INFO << "no plato or og";
      return; // Can't do much if we don't have OG or OP.
   }
   if (!haveOP) {
      originalPlato = Algorithms::SG_20C20C_toPlato(inputOG);
   }

   double const og = Algorithms::PlatoToSG_20C20C( originalPlato );
   double sg = 0;
   if (originalPlato != currentPlato) {
     sg = Algorithms::sgByStartingPlato( originalPlato, currentPlato );
   } else {
     sg = og;
   }

   double const re  = Algorithms::realExtract    (sg, currentPlato);
   double const abv = Algorithms::getABVBySGPlato(sg, currentPlato);
   double const abw = Algorithms::getABWBySGPlato(sg, currentPlato);

   // Warn the user if they gave us both original Plato and original gravity and the two don't match.
   if (haveOP && haveOG && qAbs(og - inputOG) > 0.002) {
      this->label_warning->setText(
         tr("Based on the given original plato, the OG should be %1, but you have entered %2. "
            "Continuing with the calculated OG.").arg(og,0,'f',3).arg(inputOG,0,'f',3)
      );
      this->label_warning->setVisible(true);
   }

   this->lineEdit_og->setQuantity(og);
   this->lineEdit_sg->setQuantity(sg);
   // Even if the real extract if display in Plato, it must be given in system unit.
   // Conversion is made by SmartLineEdit
   this->lineEdit_re ->setQuantity(Algorithms::PlatoToSG_20C20C(re));
   this->lineEdit_abv->setQuantity(abv);
   this->lineEdit_abw->setQuantity(abw);
   return;
}

void RefractoDialog::clearOutputFields() {
   this->label_warning->clear();
   this->label_warning->setVisible(false);
   this->lineEdit_ri ->clear();
   this->lineEdit_og ->clear();
   this->lineEdit_sg ->clear();
   this->lineEdit_re ->clear();
   this->lineEdit_abv->clear();
   this->lineEdit_abw->clear();
   return;
}

void RefractoDialog::changeEvent(QEvent * event) {
   if (event->type() == QEvent::LanguageChange) {
      this->retranslateUi(this);
   }
   this->QDialog::changeEvent(event);
   return;
}
