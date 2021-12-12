/*
 * MashStepTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#include "MashStepTableModel.h"

#include <QAbstractTableModel>
#include <QComboBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QModelIndex>
#include <QObject>
#include <QTableView>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include "brewtarget.h"
#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "model/MashStep.h"
#include "PersistentSettings.h"
#include "SimpleUndoableUpdate.h"
#include "Unit.h"

MashStepTableModel::MashStepTableModel(QTableView* parent)
   : QAbstractTableModel(parent),
     mashObs(nullptr),
     parentTableWidget(parent) {
   setObjectName("mashStepTableModel");

   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(headerView, &QWidget::customContextMenuRequested, this, &MashStepTableModel::contextMenu);
   connect(&ObjectStoreTyped<MashStep>::getInstance(), &ObjectStoreTyped<MashStep>::signalObjectInserted, this, &MashStepTableModel::addMashStep);
   connect(&ObjectStoreTyped<MashStep>::getInstance(), &ObjectStoreTyped<MashStep>::signalObjectDeleted,  this, &MashStepTableModel::removeMashStep);
   return;
}

void MashStepTableModel::addMashStep(int mashStepId) {
   // A new MashStep has been inserted in the DB, so we've been sent a signal.  If the MashStep doesn't exist(!?!), or
   // if we already have it in our list, or if it's not for the the Mash we're watching, or we're not watching a Mash,
   // then there is nothing for us to do.
   MashStep * mashStep = ObjectStoreWrapper::getByIdRaw<MashStep>(mashStepId);
   if (mashStep == nullptr ||
       this->mashObs == nullptr ||
       this->steps.contains(mashStep) ||
       this->mashObs->key() != mashStep->getMashId()) {
      return;
   }

   int size {this->steps.size()};
   qDebug() <<
      Q_FUNC_INFO << "Instance @" << static_cast<void *>(this) << "Adding MashStep" << mashStep->name() << "(#" <<
      mashStepId << ") to existing list of " << size << "steps for Mash #" << this->mashObs->key();

   beginInsertRows( QModelIndex(), size, size );
   this->steps.append(mashStep);
   connect(mashStep, &NamedEntity::changed, this, &MashStepTableModel::mashStepChanged);
   //reset(); // Tell everybody that the table has changed.
   endInsertRows();
   return;
}

void MashStepTableModel::removeMashStep(int mashStepId, std::shared_ptr<QObject> object) {
   this->remove(std::static_pointer_cast<MashStep>(object).get());
   return;
}

bool MashStepTableModel::remove(MashStep * mashStep) {

   int i {this->steps.indexOf(mashStep)};
   if (i >= 0) {
      qDebug() << Q_FUNC_INFO << "Removing MashStep" << mashStep->name() << "(#" << mashStep->key() << ")";
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( mashStep, nullptr, this, nullptr );
      this->steps.removeAt(i);
      //reset(); // Tell everybody the table has changed.
      endRemoveRows();

      return true;
   }

   return false;
}

void MashStepTableModel::setMash(Mash * m) {
   if (this->mashObs && this->steps.size() > 0) {
      qDebug() <<
         Q_FUNC_INFO << "Removing" << this->steps.size() << "MashStep rows for old Mash #" << this->mashObs->key();
      this->beginRemoveRows(QModelIndex(), 0, this->steps.size() - 1);
      // Remove mashObs and all steps.
      disconnect( mashObs, nullptr, this, nullptr );
      for(int i = 0; i < this->steps.size(); ++i) {
         disconnect(this->steps[i], nullptr, this, nullptr);
      }
      this->steps.clear();
      this->endRemoveRows();
   }

   this->mashObs = m;
   if (this->mashObs) {
      qDebug() << Q_FUNC_INFO << "Now watching Mash #" << this->mashObs->key();

      // This has to happen outside of the if{} block to make sure the mash
      // signal is connected. Otherwise, empty mashes will never be not empty.
      connect( mashObs, &Mash::mashStepsChanged, this, &MashStepTableModel::mashChanged );

      QList<MashStep*> tmpSteps = this->mashObs->mashSteps();
      if (tmpSteps.size() > 0){
         qDebug() << Q_FUNC_INFO << "Inserting" << tmpSteps.size() << "MashStep rows";
         this->beginInsertRows(QModelIndex(), 0, tmpSteps.size() - 1);
         this->steps = tmpSteps;
         for (int i = 0; i < this->steps.size(); ++i ) {
            connect(this->steps[i], &NamedEntity::changed, this, &MashStepTableModel::mashStepChanged);
         }
         this->endInsertRows();
     }
   }

   if (parentTableWidget) {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
   return;
}

Mash * MashStepTableModel::getMash() const {
   return this->mashObs;
}

void MashStepTableModel::reorderMashStep(MashStep* step, int current) {

   // doSomething will be -1 if we are moving up and 1 if we are moving down
   // and 0 if nothing is to be done (see next comment)
   int destChild   = step->stepNumber();
   int doSomething = destChild - current - 1;

   qDebug() << Q_FUNC_INFO << "Swapping" << destChild << "with" << current << ", so doSomething=" << doSomething;

   // Moving a step up or down generates two signals, one for each row
   // impacted. If we move row B above row A:
   //    1. The first signal is to move B above A, which will result in A
   //    being below B
   //    2. The second signal is to move A below B, which we just did.
   // Therefore, the second signal mostly needs to be ignored. In those
   // circusmtances, A->stepNumber() will be the same as it's position in the
   // steps list, modulo some indexing
   if ( doSomething == 0 ) {
      return;
   }

   // beginMoveRows is a little odd. When moving rows within the same parent,
   // destChild points one beyond where you want to insert the row. Think of
   // it as saying "insert before destChild". If we are moving something up,
   // we need to be one less than stepNumber. If we are moving down, it just
   // works.
   if ( doSomething < 0 ) {
      destChild--;
   }

   // We assert that we are swapping valid locations on the list as, to do otherwise implies a coding error
   qDebug() <<
      Q_FUNC_INFO << "Swap" << current + doSomething << "with" << current << ", in list of " << this->steps.size();
   Q_ASSERT(current >= 0);
   Q_ASSERT(current + doSomething >= 0);
   Q_ASSERT(current < this->steps.size());
   Q_ASSERT(current + doSomething < this->steps.size());

   this->beginMoveRows(QModelIndex(), current, current, QModelIndex(), destChild);
   // doSomething is -1 if moving up and 1 if moving down. swap current with
   // current -1 when moving up, and swap current with current+1 when moving
   // down
#if QT_VERSION < QT_VERSION_CHECK(5,13,0)
   this->steps.swap(current, current+doSomething);
#else
   this->steps.swapItemsAt(current, current+doSomething);
#endif
   this->endMoveRows();
   return;
}

MashStep* MashStepTableModel::getMashStep(unsigned int i) {
   if ( i < static_cast<unsigned int>(this->steps.size()) ) {
      return this->steps[static_cast<int>(i)];
   }

   return nullptr;
}

void MashStepTableModel::mashChanged() {
   // Remove and re-add all steps.
   this->setMash(this->mashObs);
   return;
}

void MashStepTableModel::mashStepChanged(QMetaProperty prop, QVariant val) {
   qDebug() << Q_FUNC_INFO;

   MashStep* stepSender = qobject_cast<MashStep*>(sender());
   if (stepSender) {
      if (stepSender->getMashId() != this->mashObs->key()) {
         // It really shouldn't happen that we get a notification for a MashStep that's not in the Mash we're watching,
         // but, if we do, then stop trying to process the update.
         qCritical() <<
            Q_FUNC_INFO << "Instance @" << static_cast<void *>(this) << "received update for MashStep" <<
            stepSender->key() << "of Mash" << stepSender->getMashId() << "but we are watching Mash" <<
            this->mashObs->key();
         return;
      }

      int ii = this->steps.indexOf(stepSender);
      if (ii >= 0) {
         if (prop.name() == PropertyNames::MashStep::stepNumber) {
            this->reorderMashStep(stepSender, ii);
         }

         emit dataChanged( QAbstractItemModel::createIndex(ii, 0),
                           QAbstractItemModel::createIndex(ii, MASHSTEPNUMCOLS-1));
      }
   }

   if (this->parentTableWidget) {
      this->parentTableWidget->resizeColumnsToContents();
      this->parentTableWidget->resizeRowsToContents();
   }
   return;
}

int MashStepTableModel::rowCount(const QModelIndex& /*parent*/) const {
   return this->steps.size();
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
   if( index.row() >= static_cast<int>(this->steps.size()) ) {
      qWarning() << tr("Bad model index. row = %1").arg(index.row());
      return QVariant();
   }

   row = this->steps[index.row()];

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
                ? QVariant( Brewtarget::displayAmount(row->decoctionAmount_l(), &Units::liters, 3, unit, scale ) )
                : QVariant( Brewtarget::displayAmount(row->infuseAmount_l(), &Units::liters, 3, unit, scale) );
      case MASHSTEPTEMPCOL:
         unit = displayUnit(col);
         return (row->type() == MashStep::Decoction)
                ? QVariant("---")
                : QVariant( Brewtarget::displayAmount(row->infuseTemp_c(), &Units::celsius, 3, unit, Unit::noScale) );
      case MASHSTEPTARGETTEMPCOL:
         unit = displayUnit(col);
         return QVariant( Brewtarget::displayAmount(row->stepTemp_c(), &Units::celsius,3, unit, Unit::noScale) );
      case MASHSTEPTIMECOL:
         scale = displayScale(col);
         return QVariant( Brewtarget::displayAmount(row->stepTime_min(), &Units::minutes,3,Unit::noUnit,scale) );
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

bool MashStepTableModel::setData( const QModelIndex& index, const QVariant& value, int role ) {

   if( mashObs == nullptr ) {
      return false;
   }

   if( index.row() >= static_cast<int>(this->steps.size()) || role != Qt::EditRole ) {
      return false;
   }

   MashStep *row = this->steps[index.row()];

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
                                                  PropertyNames::MashStep::type,
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
                                                        Brewtarget::qStringToSI(value.toString(),&Units::liters,dspUnit,dspScl),
                                                        tr("Change Mash Step Decoction Amount"));
            } else {
               Brewtarget::mainWindow()->doOrRedoUpdate(*row,
                                                        PropertyNames::MashStep::infuseAmount_l,
                                                        Brewtarget::qStringToSI(value.toString(),&Units::liters,dspUnit,dspScl),
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
                                                      Brewtarget::qStringToSI(value.toString(),&Units::celsius,dspUnit,dspScl),
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
                                                             Brewtarget::qStringToSI(value.toString(),&Units::celsius,dspUnit,dspScl),
                                                             tr("Change Mash Step Temp"));
            new SimpleUndoableUpdate(*row,
                                     PropertyNames::MashStep::endTemp_c,
                                     Brewtarget::qStringToSI(value.toString(),&Units::celsius,dspUnit,dspScl),
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
                                                      Brewtarget::qStringToSI(value.toString(),&Units::minutes,dspUnit,dspScl),
                                                      tr("Change Mash Step Time"));
            return true;
         }
         else
            return false;
      default:
         return false;
   }
}

