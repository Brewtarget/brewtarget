/*
 * SaltTableModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QWidget>
#include <QComboBox>
#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QLineEdit>
#include <QString>

#include <QList>
#include "database.h"
#include "WaterDialog.h"
#include "SaltTableModel.h"
#include "salt.h"
#include "unit.h"
#include "recipe.h"
#include "brewtarget.h"
#include "BtLineEdit.h"


SaltTableModel::SaltTableModel(QTableView* parent, WaterDialog* gp)
   : QAbstractTableModel(parent),
     colFlags(SALTNUMCOLS),
     recObs(nullptr),
     parentTableWidget(parent),
     dropper(gp)
{
   saltObs.clear();
   setObjectName("saltTable");

   for (int i = 0; i < SALTNAMECOL; ++i) {
      // cannot edit names
      if ( i == SALTNAMECOL )
         colFlags[i] = Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::NoItemFlags;
      else
         colFlags[i] = Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }
   QHeaderView* headerView = parentTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   parentTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   parentTableWidget->setWordWrap(false);

   connect(headerView, &QWidget::customContextMenuRequested, this, &SaltTableModel::contextMenu);
   if ( dropper )
      connect(dropper, &WaterDialog::droppedSalts, this, &SaltTableModel::catchSalts);
}

SaltTableModel::~SaltTableModel()
{
   saltObs.clear();
}

void SaltTableModel::observeRecipe(Recipe* rec)
{
   if ( recObs ) {
      disconnect( recObs, nullptr, this, nullptr );
      removeAll();
   }

   recObs = rec;
   if ( recObs ) {
      QList<Salt*> salts = recObs->salts();
      connect( recObs, &BeerXMLElement::changed, this, &SaltTableModel::changed );
      if (salts.size() > 0 ) {
         addSalts( salts );
         emit newTotals();
      }
   }
}

void SaltTableModel::addSalt(Salt* salt)
{
   if( saltObs.contains(salt) )
      return;
   // If we are observing the database, ensure that the item is undeleted and
   // fit to display.
   if( recObs == nullptr &&
      ( salt->deleted() || !salt->display() ) )
      return;

   beginInsertRows( QModelIndex(), saltObs.size(), saltObs.size() );
   saltObs.append(salt);
   connect( salt, &BeerXMLElement::changed, this, &SaltTableModel::changed );
   endInsertRows();

   if (parentTableWidget) {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }
}

void SaltTableModel::addSalts(QList<Salt*> salts)
{
   QList<Salt*> tmp;

   foreach (Salt* i, salts) {
      if( !saltObs.contains(i) )
         tmp.append(i);
   }

   int size = saltObs.size();
   if (size+tmp.size()) {
      beginInsertRows( QModelIndex(), size, size+tmp.size()-1 );
      saltObs.append(tmp);

      foreach (Salt* i, tmp) {
         connect( i, &BeerXMLElement::changed, this, &SaltTableModel::changed );
      }

      endInsertRows();
   }

   if( parentTableWidget ) {
      parentTableWidget->resizeColumnsToContents();
      parentTableWidget->resizeRowsToContents();
   }

}

void SaltTableModel::catchSalt(Salt* salt) { addSalt(salt); }

// I want some complex logic here. My current design is to allow each salt
// to only ever be added once to the mash or sparge. Of course, I have to
// handle 'both'.
// So the idea is this:
//   o if the salt is already in the list and the addTo === BOTH, don't accept;
//   o if the salt is already in the list twice, don't accept;
//   o if the salt is in the list once and addTo == SPARGE, make this "MASH"
//   o if the salt is in the list once and addTo == MASH, make this one SPARGE
void SaltTableModel::catchSalts(QList<Salt*> salts)
{
   QList<Salt*> accepted;
   foreach (Salt* i, salts) {
      int cnt = 0;
      Salt* found = nullptr;
      foreach( Salt* tmp, saltObs) {
         if ( tmp->type() == i->type() ) {
            cnt++;
            found = tmp;
         }
      }

      if( cnt == 0 ) {
         accepted.append(i);
      }
      else if ( cnt == 1 ) {
         if ( found->addTo() == Salt::MASH ) {
            i->setAddTo(Salt::SPARGE);
            accepted.append(i);
         }
         else if ( found->addTo() == Salt::SPARGE ) {
            i->setAddTo(Salt::MASH);
            accepted.append(i);
         }
      }
   }

   int size = saltObs.size();
   if (accepted.size()) {
      beginInsertRows( QModelIndex(), size, size+accepted.size()-1 );
      saltObs.append(accepted);

      foreach (Salt* i, accepted) {
         connect( i, &BeerXMLElement::changed, this, &SaltTableModel::changed );
      }

      endInsertRows();
   }
}

double SaltTableModel::total_Ca() const
{
   double ret = 0;
   foreach(Salt* i, saltObs) {
      // if one thing is being added to both mash and
      // sparge, we need to double the amount shown
      int mult = i->addTo() == Salt::BOTH ? 2 : 1;
      ret += mult * i->Ca();
   }

   return ret;
}

double SaltTableModel::total_Cl() const
{
   double ret = 0;
   foreach(Salt* i, saltObs) {
      int mult = i->addTo() == Salt::BOTH ? 2 : 1;
      ret += mult * i->Cl();
   }

   return ret;
}

double SaltTableModel::total_CO3() const
{
   double ret = 0;
   foreach(Salt* i, saltObs) {
      int mult = i->addTo() == Salt::BOTH ? 2 : 1;
      ret += mult * i->CO3();
   }

   return ret;
}

double SaltTableModel::total_HCO3() const
{
   double ret = 0;
   foreach(Salt* i, saltObs) {
      int mult = i->addTo() == Salt::BOTH ? 2 : 1;
      ret += mult * i->HCO3();
   }

   return ret;
}

double SaltTableModel::total_Mg() const
{
   double ret = 0;
   foreach(Salt* i, saltObs) {
      int mult = i->addTo() == Salt::BOTH ? 2 : 1;
      ret += mult * i->Mg();
   }

   return ret;
}

double SaltTableModel::total_Na() const
{
   double ret = 0;
   foreach(Salt* i, saltObs) {
      int mult = i->addTo() == Salt::BOTH ? 2 : 1;
      ret += mult * i->Na();
   }

   return ret;
}

double SaltTableModel::total_SO4() const
{
   double ret = 0;
   foreach(Salt* i, saltObs) {
      int mult = i->addTo() == Salt::BOTH ? 2 : 1;
      ret += mult * i->SO4();
   }

   return ret;
}

double SaltTableModel::total(Salt::Types type) const
{
   double ret = 0;
   foreach(Salt* i, saltObs) {
      if ( i->type() == type ) {
         int mult = i->addTo() == Salt::BOTH ? 2 : 1;
         ret += mult * i->amount();
      }
   }
   return ret;
}

void SaltTableModel::removeSalt(Salt* salt)
{
   int i;

   i = saltObs.indexOf(salt);
   if( i >= 0 ) {
      beginRemoveRows( QModelIndex(), i, i );
      disconnect( salt, nullptr, this, nullptr );
      saltObs.removeAt(i);
      endRemoveRows();

      if(parentTableWidget) {
         parentTableWidget->resizeColumnsToContents();
         parentTableWidget->resizeRowsToContents();
      }
   }
}

void SaltTableModel::removeAll()
{
   beginRemoveRows( QModelIndex(), 0, saltObs.size()-1 );
   while( !saltObs.isEmpty() ) {
      disconnect( saltObs.takeLast(), nullptr, this, nullptr );
   }
   endRemoveRows();
}

void SaltTableModel::changed(QMetaProperty prop, QVariant /*val*/)
{
   int i;

   // Find the notifier in the list
   Salt* saltSender = qobject_cast<Salt*>(sender());
   if( saltSender ) {
      i = saltObs.indexOf(saltSender);
      if( i >= 0 )
         emit dataChanged( QAbstractItemModel::createIndex(i, 0),
                           QAbstractItemModel::createIndex(i, SALTNUMCOLS-1));
         emit headerDataChanged( Qt::Vertical, i, i);
      return;
   }

   // See if sender is our recipe.
   Recipe* recSender = qobject_cast<Recipe*>(sender());
   if( recSender && recSender == recObs )
   {
      if( QString(prop.name()) == "salts" ) {
         removeAll();
         addSalts( recObs->salts() );
      }
      if( rowCount() > 0 )
         emit headerDataChanged( Qt::Vertical, 0, rowCount()-1 );
   }
}

