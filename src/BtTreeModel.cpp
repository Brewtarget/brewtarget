/*
 * BtTreeModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Mik Firestone <mikfire@gmail.com>
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

#include <QModelIndex>
#include <QMessageBox>
#include <QVariant>
#include <QList>
#include <QAbstractItemModel>
#include <Qt>
#include <QObject>
#include <QStringBuilder>
#include <QMimeData>

#include "brewtarget.h"
#include "AncestorDialog.h"
#include "BtTreeItem.h"
#include "BtTreeModel.h"
#include "BtTreeView.h"
#include "RecipeFormatter.h"
#include "database.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Yeast.h"
#include "model/BrewNote.h"
#include "model/Style.h"
#include "model/Water.h"

// =========================================================================
// ============================ CLASS STUFF ================================
// =========================================================================

BtTreeModel::BtTreeModel(BtTreeView *parent, TypeMasks type)
   : QAbstractItemModel(parent)
{
   // Initialize the tree structure
   int items = 0;
   rootItem = new BtTreeItem();

   switch (type)
   {
      case RECIPEMASK:
         rootItem->insertChildren(items,1,BtTreeItem::RECIPE);
         connect( &(Database::instance()), qOverload<Recipe*>(&Database::createdSignal),     this, qOverload<Recipe*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<Recipe*>(&Database::deletedSignal), this, qOverload<Recipe*>(&BtTreeModel::elementRemoved));
         // Brewnotes need love too!
         connect( &(Database::instance()), qOverload<BrewNote*>(&Database::createdSignal),     this, qOverload<BrewNote*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<BrewNote*>(&Database::deletedSignal), this, qOverload<BrewNote*>(&BtTreeModel::elementRemoved));
         // And some versioning stuff, because why not?
         connect( &(Database::instance()), &Database::spawned, this, &BtTreeModel::versionedRecipe );

         _type = BtTreeItem::RECIPE;
         _mimeType = "application/x-brewtarget-recipe";
         m_maxColumns = BtTreeItem::RECIPENUMCOLS;
         break;
      case EQUIPMASK:
         rootItem->insertChildren(items,1,BtTreeItem::EQUIPMENT);
         connect( &(Database::instance()), qOverload<Equipment*>(&Database::createdSignal),     this, qOverload<Equipment*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<Equipment*>(&Database::deletedSignal), this, qOverload<Equipment*>(&BtTreeModel::elementRemoved));
         _type = BtTreeItem::EQUIPMENT;
         _mimeType = "application/x-brewtarget-recipe";
         m_maxColumns = BtTreeItem::EQUIPMENTNUMCOLS;
         break;
      case FERMENTMASK:
         rootItem->insertChildren(items,1,BtTreeItem::FERMENTABLE);
         connect( &(Database::instance()), qOverload<Fermentable*>(&Database::createdSignal),     this, qOverload<Fermentable*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<Fermentable*>(&Database::deletedSignal), this, qOverload<Fermentable*>(&BtTreeModel::elementRemoved));
         _type = BtTreeItem::FERMENTABLE;
         _mimeType = "application/x-brewtarget-ingredient";
         m_maxColumns = BtTreeItem::FERMENTABLENUMCOLS;
         break;
      case HOPMASK:
         rootItem->insertChildren(items,1,BtTreeItem::HOP);
         connect( &(Database::instance()), qOverload<Hop*>(&Database::createdSignal),this, qOverload<Hop*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<Hop*>(&Database::deletedSignal),this, qOverload<Hop*>(&BtTreeModel::elementRemoved));
         _type = BtTreeItem::HOP;
         _mimeType = "application/x-brewtarget-ingredient";
         m_maxColumns = BtTreeItem::HOPNUMCOLS;
         break;
      case MISCMASK:
         rootItem->insertChildren(items,1,BtTreeItem::MISC);
         connect( &(Database::instance()), qOverload<Misc*>(&Database::createdSignal),this, qOverload<Misc*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<Misc*>(&Database::deletedSignal),this, qOverload<Misc*>(&BtTreeModel::elementRemoved));
         _type = BtTreeItem::MISC;
         _mimeType = "application/x-brewtarget-ingredient";
         m_maxColumns = BtTreeItem::MISCNUMCOLS;
         break;
      case STYLEMASK:
         rootItem->insertChildren(items,1,BtTreeItem::STYLE);
         connect( &(Database::instance()), qOverload<Style*>(&Database::createdSignal),this, qOverload<Style*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<Style*>(&Database::deletedSignal),this, qOverload<Style*>(&BtTreeModel::elementRemoved));
         _type = BtTreeItem::STYLE;
         _mimeType = "application/x-brewtarget-recipe";
         m_maxColumns = BtTreeItem::STYLENUMCOLS;
         break;
      case YEASTMASK:
         rootItem->insertChildren(items,1,BtTreeItem::YEAST);
         connect( &(Database::instance()), qOverload<Yeast*>(&Database::createdSignal),this, qOverload<Yeast*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<Yeast*>(&Database::deletedSignal),this, qOverload<Yeast*>(&BtTreeModel::elementRemoved));
         _type = BtTreeItem::YEAST;
         _mimeType = "application/x-brewtarget-ingredient";
         m_maxColumns = BtTreeItem::YEASTNUMCOLS;
         break;
      case WATERMASK:
         rootItem->insertChildren(items,1,BtTreeItem::WATER);
         connect( &(Database::instance()), qOverload<Water*>(&Database::createdSignal),this, qOverload<Water*>(&BtTreeModel::elementAdded));
         connect( &(Database::instance()), qOverload<Water*>(&Database::deletedSignal),this, qOverload<Water*>(&BtTreeModel::elementRemoved));
         _type = BtTreeItem::WATER;
         _mimeType = "application/x-brewtarget-ingredient";
         m_maxColumns = BtTreeItem::WATERNUMCOLS;
         break;
      default:
         qWarning() << QString("Invalid treemask: %1").arg(type);
   }

   treeMask = type;
   parentTree = parent;
   loadTreeModel();
}

BtTreeModel::~BtTreeModel()
{
   delete rootItem;
   rootItem = nullptr;
}

// =========================================================================
// =================== ABSTRACTITEMMODEL STUFF =============================
// =========================================================================

BtTreeItem *BtTreeModel::item( const QModelIndex &index ) const
{
   if ( index.isValid())
   {
      BtTreeItem *item = static_cast<BtTreeItem*>(index.internalPointer());
      if (item)
         return item;
   }

   return rootItem;
}

int BtTreeModel::rowCount(const QModelIndex &parent) const
{
   if (! parent.isValid())
      return rootItem->childCount();

   return item(parent)->childCount();
}

int BtTreeModel::columnCount( const QModelIndex &parent) const
{
   Q_UNUSED(parent)
   return m_maxColumns;
}

Qt::ItemFlags BtTreeModel::flags(const QModelIndex &index) const
{
   if (!index.isValid())
      return Qt::ItemIsDropEnabled;

   return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QModelIndex BtTreeModel::index( int row, int column, const QModelIndex &parent) const
{
   BtTreeItem *pItem, *cItem;

   if ( parent.isValid() && parent.column() >= columnCount(parent) )
      return QModelIndex();

   pItem = item(parent);
   cItem = pItem->child(row);

   if (cItem)
      return createIndex(row,column,cItem);
   else
      return QModelIndex();
}

QModelIndex BtTreeModel::parent(const QModelIndex &index) const
{
   BtTreeItem *pItem, *cItem;

   if (!index.isValid())
      return QModelIndex();

   cItem = item(index);

   if ( cItem == nullptr )
      return QModelIndex();

   pItem = cItem->parent();

   if (pItem == rootItem || pItem == nullptr )
      return QModelIndex();

   return createIndex(pItem->childNumber(),0,pItem);
}

QModelIndex BtTreeModel::first()
{
   BtTreeItem* pItem;

   // get the first item in the list, which is the place holder
   pItem = rootItem->child(0);
   if ( pItem->childCount() > 0 )
      return createIndex(0,0,pItem->child(0));

   return QModelIndex();
}

QVariant BtTreeModel::data(const QModelIndex &index, int role) const
{
   int maxColumns;

   if ( treeMask == FOLDERMASK ) {
      maxColumns = BtTreeItem::FOLDERNUMCOLS;
   }
   else {
      maxColumns = m_maxColumns;
   }

   if ( !rootItem || !index.isValid() || index.column() < 0 || index.column() >= maxColumns)
      return QVariant();

   BtTreeItem* itm = item(index);

   QFont font;
   switch(role) {
      case Qt::ToolTipRole:
         return toolTipData(index);
      case Qt::DisplayRole:
         return itm->data(index.column());
      case Qt::DecorationRole:
         if ( index.column() == 0 && itm->type() == BtTreeItem::FOLDER ) {
            return QIcon(":images/folder.png");
         }
         break;
      default:
         break;
   }

   return QVariant();
}

QVariant BtTreeModel::toolTipData(const QModelIndex &index) const
{
   RecipeFormatter* whiskey = new RecipeFormatter();

   switch(treeMask)
   {
      case RECIPEMASK:
         return whiskey->getToolTip(qobject_cast<Recipe*>(thing(index)));
      case STYLEMASK:
         return whiskey->getToolTip( qobject_cast<Style*>(thing(index)));
      case EQUIPMASK:
         return whiskey->getToolTip( qobject_cast<Equipment*>(thing(index)));
      case FERMENTMASK:
         return whiskey->getToolTip( qobject_cast<Fermentable*>(thing(index)));
      case HOPMASK:
         return whiskey->getToolTip( qobject_cast<Hop*>(thing(index)));
      case MISCMASK:
         return whiskey->getToolTip( qobject_cast<Misc*>(thing(index)));
      case YEASTMASK:
         return whiskey->getToolTip( qobject_cast<Yeast*>(thing(index)));
      case WATERMASK:
         return whiskey->getToolTip( qobject_cast<Water*>(thing(index)));
      default:
         return item(index)->name();
   }
}

// This is much better, assuming the rest can be made to work
QVariant BtTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
      return QVariant();

   switch(treeMask)
   {
   case RECIPEMASK:
      return recipeHeader(section);
   case EQUIPMASK:
      return equipmentHeader(section);
   case FERMENTMASK:
      return fermentableHeader(section);
   case HOPMASK:
      return hopHeader(section);
   case MISCMASK:
      return miscHeader(section);
   case YEASTMASK:
      return yeastHeader(section);
   case STYLEMASK:
      return styleHeader(section);
   case FOLDERMASK:
      return folderHeader(section);
   case WATERMASK:
      return waterHeader(section);
   default:
      return QVariant();
   }
}

QVariant BtTreeModel::recipeHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::RECIPENAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::RECIPEANCCOUNT:
      return QVariant(tr("Snapshots"));
   case BtTreeItem::RECIPEBREWDATECOL:
      return QVariant(tr("Brew Date"));
   case BtTreeItem::RECIPESTYLECOL:
      return QVariant(tr("Style"));
   }

   qWarning() << QString("BtTreeModel::getRecipeHeader Bad column: %1").arg(section);
   return QVariant();
}

QVariant BtTreeModel::equipmentHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::EQUIPMENTNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::EQUIPMENTBOILTIMECOL:
      return QVariant(tr("Boil Time"));
   }

   qWarning() << QString("BtTreeModel::getEquipmentHeader Bad column: %1").arg(section);
   return QVariant();
}

QVariant BtTreeModel::fermentableHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::FERMENTABLENAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::FERMENTABLECOLORCOL:
      return QVariant(tr("Color"));
   case BtTreeItem::FERMENTABLETYPECOL:
      return QVariant(tr("Type"));
   }

   qWarning() << QString("BtTreeModel::getFermentableHeader Bad column: %1").arg(section);
   return QVariant();
}

QVariant BtTreeModel::hopHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::HOPNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::HOPFORMCOL:
      return QVariant(tr("Type"));
   case BtTreeItem::HOPUSECOL:
      return QVariant(tr("Use"));
   }

   qWarning() << QString("BtTreeModel::getHopHeader Bad column: %1").arg(section);
   return QVariant();
}

QVariant BtTreeModel::miscHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::MISCNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::MISCTYPECOL:
      return QVariant(tr("Type"));
   case BtTreeItem::MISCUSECOL:
      return QVariant(tr("Use"));
   }

   qWarning() << QString("BtTreeModel::getMiscHeader Bad column: %1").arg(section);
   return QVariant();
}

QVariant BtTreeModel::yeastHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::YEASTNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::YEASTTYPECOL:
      return QVariant(tr("Type"));
   case BtTreeItem::YEASTFORMCOL:
      return QVariant(tr("Form"));
   }

   qWarning() << QString("BtTreeModel::getYeastHeader Bad column: %1").arg(section);
   return QVariant();
}

QVariant BtTreeModel::styleHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::STYLENAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::STYLECATEGORYCOL:
      return QVariant(tr("Category"));
   case BtTreeItem::STYLENUMBERCOL:
      return QVariant(tr("Number"));
   case BtTreeItem::STYLELETTERCOL:
      return QVariant(tr("Letter"));
   case BtTreeItem::STYLEGUIDECOL:
      return QVariant(tr("Guide"));
   }

   qWarning() << QString("BtTreeModel::getYeastHeader Bad column: %1").arg(section);
   return QVariant();
}

QVariant BtTreeModel::folderHeader(int section) const
{
   switch(section)
   {
      case BtTreeItem::FOLDERNAMECOL:
         return QVariant(tr("Name"));
      case BtTreeItem::FOLDERPATHCOL:
         return QVariant(tr("PATH"));
      case BtTreeItem::FOLDERFULLCOL:
         return QVariant(tr("FULLPATH"));
   }

   qWarning() << QString("BtTreeModel::getFolderHeader Bad column: %1").arg(section);
   return QVariant();
}

QVariant BtTreeModel::waterHeader(int section) const
{
   switch(section)
   {
   case BtTreeItem::WATERNAMECOL:
      return QVariant(tr("Name"));
   case BtTreeItem::WATERCACOL:
      return QVariant(tr("Ca"));
   case BtTreeItem::WATERHCO3COL:
      return QVariant(tr("HCO3"));
   case BtTreeItem::WATERSO4COL:
      return QVariant(tr("SO4"));
   case BtTreeItem::WATERCLCOL:
      return QVariant(tr("Cl"));
   case BtTreeItem::WATERNACOL:
      return QVariant(tr("Na"));
   case BtTreeItem::WATERMGCOL:
      return QVariant(tr("Mg"));
   case BtTreeItem::WATERpHCOL:
      return QVariant(tr("pH"));
   }
   qWarning() << QString("BtTreeModel::waterHeader Bad column: %1").arg(section);
   return QVariant();

}

bool BtTreeModel::insertRow(int row, const QModelIndex &parent, QObject* victim, int victimType )
{
   if ( ! parent.isValid() )
      return false;

   BtTreeItem *pItem = item(parent);
   int type = pItem->type();

   bool success = true;

   beginInsertRows(parent,row,row);
   success = pItem->insertChildren(row,1,type);
   if ( victim && success )
   {
      type = victimType == -1 ? type : victimType;
      BtTreeItem* added = pItem->child(row);
      added->setData(type, victim);
   }
   endInsertRows();

   return success;
}

bool BtTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
   BtTreeItem *pItem = item(parent);
   bool success = true;

   beginRemoveRows(parent, row, row + count -1 );
   success = pItem->removeChildren(row,count);
   endRemoveRows();

   return success;
}

// =========================================================================
// ====================== BREWTARGET STUFF =================================
// =========================================================================

// One find method for all things. This .. is nice
QModelIndex BtTreeModel::findElement(NamedEntity* thing, BtTreeItem* parent)
{
   BtTreeItem* pItem;
   QList<BtTreeItem*> folders;

   int i;

   if ( parent == nullptr )
      pItem = rootItem->child(0);
   else
      pItem = parent;

   if (! thing )
      return createIndex(0,0,pItem);

   folders.append(pItem);

   while ( ! folders.isEmpty() )
   {
      BtTreeItem* target = folders.takeFirst();
      for(i=0; i < target->childCount(); ++i)
      {
         // If we've found what we are looking for, return
         if ( target->child(i)->thing() == thing )
            return createIndex(i,0,target->child(i));

         // If we have a folder, or we are looking for a brewnote and have a
         // recipe in hand, push the child onto the stack
         if ( target->child(i)->type() == BtTreeItem::FOLDER ||
              (qobject_cast<BrewNote*>(thing) && target->child(i)->type() == BtTreeItem::RECIPE ) )
            folders.append(target->child(i));
      }
   }
   return QModelIndex();
}

QList<NamedEntity*> BtTreeModel::elements()
{
   QList<NamedEntity*> elements;
   switch(treeMask)
   {
   case RECIPEMASK:
      foreach( NamedEntity* elem, Database::instance().recipes() )
         elements.append(elem);
      break;
   case EQUIPMASK:
      foreach( NamedEntity* elem, Database::instance().equipments() )
         elements.append(elem);
      break;
   case FERMENTMASK:
      foreach( NamedEntity* elem, Database::instance().fermentables() )
         elements.append(elem);
      break;
   case HOPMASK:
      foreach( NamedEntity* elem, Database::instance().hops() )
         elements.append(elem);
      break;
   case MISCMASK:
      foreach( NamedEntity* elem, Database::instance().miscs() )
         elements.append(elem);
      break;
   case YEASTMASK:
      foreach( NamedEntity* elem, Database::instance().yeasts() )
         elements.append(elem);
      break;
   case STYLEMASK:
      foreach( NamedEntity* elem, Database::instance().styles() )
         elements.append(elem);
      break;
   case WATERMASK:
      foreach( NamedEntity* elem, Database::instance().waters() )
         elements.append(elem);
      break;
   default:
      qWarning() << QString("Invalid treemask: %1").arg(treeMask);
   }
   return elements;
}

void BtTreeModel::loadTreeModel()
{
   int i;

   QModelIndex ndxLocal;
   BtTreeItem* local = nullptr;
   QList<NamedEntity*> elems = elements();

   foreach( NamedEntity* elem, elems ) {

      if (! elem->folder().isEmpty() ) {
         ndxLocal = findFolder( elem->folder(), rootItem->child(0), true );
         // I cannot imagine this failing, but what the hell
         if ( ! ndxLocal.isValid() ) {
            qWarning() << "Invalid return from findFolder in loadTreeModel()";
            continue;
         }
         local = item(ndxLocal);
         i = local->childCount();
      }
      else {
         local = rootItem->child(0);
         i = local->childCount();
         ndxLocal = createIndex(i,0,local);
      }

      if ( ! insertRow(i,ndxLocal,elem,_type) ) {
         qWarning() << "Insert failed in loadTreeModel()";
         continue;
      }

      // If we have brewnotes, set them up here.
      if ( treeMask & RECIPEMASK ) {
         Recipe *holdmebeer = qobject_cast<Recipe*>(elem);
         if ( Brewtarget::option("showsnapshots", false).toBool() && holdmebeer->hasAncestors() ) {
            setShowChild(ndxLocal,true);
            addAncestoralTree(holdmebeer, i, local);
            addBrewNoteSubTree(holdmebeer,i,local,false);
         }
         else {
            addBrewNoteSubTree(holdmebeer,i,local);
         }
      }
      observeElement(elem);
   }
}

void BtTreeModel::addAncestoralTree(Recipe* rec, int i, BtTreeItem* parent)
{
   BtTreeItem* temp = parent->child(i);
   int j = 0;

   foreach( Recipe* stor, rec->ancestors() ) {
      // a recipe's ancestor list always has itself in it. Skip that entry
      if ( stor == rec ) {
         continue;
      }
      // insert the ancestor. This is most of magic. One day, I understood it.
      // Now I simply copy/paste it
      if ( ! insertRow(j, createIndex(i,0,temp), stor, BtTreeItem::RECIPE) ) {
         qWarning() << "Ancestor insert failed in loadTreeModel()";
         continue;
      }
      // we need to find the index of what we just inserted
      QModelIndex cIndex = findElement(stor,temp);
      // and set showChild on it
      setShowChild(cIndex,true);

      // finally, add this ancestors brewnotes but do not recurse
      addBrewNoteSubTree(stor,j,temp,false);
      observeElement(stor);
      ++j;
   }
}

void BtTreeModel::addBrewNoteSubTree(Recipe* rec, int i, BtTreeItem* parent, bool recurse)
{
   QList<BrewNote*> notes = rec->brewNotes(recurse);
   BtTreeItem* temp = parent->child(i);

   int j = 0;

   foreach( BrewNote* note, notes )
   {
      // In previous insert loops, we ignore the error and soldier on. So we
      // will do that here too
      if ( ! insertRow(j, createIndex(i,0,temp), note, BtTreeItem::BREWNOTE) )
      {
         qWarning() << "Brewnote insert failed in loadTreeModel()";
         continue;
      }
      observeElement(note);
      ++j;
   }
}

Recipe* BtTreeModel::recipe(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->recipe() : nullptr;
}

Equipment* BtTreeModel::equipment(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->equipment() : nullptr;
}

Fermentable* BtTreeModel::fermentable(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->fermentable() : nullptr;
}

Hop* BtTreeModel::hop(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->hop() : nullptr;
}

Misc* BtTreeModel::misc(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->misc() : nullptr;
}

Yeast* BtTreeModel::yeast(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->yeast() : nullptr;
}

Style* BtTreeModel::style(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->style() : nullptr;
}

BrewNote* BtTreeModel::brewNote(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->brewNote() : nullptr;
}

BtFolder* BtTreeModel::folder(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->folder() : nullptr;
}

Water* BtTreeModel::water(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->water() : nullptr;
}

NamedEntity* BtTreeModel::thing(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->thing() : nullptr;
}

bool BtTreeModel::isRecipe(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::RECIPE;
}

bool BtTreeModel::isEquipment(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::EQUIPMENT;
}

bool BtTreeModel::isFermentable(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::FERMENTABLE;
}

bool BtTreeModel::isHop(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::HOP;
}

bool BtTreeModel::isMisc(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::MISC;
}

bool BtTreeModel::isYeast(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::YEAST;
}

bool BtTreeModel::isStyle(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::STYLE;
}

bool BtTreeModel::isBrewNote(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::BREWNOTE;
}

bool BtTreeModel::isWater(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::WATER;
}

bool BtTreeModel::isFolder(const QModelIndex &index) const
{
   return type(index) == BtTreeItem::FOLDER;
}

int BtTreeModel::type(const QModelIndex &index) const
{
   return index.isValid() ? item(index)->type() : -1;
}

QString BtTreeModel::name(const QModelIndex &idx)
{
   return idx.isValid() ? item(idx)->name() : QString();
}

int BtTreeModel::mask()
{
   return treeMask;
}

void BtTreeModel::copySelected(QList< QPair<QModelIndex, QString> > toBeCopied)
{
   bool failed = false;

   while ( ! toBeCopied.isEmpty() )
   {
      QPair<QModelIndex,QString> thisPair = toBeCopied.takeFirst();
      QModelIndex ndx = thisPair.first;
      QString name = thisPair.second;

      switch ( type(ndx) ) {
         case BtTreeItem::EQUIPMENT:
            Equipment *oldKit, *copyKit;
            oldKit = equipment(ndx);
            copyKit = Database::instance().newEquipment(oldKit); // Create a deep copy.
            if (copyKit) {
               copyKit->setName(name);
               folderChanged(copyKit);
            }
            else {
               failed = true;
            }
            break;
         case BtTreeItem::FERMENTABLE:
            Fermentable *oldFerm, *copyFerm;
            oldFerm = fermentable(ndx);
            // Create a deep copy with a new inventory row
            copyFerm = Database::instance().newFermentable(oldFerm,true);
            if ( copyFerm ) {
               copyFerm->setName(name);
               folderChanged(copyFerm);
            }
            else
               failed = true;
            break;
         case BtTreeItem::HOP:
            Hop *copyHop,  *oldHop;
            oldHop = hop(ndx);
            // Create a deep copy with a new inventory row
            copyHop = Database::instance().newHop(oldHop,true);
            if ( copyHop ) {
               copyHop->setName(name);
               folderChanged(copyHop);
            }
            else
               failed = true;
            break;
         case BtTreeItem::MISC:
            Misc *copyMisc, *oldMisc;
            oldMisc = misc(ndx);
            // Create a deep copy with a new inventory row
            copyMisc = Database::instance().newMisc(oldMisc,true);
            if ( copyMisc ) {
               copyMisc->setName(name);
               folderChanged(copyMisc);
            }
            else
               failed = true;
            break;
         case BtTreeItem::RECIPE:
            Recipe *copyRec,  *oldRec;
            oldRec = recipe(ndx);
            copyRec = Database::instance().newRecipe(oldRec); // Create a deep copy.
            qInfo() << Q_FUNC_INFO << "display:" <<  copyRec->display() << "isLocked:" << copyRec->locked() << "hasDescendants:" << copyRec->hasDescendants();
            if ( copyRec ) {
               copyRec->setName(name);
               folderChanged(copyRec);
            }
            else
               failed = true;
            break;
         case BtTreeItem::STYLE:
            Style *copyStyle, *oldStyle;
            oldStyle = style(ndx);
            copyStyle = Database::instance().newStyle(oldStyle); // Create a deep copy.
            if ( copyStyle ) {
               copyStyle->setName(name);
               folderChanged(copyStyle);
            }
            else
               failed = true;
            break;
         case BtTreeItem::YEAST:
            Yeast *copyYeast, *oldYeast;
            oldYeast = yeast(ndx);
            // Create a deep copy with a new inventory row
            copyYeast = Database::instance().newYeast(oldYeast,true);
            if ( copyYeast ) {
               copyYeast->setName(name);
               folderChanged(copyYeast);
            }
            else
               failed = true;
            break;
         case BtTreeItem::WATER:
            Water *copyWater, *oldWater;
            oldWater = water(ndx);
            copyWater = Database::instance().newWater(oldWater); // Create a deep copy.
            if ( copyWater ) {
               copyWater->setName(name);
               folderChanged(copyWater);
            }
            else
               failed = true;
            break;
         default:
            qWarning() << QString("copySelected:: unknown type %1").arg(type(ndx));
      }
      if ( failed ) {
         QMessageBox::warning(nullptr,
                              tr("Could not copy"),
                              tr("There was an unexpected error creating %1").arg(name));
         return;
      }
   }
}

void BtTreeModel::deleteSelected(QModelIndexList victims)
{
   QModelIndexList toBeDeleted = victims; // trust me
   Recipe *rec;
   int deletewhat;

   // There are black zones of shadow close to our daily paths,
   // and now and then some evil soul breaks a passage through.
   while ( ! toBeDeleted.isEmpty() )
   {
      QModelIndex ndx = toBeDeleted.takeFirst();
      switch ( type(ndx) )
      {
         case BtTreeItem::EQUIPMENT:
            Database::instance().remove( equipment(ndx) );
            break;
         case BtTreeItem::FERMENTABLE:
            Database::instance().remove( fermentable(ndx) );
            break;
         case BtTreeItem::HOP:
            Database::instance().remove( hop(ndx) );
            break;
         case BtTreeItem::MISC:
            Database::instance().remove( misc(ndx) );
            break;
         case BtTreeItem::RECIPE:
            rec = recipe(ndx);
            deletewhat = Brewtarget::option("deletewhat", Brewtarget::DESCENDANT).toInt();
            if ( deletewhat == Brewtarget::DESCENDANT ) {
               orphanRecipe(ndx);
               Database::instance().remove( rec );
            }
            else {
               Database::instance().remove(  rec->ancestors() );
            }
            break;
         case BtTreeItem::STYLE:
            Database::instance().remove( style(ndx) );
            break;
         case BtTreeItem::YEAST:
            Database::instance().remove( yeast(ndx) );
            break;
         case BtTreeItem::BREWNOTE:
            Database::instance().remove( brewNote(ndx) );
            break;
         case BtTreeItem::WATER:
            Database::instance().remove( water(ndx) );
            break;
         case BtTreeItem::FOLDER:
            // This one is weird.
            toBeDeleted += allChildren(ndx);
            removeFolder(ndx);
            break;
         default:
            qWarning() << QString("deleteSelected:: unknown type %1").arg(type(ndx));
      }
   }
}

// =========================================================================
// ============================ FOLDER STUFF ===============================
// =========================================================================

// The actual magic shouldn't be hard. Once we trap the signal, find the
// recipe, remove it from the parent and add it to the target folder.
// It is not easy. Indexes are ephemeral things. We MUST calculate the insert
// index after we have removed the recipe. BAD THINGS happen otherwise.
//
void BtTreeModel::folderChanged(QString name)
{
   NamedEntity* test = qobject_cast<NamedEntity*>(sender());

   Q_UNUSED(name)
   if ( ! test )
      return;

   folderChanged(test);
}

void BtTreeModel::folderChanged(NamedEntity* test)
{
   // Find it.
   QModelIndex ndx = findElement(test);
   if ( ! ndx.isValid() ) {
      qWarning() << "folderChanged:: could not find element";
      return;
   }


   QModelIndex pIndex;
   bool expand = true;

   pIndex = parent(ndx); // Get the parent
   // If the parent isn't valid, its the root
   if ( ! pIndex.isValid() )
      pIndex = createIndex(0,0,rootItem->child(0));

   int i = item(ndx)->childNumber();

   // Remove it
   if ( ! removeRows(i, 1, pIndex) ) {
      qWarning() << "folderChanged:: could not remove row";
      return;
   }

   // Find the new parent
   // That's awkward, but dropping a folder prolly does need a the folder
   // created.
   QModelIndex newNdx = findFolder(test->folder(), rootItem->child(0), true);
   if ( ! newNdx.isValid() ) {
      newNdx = createIndex(0,0,rootItem->child(0));
      expand = false;
   }

   BtTreeItem* local = item(newNdx);
   int j = local->childCount();

   if ( !  insertRow(j,newNdx,test,_type) ) {
      qWarning() << "folderChanged:: could not insert row";
      return;
   }
   // If we have brewnotes, set them up here.
   if ( treeMask & RECIPEMASK )
      addBrewNoteSubTree(qobject_cast<Recipe*>(test),j,local);

   if ( expand )
      emit expandFolder(treeMask,newNdx);
   return;
}

bool BtTreeModel::addFolder(QString name)
{
   return findFolder(name, rootItem->child(0), true).isValid();
}

bool BtTreeModel::removeFolder(QModelIndex ndx)
{
   if ( ! ndx.isValid() )
      return false;

   int i = -1;
   QModelIndex pInd = parent(ndx);

   if ( ! pInd.isValid() )
      return false;

   BtTreeItem* start = item(ndx);

   // Remove the victim.
   i = start->childNumber();
   return removeRows(i, 1, pInd);
}

QModelIndexList BtTreeModel::allChildren(QModelIndex ndx)
{
   QModelIndexList leafNodes;
   QList<BtTreeItem*> folders;
   int i;

   // Don't send an invalid index or something that isn't a folder
   if ( ! ndx.isValid() || type(ndx) != BtTreeItem::FOLDER )
      return leafNodes;

   BtTreeItem* start = item(ndx);
   folders.append(start);

   while ( ! folders.isEmpty() )
   {
      BtTreeItem* target = folders.takeFirst();

      for (i=0; i < target->childCount(); ++i)
      {
         BtTreeItem* next = target->child(i);
         // If a folder, push it onto the folders stack for later processing
         if ( next->type() == BtTreeItem::FOLDER )
            folders.append(next);
         else // Leafnode
            leafNodes.append(createIndex(i,0,next));
      }
   }
   return leafNodes;
}

bool BtTreeModel::renameFolder(BtFolder* victim, QString newName)
{
   QModelIndex ndx = findFolder(victim->fullPath(), nullptr, false);
   QModelIndex pInd;
   QString targetPath = newName % "/" % victim->name();
   QPair<QString,BtTreeItem*> f;
   QList<QPair<QString, BtTreeItem*> > folders;
   // This space is important       ^
   int i, kids,src;

   if ( ! ndx.isValid() )
      return false;

   pInd = parent(ndx);
   if ( ! pInd.isValid() )
      return false;

   BtTreeItem* start = item(ndx);
   f.first  = targetPath;
   f.second = start;

   folders.append(f);

   while ( ! folders.isEmpty() )
   {
      // This looks weird, but it is needed for later
      f = folders.takeFirst();
      targetPath = f.first;
      BtTreeItem* target = f.second;

      // As we move things, childCount changes. This makes sure we loop
      // through all of the kids
      kids = target->childCount();
      src = 0;
      // Ok. We have a start and an index.
      for (i=0; i < kids; ++i)
      {
         // This looks weird and it is. As we move children out, the 0 items
         // changes to the next child. In the case of a folder, though, we
         // don't move it, so we need to get the item beyond that.
         BtTreeItem* next = target->child(src);
         // If a folder, push it onto the folders stack for latter processing
         if ( next->type() == BtTreeItem::FOLDER )
         {
            QPair<QString,BtTreeItem*> newTarget;
            newTarget.first = targetPath % "/" % next->name();
            newTarget.second = next;
            folders.append(newTarget);
            src++;
         }
         else // Leafnode
            next->thing()->setFolder(targetPath);
      }
   }
   // Last thing is to remove the victim.
   i = start->childNumber();
   return removeRows(i, 1, pInd);
}

QModelIndex BtTreeModel::createFolderTree( QStringList dirs, BtTreeItem* parent, QString pPath)
{
   BtTreeItem* pItem = parent;

   // Start the loop. We are going to return ndx at the end,
   // so we need to declare and initialize outside of the loop
   QModelIndex ndx = createIndex(pItem->childCount(),0,pItem);

   // Need to call this because we are adding different things with different
   // column counts. Just using the rowsAboutToBeAdded throws ugly errors and
   // then a sigsegv
   emit layoutAboutToBeChanged();
   foreach ( QString cur, dirs )
   {
      QString fPath;
      BtFolder* temp = new BtFolder();
      int i;

      // If the parent item is a folder, use its full path
      if ( pItem->type() == BtTreeItem::FOLDER )
         fPath = pItem->folder()->fullPath() % "/" % cur;
      else
         fPath = pPath % "/" % cur; // If it isn't we need the parent path

      fPath.replace(QRegExp("//"), "/");

      // Set the full path, which will set the name and the path
      temp->setfullPath(fPath);
      i = pItem->childCount();

      pItem->insertChildren(i, 1, BtTreeItem::FOLDER);
      pItem->child(i)->setData(BtTreeItem::FOLDER, temp);

      // Set the parent item to point to the newly created tree
      pItem = pItem->child(i);

      // And this for the return
      ndx = createIndex(pItem->childCount(), 0, pItem);
   }
   emit layoutChanged();

   // May K&R have mercy on my soul
   return ndx;
}

QModelIndex BtTreeModel::findFolder( QString name, BtTreeItem* parent, bool create )
{
   BtTreeItem* pItem;
   QStringList dirs;
   QString current, fullPath, targetPath;
   int i;

   pItem = parent ? parent : rootItem->child(0);

   // Upstream interfaces should handle this for me, but I like belt and
   // suspenders
   name = name.simplified();
   // I am assuming asking me to find an empty name means find the root of the
   // tree.
   if ( name.isEmpty() )
      return createIndex(0,0,pItem);

   // Prepare all the variables for the first loop

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   dirs = name.split("/", QString::SkipEmptyParts);
#else
   dirs = name.split("/", Qt::SkipEmptyParts);
#endif

   if ( dirs.isEmpty() )
      return QModelIndex();

   current = dirs.takeFirst();
   fullPath = "/";
   targetPath = fullPath % current;

   i = 0;

   // Time to get funky with no recursion!
   while( i < pItem->childCount() ) {
      BtTreeItem* kid = pItem->child(i);
      // The kid is a folder
      if ( kid->type() == BtTreeItem::FOLDER ) {
         // The folder name matches the part we are looking at
         if ( kid->folder()->isFolder(targetPath) ) {
            // If there are no more subtrees to look for, we found it
            if ( dirs.isEmpty() )
               return createIndex(i,0,kid);
            // Otherwise, we found a parent folder in our path
            else {
               // get the next folder in the path
               current = dirs.takeFirst();
               // append that to the fullPath we are looking for
               fullPath = targetPath;
               targetPath = fullPath % "/" % current;

               // Set the parent to the folder
               pItem = kid;
               // Reset the counter
               i = 0;
               // And do the time warp again!
               continue;
            }
         }
      }
      // If we got this far, it wasn't a folder or it wasn't a match.
      i++;
   }
   // If we get here, we found no match.

   // If we are supposed to create something, then lets get busy
   if ( create ) {
      // push the current dir back on the stack
      dirs.prepend(current);
      // And start with the madness
      return createFolderTree( dirs, pItem, fullPath);
   }

   // If we weren't supposed to create, we drop to here and return an empty
   // index.
   return QModelIndex();
}

// =========================================================================
// ============================ SLOT STUFF ===============================
// =========================================================================

void BtTreeModel::elementChanged()
{
   NamedEntity* d = qobject_cast<NamedEntity*>(sender());
   if( !d )
      return;

   QModelIndex ndxLeft = findElement(d);
   if( ! ndxLeft.isValid() )
      return;

   QModelIndex ndxRight = createIndex(ndxLeft.row(), columnCount(ndxLeft)-1, ndxLeft.internalPointer());
   emit dataChanged( ndxLeft, ndxRight );
}

/* I don't like this part, but Qt's signal/slot mechanism are pretty
 * simplistic and do a string compare on signatures. Each one of these one
 * liners is required to give the right signature and to be able to call
 * addElement() properly
 */
