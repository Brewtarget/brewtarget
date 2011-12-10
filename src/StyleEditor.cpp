/*
 * StyleEditor.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "StyleEditor.h"
#include <QInputDialog>
#include "style.h"
#include "StyleComboBox.h"
#include "unit.h"
#include "brewtarget.h"

StyleEditor::StyleEditor(QWidget* parent)
   : QDialog(parent), obsStyle(0)
{
   setupUi(this);

   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newStyle() ) );
   connect( pushButton_cancel, SIGNAL( clicked() ), this, SLOT( clearAndClose() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT(removeStyle()) );
   connect( styleComboBox, SIGNAL(activated( const QString& )), this, SLOT( styleSelected(const QString&) ) );
}

void StyleEditor::setStyle( Style* s )
{
   if( obsStyle )
      disconnect( obsStyle, 0, this, 0 );
   
   obsStyle = s;
   if( obsStyle )
   {
      connect( obsStyle, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
      showChanges();
   }
   
   styleComboBox->setIndexByStyle(obsStyle);
}

void StyleEditor::removeStyle()
{
   if( obsStyle )
      Database::instance().removeStyle(obsStyle);

   setStyle(0);
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

   //s->disableNotification();

   s->setName( lineEdit_name->text() );
   s->setCategory( lineEdit_category->text() );
   s->setCategoryNumber( lineEdit_categoryNumber->text() );
   s->setStyleLetter( lineEdit_styleLetter->text() );
   s->setStyleGuide( lineEdit_styleGuide->text() );
   s->setType( static_cast<Style::Type>(comboBox_type->currentIndex()) );
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
   s->setProfile( textEdit_profile->toPlainText() );
   s->setIngredients( textEdit_ingredients->toPlainText() );
   s->setExamples( textEdit_examples->toPlainText() );
   s->setNotes( textEdit_notes->toPlainText() );
   
   //s->reenableNotification();
   //s->forceNotify();

   setVisible(false);
}

void StyleEditor::newStyle()
{
   QString name = QInputDialog::getText(this, tr("Style name"),
                                          tr("Style name:"));
   if( name.isEmpty() )
      return;

   Style *s = Database::instance().newStyle();
   s->setName( name );

   setStyle(s);
}

void StyleEditor::clearAndClose()
{
   setVisible(false);
}

void StyleEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == obsStyle )
      showChanges(&prop);
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

void StyleEditor::showChanges(QMetaProperty* metaProp)
{
   bool updateAll = false;
   QString propName;
   QVariant val;
   Style *s = obsStyle;
   if( s == 0 )
   {
      clear();
      return;
   }

   if( metaProp == 0 )
      updateAll = true;
   else
   {
      propName = metaProp->name();
      val = metaProp->read(s);
   }

   styleComboBox->setIndexByStyle(s);

   if( propName == "name" || updateAll )
      lineEdit_name->setText(val.toString());
   else if( propName == "category" || updateAll )
      lineEdit_category->setText(val.toString());
   else if( propName == "categoryNumber" || updateAll )
      lineEdit_categoryNumber->setText(val.toString());
   else if( propName == "styleLetter" || updateAll )
      lineEdit_styleLetter->setText(val.toString());
   else if( propName == "styleGuide" || updateAll )
      lineEdit_styleGuide->setText(val.toString());
   else if( propName == "type" || updateAll )
      comboBox_type->setCurrentIndex(val.toInt());
   else if( propName == "ogMin" || updateAll )
      lineEdit_ogMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "ogMax" || updateAll )
      lineEdit_ogMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "fgMin" || updateAll )
      lineEdit_fgMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "fgMax" || updateAll )
      lineEdit_fgMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "ibuMin" || updateAll )
      lineEdit_ibuMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "ibuMax" || updateAll )
      lineEdit_ibuMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "colorMin_srm" || updateAll )
      lineEdit_colorMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "colorMax_srm" || updateAll )
      lineEdit_colorMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "carbMin_vol" || updateAll )
      lineEdit_carbMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "carbMax_vol" || updateAll )
      lineEdit_carbMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "abvMin_pct" || updateAll )
      lineEdit_abvMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "abvMax_pct" || updateAll )
      lineEdit_abvMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "profile" || updateAll )
      textEdit_profile->setText(val.toString());
   else if( propName == "ingredients" || updateAll )
      textEdit_ingredients->setText(val.toString());
   else if( propName == "examples" || updateAll )
      textEdit_examples->setText(val.toString());
   else if( propName == "notes" || updateAll )
      textEdit_notes->setText(val.toString());
}
