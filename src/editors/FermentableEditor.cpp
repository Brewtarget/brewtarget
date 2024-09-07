/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/FermentableEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
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
#include "editors/FermentableEditor.h"

#include <QIcon>
#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

// TODO: Need a separate editor for inventory

FermentableEditor::FermentableEditor(QWidget* parent) :
   QDialog(parent),
   EditorBase<FermentableEditor, Fermentable>() {
   setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   SMART_FIELD_INIT(FermentableEditor, label_name          , lineEdit_name          , Fermentable, PropertyNames::NamedEntity::name                     );
   SMART_FIELD_INIT(FermentableEditor, label_color         , lineEdit_color         , Fermentable, PropertyNames::Fermentable::color_srm             , 0);
   SMART_FIELD_INIT(FermentableEditor, label_diastaticPower, lineEdit_diastaticPower, Fermentable, PropertyNames::Fermentable::diastaticPower_lintner   );
   SMART_FIELD_INIT(FermentableEditor, label_coarseFineDiff, lineEdit_coarseFineDiff, Fermentable, PropertyNames::Fermentable::coarseFineDiff_pct    , 0);
   SMART_FIELD_INIT(FermentableEditor, label_ibuGalPerLb   , lineEdit_ibuGalPerLb   , Fermentable, PropertyNames::Fermentable::ibuGalPerLb           , 0);
   SMART_FIELD_INIT(FermentableEditor, label_maxInBatch    , lineEdit_maxInBatch    , Fermentable, PropertyNames::Fermentable::maxInBatch_pct        , 0);
   SMART_FIELD_INIT(FermentableEditor, label_moisture      , lineEdit_moisture      , Fermentable, PropertyNames::Fermentable::moisture_pct          , 0);
   SMART_FIELD_INIT(FermentableEditor, label_protein       , lineEdit_protein       , Fermentable, PropertyNames::Fermentable::protein_pct           , 0);
   SMART_FIELD_INIT(FermentableEditor, label_inventory     , lineEdit_inventory     , Fermentable, PropertyNames::Ingredient::totalInventory         , 1);
   SMART_FIELD_INIT(FermentableEditor, label_origin        , lineEdit_origin        , Fermentable, PropertyNames::Fermentable::origin                   );
   SMART_FIELD_INIT(FermentableEditor, label_supplier      , lineEdit_supplier      , Fermentable, PropertyNames::Fermentable::supplier                 );

   BT_COMBO_BOX_INIT(FermentableEditor, comboBox_type      , Fermentable, type      );

   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   BT_COMBO_BOX_INIT(FermentableEditor, comboBox_grainGroup, Fermentable, grainGroup);

   SMART_FIELD_INIT(FermentableEditor, label_producer              , lineEdit_producer              , Fermentable, PropertyNames::Fermentable::producer                 );
   SMART_FIELD_INIT(FermentableEditor, label_productId             , lineEdit_productId             , Fermentable, PropertyNames::Fermentable::productId                );
   SMART_FIELD_INIT(FermentableEditor, label_fineGrindYield_pct    , lineEdit_fineGrindYield_pct    , Fermentable, PropertyNames::Fermentable::fineGrindYield_pct    , 1);
   SMART_FIELD_INIT(FermentableEditor, label_coarseGrindYield_pct  , lineEdit_coarseGrindYield_pct  , Fermentable, PropertyNames::Fermentable::coarseGrindYield_pct  , 1);
   SMART_FIELD_INIT(FermentableEditor, label_potentialYield_sg     , lineEdit_potentialYield_sg     , Fermentable, PropertyNames::Fermentable::potentialYield_sg        );
   SMART_FIELD_INIT(FermentableEditor, label_alphaAmylase_dextUnits, lineEdit_alphaAmylase_dextUnits, Fermentable, PropertyNames::Fermentable::alphaAmylase_dextUnits   );
   SMART_FIELD_INIT(FermentableEditor, label_kolbachIndex_pct      , lineEdit_kolbachIndex_pct      , Fermentable, PropertyNames::Fermentable::kolbachIndex_pct      , 1);
   SMART_FIELD_INIT(FermentableEditor, label_hardnessPrpGlassy_pct , lineEdit_hardnessPrpGlassy_pct , Fermentable, PropertyNames::Fermentable::hardnessPrpGlassy_pct , 1);
   SMART_FIELD_INIT(FermentableEditor, label_hardnessPrpHalf_pct   , lineEdit_hardnessPrpHalf_pct   , Fermentable, PropertyNames::Fermentable::hardnessPrpHalf_pct   , 1);
   SMART_FIELD_INIT(FermentableEditor, label_hardnessPrpMealy_pct  , lineEdit_hardnessPrpMealy_pct  , Fermentable, PropertyNames::Fermentable::hardnessPrpMealy_pct  , 1);
   SMART_FIELD_INIT(FermentableEditor, label_kernelSizePrpPlump_pct, lineEdit_kernelSizePrpPlump_pct, Fermentable, PropertyNames::Fermentable::kernelSizePrpPlump_pct, 1);
   SMART_FIELD_INIT(FermentableEditor, label_kernelSizePrpThin_pct , lineEdit_kernelSizePrpThin_pct , Fermentable, PropertyNames::Fermentable::kernelSizePrpThin_pct , 1);
   SMART_FIELD_INIT(FermentableEditor, label_friability_pct        , lineEdit_friability_pct        , Fermentable, PropertyNames::Fermentable::friability_pct        , 1);
   SMART_FIELD_INIT(FermentableEditor, label_di_ph                 , lineEdit_di_ph                 , Fermentable, PropertyNames::Fermentable::di_ph                 , 1);
   SMART_FIELD_INIT(FermentableEditor, label_viscosity_cP          , lineEdit_viscosity_cP          , Fermentable, PropertyNames::Fermentable::viscosity_cP             );
   SMART_FIELD_INIT(FermentableEditor, label_dmsP                  , lineEdit_dmsP                  , Fermentable, PropertyNames::Fermentable::dmsP_ppm              , 1);
   SMART_FIELD_INIT(FermentableEditor, label_fan                   , lineEdit_fan                   , Fermentable, PropertyNames::Fermentable::fan_ppm               , 1);
   SMART_FIELD_INIT(FermentableEditor, label_fermentability_pct    , lineEdit_fermentability_pct    , Fermentable, PropertyNames::Fermentable::fermentability_pct    , 1);
   SMART_FIELD_INIT(FermentableEditor, label_betaGlucan            , lineEdit_betaGlucan            , Fermentable, PropertyNames::Fermentable::betaGlucan_ppm        , 1);

///   SMART_CHECK_BOX_INIT(FermentableEditor, checkBox_amountIsWeight           , label_amountIsWeight           , lineEdit_inventory , Fermentable, amountIsWeight           );

   BT_COMBO_BOX_INIT_COPQ(FermentableEditor, comboBox_amountType, Fermentable, PropertyNames::Ingredient::totalInventory, lineEdit_inventory);

   this->connectSignalsAndSlots();
   return;
}

