/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * catalogs/CatalogBase.h is part of Brewtarget, and is copyright the following authors 2023-2026:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef CATALOGS_CATALOGBASE_H
#define CATALOGS_CATALOGBASE_H
#pragma once

#include <tuple>
#include <utility>

#include <QHBoxLayout>
#include <QList>
#include <QMetaObject>
#include <QSpacerItem>
#include <QStringLiteral>
#include <QVBoxLayout>

#include <QToolButton>

#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "model/Ingredient.h"
#include "PersistentSettings.h"
#include "StockWindow.h"
#include "utils/BtStringStream.h"
#include "utils/CastAndConvert.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/PropertyHelper.h"
#include "widgets/CommonContextMenus.h"

// TBD: Double-click allows in-place edit in a recipe, but not in a catalog.  Maybe we should also allow that in the
//      latter?

/**
 * \brief This is used as a template parameter to turn on and off various \b small features in \c CatalogBase (in
 *        conjunction with the concepts defined below).
 *
 * \sa CatalogBase
 */
struct CatalogBaseOptions {
   /**
    * \brief This should be enabled for things such as Mash, Style, Equipment, where there is only one per Recipe (so
    *        it makes sense to show "Use in recipe" rather than "Add to recipe").
    *
    *        We could probably deduce this in another way, eg by seeing if \c NE inherits from \c Ingredient.  But this
    *        is simpler and more direct, without adding much extra code.
    */
   bool onePerRecipe = false;
};
template <CatalogBaseOptions cb> struct is_OnePerRecipe : public std::integral_constant<bool, cb.onePerRecipe>{};
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <CatalogBaseOptions cb> concept CONCEPT_FIX_UP IsOnePerRecipe = is_OnePerRecipe<cb>::value;

/**
 * \class CatalogBase
 *
 * \brief This is one of the base classes for \c HopCatalog, \c FermentableCatalog, etc.  Essentially each of these
 *        classes is a UI element that shows a list of all model items of a certain type, eg all hops or all
 *        fermentables, etc.
 *
 *        (The classes used to be called \c HopDialog, \c FermentableDialog, etc, which wasn't incorrect, but hopefully
 *        the new names are more descriptive.  In the UI, we also use phrases such as "hop database" for "list of all
 *        types of hop we know about", but that's confusing in the code, where \c Database has a more technical meaning.
 *        So, in the code, we prefer "hop catalog" as a more old-school synonym for "list/directory of all hops" etc.)
 *
 *        See editors/EditorBase.h for the idea behind what we're doing with the class structure here.  These catalog
 *        classes are "simpler" in that they don't have .ui files, but the use of the Curiously Recurring Template
 *        Pattern to minimise code duplication is the same.
 *
 *           QObject
 *                \
 *                ...
 *                  \
 *                  QDialog       CatalogBase<HopCatalog, Hop, HopTableModel, HopSortFilterProxyModel, HopEditor>
 *                        \       /
 *                         \     /
 *                        HopCatalog
 *
 *        Because the TableModel classes (\c HopTableModel, \c FermentableTableModel, etc) are doing most of the work,
 *        these Catalog classes are relatively simple.  NOTE that the columns displayed in \c HopCatalog are defined in
 *        \c HopTableModel (which is not to be confused with \c RecipeAdditionHopTableModel).
 *
 *        Classes inheriting from this one need to include the CATALOG_COMMON_DECL macro in their header file and
 *        the CATALOG_COMMON_CODE macro in their .cpp file.  Eg, in HopDialog.cpp, we need:
 *
 *          CATALOG_COMMON_CODE(Hop)
 *
 *        There is not much to the rest of the derived class (eg HopDialog).
 */
template<class Derived> class CatalogPhantom;
template<class Derived,
         class NE,
         class NeTableModel,
         class NeSortFilterProxyModel,
         class NeEditor,
         CatalogBaseOptions catalogBaseOptions>
