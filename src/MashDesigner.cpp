/*
 * MashDesigner.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Jonathon Harding <github@jrhardin.net>
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
#include "MashDesigner.h"

#include <QInputDialog>
#include <QMessageBox>

#include "database/ObjectStoreWrapper.h"
#include "HeatCalculations.h"
#include "measurement/Measurement.h"
#include "model/Fermentable.h"
#include "PhysicalConstants.h"

MashDesigner::MashDesigner(QWidget * parent) : QDialog     {parent},
                                               recObs      {nullptr},
                                               mash        {nullptr},
                                               equip       {nullptr},
                                               mashStep    {nullptr},
                                               prevStep    {nullptr},
                                               addedWater_l{0} {
   this->setupUi(this);

   this->label_zeroVol->setText(Measurement::displayAmount(Measurement::Amount{0, Measurement::Units::liters}));
   this->label_zeroWort->setText(Measurement::displayAmount(Measurement::Amount{0, Measurement::Units::liters}));

   // Update temp slider when we move amount slider.
   connect(horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateTempSlider);
   // Update amount slider when we move temp slider.
   connect(horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateAmtSlider);
   // Update tun fullness bar when either slider moves.
   connect(horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateFullness);
   connect(horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateFullness);
   // Update amount/temp text when sliders move.
   connect(horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateAmt);
   connect(horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateTemp);
   connect(horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateAmt);
   connect(horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateTemp);
   // Update collected wort when sliders move.
   connect(horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateCollectedWort);
   connect(horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateCollectedWort);
   // Save the target temp whenever it's changed.
   connect(lineEdit_temp, &BtLineEdit::textModified, this, &MashDesigner::saveTargetTemp);
   // Move to next step.
   connect(pushButton_next, &QAbstractButton::clicked, this, &MashDesigner::proceed);
   // Do correct calcs when the mash step type is selected.
   connect(comboBox_type, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MashDesigner::typeChanged);

   // I still dislike this part. But I also need to "fix" the form
   // connect(checkBox_batchSparge, SIGNAL(clicked()), this, SLOT(updateMaxAmt()));
   connect(pushButton_finish, &QAbstractButton::clicked, this, &MashDesigner::saveAndClose);

   return;
}

void MashDesigner::proceed()
{
   nextStep(++curStep);
}

void MashDesigner::setRecipe(Recipe* rec) {
   this->recObs = rec;
   if (isVisible()) {
      setVisible(false);
   }
   return;
}

void MashDesigner::show() {
   // No point to run unless we have fermentables.
   if (this->recObs && this->recObs->fermentables().size() == 0) {
      QMessageBox::information(
         this,
         tr("No Fermentables"),
         tr("Your recipe must have fermentables to design a mash.")
     );
      return;
   }

   this->setVisible(this->nextStep(0));
   return;
}

void MashDesigner::saveAndClose() {
   this->saveStep();
   this->setVisible(false);
   return;
}

bool MashDesigner::nextStep(int step) {

   if (step == 0  && !this->initializeMash()) {
      return false;
   }

   if (step > 0) {
      this->saveStep();
   }

   this->prevStep = this->mashStep;
   if (this->mashStep) {
      // NOTE: This needs to be changed. Assumes 1L of water is 1 kg.
      this->MC += this->mashStep->infuseAmount_l() * HeatCalculations::Cw_calGC;
      this->addedWater_l += this->mashStep->infuseAmount_l();

      if (!this->prevStep) {
         // If the last step is null, we need to add the influence of the tun.
         this->MC += this->mash->tunSpecificHeat_calGC() * this->mash->tunWeight_kg();
      }
   }

   // If we have a step number, and the step is smaller than the current
   // number of mashsteps. How can this happen? When you get into
   // the mash designer, the first thing it does is clear all the steps.
   if (step >= 0 && step < this->mash->mashSteps().size()) {
      this->mashStep = this->mash->mashSteps()[step];
   } else {
      this->mashStep = std::make_shared<MashStep>("");
   }

   // Clear out some of the fields.
   this->lineEdit_name->clear();
   this->lineEdit_temp->clear();
   this->lineEdit_time->clear();

   this->horizontalSlider_amount->setValue(0); // Least amount of water.

   // Update max amount here, instead of later. Cause later makes no sense.
   this->updateMaxAmt();

   return true;
}

void MashDesigner::saveStep() {
   this->mashStep->setName(this->lineEdit_name->text());
   this->mashStep->setType(static_cast<MashStep::Type>(comboBox_type->currentIndex()));
   // Bound the target temperature to what can be achieved
   this->mashStep->setStepTemp_c(this->bound_temp_c(this->lineEdit_temp->toSI().quantity));
   this->mashStep->setStepTime_min(lineEdit_time->toSI().quantity);

   // finish a few things -- this may be premature optimization
   if (isInfusion()) {
      this->mashStep->setInfuseAmount_l(selectedAmount_l());
      this->mashStep->setInfuseTemp_c(this->selectedTemp_c());
   }

   // Mash::addMashStep() will ensure the mash step is stored in the DB and has the correct mash ID etc
   this->mash->addMashStep(this->mashStep);
   return;
}

double MashDesigner::stepTemp_c() {
   return lineEdit_temp->toSI().quantity;
}

bool MashDesigner::heating() {
   // Returns true if the current step is hotter than the previous step
   return stepTemp_c() >= (this->prevStep ? this->prevStep->stepTemp_c() : this->mash->grainTemp_c());
}

double MashDesigner::boilingTemp_c() {
   // Returns the equipment boiling point if available, otherwise 100.0
   if (recObs && recObs->equipment()) {
      return recObs->equipment()->boilingPoint_c();
   }
   return 100;
}

double MashDesigner::maxTemp_c() {
   return (heating())? boilingTemp_c() : tempFromVolume_c(maxAmt_l());
}

double MashDesigner::minTemp_c() {
   return (heating())? tempFromVolume_c(maxAmt_l()) : 0.0;
}

double MashDesigner::bound_temp_c(double temp_c) {
   // Returns the closest achievable temperature to temp_c
   return (heating()) ? std::min(temp_c, maxTemp_c()) : std::max(temp_c, minTemp_c());
}

// The mash volume up to and not including the step currently being edited.
double MashDesigner::mashVolume_l() {
   return grain_kg/PhysicalConstants::grainDensity_kgL + addedWater_l;
}

double MashDesigner::minAmt_l() {
   double minVol_l = volFromTemp_l((heating())? maxTemp_c() : minTemp_c());

   // Sanity checks
   // TODO: If minVol_l > maxAmt_l(), change the target temp?
   minVol_l = std::min(minVol_l, maxAmt_l());

   return minVol_l;
}

// However much more we can add at this step.
double MashDesigner::maxAmt_l() {
   double amt = 0;

   if (equip == nullptr) {
      return amt;
   }

   // However much more we can fit in the tun.
   if (!isSparge()) {
      amt = equip->tunVolume_l() - mashVolume_l();
   } else {
      amt = equip->tunVolume_l() - grainVolume_l();
   }

   return std::min(amt, recObs->targetTotalMashVol_l() - addedWater_l);
}

// Returns the required volume of water to infuse if the strike water is
// at temp_c degrees Celsius.
double MashDesigner::volFromTemp_l(double temp_c) {
   if (!this->mashStep || !this->mash) {
      return 0.0;
   }

   double tw = temp_c;
   // Final temp is target temp.
   double tf = stepTemp_c();
   // Initial temp is the last step's temp if the last step exists, otherwise the grain temp.
   double t1 = (!this->prevStep) ? mash->grainTemp_c() : this->prevStep->stepTemp_c();
   double mt = mash->tunSpecificHeat_calGC();
   double ct = mash->tunWeight_kg();

   double mw = 1/(HeatCalculations::Cw_calGC * (tw - tf)) *
      (MC * (tf - t1) + ((!this->prevStep) ? mt * ct * (tf - this->mash->tunTemp_c()) : 0));

   // Sanity check for unlikely edge cases
   mw = std::max(0., mw);

   // NOTE: This needs to be changed. Assumes 1L of water is 1 kg.
   return mw;
}

// Returns the required temp of strike water required if
// the volume of strike water is vol_l liters.
double MashDesigner::tempFromVolume_c(double vol_l) {
   if (!this->mashStep || !this->mash) {
      return 0.0;
   }

   double absorption_LKg;
   if (this->equip) {
      absorption_LKg = this->equip->grainAbsorption_LKg();
   } else {
      absorption_LKg = PhysicalConstants::grainAbsorption_Lkg;
   }

   double tf = stepTemp_c();

   // NOTE: This needs to be changed. Assumes 1L = 1 kg.
   double mw = vol_l;
   if (mw <= 0) {
      return 0.0;
   }
   double cw = HeatCalculations::Cw_calGC;
   // Initial temp is the last step's temp if the last step exists, otherwise the grain temp.
   double t1 = (!this->prevStep) ? this->mash->grainTemp_c() : this->prevStep->stepTemp_c();
   // When batch sparging, you lose about 10C from previous step.
   if (isSparge()) {
      t1 = (!this->prevStep) ? this->mash->grainTemp_c() : this->prevStep->stepTemp_c() - 10;
   }
   double mt = this->mash->tunSpecificHeat_calGC();
   double ct = this->mash->tunWeight_kg();

   double batchMC = grain_kg * HeatCalculations::Cgrain_calGC
                    + absorption_LKg * grain_kg * HeatCalculations::Cw_calGC
                    + this->mash->tunWeight_kg() * this->mash->tunSpecificHeat_calGC();

   double tw = 1 / (mw * cw) * (
      (this->isSparge() ? batchMC : MC) * (tf - t1) + (!this->prevStep ? mt * ct * (tf - this->mash->tunTemp_c()) : 0)
   ) + tf;

   // Sanity check this value
   tw = std::min(tw, boilingTemp_c());  // Can't add water above boiling
   tw = std::max(tw, 0.0);  // (Probably) won't add water below freezing

   return tw;
}

// How many liters of grain are in the tun.
double MashDesigner::grainVolume_l() {
   return grain_kg / PhysicalConstants::grainDensity_kgL;
}

// After this, mash and equip are non-null iff we return true.
bool MashDesigner::initializeMash() {
   qDebug() << Q_FUNC_INFO << "Observing" << this->recObs;
   if (this->recObs == nullptr) {
      return false;
   }

   this->equip = this->recObs->equipment();
   if (this->equip == nullptr) {
      QMessageBox::warning(this,
                           tr("No Equipment"),
                           tr("You have not set an equipment for this recipe. We really cannot continue without one."));
      return false;
   }

   bool ok = false;
   QString dialogText = QInputDialog::getText(
      this,
      tr("Tun Temp"),
      tr("Enter the temperature of the tun before your first infusion."),
      QLineEdit::Normal, //default,
      QString(),
      &ok
      //don't need the widget pointer - default is parent
  );

   //if user hits cancel, cancel out of the dialog and quit the mashDesigner
   //edited jazzbeerman (dcavanagh) 8/20/10
   if (!ok) {
      return false;
   }

   this->mash = this->recObs->getMash();
   if (this->mash == nullptr) {
      qDebug() << Q_FUNC_INFO << "Create new Mash";
      this->mash = std::make_shared<Mash>("");
   } else {
      qDebug() << Q_FUNC_INFO << "Clear all steps of existing Mash";
      this->mash->removeAllMashSteps();
   }

   // Order matters. Don't do this until every that could return false has
   this->mash->setTunSpecificHeat_calGC(this->equip->tunSpecificHeat_calGC());
   this->mash->setTunWeight_kg(this->equip->tunWeight_kg());
   this->mash->setTunTemp_c(Measurement::qStringToSI(dialogText, Measurement::PhysicalQuantity::Temperature).quantity);

   this->curStep = 0;
   this->addedWater_l = 0;
   this->mashStep.reset();
   this->prevStep.reset();

   this->MC = recObs->grainsInMash_kg() * HeatCalculations::Cgrain_calGC;
   this->grain_kg = recObs->grainsInMash_kg();

   this->label_tunVol->setText(Measurement::displayAmount(Measurement::Amount{equip->tunVolume_l(), Measurement::Units::liters}));
   this->label_wortMax->setText(Measurement::displayAmount(Measurement::Amount{recObs->targetCollectedWortVol_l(), Measurement::Units::liters}));

   this->updateMinAmt();
   this->updateMaxAmt();
   this->updateMinTemp();
   this->updateMaxTemp();
   this->updateFullness();
   this->horizontalSlider_amount->setValue(0); // As thick as possible initially.

   if (this->mash->key() < 0) {
      qDebug() << Q_FUNC_INFO << "Add new Mash to Recipe";
      ObjectStoreWrapper::insert(*mash);
      this->recObs->setMash(mash);
   }
   return true;
}

void MashDesigner::updateFullness() {
   if (!this->mashStep) {
      return;
   }

   if (this->equip == nullptr) {
      this->progressBar_fullness->setValue(0);
      return;
   }

   double vol_l;
   if (!isSparge()) {
      vol_l = mashVolume_l() + (isInfusion() ? selectedAmount_l() : 0);
   } else {
      vol_l = grainVolume_l() + selectedAmount_l();
   }

   double ratio = vol_l / equip->tunVolume_l();
   if (ratio < 0) {
     ratio = 0;
   } else if (ratio > 1) {
     ratio = 1;
   }

   this->progressBar_fullness->setValue(static_cast<int>(ratio*progressBar_fullness->maximum()));
   this->label_mashVol->setText(Measurement::displayAmount(Measurement::Amount{vol_l, Measurement::Units::liters}));
   this->label_thickness->setText(Measurement::displayThickness((addedWater_l + (isInfusion() ? selectedAmount_l() : 0))/grain_kg));
   return;
}

double MashDesigner::waterFromMash_l() {
   if (this->recObs == nullptr) {
      return 0.0;
   }

   double waterAdded_l = this->mash->totalMashWater_l();

   // A newly-created mash step will not yet have been added to the mash
   if (this->mashStep && this->mashStep->getMashId() <= 0) {
      if (this->isInfusion()) {
         waterAdded_l += this->mashStep->infuseAmount_l();
      }
   }

   double absorption_lKg;
   if (equip) {
      absorption_lKg = equip->grainAbsorption_LKg();
   } else {
      absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;
   }

   qDebug() <<
      Q_FUNC_INFO << "waterAdded_l=" << waterAdded_l << ", absorption_lKg=" << absorption_lKg << "grainsInMash_kg=" <<
      this->recObs->grainsInMash_kg();

   return (waterAdded_l - absorption_lKg * this->recObs->grainsInMash_kg());
}

void MashDesigner::updateCollectedWort() {
   qDebug() << Q_FUNC_INFO;
   if (recObs == nullptr) {
      return;
   }

   // double wort_l = this->recObs->wortFromMash_l();
   double wort_l = this->waterFromMash_l();
   double targetCollectedWort_l = this->recObs->targetCollectedWortVol_l();

   double ratio = wort_l / targetCollectedWort_l;
   qDebug() <<
      Q_FUNC_INFO << "waterFromMash=" << wort_l << "Liters, targetCollectedWort=" << targetCollectedWort_l << "Litres, "
      "Unadjusted ratio=" << ratio;
   if (ratio < 0) {
     ratio = 0;
   } else if (ratio > 1) {
     ratio = 1;
   }

   this->label_wort->setText(Measurement::displayAmount(Measurement::Amount{wort_l, Measurement::Units::liters}));
   this->progressBar_wort->setValue(static_cast<int>(ratio * this->progressBar_wort->maximum()));
   return;
}

void MashDesigner::updateMinAmt() {
   label_amtMin->setText(Measurement::displayAmount(Measurement::Amount{minAmt_l(), Measurement::Units::liters}));
}

void MashDesigner::updateMaxAmt() {
   label_amtMax->setText(Measurement::displayAmount(Measurement::Amount{maxAmt_l(), Measurement::Units::liters}));
}

void MashDesigner::updateMinTemp() {
   label_tempMin->setText(Measurement::displayAmount(Measurement::Amount{minTemp_c(), Measurement::Units::celsius}));
}

void MashDesigner::updateMaxTemp() {
   label_tempMax->setText(Measurement::displayAmount(Measurement::Amount{maxTemp_c(), Measurement::Units::celsius}));
}

double MashDesigner::selectedAmount_l() {
   double ratio = horizontalSlider_amount->sliderPosition() / static_cast<double>(horizontalSlider_amount->maximum());
   double minAmt = minAmt_l();
   double maxAmt = maxAmt_l();
   double amt = minAmt + (maxAmt - minAmt)*ratio;

   return amt;
}

double MashDesigner::selectedTemp_c() {
   double ratio = horizontalSlider_temp->sliderPosition() / static_cast<double>(horizontalSlider_temp->maximum());
   double minT = minTemp_c();
   double maxT = maxTemp_c();
   double T = minT + (maxT - minT)*ratio;

   return T;
}

void MashDesigner::updateTempSlider() {
   if (!this->mashStep) {
      return;
   }

   if (isInfusion()) {
      double temp = tempFromVolume_c(selectedAmount_l());

      double ratio = (temp-minTemp_c()) / (maxTemp_c() - minTemp_c());
      horizontalSlider_temp->setValue(static_cast<int>(ratio*horizontalSlider_temp->maximum()));

      if (this->mashStep) {
         this->mashStep->setInfuseTemp_c(temp);
      }
   } else if (isDecoction()) {
      horizontalSlider_temp->setValue(horizontalSlider_temp->maximum());
   } else {
      horizontalSlider_temp->setValue(static_cast<int>(0.5*horizontalSlider_temp->maximum()));
   }
   return;
}

void MashDesigner::updateAmtSlider() {
   if (!this->mashStep) {
      return;
   }

   if (isInfusion()) {
      double vol = volFromTemp_l(selectedTemp_c());
      double ratio = (vol - minAmt_l()) / (maxAmt_l() - minAmt_l());

      horizontalSlider_amount->setValue(static_cast<int>(ratio*horizontalSlider_amount->maximum()));
      if (this->mashStep) {
         this->mashStep->setInfuseAmount_l(vol);
      }
   }
   else {
      horizontalSlider_amount->setValue(static_cast<int>(0.5*horizontalSlider_amount->maximum()));
   }
   return;
}

void MashDesigner::updateAmt() {
   if (!this->mashStep) {
      return;
   }

   if (this->isInfusion()) {
      double vol = horizontalSlider_amount->sliderPosition() / static_cast<double>(horizontalSlider_amount->maximum())* (maxAmt_l() - minAmt_l()) + minAmt_l();

      label_amt->setText(Measurement::displayAmount(Measurement::Amount{vol, Measurement::Units::liters}));

      if (this->mashStep) {
         this->mashStep->setInfuseAmount_l(vol);
      }
   } else if (isDecoction()) {
      label_amt->setText(Measurement::displayAmount(Measurement::Amount{this->mashStep->decoctionAmount_l(), Measurement::Units::liters}));
   } else {
      label_amt->setText(Measurement::displayAmount(Measurement::Amount{0, Measurement::Units::liters}));
   }
   return;
}

void MashDesigner::updateTemp() {
   if (!this->mashStep) {
      return;
   }

   if (isInfusion())  {
      double temp = horizontalSlider_temp->sliderPosition() / static_cast<double>(horizontalSlider_temp->maximum()) * (maxTemp_c() - minTemp_c()) + minTemp_c();
      double maxT = maxTemp_c();
      if (temp > maxT)
         temp = maxT;

      label_temp->setText(Measurement::displayAmount(Measurement::Amount{temp, Measurement::Units::celsius}));

      if (this->mashStep) {
         this->mashStep->setInfuseTemp_c(temp);
      }
   } else if (isDecoction()) {
      label_temp->setText(Measurement::displayAmount(Measurement::Amount{maxTemp_c(), Measurement::Units::celsius}));
   } else {
      label_temp->setText(Measurement::displayAmount(Measurement::Amount{stepTemp_c(), Measurement::Units::celsius}));
   }
}

void MashDesigner::saveTargetTemp() {
   double temp = this->bound_temp_c(this->stepTemp_c());

   // be nice and reset the field so it displays in proper units
   this->lineEdit_temp->setText(temp);
   if (this->mashStep) {
      this->mashStep->setStepTemp_c(temp);
   }

   if (this->isDecoction()) {
      if (this->mashStep) {
         this->mashStep->setDecoctionAmount_l(this->getDecoctionAmount_l());
      }

      this->updateAmtSlider();
      this->updateAmt();
      this->updateTempSlider();
      this->updateTemp();
   }

   this->updateMinAmt();
   this->updateMaxAmt();
   this->updateMinTemp();
   this->updateMaxTemp();
   this->updateAmt();
   this->updateTempSlider();
   this->updateTemp();
   this->updateFullness();
   this->updateCollectedWort();
   return;
}

double MashDesigner::getDecoctionAmount_l() {
   double m_w, m_g, r;
   double c_w, c_g;
   double tf, t1;

   if (!this->prevStep) {
      QMessageBox::critical(this, tr("Decoction error"), tr("The first mash step cannot be a decoction."));
      qCritical() << "MashDesigner: First step not a decoction.";
      return 0;
   }
   tf = stepTemp_c();
   t1 = this->prevStep->stepTemp_c();

   m_w = addedWater_l; // NOTE: this is bad. Assumes 1L = 1 kg.
   m_g = grain_kg;

   c_w = HeatCalculations::Cw_calGC;
   c_g = HeatCalculations::Cgrain_calGC;

   // r is the ratio of water and grain to take out for decoction.
   r = ((MC)*(tf-t1)) / ((m_w*c_w + m_g*c_g)*(maxTemp_c()-tf) + (m_w*c_w + m_g*c_g)*(tf-t1));
   if (r < 0 || r > 1) {
      //QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation."));
      //Brewtarget::log(Brewtarget::ERROR, QString("MashDesigner Decoction: r=%1").arg(r));
      return 0;
   }

   return r*mashVolume_l();
}

bool MashDesigner::isBatchSparge() const {
   MashStep::Type stepType = type();
   return (stepType == MashStep::batchSparge);
}

bool MashDesigner::isFlySparge() const {
   MashStep::Type stepType = type();
   return (stepType == MashStep::flySparge);
}

bool MashDesigner::isSparge() const {
   return (isBatchSparge() || isFlySparge());
}

bool MashDesigner::isInfusion() const {
   MashStep::Type stepType = type();
   return (stepType == MashStep::Infusion || isSparge());
}

bool MashDesigner::isDecoction() const {
   MashStep::Type stepType = type();
   return (stepType == MashStep::Decoction);
}

bool MashDesigner::isTemperature() const {
   MashStep::Type stepType = type();
   return (stepType == MashStep::Temperature);
}

MashStep::Type MashDesigner::type() const {
   int curIdx = comboBox_type->currentIndex();
   return static_cast<MashStep::Type>(curIdx);
}

void MashDesigner::typeChanged() {
   MashStep::Type stepType = type();

   if (this->mashStep) {
      this->mashStep->setType(stepType);
   }

   // fly sparge is the end of the line. No more steps can be added after
   if (isFlySparge()) {
      // more will happen here, but I need to understand a few things
      pushButton_next->setEnabled(false);
   } else if (! pushButton_next->isEnabled()) {
      pushButton_next->setEnabled(true);
   }

   if (isInfusion()) {
      horizontalSlider_amount->setEnabled(true);
      horizontalSlider_temp->setEnabled(true);
      updateMaxAmt();
   } else if (isDecoction()) {
      horizontalSlider_amount->setEnabled(false);
      horizontalSlider_temp->setEnabled(false);

      if (this->mashStep) {
         this->mashStep->setDecoctionAmount_l(getDecoctionAmount_l());
      }

      updateAmtSlider();
      updateAmt();
      updateTempSlider();
      updateTemp();
   } else if (isTemperature()) {
      horizontalSlider_amount->setEnabled(false);
      horizontalSlider_temp->setEnabled(false);
   }
   return;
}
