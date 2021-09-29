/*
 * HopTableModel.h is part of Brewtarget, and is Copyright the following
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
#ifndef HOPTABLEMODEL_H
#define HOPTABLEMODEL_H

#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QModelIndex>
#include <QTableView>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include "model/Hop.h"
#include "model/Recipe.h"

class BtStringConst;
class HopTableModel;
class HopItemDelegate;

enum{HOPNAMECOL, HOPALPHACOL, HOPAMOUNTCOL, HOPINVENTORYCOL, HOPFORMCOL, HOPUSECOL, HOPTIMECOL, HOPNUMCOLS /*This one MUST be last*/};

/*!
 * \class HopTableModel
 *
 *
 * \brief Model class for a list of hops.
 */
class HopTableModel : public QAbstractTableModel
{
   Q_OBJECT

public:

   HopTableModel(QTableView* parent=nullptr, bool editable=true);
   virtual ~HopTableModel();
   //! \brief Observe a recipe's list of fermentables.
   void observeRecipe(Recipe* rec);
   //! \brief If true, we model the database's list of hops.
   void observeDatabase(bool val);
   //! \brief Show ibus in the vertical header.
   void setShowIBUs( bool var );
   //! \brief Watch all the \c hops for changes.
   void addHops(QList<Hop*> hops);
   //! \brief Return the \c i-th hop in the model.
   Hop* getHop(int i);
   //! \brief Clear the model.
   void removeAll();

   /*!
    * \brief True if the inventory column should be editable, false otherwise.
    *
    * The default is that the inventory column is not editable
    */
   void setInventoryEditable( bool var ) { _inventoryEditable = var; colFlags[HOPINVENTORYCOL] = Qt::ItemIsEnabled | (_inventoryEditable ? Qt::ItemIsEditable : Qt::NoItemFlags); }

   //! \brief Reimplemented from QAbstractTableModel.
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   //! \brief Reimplemented from QAbstractTableModel.
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

   // Stuff for setting display units and scales -- per cell first, then by
   // column

   Unit::unitDisplay displayUnit(int column) const;
   Unit::unitScale displayScale(int column) const;
   void setDisplayUnit(int column, Unit::unitDisplay displayUnit);
   void setDisplayScale(int column, Unit::unitScale displayScale);


   QString generateName(int column) const;

   //! \returns true if "hop" is successfully found and removed.
   bool remove(Hop* hop);

public slots:
   void changed(QMetaProperty, QVariant);
   void changedInventory(int invKey, BtStringConst const & propertyName);
   //! \brief Add a hop to the model.
//   void addHop(Hop* hop);
   void addHop(int hopId);
   void removeHop(int hopId, std::shared_ptr<QObject> object);

   void contextMenu(const QPoint &point);

private:
   QVector<Qt::ItemFlags> colFlags;
   bool _inventoryEditable;
   QList<Hop*> hopObs;
   Recipe* recObs;
   QTableView* parentTableWidget;
   bool showIBUs; // True if you want to show the IBU contributions in the table rows.
};

/*!
 *  \class HopItemDelegate
 *
 *
 *  \brief An item delegate for hop tables.
 *  \sa HopTableModel
 */
class HopItemDelegate : public QItemDelegate
{
   Q_OBJECT

public:
   HopItemDelegate(QObject* parent = nullptr);

   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

#endif
