/*
 * WaterEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#include "WaterEditor.h"
#include "brewtarget.h"
WaterEditor::WaterEditor(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    obs = 0;
}

void WaterEditor::setWater(Water *water)
{
   obs = water;
   setObserved(water);

   showChanges();
}

void WaterEditor::showChanges()
{
   if( obs == 0 )
      return;

   lineEdit_ca->setText(Brewtarget::displayAmount(obs->getCalcium_ppm(),0,0));
   lineEdit_mg->setText(Brewtarget::displayAmount(obs->getMagnesium_ppm(),0,0));
   lineEdit_so4->setText(Brewtarget::displayAmount(obs->getSulfate_ppm(),0,0));
   lineEdit_na->setText(Brewtarget::displayAmount(obs->getSodium_ppm(),0,0));
   lineEdit_cl->setText(Brewtarget::displayAmount(obs->getChloride_ppm(),0,0));
   lineEdit_alk->setText(Brewtarget::displayAmount(obs->getBicarbonate_ppm(),0,0));
   lineEdit_ph->setText(Brewtarget::displayAmount(obs->getPh(),0,1));

   // Make sure the combo box is showing bicarbonate.
   comboBox_alk->setCurrentIndex( comboBox_alk->findText("HCO3") );
}

void WaterEditor::notify(Observable *notifier, QVariant info)
{
   // Do nothing.
}

void WaterEditor::saveAndClose()
{
   if( obs == 0 )
      return;

   obs->setCalcium_ppm( lineEdit_ca->text().toDouble() );
   obs->setMagnesium_ppm( lineEdit_mg->text().toDouble() );
   obs->setSulfate_ppm( lineEdit_so4->text().toDouble() );
   obs->setSodium_ppm( lineEdit_na->text().toDouble() );
   obs->setChloride_ppm( lineEdit_cl->text().toDouble() );
   obs->setPh( lineEdit_ph->text().toDouble() );

   // Might need to convert alkalinity as CaCO3 to HCO3
   if( comboBox_alk->currentText() == QString("CaCO3") )
      obs->setBicarbonate_ppm(1.22 * lineEdit_alk->text().toDouble());
   else
      obs->setBicarbonate_ppm(lineEdit_alk->text().toDouble());

   setVisible(false);
}
