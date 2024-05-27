/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/MashStepTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "tableModels/MashStepTableModel.h"

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

#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/MashStep.h"
#include "PersistentSettings.h"

MashStepTableModel::MashStepTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(MashStep, Name      , tr("Name"         ), PropertyNames::NamedEntity::name      ),
         TABLE_MODEL_HEADER(MashStep, Type      , tr("Type"         ), PropertyNames::MashStep::type         , EnumInfo{MashStep::typeStringMapping, MashStep::typeDisplayNames}),
         TABLE_MODEL_HEADER(MashStep, Amount    , tr("Amount"       ), PropertyNames::MashStep::amount_l     ),
         TABLE_MODEL_HEADER(MashStep, Temp      , tr("Infusion Temp"), PropertyNames::MashStep::infuseTemp_c ),
         TABLE_MODEL_HEADER(MashStep, TargetTemp, tr("Target Temp"  ), PropertyNames::    Step::startTemp_c  ),
         TABLE_MODEL_HEADER(MashStep, Time      , tr("Time"         ), PropertyNames::    Step::stepTime_mins),
      }
   },
   TableModelBase<MashStepTableModel, MashStep>{},
   StepTableModelBase<MashStepTableModel, MashStep, Mash>{} {
   this->setObjectName("mashStepTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &MashStepTableModel::contextMenu);
   //
   // Whilst, in principle, we could connect to ObjectStoreTyped<MashStep>::getInstance() to listen for signals
   // &ObjectStoreTyped<MashStep>::signalObjectInserted and &ObjectStoreTyped<MashStep>::signalObjectDeleted, this is
   // less useful in practice because (a) we get updates about MashSteps in Mashes other than the one we are watching
   // (so we have to filter them out) and (b) when a new MashStep is created, it doesn't have a Mash, so it's not useful
   // for us to receive a signal about it until after it has been added to a Mash.  Fortunately, all we have to do is
   // connect to the Mash we are watching and listen for Mash::mashStepsChanged, which we'll get whenever a MashStep is
   // added to, or removed from, the Mash, as well as when the MashStep order changes.  We then just reread all the
   // MashSteps from the Mash which gives us simplicity for a miniscule overhead (because the number of MashSteps in a
   // Mash is never going to be enormous).
   //
   return;
}

MashStepTableModel::~MashStepTableModel() = default;

void MashStepTableModel::added  ([[maybe_unused]] std::shared_ptr<MashStep> item) { return; }
void MashStepTableModel::removed([[maybe_unused]] std::shared_ptr<MashStep> item) { return; }
void MashStepTableModel::updateTotals()                                      { return; }

///BtTableModel::ColumnInfo const & MashStepTableModel::getColumnInfo(MashStepTableModel::ColumnIndex const columnIndex) const {
///   return this->BtTableModel::getColumnInfo(static_cast<size_t>(columnIndex));
///}

///bool MashStepTableModel::remove(std::shared_ptr<MashStep> mashStep) {
///
///   int ii {this->rows.indexOf(mashStep)};
///   if (ii >= 0) {
///      qDebug() << Q_FUNC_INFO << "Removing MashStep" << mashStep->name() << "(#" << mashStep->key() << ")";
///      beginRemoveRows( QModelIndex(), ii, ii );
///      disconnect(mashStep.get(), nullptr, this, nullptr);
///      this->rows.removeAt(ii);
///      //reset(); // Tell everybody the table has changed.
///      endRemoveRows();
///
///      return true;
///   }
///
///   return false;
///}

///void MashStepTableModel::setMash(Mash * m) {
///   if (this->mashObs && this->rows.size() > 0) {
///      qDebug() <<
///         Q_FUNC_INFO << "Removing" << this->rows.size() << "MashStep rows for old Mash #" << this->mashObs->key();
///      this->beginRemoveRows(QModelIndex(), 0, this->rows.size() - 1);
///
///      for (auto step : this->rows) {
///         disconnect(step.get(), nullptr, this, nullptr);
///      }
///      this->rows.clear();
///      this->endRemoveRows();
///   }
///
///   // Disconnect old signals if any were connected and we're changing Mash
///   if (this->mashObs && this->mashObs != m) {
///      // Remove mashObs and all steps.
///      disconnect(mashObs, nullptr, this, nullptr);
///   }
///
///   // Connect new signals, unless there is no new Mash or we're not changing Mash
///   if (m && this->mashObs != m) {
///      connect(m, &Mash::stepsChanged, this, &MashStepTableModel::mashChanged);
///   }
///
///   this->mashObs = m;
///   if (this->mashObs) {
///      qDebug() << Q_FUNC_INFO << "Now watching Mash #" << this->mashObs->key();
///
///      auto tmpSteps = this->mashObs->mashSteps();
///      if (tmpSteps.size() > 0) {
///         qDebug() << Q_FUNC_INFO << "Inserting" << tmpSteps.size() << "MashStep rows";
///         this->beginInsertRows(QModelIndex(), 0, tmpSteps.size() - 1);
///         this->rows = tmpSteps;
///         for (auto step : this->rows) {
///            connect(step.get(), &NamedEntity::changed, this, &MashStepTableModel::mashStepChanged);
///         }
///         this->endInsertRows();
///     }
///   }
///
///   if (m_parentTableWidget) {
///      m_parentTableWidget->resizeColumnsToContents();
///      m_parentTableWidget->resizeRowsToContents();
///   }
///   return;
///}

///Mash * MashStepTableModel::getMash() const {
///   return this->mashObs;
///}

///void MashStepTableModel::reorderMashStep(std::shared_ptr<MashStep> step, int current) {
///   // doSomething will be -1 if we are moving up and 1 if we are moving down
///   // and 0 if nothing is to be done (see next comment)
///   int destChild   = step->stepNumber();
///   int doSomething = destChild - current - 1;
///
///   qDebug() << Q_FUNC_INFO << "Swapping" << destChild << "with" << current << ", so doSomething=" << doSomething;
///
///   // Moving a step up or down generates two signals, one for each row
///   // impacted. If we move row B above row A:
///   //    1. The first signal is to move B above A, which will result in A
///   //    being below B
///   //    2. The second signal is to move A below B, which we just did.
///   // Therefore, the second signal mostly needs to be ignored. In those
///   // circumstances, A->stepNumber() will be the same as its position in the
///   // steps list, modulo some indexing
///   if (doSomething == 0) {
///      return;
///   }
///
///   // beginMoveRows is a little odd. When moving rows within the same parent,
///   // destChild points one beyond where you want to insert the row. Think of
///   // it as saying "insert before destChild". If we are moving something up,
///   // we need to be one less than stepNumber. If we are moving down, it just
///   // works.
///   if (doSomething < 0) {
///      destChild--;
///   }
///
///   // We assert that we are swapping valid locations on the list as, to do otherwise implies a coding error
///   qDebug() <<
///      Q_FUNC_INFO << "Swap" << current + doSomething << "with" << current << ", in list of " << this->rows.size();
///   Q_ASSERT(current >= 0);
///   Q_ASSERT(current + doSomething >= 0);
///   Q_ASSERT(current < this->rows.size());
///   Q_ASSERT(current + doSomething < this->rows.size());
///
///   this->beginMoveRows(QModelIndex(), current, current, QModelIndex(), destChild);
///
///   // doSomething is -1 if moving up and 1 if moving down. swap current with
///   // current -1 when moving up, and swap current with current+1 when moving
///   // down
///#if QT_VERSION < QT_VERSION_CHECK(5,13,0)
///   this->rows.swap(current,current+doSomething);
///#else
///   this->rows.swapItemsAt(current,current+doSomething);
///#endif
///   this->endMoveRows();
///   return;
///}

///void MashStepTableModel::mashChanged() {
///   // A mash step was added, removed or change order.  Remove and re-add all steps.
///   qDebug() << Q_FUNC_INFO << "Re-reading mash steps for" << this->mashObs;
///   this->setMash(this->mashObs);
///   return;
///}

///void MashStepTableModel::mashStepChanged(QMetaProperty prop,
///                                         [[maybe_unused]] QVariant val) {
///   qDebug() << Q_FUNC_INFO;
///
///   MashStep* stepSender = qobject_cast<MashStep*>(sender());
///   if (stepSender) {
///      if (stepSender->ownerId() != this->mashObs->key()) {
///         // It really shouldn't happen that we get a notification for a MashStep that's not in the Mash we're watching,
///         // but, if we do, then stop trying to process the update.
///         qCritical() <<
///            Q_FUNC_INFO << "Instance @" << static_cast<void *>(this) << "received update for MashStep" <<
///            stepSender->key() << "of Mash" << stepSender->ownerId() << "but we are watching Mash" <<
///            this->mashObs->key();
///         return;
///      }
///
///      int ii = this->findIndexOf(stepSender);
///      if (ii >= 0) {
///         if (prop.name() == PropertyNames::Step::stepNumber) {
///            this->reorderMashStep(this->rows.at(ii), ii);
///         }
///
///         emit dataChanged(QAbstractItemModel::createIndex(ii, 0),
///                          QAbstractItemModel::createIndex(ii, this->columnCount() - 1));
///      }
///
///   }
///
///    if (this->m_parentTableWidget) {
///      this->m_parentTableWidget->resizeColumnsToContents();
///      this->m_parentTableWidget->resizeRowsToContents();
///   }
///   return;
///}

///int MashStepTableModel::rowCount(const QModelIndex& /*parent*/) const {
///   return this->rows.size();
///}

QVariant MashStepTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_stepOwnerObs) {
      return QVariant();
   }

   if (!this->isIndexOk(index)) {
      return QVariant();
   }

   // Make sure we only respond to the DisplayRole role.
   if (role != Qt::DisplayRole) {
      return QVariant();
   }

