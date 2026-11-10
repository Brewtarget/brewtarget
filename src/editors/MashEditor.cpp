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

#include <algorithm>

#include <QDebug>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QUndoStack>
#include <QWidget>

#include "MashAutoAdjuster.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/Unit.h"
#include "model/Equipment.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "model/RecipeUtils.h"
#include "qtModels/tableModels/MashStepTableModel.h"
#include "undoRedo/Undoable.h"
#include "undoRedo/UndoableAutoAdjust.h"
#include "widgets/SmartAmounts.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MashEditor.cpp"
#endif

MashEditor::MashEditor(QWidget* parent, QString const editorName) :
   QDialog(parent),
   EditorBase<MashEditor, Mash, MashEditorOptions>(editorName) {
   this->setupUi(this);
   this->postSetupUiInit({
      EDITOR_FIELD_NORM(Mash, label_name          , lineEdit_name       , NamedEntity::name             ),
      EDITOR_FIELD_NORM(Mash, label_grainTemp     , lineEdit_grainTemp  , Mash::grainTemp_c          , 1),
      EDITOR_FIELD_NORM(Mash, label_spargeTemp    , lineEdit_spargeTemp , Mash::spargeTemp_c         , 1),
      EDITOR_FIELD_NORM(Mash, label_spargePh      , lineEdit_spargePh   , Mash::ph                   , 0),
      EDITOR_FIELD_NORM(Mash, label_tunTemp       , lineEdit_tunTemp    , Mash::initialTunTemp_c     , 1),
      EDITOR_FIELD_NORM(Mash, label_tunMass       , lineEdit_tunMass    , Mash::tunWeight_kg            ),
      EDITOR_FIELD_NORM(Mash, label_tunSpHeat     , lineEdit_tunSpHeat  , Mash::tunSpecificHeat_calGC, 1),
      EDITOR_FIELD_NORM(Mash, label_totalMashWater, value_totalMashWater, Mash::totalMashWater_l     , 1),
      EDITOR_FIELD_NORM(Mash, label_totalTime     , value_totalTime     , Mash::totalTime_mins       , 0),
      EDITOR_FIELD_NORM(Mash, tab_notes           , textEdit_notes      , Mash::notes                   ),
   });

   //
   // The calculation controls are not fields of the Mash, so we set them up separately.  The thickness field shows
   // the same quantity as MashStep::liquorToGristRatio_lKg, which is where the calculation records it.
   //
   SMART_FIELD_INIT(MashEditor, label_thickness, lineEdit_thickness, MashStep, PropertyNames::MashStep::liquorToGristRatio_lKg, 2);
   // Order here must match MashAutoAdjuster::SpargeMethod
   this->comboBox_spargeMethod->addItem(tr("No sparge"   ), static_cast<int>(MashAutoAdjuster::SpargeMethod::NoSparge   ));
   this->comboBox_spargeMethod->addItem(tr("Batch sparge"), static_cast<int>(MashAutoAdjuster::SpargeMethod::BatchSparge));
   this->comboBox_spargeMethod->addItem(tr("Fly sparge"  ), static_cast<int>(MashAutoAdjuster::SpargeMethod::FlySparge  ));
   this->connect(this->comboBox_spargeMethod, &QComboBox::currentIndexChanged, this, &MashEditor::updateAutoAdjustControls);
   this->connect(this->pushButton_autoAdjust , &QAbstractButton::clicked      , this, &MashEditor::autoAdjust               );
   this->connect(this->checkBox_keptInSync   , &QAbstractButton::toggled      , this, &MashEditor::keptInSyncToggled       );

   //
   // Step validation (see MashStepTableModel::validate) should use the values the user has entered for the mash's own
   // fields, even if they haven't yet been saved, and should be redone when those values change.
   //
   MashStepTableModel * const tableModel = this->enumeratedItemsWidget->tableModel();
   tableModel->setInputsAdjuster([this](MashAutoAdjuster::Inputs & inputs) { this->useFieldValues(inputs); return; });
   for (SmartLineEdit * field : {this->lineEdit_grainTemp, this->lineEdit_tunTemp}) {
      this->connect(field, &QLineEdit::textChanged, tableModel, &MashStepTableModel::scheduleValidation);
   }
   this->connect(tableModel, &MashStepTableModel::validationChanged, this, &MashEditor::updateMashProblems);
   this->connect(this->tabWidget_editor, &QTabWidget::currentChanged, this, &MashEditor::updateMashProblems);
   this->updateMashProblems();
   this->refreshAutoAdjust();

   // Undo and redo can turn keep in sync on and off (see UndoableAutoAdjust)
   this->connect(&Undoable::getStack(), &QUndoStack::indexChanged, this, &MashEditor::undoStackChanged);

   return;
}

