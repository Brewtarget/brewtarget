/*
 * tableModels/BtTableModel.h is part of Brewtarget, and is copyright the following
 * authors 2021-2022:
 * - Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TABLEMODELS_BTTABLEMODEL_H
#define TABLEMODELS_BTTABLEMODEL_H
#pragma once

#include <optional>

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QTableView>

#include "BtFieldType.h"
#include "measurement/UnitSystem.h"

class Recipe;
class NamedEntity;

/**
 * \class BtTableModelData
 *
 * \brief Unfortunately we can't template \c BtTableModel because it inherits from a \c QObject and the Qt meta-object
 *        compiler (moc) can't handle templated classes in QObject-derived classes (though it is fine with templated
 *        member functions in such classes, as long as the are not signals or slots).  We might one day look at
 *        https://github.com/woboq/verdigris, which overcomes these limitations, but, for now, we live within Qt's
 *        limitations and try to pull out as much common code as possible using a limited form of multiple inheritance.
 *
 *              QObject
 *                   \
 *                   ...
 *                     \
 *              QAbstractTableModel
 *                           \
 *                            \
 *                          BtTableModel               BtTableModelData
 *                                /   \                /     /     /
 *                               /     \              /     /     /
 *                              /      MashStepTableModel  /     /
 *                             /                          /     /
 *                            /                          /     /
 *                         BtTableModelRecipeObserver   /     /
 *                              \       \              /     /
 *                               \       \            /     /
 *                                \      SaltTableModel    /
 *                                 \    WaterTableModel   /
 *                                  \                    /
 *                                   \                  /
 *                         BtTableModelInventory       /
 *                                    \               /
 *                                     \             /
 *                                FermentableTableModel
 *                                    HopTableModel
 *                                   MiscTableModel
 *                                   YeastTableModel
 *
 *        (I did start trying to do something clever with a common base class to try to expose functions from
 *        \c BtTableModelData to the implementation of \c BtTableModel / \c BtTableModelRecipeObserver /
 *        \c BtTableModelInventory, but it quickly starts getting more complicated than it's worth IMHO because (a)
 *        \c BtTableModelData is templated but a common base class cannot be and (b) templated functions cannot be virtual.)
 */
template<class NE>
class BtTableModelData {
protected:
   BtTableModelData() : rows{} {
      return;
   }
public:
   /**
    * \brief Return the \c i-th row in the model.
    *        Returns \c nullptr on failure.
    */
   std::shared_ptr<NE> getRow(int ii) {
      if (!(this->rows.isEmpty())) {
         if (ii >= 0 && ii < this->rows.size()) {
            return this->rows[ii];
         }
         qWarning() << Q_FUNC_INFO << "index out of range (" << ii << "/" << this->rows.size() << ")";
      } else {
         qWarning() << Q_FUNC_INFO << "this->rows is empty (" << ii << "/" << this->rows.size() << ")";
      }
      return nullptr;
   }

   /**
    * \brief Remove duplicates and non-displayable items from the supplied list
    */
   QList< std::shared_ptr<NE> > removeDuplicates(QList< std::shared_ptr<NE> > items, Recipe const * recipe = nullptr) {
      decltype(items) tmp;

      for (auto ii : items) {
         if (!recipe && (ii->deleted() || !ii->display())) {
               continue;
         }
         if (!this->rows.contains(ii) ) {
            tmp.append(ii);
         }
      }
      return tmp;
   }

protected:
   virtual std::shared_ptr<NamedEntity> getRowAsNamedEntity(int ii) {
      return std::static_pointer_cast<NamedEntity>(this->getRow(ii));
   }


   QList< std::shared_ptr<NE> > rows;
};

/*!
 * \class BtTableModel
 *
 * \brief Shared interface & code for all the table models we use
 */
class BtTableModel : public QAbstractTableModel {
   Q_OBJECT
public:
   struct ColumnInfo {
      QString headerName;
      BtFieldType fieldType;
      QString attribute;
   };

   BtTableModel(QTableView * parent,
                bool editable,
                std::initializer_list<std::pair<int const, ColumnInfo> > columnIdToInfo);
   virtual ~BtTableModel();

   // Stuff for setting display units and scales -- per cell column
   std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurementForColumn(int column) const;
   std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScaleForColumn(int column) const;
   void setForcedSystemOfMeasurementForColumn(int column,
                                              std::optional<Measurement::SystemOfMeasurement> systemOfMeasurement);
   void setForcedRelativeScaleForColumn(int column,
                                        std::optional<Measurement::UnitSystem::RelativeScale> relativeScale);

   //! \brief Called from \c headerData()
   QVariant getColumName(int column) const;

   // Per https://doc.qt.io/qt-5/qabstracttablemodel.html, when subclassing QAbstractTableModel, you must implement
   // rowCount(), columnCount(), and data(). Default implementations of the index() and parent() functions are provided
   // by QAbstractTableModel. Well behaved models will also implement headerData().

   //! \brief Reimplemented from QAbstractTableModel
   virtual int columnCount(QModelIndex const & parent = QModelIndex()) const;

private:
   QString     columnGetAttribute(int column) const;
   BtFieldType columnGetFieldType(int column) const;
   void doContextMenu(QPoint const & point, QHeaderView * hView, QMenu * menu, int selected);

public slots:
   //! \brief pops the context menu for changing units and scales
   void contextMenu(QPoint const & point);

protected:
   QTableView* parentTableWidget;
   bool editable;
private:
   QMap<int, ColumnInfo> columnIdToInfo;

};

class BtTableModelRecipeObserver : public BtTableModel {
public:
   BtTableModelRecipeObserver(QTableView * parent,
                              bool editable,
                              std::initializer_list<std::pair<int const, ColumnInfo> > columnIdToInfo);
   ~BtTableModelRecipeObserver();

protected:
   Recipe* recObs;
};
#endif
