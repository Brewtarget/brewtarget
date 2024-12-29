/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * editors/EquipmentEditor.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • David Grundberg <individ@acc.umu.se>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mike Evans <mikee@saxicola.co.uk>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Priceless Brewing <shadowchao99@gmail.com>
 *   • Tyler Cipriani <tcipriani@wikimedia.org>
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
#include "editors/EquipmentEditor.h"

#include <QCloseEvent>
#include <QDebug>
#include <QIcon>
#include <QInputDialog>
#include <QMessageBox>

#include "config.h"
#include "BtHorizontalTabs.h"
#include "database/ObjectStoreWrapper.h"
#include "HeatCalculations.h"
#include "Localization.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "NamedEntitySortProxyModel.h"
#include "PersistentSettings.h"
#include "PhysicalConstants.h"

//
// TODO: According to https://www.engineersedge.com/materials/specific_heat_capacity_of_metals_13259.htm, the specific
// heat capacity of 304 grade stainless steel is 502.416 J/kg·K = 0.120080 c/g·C.  Would be nice to have a way for the
// user to grab this value (and that of other common materials if we can find them).
//

EquipmentEditor::EquipmentEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<EquipmentEditor, Equipment, EquipmentEditorOptions>(editorName) {
   this->setupUi(this);
   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);
   this->postSetupUiInit({
      EDITOR_FIELD_NORM(Equipment, label_name                    , lineEdit_name                    , NamedEntity::name                      ),
      EDITOR_FIELD_NORM(Equipment, label_mashTunSpecificHeat     , lineEdit_mashTunSpecificHeat     , Equipment::mashTunSpecificHeat_calGC   ),
      EDITOR_FIELD_NORM(Equipment, label_mashTunGrainAbsorption  , lineEdit_mashTunGrainAbsorption  , Equipment::mashTunGrainAbsorption_LKg  ),
      EDITOR_FIELD_NORM(Equipment, label_hopUtilization          , lineEdit_hopUtilization          , Equipment::hopUtilization_pct       , 0),
      EDITOR_FIELD_NORM(Equipment, label_mashTunWeight           , lineEdit_mashTunWeight           , Equipment::mashTunWeight_kg            ),
      EDITOR_FIELD_NORM(Equipment, label_boilingPoint            , lineEdit_boilingPoint            , Equipment::boilingPoint_c           , 1),
      EDITOR_FIELD_NORM(Equipment, label_boilTime                , lineEdit_boilTime                , Equipment::boilTime_min                ),
      EDITOR_FIELD_NORM(Equipment, label_fermenterBatchSize      , lineEdit_fermenterBatchSize      , Equipment::fermenterBatchSize_l        ),
      EDITOR_FIELD_NORM(Equipment, label_kettleBoilSize          , lineEdit_kettleBoilSize          , Equipment::kettleBoilSize_l            ),
      EDITOR_FIELD_NORM(Equipment, label_kettleEvaporationPerHour, lineEdit_kettleEvaporationPerHour, Equipment::kettleEvaporationPerHour_l  ),
      EDITOR_FIELD_NORM(Equipment, label_lauterTunDeadspaceLoss  , lineEdit_lauterTunDeadspaceLoss  , Equipment::lauterTunDeadspaceLoss_l    ),
      EDITOR_FIELD_NORM(Equipment, label_topUpKettle             , lineEdit_topUpKettle             , Equipment::topUpKettle_l               ),
      EDITOR_FIELD_NORM(Equipment, label_topUpWater              , lineEdit_topUpWater              , Equipment::topUpWater_l                ),
      EDITOR_FIELD_NORM(Equipment, label_kettleTrubChillerLoss   , lineEdit_kettleTrubChillerLoss   , Equipment::kettleTrubChillerLoss_l     ),
      EDITOR_FIELD_NORM(Equipment, label_mashTunVolume           , lineEdit_mashTunVolume           , Equipment::mashTunVolume_l             ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      EDITOR_FIELD_NORM(Equipment, label_hltType                 , lineEdit_hltType                 , Equipment::hltType                     ),
      EDITOR_FIELD_NORM(Equipment, label_mashTunType             , lineEdit_mashTunType             , Equipment::mashTunType                 ),
      EDITOR_FIELD_NORM(Equipment, label_lauterTunType           , lineEdit_lauterTunType           , Equipment::lauterTunType               ),
      EDITOR_FIELD_NORM(Equipment, label_kettleType              , lineEdit_kettleType              , Equipment::kettleType                  ),
      EDITOR_FIELD_NORM(Equipment, label_fermenterType           , lineEdit_fermenterType           , Equipment::fermenterType               ),
      EDITOR_FIELD_NORM(Equipment, label_agingVesselType         , lineEdit_agingVesselType         , Equipment::agingVesselType             ),
      EDITOR_FIELD_NORM(Equipment, label_packagingVesselType     , lineEdit_packagingVesselType     , Equipment::packagingVesselType         ),
      EDITOR_FIELD_NORM(Equipment, label_hltVolume               , lineEdit_hltVolume               , Equipment::hltVolume_l                 ),
      EDITOR_FIELD_NORM(Equipment, label_lauterTunVolume         , lineEdit_lauterTunVolume         , Equipment::lauterTunVolume_l           ),
      EDITOR_FIELD_NORM(Equipment, label_agingVesselVolume       , lineEdit_agingVesselVolume       , Equipment::agingVesselVolume_l         ),
      EDITOR_FIELD_NORM(Equipment, label_packagingVesselVolume   , lineEdit_packagingVesselVolume   , Equipment::packagingVesselVolume_l     ),
      EDITOR_FIELD_NORM(Equipment, label_hltLoss                 , lineEdit_hltLoss                 , Equipment::hltLoss_l                   ),
      EDITOR_FIELD_NORM(Equipment, label_mashTunLoss             , lineEdit_mashTunLoss             , Equipment::mashTunLoss_l               ),
      EDITOR_FIELD_NORM(Equipment, label_fermenterLoss           , lineEdit_fermenterLoss           , Equipment::fermenterLoss_l             ),
      EDITOR_FIELD_NORM(Equipment, label_agingVesselLoss         , lineEdit_agingVesselLoss         , Equipment::agingVesselLoss_l           ),
      EDITOR_FIELD_NORM(Equipment, label_packagingVesselLoss     , lineEdit_packagingVesselLoss     , Equipment::packagingVesselLoss_l       ),
      EDITOR_FIELD_NORM(Equipment, label_kettleOutflowPerMinute  , lineEdit_kettleOutflowPerMinute  , Equipment::kettleOutflowPerMinute_l    ),
      EDITOR_FIELD_NORM(Equipment, label_kettleInternalDiameter  , lineEdit_kettleInternalDiameter  , Equipment::kettleInternalDiameter_cm, 1),
      EDITOR_FIELD_NORM(Equipment, label_kettleOpeningDiameter   , lineEdit_kettleOpeningDiameter   , Equipment::kettleOpeningDiameter_cm , 1),
      EDITOR_FIELD_NORM(Equipment, label_hltWeight               , lineEdit_hltWeight               , Equipment::hltWeight_kg                ),
      EDITOR_FIELD_NORM(Equipment, label_lauterTunWeight         , lineEdit_lauterTunWeight         , Equipment::lauterTunWeight_kg          ),
      EDITOR_FIELD_NORM(Equipment, label_kettleWeight            , lineEdit_kettleWeight            , Equipment::kettleWeight_kg             ),
      EDITOR_FIELD_NORM(Equipment, label_hltSpecificHeat         , lineEdit_hltSpecificHeat         , Equipment::hltSpecificHeat_calGC       ),
      EDITOR_FIELD_NORM(Equipment, label_lauterTunSpecificHeat   , lineEdit_lauterTunSpecificHeat   , Equipment::lauterTunSpecificHeat_calGC ),
      EDITOR_FIELD_NORM(Equipment, label_kettleSpecificHeat      , lineEdit_kettleSpecificHeat      , Equipment::kettleSpecificHeat_calGC    ),
   });

   // Connect all the boxen
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
   connect(this->checkBox_showHlt                  , &QCheckBox::checkStateChanged, this, &EquipmentEditor::hideOrShowOptionalVessels);
   connect(this->checkBox_showLauterTun            , &QCheckBox::checkStateChanged, this, &EquipmentEditor::hideOrShowOptionalVessels);
   connect(this->checkBox_showAgingVessel          , &QCheckBox::checkStateChanged, this, &EquipmentEditor::hideOrShowOptionalVessels);
   connect(this->checkBox_showPackagingVessel      , &QCheckBox::checkStateChanged, this, &EquipmentEditor::hideOrShowOptionalVessels);
   connect(this->checkBox_defaultEquipment         , &QCheckBox::checkStateChanged, this, &EquipmentEditor::updateDefaultEquipment   );
   connect(this->checkBox_calcBoilVolume           , &QCheckBox::checkStateChanged, this, &EquipmentEditor::updateCalcBoilVolume     );
#else
   connect(this->checkBox_showHlt                  , &QCheckBox::stateChanged     , this, &EquipmentEditor::hideOrShowOptionalVessels);
   connect(this->checkBox_showLauterTun            , &QCheckBox::stateChanged     , this, &EquipmentEditor::hideOrShowOptionalVessels);
   connect(this->checkBox_showAgingVessel          , &QCheckBox::stateChanged     , this, &EquipmentEditor::hideOrShowOptionalVessels);
   connect(this->checkBox_showPackagingVessel      , &QCheckBox::stateChanged     , this, &EquipmentEditor::hideOrShowOptionalVessels);
   connect(this->checkBox_defaultEquipment         , &QCheckBox::stateChanged     , this, &EquipmentEditor::updateDefaultEquipment   );
   connect(this->checkBox_calcBoilVolume           , &QCheckBox::stateChanged     , this, &EquipmentEditor::updateCalcBoilVolume     );
#endif
   connect(this->lineEdit_boilTime                 , &SmartLineEdit::textModified , this, &EquipmentEditor::updateCalcBoilVolume     );
   connect(this->lineEdit_kettleEvaporationPerHour , &SmartLineEdit::textModified , this, &EquipmentEditor::updateCalcBoilVolume     );
   connect(this->lineEdit_topUpWater               , &SmartLineEdit::textModified , this, &EquipmentEditor::updateCalcBoilVolume     );
   connect(this->lineEdit_kettleTrubChillerLoss    , &SmartLineEdit::textModified , this, &EquipmentEditor::updateCalcBoilVolume     );
   connect(this->lineEdit_fermenterBatchSize       , &SmartLineEdit::textModified , this, &EquipmentEditor::updateCalcBoilVolume     );
   connect(this->pushButton_absorption             , &QAbstractButton::clicked    , this, &EquipmentEditor::resetAbsorption          );

   return;
}

