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

MashDesigner::MashDesigner(QWidget* parent) : QDialog(parent) {
   this->setupUi(this);

   this->recObs = nullptr;
   this->mash = nullptr;
   this->equip = nullptr;
   this->addedWater_l = 0;
   this->mashStep = nullptr;
   this->prevStep = nullptr;

   this->label_zeroVol->setText(Measurement::displayAmount(Measurement::Amount{0, Measurement::Units::liters}));
   this->label_zeroWort->setText(Measurement::displayAmount(Measurement::Amount{0, Measurement::Units::liters}));

   // Update temp slider when we move amount slider.
   connect( horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateTempSlider );
   // Update amount slider when we move temp slider.
   connect( horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateAmtSlider );
   // Update tun fullness bar when either slider moves.
   connect( horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateFullness );
   connect( horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateFullness );
   // Update amount/temp text when sliders move.
   connect( horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateAmt );
   connect( horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateTemp );
   connect( horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateAmt );
   connect( horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateTemp );
   // Update collected wort when sliders move.
   connect( horizontalSlider_amount, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateCollectedWort );
   connect( horizontalSlider_temp, &QAbstractSlider::sliderMoved, this, &MashDesigner::updateCollectedWort );
   // Save the target temp whenever it's changed.
   connect( lineEdit_temp, &BtLineEdit::textModified, this, &MashDesigner::saveTargetTemp );
   // Move to next step.
   connect( pushButton_next, &QAbstractButton::clicked, this, &MashDesigner::proceed );
   // Do correct calcs when the mash step type is selected.
   connect( comboBox_type, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MashDesigner::typeChanged );

   // I still dislike this part. But I also need to "fix" the form
   // connect( checkBox_batchSparge, SIGNAL(clicked()), this, SLOT(updateMaxAmt()) );
   connect( pushButton_finish, &QAbstractButton::clicked, this, &MashDesigner::saveAndClose );

   return;
}

void MashDesigner::proceed()
{
   nextStep(++curStep);
}

void MashDesigner::setRecipe(Recipe* rec)
{
   recObs = rec;
   if( isVisible() )
      setVisible(false);
}

void MashDesigner::show()
{
   // No point to run unless we have fermentables.
   if( recObs && recObs->fermentables().size() == 0 )
   {
      QMessageBox::information(
         this,
         tr("No Fermentables"),
         tr("Your recipe must have fermentables to design a mash.")
      );
      return;
   }
   setVisible(nextStep(0));
}

void MashDesigner::saveAndClose()
{
   saveStep();
   setVisible(false);
}

bool MashDesigner::nextStep(int step)
{

   if( step == 0  && ! initializeMash() ) {
      return false;
   }
   else if( step > 0 ) {
      saveStep();
   }

   prevStep = mashStep;
   if ( mashStep != nullptr ) {
      // NOTE: This needs to be changed. Assumes 1L of water is 1 kg.
      MC += mashStep->infuseAmount_l() * HeatCalculations::Cw_calGC;
      addedWater_l += mashStep->infuseAmount_l();

      if( prevStep == nullptr ) // If the last step is null, we need to add the influence of the tun.
         MC += mash->tunSpecificHeat_calGC() * mash->tunWeight_kg();
   }

   // If we have a step number, and the step is smaller than the current
   // number of mashsteps. How can this happen? When you get into
   // the mash designer, the first thing it does is clear all the steps.
   if ( step >= 0 && step < mash->mashSteps().size() )
      mashStep = mash->mashSteps()[step];
   else {
      // .:TODO:. Change to shared_ptr as potential memory leak
      mashStep = new MashStep("");
   }

   // Clear out some of the fields.
   lineEdit_name->clear();
   lineEdit_temp->clear();
   lineEdit_time->clear();

   horizontalSlider_amount->setValue(0); // Least amount of water.
   // Update max amount here, instead of later. Cause later makes no sense.
   updateMaxAmt();

   return true;
}