int SaltTableModel::rowCount(const QModelIndex& /*parent*/) const
{
   return saltObs.size();
}

int SaltTableModel::columnCount(const QModelIndex& /*parent*/) const
{
   return SALTNUMCOLS;
}

QVariant SaltTableModel::data( const QModelIndex& index, int role ) const
{
   Salt* row;
   int col = index.column();
   Unit::unitScale scale;
   Unit::unitDisplay unit;
   QStringList addToName = QStringList() << "Never" << "Mash" << "Sparge" << "Both";

   // Ensure the row is ok.
   if( index.row() >= static_cast<int>(saltObs.size()) ) {
      Brewtarget::logW(tr("Bad model index. row = %1").arg(index.row()));
      return QVariant();
   }
   else
      row = saltObs[index.row()];

   // Make sure we only respond to the DisplayRole role.
   if( role != Qt::DisplayRole )
      return QVariant();

   switch( index.column() ) {
      case SALTNAMECOL:
         return QVariant(row->name());
      case SALTAMOUNTCOL:
         unit = displayUnit(col);
         scale = displayScale(col);

         return QVariant( Brewtarget::displayAmount(row->amount(), Units::kilograms,3, unit, scale));
      case SALTADDTOCOL:
         if ( row->addTo() < Salt::NEVER || row->addTo() > Salt::BOTH ) {
            return QVariant();
         }
         else {
            return QVariant( addToName.at(row->addTo()));
         }
      default :
         Brewtarget::logW(tr("Bad column: %1").arg(index.column()));
         return QVariant();
   }
}