MashEditor::~MashEditor() = default;

// Insert the boilerplate stuff that we cannot do in EditorWithRecipeBase
EDITOR_COMMON_CODE(Mash)

void MashEditor::postSetEditItem() {
   this->m_autoAdjustUndoIndex = -1;
   this->refreshAutoAdjust();
   return;
}

void MashEditor::showEvent(QShowEvent * event) {
   // When we're editing a new mash for a recipe, m_newItemForRecipe is set after setEditItem, so we need to refresh here
   // too.
   this->refreshAutoAdjust();
   this->QDialog::showEvent(event);
   // Which fields are visible affects what we show (see updateMashProblems)
   this->updateMashProblems();
   return;
}

void MashEditor::focusAutoAdjust() {
   this->tabWidget_editor->setCurrentWidget(this->tab_main);
   this->pushButton_autoAdjust->setFocus();
   return;
}

Recipe * MashEditor::recipeForAutoAdjust() const {
   if (!this->m_editItem || !this->m_recipeObs) {
      return nullptr;
   }
   if (this->m_newItemForRecipe == this->m_editItem) {
      // New mash that will be used by the recipe when the user clicks Save
      return this->m_recipeObs;
   }
   return this->currentRecipeIfItUsesEditItem();
}

void MashEditor::refreshAutoAdjust() {
   Recipe const * const recipe = this->recipeForAutoAdjust();
   QList<MashAutoAdjuster::Step> const steps =
      this->m_editItem ? MashAutoAdjuster::stepsFrom(*this->m_editItem) : QList<MashAutoAdjuster::Step>{};

   //
   // Show the sparge method and number of batches the mash currently has
   //
   MashAutoAdjuster::SpargeMethod const spargeMethod = MashAutoAdjuster::spargeMethod(steps);
   {
      QSignalBlocker const blocker{this->comboBox_spargeMethod};
      this->comboBox_spargeMethod->setCurrentIndex(
         this->comboBox_spargeMethod->findData(static_cast<int>(spargeMethod))
      );
   }
   int const numBatchSparges = std::count_if(
      steps.cbegin(), steps.cend(), [](auto const & step) { return step.type == MashStep::Type::BatchSparge; }
   );
   this->spinBox_numBatches->setValue(std::max(numBatchSparges, 1));

   //
   // Show the mash thickness the calculation would use.  (If there's no sparge, the strike water is normally all the
   // water, so its thickness isn't a useful starting point if the user switches to a sparge method.)
   //
   double thickness_LKg = MashAutoAdjuster::defaultThickness_LKg;
   if (!steps.isEmpty() && spargeMethod != MashAutoAdjuster::SpargeMethod::NoSparge) {
      MashAutoAdjuster::Step const & firstStep = steps.first();
      if (firstStep.liquorToGristRatio_lKg && *firstStep.liquorToGristRatio_lKg > 0.0) {
         thickness_LKg = *firstStep.liquorToGristRatio_lKg;
      } else if (recipe && firstStep.amount_l > 0.0) {
         thickness_LKg = recipe->mashThicknessForStrikeWater_LKg(firstStep.amount_l).value_or(thickness_LKg);
         if (thickness_LKg <= 0.0) {
            thickness_LKg = MashAutoAdjuster::defaultThickness_LKg;
         }
      }
   }
   this->lineEdit_thickness->setQuantity(thickness_LKg);

   this->comboBox_spargeMethod->setEnabled(recipe != nullptr);
   this->pushButton_autoAdjust ->setEnabled(recipe != nullptr);
   this->checkBox_keptInSync  ->setEnabled(recipe != nullptr);
   {
      QSignalBlocker const blocker{this->checkBox_keptInSync};
      this->checkBox_keptInSync->setChecked(this->m_editItem && this->m_editItem->keptInSync());
   }
   this->updateAutoAdjustControls();

   if (!this->m_editItem) {
      this->label_calculateInfo->clear();
   } else if (!recipe) {
      this->showAutoAdjustMessage(
         tr("To adjust the mash, edit it from a recipe that uses it (eg with the Edit button on the recipe's Mash tab), "
            "as the amounts and temperatures depend on the recipe's grain bill and equipment.")
      );
   } else {
      this->showAutoAdjustMessage(
         tr("Adjusts the water amounts and temperatures in the steps to suit the grain bill and equipment of recipe "
            "\"%1\".  Pinned values are not changed.  Changes to the steps take effect straight away (ie without "
            "clicking Save).").arg(recipe->name())
      );
   }
   return;
}