void MashDesigner::saveStep() {
   MashStep::Type type = static_cast<MashStep::Type>(comboBox_type->currentIndex());
   // Bound the target temperature to what can be achieved
   double temp = this->bound_temp_c(lineEdit_temp->toSI().quantity);

   this->mashStep->setName(lineEdit_name->text());
   this->mashStep->setType(type);
   this->mashStep->setStepTemp_c(temp);
   this->mashStep->setStepTime_min(lineEdit_time->toSI().quantity);

   // finish a few things -- this may be premature optimization
   if (isInfusion()) {
      this->mashStep->setInfuseAmount_l( selectedAmount_l() );
      temp = selectedTemp_c();
      this->mashStep->setInfuseTemp_c( temp );
   }

   if (this->mashStep->key() < 0) {
      this->mashStep->setMashId(this->mash->key());
      ObjectStoreWrapper::insert(*this->mashStep);
   }
   return;
}

double MashDesigner::stepTemp_c()
{
   return lineEdit_temp->toSI().quantity;
}

bool MashDesigner::heating()
{
   // Returns true if the current step is hottern than the previous step
   return stepTemp_c() >= ((prevStep) ? prevStep->stepTemp_c() : mash->grainTemp_c());
}

double MashDesigner::boilingTemp_c()
{
   // Returns the equipment boiling point if available, otherwise 100.0
   if (recObs && recObs->equipment())
      return recObs->equipment()->boilingPoint_c();
   else
      return 100;
}

double MashDesigner::maxTemp_c()
{
   return (heating())? boilingTemp_c() : tempFromVolume_c(maxAmt_l());
}

double MashDesigner::minTemp_c()
{
   return (heating())? tempFromVolume_c(maxAmt_l()) : 0.0;
}

double MashDesigner::bound_temp_c(double temp_c)
{
   // Returns the closest achievable temperature to temp_c
   return (heating()) ? std::min(temp_c, maxTemp_c()) : std::max(temp_c, minTemp_c());
}

// The mash volume up to and not including the step currently being edited.
double MashDesigner::mashVolume_l()
{
   return grain_kg/PhysicalConstants::grainDensity_kgL + addedWater_l;
}

double MashDesigner::minAmt_l()
{
   double minVol_l = volFromTemp_l( (heating())? maxTemp_c() : minTemp_c() );

   // Sanity checks
   // TODO: If minVol_l > maxAmt_l(), change the target temp?
   minVol_l = std::min(minVol_l, maxAmt_l());

   return minVol_l;
}

// However much more we can add at this step.
double MashDesigner::maxAmt_l()
{
   double amt = 0;

   if ( equip == nullptr )
      return amt;

   // However much more we can fit in the tun.
   if( ! isSparge() )
   {
      amt = equip->tunVolume_l() - mashVolume_l();
   }
   else
   {
      amt = equip->tunVolume_l() - grainVolume_l();
   }

   amt = std::min(amt, recObs->targetTotalMashVol_l() - addedWater_l);

   return amt;
}

// Returns the required volume of water to infuse if the strike water is
// at temp_c degrees Celsius.
double MashDesigner::volFromTemp_l( double temp_c )
{
   if( mashStep == nullptr || mash == nullptr )
      return 0.0;

   double tw = temp_c;
   // Final temp is target temp.
   double tf = stepTemp_c();
   // Initial temp is the last step's temp if the last step exists, otherwise the grain temp.
   double t1 = (prevStep==nullptr)? mash->grainTemp_c() : prevStep->stepTemp_c();
   double mt = mash->tunSpecificHeat_calGC();
   double ct = mash->tunWeight_kg();

   double mw = 1/(HeatCalculations::Cw_calGC * (tw-tf)) * (MC*(tf-t1) + ((prevStep==nullptr)? mt*ct*(tf-mash->tunTemp_c()) : 0) );

   // Sanity check for unlikely edge cases
   mw = std::max(0., mw);

   // NOTE: This needs to be changed. Assumes 1L of water is 1 kg.
   return mw;
}

