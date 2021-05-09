/*
 * MashStepTableModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Mik Firestone <mikfire@gmail.com>
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

#ifndef _MASHSTEPTABLEMODEL_H
#define   _MASHSTEPTABLEMODEL_H

class MashStepTableModel;
class MashStepItemDelegate;

#include <QAbstractTableModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <QMetaProperty>
#include <QVariant>
#include <QItemDelegate>
#include <QVector>
#include <QTableView>

#include "model/MashStep.h"
#include "model/Mash.h"
#include "Unit.h"

enum{ MASHSTEPNAMECOL, MASHSTEPTYPECOL, MASHSTEPAMOUNTCOL, MASHSTEPTEMPCOL, MASHSTEPTARGETTEMPCOL, MASHSTEPTIMECOL, MASHSTEPNUMCOLS /*This one MUST be last*/};

/*!
 * \class MashStepTableModel
 * \author Philip G. Lee
 *
 * \brief Model for the list of mash steps in a mash.
 */
class MashStepTableModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   MashStepTableModel(QTableView* parent=0);
   virtual ~MashStepTableModel() {}
   //! Set the mash whose mash steps we want to model.
   void setMash( Mash* m );
   //! \returns the mash step at model index \b i.
   MashStep* getMashStep(unsigned int i);

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

   Unit::unitDisplay displayUnit(int column) const;
   Unit::unitScale displayScale(int column) const;
   void setDisplayUnit(int column, Unit::unitDisplay displayUnit);
   void setDisplayScale(int column, Unit::unitScale displayScale);
   QString generateName(int column) const;

public slots:
   //! \brief Add a MashStep to the model.
   void addMashStep(MashStep * mashStep);
   //! \returns true if mashStep is successfully found and removed.
   bool removeMashStep(MashStep * MashStep);

   void moveStepUp(int i);
   void moveStepDown(int i);
   void mashChanged();
   void mashStepChanged(QMetaProperty,QVariant);

   void contextMenu(const QPoint &point);

private:
   Mash* mashObs;
   QTableView* parentTableWidget;
   QList<MashStep*> steps;

//   void reorderMashSteps();
   void reorderMashStep(MashStep *step, int current);
};

/*!
 * \class MashStepItemDelegate
 * \author Philip G. Lee
 *
 * An item delegate for mash step tables.
 */
class MashStepItemDelegate : public QItemDelegate
{
   Q_OBJECT

public:
   MashStepItemDelegate(QObject* parent = 0);

   // Inherited functions.
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
};

#endif   /* _MASHSTEPTABLEMODEL_H */

