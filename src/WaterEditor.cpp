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

#include <QDebug>
#include <QInputDialog>

#include "WaterEditor.h"
#include "WaterSchema.h"
#include "TableSchemaConst.h"
#include "water.h"
#include "brewtarget.h"
#include "database.h"

WaterEditor::WaterEditor(QWidget *parent) : QDialog(parent)
{
   setupUi(this);
   obs = nullptr;

   connect( buttonBox, &QDialogButtonBox::accepted, this, &WaterEditor::saveAndClose);
   connect( buttonBox, &QDialogButtonBox::rejected, this, &WaterEditor::clearAndClose);
}

void WaterEditor::setWater(Water *water)
{
   if( obs )
      disconnect( obs, nullptr, this, nullptr );

   obs = water;
   if( obs )
   {
      connect( obs, &Ingredient::changed, this, &WaterEditor::changed );
      showChanges();
   }
}

void WaterEditor::newWater(QString folder)
{
   QString name = QInputDialog::getText(this, tr("Water name"),
                                              tr("Water name:"));
   if(name.isEmpty())
      return;

   Water* w = new Water(name);
   if ( ! folder.isEmpty() )
      w->setFolder(folder);

   setWater(w);
   setVisible(true);

}

void WaterEditor::showChanges(QMetaProperty* prop)
{
   if( obs == nullptr )
      return;

   QString propName;
   QVariant val;

   bool updateAll = false;

   if ( prop == nullptr ) {
      updateAll = true;
   }
   else {
      propName = prop->name();
      val = prop->read(obs);
   }

   if ( propName == kpropName || updateAll ) {
      lineEdit_name->setText(obs->name());
      if ( ! updateAll ) return;
   }
   if( propName == kpropCalcium || updateAll ) {
      lineEdit_ca->setText(obs->calcium_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == kpropMagnesium || updateAll ) {
      lineEdit_mg->setText(obs->magnesium_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == kpropSulfate || updateAll ){
      lineEdit_so4->setText(obs->sulfate_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == kpropSodium || updateAll ){
      lineEdit_na->setText(obs->sodium_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == kpropChloride || updateAll ){
      lineEdit_cl->setText(obs->chloride_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == kpropBiCarbonate || updateAll ){
      lineEdit_alk->setText(obs->bicarbonate_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == kpropPH || updateAll ){
      lineEdit_ph->setText(obs->ph(),2);
      if ( ! updateAll ) return;
   }
   if (propName == kpropAsHCO3 || updateAll ) {
      bool typeless = obs->alkalinityAsHCO3();
      comboBox_alk->setCurrentIndex(comboBox_alk->findText(typeless ? "HCO3" : "CO3"));
      if ( ! updateAll ) return;
   }
   if (propName == kpropNotes || updateAll ) {
      plainTextEdit_notes->setPlainText(obs->notes());
      if ( ! updateAll ) return;
   }

}

void WaterEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == obs )
      showChanges(&prop);
}

void WaterEditor::saveAndClose()
{
   if( obs == nullptr )
      return;

   obs->setName( lineEdit_name->text());
   obs->setAmount(0.0);
   obs->setBicarbonate_ppm( lineEdit_alk->toSI() );
   obs->setCalcium_ppm( lineEdit_ca->toSI() );
   obs->setMagnesium_ppm( lineEdit_mg->toSI() );
   obs->setSulfate_ppm( lineEdit_so4->toSI() );
   obs->setSodium_ppm( lineEdit_na->toSI() );
   obs->setChloride_ppm( lineEdit_cl->toSI() );
   obs->setPh( lineEdit_ph->toSI() );
   obs->setAlkalinity( lineEdit_alk->toSI());
   obs->setAlkalinityAsHCO3(comboBox_alk->currentText() == QString("HCO3"));
   obs->setNotes( plainTextEdit_notes->toPlainText());

   if ( obs->cacheOnly() ) {
      qDebug() << Q_FUNC_INFO << "writing " << obs->name();
      Database::instance().insertWater(obs);
   }

   setVisible(false);
}

void WaterEditor::clearAndClose()
{
   setWater(nullptr);
   setVisible(false); // Hide the window.
}
