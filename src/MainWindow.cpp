/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * MainWindow.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "InventoryFormatter.h"
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
#include "WaterDialog.h"
#include "catalogs/EquipmentCatalog.h"
#include "catalogs/FermentableCatalog.h"
#include "catalogs/HopCatalog.h"
#include "catalogs/MiscCatalog.h"
#include "catalogs/SaltCatalog.h"
#include "catalogs/StyleCatalog.h"
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
#include "editors/NamedMashEditor.h"
#include "editors/SaltEditor.h"
#include "editors/StyleEditor.h"
#include "editors/WaterEditor.h"
#include "editors/YeastEditor.h"
#include "qtModels/listModels/EquipmentListModel.h"
#include "qtModels/listModels/MashListModel.h"
#include "qtModels/listModels/StyleListModel.h"
#include "qtModels/listModels/WaterListModel.h"
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
#include "utils/OptionalHelpers.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MainWindow.cpp"
#endif

namespace {

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
      m_boilStepTableModel            {nullptr},
      m_fermentationStepTableModel    {nullptr},
      m_mashStepTableModel            {nullptr},
      m_fermentableAdditionsTableModel{nullptr},
      m_hopAdditionsTableModel        {nullptr},
      m_miscAdditionsTableModel       {nullptr},
      m_yeastAdditionsTableModel      {nullptr},
      m_fermentableAdditionsTableProxy{nullptr},
      m_hopAdditionsTableProxy        {nullptr},
      m_miscAdditionsTableProxy       {nullptr},
      m_yeastAdditionsTableProxy      {nullptr},
      m_saltAdditionsTableProxy       {nullptr} {
      return;
   }

   ~impl() = default;

   /**
    * \brief Configure the tables and their proxies
    *
    *        Anything creating new tables models, filter proxies and configuring the two should go in here
    */
   void setupTables() {
      // Set table models.
      // Fermentable Additions
      m_fermentableAdditionsTableModel = std::make_unique<RecipeAdditionFermentableTableModel>(m_self.fermentableAdditionTable);
      m_fermentableAdditionsTableProxy = std::make_unique<RecipeAdditionFermentableSortFilterProxyModel>(m_self.fermentableAdditionTable, false);
      m_fermentableAdditionsTableProxy->setSourceModel(m_fermentableAdditionsTableModel.get());
      m_self.fermentableAdditionTable->setItemDelegate(new RecipeAdditionFermentableItemDelegate(m_self.fermentableAdditionTable, *m_fermentableAdditionsTableModel));
      m_self.fermentableAdditionTable->setModel(m_fermentableAdditionsTableProxy.get());
      // Make the fermentable table show grain percentages in row headers.
      m_fermentableAdditionsTableModel->setDisplayPercentages(true);
      // Double clicking the name column pops up an edit dialog for the selected item
      connect(m_self.fermentableAdditionTable, &QTableView::doubleClicked, &m_self, [&](const QModelIndex &idx) {
         if (idx.column() == 0) {
            m_self.editFermentableOfSelectedFermentableAddition();
         }
      });

      // Hop additions
      m_hopAdditionsTableModel = std::make_unique<RecipeAdditionHopTableModel>(m_self.hopAdditionTable);
      m_hopAdditionsTableProxy = std::make_unique<RecipeAdditionHopSortFilterProxyModel>(m_self.hopAdditionTable, false);
      m_hopAdditionsTableProxy->setSourceModel(m_hopAdditionsTableModel.get());
      m_self.hopAdditionTable->setItemDelegate(new RecipeAdditionHopItemDelegate(m_self.hopAdditionTable, *m_hopAdditionsTableModel));
      m_self.hopAdditionTable->setModel(m_hopAdditionsTableProxy.get());
      // RecipeAdditionHop table show IBUs in row headers.
      m_hopAdditionsTableModel->setShowIBUs(true);
      connect(m_self.hopAdditionTable, &QTableView::doubleClicked, &m_self, [&](const QModelIndex &idx) {
         if (idx.column() == 0) {
            m_self.editHopOfSelectedHopAddition();
         }
      });

      // Misc
      m_miscAdditionsTableModel = std::make_unique<RecipeAdditionMiscTableModel>(m_self.miscAdditionTable);
      m_miscAdditionsTableProxy = std::make_unique<RecipeAdditionMiscSortFilterProxyModel>(m_self.miscAdditionTable, false);
      m_miscAdditionsTableProxy->setSourceModel(m_miscAdditionsTableModel.get());
      m_self.miscAdditionTable->setItemDelegate(new RecipeAdditionMiscItemDelegate(m_self.miscAdditionTable, *m_miscAdditionsTableModel));
      m_self.miscAdditionTable->setModel(m_miscAdditionsTableProxy.get());
      connect(m_self.miscAdditionTable, &QTableView::doubleClicked, &m_self, [&](const QModelIndex &idx) {
         if (idx.column() == 0) {
            m_self.editMiscOfSelectedMiscAddition();
         }
      });

      // Yeast
      m_yeastAdditionsTableModel = std::make_unique<RecipeAdditionYeastTableModel>(m_self.yeastAdditionTable);
      m_yeastAdditionsTableProxy = std::make_unique<RecipeAdditionYeastSortFilterProxyModel>(m_self.yeastAdditionTable, false);
      m_yeastAdditionsTableProxy->setSourceModel(m_yeastAdditionsTableModel.get());
      m_self.yeastAdditionTable->setItemDelegate(new RecipeAdditionYeastItemDelegate(m_self.yeastAdditionTable, *m_yeastAdditionsTableModel));
      m_self.yeastAdditionTable->setModel(m_yeastAdditionsTableProxy.get());
      connect(m_self.yeastAdditionTable, &QTableView::doubleClicked, &m_self, [&](const QModelIndex &idx) {
         if (idx.column() == 0) {
            m_self.editYeastOfSelectedYeastAddition();
         }
      });

      // Salt
      m_saltAdditionsTableModel = std::make_unique<RecipeAdjustmentSaltTableModel>(m_self.saltAdditionTable);
      m_saltAdditionsTableProxy = std::make_unique<RecipeAdjustmentSaltSortFilterProxyModel>(m_self.saltAdditionTable, false);
      m_saltAdditionsTableProxy->setSourceModel(m_saltAdditionsTableModel.get());
      m_self.saltAdditionTable->setItemDelegate(new RecipeAdjustmentSaltItemDelegate(m_self.saltAdditionTable, *m_saltAdditionsTableModel));
      m_self.saltAdditionTable->setModel(m_saltAdditionsTableProxy.get());
      connect(m_self.saltAdditionTable, &QTableView::doubleClicked, &m_self, [&](const QModelIndex &idx) {
         if (idx.column() == 0) {
            m_self.editSaltOfSelectedSaltAddition();
         }
      });

      // Mashes
      m_mashStepTableModel = std::make_unique<MashStepTableModel>(m_self.mashStepTableWidget);
      m_self.mashStepTableWidget->setItemDelegate(new MashStepItemDelegate(m_self.mashStepTableWidget, *m_mashStepTableModel));
      m_self.mashStepTableWidget->setModel(m_mashStepTableModel.get());
      connect(m_self.mashStepTableWidget, &QTableView::doubleClicked, &m_self, [&](const QModelIndex &idx) {
         if (idx.column() == 0) {
            m_self.editSelectedMashStep();
         }
      });

      // Boils
      m_boilStepTableModel = std::make_unique<BoilStepTableModel>(m_self.boilStepTableWidget);
      m_self.boilStepTableWidget->setItemDelegate(new BoilStepItemDelegate(m_self.boilStepTableWidget, *m_boilStepTableModel));
      m_self.boilStepTableWidget->setModel(m_boilStepTableModel.get());
      connect(m_self.boilStepTableWidget, &QTableView::doubleClicked, &m_self, [&](const QModelIndex &idx) {
         if (idx.column() == 0) {
            m_self.editSelectedBoilStep();
         }
      });

      // Fermentations
      m_fermentationStepTableModel = std::make_unique<FermentationStepTableModel>(m_self.fermentationStepTableWidget);
      m_self.fermentationStepTableWidget->setItemDelegate(new FermentationStepItemDelegate(m_self.fermentationStepTableWidget, *m_fermentationStepTableModel));
      m_self.fermentationStepTableWidget->setModel(m_fermentationStepTableModel.get());
      connect(m_self.fermentationStepTableWidget, &QTableView::doubleClicked, &m_self, [&](const QModelIndex &idx) {
         if (idx.column() == 0) {
            m_self.editSelectedFermentationStep();
         }
      });

      // Enable sorting in the main tables.
      m_self.fermentableAdditionTable->horizontalHeader()->setSortIndicator(static_cast<int>(RecipeAdditionFermentableTableModel::ColumnIndex::Amount), Qt::DescendingOrder );
      m_self.fermentableAdditionTable->setSortingEnabled(true);
      m_fermentableAdditionsTableProxy->setDynamicSortFilter(true);
      m_self.hopAdditionTable->horizontalHeader()->setSortIndicator(static_cast<int>(RecipeAdditionHopTableModel::ColumnIndex::Time), Qt::DescendingOrder );
      m_self.hopAdditionTable->setSortingEnabled(true);
      m_hopAdditionsTableProxy->setDynamicSortFilter(true);
      m_self.miscAdditionTable->horizontalHeader()->setSortIndicator(static_cast<int>(RecipeAdditionMiscTableModel::ColumnIndex::Time), Qt::DescendingOrder );
      m_self.miscAdditionTable->setSortingEnabled(true);
      m_miscAdditionsTableProxy->setDynamicSortFilter(true);
      m_self.yeastAdditionTable->horizontalHeader()->setSortIndicator(static_cast<int>(RecipeAdditionYeastTableModel::ColumnIndex::Name), Qt::DescendingOrder );
      m_self.yeastAdditionTable->setSortingEnabled(true);
      m_yeastAdditionsTableProxy->setDynamicSortFilter(true);
      m_self.saltAdditionTable->horizontalHeader()->setSortIndicator(static_cast<int>(RecipeAdjustmentSaltTableModel::ColumnIndex::Name), Qt::DescendingOrder );
      m_self.saltAdditionTable->setSortingEnabled(true);
      m_saltAdditionsTableProxy->setDynamicSortFilter(true);
   }

