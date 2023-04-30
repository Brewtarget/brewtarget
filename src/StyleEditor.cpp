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

   this->styleListModel = new StyleListModel(styleComboBox);
   this->styleProxyModel = new StyleSortFilterProxyModel(styleComboBox);
   this->styleProxyModel->setDynamicSortFilter(true);
   this->styleProxyModel->setSourceModel(styleListModel);
   this->styleComboBox->setModel(styleProxyModel);

   // Note that the Min / Max pairs of entry fields each share a label (which is shown to the left of both fields)
   SMART_FIELD_INIT(StyleEditor, label_name          , lineEdit_name          , Style, PropertyNames::NamedEntity::name       );
   SMART_FIELD_INIT(StyleEditor, label_category      , lineEdit_category      , Style, PropertyNames::Style::category         );
   SMART_FIELD_INIT(StyleEditor, label_categoryNumber, lineEdit_categoryNumber, Style, PropertyNames::Style::categoryNumber   );
   SMART_FIELD_INIT(StyleEditor, label_styleLetter   , lineEdit_styleLetter   , Style, PropertyNames::Style::styleLetter      );
   SMART_FIELD_INIT(StyleEditor, label_styleGuide    , lineEdit_styleGuide    , Style, PropertyNames::Style::styleGuide       );
   SMART_FIELD_INIT(StyleEditor, label_og            , lineEdit_ogMin         , Style, PropertyNames::Style::ogMin            );
   SMART_FIELD_INIT(StyleEditor, label_og            , lineEdit_ogMax         , Style, PropertyNames::Style::ogMax            );
   SMART_FIELD_INIT(StyleEditor, label_fg            , lineEdit_fgMin         , Style, PropertyNames::Style::fgMin            );
   SMART_FIELD_INIT(StyleEditor, label_fg            , lineEdit_fgMax         , Style, PropertyNames::Style::fgMax            );
   SMART_FIELD_INIT(StyleEditor, label_ibu           , lineEdit_ibuMin        , Style, PropertyNames::Style::ibuMin        , 0);
   SMART_FIELD_INIT(StyleEditor, label_ibu           , lineEdit_ibuMax        , Style, PropertyNames::Style::ibuMax        , 0);
   SMART_FIELD_INIT(StyleEditor, label_color         , lineEdit_colorMin      , Style, PropertyNames::Style::colorMin_srm     );
   SMART_FIELD_INIT(StyleEditor, label_color         , lineEdit_colorMax      , Style, PropertyNames::Style::colorMax_srm     );
   SMART_FIELD_INIT(StyleEditor, label_carb          , lineEdit_carbMin       , Style, PropertyNames::Style::carbMin_vol   , 0);
   SMART_FIELD_INIT(StyleEditor, label_carb          , lineEdit_carbMax       , Style, PropertyNames::Style::carbMax_vol   , 0);
   SMART_FIELD_INIT(StyleEditor, label_abv           , lineEdit_abvMin        , Style, PropertyNames::Style::abvMin_pct    , 1);
   SMART_FIELD_INIT(StyleEditor, label_abv           , lineEdit_abvMax        , Style, PropertyNames::Style::abvMax_pct    , 1);

   // Note, per https://wiki.qt.io/New_Signal_Slot_Syntax#Default_arguments_in_slot, the use of a trivial lambda
   // function to allow use of default argument on newStyle() slot
   connect(this->pushButton_save  , &QAbstractButton::clicked     , this, &StyleEditor::save                     );
   connect(this->pushButton_new   , &QAbstractButton::clicked     , this, [this]() { this->newStyle(); return; } );
   connect(this->pushButton_cancel, &QAbstractButton::clicked     , this, &StyleEditor::clearAndClose            );
   connect(this->pushButton_remove, &QAbstractButton::clicked     , this, &StyleEditor::removeStyle              );
   connect(this->styleComboBox    , &QComboBox::currentTextChanged, this, &StyleEditor::styleSelected            );

   this->setStyle(styleListModel->at(styleComboBox->currentIndex()));
   return;
}

StyleEditor::~StyleEditor() = default;