class CatalogBase : public CuriouslyRecurringTemplateBase<CatalogPhantom, Derived> {
public:
   explicit CatalogBase(MainWindow * parent) :
      m_parent            {parent                           },
      m_neEditor          {new NeEditor(&this->derived())   },
      m_verticalLayout    {new QVBoxLayout(&this->derived())},
      m_tableWidget       {new QTableView (&this->derived())},
      m_horizontalLayout  {new QHBoxLayout()                },
      m_searchIcon        {new QToolButton(&this->derived())},
      m_lineEdit_searchBox{new QLineEdit()                  },
      m_horizontalSpacer  {new QSpacerItem(40,
                                           20,
                                           QSizePolicy::Expanding,
                                           QSizePolicy::Minimum) },
      m_pushButton_showStockPurchases{new QPushButton(&this->derived())},
      m_pushButton_newStockPurchase  {new QPushButton(&this->derived())},
      m_pushButton_addToRecipe       {new QPushButton(&this->derived())},
      m_pushButton_new               {new QPushButton(&this->derived())},
      m_pushButton_edit              {new QPushButton(&this->derived())},
      m_pushButton_delete            {new QPushButton(&this->derived())},
      m_contextMenus   {this->derived()},
      m_neTableModel   {new NeTableModel(m_tableWidget, false)},
      m_sortFilterProxy{new NeSortFilterProxyModel(m_tableWidget,
                                                   true,
                                                   m_neTableModel)} {
      if constexpr (std::is_base_of_v<Ingredient, NE>) {
         std::get<QCheckBox *>(this->m_inventoryFilter) = new QCheckBox(&this->derived());
         std::get<QLabel    *>(this->m_inventoryFilter) = new QLabel   (&this->derived());
         std::get<QCheckBox *>(this->m_inventoryFilter)->setChecked(false);
      }
      this->m_sortFilterProxy->setDynamicSortFilter(false);
      this->m_tableWidget->setModel(m_sortFilterProxy);
      this->m_tableWidget->setSortingEnabled(true);
      this->m_tableWidget->sortByColumn(static_cast<int>(NeTableModel::ColumnIndex::Name), Qt::AscendingOrder);
      this->m_sortFilterProxy->setFilterKeyColumn(1);

      this->m_lineEdit_searchBox->setMaxLength(30);

      this->m_pushButton_showStockPurchases->setObjectName(QStringLiteral("m_pushButton_showStockPurchases"));
      this->m_pushButton_newStockPurchase  ->setObjectName(QStringLiteral("m_pushButton_newStockPurchase"  ));
      this->m_pushButton_addToRecipe       ->setObjectName(QStringLiteral("pushButton_addToRecipe"         ));
      this->m_pushButton_new               ->setObjectName(QStringLiteral("pushButton_new"                 ));
      this->m_pushButton_edit              ->setObjectName(QStringLiteral("pushButton_edit"                ));
      this->m_pushButton_delete            ->setObjectName(QStringLiteral("pushButton_delete"              ));

      this->m_pushButton_showStockPurchases->setAutoDefault(false);
      this->m_pushButton_newStockPurchase  ->setAutoDefault(false);
      this->m_pushButton_addToRecipe       ->setAutoDefault(false);
      this->m_pushButton_new               ->setAutoDefault(false);
      this->m_pushButton_edit              ->setAutoDefault(false);
      this->m_pushButton_delete            ->setAutoDefault(false);

      this->m_pushButton_addToRecipe->setDefault(true);

      //
      // Although it would be logical to have m_searchIcon as a QLabel, and put the image on it via setPixmap, this
      // results in the image being too small.  (It's the same story if we use setTextFormat(Qt::RichText) to allow the
      // QLabel text to be rich text pointing to the image.)
      //
      // TBD: Probably we need to create our own subclass of QLabel that takes a QIcon and generates a correctly scaled
      // QPixmap before calling setPixmap.  However, in the meantime, we do a hack where we use a QToolButton instead of
      // QLabel and just remove the button styling.  The button doesn't look like a button, and doesn't do anything,
      // other than display the image at the size we want.
      //
      this->m_searchIcon->setStyleSheet("QToolButton {border-style: outset; border-width: 0px;}");
      this->m_searchIcon                 ->setIcon(QIcon{QStringLiteral(":/images/iconSearch.svg"          )});

      this->m_pushButton_showStockPurchases->setIcon(QIcon{QStringLiteral(":/images/iconInventory.svg"       )});
      this->m_pushButton_newStockPurchase  ->setIcon(QIcon{QStringLiteral(":/images/iconNewStockPurchase.svg")});
      this->m_pushButton_edit              ->setIcon(QIcon{QStringLiteral(":/images/edit.svg"                )});
      this->m_pushButton_delete            ->setIcon(QIcon{QStringLiteral(":/images/smallMinus.svg"          )});
      this->m_pushButton_new               ->setIcon(QIcon{QStringLiteral(":/images/smallPlus.svg"           )});

      // The order we add things to m_horizontalLayout determines their left-to-right order in that layout
      this->m_horizontalLayout->addWidget(this->m_searchIcon        );
      this->m_horizontalLayout->addWidget(this->m_lineEdit_searchBox);
      this->m_horizontalLayout->addItem  (this->m_horizontalSpacer  );
      if constexpr (std::is_base_of_v<Ingredient, NE>) {
         this->m_horizontalLayout->addWidget(std::get<QCheckBox *>(this->m_inventoryFilter));
         this->m_horizontalLayout->addWidget(std::get<QLabel    *>(this->m_inventoryFilter));
      }

      if constexpr (CanHaveStockPurchase<NE>) {
         this->m_horizontalLayout->addWidget(this->m_pushButton_showStockPurchases);
         this->m_horizontalLayout->addWidget(this->m_pushButton_newStockPurchase);
      }
      this->m_horizontalLayout->addWidget(this->m_pushButton_addToRecipe);
      this->m_horizontalLayout->addWidget(this->m_pushButton_new        );
      this->m_horizontalLayout->addWidget(this->m_pushButton_edit       );
      this->m_horizontalLayout->addWidget(this->m_pushButton_delete     );
      this->m_verticalLayout  ->addWidget(this->m_tableWidget           );
      this->m_verticalLayout  ->addLayout(this->m_horizontalLayout      );

      this->derived().setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      this->retranslateUi();

      // Not sure why this is called?
//      QMetaObject::connectSlotsByName(&this->derived());

      this->derived().connect(m_lineEdit_searchBox    , &QLineEdit::textEdited   , &this->derived(), &Derived::filterItems   );
      if constexpr (std::is_base_of_v<Ingredient, NE>) {
         this->derived().connect(
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            std::get<QCheckBox *>(this->m_inventoryFilter), &QCheckBox::checkStateChanged, &this->derived(), &Derived::inventoryFilter
#else
            std::get<QCheckBox *>(this->m_inventoryFilter), &QCheckBox::stateChanged     , &this->derived(), &Derived::inventoryFilter
#endif
         );
      }

      if constexpr (CanHaveStockPurchase<NE>) {
         this->derived().connect(m_pushButton_showStockPurchases, &QAbstractButton::clicked, &this->derived(), &Derived::showStockPurchases);
         this->derived().connect(m_pushButton_newStockPurchase  , &QAbstractButton::clicked, &this->derived(), &Derived::newStockPurchase  );
      }
      this->derived().connect(m_pushButton_addToRecipe, &QAbstractButton::clicked, &this->derived(), &Derived::addSelectedToRecipe);
      this->derived().connect(m_pushButton_edit       , &QAbstractButton::clicked, &this->derived(), &Derived::editSelected  );
      this->derived().connect(m_pushButton_delete     , &QAbstractButton::clicked, &this->derived(), &Derived::deleteSelected);
      // Rename is on the context menu but does not have its own button
      this->derived().connect(m_pushButton_new        , &QAbstractButton::clicked, &this->derived(), &Derived::newItem       );

      this->m_neTableModel->observeDatabase(true);

      return;
   }
   virtual ~CatalogBase() = default;

