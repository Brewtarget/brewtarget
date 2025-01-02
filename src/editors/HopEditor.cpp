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

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_HopEditor.cpp"
#endif

// TODO: Need a separate editor for inventory

HopEditor::HopEditor(QWidget * parent, QString const editorName) :
   QDialog(parent),
   EditorBase<HopEditor, Hop, HopEditorOptions>(editorName) {
   setupUi(this);
   this->postSetupUiInit({
      //
      // Write inventory late to make sure we've the row in the inventory table (because total inventory amount isn't
      // really an attribute of the Fermentable).
      //
      // Note that we do not need to store the value of comboBox_amountType.  It merely controls the available unit for
      // lineEdit_inventory
      //
      EDITOR_FIELD_NORM(Hop, label_name              , lineEdit_name              , NamedEntity::name            ),
      EDITOR_FIELD_NORM(Hop, tab_notes               , textEdit_notes             , Hop::notes                   ),
      EDITOR_FIELD_NORM(Hop, label_alpha             , lineEdit_alpha             , Hop::alpha_pct            , 1),
      EDITOR_FIELD_NORM(Hop, label_inventory         , lineEdit_inventory         , Ingredient::totalInventory, 1, WhenToWriteField::Late),
      EDITOR_FIELD_COPQ(Hop, label_amountType        , comboBox_amountType        , Ingredient::totalInventory, lineEdit_inventory, WhenToWriteField::Never),
      EDITOR_FIELD_NORM(Hop, label_beta              , lineEdit_beta              , Hop::beta_pct             , 1),
      EDITOR_FIELD_NORM(Hop, label_HSI               , lineEdit_HSI               , Hop::hsi_pct              , 0),
      EDITOR_FIELD_NORM(Hop, label_origin            , lineEdit_origin            , Hop::origin                  ),
      EDITOR_FIELD_NORM(Hop, label_humulene          , lineEdit_humulene          , Hop::humulene_pct         , 2),
      EDITOR_FIELD_NORM(Hop, label_caryophyllene     , lineEdit_caryophyllene     , Hop::caryophyllene_pct    , 2),
      EDITOR_FIELD_NORM(Hop, label_cohumulone        , lineEdit_cohumulone        , Hop::cohumulone_pct       , 2),
      EDITOR_FIELD_NORM(Hop, label_myrcene           , lineEdit_myrcene           , Hop::myrcene_pct          , 2),
      EDITOR_FIELD_ENUM(Hop, label_type              , comboBox_hopType           , Hop::type                    ),
      EDITOR_FIELD_ENUM(Hop, label_form              , comboBox_hopForm           , Hop::form                    ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      EDITOR_FIELD_NORM(Hop, label_producer          , lineEdit_producer          , Hop::producer                ),
      EDITOR_FIELD_NORM(Hop, label_productId         , lineEdit_productId         , Hop::productId               ),
      EDITOR_FIELD_NORM(Hop, label_year              , lineEdit_year              , Hop::year                    ),
      EDITOR_FIELD_NORM(Hop, label_totalOil_mlPer100g, lineEdit_totalOil_mlPer100g, Hop::totalOil_mlPer100g      ),
      EDITOR_FIELD_NORM(Hop, label_farnesene         , lineEdit_farnesene         , Hop::farnesene_pct        , 2),
      EDITOR_FIELD_NORM(Hop, label_geraniol          , lineEdit_geraniol          , Hop::geraniol_pct         , 2),
      EDITOR_FIELD_NORM(Hop, label_bPinene           , lineEdit_bPinene           , Hop::bPinene_pct          , 2),
      EDITOR_FIELD_NORM(Hop, label_linalool          , lineEdit_linalool          , Hop::linalool_pct         , 2),
      EDITOR_FIELD_NORM(Hop, label_limonene          , lineEdit_limonene          , Hop::limonene_pct         , 2),
      EDITOR_FIELD_NORM(Hop, label_nerol             , lineEdit_nerol             , Hop::nerol_pct            , 2),
      EDITOR_FIELD_NORM(Hop, label_pinene            , lineEdit_pinene            , Hop::pinene_pct           , 2),
      EDITOR_FIELD_NORM(Hop, label_polyphenols       , lineEdit_polyphenols       , Hop::polyphenols_pct      , 2),
      EDITOR_FIELD_NORM(Hop, label_xanthohumol       , lineEdit_xanthohumol       , Hop::xanthohumol_pct      , 2),
   });
   return;
}

HopEditor::~HopEditor() = default;

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(Hop)
