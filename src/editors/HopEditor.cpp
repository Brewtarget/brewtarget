/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/HopEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#include "editors/HopEditor.h"

#include <QtGui>
#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

// TODO: Need a separate editor for inventory

HopEditor::HopEditor(QWidget * parent) :
   QDialog(parent),
   EditorBase<HopEditor, Hop>() {
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   SMART_FIELD_INIT(HopEditor, label_name              , lineEdit_name              , Hop, PropertyNames::NamedEntity::name            );
   SMART_FIELD_INIT(HopEditor, label_alpha             , lineEdit_alpha             , Hop, PropertyNames::Hop::alpha_pct            , 1);
   SMART_FIELD_INIT(HopEditor, label_inventory         , lineEdit_inventory         , Hop, PropertyNames::Ingredient::totalInventory, 1);
   SMART_FIELD_INIT(HopEditor, label_beta              , lineEdit_beta              , Hop, PropertyNames::Hop::beta_pct             , 1);
   SMART_FIELD_INIT(HopEditor, label_HSI               , lineEdit_HSI               , Hop, PropertyNames::Hop::hsi_pct              , 0);
   SMART_FIELD_INIT(HopEditor, label_origin            , lineEdit_origin            , Hop, PropertyNames::Hop::origin                  );
   SMART_FIELD_INIT(HopEditor, label_humulene          , lineEdit_humulene          , Hop, PropertyNames::Hop::humulene_pct         , 2);
   SMART_FIELD_INIT(HopEditor, label_caryophyllene     , lineEdit_caryophyllene     , Hop, PropertyNames::Hop::caryophyllene_pct    , 2);
   SMART_FIELD_INIT(HopEditor, label_cohumulone        , lineEdit_cohumulone        , Hop, PropertyNames::Hop::cohumulone_pct       , 2);
   SMART_FIELD_INIT(HopEditor, label_myrcene           , lineEdit_myrcene           , Hop, PropertyNames::Hop::myrcene_pct          , 2);
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SMART_FIELD_INIT(HopEditor, label_producer          , lineEdit_producer          , Hop, PropertyNames::Hop::producer                );
   SMART_FIELD_INIT(HopEditor, label_productId         , lineEdit_productId         , Hop, PropertyNames::Hop::productId               );
   SMART_FIELD_INIT(HopEditor, label_year              , lineEdit_year              , Hop, PropertyNames::Hop::year                    );
   SMART_FIELD_INIT(HopEditor, label_totalOil_mlPer100g, lineEdit_totalOil_mlPer100g, Hop, PropertyNames::Hop::totalOil_mlPer100g      );
   SMART_FIELD_INIT(HopEditor, label_farnesene         , lineEdit_farnesene         , Hop, PropertyNames::Hop::farnesene_pct        , 2);
   SMART_FIELD_INIT(HopEditor, label_geraniol          , lineEdit_geraniol          , Hop, PropertyNames::Hop::geraniol_pct         , 2);
   SMART_FIELD_INIT(HopEditor, label_bPinene           , lineEdit_bPinene           , Hop, PropertyNames::Hop::bPinene_pct         , 2);
   SMART_FIELD_INIT(HopEditor, label_linalool          , lineEdit_linalool          , Hop, PropertyNames::Hop::linalool_pct         , 2);
   SMART_FIELD_INIT(HopEditor, label_limonene          , lineEdit_limonene          , Hop, PropertyNames::Hop::limonene_pct         , 2);
   SMART_FIELD_INIT(HopEditor, label_nerol             , lineEdit_nerol             , Hop, PropertyNames::Hop::nerol_pct            , 2);
   SMART_FIELD_INIT(HopEditor, label_pinene            , lineEdit_pinene            , Hop, PropertyNames::Hop::pinene_pct           , 2);
   SMART_FIELD_INIT(HopEditor, label_polyphenols       , lineEdit_polyphenols       , Hop, PropertyNames::Hop::polyphenols_pct      , 2);
   SMART_FIELD_INIT(HopEditor, label_xanthohumol       , lineEdit_xanthohumol       , Hop, PropertyNames::Hop::xanthohumol_pct      , 2);

///   SMART_CHECK_BOX_INIT(HopEditor, checkBox_amountIsWeight           , label_amountIsWeight           , lineEdit_inventory , Hop, amountIsWeight           );

   BT_COMBO_BOX_INIT_COPQ(HopEditor, comboBox_amountType, Hop, PropertyNames::Ingredient::totalInventory, lineEdit_inventory);

   BT_COMBO_BOX_INIT(HopEditor, comboBox_hopType, Hop, type);
   BT_COMBO_BOX_INIT(HopEditor, comboBox_hopForm, Hop, form);

   this->connectSignalsAndSlots();
   return;
}

