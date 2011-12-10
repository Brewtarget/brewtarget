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
#include "water.h"
#include "brewtarget.h"

WaterEditor::WaterEditor(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    obs = 0;
}

void WaterEditor::setWater(Water *water)
{
   if( obs )
      disconnect( obs, 0, this, 0 );
   
   obs = water;
   if( obs )
   {
      connect( obs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void WaterEditor::showChanges(QMetaProperty* prop)
{
   if( obs == 0 )
      return;

   QString propName;
   QVariant val;
   
   bool updateAll = (prop == 0);
   if( prop )
   {
      propName = prop->name();
      val = prop->read(obs);
   }
   
   if( propName == "calcium_ppm" || updateAll )
      lineEdit_ca->setText(Brewtarget::displayAmount(val.toDouble(),0,0));
   else if( propName == "magnesium_ppm" || updateAll )
      lineEdit_mg->setText(Brewtarget::displayAmount(val.toDouble(),0,0));
   else if( propName == "sulfate_ppm" || updateAll )
      lineEdit_so4->setText(Brewtarget::displayAmount(val.toDouble(),0,0));
   else if( propName == "sodium_ppm" || updateAll )
      lineEdit_na->setText(Brewtarget::displayAmount(val.toDouble(),0,0));
   else if( propName == "chloride_ppm" || updateAll )
      lineEdit_cl->setText(Brewtarget::displayAmount(val.toDouble(),0,0));
   else if( propName == "bicarbonate_ppm" || updateAll )
      lineEdit_alk->setText(Brewtarget::displayAmount(val.toDouble(),0,0));
   else if( propName == "ph" || updateAll )
      lineEdit_ph->setText(Brewtarget::displayAmount(val.toDouble(),0,1));

   // Make sure the combo box is showing bicarbonate.
   comboBox_alk->setCurrentIndex( comboBox_alk->findText("HCO3") );
}

void WaterEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == obs )
      showChanges(&prop);
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
