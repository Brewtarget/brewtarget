/*
 * MashDesigner.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#ifndef MASHDESIGNER_H
#define MASHDESIGNER_H

class MashDesigner;

#include "ui_mashDesigner.h"
#include "recipe.h"
#include "mash.h"
#include "mashstep.h"
#include "equipment.h"
#include <QDialog>
#include <QWidget>

/*!
 * \class MashDesigner
 * \author Philip G. Lee
 *
 * \brief View/controller dialog that gives you more control over mash design than MashWizard does.
 */
class MashDesigner : public QDialog, public Ui::mashDesigner
{
   Q_OBJECT
public:
   MashDesigner(QWidget* parent = nullptr);
   //! Set the recipe we want to design the mash for.
   void setRecipe(Recipe* rec);

public slots:
   void show();

private slots:
   void updateFullness();
   void updateCollectedWort();
   void updateMinAmt();
   void updateMaxAmt();
   void updateMinTemp();
   void updateMaxTemp();
   void updateAmt();
   void updateAmtSlider();
   void updateTemp();
   void updateTempSlider();
   void saveTargetTemp();
   void proceed(); // Go to next step.
   void saveAndClose();
   void typeChanged();

private:
   bool nextStep(int step);
   void saveStep();
   bool initializeMash();
   double minTemp_c();
   double maxTemp_c();
   double minAmt_l();
   double maxAmt_l();
   double mashVolume_l();
   double grainVolume_l();
   double tempFromVolume_c( double vol_l );
   double volFromTemp_l( double temp_c );
   double getDecoctionAmount_l();
   double waterFromMash_l();
   bool heating();
   double boilingTemp_c();
   double bound_temp_c(double temp_c);

   // I have developed a distaste for "getBlah"
   double selectedAmount_l();
   double selectedTemp_c();
   double stepTemp_c();

   MashStep::Type type() const;
   bool isBatchSparge() const;
   bool isFlySparge() const;
   bool isSparge() const;
   bool isInfusion() const;
   bool isTemperature() const;
   bool isDecoction() const;


   Recipe* recObs;
   Mash* mash;
   Equipment* equip;
   MashStep* mashStep;
   MashStep* prevStep;
   double addedWater_l;
   double grain_kg;
   double MC;
   int curStep;
};

#endif // MASHDESIGNER_H
