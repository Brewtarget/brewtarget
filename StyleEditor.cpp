/*
 * StyleEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QInputDialog>
#include <QIcon>
#include <string>
#include <iostream>

#include "style.h"
#include "StyleEditor.h"
#include "StyleComboBox.h"
#include "stringparsing.h"
#include "config.h"
#include "unit.h"

StyleEditor::StyleEditor(QWidget* parent)
        : QDialog(parent)
{
   setupUi(this);

   setWindowIcon(QIcon(SMALLSTYLE));

   styleComboBox->startObservingDB();

   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newStyle() ) );
   connect( pushButton_cancel, SIGNAL( clicked() ), this, SLOT( clearAndClose() ) );
   connect( styleComboBox, SIGNAL(activated( const QString& )), this, SLOT( styleSelected(const QString&) ) );
}

void StyleEditor::setStyle( Style* s )
{
   if( s && s != obsStyle )
   {
      obsStyle = s;
      setObserved(obsStyle);
      showChanges();
   }
}

void StyleEditor::styleSelected( const QString& /*text*/ )
{
   setStyle( styleComboBox->getSelected() );
}

void StyleEditor::save()
{
   Style* s = obsStyle;

   s->disableNotification();

   s->setName( lineEdit_name->text().toStdString() );
   s->setCategory( lineEdit_category->text().toStdString() );
   s->setCategoryNumber( lineEdit_categoryNumber->text().toStdString() );
   s->setStyleLetter( lineEdit_styleLetter->text().toStdString() );
   s->setStyleGuide( lineEdit_styleGuide->text().toStdString() );
   s->setType( comboBox_type->currentText().toStdString() );
   s->setOgMin( Unit::qstringToSI(lineEdit_ogMin->text()) );
   s->setOgMax( Unit::qstringToSI(lineEdit_ogMax->text()) );
   s->setFgMin( Unit::qstringToSI(lineEdit_fgMin->text()) );
   s->setFgMax( Unit::qstringToSI(lineEdit_fgMax->text()) );
   s->setIbuMin( Unit::qstringToSI(lineEdit_ibuMin->text()) );
   s->setIbuMax( Unit::qstringToSI(lineEdit_ibuMax->text()) );
   s->setColorMin_srm( Unit::qstringToSI(lineEdit_colorMin->text()) );
   s->setColorMax_srm( Unit::qstringToSI(lineEdit_colorMax->text()) );
   s->setCarbMin_vol( Unit::qstringToSI(lineEdit_carbMin->text()) );
   s->setCarbMax_vol( Unit::qstringToSI(lineEdit_carbMax->text()) );
   s->setAbvMin_pct( Unit::qstringToSI(lineEdit_abvMin->text()) );
   s->setAbvMax_pct( Unit::qstringToSI(lineEdit_abvMax->text()) );
   s->setProfile( textEdit_profile->toPlainText().toStdString() );
   s->setIngredients( textEdit_ingredients->toPlainText().toStdString() );
   s->setExamples( textEdit_examples->toPlainText().toStdString() );
   s->setNotes( textEdit_notes->toPlainText().toStdString() );
   
   s->reenableNotification();
   s->forceNotify();

   Database::getDatabase()->resortAll(); // If the name changed, need to resort.
}

void StyleEditor::newStyle()
{
   QString name = QInputDialog::getText(this, tr("Style name"),
                                          tr("Style name:"));
   if( name.isEmpty() )
      return;

   Style *s = new Style();
   s->setName( name.toStdString() );

   Database::getDatabase()->addStyle(s);

   setStyle(s);
}

void StyleEditor::clearAndClose()
{
   setVisible(false);
}

void StyleEditor::notify(Observable* /*notifier*/)
{
   showChanges();
}

void StyleEditor::showChanges()
{
   Style *s = obsStyle;

   styleComboBox->setIndexByStyleName(s->getName());

   lineEdit_name->setText(s->getName().c_str());
   lineEdit_category->setText(s->getCategory().c_str());
   lineEdit_categoryNumber->setText(s->getCategoryNumber().c_str());
   lineEdit_styleLetter->setText(s->getStyleLetter().c_str());
   lineEdit_styleGuide->setText(s->getStyleGuide().c_str());
   comboBox_type->setCurrentIndex(comboBox_type->findText(s->getType().c_str(), Qt::MatchExactly));
   lineEdit_ogMin->setText(doubleToString(s->getOgMin()).c_str());
   lineEdit_ogMax->setText(doubleToString(s->getOgMax()).c_str());
   lineEdit_fgMin->setText(doubleToString(s->getFgMin()).c_str());
   lineEdit_fgMax->setText(doubleToString(s->getFgMax()).c_str());
   lineEdit_ibuMin->setText(doubleToString(s->getIbuMin()).c_str());
   lineEdit_ibuMax->setText(doubleToString(s->getIbuMax()).c_str());
   lineEdit_colorMin->setText(doubleToString(s->getColorMin_srm()).c_str());
   lineEdit_colorMax->setText(doubleToString(s->getColorMax_srm()).c_str());
   lineEdit_carbMin->setText(doubleToString(s->getCarbMin_vol()).c_str());
   lineEdit_carbMax->setText(doubleToString(s->getCarbMax_vol()).c_str());
   lineEdit_abvMin->setText(doubleToString(s->getAbvMin_pct()).c_str());
   lineEdit_abvMax->setText(doubleToString(s->getAbvMax_pct()).c_str());
   textEdit_profile->setText(s->getProfile().c_str());
   textEdit_ingredients->setText(s->getIngredients().c_str());
   textEdit_examples->setText(s->getExamples().c_str());
   textEdit_notes->setText(s->getNotes().c_str());
   /*
   lineEdit_name->setText(e->getName().c_str());
   lineEdit_name->setCursorPosition(0);
   lineEdit_boilSize->setText(doubleToString(e->getBoilSize_l()).c_str());
   checkBox_calcBoilVolume->setCheckState( (e->getCalcBoilVolume())? Qt::Checked : Qt::Unchecked );
   lineEdit_batchSize->setText(doubleToString(e->getBatchSize_l()).c_str());

   lineEdit_tunVolume->setText(doubleToString(e->getTunVolume_l()).c_str());
   lineEdit_tunWeight->setText(doubleToString(e->getTunWeight_kg()).c_str());
   lineEdit_tunSpecificHeat->setText(doubleToString(e->getTunSpecificHeat_calGC()).c_str());

   lineEdit_boilTime->setText(doubleToString(e->getBoilTime_min()).c_str());
   lineEdit_evaporationRate->setText(doubleToString(e->getEvapRate_pctHr()).c_str());
   lineEdit_topUpKettle->setText(doubleToString(e->getTopUpKettle_l()).c_str());
   lineEdit_topUpWater->setText(doubleToString(e->getTopUpWater_l()).c_str());
   lineEdit_hopUtilization->setText(doubleToString(e->getHopUtilization_pct()).c_str());

   lineEdit_trubChillerLoss->setText(doubleToString(e->getTrubChillerLoss_l()).c_str());
   lineEdit_lauterDeadspace->setText(doubleToString(e->getLauterDeadspace_l()).c_str());

   textEdit_notes->setText(e->getNotes().c_str());
    */
}