FermentableEditor::~FermentableEditor() = default;

void FermentableEditor::writeFieldsToEditItem() {
   this->m_editItem->setType(this->comboBox_type      ->getNonOptValue<Fermentable::Type      >());

   this->m_editItem->setName                  (this->lineEdit_name          ->text                 ());
   this->m_editItem->setColor_srm             (this->lineEdit_color         ->getNonOptCanonicalQty());
   this->m_editItem->setOrigin                (this->lineEdit_origin        ->text                 ());
   this->m_editItem->setSupplier              (this->lineEdit_supplier      ->text                 ());
   this->m_editItem->setCoarseFineDiff_pct    (this->lineEdit_coarseFineDiff->getOptValue<double>  ());
   this->m_editItem->setMoisture_pct          (this->lineEdit_moisture      ->getOptValue<double>  ());
   this->m_editItem->setDiastaticPower_lintner(this->lineEdit_diastaticPower->getOptCanonicalQty   ());
   this->m_editItem->setProtein_pct           (this->lineEdit_protein       ->getOptValue<double>  ());
   // See below for call to setTotalInventory, which needs to be done "late"
   this->m_editItem->setMaxInBatch_pct        (this->lineEdit_maxInBatch    ->getOptValue<double>  ());
   this->m_editItem->setRecommendMash         (this->checkBox_recommendMash ->checkState() == Qt::Checked);
   this->m_editItem->setIbuGalPerLb           (this->lineEdit_ibuGalPerLb   ->getOptValue<double>()); // .:TBD:. No metric measure?
   this->m_editItem->setNotes                 (this->textEdit_notes         ->toPlainText           ());
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   this->m_editItem->setGrainGroup               (this->comboBox_grainGroup               ->getOptValue<Fermentable::GrainGroup>());
   this->m_editItem->setProducer                 (this->lineEdit_producer                 ->text               ());
   this->m_editItem->setProductId                (this->lineEdit_productId                ->text               ());
   this->m_editItem->setFineGrindYield_pct       (this->lineEdit_fineGrindYield_pct       ->getOptValue<double>());
   this->m_editItem->setCoarseGrindYield_pct     (this->lineEdit_coarseGrindYield_pct     ->getOptValue<double>());
   this->m_editItem->setPotentialYield_sg        (this->lineEdit_potentialYield_sg        ->getOptValue<double>());
   this->m_editItem->setAlphaAmylase_dextUnits   (this->lineEdit_alphaAmylase_dextUnits   ->getOptValue<double>());
   this->m_editItem->setKolbachIndex_pct         (this->lineEdit_kolbachIndex_pct         ->getOptValue<double>());
   this->m_editItem->setHardnessPrpGlassy_pct    (this->lineEdit_hardnessPrpGlassy_pct    ->getOptValue<double>());
   this->m_editItem->setHardnessPrpHalf_pct      (this->lineEdit_hardnessPrpHalf_pct      ->getOptValue<double>());
   this->m_editItem->setHardnessPrpMealy_pct     (this->lineEdit_hardnessPrpMealy_pct     ->getOptValue<double>());
   this->m_editItem->setKernelSizePrpPlump_pct   (this->lineEdit_kernelSizePrpPlump_pct   ->getOptValue<double>());
   this->m_editItem->setKernelSizePrpThin_pct    (this->lineEdit_kernelSizePrpThin_pct    ->getOptValue<double>());
   this->m_editItem->setFriability_pct           (this->lineEdit_friability_pct           ->getOptValue<double>());
   this->m_editItem->setDi_ph                    (this->lineEdit_di_ph                    ->getOptValue<double>());
   this->m_editItem->setViscosity_cP             (this->lineEdit_viscosity_cP             ->getOptValue<double>());
   this->m_editItem->setDmsP_ppm                 (this->lineEdit_dmsP                     ->getOptCanonicalQty());
   this->m_editItem->setFan_ppm                  (this->lineEdit_fan                      ->getOptCanonicalQty());
   this->m_editItem->setFermentability_pct       (this->lineEdit_fermentability_pct       ->getOptValue<double>());
   this->m_editItem->setBetaGlucan_ppm           (this->lineEdit_betaGlucan               ->getOptCanonicalQty());

   return;
}

