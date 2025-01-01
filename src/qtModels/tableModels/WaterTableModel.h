/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/WaterTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Forward declarations.
class Water;
class Recipe;
class RecipeUseOfWater;

class WaterItemDelegate;

// You have to get the order of everything right with traits classes, but the end result is that we can refer to
// WaterTableModel::ColumnIndex::Calcium etc.
class WaterTableModel;
template <> struct TableModelTraits<WaterTableModel> {
   enum class ColumnIndex {
      Name       ,
///      Amount     ,
      Calcium    ,
      Bicarbonate,
      Sulfate    ,
      Chloride   ,
      Sodium     ,
      Magnesium  ,
   };
};

/*!
 * \class WaterTableModel
 *
 * \brief Table model for waters.
 */
class WaterTableModel : public BtTableModel, public TableModelBase<WaterTableModel, Water> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(Water)
///
///public:
///   WaterTableModel(QTableView * parent = nullptr);
///   virtual ~WaterTableModel();
///
///   //! \brief Casting wrapper for \c BtTableModel::getColumnInfo
///   ColumnInfo const & getColumnInfo(ColumnIndex const columnIndex) const;
///
///   void addWaters(QList<std::shared_ptr<Water> > waters);
///   void addWaters(Recipe const & recipe);
///   void observeRecipe(Recipe* rec);
///   void observeDatabase(bool val);
///   void removeAll();
///
///   //! Reimplemented from QAbstractTableModel.
///   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
///   //! Reimplemented from QAbstractTableModel.
///   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
///   //! Reimplemented from QAbstractTableModel.
///   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
///   //! Reimplemented from QAbstractTableModel.
///   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
///   //! Reimplemented from QAbstractTableModel.
///   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
///
///public slots:
///   void changed(QMetaProperty,QVariant);
///   void addWater(int waterId);
///   void removeWater(int waterId, std::shared_ptr<QObject> object);
};

//=============================================== CLASS WaterItemDelegate ===============================================

/*!
 * \brief An item delegate for Water tables.
 * \sa WaterTableModel.
 */
class WaterItemDelegate : public QItemDelegate,
                          public ItemDelegate<WaterItemDelegate, WaterTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(Water)
};


////*!
/// * \class WaterItemDelegate
/// *
/// * \brief Item delegate for water tables.
/// */
///class WaterItemDelegate : public QItemDelegate {
///   Q_OBJECT
///
///public:
///   WaterItemDelegate(QObject* parent = nullptr);
///
///   // Inherited functions.
///   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
///   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
///   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
///   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
///
///private:
///};

#endif
