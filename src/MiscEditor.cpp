/*
 * miscEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "misc.h"

#include <QtGui>
#include <iostream>
#include <string>
#include <QIcon>
#include "MiscEditor.h"
#include "stringparsing.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"

MiscEditor::MiscEditor( QWidget* /*parent*/ )
{
   setupUi(this);

   setWindowIcon(QIcon(SMALLQUESTION));
   
   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( save() ));
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( clearAndClose() ));
   
   obsMisc = 0;
}

void MiscEditor::setMisc( Misc* m )
{
   if( m && m != obsMisc )
   {
      obsMisc = m;
      setObserved(m);
      showChanges();
   }
}

void MiscEditor::save()
{
   Misc *m = obsMisc;
   
   if( m == 0 )
   {
      setVisible(false);
      return;
   }
   
   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   m->disableNotification();

   m->setName(lineEdit_name->text().toStdString());
   m->setType( static_cast<Misc::Type>(comboBox_type->currentIndex()) );
   m->setUse( static_cast<Misc::Use>(comboBox_use->currentIndex()) );
   // TODO: fill in the rest of the "set" methods.
   m->setTime(Brewtarget::timeQStringToSI(lineEdit_time->text()));
   m->setAmountIsWeight( (checkBox_isWeight->checkState() == Qt::Checked)? true : false );
   m->setAmount( m->getAmountIsWeight() ? Brewtarget::weightQStringToSI(lineEdit_amount->text()) : Brewtarget::volQStringToSI(lineEdit_amount->text()));
   m->setUseFor(textEdit_useFor->toPlainText().toStdString());
   m->setNotes( textEdit_notes->toPlainText().toStdString() );

   m->reenableNotification();
   m->forceNotify();

   Database::getDatabase()->resortMiscs(); // If the name changed, need to resort.

   setVisible(false);
}

void MiscEditor::clearAndClose()
{
   if( obsMisc )
   {
      obsMisc->removeObserver(this);
      obsMisc = 0;
   }
   setVisible(false); // Hide the window.
}

void MiscEditor::notify(Observable* notifier, QVariant info)
{
   if( notifier == obsMisc ) 
      showChanges();
}

void MiscEditor::showChanges()
{
   if( obsMisc == 0 )
      return;
   
   lineEdit_name->setText(obsMisc->getName().c_str());
   lineEdit_name->setCursorPosition(0);
   comboBox_type->setCurrentIndex(obsMisc->getType());
   comboBox_use->setCurrentIndex(obsMisc->getUse());
   lineEdit_time->setText(Brewtarget::displayAmount(obsMisc->getTime(), Units::minutes));
   lineEdit_amount->setText(Brewtarget::displayAmount(obsMisc->getAmount(), (obsMisc->getAmountIsWeight()) ? (Unit*)Units::kilograms : (Unit*)Units::liters  ));
   checkBox_isWeight->setCheckState( obsMisc->getAmountIsWeight()? Qt::Checked : Qt::Unchecked );
   textEdit_useFor->setPlainText( obsMisc->getUseFor().c_str() );
   textEdit_notes->setPlainText( obsMisc->getNotes().c_str() );
}