HopEditor::~HopEditor() = default;

void HopEditor::writeFieldsToEditItem() {
   this->m_editItem->setName             (this->lineEdit_name         ->text                  ());
   this->m_editItem->setAlpha_pct        (this->lineEdit_alpha        ->getNonOptValue<double>());
   this->m_editItem->setBeta_pct         (this->lineEdit_beta         ->getOptValue<double>   ());
   this->m_editItem->setHsi_pct          (this->lineEdit_HSI          ->getOptValue<double>   ());
   this->m_editItem->setOrigin           (this->lineEdit_origin       ->text                  ());
   this->m_editItem->setHumulene_pct     (this->lineEdit_humulene     ->getOptValue<double>   ());
   this->m_editItem->setCaryophyllene_pct(this->lineEdit_caryophyllene->getOptValue<double>   ());
   this->m_editItem->setCohumulone_pct   (this->lineEdit_cohumulone   ->getOptValue<double>   ());
   this->m_editItem->setMyrcene_pct      (this->lineEdit_myrcene      ->getOptValue<double>   ());
   this->m_editItem->setSubstitutes      (this->textEdit_substitutes  ->toPlainText           ());
   this->m_editItem->setNotes            (this->textEdit_notes        ->toPlainText           ());

   this->m_editItem->setType             (this->comboBox_hopType      ->getOptValue<Hop::Type>());
   this->m_editItem->setForm             (this->comboBox_hopForm      ->getOptValue<Hop::Form>());

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
///   this->m_editItem->setAmountIsWeight       (this->checkBox_amountIsWeight       ->isChecked          ());
   this->m_editItem->setProducer          (this->lineEdit_producer          ->text               ());
   this->m_editItem->setProductId         (this->lineEdit_productId         ->text               ());
   this->m_editItem->setYear              (this->lineEdit_year              ->text               ());
   this->m_editItem->setTotalOil_mlPer100g(this->lineEdit_totalOil_mlPer100g->getOptValue<double>());
   this->m_editItem->setFarnesene_pct     (this->lineEdit_farnesene         ->getOptValue<double>());
   this->m_editItem->setGeraniol_pct      (this->lineEdit_geraniol          ->getOptValue<double>());
   this->m_editItem->setBPinene_pct       (this->lineEdit_bPinene           ->getOptValue<double>());
   this->m_editItem->setLinalool_pct      (this->lineEdit_linalool          ->getOptValue<double>());
   this->m_editItem->setLimonene_pct      (this->lineEdit_limonene          ->getOptValue<double>());
   this->m_editItem->setNerol_pct         (this->lineEdit_nerol             ->getOptValue<double>());
   this->m_editItem->setPinene_pct        (this->lineEdit_pinene            ->getOptValue<double>());
   this->m_editItem->setPolyphenols_pct   (this->lineEdit_polyphenols       ->getOptValue<double>());
   this->m_editItem->setXanthohumol_pct   (this->lineEdit_xanthohumol       ->getOptValue<double>());
   return;
}