   void retranslateUi() {
      this->m_contextMenus.retranslateUi();

      this->derived().setWindowTitle(QString(QObject::tr("%1 Catalog / Database")).arg(NE::localisedName()));

      if constexpr (std::is_base_of_v<Ingredient, NE>) {
         std::get<QLabel *>(this->m_inventoryFilter)->setText(Derived::tr("Show only non-zero inventory"));
      }

      if constexpr (CanHaveStockPurchase<NE>) {
         this->m_pushButton_showStockPurchases->setText(QString());
         this->m_pushButton_newStockPurchase  ->setText(QString());
      }

      //
      // Many of the button tooltips are the same text as the equivalent context menu entries, so we re-use those rather
      // than duplicate the translation strings.
      //

      this->m_pushButton_addToRecipe->setText(this->m_contextMenus.m_action_addToRecipe.text());
      this->m_pushButton_new   ->setText(this->m_contextMenus.m_action_newItem.text());
      this->m_pushButton_edit  ->setText(QString());
      this->m_pushButton_delete->setText(QString());
#ifndef QT_NO_TOOLTIP
      this->m_searchIcon->setToolTip(QString(QObject::tr("Filter search")));
      if constexpr (CanHaveStockPurchase<NE>) {
         this->m_pushButton_showStockPurchases->setToolTip(QString(QObject::tr("Show stock purchases")));
         this->m_pushButton_newStockPurchase  ->setToolTip(QString(QObject::tr("New stock purchase")));
      }
      if constexpr (IsTableModel<NeTableModel>) {
         if constexpr (IsOnePerRecipe<catalogBaseOptions>) {
            this->m_pushButton_addToRecipe->setToolTip(QString(QObject::tr("Set selected %1 for current recipe")).arg(NE::localisedName()));
         } else {
            this->m_pushButton_addToRecipe->setToolTip(QString(QObject::tr("Add selected %1 to current recipe")).arg(NE::localisedName()));
         }
      }
      this->m_pushButton_new   ->setToolTip(QString(QObject::tr("Create new %1"     )).arg(NE::localisedName()));
      this->m_pushButton_edit  ->setToolTip(QString(QObject::tr("Edit selected %1"  )).arg(NE::localisedName()));
      this->m_pushButton_delete->setToolTip(QString(QObject::tr("Delete selected %1")).arg(NE::localisedName()));
#endif

      this->m_lineEdit_searchBox->setPlaceholderText(QObject::tr("Enter filter"));
      return;
   }

