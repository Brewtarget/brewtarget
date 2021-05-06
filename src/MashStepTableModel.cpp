/*
 * MashStepTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2020
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
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

#include <QAbstractTableModel>
#include <QWidget>
#include <QModelIndex>
#include <QVariant>
#include <QTableView>
#include <QItemDelegate>
#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <QVector>
#include <QHeaderView>
#include "database.h"
#include "model/MashStep.h"
#include "MashStepTableModel.h"
#include "unit.h"
#include "brewtarget.h"
#include "SimpleUndoableUpdate.h"
#include "MainWindow.h"

MashStepTableModel::MashStepTableModel(QTableView* parent)
   : QAbstractTableModel(parent),
     mashObs(nullptr),
     parentTableWidget(parent)
{
   setObjectName("mashStepTableModel");

   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(headerView, &QWidget::customContextMenuRequested, this, &MashStepTableModel::contextMenu);
}

void MashStepTableModel::addMashStep(MashStep * mashStep)
{
   qDebug() << QString("%1").arg(Q_FUNC_INFO);

   if (mashStep == nullptr || this->steps.contains(mashStep)) {
      return;
   }

   int size {this->steps.size()};
   beginInsertRows( QModelIndex(), size, size );
   this->steps.append(mashStep);
   connect( mashStep, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
   return;
}

bool MashStepTableModel::removeMashStep(MashStep * mashStep)
{
   qDebug() << QString("%1").arg(Q_FUNC_INFO);

   int i {this->steps.indexOf(mashStep)};
   if( i >= 0 )
   {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( mashStep, nullptr, this, nullptr );
      this->steps.removeAt(i);
      //reset(); // Tell everybody the table has changed.
      endRemoveRows();

      return true;
   }

   return false;
}

void MashStepTableModel::setMash( Mash* m )
{
   int i;
   if( mashObs && steps.size() > 0)
   {
      beginRemoveRows( QModelIndex(), 0, steps.size()-1 );
      // Remove mashObs and all steps.
      disconnect( mashObs, nullptr, this, nullptr );
      for( i = 0; i < steps.size(); ++i )
         disconnect( steps[i], nullptr, this, nullptr );
      steps.clear();
      endRemoveRows();
   }

   mashObs = m;
   if( mashObs )
   {
      // This has to happen outside of the if{} block to make sure the mash
      // signal is connected. Otherwise, empty mashes will never be not empty.
      connect( mashObs, &Mash::mashStepsChanged, this, &MashStepTableModel::mashChanged );

      QList<MashStep*> tmpSteps = mashObs->mashSteps();
      if(tmpSteps.size() > 0){
         beginInsertRows( QModelIndex(), 0, tmpSteps.size()-1 );
         steps = tmpSteps;
         for( i = 0; i < steps.size(); ++i )
            connect( steps[i], SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(mashStepChanged(QMetaProperty,QVariant)) );
         endInsertRows();
     }
   }

   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void MashStepTableModel::reorderMashStep(MashStep* step, int current)
{
   // doSomething will be -1 if we are moving up and 1 if we are moving down
   // and 0 if nothing is to be done (see next comment)
   int doSomething = step->stepNumber() - current - 1;
   int destChild   = step->stepNumber();

   // Moving a step up or down generates two signals, one for each row
   // impacted. If we move row B above row A:
   //    1. The first signal is to move B above A, which will result in A
   //    being below B
   //    2. The second signal is to move A below B, which we just did.
   // Therefore, the second signal mostly needs to be ignored. In those
   // circusmtances, A->stepNumber() will be the same as it's position in the
   // steps list, modulo some indexing
   if ( doSomething == 0 )
      return;

   // beginMoveRows is a little odd. When moving rows within the same parent,
   // destChild points one beyond where you want to insert the row. Think of
   // it as saying "insert before destChild". If we are moving something up,
   // we need to be one less than stepNumber. If we are moving down, it just
   // works.
   if ( doSomething < 0 )
      destChild--;

   beginMoveRows(QModelIndex(),current,current,QModelIndex(),destChild);
   // doSomething is -1 if moving up and 1 if moving down. swap current with
   // current -1 when moving up, and swap current with current+1 when moving
   // down
#if QT_VERSION < QT_VERSION_CHECK(5,13,0)
   steps.swap(current,current+doSomething);
#else
   steps.swapItemsAt(current,current+doSomething);
#endif
   endMoveRows();

}

MashStep* MashStepTableModel::getMashStep(unsigned int i)
{
   if( i < static_cast<unsigned int>(steps.size()) )
      return steps[static_cast<int>(i)];
   else
      return nullptr;
}

void MashStepTableModel::mashChanged()
{
   // Remove and re-add all steps.
   setMash( mashObs );
}

void MashStepTableModel::mashStepChanged(QMetaProperty prop, QVariant val)
{
   int i;

   MashStep* stepSender = qobject_cast<MashStep*>(sender());
   if( stepSender && (i = steps.indexOf(stepSender)) >= 0 )
   {
      if ( prop.name() == QString(PropertyNames::MashStep::stepNumber) ) {
         reorderMashStep(stepSender,i);
      }

      emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                        QAbstractItemModel::createIndex(i, MASHSTEPNUMCOLS-1));
   }

   if( parentTableWidget )
   {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

int MashStepTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return steps.size();
}

int MashStepTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return MASHSTEPNUMCOLS;
}

QVariant MashStepTableModel::data( const QModelIndex& index, int role ) const
{
   MashStep* row;
   Unit::unitDisplay unit;
   Unit::unitScale scale;
   int col = index.column();

   if( mashObs == nullptr )
      return QVariant();

   // Ensure the row is ok.
   if( index.row() >= static_cast<int>(steps.size()) )
   {
      qWarning() << tr("Bad model index. row = %1").arg(index.row());
      return QVariant();
   }
   else
      row = steps[index.row()];

   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();

   switch( col )
   {
      case MASHSTEPNAMECOL:
         return QVariant(row->name());
      case MASHSTEPTYPECOL:
         return QVariant(row->typeStringTr());
      case MASHSTEPAMOUNTCOL:

         unit = displayUnit(col);
         scale = displayScale(col);

         return (row->type() == MashStep::Decoction)
                ? QVariant( Brewtarget::displayAmount(row->decoctionAmount_l(), Units::liters, 3, unit, scale ) )
                : QVariant( Brewtarget::displayAmount(row->infuseAmount_l(), Units::liters, 3, unit, scale) );
      case MASHSTEPTEMPCOL:
         unit = displayUnit(col);
         return (row->type() == MashStep::Decoction)
                ? QVariant("---")
                : QVariant( Brewtarget::displayAmount(row->infuseTemp_c(), Units::celsius, 3, unit, Unit::noScale) );
      case MASHSTEPTARGETTEMPCOL:
         unit = displayUnit(col);
         return QVariant( Brewtarget::displayAmount(row->stepTemp_c(), Units::celsius,3, unit, Unit::noScale) );
      case MASHSTEPTIMECOL:
         scale = displayScale(col);
         return QVariant( Brewtarget::displayAmount(row->stepTime_min(), Units::minutes,3,Unit::noUnit,scale) );
      default :
         qWarning() << tr("Bad column: %1").arg(index.column());
         return QVariant();
   }
}

QVariant MashStepTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
   {
      switch( section )
      {
         case MASHSTEPNAMECOL:
            return QVariant(tr("Name"));
         case MASHSTEPTYPECOL:
            return QVariant(tr("Type"));
         case MASHSTEPAMOUNTCOL:
            return QVariant(tr("Amount"));
         case MASHSTEPTEMPCOL:
            return QVariant(tr("Infusion Temp"));
         case MASHSTEPTARGETTEMPCOL:
            return QVariant(tr("Target Temp"));
         case MASHSTEPTIMECOL:
            return QVariant(tr("Time"));
         default:
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags MashStepTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();
   switch(col)
   {
      case MASHSTEPNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      default:
         return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool MashStepTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   MashStep *row;

   if( mashObs == nullptr )
      return false;

   if( index.row() >= static_cast<int>(steps.size()) || role != Qt::EditRole )
      return false;
   else
      row = steps[index.row()];

   Unit::unitDisplay dspUnit = displayUnit(index.column());
   Unit::unitScale   dspScl  = displayScale(index.column());

   switch( index.column() )
   {
      case MASHSTEPNAMECOL:
         if( value.canConvert(QVariant::String))
         {
            Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                     PropertyNames::NamedEntity::name,
                                                     value.toString(),
                                                     tr("Change Mash Step Name"));
            return true;
         }
         else
            return false;
      case MASHSTEPTYPECOL:
         if( value.canConvert(QVariant::Int) )
         {
            Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                     "type",
                                                     static_cast<MashStep::Type>(value.toInt()),
                                                     tr("Change Mash Step Type"));
            return true;
         }
         else
            return false;
      case MASHSTEPAMOUNTCOL:
         if( value.canConvert(QVariant::String) )
         {
            if( row->type() == MashStep::Decoction ) {
               Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                        PropertyNames::MashStep::decoctionAmount_l,
                                                        Brewtarget::qStringToSI(value.toString(),Units::liters,dspUnit,dspScl),
                                                        tr("Change Mash Step Decoction Amount"));
            } else {
               Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                        PropertyNames::MashStep::infuseAmount_l,
                                                        Brewtarget::qStringToSI(value.toString(),Units::liters,dspUnit,dspScl),
                                                        tr("Change Mash Step Infuse Amount"));
            }
            return true;
         }
         else
            return false;
      case MASHSTEPTEMPCOL:
         if( value.canConvert(QVariant::String) && row->type() != MashStep::Decoction )
         {
            Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                      PropertyNames::MashStep::infuseTemp_c,
                                                      Brewtarget::qStringToSI(value.toString(),Units::celsius,dspUnit,dspScl),
                                                      tr("Change Mash Step Infuse Temp"));
            return true;
         }
         else
            return false;
      case MASHSTEPTARGETTEMPCOL:
         if( value.canConvert(QVariant::String) )
         {
            // Two changes, but we want to group together as one undo/redo step
            //
            // We don't assign the pointer (returned by new) to second SimpleUndoableUpdate we create because, in the
            // constructor, it gets linked to the first one, which then "owns" it.
            auto targetTempUpdate = new SimpleUndoableUpdate(*row,
                                                             PropertyNames::MashStep::stepTemp_c,
                                                             Brewtarget::qStringToSI(value.toString(),Units::celsius,dspUnit,dspScl),
                                                             tr("Change Mash Step Temp"));
            new SimpleUndoableUpdate(*row,
                                     PropertyNames::MashStep::endTemp_c,
                                     Brewtarget::qStringToSI(value.toString(),Units::celsius,dspUnit,dspScl),
                                     tr("Change Mash Step End Temp"),
                                     targetTempUpdate);
            Brewtarget::mainWindow()->doOrRedoUpdate(targetTempUpdate);
            return true;
         }
         else
            return false;
      case MASHSTEPTIMECOL:
         if( value.canConvert(QVariant::String) )
         {
            Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                      PropertyNames::MashStep::stepTime_min,
                                                      Brewtarget::qStringToSI(value.toString(),Units::minutes,dspUnit,dspScl),
                                                      tr("Change Mash Step Time"));
            return true;
         }
         else
            return false;
      default:
         return false;
   }
}

void MashStepTableModel::moveStepUp(int i)
{
   if( mashObs == nullptr || i == 0 || i >= steps.size() )
      return;

   Database::instance().swapMashStepOrder( steps[i], steps[i-1] );
}

void MashStepTableModel::moveStepDown(int i)
{
   if( mashObs == nullptr ||  i+1 >= steps.size() )
      return;

   Database::instance().swapMashStepOrder( steps[i], steps[i+1] );
}

Unit::unitDisplay MashStepTableModel::displayUnit(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noUnit;

   return static_cast<Unit::unitDisplay>(Brewtarget::option(attribute, Unit::noUnit, this->objectName(), Brewtarget::UNIT).toInt());
}

Unit::unitScale MashStepTableModel::displayScale(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noScale;

   return static_cast<Unit::unitScale>(Brewtarget::option(attribute, Unit::noScale, this->objectName(), Brewtarget::SCALE).toInt());
}

// We need to:
//   o clear the custom scale if set
//   o clear any custom unit from the rows
//      o which should have the side effect of clearing any scale
void MashStepTableModel::setDisplayUnit(int column, Unit::unitDisplay displayUnit)
{
   // MashStep* row; // disabled per-cell magic
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayUnit,this->objectName(),Brewtarget::UNIT);
   Brewtarget::setOption(attribute,Unit::noScale,this->objectName(),Brewtarget::SCALE);

   /* Disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getMashStep(i);
      row->setDisplayUnit(Unit::noUnit);
   }
   */
}

