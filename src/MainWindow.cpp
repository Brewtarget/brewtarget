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
#include "editors/StyleEditor.h"
#include "editors/WaterEditor.h"
#include "editors/YeastEditor.h"
#include "listModels/EquipmentListModel.h"
#include "listModels/MashListModel.h"
#include "listModels/StyleListModel.h"
#include "listModels/WaterListModel.h"
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
#include "model/Style.h"
#include "model/Yeast.h"
#include "serialization/ImportExport.h"
#include "sortFilterProxyModels/FermentableSortFilterProxyModel.h"
#include "sortFilterProxyModels/RecipeAdditionFermentableSortFilterProxyModel.h"
#include "sortFilterProxyModels/RecipeAdditionHopSortFilterProxyModel.h"
#include "sortFilterProxyModels/RecipeAdditionMiscSortFilterProxyModel.h"
#include "sortFilterProxyModels/RecipeAdditionYeastSortFilterProxyModel.h"
#include "sortFilterProxyModels/StyleSortFilterProxyModel.h"
#include "tableModels/BoilStepTableModel.h"
#include "tableModels/FermentableTableModel.h"
#include "tableModels/FermentationStepTableModel.h"
#include "tableModels/MashStepTableModel.h"
#include "tableModels/RecipeAdditionFermentableTableModel.h"
#include "tableModels/RecipeAdditionHopTableModel.h"
#include "tableModels/RecipeAdditionMiscTableModel.h"
#include "tableModels/RecipeAdditionYeastTableModel.h"
#include "undoRedo/RelationalUndoableUpdate.h"
#include "undoRedo/UndoableAddOrRemove.h"
#include "undoRedo/UndoableAddOrRemoveList.h"
#include "utils/BtStringConst.h"
#include "utils/OptionalHelpers.h"

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
      m_undoStack{std::make_unique<QUndoStack>(&m_self)},
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
      m_styleProxyModel               {nullptr} {
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
      // Set equipment combo box model.
      m_equipmentListModel = std::make_unique<EquipmentListModel>(m_self.equipmentComboBox);
      m_self.equipmentComboBox->setModel(m_equipmentListModel.get());

      // Set the style combo box
      m_styleListModel = std::make_unique<StyleListModel>(m_self.styleComboBox);
      m_styleProxyModel = std::make_unique<StyleSortFilterProxyModel>(m_self.styleComboBox);
      m_styleProxyModel->setDynamicSortFilter(true);
      m_styleProxyModel->setSortLocaleAware(true);
      m_styleProxyModel->setSourceModel(m_styleListModel.get());
      m_styleProxyModel->sort(0);
      m_self.styleComboBox->setModel(m_styleProxyModel.get());

      // Set the mash combo box
      m_mashListModel = std::make_unique<MashListModel>(m_self.mashComboBox);
      m_self.mashComboBox->setModel(m_mashListModel.get());

      // Nothing to say.
      m_namedMashEditor = std::make_unique<NamedMashEditor>(&m_self, m_mashStepEditor.get());
      // I don't think this is used yet
      m_singleNamedMashEditor = std::make_unique<NamedMashEditor>(&m_self, m_mashStepEditor.get(), true);
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

   RecipeAdditionFermentable * selectedFermentableAddition() { return this->selected<RecipeAdditionFermentable>(m_self.fermentableAdditionTable, this->m_fermentableAdditionsTableProxy.get(), this->m_fermentableAdditionsTableModel.get()); }
   RecipeAdditionHop *         selectedHopAddition        () { return this->selected<RecipeAdditionHop        >(m_self.hopAdditionTable        , this->m_hopAdditionsTableProxy.get()        , this->m_hopAdditionsTableModel.get()); }
   RecipeAdditionMisc *        selectedMiscAddition       () { return this->selected<RecipeAdditionMisc       >(m_self.miscAdditionTable       , this->m_miscAdditionsTableProxy.get()       , this->m_miscAdditionsTableModel.get()); }
   RecipeAdditionYeast *       selectedYeastAddition      () { return this->selected<RecipeAdditionYeast      >(m_self.yeastAdditionTable      , this->m_yeastAdditionsTableProxy.get()      , this->m_yeastAdditionsTableModel.get()); }

   /**
    * \brief Use this for adding \c RecipeAdditionHop etc
    *
    * \param ra The recipe addition object - eg \c RecipeAdditionFermentable, \c RecipeAdditionHop, etc
    */
   template<class RA>
   void doRecipeAddition(std::shared_ptr<RA> ra) {
      Q_ASSERT(ra);

      this->m_self.doOrRedoUpdate(
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
         this->m_self.doOrRedoUpdate(
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

      // This ultimately gets stored in MainWindow::addStepToStepOwner() etc
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
      this->m_self.doOrRedoUpdate(
         newUndoableAddOrRemove(*stepOwner,
                                &StepClass::StepOwnerClass::removeStep,
                                step,
                                &StepClass::StepOwnerClass::addStep,
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

   void setBrewNoteByIndex(const QModelIndex &index) {

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
      QModelIndex pNdx = this->m_self.treeView_recipe->parent(index);

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
               if (this->m_self.tabWidget_recipeView->widget(i)->objectName() == "BrewNoteWidget")
                  this->m_self.tabWidget_recipeView->removeTab(i);
            }
            this->m_self.setRecipe(parent);
         }
      }

      BrewNoteWidget * ni = this->findBrewNoteWidget(bNote);
      if (!ni) {
         ni = new BrewNoteWidget(this->m_self.tabWidget_recipeView);
         ni->setBrewNote(bNote);
      }

      this->m_self.tabWidget_recipeView->addTab(ni,bNote->brewDate_short());
      this->m_self.tabWidget_recipeView->setCurrentWidget(ni);
      return;
   }

   void setBrewNote(BrewNote * bNote) {
///      QString tabname;
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

   // Undo / Redo, using the Qt Undo framework
   std::unique_ptr<QUndoStack> m_undoStack;


   Recipe * m_recipeObs = nullptr;


   // all things tables should go here.
   std::unique_ptr<BoilStepTableModel                 > m_boilStepTableModel            ;
   std::unique_ptr<FermentationStepTableModel         > m_fermentationStepTableModel    ;
   std::unique_ptr<MashStepTableModel                 > m_mashStepTableModel            ;
   std::unique_ptr<RecipeAdditionFermentableTableModel> m_fermentableAdditionsTableModel;
   std::unique_ptr<RecipeAdditionHopTableModel        > m_hopAdditionsTableModel        ;
   std::unique_ptr<RecipeAdditionMiscTableModel       > m_miscAdditionsTableModel       ;
   std::unique_ptr<RecipeAdditionYeastTableModel      > m_yeastAdditionsTableModel      ;

   // all things sort/filter proxy go here
   std::unique_ptr<RecipeAdditionFermentableSortFilterProxyModel> m_fermentableAdditionsTableProxy;
   std::unique_ptr<RecipeAdditionHopSortFilterProxyModel        > m_hopAdditionsTableProxy        ;
   std::unique_ptr<RecipeAdditionMiscSortFilterProxyModel       > m_miscAdditionsTableProxy       ;
   std::unique_ptr<RecipeAdditionYeastSortFilterProxyModel      > m_yeastAdditionsTableProxy      ;
   std::unique_ptr<StyleSortFilterProxyModel                    > m_styleProxyModel               ;

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
   std::unique_ptr<StyleCatalog          > m_styleCatalog          ;
   std::unique_ptr<StyleEditor           > m_styleEditor           ;
   std::unique_ptr<TimerMainDialog       > m_timerMainDialog       ;
   std::unique_ptr<WaterDialog           > m_waterDialog           ;
   std::unique_ptr<WaterEditor           > m_waterEditor           ;
   std::unique_ptr<YeastCatalog          > m_yeastCatalog          ;
   std::unique_ptr<YeastEditor           > m_yeastEditor           ;

   // all things lists should go here
   std::unique_ptr<EquipmentListModel> m_equipmentListModel;
   std::unique_ptr<MashListModel     > m_mashListModel     ;
   std::unique_ptr<StyleListModel    > m_styleListModel    ;
//   std::unique_ptr<WaterListModel> waterListModel;  Appears to be unused...
   std::unique_ptr<NamedMashEditor> m_namedMashEditor;
   std::unique_ptr<NamedMashEditor> m_singleNamedMashEditor;

   QString highSS, lowSS, goodSS, boldSS; // Palette replacements
///   QPrinter * printer = nullptr;

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

///   // Set up the printer
///   this->pimpl->printer = new QPrinter;
///#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
///   this->pimpl->printer->setPageSize(QPrinter::Letter);
///#else
///   this->pimpl->printer->setPageSize(QPageSize(QPageSize::Letter));
///#endif

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
   this->setupContextMenu();
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

   // I do not like this connection here.
   connect(this->pimpl->m_ancestorDialog,  &AncestorDialog::ancestoryChanged, treeView_recipe->model(), &TreeModel::versionedRecipe);
   connect(this->pimpl->m_optionDialog,    &OptionDialog::showAllAncestors,   treeView_recipe->model(), &TreeModel::catchAncestors );
   connect(this->treeView_recipe, &TreeView::recipeSpawn,          this,                     &MainWindow::versionedRecipe );

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
void MainWindow::setupShortCuts()
{
   actionNewRecipe->setShortcut(QKeySequence::New);
   actionCopy_Recipe->setShortcut(QKeySequence::Copy);
   actionDeleteSelected->setShortcut(QKeySequence::Delete);
   actionUndo->setShortcut(QKeySequence::Undo);
   actionRedo->setShortcut(QKeySequence::Redo);
}

void MainWindow::setUpStateChanges()
{
   connect( checkBox_locked, &QCheckBox::stateChanged, this, &MainWindow::lockRecipe );
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
   setUndoRedoEnable();
   connect(actionEquipments                , &QAction::triggered, this->pimpl->m_equipCatalog.get()         , &QWidget::show                     ); // > View > Equipments
   connect(actionMashs                     , &QAction::triggered, this->pimpl->m_namedMashEditor.get()      , &QWidget::show                     ); // > View > Mashs
   connect(actionStyles                    , &QAction::triggered, this->pimpl->m_styleCatalog.get()         , &QWidget::show                     ); // > View > Styles
   connect(actionFermentables              , &QAction::triggered, this->pimpl->m_fermCatalog.get()          , &QWidget::show                     ); // > View > Fermentables
   connect(actionHops                      , &QAction::triggered, this->pimpl->m_hopCatalog.get()           , &QWidget::show                     ); // > View > Hops
   connect(actionMiscs                     , &QAction::triggered, this->pimpl->m_miscCatalog.get()          , &QWidget::show                     ); // > View > Miscs
   connect(actionYeasts                    , &QAction::triggered, this->pimpl->m_yeastCatalog.get()         , &QWidget::show                     ); // > View > Yeasts
   connect(actionOptions                   , &QAction::triggered, this->pimpl->m_optionDialog.get()         , &OptionDialog::show                ); // > Tools > Options
//   connect( actionManual, &QAction::triggered, this, &MainWindow::openManual);                                               // > About > Manual
   connect(actionScale_Recipe              , &QAction::triggered, this->pimpl->m_recipeScaler.get()         , &QWidget::show                     ); // > Tools > Scale Recipe
   connect(action_recipeToTextClipboard    , &QAction::triggered, this->pimpl->m_recipeFormatter.get()      , &RecipeFormatter::toTextClipboard  ); // > Tools > Recipe to Clipboard as Text
   connect(actionConvert_Units             , &QAction::triggered, this->pimpl->m_converterTool.get()        , &QWidget::show                     ); // > Tools > Convert Units
   connect(actionHydrometer_Temp_Adjustment, &QAction::triggered, this->pimpl->m_hydrometerTool.get()       , &QWidget::show                     ); // > Tools > Hydrometer Temp Adjustment
   connect(actionAlcohol_Percentage_Tool   , &QAction::triggered, this->pimpl->m_alcoholTool.get()          , &QWidget::show                     ); // > Tools > Alcohol
   connect(actionOG_Correction_Help        , &QAction::triggered, this->pimpl->m_ogAdjuster.get()           , &QWidget::show                     ); // > Tools > OG Correction Help
   connect(actionCopy_Recipe               , &QAction::triggered, this                                      , &MainWindow::copyRecipe            ); // > File > Copy Recipe
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
   connect(this->mashButton               , &QAbstractButton::clicked, this->pimpl->m_mashEditor        , &MashEditor::showEditor);
   // TODO: Make these buttons!
//   connect(this->boilButton               , &QAbstractButton::clicked, this->pimpl->m_boilEditor        , &BoilEditor::showEditor);
//   connect(this->fermentationButton       , &QAbstractButton::clicked, this->pimpl->m_fermentationEditor, &FermentationEditor::showEditor);
   connect(this->pushButton_addFerm       , &QAbstractButton::clicked, this->pimpl->m_fermCatalog , &QWidget::show         );
   connect(this->pushButton_addHop        , &QAbstractButton::clicked, this->pimpl->m_hopCatalog  , &QWidget::show         );
   connect(this->pushButton_addMisc       , &QAbstractButton::clicked, this->pimpl->m_miscCatalog , &QWidget::show         );
   connect(this->pushButton_addYeast      , &QAbstractButton::clicked, this->pimpl->m_yeastCatalog, &QWidget::show         );
   connect(this->pushButton_removeFerm    , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedFermentableAddition);
   connect(this->pushButton_removeHop     , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedHopAddition        );
   connect(this->pushButton_removeMisc    , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedMiscAddition       );
   connect(this->pushButton_removeYeast   , &QAbstractButton::clicked, this                       , &MainWindow::removeSelectedYeastAddition      );
   connect(this->pushButton_editFerm      , &QAbstractButton::clicked, this                       , &MainWindow::editFermentableOfSelectedFermentableAddition);
   connect(this->pushButton_editMisc      , &QAbstractButton::clicked, this                       , &MainWindow::editMiscOfSelectedMiscAddition              );
   connect(this->pushButton_editHop       , &QAbstractButton::clicked, this                       , &MainWindow::editHopOfSelectedHopAddition                );
   connect(this->pushButton_editYeast     , &QAbstractButton::clicked, this                       , &MainWindow::editYeastOfSelectedYeastAddition            );

   connect(this->pushButton_editMash      , &QAbstractButton::clicked, this->pimpl->m_mashEditor, &MashEditor::showEditor                   );
   connect(this->pushButton_addMashStep   , &QAbstractButton::clicked, this                     , &MainWindow::addMashStep              );
   connect(this->pushButton_removeMashStep, &QAbstractButton::clicked, this                     , &MainWindow::removeSelectedMashStep   );
   connect(this->pushButton_editMashStep  , &QAbstractButton::clicked, this                     , &MainWindow::editSelectedMashStep     );
   connect(this->pushButton_mashWizard    , &QAbstractButton::clicked, this->pimpl->m_mashWizard, &MashWizard::show      );
   connect(this->pushButton_saveMash      , &QAbstractButton::clicked, this                     , &MainWindow::saveMash                 );
   connect(this->pushButton_mashDes       , &QAbstractButton::clicked, this->pimpl->m_mashDesigner, &MashDesigner::show    );
   connect(this->pushButton_mashUp        , &QAbstractButton::clicked, this                       , &MainWindow::moveSelectedMashStepUp   );
   connect(this->pushButton_mashDown      , &QAbstractButton::clicked, this                       , &MainWindow::moveSelectedMashStepDown );
   connect(this->pushButton_mashRemove    , &QAbstractButton::clicked, this                       , &MainWindow::removeMash               );

   connect(this->pushButton_editBoil      , &QAbstractButton::clicked, this->pimpl->m_boilEditor, &BoilEditor::showEditor                   );
   connect(this->pushButton_addBoilStep   , &QAbstractButton::clicked, this                     , &MainWindow::addBoilStep              );
   connect(this->pushButton_removeBoilStep, &QAbstractButton::clicked, this                     , &MainWindow::removeSelectedBoilStep   );
   connect(this->pushButton_editBoilStep  , &QAbstractButton::clicked, this                     , &MainWindow::editSelectedBoilStep     );
//   connect(this->pushButton_saveBoil      , &QAbstractButton::clicked, this        , &MainWindow::saveBoil                 ); TODO!
   connect(this->pushButton_boilUp        , &QAbstractButton::clicked, this        , &MainWindow::moveSelectedBoilStepUp   );
   connect(this->pushButton_boilDown      , &QAbstractButton::clicked, this        , &MainWindow::moveSelectedBoilStepDown );
//   connect(this->pushButton_boilRemove    , &QAbstractButton::clicked, this        , &MainWindow::removeBoil               ); TODO!

   connect(this->pushButton_editFermentation      , &QAbstractButton::clicked, this->pimpl->m_fermentationEditor, &FermentationEditor::showEditor);
   connect(this->pushButton_addFermentationStep   , &QAbstractButton::clicked, this, &MainWindow::addFermentationStep              );
   connect(this->pushButton_removeFermentationStep, &QAbstractButton::clicked, this, &MainWindow::removeSelectedFermentationStep   );
   connect(this->pushButton_editFermentationStep  , &QAbstractButton::clicked, this, &MainWindow::editSelectedFermentationStep     );
//   connect(this->pushButton_saveFermentation      , &QAbstractButton::clicked, this, &MainWindow::saveFermentation                 ); TODO!
   connect(this->pushButton_fermentationUp        , &QAbstractButton::clicked, this, &MainWindow::moveSelectedFermentationStepUp   );
   connect(this->pushButton_fermentationDown      , &QAbstractButton::clicked, this, &MainWindow::moveSelectedFermentationStepDown );
//   connect(this->pushButton_fermentationRemove    , &QAbstractButton::clicked, this, &MainWindow::removeFermentation               ); TODO!

   return;
}

// comboBoxes with a SIGNAL of activated() should go in here.
void MainWindow::setupActivate() {
   connect(this->equipmentComboBox, QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeEquipment);
   connect(this->styleComboBox,     QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeStyle);
   connect(this->mashComboBox,      QOverload<int>::of(&QComboBox::activated), this, &MainWindow::updateRecipeMash);
   return;
}

// lineEdits with either an editingFinished() or a textModified() should go in
// here
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
   return;
}

void MainWindow::deleteSelected() {
   QModelIndexList selected;
   TreeView* active = qobject_cast<TreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   // This happens after startup when nothing is selected
   if (!active) {
      qDebug() << Q_FUNC_INFO << "Nothing selected, so nothing to delete";
      return;
   }

   QModelIndex start = active->selectionModel()->selectedRows().first();
   qDebug() << Q_FUNC_INFO << "Delete starting from row" << start.row();
   active->deleteSelected(active->selectionModel()->selectedRows());

   //
   // Now that we deleted the selected recipe, we don't want it to appear in the main window any more, so let's select
   // another one.
   //
   // Most of the time, after deleting the nth recipe, the new nth item is also a recipe.  If there isn't an nth item
   // (eg because the recipe(s) we deleted were at the end of the list) then let's go back to the 1st item.  But then
   // we have to make sure to skip over folders.
   //
   // .:TBD:. This works if you have plenty of recipes outside folders.  If all your recipes are inside folders, then
   // we should so a proper search through the tree to find the first recipe and then expand the folder that it's in.
   // Doesn't feel like that logic belongs here.  Would be better to create TreeView::firstNonFolder() or similar.
   //
   if (!start.isValid() || !active->type(start)) {
      int oldRow = start.row();
      start = active->first();
      qDebug() << Q_FUNC_INFO << "Row" << oldRow << "no longer valid, so returning to first (" << start.row() << ")";
   }

   while (start.isValid() && active->type(start) == TreeNode::Type::Folder) {
      qDebug() << Q_FUNC_INFO << "Skipping over folder at row" << start.row();
      // Once all platforms are on Qt 5.11 or later, we can write:
      // start = start.siblingAtRow(start.row() + 1);
      start = start.sibling(start.row() + 1, start.column());
   }

   if (start.isValid()) {
      qDebug() << Q_FUNC_INFO << "Row" << start.row() << "is" << active->type(start);
      if (active->type(start) == TreeNode::Type::Recipe) {
         this->setRecipe(treeView_recipe->getItem<Recipe>(start));
      }
      this->setTreeSelection(start);
   }

   return;
}

void MainWindow::treeActivated(const QModelIndex &index) {
   QObject* calledBy = sender();
   // Not sure how this could happen, but better safe the sigsegv'd
   if (!calledBy) {
      return;
   }

   TreeView* active = qobject_cast<TreeView*>(calledBy);
   // If the sender cannot be morphed into a TreeView object
   if (!active) {
      qWarning() << Q_FUNC_INFO << "Unrecognised sender" << calledBy->metaObject()->className();
      return;
   }

   auto nodeType = active->type(index);
   if (!nodeType) {
      qWarning() << Q_FUNC_INFO << "Unknown type for index" << index;
   } else {
      switch (*nodeType) {
         case TreeNode::Type::Recipe:
            setRecipe(treeView_recipe->getItem<Recipe>(index));
            break;
         case TreeNode::Type::Equipment:
            {
               Equipment * kit = active->getItem<Equipment>(index);
               if ( kit ) {
                  this->pimpl->m_equipEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(kit));
                  this->pimpl->m_equipEditor->show();
               }
            }
            break;
         case TreeNode::Type::Fermentable:
            {
               Fermentable * ferm = active->getItem<Fermentable>(index);
               if (ferm) {
                  this->pimpl->m_fermentableEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(ferm));
                  this->pimpl->m_fermentableEditor->show();
               }
            }
            break;
         case TreeNode::Type::Hop:
            {
               Hop* hop = active->getItem<Hop>(index);
               if (hop) {
                  this->pimpl->m_hopEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(hop));
                  this->pimpl->m_hopEditor->show();
               }
            }
            break;
         case TreeNode::Type::Misc:
            {
               Misc * misc = active->getItem<Misc>(index);
               if (misc) {
                  this->pimpl->m_miscEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(misc));
                  this->pimpl->m_miscEditor->show();
               }
            }
            break;
         case TreeNode::Type::Style:
            {
               Style * style = active->getItem<Style>(index);
               if (style) {
                  this->pimpl->m_styleEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(style));
                  this->pimpl->m_styleEditor->show();
               }
            }
            break;
         case TreeNode::Type::Yeast:
            {
               Yeast * yeast = active->getItem<Yeast>(index);
               if (yeast) {
                  this->pimpl->m_yeastEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(yeast));
                  this->pimpl->m_yeastEditor->show();
               }
            }
            break;
         case TreeNode::Type::BrewNote:
            this->pimpl->setBrewNoteByIndex(index);
            break;
         case TreeNode::Type::Folder:  // default behavior is fine, but no warning
            break;
         case TreeNode::Type::Water:
            {
               Water * w = active->getItem<Water>(index);
               if (w) {
                  this->pimpl->m_waterEditor->setEditItem(ObjectStoreWrapper::getSharedFromRaw(w));
                  this->pimpl->m_waterEditor->show();
               }
            }
            break;
      }
   }
   treeView_recipe->setCurrentIndex(index);
   return;
}

