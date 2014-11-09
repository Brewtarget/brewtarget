/*
 * MashDesigner.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Dan Cavanagh <dan@dancavanagh.com>
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

#include "database.h"
#include "MashDesigner.h"
#include "equipment.h"
#include "mash.h"
#include "mashstep.h"
#include "brewtarget.h"
#include "HeatCalculations.h"
#include "PhysicalConstants.h"
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
   connect( comboBox_type, SIGNAL(activated(int)), this, SLOT(typeChanged(int)) );

   connect( checkBox_batchSparge, SIGNAL(clicked()), this, SLOT(updateMaxAmt()) );
   connect( pushButton_finish, SIGNAL(clicked()), this, SLOT(saveAndClose()) );

   connect( lineEdit_time, SIGNAL(editingFinished()), this, SLOT( updateField()));
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
      MC += mashStep->infuseAmount_l() * HeatCalculations::Cw_calGC;
      addedWater_l += mashStep->infuseAmount_l();

      if( prevStep == 0 ) // If the last step is null, we need to add the influence of the tun.
         MC += mash->tunSpecificHeat_calGC() * mash->tunWeight_kg();
   }

   prevStep = mashStep;
   mashStep = (step>=0 && step<mash->mashSteps().size())? mash->mashSteps()[step] : 0;
   if( mashStep == 0 )
   {
      mashStep = Database::instance().newMashStep( mash ); // TODO: Come back to check on this later. Really need this new step to be inserted in right place.
   }

   // Clear out some of the fields.
   lineEdit_name->clear();
   lineEdit_temp->clear();
   lineEdit_time->clear();

   horizontalSlider_amount->setValue(0); // Least amount of water.

   return true;
}

void MashDesigner::saveStep()
{
   MashStep::Type type = static_cast<MashStep::Type>(comboBox_type->currentIndex());

   //mashStep->disableNotification();
   mashStep->setName( lineEdit_name->text() );
   mashStep->setType( type );
   mashStep->setStepTemp_c( Brewtarget::tempQStringToSI(lineEdit_temp->text()) );
   mashStep->setStepTime_min( Brewtarget::timeQStringToSI(lineEdit_time->text()) );

   if( type == MashStep::Infusion)
   {
      mashStep->setInfuseAmount_l( getSelectedAmount_l() );
      mashStep->setInfuseTemp_c( getSelectedTemp_c() );
   }

   //mashStep->reenableNotification();
   //mashStep->forceNotify();
}

double MashDesigner::maxTemp_c()
{
   if ( recObs && recObs->equipment())
   {
      return recObs->equipment()->boilingPoint_c();
   }
   else
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
   return grain_kg/PhysicalConstants::grainDensity_kgL + addedWater_l;
}

double MashDesigner::minAmt_l()
{
   // Minimum amount occurs with maximum temperature.
   return volFromTemp_l( maxTemp_c() );
}

// However much more we can add at this step.
double MashDesigner::maxAmt_l()
{
   // However much more we can fit in the tun.
   if( ! isBatchSparge() )
      return (equip==0)? 0 : equip->tunVolume_l() - mashVolume_l();
   else
      return (equip == 0)? 0 : equip->tunVolume_l() - grainVolume_l();
}

// Returns the required volume of water to infuse if the strike water is
// at temp_c degrees Celsius.
double MashDesigner::volFromTemp_l( double temp_c )
{
   if( mashStep == 0 || mash == 0 )
      return 0.0;

   double tw = temp_c;
   // Final temp is target temp.
   double tf = mashStep->stepTemp_c();
   // Initial temp is the last step's temp if the last step exists, otherwise the grain temp.
   double t1 = (prevStep==0)? mash->grainTemp_c() : prevStep->stepTemp_c();
   double mt = mash->tunSpecificHeat_calGC();
   double ct = mash->tunWeight_kg();

   double mw = 1/(HeatCalculations::Cw_calGC * (tw-tf)) * (MC*(tf-t1) + ((prevStep==0)? mt*ct*(tf-mash->tunTemp_c()) : 0) );

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
      absorption_LKg = equip->grainAbsorption_LKg();
   else
      absorption_LKg = PhysicalConstants::grainAbsorption_Lkg;

   double tf = mashStep->stepTemp_c();
   // NOTE: This needs to be changed. Assumes 1L = 1 kg.
   double mw = vol_l;
   if( mw <= 0 )
      return 0.0;
   double cw = HeatCalculations::Cw_calGC;
   // Initial temp is the last step's temp if the last step exists, otherwise the grain temp.
   double t1 = (prevStep==0)? mash->grainTemp_c() : prevStep->stepTemp_c();
   // When batch sparging, you lose about 10C from previous step.
   if( isBatchSparge() )
      t1 = (prevStep==0)? mash->grainTemp_c() : prevStep->stepTemp_c() - 10;
   double mt = mash->tunSpecificHeat_calGC();
   double ct = mash->tunWeight_kg();

   double batchMC = grain_kg * HeatCalculations::Cgrain_calGC
                    + absorption_LKg * grain_kg * HeatCalculations::Cw_calGC
                    + mash->tunWeight_kg() * mash->tunSpecificHeat_calGC();

   double tw = 1/(mw*cw) * ( (isBatchSparge()? batchMC : MC) * (tf-t1) + ((prevStep==0)? mt*ct*(tf-mash->tunTemp_c()) : 0) ) + tf;

   return tw;
}

// How many liters of grain are in the tun.
double MashDesigner::grainVolume_l()
{
   return grain_kg / PhysicalConstants::grainDensity_kgL;
}

// After this, mash and equip are non-null iff we return true.
bool MashDesigner::initializeMash()
{
   if(recObs == 0)
      return false;

   mash = recObs->mash();
   if( mash == 0 )
      mash = Database::instance().newMash( recObs );

   equip = recObs->equipment();
   if( equip == 0 )
   {
      QMessageBox::warning(this, tr("No Equipment"), tr("You have not set an equipment for this recipe. We really cannot continue without one."));
      return false;
   }

   mash->setTunSpecificHeat_calGC( equip->tunSpecificHeat_calGC() );
   mash->setTunWeight_kg( equip->tunWeight_kg() );
   
   bool ok;
   QString dialogText = QInputDialog::getText(
                                        this,
                                        tr("Tun Temp"),
                                        tr("Enter the temperature of the tun before your first infusion."),
                                        QLineEdit::Normal, //default,
                                        QString::null,
                                        &ok
                                       //don't need the widget pointer - default is parent
                                              );
   
   //if user hits cancel, cancel out of the dialog and quit the mashDesigner
   //edited jazzbeerman (dcavanagh) 8/20/10
   if(!ok)
      return false;
   
   //otherwise continue - get the text and keep going
   mash->setTunTemp_c( Brewtarget::tempQStringToSI( dialogText ) );

   mash->removeAllMashSteps();
   curStep = 0;
   MC = recObs->grainsInMash_kg() * HeatCalculations::Cgrain_calGC;
   addedWater_l = 0;
   mashStep = 0;
   prevStep = 0;
   grain_kg = recObs->grainsInMash_kg();

   label_tunVol->setText(Brewtarget::displayAmount(equip->tunVolume_l(), Units::liters));
   label_wortMax->setText(Brewtarget::displayAmount(recObs->boilSize_l(), Units::liters));
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

   MashStep::Type type = mashStep->type();

   if( equip == 0 )
   {
      progressBar_fullness->setValue(0);
      return;
   }

   double vol_l;
   if( ! isBatchSparge() )
      vol_l = mashVolume_l() + ( (type==MashStep::Infusion) ? getSelectedAmount_l() : 0);
   else
      vol_l = grainVolume_l() + getSelectedAmount_l();
   double ratio = vol_l / equip->tunVolume_l();
   if( ratio < 0 )
     ratio = 0;
   if( ratio > 1 )
     ratio = 1;

   progressBar_fullness->setValue(ratio*progressBar_fullness->maximum());
   label_mashVol->setText(Brewtarget::displayAmount(vol_l, Units::liters));
   label_thickness->setText(Brewtarget::displayThickness( (addedWater_l + ((type==MashStep::Infusion) ? getSelectedAmount_l() : 0) )/grain_kg ));
}

void MashDesigner::updateCollectedWort()
{
   if( recObs == 0 )
      return;

   double wort_l = recObs->wortFromMash_l();
   double ratio = wort_l / recObs->boilSize_l();
   if( ratio < 0 )
     ratio = 0;
   if( ratio > 1 )
     ratio = 1;

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

   if( mashStep->type() == MashStep::Infusion )
   {
      double temp = tempFromVolume_c( getSelectedAmount_l() );

      double ratio = (temp-minTemp_c()) / (maxTemp_c() - minTemp_c());
      horizontalSlider_temp->setValue(ratio*horizontalSlider_temp->maximum());

      if( mashStep != 0 )
         mashStep->setInfuseTemp_c( temp );
   }
   else if( mashStep->type() == MashStep::Decoction )
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

   if( mashStep->type() == MashStep::Infusion )
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

   if( mashStep->type() == MashStep::Infusion )
   {
      double vol = horizontalSlider_amount->value() / (double)(horizontalSlider_amount->maximum())* (maxAmt_l() - minAmt_l()) + minAmt_l();

      label_amt->setText(Brewtarget::displayAmount( vol, Units::liters));

      if( mashStep != 0 )
         mashStep->setInfuseAmount_l( vol );
   }
   else if( mashStep->type() == MashStep::Decoction )
      label_amt->setText(Brewtarget::displayAmount(mashStep->decoctionAmount_l(), Units::liters));
   else
      label_amt->setText(Brewtarget::displayAmount(0, Units::liters));
}

void MashDesigner::updateTemp()
{
   if( mashStep == 0 )
      return;

   if( mashStep->type() == MashStep::Infusion )
   {
      double temp = horizontalSlider_temp->value() / (double)(horizontalSlider_temp->maximum()) * (maxTemp_c() - minTemp_c()) + minTemp_c();

      label_temp->setText(Brewtarget::displayAmount( temp, Units::celsius));

      if( mashStep != 0 )
         mashStep->setInfuseTemp_c( temp );
   }
   else if( mashStep->type() == MashStep::Decoction )
      label_temp->setText(Brewtarget::displayAmount( maxTemp_c(), Units::celsius));
   else
      label_temp->setText(Brewtarget::displayAmount( mashStep->stepTemp_c(), Units::celsius));
}

void MashDesigner::saveTargetTemp()
{
   double temp = Brewtarget::tempQStringToSI(lineEdit_temp->text());

   // be nice and reset the field so it displays in proper units
   lineEdit_temp->setText( Brewtarget::displayAmount(temp, Units::celsius));
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
   updateCollectedWort();
}

double MashDesigner::getDecoctionAmount_l()
{
   double m_w, m_g, r;
   double c_w, c_g;
   double tf, t1;

   tf = mashStep->stepTemp_c();
   if( prevStep == 0 )
   {
      QMessageBox::critical(this, tr("Decoction error"), tr("The first mash step cannot be a decoction."));
      Brewtarget::logE(QString("MashDesigner: First step not a decoction."));
      return 0;
   }
   t1 = prevStep->stepTemp_c();

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

void MashDesigner::typeChanged(int t)
{
   MashStep::Type type = static_cast<MashStep::Type>(t);

   if( mashStep != 0 )
      mashStep->setType(type);

   if( type == MashStep::Infusion )
   {
      horizontalSlider_amount->setEnabled(true);
      horizontalSlider_temp->setEnabled(true);
   }
   else if( type == MashStep::Decoction )
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
   else if( type == MashStep::Temperature )
   {
      horizontalSlider_amount->setEnabled(false);
      horizontalSlider_temp->setEnabled(false);
   }
}

void MashDesigner::updateField()
{

   QObject* selection = sender();
   QLineEdit* field = qobject_cast<QLineEdit*>(selection);
   double val;

   // Only the time gets abused, but I'm being consistent  
   if ( field == lineEdit_time ) 
   {
      val = Brewtarget::timeQStringToSI(field->text());
      field->setText(Brewtarget::displayAmount( val, Units::minutes));
   }

}

