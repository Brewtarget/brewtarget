/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/YeastEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "editors/YeastEditor.h"

#include <cmath>

#include <QInputDialog>

#include "BtHorizontalTabs.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"

YeastEditor::YeastEditor(QWidget * parent, QString const editorName) :
   QDialog(parent),
   EditorBase<YeastEditor, Yeast, YeastEditorOptions>(editorName) {
   setupUi(this);
   this->postSetupUiInit(
      {
         //
         // Write inventory late to make sure we've the row in the inventory table (because total inventory amount isn't
         // really an attribute of the Yeast).
         //
         // Note that we do not need to store the value of comboBox_amountType.  It merely controls the available unit
         // for lineEdit_inventory
         //
         EDITOR_FIELD_NORM(Yeast, label_name          , lineEdit_name             , NamedEntity::name            ),
         EDITOR_FIELD_NORM(Yeast, label_laboratory    , lineEdit_laboratory       , Yeast::laboratory            ),
         EDITOR_FIELD_NORM(Yeast, label_inventory     , lineEdit_inventory        , Ingredient::totalInventory, 1, WhenToWriteField::Late),
         EDITOR_FIELD_COPQ(Yeast, label_amountType    , comboBox_amountType       , Ingredient::totalInventory, lineEdit_inventory, WhenToWriteField::Never),
         EDITOR_FIELD_NORM(Yeast, label_productId     , lineEdit_productId        , Yeast::productId             ),
         EDITOR_FIELD_NORM(Yeast, label_minTemperature, lineEdit_minTemperature   , Yeast::minTemperature_c, 1   ),
         EDITOR_FIELD_NORM(Yeast, label_maxTemperature, lineEdit_maxTemperature   , Yeast::maxTemperature_c, 1   ),
         EDITOR_FIELD_NORM(Yeast, label_maxReuse      , lineEdit_maxReuse         , Yeast::maxReuse              ),
         EDITOR_FIELD_ENUM(Yeast, label_type          , comboBox_yeastType        , Yeast::type                  ),
         EDITOR_FIELD_ENUM(Yeast, label_form          , comboBox_yeastForm        , Yeast::form                  ),
         EDITOR_FIELD_ENUM(Yeast, label_flocculation  , comboBox_yeastFlocculation, Yeast::flocculation          ),
         EDITOR_FIELD_NORM(Yeast, tab_bestFor         , textEdit_bestFor          , Yeast::bestFor               ),
         EDITOR_FIELD_NORM(Yeast, tab_notes           , textEdit_notes            , Yeast::notes                 ),
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         EDITOR_FIELD_NORM(Yeast, label_alcoholTolerance         , lineEdit_alcoholTolerance          , Yeast::alcoholTolerance_pct  , 1),
         EDITOR_FIELD_NORM(Yeast, label_attenuationMin           , lineEdit_attenuationMin            , Yeast::attenuationMin_pct    , 1),
         EDITOR_FIELD_NORM(Yeast, label_attenuationMax           , lineEdit_attenuationMax            , Yeast::attenuationMax_pct    , 1),
         EDITOR_FIELD_NORM(Yeast, label_phenolicOffFlavorPositive, boolCombo_phenolicOffFlavorPositive, Yeast::phenolicOffFlavorPositive),
         EDITOR_FIELD_NORM(Yeast, label_glucoamylasePositive     , boolCombo_glucoamylasePositive     , Yeast::glucoamylasePositive     ),
         EDITOR_FIELD_NORM(Yeast, label_killerProducingK1Toxin   , boolCombo_killerProducingK1Toxin   , Yeast::killerProducingK1Toxin   ),
         EDITOR_FIELD_NORM(Yeast, label_killerProducingK2Toxin   , boolCombo_killerProducingK2Toxin   , Yeast::killerProducingK2Toxin   ),
         EDITOR_FIELD_NORM(Yeast, label_killerProducingK28Toxin  , boolCombo_killerProducingK28Toxin  , Yeast::killerProducingK28Toxin  ),
         EDITOR_FIELD_NORM(Yeast, label_killerProducingKlusToxin , boolCombo_killerProducingKlusToxin , Yeast::killerProducingKlusToxin ),
         EDITOR_FIELD_NORM(Yeast, label_killerNeutral            , boolCombo_killerNeutral            , Yeast::killerNeutral            )

      }
   );

   return;
}

YeastEditor::~YeastEditor() = default;


// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(Yeast)