///   // Ensure the row is ok.
///   if (index.row() >= static_cast<int>(this->rows.size())) {
///      qWarning() << Q_FUNC_INFO << "Bad model index. row = " << index.row();
///      return QVariant();
///   }

   auto row = this->rows[index.row()];

   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case MashStepTableModel::ColumnIndex::Name:
      case MashStepTableModel::ColumnIndex::Type:
      case MashStepTableModel::ColumnIndex::Amount:
      case MashStepTableModel::ColumnIndex::TargetTemp:
      case MashStepTableModel::ColumnIndex::Time:
         return this->readDataFromModel(index, role);

      case MashStepTableModel::ColumnIndex::Temp:
         if (row->type() == MashStep::Type::Decoction) {
            return QVariant("---");
         }
         return this->readDataFromModel(index, role);

///      case MashStepTableModel::ColumnIndex::Name:
///         return QVariant(row->name());
///      case MashStepTableModel::ColumnIndex::Type:
///         return QVariant(row->typeStringTr());
///      case MashStepTableModel::ColumnIndex::Amount:
///         return QVariant(
///            Measurement::displayAmount(
///               Measurement::Amount{
///                  row->type() == MashStep::Type::Decoction ? row->decoctionAmount_l() : row->infuseAmount_l(),
///                  Measurement::Units::liters
///               },
///               3,
///               this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///               this->getColumnInfo(columnIndex).getForcedRelativeScale()
///            )
///         );
///      case MashStepTableModel::ColumnIndex::Temp:
///         if (row->type() == MashStep::Type::Decoction) {
///            return QVariant("---");
///         }
///         return QVariant(
///            Measurement::displayAmount(Measurement::Amount{row->infuseTemp_c(), Measurement::Units::celsius},
///                                       3,
///                                       this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///                                       std::nullopt)
///         );
///      case MashStepTableModel::ColumnIndex::TargetTemp:
///         return QVariant(
///            Measurement::displayAmount(Measurement::Amount{row->stepTemp_c(), Measurement::Units::celsius},
///                                       3,
///                                       this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///                                       std::nullopt)
///         );
///      case MashStepTableModel::ColumnIndex::Time:
///         return QVariant(
///            Measurement::displayAmount(Measurement::Amount{row->stepTime_mins(), Measurement::Units::minutes},
///                                       3,
///                                       std::nullopt,
///                                       this->getColumnInfo(columnIndex).getForcedRelativeScale())
///         );
///      default :
///         qWarning() << Q_FUNC_INFO << "Bad column: " << index.column();
///         return QVariant();

      // No default case as we want the compiler to warn us if we missed one
   }
   return QVariant();
}