   /**
    * \brief This gets called from \c MainWindow to prevent a locked Recipe from being modified
    *
    * @param enabled
    */
   void setEnableAddToRecipe(bool enabled) {
      if constexpr (IsTableModel<NeTableModel>) {
         this->m_pushButton_addToRecipe->setEnabled(enabled);
         this->m_contextMenus.m_action_addToRecipe.setEnabled(enabled);
      }
      return;
   }

   /**
    * \brief Because \c QItemSelectionModel::selectedIndexes can contain entries for multiple columns as well as
    *        multiple rows, we need a function to just give us the unique rows selected.
    */
   QList<int> getSelectedSourceRowNumbers() const {
      // QList is pretty much std::vector.  It's not the greatest data structure for checking whether it already
      // contains an item (as we do in the loop below), but we're typically dealing with such small data sets that it's
      // not worth trying to optimise further.
      QList<int> selectedSourceRowNumbers;
      for (QModelIndex viewIndex : this->m_tableWidget->selectionModel()->selectedIndexes()) {
         QModelIndex const sourceIndex = this->m_sortFilterProxy->mapToSource(viewIndex);
         int const rowNumber = sourceIndex.row();
         if (!selectedSourceRowNumbers.contains(rowNumber)) {
            selectedSourceRowNumbers.append(rowNumber);
         }
      }
      return selectedSourceRowNumbers;
   }

