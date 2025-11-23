/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * MainWindow.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Aidan Roberts <aidanr67@gmail.com>
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Dan Cavanagh <dan@dancavanagh.com>
 *   • Daniel Moreno <danielm5@users.noreply.github.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • David Grundberg <individ@acc.umu.se>
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Markus Mårtensson <mackan.90@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Ryan Hoobler <rhoob@yahoo.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Ted Wright <tedwright@users.sourceforge.net>
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
#include "MainWindow.h"

#if defined(Q_OS_WIN)
   #include <windows.h>
#endif

#include <algorithm>
#include <memory>
#include <mutex> // For std::once_flag etc

#include <QAction>
#include <QBrush>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QIODevice>
#include <QLinearGradient>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QPen>
#include <QPixmap>
#include <QScreen>
#include <QSize>
#include <QString>
#include <QTextStream>
#include <QtGui>
#include <QToolButton>
#include <QUndoStack>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "AboutDialog.h"
#include "AlcoholTool.h"
#include "Algorithms.h"
#include "AncestorDialog.h"
#include "Application.h"
#include "BrewNoteWidget.h"
#include "BtDatePopup.h"
#include "model/Folder.h"
#include "BtHorizontalTabs.h"
#include "BtTabWidget.h"
#include "ConverterTool.h"
#include "HelpDialog.h"
#include "Html.h"
#include "HydrometerTool.h"
#include "StockFormatter.h"
#include "StockWindow.h"
#include "MashDesigner.h"
#include "MashWizard.h"
#include "OgAdjuster.h"
#include "OptionDialog.h"
#include "PersistentSettings.h"
#include "PitchDialog.h"
#include "PrimingDialog.h"
#include "PrintAndPreviewDialog.h"
#include "RangedSlider.h"
#include "RecipeFormatter.h"
#include "RefractoDialog.h"
#include "ScaleRecipeTool.h"
#include "StrikeWaterDialog.h"
#include "TimerMainDialog.h"
#include "WaterProfileAdjustmentTool.h"
#include "catalogs/BoilCatalog.h"
#include "catalogs/EquipmentCatalog.h"
#include "catalogs/FermentableCatalog.h"
#include "catalogs/FermentationCatalog.h"
#include "catalogs/HopCatalog.h"
#include "catalogs/MashCatalog.h"
#include "catalogs/MiscCatalog.h"
#include "catalogs/SaltCatalog.h"
#include "catalogs/StyleCatalog.h"
#include "catalogs/WaterCatalog.h"
#include "catalogs/YeastCatalog.h"
#include "config.h"
#include "database/Database.h"
#include "database/ObjectStoreWrapper.h"
#include "editors/BoilEditor.h"
#include "editors/BoilStepEditor.h"
#include "editors/EquipmentEditor.h"
#include "editors/FermentableEditor.h"
#include "editors/FermentationEditor.h"
#include "editors/FermentationStepEditor.h"
#include "editors/HopEditor.h"
#include "editors/MashEditor.h"
#include "editors/MashStepEditor.h"
#include "editors/MiscEditor.h"
#include "editors/SaltEditor.h"
#include "editors/StyleEditor.h"
#include "editors/WaterEditor.h"
#include "editors/YeastEditor.h"
#include "measurement/ColorMethods.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Boil.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/Mash.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeAdjustmentSalt.h"
#include "model/Style.h"
#include "model/Yeast.h"
#include "serialization/ImportExport.h"
#include "qtModels/sortFilterProxyModels/FermentableSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionFermentableSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionHopSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionMiscSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdditionYeastSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/RecipeAdjustmentSaltSortFilterProxyModel.h"
#include "qtModels/sortFilterProxyModels/StyleSortFilterProxyModel.h"
#include "qtModels/tableModels/BoilStepTableModel.h"
#include "qtModels/tableModels/FermentableTableModel.h"
#include "qtModels/tableModels/FermentationStepTableModel.h"
#include "qtModels/tableModels/MashStepTableModel.h"
#include "qtModels/tableModels/RecipeAdditionFermentableTableModel.h"
#include "qtModels/tableModels/RecipeAdditionHopTableModel.h"
#include "qtModels/tableModels/RecipeAdditionMiscTableModel.h"
#include "qtModels/tableModels/RecipeAdditionYeastTableModel.h"
#include "qtModels/tableModels/RecipeAdjustmentSaltTableModel.h"
#include "undoRedo/RelationalUndoableUpdate.h"
#include "undoRedo/Undoable.h"
#include "undoRedo/UndoableAddOrRemove.h"
#include "undoRedo/UndoableAddOrRemoveList.h"
#include "utils/BtStringConst.h"
#include "utils/VeriTable.h"
#include "utils/OptionalHelpers.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MainWindow.cpp"
#endif

namespace {
   using VoidFunctionNoParams = void (MainWindow::*)(void);

   template<class NE> constexpr VoidFunctionNoParams callbackFunctionFor();
   template<> constexpr VoidFunctionNoParams callbackFunctionFor<Style       >() { return &MainWindow::updateStyleInUi;        }
   template<> constexpr VoidFunctionNoParams callbackFunctionFor<Equipment   >() { return &MainWindow::updateEquipmentInUi;    }
   template<> constexpr VoidFunctionNoParams callbackFunctionFor<Mash        >() { return &MainWindow::updateMashInUi;         }
   template<> constexpr VoidFunctionNoParams callbackFunctionFor<Boil        >() { return &MainWindow::updateBoilInUi;         }
   template<> constexpr VoidFunctionNoParams callbackFunctionFor<Fermentation>() { return &MainWindow::updateFermentationInUi; }
   template<> constexpr VoidFunctionNoParams callbackFunctionFor<Water       >() { return nullptr; }

   /**
    * \brief Generates the pop-up you see when you hover over the application logo image above the trees, which is
    *        supposed to show the database type you are connected to, and some useful related information.
    */
   QString getLabelToolTip() {
      Database const & database = Database::instance();
      QString toolTip{};
      QTextStream toolTipAsStream{&toolTip};
      toolTipAsStream <<
         "<html><head><style type=\"text/css\">" << Html::getCss(":/css/tooltip.css") << "</style></head>"
         "<body>"
         "<div id=\"headerdiv\">"
         "<table id=\"tooltip\">"
         "<caption>Using " << DatabaseHelper::getNameFromDbTypeName(database.dbType()) << "</caption>";
      auto connectionParms = database.displayableConnectionParms();
      for (auto parm : connectionParms) {
         toolTipAsStream <<
            "<tr><td class=\"left\">" << parm.first << ": </td><td class=\"value\">" << parm.second << "</td>";
      }
      toolTipAsStream << "</table></body></html>";
      return toolTip;
   }

   // We only want one instance of MainWindow, but we'd also like to be able to delete it when the program shuts down
   MainWindow * mainWindowInstance = nullptr;

   void createMainWindowInstance() {
      mainWindowInstance = new MainWindow();
      return;
   }

   /**
    *
    */
   void updateDensitySlider(RangedSlider & slider,
                            SmartLabel const & label,
                            double const minCanonicalValue,
                            double const maxCanonicalValue,
                            double const maxPossibleCanonicalValue) {
      slider.setPreferredRange(label.getRangeToDisplay(minCanonicalValue, maxCanonicalValue        ));
      slider.setRange         (label.getRangeToDisplay(1.000            , maxPossibleCanonicalValue));

      Measurement::UnitSystem const & displayUnitSystem = label.getDisplayUnitSystem();
      if (displayUnitSystem == Measurement::UnitSystems::density_Plato) {
         slider.setPrecision(1);
         slider.setTickMarks(2, 5);
      } else {
         slider.setPrecision(3);
         slider.setTickMarks(0.010, 2);
      }
      return;
   }

   /**
    *
    */
   void updateColorSlider(RangedSlider & slider,
                          SmartLabel const & label,
                          double const minCanonicalValue,
                          double const maxCanonicalValue) {
      slider.setPreferredRange(label.getRangeToDisplay(minCanonicalValue, maxCanonicalValue));
      slider.setRange         (label.getRangeToDisplay(1                , 44               ));

      Measurement::UnitSystem const & displayUnitSystem = label.getDisplayUnitSystem();
      slider.setTickMarks(displayUnitSystem == Measurement::UnitSystems::color_StandardReferenceMethod ? 10 : 40, 2);

      return;
   }
}

// This private implementation class holds all private non-virtual members of MainWindow
class MainWindow::impl {
public:

   impl(MainWindow & self) :
      m_self{self},
      m_fermentableAdditionsVeriTable{},
      m_hopAdditionsVeriTable        {},
      m_miscAdditionsVeriTable       {},
      m_yeastAdditionsVeriTable      {},
      m_saltAdditionsVeriTable       {} {
      return;
   }

   ~impl() = default;

   /**
    * \brief Configure the tables and their proxies
    *
    *        Anything creating new tables models, filter proxies and configuring the two should go in here
    */
   void setupTables() {
      this->m_fermentableAdditionsVeriTable.setup(m_self.fermentableAdditionTable, this->m_fermentableEditor.get());
      this->        m_hopAdditionsVeriTable.setup(m_self.        hopAdditionTable, this->        m_hopEditor.get());
      this->       m_miscAdditionsVeriTable.setup(m_self.       miscAdditionTable, this->       m_miscEditor.get());
      this->      m_yeastAdditionsVeriTable.setup(m_self.      yeastAdditionTable, this->      m_yeastEditor.get());
      this->       m_saltAdditionsVeriTable.setup(m_self.       saltAdditionTable, this->       m_saltEditor.get());

      // Make the fermentable table show grain percentages in row headers.
      this->m_fermentableAdditionsVeriTable.m_tableModel->setDisplayPercentages(true);

      // RecipeAdditionHop table show IBUs in row headers.
      this->m_hopAdditionsVeriTable.m_tableModel->setShowIBUs(true);

      // Enable sorting in the main tables.
      this->m_fermentableAdditionsVeriTable.setSortColumn(RecipeAdditionFermentableTableModel::ColumnIndex::Amount);
      this->        m_hopAdditionsVeriTable.setSortColumn(        RecipeAdditionHopTableModel::ColumnIndex::Time  );
      this->       m_miscAdditionsVeriTable.setSortColumn(       RecipeAdditionMiscTableModel::ColumnIndex::Time  );
      this->      m_yeastAdditionsVeriTable.setSortColumn(      RecipeAdditionYeastTableModel::ColumnIndex::Name  );
      this->       m_saltAdditionsVeriTable.setSortColumn(     RecipeAdjustmentSaltTableModel::ColumnIndex::Name  );

      return;
   }

   //! \brief Previously called setupContextMenu
   void setupTreeViews() {

      m_self.treeView_recipe->init(*this->m_ancestorDialog, *this->m_optionDialog);

      m_self.treeView_style       ->init(*this->m_styleEditor       );
      m_self.treeView_equipment   ->init(*this->m_equipmentEditor   );
      m_self.treeView_mash        ->init(*this->m_mashEditor        );
      m_self.treeView_boil        ->init(*this->m_boilEditor        );
      m_self.treeView_fermentation->init(*this->m_fermentationEditor);
      m_self.treeView_fermentable ->init(*this->m_fermentableEditor );
      m_self.treeView_hop         ->init(*this->m_hopEditor         );
      m_self.treeView_misc        ->init(*this->m_miscEditor        );
      m_self.treeView_salt        ->init(*this->m_saltEditor        );
      m_self.treeView_yeast       ->init(*this->m_yeastEditor       );
      m_self.treeView_water       ->init(*this->m_waterEditor       );

      connect(m_self.treeView_recipe, &RecipeTreeView::recipeSpawn, &m_self, &MainWindow::versionedRecipe);
      return;
   }

   /**
    * \brief Create the dialogs, including the file dialogs
    *
    *        Most dialogs are initialized in here. That should include any initial configurations as well.
    */
   void setupDialogs() {
      // Catalogs
      m_boilCatalog         = std::make_unique<        BoilCatalog>(&m_self);
      m_equipmentCatalog    = std::make_unique<   EquipmentCatalog>(&m_self);
      m_fermentableCatalog  = std::make_unique< FermentableCatalog>(&m_self);
      m_fermentationCatalog = std::make_unique<FermentationCatalog>(&m_self);
      m_hopCatalog          = std::make_unique<         HopCatalog>(&m_self);
      m_mashCatalog         = std::make_unique<        MashCatalog>(&m_self);
      m_miscCatalog         = std::make_unique<        MiscCatalog>(&m_self);
      m_saltCatalog         = std::make_unique<        SaltCatalog>(&m_self);
      m_styleCatalog        = std::make_unique<       StyleCatalog>(&m_self);
      m_waterCatalog        = std::make_unique<       WaterCatalog>(&m_self);
      m_yeastCatalog        = std::make_unique<       YeastCatalog>(&m_self);

      // Editors
      m_boilEditor                 = std::make_unique<BoilEditor                >(&m_self);
      m_boilStepEditor             = std::make_unique<BoilStepEditor            >(&m_self);
      m_equipmentEditor            = std::make_unique<EquipmentEditor           >(&m_self);
      m_fermentableEditor          = std::make_unique<FermentableEditor         >(&m_self);
      m_fermentationEditor         = std::make_unique<FermentationEditor        >(&m_self);
      m_fermentationStepEditor     = std::make_unique<FermentationStepEditor    >(&m_self);
      m_hopEditor                  = std::make_unique<HopEditor                 >(&m_self);
      m_mashEditor                 = std::make_unique<MashEditor                >(&m_self);
      m_mashStepEditor             = std::make_unique<MashStepEditor            >(&m_self);
      m_miscEditor                 = std::make_unique<MiscEditor                >(&m_self);
      m_saltEditor                 = std::make_unique<SaltEditor                >(&m_self);
      m_styleEditor                = std::make_unique<StyleEditor               >(&m_self);
      m_waterEditor                = std::make_unique<WaterEditor               >(&m_self);
      m_yeastEditor                = std::make_unique<YeastEditor               >(&m_self);

      // Other
      m_aboutDialog                = std::make_unique<AboutDialog               >(&m_self);
      m_helpDialog                 = std::make_unique<HelpDialog                >(&m_self);
      m_mashWizard                 = std::make_unique<MashWizard                >(&m_self);
      m_stockWindow                = std::make_unique<StockWindow               >(&m_self);
      m_optionDialog               = std::make_unique<OptionDialog              >(&m_self);
      m_recipeScaler               = std::make_unique<ScaleRecipeTool           >(&m_self);
      m_recipeFormatter            = std::make_unique<RecipeFormatter           >(&m_self);
      m_printAndPreviewDialog      = std::make_unique<PrintAndPreviewDialog     >(&m_self);
      m_ogAdjuster                 = std::make_unique<OgAdjuster                >(&m_self);
      m_converterTool              = std::make_unique<ConverterTool             >(&m_self);
      m_hydrometerTool             = std::make_unique<HydrometerTool            >(&m_self);
      m_alcoholTool                = std::make_unique<AlcoholTool               >(&m_self);
      m_timerMainDialog            = std::make_unique<TimerMainDialog           >(&m_self);
      m_primingDialog              = std::make_unique<PrimingDialog             >(&m_self);
      m_strikeWaterDialog          = std::make_unique<StrikeWaterDialog         >(&m_self);
      m_refractoDialog             = std::make_unique<RefractoDialog            >(&m_self);
      m_mashDesigner               = std::make_unique<MashDesigner              >(&m_self);
      m_pitchDialog                = std::make_unique<PitchDialog               >(&m_self);
      m_btDatePopup                = std::make_unique<BtDatePopup               >(&m_self);
      m_waterProfileAdjustmentTool = std::make_unique<WaterProfileAdjustmentTool>(&m_self);
      m_ancestorDialog             = std::make_unique<AncestorDialog            >(&m_self);

      return;
   }

