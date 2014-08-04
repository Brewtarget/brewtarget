/*
 * StyleEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include "database.h"
#include "StyleEditor.h"
#include <QInputDialog>
#include "style.h"
#include "StyleListModel.h"
#include "StyleSortFilterProxyModel.h"
#include "unit.h"
#include "brewtarget.h"

StyleEditor::StyleEditor(QWidget* parent, bool singleStyleEditor)
   : QDialog(parent), obsStyle(0)
{
   setupUi(this);
   if ( singleStyleEditor ) 
   {
      for(int i = 0; i < horizontalLayout_styles->count(); ++i)
      {
         QWidget* w = horizontalLayout_styles->itemAt(i)->widget();
         if(w)
            w->setVisible(false);
      }
      
      pushButton_new->setVisible(false);
   }

   styleListModel = new StyleListModel(styleComboBox);
   styleProxyModel = new StyleSortFilterProxyModel(styleComboBox);
   styleProxyModel->setDynamicSortFilter(true);
   styleProxyModel->setSourceModel(styleListModel);
   styleComboBox->setModel(styleProxyModel);
   
   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newStyle() ) );
   connect( pushButton_cancel, SIGNAL( clicked() ), this, SLOT( clearAndClose() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT(removeStyle()) );
   connect( styleComboBox, SIGNAL(activated( const QString& )), this, SLOT( styleSelected(const QString&) ) );

   setStyle( styleListModel->at(styleComboBox->currentIndex()));

   connect( lineEdit_ogMin, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_fgMin, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_ibuMin, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_colorMin, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_carbMin, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_abvMin, SIGNAL(editingFinished()), this, SLOT(updateField()));

   connect( lineEdit_ogMax, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_fgMax, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_ibuMax, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_colorMax, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_carbMax, SIGNAL(editingFinished()), this, SLOT(updateField()));
   connect( lineEdit_abvMax, SIGNAL(editingFinished()), this, SLOT(updateField()));
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
 
   styleComboBox->setCurrentIndex(styleListModel->indexOf(obsStyle));
}

void StyleEditor::removeStyle()
{
   if( obsStyle )
      Database::instance().remove(obsStyle);

   setStyle(0);
}

void StyleEditor::styleSelected( const QString& /*text*/ )
{
   QModelIndex proxyIndex( styleProxyModel->index(styleComboBox->currentIndex(),0) );
   QModelIndex sourceIndex( styleProxyModel->mapToSource(proxyIndex) );
   setStyle( styleListModel->at(sourceIndex.row()) );
}

void StyleEditor::save()
{
   Style* s = obsStyle;
   if( s == 0 )
   {
      setVisible(false);
      return;
   }
   
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
   show();
}

void StyleEditor::clearAndClose()
{
   setVisible(false);
}