   /**
    * \brief Returns a (possibly empty) list of all the selected items
    */
   QList<std::shared_ptr<NE>> getMultipleSelected() const {
      QList<std::shared_ptr<NE>> selectedItems;
      for (int rowNumber : this->getSelectedSourceRowNumbers()) {
         selectedItems.append(this->m_neTableModel->getRow(rowNumber));
      }
      return selectedItems;
   }

   /**
    * \brief If a single item is selected, returns a pointer to it
    *
    * \return \c nullptr if nothing is selected or if more than one item is selected
    */
   std::shared_ptr<NE> getFirstSelected() const {
      QList<std::shared_ptr<NE>> selectedItems = this->getMultipleSelected();

      if (selectedItems.size() != 1) {
         return nullptr;
      }

      return selectedItems[0];
   }

   /**
    * \brief Returns how many items are selected.  This is used when we display the context menu to decide which actions
    *        should be enabled.
    */
   int getNumSelected() const {
      return this->getSelectedSourceRowNumbers().size();
   }

   /**
    * \brief Subclass should call this from its \c showStockPurchases slot
    */
   void doShowStockPurchases() const {
      if constexpr (CanHaveStockPurchase<NE>) {
         std::shared_ptr<NE> selected = this->getFirstSelected();
         // Unlike with doNewStockPurchase, if nothing is selected, we still want to do something: pull up the relevant
         // window & tab, but with no filter applied.
         WindowDistributor::get<StockWindow>().showStockPurchasesFor(selected.get());
      }
      return;
   }

   /**
    * \brief Subclass should call this from its \c newStockPurchase slot
    */
   void doNewStockPurchase() const {
      if constexpr (CanHaveStockPurchase<NE>) {
         std::shared_ptr<NE> selected = this->getFirstSelected();
         if (selected) {
            WindowDistributor::editorForNewStockPurchase(selected.get());
         }
      }
      return;
   }

   /**
    * \brief Subclass should call this from its \c addSelectedToRecipe slot
    */
   void doAddSelectedToRecipe() const {
      std::shared_ptr<NE> selected = this->getFirstSelected();
      this->m_contextMenus.addToOrSetForRecipe(selected);
      return;
   }

   /**
    * \brief Subclass should call this from its \c copySelected slot
    */
   void doCopySelected() {
      QList<std::shared_ptr<NE>> selectedItems = this->getMultipleSelected();
      this->m_contextMenus.copyItems(selectedItems);
      return;
   }

   /**
    * \brief Subclass should call this from its \c deleteSelected slot
    */
   void doDeleteSelected() {
      QList<std::shared_ptr<NE>> selectedItems = this->getMultipleSelected();
      this->m_contextMenus.deletePrimaryItems(selectedItems);
      return;
   }

   /**
    * \brief Subclass should call this from its \c renameSelected slot
    */
   void doRenameSelected() {
      this->m_contextMenus.renamePrimaryItem(*this->getFirstSelected());
      return;
   }

   /**
    * \brief Subclass should call this from its \c editSelected slot
    */
   void doEditSelected() {
      std::shared_ptr<NE> item = this->getFirstSelected();
      if (item) {
         this->m_neEditor->setEditItem(item);
         this->m_neEditor->show();
      }

      return;
   }

   /**
    * \brief Subclass should call this from its \c newItem slot.
    *
    *        Note that the \c newItem slot doesn't take a parameter and always relies on the default folder
    *        parameter here, whereas direct callers can specify a folder.
    *
    *        \c makeNew is not the greatest name, but `new` is a reserved word and `create` is already taken by QWidget
    *
    * \param folderPath
    */
   void makeNew(QString folderPath = "") {
      this->m_neEditor->newEditItem(folderPath);
      return;
   }