   /**
    * \brief Configure combo boxes and their list models
    *
    *        Any new combo boxes, along with their list models, should be initialized here
    */
   void setupComboBoxes() {
      m_self.   equipmentComboBox->init();
      m_self.       styleComboBox->init();
      m_self.        mashComboBox->init();
      m_self.        boilComboBox->init();
      m_self.fermentationComboBox->init();

      return;
   }

   /**
    * \brief Use this for adding \c RecipeAdditionHop etc
    *
    * \param ra The recipe addition object - eg \c RecipeAdditionFermentable, \c RecipeAdditionHop, etc
    */
   template<class RA>
   void doRecipeAddition(std::shared_ptr<RA> ra) {
      Q_ASSERT(ra);

      Undoable::doOrRedoUpdate(
         newUndoableAddOrRemove(*this->m_recipeObs,
                                &Recipe::addAddition<RA>,
                                ra,
                                &Recipe::removeAddition<RA>,
                                QString(tr("Add %1 to recipe")).arg(RA::localisedName()))
      );

      //
      // Since we just added an ingredient, switch the focus to the tab that lists that type of ingredient.  We rely here
      // on the individual tabs following a naming convention (recipeHopTab, recipeFermentableTab, etc)
      // Note that we want the untranslated class name because this is not for display but to refer to a QWidget inside
      // tabWidget_ingredients
      //
      auto const widgetName = QString("recipe%1Tab").arg(RA::IngredientClass::staticMetaObject.className());
      qDebug() << Q_FUNC_INFO << widgetName;
      QWidget * widget = this->m_self.tabWidget_ingredients->findChild<QWidget *>(widgetName);
      Q_ASSERT(widget);
      this->m_self.tabWidget_ingredients->setCurrentWidget(widget);

      // We don't need to call this->pimpl->m_hopAdditionsTableModel->addHop(hop) here (or the equivalent for fermentable, misc or
      // yeast) because the change to the recipe will already have triggered the necessary updates to
      // this->pimpl->m_hopAdditionsTableModel/this->pimpl->m_fermentableTableModel/etc.
      return;
   }

   // This Identifier struct is a "trick" to use overloading to get around the fact that we can't specialise a templated
   // function inside the class declaration.
   template<typename T> struct Identifier { typedef T type; };
   auto & stepEditor(Identifier<MashStep        > const) const { return *this->        m_mashStepEditor; }
   auto & stepEditor(Identifier<BoilStep        > const) const { return *this->        m_boilStepEditor; }
   auto & stepEditor(Identifier<FermentationStep> const) const { return *this->m_fermentationStepEditor; }
   template<class StepClass> auto & getStepEditor() const { return this->stepEditor(Identifier<StepClass>{}); }

   // Here we have a parameter anyway, so we can just use overloading directly
   void setStepOwner(std::shared_ptr<Mash> stepOwner) {
      this->m_recipeObs->setMash(stepOwner);
      this->m_mashStepEditor->setOwner(stepOwner);
      this->m_self.mashButton->setMash(stepOwner);
      this->m_self.mashStepsWidget->setOwner(stepOwner);
      return;
   }
   void setStepOwner(std::shared_ptr<Boil> stepOwner) {
      this->m_recipeObs->setBoil(stepOwner);
      this->m_boilStepEditor->setOwner(stepOwner);
      this->m_self.boilButton->setBoil(stepOwner);
      this->m_self.boilStepsWidget->setOwner(stepOwner);
      return;
   }
   void setStepOwner(std::shared_ptr<Fermentation> stepOwner) {
      this->m_recipeObs->setFermentation(stepOwner);
      this->m_fermentationStepEditor->setOwner(stepOwner);
      this->m_self.fermentationButton->setFermentation(stepOwner);
      this->m_self.fermentationStepsWidget->setOwner(stepOwner);
      return;
   }

   template<class StepClass> void showStepEditor(std::shared_ptr<StepClass> step) {
      auto & stepEditor = this->getStepEditor<StepClass>();
      stepEditor.setEditItem(step);
      stepEditor.setVisible(true);
      return;
   }

   //! \brief Fix pixel dimensions according to dots-per-inch (DPI) of screen we're on.
   void setSizesInPixelsBasedOnDpi() {
      //
      // Default icon sizes are fine for low DPI monitors, but need changing on high-DPI systems.
      //
      // Fortunately, the icons are already SVGs, so we don't need to do anything more complicated than tell Qt what size
      // in pixels to render them.
      //
      // For the moment, we assume we don't need to change the icon size after set-up.  (In theory, it would be nice
      // to detect, on a multi-monitor system, whether we have moved from a high DPI to a low DPI screen or vice versa.
      // See https://doc.qt.io/qt-5/qdesktopwidget.html#screen-geometry for more on this.
      // But, for now, TBD how important a use case that is.  Perhaps a future enhancement...)
      //
      // Low DPI monitors are 72 or 96 DPI typically.  High DPI monitors can be 168 DPI (as reported by logicalDpiX(),
      // logicalDpiX()).  Default toolbar icon size of 22×22 looks fine on low DPI monitor.  So it seems 1/4-inch is a
      // good width and height for these icons.  Therefore divide DPI by 4 to get icon size.
      //
      auto const dpiX = this->m_self.logicalDpiX();
      auto const dpiY = this->m_self.logicalDpiY();
      qDebug() << QString("Logical DPI: %1,%2.  Physical DPI: %3,%4")
         .arg(dpiX)
         .arg(dpiY)
         .arg(this->m_self.physicalDpiX())
         .arg(this->m_self.physicalDpiY());
      auto const defaultToolBarIconSize = this->m_self.toolBar->iconSize();
      qDebug() <<
         Q_FUNC_INFO << "Default toolbar icon size:" << defaultToolBarIconSize.width() << "×" <<
         defaultToolBarIconSize.height();
      this->m_self.toolBar->setIconSize(QSize(dpiX/4,dpiY/4));

      //
      // Historically, tab icon sizes were, by default, smaller (16×16), but it seems more logical for them to be the same
      // size as the toolbar ones.
      //
      auto defaultTabIconSize = this->m_self.tabWidget_Trees->iconSize();
      qDebug() <<
         Q_FUNC_INFO << "Default tab icon size:" << defaultTabIconSize.width() << "×" << defaultTabIconSize.height();
      this->m_self.tabWidget_Trees->setIconSize(QSize(dpiX/4,dpiY/4));

      //
      // Default logo size is 100×30 pixels, which is actually the wrong aspect ratio for the underlying image (currently
      // 265 × 66 - ie aspect ratio of 4.015:1).
      //
      // Setting height to be 1/3 inch seems plausible for the default size, but looks a bit wrong in practice.  Using 1/2
      // height looks better.  Then width 265/66 × height.  (Note that we actually put the fraction in double literals to
      // avoid premature rounding.)
      //
      // This is a bit more work to implement because its a PNG image in a QLabel object
      //
      qDebug() <<
         Q_FUNC_INFO << "Logo default size:" << this->m_self.label_Brewtarget->width() << "×" <<
         this->m_self.label_Brewtarget->height();
      this->m_self.label_Brewtarget->setScaledContents(true);
      this->m_self.label_Brewtarget->setFixedSize((265.0/66.0) * dpiX/2,  // width = 265/66 × height = 265/66 × half an inch = (265/66) × (dpiX/2)
                                               dpiY/2);                // height = half an inch = dpiY/2
      qDebug() <<
         Q_FUNC_INFO << "Logo new size:" << this->m_self.label_Brewtarget->width() << "×" <<
         this->m_self.label_Brewtarget->height();

      return;
   }

   //! \brief Find an open brew note tab
   BrewNoteWidget * findBrewNoteWidget(BrewNote const & brewNote) const {
      for (int ii = 0; ii < this->m_self.tabWidget_recipeView->count(); ++ii) {
         if (this->m_self.tabWidget_recipeView->widget(ii)->objectName() ==
             BrewNoteWidget::staticMetaObject.className()) {
            BrewNoteWidget* ni = qobject_cast<BrewNoteWidget*>(this->m_self.tabWidget_recipeView->widget(ii));
            if (&brewNote == ni->brewNote()) {
               return ni;
            }
         }
      }
      return nullptr;
   }

   void updateBrewNoteTabText(BrewNote const & brewNote, BrewNoteWidget const & widget) const {
      auto const tabIndex = this->m_self.tabWidget_recipeView->indexOf(&widget);
      this->m_self.tabWidget_recipeView->setTabText(tabIndex, brewNote.brewDate_short());
      this->m_self.tabWidget_recipeView->setTabToolTip(
         tabIndex,
         tr("Brew Note #%1 for brew on %2").arg(brewNote.key()).arg(brewNote.brewDate_short())
      );
      this->m_self.tabWidget_recipeView->setTabWhatsThis(
         tabIndex,
         tr("Notes from brew day on %1").arg(brewNote.brewDate_short())
      );
      return;
   }

   void updateBrewNoteTabText(BrewNote const & brewNote) const {
      // See if this BrewNote is open in a tab
      BrewNoteWidget * widget = this->findBrewNoteWidget(brewNote);
      if (widget) {
         this->updateBrewNoteTabText(brewNote, *widget);
      }
      return;
   }

   void closeBrewNoteTab(BrewNote const & brewNote) const {
      BrewNoteWidget* widget = this->findBrewNoteWidget(brewNote);
      if (!widget) {
         qDebug() << Q_FUNC_INFO << "Could not find tab for BrewNote" << brewNote;
         return;
      }

      qDebug() << Q_FUNC_INFO << "Closing tab for BrewNote" << brewNote;
      auto const tabIndex = this->m_self.tabWidget_recipeView->indexOf(widget);
      this->m_self.tabWidget_recipeView->removeTab(tabIndex);

      return;
   }

   //! Clean out any brew notes
   void closeAllBrewNoteTabs() const {
      qDebug() << Q_FUNC_INFO << "Closing all BrewNote tabs";
      this->m_self.tabWidget_recipeView->setCurrentIndex(0);

      // Start closing from the right (highest index) down. Anything else dumps
      // core in the most unpleasant of fashions
      int const maxTabNum = this->m_self.tabWidget_recipeView->count() - 1;
      for (int ii = maxTabNum; ii > 0; --ii) {
         if (this->m_self.tabWidget_recipeView->widget(ii)->objectName() ==
            BrewNoteWidget::staticMetaObject.className()) {
            this->m_self.tabWidget_recipeView->removeTab(ii);
         }
      }
      return;
   }

   void setBrewNote(BrewNote & brewNote) const {
      BrewNoteWidget* widget = this->findBrewNoteWidget(brewNote);
      if (!widget) {
         widget = new BrewNoteWidget(this->m_self.tabWidget_recipeView);
         widget->setBrewNote(&brewNote);
         this->m_self.tabWidget_recipeView->addTab(widget, brewNote.brewDate_short());
      }

      this->updateBrewNoteTabText(brewNote, widget);
      this->m_self.tabWidget_recipeView->setCurrentWidget(widget);
      return;
   }

   void setBrewNoteByIndex(QModelIndex const & index) const {

      auto bNote = this->m_self.treeView_recipe->getItem<BrewNote>(index);
      if (!bNote) {
         return;
      }

      Recipe* parent  = ObjectStoreWrapper::getByIdRaw<Recipe>(bNote->recipeId());
      QModelIndex pNdx = this->m_self.treeView_recipe->parentIndex(index);

      // This gets complex. Versioning means we can't just clear the open brewnote tabs out.
      if (parent != this->m_recipeObs) {
         if (!this->m_recipeObs->isMyAncestor(*parent)) {
            this->m_self.setRecipe(parent);
         } else if (this->m_self.treeView_recipe->ancestorsAreShowing(pNdx)) {
            this->closeAllBrewNoteTabs();

            this->m_self.setRecipe(parent);
         }
      }

      this->setBrewNote(*bNote);
      return;
   }
   /**
    * \brief Gets the first selected Recipe
    */
   std::shared_ptr<Recipe> getSelectedRecipe() {
      for (QModelIndex selected : this->m_self.treeView_recipe->selectionModel()->selectedRows()) {
         auto recipe   = this->m_self.treeView_recipe->getItem<Recipe  >(selected);
         if (recipe) {
            return recipe;
         }
      }
      return nullptr;
   }

   /**
    * \brief Gets the first selected BrewNote and its Recipe
    */
   std::tuple<std::shared_ptr<BrewNote>, std::shared_ptr<Recipe>> getSelectedBrewNoteAndRecipe() {
      for (QModelIndex selected : this->m_self.treeView_recipe->selectionModel()->selectedRows()) {
         QModelIndex parent = this->m_self.treeView_recipe->parentIndex(selected);
         auto brewNote = this->m_self.treeView_recipe->getItem<BrewNote>(selected);
         auto recipe   = this->m_self.treeView_recipe->getItem<Recipe  >(parent  );
         if (brewNote && recipe) {
            return {brewNote, recipe};
         }
      }
      return {nullptr, nullptr};
   }

   //! \brief copies an existing brewnote to a new brewday
   BrewNote * copySelectedBrewNote() {
      auto [selectedBrewNote, recipe] = this->getSelectedBrewNoteAndRecipe();
      if (!selectedBrewNote || !recipe) {
         return nullptr;
      }

      auto newBrewNote = std::make_shared<BrewNote>(*selectedBrewNote);
      newBrewNote->setBrewDate();
      ObjectStoreWrapper::insert(newBrewNote);

      if (recipe.get() != this->m_recipeObs) {
         this->m_self.setRecipe(recipe.get());
      }

      this->setBrewNote(*newBrewNote);

      this->m_self.setTreeSelection(this->m_self.treeView_recipe->findElement(newBrewNote.get()));

      return newBrewNote.get();
   }

   /**
    * \brief creates a new brewnote
    */
   BrewNote * createBrewNote() {
      auto recipe = this->getSelectedRecipe();
      if (!recipe) {
         return nullptr;
      }

      // Make sure everything is properly set and selected
      if (recipe.get() != this->m_recipeObs) {
         this->m_self.setRecipe(recipe.get());
      }

      auto brewNote = std::make_shared<BrewNote>(*recipe);
      brewNote->populateNote(recipe.get());
      brewNote->setBrewDate();
      ObjectStoreWrapper::insert(brewNote);

      this->setBrewNote(*brewNote);

      QModelIndex brewNoteIndex = this->m_self.treeView_recipe->findElement(brewNote.get());
      if (brewNoteIndex.isValid()) {
         this->m_self.setTreeSelection(brewNoteIndex);
      } else {
         qWarning() << Q_FUNC_INFO << "Unable to find newly created BrewNote in Recipe tree";
      }

      return brewNote.get();
   }

   /**
    * \brief Reduces the inventory by the selected recipes
    */
   void reduceInventory(BrewNote & brewNote) {
      std::shared_ptr<Recipe> rec = brewNote.recipe();
      if (!rec) {
         // This shouldn't happen
         qCritical() << Q_FUNC_INFO << "No recipe for" << brewNote;
         return;
      }

      // Make sure everything is properly set and selected
      if (rec.get() != this->m_recipeObs) {
         this->m_self.setRecipe(rec.get());
      }

      //
      // Reduce fermentables, miscs, hops, yeasts
      //
      // Note that the amount can be mass, volume or (for Yeast and Misc) count.  We don't worry about which here as
      // we assume that a given type of ingredient is always measured in the same way.
      //
      for (auto addition : rec->fermentableAdditions()) {
         StockPurchaseFermentable::reduceTotalInventory(*addition->ingredientRaw(),
                                                        addition->amount(),
                                                        brewNote);
      }
      for (auto addition : rec->hopAdditions()) {
         StockPurchaseHop::reduceTotalInventory(*addition->ingredientRaw(),
                                                addition->amount(),
                                                brewNote);
      }
      for (auto addition : rec->miscAdditions()) {
         StockPurchaseMisc::reduceTotalInventory(*addition->ingredientRaw(),
                                                 addition->amount(),
                                                 brewNote);
      }
      for (auto addition : rec->yeastAdditions()) {
         StockPurchaseYeast::reduceTotalInventory(*addition->ingredientRaw(),
                                                  addition->amount(),
                                                  brewNote);
      }
      for (auto addition : rec->saltAdjustments()) {
         StockPurchaseSalt::reduceTotalInventory(*addition->ingredientRaw(),
                                                 addition->amount(),
                                                 brewNote);
      }

      return;
   }


