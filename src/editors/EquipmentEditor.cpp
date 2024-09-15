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

EquipmentEditor::EquipmentEditor(QWidget* parent/*, bool singleEquipEditor*/) :
   QDialog(parent),
   EditorBase<EquipmentEditor, Equipment>() {
   this->setupUi(this);

   this->tabWidget_editor->tabBar()->setStyle(new BtHorizontalTabs);

   SMART_FIELD_INIT(EquipmentEditor, label_name                    , lineEdit_name                    , Equipment, PropertyNames::NamedEntity::name                     );
   SMART_FIELD_INIT(EquipmentEditor, label_mashTunSpecificHeat     , lineEdit_mashTunSpecificHeat     , Equipment, PropertyNames::Equipment::mashTunSpecificHeat_calGC  );
   SMART_FIELD_INIT(EquipmentEditor, label_mashTunGrainAbsorption  , lineEdit_mashTunGrainAbsorption  , Equipment, PropertyNames::Equipment::mashTunGrainAbsorption_LKg );
   SMART_FIELD_INIT(EquipmentEditor, label_hopUtilization          , lineEdit_hopUtilization          , Equipment, PropertyNames::Equipment::hopUtilization_pct         , 0);
   SMART_FIELD_INIT(EquipmentEditor, label_mashTunWeight           , lineEdit_mashTunWeight           , Equipment, PropertyNames::Equipment::mashTunWeight_kg           );
   SMART_FIELD_INIT(EquipmentEditor, label_boilingPoint            , lineEdit_boilingPoint            , Equipment, PropertyNames::Equipment::boilingPoint_c             , 1);
   SMART_FIELD_INIT(EquipmentEditor, label_boilTime                , lineEdit_boilTime                , Equipment, PropertyNames::Equipment::boilTime_min               );
   SMART_FIELD_INIT(EquipmentEditor, label_fermenterBatchSize      , lineEdit_fermenterBatchSize      , Equipment, PropertyNames::Equipment::fermenterBatchSize_l       );
   SMART_FIELD_INIT(EquipmentEditor, label_kettleBoilSize          , lineEdit_kettleBoilSize          , Equipment, PropertyNames::Equipment::kettleBoilSize_l           );
   SMART_FIELD_INIT(EquipmentEditor, label_kettleEvaporationPerHour, lineEdit_kettleEvaporationPerHour, Equipment, PropertyNames::Equipment::kettleEvaporationPerHour_l );
   SMART_FIELD_INIT(EquipmentEditor, label_lauterTunDeadspaceLoss  , lineEdit_lauterTunDeadspaceLoss  , Equipment, PropertyNames::Equipment::lauterTunDeadspaceLoss_l   );
   SMART_FIELD_INIT(EquipmentEditor, label_topUpKettle             , lineEdit_topUpKettle             , Equipment, PropertyNames::Equipment::topUpKettle_l              );
   SMART_FIELD_INIT(EquipmentEditor, label_topUpWater              , lineEdit_topUpWater              , Equipment, PropertyNames::Equipment::topUpWater_l               );
   SMART_FIELD_INIT(EquipmentEditor, label_kettleTrubChillerLoss   , lineEdit_kettleTrubChillerLoss   , Equipment, PropertyNames::Equipment::kettleTrubChillerLoss_l    );
   SMART_FIELD_INIT(EquipmentEditor, label_mashTunVolume           , lineEdit_mashTunVolume           , Equipment, PropertyNames::Equipment::mashTunVolume_l            );
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SMART_FIELD_INIT(EquipmentEditor, label_hltType                 , lineEdit_hltType                 , Equipment, PropertyNames::Equipment::hltType                    );
   SMART_FIELD_INIT(EquipmentEditor, label_mashTunType             , lineEdit_mashTunType             , Equipment, PropertyNames::Equipment::mashTunType                );
   SMART_FIELD_INIT(EquipmentEditor, label_lauterTunType           , lineEdit_lauterTunType           , Equipment, PropertyNames::Equipment::lauterTunType              );
   SMART_FIELD_INIT(EquipmentEditor, label_kettleType              , lineEdit_kettleType              , Equipment, PropertyNames::Equipment::kettleType                 );
   SMART_FIELD_INIT(EquipmentEditor, label_fermenterType           , lineEdit_fermenterType           , Equipment, PropertyNames::Equipment::fermenterType              );
   SMART_FIELD_INIT(EquipmentEditor, label_agingVesselType         , lineEdit_agingVesselType         , Equipment, PropertyNames::Equipment::agingVesselType            );
   SMART_FIELD_INIT(EquipmentEditor, label_packagingVesselType     , lineEdit_packagingVesselType     , Equipment, PropertyNames::Equipment::packagingVesselType        );
   SMART_FIELD_INIT(EquipmentEditor, label_hltVolume               , lineEdit_hltVolume               , Equipment, PropertyNames::Equipment::hltVolume_l                );
   SMART_FIELD_INIT(EquipmentEditor, label_lauterTunVolume         , lineEdit_lauterTunVolume         , Equipment, PropertyNames::Equipment::lauterTunVolume_l          );
   SMART_FIELD_INIT(EquipmentEditor, label_agingVesselVolume       , lineEdit_agingVesselVolume       , Equipment, PropertyNames::Equipment::agingVesselVolume_l        );
   SMART_FIELD_INIT(EquipmentEditor, label_packagingVesselVolume   , lineEdit_packagingVesselVolume   , Equipment, PropertyNames::Equipment::packagingVesselVolume_l    );
   SMART_FIELD_INIT(EquipmentEditor, label_hltLoss                 , lineEdit_hltLoss                 , Equipment, PropertyNames::Equipment::hltLoss_l                  );
   SMART_FIELD_INIT(EquipmentEditor, label_mashTunLoss             , lineEdit_mashTunLoss             , Equipment, PropertyNames::Equipment::mashTunLoss_l              );
   SMART_FIELD_INIT(EquipmentEditor, label_fermenterLoss           , lineEdit_fermenterLoss           , Equipment, PropertyNames::Equipment::fermenterLoss_l            );
   SMART_FIELD_INIT(EquipmentEditor, label_agingVesselLoss         , lineEdit_agingVesselLoss         , Equipment, PropertyNames::Equipment::agingVesselLoss_l          );
   SMART_FIELD_INIT(EquipmentEditor, label_packagingVesselLoss     , lineEdit_packagingVesselLoss     , Equipment, PropertyNames::Equipment::packagingVesselLoss_l      );
   SMART_FIELD_INIT(EquipmentEditor, label_kettleOutflowPerMinute  , lineEdit_kettleOutflowPerMinute  , Equipment, PropertyNames::Equipment::kettleOutflowPerMinute_l   );
   SMART_FIELD_INIT(EquipmentEditor, label_hltWeight               , lineEdit_hltWeight               , Equipment, PropertyNames::Equipment::hltWeight_kg               );
   SMART_FIELD_INIT(EquipmentEditor, label_lauterTunWeight         , lineEdit_lauterTunWeight         , Equipment, PropertyNames::Equipment::lauterTunWeight_kg         );
   SMART_FIELD_INIT(EquipmentEditor, label_kettleWeight            , lineEdit_kettleWeight            , Equipment, PropertyNames::Equipment::kettleWeight_kg            );
   SMART_FIELD_INIT(EquipmentEditor, label_hltSpecificHeat         , lineEdit_hltSpecificHeat         , Equipment, PropertyNames::Equipment::hltSpecificHeat_calGC      );
   SMART_FIELD_INIT(EquipmentEditor, label_lauterTunSpecificHeat   , lineEdit_lauterTunSpecificHeat   , Equipment, PropertyNames::Equipment::lauterTunSpecificHeat_calGC);
   SMART_FIELD_INIT(EquipmentEditor, label_kettleSpecificHeat      , lineEdit_kettleSpecificHeat      , Equipment, PropertyNames::Equipment::kettleSpecificHeat_calGC   );

   // Connect all the boxen
   connect(this->checkBox_showHlt            , &QCheckBox::stateChanged    , this, &EquipmentEditor::hideOrShowOptionalVessels       );
   connect(this->checkBox_showLauterTun      , &QCheckBox::stateChanged    , this, &EquipmentEditor::hideOrShowOptionalVessels       );
   connect(this->checkBox_showAgingVessel    , &QCheckBox::stateChanged    , this, &EquipmentEditor::hideOrShowOptionalVessels       );
   connect(this->checkBox_showPackagingVessel, &QCheckBox::stateChanged    , this, &EquipmentEditor::hideOrShowOptionalVessels       );
   connect(this->checkBox_defaultEquipment   , &QCheckBox::stateChanged    , this, &EquipmentEditor::updateDefaultEquipment);
   connect(this->checkBox_calcBoilVolume     , &QCheckBox::stateChanged    , this, &EquipmentEditor::updateCalcBoilVolume  );
   connect(lineEdit_boilTime                 , &SmartLineEdit::textModified, this, &EquipmentEditor::updateCalcBoilVolume  );
   connect(lineEdit_kettleEvaporationPerHour , &SmartLineEdit::textModified, this, &EquipmentEditor::updateCalcBoilVolume  );
   connect(lineEdit_topUpWater               , &SmartLineEdit::textModified, this, &EquipmentEditor::updateCalcBoilVolume  );
   connect(lineEdit_kettleTrubChillerLoss    , &SmartLineEdit::textModified, this, &EquipmentEditor::updateCalcBoilVolume  );
   connect(lineEdit_fermenterBatchSize       , &SmartLineEdit::textModified, this, &EquipmentEditor::updateCalcBoilVolume  );
   connect(pushButton_absorption             , &QAbstractButton::clicked   , this, &EquipmentEditor::resetAbsorption       );

   this->connectSignalsAndSlots();
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


void EquipmentEditor::writeFieldsToEditItem() {

   m_editItem->setName                       (lineEdit_name                    ->text                 ());
   m_editItem->setKettleBoilSize_l           (lineEdit_kettleBoilSize          ->getNonOptCanonicalQty());
   m_editItem->setFermenterBatchSize_l       (lineEdit_fermenterBatchSize      ->getNonOptCanonicalQty());
   m_editItem->setMashTunVolume_l            (lineEdit_mashTunVolume           ->getNonOptCanonicalQty());
   m_editItem->setMashTunWeight_kg           (lineEdit_mashTunWeight           ->getOptCanonicalQty   ());
   m_editItem->setMashTunSpecificHeat_calGC  (lineEdit_mashTunSpecificHeat     ->getOptCanonicalQty   ());
   m_editItem->setBoilTime_min               (lineEdit_boilTime                ->getOptCanonicalQty   ());
   m_editItem->setKettleEvaporationPerHour_l (lineEdit_kettleEvaporationPerHour->getOptCanonicalQty   ());
   m_editItem->setTopUpKettle_l              (lineEdit_topUpKettle             ->getOptCanonicalQty   ());
   m_editItem->setTopUpWater_l               (lineEdit_topUpWater              ->getOptCanonicalQty   ());
   m_editItem->setKettleTrubChillerLoss_l    (lineEdit_kettleTrubChillerLoss   ->getNonOptCanonicalQty());
   m_editItem->setLauterTunDeadspaceLoss_l   (lineEdit_lauterTunDeadspaceLoss  ->getNonOptCanonicalQty());
   m_editItem->setMashTunGrainAbsorption_LKg (lineEdit_mashTunGrainAbsorption  ->getOptCanonicalQty   ());
   m_editItem->setBoilingPoint_c             (lineEdit_boilingPoint            ->getNonOptCanonicalQty());
   m_editItem->setHopUtilization_pct         (lineEdit_hopUtilization          ->getOptValue<double>  ());
   m_editItem->setKettleNotes                (textEdit_kettleNotes             ->toPlainText          ());
   m_editItem->setCalcBoilVolume             (checkBox_calcBoilVolume          ->isChecked            ());
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_editItem->setHltType                    (lineEdit_hltType                 ->text                 ());
   m_editItem->setMashTunType                (lineEdit_mashTunType             ->text                 ());
   m_editItem->setLauterTunType              (lineEdit_lauterTunType           ->text                 ());
   m_editItem->setKettleType                 (lineEdit_kettleType              ->text                 ());
   m_editItem->setFermenterType              (lineEdit_fermenterType           ->text                 ());
   m_editItem->setAgingVesselType            (lineEdit_agingVesselType         ->text                 ());
   m_editItem->setPackagingVesselType        (lineEdit_packagingVesselType     ->text                 ());
   m_editItem->setHltVolume_l                (lineEdit_hltVolume               ->getNonOptCanonicalQty());
   m_editItem->setLauterTunVolume_l          (lineEdit_lauterTunVolume         ->getNonOptCanonicalQty());
   m_editItem->setAgingVesselVolume_l        (lineEdit_agingVesselVolume       ->getNonOptCanonicalQty());
   m_editItem->setPackagingVesselVolume_l    (lineEdit_packagingVesselVolume   ->getNonOptCanonicalQty());
   m_editItem->setHltLoss_l                  (lineEdit_hltLoss                 ->getNonOptCanonicalQty());
   m_editItem->setMashTunLoss_l              (lineEdit_mashTunLoss             ->getNonOptCanonicalQty());
   m_editItem->setFermenterLoss_l            (lineEdit_fermenterLoss           ->getNonOptCanonicalQty());
   m_editItem->setAgingVesselLoss_l          (lineEdit_agingVesselLoss         ->getNonOptCanonicalQty());
   m_editItem->setPackagingVesselLoss_l      (lineEdit_packagingVesselLoss     ->getNonOptCanonicalQty());
   m_editItem->setKettleOutflowPerMinute_l   (lineEdit_kettleOutflowPerMinute  ->getOptCanonicalQty   ());
   m_editItem->setHltWeight_kg               (lineEdit_hltWeight               ->getOptCanonicalQty   ());
   m_editItem->setLauterTunWeight_kg         (lineEdit_lauterTunWeight         ->getOptCanonicalQty   ());
   m_editItem->setKettleWeight_kg            (lineEdit_kettleWeight            ->getOptCanonicalQty   ());
   m_editItem->setHltSpecificHeat_calGC      (lineEdit_hltSpecificHeat         ->getOptCanonicalQty   ());
   m_editItem->setLauterTunSpecificHeat_calGC(lineEdit_lauterTunSpecificHeat   ->getOptCanonicalQty   ());
   m_editItem->setKettleSpecificHeat_calGC   (lineEdit_kettleSpecificHeat      ->getOptCanonicalQty   ());
   m_editItem->setHltNotes                   (textEdit_hltNotes                ->toPlainText          ());
   m_editItem->setMashTunNotes               (textEdit_mashTunNotes            ->toPlainText          ());
   m_editItem->setLauterTunNotes             (textEdit_lauterTunNotes          ->toPlainText          ());
   m_editItem->setFermenterNotes             (textEdit_fermenterNotes          ->toPlainText          ());
   m_editItem->setAgingVesselNotes           (textEdit_agingVesselNotes        ->toPlainText          ());
   m_editItem->setPackagingVesselNotes       (textEdit_packagingVesselNotes    ->toPlainText          ());

   return;
}

void EquipmentEditor::writeLateFieldsToEditItem() {
   // Nothing to do here for Equipment
   return;
}

void EquipmentEditor::readFieldsFromEditItem(std::optional<QString> propName) {

   if (!propName || *propName == PropertyNames::NamedEntity::name    ) { this->lineEdit_name          ->setTextCursor(m_editItem->name          ()); // Continues to next line
                                                                         /* this->tabWidget_editor->setTabText(0, m_editItem->name()); */                 if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::kettleBoilSize_l           ) { this->lineEdit_kettleBoilSize          ->setQuantity    (m_editItem->kettleBoilSize_l           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::fermenterBatchSize_l       ) { this->lineEdit_fermenterBatchSize      ->setQuantity    (m_editItem->fermenterBatchSize_l       ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::mashTunVolume_l            ) { this->lineEdit_mashTunVolume           ->setQuantity    (m_editItem->mashTunVolume_l            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::mashTunWeight_kg           ) { this->lineEdit_mashTunWeight           ->setQuantity    (m_editItem->mashTunWeight_kg           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::mashTunSpecificHeat_calGC  ) { this->lineEdit_mashTunSpecificHeat     ->setQuantity    (m_editItem->mashTunSpecificHeat_calGC  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::boilTime_min               ) { this->lineEdit_boilTime                ->setQuantity    (m_editItem->boilTime_min               ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::kettleEvaporationPerHour_l ) { this->lineEdit_kettleEvaporationPerHour->setQuantity    (m_editItem->kettleEvaporationPerHour_l ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::topUpKettle_l              ) { this->lineEdit_topUpKettle             ->setQuantity    (m_editItem->topUpKettle_l              ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::topUpWater_l               ) { this->lineEdit_topUpWater              ->setQuantity    (m_editItem->topUpWater_l               ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::kettleTrubChillerLoss_l    ) { this->lineEdit_kettleTrubChillerLoss   ->setQuantity    (m_editItem->kettleTrubChillerLoss_l    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::lauterTunDeadspaceLoss_l   ) { this->lineEdit_lauterTunDeadspaceLoss  ->setQuantity    (m_editItem->lauterTunDeadspaceLoss_l   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::kettleNotes                ) { this->textEdit_kettleNotes             ->setText      (m_editItem->kettleNotes                ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::mashTunGrainAbsorption_LKg ) { this->lineEdit_mashTunGrainAbsorption  ->setQuantity    (m_editItem->mashTunGrainAbsorption_LKg ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::boilingPoint_c             ) { this->lineEdit_boilingPoint            ->setQuantity    (m_editItem->boilingPoint_c             ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::hopUtilization_pct         ) { this->lineEdit_hopUtilization          ->setQuantity    (m_editItem->hopUtilization_pct         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::calcBoilVolume             ) { this->checkBox_calcBoilVolume          ->setChecked   (m_editItem->calcBoilVolume             ()); if (propName) { return; } }
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   if (!propName || *propName == PropertyNames::Equipment::hltType                    ) { this->lineEdit_hltType                 ->setTextCursor(m_editItem->hltType                    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::mashTunType                ) { this->lineEdit_mashTunType             ->setTextCursor(m_editItem->mashTunType                ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::lauterTunType              ) { this->lineEdit_lauterTunType           ->setTextCursor(m_editItem->lauterTunType              ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::kettleType                 ) { this->lineEdit_kettleType              ->setTextCursor(m_editItem->kettleType                 ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::fermenterType              ) { this->lineEdit_fermenterType           ->setTextCursor(m_editItem->fermenterType              ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::agingVesselType            ) { this->lineEdit_agingVesselType         ->setTextCursor(m_editItem->agingVesselType            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::packagingVesselType        ) { this->lineEdit_packagingVesselType     ->setTextCursor(m_editItem->packagingVesselType        ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::hltVolume_l                ) { this->lineEdit_hltVolume               ->setQuantity    (m_editItem->hltVolume_l                ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::lauterTunVolume_l          ) { this->lineEdit_lauterTunVolume         ->setQuantity    (m_editItem->lauterTunVolume_l          ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::agingVesselVolume_l        ) { this->lineEdit_agingVesselVolume       ->setQuantity    (m_editItem->agingVesselVolume_l        ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::packagingVesselVolume_l    ) { this->lineEdit_packagingVesselVolume   ->setQuantity    (m_editItem->packagingVesselVolume_l    ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::hltLoss_l                  ) { this->lineEdit_hltLoss                 ->setQuantity    (m_editItem->hltLoss_l                  ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::mashTunLoss_l              ) { this->lineEdit_mashTunLoss             ->setQuantity    (m_editItem->mashTunLoss_l              ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::fermenterLoss_l            ) { this->lineEdit_fermenterLoss           ->setQuantity    (m_editItem->fermenterLoss_l            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::agingVesselLoss_l          ) { this->lineEdit_agingVesselLoss         ->setQuantity    (m_editItem->agingVesselLoss_l          ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::packagingVesselLoss_l      ) { this->lineEdit_packagingVesselLoss     ->setQuantity    (m_editItem->packagingVesselLoss_l      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::kettleOutflowPerMinute_l   ) { this->lineEdit_kettleOutflowPerMinute  ->setQuantity    (m_editItem->kettleOutflowPerMinute_l   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::hltWeight_kg               ) { this->lineEdit_hltWeight               ->setQuantity    (m_editItem->hltWeight_kg               ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::lauterTunWeight_kg         ) { this->lineEdit_lauterTunWeight         ->setQuantity    (m_editItem->lauterTunWeight_kg         ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::kettleWeight_kg            ) { this->lineEdit_kettleWeight            ->setQuantity    (m_editItem->kettleWeight_kg            ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::hltSpecificHeat_calGC      ) { this->lineEdit_hltSpecificHeat         ->setQuantity    (m_editItem->hltSpecificHeat_calGC      ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::lauterTunSpecificHeat_calGC) { this->lineEdit_lauterTunSpecificHeat   ->setQuantity    (m_editItem->lauterTunSpecificHeat_calGC()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::kettleSpecificHeat_calGC   ) { this->lineEdit_kettleSpecificHeat      ->setQuantity    (m_editItem->kettleSpecificHeat_calGC   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::hltNotes                   ) { this->textEdit_hltNotes                ->setText      (m_editItem->hltNotes                   ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::mashTunNotes               ) { this->textEdit_mashTunNotes            ->setText      (m_editItem->mashTunNotes               ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::lauterTunNotes             ) { this->textEdit_lauterTunNotes          ->setText      (m_editItem->lauterTunNotes             ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::fermenterNotes             ) { this->textEdit_fermenterNotes          ->setText      (m_editItem->fermenterNotes             ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::agingVesselNotes           ) { this->textEdit_agingVesselNotes        ->setText      (m_editItem->agingVesselNotes           ()); if (propName) { return; } }
   if (!propName || *propName == PropertyNames::Equipment::packagingVesselNotes       ) { this->textEdit_packagingVesselNotes    ->setText      (m_editItem->packagingVesselNotes       ()); if (propName) { return; } }

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
EDITOR_COMMON_CODE(EquipmentEditor)
