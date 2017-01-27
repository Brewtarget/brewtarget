/*
 * WaterEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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
      connect( obs, &BeerXMLElement::changed, this, &WaterEditor::changed );
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
      lineEdit_ca->setText(val);
   else if( propName == "magnesium_ppm" || updateAll )
      lineEdit_mg->setText(val);
   else if( propName == "sulfate_ppm" || updateAll )
      lineEdit_so4->setText(val);
   else if( propName == "sodium_ppm" || updateAll )
      lineEdit_na->setText(val);
   else if( propName == "chloride_ppm" || updateAll )
      lineEdit_cl->setText(val);
   else if( propName == "bicarbonate_ppm" || updateAll )
      lineEdit_alk->setText(val);
   else if( propName == "ph" || updateAll )
      lineEdit_ph->setText(val);

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

   obs->setCalcium_ppm( lineEdit_ca->toSI() );
   obs->setMagnesium_ppm( lineEdit_mg->toSI() );
   obs->setSulfate_ppm( lineEdit_so4->toSI() );
   obs->setSodium_ppm( lineEdit_na->toSI() );
   obs->setChloride_ppm( lineEdit_cl->toSI() );
   obs->setPh( lineEdit_ph->toSI() );

   // Might need to convert alkalinity as CaCO3 to HCO3
   if( comboBox_alk->currentText() == QString("CaCO3") )
      obs->setBicarbonate_ppm(1.22 * lineEdit_alk->toSI() );
   else
      obs->setBicarbonate_ppm(lineEdit_alk->toSI() );

   setVisible(false);
}
