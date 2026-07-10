/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/sortFilterProxyModels/SortFilterProxyModelBase.h is part of Brewtarget, and is copyright the following authors
 * 2023-2026:
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
#ifndef SORTFILTERPROXYMODELS_SORTFILTERPROXYMODELBASE_H
#define SORTFILTERPROXYMODELS_SORTFILTERPROXYMODELBASE_H
#pragma once

#include <QDebug>
#include <QModelIndex>

#include "model/Ingredient.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \brief Curiously Recurring Template Pattern (CRTP) base class for HopSortFilterProxyModel,
 *        FermentableSortFilterProxyModel, etc
 *
 *           QSortFilterProxyModel     SortFilterProxyModelBase<HopSortFilterProxyModel, HopTableModel>
 *                           \            /
 *                            \          /
 *                          HopSortFilterProxyModel
 *
 *        Derived classes need include \c SORT_FILTER_PROXY_MODEL_COMMON_DECL in their header file and
 *        \c SORT_FILTER_PROXY_MODEL_COMMON_CODE in their \c .cpp file.  This will provide an appropriate override of
 *        \c QSortFilterProxyModel::lessThan to do the per-column logic for sorting (via \c TableModelBase::isLessThan).
 */
template<class Derived> class SortFilterProxyModelPhantom;
template<class Derived, class NeTableModel, class NeListModel>
class SortFilterProxyModelBase : public CuriouslyRecurringTemplateBase<SortFilterProxyModelPhantom, Derived> {
public:
   /**
    * \param onlyShowDisplayable If \c true then we only show "displayable" items; if \c false then we show everything
    */
   explicit SortFilterProxyModelBase(bool const onlyShowDisplayable) :
      m_onlyShowDisplayable{onlyShowDisplayable} {
      return;
   }

   void setHideZeroInventoryItems(bool const val) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
      // New way, since Qt 6.10
      this->derived().beginFilterChange();
      this->m_hideZeroInventoryItems = val;
      this->derived().endFilterChange();
#else
      // Old way will be deprecated in Qt 6.13
      this->m_hideZeroInventoryItems = val;
      this->derived().invalidateFilter();
#endif

      return;
   }

   void setAdditionalFilter(
      std::optional<std::function<bool(typename NeTableModel::UnderlyingItem const & underlyingItem)>> additionalFilter
   ) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
      // New way, since Qt 6.10
      this->derived().beginFilterChange();
      this->m_additionalFilter = additionalFilter;
      this->derived().endFilterChange();
#else
      // Old way will be deprecated in Qt 6.13
      this->m_additionalFilter = additionalFilter;
      this->derived().invalidateFilter();
#endif
      return;
   }