   //! \brief Previously called setupContextMenu
   void setupTreeViews() {

      m_self.treeView_recipe->init(*this->m_ancestorDialog, *this->m_optionDialog);

      m_self.treeView_style ->init(*this->m_styleEditor      );
      m_self.treeView_equip ->init(*this->m_equipEditor      );
      m_self.treeView_ferm  ->init(*this->m_fermentableEditor);
      m_self.treeView_hops  ->init(*this->m_hopEditor        );
      m_self.treeView_mash  ->init(*this->m_mashEditor       );
      m_self.treeView_misc  ->init(*this->m_miscEditor       );
      m_self.treeView_salt  ->init(*this->m_saltEditor       );
      m_self.treeView_yeast ->init(*this->m_yeastEditor      );
      m_self.treeView_water ->init(*this->m_waterEditor      );

      connect(m_self.treeView_recipe, &RecipeTreeView::recipeSpawn        , &m_self, &MainWindow::versionedRecipe);
      return;
   }

   /**
    * \brief Create the dialogs, including the file dialogs
    *
    *        Most dialogs are initialized in here. That should include any initial configurations as well.
    */
   void setupDialogs() {
      m_aboutDialog            = std::make_unique<AboutDialog           >(&m_self);
      m_helpDialog             = std::make_unique<HelpDialog            >(&m_self);
      m_equipCatalog           = std::make_unique<EquipmentCatalog      >(&m_self);
      m_equipEditor            = std::make_unique<EquipmentEditor       >(&m_self);
      m_fermCatalog            = std::make_unique<FermentableCatalog    >(&m_self);
      m_fermentableEditor      = std::make_unique<FermentableEditor     >(&m_self);
      m_hopCatalog             = std::make_unique<HopCatalog            >(&m_self);
      m_hopEditor              = std::make_unique<HopEditor             >(&m_self);
      m_mashEditor             = std::make_unique<MashEditor            >(&m_self);
      m_mashStepEditor         = std::make_unique<MashStepEditor        >(&m_self);
      m_boilEditor             = std::make_unique<BoilEditor            >(&m_self);
      m_boilStepEditor         = std::make_unique<BoilStepEditor        >(&m_self);
      m_fermentationEditor     = std::make_unique<FermentationEditor    >(&m_self);
      m_fermentationStepEditor = std::make_unique<FermentationStepEditor>(&m_self);
      m_mashWizard             = std::make_unique<MashWizard            >(&m_self);
      m_miscCatalog            = std::make_unique<MiscCatalog           >(&m_self);
      m_miscEditor             = std::make_unique<MiscEditor            >(&m_self);
      m_saltCatalog            = std::make_unique<SaltCatalog           >(&m_self);
      m_saltEditor             = std::make_unique<SaltEditor            >(&m_self);
      m_styleCatalog           = std::make_unique<StyleCatalog          >(&m_self);
      m_styleEditor            = std::make_unique<StyleEditor           >(&m_self);
      m_yeastCatalog           = std::make_unique<YeastCatalog          >(&m_self);
      m_yeastEditor            = std::make_unique<YeastEditor           >(&m_self);
      m_optionDialog           = std::make_unique<OptionDialog          >(&m_self);
      m_recipeScaler           = std::make_unique<ScaleRecipeTool       >(&m_self);
      m_recipeFormatter        = std::make_unique<RecipeFormatter       >(&m_self);
      m_printAndPreviewDialog  = std::make_unique<PrintAndPreviewDialog >(&m_self);
      m_ogAdjuster             = std::make_unique<OgAdjuster            >(&m_self);
      m_converterTool          = std::make_unique<ConverterTool         >(&m_self);
      m_hydrometerTool         = std::make_unique<HydrometerTool        >(&m_self);
      m_alcoholTool            = std::make_unique<AlcoholTool           >(&m_self);
      m_timerMainDialog        = std::make_unique<TimerMainDialog       >(&m_self);
      m_primingDialog          = std::make_unique<PrimingDialog         >(&m_self);
      m_strikeWaterDialog      = std::make_unique<StrikeWaterDialog     >(&m_self);
      m_refractoDialog         = std::make_unique<RefractoDialog        >(&m_self);
      m_mashDesigner           = std::make_unique<MashDesigner          >(&m_self);
      m_pitchDialog            = std::make_unique<PitchDialog           >(&m_self);
      m_btDatePopup            = std::make_unique<BtDatePopup           >(&m_self);
      m_waterDialog            = std::make_unique<WaterDialog           >(&m_self);
      m_waterEditor            = std::make_unique<WaterEditor           >(&m_self);
      m_ancestorDialog         = std::make_unique<AncestorDialog        >(&m_self);

      return;
   }

   /**
    * \brief Configure combo boxes and their list models
    *
    *        Any new combo boxes, along with their list models, should be initialized here
    */
   void setupComboBoxes() {
      m_self.equipmentComboBox->init();
      m_self.styleComboBox->init();
      m_self.mashComboBox->init();
      m_self.boilComboBox->init();
      m_self.fermentationComboBox->init();

      // Nothing to say.
      m_namedMashEditor = std::make_unique<NamedMashEditor>(&m_self, m_mashStepEditor.get());
      // I don't think this is used yet
      m_singleNamedMashEditor = std::make_unique<NamedMashEditor>(&m_self, m_mashStepEditor.get(), true);
      return;
   }

   /**
    * \brief Common code for getting the currently highlighted entry in one of the recipe additions tables
    *        (hopAdditions, etc).
    */
   template<class NE, class Table, class Proxy, class TableModel>
   NE * selected(Table * table, Proxy * proxy, TableModel * tableModel) {
      QModelIndexList selected = table->selectionModel()->selectedIndexes();

      int size = selected.size();
      if (size == 0) {
         return nullptr;
      }

      // Make sure only one row is selected.
      QModelIndex viewIndex = selected[0];
      int row = viewIndex.row();
      for (int i = 1; i < size; ++i ) {
         if (selected[i].row() != row) {
            return nullptr;
         }
      }

      QModelIndex modelIndex = proxy->mapToSource(viewIndex);
      return tableModel->getRow(modelIndex.row()).get();
   }