void MashStepTableModel::moveStepUp(int i) {
   if( this->mashObs == nullptr || i == 0 || i >= this->steps.size() ) {
      return;
   }

   this->mashObs->swapMashSteps(*this->steps[i], *this->steps[i-1]);
   return;
}

void MashStepTableModel::moveStepDown(int i) {
   if( this->mashObs == nullptr ||  i+1 >= steps.size() ) {
      return;
   }

   this->mashObs->swapMashSteps(*this->steps[i], *this->steps[i+1]);
   return;
}

Unit::unitDisplay MashStepTableModel::displayUnit(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noUnit;

   return static_cast<Unit::unitDisplay>(PersistentSettings::value(attribute, Unit::noUnit, this->objectName(), PersistentSettings::UNIT).toInt());
}

Unit::unitScale MashStepTableModel::displayScale(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noScale;

   return static_cast<Unit::unitScale>(PersistentSettings::value(attribute, Unit::noScale, this->objectName(), PersistentSettings::SCALE).toInt());
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

   PersistentSettings::insert(attribute, displayUnit, this->objectName(), PersistentSettings::UNIT);
   PersistentSettings::insert(attribute, Unit::noScale, this->objectName(), PersistentSettings::SCALE);

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

   PersistentSettings::insert(attribute,displayScale, this->objectName(), PersistentSettings::SCALE);

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
         attribute = "amount"; // Not a real property name
         break;
      case MASHSTEPTEMPCOL:
         attribute = *PropertyNames::MashStep::infuseTemp_c;
         break;
      case MASHSTEPTARGETTEMPCOL:
         attribute = *PropertyNames::MashStep::stepTemp_c;
         break;
      case MASHSTEPTIMECOL:
         attribute = *PropertyNames::Misc::time;
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
