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
#include "salt.h"

#include "unit.h"
// Forward declarations.
class Recipe;
class WaterDialog;

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
   SaltTableModel(QTableView* parent=nullptr, WaterDialog* gp = nullptr);
   ~SaltTableModel();
   void observeRecipe(Recipe* rec);
   void addSalt(Salt* salt);
   void addSalts(QList<Salt*> salts);
   // void observeDatabase(bool val);
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

    double total_Ca() const;
    double total_Cl() const;
    double total_CO3() const;
    double total_HCO3() const;
    double total_Mg() const;
    double total_Na() const;
    double total_SO4() const;

    double total( Salt::Types type ) const;

protected slots:
   void changed(QMetaProperty,QVariant);
   void removeSalt(Salt* salt);
   void catchSalt(Salt* salt);
   void catchSalts(QList<Salt*> salt);
   void contextMenu(const QPoint &point);

signals:
   void newTotals();

private:
   QVector<Qt::ItemFlags> colFlags;
   QList<Salt*> saltObs;
   Recipe* recObs;
   QTableView* parentTableWidget;
   WaterDialog* dropper;

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

