/*
 * MashDesigner.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2010.
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

#include "MashDesigner.h"
#include "equipment.h"
#include "mash.h"
#include "mashstep.h"
#include "brewtarget.h"
#include "HeatCalculations.h"
#include "unit.h"
#include <QMessageBox>
#include <QInputDialog>

MashDesigner::MashDesigner(QWidget* parent) : QDialog(parent)
{
   setupUi(this);

   recObs = 0;
   mash = 0;
   equip = 0;
   addedWater_l = 0;
   mashStep = 0;
   prevStep = 0;

   label_zeroVol->setText(Brewtarget::displayAmount(0, Units::liters));
   label_zeroWort->setText(Brewtarget::displayAmount(0, Units::liters));

   // Update temp slider when we move amount slider.
   connect( horizontalSlider_amount, SIGNAL(sliderMoved(int)), this, SLOT(updateTempSlider()) );
   // Update amount slider when we move temp slider.
   connect( horizontalSlider_temp, SIGNAL(sliderMoved(int)), this, SLOT(updateAmtSlider()) );
   // Update tun fullness bar when the amount slider moves.
   connect( horizontalSlider_amount, SIGNAL(valueChanged(int)), this, SLOT(updateFullness()) );
   connect( horizontalSlider_amount, SIGNAL(valueChanged(int)), this, SLOT(updateCollectedWort()) );
   // Update amount/temp text when sliders move.
   connect( horizontalSlider_amount, SIGNAL(valueChanged(int)), this, SLOT(updateAmt()) );
   connect( horizontalSlider_temp, SIGNAL(valueChanged(int)), this, SLOT(updateTemp()) );
   // Save the target temp whenever it's changed.
   connect( lineEdit_temp, SIGNAL(editingFinished()), this, SLOT(saveTargetTemp()) );
   // Move to next step.
   connect( pushButton_next, SIGNAL(clicked()), this, SLOT(proceed()) );
   // Do correct calcs when the mash step type is selected.
   connect( comboBox_type, SIGNAL(activated(QString)), this, SLOT(typeChanged(QString)) );

   connect( checkBox_batchSparge, SIGNAL(clicked()), this, SLOT(updateMaxAmt()) );
   connect( pushButton_finish, SIGNAL(clicked()), this, SLOT(saveAndClose()) );
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
   setVisible(nextStep(0));
}

void MashDesigner::saveAndClose()
{
   saveStep();
   setVisible(false);
}

bool MashDesigner::nextStep(int step)
{
   bool success = true;

   if( step == 0 )
   {
      success = initializeMash();
      if( ! success )
         return false;
   }
   else if( step > 0 )
      saveStep();

   if( mashStep != 0 )
   {
      // NOTE: This needs to be changed. Assumes 1L of water is 1 kg.
      MC += mashStep->getInfuseAmount_l() * HeatCalculations::Cw_calGC;
      addedWater_l += mashStep->getInfuseAmount_l();

      if( prevStep == 0 ) // If the last step is null, we need to add the influence of the tun.
         MC += mash->getTunSpecificHeat_calGC() * mash->getTunWeight_kg();
   }

   prevStep = mashStep;
   mashStep = mash->getMashStep(step);
   if( mashStep == 0 )
   {
      mashStep = new MashStep();
      mash->addMashStep(mashStep); // Come back to check on this later. Really need this new step to be inserted in right place.
   }

   // Clear out some of the fields.
   lineEdit_name->clear();
   lineEdit_temp->clear();
   lineEdit_time->clear();

   return true;
}

void MashDesigner::saveStep()
{
   QString type = comboBox_type->currentText();

   mashStep->setName( lineEdit_name->text().toStdString() );
   mashStep->setType( type.toStdString() );
   mashStep->setStepTemp_c( Brewtarget::tempQStringToSI(lineEdit_temp->text()) );
   mashStep->setStepTime_min( Brewtarget::timeQStringToSI(lineEdit_time->text()) );

   if( type.compare("Infusion") == 0 )
   {
      mashStep->setInfuseAmount_l( getSelectedAmount_l() );
      mashStep->setInfuseTemp_c( getSelectedTemp_c() );
   }
   /*
   else if( type.compare("Decoction") == 0 )
      mashStep->setDecoctionAmount_l(  );
    */
}

double MashDesigner::maxTemp_c()
{
   return 100;
}

double MashDesigner::minTemp_c()
{
   // The minimum temp depends on how much more water we can fit in the tun.
   return tempFromVolume_c( maxAmt_l() );
}

