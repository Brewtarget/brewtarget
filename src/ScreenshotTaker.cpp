/*======================================================================================================================
 * ScreenshotTaker.cpp is part of Brewtarget, and is copyright the following authors 2026:
 *   • Matt Young <mfsy@yahoo.com>
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
#include "ScreenshotTaker.h"

#include <functional>
#include <memory>

#include <QApplication>
#include <QDate>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QHeaderView>
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>
#include <QWidget>

#include "BrewLogWidget.h"
#include "MainWindow.h"
#include "PersistentSettings.h"
#include "database/ObjectStoreWrapper.h"
#include "editors/EquipmentEditor.h"
#include "editors/MashEditor.h"
#include "measurement/Unit.h"
#include "model/BrewLog.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Hop.h"
#include "model/Mash.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUtils.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "widgets/RecipeWaterWidget.h"

namespace {
   //
   // The example recipe from Chapter 1 of the manual.  If you change anything here, check that the text of the manual
   // still matches.  All the items named here have to be in the default content (see data/DefaultContent*).
   //
   QString const recipeName            {"Best Bitter"                          };
   QString const equipmentName         {"Grainfather G30 (v3)"                 };
   QString const styleName             {"Best Bitter"                          };
   QString const paleMaltName          {"Maris Otter"                          };
   QString const crystalMaltName       {"Caramel/Crystal Malt – 60L"           };
   QString const bitteringHopName      {"Challenger"                           };
   QString const aromaHopName          {"East Kent Goldings"                   };
   QString const yeastName             {"SafAle S-04"                          };
   QString const mashName              {"Single Infusion, Medium Body, Fly Sparge"};
   QString const baseWaterName         {"Paris tap water (2026)"               };
   QString const targetWaterName       {"Amber, Hoppy (Bitter, ESB, Amber Ale)"};
   double  const mashThickness_lKg     {2.7 };
   double  const targetMash_pH         {5.4 };
   double  const fermentationTemp_c    {19.0};
   double  const fermentationTime_days {10.0};
   //! Fixed, so that the screenshots don't change every time they are generated
   QDate   const brewDate              {2026, 3, 14};

   //
   // Logical sizes (ie before any scaling from QT_SCALE_FACTOR) of the windows we take screenshots of
   //
   QSize const mainWindowSize     {1600, 1000};
   QSize const equipmentEditorSize{ 900,  540};
   QSize const mashEditorSize     { 960,  700};

   /**
    * \brief Let Qt process pending events (including any zero-length timers and deferred layouts), so that what we grab
    *        is up to date.
    */
   void settle() {
      for (int ii = 0; ii < 5; ++ii) {
         QApplication::processEvents();
      }
      return;
   }

   /**
    * \brief Find an item in the database by name, logging an error if it isn't there
    */
   template<class NE> std::shared_ptr<NE> findByName(QString const & name) {
      auto result = ObjectStoreWrapper::findFirstMatching<NE>(
         [&name](std::shared_ptr<NE> ne) { return ne->name() == name; }
      );
      if (!result) {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Could not find" << NE::staticMetaObject.className() << "\"" << name << "\" in the database "
            "(has the default content changed?)";
      }
      return result;
   }

   template<class RA, class Ingr> std::shared_ptr<RA> addToRecipe(Recipe & recipe,
                                                                  Ingr & ingredient,
                                                                  Measurement::Amount const & amount) {
      auto addition = std::make_shared<RA>(recipe, ingredient);
      addition->setAmount(amount);
      recipe.addAddition<RA>(addition);
      return addition;
   }

   class Taker {
   public:
      Taker(MainWindow & mainWindow, QDir const & directory) : m_mainWindow{mainWindow}, m_directory{directory} {
         return;
      }

      /**
       * \brief Save a screenshot of \c widget as \c fileName in the output directory
       */
      bool shoot(QWidget & widget, QString const & fileName) {
         settle();
         QString const filePath = this->m_directory.filePath(fileName);
         QPixmap const pixmap = widget.grab();
         if (pixmap.isNull() || !pixmap.save(filePath, "PNG")) {
            qCritical().noquote() << Q_FUNC_INFO << "Unable to save screenshot" << filePath;
            return false;
         }
         qInfo().noquote() <<
            Q_FUNC_INFO << "Saved" << filePath << "(" << pixmap.width() << "x" << pixmap.height() << "pixels)";
         return true;
      }

      bool run() {
         MainWindow & mw = this->m_mainWindow;
         mw.resize(mainWindowSize);
         // Give the trees on the left enough room to show item names
         mw.splitter_vertical->setSizes({420, mainWindowSize.width() - 420});
         settle();

         //
         // Find everything we need up front, so that we fail early (and with a clear message) if the default content
         // has changed.
         //
         auto const equipment     = findByName<Equipment>(equipmentName   );
         auto const style         = findByName<Style    >(styleName       );
         auto const paleMalt      = findByName<Fermentable>(paleMaltName  );
         auto const crystalMalt   = findByName<Fermentable>(crystalMaltName);
         auto const bitteringHop  = findByName<Hop      >(bitteringHopName);
         auto const aromaHop      = findByName<Hop      >(aromaHopName    );
         auto const yeast         = findByName<Yeast    >(yeastName       );
         auto const exampleMash   = findByName<Mash     >(mashName        );
         auto const baseWater     = findByName<Water    >(baseWaterName   );
         auto const targetWater   = findByName<Water    >(targetWaterName );
         if (!equipment || !style || !paleMalt || !crystalMalt || !bitteringHop || !aromaHop || !yeast ||
             !exampleMash || !baseWater || !targetWater) {
            return false;
         }

         //
         // Equipment: make it the default (as the manual suggests), so that the new recipe picks it up
         //
         PersistentSettings::insert_ck(PersistentSettings::Names::defaultEquipmentKey, equipment->key());
         {
            auto & editor = mw.getEditor<Equipment>();
            editor.setEditItem(equipment);
            editor.resize(equipmentEditorSize);
            editor.show();
            editor.tabWidget_editor->setCurrentIndex(0);
            if (!this->shoot(editor, "equipmentEditor.png")) { return false; }
            editor.hide();
         }

         //
         // New recipe, with style
         //
         std::shared_ptr<Recipe> const recipe = mw.createRecipe(recipeName);
         if (!recipe) {
            qCritical() << Q_FUNC_INFO << "Unable to create recipe";
            return false;
         }
         // Collapse the folders of example recipes, so that the new recipe is easy to see in the tree
         mw.treeView_recipe->collapseAll();
         mw.treeView_recipe->header()->resizeSection(0, 200);
         mw.treeView_recipe->setSelected(mw.treeView_recipe->findElement(recipe.get()));
         recipe->setStyle(style);
         mw.tabWidget_recipeView->setCurrentWidget(mw.tab_recipe);
         if (!this->shoot(*mw.tabWidget_recipeView, "recipeTab.png")) { return false; }

         //
         // Ingredients
         //
         using Measurement::Units::kilograms;
         addToRecipe<RecipeAdditionFermentable>(*recipe, *paleMalt   , Measurement::Amount{5.00, kilograms});
         addToRecipe<RecipeAdditionFermentable>(*recipe, *crystalMalt, Measurement::Amount{0.40, kilograms});

         // Step 2 is the boil proper in the standard boil profile that a new recipe gets (see Boil::ensureStandardProfile)
         auto bittering = addToRecipe<RecipeAdditionHop>(*recipe, *bitteringHop, Measurement::Amount{0.035, kilograms});
         bittering->setStage(RecipeAddition::Stage::Boil);
         bittering->setStep(2);
         bittering->setAddAtTime_mins(60.0);
         auto aroma = addToRecipe<RecipeAdditionHop>(*recipe, *aromaHop, Measurement::Amount{0.030, kilograms});
         aroma->setStage(RecipeAddition::Stage::Boil);
         aroma->setStep(2);
         aroma->setAddAtTime_mins(15.0);

         addToRecipe<RecipeAdditionYeast>(*recipe, *yeast, Measurement::Amount{0.0115, kilograms});

         mw.tabWidget_ingredients->setCurrentWidget(mw.recipeFermentableTab);
         if (!this->shoot(*mw.tabWidget_ingredients, "fermentablesTab.png")) { return false; }

         //
         // Mash: use the (locked) example profile, then do what the user does when they click Auto-Adjust in the mash
         // editor and choose to make a copy.  We make the copy here rather than via the editor, as the editor asks about
         // it with a message box.
         //
         recipe->setMash(exampleMash);
         RecipeUtils::useCopyFor<Mash>(*recipe);
         mw.editRecipeMash();
         {
            auto & editor = mw.getEditor<Mash>();
            editor.resize(mashEditorSize);
            settle();
            editor.lineEdit_thickness->setQuantity(mashThickness_lKg);
            // Ticking this box does an Auto-Adjust and keeps the mash in sync from then on
            editor.checkBox_keptInSync->setChecked(true);
            if (!this->shoot(editor, "mashEditor.png")) { return false; }
            editor.pushButton_save->click();
            settle();
         }

         //
         // Water
         //
         // We choose the profiles in the drop-downs, as the user would, as the widget only reads them from the recipe
         // when it is first shown.
         mw.tabWidget_recipeView->setCurrentWidget(mw.tab_water);
         settle();
         {
            RecipeWaterWidget & water = *mw.recipeWaterWidget;
            water.combo_waterBase->setItem(baseWater);
            water.updateBaseProfile(water.combo_waterBase->currentIndex());
            water.combo_waterTarget->setItem(targetWater);
            water.updateTargetProfile(water.combo_waterTarget->currentIndex());
            water.checkBox_targetMashPh->setChecked(true);
            water.doubleSpinBox_targetMashPh->setValue(targetMash_pH);
            settle();
            water.autoAdjust();
         }
         if (!this->shoot(*mw.tabWidget_recipeView, "waterTab.png")) { return false; }

         //
         // Fermentation
         //
         if (auto const fermentation = recipe->fermentation(); fermentation && !fermentation->steps().isEmpty()) {
            auto const primary = fermentation->steps().first();
            primary->setStartTemp_c  (fermentationTemp_c   );
            primary->setStepTime_days(fermentationTime_days);
         }

         //
         // Finished recipe
         //
         mw.tabWidget_recipeView->setCurrentWidget(mw.tab_recipe);
         if (!this->shoot(*mw.tabWidget_recipeView, "recipeTabDone.png")) { return false; }
         mw.tabWidget_ingredients->setCurrentWidget(mw.recipeHopTab);
         if (!this->shoot(mw, "mainWindow.png")) { return false; }

         //
         // Brew day
         //
         //
         // NB: Brew It! acts on the recipe selected in the tree, so we make sure that is our recipe.
         //
         mw.treeView_recipe->setSelected(mw.treeView_recipe->findElement(recipe.get()));
         settle();
         mw.brewItHelper();
         settle();
         //
         // Brew It! opens the new brew log in a tab of its own.  (We get it from there rather than searching by recipe
         // because, with automatic snapshots turned on, brewing a recipe can change which recipe record it belongs to.)
         //
         auto const brewLogWidget = qobject_cast<BrewLogWidget *>(mw.tabWidget_recipeView->currentWidget());
         BrewLog * const brewLog = brewLogWidget ? brewLogWidget->brewLog() : nullptr;
         if (!brewLog) {
            qCritical() << Q_FUNC_INFO << "No brew log after Brew It!";
            return false;
         }
         brewLog->setBrewDate(brewDate);
         brewLog->setName("BB-01");
         // Plausible measurements, a little short of what the recipe predicted
         brewLog->setMeasuredStrikeTemp_c        (72.5  );
         brewLog->setMeasuredMashFinalTemp_c     (66.0  );
         brewLog->setMeasuredPreBoilGravity_sg   (1.033 );
         brewLog->setMeasuredPreBoilVolume_l     (30.0  );
         brewLog->setMeasuredOriginalGravity_sg  (1.040 );
         brewLog->setMeasuredPostBoilVolume_l    (27.0  );
         brewLog->setMeasuredVolumeIntoFv_l      (24.5  );
         brewLog->setMeasuredPitchTemp_c         (18.0  );
         if (!this->shoot(*mw.tabWidget_recipeView, "brewLog.png")) { return false; }

         return true;
      }

   private:
      MainWindow & m_mainWindow;
      QDir const m_directory;
   };
}