void StyleEditor::setStyle( Style* s ) {
   if (this->obsStyle) {
      disconnect(this->obsStyle, 0, this, 0);
   }

   this->obsStyle = s;
   if (this->obsStyle) {
      connect(this->obsStyle, &NamedEntity::changed, this, &StyleEditor::changed);
      qDebug() << Q_FUNC_INFO << "Editing style #" << this->obsStyle->key() << ":" << this->obsStyle->name();
      showChanges();
   }

   styleComboBox->setCurrentIndex(styleListModel->indexOf(this->obsStyle));
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
   qDebug() << Q_FUNC_INFO;
   if (!this->obsStyle) {
      setVisible(false);
      return;
   }

   this->obsStyle->setName          (lineEdit_name          ->text()                        );
   this->obsStyle->setCategory      (lineEdit_category      ->text()                        );
   this->obsStyle->setCategoryNumber(lineEdit_categoryNumber->text()                        );
   this->obsStyle->setStyleLetter   (lineEdit_styleLetter   ->text()                        );
   this->obsStyle->setStyleGuide    (lineEdit_styleGuide    ->text()                        );
   this->obsStyle->setType          (static_cast<Style::Type>(comboBox_type->currentIndex()));
   this->obsStyle->setOgMin         (lineEdit_ogMin         ->toCanonical().quantity()      );
   this->obsStyle->setOgMax         (lineEdit_ogMax         ->toCanonical().quantity()      );
   this->obsStyle->setFgMin         (lineEdit_fgMin         ->toCanonical().quantity()      );
   this->obsStyle->setFgMax         (lineEdit_fgMax         ->toCanonical().quantity()      );
   this->obsStyle->setIbuMin        (lineEdit_ibuMin        ->getValueAs<double>()          );
   this->obsStyle->setIbuMax        (lineEdit_ibuMax        ->getValueAs<double>()          );
   this->obsStyle->setColorMin_srm  (lineEdit_colorMin      ->toCanonical().quantity()      );
   this->obsStyle->setColorMax_srm  (lineEdit_colorMax      ->toCanonical().quantity()      );
   this->obsStyle->setCarbMin_vol   (lineEdit_carbMin       ->toCanonical().quantity()      );
   this->obsStyle->setCarbMax_vol   (lineEdit_carbMax       ->toCanonical().quantity()      );
   this->obsStyle->setAbvMin_pct    (lineEdit_abvMin        ->getValueAs<double>()          );
   this->obsStyle->setAbvMax_pct    (lineEdit_abvMax        ->getValueAs<double>()          );
   this->obsStyle->setProfile       (textEdit_profile       ->toPlainText()                 );
   this->obsStyle->setIngredients   (textEdit_ingredients   ->toPlainText()                 );
   this->obsStyle->setExamples      (textEdit_examples      ->toPlainText()                 );
   this->obsStyle->setNotes         (textEdit_notes         ->toPlainText()                 );

   if (this->obsStyle->key() < 0) {
      ObjectStoreWrapper::insert(*this->obsStyle);
   }

   setVisible(false);
   return;
}

void StyleEditor::newStyle(QString folder) {
   QString name = QInputDialog::getText(this, tr("Style name"), tr("Style name:"));
   if (name.isEmpty()) {
      return;
   }

   Style *s = new Style(name);
   if (!folder.isEmpty()) {
      s->setFolder(folder);
   }

   this->setStyle(s);
   this->show();
   return;
}

void StyleEditor::clearAndClose() {
   this->setVisible(false);
   return;
}

void StyleEditor::changed(QMetaProperty const property, QVariant const value) {
   qDebug() << Q_FUNC_INFO << property.name() << "=" << value;
   this->showChanges(&property);
   return;
}

void StyleEditor::clear() {
   lineEdit_name          ->setText(QString(""));
   lineEdit_category      ->setText(QString(""));
   lineEdit_categoryNumber->setText(QString(""));
   lineEdit_styleLetter   ->setText(QString(""));
   lineEdit_styleGuide    ->setText(QString(""));
   lineEdit_ogMin         ->setText(QString(""));
   lineEdit_ogMax         ->setText(QString(""));
   lineEdit_fgMin         ->setText(QString(""));
   lineEdit_fgMax         ->setText(QString(""));
   lineEdit_ibuMin        ->setText(QString(""));
   lineEdit_ibuMax        ->setText(QString(""));
   lineEdit_colorMin      ->setText(QString(""));
   lineEdit_colorMax      ->setText(QString(""));
   lineEdit_carbMin       ->setText(QString(""));
   lineEdit_carbMax       ->setText(QString(""));
   lineEdit_abvMin        ->setText(QString(""));
   lineEdit_abvMax        ->setText(QString(""));
   textEdit_profile       ->setText(QString(""));
   textEdit_ingredients   ->setText(QString(""));
   textEdit_examples      ->setText(QString(""));
   textEdit_notes         ->setText(QString(""));
   return;
}

