/*
 * MashWizard.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
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

#ifndef _MASHWIZARD_H
#define _MASHWIZARD_H

class MashWizard;

#include "ui_mashWizard.h"
#include <QDialog>
#include "recipe.h"
#include "unit.h"

/*!
 * \class MashWizard
 * \author Philip G. Lee
 *
 * \brief View/controller dialog that helps you design a mash.
 */
class MashWizard : public QDialog, public Ui::mashWizard
{
   Q_OBJECT
public:
   MashWizard(QWidget* parent=nullptr);
   //! Set the recipe to do mash wizardry on.
   void setRecipe(Recipe* rec);

public slots:
   void wizardry(); // Do what the wizard is supposed to do.
   void show();
   void toggleSpinBox(QAbstractButton* button);

private:
   Recipe* recObs;
   Unit *weightUnit;
   Unit *volumeUnit;

   //!brief just need a holder for the three buttons
   QButtonGroup* bGroup;

   //!brief helper method to calculate the volume of a decocation step
   double calcDecoctionAmount( MashStep* step, Mash* mash, double waterMass, double grainMass, double lastTemp, double boiling);

};

#endif   /* _MASHWIZARD_H */