QVariant MashStepTableModel::headerData( int section, Qt::Orientation orientation, int role ) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumnLabel(section);
   }
   return QVariant();
}

Qt::ItemFlags MashStepTableModel::flags(const QModelIndex& index ) const {
   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
   if (columnIndex == MashStepTableModel::ColumnIndex::Name) {
      return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool MashStepTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_stepOwnerObs) {
      return false;
   }

   if (!this->isIndexOk(index)) {
      return false;
   }

   if (index.row() >= static_cast<int>(this->rows.size()) || role != Qt::EditRole ) {
      return false;
   }


   bool retVal = false;
///   auto row = this->rows[index.row()];

   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case MashStepTableModel::ColumnIndex::Name:
      case MashStepTableModel::ColumnIndex::Type:
      case MashStepTableModel::ColumnIndex::Temp:
      case MashStepTableModel::ColumnIndex::TargetTemp:
      case MashStepTableModel::ColumnIndex::Time:
         retVal = this->writeDataToModel(index, value, role);
         break;

      case MashStepTableModel::ColumnIndex::Amount:
         retVal = this->writeDataToModel(index, value, role, Measurement::PhysicalQuantity::Volume);
         break;

///      case MashStepTableModel::ColumnIndex::Name:
///         if (value.canConvert(QVariant::String)) {
///            MainWindow::instance().doOrRedoUpdate(*row,
///                                                  TYPE_INFO(MashStep, NamedEntity, name),
///                                                  value.toString(),
///                                                  tr("Change Mash Step Name"));
///            return true;
///         }
///         return false;
///
///      case MashStepTableModel::ColumnIndex::Type:
///         if (value.canConvert(QVariant::Int)) {
///            MainWindow::instance().doOrRedoUpdate(*row,
///                                                  TYPE_INFO(MashStep, type),
///                                                  value.toInt(),
///                                                  tr("Change Mash Step Type"));
///            return true;
///         }
///         return false;
///
///      case MashStepTableModel::ColumnIndex::Amount:
///         if (value.canConvert(QVariant::String)) {
///            if (row->type() == MashStep::Type::Decoction ) {
///               MainWindow::instance().doOrRedoUpdate(
///                  *row,
///                  TYPE_INFO(MashStep, decoctionAmount_l),
///                  Measurement::qStringToSI(value.toString(),
///                                           Measurement::PhysicalQuantity::Volume,
///                                           this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///                                           this->getColumnInfo(columnIndex).getForcedRelativeScale()).quantity(),
///                  tr("Change Mash Step Decoction Amount")
///               );
///            } else {
///               MainWindow::instance().doOrRedoUpdate(
///                  *row,
///                  TYPE_INFO(MashStep, infuseAmount_l),
///                  Measurement::qStringToSI(value.toString(),
///                                           Measurement::PhysicalQuantity::Volume,
///                                           this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///                                           this->getColumnInfo(columnIndex).getForcedRelativeScale()).quantity(),
///                  tr("Change Mash Step Infuse Amount")
///               );
///            }
///            return true;
///         }
///         return false;
///
///      case MashStepTableModel::ColumnIndex::Temp:
///         if (value.canConvert(QVariant::String) && row->type() != MashStep::Type::Decoction) {
///            MainWindow::instance().doOrRedoUpdate(
///               *row,
///               TYPE_INFO(MashStep, infuseTemp_c),
///               Measurement::qStringToSI(value.toString(),
///                                        Measurement::PhysicalQuantity::Temperature,
///                                        this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///                                        this->getColumnInfo(columnIndex).getForcedRelativeScale()).quantity(),
///               tr("Change Mash Step Infuse Temp")
///            );
///            return true;
///         }
///         return false;
///
///      case MashStepTableModel::ColumnIndex::TargetTemp:
///         if (value.canConvert(QVariant::String)) {
///            // Two changes, but we want to group together as one undo/redo step
///            //
///            // We don't assign the pointer (returned by new) to second SimpleUndoableUpdate we create because, in the
///            // constructor, it gets linked to the first one, which then "owns" it.
///            auto targetTempUpdate = new SimpleUndoableUpdate(
///               *row,
///               TYPE_INFO(MashStep, stepTemp_c),
///               Measurement::qStringToSI(value.toString(),
///                                        Measurement::PhysicalQuantity::Temperature,
///                                        this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///                                        this->getColumnInfo(columnIndex).getForcedRelativeScale()).quantity(),
///               tr("Change Mash Step Temp")
///            );
///            MainWindow::instance().doOrRedoUpdate(targetTempUpdate);
///            return true;
///         }
///         return false;
///
///      case MashStepTableModel::ColumnIndex::Time:
///         if (value.canConvert(QVariant::String)) {
///            MainWindow::instance().doOrRedoUpdate(
///               *row,
///               TYPE_INFO(MashStep, stepTime_mins),
///               Measurement::qStringToSI(value.toString(),
///                                        Measurement::PhysicalQuantity::Time,
///                                        this->getColumnInfo(columnIndex).getForcedSystemOfMeasurement(),
///                                        this->getColumnInfo(columnIndex).getForcedRelativeScale()).quantity(),
///               tr("Change Mash Step Time")
///            );
///            return true;
///         }
///         return false;
///
///      default:
///         return false;


      // No default case as we want the compiler to warn us if we missed one
   }
   return retVal;
}