// Returns the required temp of strike water required if
// the volume of strike water is vol_l liters.
double MashDesigner::tempFromVolume_c( double vol_l )
{
   if( mashStep == nullptr || mash == nullptr )
      return 0.0;

   double absorption_LKg;
   if( equip != nullptr )
      absorption_LKg = equip->grainAbsorption_LKg();
   else
      absorption_LKg = PhysicalConstants::grainAbsorption_Lkg;

   double tf = stepTemp_c();

   // NOTE: This needs to be changed. Assumes 1L = 1 kg.
   double mw = vol_l;
   if( mw <= 0 )
      return 0.0;
   double cw = HeatCalculations::Cw_calGC;
   // Initial temp is the last step's temp if the last step exists, otherwise the grain temp.
   double t1 = (prevStep==nullptr)? mash->grainTemp_c() : prevStep->stepTemp_c();
   // When batch sparging, you lose about 10C from previous step.
   if( isSparge() )
      t1 = (prevStep==nullptr)? mash->grainTemp_c() : prevStep->stepTemp_c() - 10;
   double mt = mash->tunSpecificHeat_calGC();
   double ct = mash->tunWeight_kg();

   double batchMC = grain_kg * HeatCalculations::Cgrain_calGC
                    + absorption_LKg * grain_kg * HeatCalculations::Cw_calGC
                    + mash->tunWeight_kg() * mash->tunSpecificHeat_calGC();

   double tw = 1/(mw*cw) * ( (isSparge()? batchMC : MC) * (tf-t1) + ((prevStep==nullptr)? mt*ct*(tf-mash->tunTemp_c()) : 0) ) + tf;

   // Sanity check this value
   tw = std::min( tw, boilingTemp_c() );  // Can't add water above boiling
   tw = std::max( tw, 0.0 );  // (Probably) won't add water below freezing

   return tw;
}

// How many liters of grain are in the tun.
double MashDesigner::grainVolume_l()
{
   return grain_kg / PhysicalConstants::grainDensity_kgL;
}

// After this, mash and equip are non-null iff we return true.
bool MashDesigner::initializeMash() {
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

   bool ok;
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

   this->mash = recObs->mash();
   if (this->mash == nullptr) {
      // .:TODO:. Change to shared_ptr as potential memory leak
      this->mash = new Mash("");
   } else {
      this->mash->removeAllMashSteps();
   }

   // Order matters. Don't do this until every that could return false has
   this->mash->setTunSpecificHeat_calGC(this->equip->tunSpecificHeat_calGC());
   this->mash->setTunWeight_kg(this->equip->tunWeight_kg());
   this->mash->setTunTemp_c(Measurement::qStringToSI(dialogText, Measurement::PhysicalQuantity::Temperature).quantity);

   this->curStep = 0;
   this->addedWater_l = 0;
   this->mashStep = nullptr;
   this->prevStep = nullptr;

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
      ObjectStoreWrapper::insert(*mash);
      this->recObs->setMash(mash);
   }
   return true;
}

