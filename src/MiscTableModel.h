/*
 * MiscTableModel.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MISCTABLEMODEL_H
#define _MISCTABLEMODEL_H

class MiscTableModel;
class MiscItemDelegate;

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QList>
#include <QMetaProperty>
#include <QTableView>

#include "unit.h"

// Forward declarations.
class Misc;
class MiscTableWidget;
class Recipe;

enum{MISCNAMECOL, MISCTYPECOL, MISCUSECOL, MISCTIMECOL, MISCAMOUNTCOL, MISCNUMCOLS /*This one MUST be last*/};

/*!
 * \class MiscTableModel
 * \author Philip G. Lee
 *
 * \brief Table model for a list of miscs.
 */
class MiscTableModel : public QAbstractTableModel
{
   Q_OBJECT
   
public:
   MiscTableModel(QTableView* parent=0, bool editable=true);
   virtual ~MiscTableModel() {}
   //! \brief Observe a recipe's list of miscs.
   void observeRecipe(Recipe* rec);
   //! \brief If true, we model the database's list of miscs.
   void observeDatabase(bool val);
   //! \brief Add \c miscs to the model.
   void addMiscs(QList<Misc*> miscs);
   //! \returns the \c Misc at model index \b i.
   Misc* getMisc(unsigned int i);
   //! \brief Clear the model.
   void removeAll();
   
   //! \brief Reimplemented from QAbstractTableModel
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   //! \brief Reimplemented from QAbstractTableModel
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

   unitDisplay displayUnit(int column) const;
   unitScale displayScale(int column) const;
   void setDisplayUnit(int column, unitDisplay displayUnit);
   void setDisplayScale(int column, unitScale displayScale);
   QString generateName(int column) const;

public slots:
   //! \brief Add a misc to the model.
   void addMisc(Misc* misc);
   //! \brief Remove a misc from the model.
   bool removeMisc(Misc* misc);

private slots:
   //! \brief Catch changes to Recipe, Database, and Misc.
   void changed(QMetaProperty, QVariant);

private:
   bool editable;
   QList<Misc*> miscObs;
   Recipe* recObs;
   QTableView* parentTableWidget;
};

/*!
 *  \class MiscItemDelegate
 *  \author Philip G. Lee
 *
 *  \brief Item delegate for misc tables.
 *  \sa MiscTableModel
 */
class MiscItemDelegate : public QItemDelegate
{
   Q_OBJECT
           
public:
   MiscItemDelegate(QObject* parent = 0);
   
   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   
private:
};

#endif   /* _MISCTABLEMODEL_H */
