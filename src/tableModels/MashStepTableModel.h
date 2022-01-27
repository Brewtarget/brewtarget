/*
 * MashStepTableModel.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef TABLEMODELS_MASHSTEPTABLEMODEL_H
#define TABLEMODELS_MASHSTEPTABLEMODEL_H

#include <QItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include "measurement/Unit.h"
#include "model/MashStep.h"
#include "model/Mash.h"
#include "tableModels/BtTableModel.h"

class MashStepItemDelegate;

enum{ MASHSTEPNAMECOL, MASHSTEPTYPECOL, MASHSTEPAMOUNTCOL, MASHSTEPTEMPCOL, MASHSTEPTARGETTEMPCOL, MASHSTEPTIMECOL, MASHSTEPNUMCOLS /*This one MUST be last*/};

/*!
 * \class MashStepTableModel
 *
 * \brief Model for the list of mash steps in a mash.
 */
class MashStepTableModel : public BtTableModel {
   Q_OBJECT

public:
   MashStepTableModel(QTableView* parent = nullptr);
   virtual ~MashStepTableModel();

   //! Set the mash whose mash steps we want to model.
   void setMash( Mash* m );

   Mash * getMash() const;

   //! \returns the mash step at model index \b i.
   MashStep* getMashStep(unsigned int i);

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

   //! \returns true if mashStep is successfully found and removed.
   bool remove(MashStep * MashStep);

public slots:
   //! \brief Add a MashStep to the model.
   void addMashStep(int mashStep);

   void removeMashStep(int mashStepId, std::shared_ptr<QObject> object);

   void moveStepUp(int i);
   void moveStepDown(int i);
   void mashChanged();
   void mashStepChanged(QMetaProperty,QVariant);

private:
   Mash* mashObs;
   QList<MashStep*> steps;

   void reorderMashStep(MashStep *step, int current);
};

/*!
 * \class MashStepItemDelegate
 *
 * An item delegate for mash step tables.
 */
class MashStepItemDelegate : public QItemDelegate {
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

#endif
