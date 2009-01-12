
#ifndef _MASHSTEPTABLEMODEL_H
#define	_MASHSTEPTABLEMODEL_H

#include <QAbstractTableModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <QItemDelegate>
#include <vector>
#include "mashstep.h"
#include "observable.h"

class MashStepTableModel;
class MashStepItemDelegate;

enum{ MASHSTEPNAMECOL, MASHSTEPTYPECOL, MASHSTEPAMOUNTCOL, MASHSTEPTEMPCOL, MASHSTEPTIMECOL, MASHSTEPNUMCOLS /*This one MUST be last*/};

class MashStepTableModel : public QAbstractTableModel, public MultipleObserver
{
   Q_OBJECT

public:
   MashStepTableModel(QWidget* parent=0);
   void addMashStep(MashStep* step);
   bool removeMashStep(MashStep* step); // Returns true if "step" is successfully found and removed.
   virtual void notify(Observable* notifier); // Inherited from Observer via MultipleObserver.

   // Inherit the following from QAbstractItemModel via QAbstractTableModel
   virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
   virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
   virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
   virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
   virtual Qt::ItemFlags flags(const QModelIndex& index ) const;
   virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

private:
   std::vector<MashStep*> stepObs;
};

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

#endif	/* _MASHSTEPTABLEMODEL_H */