// Setting the scale should clear any cell-level scaling options
void MashStepTableModel::setDisplayScale(int column, Unit::unitScale displayScale)
{
   // MashStep* row; //disabled per-cell magic

   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayScale,this->objectName(),Brewtarget::SCALE);

   /* disabled cell-specific code
   for (int i = 0; i < rowCount(); ++i )
   {
      row = getMashStep(i);
      row->setDisplayScale(Unit::noScale);
   }
   */
}

QString MashStepTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case MASHSTEPAMOUNTCOL:
         attribute = "amount";
         break;
      case MASHSTEPTEMPCOL:
         attribute = PropertyNames::MashStep::infuseTemp_c;
         break;
      case MASHSTEPTARGETTEMPCOL:
         attribute = PropertyNames::MashStep::stepTemp_c;
         break;
      case MASHSTEPTIMECOL:
         attribute = PropertyNames::Misc::time;
         break;
      default:
         attribute = "";
   }
   return attribute;
}

void MashStepTableModel::contextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   Unit::unitDisplay currentUnit;
   Unit::unitScale  currentScale;

   // Since we need to call generateVolumeMenu() two different ways, we need
   // to figure out the currentUnit and Scale here

   currentUnit  = displayUnit(selected);
   currentScale = displayScale(selected);

   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case MASHSTEPAMOUNTCOL:
         menu = Brewtarget::setupVolumeMenu(parentTableWidget,currentUnit, currentScale);
         break;
      case MASHSTEPTEMPCOL:
      case MASHSTEPTARGETTEMPCOL:
         menu = Brewtarget::setupTemperatureMenu(parentTableWidget,currentUnit);
         break;
      case MASHSTEPTIMECOL:
         menu = Brewtarget::setupTimeMenu(parentTableWidget,currentScale);
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == nullptr )
      return;

   QWidget* pMenu = invoked->parentWidget();
   if ( pMenu == menu )
      setDisplayUnit(selected,static_cast<Unit::unitDisplay>(invoked->data().toInt()));
   else
      setDisplayScale(selected,static_cast<Unit::unitScale>(invoked->data().toInt()));
}

