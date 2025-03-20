/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/sortFilterProxyModels/SortFilterProxyModelBase.h is part of Brewtarget, and is copyright the following authors
 * 2023-2024:
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
 *        Derived classes need to implement lessThan to provide the right per-column logic for this.
 */
template<class Derived> class SortFilterProxyModelPhantom;
template<class Derived, class NeTableModel, class NeListModel>
class SortFilterProxyModelBase : public CuriouslyRecurringTemplateBase<SortFilterProxyModelPhantom, Derived> {
public:
   /**
    * \param filter If \c true then we only show "displayable" items; if \c false then we show everything
    */
   SortFilterProxyModelBase(bool filter) :
      m_filter{filter} {
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
      NeTableModel * tableModel = qobject_cast<NeTableModel *>(this->derived().sourceModel());
      if (tableModel) {
         QModelIndex index = tableModel->index(source_row, 0, source_parent);

         if (!this->m_filter) {
            // No filter, so we accept
            return true;
         }
         if (!tableModel->getRow(source_row)->display()) {
            // Row not displayed, so reject
            return false;
         }

         // The filterRegularExpression() member function we call here is inherited from QSortFilterProxyModel
         QRegularExpression const filterRegExp {this->derived().filterRegularExpression()};
         QString const dataAsString {tableModel->data(index).toString()};
         QRegularExpressionMatch const match {filterRegExp.match(dataAsString)};
         return match.hasMatch();
      }

      NeListModel* listModel = qobject_cast<NeListModel*>(this->derived().sourceModel());
      if (listModel) {
         auto listItem = listModel->at(source_row);
         if (!listItem) {
            qWarning() << Q_FUNC_INFO << "Non-existent item at row" << source_row;
            return true;
         }

         return listItem->display() && !listItem->deleted();
      }

      qWarning() << Q_FUNC_INFO << "Unrecognised source model";
      return true;
   }

   bool doLessThan(QModelIndex const & left, QModelIndex const & right) const {
      QAbstractItemModel * source = this->derived().sourceModel();
      QVariant leftItem, rightItem;
      if (source) {
         leftItem = source->data(left);
         rightItem = source->data(right);
      }

      // As per more detailed comment in qtModels/tableModels/ItemDelegate.h, we need "typename" here only until Apple ship Clang
      // 16 or later as their standard C++ compiler.
      auto const columnIndex = static_cast<typename NeTableModel::ColumnIndex>(left.column());
      return this->derived().isLessThan(columnIndex, leftItem, rightItem);
   }

private:
   bool const m_filter;
};


/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define SORT_FILTER_PROXY_MODEL_COMMON_DECL(NeName) \
   /* This allows SortFilterProxyModelBase to call protected and private members of Derived */  \
   friend class SortFilterProxyModelBase<NeName##SortFilterProxyModel,                          \
                                         NeName##TableModel,                                    \
                                         NeName##ListModel>;                                    \
                                                                                                \
   public:                                                                                      \
      NeName##SortFilterProxyModel(QObject *            parent      = nullptr,                  \
                                   bool                 filter      = true   ,                  \
                                   QAbstractItemModel * sourceModel = nullptr);                 \
      virtual ~NeName##SortFilterProxyModel();                                                  \
                                                                                                \
   protected:                                                                                   \
      /* Override QSortFilterProxyModel::filterAcceptsRow                          */           \
      /* Returns true if the item in the row indicated by the given source_row and */           \
      /* source_parent should be included in the model; otherwise returns false.   */           \
      virtual bool filterAcceptsRow(int source_row, QModelIndex const & source_parent) const;   \
      /* Override QSortFilterProxyModel::lessThan                                  */           \
      virtual bool lessThan(QModelIndex const & left, QModelIndex const & right) const;         \
   private:                                                                                     \
      /* Called from lessThan to do the work specific to this class                */           \
      bool isLessThan(NeName##TableModel::ColumnIndex const columnIndex,                        \
                      QVariant const & leftItem,                                                \
                      QVariant const & rightItem) const;                                        \

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
   bool NeName##SortFilterProxyModel::filterAcceptsRow(int source_row,                            \
                                                       QModelIndex const & source_parent) const { \
      return this->doFilterAcceptsRow(source_row, source_parent);                                 \
   }                                                                                              \
   bool NeName##SortFilterProxyModel::lessThan(QModelIndex const & left,                          \
                                               QModelIndex const & right) const {                 \
      return this->doLessThan(left, right);                                                       \
   }                                                                                              \

#endif
