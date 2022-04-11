/*
 * MiscTableModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#ifndef TABLEMODELS_MISCTABLEMODEL_H
#define TABLEMODELS_MISCTABLEMODEL_H
#pragma once

#include <memory>

#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "measurement/Unit.h"
#include "tableModels/BtTableModelInventory.h"


// Forward declarations.
class BtStringConst;
class MiscItemDelegate;
class Misc;
class MiscTableWidget;
class Recipe;

enum{MISCNAMECOL, MISCTYPECOL, MISCUSECOL, MISCTIMECOL, MISCAMOUNTCOL, MISCINVENTORYCOL, MISCISWEIGHT, MISCNUMCOLS /*This one MUST be last*/};

/*!
 * \class MiscTableModel
 *
 * \brief Table model for a list of miscs.
 */
class MiscTableModel : public BtTableModelInventory, public BtTableModelData<Misc> {
   Q_OBJECT

public:
   MiscTableModel(QTableView* parent=nullptr, bool editable=true);
   virtual ~MiscTableModel();

   //! \brief Observe a recipe's list of miscs.
   void observeRecipe(Recipe* rec);
   //! \brief If true, we model the database's list of miscs.
   void observeDatabase(bool val);
   //! \brief Add \c miscs to the model.
   void addMiscs(QList<std::shared_ptr<Misc> > miscs);
   //! \brief Clear the model.
   void removeAll();

   //! \brief Reimplemented from QAbstractTableModel
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
   bool remove(std::shared_ptr<Misc> misc);

public slots:
   //! \brief Add a misc to the model.
   void addMisc(int miscId);
   //! \brief Remove a misc from the model.
   void removeMisc(int miscId, std::shared_ptr<QObject> object);

private slots:
   //! \brief Catch changes to Recipe, Database, and Misc.
   void changed(QMetaProperty, QVariant);
   void changedInventory(int invKey, BtStringConst const & propertyName);
};

/*!
 * \class MiscItemDelegate
 *
 * \brief Item delegate for misc tables.
 * \sa MiscTableModel
 */
class MiscItemDelegate : public QItemDelegate {
   Q_OBJECT

public:
   MiscItemDelegate(QObject* parent = nullptr);

   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

#endif