   //================================================ MEMBER VARIABLES =================================================
   MainWindow & m_self;

   Recipe * m_recipeObs = nullptr;

   VeriTable<RecipeAdditionFermentable> m_fermentableAdditionsVeriTable;
   VeriTable<RecipeAdditionHop        > m_hopAdditionsVeriTable        ;
   VeriTable<RecipeAdditionMisc       > m_miscAdditionsVeriTable       ;
   VeriTable<RecipeAdditionYeast      > m_yeastAdditionsVeriTable      ;
   VeriTable<RecipeAdjustmentSalt     > m_saltAdditionsVeriTable       ;

   // All initialised in setupDialogs
   std::unique_ptr<        BoilCatalog>         m_boilCatalog;
   std::unique_ptr<   EquipmentCatalog>    m_equipmentCatalog;
   std::unique_ptr< FermentableCatalog>  m_fermentableCatalog;
   std::unique_ptr<FermentationCatalog> m_fermentationCatalog;
   std::unique_ptr<         HopCatalog>          m_hopCatalog;
   std::unique_ptr<        MashCatalog>         m_mashCatalog;
   std::unique_ptr<        MiscCatalog>         m_miscCatalog;
   std::unique_ptr<        SaltCatalog>         m_saltCatalog;
   std::unique_ptr<       StyleCatalog>        m_styleCatalog;
   std::unique_ptr<       WaterCatalog>        m_waterCatalog;
   std::unique_ptr<       YeastCatalog>        m_yeastCatalog;

   std::unique_ptr<            BoilEditor>             m_boilEditor;
   std::unique_ptr<        BoilStepEditor>         m_boilStepEditor;
   std::unique_ptr<       EquipmentEditor>        m_equipmentEditor;
   std::unique_ptr<     FermentableEditor>      m_fermentableEditor;
   std::unique_ptr<    FermentationEditor>     m_fermentationEditor;
   std::unique_ptr<FermentationStepEditor> m_fermentationStepEditor;
   std::unique_ptr<             HopEditor>              m_hopEditor;
   std::unique_ptr<            MashEditor>             m_mashEditor;
   std::unique_ptr<        MashStepEditor>         m_mashStepEditor;
   std::unique_ptr<            MiscEditor>             m_miscEditor;
   std::unique_ptr<            SaltEditor>             m_saltEditor;
   std::unique_ptr<           StyleEditor>            m_styleEditor;
   std::unique_ptr<           WaterEditor>            m_waterEditor;
   std::unique_ptr<           YeastEditor>            m_yeastEditor;

   //
   // TBD: Have another look at the naming of these windows -- dialog vs tool etc
   //
   std::unique_ptr<AboutDialog               > m_aboutDialog           ;
   std::unique_ptr<AlcoholTool               > m_alcoholTool           ;
   std::unique_ptr<AncestorDialog            > m_ancestorDialog        ;
   std::unique_ptr<BtDatePopup               > m_btDatePopup           ;
   std::unique_ptr<ConverterTool             > m_converterTool         ;
   std::unique_ptr<HelpDialog                > m_helpDialog            ;
   std::unique_ptr<HydrometerTool            > m_hydrometerTool        ;
   std::unique_ptr<MashDesigner              > m_mashDesigner          ;
   std::unique_ptr<MashWizard                > m_mashWizard            ;
   std::unique_ptr<OgAdjuster                > m_ogAdjuster            ;
   std::unique_ptr<OptionDialog              > m_optionDialog          ;
   std::unique_ptr<PitchDialog               > m_pitchDialog           ;
   std::unique_ptr<PrimingDialog             > m_primingDialog         ;
   std::unique_ptr<PrintAndPreviewDialog     > m_printAndPreviewDialog ;
   std::unique_ptr<RecipeFormatter           > m_recipeFormatter       ;
   std::unique_ptr<RefractoDialog            > m_refractoDialog        ;
   std::unique_ptr<ScaleRecipeTool           > m_recipeScaler          ;
   std::unique_ptr<StrikeWaterDialog         > m_strikeWaterDialog     ;
   std::unique_ptr<StockWindow               > m_stockWindow           ;
   std::unique_ptr<TimerMainDialog           > m_timerMainDialog       ;
   std::unique_ptr<WaterProfileAdjustmentTool> m_waterProfileAdjustmentTool;

   QString highSS, lowSS, goodSS, boldSS; // Palette replacements
};


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), pimpl{std::make_unique<impl>(*this)} {
   qDebug() << Q_FUNC_INFO;

   // Need to call this parent class method to get all the widgets added (I think).
   this->setupUi(this);

   // Initialise smart labels etc early, but after call to this->setupUi() because otherwise member variables such as
   // label_name will not yet be set.
   //
   // TBD: Not sure what original difference was supposed to be between label_targetBatchSize & label_batchSize or
   //      between label_targetBoilSize and label_boilSize.
   //
   SMART_FIELD_INIT(MainWindow, label_name           , lineEdit_name       , Recipe, PropertyNames::NamedEntity::name        );
   SMART_FIELD_INIT(MainWindow, label_targetBatchSize, lineEdit_batchSize  , Recipe, PropertyNames::Recipe::batchSize_l   , 2);
   SMART_FIELD_INIT(MainWindow, label_targetBoilSize , value_targetBoilSize, Boil  , PropertyNames::Boil::preBoilSize_l   , 2);
   SMART_FIELD_INIT(MainWindow, label_efficiency     , lineEdit_efficiency , Recipe, PropertyNames::Recipe::efficiency_pct, 1);
   SMART_FIELD_INIT(MainWindow, label_boilTime       , value_boilTime      , Boil  , PropertyNames::Boil::boilTime_mins   , 0);
   SMART_FIELD_INIT(MainWindow, label_boilSg         , value_boilSg        , Recipe, PropertyNames::Recipe::boilGrav      , 3);

   SMART_FIELD_INIT_NO_SF(MainWindow, label_og       , Recipe, PropertyNames::Recipe::og         );
   SMART_FIELD_INIT_NO_SF(MainWindow, label_fg       , Recipe, PropertyNames::Recipe::fg         );
   SMART_FIELD_INIT_NO_SF(MainWindow, label_color    , Recipe, PropertyNames::Recipe::color_srm  );
   SMART_FIELD_INIT_NO_SF(MainWindow, label_batchSize, Recipe, PropertyNames::Recipe::batchSize_l);
   SMART_FIELD_INIT_NO_SF(MainWindow, label_boilSize , Boil  , PropertyNames::Boil::preBoilSize_l);

   // Stop things looking ridiculously tiny on high DPI displays
   this->pimpl->setSizesInPixelsBasedOnDpi();

   // Horizontal tabs, please -- even on Mac OS, as the tabs contain square icons
   tabWidget_Trees->tabBar()->setStyle(new BtHorizontalTabs(true));

   //-------------------------------------------------------------------------------------------------------------------
   // NOTE for testing internationalization
   //
   // Older versions of the software had some commented-out code here to allow a "forced" locale via a code edit:
   //
   //    QLocale german(QLocale::German,QLocale::Germany);
   //    QLocale::setDefault(german);
   //
   // However, this is no longer needed.  Per code and comments in Localization.cpp, a forced locale can be set at
   // runtime via the config file, eg:
   //
   //    forcedLocale=fr_FR
   //
   //-------------------------------------------------------------------------------------------------------------------

   // If the database doesn't load, we bail
   if (!Database::instance().loadSuccessful() ) {
      qCritical() << Q_FUNC_INFO << "Could not load database";

      // Ask the application nicely to quit
      QCoreApplication::quit();
      // If it didn't, we ask more firmly
      QCoreApplication::exit(1);
      // If the framework won't play ball, we invoke a higher power.
      exit(1);
   }

   // Now let's ensure all the data is read in from the DB
   QString errorMessage{};
   if (!InitialiseAllObjectStores(errorMessage)) {
      bool bail = true;
      if (Application::isInteractive()) {
         // Can't use QErrorMessage here as it's not flexible enough for what we need
         QMessageBox dataLoadErrorMessageBox;
         dataLoadErrorMessageBox.setWindowTitle(tr("Error Loading Data"));
         dataLoadErrorMessageBox.setText(errorMessage);
         dataLoadErrorMessageBox.setInformativeText(
            tr("The program may not work if you ignore this error.\n\n"
               "See logs for more details.\n\n"
               "If you need help, please open an issue "
               "at %1").arg(CONFIG_GITHUB_URL)
         );
         dataLoadErrorMessageBox.setStandardButtons(QMessageBox::Ignore | QMessageBox::Close);
         dataLoadErrorMessageBox.setDefaultButton(QMessageBox::Close);
         int ret = dataLoadErrorMessageBox.exec();
         if (ret == QMessageBox::Close) {
            qDebug() << Q_FUNC_INFO << "User clicked \"Close\".  Exiting.";
         } else {
            qWarning() <<
               Q_FUNC_INFO << "User clicked \"Ignore\" after errors loading data.  PROGRAM MAY NOT FUNCTION CORRECTLY!";
            bail = false;
         }
      }
      if (bail) {
         // Either the user clicked Close, or we're not interactive.  Either way, we quit in the same way as above.
         qDebug() << Q_FUNC_INFO << "Exiting...";
         QCoreApplication::quit();
         qDebug() << Q_FUNC_INFO << "Still Exiting...";
         QCoreApplication::exit(1);
         qDebug() << Q_FUNC_INFO << "Really Exiting now...";
         exit(1);
      }
   }

   // Set the window title and a couple of other strings.  (This replaces the corresponding texts in the mainWindow.ui
   // file.)
   this->setWindowTitle(QString{"%1 - %2"}.arg(CONFIG_APPLICATION_NAME_UC, CONFIG_VERSION_STRING) );
   this->actionAbout->setText   (tr("About %1").arg(CONFIG_APPLICATION_NAME_UC));
   this->actionAbout->setToolTip(tr("About %1").arg(CONFIG_APPLICATION_NAME_UC));

   // Null out the recipe
   this->pimpl->m_recipeObs = nullptr;

   return;
}

void MainWindow::initialiseAndMakeVisible() {
   qDebug() << Q_FUNC_INFO;

   this->setupCSS();
   // initialize all of the dialog windows
   this->pimpl->setupDialogs();
   // initialize the ranged sliders
   this->setupRanges();
   // the dialogs have to be setup before this is called
   this->pimpl->setupComboBoxes();
   // do all the work to configure the tables models and their proxies
   this->pimpl->setupTables();
   // Create the keyboard shortcuts
   this->setupShortCuts();
   // Once more with the context menus too
   this->pimpl->setupTreeViews();
   // do all the work for checkboxes (just one right now)
   this->setUpStateChanges();

   // Connect menu item slots to triggered() signals
   this->setupTriggers();
   // Connect pushbutton slots to clicked() signals
   this->setupClicks();
   // connect combobox slots to activate() signals
   this->setupActivate();
   // connect signal slots for the line edits
   this->setupTextEdit();
   // connect the remaining labels
   this->setupLabels();
   // set up the drag/drop parts
   this->setupDrops();

   // No connections from the database yet? Oh FSM, that probably means I'm
   // doing it wrong again.
   // .:TODO:. Change this so we use the newer deleted signal!
   connect(&ObjectStoreTyped<BrewNote>::getInstance(), &ObjectStoreTyped<BrewNote>::signalObjectDeleted, this, &MainWindow::brewNoteDeleted);

   //
   // Read in any new ingredients, styles, example recipes etc
   //
   // (In the past this was done in Application::run() because we were reading raw DB files.  Now that default
   // ingredients etc are stored in BeerXML and BeerJSON, we need to read them in a bit later, after the MainWindow
   // object exists (ie after its constructor finishes running!) and after the call InitialiseAllObjectStores.)
   //
   Database::instance().checkForNewDefaultData();

   // This sets up things that might have been 'remembered' (ie stored in the config file) from a previous run of the
   // program - eg window size, which is stored in MainWindow::closeEvent().
   // Breaks the naming convention, doesn't it?
   this->restoreSavedState();

   // Set up the pretty tool tip. It doesn't really belong anywhere, so here it is
   // .:TODO:. When we allow users to change databases without restarting, we'll need to make sure to call this whenever
   // the database is changed (as setToolTip() just takes static text as its parameter).
   label_Brewtarget->setToolTip(getLabelToolTip());

   this->setVisible(true);

   emit initialisedAndVisible();

   qDebug() << Q_FUNC_INFO << "MainWindow initialisation complete";
   return;
}


// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header file)
MainWindow::~MainWindow() = default;

MainWindow & MainWindow::instance() {
   //
   // Since C++11, there is a standard thread-safe way to ensure a function is called exactly once
   //
   static std::once_flag initFlag_MainWindow;

   std::call_once(initFlag_MainWindow, createMainWindowInstance);

   Q_ASSERT(mainWindowInstance);
   return *mainWindowInstance;
}

void MainWindow::DeleteMainWindow() {
   delete mainWindowInstance;
   mainWindowInstance = nullptr;
}

// Setup the keyboard shortcuts
void MainWindow::setupShortCuts() {
   this->actionNewRecipe->setShortcut(QKeySequence::New);
   this->actionCopySelected->setShortcut(QKeySequence::Copy);
   this->actionDeleteSelected->setShortcut(QKeySequence::Delete);
   this->actionUndo->setShortcut(QKeySequence::Undo);
   this->actionRedo->setShortcut(QKeySequence::Redo);
   return;
}

void MainWindow::setUpStateChanges() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
   connect(checkBox_locked, &QCheckBox::checkStateChanged, this, &MainWindow::lockRecipe);
#else
   connect(checkBox_locked, &QCheckBox::stateChanged     , this, &MainWindow::lockRecipe);
#endif
   return;
}

// Any manipulation of CSS for the MainWindow should be in here
void MainWindow::setupCSS() {
   // Different palettes for some text. This is all done via style sheets now.
   QColor wPalette = tabWidget_recipeView->palette().color(QPalette::Active,QPalette::Base);

   //
   // NB: Using pixels for font sizes in Qt is bad because, given the significant variations in pixels-per-inch (aka
   // dots-per-inch / DPI) between "normal" and "high DPI" displays, a size specified in pixels will most likely be
   // dramatically wrong on some displays.  The simple solution is instead to use points (which are device independent)
   // to specify font size.
   //
   this->pimpl->goodSS = QString("QLineEdit:read-only { color: #008800; background: %1 }").arg(wPalette.name());
   this->pimpl->lowSS  = QString("QLineEdit:read-only { color: #0000D0; background: %1 }").arg(wPalette.name());
   this->pimpl->highSS = QString("QLineEdit:read-only { color: #D00000; background: %1 }").arg(wPalette.name());
   this->pimpl->boldSS = QString("QLineEdit:read-only { font: bold 10pt; color: #000000; background: %1 }").arg(wPalette.name());

   // Disabled fields should change color, but not become unreadable. Mucking
   // with the css seems the most reasonable way to do that.
   QString tabDisabled = QString("QWidget:disabled { color: #000000; background: #F0F0F0 }");
   tab_recipe->setStyleSheet(tabDisabled);
   tabWidget_ingredients->setStyleSheet(tabDisabled);

   return;
}

