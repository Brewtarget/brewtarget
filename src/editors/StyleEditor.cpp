/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/StyleEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "editors/StyleEditor.h"

#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "sortFilterProxyModels/StyleSortFilterProxyModel.h"

StyleEditor::StyleEditor(QWidget* parent) :
   QDialog{parent},
   EditorBase<StyleEditor, Style>() {
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

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

   BT_COMBO_BOX_INIT(StyleEditor, comboBox_type, Style, type);

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   // [Nothing here as all new fields are text-only]

   this->connectSignalsAndSlots();
   return;
}

StyleEditor::~StyleEditor() = default;

void StyleEditor::writeFieldsToEditItem() {

   m_editItem->setName          (this->lineEdit_name          ->text                       ());
   m_editItem->setCategory      (this->lineEdit_category      ->text                       ());
   m_editItem->setCategoryNumber(this->lineEdit_categoryNumber->text                       ());
   m_editItem->setStyleLetter   (this->lineEdit_styleLetter   ->text                       ());
   m_editItem->setStyleGuide    (this->lineEdit_styleGuide    ->text                       ());
   m_editItem->setType          (this->comboBox_type          ->getNonOptValue<Style::Type>());
   m_editItem->setOgMin         (this->lineEdit_ogMin         ->getNonOptCanonicalQty      ());
   m_editItem->setOgMax         (this->lineEdit_ogMax         ->getNonOptCanonicalQty      ());
   m_editItem->setFgMin         (this->lineEdit_fgMin         ->getNonOptCanonicalQty      ());
   m_editItem->setFgMax         (this->lineEdit_fgMax         ->getNonOptCanonicalQty      ());
   m_editItem->setIbuMin        (this->lineEdit_ibuMin        ->getNonOptValue<double>     ());
   m_editItem->setIbuMax        (this->lineEdit_ibuMax        ->getNonOptValue<double>     ());
   m_editItem->setColorMin_srm  (this->lineEdit_colorMin      ->getNonOptCanonicalQty      ());
   m_editItem->setColorMax_srm  (this->lineEdit_colorMax      ->getNonOptCanonicalQty      ());
   m_editItem->setCarbMin_vol   (this->lineEdit_carbMin       ->getNonOptCanonicalQty      ());
   m_editItem->setCarbMax_vol   (this->lineEdit_carbMax       ->getNonOptCanonicalQty      ());
   m_editItem->setAbvMin_pct    (this->lineEdit_abvMin        ->getNonOptValue<double>     ());
   m_editItem->setAbvMax_pct    (this->lineEdit_abvMax        ->getNonOptValue<double>     ());
///   this->m_editItem->setProfile       (textEdit_profile       ->toPlainText                ());
   m_editItem->setIngredients   (this->textEdit_ingredients   ->toPlainText                ());
   m_editItem->setExamples      (this->textEdit_examples      ->toPlainText                ());
   m_editItem->setNotes         (this->textEdit_notes         ->toPlainText                ());
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_editItem->setAroma            (this->textEdit_aroma            ->toPlainText                ());
   m_editItem->setAppearance       (this->textEdit_appearance       ->toPlainText                ());
   m_editItem->setFlavor           (this->textEdit_flavor           ->toPlainText                ());
   m_editItem->setMouthfeel        (this->textEdit_mouthfeel        ->toPlainText                ());
   m_editItem->setOverallImpression(this->textEdit_overallImpression->toPlainText                ());

   return;
}

void StyleEditor::writeLateFieldsToEditItem() {
   // Nothing to do here for Style
   return;
}

void StyleEditor::readFieldsFromEditItem(std::optional<QString> propName) {
   if (!propName || *propName == PropertyNames::NamedEntity::name    ) { this->lineEdit_name          ->setTextCursor(m_editItem->name          ()); // Continues to next line
                                                                         /* this->tabWidget_editor->setTabText(0, m_editItem->name()); */                 if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::category      ) { lineEdit_category      ->setText   (m_editItem->category      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::categoryNumber) { lineEdit_categoryNumber->setText   (m_editItem->categoryNumber()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::styleLetter   ) { lineEdit_styleLetter   ->setText   (m_editItem->styleLetter   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::styleGuide    ) { lineEdit_styleGuide    ->setText   (m_editItem->styleGuide    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::type          ) { comboBox_type          ->setValue  (m_editItem->type          ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::ogMin         ) { lineEdit_ogMin         ->setQuantity (m_editItem->ogMin         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::ogMax         ) { lineEdit_ogMax         ->setQuantity (m_editItem->ogMax         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::fgMin         ) { lineEdit_fgMin         ->setQuantity (m_editItem->fgMin         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::fgMax         ) { lineEdit_fgMax         ->setQuantity (m_editItem->fgMax         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::ibuMin        ) { lineEdit_ibuMin        ->setQuantity (m_editItem->ibuMin        ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::ibuMax        ) { lineEdit_ibuMax        ->setQuantity (m_editItem->ibuMax        ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::colorMin_srm  ) { lineEdit_colorMin      ->setQuantity (m_editItem->colorMin_srm  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::colorMax_srm  ) { lineEdit_colorMax      ->setQuantity (m_editItem->colorMax_srm  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::carbMin_vol   ) { lineEdit_carbMin       ->setQuantity (m_editItem->carbMin_vol   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::carbMax_vol   ) { lineEdit_carbMax       ->setQuantity (m_editItem->carbMax_vol   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::abvMin_pct    ) { lineEdit_abvMin        ->setQuantity (m_editItem->abvMin_pct    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::abvMax_pct    ) { lineEdit_abvMax        ->setQuantity (m_editItem->abvMax_pct    ()); if (propName) { return; } }
///   if (!propName || *propName == PropertyNames::Style::profile       ) { textEdit_profile       ->setText   (m_editItem->profile       ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::ingredients   ) { textEdit_ingredients   ->setText   (m_editItem->ingredients   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::examples      ) { textEdit_examples      ->setText   (m_editItem->examples      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::notes         ) { textEdit_notes         ->setText   (m_editItem->notes         ()); if (propName) { return; } }
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   if (!propName || *propName == PropertyNames::Style::aroma            ) { textEdit_aroma            ->setText   (m_editItem->aroma            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::appearance       ) { textEdit_appearance       ->setText   (m_editItem->appearance       ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::flavor           ) { textEdit_flavor           ->setText   (m_editItem->flavor           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::mouthfeel        ) { textEdit_mouthfeel        ->setText   (m_editItem->mouthfeel        ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Style::overallImpression) { textEdit_overallImpression->setText   (m_editItem->overallImpression()); if (propName) { return; } }

   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_SLOT_DEFINITIONS(StyleEditor)