EquipmentEditor::~EquipmentEditor() = default;

bool EquipmentEditor::validateBeforeSave() {
   bool problems = false;

   // Do some prewarning things. I would prefer to do this only on change, but
   // we need to be worried about new equipment too.

   //
   // Note that the latest qFuzzyCompare documentation says it "will not work" if either of the parameters is 0.0, and
   // we are directed to use qFuzzyIsNull instead in such circumstances.  The latter checks whether its argument is
   // smaller than 0.000000000001, which is good enough for our purposes here.
   //
   // We could, I suppose, validate that vessel volumes etc are within some sane bounds (say larger than a liter and
   // smaller than a swimming pool), but, for the moment at least we are just looking for some obvious "You might not
   // realise but this will break things" errors.  We otherwise trust the user to be able to spot if s/he has
   // accidentally entered nonsensical field values.
   //
   QString message = tr("This equipment profile may break %1's maths").arg(CONFIG_APPLICATION_NAME_UC);
   QString inform = QString("%1%2").arg(tr("The following values are not set:")).arg(QString("<ul>"));
   if (qFuzzyIsNull(lineEdit_mashTunVolume->getNonOptCanonicalQty()))    {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("mash tun volume (all-grain and BIAB only)"));
   }

   if (qFuzzyIsNull(lineEdit_fermenterBatchSize->getNonOptCanonicalQty())) {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("batch size"));
   }

   if (qFuzzyIsNull(lineEdit_hopUtilization->getOptValue<double>().value_or(0.0))) {
      problems = true;
      inform = inform + QString("<li>%1</li>").arg(tr("hop utilization"));
   }
   inform = inform + QString("</ul>");

   if (problems) {
      QMessageBox theQuestion;
      theQuestion.setWindowTitle( tr("Calculation Warnings") );
      theQuestion.setText(message);
      theQuestion.setInformativeText(inform);
      theQuestion.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
      theQuestion.setDefaultButton(QMessageBox::Save);
      theQuestion.setIcon(QMessageBox::Warning);
      if (theQuestion.exec() == QMessageBox::Cancel) {
         return false;
      }
   }

   //
   // Maybe two separate alert boxes is a but much, but I worry about putting too much info in a single one
   //
   // What we particularly care about here is not having, eg, lauter deadspace set when the lauter tun volume is 0.0
   // (and thus, by default, hidden in the UI).  We are rather less worried about, say, lauter tun notes being set in
   // this circumstance, as it won't break anything.
   //
   message = tr("Numerical values on zero-sized \"optional\" vessels will be unset or zeroed when you save");
   inform = QString("%1%2").arg(tr("The following values will be discarded:")).arg(QString("<ul>"));
   if (qFuzzyIsNull(lineEdit_hltVolume->getNonOptCanonicalQty())) {
      if (lineEdit_hltLoss               ->getNonOptCanonicalQty()            > 0.0) { problems = true; inform = inform + QString("<li>%1</li>").arg(tr("Hot liquor tank losses"                )); }
      if (lineEdit_hltWeight             ->getOptCanonicalQty().value_or(0.0) > 0.0) { problems = true; inform = inform + QString("<li>%1</li>").arg(tr("Hot liquor tank weight"                )); }
      if (lineEdit_hltSpecificHeat       ->getOptCanonicalQty().value_or(0.0) > 0.0) { problems = true; inform = inform + QString("<li>%1</li>").arg(tr("Hot liquor tank specific heat capacity")); }
   }
   if (qFuzzyIsNull(lineEdit_lauterTunVolume->getNonOptCanonicalQty())) {
      if (lineEdit_lauterTunDeadspaceLoss->getNonOptCanonicalQty()            > 0.0) { problems = true; inform = inform + QString("<li>%1</li>").arg(tr("Lauter tun losses"                     )); }
      if (lineEdit_lauterTunWeight       ->getOptCanonicalQty().value_or(0.0) > 0.0) { problems = true; inform = inform + QString("<li>%1</li>").arg(tr("Lauter tun weight"                     )); }
      if (lineEdit_lauterTunSpecificHeat ->getOptCanonicalQty().value_or(0.0) > 0.0) { problems = true; inform = inform + QString("<li>%1</li>").arg(tr("Lauter tun specific heat capacity"     )); }
   }
   if (qFuzzyIsNull(lineEdit_packagingVesselVolume->getNonOptCanonicalQty())) {
      if (lineEdit_packagingVesselLoss   ->getNonOptCanonicalQty()            > 0.0) { problems = true; inform = inform + QString("<li>%1</li>").arg(tr("Packaging vessel losses"               )); }
   }
   if (qFuzzyIsNull(lineEdit_agingVesselVolume->getNonOptCanonicalQty())) {
      if (lineEdit_agingVesselLoss       ->getNonOptCanonicalQty()            > 0.0) { problems = true; inform = inform + QString("<li>%1</li>").arg(tr("Aging vessel losses"                   )); }
   }

   if (problems) {
      QMessageBox theQuestion;
      theQuestion.setWindowTitle( tr("Data Discard Warnings") );
      theQuestion.setText(message);
      theQuestion.setInformativeText( inform );
      theQuestion.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
      theQuestion.setDefaultButton(QMessageBox::Save);
      theQuestion.setIcon(QMessageBox::Warning);
      if (theQuestion.exec() == QMessageBox::Cancel) {
         return false;
      }
   }

   // Now zero out / unset anything we said we would
   if (qFuzzyIsNull(lineEdit_hltVolume->getNonOptCanonicalQty())) {
      lineEdit_hltVolume      ->setQuantity(0.0);
      lineEdit_hltLoss        ->setQuantity(0.0);
      lineEdit_hltWeight      ->setQuantity<double>(std::nullopt);
      lineEdit_hltSpecificHeat->setQuantity<double>(std::nullopt);
   }
   if (qFuzzyIsNull(lineEdit_lauterTunVolume->getNonOptCanonicalQty())) {
      lineEdit_lauterTunVolume       ->setQuantity(0.0);
      lineEdit_lauterTunDeadspaceLoss->setQuantity(0.0);
      lineEdit_lauterTunWeight       ->setQuantity<double>(std::nullopt);
      lineEdit_lauterTunSpecificHeat ->setQuantity<double>(std::nullopt);
   }
   if (qFuzzyIsNull(lineEdit_packagingVesselVolume->getNonOptCanonicalQty())) {
      lineEdit_packagingVesselVolume->setQuantity(0.0);
      lineEdit_packagingVesselLoss  ->setQuantity(0.0);
   }
   if (qFuzzyIsNull(lineEdit_agingVesselVolume->getNonOptCanonicalQty())) {
      lineEdit_agingVesselVolume->setQuantity(0.0);
      lineEdit_agingVesselLoss  ->setQuantity(0.0);
   }

   return true;
}


