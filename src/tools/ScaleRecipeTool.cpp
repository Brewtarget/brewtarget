/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tools/ScaleRecipeTool.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
 =====================================================================================================================*/
#include "tools/ScaleRecipeTool.h"

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
#include "model/Water.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_ScaleRecipeTool.cpp"
#endif

ScaleRecipeTool::ScaleRecipeTool(QWidget* parent) :
   QWizard(parent) {

   addPage(new ScaleRecipeIntroPage());
   addPage(new ScaleRecipeEquipmentPage());
   return;
}

void ScaleRecipeTool::accept() {

   int const equipmentId = this->field("m_equipComboBox").toInt();
   if (equipmentId < 0) {
      return;
   }
   Equipment* selectedEquipment = ObjectStoreWrapper::getByIdRaw<Equipment>(equipmentId);
   double const newEfficiency = this->field("m_efficiencyLineEdit").toString().toDouble();
   this->scale(selectedEquipment, newEfficiency);

   QWizard::accept();
   return;
}

void ScaleRecipeTool::setRecipe(Recipe* rec) {
   this->m_recObs = rec;
   return;
}

void ScaleRecipeTool::scale(Equipment* equip, double const newEff) {
   if (!this->m_recObs || !equip) {
      return;
   }

   auto const equipment = ObjectStoreWrapper::getSharedFromRaw(equip);

   // Calculate volume ratio
   double const currentBatchSize_l = m_recObs->batchSize_l();
   double const newBatchSize_l = equipment->fvBatchSize_l();
   double const volRatio = newBatchSize_l / currentBatchSize_l;

   // Calculate efficiency ratio
   double const oldEfficiency = m_recObs->efficiency_pct();
   double const effRatio = oldEfficiency / newEff;

   this->m_recObs->setEquipment(equipment);
   this->m_recObs->setBatchSize_l(newBatchSize_l);
   // We assume boil time remains unchanged (see below), so the pre-boil volume needs to be for the recipe's boil time
   auto const boil = this->m_recObs->nonOptBoil();
   boil->setPreBoilSize_l(equipment->defaultPreBoilSize_l(boil->boilTime_mins()));
   this->m_recObs->setEfficiency_pct(newEff);

   // We assume boil time remains unchanged

   for (auto const & fermentableAddition : this->m_recObs->fermentableAdditions()) {
      // We assume volumes and masses get scaled the same way.  Only ingredients whose extract is subject to mash
      // efficiency (see Recipe::sugarTotals) need adjusting for a change of efficiency.
      if (!fermentableAddition->fermentable()->isExistingSugar()) {
         fermentableAddition->setQuantity(fermentableAddition->quantity() * effRatio * volRatio);
      } else {
         fermentableAddition->setQuantity(fermentableAddition->quantity() * volRatio);
      }
   }

   for (auto const & hopAddition : this->m_recObs->hopAdditions()) {
      // We assume volumes and masses get scaled the same way
      hopAddition->setQuantity(hopAddition->quantity() * volRatio);
   }

   for (auto const & miscAddition : this->m_recObs->miscAdditions()) {
      // We assume volumes and masses get scaled the same way
      miscAddition->setQuantity(miscAddition->quantity() * volRatio);
   }

   // TBD: For now we don't scale the yeasts, but it might be good to give the option on this if user is doing a big
   //      scale-up or down.

   //
   // We used to set all the mash step amounts to zero here, so that the user would know to re-run the mash wizard.
   // But, with no water in the mash, the recipe falls back to assuming it will hit the target pre-boil volume (see
   // Recipe::recalcVolumeEstimates), so nothing on the recipe tab showed that there was a problem.  Leaving the old
   // amounts in place means the calculated boil volume shows up as too low (or too high), and, if the user brews the
   // recipe without fixing it, they get a warning (see MainWindow).
   //
   // If the mash is kept in sync with the recipe, the change of equipment (and of pre-boil volume) will already have
   // scheduled an Auto-Adjust (see Recipe::scheduleMashAutoAdjust).  It runs once we return to the event loop, so it
   // sees the scaled grain bill.
   //
   QString message = tr("The recipe has been scaled.");
   if (auto const mash = this->m_recObs->mash()) {
      message += " ";
      if (mash->keptInSync()) {
         message += tr("The mash is kept in sync with the recipe, so its water amounts and temperatures will be "
                       "recalculated automatically.  Check the mash editor for any problems.");
      } else {
         message += tr("The amounts of water in the mash have not been changed, so you will probably need to use "
                       "Auto-Adjust in the mash editor to work out new ones.");
      }
   }
   QMessageBox::information(this, tr("Recipe Scaled"), message);
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
   label->setText(tr("This wizard will help you scale a recipe to another size or efficiency.  "
                     "Select another equipment with the new batch size and/or efficiency and "
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

ScaleRecipeEquipmentPage::ScaleRecipeEquipmentPage(QWidget* parent) :
   QWizardPage(parent),
   layout(new QFormLayout),
   m_equipLabel(new QLabel),
   m_equipComboBox(new BtComboBoxEquipment),
   m_efficiencyLabel(new QLabel),
   m_efficiencyLineEdit(new QLineEdit) {

   doLayout();
   retranslateUi();
   this->m_equipComboBox->init();

   registerField("m_equipComboBox", m_equipComboBox, *PropertyNames::BtComboBoxObject::currentId, "currentIndexChanged");
   registerField("m_efficiencyLineEdit", m_efficiencyLineEdit);
   return;
}

void ScaleRecipeEquipmentPage::doLayout() {

   layout->addRow(m_equipLabel, m_equipComboBox);
   layout->addRow(m_efficiencyLabel, m_efficiencyLineEdit);
   m_efficiencyLineEdit->setText(PersistentSettings::value_ck(PersistentSettings::Names::defaultEfficiency_pct, Recipe::default_efficiency_pct).toString());
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