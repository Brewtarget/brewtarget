/*
 * FermentableEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QIcon>
#include "FermentableEditor.h"
#include "fermentable.h"
#include "observable.h"
#include "database.h"
#include "config.h"
#include "unit.h"
#include "brewtarget.h"

FermentableEditor::FermentableEditor( QWidget* parent )
        : QDialog(parent)
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
   {
      setVisible(false);
      return;
   }

   // Need to disable notification since every "set" method will cause a "showChanges" that
   // will revert any changes made.
   obsFerm->disableNotification();

   obsFerm->setName(lineEdit_name->text());
   //obsFerm->setType(comboBox_type->currentText());
   // NOTE: the following assumes that Fermentable::Type is enumerated in the same
   // order as the combobox.
   obsFerm->setType( static_cast<Fermentable::Type>(comboBox_type->currentIndex()) );
   obsFerm->setAmount_kg(Brewtarget::weightQStringToSI(lineEdit_amount->text()));
   obsFerm->setYield_pct(lineEdit_yield->text().toDouble());
   obsFerm->setColor_srm(Brewtarget::colorQStringToSI(lineEdit_color->text()));
   obsFerm->setAddAfterBoil( (checkBox_addAfterBoil->checkState() == Qt::Checked)? true : false );
   obsFerm->setOrigin( lineEdit_origin->text() );
   obsFerm->setSupplier( lineEdit_supplier->text() );
   obsFerm->setCoarseFineDiff_pct( lineEdit_coarseFineDiff->text().toDouble() );
   obsFerm->setMoisture_pct( lineEdit_moisture->text().toDouble() );
   obsFerm->setDiastaticPower_lintner( lineEdit_diastaticPower->text().toDouble() );
   obsFerm->setProtein_pct( lineEdit_protein->text().toDouble() );
   obsFerm->setMaxInBatch_pct( lineEdit_maxInBatch->text().toDouble() );
   obsFerm->setRecommendMash( (checkBox_recommendMash->checkState() == Qt::Checked) ? true : false );
   obsFerm->setIsMashed( (checkBox_isMashed->checkState() == Qt::Checked) ? true : false );
   obsFerm->setIbuGalPerLb( lineEdit_ibuGalPerLb->text().toDouble() );
   obsFerm->setNotes( textEdit_notes->toPlainText() );

   obsFerm->reenableNotification();
   obsFerm->forceNotify();

   Database::getDatabase()->resortFermentables(); // If the name changed, need to resort.

   setVisible(false);
   return;
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
   if( obsFerm == 0 )
      return;

   if (Brewtarget::getColorUnit() == Brewtarget::SRM)
      label_5->setText(QString("Color (Lovibond)"));
   else
      label_5->setText(QString("Color (EBC)"));

   lineEdit_name->setText(obsFerm->getName());
   lineEdit_name->setCursorPosition(0);
   // NOTE: assumes the comboBox entries are in same order as Fermentable::Type
   comboBox_type->setCurrentIndex(obsFerm->getType());

   lineEdit_amount->setText(Brewtarget::displayAmount(obsFerm->getAmount_kg(), Units::kilograms));
   lineEdit_yield->setText(Brewtarget::displayAmount(obsFerm->getYield_pct(), 0));
   lineEdit_color->setText(Brewtarget::displayColor(obsFerm->getColor_srm(), false));
   checkBox_addAfterBoil->setCheckState( obsFerm->getAddAfterBoil()? Qt::Checked : Qt::Unchecked );
   lineEdit_origin->setText(obsFerm->getOrigin());
   lineEdit_origin->setCursorPosition(0);
   lineEdit_supplier->setText(obsFerm->getSupplier());
   lineEdit_supplier->setCursorPosition(0);
   lineEdit_coarseFineDiff->setText(Brewtarget::displayAmount(obsFerm->getCoarseFineDiff_pct(), 0));
   lineEdit_moisture->setText(Brewtarget::displayAmount(obsFerm->getMoisture_pct(), 0));
   lineEdit_diastaticPower->setText(Brewtarget::displayAmount(obsFerm->getDiastaticPower_lintner(), 0));
   lineEdit_protein->setText(Brewtarget::displayAmount(obsFerm->getProtein_pct(), 0));
   lineEdit_maxInBatch->setText(Brewtarget::displayAmount(obsFerm->getMaxInBatch_pct(), 0));
   checkBox_recommendMash->setCheckState( obsFerm->getRecommendMash()? Qt::Checked : Qt::Unchecked );
   checkBox_isMashed->setCheckState( obsFerm->getIsMashed() ? Qt::Checked : Qt::Unchecked );
   lineEdit_ibuGalPerLb->setText(Brewtarget::displayAmount(obsFerm->getIbuGalPerLb(), 0));
   textEdit_notes->setPlainText( obsFerm->getNotes() );
}
