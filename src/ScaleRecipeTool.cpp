/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * ScaleRecipeTool.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "NamedEntitySortProxyModel.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_ScaleRecipeTool.cpp"

ScaleRecipeTool::ScaleRecipeTool(QWidget* parent) :
   QWizard(parent),
   equipListModel(new EquipmentListModel(this)),
   equipSortProxyModel(new NamedEntitySortProxyModel(equipListModel)) {
   addPage(new ScaleRecipeIntroPage);
   addPage(new ScaleRecipeEquipmentPage(equipSortProxyModel));
   return;
}

void ScaleRecipeTool::accept() {
   int row = field("equipComboBox").toInt();
   QModelIndex equipProxyNdx( equipSortProxyModel->index(row, 0));
   QModelIndex equipNdx = equipSortProxyModel->mapToSource(equipProxyNdx);

   Equipment* selectedEquip = equipListModel->at(equipNdx.row());
   double newEff = field("effLineEdit").toString().toDouble();
   scale(selectedEquip, newEff);

   QWizard::accept();
   return;
}

void ScaleRecipeTool::setRecipe(Recipe* rec) {
   this->recObs = rec;
   return;
}

void ScaleRecipeTool::scale(Equipment* equip, double newEff) {
   if (!this->recObs || !equip) {
      return;
   }

   auto equipment = ObjectStoreWrapper::getSharedFromRaw(equip);

   // Calculate volume ratio
   double currentBatchSize_l = recObs->batchSize_l();
   double newBatchSize_l = equipment->fermenterBatchSize_l();
   double volRatio = newBatchSize_l / currentBatchSize_l;

   // Calculate efficiency ratio
   double oldEfficiency = recObs->efficiency_pct();
   double effRatio = oldEfficiency / newEff;

   this->recObs->setEquipment(equipment);
   this->recObs->setBatchSize_l(newBatchSize_l);
   this->recObs->nonOptBoil()->setPreBoilSize_l(equipment->kettleBoilSize_l());
   this->recObs->setEfficiency_pct(newEff);
   if (this->recObs->boil()) {
      this->recObs->boil()->setBoilTime_mins(equipment->boilTime_min().value_or(Equipment::default_boilTime_mins));
   }

   for (auto fermAddition : this->recObs->fermentableAdditions()) {
      // We assume volumes and masses get scaled the same way
      if (!fermAddition->fermentable()->isSugar() && !fermAddition->fermentable()->isExtract()) {
         fermAddition->setQuantity(fermAddition->quantity() * effRatio * volRatio);
      } else {
         fermAddition->setQuantity(fermAddition->quantity() * volRatio);
      }
   }

   for (auto hopAddition : this->recObs->hopAdditions()) {
      // We assume volumes and masses get scaled the same way
      hopAddition->setQuantity(hopAddition->quantity() * volRatio);
   }

   for (auto miscAddition : this->recObs->miscAdditions()) {
      // We assume volumes and masses get scaled the same way
      miscAddition->setQuantity(miscAddition->quantity() * volRatio);
   }

   for (auto waterUse : this->recObs->waterUses()) {
      waterUse->setVolume_l(waterUse->volume_l() * volRatio);
   }

   auto mash = this->recObs->mash();
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
   equipLabel(new QLabel),
   equipComboBox(new QComboBox),
   equipListModel(listModel),
   effLabel(new QLabel),
   effLineEdit(new QLineEdit) {

   doLayout();
   retranslateUi();

   registerField("equipComboBox", equipComboBox);
   registerField("effLineEdit", effLineEdit);
   return;
}

void ScaleRecipeEquipmentPage::doLayout() {

   layout->addRow(equipLabel, equipComboBox);
      equipComboBox->setModel(equipListModel);
   layout->addRow(effLabel, effLineEdit);
      effLineEdit->setText("70.0");
   setLayout(layout);
   return;
}

void ScaleRecipeEquipmentPage::retranslateUi() {
   setTitle(tr("Select Equipment"));
   setSubTitle(tr("The recipe will be scaled to match the batch size and efficiency of the selected equipment"));

   equipLabel->setText(tr("New Equipment"));
   effLabel->setText(tr("New Efficiency (%)"));
   return;
}

void ScaleRecipeEquipmentPage::changeEvent(QEvent* event) {
   if(event->type() == QEvent::LanguageChange) {
      retranslateUi();
   }
   QWidget::changeEvent(event);
   return;
}