// The mash volume up to and not including the step currently being edited.
double MashDesigner::mashVolume_l()
{
   return grain_kg/HeatCalculations::rhoGrain_KgL + addedWater_l;
}

double MashDesigner::minAmt_l()
{
   // Minimum amount occurs with maximum temperature.
   return volFromTemp_l( maxTemp_c() );
}

// Returns the required volume of water to infuse if the strike water is
// at temp_c degrees Celsius.
double MashDesigner::volFromTemp_l( double temp_c )
{
   if( mashStep == 0 || mash == 0 )
      return 0.0;

   double tw = temp_c;
   // Final temp is target temp.
   double tf = mashStep->getStepTemp_c();
   // Initial temp is the last step's temp if the last step exists, otherwise the grain temp.
   double t1 = (prevStep==0)? mash->getGrainTemp_c() : prevStep->getStepTemp_c();
   double mt = mash->getTunSpecificHeat_calGC();
   double ct = mash->getTunWeight_kg();

   double mw = 1/(HeatCalculations::Cw_calGC * (tw-tf)) * (MC*(tf-t1) + ((prevStep==0)? mt*ct*(tf-mash->getTunTemp_c()) : 0) );

   // NOTE: This needs to be changed. Assumes 1L of water is 1 kg.
   return mw;
}

// Returns the required temp of strike water required if
// the volume of strike water is vol_l liters.
double MashDesigner::tempFromVolume_c( double vol_l )
{
   if( mashStep == 0 || mash == 0 )
      return 0.0;

   double absorption_LKg;
   if( equip != 0 )
      absorption_LKg = equip->getGrainAbsorption_LKg();
   else
      absorption_LKg = HeatCalculations::absorption_LKg;

   double tf = mashStep->getStepTemp_c();
   // NOTE: This needs to be changed. Assumes 1L = 1 kg.
   double mw = vol_l;
   if( mw <= 0 )
      return 0.0;
   double cw = HeatCalculations::Cw_calGC;
   // Initial temp is the last step's temp if the last step exists, otherwise the grain temp.
   double t1 = (prevStep==0)? mash->getGrainTemp_c() : prevStep->getStepTemp_c();
   // When batch sparging, you lose about 10C from previous step.
   if( isBatchSparge() )
      t1 = (prevStep==0)? mash->getGrainTemp_c() : prevStep->getStepTemp_c() - 10;
   double mt = mash->getTunSpecificHeat_calGC();
   double ct = mash->getTunWeight_kg();

   double batchMC = grain_kg * HeatCalculations::Cgrain_calGC
                    + absorption_LKg * grain_kg * HeatCalculations::Cw_calGC
                    + mash->getTunWeight_kg() * mash->getTunSpecificHeat_calGC();

   double tw = 1/(mw*cw) * ( (isBatchSparge()? batchMC : MC) * (tf-t1) + ((prevStep==0)? mt*ct*(tf-mash->getTunTemp_c()) : 0) ) + tf;

   return tw;
}

// However much more we can add at this step.
double MashDesigner::maxAmt_l()
{
   // However much more we can fit in the tun.
   if( ! isBatchSparge() )
      return (equip==0)? 0 : equip->getTunVolume_l() - mashVolume_l();
   else
      return (equip == 0)? 0 : equip->getTunVolume_l() - grainVolume_l();
}

// How many liters of grain are in the tun.
double MashDesigner::grainVolume_l()
{
   return grain_kg / HeatCalculations::rhoGrain_KgL;
}

// After this, mash and equip are non-null iff we return true.
bool MashDesigner::initializeMash()
{
   if(recObs == 0)
      return false;

   mash = recObs->getMash();
   if( mash == 0 )
   {
      mash = new Mash();
      recObs->setMash(mash);
   }

   equip = recObs->getEquipment();
   if( equip == 0 )
   {
      QMessageBox::warning(this, tr("No Equipment"), tr("You have not set an equipment for this recipe. We really cannot continue without one."));
      return false;
   }

   mash->setTunSpecificHeat_calGC( equip->getTunSpecificHeat_calGC() );
   mash->setTunWeight_kg( equip->getTunWeight_kg() );
   mash->setTunTemp_c( Brewtarget::tempQStringToSI( QInputDialog::getText(this, tr("Tun Temp"), tr("Enter the temperature of the tun before your first infusion.")) ) );

   mash->removeAllMashSteps();
   curStep = 0;
   MC = recObs->getGrainsInMash_kg() * HeatCalculations::Cgrain_calGC;
   addedWater_l = 0;
   mashStep = 0;
   prevStep = 0;
   grain_kg = recObs->getGrainsInMash_kg();

   label_tunVol->setText(Brewtarget::displayAmount(equip->getTunVolume_l(), Units::liters));
   label_wortMax->setText(Brewtarget::displayAmount(recObs->getBoilSize_l(), Units::liters));
   updateMinAmt();
   updateMaxAmt();
   updateMinTemp();
   updateMaxTemp();
   updateFullness();
   horizontalSlider_amount->setValue(0); // As thick as possible initially.

   return true;
}