   /**
    * \brief Subclass should call this from its \c exportSelected slot
    */
   void doExportSelected() const {
      QList<std::shared_ptr<NE>> selectedItems = this->getMultipleSelected();
      this->m_contextMenus.exportItems(CastAndConvert::toConstRaw(selectedItems));
      return;
   }

   /**
    * \brief Subclass should call this from its \c importFiles slot
    *
    *        NOTE: We import whatever is in the files, so, if you click "import" from the Fermentables catalog and
    *              supply a file of Styles and Hops then we'll import those even though you were not in the Styles or
    *              Hops bit of the UI.  I think that's reasonable behaviour.  If it turns out that being able to
    *              filter the imports (eg only import Fermentable records from this file) is important to a lot of
    *              users, then we could have a rethink.
    */
   void doImportFromFiles() const {
      ImportExport::importFromFiles();
      return;
   }

   /**
    * \brief Subclass should call this from its \c mergeSelected slot
    */
   void doMergeSelected() {
      QList<std::shared_ptr<NE>> const selectedItems = this->getMultipleSelected();
      this->m_contextMenus.mergeItems(selectedItems);
      return;
   }

   /**
    * \brief Subclass should call this from its \c filterItems slot
    */
   void filter(QString searchExpression) {
      this->m_sortFilterProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
      this->m_sortFilterProxy->setFilterFixedString(searchExpression);
      return;
   }

   void doInventoryFilter(bool filterOn) {
      if constexpr (std::is_base_of_v<Ingredient, NE>) {
         this->m_sortFilterProxy->setHideZeroInventoryItems(filterOn);
      }
      return;
   }

   void doContextMenu(QPoint const & point) {
      QModelIndex selectedViewIndex = this->m_tableWidget->indexAt(point);
      if (!selectedViewIndex.isValid()) {
         return;
      }

      this->m_contextMenus.showContextMenu(
         this->derived().mapToGlobal(point),
         CommonContextMenuHelper::Selected<NE>{
            .numPrimary = this->getNumSelected(),
            .firstPrimary = this->getFirstSelected()
         }
      );
      return;
   }

   void saveUiState(BtStringConst const & property,
                    BtStringConst const & section = PersistentSettings::Sections::MainWindow) const {
      this->m_neTableModel->saveUiState(property, section);
      return;
   }

   void restoreUiState(BtStringConst const & property,
                       BtStringConst const & section = PersistentSettings::Sections::MainWindow) {
      this->m_neTableModel->restoreUiState(property, section);
      return;
   }

   //================================================ Member Variables =================================================

   // Arguably we don't need to store this pointer as MainWindow is a singleton.  However, we get given it at
   // construction, so, why not...
   MainWindow * m_parent;

   NeEditor *   m_neEditor;

   //! \name Public UI Variables
   //! @{
   QVBoxLayout * m_verticalLayout;
   QTableView  * m_tableWidget;
   QHBoxLayout * m_horizontalLayout;
   QToolButton * m_searchIcon;
   QLineEdit   * m_lineEdit_searchBox;
   QSpacerItem * m_horizontalSpacer;
   QPushButton * m_pushButton_showStockPurchases;
   QPushButton * m_pushButton_newStockPurchase  ;
   QPushButton * m_pushButton_addToRecipe       ;
   QPushButton * m_pushButton_new               ;
   QPushButton * m_pushButton_edit              ;
   QPushButton * m_pushButton_delete            ;

   CommonContextMenus<Derived, NE, void, false> m_contextMenus;

   //
   // See comment in trees/TreeNodeBase.h for more details of this trick we use to get close to conditional member
   // variables.  Using a tuple as a nameless struct is a separate trick that works when all the contained types are
   // different.
   //
   struct Empty { };
   [[no_unique_address]] std::conditional_t<std::is_base_of_v<Ingredient, NE>,
                                            std::tuple<QCheckBox *, QLabel *>,
                                            Empty>  m_inventoryFilter;

   //! @}