protected:
   bool doFilterAcceptsRow(int source_row, QModelIndex const & source_parent) const {
      //
      // Note that sourceModel can be either a subclass of QAbstractListModel (eg StyleListModel) or a subclass of
      // QAbstractTableModel (eg StyleTableModel)
      //
      //                                             QAbstractItemModel
      //                                                |         |
      //                                                |         |
      //                                 QAbstractListModel      QAbstractTableModel
      //                                         |                        |
      //                                         |                        |
      //  ListModelBase<StyleListModel, Style>   |                 BtTableModel   TableModelBase<StyleTableModel, Style>
      //                               \         |                        |         /
      //                                \        |                        |        /
      //                                 \       |                       ...      /
      //                                  \      |                        |      /
      //                                   \     |                        |     /
      //                                   StyleListModel          StyleTableModel
      //
      // In some cases, we can just treat sourceModel as QAbstractItemModel and rely on virtual member functions, such
      // as index() and data().  In others, we need to cast as:
      //
      //    - getRow() is only in TableModelBase
      //    - at() is only in ListModelBase
      //
      // The simplest way round this is to try both options.  This requires we have a ListModel class for every
      // TableModel class and vice versa (ie, if FooTableModel exists then so must FooListModel), which means, in
      // practice, that we create a few ListModel classes that we don't otherwise use.  However, since ListModelBase
      // does all the work, this is actually almost no overhead.  The obvious alternative, of creating a common base
      // class from which ListModelBase and TableModelBase inherit, seems a fair bit of work in comparison!
      //

      typename NeTableModel::UnderlyingItem const * underlyingItem = nullptr;
      QString dataAsString = "";
      if (NeTableModel * tableModel = qobject_cast<NeTableModel *>(this->derived().sourceModel())) {
         underlyingItem = tableModel->getRow(source_row).get();
         QModelIndex const index = tableModel->index(source_row, 0, source_parent);
         dataAsString = tableModel->data(index).toString();

      } else if (NeListModel* listModel = qobject_cast<NeListModel*>(this->derived().sourceModel())) {
         underlyingItem = listModel->at(source_row);
         QModelIndex const index = listModel->index(source_row, 0);
         dataAsString = listModel->data(index).toString();

      } else {
         // It's a coding error if we get here!
         qCritical() << Q_FUNC_INFO << "Unrecognised source model";
         return true;
      }

      if (!underlyingItem) {
         qWarning() << Q_FUNC_INFO << "Non-existent item at row" << source_row;
         return true;
      }

      if (this->m_onlyShowDisplayable && underlyingItem->deleted()) {
         // Row deleted, so reject
         return false;
      }

      if constexpr (std::is_base_of_v<Ingredient, typename NeTableModel::UnderlyingItem>) {
         if (this->m_hideZeroInventoryItems && !underlyingItem->isOnHand()) {
            return false;
         }
      }

      if (this->m_additionalFilter && !(*this->m_additionalFilter)(*underlyingItem)) {
         return false;
      }

      // The filterRegularExpression() member function we call here is inherited from QSortFilterProxyModel
      QRegularExpression const filterRegExp {this->derived().filterRegularExpression()};
      QRegularExpressionMatch const match {filterRegExp.match(dataAsString)};
      return match.hasMatch();
   }

   /**
    *
    * NOTE Per https://doc.qt.io/qt-6/qsortfilterproxymodel.html#lessThan that the indices passed in correspond to the
    *      source model.
    */
   bool doLessThan(QModelIndex const & sourceLeft, QModelIndex const & sourceRight) const {
      if (NeTableModel * tableModel = qobject_cast<NeTableModel *>(this->derived().sourceModel())) {
         return tableModel->isLessThan(sourceLeft, sourceRight);
      }
      if (NeListModel* listModel = qobject_cast<NeListModel*>(this->derived().sourceModel())) {
         // List model is for a single column -- eg a combo box -- and we assume it's always a string
         QVariant const  leftItem = this->derived().data( sourceLeft);
         QVariant const rightItem = this->derived().data(sourceRight);
         return leftItem.toString() < rightItem.toString();
      }
      qWarning() << Q_FUNC_INFO << "Unrecognised source model";
      return true;
   }

   QModelIndex doMapToSource(QModelIndex const & proxyIndex) const {
      //
      // We _normally_ shouldn't need to do anything here other than call the base class member function...
      //
      if (proxyIndex.isValid() && proxyIndex.model() != &this->derived()) {
         //
         // This shouldn't happen, but it does.  I think there is a bug somewhere in ListModelBase,
         // SortFilterProxyModelBase or BtComboBoxNamedEntity.  In BtComboBoxNamedEntityBase::init(), We create a
         // NeListModel and a NeSortFilterProxyModel, and setting the former as source model for the latter.  When we
         // call sort(0) on the NeSortFilterProxyModel, we get a lot of logs of
         //
         //    WARNING : QSortFilterProxyModel: index from wrong model passed to mapToSource
         //
         // See QSortFilterProxyModelPrivate::proxy_to_source() (eg at
         // https://codebrowser.dev/qt5/qtbase/src/corelib/itemmodels/qsortfilterproxymodel.cpp.html) in the Qt source
         // for where this message is logged.
         //
         // AFAICT we have a source model index being passed in as a proxy model index.  Until we work out how to fix
         // the bug properly, this is a workaround.
         //
         if (this->derived().sourceModel() == proxyIndex.model()) {
            //
            // We want to avoid logging this error message more than once per instance of this class.  The logic here
            // means we'll see the log a few times rather than have hundreds of repetitions of the same warning.
            //
            static bool alreadyLogged = false;
            if (!alreadyLogged) {
               qWarning() <<
                  Q_FUNC_INFO << "Index refers to source-model instead of proxy-model in call to mapToSource().";
               alreadyLogged = true;
            }
            return proxyIndex;
         }
         //
         // Not expecting to ever get here, but might as well log something if we do!
         //
         qCritical() << Q_FUNC_INFO << "Index refers to unrecognised model in call to mapSource()";
      }
      return this->derived().QSortFilterProxyModel::mapToSource(proxyIndex);
   }

