/*
 * YeastTableModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
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
#ifndef TABLEMODELS_YEASTTABLEMODEL_H
#define TABLEMODELS_YEASTTABLEMODEL_H
#pragma once

#include <memory>

#include <QItemDelegate>
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QTableView>
#include <QVariant>
#include <QWidget>

#include "measurement/Unit.h"
#include "tableModels/BtTableModelInventory.h"
#include "model/Yeast.h"

// Forward declarations.
class BtStringConst;
class YeastTableWidget;
class YeastItemDelegate;
class Recipe;

/*!
 * \class YeastTableModel
 *
 * \brief Table model for yeasts.
 */
class YeastTableModel : public BtTableModelInventory, public BtTableModelData<Yeast> {
   Q_OBJECT

public:
   enum class ColumnIndex {
      Name     ,
      Lab      ,
      ProdId   ,
      Type     ,
      Form     ,
      Amount   ,
      Inventory,
   };
   YeastTableModel(QTableView * parent = nullptr, bool editable = true);
   virtual ~YeastTableModel();

   //! \brief Casting wrapper for \c BtTableModel::getColumnInfo
   ColumnInfo const & getColumnInfo(ColumnIndex const columnIndex) const;

   //! \brief Observe a recipe's list of fermentables.
   void observeRecipe(Recipe* rec);
   //! \brief If true, we model the database's list of yeasts.
   void observeDatabase(bool val);
   //! \brief Add \c yeasts to the model.
   void addYeasts(QList<std::shared_ptr<Yeast> > yeasts);
   //! \brief Clear the model.
   void removeAll();

   //! \brief Reimplemented from QAbstractTableModel.
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

   void remove(std::shared_ptr<Yeast> yeast);

public slots:
   //! \brief Add a \c yeast to the model.
   void addYeast(int yeastId);
   //! \brief Remove a \c yeast from the model.
   void removeYeast(int yeastId, std::shared_ptr<QObject> object);

private slots:
   //! \brief Catch changes to Recipe, Database, and Yeast.
   void changed(QMetaProperty, QVariant);
   void changedInventory(int invKey, BtStringConst const & propertyName);
};

/*!
 * \class YeastItemDelegate
 *
 * \brief Item delegate for yeast tables.
 */
class YeastItemDelegate : public QItemDelegate {
   Q_OBJECT

public:
   YeastItemDelegate(QObject* parent = nullptr);

   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

#endif
