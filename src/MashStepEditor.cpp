/*
 * MashStepEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "unit.h"
#include "brewtarget.h"
#include "MashStepEditor.h"
#include "mashstep.h"

MashStepEditor::MashStepEditor(QWidget* parent)
   : QDialog(parent), obs(0)
{
   setupUi(this);

   comboBox_type->setCurrentIndex(-1);

   connect( buttonBox, SIGNAL( accepted() ), this, SLOT(saveAndClose()) );
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT(close()) );
   connect( comboBox_type, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(grayOutStuff(const QString &)) );
}

void MashStepEditor::showChanges(QMetaProperty* metaProp)
{
   if( obs == 0 )
   {
      clear();
      return;
   }

   QString propName;
   QVariant value;
   bool updateAll = false;
   if( metaProp == 0 )
      updateAll = true;
   else
   {
      propName = metaProp->name();
      value = metaProp->read(obs);
   }

   if( propName == "name" || updateAll )
      lineEdit_name->setText(obs->name());
   else if( propName == "type" || updateAll )
      comboBox_type->setCurrentIndex(obs->type());
   else if( propName == "infuseAmount_l" || updateAll )
      lineEdit_infuseAmount->setText(Brewtarget::displayAmount(obs->infuseAmount_l(), Units::liters));
   else if( propName == "infuseTemp_c" || updateAll )
      lineEdit_infuseTemp->setText(Brewtarget::displayAmount(obs->infuseTemp_c(), Units::celsius));
   else if( propName == "decoctionAmount_l" || updateAll )
      lineEdit_decoctionAmount->setText(Brewtarget::displayAmount(obs->decoctionAmount_l(), Units::liters));
   else if( propName == "stepTemp_c" || updateAll )
      lineEdit_stepTemp->setText(Brewtarget::displayAmount(obs->stepTemp_c(), Units::celsius));
   else if( propName == "stepTime_min" || updateAll )
      lineEdit_stepTime->setText(Brewtarget::displayAmount(obs->stepTime_min(), Units::minutes));
   else if( propName == "rampTime_min" || updateAll )
      lineEdit_rampTime->setText(Brewtarget::displayAmount(obs->rampTime_min(), Units::minutes));
   else if( propName == "endTemp_c" || updateAll )
      lineEdit_endTemp->setText(Brewtarget::displayAmount(obs->endTemp_c(), Units::celsius));
}

void MashStepEditor::clear()
{
   lineEdit_name->setText("");
   comboBox_type->setCurrentIndex(0);
   lineEdit_infuseAmount->setText("");
   lineEdit_infuseTemp->setText("");
   lineEdit_decoctionAmount->setText("");
   lineEdit_stepTemp->setText("");
   lineEdit_stepTime->setText("");
   lineEdit_rampTime->setText("");
   lineEdit_endTemp->setText("");
}

void MashStepEditor::close()
{
   setVisible(false);
}

void MashStepEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() != obs )
      return;

   showChanges(&prop);
}

void MashStepEditor::setMashStep(MashStep* step)
{
   if( obs )
      disconnect( obs, 0, this, 0 );
   
   if( step )
   {
      obs = step;
      connect( obs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void MashStepEditor::saveAndClose()
{
   // TODO: check this out with 1.2.5.
   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   //obs->disableNotification();

   obs->setName(lineEdit_name->text());
   obs->setType(static_cast<MashStep::Type>(comboBox_type->currentIndex()));
   obs->setInfuseAmount_l(Brewtarget::volQStringToSI(lineEdit_infuseAmount->text()));
   obs->setInfuseTemp_c(Brewtarget::tempQStringToSI(lineEdit_infuseTemp->text()));
   obs->setDecoctionAmount_l(Brewtarget::volQStringToSI(lineEdit_decoctionAmount->text()));
   obs->setStepTemp_c(Brewtarget::tempQStringToSI(lineEdit_stepTemp->text()));
   obs->setStepTime_min(Brewtarget::timeQStringToSI(lineEdit_stepTime->text()));
   obs->setRampTime_min(Brewtarget::timeQStringToSI(lineEdit_rampTime->text()));
   obs->setEndTemp_c(Brewtarget::tempQStringToSI(lineEdit_endTemp->text()));

   //obs->reenableNotification();
   //obs->forceNotify();

   setVisible(false);
}

void MashStepEditor::grayOutStuff(const QString& text)
{
   if( text == "Infusion" )
   {
      lineEdit_infuseAmount->setEnabled(true);
      lineEdit_infuseTemp->setEnabled(true);
      lineEdit_decoctionAmount->setEnabled(false);
   }
   else if( text == "Decoction" )
   {
      lineEdit_infuseAmount->setEnabled(false);
      lineEdit_infuseTemp->setEnabled(false);
      lineEdit_decoctionAmount->setEnabled(true);
   }
   else if( text == "Temperature" )
   {
      lineEdit_infuseAmount->setEnabled(false);
      lineEdit_infuseTemp->setEnabled(false);
      lineEdit_decoctionAmount->setEnabled(false);
   }
   else
   {
      lineEdit_infuseAmount->setEnabled(true);
      lineEdit_infuseTemp->setEnabled(true);
      lineEdit_decoctionAmount->setEnabled(true);
   }
}
