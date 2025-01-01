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
#include "qtModels/sortFilterProxyModels/StyleSortFilterProxyModel.h"

StyleEditor::StyleEditor(QWidget* parent, QString const editorName) :
   QDialog{parent},
   EditorBase<StyleEditor, Style, StyleEditorOptions>(editorName) {
   setupUi(this);
   this->postSetupUiInit({
      // Note that the Min / Max pairs of entry fields each share a label (which is shown to the left of both fields)
      EDITOR_FIELD_NORM(Style, label_name             , lineEdit_name             , NamedEntity::name       ),
      EDITOR_FIELD_NORM(Style, label_category         , lineEdit_category         , Style::category         ),
      EDITOR_FIELD_NORM(Style, label_categoryNumber   , lineEdit_categoryNumber   , Style::categoryNumber   ),
      EDITOR_FIELD_NORM(Style, label_styleLetter      , lineEdit_styleLetter      , Style::styleLetter      ),
      EDITOR_FIELD_NORM(Style, label_styleGuide       , lineEdit_styleGuide       , Style::styleGuide       ),
      EDITOR_FIELD_NORM(Style, label_og               , lineEdit_ogMin            , Style::ogMin            ),
      EDITOR_FIELD_NORM(Style, label_og               , lineEdit_ogMax            , Style::ogMax            ),
      EDITOR_FIELD_NORM(Style, label_fg               , lineEdit_fgMin            , Style::fgMin            ),
      EDITOR_FIELD_NORM(Style, label_fg               , lineEdit_fgMax            , Style::fgMax            ),
      EDITOR_FIELD_NORM(Style, label_ibu              , lineEdit_ibuMin           , Style::ibuMin        , 0),
      EDITOR_FIELD_NORM(Style, label_ibu              , lineEdit_ibuMax           , Style::ibuMax        , 0),
      EDITOR_FIELD_NORM(Style, label_color            , lineEdit_colorMin         , Style::colorMin_srm     ),
      EDITOR_FIELD_NORM(Style, label_color            , lineEdit_colorMax         , Style::colorMax_srm     ),
      EDITOR_FIELD_NORM(Style, label_carb             , lineEdit_carbMin          , Style::carbMin_vol   , 0),
      EDITOR_FIELD_NORM(Style, label_carb             , lineEdit_carbMax          , Style::carbMax_vol   , 0),
      EDITOR_FIELD_NORM(Style, label_abv              , lineEdit_abvMin           , Style::abvMin_pct    , 1),
      EDITOR_FIELD_NORM(Style, label_abv              , lineEdit_abvMax           , Style::abvMax_pct    , 1),
      EDITOR_FIELD_ENUM(Style, label_type             , comboBox_type             , Style::type             ),
      EDITOR_FIELD_NORM(Style, tab_ingredients        , textEdit_ingredients      , Style::ingredients      ),
      EDITOR_FIELD_NORM(Style, tab_examples           , textEdit_examples         , Style::examples         ),
      EDITOR_FIELD_NORM(Style, tab_notes              , textEdit_notes            , Style::notes            ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      EDITOR_FIELD_NORM(Style, tab_aroma              , textEdit_aroma            , Style::aroma            ),
      EDITOR_FIELD_NORM(Style, tab_appearance         , textEdit_appearance       , Style::appearance       ),
      EDITOR_FIELD_NORM(Style, tab_flavor             , textEdit_flavor           , Style::flavor           ),
      EDITOR_FIELD_NORM(Style, tab_mouthfeel          , textEdit_mouthfeel        , Style::mouthfeel        ),
      EDITOR_FIELD_NORM(Style, tab_overallImpression  , textEdit_overallImpression, Style::overallImpression),
   });

   // EditorBase doesn't do this for us because we don't put the style name in the tab (possibly because it can be quite
   // long).
   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);
   return;
}

StyleEditor::~StyleEditor() = default;

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(Style)