void MashDesigner::updateFullness() {
   if (this->mashStep == nullptr) {
      return;
   }

   if (this->equip == nullptr) {
      this->progressBar_fullness->setValue(0);
      return;
   }

   double vol_l;
   if (!isSparge()) {
      vol_l = mashVolume_l() + ( isInfusion() ? selectedAmount_l() : 0);
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
   this->label_thickness->setText(Measurement::displayThickness( (addedWater_l + (isInfusion() ? selectedAmount_l() : 0) )/grain_kg ));
   return;
}

double MashDesigner::waterFromMash_l()
{
   double waterAdded_l = mash->totalMashWater_l();
   double absorption_lKg;

   if ( recObs == nullptr )
      return 0.0;

   if( equip )
      absorption_lKg = equip->grainAbsorption_LKg();
   else
      absorption_lKg = PhysicalConstants::grainAbsorption_Lkg;

   return (waterAdded_l - absorption_lKg * recObs->grainsInMash_kg());
}

void MashDesigner::updateCollectedWort() {
   if( recObs == nullptr )
      return;

   // double wort_l = recObs->wortFromMash_l();
   double wort_l = waterFromMash_l();

   double ratio = wort_l / recObs->targetCollectedWortVol_l();
   if( ratio < 0 )
     ratio = 0;
   if( ratio > 1 )
     ratio = 1;

   label_wort->setText(Measurement::displayAmount(Measurement::Amount{wort_l, Measurement::Units::liters}));
   progressBar_wort->setValue( static_cast<int>(ratio * progressBar_wort->maximum() ));
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

double MashDesigner::selectedTemp_c()
{
   double ratio = horizontalSlider_temp->sliderPosition() / static_cast<double>(horizontalSlider_temp->maximum());
   double minT = minTemp_c();
   double maxT = maxTemp_c();
   double T = minT + (maxT - minT)*ratio;

   return T;
}

void MashDesigner::updateTempSlider()
{
   if( mashStep == nullptr )
      return;

   if( isInfusion() )
   {
      double temp = tempFromVolume_c( selectedAmount_l() );

      double ratio = (temp-minTemp_c()) / (maxTemp_c() - minTemp_c());
      horizontalSlider_temp->setValue(static_cast<int>(ratio*horizontalSlider_temp->maximum()));

      if( mashStep != nullptr )
         mashStep->setInfuseTemp_c( temp );
   }
   else if( isDecoction() )
   {
      horizontalSlider_temp->setValue(horizontalSlider_temp->maximum());
   }
   else
      horizontalSlider_temp->setValue(static_cast<int>(0.5*horizontalSlider_temp->maximum()));
}

void MashDesigner::updateAmtSlider()
{
   if( mashStep == nullptr )
      return;

   if( isInfusion() )
   {
      double vol = volFromTemp_l( selectedTemp_c() );
      double ratio = (vol - minAmt_l()) / (maxAmt_l() - minAmt_l());

      horizontalSlider_amount->setValue(static_cast<int>(ratio*horizontalSlider_amount->maximum()));
      if( mashStep != nullptr )
         mashStep->setInfuseAmount_l(vol);
   }
   else
      horizontalSlider_amount->setValue(static_cast<int>(0.5*horizontalSlider_amount->maximum()));
}

void MashDesigner::updateAmt()
{
   if( mashStep == nullptr )
      return;

   if( isInfusion() )
   {
      double vol = horizontalSlider_amount->sliderPosition() / static_cast<double>(horizontalSlider_amount->maximum())* (maxAmt_l() - minAmt_l()) + minAmt_l();

      label_amt->setText(Measurement::displayAmount(Measurement::Amount{vol, Measurement::Units::liters}));

      if( mashStep != nullptr )
         mashStep->setInfuseAmount_l( vol );
   }
   else if( isDecoction() )
      label_amt->setText(Measurement::displayAmount(Measurement::Amount{mashStep->decoctionAmount_l(), Measurement::Units::liters}));
   else
      label_amt->setText(Measurement::displayAmount(Measurement::Amount{0, Measurement::Units::liters}));
}

void MashDesigner::updateTemp() {

   if (mashStep == nullptr) {
      return;
   }

   if (isInfusion())  {
      double temp = horizontalSlider_temp->sliderPosition() / static_cast<double>(horizontalSlider_temp->maximum()) * (maxTemp_c() - minTemp_c()) + minTemp_c();
      double maxT = maxTemp_c();
      if ( temp > maxT )
         temp = maxT;

      label_temp->setText(Measurement::displayAmount(Measurement::Amount{temp, Measurement::Units::celsius}));

      if (mashStep != nullptr) {
         mashStep->setInfuseTemp_c(temp);
      }
   } else if (isDecoction()) {
      label_temp->setText(Measurement::displayAmount(Measurement::Amount{maxTemp_c(), Measurement::Units::celsius}));
   } else {
      label_temp->setText(Measurement::displayAmount(Measurement::Amount{stepTemp_c(), Measurement::Units::celsius}));
   }
}

void MashDesigner::saveTargetTemp() {
   double temp = stepTemp_c();

   temp = bound_temp_c(temp);

   // be nice and reset the field so it displays in proper units
   lineEdit_temp->setText(temp);
   if (mashStep != nullptr) {
      mashStep->setStepTemp_c(temp);
   }

   if (isDecoction()) {
      if (mashStep != nullptr) {
         mashStep->setDecoctionAmount_l(getDecoctionAmount_l());
      }

      updateAmtSlider();
      updateAmt();
      updateTempSlider();
      updateTemp();
   }

   updateMinAmt();
   updateMaxAmt();
   updateMinTemp();
   updateMaxTemp();
   updateAmt();
   updateTempSlider();
   updateTemp();
   updateFullness();
   updateCollectedWort();
}

double MashDesigner::getDecoctionAmount_l() {
   double m_w, m_g, r;
   double c_w, c_g;
   double tf, t1;

   if( prevStep == nullptr ) {
      QMessageBox::critical(this, tr("Decoction error"), tr("The first mash step cannot be a decoction."));
      qCritical() << "MashDesigner: First step not a decoction.";
      return 0;
   }
   tf = stepTemp_c();
   t1 = prevStep->stepTemp_c();

   m_w = addedWater_l; // NOTE: this is bad. Assumes 1L = 1 kg.
   m_g = grain_kg;

   c_w = HeatCalculations::Cw_calGC;
   c_g = HeatCalculations::Cgrain_calGC;

   // r is the ratio of water and grain to take out for decoction.
   r = ((MC)*(tf-t1)) / ((m_w*c_w + m_g*c_g)*(maxTemp_c()-tf) + (m_w*c_w + m_g*c_g)*(tf-t1));
   if( r < 0 || r > 1 ) {
      //QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation.") );
      //Brewtarget::log(Brewtarget::ERROR, QString("MashDesigner Decoction: r=%1").arg(r));
      return 0;
   }

   return r*mashVolume_l();
}

bool MashDesigner::isBatchSparge() const
{
   MashStep::Type _type = type();
   return (_type == MashStep::batchSparge);
}

bool MashDesigner::isFlySparge() const
{
   MashStep::Type _type = type();
   return (_type == MashStep::flySparge);
}

bool MashDesigner::isSparge() const
{
   return ( isBatchSparge() || isFlySparge() );
}

bool MashDesigner::isInfusion() const
{
   MashStep::Type _type = type();
   return ( _type == MashStep::Infusion || isSparge() );
}

bool MashDesigner::isDecoction() const
{
   MashStep::Type _type = type();
   return ( _type == MashStep::Decoction );
}

bool MashDesigner::isTemperature() const
{
   MashStep::Type _type = type();
   return ( _type == MashStep::Temperature );
}

MashStep::Type MashDesigner::type() const
{
   int curIdx = comboBox_type->currentIndex();
   return static_cast<MashStep::Type>(curIdx);
}

void MashDesigner::typeChanged()
{
   MashStep::Type _type = type();

   if( mashStep != nullptr )
      mashStep->setType(_type);

   // fly sparge is the end of the line. No more steps can be added after
   if ( isFlySparge() )
   {
      // more will happen here, but I need to understand a few things
      pushButton_next->setEnabled(false);
   }
   else if ( ! pushButton_next->isEnabled() )
      pushButton_next->setEnabled(true);

   if( isInfusion() )
   {
      horizontalSlider_amount->setEnabled(true);
      horizontalSlider_temp->setEnabled(true);
      updateMaxAmt();
   }
   else if( isDecoction() )
   {
      horizontalSlider_amount->setEnabled(false);
      horizontalSlider_temp->setEnabled(false);

      if( mashStep != nullptr )
         mashStep->setDecoctionAmount_l( getDecoctionAmount_l() );

      updateAmtSlider();
      updateAmt();
      updateTempSlider();
      updateTemp();
   }
   else if( isTemperature() )
   {
      horizontalSlider_amount->setEnabled(false);
      horizontalSlider_temp->setEnabled(false);
   }
}
