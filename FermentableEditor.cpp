/*
 * fermentableEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QIcon>
#include "FermentableEditor.h"
#include "fermentable.h"
#include "observable.h"
#include "stringparsing.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"

FermentableEditor::FermentableEditor( QWidget* parent )
{
   setupUi(this);

   setWindowIcon(QIcon(SMALLBARLEY));

   connect( this, SIGNAL( accepted() ), this, SLOT( save() ));
   connect( this, SIGNAL( rejected() ), this, SLOT( clearAndClose() ));

   obsFerm = 0;
}

void FermentableEditor::setFermentable( Fermentable* f )
{
   if( f && f != obsFerm )
   {
      obsFerm = f;
      setObserved(f);
      showChanges();
   }
}

void FermentableEditor::save()
{
   if( obsFerm == 0 )
      return;

   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   obsFerm->disableNotification();

   obsFerm->setName(lineEdit_name->text().toStdString());
   obsFerm->setType(comboBox_type->currentText().toStdString());
   obsFerm->setAmount_kg(Unit::qstringToSI(lineEdit_amount->text()));
   obsFerm->setYield_pct(Unit::qstringToSI(lineEdit_yield->text()));
   obsFerm->setColor_srm(Unit::qstringToSI(lineEdit_color->text()));
   obsFerm->setAddAfterBoil( (checkBox_addAfterBoil->checkState() == Qt::Checked)? true : false );
   obsFerm->setOrigin( lineEdit_origin->text().toStdString() );
   obsFerm->setSupplier( lineEdit_supplier->text().toStdString() );
   obsFerm->setCoarseFineDiff_pct( Unit::qstringToSI(lineEdit_coarseFineDiff->text()) );
   obsFerm->setMoisture_pct( Unit::qstringToSI(lineEdit_moisture->text()) );
   obsFerm->setDiastaticPower_lintner( Unit::qstringToSI(lineEdit_diastaticPower->text()) );
   obsFerm->setProtein_pct( Unit::qstringToSI(lineEdit_protein->text()) );
   obsFerm->setMaxInBatch_pct( Unit::qstringToSI(lineEdit_maxInBatch->text()) );
   obsFerm->setRecommendMash( (checkBox_recommendMash->checkState() == Qt::Checked) ? true : false );
   obsFerm->setIbuGalPerLb( Unit::qstringToSI(lineEdit_ibuGalPerLb->text()) );
   obsFerm->setNotes( textEdit_notes->toPlainText().toStdString() );

   obsFerm->reenableNotification();
   obsFerm->forceNotify();

   Database::getDatabase()->resortAll(); // If the name changed, need to resort.
}

void FermentableEditor::clearAndClose()
{
   obsFerm->removeObserver(this);
   obsFerm = 0;
   setVisible(false); // Hide the window.
}

void FermentableEditor::notify(Observable* notifier, QVariant info)
{
   if( notifier == obsFerm )
      showChanges();
}

void FermentableEditor::showChanges()
{
   int tmp;

   if( obsFerm == 0 )
      return;

   lineEdit_name->setText(obsFerm->getName().c_str());
   lineEdit_name->setCursorPosition(0);
   tmp = comboBox_type->findText(obsFerm->getType().c_str());
   comboBox_type->setCurrentIndex(tmp);

   lineEdit_amount->setText(Brewtarget::displayAmount(obsFerm->getAmount_kg(), Units::kilograms));
   lineEdit_yield->setText(Brewtarget::displayAmount(obsFerm->getYield_pct(), 0));
   lineEdit_color->setText(Brewtarget::displayAmount(obsFerm->getColor_srm(), 0));
   checkBox_addAfterBoil->setCheckState( obsFerm->getAddAfterBoil()? Qt::Checked : Qt::Unchecked );
   lineEdit_origin->setText(obsFerm->getOrigin().c_str());
   lineEdit_origin->setCursorPosition(0);
   lineEdit_supplier->setText(obsFerm->getSupplier().c_str());
   lineEdit_supplier->setCursorPosition(0);
   lineEdit_coarseFineDiff->setText(Brewtarget::displayAmount(obsFerm->getCoarseFineDiff_pct(), 0));
   lineEdit_moisture->setText(Brewtarget::displayAmount(obsFerm->getMoisture_pct(), 0));
   lineEdit_diastaticPower->setText(Brewtarget::displayAmount(obsFerm->getDiastaticPower_lintner(), 0));
   lineEdit_protein->setText(Brewtarget::displayAmount(obsFerm->getProtein_pct(), 0));
   lineEdit_maxInBatch->setText(Brewtarget::displayAmount(obsFerm->getMaxInBatch_pct(), 0));
   checkBox_recommendMash->setCheckState( obsFerm->getRecommendMash()? Qt::Checked : Qt::Unchecked );
   lineEdit_ibuGalPerLb->setText(Brewtarget::displayAmount(obsFerm->getIbuGalPerLb(), 0));
   textEdit_notes->setPlainText( obsFerm->getNotes().c_str() );
}
