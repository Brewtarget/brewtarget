/*
 * FermentableTableModel.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#ifndef _FERMENTABLETABLEMODEL_H
#define   _FERMENTABLETABLEMODEL_H

class FermentableTableModel;
class FermentableItemDelegate;

#include <QAbstractTableModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QAbstractItemDelegate>
#include <QVector>
#include "fermentable.h"
#include "FermentableTableWidget.h"
#include "observable.h"

enum{FERMNAMECOL, FERMTYPECOL, FERMAMOUNTCOL, FERMISMASHEDCOL, FERMAFTERBOIL, FERMYIELDCOL, FERMCOLORCOL, FERMNUMCOLS /*This one MUST be last*/};

class FermentableTableModel : public QAbstractTableModel, public MultipleObserver
{
   Q_OBJECT
           
public:
   FermentableTableModel(FermentableTableWidget* parent=0);
   virtual ~FermentableTableModel() {}
   void addFermentable(Fermentable* ferm);
   bool removeFermentable(Fermentable* ferm); // Returns true if "ferm" is successfully found and removed.
   void removeAll();
   Fermentable* getFermentable(unsigned int i);
   void setDisplayPercentages( bool var );
   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer via MultipleObserver.
   
   // Inherit the following from QAbstractItemModel via QAbstractTableModel
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
   
   FermentableTableWidget* parentTableWidget;
   
private:
   void updateTotalGrains();
   
   QVector<Fermentable*> fermObs;
   bool displayPercentages; // True if you want to display percent of each grain in the row header.
   double totalFermMass_kg;
};

class FermentableItemDelegate : public QStyledItemDelegate
{
   Q_OBJECT
           
public:
   FermentableItemDelegate(QObject* parent = 0);
   
   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   //virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
   
public slots:
   void destroyWidget(QWidget* widget, QAbstractItemDelegate::EndEditHint hint);
private:
};

#endif   /* _FERMENTABLETABLEMODEL_H */