void MainWindow::setAncestor()
{
   Recipe* rec;
   if ( this->pimpl->m_recipeObs ) {
      rec = this->pimpl->m_recipeObs;
   } else {
      QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
      rec = treeView_recipe->getItem<Recipe>(indexes[0]);
   }

   this->pimpl->m_ancestorDialog->setAncestor(rec);
   this->pimpl->m_ancestorDialog->show();
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
   this->pimpl->m_hopAdditionsTableModel->observeRecipe(recipe);
   this->pimpl->m_miscAdditionsTableModel->observeRecipe(recipe);
   this->pimpl->m_yeastAdditionsTableModel->observeRecipe(recipe);
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
   this->pimpl->m_equipmentListModel->observeRecipe(recipe);
   this->pimpl->m_recipeFormatter->setRecipe(recipe);
   this->pimpl->m_ogAdjuster->setRecipe(recipe);
   recipeExtrasWidget->setRecipe(recipe);
   this->pimpl->m_mashDesigner->setRecipe(recipe);
   equipmentButton->setRecipe(recipe);
   if (recipe->equipment()) {
      this->pimpl->m_equipEditor->setEditItem(recipe->equipment());
   }
   styleButton->setRecipe(recipe);
   if (recipe->style()) {
      this->pimpl->m_styleEditor->setEditItem(recipe->style());
   }

   this->pimpl->m_mashEditor->setMash(this->pimpl->m_recipeObs->mash());
   this->pimpl->m_mashEditor->setRecipe(this->pimpl->m_recipeObs);

   this->pimpl->m_boilEditor->setEditItem(this->pimpl->m_recipeObs->boil());
   this->pimpl->m_boilEditor->setRecipe(this->pimpl->m_recipeObs);

   this->pimpl->m_fermentationEditor->setEditItem(this->pimpl->m_recipeObs->fermentation());
   this->pimpl->m_fermentationEditor->setRecipe(this->pimpl->m_recipeObs);

   mashButton->setMash(this->pimpl->m_recipeObs->mash());
   this->pimpl->m_recipeScaler->setRecipe(this->pimpl->m_recipeObs);

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
void MainWindow::lockRecipe(int state) {
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

   this->pimpl->m_fermCatalog ->setEnableAddToRecipe(enabled);
   this->pimpl->m_hopCatalog  ->setEnableAddToRecipe(enabled);
   this->pimpl->m_miscCatalog ->setEnableAddToRecipe(enabled);
   this->pimpl->m_yeastCatalog->setEnableAddToRecipe(enabled);
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
   this->lineEdit_boilTime  ->setQuantity(this->pimpl->m_recipeObs->boil()->boilTime_mins());
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

   this->doOrRedoUpdate(*this->pimpl->m_recipeObs,
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
   if (!style) {
      return;
   }

   this->styleRangeWidget_og->setPreferredRange(this->oGLabel->getRangeToDisplay(style->ogMin(), style->ogMax()));

   this->styleRangeWidget_fg->setPreferredRange(this->fGLabel->getRangeToDisplay(style->ogMin(), style->ogMax()));

   // If min and/or max ABV is not set on the Style, then use some sensible outer limit(s)
   this->styleRangeWidget_abv->setPreferredRange(style->abvMin_pct().value_or(0.0), style->abvMax_pct().value_or(50.0));
   this->styleRangeWidget_ibu->setPreferredRange(style->ibuMin(), style->ibuMax());
   this->styleRangeWidget_srm->setPreferredRange(this->colorSRMLabel->getRangeToDisplay(style->colorMin_srm(),
                                                                                        style->colorMax_srm()));
   this->styleButton->setStyle(style);

   return;
}

//
// TODO: Would be good to harmonise how these updatRecipeFoo and dropRecipeFoo functions work
//

void MainWindow::updateRecipeStyle() {
   if (this->pimpl->m_recipeObs == nullptr) {
      return;
   }

   QModelIndex proxyIndex( this->pimpl->m_styleProxyModel->index(styleComboBox->currentIndex(),0) );
   QModelIndex sourceIndex( this->pimpl->m_styleProxyModel->mapToSource(proxyIndex) );
   auto selected = ObjectStoreWrapper::getSharedFromRaw(this->pimpl->m_styleListModel->at(sourceIndex.row()));
   if (selected) {
      this->doOrRedoUpdate(
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
   if (this->pimpl->m_recipeObs == nullptr) {
      return;
   }

   auto selectedMash = ObjectStoreWrapper::getSharedFromRaw(
      this->pimpl->m_mashListModel->at(mashComboBox->currentIndex())
   );
   if (selectedMash) {
      // The Recipe will decide whether it needs to make a copy of the Mash, hence why we don't reuse "selectedMash" below
      this->pimpl->m_recipeObs->setMash(selectedMash);
      this->pimpl->m_mashEditor->setMash(this->pimpl->m_recipeObs->mash());
      mashButton->setMash(this->pimpl->m_recipeObs->mash());
   }
   return;
}

void MainWindow::updateRecipeEquipment() {
  droppedRecipeEquipment(this->pimpl->m_equipmentListModel->at(equipmentComboBox->currentIndex()));
  return;
}

void MainWindow::updateEquipmentButton() {
   if (this->pimpl->m_recipeObs != nullptr) {
      this->equipmentButton->setEquipment(this->pimpl->m_recipeObs->equipment());
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
                                                      &MainWindow::updateEquipmentButton,
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
   this->doOrRedoUpdate(equipmentUpdate);
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
   this->doOrRedoUpdate(
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

   this->doOrRedoUpdate(
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

   this->doOrRedoUpdate(
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

   this->doOrRedoUpdate(
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

   this->doOrRedoUpdate(
      newUndoableAddOrRemoveList(*this->pimpl->m_recipeObs,
                                 &Recipe::addAddition<RecipeAdditionYeast>,
                                 yeastAdditions,
                                 &Recipe::removeAddition<RecipeAdditionYeast>,
                                 tr("Drop yeast(s) on a recipe"))
   );
   return;
}

void MainWindow::updateRecipeBatchSize() {
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   this->doOrRedoUpdate(*this->pimpl->m_recipeObs,
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
///   this->doOrRedoUpdate(*boil,
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
///      this->doOrRedoUpdate(*kit, TYPE_INFO(Equipment, boilTime_min), boilTime, tr("Change Boil Time"));
///   } else {
///      auto boil = this->pimpl->m_recipeObs->nonOptBoil();
///      this->doOrRedoUpdate(*boil, TYPE_INFO(Boil, boilTime_mins), boilTime, tr("Change Boil Time"));
///   }
///
///   return;
///}

void MainWindow::updateRecipeEfficiency() {
   qDebug() << Q_FUNC_INFO << lineEdit_efficiency->getNonOptValue<double>();
   if (!this->pimpl->m_recipeObs) {
      return;
   }

   this->doOrRedoUpdate(*this->pimpl->m_recipeObs,
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


template<class StepOwnerClass, class StepClass>
void MainWindow::addStepToStepOwner(StepOwnerClass & stepOwner, std::shared_ptr<StepClass> step) {
   qDebug() << Q_FUNC_INFO;
   //
   // Mash/Boil/Fermentation Steps are a bit different from most other NamedEntity objects in that they don't really
   // have an independent existence.  Taking Mash as an example, if you ask a Mash to remove a MashStep then it will
   // also tell the ObjectStore to delete it, but, when we're adding a MashStep to a Mash it's easier (for eg the
   // implementation of undo/redo) if we add it to the ObjectStore before we call Mash::addMashStep().
   //
   // However, normally, at this point, the new step will already have been added to the DB by
   // EditorBase::doSaveAndClose.  So we are just belt-and-braces here checking whether it needs to be added.
   //
   if (step->key() < 0) {
      qWarning() << Q_FUNC_INFO << step->metaObject()->className() << "unexpectedly not in DB, so inserting it now.";
      ObjectStoreWrapper::insert(step);
   }
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(stepOwner,
                             &StepOwnerClass::addStep,
                             step,
                             &StepOwnerClass::removeStep,
                             tr("Add %1 step to recipe").arg(StepOwnerClass::localisedName()))
   );
   // We don't need to call this->pimpl->m_mashStepTableModel->addMashStep(mashStep) etc here because the change to
   // the mash/boil/ferementation will already have triggered the necessary updates to
   // this->pimpl->m_mashStepTableModel/this->pimpl->m_boilStepTableModel/etc.
   return;
}

template<class StepOwnerClass, class StepClass>
void MainWindow::addStepToStepOwner(std::shared_ptr<StepOwnerClass> stepOwner, std::shared_ptr<StepClass> step) {
   this->addStepToStepOwner(*stepOwner, step);
}
void MainWindow::addStepToStepOwner(std::shared_ptr<MashStep> mashStep) {
   this->addStepToStepOwner(this->pimpl->m_recipeObs->mash(), mashStep);
   return;
}
void MainWindow::addStepToStepOwner(std::shared_ptr<BoilStep> boilStep) {
   this->addStepToStepOwner(this->pimpl->m_recipeObs->boil(), boilStep);
   return;
}
void MainWindow::addStepToStepOwner(std::shared_ptr<FermentationStep> fermentationStep) {
   this->addStepToStepOwner(this->pimpl->m_recipeObs->fermentation(), fermentationStep);
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
   Q_ASSERT(this->pimpl->m_undoStack != 0);
   actionUndo->setEnabled(this->pimpl->m_undoStack->canUndo());
   actionRedo->setEnabled(this->pimpl->m_undoStack->canRedo());

   actionUndo->setText(QString(tr("Undo %1").arg(this->pimpl->m_undoStack->undoText())));
   actionRedo->setText(QString(tr("Redo %1").arg(this->pimpl->m_undoStack->redoText())));

   return;
}

void MainWindow::doOrRedoUpdate(QUndoCommand * update) {
   Q_ASSERT(this->pimpl->m_undoStack != nullptr);
   Q_ASSERT(update != nullptr);
   this->pimpl->m_undoStack->push(update);
   this->setUndoRedoEnable();
   return;
}

// For undo/redo, we use Qt's Undo framework
void MainWindow::editUndo() {
   Q_ASSERT(this->pimpl->m_undoStack != 0);
   if ( !this->pimpl->m_undoStack->canUndo() ) {
      qDebug() << "Undo called but nothing to undo";
   } else {
      this->pimpl->m_undoStack->undo();
   }

   setUndoRedoEnable();
   return;
}

void MainWindow::editRedo() {
   Q_ASSERT(this->pimpl->m_undoStack != 0);
   if ( !this->pimpl->m_undoStack->canRedo() ) {
      qDebug() << "Redo called but nothing to redo";
   } else {
      this->pimpl->m_undoStack->redo();
   }

   setUndoRedoEnable();
   return;
}

template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionHop        > itemToRemove) { this->pimpl->        m_hopAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionFermentable> itemToRemove) { this->pimpl->m_fermentableAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionMisc       > itemToRemove) { this->pimpl->       m_miscAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<RecipeAdditionYeast      > itemToRemove) { this->pimpl->      m_yeastAdditionsTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<MashStep                 > itemToRemove) { this->pimpl->            m_mashStepTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<BoilStep                 > itemToRemove) { this->pimpl->            m_boilStepTableModel->remove(itemToRemove); return; }
template<> void MainWindow::remove(std::shared_ptr<FermentationStep         > itemToRemove) { this->pimpl->    m_fermentationStepTableModel->remove(itemToRemove); return; }

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
}

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

void MainWindow::newRecipe() {
   QString const name = QInputDialog::getText(this, tr("Recipe name"), tr("Recipe name:"));
   if (name.isEmpty()) {
      return;
   }

   std::shared_ptr<Recipe> newRec = std::make_shared<Recipe>(name);

   // bad things happened -- let somebody know
   if (!newRec) {
      QMessageBox::warning(this,
                           tr("Error creating recipe"),
                           tr("An error was returned while creating %1").arg(name));
      return;
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

   std::shared_ptr<Fermentation> newFermentation = std::make_shared<Fermentation>(tr("Automatically-created Fermentation for %1").arg(name));
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

   // A new recipe will be put in a folder if you right click on a recipe or
   // folder. Otherwise, it goes into the main window?
   QObject* selection = this->sender();
   if (selection) {
      TreeView* sent = qobject_cast<TreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
      if (sent) {
         QModelIndexList indexes = sent->selectionModel()->selectedRows();
         // This is a little weird. There is an edge case where nothing is
         // selected and you click the big blue + button.
         if (indexes.size() > 0) {
            if (sent->type(indexes.at(0)) == TreeNode::Type::Recipe) {
               auto foo = sent->getItem<Recipe>(indexes.at(0));
               if (foo && ! foo->folder().isEmpty()) {
                  newRec->setFolder( foo->folder() );
               }
            } else if (sent->type(indexes.at(0)) == TreeNode::Type::Folder) {
               Folder* foo = sent->getItem<Folder>(indexes.at(0));
               if (foo) {
                  newRec->setFolder(foo->fullPath());
               }
            }
         }
      }
   }
   this->setTreeSelection(treeView_recipe->findElement(newRec.get()));
   this->setRecipe(newRec.get());
   return;
}

void MainWindow::newFolder() {
   // get the currently active tree
   TreeView* active = qobject_cast<TreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   if (!active) {
      return;
   }

   QModelIndexList indexes = active->selectionModel()->selectedRows();
   QModelIndex starter = indexes.at(0);

   // Where to start from
   QString dPath = active->folderName(starter);

   QString name = QInputDialog::getText(this, tr("Folder name"), tr("Folder name:"), QLineEdit::Normal, dPath);
   // User clicks cancel
   if (name.isEmpty())
      return;
   // Do some input validation here.

   // Nice little builtin to collapse leading and following white space
   name = name.simplified();
   if ( name.isEmpty() ) {
      QMessageBox::critical( this, tr("Bad Name"),
                             tr("A folder name must have at least one non-whitespace character in it"));
      return;
   }

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   QString::SplitBehavior skip = QString::SkipEmptyParts;
#else
   Qt::SplitBehaviorFlags skip = Qt::SkipEmptyParts;
#endif
   if ( name.split("/", skip).isEmpty() ) {
      QMessageBox::critical( this, tr("Bad Name"), tr("A folder name must have at least one non-/ character in it"));
      return;
   }
   active->addFolder(name);
}

void MainWindow::renameFolder() {
   TreeView* active = qobject_cast<TreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   // If the sender cannot be morphed into a TreeView object
   if ( active == nullptr ) {
      return;
   }

   // I don't think I can figure out what the behavior will be if you select
   // many items
   QModelIndexList indexes = active->selectionModel()->selectedRows();
   QModelIndex starter = indexes.at(0);

   // The item to be renamed
   // Don't rename anything other than a folder
   if ( active->type(starter) != TreeNode::Type::Folder) {
      return;
   }

   Folder* victim = active->getItem<Folder>(starter);
   QString newName = QInputDialog::getText(this,
                                           tr("Folder name"),
                                           tr("Folder name:"),
                                           QLineEdit::Normal,
                                           victim->name());

   // User clicks cancel
   if (newName.isEmpty()) {
      return;
   }
   // Do some input validation here.

   // Nice little builtin to collapse leading and following white space
   newName = newName.simplified();
   if (newName.isEmpty()) {
      QMessageBox::critical( this, tr("Bad Name"),
                             tr("A folder name must have at least one non-whitespace character in it"));
      return;
   }

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   QString::SplitBehavior skip = QString::SkipEmptyParts;
#else
   Qt::SplitBehaviorFlags skip = Qt::SkipEmptyParts;
#endif
   if ( newName.split("/", skip).isEmpty() ) {
      QMessageBox::critical( this, tr("Bad Name"), tr("A folder name must have at least one non-/ character in it"));
      return;
   }
   newName = victim->path() % "/" % newName;

   // Delgate this work to the tree.
   active->renameFolder(victim,newName);
}

void MainWindow::setTreeSelection(QModelIndex item) {
   qDebug() << Q_FUNC_INFO;

   if (! item.isValid()) {
      qDebug() << Q_FUNC_INFO << "Item not valid";
      return;
   }

   TreeView *active = qobject_cast<TreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
   if ( active == nullptr ) {
      active = qobject_cast<TreeView*>(treeView_recipe);
   }

   // Couldn't cast the active item to a TreeView
   if ( active == nullptr ) {
      qDebug() << Q_FUNC_INFO << "Couldn't cast the active item to a TreeView";
      return;
   }

   QModelIndex parent = active->parent(item);

   active->setCurrentIndex(item);
   if ( active->type(parent) == TreeNode::Type::Folder && ! active->isExpanded(parent) ) {
      active->setExpanded(parent, true);
   }
   active->scrollTo(item,QAbstractItemView::PositionAtCenter);
   return;
}

// reduces the inventory by the selected recipes
void MainWindow::reduceInventory() {

   for (QModelIndex selected : treeView_recipe->selectionModel()->selectedRows()) {
      Recipe* rec = treeView_recipe->getItem<Recipe>(selected);
      if (rec == nullptr) {
         // Try the parent recipe
         rec = treeView_recipe->getItem<Recipe>(treeView_recipe->parent(selected));
         if (rec == nullptr) {
            continue;
         }
      }

      // Make sure everything is properly set and selected
      if (rec != this->pimpl->m_recipeObs) {
         setRecipe(rec);
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
   QModelIndex bIndex;

   for (QModelIndex selected : indexes) {
      Recipe*   rec   = treeView_recipe->getItem<Recipe>(selected);
      if (!rec) {
         continue;
      }

      // Make sure everything is properly set and selected
      if (rec != this->pimpl->m_recipeObs) {
         setRecipe(rec);
      }

      auto bNote = std::make_shared<BrewNote>(*rec);
      bNote->populateNote(rec);
      bNote->setBrewDate();
      ObjectStoreWrapper::insert(bNote);

      this->pimpl->setBrewNote(bNote.get());

      bIndex = treeView_recipe->findElement(bNote.get());
      if (bIndex.isValid()) {
         setTreeSelection(bIndex);
      }
   }
   return;
}

void MainWindow::reBrewNote() {
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
   for (QModelIndex selected : indexes) {
      BrewNote* old   = treeView_recipe->getItem<BrewNote>(selected);
      Recipe* rec     = treeView_recipe->getItem<Recipe>(treeView_recipe->parent(selected));

      if (! old || ! rec) {
         return;
      }

      auto bNote = std::make_shared<BrewNote>(*old);
      bNote->setBrewDate();
      ObjectStoreWrapper::insert(bNote);

      if (rec != this->pimpl->m_recipeObs) {
         setRecipe(rec);
      }

      this->pimpl->setBrewNote(bNote.get());

      setTreeSelection(treeView_recipe->findElement(bNote.get()));
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

// Imports all the recipes, hops, equipment or whatever from a BeerXML file into the database.
void MainWindow::importFiles() {
   ImportExport::importFromFiles();
   return;
}

///bool MainWindow::verifyImport(QString tag, QString name) {
///   return QMessageBox::question(this, tr("Import %1?").arg(tag), tr("Import %1?").arg(name),
///                                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
///}

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

void MainWindow::copyRecipe() {
   QString name = QInputDialog::getText( this, tr("Copy Recipe"), tr("Enter a unique name for the copy.") );
   if (name.isEmpty()) {
      return;
   }

   auto newRec = std::make_shared<Recipe>(*this->pimpl->m_recipeObs); // Create a deep copy
   newRec->setName(name);
   ObjectStoreTyped<Recipe>::getInstance().insert(newRec);
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

// We build the menus at start up time.  This just needs to exec the proper
// menu.
void MainWindow::contextMenu(const QPoint &point) {
   QObject* calledBy = sender();
   // Not sure how this could happen, but better safe the sigsegv'd
   if (calledBy == nullptr) {
      return;
   }

   TreeView * active = qobject_cast<TreeView*>(calledBy);
   // If the sender cannot be morphed into a TreeView object
   if (active == nullptr) {
      return;
   }

   QModelIndex selected = active->indexAt(point);
   if (!selected.isValid()) {
      return;
   }

   QMenu * tempMenu = active->contextMenu(selected);
   if (tempMenu) {
      tempMenu->exec(active->mapToGlobal(point));
   }
   return;
}

void MainWindow::setupContextMenu() {

   this->treeView_recipe->setupContextMenu(this, this);
   this->treeView_style ->setupContextMenu(this, this->pimpl->m_styleEditor      .get());
   this->treeView_equip ->setupContextMenu(this, this->pimpl->m_equipEditor      .get());
   this->treeView_ferm  ->setupContextMenu(this, this->pimpl->m_fermentableEditor.get());
   this->treeView_hops  ->setupContextMenu(this, this->pimpl->m_hopEditor        .get());
   this->treeView_misc  ->setupContextMenu(this, this->pimpl->m_miscEditor       .get());
   this->treeView_yeast ->setupContextMenu(this, this->pimpl->m_yeastEditor      .get());
   this->treeView_water ->setupContextMenu(this, this->pimpl->m_waterEditor      .get());

   // TreeView for clicks, both double and right
   connect(treeView_recipe, &QAbstractItemView::doubleClicked   , this, &MainWindow::treeActivated);
   connect(treeView_recipe, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu  );
   connect(treeView_style , &QAbstractItemView::doubleClicked   , this, &MainWindow::treeActivated);
   connect(treeView_style , &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu  );
   connect(treeView_equip , &QAbstractItemView::doubleClicked   , this, &MainWindow::treeActivated);
   connect(treeView_equip , &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu  );
   connect(treeView_ferm  , &QAbstractItemView::doubleClicked   , this, &MainWindow::treeActivated);
   connect(treeView_ferm  , &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu  );
   connect(treeView_hops  , &QAbstractItemView::doubleClicked   , this, &MainWindow::treeActivated);
   connect(treeView_hops  , &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu  );
   connect(treeView_misc  , &QAbstractItemView::doubleClicked   , this, &MainWindow::treeActivated);
   connect(treeView_misc  , &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu  );
   connect(treeView_yeast , &QAbstractItemView::doubleClicked   , this, &MainWindow::treeActivated);
   connect(treeView_yeast , &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu  );
   connect(treeView_water , &QAbstractItemView::doubleClicked   , this, &MainWindow::treeActivated);
   connect(treeView_water , &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu  );
   return;
}

void MainWindow::copySelected() {
///   QModelIndexList selected;
   TreeView* active = qobject_cast<TreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
   active->copySelected(active->selectionModel()->selectedRows());
   return;
}

void MainWindow::exportSelected() {
   TreeView const * active = qobject_cast<TreeView*>(this->tabWidget_Trees->currentWidget()->focusWidget());
   if (active == nullptr) {
      qDebug() << Q_FUNC_INFO << "No active tree so can't get a selection";
      return;
   }

   QModelIndexList selected = active->selectionModel()->selectedRows();
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
      auto nodeType = active->type(selection);
      if (!nodeType) {
         qWarning() << Q_FUNC_INFO << "Unknown type for selection" << selection;
      } else {
         switch(*nodeType) {
            case TreeNode::Type::Recipe:
               recipes.append(treeView_recipe->getItem<Recipe>(selection));
               ++count;
               break;
            case TreeNode::Type::Equipment:
               equipments.append(treeView_equip->getItem<Equipment>(selection));
               ++count;
               break;
            case TreeNode::Type::Fermentable:
               fermentables.append(treeView_ferm->getItem<Fermentable>(selection));
               ++count;
               break;
            case TreeNode::Type::Hop:
               hops.append(treeView_hops->getItem<Hop>(selection));
               ++count;
               break;
            case TreeNode::Type::Misc:
               miscs.append(treeView_misc->getItem<Misc>(selection));
               ++count;
               break;
            case TreeNode::Type::Style:
               styles.append(treeView_style->getItem<Style>(selection));
               ++count;
               break;
            case TreeNode::Type::Water:
               waters.append(treeView_water->getItem<Water>(selection));
               ++count;
               break;
            case TreeNode::Type::Yeast:
               yeasts.append(treeView_yeast->getItem<Yeast>(selection));
               ++count;
               break;
            case TreeNode::Type::Folder:
               qDebug() << Q_FUNC_INFO << "Can't export selected Folder to XML as BeerXML does not support it";
               break;
            case TreeNode::Type::BrewNote:
               qDebug() << Q_FUNC_INFO << "Can't export selected BrewNote to XML as BeerXML does not support it";
               break;
            default:
               // This shouldn't happen, because we should explicitly cover all the types above
               qWarning() << Q_FUNC_INFO << "Don't know how to export TreeNode type" << static_cast<int>(*nodeType);
               break;
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
         BrewNoteWidget* ni = this->pimpl->findBrewNoteWidget(target);
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

      auto noteParent = treeView_recipe->getItem<Recipe>( treeView_recipe->parent(selected));

      if ( ! noteParent ) {
         continue;
      }

      target->recalculateEff(noteParent);
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
   QModelIndex ndx = treeView_recipe->findElement(descendant);
   setRecipe(descendant);
   treeView_recipe->setCurrentIndex(ndx);
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
