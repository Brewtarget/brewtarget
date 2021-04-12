/*
 * WaterEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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

WaterEditor::WaterEditor(QWidget *parent) : QDialog(parent), obs{nullptr} {
   setupUi(this);

   connect( buttonBox, &QDialogButtonBox::accepted, this, &WaterEditor::saveAndClose);
   connect( buttonBox, &QDialogButtonBox::rejected, this, &WaterEditor::clearAndClose);

   this->waterEditRadarChart->init(
      tr("PPM"),
      50,
      {
         {PropertyNames::Water::calcium_ppm,     tr("Calcium")},
         {PropertyNames::Water::bicarbonate_ppm, tr("Bicarbonate")},
         {PropertyNames::Water::sulfate_ppm,     tr("Sulfate")},
         {PropertyNames::Water::chloride_ppm,    tr("Chloride")},
         {PropertyNames::Water::sodium_ppm,      tr("Sodium")},
         {PropertyNames::Water::magnesium_ppm,   tr("Magnesium")}
      }
   );

   return;
}

void WaterEditor::setWater(Water *water) {
   qDebug() << Q_FUNC_INFO;

   if (this->obs) {
      disconnect( this->obs, nullptr, this, nullptr );
   }

   this->obs = water;
   if (this->obs) {
      this->waterEditRadarChart->addSeries("Current", Qt::darkGreen, *water);

      connect( this->obs, &NamedEntity::changed, this, &WaterEditor::changed );
      showChanges();
   }

   return;
}

void WaterEditor::newWater(QString folder) {
   QString name = QInputDialog::getText(this, tr("Water name"),
                                              tr("Water name:"));
   if (name.isEmpty()) {
      return;
   }

   qDebug() << Q_FUNC_INFO << "Creating new Water, " << name;

   Water* w = new Water(name);
   if ( ! folder.isEmpty() ) {
      w->setFolder(folder);
   }

   setWater(w);
   setVisible(true);

   return;
}

void WaterEditor::showChanges(QMetaProperty* prop)
{

   if (this->obs == nullptr) {
      return;
   }

   QString propName;
   QVariant val;

   bool updateAll = false;

   if ( prop == nullptr ) {
      updateAll = true;
   }
   else {
      propName = prop->name();
      val = prop->read(this->obs);
   }

   if ( propName == PropertyNames::NamedEntity::name || updateAll ) {
      lineEdit_name->setText(this->obs->name());
      if ( ! updateAll ) return;
   }
   if( propName == PropertyNames::Water::calcium_ppm || updateAll ) {
      lineEdit_ca->setText(this->obs->calcium_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == PropertyNames::Water::magnesium_ppm || updateAll ) {
      lineEdit_mg->setText(this->obs->magnesium_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == PropertyNames::Water::sulfate_ppm || updateAll ){
      lineEdit_so4->setText(this->obs->sulfate_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == PropertyNames::Water::sodium_ppm || updateAll ){
      lineEdit_na->setText(this->obs->sodium_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == PropertyNames::Water::chloride_ppm || updateAll ){
      lineEdit_cl->setText(this->obs->chloride_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == PropertyNames::Water::bicarbonate_ppm || updateAll ){
      lineEdit_alk->setText(this->obs->bicarbonate_ppm(),2);
      if ( ! updateAll ) return;
   }
   if( propName == PropertyNames::Water::ph || updateAll ){
      lineEdit_ph->setText(this->obs->ph(),2);
      if ( ! updateAll ) return;
   }
   if (propName == PropertyNames::Water::alkalinityAsHCO3 || updateAll ) {
      bool typeless = this->obs->alkalinityAsHCO3();
      comboBox_alk->setCurrentIndex(comboBox_alk->findText(typeless ? "HCO3" : "CaCO3"));
      if ( ! updateAll ) return;
   }
   if (propName == PropertyNames::Water::notes || updateAll ) {
      plainTextEdit_notes->setPlainText(obs->notes());
      if ( ! updateAll ) return;
   }

   return;
}

void WaterEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if ( sender() == this->obs ) {
      showChanges(&prop);
   }

   this->waterEditRadarChart->replot();
   return;
}

void WaterEditor::saveAndClose()
{
   if (this->obs == nullptr) {
      return;
   }

   this->obs->setName( lineEdit_name->text());
   this->obs->setAmount(0.0);
   this->obs->setBicarbonate_ppm( lineEdit_alk->toSI() );
   this->obs->setCalcium_ppm( lineEdit_ca->toSI() );
   this->obs->setMagnesium_ppm( lineEdit_mg->toSI() );
   this->obs->setSulfate_ppm( lineEdit_so4->toSI() );
   this->obs->setSodium_ppm( lineEdit_na->toSI() );
   this->obs->setChloride_ppm( lineEdit_cl->toSI() );
   this->obs->setPh( lineEdit_ph->toSI() );
   this->obs->setAlkalinity( lineEdit_alk->toSI());
   this->obs->setAlkalinityAsHCO3(comboBox_alk->currentText() == QString("HCO3"));
   this->obs->setNotes( plainTextEdit_notes->toPlainText());

   if (this->obs->cacheOnly()) {
      qDebug() << Q_FUNC_INFO << "writing " << this->obs->name();
      this->obs->insertInDatabase();
   }

   setVisible(false);
   return;
}

void WaterEditor::clearAndClose()
{
   qDebug() << Q_FUNC_INFO;
   setWater(nullptr);
   setVisible(false); // Hide the window.
   return;
}