void MashEditor::updateAutoAdjustControls() {
   bool const enabled = (this->recipeForAutoAdjust() != nullptr);
   auto const spargeMethod =
      static_cast<MashAutoAdjuster::SpargeMethod>(this->comboBox_spargeMethod->currentData().toInt());
   this->spinBox_numBatches->setEnabled(enabled && spargeMethod == MashAutoAdjuster::SpargeMethod::BatchSparge);
   this->lineEdit_thickness->setEnabled(enabled && spargeMethod != MashAutoAdjuster::SpargeMethod::NoSparge);
   return;
}

void MashEditor::updateMashProblems() {
   MashStepTableModel const & model = *this->enumeratedItemsWidget->tableModel();

   //
   // Highlight any of the mash's own fields that have problems, with the problems in the tool tip.  (Problems with
   // individual steps are highlighted in the steps table by MashStepTableModel.)
   //
   QHash<QString, QStringList> const & fieldProblems = model.mashFieldProblems();
   std::initializer_list<std::pair<BtStringConst const &, QWidget *>> const fields{
      {PropertyNames::Mash::grainTemp_c     , this->lineEdit_grainTemp},
      {PropertyNames::Mash::initialTunTemp_c, this->lineEdit_tunTemp  },
   };
   // Problems with fields the user can't currently see (eg because they're on another tab) get listed below
   QStringList hiddenFieldProblems;
   for (auto const & [propertyName, field] : fields) {
      QStringList const problems = fieldProblems.value(*propertyName);
      MashEditor::highlightField(*field, problems);
      if (!field->isVisible()) {
         hiddenFieldProblems.append(problems);
      }
   }

   if (!model.hasProblems()) {
      this->label_mashProblems->clear();
      this->label_mashProblems->setVisible(false);
      return;
   }
   QString text = tr("⚠ This mash has problems.").toHtmlEscaped();
   //
   // Problems with steps or fields are shown by highlighting them.  Problems with the mash as a whole (eg the water
   // not adding up) have nowhere else to go, so we list them here.  So do problems with fields that are not on the tab
   // the user is looking at.
   //
   QStringList overallProblems = model.mashOverallProblems();
   overallProblems.append(hiddenFieldProblems);
   if (!overallProblems.isEmpty()) {
      text += tr("  General problems are:");
      text += "<ul>";
      for (auto const & detail : overallProblems) {
         text += QString{"<li>%1</li>"}.arg(detail.toHtmlEscaped());
      }
      text += "</ul>";
   }
   this->label_mashProblems->setText(text);
   this->label_mashProblems->setVisible(true);
   return;
}

void MashEditor::highlightField(QWidget & field, QStringList const & problems) {
   //
   // We stash the field's normal tool tip in a dynamic property the first time we need to change it, so we can put it
   // back when the problem goes away.
   //
   char const * const normalToolTipProperty = "normalToolTip";
   QVariant const normalToolTip = field.property(normalToolTipProperty);
   if (!normalToolTip.isValid()) {
      field.setProperty(normalToolTipProperty, field.toolTip());
   }
   QString const toolTip = field.property(normalToolTipProperty).toString();

   if (problems.isEmpty()) {
      field.setStyleSheet("");
      field.setToolTip(toolTip);
   } else {
      //
      // Same semi-transparent red as for steps with errors in MashStepTableModel, so it works with light and dark
      // themes.  (We don't build this string as a namespace-level constant because MashStepTableModel::errorRowColor
      // is defined in another translation unit, so might not yet be initialised when such a constant would be.)
      //
      QColor const & color = MashStepTableModel::errorRowColor;
      field.setStyleSheet(
         QString{"background-color: rgba(%1, %2, %3, %4);"}.arg(color.red()).arg(color.green()).arg(color.blue()).arg(
            color.alpha()
         )
      );
      field.setToolTip(
         problems.join("\n") + (toolTip.isEmpty() ? QString{} : QString{"\n\n"} + toolTip)
      );
   }
   return;
}

void MashEditor::useFieldValues(MashAutoAdjuster::Inputs & inputs) const {
   bool grainTempOk = false;
   double const grainTemp_c = this->lineEdit_grainTemp->getNonOptCanonicalQty(&grainTempOk);
   if (grainTempOk) {
      inputs.grainTemp_c = grainTemp_c;
   }
   inputs.tunTemp_c = this->lineEdit_tunTemp->getOptCanonicalQty().value_or(inputs.grainTemp_c);
   return;
}

