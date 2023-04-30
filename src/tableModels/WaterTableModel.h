/*
 * WaterTableModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef TABLEMODELS_WATERTABLEMODEL_H
#define TABLEMODELS_WATERTABLEMODEL_H
#pragma once

#include <memory>

#include <QItemDelegate>
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "measurement/Unit.h"
#include "model/Water.h"
#include "tableModels/BtTableModel.h"

// Forward declarations.
class Water;
class WaterTableWidget;
class Recipe;

class WaterItemDelegate;

/*!
 * \class WaterTableModel
 *
 * \brief Table model for waters.
 */
class WaterTableModel : public BtTableModelRecipeObserver, public BtTableModelData<Water> {
   Q_OBJECT

public:
   enum class ColumnIndex {
      Name       ,
      Amount     ,
      Calcium    ,
      Bicarbonate,
      Sulfate    ,
      Chloride   ,
      Sodium     ,
      Magnesium  ,
   };
   WaterTableModel(WaterTableWidget* parent = nullptr);
   virtual ~WaterTableModel();

   //! \brief Casting wrapper for \c BtTableModel::getColumnInfo
   ColumnInfo const & getColumnInfo(ColumnIndex const columnIndex) const;

   void addWaters(QList<std::shared_ptr<Water> > waters);
   void observeRecipe(Recipe* rec);
   void observeDatabase(bool val);
   void removeAll();

   //! Reimplemented from QAbstractTableModel.
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   //! Reimplemented from QAbstractTableModel.
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   //! Reimplemented from QAbstractTableModel.
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
   //! Reimplemented from QAbstractTableModel.
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   //! Reimplemented from QAbstractTableModel.
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

public slots:
   void changed(QMetaProperty,QVariant);
   void addWater(int waterId);
   void removeWater(int waterId, std::shared_ptr<QObject> object);
};

/*!
 * \class WaterItemDelegate
 *
 * \brief Item delegate for water tables.
 */
class WaterItemDelegate : public QItemDelegate {
   Q_OBJECT

public:
   WaterItemDelegate(QObject* parent = 0);

   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

#endif