private:
   bool const m_onlyShowDisplayable;
   /**
    * \brief This is only meaningful for Ingredients, but it's too much hassle to optimise it out for other types
    */
   bool m_hideZeroInventoryItems = false;

   std::optional<
      std::function<bool(typename NeTableModel::UnderlyingItem const & underlyingItem)>
   > m_additionalFilter = std::nullopt;
};


/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define SORT_FILTER_PROXY_MODEL_COMMON_DECL(NeName) \
   /* This allows SortFilterProxyModelBase to call protected and private members of Derived */        \
   friend class SortFilterProxyModelBase<NeName##SortFilterProxyModel,                                \
                                         NeName##TableModel,                                          \
                                         NeName##ListModel>;                                          \
                                                                                                      \
   public:                                                                                            \
      NeName##SortFilterProxyModel(QObject *            parent      = nullptr,                        \
                                   bool                 filter      = true   ,                        \
                                   QAbstractItemModel * sourceModel = nullptr);                       \
      virtual ~NeName##SortFilterProxyModel();                                                        \
                                                                                                      \
      /* Override QSortFilterProxyModel::mapToSource for diagnostic purposes */                       \
      virtual QModelIndex mapToSource(QModelIndex const & proxyIndex) const override;                 \
                                                                                                      \
   protected:                                                                                         \
      /* Override QSortFilterProxyModel::filterAcceptsRow                          */                 \
      /* Returns true if the item in the row indicated by the given source_row and */                 \
      /* source_parent should be included in the model; otherwise returns false.   */                 \
      virtual bool filterAcceptsRow(int source_row,                                                   \
                                    QModelIndex const & source_parent) const override;                \
      /* Override QSortFilterProxyModel::lessThan                                  */                 \
      virtual bool lessThan(QModelIndex const & source_left,                                          \
                            QModelIndex const & source_right) const override;                         \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define SORT_FILTER_PROXY_MODEL_COMMON_CODE(NeName)            \
   NeName##SortFilterProxyModel::NeName##SortFilterProxyModel(QObject *            parent     ,  \
                                                              bool                 filter     ,  \
                                                              QAbstractItemModel * sourceModel): \
      QSortFilterProxyModel{parent},                         \
      SortFilterProxyModelBase<NeName##SortFilterProxyModel, \
                               NeName##TableModel,           \
                               NeName##ListModel>{filter} {  \
      if (sourceModel) {                                     \
         this->setSourceModel(sourceModel);                  \
      }                                                      \
      return;                                                \
   }                                                         \
                                                             \
   NeName##SortFilterProxyModel::~NeName##SortFilterProxyModel() = default;                       \
                                                                                                  \
   QModelIndex NeName##SortFilterProxyModel::mapToSource(QModelIndex const & proxyIndex) const {  \
      return this->doMapToSource(proxyIndex);                                                     \
   }                                                                                              \
                                                                                                  \
   bool NeName##SortFilterProxyModel::filterAcceptsRow(int source_row,                            \
                                                       QModelIndex const & source_parent) const { \
      return this->doFilterAcceptsRow(source_row, source_parent);                                 \
   }                                                                                              \
   bool NeName##SortFilterProxyModel::lessThan(QModelIndex const & source_left,                   \
                                               QModelIndex const & source_right) const {          \
      return this->doLessThan(source_left, source_right);                                         \
   }                                                                                              \

#endif