QVariant SaltTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   if( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
      switch( section ) {
         case SALTNAMECOL:
            return QVariant(tr("Name"));
         case SALTAMOUNTCOL:
            return QVariant(tr("Amount"));
         case SALTADDTOCOL:
            return QVariant(tr("Added To"));
         default:
            Brewtarget::logW(tr("Bad column: %1").arg(section));
            return QVariant();
      }
   }
   else
      return QVariant();
}

Qt::ItemFlags SaltTableModel::flags(const QModelIndex& index ) const
{
   int col = index.column();
   switch(col) {
      case SALTNAMECOL:
         return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
      default:
         return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
            Qt::ItemIsEnabled;
   }
}

bool SaltTableModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
   Salt *row;
   bool retval = false;

   if( index.row() >= saltObs.size() || role != Qt::EditRole ) {
      qDebug() << Q_FUNC_INFO << "bugging out" << index.row() << saltObs.size() << role << Qt::EditRole;
      return false;
   }

   row = saltObs[index.row()];

   Unit::unitDisplay dspUnit = displayUnit(index.column());
   Unit::unitScale   dspScl  = displayScale(index.column());

   switch( index.column() ) {
      case SALTNAMECOL:
         retval = value.canConvert(QVariant::String);
         if ( retval ) {
            row->setName(value.toString());
         }
         break;
      case SALTAMOUNTCOL:
         retval = value.canConvert(QVariant::Double);
         if ( retval ) {
            row->setAmount( Brewtarget::qStringToSI(value.toString(), Units::kilograms, dspUnit, dspScl) );
            emit newTotals();
         }
         break;
      case SALTADDTOCOL:
         retval = value.canConvert(QVariant::Int);
         if ( retval ) {
            row->setAddTo( static_cast<Salt::WhenToAdd>(value.toInt()) );
         }
         break;
      default:
         retval = false;
         Brewtarget::logW(tr("Bad column: %1").arg(index.column()));
   }

   return retval;
}

Unit::unitDisplay SaltTableModel::displayUnit(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noUnit;

   return static_cast<Unit::unitDisplay>(Brewtarget::option(attribute, QVariant(-1), this->objectName(), Brewtarget::UNIT).toInt());
}

Unit::unitScale SaltTableModel::displayScale(int column) const
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return Unit::noScale;

   return static_cast<Unit::unitScale>(Brewtarget::option(attribute, QVariant(-1), this->objectName(), Brewtarget::SCALE).toInt());
}

void SaltTableModel::setDisplayUnit(int column, Unit::unitDisplay displayUnit)
{
   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayUnit,this->objectName(),Brewtarget::UNIT);
   Brewtarget::setOption(attribute,Unit::noScale,this->objectName(),Brewtarget::SCALE);

}

// Setting the scale should clear any cell-level scaling options
void SaltTableModel::setDisplayScale(int column, Unit::unitScale displayScale)
{

   QString attribute = generateName(column);

   if ( attribute.isEmpty() )
      return;

   Brewtarget::setOption(attribute,displayScale,this->objectName(),Brewtarget::SCALE);

}

QString SaltTableModel::generateName(int column) const
{
   QString attribute;

   switch(column)
   {
      case SALTAMOUNTCOL:
         attribute = "amount";
         break;
      default:
         attribute = "";
   }
   return attribute;
}

void SaltTableModel::contextMenu(const QPoint &point)
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
      case SALTAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(parentTableWidget,currentUnit, currentScale);
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

//==========================CLASS SaltItemDelegate===============================

SaltItemDelegate::SaltItemDelegate(QObject* parent)
        : QItemDelegate(parent)
{
}

QWidget* SaltItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
   if ( index.column() == SALTADDTOCOL ) {
      QComboBox *box = new QComboBox(parent);

      box->addItem(tr("Never"));
      box->addItem(tr("Mash"));
      box->addItem(tr("Sparge"));
      box->addItem(tr("Both"));
      box->setMinimumWidth( box->minimumSizeHint().width());
      box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      return box;
   }

   return new QLineEdit(parent);
}

void SaltItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   if ( index.column() == SALTADDTOCOL ) {
      QComboBox* box = static_cast<QComboBox*>(editor);
      box->setCurrentIndex(index.model()->data(index,Qt::UserRole).toInt());
   }
   else {
      QLineEdit* line = static_cast<QLineEdit*>(editor);
      line->setText(index.model()->data(index, Qt::DisplayRole).toString());
   }
}

void SaltItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{

   if ( index.column() == SALTADDTOCOL ) {
      QComboBox* box = static_cast<QComboBox*>(editor);
      int curr = box->currentIndex();
      int ndx = index.model()->data(index,Qt::UserRole).toInt();

      if ( curr != ndx ) {
         model->setData(index,curr,Qt::EditRole);
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