void BtTreeModel::elementAdded(Recipe* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementAdded(Equipment* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementAdded(Fermentable* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementAdded(Hop* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementAdded(Misc* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementAdded(Style* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementAdded(Yeast* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementAdded(BrewNote* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementAdded(Water* victim) { elementAdded(qobject_cast<NamedEntity*>(victim)); }

// I guess this isn't too bad. Better than this same function copied 7 times
void BtTreeModel::elementAdded(NamedEntity* victim)
{
   QModelIndex pIdx;
   int lType = _type;

   if ( ! victim->display() )
      return;

   if ( qobject_cast<BrewNote*>(victim) )
   {
      pIdx = findElement(Database::instance().getParentRecipe(qobject_cast<BrewNote*>(victim)));
      lType = BtTreeItem::BREWNOTE;
   }
   else
      pIdx = createIndex(0,0,rootItem->child(0));

   if ( ! pIdx.isValid() )
      return;

   int breadth = rowCount(pIdx);

   if ( ! insertRow(breadth,pIdx,victim,lType) )
      return;

   // We need some special processing here to add brewnotes on a recipe import
   if ( qobject_cast<Recipe*>(victim) ) {
      Recipe* parent = qobject_cast<Recipe*>(victim);
      QList<BrewNote*> notes = parent->brewNotes();

      if ( notes.size() )
      {
         pIdx = findElement(parent);
         lType = BtTreeItem::BREWNOTE;
         int row = 0;
         foreach (BrewNote* note, notes)
            insertRow(row++,pIdx,note,lType);
      }
   }
   observeElement(victim);
}

void BtTreeModel::elementRemoved(Recipe* victim)      { elementRemoved(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementRemoved(Equipment* victim)   { elementRemoved(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementRemoved(Fermentable* victim) { elementRemoved(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementRemoved(Hop* victim)         { elementRemoved(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementRemoved(Misc* victim)        { elementRemoved(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementRemoved(Style* victim)       { elementRemoved(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementRemoved(Yeast* victim)       { elementRemoved(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementRemoved(BrewNote* victim)    { elementRemoved(qobject_cast<NamedEntity*>(victim)); }
void BtTreeModel::elementRemoved(Water* victim)       { elementRemoved(qobject_cast<NamedEntity*>(victim)); }

void BtTreeModel::elementRemoved(NamedEntity* victim)
{
   QModelIndex index,pIndex;

   if ( ! victim )
      return;

   index = findElement(victim);
   if ( ! index.isValid() )
      return;

   pIndex = parent(index);
   if ( ! pIndex.isValid() )
      return;

   if ( ! removeRows(index.row(),1,pIndex) )
      return;

   disconnect( victim, nullptr, this, nullptr );
}

void BtTreeModel::observeElement(NamedEntity* d)
{
   if ( ! d )
      return;

   if ( qobject_cast<BrewNote*>(d) )
      connect( d, SIGNAL(brewDateChanged(QDateTime)), this, SLOT(elementChanged()) );
   else
   {
      connect( d, SIGNAL(changedName(QString)), this, SLOT(elementChanged()) );
      connect( d, SIGNAL(changedFolder(QString)), this, SLOT(folderChanged(QString)));
   }
}


// =========================================================================
// ===================== DRAG AND DROP STUFF ===============================
// =========================================================================

NamedEntity* getElement(int oType, int id)
{
   switch(oType)
   {
      case BtTreeItem::RECIPE:
         return Database::instance().recipe(id);
      case BtTreeItem::EQUIPMENT:
         return Database::instance().equipment(id);
      case BtTreeItem::FERMENTABLE:
         return Database::instance().fermentable(id);
      case BtTreeItem::HOP:
         return Database::instance().hop(id);
      case BtTreeItem::MISC:
         return Database::instance().misc(id);
      case BtTreeItem::STYLE:
         return Database::instance().style(id);
      case BtTreeItem::YEAST:
         return Database::instance().yeast(id);
      case BtTreeItem::FOLDER:
         break;
      default:
         return nullptr;
   }
   return nullptr;
}
   
bool BtTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                               int row, int column, const QModelIndex &parent)
{
   QByteArray encodedData;

   if ( data->hasFormat(_mimeType) )
      encodedData = data->data(_mimeType);
   else if ( data->hasFormat("application/x-brewtarget-folder") )
      encodedData = data->data("application/x-brewtarget-folder");
   else
      return false; // Don't know what we got, but we don't want it


   QDataStream stream( &encodedData, QIODevice::ReadOnly);
   int oType, id;
   QString target = "";
   QString name = "";
   NamedEntity* something = nullptr;

   if ( ! parent.isValid() )
      return false;

   if ( isFolder(parent) )
      target = folder(parent)->fullPath();
   else
   {
      something = thing(parent);

      // Did you know there's a space between elements in a tree, and you can
      // actually drop things there? If somebody drops something there, don't
      // do anything
      if ( ! something )
         return false;

      target = something->folder();
   }

   // Pull the stream apart and do that which needs done. Late binding ftw!
   while( !stream.atEnd() )
   {
      QString text;
      stream >> oType >> id >> name;
      NamedEntity* elem = getElement(oType,id);

      if ( elem == nullptr && oType != BtTreeItem::FOLDER )
         return false;

      // this is the work.
      if ( oType != BtTreeItem::FOLDER ) {
         elem->setFolder(target);
      }
      else {
         BtFolder *victim = new BtFolder;
         victim->setfullPath(name);
         renameFolder(victim,target);
      }
   }

   return true;
}

// do not call up any that you can not put down
void BtTreeModel::makeAncestors(NamedEntity* ancestor, NamedEntity* descendant)
{
   if ( ancestor == descendant )
      return;

   // I need these as their recipes later, so cast'em
   Recipe * r_desc = qobject_cast<Recipe*>(descendant);
   Recipe * r_anc = qobject_cast<Recipe*>(ancestor);

   // find the ancestor in the tree
   QModelIndex ancNdx = findElement(ancestor);

   // remove the ancestor
   removeRows(ancNdx.row(), 1, this->parent(ancNdx));

   // this does the database work
   r_desc->setAncestor(r_anc);

   // now we need to find the descendant. This has to be done after we remove
   // the rows.
   QModelIndex descNdx = findElement(descendant);
   BtTreeItem* node = item(descNdx);

   // Remove all the brewnotes in the tree first. We get dupes otherwise.
   removeRows(0, node->childCount(), descNdx);

   // Add the ancestor's brewnotes to the descendant
   addBrewNoteSubTree( r_desc, descNdx.row(), node->parent(), true);
}

void BtTreeModel::orphanRecipe(QModelIndex ndx)
{
   BtTreeItem* node = item(ndx);
   BtTreeItem* pNode = node->parent();
   QModelIndex pIndex = parent(ndx);

   // I need the recipe referred to by the index
   Recipe *orphan = recipe(ndx);

   // don't do anything if there is nothing to do
   if ( ! orphan->hasAncestors() ) {
      return;
   }

   // And I need its immediate ancestor. Remember, the ancestor list always
   // has the recipe in it, so we need to reference the second item
   Recipe *ancestor = orphan->ancestors().at(1);

   // Deal with the soon-to-be orphan first
   // Remove all the rows associated with the orphan
   removeRows(0,node->childCount(),ndx);

   // This looks weird, but I think it will do what I need -- set
   // the ancestor_id to itself and reload the ancestors array. setAncestor
   // handles the locked and display flags
   orphan->setAncestor(orphan);
   // Display all of its brewnotes
   addBrewNoteSubTree(orphan, ndx.row(), pNode, false);

   // set the ancestor to visible. Not sure this is required?
   ancestor->setDisplay(true);
   ancestor->setLocked(false);

   // Put the ancestor into the tree
   if ( ! insertRow(pIndex.row(), pIndex, ancestor, BtTreeItem::RECIPE) )
      qWarning() << Q_FUNC_INFO << "Could not add ancestor to tree";

   // Find the ancestor in the tree
   QModelIndex ancNdx = findElement(ancestor);
   if ( ! ancNdx.isValid() ) {
      qWarning() << Q_FUNC_INFO << "Couldn't find the ancestor";
   }

   // Add the ancestor's brewnotes to the descendant
   addBrewNoteSubTree(ancestor,ancNdx.row(),pNode);

   return;
}

void BtTreeModel::spawnRecipe(QModelIndex ndx)
{
   Recipe *ancestor = recipe(ndx);
   Recipe* descendant = Database::instance().newRecipe(ancestor,true);

   // First, we remove the ancestor from the tree
   removeRows(ndx.row(),1,this->parent(ndx));

   // Now we need to find the descendant in the tree. This has to be done
   // after we removed the rows.
   QModelIndex decNdx = findElement(descendant);

   emit dataChanged(decNdx,decNdx);
   emit recipeSpawn(descendant);
}

void BtTreeModel::versionedRecipe(Recipe* ancestor, Recipe* descendant)
{
   QModelIndex ndx = findElement(ancestor);

   // like before, remove the ancestor
   removeRows(ndx.row(),1,this->parent(ndx));

   // add the descendant in, but get the index only after we removed the
   // ancestor
   ndx = findElement(descendant);

   // do not mess with this order. We have to signal the data is in the tree
   // first (dataChanged), then put it in the right folder (folderChanged) and
   // finally tell MainWindow something happened (recipeSpawn).
   // Any other order doesn't work, or dumps core
   emit dataChanged(ndx,ndx);
   folderChanged(descendant);
   emit recipeSpawn(descendant);
}

QStringList BtTreeModel::mimeTypes() const
{
   QStringList types;
   // accept whatever type we like, and folders
   types << _mimeType << "application/x-brewtarget-folder";

   return types;
}

Qt::DropActions BtTreeModel::supportedDropActions() const
{
   return Qt::CopyAction | Qt::MoveAction;
}

// =========================================================================
// ===================== RECIPE VERSION STUFF ==============================
// =========================================================================
//
bool BtTreeModel::showChild(QModelIndex child) const
{
   BtTreeItem* node = item(child);
   return node->showMe();
}

void BtTreeModel::setShowChild( QModelIndex child, bool val )
{
   BtTreeItem* node = item(child);
   return node->setShowMe(val);
}

void BtTreeModel::showAncestors(QModelIndex ndx)
{

   if ( ! ndx.isValid() ) {
      return;
   }

   BtTreeItem* node = item(ndx);
   Recipe *descendant = recipe(ndx);
   QList<Recipe*> ancestors = descendant->ancestors();

   removeRows(0,node->childCount(),ndx);

   // add the brewnotes for this version back
   addBrewNoteSubTree(descendant, ndx.row(), node->parent(), false);

   // set showChild on the leaf node. I use this for drawing menus
   setShowChild(ndx,true);

   // Now loop through the ancestors. The nature of the beast is nearest
   // ancestors are first
   foreach( Recipe* ancestor, ancestors ) {
      int j = node->childCount();
      if ( ancestor == descendant ) {
         continue;
      }
      if ( ! insertRow(j, ndx, ancestor, BtTreeItem::RECIPE) ) {
         qWarning() << "Could not add ancestoral brewnotes";
      }
      QModelIndex cIndex = findElement(ancestor,node);
      setShowChild(cIndex,true);
      // ew, but apparently this has to happen here.
      emit dataChanged(cIndex,cIndex);

      // add the brewnotes to the ancestors, but make sure we don't recurse
      addBrewNoteSubTree(ancestor,j,node,false);
   }
}

void BtTreeModel::hideAncestors(QModelIndex ndx)
{
   BtTreeItem* node = item(ndx);

   // This has no potential to be clever. None.
   if ( ! ndx.isValid() ) {
      return;
   }

   // remove all the currently shown children
   removeRows(0,node->childCount(),ndx);
   Recipe *descendant = recipe(ndx);

   // put the brewnotes back, including those from the ancestors.
   addBrewNoteSubTree(descendant,ndx.row(), node->parent());

   // This is for menus
   setShowChild(ndx,false);

   // Now we just need to mark each ancestor invisible again
   foreach( Recipe* ancestor, descendant->ancestors() ) {
      QModelIndex aIndex = findElement(ancestor,node);
      setShowChild(aIndex,false);
      emit dataChanged(aIndex,aIndex);
   }
}

// more cleverness must happen. Wonder if I can figure it out.
void BtTreeModel::catchAncestors(bool showem)
{
   QModelIndex ndxLocal;
   BtTreeItem* local = nullptr;
   QList<NamedEntity*> elems = elements();

   foreach( NamedEntity* elem, elems ) {
      Recipe *rec = qobject_cast<Recipe*>(elem);

      local = rootItem->child(0);
      ndxLocal = findElement(elem, local);

      if ( rec->hasAncestors() ) {
         showem ? showAncestors(ndxLocal) : hideAncestors(ndxLocal);
      }
   }
}