void MashDesigner::updateFullness()
{
   if( mashStep == 0 )
      return;

   QString type(mashStep->getType().c_str());

   if( equip == 0 )
   {
      progressBar_fullness->setValue(0);
      return;
   }

   double vol_l;
   if( ! isBatchSparge() )
      vol_l = mashVolume_l() + ( (type.compare("Infusion") == 0) ? getSelectedAmount_l() : 0);
   else
      vol_l = grainVolume_l() + getSelectedAmount_l();
   double ratio = vol_l / equip->getTunVolume_l();

   progressBar_fullness->setValue(ratio*progressBar_fullness->maximum());
   label_mashVol->setText(Brewtarget::displayAmount(vol_l, Units::liters));
   label_thickness->setText(Brewtarget::displayThickness( (addedWater_l + ((mashStep->getType().compare("Infusion") == 0) ? getSelectedAmount_l() : 0) )/grain_kg ));
}

void MashDesigner::updateCollectedWort()
{
   if( recObs == 0 )
      return;

   double wort_l = recObs->estimateWortFromMash_l();
   double ratio = wort_l / recObs->getBoilSize_l();

   label_wort->setText(Brewtarget::displayAmount(wort_l, Units::liters));
   progressBar_wort->setValue( ratio * progressBar_wort->maximum() );
}

void MashDesigner::updateMinAmt()
{
   label_amtMin->setText(Brewtarget::displayAmount(minAmt_l(), Units::liters));
}

void MashDesigner::updateMaxAmt()
{
   label_amtMax->setText(Brewtarget::displayAmount(maxAmt_l(), Units::liters));
}

void MashDesigner::updateMinTemp()
{
   label_tempMin->setText(Brewtarget::displayAmount(minTemp_c(), Units::celsius));
}

void MashDesigner::updateMaxTemp()
{
   label_tempMax->setText(Brewtarget::displayAmount(maxTemp_c(), Units::celsius));
}

double MashDesigner::getSelectedAmount_l()
{
   double ratio = horizontalSlider_amount->value() / (double)(horizontalSlider_amount->maximum());
   double minAmt = minAmt_l();
   double maxAmt = maxAmt_l();
   double amt = minAmt + (maxAmt - minAmt)*ratio;

   return amt;
}

double MashDesigner::getSelectedTemp_c()
{
   double ratio = horizontalSlider_temp->value() / (double)(horizontalSlider_temp->maximum());
   double minT = minTemp_c();
   double maxT = maxTemp_c();
   double T = minT + (maxT - minT)*ratio;

   return T;
}

void MashDesigner::updateTempSlider()
{
   if( mashStep == 0 )
      return;

   if( mashStep->getType().compare("Infusion") == 0 )
   {
      double temp = tempFromVolume_c( getSelectedAmount_l() );

      double ratio = (temp-minTemp_c()) / (maxTemp_c() - minTemp_c());
      horizontalSlider_temp->setValue(ratio*horizontalSlider_temp->maximum());

      if( mashStep != 0 )
         mashStep->setInfuseTemp_c( temp );
   }
   else if( mashStep->getType().compare("Decoction") == 0 )
   {
      horizontalSlider_temp->setValue(horizontalSlider_temp->maximum());
   }
   else
      horizontalSlider_temp->setValue(0.5*horizontalSlider_temp->maximum());
}

void MashDesigner::updateAmtSlider()
{
   if( mashStep == 0 )
      return;

   if( mashStep->getType().compare("Infusion") == 0 )
   {
      double vol = volFromTemp_l( getSelectedTemp_c() );
      double ratio = (vol - minAmt_l()) / (maxAmt_l() - minAmt_l());

      horizontalSlider_amount->setValue(ratio*horizontalSlider_amount->maximum());
      if( mashStep != 0 )
         mashStep->setInfuseAmount_l(vol);
   }
   else
      horizontalSlider_amount->setValue(0.5*horizontalSlider_amount->maximum());
}

