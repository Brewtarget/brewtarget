/*
 * SaltTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - swstim <swstim@gmail.com>
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
#include "tableModels/SaltTableModel.h"

#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QComboBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QStandardItemModel>
#include <QString>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "WaterDialog.h"

static QStringList addToName = QStringList() << QObject::tr("Never")
                                             << QObject::tr("Mash")
                                             << QObject::tr("Sparge")
                                             << QObject::tr("Ratio")
                                             << QObject::tr("Both");

static QStringList saltNames = QStringList() << QObject::tr("None")
                                             << QObject::tr("CaCl2")
                                             << QObject::tr("CaCO3")
                                             << QObject::tr("CaSO4")
                                             << QObject::tr("MgSO4")
                                             << QObject::tr("NaCl")
                                             << QObject::tr("NaHCO3")
                                             << QObject::tr("Lactic acid")
                                             << QObject::tr("H3PO4")
                                             << QObject::tr("Acid malt");

SaltTableModel::SaltTableModel(QTableView* parent) :
   BtTableModelRecipeObserver{
      parent,
      false,
      {{SALTNAMECOL,    {tr("Name"),     NonPhysicalQuantity::String,          ""      }},
       {SALTAMOUNTCOL,  {tr("Amount"),   Measurement::PhysicalQuantity::Mixed, "amount"}},
       {SALTADDTOCOL,   {tr("Added To"), NonPhysicalQuantity::String,          ""      }},
       {SALTPCTACIDCOL, {tr("% Acid"),   NonPhysicalQuantity::Percentage,      ""      }}}
   },
   BtTableModelData<Salt>{} {
   setObjectName("saltTable");

   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   headerView->setMinimumSectionSize(parent->width()/SALTNUMCOLS);
   headerView->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->setWordWrap(false);

   connect(headerView, &QWidget::customContextMenuRequested, this, &SaltTableModel::contextMenu);
   return;
}

SaltTableModel::~SaltTableModel() {
   this->rows.clear();
   return;
}

void SaltTableModel::observeRecipe(Recipe* rec)
{
   if ( this->recObs ) {
      QObject::disconnect( this->recObs, nullptr, this, nullptr );
      removeAll();
   }

   this->recObs = rec;
   if ( this->recObs ) {
      connect( this->recObs, &NamedEntity::changed, this, &SaltTableModel::changed );
      this->addSalts(this->recObs->getAll<Salt>());
      if ( this->recObs->mash() ) {
         spargePct = this->recObs->mash()->totalSpargeAmount_l()/this->recObs->mash()->totalInfusionAmount_l();
      }
   }
}

void SaltTableModel::addSalt(std::shared_ptr<Salt> salt) {
   if (this->rows.contains(salt) ) {
      return;
   }

   beginInsertRows( QModelIndex(), this->rows.size(), this->rows.size() );
   this->rows.append(salt);
   connect(salt.get(), &NamedEntity::changed, this, &SaltTableModel::changed );
   endInsertRows();

   if (parentTableWidget) {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void SaltTableModel::addSalts(QList<std::shared_ptr<Salt> > salts) {
   auto tmp = this->removeDuplicatesIgnoreDisplay(salts);

   int size = this->rows.size();
   if (size+tmp.size()) {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      this->rows.append(tmp);
      endInsertRows();

      for (auto salt : tmp) {
         connect(salt.get(), &NamedEntity::changed, this, &SaltTableModel::changed);
      }

   }

   if (parentTableWidget ) {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void SaltTableModel::catchSalt() {
   // This gets stored in the DB in saveAndClose()
   auto gaq = std::make_shared<Salt>("");
   this->addSalt(gaq);
   return;
}

double SaltTableModel::multiplier(Salt & salt) const {
   double ret = 1.0;

   if ( ! this->recObs->mash()->hasSparge() ) {
      return ret;
   }

   if (salt.addTo() == Salt::WhenToAdd::EQUAL ) {
      ret = 2.0;
   }
   // If we are adding a proportional amount to both,
   // this should handle that math.
   else if (salt.addTo() == Salt::WhenToAdd::RATIO ) {
      ret = 1.0 + spargePct;
   }

   return ret;
}

// total salt in ppm. Not sure this is helping.
double SaltTableModel::total_Ca() const {
   double ret = 0.0;
   for (auto salt : this->rows) {
      double mult = this->multiplier(*salt);
      ret += mult * salt->Ca();
   }
   return ret;
}

double SaltTableModel::total_Cl() const {
   double ret = 0.0;
   for (auto salt : this->rows) {
      double mult  = multiplier(*salt);
      ret += mult * salt->Cl();
   }
   return ret;
}

double SaltTableModel::total_CO3() const {
   double ret = 0.0;
   for (auto salt : this->rows) {
      double mult  = multiplier(*salt);
      ret += mult * salt->CO3();
   }
   return ret;
}

double SaltTableModel::total_HCO3() const {
   double ret = 0.0;
   for (auto salt : this->rows) {
      double mult  = multiplier(*salt);
      ret += mult * salt->HCO3();
   }
   return ret;
}

double SaltTableModel::total_Mg() const {
   double ret = 0.0;
   for (auto salt : this->rows) {
      double mult  = multiplier(*salt);
      ret += mult * salt->Mg();
   }
   return ret;
}

double SaltTableModel::total_Na() const {
   double ret = 0.0;
   for (auto salt : this->rows) {
      double mult  = multiplier(*salt);
      ret += mult * salt->Na();
   }
   return ret;
}

double SaltTableModel::total_SO4() const {
   double ret = 0.0;
   for (auto salt : this->rows) {
      double mult  = multiplier(*salt);
      ret += mult * salt->SO4();
   }
   return ret;
}

double SaltTableModel::total(Water::Ions ion) const {
   switch(ion) {
      case Water::Ions::Ca:   return total_Ca();
      case Water::Ions::Cl:   return total_Cl();
      case Water::Ions::HCO3: return total_HCO3();
      case Water::Ions::Mg:   return total_Mg();
      case Water::Ions::Na:   return total_Na();
      case Water::Ions::SO4:  return total_SO4();
      default: return 0.0;
   }
   return 0.0;
}

double SaltTableModel::total(Salt::Types type) const {
   double ret = 0.0;
   if (type != Salt::Types::NONE) {
      for (auto salt : this->rows) {
         if (salt->type() == type && salt->addTo() != Salt::WhenToAdd::NEVER) {
            double mult  = multiplier(*salt);
            ret += mult * salt->amount();
         }
      }
   }
   return ret;
}

double SaltTableModel::totalAcidWeight(Salt::Types type) const
{
   const double H3PO4_density = 1.685;
   const double lactic_density = 1.2;

   double ret = 0.0;
   if (type != Salt::Types::NONE) {
      for (auto salt : this->rows) {
         if ( salt->type() == type && salt->addTo() != Salt::WhenToAdd::NEVER) {
            double mult  = multiplier(*salt);
            // Acid malts are easy
            if ( type == Salt::Types::ACIDMLT ) {
               ret += 1000.0 * salt->amount() * salt->percentAcid();
            }
            // Lactic acid isn't quite so easy
            else if ( type == Salt::Types::LACTIC ) {
               double density = salt->percentAcid()/88.0 * (lactic_density - 1.0) + 1.0;
               double lactic_wgt = 1000.0 * salt->amount() * mult * density;
               ret += (salt->percentAcid()/100.0) * lactic_wgt;
            }
            else if ( type == Salt::Types::H3PO4 ) {
               double density = salt->percentAcid()/85.0 * (H3PO4_density - 1.0) + 1.0;
               double H3PO4_wgt = 1000.0 * salt->amount() * density;
               ret += (salt->percentAcid()/100.0) * H3PO4_wgt;
            }
         }
      }
   }
   return ret;
}

void SaltTableModel::remove(std::shared_ptr<Salt> salt) {
   int i = this->rows.indexOf(salt);

   if (i >= 0 ) {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect(salt.get(), nullptr, this, nullptr);
      this->rows.removeAt(i);
      endRemoveRows();

      if(parentTableWidget) {
         parentTableWidget->resizeColumnsToContents();
         parentTableWidget->resizeRowsToContents();
      }
   }
   emit newTotals();
}

void SaltTableModel::removeSalts(QList<int>deadSalts) {
   decltype(this->rows) dead;

   // I am removing the salts so the index of any salt
   // will change. I think this will work
   for (auto rowNum : deadSalts) {
      dead.append( this->rows.at(rowNum));
   }

   for (auto zombie : dead) {
      int i = this->rows.indexOf(zombie);

      if ( i >= 0 ) {
         beginRemoveRows( QModelIndex(), i, i );
         disconnect(zombie.get(), nullptr, this, nullptr );
         this->rows.removeAt(i);
         endRemoveRows();

         // Dead salts do not malinger in the database. This will
         // delete the thing, not just mark it deleted
         if (zombie->key() > 0) {
            this->recObs->remove(zombie);
            ObjectStoreWrapper::hardDelete(*zombie);
         }
      }
   }
   emit newTotals();
   return;
}

void SaltTableModel::removeAll() {
   beginRemoveRows( QModelIndex(), 0, this->rows.size()-1 );
   while (!this->rows.isEmpty() ) {
      disconnect(this->rows.takeLast().get(), nullptr, this, nullptr );
   }
   endRemoveRows();
}

void SaltTableModel::changed(QMetaProperty prop, [[maybe_unused]] QVariant val) {
   // Find the notifier in the list
   Salt * saltSender = qobject_cast<Salt*>(sender());
   if (saltSender) {
      int ii = this->findIndexOf(saltSender);
      if (ii >= 0) {
         emit dataChanged(QAbstractItemModel::createIndex(ii, 0),
                          QAbstractItemModel::createIndex(ii, SALTNUMCOLS-1));
         emit headerDataChanged(Qt::Vertical, ii, ii);
      }
      return;
   }

   // See if sender is our recipe.
   Recipe* recSender = qobject_cast<Recipe*>(sender());
   if (recSender && recSender == this->recObs ) {
      if (QString(prop.name()) == "salts") {
         removeAll();
         addSalts(this->recObs->getAll<Salt>());
      }
      if (rowCount() > 0) {
         emit headerDataChanged( Qt::Vertical, 0, rowCount()-1 );
      }
   }
   return;
}

int SaltTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return this->rows.size();
}

QVariant SaltTableModel::data(QModelIndex const & index, int role) const {
   // Ensure the row is ok.
   if (index.row() >= static_cast<int>(this->rows.size())) {
      qWarning() << Q_FUNC_INFO << "Bad model index. row = " << index.row();
      return QVariant();
   }

   auto row = this->rows[index.row()];

   int column = index.column();
   switch (column) {
      case SALTNAMECOL:
         if (role == Qt::DisplayRole) {
            return QVariant(saltNames.at(static_cast<int>(row->type())));
         }
         if (role == Qt::UserRole) {
            return QVariant(static_cast<int>(row->type()));
         }
         return QVariant();
      case SALTAMOUNTCOL:
         if (role != Qt::DisplayRole) {
            return QVariant();
         }
         return QVariant(
            Measurement::displayAmount(
               Measurement::Amount{
                  row->amount(),
                  row->amountIsWeight() ? Measurement::Units::kilograms : Measurement::Units::liters
               },
               3,
               this->getForcedSystemOfMeasurementForColumn(column),
               std::nullopt
            )
         );
      case SALTADDTOCOL:
         if (role == Qt::DisplayRole) {
            return QVariant( addToName.at(static_cast<int>(row->addTo())));
         }
         if (role == Qt::UserRole) {
            return QVariant(static_cast<int>(row->addTo()));
         }
         return QVariant();
      case SALTPCTACIDCOL:
         if (role == Qt::DisplayRole && row->isAcid()) {
            return QVariant( row->percentAcid() );
         }
         return QVariant();
      default :
         qWarning() << Q_FUNC_INFO << "Bad column: " << column;
         return QVariant();
   }
}

QVariant SaltTableModel::headerData( int section, Qt::Orientation orientation, int role ) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumName(section);
   }
   return QVariant();
}

Qt::ItemFlags SaltTableModel::flags(const QModelIndex& index ) const
{
   // Q_UNUSED(index)
   if (index.row() >= this->rows.size() )
      return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;

   auto row = this->rows[index.row()];

   if ( !row->isAcid() && index.column() == SALTPCTACIDCOL )  {
      return Qt::NoItemFlags;
   }
   return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool SaltTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (index.row() >= this->rows.size() || role != Qt::EditRole) {
      return false;
   }

   bool retval = false;

   auto row = this->rows[index.row()];

   int const column = index.column();
   switch (column) {
      case SALTNAMECOL:
         retval = value.canConvert(QVariant::Int);
         if (retval) {
            int newType = value.toInt();
            Salt::Types oldType = row->type();
            row->setType(static_cast<Salt::Types>(newType));
            row->setName(saltNames.at(newType));
            if ( oldType == Salt::Types::NONE ) {
               switch(  static_cast<Salt::Types>(newType) ) {
                  case Salt::Types::LACTIC: row->setPercentAcid(88); break;
                  case Salt::Types::H3PO4:  row->setPercentAcid(10); break;
                  case Salt::Types::ACIDMLT: row->setPercentAcid(2); break;
                  default: row->setPercentAcid(0);
               }
            }
         }
         break;
      case SALTAMOUNTCOL:
         retval = value.canConvert(QVariant::Double);
         if (retval) {
            row->setAmount(
               Measurement::qStringToSI(
                  value.toString(),
                  row->amountIsWeight() ? Measurement::PhysicalQuantity::Mass : Measurement::PhysicalQuantity::Volume,
                  this->getForcedSystemOfMeasurementForColumn(column),
                  this->getForcedRelativeScaleForColumn(column)
               ).quantity()
            );
         }
         break;
      case SALTADDTOCOL:
         retval = value.canConvert(QVariant::Int);
         if (retval) {
            row->setAddTo( static_cast<Salt::WhenToAdd>(value.toInt()) );
         }
         break;
      case SALTPCTACIDCOL:
         retval = row->isAcid() && value.canConvert(QVariant::Double);
         if (retval) {
            row->setPercentAcid(value.toDouble());
         }
         break;
      default:
         retval = false;
         qWarning() << tr("Bad column: %1").arg(index.column());
   }

   if ( retval && row->addTo() != Salt::WhenToAdd::NEVER ) {
      emit newTotals();
   }
   emit dataChanged(index,index);
   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->resizeSections(QHeaderView::ResizeToContents);

   return retval;
}

void SaltTableModel::saveAndClose() {
   // all of the writes should have been instantaneous unless
   // we've added a new salt. Wonder if this will work?
   for (auto salt : this->rows) {
      if (salt->key() < 0 && salt->type() != Salt::Types::NONE && salt->addTo() != Salt::WhenToAdd::NEVER) {
         ObjectStoreWrapper::insert(salt);
         this->recObs->add(salt);
      }
   }
   return;
}
//==========================CLASS SaltItemDelegate===============================

SaltItemDelegate::SaltItemDelegate(QObject* parent)
        : QItemDelegate(parent),
        m_mash(nullptr)
{
}

QWidget* SaltItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   Q_UNUSED(option)
   if ( index.column() == SALTNAMECOL ) {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("NONE")  ,      static_cast<int>(Salt::Types::NONE   ));
      box->addItem(tr("CaCl2") ,      static_cast<int>(Salt::Types::CACL2  ));
      box->addItem(tr("CaCO3") ,      static_cast<int>(Salt::Types::CACO3  ));
      box->addItem(tr("CaSO4") ,      static_cast<int>(Salt::Types::CASO4  ));
      box->addItem(tr("MgSO4") ,      static_cast<int>(Salt::Types::MGSO4  ));
      box->addItem(tr("NaCl")  ,      static_cast<int>(Salt::Types::NACL   ));
      box->addItem(tr("NaHCO3"),      static_cast<int>(Salt::Types::NAHCO3 ));
      box->addItem(tr("Lactic acid"), static_cast<int>(Salt::Types::LACTIC ));
      box->addItem(tr("H3PO4"),       static_cast<int>(Salt::Types::H3PO4  ));
      box->addItem(tr("Acid malt"),   static_cast<int>(Salt::Types::ACIDMLT));
      box->setMinimumWidth( box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;

   }
   else if ( index.column() == SALTADDTOCOL ) {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Never"),  static_cast<int>(Salt::WhenToAdd::NEVER ));
      box->addItem(tr("Mash"),   static_cast<int>(Salt::WhenToAdd::MASH  ));
      box->addItem(tr("Sparge"), static_cast<int>(Salt::WhenToAdd::SPARGE));
      box->addItem(tr("Ratio"),  static_cast<int>(Salt::WhenToAdd::RATIO ));
      box->addItem(tr("Equal"),  static_cast<int>(Salt::WhenToAdd::EQUAL ));
      box->setMinimumWidth( box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);

      if ( m_mash != nullptr ) {
         QStandardItemModel* i_model = qobject_cast<QStandardItemModel*>(box->model());
         if ( ! m_mash->hasSparge() ) {
            for( int i = 2; i < 5; ++i ) {
               QStandardItem* entry = i_model->item(i);
               if ( entry )
                  entry->setEnabled(false);
            }
         }
      }

      return box;
   }
   else {
      return new QLineEdit(parent);
   }
}

void SaltItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   int column = index.column();

   if ( column == SALTNAMECOL || column == SALTADDTOCOL ) {
      QComboBox *box = qobject_cast<QComboBox*>(editor);
      box->setCurrentIndex(index.model()->data(index,Qt::UserRole).toInt());
   }
   else {
      QLineEdit* line = qobject_cast<QLineEdit*>(editor);
      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }
}

void SaltItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
   int column = index.column();

   if ( column == SALTNAMECOL || column == SALTADDTOCOL ) {
      QComboBox* box = static_cast<QComboBox*>(editor);
      int selected = box->currentData().toInt();
      int stored = model->data(index,Qt::UserRole).toInt();

      if ( selected != stored ) {
         model->setData(index,selected,Qt::EditRole);
      }
   }
   else {
      QLineEdit* line = static_cast<QLineEdit*>(editor);

      if ( line->isModified() )
         model->setData(index, line->text(), Qt::EditRole);
   }
}

void SaltItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
   editor->setGeometry(option.rect);
}

void SaltItemDelegate::observeRecipe( Recipe* rec )
{
   m_mash = rec->mash();
}
