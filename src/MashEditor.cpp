/*
 * MashEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include "MashEditor.h"
#include <QWidget>
#include "mash.h"
#include "brewtarget.h"
#include "unit.h"
#include "equipment.h"

MashEditor::MashEditor(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
   rec = 0;

   connect(pushButton_fromEquipment, SIGNAL(clicked()), this, SLOT(fromEquipment()) );
   connect(this, SIGNAL(accepted()), this, SLOT(saveAndClose()) );
   connect(this, SIGNAL(rejected()), this, SLOT(closeEditor()) );
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
   Mash* mash;

   // Create a new mash if the recipe has none.
   if( rec == 0 )
      return;
   else if( rec->getMash() == 0 )
   {
      mash = new Mash();
      rec->setMash(mash);
   }
   else
   {
      mash = rec->getMash();
   }
   
   mash->disableNotification(); // If we don't do this, the notification will propagate to a showChanges() and we'll lose any info we want saved.
   mash->setEquipAdjust( true ); // BeerXML won't like me, but it's just stupid not to adjust for the equipment when you're able.

   mash->setName( lineEdit_name->text().toStdString() );
   mash->setGrainTemp_c(Unit::qstringToSI(lineEdit_grainTemp->text()));
   mash->setSpargeTemp_c(Unit::qstringToSI(lineEdit_spargeTemp->text()));
   mash->setPh(lineEdit_spargePh->text().toDouble());
   mash->setTunTemp_c(Unit::qstringToSI(lineEdit_tunTemp->text()));
   mash->setTunWeight_kg(Unit::qstringToSI(lineEdit_tunMass->text()));
   mash->setTunSpecificHeat_calGC(lineEdit_tunSpHeat->text().toDouble() );

   mash->setNotes( textEdit_notes->toPlainText().toStdString() );
   
   mash->reenableNotification();
   mash->forceNotify();
}

void MashEditor::fromEquipment()
{
   if( rec == 0 || rec->getEquipment() == 0 )
   {
      return;
   }

   Equipment* equip = rec->getEquipment();

   lineEdit_tunMass->setText(Brewtarget::displayAmount(equip->getTunWeight_kg(), Units::kilograms));
   lineEdit_tunSpHeat->setText(Brewtarget::displayAmount(equip->getTunSpecificHeat_calGC()));
}

void MashEditor::setRecipe(Recipe* recipe)
{
   rec = recipe;
   showChanges();
}

void MashEditor::showChanges()
{
   if( rec == 0 || rec->getMash() == 0 )
   {
      clear();
      return;
   }

   Mash* mash = rec->getMash();
   
   lineEdit_name->setText(mash->getName().c_str());
   lineEdit_grainTemp->setText(Brewtarget::displayAmount(mash->getGrainTemp_c(), Units::celsius));
   lineEdit_spargeTemp->setText(Brewtarget::displayAmount(mash->getSpargeTemp_c(), Units::celsius));
   lineEdit_spargePh->setText(Brewtarget::displayAmount(mash->getPh()));
   lineEdit_tunTemp->setText(Brewtarget::displayAmount(mash->getTunTemp_c(), Units::celsius));
   lineEdit_tunMass->setText(Brewtarget::displayAmount(mash->getTunWeight_kg(), Units::kilograms));
   lineEdit_tunSpHeat->setText(Brewtarget::displayAmount(mash->getTunSpecificHeat_calGC()));

   textEdit_notes->setPlainText(mash->getNotes().c_str());
}

void MashEditor::clear()
{
   lineEdit_name->setText("");
   lineEdit_grainTemp->setText("");
   lineEdit_spargeTemp->setText("");
   lineEdit_spargePh->setText("");
   lineEdit_tunTemp->setText("");
   lineEdit_tunMass->setText("");
   lineEdit_tunSpHeat->setText("");

   textEdit_notes->setPlainText("");
}