bool MashEditor::ensureChangeableForRecipe(Recipe & recipe) {
   //
   // Adjusting is for this recipe, so we mustn't change a mash that other recipes use (as the results would be wrong
   // for them), nor one that is locked.  In either case, we offer to make a copy for this recipe.  (A new mash for the
   // recipe is not yet used by anything, so is fine.)
   //
   if (this->m_newItemForRecipe == this->m_editItem) {
      return true;
   }
   bool const canChange = RecipeUtils::lockReason(*this->m_editItem) == RecipeUtils::LockReason::NotLocked;
   bool const isShared  = RecipeUtils::recipesUsing(*this->m_editItem).size() > 1;
   if (canChange && !isShared) {
      return true;
   }
   if (recipe.locked()) {
      QMessageBox::information(this,
                               tr("Recipe is locked"),
                               tr("The recipe \"%1\" is locked, so its mash cannot be changed.").arg(recipe.name()));
      return false;
   }
   QMessageBox messageBox{this};
   messageBox.setIcon(QMessageBox::Question);
   messageBox.setWindowTitle(tr("Make a copy of mash?"));
   messageBox.setText(RecipeUtils::describeSharingAndLocking(*this->m_editItem, &recipe));
   messageBox.setInformativeText(
      tr("Adjusting the mash changes its steps to suit the grain bill and equipment of recipe \"%1\".\n\n"
         "Do you want to make a copy of this mash for that recipe, so that you can do this without affecting anything "
         "else?").arg(recipe.name())
   );
   QPushButton * copyButton = messageBox.addButton(tr("Make a copy"), QMessageBox::AcceptRole);
   messageBox.addButton(QMessageBox::Cancel);
   messageBox.setDefaultButton(copyButton);
   messageBox.exec();
   if (messageBox.clickedButton() != copyButton) {
      return false;
   }
   this->makeCopyForRecipe();
   return true;
}

void MashEditor::keptInSyncToggled(bool const checked) {
   Recipe * const recipe = this->recipeForAutoAdjust();
   if (!this->m_editItem || !recipe) {
      // Check box should have been disabled
      return;
   }
   //
   // Like the steps, this takes effect straight away, because it's about how the mash relates to the recipe rather
   // than a field the user is editing.  Turning it on also adjusts the mash immediately.
   //
   if (checked) {
      this->doAutoAdjust(true);
   } else {
      Undoable::doOrRedoUpdate(*this->m_editItem,
                               TYPE_INFO(Mash, keptInSync),
                               false,
                               tr("Turn off automatic adjustment of mash"));
   }
   // In case we didn't make the change (eg the user declined to make a copy of a shared mash)
   this->undoStackChanged();
   return;
}

void MashEditor::undoStackChanged() {
   // If our last auto-adjust has been undone, the message about it no longer applies
   if (this->m_autoAdjustUndoIndex >= 0 && Undoable::getStack().index() < this->m_autoAdjustUndoIndex) {
      this->showAutoAdjustMessage(tr("Auto-Adjust undone."));
      this->m_autoAdjustUndoIndex = -1;
   }
   QSignalBlocker const blocker{this->checkBox_keptInSync};
   this->checkBox_keptInSync->setChecked(this->m_editItem && this->m_editItem->keptInSync());
   if (this->m_editItem) {
      // Undo/redo of an auto-adjust changes the steps, and hence the totals.  As in doAutoAdjust, we only re-read these
      // fields, so as not to lose any unsaved changes to the others.
      using Base = EditorBase<MashEditor, Mash, MashEditorOptions>;
      this->Base::readFieldsFromEditItem(*PropertyNames::Mash::totalMashWater_l);
      this->Base::readFieldsFromEditItem(*PropertyNames::Mash::totalTime_mins  );
   }
   return;
}

void MashEditor::showAutoAdjustMessage(QString const & heading, QStringList const & details) {
   QString text = heading.toHtmlEscaped();
   if (!details.isEmpty()) {
      text += "<ul>";
      for (auto const & detail : details) {
         text += QString{"<li>%1</li>"}.arg(detail.toHtmlEscaped());
      }
      text += "</ul>";
   }
   this->label_calculateInfo->setText(text);
   return;
}

void MashEditor::autoAdjust() {
   this->doAutoAdjust(false);
   return;
}