// Configures the range widgets for the bubbles
void MainWindow::setupRanges() {
   //
   // The right-hand side of the Recipe pane shows the following:
   //    OG
   //    FG
   //    ABV
   //    Bitterness (IBU)
   //    Color
   //    IBU/GU
   //    Batch Size
   //    Boil Size
   //

   styleRangeWidget_og->setRange(1.000, 1.120);
   styleRangeWidget_og->setPrecision(3);
   styleRangeWidget_og->setTickMarks(0.010, 2);

   styleRangeWidget_fg->setRange(1.000, 1.030);
   styleRangeWidget_fg->setPrecision(3);
   styleRangeWidget_fg->setTickMarks(0.010, 2);

   styleRangeWidget_abv->setRange(0.0, 15.0);
   styleRangeWidget_abv->setPrecision(1);
   styleRangeWidget_abv->setTickMarks(1, 2);

   styleRangeWidget_ibu->setRange(0.0, 120.0);
   styleRangeWidget_ibu->setPrecision(1);
   styleRangeWidget_ibu->setTickMarks(10, 2);

   // definitely cheating, but I don't feel like making a whole subclass just to support this
   // or the next.
   rangeWidget_batchSize->setRange(0, this->pimpl->m_recipeObs == nullptr ? 19.0 : this->pimpl->m_recipeObs->batchSize_l());
   rangeWidget_batchSize->setPrecision(1);
   rangeWidget_batchSize->setTickMarks(2,5);

   rangeWidget_batchSize->setBackgroundBrush(QColor(255,255,255));
   rangeWidget_batchSize->setPreferredRangeBrush(QColor(55,138,251));
   rangeWidget_batchSize->setMarkerBrush(QBrush(Qt::NoBrush));

   rangeWidget_boilsize->setRange(0, this->pimpl->m_recipeObs == nullptr? 24.0 : this->pimpl->m_recipeObs->boilVolume_l());
   rangeWidget_boilsize->setPrecision(1);
   rangeWidget_boilsize->setTickMarks(2,5);

   rangeWidget_boilsize->setBackgroundBrush(QColor(255,255,255));
   rangeWidget_boilsize->setPreferredRangeBrush(QColor(55,138,251));
   rangeWidget_boilsize->setMarkerBrush(QBrush(Qt::NoBrush));

   int const srmMax = 50;
   styleRangeWidget_srm->setRange(0.0, static_cast<double>(srmMax));
   styleRangeWidget_srm->setPrecision(1);
   styleRangeWidget_srm->setTickMarks(10, 2);
   // Need to change appearance of color slider
   {
      // The styleRangeWidget_srm should display beer color in the background
      QLinearGradient grad( 0,0, 1,0 );
      grad.setCoordinateMode(QGradient::ObjectBoundingMode);
      for (int ii = 0; ii <= srmMax; ++ii) {
         double const srm = ii;
         grad.setColorAt( srm/static_cast<double>(srmMax), ColorMethods::srmToDisplayColor(srm));
      }
      styleRangeWidget_srm->setBackgroundBrush(grad);

      // The styleRangeWidget_srm should display a "window" to show acceptable colors for the style
      styleRangeWidget_srm->setPreferredRangeBrush(QColor(0,0,0,0));
      styleRangeWidget_srm->setPreferredRangePen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

      // Half-height "tick" for color marker
      grad = QLinearGradient( 0,0, 0,1 );
      grad.setCoordinateMode(QGradient::ObjectBoundingMode);
      grad.setColorAt( 0, QColor(255,255,255,255) );
      grad.setColorAt( 0.49, QColor(255,255,255,255) );
      grad.setColorAt( 0.50, QColor(255,255,255,0) );
      grad.setColorAt( 1, QColor(255,255,255,0) );
      styleRangeWidget_srm->setMarkerBrush(grad);
   }
   return;
}

// Anything resulting in a restoreState() should go in here
void MainWindow::restoreSavedState() {

   // If we saved a size the last time we ran, use it
   if (PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry   , *this, BtString::EMPTY_STR)) {
       PersistentSettings::restoreUiState (PersistentSettings::Names::windowState, *this, BtString::EMPTY_STR);
   } else {
      // otherwise, guess a reasonable size at 1/4 of the screen.
      QScreen * screen = this->screen();
      QRect const desktop = screen->availableGeometry();
      int const width = desktop.width();
      int const height = desktop.height();
      this->resize(width/2,height/2);

      // Or we could do the same in one line:
      // this->resize(this->screen().availableGeometry().size() * 0.5);
   }

   // If we saved the selected recipe name the last time we ran, select it and show it.
   int key = -1;
   if (PersistentSettings::contains_ck(PersistentSettings::Names::recipeKey)) {
      key = PersistentSettings::value_ck(PersistentSettings::Names::recipeKey).toInt();
   } else {
      auto firstRecipeWeFind = ObjectStoreTyped<Recipe>::getInstance().findFirstMatching(
         // This trivial lambda gives us the first recipe in the list, if there is one
         []([[maybe_unused]] std::shared_ptr<Recipe> obj) {return true;}
      );
      if (firstRecipeWeFind) {
         key = firstRecipeWeFind->key();
      }
   }
   if (key > -1) {
      // We can't assume that the "remembered" recipe exists.  The user might have restored to an older DB file since
      // the last time the program was run.
      Recipe * recipe = ObjectStoreWrapper::getByIdRaw<Recipe>(key);
      qDebug() << Q_FUNC_INFO << "Recipe #" << key << (recipe ? "found" : "not found");
      if (recipe) {
         // We trust setRecipe to do any necessary checks and UI updates
         this->setRecipe(recipe);
      }
   }

   // UI restore state
   PersistentSettings::restoreUiState(PersistentSettings::Names::splitter_vertical_State         , *this->splitter_vertical             );
   PersistentSettings::restoreUiState(PersistentSettings::Names::splitter_horizontal_State       , *this->splitter_horizontal           );
   PersistentSettings::restoreUiState(PersistentSettings::Names::treeView_recipe_headerState     , *this->treeView_recipe     ->header());
   PersistentSettings::restoreUiState(PersistentSettings::Names::treeView_style_headerState      , *this->treeView_style      ->header());
   PersistentSettings::restoreUiState(PersistentSettings::Names::treeView_equipment_headerState  , *this->treeView_equipment  ->header());
   PersistentSettings::restoreUiState(PersistentSettings::Names::treeView_fermentable_headerState, *this->treeView_fermentable->header());
   PersistentSettings::restoreUiState(PersistentSettings::Names::treeView_hop_headerState        , *this->treeView_hop        ->header());
   PersistentSettings::restoreUiState(PersistentSettings::Names::treeView_misc_headerState       , *this->treeView_misc       ->header());
   PersistentSettings::restoreUiState(PersistentSettings::Names::treeView_yeast_headerState      , *this->treeView_yeast      ->header());

   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_boilCatalog        , *this->pimpl->m_boilCatalog        );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_equipmentCatalog   , *this->pimpl->m_equipmentCatalog   );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_fermentableCatalog , *this->pimpl->m_fermentableCatalog );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_fermentationCatalog, *this->pimpl->m_fermentationCatalog);
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_hopCatalog         , *this->pimpl->m_hopCatalog         );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_mashCatalog        , *this->pimpl->m_mashCatalog        );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_miscCatalog        , *this->pimpl->m_miscCatalog        );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_saltCatalog        , *this->pimpl->m_saltCatalog        );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_styleCatalog       , *this->pimpl->m_styleCatalog       );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_waterCatalog       , *this->pimpl->m_waterCatalog       );
   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_yeastCatalog       , *this->pimpl->m_yeastCatalog       );

   this->pimpl->m_boilCatalog        ->restoreUiState(PersistentSettings::Names::uiState_boilCatalog        );
   this->pimpl->m_equipmentCatalog   ->restoreUiState(PersistentSettings::Names::uiState_equipmentCatalog   );
   this->pimpl->m_fermentableCatalog ->restoreUiState(PersistentSettings::Names::uiState_fermentableCatalog );
   this->pimpl->m_fermentationCatalog->restoreUiState(PersistentSettings::Names::uiState_fermentationCatalog);
   this->pimpl->m_hopCatalog         ->restoreUiState(PersistentSettings::Names::uiState_hopCatalog         );
   this->pimpl->m_mashCatalog        ->restoreUiState(PersistentSettings::Names::uiState_mashCatalog        );
   this->pimpl->m_miscCatalog        ->restoreUiState(PersistentSettings::Names::uiState_miscCatalog        );
   this->pimpl->m_saltCatalog        ->restoreUiState(PersistentSettings::Names::uiState_saltCatalog        );
   this->pimpl->m_styleCatalog       ->restoreUiState(PersistentSettings::Names::uiState_styleCatalog       );
   this->pimpl->m_waterCatalog       ->restoreUiState(PersistentSettings::Names::uiState_waterCatalog       );
   this->pimpl->m_yeastCatalog       ->restoreUiState(PersistentSettings::Names::uiState_yeastCatalog       );

   this->        mashStepsWidget->restoreUiState(PersistentSettings::Names::        mashStepTableWidget_headerState, PersistentSettings::Sections::MainWindow);
   this->        boilStepsWidget->restoreUiState(PersistentSettings::Names::        boilStepTableWidget_headerState, PersistentSettings::Sections::MainWindow);
   this->fermentationStepsWidget->restoreUiState(PersistentSettings::Names::fermentationStepTableWidget_headerState, PersistentSettings::Sections::MainWindow);

   PersistentSettings::restoreGeometry(PersistentSettings::Names::geometry_stockWindow, *this->pimpl->m_stockWindow);
   this->pimpl->m_stockWindow->restoreUiState();
   return;
}

// menu items with a SIGNAL of triggered() should go in here.
void MainWindow::setupTriggers() {
   // Connect actions defined in *.ui files to methods in code
   connect(actionExit                      , &QAction::triggered, this                                      , &QWidget::close                    ); // > File > Exit
   connect(actionAbout                     , &QAction::triggered, this->pimpl->m_aboutDialog.get()          , &QWidget::show                     ); // > About > About Brewtarget
   connect(actionHelp                      , &QAction::triggered, this->pimpl->m_helpDialog.get()           , &QWidget::show                     ); // > About > Help

   connect(actionNewRecipe                 , &QAction::triggered, this                                      , &MainWindow::newRecipe             ); // > File > New Recipe
   connect(actionImportFromXml             , &QAction::triggered, this                                      , &MainWindow::importFiles           ); // > File > Import Recipes
   connect(actionExportToXml               , &QAction::triggered, this                                      , &MainWindow::exportRecipe          ); // > File > Export Recipes
   connect(actionUndo                      , &QAction::triggered, this                                      , &MainWindow::editUndo              ); // > Edit > Undo
   connect(actionRedo                      , &QAction::triggered, this                                      , &MainWindow::editRedo              ); // > Edit > Redo
   this->setUndoRedoEnable();
   connect(actionEquipments                , &QAction::triggered, this->pimpl->m_equipmentCatalog.get()     , &QWidget::show                     ); // > View > Equipments
   connect(actionMashes                    , &QAction::triggered, this->pimpl->m_mashCatalog.get()          , &QWidget::show                     ); // > View > Mash Profiles
   connect(actionBoils                     , &QAction::triggered, this->pimpl->m_boilCatalog.get()          , &QWidget::show                     ); // > View > Boil Profiles
   connect(actionFermentations             , &QAction::triggered, this->pimpl->m_fermentationCatalog.get()  , &QWidget::show                     ); // > View > Fermentation Profiles

   connect(actionStyles                    , &QAction::triggered, this->pimpl->m_styleCatalog.get()         , &QWidget::show                     ); // > View > Styles
   connect(actionFermentables              , &QAction::triggered, this->pimpl->m_fermentableCatalog.get()          , &QWidget::show                     ); // > View > Fermentables
   connect(actionHops                      , &QAction::triggered, this->pimpl->m_hopCatalog.get()           , &QWidget::show                     ); // > View > Hops
   connect(actionMiscs                     , &QAction::triggered, this->pimpl->m_miscCatalog.get()          , &QWidget::show                     ); // > View > Miscs
   connect(actionYeasts                    , &QAction::triggered, this->pimpl->m_yeastCatalog.get()         , &QWidget::show                     ); // > View > Yeasts
   connect(actionSalts                     , &QAction::triggered, this->pimpl->m_saltCatalog.get()          , &QWidget::show                     ); // > View > Salts
   connect(actionWaters                    , &QAction::triggered, this->pimpl->m_waterCatalog.get()         , &QWidget::show                     ); // > View > Waters
   connect(actionInventory                 , &QAction::triggered, this->pimpl->m_stockWindow.get()      , &QWidget::show                     ); // > View > Inventory
   connect(actionOptions                   , &QAction::triggered, this->pimpl->m_optionDialog.get()         , &OptionDialog::show                ); // > Tools > Options
//   connect( actionManual, &QAction::triggered, this, &MainWindow::openManual);                                               // > About > Manual
   connect(actionScale_Recipe              , &QAction::triggered, this->pimpl->m_recipeScaler.get()         , &QWidget::show                     ); // > Tools > Scale Recipe
   connect(action_recipeToTextClipboard    , &QAction::triggered, this->pimpl->m_recipeFormatter.get()      , &RecipeFormatter::toTextClipboard  ); // > Tools > Recipe to Clipboard as Text
   connect(actionConvert_Units             , &QAction::triggered, this->pimpl->m_converterTool.get()        , &QWidget::show                     ); // > Tools > Convert Units
   connect(actionHydrometer_Temp_Adjustment, &QAction::triggered, this->pimpl->m_hydrometerTool.get()       , &QWidget::show                     ); // > Tools > Hydrometer Temp Adjustment
   connect(actionAlcohol_Percentage_Tool   , &QAction::triggered, this->pimpl->m_alcoholTool.get()          , &QWidget::show                     ); // > Tools > Alcohol
   connect(actionOG_Correction_Help        , &QAction::triggered, this->pimpl->m_ogAdjuster.get()           , &QWidget::show                     ); // > Tools > OG Correction Help
   connect(actionCopySelected              , &QAction::triggered, this                                      , &MainWindow::copySelected          ); // > File > Copy Selected
   connect(actionPriming_Calculator        , &QAction::triggered, this->pimpl->m_primingDialog.get()        , &QWidget::show                     ); // > Tools > Priming Calculator
   connect(actionStrikeWater_Calculator    , &QAction::triggered, this->pimpl->m_strikeWaterDialog.get()    , &QWidget::show                     ); // > Tools > Strike Water Calculator
   connect(actionRefractometer_Tools       , &QAction::triggered, this->pimpl->m_refractoDialog.get()       , &QWidget::show                     ); // > Tools > Refractometer Tools
   connect(actionPitch_Rate_Calculator     , &QAction::triggered, this                                      , &MainWindow::showPitchDialog       ); // > Tools > Pitch Rate Calculator
   connect(actionTimers                    , &QAction::triggered, this->pimpl->m_timerMainDialog.get()      , &QWidget::show                     ); // > Tools > Timers
   connect(actionDeleteSelected            , &QAction::triggered, this                                      , &MainWindow::deleteSelected        );
   connect(actionWaterProfileAdjustmentTool, &QAction::triggered, this                                      , &MainWindow::showWaterProfileAdjustmentTool); // > Tools > Water Chemistry
   connect(actionAncestors                 , &QAction::triggered, this                                      , &MainWindow::setAncestor           ); // > Tools > Ancestors
   connect(action_brewit                   , &QAction::triggered, this                                      , &MainWindow::brewItHelper          );
   //One Dialog to rule them all, at least all printing and export.
   connect(actionPrint                     , &QAction::triggered, this->pimpl->m_printAndPreviewDialog.get(), &QWidget::show                     ); // > File > Print and Preview

   // postgresql cannot backup or restore yet. I would like to find some way
   // around this, but for now just disable
   if ( Database::instance().dbType() == Database::DbType::PGSQL ) {
      actionBackup_Database->setEnabled(false);                                                                         // > File > Database > Backup
      actionRestore_Database->setEnabled(false);                                                                        // > File > Database > Restore
   }
   else {
      connect( actionBackup_Database, &QAction::triggered, this, &MainWindow::backup );                                 // > File > Database > Backup
      connect( actionRestore_Database, &QAction::triggered, this, &MainWindow::restoreFromBackup );                     // > File > Database > Restore
   }
   return;
}

