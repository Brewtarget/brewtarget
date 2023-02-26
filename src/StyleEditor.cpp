/*
 * StyleEditor.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Matt Young <mfsy@yahoo.com>
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
#include "StyleEditor.h"

#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Style.h"
#include "StyleListModel.h"
#include "StyleSortFilterProxyModel.h"

StyleEditor::StyleEditor(QWidget* parent, bool singleStyleEditor) : QDialog{parent}, obsStyle{nullptr} {
   setupUi(this);
   if (singleStyleEditor) {
      for (int i = 0; i < horizontalLayout_styles->count(); ++i) {
         QWidget* w = horizontalLayout_styles->itemAt(i)->widget();
         if (w) {
            w->setVisible(false);
         }
      }

      pushButton_new->setVisible(false);
   }

   this->tabWidget_profile->tabBar()->setStyle(new BtHorizontalTabs);

   styleListModel = new StyleListModel(styleComboBox);
   styleProxyModel = new StyleSortFilterProxyModel(styleComboBox);
   styleProxyModel->setDynamicSortFilter(true);
   styleProxyModel->setSourceModel(styleListModel);
   styleComboBox->setModel(styleProxyModel);

   connect( pushButton_save, &QAbstractButton::clicked, this, &StyleEditor::save );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newStyle() ) );
   connect( pushButton_cancel, &QAbstractButton::clicked, this, &StyleEditor::clearAndClose );
   connect( pushButton_remove, &QAbstractButton::clicked, this, &StyleEditor::removeStyle );
   connect( styleComboBox, SIGNAL(activated( const QString& )), this, SLOT( styleSelected(const QString&) ) );

   setStyle( styleListModel->at(styleComboBox->currentIndex()));
   return;
}

void StyleEditor::setStyle( Style* s ) {
   if (obsStyle) {
      disconnect( obsStyle, 0, this, 0 );
   }

   obsStyle = s;
   if (obsStyle) {
      connect( obsStyle, &NamedEntity::changed, this, &StyleEditor::changed );
      showChanges();
   }

   styleComboBox->setCurrentIndex(styleListModel->indexOf(obsStyle));
   return;
}

void StyleEditor::removeStyle() {
   if (this->obsStyle) {
      ObjectStoreWrapper::softDelete(*this->obsStyle);
   }

   setStyle(0);
   return;
}

void StyleEditor::styleSelected( const QString& /*text*/ ) {
   QModelIndex proxyIndex( styleProxyModel->index(styleComboBox->currentIndex(),0) );
   QModelIndex sourceIndex( styleProxyModel->mapToSource(proxyIndex) );
   setStyle( styleListModel->at(sourceIndex.row()) );
   return;
}

void StyleEditor::save() {
   if (!this->obsStyle) {
      setVisible(false);
      return;
   }

   this->obsStyle->setName(lineEdit_name->text());
   this->obsStyle->setCategory( lineEdit_category->text() );
   this->obsStyle->setCategoryNumber( lineEdit_categoryNumber->text() );
   this->obsStyle->setStyleLetter( lineEdit_styleLetter->text() );
   this->obsStyle->setStyleGuide( lineEdit_styleGuide->text() );
   this->obsStyle->setType( static_cast<Style::Type>(comboBox_type->currentIndex()) );
   this->obsStyle->setOgMin( lineEdit_ogMin->toCanonical().quantity() );
   this->obsStyle->setOgMax( lineEdit_ogMax->toCanonical().quantity() );
   this->obsStyle->setFgMin( lineEdit_fgMin->toCanonical().quantity() );
   this->obsStyle->setFgMax( lineEdit_fgMax->toCanonical().quantity() );
   this->obsStyle->setIbuMin( lineEdit_ibuMin->toCanonical().quantity() );
   this->obsStyle->setIbuMax( lineEdit_ibuMax->toCanonical().quantity() );
   this->obsStyle->setColorMin_srm( lineEdit_colorMin->toCanonical().quantity() );
   this->obsStyle->setColorMax_srm( lineEdit_colorMax->toCanonical().quantity() );
   this->obsStyle->setCarbMin_vol( lineEdit_carbMin->toCanonical().quantity() );
   this->obsStyle->setCarbMax_vol( lineEdit_carbMax->toCanonical().quantity() );
   this->obsStyle->setAbvMin_pct( lineEdit_abvMin->toCanonical().quantity() );
   this->obsStyle->setAbvMax_pct( lineEdit_abvMax->toCanonical().quantity() );
   this->obsStyle->setProfile( textEdit_profile->toPlainText() );
   this->obsStyle->setIngredients( textEdit_ingredients->toPlainText() );
   this->obsStyle->setExamples( textEdit_examples->toPlainText() );
   this->obsStyle->setNotes( textEdit_notes->toPlainText() );

   if (this->obsStyle->key() < 0) {
      ObjectStoreWrapper::insert(*this->obsStyle);
   }

   setVisible(false);
   return;
}