void StyleEditor::changed(QMetaProperty prop, QVariant /*val*/)
{
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

void StyleEditor::updateField()
{

   QObject* selection = sender();
   QLineEdit* field = qobject_cast<QLineEdit*>(selection);
   double val;

   if ( field == lineEdit_ogMin || field == lineEdit_fgMin ||
        field == lineEdit_ogMax || field == lineEdit_fgMax  )
   {
      val = Brewtarget::densityQStringToSI(field->text());
      field->setText(Brewtarget::displayOG( val, noUnit, true));
   }
   else if ( field == lineEdit_colorMin || field == lineEdit_colorMax )
   {
      val = field->text().toDouble();
      field->setText(Brewtarget::displayColor(val, noUnit, false));
   }
   else 
   {
      //Everything else is a number, so just format nicely
      val = field->text().toDouble();
      field->setText(Brewtarget::displayAmount(val));
   }

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

   //styleComboBox->setIndexByStyle(s);
   // Update the color label text.
   if (Brewtarget::getColorUnit() == displaySrm)
      label_10->setText(QString("Color (Lovibond)"));
   else
      label_10->setText(QString("Color (EBC)"));

   if( updateAll )
   {
      lineEdit_name->setText(s->name());
      lineEdit_category->setText(s->category());
      lineEdit_categoryNumber->setText(s->categoryNumber());
      lineEdit_styleLetter->setText(s->styleLetter());
      lineEdit_styleGuide->setText(s->styleGuide());
      comboBox_type->setCurrentIndex(s->type());
      lineEdit_ogMin->setText(Brewtarget::displayOG(s->ogMin(),noUnit,true));
      lineEdit_ogMax->setText(Brewtarget::displayOG(s->ogMax(),noUnit,true));
      lineEdit_fgMin->setText(Brewtarget::displayFG(s->fgMin(),s->ogMin(),noUnit,true));
      lineEdit_fgMax->setText(Brewtarget::displayFG(s->fgMax(),s->ogMax(),noUnit,true));
      lineEdit_ibuMin->setText(Brewtarget::displayAmount(s->ibuMin(), 0));
      lineEdit_ibuMax->setText(Brewtarget::displayAmount(s->ibuMax(), 0));
      lineEdit_colorMin->setText(Brewtarget::displayColor(s->colorMin_srm(),noUnit,false));
      lineEdit_colorMax->setText(Brewtarget::displayColor(s->colorMax_srm(),noUnit,false));
      lineEdit_carbMin->setText(Brewtarget::displayAmount(s->carbMin_vol(), 0));
      lineEdit_carbMax->setText(Brewtarget::displayAmount(s->carbMax_vol(), 0));
      lineEdit_abvMin->setText(Brewtarget::displayAmount(s->abvMin_pct(), 0));
      lineEdit_abvMax->setText(Brewtarget::displayAmount(s->abvMax_pct(), 0));
      textEdit_profile->setText(s->profile());
      textEdit_ingredients->setText(s->ingredients());
      textEdit_examples->setText(s->examples());
      textEdit_notes->setText(s->notes());
      
      return;
   }
   
   if( propName == "name" )
      lineEdit_name->setText(val.toString());
   else if( propName == "category" )
      lineEdit_category->setText(val.toString());
   else if( propName == "categoryNumber" )
      lineEdit_categoryNumber->setText(val.toString());
   else if( propName == "styleLetter" )
      lineEdit_styleLetter->setText(val.toString());
   else if( propName == "styleGuide" )
      lineEdit_styleGuide->setText(val.toString());
   else if( propName == "type" )
      comboBox_type->setCurrentIndex(val.toInt());
   else if( propName == "ogMin" )
      lineEdit_ogMin->setText(Brewtarget::displayOG(val.toDouble(),noUnit,true));
   else if( propName == "ogMax" )
      lineEdit_ogMax->setText(Brewtarget::displayOG(val.toDouble(),noUnit,true));
   else if( propName == "fgMin" )
      lineEdit_fgMin->setText(Brewtarget::displayFG(val.toDouble(),s->ogMin(), noUnit,true));
   else if( propName == "fgMax" )
      lineEdit_fgMax->setText(Brewtarget::displayFG(val.toDouble(),s->ogMax(), noUnit,true));
   else if( propName == "ibuMin" )
      lineEdit_ibuMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "ibuMax" )
      lineEdit_ibuMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "colorMin_srm" )
      lineEdit_colorMin->setText(Brewtarget::displayColor(val.toDouble(),noUnit,false));
   else if( propName == "colorMax_srm" )
      lineEdit_colorMax->setText(Brewtarget::displayColor(val.toDouble(),noUnit,false));
   else if( propName == "carbMin_vol" )
      lineEdit_carbMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "carbMax_vol" )
      lineEdit_carbMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "abvMin_pct" )
      lineEdit_abvMin->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "abvMax_pct" )
      lineEdit_abvMax->setText(Brewtarget::displayAmount(val.toDouble(), 0));
   else if( propName == "profile" )
      textEdit_profile->setText(val.toString());
   else if( propName == "ingredients" )
      textEdit_ingredients->setText(val.toString());
   else if( propName == "examples" )
      textEdit_examples->setText(val.toString());
   else if( propName == "notes" )
      textEdit_notes->setText(val.toString());
}