void EquipmentEditor::postReadFieldsFromEditItem([[maybe_unused]] std::optional<QString> propName) {

   // These aren't fields we store in the DB, rather they control which bits of the editor are visible
   this->checkBox_showHlt            ->setChecked(m_editItem->hltVolume_l            () != 0);
   this->checkBox_showLauterTun      ->setChecked(m_editItem->lauterTunVolume_l      () != 0);
   this->checkBox_showAgingVessel    ->setChecked(m_editItem->agingVesselVolume_l    () != 0);
   this->checkBox_showPackagingVessel->setChecked(m_editItem->packagingVesselVolume_l() != 0);

   this->hideOrShowOptionalVessels();

   this->checkBox_defaultEquipment->blockSignals(true);
   this->checkBox_defaultEquipment->setChecked(m_editItem->key() == PersistentSettings::value(PersistentSettings::Names::defaultEquipmentKey, -1));
   this->checkBox_defaultEquipment->blockSignals(false);

   return;
}

void EquipmentEditor::hideOrShowOptionalVessels() {
   QObject * sender = this->sender();
   // Believe it or not, QTabWidget::setTabVisible was only introduced in Qt 5.15.  There were various ghastly
   // workarounds prior to that - eg removing and re-adding the tab you want to hide/show.  But, since it's only Ubuntu
   // 20.04 LTS running a too-old version of Qt (5.12.8), and we won't be supporting that forever, I'm just going to
   // disable the tab instead (which greys out its contents) if Qt is too old.  It's not quite as good, but it's not
   // hideous either IMHO.
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   if (!sender || sender == this->checkBox_showHlt            ) { this->tab_hlt            ->setEnabled(this->checkBox_showHlt            ->isChecked()); if (sender) { return; } }
   if (!sender || sender == this->checkBox_showLauterTun      ) { this->tab_lauterTun      ->setEnabled(this->checkBox_showLauterTun      ->isChecked()); if (sender) { return; } }
   if (!sender || sender == this->checkBox_showAgingVessel    ) { this->tab_agingVessel    ->setEnabled(this->checkBox_showAgingVessel    ->isChecked()); if (sender) { return; } }
   if (!sender || sender == this->checkBox_showPackagingVessel) { this->tab_packagingVessel->setEnabled(this->checkBox_showPackagingVessel->isChecked()); if (sender) { return; } }
#else
   if (!sender || sender == this->checkBox_showHlt            ) { this->tabWidget_editor->setTabVisible(this->tabWidget_editor->indexOf(this->tab_hlt            ), this->checkBox_showHlt            ->isChecked()); if (sender) { return; } }
   if (!sender || sender == this->checkBox_showLauterTun      ) { this->tabWidget_editor->setTabVisible(this->tabWidget_editor->indexOf(this->tab_lauterTun      ), this->checkBox_showLauterTun      ->isChecked()); if (sender) { return; } }
   if (!sender || sender == this->checkBox_showAgingVessel    ) { this->tabWidget_editor->setTabVisible(this->tabWidget_editor->indexOf(this->tab_agingVessel    ), this->checkBox_showAgingVessel    ->isChecked()); if (sender) { return; } }
   if (!sender || sender == this->checkBox_showPackagingVessel) { this->tabWidget_editor->setTabVisible(this->tabWidget_editor->indexOf(this->tab_packagingVessel), this->checkBox_showPackagingVessel->isChecked()); if (sender) { return; } }
#endif

   return;
}