// pushbuttons with a SIGNAL of clicked() should go in here.
void MainWindow::setupClicks() {
   //
   // Note that, if the third parameter to connect is null, we'll get a warning log along the lines of
   // `QObject::connect(QPushButton, Unknown): invalid nullptr parameter`.  Assuming this is the only (or at least the
   // first) warning, a quick way to diagnose these is to set the environment variable QT_FATAL_WARNINGS and re-run the
   // program.  This will force a core dump when the warning occurs, and then, from the core file, you can see the call
   // stack.
   //
   connect(this->   equipmentButton       , &QAbstractButton::clicked, this, &MainWindow::editRecipeEquipment   );
   connect(this->       styleButton       , &QAbstractButton::clicked, this, &MainWindow::editRecipeStyle       );
   connect(this->        mashButton       , &QAbstractButton::clicked, this, &MainWindow::editRecipeMash        );
   connect(this->        boilButton       , &QAbstractButton::clicked, this, &MainWindow::editRecipeBoil        );
   connect(this->fermentationButton       , &QAbstractButton::clicked, this, &MainWindow::editRecipeFermentation);

   connect(this->pushButton_addFerm       , &QAbstractButton::clicked, this->pimpl-> m_fermentableCatalog.get(), &QWidget::show);
   connect(this->pushButton_addHop        , &QAbstractButton::clicked, this->pimpl->         m_hopCatalog.get(), &QWidget::show);
   connect(this->pushButton_addMisc       , &QAbstractButton::clicked, this->pimpl->        m_miscCatalog.get(), &QWidget::show);
   connect(this->pushButton_addYeast      , &QAbstractButton::clicked, this->pimpl->       m_yeastCatalog.get(), &QWidget::show);
   connect(this->pushButton_addSalt       , &QAbstractButton::clicked, this->pimpl->        m_saltCatalog.get(), &QWidget::show);
   // NB: We don't currently have pushButton_addWater

   connect(this->pushButton_removeFerm    , &QAbstractButton::clicked, this, &MainWindow::removeSelectedFermentableAddition);
   connect(this->pushButton_removeHop     , &QAbstractButton::clicked, this, &MainWindow::removeSelectedHopAddition        );
   connect(this->pushButton_removeMisc    , &QAbstractButton::clicked, this, &MainWindow::removeSelectedMiscAddition       );
   connect(this->pushButton_removeYeast   , &QAbstractButton::clicked, this, &MainWindow::removeSelectedYeastAddition      );
   connect(this->pushButton_removeSalt    , &QAbstractButton::clicked, this, &MainWindow::removeSelectedSaltAddition       );

   connect(this->pushButton_editFerm      , &QAbstractButton::clicked, this, &MainWindow::editFermentableOfSelectedFermentableAddition);
   connect(this->pushButton_editHop       , &QAbstractButton::clicked, this, &MainWindow::editHopOfSelectedHopAddition                );
   connect(this->pushButton_editMisc      , &QAbstractButton::clicked, this, &MainWindow::editMiscOfSelectedMiscAddition              );
   connect(this->pushButton_editYeast     , &QAbstractButton::clicked, this, &MainWindow::editYeastOfSelectedYeastAddition            );
   connect(this->pushButton_editSalt      , &QAbstractButton::clicked, this, &MainWindow::editSaltOfSelectedSaltAddition              );

   connect(this->pushButton_mashWizard, &QAbstractButton::clicked, this->pimpl->m_mashWizard.get()  , &MashWizard::show  );
   connect(this->pushButton_mashDesigner   , &QAbstractButton::clicked, this->pimpl->m_mashDesigner.get(), &MashDesigner::show);

   return;
}

// comboBoxes with a SIGNAL of activated() should go in here.
void MainWindow::setupActivate() {
   connect(this->   equipmentComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeEquipment   );
   connect(this->       styleComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeStyle       );
   connect(this->        mashComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeMash        );
   connect(this->        boilComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeBoil        );
   connect(this->fermentationComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeFermentation);
   return;
}

// lineEdits with either an editingFinished() or a textModified() should go in here
void MainWindow::setupTextEdit() {
   connect(this->lineEdit_name      , &QLineEdit::editingFinished,  this, &MainWindow::updateRecipeName);
   connect(this->lineEdit_batchSize , &SmartLineEdit::textModified, this, &MainWindow::updateRecipeBatchSize);
   connect(this->lineEdit_efficiency, &SmartLineEdit::textModified, this, &MainWindow::updateRecipeEfficiency);
   return;
}

// anything using a SmartLabel::changedSystemOfMeasurementOrScale signal should go in here
void MainWindow::setupLabels() {
   // These are the sliders. I need to consider these harder, but small steps
   connect(this->label_og,       &SmartLabel::changedSystemOfMeasurementOrScale, this, &MainWindow::redisplayLabel);
   connect(this->label_fg,       &SmartLabel::changedSystemOfMeasurementOrScale, this, &MainWindow::redisplayLabel);
   connect(this->label_color, &SmartLabel::changedSystemOfMeasurementOrScale, this, &MainWindow::redisplayLabel);
   return;
}

// anything with a BtTabWidget::set* signal should go in here
void MainWindow::setupDrops() {
   // drag and drop. maybe
   connect(this->tabWidget_recipeView,  &BtTabWidget::setRecipe,       this, &MainWindow::setRecipe);
   connect(this->tabWidget_recipeView,  &BtTabWidget::setEquipment,    this, &MainWindow::droppedRecipeEquipment);
   connect(this->tabWidget_recipeView,  &BtTabWidget::setStyle,        this, &MainWindow::droppedRecipeStyle);
   connect(this->tabWidget_ingredients, &BtTabWidget::setFermentables, this, &MainWindow::droppedRecipeFermentable);
   connect(this->tabWidget_ingredients, &BtTabWidget::setHops,         this, &MainWindow::droppedRecipeHop);
   connect(this->tabWidget_ingredients, &BtTabWidget::setMiscs,        this, &MainWindow::droppedRecipeMisc);
   connect(this->tabWidget_ingredients, &BtTabWidget::setYeasts,       this, &MainWindow::droppedRecipeYeast);
   connect(this->tabWidget_ingredients, &BtTabWidget::setSalts,        this, &MainWindow::droppedRecipeSalt);
   return;
}

void MainWindow::deleteSelected() {
   TreeView * activeTreeView = this->getActiveTreeView();

   // This happens after startup when nothing is selected
   if (!activeTreeView) {
      qDebug() << Q_FUNC_INFO << "Nothing selected, so nothing to delete";
      return;
   }
   activeTreeView->deleteSelected();

   return;
}

void MainWindow::setAncestor() {
   Recipe * recipe = this->pimpl->m_recipeObs;
   if (!recipe) {
      QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
      recipe = treeView_recipe->getItem<Recipe>(indexes[0]).get();
   }

   this->pimpl->m_ancestorDialog->setAncestor(recipe);
   this->pimpl->m_ancestorDialog->show();
   return;
}


// Can handle null recipes.
void MainWindow::setRecipe(Recipe * recipe) {
   // Don't like void pointers.
   if (!recipe) {
      return;
   }

   qDebug() << Q_FUNC_INFO << "Recipe #" << recipe->key() << ":" << recipe->name();


   // Make sure this MainWindow is paying attention...
   if (this->pimpl->m_recipeObs) {
      disconnect(this->pimpl->m_recipeObs, nullptr, this, nullptr);
      auto boil = this->pimpl->m_recipeObs->boil();
      if (boil) {
         disconnect(boil.get(), nullptr, this, nullptr);
      }
   }
   this->pimpl->m_recipeObs = recipe;

   this->displayRangesEtcForCurrentRecipeStyle();

   // Reset all previous recipe shit.
   this->pimpl->m_fermentableAdditionsVeriTable.m_tableModel->observeRecipe(recipe);
   this->pimpl->        m_hopAdditionsVeriTable.m_tableModel->observeRecipe(recipe);
   this->pimpl->       m_miscAdditionsVeriTable.m_tableModel->observeRecipe(recipe);
   this->pimpl->      m_yeastAdditionsVeriTable.m_tableModel->observeRecipe(recipe);
   this->pimpl->       m_saltAdditionsVeriTable.m_tableModel->observeRecipe(recipe);

   this->pimpl->closeAllBrewNoteTabs();

   // Tell some of our other widgets to observe the new recipe.
   this->pimpl->m_mashWizard->setRecipe(recipe);
   brewDayScrollWidget->setRecipe(recipe);
   this->pimpl->m_recipeFormatter->setRecipe(recipe);
   this->pimpl->m_ogAdjuster->setRecipe(recipe);
   recipeExtrasWidget->setRecipe(recipe);
   this->pimpl->m_mashDesigner->setRecipe(recipe);
   this->equipmentButton->setRecipe(recipe);
   this->equipmentComboBox->setItem(recipe->equipment());
   if (recipe->equipment()) {
      this->pimpl->m_equipmentEditor->setEditItem(recipe->equipment());
   }
   this->styleButton->setRecipe(recipe);
   this->styleComboBox->setItem(recipe->style());
   if (recipe->style()) {
      this->pimpl->m_styleEditor->setEditItem(recipe->style());
   }

   //
   // Note that on mashButton, boilButton etc, we want to call setRecipe rather than setMash, setBoil etc.  This means
   // the button will automatically get updated if/when the recipe's mash/boil/etc changes.  Same goes for
   // mashStepsWidget, boilStepsWidget, etc.
   //

   this->mashButton->setRecipe(recipe);
   this->mashComboBox->setItem(recipe->mash());
   this->mashStepsWidget->setRecipe(recipe);

   this->boilButton->setRecipe(recipe);
   this->boilComboBox->setItem(recipe->boil());
   this->boilStepsWidget->setRecipe(recipe);

   this->fermentationButton->setRecipe(recipe);
   this->fermentationComboBox->setItem(recipe->fermentation());
   this->fermentationStepsWidget->setRecipe(recipe);

   this->pimpl->m_recipeScaler->setRecipe(recipe);

   // Set the locked flag as required
   checkBox_locked->setCheckState(recipe->locked() ? Qt::Checked : Qt::Unchecked);
   lockRecipe(recipe->locked() ? Qt::Checked : Qt::Unchecked);

   checkBox_locked->setEnabled(true);

   checkBox_locked->setCheckState( recipe->locked() ? Qt::Checked : Qt::Unchecked);
   lockRecipe(recipe->locked() ? Qt::Checked : Qt::Unchecked);

   // changes in how the data is loaded means we may not have fired all the signals we should have
   // this makes sure the signals are fired. This is likely a 5kg hammer driving a finishing nail.
   recipe->recalcAll();

   // If you don't connect this late, every previous set of an attribute
   // causes this signal to be slotted, which then causes showChanges() to be
   // called.
   connect(this->pimpl->m_recipeObs, &NamedEntity::changed, this, &MainWindow::changed);
   auto boil = this->pimpl->m_recipeObs->boil();
   if (boil) {
      connect(boil.get(), &NamedEntity::changed, this, &MainWindow::changed);
   }

   // TBD: Had some problems with this that we should come back to once rework of TreeView etc is complete
//   QModelIndex rIdx = treeView_recipe->findElement(this->pimpl->m_recipeObs);
//   setTreeSelection(rIdx);

   this->showChanges();
   return;
}

