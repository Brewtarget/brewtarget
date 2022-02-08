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

#endif