bool ScreenshotTaker::takeAll(MainWindow & mainWindow, QString const & directory) {
   QDir const dir{directory};
   if (!dir.mkpath(".")) {
      qCritical().noquote() << Q_FUNC_INFO << "Unable to create directory" << directory;
      return false;
   }
   qInfo().noquote() << Q_FUNC_INFO << "Taking screenshots in" << dir.absolutePath();

   //
   // Nobody is watching, so a modal dialog (eg a QMessageBox) would wait for ever.  The code above tries to avoid
   // anything that shows one but, in case something does, we check for them on a timer (which still runs while a modal
   // dialog has its own event loop), log what the dialog said, and close it.  Since this means something has not gone
   // as planned, the screenshots then count as failed.
   //
   bool sawUnexpectedDialog = false;
   QTimer dialogWatchdog;
   QObject::connect(&dialogWatchdog, &QTimer::timeout, [&sawUnexpectedDialog]() {
      QWidget * const modal = QApplication::activeModalWidget();
      if (!modal) {
         return;
      }
      sawUnexpectedDialog = true;
      if (auto const messageBox = qobject_cast<QMessageBox *>(modal)) {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Closing unexpected message box \"" << messageBox->windowTitle() << "\":" <<
            messageBox->text() << messageBox->informativeText();
      } else {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Closing unexpected dialog" << modal->metaObject()->className() << modal->windowTitle();
      }
      if (auto const dialog = qobject_cast<QDialog *>(modal)) {
         dialog->reject();
      } else {
         modal->close();
      }
      return;
   });
   dialogWatchdog.start(500);

   Taker taker{mainWindow, dir};
   bool const succeeded = taker.run() && !sawUnexpectedDialog;
   dialogWatchdog.stop();
   qInfo() << Q_FUNC_INFO << (succeeded ? "Finished taking screenshots" : "Failed to take screenshots");
   return succeeded;
}