void HopEditor::writeLateFieldsToEditItem() {
   // Do this late to make sure we've the row in the inventory table
   this->m_editItem->setTotalInventory(lineEdit_inventory->getNonOptCanonicalAmt());
   return;
}

void HopEditor::readFieldsFromEditItem(std::optional<QString> propName) {
   if (!propName || *propName == PropertyNames::Hop::type                 ) { this->comboBox_hopType              ->setValue       (m_editItem->type                 ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::form                 ) { this->comboBox_hopForm              ->setValue       (m_editItem->form                 ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::NamedEntity::name         ) { this->lineEdit_name                 ->setTextCursor  (m_editItem->name                 ()); // Continues to next line
                                                                              this->tabWidget_editor->setTabText(0, m_editItem->name());                                  if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::origin               ) { this->lineEdit_origin               ->setTextCursor  (m_editItem->origin               ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::alpha_pct            ) { this->lineEdit_alpha                ->setQuantity    (m_editItem->alpha_pct            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::beta_pct             ) { this->lineEdit_beta                 ->setQuantity    (m_editItem->beta_pct             ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::hsi_pct              ) { this->lineEdit_HSI                  ->setQuantity    (m_editItem->hsi_pct              ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::humulene_pct         ) { this->lineEdit_humulene             ->setQuantity    (m_editItem->humulene_pct         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::caryophyllene_pct    ) { this->lineEdit_caryophyllene        ->setQuantity    (m_editItem->caryophyllene_pct    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::cohumulone_pct       ) { this->lineEdit_cohumulone           ->setQuantity    (m_editItem->cohumulone_pct       ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::myrcene_pct          ) { this->lineEdit_myrcene              ->setQuantity    (m_editItem->myrcene_pct          ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::substitutes          ) { this->textEdit_substitutes          ->setPlainText   (m_editItem->substitutes          ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::notes                ) { this->textEdit_notes                ->setPlainText   (m_editItem->notes                ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Ingredient::totalInventory) { this->lineEdit_inventory            ->setAmount      (m_editItem->totalInventory       ()); if (propName) { return; } }
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
///   if (!propName || *propName == PropertyNames::Hop::amountIsWeight              ) { this->checkBox_amountIsWeight       ->setChecked   (m_editItem->amountIsWeight       ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::producer             ) { this->lineEdit_producer             ->setText        (m_editItem->producer             ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::productId           ) { this->lineEdit_productId           ->setText        (m_editItem->productId           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::year                 ) { this->lineEdit_year                 ->setText        (m_editItem->year                 ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::totalOil_mlPer100g) { this->lineEdit_totalOil_mlPer100g->setQuantity    (m_editItem->totalOil_mlPer100g()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::farnesene_pct        ) { this->lineEdit_farnesene            ->setQuantity    (m_editItem->farnesene_pct        ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::geraniol_pct         ) { this->lineEdit_geraniol             ->setQuantity    (m_editItem->geraniol_pct         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::bPinene_pct         ) { this->lineEdit_bPinene             ->setQuantity    (m_editItem->bPinene_pct         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::linalool_pct         ) { this->lineEdit_linalool             ->setQuantity    (m_editItem->linalool_pct         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::limonene_pct         ) { this->lineEdit_limonene             ->setQuantity    (m_editItem->limonene_pct         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::nerol_pct            ) { this->lineEdit_nerol                ->setQuantity    (m_editItem->nerol_pct            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::pinene_pct           ) { this->lineEdit_pinene               ->setQuantity    (m_editItem->pinene_pct           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::polyphenols_pct      ) { this->lineEdit_polyphenols          ->setQuantity    (m_editItem->polyphenols_pct      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Hop::xanthohumol_pct      ) { this->lineEdit_xanthohumol          ->setQuantity    (m_editItem->xanthohumol_pct      ()); if (propName) { return; } }

   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_SLOT_DEFINITIONS(HopEditor)