//==========================CLASS MashStepItemDelegate===============================

MashStepItemDelegate::MashStepItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}

QWidget* MashStepItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
   if( index.column() == MASHSTEPTYPECOL )
   {
      QComboBox *box = new QComboBox(parent);

      foreach( QString mtype, MashStep::types )
         box->addItem(mtype);

      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      return box;
   }
   else
      return new QLineEdit(parent);
}

void MashStepItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   if( index.column() == MASHSTEPTYPECOL )
   {
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      QString text = index.model()->data(index, Qt::DisplayRole).toString();

      int index = box->findText(text);
      box->setCurrentIndex(index);
   }
   else
   {
      QLineEdit* line = qobject_cast<QLineEdit*>(editor);

      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }

}

void MashStepItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   QStringList typesTr = QStringList() << QObject::tr("Infusion") << QObject::tr("Temperature") << QObject::tr("Decoction");
   if( index.column() == MASHSTEPTYPECOL )
   {
      QComboBox* box = qobject_cast<QComboBox*>(editor);
      int ndx = box->currentIndex();
      int curr  = typesTr.indexOf(model->data(index,Qt::DisplayRole).toString());

      if ( ndx != curr )
         model->setData(index, ndx, Qt::EditRole);
   }
   else
   {
      QLineEdit* line = qobject_cast<QLineEdit*>(editor);

      if ( line->isModified() )
         model->setData(index, line->text(), Qt::EditRole);
   }
}

void MashStepItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}
