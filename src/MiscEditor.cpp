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
   : QDialog(parent), obsMisc(nullptr)
{
   setupUi(this);

   connect( buttonBox, &QDialogButtonBox::accepted, this, &MiscEditor::save);
   connect( buttonBox, &QDialogButtonBox::rejected, this, &MiscEditor::clearAndClose);

}

void MiscEditor::setMisc( Misc* m )
{
   if( obsMisc )
      disconnect( obsMisc, nullptr, this, nullptr );

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

   if( m == nullptr )
   {
      setVisible(false);
      return;
   }

   m->setName(lineEdit_name->text(),m->cacheOnly());
   m->setType( static_cast<Misc::Type>(comboBox_type->currentIndex()) );
   m->setUse( static_cast<Misc::Use>(comboBox_use->currentIndex()) );
   m->setTime(lineEdit_time->toSI());
   m->setAmountIsWeight( (checkBox_isWeight->checkState() == Qt::Checked)? true : false );
   m->setAmount( lineEdit_amount->toSI());
   m->setUseFor(textEdit_useFor->toPlainText());
   m->setNotes( textEdit_notes->toPlainText() );

   if ( m->cacheOnly() ) {
      Database::instance().insertMisc(m);
   }
   // do this late to make sure we've the row in the inventory table
   m->setInventoryAmount(lineEdit_inventory->toSI());
   setVisible(false);
}

void MiscEditor::clearAndClose()
{
   setMisc(nullptr);
   setVisible(false); // Hide the window.
}

void MiscEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == obsMisc )
      showChanges(&prop);
}

void MiscEditor::showChanges(QMetaProperty* metaProp)
{
   if( obsMisc == nullptr )
      return;

   QString propName;
   QVariant value;
   bool updateAll = false;
   if( metaProp == nullptr )
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