///void MashStepTableModel::moveStepUp(int i) {
///   if (this->mashObs == nullptr || i == 0 || i >= this->rows.size()) {
///      return;
///   }
///
///   this->mashObs->swapSteps(*this->rows[i], *this->rows[i-1]);
///   return;
///}
///
///void MashStepTableModel::moveStepDown(int i) {
///   if (this->mashObs == nullptr ||  i+1 >= this->rows.size()) {
///      return;
///   }
///
///   this->mashObs->swapSteps(*this->rows[i], *this->rows[i+1]);
///   return;
///}

/////==========================CLASS MashStepItemDelegate===============================
///
///MashStepItemDelegate::MashStepItemDelegate(QObject* parent) : QItemDelegate(parent) {
///   return;
///}
///
///QWidget* MashStepItemDelegate::createEditor(QWidget * parent,
///                                            QStyleOptionViewItem const &/*option*/,
///                                            QModelIndex const & index) const {
///   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
///   if (columnIndex == MashStepTableModel::ColumnIndex::Type) {
///      QComboBox *box = new QComboBox(parent);
///
///      foreach( QString mtype, MashStep::types )
///         box->addItem(mtype);
///
///      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
///
///      return box;
///   }
///
///   return new QLineEdit(parent);
///}
///
///void MashStepItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
///   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
///   if (columnIndex == MashStepTableModel::ColumnIndex::Type) {
///      QComboBox* box = qobject_cast<QComboBox*>(editor);
///      QString text = index.model()->data(index, Qt::DisplayRole).toString();
///
///      int index = box->findText(text);
///      box->setCurrentIndex(index);
///   } else {
///      QLineEdit* line = qobject_cast<QLineEdit*>(editor);
///
///      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
///   }
///   return;
///}
///
///void MashStepItemDelegate::setModelData(QWidget * editor,
///                                        QAbstractItemModel * model,
///                                        QModelIndex const & index) const {
///   QStringList typesTr = QStringList() << QObject::tr("Infusion") << QObject::tr("Temperature") << QObject::tr("Decoction");
///   auto const columnIndex = static_cast<MashStepTableModel::ColumnIndex>(index.column());
///   if (columnIndex == MashStepTableModel::ColumnIndex::Type) {
///      QComboBox* box = qobject_cast<QComboBox*>(editor);
///      int ndx = box->currentIndex();
///      int curr  = typesTr.indexOf(model->data(index,Qt::DisplayRole).toString());
///      if ( ndx != curr ) {
///         model->setData(index, ndx, Qt::EditRole);
///      }
///   } else {
///      QLineEdit* line = qobject_cast<QLineEdit*>(editor);
///      if ( line->isModified() ) {
///         model->setData(index, line->text(), Qt::EditRole);
///      }
///   }
///   return;
///}
///
///void MashStepItemDelegate::updateEditorGeometry(QWidget *editor,
///                                                QStyleOptionViewItem const & option,
///                                                QModelIndex const & /*index*/) const {
///   editor->setGeometry(option.rect);
///   return;
///}


// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(MashStep, mashStep, PropertyNames::Recipe::mashId)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
STEP_TABLE_MODEL_COMMON_CODE(Mash)
//=============================================== CLASS MashStepItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(MashStep)