// When a recipe is locked, many fields need to be disabled.
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void MainWindow::lockRecipe(Qt::CheckState state) {
#else
void MainWindow::lockRecipe(int state) {
#endif
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   // If I am locking a recipe (lock == true ), I want to disable fields
   // (enable == false). If I am unlocking (lock == false), I want to enable
   // fields (enable == true). This just makes that easy
   bool const lockIt  {state == Qt::Checked};
   bool const enabled {!lockIt};

   // Lock/unlock the recipe, then disable/enable the fields. I am leaving the
   // name field as editable. I may regret that, but if you are defining an
   // inheritance tree, you may want to remove strings from the ancestoral
   // names
   this->pimpl->m_recipeObs->setLocked(lockIt);

   // I could disable tab_recipe, but would not prevent you from unlocking the
   // recipe because that field would also be disabled
   qWidget_styleBox->setEnabled(enabled);
   qWidget_equipmentBox->setEnabled(enabled);
   lineEdit_batchSize->setEnabled(enabled);
   lineEdit_efficiency->setEnabled(enabled);

   // locked recipes cannot be deleted
   actionDeleteSelected->setEnabled(enabled);
   treeView_recipe->enableDelete(enabled);

   treeView_recipe->setDragDropMode( lockIt ? QAbstractItemView::NoDragDrop : QAbstractItemView::DragDrop);
   tabWidget_ingredients->setAcceptDrops( enabled );

   // Onto the tables. Four lines each to disable edits, drag/drop and deletes
   fermentableAdditionTable->setEnabled(enabled);
   pushButton_addFerm->setEnabled(enabled);
   pushButton_removeFerm->setEnabled(enabled);
   pushButton_editFerm->setEnabled(enabled);

   hopAdditionTable->setEnabled(enabled);
   pushButton_addHop->setEnabled(enabled);
   pushButton_removeHop->setEnabled(enabled);
   pushButton_editHop->setEnabled(enabled);

   miscAdditionTable->setEnabled(enabled);
   pushButton_addMisc->setEnabled(enabled);
   pushButton_removeMisc->setEnabled(enabled);
   pushButton_editMisc->setEnabled(enabled);

   yeastAdditionTable->setEnabled(enabled);
   pushButton_addYeast->setEnabled(enabled);
   pushButton_removeYeast->setEnabled(enabled);
   pushButton_editYeast->setEnabled(enabled);

   saltAdditionTable->setEnabled(enabled);
   pushButton_addSalt->setEnabled(enabled);
   pushButton_removeSalt->setEnabled(enabled);
   pushButton_editSalt->setEnabled(enabled);

   this->pimpl-> m_fermentableCatalog->setEnableAddToRecipe(enabled);
   this->pimpl->  m_hopCatalog->setEnableAddToRecipe(enabled);
   this->pimpl-> m_miscCatalog->setEnableAddToRecipe(enabled);
   this->pimpl->m_yeastCatalog->setEnableAddToRecipe(enabled);
   this->pimpl-> m_saltCatalog->setEnableAddToRecipe(enabled);
   // NB: Don't yet support add to recipe from water catalog

   // TODO: mashes still need dealing with
   //
   return;
}

void MainWindow::changed(QMetaProperty prop, [[maybe_unused]] QVariant val) {
   QObject * sender = this->sender();
   QString propName(prop.name());
   qDebug() << Q_FUNC_INFO << "sender:" << sender << "; propName:" << propName;

   if (propName == PropertyNames::Recipe::equipment) {
      auto equipment = this->pimpl->m_recipeObs->equipment();
      this->pimpl->m_equipmentEditor->setEditItem(equipment);
   } else if (propName == PropertyNames::Recipe::style) {
      auto style = this->pimpl->m_recipeObs->style();
      this->pimpl->m_styleEditor->setEditItem(style);
   }

   this->showChanges(&prop);
   return;
}

void MainWindow::showChanges(QMetaProperty* prop) {
   if (this->pimpl->m_recipeObs == nullptr) {
      return;
   }

   bool updateAll = (prop == nullptr);
   QString propName;
   if (prop) {
      propName = prop->name();
   }
   qDebug() << Q_FUNC_INFO << "propName:" << propName;

   // May St. Stevens preserve me
   this->lineEdit_name      ->setText    (this->pimpl->m_recipeObs->name          ());
   this->lineEdit_batchSize ->setQuantity(this->pimpl->m_recipeObs->batchSize_l   ());
   this->lineEdit_efficiency->setQuantity(this->pimpl->m_recipeObs->efficiency_pct());
   // TODO: One day we'll want to do some work to properly handle no-boil recipes....
   std::optional<double> const boilSize = this->pimpl->m_recipeObs->boil() ? this->pimpl->m_recipeObs->boil()->preBoilSize_l() : std::nullopt;
   this->value_targetBoilSize->setQuantity(boilSize);
   this->value_boilTime->setQuantity(this->pimpl->m_recipeObs->boil() ? this->pimpl->m_recipeObs->boil()->boilTime_mins() : 0.0);
   this->value_boilSg  ->setQuantity(this->pimpl->m_recipeObs->boilGrav());
   this->lineEdit_name      ->setCursorPosition(0);
   this->lineEdit_batchSize ->setCursorPosition(0);
   this->lineEdit_efficiency->setCursorPosition(0);
/*
   lineEdit_calcBatchSize->setText(this->pimpl->m_recipeObs);
   lineEdit_calcBoilSize->setText(this->pimpl->m_recipeObs);
*/

   // Color manipulation
/*
   if( 0.95*this->pimpl->m_recipeObs->batchSize_l() <= this->pimpl->m_recipeObs->finalVolume_l() && this->pimpl->m_recipeObs->finalVolume_l() <= 1.05*this->pimpl->m_recipeObs->batchSize_l() )
      lineEdit_calcBatchSize->setStyleSheet(this->pimpl->goodSS);
   else if( this->pimpl->m_recipeObs->finalVolume_l() < 0.95*this->pimpl->m_recipeObs->batchSize_l() )
      lineEdit_calcBatchSize->setStyleSheet(this->pimpl->lowSS);
   else
      lineEdit_calcBatchSize->setStyleSheet(this->pimpl->highSS);

   if( 0.95*this->pimpl->m_recipeObs->boilSize_l() <= this->pimpl->m_recipeObs->boilVolume_l() && this->pimpl->m_recipeObs->boilVolume_l() <= 1.05*this->pimpl->m_recipeObs->boilSize_l() )
      lineEdit_calcBoilSize->setStyleSheet(this->pimpl->goodSS);
   else if( this->pimpl->m_recipeObs->boilVolume_l() < 0.95* this->pimpl->m_recipeObs->boilSize_l() )
      lineEdit_calcBoilSize->setStyleSheet(this->pimpl->lowSS);
   else
      lineEdit_calcBoilSize->setStyleSheet(this->pimpl->highSS);
*/

   auto style = this->pimpl->m_recipeObs->style();
   if (style) {
      updateDensitySlider(*this->styleRangeWidget_og, *this->label_og, style->ogMin(), style->ogMax(), 1.120);
   }
   this->styleRangeWidget_og->setValue(this->label_og->getAmountToDisplay(this->pimpl->m_recipeObs->og()));

   if (style) {
      updateDensitySlider(*this->styleRangeWidget_fg, *this->label_fg, style->fgMin(), style->fgMax(), 1.030);
   }
   this->styleRangeWidget_fg->setValue(this->label_fg->getAmountToDisplay(this->pimpl->m_recipeObs->fg()));

   this->styleRangeWidget_abv->setValue(this->pimpl->m_recipeObs->ABV_pct());
   this->styleRangeWidget_ibu->setValue(this->pimpl->m_recipeObs->IBU());

   this->rangeWidget_batchSize->setRange         (0,
                                                  this->label_batchSize->getAmountToDisplay(this->pimpl->m_recipeObs->batchSize_l()));
   this->rangeWidget_batchSize->setPreferredRange(0,
                                                  this->label_batchSize->getAmountToDisplay(this->pimpl->m_recipeObs->finalVolume_l()));
   this->rangeWidget_batchSize->setValue         (this->label_batchSize->getAmountToDisplay(this->pimpl->m_recipeObs->finalVolume_l()));

   this->rangeWidget_boilsize->setRange         (0,
                                                 this->label_boilSize->getAmountToDisplay(boilSize.value_or(0.0)));
   this->rangeWidget_boilsize->setPreferredRange(0,
                                                 this->label_boilSize->getAmountToDisplay(this->pimpl->m_recipeObs->boilVolume_l()));
   this->rangeWidget_boilsize->setValue         (this->label_boilSize->getAmountToDisplay(this->pimpl->m_recipeObs->boilVolume_l()));

   // Colors need the same basic treatment as gravity
   if (style) {
      updateColorSlider(*this->styleRangeWidget_srm,
                        *this->label_color,
                        style->colorMin_srm(),
                        style->colorMax_srm());
   }
   this->styleRangeWidget_srm->setValue(this->label_color->getAmountToDisplay(this->pimpl->m_recipeObs->color_srm()));

   // In some, incomplete, recipes, OG is approximately 1.000, which then makes GU close to 0 and thus IBU/GU insanely
   // large.  Besides being meaningless, such a large number takes up a lot of space.  So, where gravity units are
   // below 1, we just show IBU on the IBU/GU slider.
   auto gravityUnits = (this->pimpl->m_recipeObs->og()-1)*1000;
   if (gravityUnits < 1) {
      gravityUnits = 1;
   }
   this->ibuGuSlider->setValue(this->pimpl->m_recipeObs->IBU()/gravityUnits);

   label_calories->setText(
      QString("%1").arg(
         Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Volume) == Measurement::UnitSystems::volume_Metric ?
         this->pimpl->m_recipeObs->caloriesPer33cl() : this->pimpl->m_recipeObs->caloriesPerUs12oz(),
         0,
         'f',
         0
      )
   );

   // See if we need to change the mash in the table.
   if (this->pimpl->m_recipeObs->mash() && (updateAll || propName == PropertyNames::Recipe::mash)) {
      this->mashStepsWidget->setOwner(this->pimpl->m_recipeObs->mash());
   }
   // See if we need to change the boil in the table.
   if (this->pimpl->m_recipeObs->boil() &&
       (updateAll ||
        propName == PropertyNames::Recipe::boil ||
        propName == PropertyNames::StepOwnerBase::steps)) {
      this->boilStepsWidget->setOwner(this->pimpl->m_recipeObs->boil());
   }
   // See if we need to change the fermentation in the table.
   if (this->pimpl->m_recipeObs->fermentation() && (updateAll || propName == PropertyNames::Recipe::fermentation)) {
      this->fermentationStepsWidget->setOwner(this->pimpl->m_recipeObs->fermentation());
   }

   // Not sure about this, but I am annoyed that modifying the hop usage
   // modifiers isn't automatically updating my display
   if (updateAll) {
     this->pimpl->m_recipeObs->recalcIfNeeded(Hop::staticMetaObject.className());
     this->pimpl->m_hopAdditionsVeriTable.m_sortFilterProxyModel->invalidate();
   }
   return;
}

void MainWindow::updateRecipeName() {
   if (this->pimpl->m_recipeObs == nullptr || ! lineEdit_name->isModified()) {
      return;
   }

   Undoable::doOrRedoUpdate(*this->pimpl->m_recipeObs,
                        TYPE_INFO(Recipe, NamedEntity, name),
                        lineEdit_name->text(),
                        tr("Change Recipe Name"));
   return;
}

void MainWindow::displayRangesEtcForCurrentRecipeStyle() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   auto const style = this->pimpl->m_recipeObs->style();
   if (style) {
      this->styleRangeWidget_og->setPreferredRange(this->label_og->getRangeToDisplay(style->ogMin(), style->ogMax()));

      this->styleRangeWidget_fg->setPreferredRange(this->label_fg->getRangeToDisplay(style->ogMin(), style->ogMax()));

      // If min and/or max ABV is not set on the Style, then use some sensible outer limit(s)
      this->styleRangeWidget_abv->setPreferredRange(style->abvMin_pct().value_or(0.0), style->abvMax_pct().value_or(50.0));
      this->styleRangeWidget_ibu->setPreferredRange(style->ibuMin(), style->ibuMax());
      this->styleRangeWidget_srm->setPreferredRange(this->label_color->getRangeToDisplay(style->colorMin_srm(),
                                                                                           style->colorMax_srm()));
   }

   return;
}

//
// TODO: Would be good to harmonise how these updateRecipeFoo and dropRecipeFoo functions work
//

template<class NE> void MainWindow::setForRecipe(std::shared_ptr<NE> val) {
   if (val) {
      Undoable::doOrRedoUpdate(
         newRelationalUndoableUpdate(
            *this->pimpl->m_recipeObs,
            &Recipe::set<NE>,
            this->pimpl->m_recipeObs->get<NE>(),
            val,
            callbackFunctionFor<NE>(),
            tr("Change %1 on %2 Recipe").arg(NE::localisedName()).arg(this->pimpl->m_recipeObs->name())
         )
      );
   }
   return;
}

template<> std::shared_ptr<Style       > MainWindow::getSelected() { return this->       styleComboBox->getItem(); }
template<> std::shared_ptr<Equipment   > MainWindow::getSelected() { return this->   equipmentComboBox->getItem(); }
template<> std::shared_ptr<Mash        > MainWindow::getSelected() { return this->        mashComboBox->getItem(); }
template<> std::shared_ptr<Boil        > MainWindow::getSelected() { return this->        boilComboBox->getItem(); }
template<> std::shared_ptr<Fermentation> MainWindow::getSelected() { return this->fermentationComboBox->getItem(); }
template<> std::shared_ptr<Water       > MainWindow::getSelected() { return nullptr; }

template<class NE> void MainWindow::updateRecipeFromSelected() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }
   std::shared_ptr<NE> selected = this->getSelected<NE>();
   this->setForRecipe(selected);
   return;
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
// The specialisation for Water below is not currently hugely meaningful, but it's simpler to have it as a no-op than
// add a bunch of special case code in the CatalogBase class.
//
template void MainWindow::updateRecipeFromSelected<Style       >();
template void MainWindow::updateRecipeFromSelected<Equipment   >();
template void MainWindow::updateRecipeFromSelected<Mash        >();
template void MainWindow::updateRecipeFromSelected<Boil        >();
template void MainWindow::updateRecipeFromSelected<Fermentation>();
template void MainWindow::updateRecipeFromSelected<Water       >();

void MainWindow::updateRecipeStyle       () { this->updateRecipeFromSelected<Style       >(); return; }
void MainWindow::updateRecipeEquipment   () { this->updateRecipeFromSelected<Equipment   >(); return; }
void MainWindow::updateRecipeMash        () { this->updateRecipeFromSelected<Mash        >(); return; }
void MainWindow::updateRecipeBoil        () { this->updateRecipeFromSelected<Boil        >(); return; }
void MainWindow::updateRecipeFermentation() { this->updateRecipeFromSelected<Fermentation>(); return; }

void MainWindow::updateStyleInUi() {
   if (this->pimpl->m_recipeObs) {
      auto style = this->pimpl->m_recipeObs->style();
      this->styleComboBox->setItem(style);
      this->displayRangesEtcForCurrentRecipeStyle();
   }
   return;
}

void MainWindow::updateEquipmentInUi() {
   if (this->pimpl->m_recipeObs) {
      auto equipment = this->pimpl->m_recipeObs->equipment();
      this->equipmentComboBox->setItem(equipment);
   }
   return;
}

//
// Note that mashButton, boilButton, fermentationButton do the right thing and update themselves based on signals.  So
// we just have to do the combo boxes here.
//
void MainWindow::updateMashInUi        () { if (this->pimpl->m_recipeObs) { this->        mashComboBox->setItem(this->pimpl->m_recipeObs->        mash()); } return; }
void MainWindow::updateBoilInUi        () { if (this->pimpl->m_recipeObs) { this->        boilComboBox->setItem(this->pimpl->m_recipeObs->        boil()); } return; }
void MainWindow::updateFermentationInUi() { if (this->pimpl->m_recipeObs) { this->fermentationComboBox->setItem(this->pimpl->m_recipeObs->fermentation()); } return; }

void MainWindow::droppedRecipeEquipment(Equipment * kitRaw) {
   if (this->pimpl->m_recipeObs == nullptr) {
      return;
   }

   // equip may be null.
   if (!kitRaw) {
      return;
   }

   auto kit = ObjectStoreWrapper::getSharedFromRaw(kitRaw);

   // We need to hang on to this QUndoCommand pointer because there might be other updates linked to it - see below
   auto equipmentUpdate = newRelationalUndoableUpdate(*this->pimpl->m_recipeObs,
                                                      &Recipe::setEquipment,
                                                      this->pimpl->m_recipeObs->equipment(),
                                                      kit,
                                                      &MainWindow::updateEquipmentInUi,
                                                      tr("Change Recipe Kit"));

   // Keep the mash tun weight and specific heat up to date.
   auto mash = this->pimpl->m_recipeObs->mash();
   if (mash) {
      new SimpleUndoableUpdate(*mash, TYPE_INFO(Mash, mashTunWeight_kg         ), kit->mashTunWeight_kg()         , tr("Change Tun Weight")       , equipmentUpdate);
      new SimpleUndoableUpdate(*mash, TYPE_INFO(Mash, mashTunSpecificHeat_calGC), kit->mashTunSpecificHeat_calGC(), tr("Change Tun Specific Heat"), equipmentUpdate);
   }

   if (QMessageBox::question(this,
                             tr("Equipment request"),
                             tr("Would you like to set the batch size, boil size and time to that requested by the equipment?"),
                             QMessageBox::Yes,
                             QMessageBox::No) == QMessageBox::Yes) {
      // If we do update batch size etc as a result of the equipment change, then we want those updates to undo/redo
      // if and when the equipment change is undone/redone.  Setting the parent field on a QUndoCommand makes that
      // parent the owner, responsible for invoking, deleting, etc.  Technically the descriptions of these subcommands
      // won't ever be seen by the user, but there's no harm in setting them.
      // (The previous call here to this->pimpl->m_mashEditor->setRecipe() was a roundabout way of calling setTunWeight_kg() and
      // setMashTunSpecificHeat_calGC() on the mash.)
      new SimpleUndoableUpdate(*this->pimpl->m_recipeObs, TYPE_INFO(Recipe, batchSize_l), kit->fermenterBatchSize_l(), tr("Change Batch Size"), equipmentUpdate);

      auto boil = this->pimpl->m_recipeObs->nonOptBoil();
      new SimpleUndoableUpdate(*boil, TYPE_INFO(Boil, preBoilSize_l), kit->kettleBoilSize_l(), tr("Change Boil Size"), equipmentUpdate);
      if (kit->boilTime_min()) {
         new SimpleUndoableUpdate(*boil, TYPE_INFO(Boil, boilTime_mins), *kit->boilTime_min(), tr("Change Boil Time"), equipmentUpdate);
      }
   }

   // This will do the equipment update and any related updates - see above
   Undoable::doOrRedoUpdate(equipmentUpdate);
   return;
}

// This isn't called when we think it is...!
void MainWindow::droppedRecipeStyle(Style * styleRaw) {
   if (!this->pimpl->m_recipeObs) {
      qDebug() << Q_FUNC_INFO;
      return;
   }
   // When the style is changed, we also need to update what is shown on the Style button
   qDebug() << Q_FUNC_INFO << "Do or redo";
   auto style = ObjectStoreWrapper::getSharedFromRaw(styleRaw);
   Undoable::doOrRedoUpdate(
      newRelationalUndoableUpdate(*this->pimpl->m_recipeObs,
                                  &Recipe::setStyle,
                                  this->pimpl->m_recipeObs->style(),
                                  style,
                                  &MainWindow::displayRangesEtcForCurrentRecipeStyle,
                                  tr("Change Recipe Style"))
   );

   return;
}

// Well, aint this a kick in the pants. Apparently I can't template a slot
void MainWindow::droppedRecipeFermentable(QList<Fermentable *> fermentables) {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   if (tabWidget_ingredients->currentWidget() != recipeFermentableTab) {
      tabWidget_ingredients->setCurrentWidget(recipeFermentableTab);
   }

   QList<std::shared_ptr<RecipeAdditionFermentable>> fermentableAdditions =
      RecipeAdditionFermentable::create(*this->pimpl->m_recipeObs, fermentables);

   Undoable::doOrRedoUpdate(
      newUndoableAddOrRemoveList(*this->pimpl->m_recipeObs,
                                 &Recipe::addAddition<RecipeAdditionFermentable>,
                                 fermentableAdditions,
                                 &Recipe::removeAddition<RecipeAdditionFermentable>,
                                 tr("Drop fermentable(s) on a recipe"))
   );
   return;
}

