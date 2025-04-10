/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * ScaleRecipeTool.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "ScaleRecipeTool.h"

#include <QMessageBox>
#include <QButtonGroup>

#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "qtModels/listModels/EquipmentListModel.h"
#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUseOfWater.h"
#include "model/Water.h"
#include "model/Yeast.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_ScaleRecipeTool.cpp"
#endif

ScaleRecipeTool::ScaleRecipeTool(QWidget* parent) :
   QWizard(parent),
   m_equipListModel(new EquipmentListModel(this)),
   m_equipSortProxyModel(new QSortFilterProxyModel(m_equipListModel)) {

   // Sorts models dynamically based on their properties' default sort behavior.
   m_equipSortProxyModel->setSourceModel(m_equipListModel);
   m_equipSortProxyModel->setDynamicSortFilter(true);
   m_equipSortProxyModel->sort(0);

   addPage(new ScaleRecipeIntroPage);
   addPage(new ScaleRecipeEquipmentPage(m_equipSortProxyModel));
   return;
}

void ScaleRecipeTool::accept() {
   int row = field("m_equipComboBox").toInt();
   QModelIndex equipProxyNdx( m_equipSortProxyModel->index(row, 0));
   QModelIndex equipNdx = m_equipSortProxyModel->mapToSource(equipProxyNdx);

   Equipment* selectedEquip = m_equipListModel->at(equipNdx.row());
   double newEff = field("m_efficiencyLineEdit").toString().toDouble();
   scale(selectedEquip, newEff);

   QWizard::accept();
   return;
}

void ScaleRecipeTool::setRecipe(Recipe* rec) {
   this->m_recObs = rec;
   return;
}

void ScaleRecipeTool::scale(Equipment* equip, double newEff) {
   if (!this->m_recObs || !equip) {
      return;
   }

   auto equipment = ObjectStoreWrapper::getSharedFromRaw(equip);

   // Calculate volume ratio
   double currentBatchSize_l = m_recObs->batchSize_l();
   double newBatchSize_l = equipment->fermenterBatchSize_l();
   double volRatio = newBatchSize_l / currentBatchSize_l;

   // Calculate efficiency ratio
   double oldEfficiency = m_recObs->efficiency_pct();
   double effRatio = oldEfficiency / newEff;

   this->m_recObs->setEquipment(equipment);
   this->m_recObs->setBatchSize_l(newBatchSize_l);
   this->m_recObs->nonOptBoil()->setPreBoilSize_l(equipment->kettleBoilSize_l());
   this->m_recObs->setEfficiency_pct(newEff);
   if (this->m_recObs->boil()) {
      this->m_recObs->boil()->setBoilTime_mins(equipment->boilTime_min().value_or(Equipment::default_boilTime_mins));
   }

   for (auto fermAddition : this->m_recObs->fermentableAdditions()) {
      // We assume volumes and masses get scaled the same way
      if (!fermAddition->fermentable()->isSugar() && !fermAddition->fermentable()->isExtract()) {
         fermAddition->setQuantity(fermAddition->quantity() * effRatio * volRatio);
      } else {
         fermAddition->setQuantity(fermAddition->quantity() * volRatio);
      }
   }

   for (auto hopAddition : this->m_recObs->hopAdditions()) {
      // We assume volumes and masses get scaled the same way
      hopAddition->setQuantity(hopAddition->quantity() * volRatio);
   }

   for (auto miscAddition : this->m_recObs->miscAdditions()) {
      // We assume volumes and masses get scaled the same way
      miscAddition->setQuantity(miscAddition->quantity() * volRatio);
   }

   for (auto waterUse : this->m_recObs->waterUses()) {
      waterUse->setVolume_l(waterUse->volume_l() * volRatio);
   }

   auto mash = this->m_recObs->mash();
   if (mash) {
      // Reset all these to zero so that the user
      // will know to re-run the mash wizard.
      for (auto step : mash->mashSteps()) {
         step->setAmount_l(0);
      }
   }

   // TBD: For now we don't scale the yeasts, but it might be good to give the option on this if user is doing a big
   //      scale-up or down.

   // Let the user know what happened.
   QMessageBox::information(this,
                            tr("Recipe Scaled"),
                            tr("The equipment and mash have been reset due to the fact that mash temperatures do not "
                               "scale easily. Please re-run the mash wizard."));
   return;
}

// ScaleRecipeIntroPage =======================================================

ScaleRecipeIntroPage::ScaleRecipeIntroPage(QWidget* parent) :
   QWizardPage(parent),
   layout(new QVBoxLayout),
   label(new QLabel) {

   doLayout();
   retranslateUi();
   return;
}

void ScaleRecipeIntroPage::doLayout() {
   static QString const logoFile = QString{":images/%1.svg"}.arg(CONFIG_APPLICATION_NAME_LC);
   setPixmap(QWizard::WatermarkPixmap, QPixmap(logoFile));

   layout->addWidget(label);
      label->setWordWrap(true);
   setLayout(layout);
   return;
}

void ScaleRecipeIntroPage::retranslateUi() {
   setTitle(tr("Scale Recipe"));
   label->setText(tr("This wizard will help you scale a recipe to another size or efficiency."
                     "Select another equipment with the new batch size and/or efficiency and"
                     "the wizard will scale the recipe ingredients automatically."));
   return;
}

void ScaleRecipeIntroPage::changeEvent(QEvent* event) {
   if(event->type() == QEvent::LanguageChange) {
      retranslateUi();
   }
   QWidget::changeEvent(event);
   return;
}

// ScaleRecipeEquipmentPage ===================================================

ScaleRecipeEquipmentPage::ScaleRecipeEquipmentPage(QAbstractItemModel* listModel, QWidget* parent) :
   QWizardPage(parent),
   layout(new QFormLayout),
   m_equipLabel(new QLabel),
   m_equipComboBox(new QComboBox),
   m_equipListModel(listModel),
   m_efficiencyLabel(new QLabel),
   m_efficiencyLineEdit(new QLineEdit) {

   doLayout();
   retranslateUi();

   registerField("m_equipComboBox", m_equipComboBox);
   registerField("m_efficiencyLineEdit", m_efficiencyLineEdit);
   return;
}

void ScaleRecipeEquipmentPage::doLayout() {

   layout->addRow(m_equipLabel, m_equipComboBox);
      m_equipComboBox->setModel(m_equipListModel);
   layout->addRow(m_efficiencyLabel, m_efficiencyLineEdit);
      m_efficiencyLineEdit->setText("70.0");
   setLayout(layout);
   return;
}

void ScaleRecipeEquipmentPage::retranslateUi() {
   setTitle(tr("Select Equipment"));
   setSubTitle(tr("The recipe will be scaled to match the batch size and efficiency of the selected equipment"));

   m_equipLabel->setText(tr("New Equipment"));
   m_efficiencyLabel->setText(tr("New Efficiency (%)"));
   return;
}

void ScaleRecipeEquipmentPage::changeEvent(QEvent* event) {
   if(event->type() == QEvent::LanguageChange) {
      retranslateUi();
   }
   QWidget::changeEvent(event);
   return;
}
