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
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/PropertyHelper.h"

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
 *        classes are "simpler" in that they don't have .ui files, but the use of the of the Curiously Recurring
 *        Template Pattern to minimise code duplication is the same.
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

   CatalogBase(MainWindow * parent) :
      m_parent                {parent                                   },
      m_neEditor              {new NeEditor(&this->derived())           },
      m_verticalLayout        {new QVBoxLayout(&this->derived())        },
      m_tableWidget           {new QTableView (&this->derived())        },
      m_horizontalLayout      {new QHBoxLayout()                        },
      m_searchIcon            {new QToolButton(&this->derived())        },

      m_lineEdit_searchBox    {new QLineEdit()                          },
      m_horizontalSpacer      {new QSpacerItem(40,
                                               20,
                                               QSizePolicy::Expanding,
                                               QSizePolicy::Minimum)    },
      m_pushButton_showStockPurchases{new QPushButton(&this->derived())     },
      m_pushButton_newStockPurchase  {new QPushButton(&this->derived())     },
      m_pushButton_addToRecipe       {new QPushButton(&this->derived())     },
      m_pushButton_new               {new QPushButton(&this->derived())     },
      m_pushButton_edit              {new QPushButton(&this->derived())     },
      m_pushButton_delete            {new QPushButton(&this->derived())     },
      m_contextMenu                  {new QMenu      (&this->derived())     },
      m_action_showStockPurchases    {new QAction    (&this->derived())     },
      m_action_newStockPurchase      {new QAction    (&this->derived())     },
      m_action_addToRecipe           {new QAction    (&this->derived())     },
      m_action_edit                  {new QAction    (&this->derived())     },
      m_action_delete                {new QAction    (&this->derived())     },
      m_action_new                   {new QAction    (&this->derived())     },
      m_action_merge                 {new QAction    (&this->derived())     },
      m_neTableModel                 {new NeTableModel(m_tableWidget, false)},
      m_sortFilterProxy              {new NeSortFilterProxyModel(m_tableWidget,
                                                                 true,
                                                                 m_neTableModel)} {
      if constexpr (IsIngredient<NE>) {
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
      this->m_horizontalLayout->addWidget(this->m_searchIcon            );
      this->m_horizontalLayout->addWidget(this->m_lineEdit_searchBox    );
      this->m_horizontalLayout->addItem  (this->m_horizontalSpacer      );
      if constexpr (IsIngredient<NE>) {
         this->m_horizontalLayout->addWidget(std::get<QCheckBox *>(this->m_inventoryFilter));
         this->m_horizontalLayout->addWidget(std::get<QLabel    *>(this->m_inventoryFilter));
      }

      if constexpr (CanHaveStockPurchase<NE>) {
         this->m_horizontalLayout->addWidget(this->m_pushButton_showStockPurchases);
         this->m_horizontalLayout->addWidget(this->m_pushButton_newStockPurchase);
      }
      this->m_horizontalLayout->addWidget(this->m_pushButton_addToRecipe     );
      this->m_horizontalLayout->addWidget(this->m_pushButton_new             );
      this->m_horizontalLayout->addWidget(this->m_pushButton_edit            );
      this->m_horizontalLayout->addWidget(this->m_pushButton_delete          );
      this->m_verticalLayout  ->addWidget(this->m_tableWidget                );
      this->m_verticalLayout  ->addLayout(this->m_horizontalLayout           );

      this->derived().setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      this->retranslateUi();
      QMetaObject::connectSlotsByName(&this->derived());

      this->derived().connect(m_lineEdit_searchBox    , &QLineEdit::textEdited   , &this->derived(), &Derived::filterItems   );
      if constexpr (IsIngredient<NE>) {
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
      this->derived().connect(m_pushButton_new        , &QAbstractButton::clicked, &this->derived(), &Derived::newItem       );

      //
      // === Context Menu ===
      //
      // Although we could add all actions directly, having separate QAction objects allows us to disable them, eg if
      // nothing is selected.  Some things (eg new) don't need to be disabled, but it seems neater to me to do all the
      // actions the same way.
      //
      // TODO: We should align this context menu more closely with the one in trees/TreeViewBase.h
      //
      if constexpr (CanHaveStockPurchase<NE>) {


         this->derived().connect(this->m_action_showStockPurchases, &QAction::triggered, &this->derived(), &Derived::showStockPurchases);
         this->derived().connect(this->m_action_newStockPurchase  , &QAction::triggered, &this->derived(), &Derived::newStockPurchase  );
      }
      this->derived().connect(this->m_action_addToRecipe, &QAction::triggered, &this->derived(), &Derived::addSelectedToRecipe);
      this->derived().connect(this->m_action_edit       , &QAction::triggered, &this->derived(), &Derived::editSelected       );
      this->derived().connect(this->m_action_delete     , &QAction::triggered, &this->derived(), &Derived::deleteSelected     );
      this->derived().connect(this->m_action_new        , &QAction::triggered, &this->derived(), &Derived::newItem            );
      this->derived().connect(this->m_action_merge      , &QAction::triggered, &this->derived(), &Derived::mergeSelected      );

      if constexpr (CanHaveStockPurchase<NE>) {
         this->m_contextMenu->addAction(this->m_action_showStockPurchases);
         this->m_contextMenu->addAction(this->m_action_newStockPurchase  );
      }
      this->m_contextMenu->addAction(this->m_action_addToRecipe);
      this->m_contextMenu->addAction(this->m_action_edit       );
      this->m_contextMenu->addAction(this->m_action_delete     );
      this->m_contextMenu->addAction(this->m_action_new        );
      this->m_contextMenu->addAction(this->m_action_merge      );

      // Setting Qt::CustomContextMenu here causes the signal customContextMenuRequested() to be emitted when the user
      // requests the context menu (by right-clicking).
      this->derived().setContextMenuPolicy(Qt::CustomContextMenu);
      this->derived().connect(&this->derived(), &QWidget::customContextMenuRequested, &this->derived(), &Derived::contextMenu);

      this->m_neTableModel->observeDatabase(true);

      return;
   }
   virtual ~CatalogBase() = default;

   void retranslateUi() {
      this->derived().setWindowTitle(QString(QObject::tr("%1 Catalog / Database")).arg(NE::localisedName()));

      if constexpr (CanHaveStockPurchase<NE>) {
         this->m_action_showStockPurchases->setText(Derived::tr("Show stock purchases for selected %1").arg(NE::localisedName()));
         this->m_action_newStockPurchase  ->setText(Derived::tr("New stock purchase for selected %1").arg(NE::localisedName()));
      }
      this->m_action_addToRecipe->setText(Derived::tr("Add %1 to recipe"         ).arg(NE::localisedName()));
      this->m_action_edit       ->setText(Derived::tr("Edit selected %1"         ).arg(NE::localisedName()));
      this->m_action_delete     ->setText(Derived::tr("Delete selected %1"       ).arg(NE::localisedName()));
      this->m_action_new        ->setText(Derived::tr("New %1"                   ).arg(NE::localisedName()));
      this->m_action_merge      ->setText(Derived::tr("Merge selected %1 records").arg(NE::localisedName()));
      if constexpr (IsIngredient<NE>) {
         std::get<QLabel *>(this->m_inventoryFilter)->setText(Derived::tr("Show only non-zero inventory"));
      }

      if constexpr (CanHaveStockPurchase<NE>) {
         this->m_pushButton_showStockPurchases->setText(QString());
         this->m_pushButton_newStockPurchase  ->setText(QString());
      }
      if constexpr (IsTableModel<NeTableModel>) {
         //
         // We say "add to recipe" for things like hops, fermentables, etc, where there can be more than one in a
         // recipe.  We say "set for recipe" for things like equipment, style, mash, etc where there is only one per
         // recipe.
         //
         if constexpr (IsOnePerRecipe<catalogBaseOptions>) {
            this->m_pushButton_addToRecipe->setText(QString(QObject::tr("Set for Recipe")));
         } else {
            this->m_pushButton_addToRecipe->setText(QString(QObject::tr("Add to Recipe")));
         }
      }
      this->m_pushButton_new   ->setText(QString(QObject::tr("New")));
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

   void setEnableAddToRecipe(bool enabled) {
      if constexpr (IsTableModel<NeTableModel>) {
         this->m_pushButton_addToRecipe->setEnabled(enabled);
      }
      return;
   }

   /**
    * \brief Because \c QItemSelectionModel::selectedIndexes can contain entries for multiple columns as well as
    *        multiple rows, we need a function to just give us the unique rows selected.
    *
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
   std::shared_ptr<NE> getSingleSelected() const {
      QList<std::shared_ptr<NE>> selectedItems = this->getMultipleSelected();

      if (selectedItems.size() != 1) {
         return nullptr;
      }

      return selectedItems[0];
   }

   /**
    * \brief Returns how many items are selected.  This is used when we display the context menu to decide which actions
    *        should be enabld.
    */
   int getNumSelected() const {
      return this->getSelectedSourceRowNumbers().size();
   }

   /**
    * \brief Subclass should call this from its \c showStockPurchases slot
    */
   void doShowStockPurchases() const {
      if constexpr (CanHaveStockPurchase<NE>) {
         std::shared_ptr<NE> selected = this->getSingleSelected();
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
         std::shared_ptr<NE> selected = this->getSingleSelected();
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
      std::shared_ptr<NE> selected = this->getSingleSelected();
      if (selected) {
         //
         // In both cases, MainWindow does the heavy lifting here, including ensuring that the action is undoable
         //
         if constexpr (IsIngredient<NE>) {
            //
            // Version for FermentableCatalog, HopCatalog, MiscCatalog, YeastCatalog, etc
            this->m_parent->addIngredientToRecipe(*selected);
         } else {
            //
            // Version for EquipmentCatalog, StyleCatalog, MashCatalog, BoilCatalog, FermentationCatalog
            //
            this->m_parent->setForRecipe(selected);
         }
      }

      return;
   }

   /**
    * \brief Subclass should call this from its \c deleteSelected slot
    */
   void doDeleteSelected() {
      std::shared_ptr<NE> item = this->getSingleSelected();
      if (!item) {
         return;
      }

      //
      // If someone tries to delete something that's used in one or more Recipes then we just say it's not allowed.
      // (The alternative would be to check that they are sure and then run through every Recipe removing the item
      // about to be deleted, which could perhaps leave that Recipe in a weird state.)
      //
      // TODO: We have similar logic in TreeViewBase which we should ideally unify somewhere.
      //
      int const numRecipesUsedIn = item->numRecipesUsedIn();
      if (numRecipesUsedIn > 0) {
         QMessageBox::warning(&this->derived(),
                              Derived::tr("%1 in use").arg(NE::localisedName()),
                              Derived::tr("Cannot delete this %1, as it is used in %n recipe(s)",
                                          "",
                                          numRecipesUsedIn).arg(NE::localisedName()),
                              QMessageBox::Ok);
         return;
      }

      QString confirmationMessage = Derived::tr("Delete %1 #%2 \"%3\"? (%1)").arg(
                                       NE::localisedName()
                                    ).arg(
                                       item->key()
                                    ).arg(
                                       item->name()
                                    ).arg(
                                       Recipe::usedInRecipes(*item)
                                    );

      auto confirmDelete = QMessageBox::question(
         &this->derived(),
         Derived::tr("Delete %1").arg(NE::localisedName()),
         confirmationMessage,
         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
         QMessageBox::No
      );

      if (confirmDelete == QMessageBox::Yes) {
         ObjectStoreWrapper::softDelete(*item);
      }

      return;
   }

   /**
    * \brief Subclass should call this from its \c editSelected slot
    */
   void doEditSelected() {
      std::shared_ptr<NE> item = this->getSingleSelected();
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
    * \brief Subclass should call this from its \c mergeSelected slot
    */
   void doMergeSelected() {
      QList<std::shared_ptr<NE>> selectedItems = this->getMultipleSelected();
      if (selectedItems.size() < 2) {
         return;
      }

      auto const confirmMerge = QMessageBox::question(
         &this->derived(),
         // It's a bit of a cheat here to use "records" to avoid having to pluralise NE::localisedName()...
         Derived::tr("Merge %1 records").arg(NE::localisedName()),
         Derived::tr("Attempt to merge %1 %2 records? (NOTE: <b>This action cannot be undone!</b>  It is intended for "
                     "use where the records are identical or as near as makes no difference.  "
                     "Use on non-identical records risks data loss.)"
                     "<br><br><b>Please make sure you have a backup of your database file before using this "
                     "function!</b>").arg(
            selectedItems.size()
         ).arg(
            NE::localisedName()
         ),
         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
         QMessageBox::No
      );

      if (confirmMerge != QMessageBox::Yes) {
         return;
      }

      //
      // We're not doing anything clever here, just repeatedly merging the first and last items on the list until either
      // we hit an error or the list only has one item.
      //
      for (; selectedItems.size() >= 2; selectedItems.pop_back()) {
         //
         // The "merge" of two records is simply the replacement of one by the other.  Specifically we will delete one
         // of the items and replace all references to it with the other one.
         //
         // To decide which direction to do the merge, look at which is used in more recipes (and prefer to retain that
         // one).
         //
         // Note that we deliberately want references to shared pointers here, so we can swap the actual elements of the
         // list if needed.
         //
         std::shared_ptr<NE> & firstItem = selectedItems.first();
         std::shared_ptr<NE> & lastItem  = selectedItems.last ();
         if (lastItem->numRecipesUsedIn() > firstItem->numRecipesUsedIn()) {
            //
            // The items being swapped are shared pointers, so this should be relatively efficient
            //
            std::swap(firstItem, lastItem);
         }
         qInfo() <<
            Q_FUNC_INFO << "Attempting to merge" << *lastItem << "(used in" << lastItem->numRecipesUsedIn() <<
            "recipes) into" << *firstItem << "(used in" << firstItem->numRecipesUsedIn() << "recipes)";

         //
         // We are going to delete lastItem and replace all references to it with references to firstItem.
         //
         // Before we do that, check whether the items are identical and, if not, give the user a chance to bail.
         //
         QList<BtStringConst const *> propertiesThatDiffer = firstItem->getPropertiesThatDiffer(*lastItem);
         int const numDifferences = propertiesThatDiffer.size();
         if (numDifferences > 0) {
            //
            // Show the user the list of properties that differ, and their values, and ask them if they really really
            // want to force the merge.  We'll show them how many fields differ in the main message, and then we'll list
            // each of the differences in the "detailed text" that can be shown by clicking "Show details..."
            //
            // Normally this is exactly what we'd use QMessageBox::setDetailedText for.  However, we really want to show
            // the data in tabular format using HTML, and QMessageBox does not support this for its detailed text field.
            // So we have to use BtMessageBox instead. TODO Create that class and replace QMessageBox below!
            //
            QMessageBox diffsFoundMessageBox;
            diffsFoundMessageBox.setWindowTitle(Derived::tr("%1 records differ").arg(NE::localisedName()));
            diffsFoundMessageBox.setText(
               Derived::tr("WARNING: %1 records to be merged have %n difference(s).  "
                           "Do you want to merge anyway?", "0", numDifferences).arg(NE::localisedName())
            );
            BtStringStream detailedText;
            detailedText << "<table>"
                            "<tr>"
                               "<th>Field</th>"
                               "<th>" << firstItem->name() << " (#" << firstItem->key() << ")" << "</th>"
                               "<th>" <<  lastItem->name() << " (#" <<  lastItem->key() << ")" << "</th>"
                            "</tr>"
                            "<tr>"
                               "<td>№ Recipes</td>"
                               "<td>" << firstItem->numRecipesUsedIn() << "</td>"
                               "<td>" <<  lastItem->numRecipesUsedIn() << "</td>"
                            "</tr>";
            for (BtStringConst const * propertyName : propertiesThatDiffer) {
               TypeInfo const & typeInfo = NE::typeLookup.getType(*propertyName);

               detailedText << "<tr>"
                               "<td>" << typeInfo.localisedName() << "</td>"
                               "<td>" << PropertyHelper::readDataFromPropertyValue(
                                            firstItem->property(**propertyName),
                                            typeInfo,
                                            Qt::DisplayRole
                                         ).toString().toHtmlEscaped() << "</td>"
                               "<td>" << PropertyHelper::readDataFromPropertyValue(
                                            lastItem->property(**propertyName),
                                            typeInfo,
                                            Qt::DisplayRole
                                         ).toString().toHtmlEscaped() << "</td>"
                               "</tr>";
            }
            detailedText << "</table>";

            diffsFoundMessageBox.setInformativeText(
               Derived::tr(
                  "%n field(s) differ between %1 #%2 and %1 #%3.  If you continue, %1 #%3 will be deleted "
                  "and all uses of it will be replaced by %1 #%2.  This <b>cannot</b> be undone.<br><br>%4",
                  "0",
                  numDifferences
               ).arg(NE::localisedName()).arg(firstItem->key()).arg(lastItem->key()).arg(detailedText.asString())
            );

            diffsFoundMessageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No |QMessageBox::Cancel);
            diffsFoundMessageBox.setDefaultButton(QMessageBox::No);

            int ret = diffsFoundMessageBox.exec();
            if (ret == QMessageBox::Cancel) {
               //
               // Cancel means stop trying to merge further records, so the user will investigate further and then
               // reselect the records s/he wants.
               //
               return;
            }
            if (ret == QMessageBox::No) {
               //
               // No means skip this record pair
               //
               continue;
            }
         }

         //
         // Find all the uses of the last item on the list and replace it with the first one
         //
         int const lastItemKey = lastItem->key();
         if constexpr (IsIngredient<NE>) {
            QList<std::shared_ptr<typename NE::RecipeAdditionClass>> recipeAdditions =
               ObjectStoreWrapper::findAllMatching<typename NE::RecipeAdditionClass>(
                  [lastItemKey](std::shared_ptr<typename NE::RecipeAdditionClass> ra) {
                     return ra->ingredientId() == lastItemKey;
                  }
               );
            qInfo() <<
               Q_FUNC_INFO << "Replacing" << recipeAdditions.size() << "uses of" << lastItem << "with" << firstItem;
            for (auto const & recipeAddition : recipeAdditions) {
               recipeAddition->setIngredientId(firstItem->key());
            }

            //
            // Now we have to do the same thing for inventory entries.  The code is almost the same because, in both
            // cases, the class we're dealing with inherits from IngredientAmount, which is the class that holds the
            // field we care about.
            //
            QList<std::shared_ptr<typename NE::StockPurchaseClass>> inventoryEntries =
               ObjectStoreWrapper::findAllMatching<typename NE::StockPurchaseClass>(
                  [lastItemKey](std::shared_ptr<typename NE::StockPurchaseClass> inv) {
                     return inv->ingredientId() == lastItemKey;
                  }
               );
            qInfo() <<
               Q_FUNC_INFO << "Assigning" << inventoryEntries.size() << "inventory entries for" << lastItem << "to" <<
               firstItem;
            for (auto const & inventoryEntry : inventoryEntries) {
               inventoryEntry->setIngredientId(firstItem->key());
            }

         } else {
            QList<std::shared_ptr<Recipe>> recipes = ObjectStoreWrapper::findAllMatching<Recipe>(
               [&lastItem](std::shared_ptr<Recipe> rec) { return rec->uses(*lastItem); }
            );
            qInfo() <<
               Q_FUNC_INFO << "Replacing" << recipes.size() << "uses of" << lastItem << "with" << firstItem;
            for (auto const & recipe : recipes) {
               recipe->set(firstItem);
            }
         }

         //
         // Item we are about to delete should no longer be used in any recipes
         //
         Q_ASSERT(0 == lastItem->numRecipesUsedIn());

         qInfo() << Q_FUNC_INFO << "Deleting" << lastItem;
         ObjectStoreWrapper::softDelete(*lastItem);
      }

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
      if constexpr (IsIngredient<NE>) {
         this->m_sortFilterProxy->setHideZeroInventoryItems(filterOn);
      }
      return;
   }

   void doContextMenu(QPoint const & point) {
      QModelIndex selectedViewIndex = this->m_tableWidget->indexAt(point);
      if (!selectedViewIndex.isValid()) {
         return;
      }

      //
      // TBD: For the moment, we don't allow multiple selections either to be deleted or to be added to the recipe.  But
      //      it would not be huge work to fix that if there is user demand for it.
      //
      int const numSelected = this->getNumSelected();
      this->m_action_addToRecipe->setEnabled(numSelected == 1);
      this->m_action_edit       ->setEnabled(numSelected == 1);
      this->m_action_delete     ->setEnabled(numSelected == 1);
      this->m_action_merge      ->setEnabled(numSelected  > 1);

      this->m_contextMenu->exec(this->derived().mapToGlobal(point));
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

   QMenu       * m_contextMenu;
   QAction     * m_action_showStockPurchases;
   QAction     * m_action_newStockPurchase;
   QAction     * m_action_addToRecipe     ;
   QAction     * m_action_edit            ;
   QAction     * m_action_delete          ;
   QAction     * m_action_new             ;
   QAction     * m_action_merge           ;

   //
   // See comment in trees/TreeNodeBase.h for more details of this trick we use to get close to conditional member
   // variables.  Using a tuple as a nameless struct is a separate trick that works when all the contained types are
   // different.
   //
   struct Empty { };
   [[no_unique_address]] std::conditional_t<IsIngredient<NE>,
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
      void deleteSelected();                                                       \
      void editSelected();                                                         \
      void newItem();                                                              \
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
   void NeName##Catalog::deleteSelected()                      { this->doDeleteSelected();       return; } \
   void NeName##Catalog::editSelected()                        { this->doEditSelected();         return; } \
   void NeName##Catalog::newItem()                             { this->makeNew();                return; } \
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
