/*
 * MashStepEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "unit.h"
#include "brewtarget.h"
#include "MashStepEditor.h"

MashStepEditor::MashStepEditor(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
   obs = 0;

   comboBox_type->setCurrentIndex(-1);

   connect( buttonBox, SIGNAL( accepted() ), this, SLOT(saveAndClose()) );
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT(close()) );
   connect( comboBox_type, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(grayOutStuff(const QString &)) );
}

void MashStepEditor::showChanges()
{
   int tmp;
   if( obs == 0 )
   {
      clear();
      return;
   }

   lineEdit_name->setText(obs->getName().c_str());
   tmp = comboBox_type->findText(obs->getType().c_str());
   comboBox_type->setCurrentIndex(tmp);
   lineEdit_infuseAmount->setText(Brewtarget::displayAmount(obs->getInfuseAmount_l(), Units::liters));
   lineEdit_infuseTemp->setText(Brewtarget::displayAmount(obs->getInfuseTemp_c(), Units::celsius));
   lineEdit_decoctionAmount->setText(Brewtarget::displayAmount(obs->getDecoctionAmount_l(), Units::liters));
   lineEdit_stepTemp->setText(Brewtarget::displayAmount(obs->getStepTemp_c(), Units::celsius));
   lineEdit_stepTime->setText(Brewtarget::displayAmount(obs->getStepTime_min(), Units::minutes));
   lineEdit_rampTime->setText(Brewtarget::displayAmount(obs->getRampTime_min(), Units::minutes));
   lineEdit_endTemp->setText(Brewtarget::displayAmount(obs->getEndTemp_c(), Units::celsius));
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

void MashStepEditor::notify(Observable* notifier, QVariant /*info*/)
{
   if( notifier != obs )
      return;

   showChanges();
}

void MashStepEditor::setMashStep(MashStep* step)
{
   setObserved(step);
   obs = step;
   showChanges();
}

void MashStepEditor::saveAndClose()
{
   obs->disableNotification();

   obs->setName(lineEdit_name->text().toStdString());
   obs->setType(comboBox_type->currentText().toStdString());
   obs->setInfuseAmount_l(Brewtarget::volQStringToSI(lineEdit_infuseAmount->text()));
   obs->setInfuseTemp_c(Brewtarget::tempQStringToSI(lineEdit_infuseTemp->text()));
   obs->setDecoctionAmount_l(Brewtarget::volQStringToSI(lineEdit_decoctionAmount->text()));
   obs->setStepTemp_c(Brewtarget::tempQStringToSI(lineEdit_stepTemp->text()));
   obs->setStepTime_min(Brewtarget::timeQStringToSI(lineEdit_stepTime->text()));
   obs->setRampTime_min(Brewtarget::timeQStringToSI(lineEdit_rampTime->text()));
   obs->setEndTemp_c(Brewtarget::tempQStringToSI(lineEdit_endTemp->text()));

   obs->reenableNotification();
   obs->forceNotify();

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