void MainWindow::droppedRecipeHop(QList<Hop *> hops) {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   if (tabWidget_ingredients->currentWidget() != recipeHopTab) {
      tabWidget_ingredients->setCurrentWidget(recipeHopTab);
   }

   auto hopAdditions = RecipeAdditionHop::create(*this->pimpl->m_recipeObs, hops);

   Undoable::doOrRedoUpdate(
      newUndoableAddOrRemoveList(*this->pimpl->m_recipeObs,
                                 &Recipe::addAddition<RecipeAdditionHop>,
                                 hopAdditions,
                                 &Recipe::removeAddition<RecipeAdditionHop>,
                                 tr("Drop hop(s) on a recipe"))
   );
   return;
}

void MainWindow::droppedRecipeMisc(QList<Misc *> miscs) {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   if (tabWidget_ingredients->currentWidget() != recipeMiscTab) {
      tabWidget_ingredients->setCurrentWidget(recipeMiscTab);
   }

   auto miscAdditions = RecipeAdditionMisc::create(*this->pimpl->m_recipeObs, miscs);

   Undoable::doOrRedoUpdate(
      newUndoableAddOrRemoveList(*this->pimpl->m_recipeObs,
                                 &Recipe::addAddition<RecipeAdditionMisc>,
                                 miscAdditions,
                                 &Recipe::removeAddition<RecipeAdditionMisc>,
                                 tr("Drop misc(s) on a recipe"))
   );
   return;
}

void MainWindow::droppedRecipeYeast(QList<Yeast *> yeasts) {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   if (tabWidget_ingredients->currentWidget() != recipeYeastTab) {
      tabWidget_ingredients->setCurrentWidget(recipeYeastTab);
   }

   auto yeastAdditions = RecipeAdditionYeast::create(*this->pimpl->m_recipeObs, yeasts);

   Undoable::doOrRedoUpdate(
      newUndoableAddOrRemoveList(*this->pimpl->m_recipeObs,
                                 &Recipe::addAddition<RecipeAdditionYeast>,
                                 yeastAdditions,
                                 &Recipe::removeAddition<RecipeAdditionYeast>,
                                 tr("Drop yeast(s) on a recipe"))
   );
   return;
}

void MainWindow::droppedRecipeSalt(QList<Salt *> salts) {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   if (tabWidget_ingredients->currentWidget() != recipeSaltTab) {
      tabWidget_ingredients->setCurrentWidget(recipeSaltTab);
   }

   auto saltAdditions = RecipeAdjustmentSalt::create(*this->pimpl->m_recipeObs, salts);

   Undoable::doOrRedoUpdate(
      newUndoableAddOrRemoveList(*this->pimpl->m_recipeObs,
                                 &Recipe::addAddition<RecipeAdjustmentSalt>,
                                 saltAdditions,
                                 &Recipe::removeAddition<RecipeAdjustmentSalt>,
                                 tr("Drop salt(s) on a recipe"))
   );
   return;
}

void MainWindow::updateRecipeBatchSize() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   Undoable::doOrRedoUpdate(*this->pimpl->m_recipeObs,
                        TYPE_INFO(Recipe, batchSize_l),
                        lineEdit_batchSize->getNonOptCanonicalQty(),
                        tr("Change Batch Size"));
   return;
}

void MainWindow::updateRecipeEfficiency() {
   qDebug() << Q_FUNC_INFO << lineEdit_efficiency->getNonOptValue<double>();
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   Undoable::doOrRedoUpdate(*this->pimpl->m_recipeObs,
                        TYPE_INFO(Recipe, efficiency_pct),
                        lineEdit_efficiency->getNonOptValue<unsigned int>(),
                        tr("Change Recipe Efficiency"));
   return;
}

template<class NE>
void MainWindow::addIngredientToRecipe(NE & ne) {
   if (!this->pimpl->m_recipeObs) {
      return;
   }
   auto neAddition = std::make_shared<typename NE::RecipeAdditionClass>(*this->pimpl->m_recipeObs, ne);
   this->pimpl->doRecipeAddition(neAddition);
   return;
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template void MainWindow::addIngredientToRecipe(Fermentable & ne);
template void MainWindow::addIngredientToRecipe(Hop         & ne);
template void MainWindow::addIngredientToRecipe(Misc        & ne);
template void MainWindow::addIngredientToRecipe(Yeast       & ne);
template void MainWindow::addIngredientToRecipe(Salt        & ne);

void MainWindow::removeSelectedFermentableAddition() { this->pimpl->m_fermentableAdditionsVeriTable.removeSelected(); return; }
void MainWindow::removeSelectedHopAddition        () { this->pimpl->        m_hopAdditionsVeriTable.removeSelected(); return; }
void MainWindow::removeSelectedMiscAddition       () { this->pimpl->       m_miscAdditionsVeriTable.removeSelected(); return; }
void MainWindow::removeSelectedYeastAddition      () { this->pimpl->      m_yeastAdditionsVeriTable.removeSelected(); return; }
void MainWindow::removeSelectedSaltAddition       () { this->pimpl->       m_saltAdditionsVeriTable.removeSelected(); return; }

//
// There is no general case for MainWindow::getEditor(), only specialisations.
//
template<>   Equipment::EditorClass & MainWindow::getEditor<  Equipment>() const { return *this->pimpl->  m_equipmentEditor; }
template<> Fermentable::EditorClass & MainWindow::getEditor<Fermentable>() const { return *this->pimpl->m_fermentableEditor; }
template<>         Hop::EditorClass & MainWindow::getEditor<        Hop>() const { return *this->pimpl->        m_hopEditor; }
template<>        Misc::EditorClass & MainWindow::getEditor<       Misc>() const { return *this->pimpl->       m_miscEditor; }
template<>        Salt::EditorClass & MainWindow::getEditor<       Salt>() const { return *this->pimpl->       m_saltEditor; }
template<>       Style::EditorClass & MainWindow::getEditor<      Style>() const { return *this->pimpl->      m_styleEditor; }
template<>       Yeast::EditorClass & MainWindow::getEditor<      Yeast>() const { return *this->pimpl->      m_yeastEditor; }

template<>             Mash::EditorClass & MainWindow::getEditor<            Mash>() const { return *this->pimpl->            m_mashEditor; }
template<>             Boil::EditorClass & MainWindow::getEditor<            Boil>() const { return *this->pimpl->            m_boilEditor; }
template<>     Fermentation::EditorClass & MainWindow::getEditor<    Fermentation>() const { return *this->pimpl->    m_fermentationEditor; }
template<>         MashStep::EditorClass & MainWindow::getEditor<        MashStep>() const { return *this->pimpl->        m_mashStepEditor; }
template<>         BoilStep::EditorClass & MainWindow::getEditor<        BoilStep>() const { return *this->pimpl->        m_boilStepEditor; }
template<> FermentationStep::EditorClass & MainWindow::getEditor<FermentationStep>() const { return *this->pimpl->m_fermentationStepEditor; }

//
// There is no general case for MainWindow::getCatalog(), only specialisations.
//
template<>   Equipment::CatalogClass & MainWindow::getCatalog<  Equipment>() const { return *this->pimpl->  m_equipmentCatalog; }
template<> Fermentable::CatalogClass & MainWindow::getCatalog<Fermentable>() const { return *this->pimpl->m_fermentableCatalog; }
template<>         Hop::CatalogClass & MainWindow::getCatalog<        Hop>() const { return *this->pimpl->        m_hopCatalog; }
template<>        Misc::CatalogClass & MainWindow::getCatalog<       Misc>() const { return *this->pimpl->       m_miscCatalog; }
template<>        Salt::CatalogClass & MainWindow::getCatalog<       Salt>() const { return *this->pimpl->       m_saltCatalog; }
template<>       Style::CatalogClass & MainWindow::getCatalog<      Style>() const { return *this->pimpl->      m_styleCatalog; }
template<>       Water::CatalogClass & MainWindow::getCatalog<      Water>() const { return *this->pimpl->      m_waterCatalog; }
template<>       Yeast::CatalogClass & MainWindow::getCatalog<      Yeast>() const { return *this->pimpl->      m_yeastCatalog; }

//
// There is no general case for MainWindow::getWindow(), only specialisations.
//
template<> StockWindow & MainWindow::getWindow<StockWindow>() const { return *this->pimpl->m_stockWindow; }


/**
 * This is akin to a special case of MainWindow::exportSelected()
 */
void MainWindow::exportRecipe() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   QList<Recipe const *> recipes;
   recipes.append(this->pimpl->m_recipeObs);

   bool const exportResult = ImportExport::exportToFile(&recipes);
   if (exportResult) {
      this->updateStatus(tr("Wrote recipe to file"));
   } else {
      this->updateStatus(tr("Error writing recipe to file"));
   }
   return;
}

Recipe* MainWindow::currentRecipe() {
   return this->pimpl->m_recipeObs;
}

void MainWindow::setUndoRedoEnable() {
   QUndoStack & undoStack { Undoable::getStack() };
   this->actionUndo->setEnabled(undoStack.canUndo());
   this->actionRedo->setEnabled(undoStack.canRedo());

   this->actionUndo->setText(QString(tr("Undo %1").arg(undoStack.undoText())));
   this->actionRedo->setText(QString(tr("Redo %1").arg(undoStack.redoText())));

   return;
}

// For undo/redo, we use Qt's Undo framework
void MainWindow::editUndo() {
   QUndoStack & undoStack { Undoable::getStack() };
   if (!undoStack.canUndo()) {
      qDebug() << "Undo called but nothing to undo";
   } else {
      undoStack.undo();
   }

   this->setUndoRedoEnable();
   return;
}

void MainWindow::editRedo() {
   QUndoStack & undoStack { Undoable::getStack() };
   if (!undoStack.canRedo()) {
      qDebug() << "Redo called but nothing to redo";
   } else {
      undoStack.redo();
   }

   this->setUndoRedoEnable();
   return;
}

template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionHop        > item) { this->pimpl->        m_hopAdditionsVeriTable.remove(item); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionFermentable> item) { this->pimpl->m_fermentableAdditionsVeriTable.remove(item); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionMisc       > item) { this->pimpl->       m_miscAdditionsVeriTable.remove(item); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionYeast      > item) { this->pimpl->      m_yeastAdditionsVeriTable.remove(item); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdjustmentSalt     > item) { this->pimpl->       m_saltAdditionsVeriTable.remove(item); return; }

void MainWindow::editFermentableOfSelectedFermentableAddition() { this->pimpl->m_fermentableAdditionsVeriTable.editSelected(); return; }
void MainWindow::editMiscOfSelectedMiscAddition              () { this->pimpl->       m_miscAdditionsVeriTable.editSelected(); return; }
void MainWindow::editHopOfSelectedHopAddition                () { this->pimpl->        m_hopAdditionsVeriTable.editSelected(); return; }
void MainWindow::editYeastOfSelectedYeastAddition            () { this->pimpl->      m_yeastAdditionsVeriTable.editSelected(); return; }
void MainWindow::editSaltOfSelectedSaltAddition              () { this->pimpl->       m_saltAdditionsVeriTable.editSelected(); return; }

std::shared_ptr<Recipe>  MainWindow::newRecipe() {
   QString const name = QInputDialog::getText(this, tr("Recipe name"), tr("Recipe name:"));
   if (name.isEmpty()) {
      return nullptr;
   }

   std::shared_ptr<Recipe> newRec = std::make_shared<Recipe>(name);

   // bad things happened -- let somebody know
   if (!newRec) {
      QMessageBox::warning(this,
                           tr("Error creating recipe"),
                           tr("An error was returned while creating %1").arg(name));
      return nullptr;
   }
   ObjectStoreWrapper::insert(newRec);

   //
   // .:TODO:. For the moment, we still assume that every Recipe has a Boil and a Fermentation.  Also, for the moment,
   // we also create a new boil and fermentation for every Recipe.  In time, I'd like to extend the UI so that, when you
   // input the name for your new Recipe, you also select Mash, Boil, Fermentation (all either from "List of existing"
   // or "Make new").
   //
   std::shared_ptr<Boil> newBoil = std::make_shared<Boil>(tr("Automatically-created Boil for %1").arg(name));
   // NB: Recipe::setBoil will ensure Boil is stored in the database
   newRec->setBoil(newBoil);
   // Since we're auto-creating a Boil, it might as well start out with the "standard" profile
   newBoil->ensureStandardProfile();

   std::shared_ptr<Fermentation> newFermentation =
      std::make_shared<Fermentation>(tr("Automatically-created Fermentation for %1").arg(name));
   // NB: Recipe::setFermentation will ensure Fermentation is stored in the database
   newRec->setFermentation(newFermentation);

   // Set the following stuff so everything appears nice
   // and the calculations don't divide by zero... things like that.
   newRec ->setBatchSize_l   (PersistentSettings::value_ck(PersistentSettings::Names::defaultBatchSize_l  , 18.93).toDouble());
   newBoil->setPreBoilSize_l (PersistentSettings::value_ck(PersistentSettings::Names::defaultPreBoilSize_l, 23.47).toDouble());
   newRec ->setEfficiency_pct(PersistentSettings::value_ck(PersistentSettings::Names::defaultEfficiency   , 70.0 ).toDouble());

   // We need a valid key, so insert the recipe before we add equipment
   QVariant const defEquipKey = PersistentSettings::value_ck(PersistentSettings::Names::defaultEquipmentKey, -1);
   if (defEquipKey != -1) {
      auto equipment = ObjectStoreWrapper::getById<Equipment>(defEquipKey.toInt());
      // I really want to do this before we've written the object to the
      // database
      if ( equipment ) {
         newRec->setBatchSize_l( equipment->fermenterBatchSize_l() );
         newBoil->setPreBoilSize_l( equipment->kettleBoilSize_l() );
         newBoil->setBoilTime_mins( equipment->boilTime_min().value_or(Equipment::default_boilTime_mins) );
         newRec->setEquipment(equipment);
      }
   }

   this->setTreeSelection(treeView_recipe->findElement(newRec.get()));
   this->setRecipe(newRec.get());
   return newRec;
}

void MainWindow::newRecipeInFolder(QString folderPath) {
   auto newRec = this->newRecipe();

   if (!folderPath.isEmpty()) {
      newRec->setFolderPath(folderPath);
   }

   return;
}

void MainWindow::setTreeSelection(QModelIndex index) {
   qDebug() << Q_FUNC_INFO;

   if (!index.isValid()) {
      qDebug() << Q_FUNC_INFO << "Index invalid";
      return;
   }

   TreeView *activeTreeView = this->getActiveTreeView();
   if (!activeTreeView) {
      activeTreeView = qobject_cast<TreeView*>(treeView_recipe);
   }

   // Couldn't cast the activeTreeView index to a TreeView
   if (!activeTreeView) {
      qDebug() << Q_FUNC_INFO << "Couldn't cast the activeTreeView index to a TreeView";
      return;
   }

   activeTreeView->setSelected(index);

   return;
}

void MainWindow::brewItHelper() {
   auto brewNote = this->pimpl->createBrewNote();
   if (brewNote) {
      this->pimpl->reduceInventory(*brewNote);
   }
   return;
}

void MainWindow::brewAgainHelper() {
   auto brewNote = this->pimpl->copySelectedBrewNote();
   if (brewNote) {
      this->pimpl->reduceInventory(*brewNote);
   }
   return;
}

void MainWindow::backup() {
   // NB: QDir does all the necessary magic of translating '/' to whatever current platform's directory separator is
   QString defaultBackupFileName = QDir::currentPath() + "/" + Database::getDefaultBackupFileName();
   QString backupFileName = QFileDialog::getSaveFileName(this, tr("Backup Database"), defaultBackupFileName);
   qDebug() << QString("Database backup filename \"%1\"").arg(backupFileName);

   // If the filename returned from the dialog is empty, it means the user clicked cancel, so we should stop trying to do the backup
   if (!backupFileName.isEmpty())
   {
      bool success = Database::instance().backupToFile(backupFileName);

      if( ! success )
         QMessageBox::warning( this, tr("Oops!"), tr("Could not copy the files for some reason."));
   }
}