void StyleEditor::showChanges(QMetaProperty const * metaProp) {
   if (!this->obsStyle) {
      this->clear();
      return;
   }

   bool updateAll = true;
   QString propName;
   if (metaProp) {
      updateAll = false;
      propName = metaProp->name();
//   QVariant val = metaProp->read(this->obsStyle);
   }

   if (updateAll || propName == PropertyNames::NamedEntity::name    ) { lineEdit_name          ->setText   (this->obsStyle->name          ()); // Continues to next line
                                                                        tabWidget_profile      ->setTabText(0, this->obsStyle->name       ()); }
   if (updateAll || propName == PropertyNames::Style::category      ) { lineEdit_category      ->setText   (this->obsStyle->category      ()); }
   if (updateAll || propName == PropertyNames::Style::categoryNumber) { lineEdit_categoryNumber->setText   (this->obsStyle->categoryNumber()); }
   if (updateAll || propName == PropertyNames::Style::styleLetter   ) { lineEdit_styleLetter   ->setText   (this->obsStyle->styleLetter   ()); }
   if (updateAll || propName == PropertyNames::Style::styleGuide    ) { lineEdit_styleGuide    ->setText   (this->obsStyle->styleGuide    ()); }
   if (updateAll || propName == PropertyNames::Style::type          ) { comboBox_type          ->setCurrentIndex(static_cast<int>(this->obsStyle->type())); }
   if (updateAll || propName == PropertyNames::Style::ogMin         ) { lineEdit_ogMin         ->setAmount (this->obsStyle->ogMin         ()); }
   if (updateAll || propName == PropertyNames::Style::ogMax         ) { lineEdit_ogMax         ->setAmount (this->obsStyle->ogMax         ()); }
   if (updateAll || propName == PropertyNames::Style::fgMin         ) { lineEdit_fgMin         ->setAmount (this->obsStyle->fgMin         ()); }
   if (updateAll || propName == PropertyNames::Style::fgMax         ) { lineEdit_fgMax         ->setAmount (this->obsStyle->fgMax         ()); }
   if (updateAll || propName == PropertyNames::Style::ibuMin        ) { lineEdit_ibuMin        ->setAmount (this->obsStyle->ibuMin        ()); }
   if (updateAll || propName == PropertyNames::Style::ibuMax        ) { lineEdit_ibuMax        ->setAmount (this->obsStyle->ibuMax        ()); }
   if (updateAll || propName == PropertyNames::Style::colorMin_srm  ) { lineEdit_colorMin      ->setAmount (this->obsStyle->colorMin_srm  ()); }
   if (updateAll || propName == PropertyNames::Style::colorMax_srm  ) { lineEdit_colorMax      ->setAmount (this->obsStyle->colorMax_srm  ()); }
   if (updateAll || propName == PropertyNames::Style::carbMin_vol   ) { lineEdit_carbMin       ->setAmount (this->obsStyle->carbMin_vol   ()); }
   if (updateAll || propName == PropertyNames::Style::carbMax_vol   ) { lineEdit_carbMax       ->setAmount (this->obsStyle->carbMax_vol   ()); }
   if (updateAll || propName == PropertyNames::Style::abvMin_pct    ) { lineEdit_abvMin        ->setAmount (this->obsStyle->abvMin_pct    ()); }
   if (updateAll || propName == PropertyNames::Style::abvMax_pct    ) { lineEdit_abvMax        ->setAmount (this->obsStyle->abvMax_pct    ()); }
   if (updateAll || propName == PropertyNames::Style::profile       ) { textEdit_profile       ->setText   (this->obsStyle->profile       ()); }
   if (updateAll || propName == PropertyNames::Style::ingredients   ) { textEdit_ingredients   ->setText   (this->obsStyle->ingredients   ()); }
   if (updateAll || propName == PropertyNames::Style::examples      ) { textEdit_examples      ->setText   (this->obsStyle->examples      ()); }
   if (updateAll || propName == PropertyNames::Style::notes         ) { textEdit_notes         ->setText   (this->obsStyle->notes         ()); }
   return;
}
