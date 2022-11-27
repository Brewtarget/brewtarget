/*
 * SaltTableModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#ifndef TABLEMODELS_SALTTABLEMODEL_H
#define TABLEMODELS_SALTTABLEMODEL_H
#pragma once

#include <QItemDelegate>
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "measurement/Unit.h"
#include "model/Salt.h"
#include "model/Water.h"
#include "tableModels/BtTableModel.h"

// Forward declarations.
class Mash;
class Recipe;
class SaltItemDelegate;
class WaterDialog;

enum{ SALTNAMECOL,
      SALTAMOUNTCOL,
      SALTADDTOCOL,
      SALTPCTACIDCOL,
      SALTNUMCOLS /*This one MUST be last*/};
/*!
 * \class SaltTableModel
 *
 * \brief Table model for salts.
 */
class SaltTableModel : public BtTableModelRecipeObserver, public BtTableModelData<Salt> {
   Q_OBJECT

public:
   SaltTableModel(QTableView* parent = nullptr);
   ~SaltTableModel();
   void observeRecipe(Recipe* rec);
private:
   void addSalt(std::shared_ptr<Salt> salt);
public:
   void addSalts(QList<std::shared_ptr<Salt> > salts);
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

   double total_Ca() const;
   double total_Cl() const;
   double total_CO3() const;
   double total_HCO3() const;
   double total_Mg() const;
   double total_Na() const;
   double total_SO4() const;

   double total(Water::Ions ion) const;
   double total( Salt::Types type ) const;
   double totalAcidWeight(Salt::Types type) const;

   void removeSalts(QList<int>deadSalts);
   void saveAndClose();

public slots:
   void changed(QMetaProperty,QVariant);
   void remove(std::shared_ptr<Salt> salt);
   void catchSalt();

signals:
   void newTotals();

private:
   double spargePct;
   double multiplier(Salt & salt) const;
};

/*!
 * \class SaltItemDelegate Item delegate for salt tables.
 */
class SaltItemDelegate : public QItemDelegate {
   Q_OBJECT

public:
   SaltItemDelegate(QObject* parent = nullptr);

   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

   void observeRecipe(Recipe *rec);

private:
   Mash* m_mash;
   // I really dislike this

};

#endif