   NeTableModel *           m_neTableModel;
   NeSortFilterProxyModel * m_sortFilterProxy;
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#define CHECK_STATE_TYPE Qt::CheckState
#else
#define CHECK_STATE_TYPE int
#endif

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define CATALOG_COMMON_DECL(NeName) \
   /* This allows CatalogBase to call protected and private members of Derived */  \
   friend class CatalogBase<NeName##Catalog,                                       \
                            NeName,                                                \
                            NeName##TableModel,                                    \
                            NeName##SortFilterProxyModel,                          \
                            NeName##Editor,                                        \
                            NeName##CatalogOptions>;                               \
                                                                                   \
   public:                                                                         \
      NeName##Catalog(MainWindow * parent);                                        \
      virtual ~NeName##Catalog();                                                  \
                                                                                   \
   public slots:                                                                   \
      void showStockPurchases() const;                                             \
      void newStockPurchase() const;                                               \
      void addSelectedToRecipe() const;                                            \
      void copySelected();                                                         \
      void deleteSelected();                                                       \
      void renameSelected();                                                       \
      void editSelected();                                                         \
      void newItem();                                                              \
      void exportSelected() const;                                                 \
      void importFromFiles() const;                                                \
      void mergeSelected();                                                        \
      void filterItems(QString searchExpression);                                  \
      void inventoryFilter(CHECK_STATE_TYPE state);                                \
      void contextMenu(QPoint const & point);                                      \
                                                                                   \
   protected:                                                                      \
      virtual void changeEvent(QEvent* event);                                     \


/**
 * \brief Derived classes should include this in their implementation file
 *
 *        Note that we cannot implement changeEvent in the base class (\c CatalogBase) because it needs access to
 *        \c QDialog::changeEvent, which is \c protected.
 */
#define CATALOG_COMMON_CODE(NeName) \
   NeName##Catalog::NeName##Catalog(MainWindow* parent) : \
      QDialog(parent),                                    \
      CatalogBase<NeName##Catalog,                        \
                  NeName,                                 \
                  NeName##TableModel,                     \
                  NeName##SortFilterProxyModel,           \
                  NeName##Editor,                         \
                  NeName##CatalogOptions>(parent) {       \
      return;                                             \
   }                                                      \
                                                          \
   NeName##Catalog::~NeName##Catalog() = default;         \
                                                          \
   void NeName##Catalog::showStockPurchases() const            { this->doShowStockPurchases();   return; } \
   void NeName##Catalog::newStockPurchase() const              { this->doNewStockPurchase();     return; } \
   void NeName##Catalog::addSelectedToRecipe() const           { this->doAddSelectedToRecipe();  return; } \
   void NeName##Catalog::copySelected()                        { this->doCopySelected();         return; } \
   void NeName##Catalog::deleteSelected()                      { this->doDeleteSelected();       return; } \
   void NeName##Catalog::renameSelected()                      { this->doRenameSelected();       return; } \
   void NeName##Catalog::editSelected()                        { this->doEditSelected();         return; } \
   void NeName##Catalog::newItem()                             { this->makeNew();                return; } \
   void NeName##Catalog::exportSelected() const                { this->doExportSelected();       return; } \
   void NeName##Catalog::importFromFiles() const               { this->doImportFromFiles();      return; } \
   void NeName##Catalog::mergeSelected()                       { this->doMergeSelected();        return; } \
   void NeName##Catalog::filterItems(QString searchExpression) { this->filter(searchExpression); return; } \
   void NeName##Catalog::inventoryFilter(CHECK_STATE_TYPE state) { \
      this->doInventoryFilter(Qt::Checked == state);               \
      return;                                                      \
   }                                                               \
   void NeName##Catalog::contextMenu(QPoint const & point) { \
      this->doContextMenu(point);                            \
      return;                                                \
   }                                                         \
   void NeName##Catalog::changeEvent(QEvent* event) { \
      if (event->type() == QEvent::LanguageChange) {  \
         this->retranslateUi();                       \
      }                                               \
      this->QDialog::changeEvent(event);              \
      return;                                         \
   }                                                  \

#endif