void MashEditor::doAutoAdjust(bool const turnOnKeepInSync) {
   Recipe * const recipe = this->recipeForAutoAdjust();
   if (!recipe || !this->m_editItem) {
      // Button should have been disabled
      qWarning() << Q_FUNC_INFO << "No recipe or no mash";
      return;
   }

   if (!this->ensureChangeableForRecipe(*recipe)) {
      return;
   }
   Mash & mash = *this->m_editItem;

   //
   // Use the values in the editor for the mash's own fields, as the user might have changed them without (yet) saving.
   //
   MashAutoAdjuster::Inputs inputs = MashAutoAdjuster::Inputs::fromRecipe(*recipe, mash);
   this->useFieldValues(inputs);

   auto const spargeMethod =
      static_cast<MashAutoAdjuster::SpargeMethod>(this->comboBox_spargeMethod->currentData().toInt());
   QList<MashAutoAdjuster::Step> steps = MashAutoAdjuster::stepsFrom(mash);
   // Older mashes might have a sparge temperature of 0 °C, which really means "not set"
   std::optional<double> const spargeTemp_c = this->lineEdit_spargeTemp->getOptCanonicalQty();
   double const targetSpargeTemp_c =
      (spargeTemp_c && *spargeTemp_c > 0.0) ? *spargeTemp_c : MashAutoAdjuster::defaultSpargeTemp_c;
   MashAutoAdjuster::setSparge(steps, spargeMethod, this->spinBox_numBatches->value(), targetSpargeTemp_c);

   MashAutoAdjuster::Options options;
   options.spargeTemp_c = targetSpargeTemp_c;
   if (spargeMethod != MashAutoAdjuster::SpargeMethod::NoSparge) {
      options.thickness_LKg = this->lineEdit_thickness->getOptCanonicalQty();
   }

   MashAutoAdjuster::Result const result = MashAutoAdjuster::calculate(inputs, steps, options);
   std::shared_ptr<Mash> const mashPtr = this->m_editItem;
   if (!result.ok()) {
      if (turnOnKeepInSync) {
         // We still turn on keep in sync, as the user asked for it, and the mash will be adjusted when the problems are
         // fixed (eg when there is some grain in the recipe).
         Undoable::doOrRedoUpdate(*mashPtr,
                                  TYPE_INFO(Mash, keptInSync),
                                  true,
                                  tr("Turn on automatic adjustment of mash"));
         this->m_autoAdjustUndoIndex = Undoable::getStack().index();
         this->showAutoAdjustMessage(tr("Could not adjust the mash yet:"),
                                     result.messages(MashAutoAdjuster::Problem::Severity::Error));
      } else {
         this->showAutoAdjustMessage(tr("Could not adjust the mash, so nothing has been changed:"),
                                     result.messages(MashAutoAdjuster::Problem::Severity::Error));
      }
      return;
   }

   //
   // Make all the changes (including turning on keep in sync if that's what we're doing) a single undoable step.  See
   // UndoableAutoAdjust for how undo interacts with keep in sync.
   //
   auto * const command = new UndoableAutoAdjust(
      turnOnKeepInSync ? tr("Turn on automatic adjustment of mash") : tr("Auto-adjust mash"),
      [mashPtr]() { return mashPtr->keptInSync(); },
      [mashPtr](bool const val) { mashPtr->setKeptInSync(val); return; }
   );
   if (turnOnKeepInSync) {
      new SimpleUndoableUpdate(*mashPtr,
                               TYPE_INFO(Mash, keptInSync),
                               true,
                               tr("Turn on automatic adjustment of mash"),
                               command);
   }
   MashAutoAdjuster::makeApplyCommand(result.steps, mashPtr, command);
   Undoable::doOrRedoUpdate(command);
   this->m_autoAdjustUndoIndex = Undoable::getStack().index();

   // Update the totals, without re-reading the other fields (which would lose any unsaved changes)
   using Base = EditorBase<MashEditor, Mash, MashEditorOptions>;
   this->Base::readFieldsFromEditItem(*PropertyNames::Mash::totalMashWater_l);
   this->Base::readFieldsFromEditItem(*PropertyNames::Mash::totalTime_mins  );

   QStringList const warnings = result.messages(MashAutoAdjuster::Problem::Severity::Warning);
   if (warnings.isEmpty()) {
      this->showAutoAdjustMessage(tr("Mash adjusted for recipe \"%1\".").arg(recipe->name()));
   } else {
      this->showAutoAdjustMessage(tr("Mash adjusted for recipe \"%1\", but please check the following:").arg(
                                     recipe->name()
                                  ),
                                  warnings);
   }
   return;
}