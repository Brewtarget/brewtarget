/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/WaterEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Jeff Bailey <skydvr38@verizon.net>
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
#include "editors/WaterEditor.h"

#include <QDebug>
#include <QInputDialog>

#include "database/ObjectStoreWrapper.h"
#include "model/Water.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_WaterEditor.cpp"
#endif

namespace {
   auto const seriesNameCurrent {WaterEditor::tr("Current" )};
   auto const seriesNameModified{WaterEditor::tr("Modified")};
}

WaterEditor::WaterEditor(QWidget *parent, QString const editorName) :
   QDialog(parent),
   EditorBase<WaterEditor, Water, WaterEditorOptions>(editorName) {
   this->setupUi(this);
   this->postSetupUiInit(
      {
       EDITOR_FIELD_NORM(Water, label_name            , lineEdit_name             , NamedEntity::name      ),
       EDITOR_FIELD_NORM(Water, label_notes           , textEdit_notes            , Water::notes           ),
       EDITOR_FIELD_NORM(Water, label_ca              , lineEdit_ca               , Water::calcium_ppm     , 2),
       EDITOR_FIELD_NORM(Water, label_cl              , lineEdit_cl               , Water::chloride_ppm    , 2),
       EDITOR_FIELD_NORM(Water, label_mg              , lineEdit_mg               , Water::magnesium_ppm   , 2),
       EDITOR_FIELD_NORM(Water, label_so4             , lineEdit_so4              , Water::sulfate_ppm     , 2),
       EDITOR_FIELD_NORM(Water, label_na              , lineEdit_na               , Water::sodium_ppm      , 2),
       EDITOR_FIELD_NORM(Water, label_alk             , lineEdit_alk              , Water::alkalinity_ppm  , 2),
       EDITOR_FIELD_NORM(Water, label_pH              , lineEdit_ph               , Water::ph              , 2),
       EDITOR_FIELD_NORM(Water, label_alkalinityAsHCO3, boolCombo_alkalinityAsHCO3, Water::alkalinityAsHCO3, tr("CaCO3"), tr("HCO3")),
       // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
       EDITOR_FIELD_NORM(Water, label_carbonate       , lineEdit_carbonate        , Water::carbonate_ppm   , 2),
       EDITOR_FIELD_NORM(Water, label_potassium       , lineEdit_potassium        , Water::potassium_ppm   , 2),
       EDITOR_FIELD_NORM(Water, label_iron            , lineEdit_iron             , Water::iron_ppm        , 2),
       EDITOR_FIELD_NORM(Water, label_nitrate         , lineEdit_nitrate          , Water::nitrate_ppm     , 2),
       EDITOR_FIELD_NORM(Water, label_nitrite         , lineEdit_nitrite          , Water::nitrite_ppm     , 2),
       EDITOR_FIELD_NORM(Water, label_fluoride        , lineEdit_fluoride         , Water::fluoride_ppm    , 2),
      }
   );

   this->waterEditRadarChart->init(
      tr("PPM"),
      50,
      {
         {PropertyNames::Water::calcium_ppm,     tr("Calcium"    )},
         {PropertyNames::Water::bicarbonate_ppm, tr("Bicarbonate")},
         {PropertyNames::Water::sulfate_ppm,     tr("Sulfate"    )},
         {PropertyNames::Water::chloride_ppm,    tr("Chloride"   )},
         {PropertyNames::Water::sodium_ppm,      tr("Sodium"     )},
         {PropertyNames::Water::magnesium_ppm,   tr("Magnesium"  )}
      }
   );

   return;
}

//WaterEditor::~WaterEditor() = default;
WaterEditor::~WaterEditor() {
   qDebug() << Q_FUNC_INFO << "Cleaning up";
   if (this->m_editItem) {
      qDebug() <<
         Q_FUNC_INFO << this->m_editorName << ": Was observing" << this->m_editItem->name() <<
         "#" << this->m_editItem->key() << " @" << static_cast<void *>(this->m_editItem.get()) <<
         " (use count" << this->m_editItem.use_count() << ")";
   }
   if (this->m_liveEditItem) {
      qDebug() <<
         Q_FUNC_INFO << this->m_editorName << ": Was editing" << this->m_liveEditItem->name() <<
         "#" << this->m_liveEditItem->key() << " @" << static_cast<void *>(this->m_liveEditItem.get());
   }
   return;
}

void WaterEditor::postSetEditItem() {
   if (this->m_editItem) {
      // Note that we don't need to remove the old series from any previous Water objects as the call to addSeries will
      // replace them.
      this->waterEditRadarChart->addSeries(seriesNameCurrent, Qt::darkGreen, *this->m_editItem);

      this->waterEditRadarChart->addSeries(seriesNameModified, Qt::green, *this->m_liveEditItem);
      this->waterEditRadarChart->replot();
   } else {
      this->waterEditRadarChart->removeSeries(seriesNameCurrent );
      this->waterEditRadarChart->removeSeries(seriesNameModified);
   }
   return;
}


void WaterEditor::postInputFieldModified() {
   //
   // Strictly speaking we don't always need to replot the radar chart - eg if a text field changed it doesn't affect
   // the chart - but, for the moment, we just keep things simple and always replot.
   //
   this->waterEditRadarChart->replot();
   return;
}

EDITOR_COMMON_CODE(Water)