void FermentableEditor::writeLateFieldsToEditItem() {
   //
   // Do this late to make sure we've the row in the inventory table (because total inventory amount isn't really an
   // attribute of the Fermentable).
   //
   // Note that we do not need to store the value of comboBox_amountType.  It merely controls the available unit for
   // lineEdit_inventory
   //
   // Note that, if the inventory field is blank, we'll treat that as meaning "don't change the inventory"
   //
   if (!this->lineEdit_inventory->isEmptyOrBlank()) {
      this->m_editItem->setTotalInventory(lineEdit_inventory->getNonOptCanonicalAmt());
   }
   return;
}

void FermentableEditor::readFieldsFromEditItem(std::optional<QString> propName) {
   if (!propName || *propName == PropertyNames::NamedEntity::name                  ) { this->lineEdit_name          ->setTextCursor(m_editItem->name                  ()); // Continues to next line
                                                                                       this->tabWidget_editor->setTabText(0, m_editItem->name());                                               if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::type                  ) { this->comboBox_type          ->setValue     (m_editItem->type                  ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::color_srm             ) { this->lineEdit_color         ->setQuantity  (m_editItem->color_srm             ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::origin                ) { this->lineEdit_origin        ->setTextCursor(m_editItem->origin                ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::supplier              ) { this->lineEdit_supplier      ->setTextCursor(m_editItem->supplier              ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::coarseFineDiff_pct    ) { this->lineEdit_coarseFineDiff->setQuantity  (m_editItem->coarseFineDiff_pct    ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::moisture_pct          ) { this->lineEdit_moisture      ->setQuantity  (m_editItem->moisture_pct          ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::diastaticPower_lintner) { this->lineEdit_diastaticPower->setQuantity  (m_editItem->diastaticPower_lintner());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::protein_pct           ) { this->lineEdit_protein       ->setQuantity  (m_editItem->protein_pct           ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Ingredient::totalInventory         ) { this->lineEdit_inventory     ->setAmount    (m_editItem->totalInventory        ());
                                                                                       this->comboBox_amountType    ->autoSetFromControlledField();
                                                                                       if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::maxInBatch_pct        ) { this->lineEdit_maxInBatch    ->setQuantity  (m_editItem->maxInBatch_pct        ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::recommendMash         ) { this->checkBox_recommendMash ->setCheckState(m_editItem->recommendMash() ? Qt::Checked : Qt::Unchecked); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::ibuGalPerLb           ) { this->lineEdit_ibuGalPerLb   ->setQuantity  (m_editItem->ibuGalPerLb           ());                      if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::notes                 ) { this->textEdit_notes         ->setPlainText (m_editItem->notes                 ());                      if (propName) { return; } }
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   if (!propName || *propName == PropertyNames::Fermentable::grainGroup               ) { this->comboBox_grainGroup               ->setValue     (m_editItem->grainGroup               ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::producer                 ) { this->lineEdit_producer                 ->setTextCursor(m_editItem->producer                 ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::productId                ) { this->lineEdit_productId                ->setTextCursor(m_editItem->productId                ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::fineGrindYield_pct       ) { this->lineEdit_fineGrindYield_pct       ->setQuantity    (m_editItem->fineGrindYield_pct       ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::coarseGrindYield_pct     ) { this->lineEdit_coarseGrindYield_pct     ->setQuantity    (m_editItem->coarseGrindYield_pct     ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::potentialYield_sg        ) { this->lineEdit_potentialYield_sg        ->setQuantity    (m_editItem->potentialYield_sg        ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::alphaAmylase_dextUnits   ) { this->lineEdit_alphaAmylase_dextUnits   ->setQuantity    (m_editItem->alphaAmylase_dextUnits   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::kolbachIndex_pct         ) { this->lineEdit_kolbachIndex_pct         ->setQuantity    (m_editItem->kolbachIndex_pct         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::hardnessPrpGlassy_pct    ) { this->lineEdit_hardnessPrpGlassy_pct    ->setQuantity    (m_editItem->hardnessPrpGlassy_pct    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::hardnessPrpHalf_pct      ) { this->lineEdit_hardnessPrpHalf_pct      ->setQuantity    (m_editItem->hardnessPrpHalf_pct      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::hardnessPrpMealy_pct     ) { this->lineEdit_hardnessPrpMealy_pct     ->setQuantity    (m_editItem->hardnessPrpMealy_pct     ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::kernelSizePrpPlump_pct   ) { this->lineEdit_kernelSizePrpPlump_pct   ->setQuantity    (m_editItem->kernelSizePrpPlump_pct   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::kernelSizePrpThin_pct    ) { this->lineEdit_kernelSizePrpThin_pct    ->setQuantity    (m_editItem->kernelSizePrpThin_pct    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::friability_pct           ) { this->lineEdit_friability_pct           ->setQuantity    (m_editItem->friability_pct           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::di_ph                    ) { this->lineEdit_di_ph                    ->setQuantity    (m_editItem->di_ph                    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::viscosity_cP             ) { this->lineEdit_viscosity_cP             ->setQuantity    (m_editItem->viscosity_cP             ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::dmsP_ppm                 ) { this->lineEdit_dmsP                     ->setQuantity    (m_editItem->dmsP_ppm                     ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::fan_ppm                  ) { this->lineEdit_fan                      ->setQuantity    (m_editItem->fan_ppm                      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::fermentability_pct       ) { this->lineEdit_fermentability_pct       ->setQuantity    (m_editItem->fermentability_pct       ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Fermentable::betaGlucan_ppm           ) { this->lineEdit_betaGlucan               ->setQuantity    (m_editItem->betaGlucan_ppm               ()); if (propName) { return; } }

   this->label_id_value->setText(QString::number(m_editItem->key()));
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_SLOT_DEFINITIONS(FermentableEditor)