void MashDesigner::updateAmt()
{
   if( mashStep == 0 )
      return;

   if( mashStep->getType().compare("Infusion") == 0 )
   {
      double vol = horizontalSlider_amount->value() / (double)(horizontalSlider_amount->maximum())* (maxAmt_l() - minAmt_l()) + minAmt_l();

      label_amt->setText(Brewtarget::displayAmount( vol, Units::liters));

      if( mashStep != 0 )
         mashStep->setInfuseAmount_l( vol );
   }
   else if( mashStep->getType().compare("Decoction") == 0 )
      label_amt->setText(Brewtarget::displayAmount(mashStep->getDecoctionAmount_l(), Units::liters));
   else
      label_amt->setText(Brewtarget::displayAmount(0, Units::liters));
}

void MashDesigner::updateTemp()
{
   if( mashStep == 0 )
      return;

   if( mashStep->getType().compare("Infusion") == 0 )
   {
      double temp = horizontalSlider_temp->value() / (double)(horizontalSlider_temp->maximum()) * (maxTemp_c() - minTemp_c()) + minTemp_c();

      label_temp->setText(Brewtarget::displayAmount( temp, Units::celsius));

      if( mashStep != 0 )
         mashStep->setInfuseTemp_c( temp );
   }
   else if( mashStep->getType().compare("Decoction") == 0 )
      label_temp->setText(Brewtarget::displayAmount( maxTemp_c(), Units::celsius));
   else
      label_temp->setText(Brewtarget::displayAmount( mashStep->getStepTemp_c(), Units::celsius));
}

void MashDesigner::saveTargetTemp()
{
   double temp = Brewtarget::tempQStringToSI(lineEdit_temp->text());

   if( mashStep != 0 )
      mashStep->setStepTemp_c(temp);

   if( comboBox_type->currentText().compare("Decoction") == 0 )
   {
      if( mashStep != 0 )
         mashStep->setDecoctionAmount_l( getDecoctionAmount_l() );

      updateAmtSlider();
      updateAmt();
      updateTempSlider();
      updateTemp();
   }

   updateMinAmt();
   updateMaxAmt();
   updateMinTemp();
   updateMaxTemp();
   updateFullness();
}

double MashDesigner::getDecoctionAmount_l()
{
   double m_w, m_g, r;
   double c_w, c_g;
   double tf, t1;

   tf = mashStep->getStepTemp_c();
   if( prevStep == 0 )
   {
      QMessageBox::critical(this, tr("Decoction error"), tr("The first mash step cannot be a decoction."));
      Brewtarget::log(Brewtarget::ERROR, QString("MashDesigner: First step not a decoction."));
      return 0;
   }
   t1 = prevStep->getStepTemp_c();

   m_w = addedWater_l; // NOTE: this is bad. Assumes 1L = 1 kg.
   m_g = grain_kg;

   c_w = HeatCalculations::Cw_calGC;
   c_g = HeatCalculations::Cgrain_calGC;

   // r is the ratio of water and grain to take out for decoction.
   r = ((MC)*(tf-t1)) / ((m_w*c_w + m_g*c_g)*(maxTemp_c()-tf) + (m_w*c_w + m_g*c_g)*(tf-t1));
   if( r < 0 || r > 1 )
   {
      //QMessageBox::critical(this, tr("Decoction error"), tr("Something went wrong in decoction calculation.") );
      //Brewtarget::log(Brewtarget::ERROR, QString("MashDesigner Decoction: r=%1").arg(r));
      return 0;
   }

   return r*mashVolume_l();
}

bool MashDesigner::isBatchSparge()
{
   return (checkBox_batchSparge->checkState() == Qt::Checked);
}

void MashDesigner::typeChanged(QString type)
{
   if( mashStep != 0 )
      mashStep->setType(type.toStdString());

   if( type.compare("Infusion") == 0 )
   {
      horizontalSlider_amount->setEnabled(true);
      horizontalSlider_temp->setEnabled(true);
   }
   else if( type.compare("Decoction") == 0 )
   {
      horizontalSlider_amount->setEnabled(false);
      horizontalSlider_temp->setEnabled(false);

      if( mashStep != 0 )
         mashStep->setDecoctionAmount_l( getDecoctionAmount_l() );

      updateAmtSlider();
      updateAmt();
      updateTempSlider();
      updateTemp();
   }
   else if( type.compare("Temperature") == 0 )
   {
      horizontalSlider_amount->setEnabled(false);
      horizontalSlider_temp->setEnabled(false);
   }
}
