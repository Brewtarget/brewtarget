/*
 * FermentableTableModel.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _FERMENTABLETABLEMODEL_H
#define   _FERMENTABLETABLEMODEL_H

class FermentableTableModel;
class FermentableItemDelegate;

#include <QAbstractTableModel>
#include <QTableView>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <QMetaProperty>
#include <QStyledItemDelegate>
#include <QAbstractItemDelegate>
#include <QList>

// Forward declarations.
class Fermentable;
class Recipe;

enum{FERMNAMECOL, FERMTYPECOL, FERMAMOUNTCOL, FERMISMASHEDCOL, FERMAFTERBOIL, FERMYIELDCOL, FERMCOLORCOL, FERMNUMCOLS /*This one MUST be last*/};

/*!
 * \class FermentableTableModel
 * \author Philip G. Lee
 *
 * A table model for a list of fermentables.
 */
class FermentableTableModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   FermentableTableModel(QTableView* parent=0);
   virtual ~FermentableTableModel() {}
   //! Observe a recipe's list of fermentables.
   void observeRecipe(Recipe* rec);
   //! Whether or not we should be looking at the database.
   void observeDatabase(bool val);
   //! Watch \b ferm for changes.
   void addFermentable(Fermentable* ferm);
   //! Watch all the \b ferms for changes.
   void addFermentables(QList<Fermentable*> ferms);
   //! \returns true if "ferm" is successfully found and removed.
   bool removeFermentable(Fermentable* ferm);
   void removeAll();
   Fermentable* getFermentable(unsigned int i);
   //! True if you want to display percent of each grain in the row header.
   void setDisplayPercentages( bool var );
   
   // Inherit the following from QAbstractItemModel via QAbstractTableModel
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
   
   QTableView* parentTableWidget;
   
public slots:
   void changed(QMetaProperty, QVariant);
private:
   void updateTotalGrains();
   
   QList<Fermentable*> fermObs;
   Recipe* recObs;
   bool displayPercentages;
   double totalFermMass_kg;
};

class FermentableItemDelegate : public QStyledItemDelegate
{
   Q_OBJECT
           
public:
   FermentableItemDelegate(QObject* parent = 0);
   
   //! Reimplemented from QStyledItemDelegate.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   //! Reimplemented from QStyledItemDelegate.
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   //! Reimplemented from QStyledItemDelegate.
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   //! Reimplemented from QStyledItemDelegate.
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   //virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
   
public slots:
   void destroyWidget(QWidget* widget, QAbstractItemDelegate::EndEditHint hint);
private:
};

#endif   /* _FERMENTABLETABLEMODEL_H */

