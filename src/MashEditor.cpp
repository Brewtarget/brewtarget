/*
 * MashEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Kregg K <gigatropolis@yahoo.com>
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

#include "MashEditor.h"
#include <QWidget>
#include <QDebug>
#include "mash.h"
#include "brewtarget.h"
#include "unit.h"
#include "equipment.h"
#include "recipe.h"

MashEditor::MashEditor(QWidget* parent) : QDialog(parent), mashObs(0)
{
   setupUi(this);

   connect(pushButton_fromEquipment, &QAbstractButton::clicked, this, &MashEditor::fromEquipment );
   connect(this, &QDialog::accepted, this, &MashEditor::saveAndClose );
   connect(this, &QDialog::rejected, this, &MashEditor::closeEditor );

}

void MashEditor::showEditor()
{
   showChanges();
   setVisible(true);
}

void MashEditor::closeEditor()
{
   setVisible(false);
}

void MashEditor::saveAndClose()
{
   if( mashObs == 0 )
      return;
   
   mashObs->setEquipAdjust( true ); // BeerXML won't like me, but it's just stupid not to adjust for the equipment when you're able.

   mashObs->setName( lineEdit_name->text() );
   mashObs->setGrainTemp_c(lineEdit_grainTemp->toSI());
   mashObs->setSpargeTemp_c(lineEdit_spargeTemp->toSI());
   mashObs->setPh(lineEdit_spargePh->toSI());
   mashObs->setTunTemp_c(lineEdit_tunTemp->toSI());
   mashObs->setTunWeight_kg(lineEdit_tunMass->toSI());
   mashObs->setTunSpecificHeat_calGC(lineEdit_tunSpHeat->toSI());

   mashObs->setNotes( textEdit_notes->toPlainText() );
   
}

void MashEditor::fromEquipment()
{
   if( mashObs == 0 )
      return;

   if ( equip == 0 )
      return;

   lineEdit_tunMass->setText(equip);
   lineEdit_tunSpHeat->setText(equip);
}

void MashEditor::setMash(Mash* mash)
{
   if( mashObs )
      disconnect( mashObs, 0, this, 0 );
   
   mashObs = mash;
   if( mashObs )
   {
      connect( mashObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void MashEditor::setEquipment(Equipment* e)
{
   if ( ! e )
      return;

   equip = e;
   if( mashObs )
   {
      // Only do this if we have to. Otherwise, it causes some uneccesary
      // updates to the database.
      if ( mashObs->tunWeight_kg() != e->tunWeight_kg() )
         mashObs->setTunWeight_kg( e->tunWeight_kg() );
      if ( mashObs->tunSpecificHeat_calGC() != e->tunSpecificHeat_calGC() )
         mashObs->setTunSpecificHeat_calGC( e->tunSpecificHeat_calGC() );
   }
}

void MashEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == mashObs )
      showChanges(&prop);
}

void MashEditor::showChanges(QMetaProperty* prop)
{
   bool updateAll = false;
   QString propName;

   if( mashObs == 0 )
   {
      clear();
      return;
   }

   if( prop == 0 )
      updateAll = true;
   else
      propName = prop->name();

   if( propName == "name" || updateAll ) {
      lineEdit_name->setText(mashObs->name());
      if( ! updateAll )
         return;
   }
   if( propName == "grainTemp_c" || updateAll ) {
      lineEdit_grainTemp->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == "spargeTemp_c" || updateAll ) {
      lineEdit_spargeTemp->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == "ph" || updateAll ) {
      lineEdit_spargePh->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == "tunTemp_c" || updateAll ) {
      lineEdit_tunTemp->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == "tunMass_kg" || updateAll ) {
      lineEdit_tunMass->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == "tunSpecificHeat_calGC" || updateAll ) {
      lineEdit_tunSpHeat->setText(mashObs);
      if( ! updateAll )
         return;
   }
   if( propName == "notes" || updateAll ) {
      textEdit_notes->setPlainText(mashObs->notes());
      if( ! updateAll )
         return;
   }
}

void MashEditor::clear()
{
   lineEdit_name->setText(QString(""));
   lineEdit_grainTemp->setText(QString(""));
   lineEdit_spargeTemp->setText(QString(""));
   lineEdit_spargePh->setText(QString(""));
   lineEdit_tunTemp->setText(QString(""));
   lineEdit_tunMass->setText(QString(""));
   lineEdit_tunSpHeat->setText(QString(""));

   textEdit_notes->setPlainText(QString(""));
}
