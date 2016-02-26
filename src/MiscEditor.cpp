/*
 * MiscEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
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

#include <QtGui>
#include <QIcon>
#include "MiscEditor.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"
#include "misc.h"

MiscEditor::MiscEditor( QWidget* parent )
   : QDialog(parent), obsMisc(0)
{
   setupUi(this);

   // Hide the tab Properties
   // Currently we don't have data for this tab, but
   // once we have, just remove the following line
   tabWidget->removeTab(1);
   
   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( save() ));
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( clearAndClose() ));

}

void MiscEditor::setMisc( Misc* m )
{
   if( obsMisc )
      disconnect( obsMisc, 0, this, 0 );
   
   obsMisc = m;
   if( obsMisc )
   {
      connect( obsMisc, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void MiscEditor::save()
{
   Misc* m = obsMisc;
   
   if( m == 0 )
   {
      setVisible(false);
      return;
   }
   
   m->setName(lineEdit_name->text());
   m->setType( static_cast<Misc::Type>(comboBox_type->currentIndex()) );
   m->setUse( static_cast<Misc::Use>(comboBox_use->currentIndex()) );
   // TODO: fill in the rest of the "set" methods.
   m->setTime(lineEdit_time->toSI());
   m->setAmountIsWeight( (checkBox_isWeight->checkState() == Qt::Checked)? true : false );
   m->setAmount( lineEdit_amount->toSI());
   m->setUseFor(textEdit_useFor->toPlainText());
   m->setNotes( textEdit_notes->toPlainText() );

   setVisible(false);
}

void MiscEditor::clearAndClose()
{
   setMisc(0);
   setVisible(false); // Hide the window.
}

void MiscEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == obsMisc ) 
      showChanges(&prop);
}

void MiscEditor::showChanges(QMetaProperty* metaProp)
{
   if( obsMisc == 0 )
      return;
   
   QString propName;
   QVariant value;
   bool updateAll = false;
   if( metaProp == 0 )
      updateAll = true;
   else
   {
      propName = metaProp->name();
      value = metaProp->read(obsMisc);
   }
   
   if( propName == "name" || updateAll )
   {
      lineEdit_name->setText(obsMisc->name());
      lineEdit_name->setCursorPosition(0);
      if( ! updateAll )
         return;
   }
   if( propName == "type" || updateAll )
   {
      comboBox_type->setCurrentIndex(obsMisc->type());
      if( ! updateAll )
         return;
   }
   if( propName == "use" || updateAll )
   {
      comboBox_use->setCurrentIndex(obsMisc->use());
      if( ! updateAll )
         return;
   }
   if( propName == "time" || updateAll )
   {
      lineEdit_time->setText(obsMisc);
      if( ! updateAll )
         return;
   }
   if( propName == "amount" || updateAll )
   {
      lineEdit_amount->setText(obsMisc);
      if( ! updateAll )
         return;
   }
   if( propName == "amountIsWeight" || updateAll )
   {
      checkBox_isWeight->setCheckState( obsMisc->amountIsWeight()? Qt::Checked : Qt::Unchecked );
      if( ! updateAll )
         return;
   }
   if( propName == "inventory" || updateAll )
   {
      lineEdit_inventory->setText(obsMisc);
      if( ! updateAll )
         return;
   }
   if( propName == "useFor" || updateAll )
   {
      textEdit_useFor->setPlainText( obsMisc->useFor() );
      if( ! updateAll )
         return;
   }
   if( propName == "notes" || updateAll )
   {
      textEdit_notes->setPlainText( obsMisc->notes() );
      if( ! updateAll )
         return;
   }
}
