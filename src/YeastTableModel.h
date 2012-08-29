/*
 * YeastTableModel.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _YEASTTABLEMODEL_H
#define   _YEASTTABLEMODEL_H

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

// Forward declarations.
class Yeast;
class YeastTableWidget;
class Recipe;

enum{ YEASTNAMECOL, YEASTLABCOL, YEASTPRODIDCOL, YEASTTYPECOL, YEASTFORMCOL, YEASTAMOUNTCOL, YEASTNUMCOLS /*This one MUST be last*/};

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
   YeastTableModel(QTableView* parent=0);
   virtual ~YeastTableModel() {}
   //! Observe a recipe's list of fermentables.
   void observeRecipe(Recipe* rec);
   //! Whether or not we should be looking at the database.
   void observeDatabase(bool val);
   void addYeasts(QList<Yeast*> yeasts);
   Yeast* getYeast(unsigned int i);
   void removeAll();

   // Inherit the following from QAbstractItemModel via QAbstractTableModel
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

   unitDisplay displayUnit(int column) const;
   unitScale displayScale(int column) const;
   void setDisplayUnit(int column, unitDisplay displayUnit);
   void setDisplayScale(int column, unitScale displayScale);
   QString generateName(int column) const;

public slots:
   void changed(QMetaProperty,QVariant);
   void addYeast(Yeast* yeast);
   void removeYeast(Yeast* yeast);
   
private:
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
   YeastItemDelegate(QObject* parent = 0);

   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

#endif   /* _YEASTTABLEMODEL_H */

