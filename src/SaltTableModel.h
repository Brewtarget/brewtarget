/*
 * SaltTableModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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

#ifndef _SALTTABLEMODEL_H
#define   _SALTTABLEMODEL_H

class SaltTableModel;
class SaltItemDelegate;

#include <QAbstractTableModel>
#include <QWidget>
#include <QModelIndex>
#include <QMetaProperty>
#include <QVariant>
#include <QItemDelegate>
#include <QList>
#include <QTableView>

#include "unit.h"
// Forward declarations.
class Salt;
class SaltTableWidget;
class Recipe;

enum{ SALTNAMECOL, SALTAMOUNTCOL, SALTADDTOCOL,
      SALTNUMCOLS /*This one MUST be last*/};

/*!
 * \class SaltTableModel
 * \author Philip G. Lee
 *
 * \brief Table model for salts.
 */
class SaltTableModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   SaltTableModel(QTableView* parent=nullptr);
   virtual ~SaltTableModel() {}
   void addSalts(QList<Salt*> salts);
   void observeRecipe(Recipe* rec);
   void observeDatabase(bool val);
   void removeAll();

   //! Reimplemented from QAbstractTableModel.
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   //! Reimplemented from QAbstractTableModel.
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
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
   void addSalt(Salt* salt);
   void removeSalt(Salt* salt);

private:
   QList<Salt*> saltObs;
   Recipe* recObs;
   SaltTableWidget* parentTableWidget;

   void setDisplayUnit(int column, Unit::unitDisplay displayUnit);
   void setDisplayScale(int column, Unit::unitScale displayScale);
   QString generateName(int column) const;
   Unit::unitDisplay displayUnit(int column) const;
   Unit::unitScale displayScale(int column) const;
};

/*!
 * \class SaltItemDelegate
 * \author Philip G. Lee
 *
 * Item delegate for salt tables.
 */
class SaltItemDelegate : public QItemDelegate
{
   Q_OBJECT

public:
   SaltItemDelegate(QObject* parent = nullptr);

   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

#endif   /* _SALTTABLEMODEL_H */