void StyleEditor::newStyle()
{
   newStyle(QString());
}

void StyleEditor::newStyle(QString folder)
{
   QString name = QInputDialog::getText(this, tr("Style name"),
                                          tr("Style name:"));
   if( name.isEmpty() )
      return;

   Style *s = new Style(name);
   if ( ! folder.isEmpty() ) {
      s->setFolder(folder);
   }

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

void StyleEditor::showChanges(QMetaProperty* metaProp) {
   Style *s = obsStyle;
   if (s == 0 ) {
      clear();
      return;
   }

   if (metaProp == 0) {
      // updateAll = true;
      lineEdit_name->setText(s->name());
      tabWidget_profile->setTabText(0, s->name() );
      lineEdit_category->setText(s->category());
      lineEdit_categoryNumber->setText(s->categoryNumber());
      lineEdit_styleLetter->setText(s->styleLetter());
      lineEdit_styleGuide->setText(s->styleGuide());
      comboBox_type->setCurrentIndex(static_cast<int>(s->type()));
      lineEdit_ogMin->setText(s);
      lineEdit_ogMax->setText(s);
      lineEdit_fgMin->setText(s);
      lineEdit_fgMax->setText(s);
      lineEdit_ibuMin->setText(s);
      lineEdit_ibuMax->setText(s);
      lineEdit_colorMin->setText(s);
      lineEdit_colorMax->setText(s);
      lineEdit_carbMin->setText(s);
      lineEdit_carbMax->setText(s);
      lineEdit_abvMin->setText(s);
      lineEdit_abvMax->setText(s);
      textEdit_profile->setText(s->profile());
      textEdit_ingredients->setText(s->ingredients());
      textEdit_examples->setText(s->examples());
      textEdit_notes->setText(s->notes());

      return;
   }

   QString propName = metaProp->name();
   QVariant val = metaProp->read(s);

   if (propName == PropertyNames::NamedEntity::name ) {
      lineEdit_name->setText(val.toString());
      tabWidget_profile->setTabText(0, s->name() );
   } else if( propName == PropertyNames::Style::category ) {
      lineEdit_category->setText(val.toString());
   } else if( propName == PropertyNames::Style::categoryNumber ) {
      lineEdit_categoryNumber->setText(val.toString());
   } else if( propName == PropertyNames::Style::styleLetter ) {
      lineEdit_styleLetter->setText(val.toString());
   } else if( propName == PropertyNames::Style::styleGuide ) {
      lineEdit_styleGuide->setText(val.toString());
   } else if( propName == PropertyNames::Style::type ) {
      comboBox_type->setCurrentIndex(val.toInt());
   } else if( propName == PropertyNames::Style::ogMin ) {
      lineEdit_ogMin->setText(val);
   } else if( propName == PropertyNames::Style::ogMax ) {
      lineEdit_ogMax->setText(val);
   } else if( propName == PropertyNames::Style::fgMin ) {
      lineEdit_fgMin->setText(val);
   } else if( propName == PropertyNames::Style::fgMax ) {
      lineEdit_fgMax->setText(val);
   } else if( propName == PropertyNames::Style::ibuMin ) {
      lineEdit_ibuMin->setText(val);
   } else if( propName == PropertyNames::Style::ibuMax ) {
      lineEdit_ibuMax->setText(val);
   } else if( propName == PropertyNames::Style::colorMin_srm ) {
      lineEdit_colorMin->setText(val);
   } else if( propName == PropertyNames::Style::colorMax_srm ) {
      lineEdit_colorMax->setText(val);
   } else if( propName == PropertyNames::Style::carbMin_vol ) {
      lineEdit_carbMin->setText(val);
   } else if( propName == PropertyNames::Style::carbMax_vol ) {
      lineEdit_carbMax->setText(val);
   } else if( propName == PropertyNames::Style::abvMin_pct ) {
      lineEdit_abvMin->setText(val);
   } else if( propName == PropertyNames::Style::abvMax_pct ) {
      lineEdit_abvMax->setText(val);
   } else if( propName == PropertyNames::Style::profile ) {
      textEdit_profile->setText(val.toString());
   } else if( propName == PropertyNames::Style::ingredients ) {
      textEdit_ingredients->setText(val.toString());
   } else if( propName == PropertyNames::Style::examples ) {
      textEdit_examples->setText(val.toString());
   } else if( propName == PropertyNames::Style::notes ) {
      textEdit_notes->setText(val.toString());
   }
   return;
}
