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

#include "FermentableEditor.h"
#include "fermentable.h"
#include "observable.h"
#include "stringparsing.h"
#include "database.h"

FermentableEditor::FermentableEditor( QWidget* parent )
{
   setupUi(this);

   setWindowIcon(parent->windowIcon());

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
   obsFerm->setAmount_kg(parseDouble(lineEdit_amount->text().toStdString()));
   obsFerm->setYield_pct(parseDouble(lineEdit_yield->text().toStdString()));
   obsFerm->setColor_srm(parseDouble(lineEdit_color->text().toStdString()));
   obsFerm->setAddAfterBoil( (checkBox_addAfterBoil->checkState() == Qt::Checked)? true : false );
   obsFerm->setOrigin( lineEdit_origin->text().toStdString() );
   obsFerm->setSupplier( lineEdit_supplier->text().toStdString() );
   obsFerm->setCoarseFineDiff_pct( parseDouble(lineEdit_coarseFineDiff->text().toStdString()) );
   obsFerm->setMoisture_pct( parseDouble(lineEdit_moisture->text().toStdString()) );
   obsFerm->setDiastaticPower_lintner( parseDouble(lineEdit_diastaticPower->text().toStdString()) );
   obsFerm->setProtein_pct( parseDouble(lineEdit_protein->text().toStdString()) );
   obsFerm->setMaxInBatch_pct( parseDouble(lineEdit_maxInBatch->text().toStdString()) );
   obsFerm->setRecommendMash( (checkBox_recommendMash->checkState() == Qt::Checked) ? true : false );
   obsFerm->setIbuGalPerLb( parseDouble(lineEdit_ibuGalPerLb->text().toStdString()) );
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

void FermentableEditor::notify(Observable* notifier)
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

   lineEdit_amount->setText(QString::number(obsFerm->getAmount_kg()));
   lineEdit_yield->setText(QString::number(obsFerm->getYield_pct()));
   lineEdit_color->setText(QString::number(obsFerm->getColor_srm()));
   checkBox_addAfterBoil->setCheckState( obsFerm->getAddAfterBoil()? Qt::Checked : Qt::Unchecked );
   lineEdit_origin->setText(obsFerm->getOrigin().c_str());
   lineEdit_origin->setCursorPosition(0);
   lineEdit_supplier->setText(obsFerm->getSupplier().c_str());
   lineEdit_supplier->setCursorPosition(0);
   lineEdit_coarseFineDiff->setText(QString::number(obsFerm->getCoarseFineDiff_pct()));
   lineEdit_moisture->setText(QString::number(obsFerm->getMoisture_pct()));
   lineEdit_diastaticPower->setText(QString::number(obsFerm->getDiastaticPower_lintner()));
   lineEdit_protein->setText(QString::number(obsFerm->getProtein_pct()));
   lineEdit_maxInBatch->setText(QString::number(obsFerm->getMaxInBatch_pct()));
   checkBox_recommendMash->setCheckState( obsFerm->getRecommendMash()? Qt::Checked : Qt::Unchecked );
   lineEdit_ibuGalPerLb->setText(QString::number(obsFerm->getIbuGalPerLb()));
   textEdit_notes->setPlainText( obsFerm->getNotes().c_str() );
}