void MainWindow::restoreFromBackup() {
   if (QMessageBox::question(
          this,
          tr("A Warning"),
          tr("This will obliterate your current set of recipes and ingredients. Do you want to continue?"),
          QMessageBox::Yes, QMessageBox::No
       ) == QMessageBox::No) {
      return;
   }

   QString restoreDbFile = QFileDialog::getOpenFileName(this, tr("Choose File"), "", tr("SQLite (*.sqlite)"));
   bool success = Database::instance().restoreFromFile(restoreDbFile);

   if (!success) {
      QMessageBox::warning( this, tr("Oops!"), tr("Operation failed.  See log file for more details.") );
   } else {
      QMessageBox::information(this, tr("Restart"), tr("Please restart %1.").arg(CONFIG_APPLICATION_NAME_UC));
      //TODO: do this without requiring restarting :)
   }
   return;
}

// Imports all the recipes, hops, equipment or whatever from a BeerXML or BeerJSON file into the database.
void MainWindow::importFiles() {
   ImportExport::importFromFiles();
   return;
}

void MainWindow::editRecipeEquipment() {
   if (this->pimpl->m_recipeObs && ! this->pimpl->m_recipeObs->equipment()) {
      QMessageBox::warning(this, tr("No equipment"), tr("You must select or define an equipment profile first."));
   } else {
      this->pimpl->m_equipmentEditor->setEditItem(this->pimpl->m_recipeObs->equipment());
      this->pimpl->m_equipmentEditor->show();
   }
   return;
}

void MainWindow::editRecipeStyle() {
   if ( this->pimpl->m_recipeObs && ! this->pimpl->m_recipeObs->style() ) {
      QMessageBox::warning( this, tr("No style"), tr("You must select a style first."));
   } else {
      this->pimpl->m_styleEditor->setEditItem(this->pimpl->m_recipeObs->style());
      this->pimpl->m_styleEditor->show();
   }
   return;
}

void MainWindow::editRecipeMash() {
   this->pimpl->m_mashEditor->setEditItem(this->pimpl->m_recipeObs->mash());
   this->pimpl->m_mashEditor->setRecipe(this->pimpl->m_recipeObs);
   this->pimpl->m_mashEditor->showEditor();
   return;
}

void MainWindow::editRecipeBoil() {
   this->pimpl->m_boilEditor->setEditItem(this->pimpl->m_recipeObs->boil());
   this->pimpl->m_boilEditor->setRecipe(this->pimpl->m_recipeObs);
   this->pimpl->m_boilEditor->showEditor();
   return;
}

void MainWindow::editRecipeFermentation() {
   this->pimpl->m_fermentationEditor->setEditItem(this->pimpl->m_recipeObs->fermentation());
   this->pimpl->m_fermentationEditor->setRecipe(this->pimpl->m_recipeObs);
   this->pimpl->m_fermentationEditor->showEditor();
   return;
}

void MainWindow::closeEvent(QCloseEvent* /*event*/) {
   Application::saveSystemOptions();
   PersistentSettings::insert_ck(PersistentSettings::Names::geometry, saveGeometry());
   PersistentSettings::insert_ck(PersistentSettings::Names::windowState, saveState());
   if (this->pimpl->m_recipeObs) {
      PersistentSettings::insert_ck(PersistentSettings::Names::recipeKey, this->pimpl->m_recipeObs->key());
   }

   // UI save state
   PersistentSettings::saveUiState(PersistentSettings::Names::splitter_vertical_State         , *this->splitter_vertical             );
   PersistentSettings::saveUiState(PersistentSettings::Names::splitter_horizontal_State       , *this->splitter_horizontal           );
   PersistentSettings::saveUiState(PersistentSettings::Names::treeView_recipe_headerState     , *this->treeView_recipe     ->header());
   PersistentSettings::saveUiState(PersistentSettings::Names::treeView_style_headerState      , *this->treeView_style      ->header());
   PersistentSettings::saveUiState(PersistentSettings::Names::treeView_equipment_headerState  , *this->treeView_equipment  ->header());
   PersistentSettings::saveUiState(PersistentSettings::Names::treeView_fermentable_headerState, *this->treeView_fermentable->header());
   PersistentSettings::saveUiState(PersistentSettings::Names::treeView_hop_headerState        , *this->treeView_hop        ->header());
   PersistentSettings::saveUiState(PersistentSettings::Names::treeView_misc_headerState       , *this->treeView_misc       ->header());
   PersistentSettings::saveUiState(PersistentSettings::Names::treeView_yeast_headerState      , *this->treeView_yeast      ->header());

   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_boilCatalog        , *this->pimpl->m_boilCatalog        );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_equipmentCatalog   , *this->pimpl->m_equipmentCatalog   );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_fermentableCatalog , *this->pimpl->m_fermentableCatalog );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_fermentationCatalog, *this->pimpl->m_fermentationCatalog);
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_hopCatalog         , *this->pimpl->m_hopCatalog         );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_mashCatalog        , *this->pimpl->m_mashCatalog        );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_miscCatalog        , *this->pimpl->m_miscCatalog        );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_saltCatalog        , *this->pimpl->m_saltCatalog        );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_styleCatalog       , *this->pimpl->m_styleCatalog       );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_waterCatalog       , *this->pimpl->m_waterCatalog       );
   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_yeastCatalog       , *this->pimpl->m_yeastCatalog       );

   this->pimpl->m_boilCatalog        ->saveUiState(PersistentSettings::Names::uiState_boilCatalog        );
   this->pimpl->m_equipmentCatalog   ->saveUiState(PersistentSettings::Names::uiState_equipmentCatalog   );
   this->pimpl->m_fermentableCatalog ->saveUiState(PersistentSettings::Names::uiState_fermentableCatalog );
   this->pimpl->m_fermentationCatalog->saveUiState(PersistentSettings::Names::uiState_fermentationCatalog);
   this->pimpl->m_hopCatalog         ->saveUiState(PersistentSettings::Names::uiState_hopCatalog         );
   this->pimpl->m_mashCatalog        ->saveUiState(PersistentSettings::Names::uiState_mashCatalog        );
   this->pimpl->m_miscCatalog        ->saveUiState(PersistentSettings::Names::uiState_miscCatalog        );
   this->pimpl->m_saltCatalog        ->saveUiState(PersistentSettings::Names::uiState_saltCatalog        );
   this->pimpl->m_styleCatalog       ->saveUiState(PersistentSettings::Names::uiState_styleCatalog       );
   this->pimpl->m_waterCatalog       ->saveUiState(PersistentSettings::Names::uiState_waterCatalog       );
   this->pimpl->m_yeastCatalog       ->saveUiState(PersistentSettings::Names::uiState_yeastCatalog       );

   this->        mashStepsWidget->saveUiState(PersistentSettings::Names::        mashStepTableWidget_headerState, PersistentSettings::Sections::MainWindow);
   this->        boilStepsWidget->saveUiState(PersistentSettings::Names::        boilStepTableWidget_headerState, PersistentSettings::Sections::MainWindow);
   this->fermentationStepsWidget->saveUiState(PersistentSettings::Names::fermentationStepTableWidget_headerState, PersistentSettings::Sections::MainWindow);

   PersistentSettings::saveGeometry(PersistentSettings::Names::geometry_stockWindow, *this->pimpl->m_stockWindow);
   this->pimpl->m_stockWindow->saveUiState();

   // After unloading the database, can't make any more queries to it, so first
   // make the main window disappear so that redraw events won't inadvertently
   // cause any more queries.
   setVisible(false);
   return;
}

TreeView * MainWindow::getActiveTreeView() const {
   TreeView * activeTreeView = qobject_cast<TreeView *>(this->tabWidget_Trees->currentWidget()->focusWidget());
   return activeTreeView;
}

void MainWindow::copySelected() {
   TreeView * activeTreeView = this->getActiveTreeView();
   if (activeTreeView) {
      activeTreeView->copySelected();
   }
   return;
}

void MainWindow::exportSelected() {
   TreeView * activeTreeView = this->getActiveTreeView();
   if (!activeTreeView) {
      qDebug() << Q_FUNC_INFO << "No active tree so can't get a selection";
      return;
   }

   QModelIndexList selected = activeTreeView->selectionModel()->selectedRows();
   if (selected.count() == 0) {
      qDebug() << Q_FUNC_INFO << "Nothing selected, so nothing to export";
      return;
   }

   //
   // I think the way that UI works at the moment, we're only going to get one type of thing selected at a time.
   // Nevertheless, if this were to change in future, there is no inherent reason not to be able to export different
   // types of things at the same time.
   //
   // We therefore gather all the selected things together so that we write out all the Hops together, all the Styles
   // together and so on, because BeerXML wants them all in group tags (<HOPS>...</HOPS>, etc).
   //
   QList<Equipment   const *> equipments;
   QList<Fermentable const *> fermentables;
   QList<Hop         const *> hops;
   QList<Misc        const *> miscs;
   QList<Recipe      const *> recipes;
   QList<Style       const *> styles;
   QList<Water       const *> waters;
   QList<Yeast       const *> yeasts;

   int count = 0;
   for (auto & selection : selected) {

      // This is all a bit clunky, but extending the trees to include mashes, boils and fermentations mean it's going to
      // be rewritten soon
      TreeNode const * node = activeTreeView->treeNode(selection);
      if (!node) {
         qWarning() << Q_FUNC_INFO << "No node at index" << selection;
      } else {
         QString const nodeClass = node->className();
         if (nodeClass == Recipe::staticMetaObject.className()) {
            auto item = treeView_recipe->getItem<Recipe>(selection);
            if (item) {
               recipes.append(item.get());
               ++count;
            }
         } else if (nodeClass == Equipment::staticMetaObject.className()) {
            auto item = treeView_equipment->getItem<Equipment>(selection);
            if (item) {
               equipments.append(item.get());
               ++count;
            }
         } else if (nodeClass == Fermentable::staticMetaObject.className()) {
            auto item = treeView_fermentable->getItem<Fermentable>(selection);
            if (item) {
               fermentables.append(item.get());
               ++count;
            }
         } else if (nodeClass == Hop::staticMetaObject.className()) {
            auto item = treeView_hop->getItem<Hop>(selection);
            if (item) {
               hops.append(item.get());
               ++count;
            }
         } else if (nodeClass == Misc::staticMetaObject.className()) {
            auto item = treeView_misc->getItem<Misc>(selection);
            if (item) {
               miscs.append(item.get());
               ++count;
            }
         } else if (nodeClass == Style::staticMetaObject.className()) {
            auto item = treeView_style->getItem<Style>(selection);
            if (item) {
               styles.append(item.get());
               ++count;
            }
         } else if (nodeClass == Water::staticMetaObject.className()) {
            auto item = treeView_water->getItem<Water>(selection);
            if (item) {
               waters.append(item.get());
               ++count;
            }
         } else if (nodeClass == Yeast::staticMetaObject.className()) {
            auto item = treeView_yeast->getItem<Yeast>(selection);
            if (item) {
               yeasts.append(item.get());
               ++count;
            }
         } else if (nodeClass == Folder::staticMetaObject.className()) {
            qDebug() << Q_FUNC_INFO << "Can't export selected Folder to XML as BeerXML does not support it";
         } else if (nodeClass == BrewNote::staticMetaObject.className()) {
            qDebug() << Q_FUNC_INFO << "Can't export selected BrewNote to XML as BeerXML does not support it";
         } else {
            // This shouldn't happen, because we should explicitly cover all the types above
            qWarning() << Q_FUNC_INFO << "Don't know how to export TreeNode type" << nodeClass;
         }
      }
   }

   if (0 == count) {
      qDebug() << Q_FUNC_INFO << "Nothing selected was exportable to XML";
      QMessageBox msgBox{QMessageBox::Critical,
                         tr("Nothing to export"),
                         tr("None of the selected items is exportable")};
      msgBox.exec();
      return;
   }

   ImportExport::exportToFile(&recipes,
                              &equipments,
                              &fermentables,
                              &hops,
                              &miscs,
                              &styles,
                              &waters,
                              &yeasts);
   return;
}

void MainWindow::redisplayLabel() {
   // There is a lot of magic going on in the showChanges(). I can either
   // duplicate that magic or I can just call showChanges().
   this->showChanges();
   return;
}

void MainWindow::showPitchDialog() {
   // First, copy the current recipe og and volume.
   if (this->pimpl->m_recipeObs) {
      this->pimpl->m_pitchDialog->setWortVolume_l( this->pimpl->m_recipeObs->finalVolume_l() );
      this->pimpl->m_pitchDialog->setWortDensity( this->pimpl->m_recipeObs->og() );
      this->pimpl->m_pitchDialog->calculate();
   }

   this->pimpl->m_pitchDialog->show();
   return;
}

void MainWindow::changeBrewDate() {
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();

   for (QModelIndex selected : indexes) {
      auto brewNote = treeView_recipe->getItem<BrewNote>(selected);

      // No idea how this could happen, but I've seen stranger things
      if ( ! brewNote ) {
         continue;
      }

      // Pop the calendar, get the date.
      if (this->pimpl->m_btDatePopup->exec() == QDialog::Accepted) {
         QDate newDate = this->pimpl->m_btDatePopup->selectedDate();
         brewNote->setBrewDate(newDate);
         this->pimpl->updateBrewNoteTabText(*brewNote);
      }
   }
   return;
}

void MainWindow::fixBrewNote() {
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();

   for (QModelIndex selected : indexes) {
      auto target = treeView_recipe->getItem<BrewNote>(selected);

      // No idea how this could happen, but I've seen stranger things
      if ( ! target ) {
         continue;
      }

      auto owningRecipe = treeView_recipe->getItem<Recipe>( treeView_recipe->parentIndex(selected));

      if (!owningRecipe) {
         continue;
      }

      target->recalculateEff(owningRecipe.get());
   }
   return;
}

void MainWindow::updateStatus(QString const status) {
   if (this->statusBar()) {
      this->statusBar()->showMessage(status, 3000);
   }
   return;
}

void MainWindow::versionedRecipe(Recipe * descendant) {
   QModelIndex ndx = treeView_recipe->findElement(descendant);
   this->setRecipe(descendant);
   this->treeView_recipe->setCurrentIndex(ndx);
   return;
}

void MainWindow::brewNoteDeleted([[maybe_unused]] int brewNoteId, std::shared_ptr<QObject> object) {
   //
   // This function receives the ObjectStore::signalObjectDeleted signal when a BrewNote is deleted
   //
   BrewNote * deletedBrewNote = std::static_pointer_cast<BrewNote>(object).get();
   Recipe * recipe = ObjectStoreWrapper::getByIdRaw<Recipe>(deletedBrewNote->recipeId());
   qDebug() << Q_FUNC_INFO << "BrewNote" << *deletedBrewNote << "deleted on Recipe" << *recipe;

   // If this isn't the focused recipe, do nothing because there are no tabs
   // to close.
   if (recipe != this->pimpl->m_recipeObs) {
      return;
   }

   this->pimpl->closeBrewNoteTab(*deletedBrewNote);

   return;
}

void MainWindow::setBrewNoteByIndex(QModelIndex const & index) {
   this->pimpl->setBrewNoteByIndex(index);
   return;
}

void MainWindow::showWaterProfileAdjustmentTool() {
   if (this->pimpl->m_recipeObs) {
      if (this->pimpl->m_recipeObs->mash() && this->pimpl->m_recipeObs->mash()->mashSteps().size() > 0) {
         this->pimpl->m_waterProfileAdjustmentTool->setRecipe(this->pimpl->m_recipeObs);
         this->pimpl->m_waterProfileAdjustmentTool->show();
         return;
      }
   }

   QMessageBox::warning( this, tr("No Mash"), tr("You must define a mash first."));
   return;
}

void MainWindow::checkAgainstLatestRelease(QVersionNumber const latestRelease) {
   Application::checkAgainstLatestRelease(latestRelease);
   return;
}