void EquipmentEditor::updateCalcBoilVolume() {
   if (this->checkBox_calcBoilVolume->isChecked()) {
      this->lineEdit_kettleBoilSize->setQuantity(this->calcBatchSize());
      this->lineEdit_kettleBoilSize->setEnabled(false);
   } else {
      this->lineEdit_kettleBoilSize->setQuantity(this->lineEdit_fermenterBatchSize->getNonOptCanonicalQty());
      this->lineEdit_kettleBoilSize->setEnabled(true);
   }
   return;
}

double EquipmentEditor::calcBatchSize() {
   double size     = lineEdit_fermenterBatchSize      ->getNonOptCanonicalQty();
   double topUp    = lineEdit_topUpWater              ->getOptCanonicalQty().value_or(Equipment::default_topUpWater_l);
   double trubLoss = lineEdit_kettleTrubChillerLoss   ->getNonOptCanonicalQty();
   double evapRate = lineEdit_kettleEvaporationPerHour->getOptCanonicalQty().value_or(Equipment::default_kettleEvaporationPerHour_l);
   double time     = lineEdit_boilTime                ->getOptCanonicalQty().value_or(Equipment::default_boilTime_mins);

   return size - topUp + trubLoss + (time/60.0)*evapRate;
}

void EquipmentEditor::resetAbsorption() {
   if (!m_editItem) {
      return;
   }

   lineEdit_mashTunGrainAbsorption->setQuantity(PhysicalConstants::grainAbsorption_Lkg);
   return;
}

void EquipmentEditor::updateDefaultEquipment() {
   if (!m_editItem) {
      return;
   }

   if (this->checkBox_defaultEquipment->isChecked()) {
      PersistentSettings::insert(PersistentSettings::Names::defaultEquipmentKey, m_editItem->key());
      return;
   }

   QVariant currentDefault = PersistentSettings::value(PersistentSettings::Names::defaultEquipmentKey, -1);
   if (currentDefault == m_editItem->key()) {
      PersistentSettings::insert(PersistentSettings::Names::defaultEquipmentKey, -1);
   }
   return;
}

// Insert the boiler-plate stuff that we cannot do in EditorBase
EDITOR_COMMON_CODE(Equipment)
