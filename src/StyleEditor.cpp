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
#include "brewtarget.h"

StyleEditor::StyleEditor(QWidget* parent)
        : QDialog(parent)
{
   setupUi(this);

   setWindowIcon(QIcon(SMALLSTYLE));

   styleComboBox->startObservingDB();
   obsStyle = 0;

   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newStyle() ) );
   connect( pushButton_cancel, SIGNAL( clicked() ), this, SLOT( clearAndClose() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT(removeStyle()) );
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

void StyleEditor::removeStyle()
{
   if( obsStyle )
      Database::getDatabase()->removeStyle(obsStyle);

   obsStyle = 0;
   setObserved(obsStyle);
   
   styleComboBox->setIndexByStyleName("");
   showChanges();
}

void StyleEditor::styleSelected( const QString& /*text*/ )
{
   setStyle( styleComboBox->getSelected() );
}

void StyleEditor::save()
{
   Style* s = obsStyle;
   if( s == 0 )
   {
      setVisible(false);
      return;
   }

   s->disableNotification();

   s->setName( lineEdit_name->text().toStdString() );
   s->setCategory( lineEdit_category->text().toStdString() );
   s->setCategoryNumber( lineEdit_categoryNumber->text().toStdString() );
   s->setStyleLetter( lineEdit_styleLetter->text().toStdString() );
   s->setStyleGuide( lineEdit_styleGuide->text().toStdString() );
   s->setType( comboBox_type->currentText().toStdString() );
   s->setOgMin( lineEdit_ogMin->text().toDouble() );
   s->setOgMax( lineEdit_ogMax->text().toDouble() );
   s->setFgMin( lineEdit_fgMin->text().toDouble() );
   s->setFgMax( lineEdit_fgMax->text().toDouble() );
   s->setIbuMin( lineEdit_ibuMin->text().toDouble() );
   s->setIbuMax( lineEdit_ibuMax->text().toDouble() );
   s->setColorMin_srm( lineEdit_colorMin->text().toDouble() );
   s->setColorMax_srm( lineEdit_colorMax->text().toDouble() );
   s->setCarbMin_vol( lineEdit_carbMin->text().toDouble() );
   s->setCarbMax_vol( lineEdit_carbMax->text().toDouble() );
   s->setAbvMin_pct( lineEdit_abvMin->text().toDouble() );
   s->setAbvMax_pct( lineEdit_abvMax->text().toDouble() );
   s->setProfile( textEdit_profile->toPlainText().toStdString() );
   s->setIngredients( textEdit_ingredients->toPlainText().toStdString() );
   s->setExamples( textEdit_examples->toPlainText().toStdString() );
   s->setNotes( textEdit_notes->toPlainText().toStdString() );
   
   s->reenableNotification();
   s->forceNotify();

   Database::getDatabase()->resortStyles(); // If the name changed, need to resort.

   setVisible(false);
   return;
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

void StyleEditor::notify(Observable* /*notifier*/, QVariant /*info*/)
{
   showChanges();
}

void StyleEditor::clear()
{
   lineEdit_name->setText(QString(""));
   lineEdit_category->setText(QString(""));
   lineEdit_categoryNumber->setText(QString(""));
   lineEdit_styleLetter->setText(QString(""));
   lineEdit_styleGuide->setText(QString(""));
   lineEdit_ogMin->setText(QString(""));
   lineEdit_ogMax->setText(QString(""));
   lineEdit_fgMin->setText(QString(""));
   lineEdit_fgMax->setText(QString(""));
   lineEdit_ibuMin->setText(QString(""));
   lineEdit_ibuMax->setText(QString(""));
   lineEdit_colorMin->setText(QString(""));
   lineEdit_colorMax->setText(QString(""));
   lineEdit_carbMin->setText(QString(""));
   lineEdit_carbMax->setText(QString(""));
   lineEdit_abvMin->setText(QString(""));
   lineEdit_abvMax->setText(QString(""));
   textEdit_profile->setText(QString(""));
   textEdit_ingredients->setText(QString(""));
   textEdit_examples->setText(QString(""));
   textEdit_notes->setText(QString(""));
}

void StyleEditor::showChanges()
{
   Style *s = obsStyle;
   if( s == 0 )
   {
      clear();
      return;
   }

   styleComboBox->setIndexByStyleName(s->getName());

   lineEdit_name->setText(s->getName().c_str());
   lineEdit_category->setText(s->getCategory().c_str());
   lineEdit_categoryNumber->setText(s->getCategoryNumber().c_str());
   lineEdit_styleLetter->setText(s->getStyleLetter().c_str());
   lineEdit_styleGuide->setText(s->getStyleGuide().c_str());
   comboBox_type->setCurrentIndex(comboBox_type->findText(s->getType().c_str(), Qt::MatchExactly));
   lineEdit_ogMin->setText(Brewtarget::displayAmount(s->getOgMin(), 0));
   lineEdit_ogMax->setText(Brewtarget::displayAmount(s->getOgMax(), 0));
   lineEdit_fgMin->setText(Brewtarget::displayAmount(s->getFgMin(), 0));
   lineEdit_fgMax->setText(Brewtarget::displayAmount(s->getFgMax(), 0));
   lineEdit_ibuMin->setText(Brewtarget::displayAmount(s->getIbuMin(), 0));
   lineEdit_ibuMax->setText(Brewtarget::displayAmount(s->getIbuMax(), 0));
   lineEdit_colorMin->setText(Brewtarget::displayAmount(s->getColorMin_srm(), 0));
   lineEdit_colorMax->setText(Brewtarget::displayAmount(s->getColorMax_srm(), 0));
   lineEdit_carbMin->setText(Brewtarget::displayAmount(s->getCarbMin_vol(), 0));
   lineEdit_carbMax->setText(Brewtarget::displayAmount(s->getCarbMax_vol(), 0));
   lineEdit_abvMin->setText(Brewtarget::displayAmount(s->getAbvMin_pct(), 0));
   lineEdit_abvMax->setText(Brewtarget::displayAmount(s->getAbvMax_pct(), 0));
   textEdit_profile->setText(s->getProfile().c_str());
   textEdit_ingredients->setText(s->getIngredients().c_str());
   textEdit_examples->setText(s->getExamples().c_str());
   textEdit_notes->setText(s->getNotes().c_str());
}