   RecipeAdditionFermentable * selectedFermentableAddition() {
      return this->selected<RecipeAdditionFermentable>(m_self.fermentableAdditionTable,
                                                       this->m_fermentableAdditionsTableProxy.get(),
                                                       this->m_fermentableAdditionsTableModel.get());
   }
   RecipeAdditionHop *         selectedHopAddition        () {
      return this->selected<RecipeAdditionHop        >(m_self.hopAdditionTable        ,
                                                       this->m_hopAdditionsTableProxy.get()        ,
                                                       this->m_hopAdditionsTableModel.get());
   }
   RecipeAdditionMisc *        selectedMiscAddition       () {
      return this->selected<RecipeAdditionMisc       >(m_self.miscAdditionTable       ,
                                                       this->m_miscAdditionsTableProxy.get()       ,
                                                       this->m_miscAdditionsTableModel.get());
   }
   RecipeAdditionYeast *       selectedYeastAddition      () {
      return this->selected<RecipeAdditionYeast      >(m_self.yeastAdditionTable      ,
                                                       this->m_yeastAdditionsTableProxy.get()      ,
                                                       this->m_yeastAdditionsTableModel.get());
   }
   RecipeAdjustmentSalt *      selectedSaltAddition       () {
      return this->selected<RecipeAdjustmentSalt     >(m_self.saltAdditionTable       ,
                                                       this->m_saltAdditionsTableProxy.get()       ,
                                                       this->m_saltAdditionsTableModel.get());
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

   /**
    * \brief Use this for removing \c RecipeAdditionHop etc
    */
   template<class NE, class Table, class Proxy, class TableModel>
   void doRemoveRecipeAddition(Table * table, Proxy * proxy, TableModel * tableModel) {
      QModelIndexList selected = table->selectionModel()->selectedIndexes();
      QList< std::shared_ptr<NE> > itemsToRemove;

      int size = selected.size();
      if (size == 0) {
         return;
      }

      for (int i = 0; i < size; i++) {
         QModelIndex viewIndex = selected.at(i);
         QModelIndex modelIndex = proxy->mapToSource(viewIndex);
         itemsToRemove.append(tableModel->getRow(modelIndex.row()));
      }

      for (auto item : itemsToRemove) {
         Undoable::doOrRedoUpdate(
            newUndoableAddOrRemove(*this->m_recipeObs,
                                    &Recipe::removeAddition<NE>,
                                    item,
                                    &Recipe::addAddition<NE>,
                                    tr("Remove %1 from recipe").arg(NE::localisedName()))
         );
         tableModel->remove(item);
      }
      return;
   }

//   template<class StepClass> auto & getStepTableModel() const;
//   template<std::same_as<MashStep        > T> auto & getTableModel<T>() const { return *this->        m_mashStepTableModel; }
//   template<std::same_as<BoilStep        > T> auto & getTableModel<T>() const { return *this->        m_boilStepTableModel; }
//   template<std::same_as<FermentationStep> T> auto & getTableModel<T>() const { return *this->m_fermentationStepTableModel; }

   // This Identifier struct is a "trick" to use overloading to get around the fact that we can't specialise a templated
   // function inside the class declaration.
   template<typename T> struct Identifier { typedef T type; };
   auto & stepTableModel(Identifier<MashStep        > const) const { return *this->        m_mashStepTableModel; }
   auto & stepTableModel(Identifier<BoilStep        > const) const { return *this->        m_boilStepTableModel; }
   auto & stepTableModel(Identifier<FermentationStep> const) const { return *this->m_fermentationStepTableModel; }
   template<class StepClass> auto & getStepTableModel() const { return this->stepTableModel(Identifier<StepClass>{}); }

   auto & stepEditor(Identifier<MashStep        > const) const { return *this->        m_mashStepEditor; }
   auto & stepEditor(Identifier<BoilStep        > const) const { return *this->        m_boilStepEditor; }
   auto & stepEditor(Identifier<FermentationStep> const) const { return *this->m_fermentationStepEditor; }
   template<class StepClass> auto & getStepEditor() const { return this->stepEditor(Identifier<StepClass>{}); }

   auto & tableView(Identifier<MashStep        > const) const { return this->m_self.        mashStepTableWidget; }
   auto & tableView(Identifier<BoilStep        > const) const { return this->m_self.        boilStepTableWidget; }
   auto & tableView(Identifier<FermentationStep> const) const { return this->m_self.fermentationStepTableWidget; }
   template<class StepClass> QTableView * getTableView() const { return this->tableView(Identifier<StepClass>{}); }

   // Here we have a parameter anyway, so we can just use overloading directly
   void setStepOwner(std::shared_ptr<Mash> stepOwner) {
      this->m_recipeObs->setMash(stepOwner);
      this->m_mashStepTableModel->setMash(stepOwner);
      this->m_mashStepEditor->setStepOwner(stepOwner);
      this->m_self.mashButton->setMash(stepOwner);
      return;
   }
   void setStepOwner(std::shared_ptr<Boil> stepOwner) {
      this->m_recipeObs->setBoil(stepOwner);
      this->m_boilStepTableModel->setBoil(stepOwner);
      this->m_boilStepEditor->setStepOwner(stepOwner);
      this->m_self.boilButton->setBoil(stepOwner);
      return;
   }
   void setStepOwner(std::shared_ptr<Fermentation> stepOwner) {
      this->m_recipeObs->setFermentation(stepOwner);
      this->m_fermentationStepTableModel->setFermentation(stepOwner);
      this->m_fermentationStepEditor->setStepOwner(stepOwner);
      this->m_self.fermentationButton->setFermentation(stepOwner);
      return;
   }

   template<class StepClass> void showStepEditor(std::shared_ptr<StepClass> step) {
      auto & stepEditor = this->getStepEditor<StepClass>();
      stepEditor.setEditItem(step);
      stepEditor.setVisible(true);
      return;
   }

   template<class StepClass>
   void newStep() {
      if (!this->m_recipeObs) {
         return;
      }

      std::shared_ptr<typename StepClass::StepOwnerClass> stepOwner =
         this->m_recipeObs->get<typename StepClass::StepOwnerClass>();
      if (stepOwner) {
         // This seems a bit circular, but guarantees that the editor knows to which step owner (eg Mash) the new step
         // (eg MashStep) should be added.
         this->setStepOwner(stepOwner);
      } else {
         auto defaultStepOwner = std::make_shared<typename StepClass::StepOwnerClass>();
         ObjectStoreWrapper::insert(defaultStepOwner);
         this->setStepOwner(defaultStepOwner);
      }

      // This ultimately gets stored in Undoable::addStepToStepOwner() etc
      auto step = std::make_shared<StepClass>("");
      this->showStepEditor(step);
      return;
   }

   //! \return -1 if no row is selected or more than one row is selected
   template<class StepClass>
   [[nodiscard]] int getSelectedRowNum() const {
      QModelIndexList selected = this->getTableView<StepClass>()->selectionModel()->selectedIndexes();
      int size = selected.size();
      if (size == 0) {
         return -1;
      }

      // Make sure only one row is selected.
      int const row = selected[0].row();
      for (int ii = 1; ii < size; ++ii) {
         if (selected[ii].row() != row) {
            return -1;
         }
      }

      return row;
   }

   template<class StepClass>
   void removeSelectedStep() {
      if (!this->m_recipeObs) {
         return;
      }

      std::shared_ptr<typename StepClass::StepOwnerClass> stepOwner =
         this->m_recipeObs->get<typename StepClass::StepOwnerClass>();
      if (!stepOwner) {
         return;
      }

      int const row = getSelectedRowNum<StepClass>();
      if (row < 0) {
         return;
      }

      auto & stepTableModel = this->getStepTableModel<StepClass>();
      auto step = stepTableModel.getRow(row);
      Undoable::doOrRedoUpdate(
         newUndoableAddOrRemove(*stepOwner,
                                &StepClass::StepOwnerClass::remove,
                                step,
                                &StepClass::StepOwnerClass::add,
                                &MainWindow::remove<StepClass>,
                                static_cast<void (MainWindow::*)(std::shared_ptr<StepClass>)>(nullptr),
                                tr("Remove %1").arg(StepClass::localisedName()))
      );

      return;
   }

   template<class StepClass>
   void moveSelectedStepUp() {
      int const row = getSelectedRowNum<StepClass>();

      // Make sure row is valid and we can actually move it up.
      if (row < 1) {
         return;
      }

      auto & stepTableModel = this->getStepTableModel<StepClass>();
      stepTableModel.moveStepUp(row);
      return;
   }

   template<class StepClass>
   void moveSelectedStepDown() {
      int const row = getSelectedRowNum<StepClass>();

      auto & stepTableModel = this->getStepTableModel<StepClass>();

      // Make sure row is valid and it's not the last row so we can move it down.
      if (row < 0 || row >= stepTableModel.rowCount() - 1) {
         return;
      }

      stepTableModel.moveStepDown(row);
      return;
   }

   template<class StepClass>
   void editSelectedStep() {
      if (!this->m_recipeObs) {
         return;
      }

      auto stepOwner = this->m_recipeObs->get<typename StepClass::StepOwnerClass>();
      if (!stepOwner) {
         return;
      }

      int const row = getSelectedRowNum<StepClass>();
      if (row < 0) {
         return;
      }

      auto step = this->getStepTableModel<StepClass>().getRow(static_cast<unsigned int>(row));
      this->showStepEditor(step);
      return;
   }

   template<class UiElement>
   void saveUiState(BtStringConst const & property, UiElement const & uiElement) {
      PersistentSettings::insert(property, uiElement->saveState(), PersistentSettings::Sections::MainWindow);
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

   //! \brief Find an open brewnote tab, if it is open
   BrewNoteWidget * findBrewNoteWidget(BrewNote * b) {
      for (int ii = 0; ii < this->m_self.tabWidget_recipeView->count(); ++ii) {
         if (this->m_self.tabWidget_recipeView->widget(ii)->objectName() == "BrewNoteWidget") {
            BrewNoteWidget* ni = qobject_cast<BrewNoteWidget*>(this->m_self.tabWidget_recipeView->widget(ii));
            if (ni->isBrewNote(b)) {
               return ni;
            }
         }
      }
      return nullptr;
   }

   void setBrewNoteByIndex(QModelIndex const & index) {

      auto bNote = this->m_self.treeView_recipe->getItem<BrewNote>(index);
      if (!bNote) {
         return;
      }

      // HERE
      // This is some clean up work. REMOVE FROM HERE TO THERE
      if ( bNote->projPoints() < 15 )
      {
         double pnts = bNote->projPoints();
         bNote->setProjPoints(pnts);
      }
      if ( bNote->effIntoBK_pct() < 10 )
      {
         bNote->calculateEffIntoBK_pct();
         bNote->calculateBrewHouseEff_pct();
      }
      // THERE

      Recipe* parent  = ObjectStoreWrapper::getByIdRaw<Recipe>(bNote->recipeId());
      QModelIndex pNdx = this->m_self.treeView_recipe->parentIndex(index);

      // This gets complex. Versioning means we can't just clear the open brewnote tabs out.
      if (parent != this->m_recipeObs) {
         if (!this->m_recipeObs->isMyAncestor(*parent)) {
            this->m_self.setRecipe(parent);
         } else if (this->m_self.treeView_recipe->ancestorsAreShowing(pNdx)) {
            this->m_self.tabWidget_recipeView->setCurrentIndex(0);
            // Start closing from the right (highest index) down. Anything else dumps
            // core in the most unpleasant of fashions
            int tabs = this->m_self.tabWidget_recipeView->count() - 1;
            for (int i = tabs; i >= 0; --i) {
               if (this->m_self.tabWidget_recipeView->widget(i)->objectName() ==
                   BrewNoteWidget::staticMetaObject.className()) {
                  this->m_self.tabWidget_recipeView->removeTab(i);
               }
            }
            this->m_self.setRecipe(parent);
         }
      }

      BrewNoteWidget * ni = this->findBrewNoteWidget(bNote.get());
      if (!ni) {
         ni = new BrewNoteWidget(this->m_self.tabWidget_recipeView);
         ni->setBrewNote(bNote.get());
      }

      this->m_self.tabWidget_recipeView->addTab(ni, bNote->brewDate_short());
      this->m_self.tabWidget_recipeView->setCurrentWidget(ni);
      return;
   }

   void setBrewNote(BrewNote * bNote) {
      BrewNoteWidget* ni = this->findBrewNoteWidget(bNote);
      if (ni) {
         this->m_self.tabWidget_recipeView->setCurrentWidget(ni);
         return;
      }

      ni = new BrewNoteWidget(this->m_self.tabWidget_recipeView);
      ni->setBrewNote(bNote);

      this->m_self.tabWidget_recipeView->addTab(ni, bNote->brewDate_short());
      this->m_self.tabWidget_recipeView->setCurrentWidget(ni);
      return;
   }

   //================================================ MEMBER VARIABLES =================================================
   MainWindow & m_self;

   Recipe * m_recipeObs = nullptr;

   // all things tables should go here.
   std::unique_ptr<BoilStepTableModel                 > m_boilStepTableModel            ;
   std::unique_ptr<FermentationStepTableModel         > m_fermentationStepTableModel    ;
   std::unique_ptr<MashStepTableModel                 > m_mashStepTableModel            ;
   std::unique_ptr<RecipeAdditionFermentableTableModel> m_fermentableAdditionsTableModel;
   std::unique_ptr<RecipeAdditionHopTableModel        > m_hopAdditionsTableModel        ;
   std::unique_ptr<RecipeAdditionMiscTableModel       > m_miscAdditionsTableModel       ;
   std::unique_ptr<RecipeAdditionYeastTableModel      > m_yeastAdditionsTableModel      ;
   std::unique_ptr<RecipeAdjustmentSaltTableModel     > m_saltAdditionsTableModel       ;

   // all things sort/filter proxy go here
   std::unique_ptr<RecipeAdditionFermentableSortFilterProxyModel> m_fermentableAdditionsTableProxy;
   std::unique_ptr<RecipeAdditionHopSortFilterProxyModel        > m_hopAdditionsTableProxy        ;
   std::unique_ptr<RecipeAdditionMiscSortFilterProxyModel       > m_miscAdditionsTableProxy       ;
   std::unique_ptr<RecipeAdditionYeastSortFilterProxyModel      > m_yeastAdditionsTableProxy      ;
   std::unique_ptr<RecipeAdjustmentSaltSortFilterProxyModel     > m_saltAdditionsTableProxy       ;

   // All initialised in setupDialogs
   std::unique_ptr<AboutDialog           > m_aboutDialog           ;
   std::unique_ptr<AlcoholTool           > m_alcoholTool           ;
   std::unique_ptr<AncestorDialog        > m_ancestorDialog        ;
   std::unique_ptr<BoilEditor            > m_boilEditor            ;
   std::unique_ptr<BoilStepEditor        > m_boilStepEditor        ;
   std::unique_ptr<BtDatePopup           > m_btDatePopup           ;
   std::unique_ptr<ConverterTool         > m_converterTool         ;
   std::unique_ptr<EquipmentCatalog      > m_equipCatalog          ;
   std::unique_ptr<EquipmentEditor       > m_equipEditor           ;
   std::unique_ptr<FermentableCatalog    > m_fermCatalog           ;
   std::unique_ptr<FermentableEditor     > m_fermentableEditor     ;
   std::unique_ptr<FermentationEditor    > m_fermentationEditor    ;
   std::unique_ptr<FermentationStepEditor> m_fermentationStepEditor;
   std::unique_ptr<HelpDialog            > m_helpDialog            ;
   std::unique_ptr<HopCatalog            > m_hopCatalog            ;
   std::unique_ptr<HopEditor             > m_hopEditor             ;
   std::unique_ptr<HydrometerTool        > m_hydrometerTool        ;
   std::unique_ptr<MashDesigner          > m_mashDesigner          ;
   std::unique_ptr<MashEditor            > m_mashEditor            ;
   std::unique_ptr<MashStepEditor        > m_mashStepEditor        ;
   std::unique_ptr<MashWizard            > m_mashWizard            ;
   std::unique_ptr<MiscCatalog           > m_miscCatalog           ;
   std::unique_ptr<MiscEditor            > m_miscEditor            ;
   std::unique_ptr<OgAdjuster            > m_ogAdjuster            ;
   std::unique_ptr<OptionDialog          > m_optionDialog          ;
   std::unique_ptr<PitchDialog           > m_pitchDialog           ;
   std::unique_ptr<PrimingDialog         > m_primingDialog         ;
   std::unique_ptr<PrintAndPreviewDialog > m_printAndPreviewDialog ;
   std::unique_ptr<RecipeFormatter       > m_recipeFormatter       ;
   std::unique_ptr<RefractoDialog        > m_refractoDialog        ;
   std::unique_ptr<ScaleRecipeTool       > m_recipeScaler          ;
   std::unique_ptr<StrikeWaterDialog     > m_strikeWaterDialog     ;
   std::unique_ptr<SaltCatalog           > m_saltCatalog           ;
   std::unique_ptr<SaltEditor            > m_saltEditor            ;
   std::unique_ptr<StyleCatalog          > m_styleCatalog          ;
   std::unique_ptr<StyleEditor           > m_styleEditor           ;
   std::unique_ptr<TimerMainDialog       > m_timerMainDialog       ;
   std::unique_ptr<WaterDialog           > m_waterDialog           ;
   std::unique_ptr<WaterEditor           > m_waterEditor           ;
   std::unique_ptr<YeastCatalog          > m_yeastCatalog          ;
   std::unique_ptr<YeastEditor           > m_yeastEditor           ;

   std::unique_ptr<NamedMashEditor> m_namedMashEditor;
   std::unique_ptr<NamedMashEditor> m_singleNamedMashEditor;

   QString highSS, lowSS, goodSS, boldSS; // Palette replacements
};


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), pimpl{std::make_unique<impl>(*this)} {
   qDebug() << Q_FUNC_INFO;

   // Need to call this parent class method to get all the widgets added (I think).
   this->setupUi(this);

   // Initialise smart labels etc early, but after call to this->setupUi() because otherwise member variables such as
   // label_name will not yet be set.
   // .:TBD:. We should fix some of these inconsistently-named labels
   //
   // TBD: Not sure what original difference was supposed to be between label_targetBatchSize & label_batchSize or
   //      between label_targetBoilSize and label_boilSize.
   //
   SMART_FIELD_INIT(MainWindow, label_name           , lineEdit_name      , Recipe, PropertyNames::NamedEntity::name        );
   SMART_FIELD_INIT(MainWindow, label_targetBatchSize, lineEdit_batchSize , Recipe, PropertyNames::Recipe::batchSize_l   , 2);
   SMART_FIELD_INIT(MainWindow, label_targetBoilSize , lineEdit_boilSize  , Boil  , PropertyNames::Boil::preBoilSize_l   , 2);
   SMART_FIELD_INIT(MainWindow, label_efficiency     , lineEdit_efficiency, Recipe, PropertyNames::Recipe::efficiency_pct, 1);
   SMART_FIELD_INIT(MainWindow, label_boilTime       , lineEdit_boilTime  , Boil  , PropertyNames::Boil::boilTime_mins   , 0);
   SMART_FIELD_INIT(MainWindow, label_boilSg         , lineEdit_boilSg    , Recipe, PropertyNames::Recipe::boilGrav      , 3);

   SMART_FIELD_INIT_NO_SF(MainWindow, oGLabel        , Recipe, PropertyNames::Recipe::og         );
   SMART_FIELD_INIT_NO_SF(MainWindow, fGLabel        , Recipe, PropertyNames::Recipe::fg         );
   SMART_FIELD_INIT_NO_SF(MainWindow, colorSRMLabel  , Recipe, PropertyNames::Recipe::color_srm  );
   SMART_FIELD_INIT_NO_SF(MainWindow, label_batchSize, Recipe, PropertyNames::Recipe::batchSize_l);
   SMART_FIELD_INIT_NO_SF(MainWindow, label_boilSize , Boil  , PropertyNames::Boil::preBoilSize_l);

   // Stop things looking ridiculously tiny on high DPI displays
   this->pimpl->setSizesInPixelsBasedOnDpi();

   // Horizontal tabs, please -- even on Mac OS, as the tabs contain square icons
   tabWidget_Trees->tabBar()->setStyle(new BtHorizontalTabs(true));

   /* PLEASE DO NOT REMOVE.
    This code is left here, commented out, intentionally. The only way I can
    test internationalization is by forcing the locale manually. I am tired
    of having to figure this out every time I need to test.
    PLEASE DO NOT REMOVE.
   QLocale german(QLocale::German,QLocale::Germany);
   QLocale::setDefault(german);
   */

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

   // Moved from Database class
   Recipe::connectSignalsForAllRecipes();
   qDebug() << Q_FUNC_INFO << "Recipe signals connected";
   Mash::connectSignals();
   qDebug() << Q_FUNC_INFO << "Mash signals connected";
   Boil::connectSignals();
   qDebug() << Q_FUNC_INFO << "Boil signals connected";
   Fermentation::connectSignals();
   qDebug() << Q_FUNC_INFO << "Fermentation signals connected";


   // No connections from the database yet? Oh FSM, that probably means I'm
   // doing it wrong again.
   // .:TODO:. Change this so we use the newer deleted signal!
   connect(&ObjectStoreTyped<BrewNote>::getInstance(), &ObjectStoreTyped<BrewNote>::signalObjectDeleted, this, &MainWindow::closeBrewNote);

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

///   // The bold style sheet doesn't change, so set it here once.
///   lineEdit_boilSg->setStyleSheet(this->pimpl->boldSS);

   // Disabled fields should change color, but not become unreadable. Mucking
   // with the css seems the most reasonable way to do that.
   QString tabDisabled = QString("QWidget:disabled { color: #000000; background: #F0F0F0 }");
   tab_recipe->setStyleSheet(tabDisabled);
   tabWidget_ingredients->setStyleSheet(tabDisabled);

   return;
}

// Configures the range widgets for the bubbles
void MainWindow::setupRanges() {
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

   const int srmMax = 50;
   styleRangeWidget_srm->setRange(0.0, static_cast<double>(srmMax));
   styleRangeWidget_srm->setPrecision(1);
   styleRangeWidget_srm->setTickMarks(10, 2);
   // Need to change appearance of color slider
   {
      // The styleRangeWidget_srm should display beer color in the background
      QLinearGradient grad( 0,0, 1,0 );
      grad.setCoordinateMode(QGradient::ObjectBoundingMode);
      for( int i=0; i <= srmMax; ++i )
      {
         double srm = i;
         grad.setColorAt( srm/static_cast<double>(srmMax), Algorithms::srmToColor(srm));
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
}

// Anything resulting in a restoreState() should go in here
void MainWindow::restoreSavedState() {

   // If we saved a size the last time we ran, use it
   if (PersistentSettings::contains(PersistentSettings::Names::geometry)) {
      restoreGeometry(PersistentSettings::value(PersistentSettings::Names::geometry).toByteArray());
      restoreState(PersistentSettings::value(PersistentSettings::Names::windowState).toByteArray());
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
   if (PersistentSettings::contains(PersistentSettings::Names::recipeKey)) {
      key = PersistentSettings::value(PersistentSettings::Names::recipeKey).toInt();
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
   if (PersistentSettings::contains(PersistentSettings::Names::splitter_vertical_State,
                                    PersistentSettings::Sections::MainWindow)) {
      splitter_vertical->restoreState(PersistentSettings::value(PersistentSettings::Names::splitter_vertical_State,
                                                                QVariant(),
                                                                PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::splitter_horizontal_State,
                                    PersistentSettings::Sections::MainWindow)) {
      splitter_horizontal->restoreState(PersistentSettings::value(PersistentSettings::Names::splitter_horizontal_State,
                                                                  QVariant(),
                                                                  PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::treeView_recipe_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      treeView_recipe->header()->restoreState(PersistentSettings::value(PersistentSettings::Names::treeView_recipe_headerState,
                                                                        QVariant(),
                                                                        PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::treeView_style_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      treeView_style->header()->restoreState(PersistentSettings::value(PersistentSettings::Names::treeView_style_headerState,
                                                                       QVariant(),
                                                                       PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::treeView_equip_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      treeView_equip->header()->restoreState(PersistentSettings::value(PersistentSettings::Names::treeView_equip_headerState,
                                                                       QVariant(),
                                                                       PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::treeView_ferm_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      treeView_ferm->header()->restoreState(PersistentSettings::value(PersistentSettings::Names::treeView_ferm_headerState,
                                                                      QVariant(),
                                                                      PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::treeView_hops_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      treeView_hops->header()->restoreState(PersistentSettings::value(PersistentSettings::Names::treeView_hops_headerState,
                                                                      QVariant(),
                                                                      PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::treeView_misc_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      treeView_misc->header()->restoreState(PersistentSettings::value(PersistentSettings::Names::treeView_misc_headerState,
                                                                      QVariant(),
                                                                      PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::treeView_yeast_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      treeView_yeast->header()->restoreState(PersistentSettings::value(PersistentSettings::Names::treeView_yeast_headerState,
                                                                       QVariant(),
                                                                       PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::mashStepTableWidget_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      mashStepTableWidget->horizontalHeader()->restoreState(PersistentSettings::value(PersistentSettings::Names::mashStepTableWidget_headerState,
                                                                                      QVariant(),
                                                                                      PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::boilStepTableWidget_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      boilStepTableWidget->horizontalHeader()->restoreState(PersistentSettings::value(PersistentSettings::Names::boilStepTableWidget_headerState,
                                                                                      QVariant(),
                                                                                      PersistentSettings::Sections::MainWindow).toByteArray());
   }
   if (PersistentSettings::contains(PersistentSettings::Names::fermentationStepTableWidget_headerState,
                                    PersistentSettings::Sections::MainWindow)) {
      fermentationStepTableWidget->horizontalHeader()->restoreState(PersistentSettings::value(PersistentSettings::Names::fermentationStepTableWidget_headerState,
                                                                                      QVariant(),
                                                                                      PersistentSettings::Sections::MainWindow).toByteArray());
   }
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
   connect(actionEquipments                , &QAction::triggered, this->pimpl->m_equipCatalog.get()         , &QWidget::show                     ); // > View > Equipments
   connect(actionMashs                     , &QAction::triggered, this->pimpl->m_namedMashEditor.get()      , &QWidget::show                     ); // > View > Mashs
   connect(actionStyles                    , &QAction::triggered, this->pimpl->m_styleCatalog.get()         , &QWidget::show                     ); // > View > Styles
   connect(actionFermentables              , &QAction::triggered, this->pimpl->m_fermCatalog.get()          , &QWidget::show                     ); // > View > Fermentables
   connect(actionHops                      , &QAction::triggered, this->pimpl->m_hopCatalog.get()           , &QWidget::show                     ); // > View > Hops
   connect(actionMiscs                     , &QAction::triggered, this->pimpl->m_miscCatalog.get()          , &QWidget::show                     ); // > View > Miscs
   connect(actionYeasts                    , &QAction::triggered, this->pimpl->m_yeastCatalog.get()         , &QWidget::show                     ); // > View > Yeasts
   connect(actionSalts                     , &QAction::triggered, this->pimpl->m_saltCatalog.get()          , &QWidget::show                     ); // > View > Salts
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
   connect(actionWater_Chemistry           , &QAction::triggered, this                                      , &MainWindow::showWaterChemistryTool); // > Tools > Water Chemistry
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
   connect(this->equipmentButton          , &QAbstractButton::clicked, this                             , &MainWindow::showEquipmentEditor);
   connect(this->styleButton              , &QAbstractButton::clicked, this                             , &MainWindow::showStyleEditor    );
   connect(this->        mashButton       , &QAbstractButton::clicked, this->pimpl->        m_mashEditor.get(),         &MashEditor::showEditor);
   connect(this->        boilButton       , &QAbstractButton::clicked, this->pimpl->        m_boilEditor.get(),         &BoilEditor::showEditor);
   connect(this->fermentationButton       , &QAbstractButton::clicked, this->pimpl->m_fermentationEditor.get(), &FermentationEditor::showEditor);
   connect(this->pushButton_addFerm       , &QAbstractButton::clicked, this->pimpl-> m_fermCatalog.get(), &QWidget::show         );
   connect(this->pushButton_addHop        , &QAbstractButton::clicked, this->pimpl->  m_hopCatalog.get(), &QWidget::show         );
   connect(this->pushButton_addMisc       , &QAbstractButton::clicked, this->pimpl-> m_miscCatalog.get(), &QWidget::show         );
   connect(this->pushButton_addYeast      , &QAbstractButton::clicked, this->pimpl->m_yeastCatalog.get(), &QWidget::show         );
   connect(this->pushButton_addSalt       , &QAbstractButton::clicked, this->pimpl-> m_saltCatalog.get(), &QWidget::show         );

   connect(this->pushButton_removeFerm    , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedFermentableAddition);
   connect(this->pushButton_removeHop     , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedHopAddition        );
   connect(this->pushButton_removeMisc    , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedMiscAddition       );
   connect(this->pushButton_removeYeast   , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedYeastAddition      );
   connect(this->pushButton_removeSalt    , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedSaltAddition       );

   connect(this->pushButton_editFerm      , &QAbstractButton::clicked, this                       , &MainWindow::editFermentableOfSelectedFermentableAddition);
   connect(this->pushButton_editHop       , &QAbstractButton::clicked, this                       , &MainWindow::editHopOfSelectedHopAddition                );
   connect(this->pushButton_editMisc      , &QAbstractButton::clicked, this                       , &MainWindow::editMiscOfSelectedMiscAddition              );
   connect(this->pushButton_editYeast     , &QAbstractButton::clicked, this                       , &MainWindow::editYeastOfSelectedYeastAddition            );
   connect(this->pushButton_editSalt      , &QAbstractButton::clicked, this                       , &MainWindow::editSaltOfSelectedSaltAddition              );

   connect(this->pushButton_editMash        , &QAbstractButton::clicked, this->pimpl->        m_mashEditor.get(),         &MashEditor::showEditor);
   connect(this->pushButton_editBoil        , &QAbstractButton::clicked, this->pimpl->        m_boilEditor.get(),         &BoilEditor::showEditor);
   connect(this->pushButton_editFermentation, &QAbstractButton::clicked, this->pimpl->m_fermentationEditor.get(), &FermentationEditor::showEditor);

   connect(this->pushButton_addMashStep        , &QAbstractButton::clicked, this, &MainWindow::addMashStep        );
   connect(this->pushButton_addBoilStep        , &QAbstractButton::clicked, this, &MainWindow::addBoilStep        );
   connect(this->pushButton_addFermentationStep, &QAbstractButton::clicked, this, &MainWindow::addFermentationStep);

   connect(this->pushButton_removeMashStep        , &QAbstractButton::clicked, this, &MainWindow::removeSelectedMashStep        );
   connect(this->pushButton_removeBoilStep        , &QAbstractButton::clicked, this, &MainWindow::removeSelectedBoilStep        );
   connect(this->pushButton_removeFermentationStep, &QAbstractButton::clicked, this, &MainWindow::removeSelectedFermentationStep);

   connect(this->pushButton_editMashStep        , &QAbstractButton::clicked, this, &MainWindow::editSelectedMashStep        );
   connect(this->pushButton_editBoilStep        , &QAbstractButton::clicked, this, &MainWindow::editSelectedBoilStep        );
   connect(this->pushButton_editFermentationStep, &QAbstractButton::clicked, this, &MainWindow::editSelectedFermentationStep);

   connect(this->pushButton_mashUp        , &QAbstractButton::clicked, this, &MainWindow::moveSelectedMashStepUp        );
   connect(this->pushButton_boilUp        , &QAbstractButton::clicked, this, &MainWindow::moveSelectedBoilStepUp        );
   connect(this->pushButton_fermentationUp, &QAbstractButton::clicked, this, &MainWindow::moveSelectedFermentationStepUp);

   connect(this->pushButton_mashDown        , &QAbstractButton::clicked, this, &MainWindow::moveSelectedMashStepDown        );
   connect(this->pushButton_boilDown        , &QAbstractButton::clicked, this, &MainWindow::moveSelectedBoilStepDown        );
   connect(this->pushButton_fermentationDown, &QAbstractButton::clicked, this, &MainWindow::moveSelectedFermentationStepDown);

   connect(this->pushButton_saveMash      , &QAbstractButton::clicked, this                     , &MainWindow::saveMash                 );
//   connect(this->pushButton_saveBoil      , &QAbstractButton::clicked, this        , &MainWindow::saveBoil                 ); TODO!
//   connect(this->pushButton_saveFermentation      , &QAbstractButton::clicked, this, &MainWindow::saveFermentation                 ); TODO!

   connect(this->pushButton_mashRemove    , &QAbstractButton::clicked, this                       , &MainWindow::removeMash               );
//   connect(this->pushButton_boilRemove    , &QAbstractButton::clicked, this        , &MainWindow::removeBoil               ); TODO!
//   connect(this->pushButton_fermentationRemove    , &QAbstractButton::clicked, this, &MainWindow::removeFermentation               ); TODO!

   connect(this->pushButton_mashWizard, &QAbstractButton::clicked, this->pimpl->m_mashWizard.get()  , &MashWizard::show  );
   connect(this->pushButton_mashDes   , &QAbstractButton::clicked, this->pimpl->m_mashDesigner.get(), &MashDesigner::show);

   return;
}

// comboBoxes with a SIGNAL of activated() should go in here.
void MainWindow::setupActivate() {
   connect(this->equipmentComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeEquipment);
   connect(this->styleComboBox,     QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeStyle);
   connect(this->mashComboBox,      QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeMash);
   return;
}

// lineEdits with either an editingFinished() or a textModified() should go in here
void MainWindow::setupTextEdit() {
   connect(this->lineEdit_name      , &QLineEdit::editingFinished,  this, &MainWindow::updateRecipeName);
   connect(this->lineEdit_batchSize , &SmartLineEdit::textModified, this, &MainWindow::updateRecipeBatchSize);
///   connect(this->lineEdit_boilSize  , &SmartLineEdit::textModified, this, &MainWindow::updateRecipeBoilSize);
   connect(this->lineEdit_efficiency, &SmartLineEdit::textModified, this, &MainWindow::updateRecipeEfficiency);
   return;
}

// anything using a SmartLabel::changedSystemOfMeasurementOrScale signal should go in here
void MainWindow::setupLabels() {
   // These are the sliders. I need to consider these harder, but small steps
   connect(this->oGLabel,       &SmartLabel::changedSystemOfMeasurementOrScale, this, &MainWindow::redisplayLabel);
   connect(this->fGLabel,       &SmartLabel::changedSystemOfMeasurementOrScale, this, &MainWindow::redisplayLabel);
   connect(this->colorSRMLabel, &SmartLabel::changedSystemOfMeasurementOrScale, this, &MainWindow::redisplayLabel);
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
void MainWindow::setRecipe(Recipe* recipe) {
   // Don't like void pointers.
   if (!recipe) {
      return;
   }

   qDebug() << Q_FUNC_INFO << "Recipe #" << recipe->key() << ":" << recipe->name();

   int tabs = 0;

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
   this->pimpl->m_fermentableAdditionsTableModel->observeRecipe(recipe);
   this->pimpl->        m_hopAdditionsTableModel->observeRecipe(recipe);
   this->pimpl->       m_miscAdditionsTableModel->observeRecipe(recipe);
   this->pimpl->      m_yeastAdditionsTableModel->observeRecipe(recipe);
   this->pimpl->       m_saltAdditionsTableModel->observeRecipe(recipe);
   this->pimpl->m_mashStepTableModel->setMash(this->pimpl->m_recipeObs->mash());
   this->pimpl->m_boilStepTableModel->setBoil(this->pimpl->m_recipeObs->boil());
   this->pimpl->m_fermentationStepTableModel->setFermentation(this->pimpl->m_recipeObs->fermentation());

   // Clean out any brew notes
   tabWidget_recipeView->setCurrentIndex(0);
   // Start closing from the right (highest index) down. Anything else dumps
   // core in the most unpleasant of fashions
   tabs = tabWidget_recipeView->count() - 1;
   for (int i = tabs; i >= 0; --i) {
      if (tabWidget_recipeView->widget(i)->objectName() == "BrewNoteWidget")
         tabWidget_recipeView->removeTab(i);
   }

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
      this->pimpl->m_equipEditor->setEditItem(recipe->equipment());
   }
   this->styleButton->setRecipe(recipe);
   this->styleComboBox->setItem(recipe->style());
   if (recipe->style()) {
      this->pimpl->m_styleEditor->setEditItem(recipe->style());
   }

   this->pimpl->m_mashEditor->setEditItem(recipe->mash());
   this->pimpl->m_mashEditor->setRecipe(recipe);
   this->mashButton->setMash(recipe->mash());
   this->mashComboBox->setItem(recipe->mash());

   this->pimpl->m_boilEditor->setEditItem(recipe->boil());
   this->pimpl->m_boilEditor->setRecipe(recipe);
   this->boilComboBox->setItem(recipe->boil());

   this->pimpl->m_fermentationEditor->setEditItem(recipe->fermentation());
   this->pimpl->m_fermentationEditor->setRecipe(recipe);
   this->fermentationComboBox->setItem(recipe->fermentation());

   this->pimpl->m_recipeScaler->setRecipe(recipe);

   // Set the locked flag as required
   checkBox_locked->setCheckState( recipe->locked() ? Qt::Checked : Qt::Unchecked );
   lockRecipe( recipe->locked() ? Qt::Checked : Qt::Unchecked );

   // Here's the fun part. If the recipe is locked and display is false, then
   // you have said "show versions" and we will not all the recipe to be
   // unlocked. Hmmm. Skeptical Mik is skeptical
   if ( recipe->locked() && ! recipe->display() ) {
      checkBox_locked->setEnabled( false );
   }
   else {
      checkBox_locked->setEnabled( true );
   }

   checkBox_locked->setCheckState( recipe->locked() ? Qt::Checked : Qt::Unchecked );
   lockRecipe(recipe->locked() ? Qt::Checked : Qt::Unchecked );

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

   this->pimpl-> m_fermCatalog->setEnableAddToRecipe(enabled);
   this->pimpl->  m_hopCatalog->setEnableAddToRecipe(enabled);
   this->pimpl-> m_miscCatalog->setEnableAddToRecipe(enabled);
   this->pimpl->m_yeastCatalog->setEnableAddToRecipe(enabled);
   this->pimpl-> m_saltCatalog->setEnableAddToRecipe(enabled);
   // TODO: mashes still need dealing with
   //
   return;
}

void MainWindow::changed(QMetaProperty prop, [[maybe_unused]] QVariant val) {
   QString propName(prop.name());

   if (propName == PropertyNames::Recipe::equipment) {
      auto equipment = this->pimpl->m_recipeObs->equipment();
      this->pimpl->m_equipEditor->setEditItem(equipment);
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
   this->lineEdit_boilSize  ->setQuantity(boilSize);
   this->lineEdit_boilTime  ->setQuantity(this->pimpl->m_recipeObs->boil() ? this->pimpl->m_recipeObs->boil()->boilTime_mins() : 0.0);
   this->lineEdit_boilSg    ->setQuantity(this->pimpl->m_recipeObs->boilGrav());
   this->lineEdit_name      ->setCursorPosition(0);
   this->lineEdit_batchSize ->setCursorPosition(0);
   this->lineEdit_boilSize  ->setCursorPosition(0);
   this->lineEdit_efficiency->setCursorPosition(0);
   this->lineEdit_boilTime  ->setCursorPosition(0);
   this->lineEdit_boilSg    ->setCursorPosition(0);
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
      updateDensitySlider(*this->styleRangeWidget_og, *this->oGLabel, style->ogMin(), style->ogMax(), 1.120);
   }
   this->styleRangeWidget_og->setValue(this->oGLabel->getAmountToDisplay(this->pimpl->m_recipeObs->og()));

   if (style) {
      updateDensitySlider(*this->styleRangeWidget_fg, *this->fGLabel, style->fgMin(), style->fgMax(), 1.030);
   }
   this->styleRangeWidget_fg->setValue(this->fGLabel->getAmountToDisplay(this->pimpl->m_recipeObs->fg()));

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

   /* Colors need the same basic treatment as gravity */
   if (style) {
      updateColorSlider(*this->styleRangeWidget_srm,
                        *this->colorSRMLabel,
                        style->colorMin_srm(),
                        style->colorMax_srm());
   }
   this->styleRangeWidget_srm->setValue(this->colorSRMLabel->getAmountToDisplay(this->pimpl->m_recipeObs->color_srm()));

   // In some, incomplete, recipes, OG is approximately 1.000, which then makes GU close to 0 and thus IBU/GU insanely
   // large.  Besides being meaningless, such a large number takes up a lot of space.  So, where gravity units are
   // below 1, we just show IBU on the IBU/GU slider.
   auto gravityUnits = (this->pimpl->m_recipeObs->og()-1)*1000;
   if (gravityUnits < 1) {
      gravityUnits = 1;
   }
   ibuGuSlider->setValue(this->pimpl->m_recipeObs->IBU()/gravityUnits);

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
      this->pimpl->m_mashStepTableModel->setMash(this->pimpl->m_recipeObs->mash());
   }
   // See if we need to change the boil in the table.
   if (this->pimpl->m_recipeObs->boil() &&
       (updateAll ||
        propName == PropertyNames::Recipe::boil ||
        propName == PropertyNames::StepOwnerBase::steps)) {
      this->pimpl->m_boilStepTableModel->setBoil(this->pimpl->m_recipeObs->boil());
   }
   // See if we need to change the fermentation in the table.
   if (this->pimpl->m_recipeObs->fermentation() && (updateAll || propName == PropertyNames::Recipe::fermentation)) {
      this->pimpl->m_fermentationStepTableModel->setFermentation(this->pimpl->m_recipeObs->fermentation());
   }

   // Not sure about this, but I am annoyed that modifying the hop usage
   // modifiers isn't automatically updating my display
   if (updateAll) {
     this->pimpl->m_recipeObs->recalcIfNeeded(Hop::staticMetaObject.className());
     this->pimpl->m_hopAdditionsTableProxy->invalidate();
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
   if ( this->pimpl->m_recipeObs == nullptr ) {
      return;
   }

   auto style = this->pimpl->m_recipeObs->style();
   this->styleButton->setStyle(style);
   this->styleComboBox->setItem(style);

   if (style) {
      this->styleRangeWidget_og->setPreferredRange(this->oGLabel->getRangeToDisplay(style->ogMin(), style->ogMax()));

      this->styleRangeWidget_fg->setPreferredRange(this->fGLabel->getRangeToDisplay(style->ogMin(), style->ogMax()));

      // If min and/or max ABV is not set on the Style, then use some sensible outer limit(s)
      this->styleRangeWidget_abv->setPreferredRange(style->abvMin_pct().value_or(0.0), style->abvMax_pct().value_or(50.0));
      this->styleRangeWidget_ibu->setPreferredRange(style->ibuMin(), style->ibuMax());
      this->styleRangeWidget_srm->setPreferredRange(this->colorSRMLabel->getRangeToDisplay(style->colorMin_srm(),
                                                                                          style->colorMax_srm()));
   }

   return;
}

//
// TODO: Would be good to harmonise how these updateRecipeFoo and dropRecipeFoo functions work
//

void MainWindow::updateRecipeStyle() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   auto selected = this->styleComboBox->getItem();
   if (selected) {
      Undoable::doOrRedoUpdate(
         newRelationalUndoableUpdate(*this->pimpl->m_recipeObs,
                                     &Recipe::setStyle,
                                     this->pimpl->m_recipeObs->style(),
                                     selected,
                                     &MainWindow::displayRangesEtcForCurrentRecipeStyle,
                                     tr("Change Recipe Style"))
      );
   }
   return;
}

void MainWindow::updateRecipeMash() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   auto selected = this->mashComboBox->getItem();
   if (selected) {
      Undoable::doOrRedoUpdate(
         newRelationalUndoableUpdate(*this->pimpl->m_recipeObs,
                                     &Recipe::setMash,
                                     this->pimpl->m_recipeObs->mash(),
                                     selected,
                                     nullptr,
                                     tr("Change Recipe Mash"))
      );
   }
   return;
}

void MainWindow::updateRecipeEquipment() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   auto selected = this->equipmentComboBox->getItem();
   if (selected) {
      Undoable::doOrRedoUpdate(
         newRelationalUndoableUpdate(*this->pimpl->m_recipeObs,
                                     &Recipe::setEquipment,
                                     this->pimpl->m_recipeObs->equipment(),
                                     selected,
                                     &MainWindow::updateEquipmentSelector,
                                     tr("Change Recipe Equipment"))
      );
   }
   return;
}

void MainWindow::updateEquipmentSelector() {
   if (this->pimpl->m_recipeObs != nullptr) {
      auto equipment = this->pimpl->m_recipeObs->equipment();
      this->equipmentButton->setEquipment(equipment);
      this->equipmentComboBox->setItem(equipment);
   }
   return;
}

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
                                                      &MainWindow::updateEquipmentSelector,
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

///void MainWindow::updateRecipeBoilSize() {
///   if (!this->pimpl->m_recipeObs) {
///      return;
///   }
///
///   // See comments in model/Boil.h for why boil size is, technically, optional
///   auto boil = this->pimpl->m_recipeObs->nonOptBoil();
///   Undoable::doOrRedoUpdate(*boil,
///                        TYPE_INFO(Boil, preBoilSize_l),
///                        lineEdit_boilSize->getOptCanonicalQty(),
///                        tr("Change Boil Size"));
///   return;
///}

///void MainWindow::updateRecipeBoilTime() {
///   if (!this->pimpl->m_recipeObs) {
///      return;
///   }
///
///   auto kit = this->pimpl->m_recipeObs->equipment();
///   double boilTime = Measurement::qStringToSI(lineEdit_boilTime->text(), Measurement::PhysicalQuantity::Time).quantity;
///
///   // Here, we rely on a signal/slot connection to propagate the equipment changes to this->pimpl->m_recipeObs->boilTime_min and maybe
///   // this->pimpl->m_recipeObs->boilSize_l
///   // NOTE: This works because kit is the recipe's equipment, not the generic equipment in the recipe drop down.
///   if (kit) {
///      Undoable::doOrRedoUpdate(*kit, TYPE_INFO(Equipment, boilTime_min), boilTime, tr("Change Boil Time"));
///   } else {
///      auto boil = this->pimpl->m_recipeObs->nonOptBoil();
///      Undoable::doOrRedoUpdate(*boil, TYPE_INFO(Boil, boilTime_mins), boilTime, tr("Change Boil Time"));
///   }
///
///   return;
///}

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

void MainWindow::removeSelectedFermentableAddition() {
   this->pimpl->doRemoveRecipeAddition<RecipeAdditionFermentable>(fermentableAdditionTable,
                                                                  this->pimpl->m_fermentableAdditionsTableProxy.get(),
                                                                  this->pimpl->m_fermentableAdditionsTableModel.get());
   return;
}

void MainWindow::removeSelectedHopAddition() {
   this->pimpl->doRemoveRecipeAddition<RecipeAdditionHop>(hopAdditionTable,
                                                          this->pimpl->m_hopAdditionsTableProxy.get(),
                                                          this->pimpl->m_hopAdditionsTableModel.get());
   return;
}

void MainWindow::removeSelectedMiscAddition() {
   this->pimpl->doRemoveRecipeAddition<RecipeAdditionMisc>(miscAdditionTable,
                                                           this->pimpl->m_miscAdditionsTableProxy.get(),
                                                           this->pimpl->m_miscAdditionsTableModel.get());
   return;
}

void MainWindow::removeSelectedYeastAddition() {
   this->pimpl->doRemoveRecipeAddition<RecipeAdditionYeast>(yeastAdditionTable,
                                                            this->pimpl->m_yeastAdditionsTableProxy.get(),
                                                            this->pimpl->m_yeastAdditionsTableModel.get());
   return;
}

void MainWindow::removeSelectedSaltAddition() {
   this->pimpl->doRemoveRecipeAddition<RecipeAdjustmentSalt>(saltAdditionTable,
                                                           this->pimpl->m_saltAdditionsTableProxy.get(),
                                                           this->pimpl->m_saltAdditionsTableModel.get());
   return;
}

void MainWindow::addStepToStepOwner(std::shared_ptr<MashStep> mashStep) {
   Undoable::addStepToStepOwner(this->pimpl->m_recipeObs->mash(), mashStep);
   return;
}
void MainWindow::addStepToStepOwner(std::shared_ptr<BoilStep> boilStep) {
   // It's a coding error if we're trying to add a BoilStep when there is no Boil
   Q_ASSERT(this->pimpl->m_recipeObs->boil());
   Undoable::addStepToStepOwner(this->pimpl->m_recipeObs->boil(), boilStep);
   return;
}
void MainWindow::addStepToStepOwner(std::shared_ptr<FermentationStep> fermentationStep) {
   Undoable::addStepToStepOwner(this->pimpl->m_recipeObs->fermentation(), fermentationStep);
   return;
}

/**
 * This is akin to a special case of MainWindow::exportSelected()
 */
void MainWindow::exportRecipe() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   QList<Recipe const *> recipes;
   recipes.append(this->pimpl->m_recipeObs);

   ImportExport::exportToFile(&recipes);
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

template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionHop        > itemToRemove) { this->pimpl->        m_hopAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionFermentable> itemToRemove) { this->pimpl->m_fermentableAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionMisc       > itemToRemove) { this->pimpl->       m_miscAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionYeast      > itemToRemove) { this->pimpl->      m_yeastAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdjustmentSalt     > itemToRemove) { this->pimpl->       m_saltAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<MashStep                 > itemToRemove) { this->pimpl->            m_mashStepTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<BoilStep                 > itemToRemove) { this->pimpl->            m_boilStepTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<FermentationStep         > itemToRemove) { this->pimpl->    m_fermentationStepTableModel->remove(itemToRemove); return; }

void MainWindow::editFermentableOfSelectedFermentableAddition() {
   RecipeAdditionFermentable * fermentableAddition = this->pimpl->selectedFermentableAddition();
   if (!fermentableAddition) {
      return;
   }

   Fermentable * fermentable = fermentableAddition->fermentable();
   if (!fermentable) {
      return;
   }

   this->pimpl->m_fermentableEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(fermentable));
   this->pimpl->m_fermentableEditor->show();
   return;
}

void MainWindow::editMiscOfSelectedMiscAddition() {
   RecipeAdditionMisc * miscAddition = this->pimpl->selectedMiscAddition();
   if (!miscAddition) {
      return;
   }

   Misc * misc = miscAddition->misc();
   if (!misc) {
      return;
   }

   this->pimpl->m_miscEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(misc));
   this->pimpl->m_miscEditor->show();
   return;
}

void MainWindow::editHopOfSelectedHopAddition() {
   RecipeAdditionHop * hopAddition = this->pimpl->selectedHopAddition();
   if (!hopAddition) {
      return;
   }

   Hop * hop = hopAddition->hop();
   if (!hop) {
      return;
   }

   this->pimpl->m_hopEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(hop));
   this->pimpl->m_hopEditor->show();
   return;
}

void MainWindow::editYeastOfSelectedYeastAddition() {
   RecipeAdditionYeast * yeastAddition = this->pimpl->selectedYeastAddition();
   if (!yeastAddition) {
      return;
   }

   Yeast * yeast = yeastAddition->yeast();
   if (!yeast) {
      return;
   }

   this->pimpl->m_yeastEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(yeast));
   this->pimpl->m_yeastEditor->show();
   return;
}

void MainWindow::editSaltOfSelectedSaltAddition() {
   RecipeAdjustmentSalt * saltAddition = this->pimpl->selectedSaltAddition();
   if (!saltAddition) {
      return;
   }

   Salt * salt = saltAddition->salt();
   if (!salt) {
      return;
   }

   this->pimpl->m_saltEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(salt));
   this->pimpl->m_saltEditor->show();
   return;
}

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
   newRec->setBatchSize_l(18.93); // 5 gallons
   newBoil->setPreBoilSize_l(23.47);  // 6.2 gallons
   newRec->setEfficiency_pct(70.0);

   // We need a valid key, so insert the recipe before we add equipment
   QVariant const defEquipKey = PersistentSettings::value(PersistentSettings::Names::defaultEquipmentKey, -1);
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

   this->setTreeSelection(treeView_recipe->findElement(newRec));
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

// reduces the inventory by the selected recipes
void MainWindow::reduceInventory() {

   for (QModelIndex selected : treeView_recipe->selectionModel()->selectedRows()) {
      auto rec = treeView_recipe->getItem<Recipe>(selected);
      if (!rec) {
         // Try the parent recipe
         rec = treeView_recipe->getItem<Recipe>(treeView_recipe->parentIndex(selected));
         if (!rec) {
            continue;
         }
      }

      // Make sure everything is properly set and selected
      if (rec.get() != this->pimpl->m_recipeObs) {
         this->setRecipe(rec.get());
      }

      //
      // Reduce fermentables, miscs, hops, yeasts
      //
      // Note that the amount can be mass, volume or (for Yeast and Misc) count.  We don't worry about which here as we
      // assume that a given type of ingredient is always measured in the same way.
      //
      for (auto ii : rec->fermentableAdditions()) {
         auto inv = ii->fermentable()->totalInventory();
         inv.quantity = std::max(inv.quantity - ii->amount().quantity, 0.0);
         ii->fermentable()->setTotalInventory(inv);
      }
      for (auto ii : rec->hopAdditions()) {
         auto inv = ii->hop()->totalInventory();
         inv.quantity = std::max(inv.quantity - ii->amount().quantity, 0.0);
         ii->hop()->setTotalInventory(inv);
      }
      for (auto ii : rec->miscAdditions()) {
         auto inv = ii->misc()->totalInventory();
         inv.quantity = std::max(inv.quantity - ii->amount().quantity, 0.0);
         ii->misc()->setTotalInventory(inv);
      }
      for (auto ii : rec->yeastAdditions()) {
         auto inv = ii->yeast()->totalInventory();
         inv.quantity = std::max(inv.quantity - ii->amount().quantity, 0.0);
         ii->yeast()->setTotalInventory(inv);
      }
   }

   return;
}

// Need to make sure the recipe tree is active, I think
void MainWindow::newBrewNote() {
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();

   for (QModelIndex selected : indexes) {
      auto recipe = this->treeView_recipe->getItem<Recipe>(selected);
      if (!recipe) {
         continue;
      }

      // Make sure everything is properly set and selected
      if (recipe.get() != this->pimpl->m_recipeObs) {
         this->setRecipe(recipe.get());
      }

      auto brewNote = std::make_shared<BrewNote>(*recipe);
      brewNote->populateNote(recipe.get());
      brewNote->setBrewDate();
      ObjectStoreWrapper::insert(brewNote);

      this->pimpl->setBrewNote(brewNote.get());

      QModelIndex brewNoteIndex = treeView_recipe->findElement(brewNote);
      if (brewNoteIndex.isValid()) {
         this->setTreeSelection(brewNoteIndex);
      }
   }
   return;
}

void MainWindow::reBrewNote() {
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
   for (QModelIndex selected : indexes) {
      auto selectedBrewNote = treeView_recipe->getItem<BrewNote>(selected);
      auto recipe           = treeView_recipe->getItem<Recipe>(treeView_recipe->parentIndex(selected));

      if (!selectedBrewNote || !recipe) {
         return;
      }

      auto bNote = std::make_shared<BrewNote>(*selectedBrewNote);
      bNote->setBrewDate();
      ObjectStoreWrapper::insert(bNote);

      if (recipe.get() != this->pimpl->m_recipeObs) {
         this->setRecipe(recipe.get());
      }

      this->pimpl->setBrewNote(bNote.get());

      this->setTreeSelection(treeView_recipe->findElement(bNote));
   }
   return;
}

void MainWindow::brewItHelper() {
   this->newBrewNote();
   this->reduceInventory();
   return;
}

void MainWindow::brewAgainHelper() {
   this->reBrewNote();
   this->reduceInventory();
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

void MainWindow::addMashStep        () { this->pimpl->newStep<MashStep        >(); return; }
void MainWindow::addBoilStep        () { this->pimpl->newStep<BoilStep        >(); return; }
void MainWindow::addFermentationStep() { this->pimpl->newStep<FermentationStep>(); return; }
void MainWindow::removeSelectedMashStep        () { this->pimpl->removeSelectedStep<MashStep        >(); return; }
void MainWindow::removeSelectedBoilStep        () { this->pimpl->removeSelectedStep<BoilStep        >(); return; }
void MainWindow::removeSelectedFermentationStep() { this->pimpl->removeSelectedStep<FermentationStep>(); return; }
void MainWindow::moveSelectedMashStepUp        () { this->pimpl->moveSelectedStepUp<MashStep        >(); return; }
void MainWindow::moveSelectedBoilStepUp        () { this->pimpl->moveSelectedStepUp<BoilStep        >(); return; }
void MainWindow::moveSelectedFermentationStepUp() { this->pimpl->moveSelectedStepUp<FermentationStep>(); return; }
void MainWindow::moveSelectedMashStepDown        () { this->pimpl->moveSelectedStepDown<MashStep        >(); return; }
void MainWindow::moveSelectedBoilStepDown        () { this->pimpl->moveSelectedStepDown<BoilStep        >(); return; }
void MainWindow::moveSelectedFermentationStepDown() { this->pimpl->moveSelectedStepDown<FermentationStep>(); return; }
void MainWindow::editSelectedMashStep        () { this->pimpl->editSelectedStep<MashStep        >(); return; }
void MainWindow::editSelectedBoilStep        () { this->pimpl->editSelectedStep<BoilStep        >(); return; }
void MainWindow::editSelectedFermentationStep() { this->pimpl->editSelectedStep<FermentationStep>(); return; }

//
// .:TBD:. Not sure we should delete the mash just because it's removed from the Recipe
//
void MainWindow::removeMash() {
   auto mashToRemove = this->mashButton->getMash();
   if (!mashToRemove) {
      return;
   }

   //due to way this is designed, we can't have a NULL mash, so
   //we need to remove all the mash steps and then remove the mash
   //from the database.
   //remove from db

   mashToRemove->removeAllSteps();
   ObjectStoreWrapper::softDelete(*mashToRemove);

   auto defaultMash = std::make_shared<Mash>();
   ObjectStoreWrapper::insert(defaultMash);
   this->pimpl->m_recipeObs->setMash(defaultMash);

   this->pimpl->m_mashStepTableModel->setMash(defaultMash);

   //remove from combobox handled automatically by qt
   this->mashButton->setMash(defaultMash);
   return;
}

void MainWindow::closeEvent(QCloseEvent* /*event*/) {
   Application::saveSystemOptions();
   PersistentSettings::insert(PersistentSettings::Names::geometry, saveGeometry());
   PersistentSettings::insert(PersistentSettings::Names::windowState, saveState());
   if ( this->pimpl->m_recipeObs )
      PersistentSettings::insert(PersistentSettings::Names::recipeKey, this->pimpl->m_recipeObs->key());

   // UI save state
   this->pimpl->saveUiState(PersistentSettings::Names::splitter_vertical_State                , splitter_vertical                              );
   this->pimpl->saveUiState(PersistentSettings::Names::splitter_horizontal_State              , splitter_horizontal                            );
   this->pimpl->saveUiState(PersistentSettings::Names::treeView_recipe_headerState            , treeView_recipe->header()                      );
   this->pimpl->saveUiState(PersistentSettings::Names::treeView_style_headerState             , treeView_style->header()                       );
   this->pimpl->saveUiState(PersistentSettings::Names::treeView_equip_headerState             , treeView_equip->header()                       );
   this->pimpl->saveUiState(PersistentSettings::Names::treeView_ferm_headerState              , treeView_ferm->header()                        );
   this->pimpl->saveUiState(PersistentSettings::Names::treeView_hops_headerState              , treeView_hops->header()                        );
   this->pimpl->saveUiState(PersistentSettings::Names::treeView_misc_headerState              , treeView_misc->header()                        );
   this->pimpl->saveUiState(PersistentSettings::Names::treeView_yeast_headerState             , treeView_yeast->header()                       );
   this->pimpl->saveUiState(PersistentSettings::Names::mashStepTableWidget_headerState        , mashStepTableWidget->horizontalHeader()        );
   this->pimpl->saveUiState(PersistentSettings::Names::boilStepTableWidget_headerState        , boilStepTableWidget->horizontalHeader()        );
   this->pimpl->saveUiState(PersistentSettings::Names::fermentationStepTableWidget_headerState, fermentationStepTableWidget->horizontalHeader());

   // After unloading the database, can't make any more queries to it, so first
   // make the main window disappear so that redraw events won't inadvertently
   // cause any more queries.
   setVisible(false);
   return;
}

void MainWindow::saveMash() {
   if (!this->pimpl->m_recipeObs || !this->pimpl->m_recipeObs->mash()) {
      return;
   }

   auto mash = this->pimpl->m_recipeObs->mash();
   // Ensure the mash has a name.
   if( mash->name() == "" ) {
      QMessageBox::information( this, tr("Oops!"), tr("Please give your mash a name before saving.") );
      return;
   }

   // The current UI doesn't make this 100% clear, but what we're actually doing here is saving a _copy_ of the current
   // Recipe's mash.

   // NOTE: should NOT displace this->pimpl->m_recipeObs' current mash.
   auto newMash = ObjectStoreWrapper::insertCopyOf(*mash);
   // NOTE: need to set the display to true for the saved, named mash to work
   newMash->setDisplay(true);
   mashButton->setMash(newMash);
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
            auto item = treeView_equip->getItem<Equipment>(selection);
            if (item) {
               equipments.append(item.get());
               ++count;
            }
         } else if (nodeClass == Fermentable::staticMetaObject.className()) {
            auto item = treeView_ferm->getItem<Fermentable>(selection);
            if (item) {
               fermentables.append(item.get());
               ++count;
            }
         } else if (nodeClass == Hop::staticMetaObject.className()) {
            auto item = treeView_hops->getItem<Hop>(selection);
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

void MainWindow::showEquipmentEditor() {
   if (this->pimpl->m_recipeObs && ! this->pimpl->m_recipeObs->equipment()) {
      QMessageBox::warning(this, tr("No equipment"), tr("You must select or define an equipment profile first."));
   } else {
      this->pimpl->m_equipEditor->setEditItem(this->pimpl->m_recipeObs->equipment());
      this->pimpl->m_equipEditor->show();
   }
   return;
}

void MainWindow::showStyleEditor() {
   if ( this->pimpl->m_recipeObs && ! this->pimpl->m_recipeObs->style() ) {
      QMessageBox::warning( this, tr("No style"), tr("You must select a style first."));
   } else {
      this->pimpl->m_styleEditor->setEditItem(this->pimpl->m_recipeObs->style());
      this->pimpl->m_styleEditor->show();
   }
   return;
}

void MainWindow::changeBrewDate() {
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();

   for (QModelIndex selected : indexes) {
      auto target = treeView_recipe->getItem<BrewNote>(selected);

      // No idea how this could happen, but I've seen stranger things
      if ( ! target )
         continue;

      // Pop the calendar, get the date.
      if ( this->pimpl->m_btDatePopup->exec() == QDialog::Accepted )
      {
         QDate newDate = this->pimpl->m_btDatePopup->selectedDate();
         target->setBrewDate(newDate);

         // If this note is open in a tab
         BrewNoteWidget * ni = this->pimpl->findBrewNoteWidget(target.get());
         if (ni) {
            tabWidget_recipeView->setTabText(tabWidget_recipeView->indexOf(ni), target->brewDate_short());
            return;
         }
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

void MainWindow::updateStatus(const QString status) {
   if (statusBar()) {
      statusBar()->showMessage(status, 3000);
   }
   return;
}

void MainWindow::versionedRecipe(Recipe* descendant) {
   QModelIndex ndx = treeView_recipe->findElement(ObjectStoreWrapper::getShared(*descendant));
   this->setRecipe(descendant);
   this->treeView_recipe->setCurrentIndex(ndx);
   return;
}

// .:TBD:. Seems redundant to pass both the brewnote ID and a pointer to it; we only need one of these
void MainWindow::closeBrewNote([[maybe_unused]] int brewNoteId, std::shared_ptr<QObject> object) {
   BrewNote* b = std::static_pointer_cast<BrewNote>(object).get();
   Recipe* parent = ObjectStoreWrapper::getByIdRaw<Recipe>(b->recipeId());

   // If this isn't the focused recipe, do nothing because there are no tabs
   // to close.
   if (parent != this->pimpl->m_recipeObs) {
      return;
   }

   BrewNoteWidget* ni = this->pimpl->findBrewNoteWidget(b);
   if (ni) {
      tabWidget_recipeView->removeTab( tabWidget_recipeView->indexOf(ni));
   }

   return;
}

void MainWindow::setBrewNoteByIndex(QModelIndex const & index) {
   this->pimpl->setBrewNoteByIndex(index);
   return;
}

void MainWindow::showWaterChemistryTool() {
   if (this->pimpl->m_recipeObs) {
      if (this->pimpl->m_recipeObs->mash() && this->pimpl->m_recipeObs->mash()->mashSteps().size() > 0) {
         this->pimpl->m_waterDialog->setRecipe(this->pimpl->m_recipeObs);
         this->pimpl->m_waterDialog->show();
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
