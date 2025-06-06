/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/MashEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
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
#include "editors/MashEditor.h"

#include <QDebug>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MashEditor.cpp"
#endif

MashEditor::MashEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<MashEditor, Mash, MashEditorOptions>(editorName) {
   this->setupUi(this);
   this->postSetupUiInit({
      EDITOR_FIELD_NORM(Mash, label_name          , lineEdit_name             , NamedEntity::name                 ),
      EDITOR_FIELD_NORM(Mash, label_grainTemp     , lineEdit_grainTemp        , Mash::grainTemp_c              , 1),
      EDITOR_FIELD_NORM(Mash, label_spargeTemp    , lineEdit_spargeTemp       , Mash::spargeTemp_c             , 1),
      EDITOR_FIELD_NORM(Mash, label_spargePh      , lineEdit_spargePh         , Mash::ph                       , 0),
      EDITOR_FIELD_NORM(Mash, label_tunTemp       , lineEdit_tunTemp          , Mash::tunTemp_c                , 1),
      EDITOR_FIELD_NORM(Mash, label_tunMass       , lineEdit_tunMass          , Mash::mashTunWeight_kg            ),
      EDITOR_FIELD_NORM(Mash, label_tunSpHeat     , lineEdit_tunSpHeat        , Mash::mashTunSpecificHeat_calGC, 1),
      EDITOR_FIELD_NORM(Mash, label_totalMashWater, label_totalMashWater_value, Mash::totalMashWater_l         , 1),
      EDITOR_FIELD_NORM(Mash, label_totalTime     , label_totalTime_value     , Mash::totalTime_mins           , 0),
      EDITOR_FIELD_NORM(Mash, tab_notes           , textEdit_notes            , Mash::notes                       ),
   });

   return;
}

MashEditor::~MashEditor() = default;

// Insert the boilerplate stuff that we cannot do in EditorWithRecipeBase
EDITOR_COMMON_CODE(Mash)
