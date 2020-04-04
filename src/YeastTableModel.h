/*
 * YeastTableModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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

#ifndef _YEASTTABLEMODEL_H
#define _YEASTTABLEMODEL_H

class YeastTableModel;
class YeastItemDelegate;

#include <QAbstractTableModel>
#include <QWidget>
#include <QModelIndex>
#include <QMetaProperty>
#include <QVariant>
#include <QItemDelegate>
#include <QList>
#include <QTableView>

#include "unit.h"
#include "brewtarget.h"

// Forward declarations.
class Yeast;
class YeastTableWidget;
class Recipe;

enum{ YEASTNAMECOL, YEASTLABCOL, YEASTPRODIDCOL, YEASTTYPECOL, YEASTFORMCOL, YEASTAMOUNTCOL, YEASTINVENTORYCOL, YEASTNUMCOLS /*This one MUST be last*/};

/*!
 * \class YeastTableModel
 * \author Philip G. Lee
 *
 * \brief Table model for yeasts.
 */
class YeastTableModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   YeastTableModel(QTableView* parent=nullptr, bool editable=true);
   virtual ~YeastTableModel() {}
   //! \brief Observe a recipe's list of fermentables.
   void observeRecipe(Recipe* rec);
   //! \brief If true, we model the database's list of yeasts.
   void observeDatabase(bool val);
   //! \brief Add \c yeasts to the model.
   void addYeasts(QList<Yeast*> yeasts);
   //! \brief Get the yeast at model index \c i.
   Yeast* getYeast(unsigned int i);
   //! \brief Clear the model.
   void removeAll();

   /*!
    * \brief True if the inventory column should be editable, false otherwise.
    *
    * The default is that the inventory column is not editable
    */
   void setInventoryEditable( bool var ) { _inventoryEditable = var; }

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

   Unit::unitDisplay displayUnit(int column) const;
   Unit::unitScale displayScale(int column) const;
   void setDisplayUnit(int column, Unit::unitDisplay displayUnit);
   void setDisplayScale(int column, Unit::unitScale displayScale);
   QString generateName(int column) const;

public slots:
   //! \brief Add a \c yeast to the model.
   void addYeast(Yeast* yeast);
   //! \brief Remove a \c yeast from the model.
   void removeYeast(Yeast* yeast);

   void contextMenu(const QPoint &point);
private slots:
   //! \brief Catch changes to Recipe, Database, and Yeast.
   void changed(QMetaProperty, QVariant);
   void changedInventory(Brewtarget::DBTable,int,QVariant);

private:
   bool editable;
   bool _inventoryEditable;
   QList<Yeast*> yeastObs;
   QTableView* parentTableWidget;
   Recipe* recObs;
};

/*!
 * \class YeastItemDelegate
 * \author Philip G. Lee
 *
 * Item delegate for yeast tables.
 */
class YeastItemDelegate : public QItemDelegate
{
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

#endif   /* _YEASTTABLEMODEL_H */

