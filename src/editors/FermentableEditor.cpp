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

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

// TODO: Need a separate editor for inventory

FermentableEditor::FermentableEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<FermentableEditor, Fermentable, FermentableEditorOptions>(editorName) {
   setupUi(this);
   this->postSetupUiInit({
      //
      // Write inventory late to make sure we've the row in the inventory table (because total inventory amount isn't
      // really an attribute of the Fermentable).
      //
      // Note that we do not need to store the value of comboBox_amountType.  It merely controls the available unit for
      // lineEdit_inventory
      //
      EDITOR_FIELD_NORM(Fermentable, label_name                  , lineEdit_name                  , NamedEntity::name                     ),
      EDITOR_FIELD_NORM(Fermentable, tab_notes                   , textEdit_notes                 , Fermentable::notes                    ),
      EDITOR_FIELD_NORM(Fermentable, label_color                 , lineEdit_color                 , Fermentable::color_srm             , 0),
      EDITOR_FIELD_NORM(Fermentable, label_diastaticPower        , lineEdit_diastaticPower        , Fermentable::diastaticPower_lintner   ),
      EDITOR_FIELD_NORM(Fermentable, label_coarseFineDiff        , lineEdit_coarseFineDiff        , Fermentable::coarseFineDiff_pct    , 0),
      EDITOR_FIELD_NORM(Fermentable, label_ibuGalPerLb           , lineEdit_ibuGalPerLb           , Fermentable::ibuGalPerLb           , 0),
      EDITOR_FIELD_NORM(Fermentable, label_maxInBatch            , lineEdit_maxInBatch            , Fermentable::maxInBatch_pct        , 0),
      EDITOR_FIELD_NORM(Fermentable, label_moisture              , lineEdit_moisture              , Fermentable::moisture_pct          , 0),
      EDITOR_FIELD_NORM(Fermentable, label_protein               , lineEdit_protein               , Fermentable::protein_pct           , 0),
      EDITOR_FIELD_NORM(Fermentable, label_inventory             , lineEdit_inventory             , Ingredient::totalInventory         , 1, WhenToWriteField::Late),
      EDITOR_FIELD_COPQ(Fermentable, label_amountType            , comboBox_amountType            , Ingredient::totalInventory, lineEdit_inventory, WhenToWriteField::Never),
      EDITOR_FIELD_NORM(Fermentable, label_origin                , lineEdit_origin                , Fermentable::origin                   ),
      EDITOR_FIELD_NORM(Fermentable, label_supplier              , lineEdit_supplier              , Fermentable::supplier                 ),
      EDITOR_FIELD_ENUM(Fermentable, label_fermentableType       , comboBox_type                  , Fermentable::type                     ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      EDITOR_FIELD_ENUM(Fermentable, label_grainGroup            , comboBox_grainGroup            , Fermentable::grainGroup               ),
      EDITOR_FIELD_NORM(Fermentable, label_producer              , lineEdit_producer              , Fermentable::producer                 ),
      EDITOR_FIELD_NORM(Fermentable, label_productId             , lineEdit_productId             , Fermentable::productId                ),
      EDITOR_FIELD_NORM(Fermentable, label_fineGrindYield_pct    , lineEdit_fineGrindYield_pct    , Fermentable::fineGrindYield_pct    , 1),
      EDITOR_FIELD_NORM(Fermentable, label_coarseGrindYield_pct  , lineEdit_coarseGrindYield_pct  , Fermentable::coarseGrindYield_pct  , 1),
      EDITOR_FIELD_NORM(Fermentable, label_potentialYield_sg     , lineEdit_potentialYield_sg     , Fermentable::potentialYield_sg        ),
      EDITOR_FIELD_NORM(Fermentable, label_alphaAmylase_dextUnits, lineEdit_alphaAmylase_dextUnits, Fermentable::alphaAmylase_dextUnits   ),
      EDITOR_FIELD_NORM(Fermentable, label_kolbachIndex_pct      , lineEdit_kolbachIndex_pct      , Fermentable::kolbachIndex_pct      , 1),
      EDITOR_FIELD_NORM(Fermentable, label_hardnessPrpGlassy_pct , lineEdit_hardnessPrpGlassy_pct , Fermentable::hardnessPrpGlassy_pct , 1),
      EDITOR_FIELD_NORM(Fermentable, label_hardnessPrpHalf_pct   , lineEdit_hardnessPrpHalf_pct   , Fermentable::hardnessPrpHalf_pct   , 1),
      EDITOR_FIELD_NORM(Fermentable, label_hardnessPrpMealy_pct  , lineEdit_hardnessPrpMealy_pct  , Fermentable::hardnessPrpMealy_pct  , 1),
      EDITOR_FIELD_NORM(Fermentable, label_kernelSizePrpPlump_pct, lineEdit_kernelSizePrpPlump_pct, Fermentable::kernelSizePrpPlump_pct, 1),
      EDITOR_FIELD_NORM(Fermentable, label_kernelSizePrpThin_pct , lineEdit_kernelSizePrpThin_pct , Fermentable::kernelSizePrpThin_pct , 1),
      EDITOR_FIELD_NORM(Fermentable, label_friability_pct        , lineEdit_friability_pct        , Fermentable::friability_pct        , 1),
      EDITOR_FIELD_NORM(Fermentable, label_di_ph                 , lineEdit_di_ph                 , Fermentable::di_ph                 , 1),
      EDITOR_FIELD_NORM(Fermentable, label_viscosity_cP          , lineEdit_viscosity_cP          , Fermentable::viscosity_cP             ),
      EDITOR_FIELD_NORM(Fermentable, label_dmsP                  , lineEdit_dmsP                  , Fermentable::dmsP_ppm              , 1),
      EDITOR_FIELD_NORM(Fermentable, label_fan                   , lineEdit_fan                   , Fermentable::fan_ppm               , 1),
      EDITOR_FIELD_NORM(Fermentable, label_fermentability_pct    , lineEdit_fermentability_pct    , Fermentable::fermentability_pct    , 1),
      EDITOR_FIELD_NORM(Fermentable, label_betaGlucan            , lineEdit_betaGlucan            , Fermentable::betaGlucan_ppm        , 1),
   });
   return;
}

FermentableEditor::~FermentableEditor() = default;

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(Fermentable)
